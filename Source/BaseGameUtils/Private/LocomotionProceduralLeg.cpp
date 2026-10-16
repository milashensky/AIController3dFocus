// Fill out your copyright notice in the Description page of Project Settings.


#include "LocomotionProceduralLeg.h"
#include "RigStepNotifyInterface.h"
#include "Async/Async.h"
#include "Rigs/RigHierarchy.h"

double EllipsoidalDistanceMultiplier(
    FVector Start,
    FVector End,
    FVector Forward,
    float YScale
) {
    FVector Delta = End - Start;
    FVector DeltaNormal = Delta.GetSafeNormal();
    FVector ForwardNormal = Forward.GetSafeNormal();
    double dotAbs = abs(ForwardNormal.Dot(DeltaNormal));
    double alpha = FMath::Clamp(dotAbs, 0, 1);
    return FMath::Lerp(YScale, 1, alpha);
}

double GetAngleBetween (FVector A, FVector B) {
    return FMath::Acos(
        FMath::Clamp(
            FVector::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()),
            -1.0f,
            1.0f
        )
    );
}


void FLocomotionProceduralLeg::Reset() {
    isReinitRequired = true;
}

void FLocomotionProceduralLeg::SetGlobals(float DeltaTime, URigHierarchy* Hierarchy) {
    isOverrotating = false;
    if (Hierarchy == nullptr)
    {
        return;
    }
    LastPosition = CurrentPosition;
    CurrentTransform = Hierarchy->GetGlobalTransform(LegElementKey) * CurrentWorldTransform;
    CurrentPosition = CurrentTransform.GetLocation();
    FTransform CurrentInitialTransform = Hierarchy->GetInitialGlobalTransform(LegElementKey) * CurrentWorldTransform;
    CurrentInitialLocation = CurrentInitialTransform.GetLocation();
    // set from parent. matters because used for elipsoidal distance
    // CurrentForward = CurrentInitialTransform.GetRotation().GetForwardVector();
    // if character stoped
    isFullyStopped = Velocity.SquaredLength() < 0.25;
    // update velocity
    Velocity = (CurrentInitialLocation - ProjectedInitialTransform.GetLocation()) / DeltaTime;
    ProjectedInitialTransform = CurrentInitialTransform;
    // if leg is off the ground
    double Distance = FVector::DistSquared(CurrentPosition, LastPosition);
    isLegMoving = FMath::SafeDivide(Distance, DeltaTime) > MovementThresholdSquared;
    FVector TargetPosition = LockTransform.GetLocation();
    FVector CurrentPositionFlat = FVector(CurrentPosition.X, CurrentPosition.Y, 0);
    double TargetDistanceMultiplier = EllipsoidalDistanceMultiplier(
        CurrentPositionFlat,
        FVector(TargetPosition.X, TargetPosition.Y, 0),
        CurrentForward,
        LateralDistanceMultiplier
    );
    double TargetDistanceSquared = FVector::DistSquared(CurrentPosition, TargetPosition);
    isInAir = (TargetDistanceSquared * (TargetDistanceMultiplier * TargetDistanceMultiplier)) > DistanceReachThresholdSquared;
    // Falling state, populated with tick input
    isLanded = !isFalling && isFallingPrev;
    isFallingPrev = isFalling;
    // If currently stable
    FVector CurrentInitialPositionFlat = FVector(CurrentInitialLocation.X, CurrentInitialLocation.Y, 0);
    double CurrentPoseMultiplier = EllipsoidalDistanceMultiplier(
        CurrentInitialPositionFlat,
        CurrentPositionFlat,
        CurrentForward,
        LateralDistanceMultiplier
    );
    double CurrentDistanceSquared = FVector::DistSquared(
        CurrentPositionFlat,
        CurrentInitialPositionFlat
    );

    isStable = (CurrentDistanceSquared * (CurrentPoseMultiplier * CurrentPoseMultiplier)) <= DistanceReachThresholdSquared;
}

void FLocomotionProceduralLeg::ResetTargets() {
    LockTransform = CurrentTransform;
    LastPosition = CurrentPosition;
    isLegMoving = false;
}

void FLocomotionProceduralLeg::UpdateAirTime(float DeltaTime) {
    if (isInAir && isLegMoving) {
        AirTime += DeltaTime;
        return;
    }
    AirTime = 0.f;
}

bool FLocomotionProceduralLeg::CastSphere(const FVector& Start, const FVector& End, float SphereRadius, FHitResult& OutHitResult) {
    if (TraceConfig.World == nullptr) {
        return false;
    }
    if (
        Start.ContainsNaN()
        || End.ContainsNaN()
        || !FMath::IsFinite(SphereRadius)
        || SphereRadius < 0.f
    ) {
        return false;
    }
    const ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceConfig.TraceChannel);
    const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SphereRadius);

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    if (IsValid(TraceConfig.Owner)) {
        QueryParams.AddIgnoredActor(TraceConfig.Owner);
    }
    else if (const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(TraceConfig.OnwerComponent)) {
        QueryParams.AddIgnoredComponent(PrimitiveComponent);
    }
    FCollisionResponseParams ResponseParams(ECR_Ignore);
    ResponseParams.CollisionResponse.SetResponse(ECC_WorldStatic, ECR_Block);
    bool isHit = TraceConfig.World->SweepSingleByChannel(
        OutHitResult,
        Start,
        End,
        FQuat::Identity,
        CollisionChannel,
        CollisionShape,
        QueryParams,
        ResponseParams
    );
    return isHit;
}


FTransform FLocomotionProceduralLeg::ProjectForTarget(FVector StopDistanceVector) {
    double StopDistanceSqrd = StopDistanceVector.SquaredLength();
    if ((StopDistanceSqrd > 0.01) && StopDistanceSqrd < MinStepLengthSquared) {
        StopDistanceVector = StopDistanceVector.GetSafeNormal() * MinStepLength;
    }
    // if just initialized or culled
    if (CurrentInitialLocation.ContainsNaN()) {
        return LockTransform;
    }
    FVector ExpectedTarget = CurrentInitialLocation + StopDistanceVector;
    FTransform target = FTransform(
        ProjectedInitialTransform.GetRotation(),
        ExpectedTarget,
        ProjectedInitialTransform.GetScale3D()
    );
    if (!TraceConfig.isTraceEnabled) {
        return target;
    }
    FHitResult HitResult;
    FVector Start = ExpectedTarget + FVector(0, 0, TraceConfig.TraceHalfHeight);
    FVector End = ExpectedTarget - FVector(0, 0, TraceConfig.TraceHalfHeight);
    const bool isHit = CastSphere(Start, End, TraceConfig.TraceRadius, HitResult);
    if (isHit)
    {
        target.SetLocation(HitResult.ImpactPoint);
        FQuat normaleRotation = FRotationMatrix::MakeFromZX(
            HitResult.ImpactNormal,
            FVector(1, 0, 0)
        ).ToQuat();
        target.SetRotation(target.GetRotation() * normaleRotation);
    }
    return target;
}

bool FLocomotionProceduralLeg::CheckBetterStoppingPosition(FTransform CurrentTarget, FVector PotentialTarget) {
    FVector CurrentTargetLocation = CurrentTarget.GetLocation();
    FVector CurrentDiff = CurrentTargetLocation - CurrentPosition;
    double CurrentDistance = CurrentDiff.SquaredLength();
    FVector PredictedDiff = PotentialTarget - CurrentPosition;
    double PredictedDistance = PredictedDiff.SquaredLength();
    if (PredictedDistance < CurrentDistance) {
        return true;
    }
    // more like if current location is still in front of direction
    bool isSameDirection = CurrentDiff.Dot(PredictedDiff) > 0;
    if (!isSameDirection) {
        return true;
    }
    if (isOverrotating) {
        return true;
    }
    return false;
}

FTransform FLocomotionProceduralLeg::GetStopPosition(const bool isForce) {
    FVector PredictedDistance = Velocity * StopTime;
    FVector PredictedLocation = CurrentInitialLocation + PredictedDistance;
    if (!isFullyStopped && !isForce) {
        bool isBetter = CheckBetterStoppingPosition(
            LockTransform,
            PredictedLocation
        );
        if (!isBetter) {
            return LockTransform;
        }
    }
    return ProjectForTarget(PredictedDistance);
}

void FLocomotionProceduralLeg::Stop(const bool isForce) {
    // ?? we still need to calculate if "stable" because of in air langing
    // so maybe just if falling ?
    if (isFalling && isStable) {
        if (!isForce) {
            return;
        }
    }
    LockTransform = GetStopPosition(isForce);
    const AActor* Owner = TraceConfig.Owner;

    double distance = FVector::DistSquared(LastNotifiedTransform.GetLocation(), LockTransform.GetLocation());
    LastNotifiedTransform = LockTransform;
    if (isFalling || distance < 10) {
        return;
    }
    FTransform SoundTransform = LockTransform;
    AsyncTask(ENamedThreads::GameThread, [Owner, SoundTransform]()
    {
        if (!IsValid(Owner))
        {
            return;
        }

        if (Owner->Implements<URigStepNotifyInterface>())
        {
            IRigStepNotifyInterface::Execute_NotifyRigStep(
                const_cast<AActor*>(Owner),
                SoundTransform
            );
        }
    });
}

void FLocomotionProceduralLeg::StartMovement() {
    Stop(true);
}

void FLocomotionProceduralLeg::RetargetLock() {
    if (isFalling) {
        return;
    }
    bool isStepping = isLegMoving || isInAir;
    bool isStopping = Velocity.SquaredLength() < 1;
    if (
        isLanded
        || (AirTime > ExpectedAirTime)
        || (AirTime > MaxAirTime)
        || (isStepping && isStopping)
        || (isFullyStopped && !isStable)
    ) {
        isLegMoving = false;
        Stop();
        return;
    }
    FQuat LockRotation = LockTransform.GetRotation();
    FQuat InitialRotation = ProjectedInitialTransform.GetRotation();
    // TODO: check correct. original was somewhat different
    float rightAngleRad = GetAngleBetween(LockRotation.GetForwardVector(), InitialRotation.GetForwardVector());
    float forwardAngleRad = GetAngleBetween(LockRotation.GetRightVector(), InitialRotation.GetRightVector());
    if (
        rightAngleRad > MaxTurnAngeRad
        || forwardAngleRad > MaxTurnAngeRad
    ) {
        isOverrotating = true;
        StartMovement();
    }
}

void FLocomotionProceduralLeg::ProcessTick(float DeltaTime, URigHierarchy* Hierarchy, FTransform WorldTransform, FVector RootForward) {
    CurrentWorldTransform = WorldTransform;
    CurrentForward = RootForward.GetSafeNormal();
    if (isReinitRequired) {
        SetGlobals(DeltaTime, Hierarchy);
        ResetTargets();
        isReinitRequired = false;
        return;
    }
    UpdateAirTime(DeltaTime);
    SetGlobals(DeltaTime, Hierarchy);
    RetargetLock();
}

void FLocomotionProceduralLeg::DrawDebug(FRigVMDrawInterface* DrawInterface) {
    FTransform WorldToGlobal = CurrentWorldTransform.Inverse();
    FLinearColor color = isStable ? FLinearColor(0.f, 1.f, 0.f) : FLinearColor(1.f, 0.f, 0.f);
    float radius = 100.f;
    // lock
    DrawInterface->DrawCircle(
        WorldToGlobal,
        LockTransform,
        20.f,
        color,
        1.f,
        12
    );
    // inital with reach
    float reach = sqrt(DistanceReachThresholdSquared);
    DrawInterface->DrawSphere(
        WorldToGlobal,
        ProjectedInitialTransform,
        reach,
        FLinearColor(.2f, 1.f, 0.f),
        0.5f,
        12
    );
    // current position
    DrawInterface->DrawBox(
        WorldToGlobal,
        FTransform(
            CurrentTransform.Rotator(),
            CurrentPosition,
            CurrentTransform.GetScale3D() * 10
        ),
        FLinearColor(.2f, 0.f, 1.f)
    );
    // Forward
    const FVector CenterGlobal = WorldToGlobal.TransformPosition(CurrentInitialLocation);
    const FVector ForwardGlobal = WorldToGlobal.TransformVectorNoScale(CurrentForward).GetSafeNormal();
    const FVector SideGlobal = WorldToGlobal.TransformVectorNoScale(FVector::UpVector).GetSafeNormal();
    const FTransform DrawOffset(FQuat::Identity, CenterGlobal);
    DrawInterface->DrawArrow(
        DrawOffset,
        ForwardGlobal * 10.0f,
        SideGlobal,
        FLinearColor(.2f, 1.f, 0.f),
        1.f
    );
}
