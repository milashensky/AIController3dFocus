// Fill out your copyright notice in the Description page of Project Settings.

#include "GrabCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

// Moves grabber based on attached actor and optional Z handling
void UGrabCharacterMovementComponent::MoveGrabberTowardsAttached(float DeltaTime, bool bClampZ)
{
    if (!AttachedActor)
    {
        return;
    }

    FTransform TargetTransform = GrabLocalOffset * AttachedActor->GetActorTransform();
    FVector PreUpdateLocation = UpdatedComponent->GetComponentLocation();
    FVector TargetLocation = TargetTransform.GetLocation();
    TargetLocation.Z = PreUpdateLocation.Z;
    FVector MoveDelta = TargetLocation - PreUpdateLocation;
    FQuat TargetQuat = TargetTransform.GetRotation();

    // Move with sweep
    FHitResult Hit;
    SafeMoveUpdatedComponent(MoveDelta, TargetQuat, true, Hit);
    if (Hit.IsValidBlockingHit())
        SlideAlongSurface(MoveDelta, 1.f - Hit.Time, Hit.Normal, Hit, true);

    FVector AttachedLocation = AttachedActor->GetActorLocation();
    // Clamp Z and vertical position to the leash length if requested
    FVector NewLocation = UpdatedComponent->GetComponentLocation();
    if (bClampZ)
    {
        float ZDiff = FMath::Abs(AttachedLocation.Z - NewLocation.Z);
        if (ZDiff > GrabMaxRelativeFall)
        {
            NewLocation.Z = FMath::Clamp(
                NewLocation.Z,
                AttachedLocation.Z - GrabMaxRelativeFall,
                AttachedLocation.Z + GrabMaxRelativeFall
            );
            UpdatedComponent->SetWorldLocation(NewLocation, false);
        }
    }

    // Compute velocity using the **possibly clamped location**
    // will drop vertical z if location unchanged
    // we've changed the NewLoc, we don't need to recompute it.
    Velocity = (NewLocation - LastLocation) / DeltaTime;
    LastLocation = NewLocation;
    // make actor look at grabbed actor
    FVector FaceDir = AttachedLocation - NewLocation;
    FaceDir.Z = 0.f;
    if (!FaceDir.IsNearlyZero())
    {
        FRotator DesiredRot = FaceDir.Rotation();
        UpdatedComponent->SetWorldRotation(DesiredRot);
    }
}

void UGrabCharacterMovementComponent::OnMovementModeChanged(EMovementMode PrevMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMode, PreviousCustomMode);

    if (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)EGrabMovementMode::GrabAttached)
    {
        if (AttachedActor)
        {
            GrabLocalOffset = UpdatedComponent->GetComponentTransform().GetRelativeTransform(AttachedActor->GetActorTransform());
            PreviousAttachedLocation = AttachedActor->GetActorLocation();
            LastLocation = UpdatedComponent->GetComponentLocation();
            bWasInGrabModeBeforeFall = false;
        }
    }
}

void UGrabCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (CustomMovementMode == (uint8)EGrabMovementMode::GrabAttached)
    {
        FFindFloorResult Floor;
        FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);
        if (!Floor.IsWalkableFloor())
        {
            bWasInGrabModeBeforeFall = true;
            SetMovementMode(MOVE_Falling);
            return;
        }
        MoveGrabberTowardsAttached(DeltaTime, false);
        return;
    }
    Super::PhysCustom(DeltaTime, Iterations);
}

void UGrabCharacterMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
    Super::PhysFalling(DeltaTime, Iterations);

    if (!AttachedActor)
    {
        return;
    }
    MoveGrabberTowardsAttached(DeltaTime, true);
    FFindFloorResult Floor;
    FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);
    if (bWasInGrabModeBeforeFall && Floor.IsWalkableFloor() && AttachedActor)
    {
        SetMovementMode(MOVE_Custom, (uint8)EGrabMovementMode::GrabAttached);
        bWasInGrabModeBeforeFall = false;
    }
}