// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SensingCharacter.h"
#include "GameFramework/Character.h"
#include "GrabbingSensingCharacter.generated.h"

class UGrabCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class BASEGAMEUTILS_API AGrabbingSensingCharacter : public ASensingCharacter
{
	GENERATED_BODY()

public:

    AGrabbingSensingCharacter(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable)
    UGrabCharacterMovementComponent* GetGrabMovementComponent() const;
};
