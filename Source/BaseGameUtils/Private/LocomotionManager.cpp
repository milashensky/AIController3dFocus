// Fill out your copyright notice in the Description page of Project Settings.


#include "LocomotionManager.h"


float CheapLerp(float Start, float Target, float Alpha) {
    if (FMath::Abs(Start - Target) > 0.001) {
        return FMath::Lerp(Start, Target, Alpha);
    } else {
        return Target;
    }
}

void FLocomotionManager::SelectGait() {
    CurrentGaitIndex = 0;
    for (int32 GaitIndex = 0; GaitIndex < AllGaits.Num(); ++GaitIndex) {
        if (AllGaits[GaitIndex].SpeedMin <= Speed) {
            CurrentGaitIndex = GaitIndex;
        }
	}
    isMoving = Speed > 0.0001;
    GaitLegOffsets = AllGaits[CurrentGaitIndex].LegOffsets;
}

void FLocomotionManager::Reset(
    const TArray<FGaitConfig> Gaits,
    const TArray<FRigElementKey>& Legs,
    float lerpSpeed,
    FLegConfig legCofing,
    FStepTraceConfig traceConfig
) {
    LerpSpeed = lerpSpeed;
    CycleDuration = 0.0f;
    DeltaProgress = 0.f;
    TotalCycleProgress = 0.f;
    CurrentGaitIndex = 0;
    AllGaits.Reset(0);
    AllGaits = Gaits;
    AllFeet.Reset(0);
    int32 LegsCount = Legs.Num();
    LegOffsets.Reset(LegsCount);
    FGaitConfig Gait = Gaits[CurrentGaitIndex];
    for (int32 FootIndex = 0; FootIndex < LegsCount; ++FootIndex) {
		AllFeet.Emplace(
            Legs[FootIndex],
            legCofing,
            traceConfig
        );
        AllFeet[FootIndex].Reset();
        LegOffsets.Add(Gait.LegOffsets[FootIndex]);
	}
}

void FLocomotionManager::LerpCycleTimes() {
    const FGaitConfig CurrentGait = AllGaits[CurrentGaitIndex];
    CycleDuration = CheapLerp(CycleDuration, CurrentGait.CycleDuration, LerpSpeed);
    ExpectedStopTime = CheapLerp(ExpectedStopTime, CurrentGait.ExpectedStopTime, LerpSpeed);
}

void FLocomotionManager::ProgressTimer(float DeltaTime) {
    if (!isMoving) {
        TotalCycleProgress = 0.f;
        DeltaProgress = 0.f;
        return;
    }
    if (!CycleDuration) {
        return;
    }
    DeltaProgress = DeltaTime / CycleDuration;
    float progress = fmod(TotalCycleProgress + DeltaProgress, 1.0f);
    TotalCycleProgress = progress;
}

bool FLocomotionManager::CheckIsOffsetNow(float Offset) {
    float WrapAroundOffset = Offset - 1;
    bool isOffsetInRange = (TotalCycleProgress > Offset) && (Offset > LastProgress);
    if (isOffsetInRange) {
        return true;
    }
    bool isWrapAroundOffsetInRange = (TotalCycleProgress > WrapAroundOffset) && (WrapAroundOffset > LastProgress);
    return isWrapAroundOffsetInRange;
}

void FLocomotionManager::ProgressLeg(
    int32 FootIndex,
    FLocomotionProceduralLeg* Leg,
    float DeltaTime,
    URigHierarchy* Hierarchy,
    FTransform WorldTransform
) {
    // set expected air time
    Leg->ExpectedAirTime = CycleDuration;
    Leg->StopTime = ExpectedStopTime;
    Leg->isFalling = isFalling;
    // upd leg offsets
    float LegOffset = LegOffsets[FootIndex];
    float TargetLegOffset = GaitLegOffsets[FootIndex];
    LegOffset = CheapLerp(LegOffset, TargetLegOffset, LerpSpeed);
    LegOffsets[FootIndex] = LegOffset;
    FVector ForwardVector = LastRotation.Quaternion().GetForwardVector();
    Leg->ProcessTick(DeltaTime, Hierarchy, WorldTransform, ForwardVector);
    // check should move
    bool isShouldMove = isMoving && CheckIsOffsetNow(LegOffset);
    if (isShouldMove) {
        Leg->StartMovement();
    }
}

void FLocomotionManager::ProcessTick(
    float DeltaTime,
    URigHierarchy* Hierarchy,
    FTransform WorldTransform,
    bool isFallingIn
) {
    isFalling = isFallingIn;
    SelectGait();
    LerpCycleTimes();
    ProgressTimer(DeltaTime);
    LastProgress = TotalCycleProgress - DeltaProgress;
	for (int32 FootIndex = 0; FootIndex < AllFeet.Num(); ++FootIndex) {
		ProgressLeg(
            FootIndex, 
            &AllFeet[FootIndex], 
            DeltaTime, 
            Hierarchy, 
            WorldTransform
        );
	}
}

FVector GetProgressPoint(
    FVector Center,
    float radius,
    float progress
) {
    float progressRad = progress * PI * 2;
    FVector End = Center + FVector(
        cos(progressRad) * radius,
        sin(progressRad) * radius,
        0
    );
    return End;
}

void FLocomotionManager::DrawDebug(
    FRigVMDrawInterface* DrawInterface,
    FTransform RootTransform,
    FTransform WorldToGlobal
) {
    if (DrawInterface == nullptr) {
        return;
    }
    FLinearColor color = isMoving ? FLinearColor(1, 0, 0) : FLinearColor(0, 0, 1);
    float radius = 100.f;
    DrawInterface->DrawCircle(
        WorldToGlobal,
        RootTransform,
        100.f,
        color,
        2.f,
        12
    );
    FVector Center = RootTransform.GetLocation();
    DrawInterface->DrawLine(
        WorldToGlobal,
        Center,
        GetProgressPoint(Center, radius, 0),
        FColor(0.f, 0.f, 0.f),
        1.0f
    );
    DrawInterface->DrawLine(
        WorldToGlobal,
        Center,
        GetProgressPoint(Center, radius, TotalCycleProgress),
        FColor(0.f, 1.f, 0.f),
        1.0f
    );
  
    for (int32 FootIndex = 0; FootIndex < AllFeet.Num(); ++FootIndex) {
        DrawInterface->DrawPoint(
            WorldToGlobal,
            GetProgressPoint(
                Center, 
                radius, 
                LegOffsets[FootIndex]
            ),
            10.f,
            FColor(1.f, 1.f, 1.f)
        );
        AllFeet[FootIndex].DrawDebug(DrawInterface);
    }
}
