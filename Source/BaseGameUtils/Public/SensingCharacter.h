// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SensingCharacter.generated.h"

UCLASS()
class BASEGAMEUTILS_API ASensingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASensingCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetActorEyesViewPoint(
		FVector& OutLocation,
		FRotator& OutRotation
	) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	FName SenseSocketBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	bool UseSenseSocket = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	bool AdjustForInvalid = true;
};
