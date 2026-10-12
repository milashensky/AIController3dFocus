// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LocomotionProceduralLeg.h"
#include "LocomotionManager.generated.h"


USTRUCT()
struct BASEGAMEUTILS_API FGaitConfig
{
	GENERATED_BODY()

	FGaitConfig() :
		CycleDuration(0.6f),
		ExpectedStopTime(0.3f),
		SpeedMin(0.0f) {
	}

	// how long is a single simulation cycle, sec
	UPROPERTY(meta = (Input))
	float CycleDuration = 0.6f;

	// the how much time leg can stay in the air without landing usually, sec
	UPROPERTY(meta = (Input))
	float ExpectedStopTime = 0.3f;

	// the minimum movement speed in cm/s
	UPROPERTY(meta = (Input))
	float SpeedMin = 0.0f;

	// cycle start timings for each leg, sec
	UPROPERTY(meta = (Input))
	TArray<float> LegOffsets;
};

/**
 *
 */
USTRUCT()
struct FLocomotionManager
{
	GENERATED_BODY()

public:
	FLocomotionManager() = default;

    void ProcessTick(
        float DeltaTime,
        URigHierarchy* Hierarchy,
        FTransform WorldTransform,
        bool isFalling
    );
    void DrawDebug(FRigVMDrawInterface* DrawInterface, FTransform RootTransform, FTransform WorldToGlobal);
    void Reset(
        const TArray<FGaitConfig> Gaits,
        const TArray<FRigElementKey>& Legs,
        float lerpSpeed,
        FLegConfig legCofing,
        FStepTraceConfig traceConfig
    );

    // get final foot transforms
    void GetFeetCurrent(TArray<FTransform> OutTransforms) const;

    // get access to all feet
    TArray<FLocomotionProceduralLeg>& GetFeet() {return AllFeet; };
    const TArray<FLocomotionProceduralLeg>& GetFeet() const {return AllFeet; };
    bool HasFeet() const { return !AllFeet.IsEmpty(); };

    float Speed;
    FRotator LastRotation;
    FVector LastLocation;

private:
    bool CheckIsOffsetNow(float Offset);
    void ProgressLeg(
        int32 FootIndex,
        FLocomotionProceduralLeg* Leg,
        float DeltaTime,
        URigHierarchy* Hierarchy,
        FTransform WorldTransform
    );
    void ProgressTimer(float DeltaTime);
    void LerpCycleTimes();
    void SelectGait();

    TArray<FLocomotionProceduralLeg> AllFeet;
    TArray<FGaitConfig> AllGaits;
    int32 CurrentGaitIndex;
    float DeltaProgress;
    float TotalCycleProgress;
    float CycleDuration = 1.f;
    float ExpectedStopTime;
    float LerpSpeed = 0.01f;
    float LastProgress = 0.f;
    bool isMoving;
    bool isFalling;
    TArray<float> LegOffsets;
    TArray<float> GaitLegOffsets;
};
