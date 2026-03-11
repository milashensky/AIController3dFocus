// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GrabCharacterMovementComponent.generated.h"


UENUM(BlueprintType)
enum class EGrabMovementMode : uint8
{
	GrabAttached UMETA(DisplayName = "GrabAttached")
};


/**
 * a character movement component with a grab mode: follows attached actor
 */
UCLASS()
class BASEGAMEUTILS_API UGrabCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

    // Actor being grabbed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    AActor* AttachedActor;

    // Maximum allowed vertical offset below attached actor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
    float GrabMaxRelativeFall = 75.f;

protected:

    // Grabber transform relative to attached actor at grab
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab")
    FTransform GrabLocalOffset;

    // For calculating delta velocity
    FVector LastLocation;
    // Previous attached actor location
    FVector PreviousAttachedLocation;
    // for mode restoration
    bool bWasInGrabModeBeforeFall;

    virtual void OnMovementModeChanged(EMovementMode PrevMode, uint8 PreviousCustomMode) override;
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
    virtual void PhysFalling(float DeltaTime, int32 Iterations) override;

    void MoveGrabberTowardsAttached(float DeltaTime, bool bIgnoreZ);
    FVector ClampVelocityToMax(const FVector& InVelocity) const;
};
