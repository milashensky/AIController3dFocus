// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabbingSensingCharacter.h"
#include "GrabCharacterMovementComponent.h"

AGrabbingSensingCharacter::AGrabbingSensingCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UGrabCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
}

UGrabCharacterMovementComponent* AGrabbingSensingCharacter::GetGrabMovementComponent() const
{
    return Cast<UGrabCharacterMovementComponent>(GetCharacterMovement());
}