// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Units/RigUnit.h"
#include "Units/RigUnitContext.h"
#include "Units/Highlevel/RigUnit_HighlevelBase.h"
#include "LocomotionManager.h"
#include "LocomotionProceduralLeg.h"

#include "FRG_RigLocomotionManager.generated.h"


USTRUCT()
struct BASEGAMEUTILS_API FLegManagerConfig
{
	GENERATED_BODY()

	FLegManagerConfig() :
		LerpSpeed(0.15f) {
	}

	// speed of gait interp
	UPROPERTY(meta = (Input))
	float LerpSpeed = 0.15f;

	// settings for gaits
	UPROPERTY(meta = (Input))
	TArray<FGaitConfig> gaits;

    // leg controls
	UPROPERTY(meta = (Input))
	TArray<FRigElementKey> legs;
};

/**
 * control rig based step target position functionality
 */
USTRUCT(meta = (DisplayName = "Locomotion Manager", Category = "Simulation", Keywords = "Foot placement control rig unit"))
struct BASEGAMEUTILS_API FRG_RigLocomotionManager : public FRigUnit_HighlevelBaseMutable
{
	GENERATED_BODY()

	RIGVM_METHOD()
	virtual void Execute() override;

	FRG_RigLocomotionManager() :
		CachedRootElement(FCachedRigElement()) {}

	// root bone placement
	UPROPERTY(meta = (Input))
	FRigElementKey RootControl;

    // general settings
	UPROPERTY(meta = (Input))
	FLegManagerConfig Config;

    // trace settings
	UPROPERTY(meta = (Input))
	FStepTraceConfig TraceConfig;

    // legs settings
	UPROPERTY(meta = (Input))
	FLegConfig LegConfig;

	// Used to cache the root element
	UPROPERTY()
	FCachedRigElement CachedRootElement;

	// list of limbs to walk on.
	UPROPERTY(meta = (Output, DefaultArraySize = 2))
	TArray<FTransform> FeetTransforms;

    // if currently falling. stops simulation
	UPROPERTY(meta = (Input))
	bool isFalling = false;

	// if should draw debug
	UPROPERTY(meta = (Input))
	bool isDrawDebug;

    // the locomotion simulator
    UPROPERTY(Transient)
    FLocomotionManager Manager;
};
