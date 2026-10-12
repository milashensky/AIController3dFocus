// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Units/RigUnitContext.h"
#include "LocomotionProceduralLeg.generated.h"

USTRUCT()
struct BASEGAMEUTILS_API FLegConfig
{
	GENERATED_BODY()

	FLegConfig() :
		MinStepLength(50.f),
		MaxTurnAngle(60.f),
		PositionStabilityThreshold(40.f),
		LateralDistanceMultiplier(1.f),
		MaxAirTime(.8f) {
	}

	// min step in sm
	UPROPERTY(meta = (Input))
	float MinStepLength = 50.f;

	// max angle in deg until we reposition
	UPROPERTY(meta = (Input))
	float MaxTurnAngle = 60.f;

	// how far initial position from current is until we deem it unstable.
	// or close to the target until we're stable
	UPROPERTY(meta = (Input))
	float PositionStabilityThreshold = 40.f;

	// how much lateral distance is multiplied by in distance calculations
	UPROPERTY(meta = (Input))
	float LateralDistanceMultiplier = 1.f;

	// how long leg can stay in air max, sec
	UPROPERTY(meta = (Input))
	float MaxAirTime = .8f;
};

USTRUCT()
struct BASEGAMEUTILS_API FStepTraceConfig
{
	GENERATED_BODY()

	FStepTraceConfig() :
		TraceHalfHeight(40.f),
		TraceRadius(5.f),
		isTraceEnabled(true),
		TraceChannel(ETraceTypeQuery::TraceTypeQuery1) {
	}

	// Half height of trace capsule
	UPROPERTY(meta = (Input))
	float TraceHalfHeight = 40.f;

	// TraceRadius
	UPROPERTY(meta = (Input))
	float TraceRadius = 5.f;

	// if should trace at all
	UPROPERTY(meta = (Input))
	bool isTraceEnabled;

	// trace channel, visiblity
	UPROPERTY(meta = (Input))
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	const AActor* Owner = nullptr;
	const UActorComponent* OnwerComponent = nullptr;
	// the world to collide against
	const UWorld* World = nullptr;
};

/**
 *
 */
USTRUCT()
struct FLocomotionProceduralLeg
{
	GENERATED_BODY()

	FLocomotionProceduralLeg() = default;
	FLocomotionProceduralLeg(
		const FRigElementKey& InLegControlElement,
		FLegConfig legConfig,
		FStepTraceConfig inTraceConfig
	): TraceConfig(inTraceConfig), LegElementKey(InLegControlElement) {
		DistanceReachThresholdSquared = legConfig.PositionStabilityThreshold * legConfig.PositionStabilityThreshold;;
		/*MovementThresholdSquared = */
		MaxTurnAngeRad = FMath::DegreesToRadians(legConfig.MaxTurnAngle);
		MaxAirTime = legConfig.MaxAirTime;
		LateralDistanceMultiplier = legConfig.LateralDistanceMultiplier;
		MinStepLength = legConfig.MinStepLength;
		MinStepLengthSquared = MinStepLength * MinStepLength;
    }

public:
    void StartMovement();
	void ProcessTick(
		float DeltaTime,
		URigHierarchy* Hierarchy,
		FTransform WorldTransform,
		FVector RootForward
	);
	void DrawDebug(FRigVMDrawInterface* DrawInterface);
	void Reset();

    float ExpectedAirTime = 0.5f;
    float StopTime = 0.01f;
	bool isFalling = false;
    FTransform LockTransform;

private:
	void SetGlobals(float DeltaTime, URigHierarchy* Hierarchy);
	void ResetTargets();
	void UpdateAirTime(float DeltaTime);
	bool CastSphere(const FVector& Start, const FVector& End, float SphereRadius, FHitResult& OutHitResult);
	bool CheckBetterStoppingPosition(FTransform CurrentTarget, FVector PotentialTarget);
	FTransform ProjectForTarget(FVector StopDistanceVector);
	FTransform GetStopPosition(const bool isForce);
	void Stop(bool isForce = false);
	void RetargetLock();

    bool isInAir;
    bool isLegMoving;
    bool isLanded;
    bool isStable;
    bool isFullyStopped;
    bool isReinitRequired = true;
	bool isFallingPrev = false;
	bool isOverrotating = false;

    FVector Velocity;
    FVector CurrentPosition;
    FVector CurrentForward;
    FVector LastPosition;
	FVector CurrentInitialLocation;
    FTransform ProjectedInitialTransform;
    FTransform CurrentTransform;
	FTransform CurrentWorldTransform;
	FTransform LastNotifiedTransform;

    float MovementThresholdSquared = 50.f;
    float DistanceReachThresholdSquared = 2500.f;
	float LateralDistanceMultiplier = 1.f;
    float MaxTurnAngeRad = 1.f;
	float MaxAirTime = 0.9f;
    float AirTime = 0.0f;
	float MinStepLengthSquared = 2500.0f;
	float MinStepLength = 50.0f;
	FStepTraceConfig TraceConfig;
    FRigElementKey LegElementKey;
};
