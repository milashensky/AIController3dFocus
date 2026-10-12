// Fill out your copyright notice in the Description page of Project Settings.


#include "FRG_RigLocomotionManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Units/RigUnitContext.h"
#include "Math/ControlRigMathLibrary.h"

FRG_RigLocomotionManager_Execute()
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_RIGUNIT()

	// awaiting hierarchy
	URigHierarchy* Hierarchy = ExecuteContext.Hierarchy;
	if (Hierarchy == nullptr)
	{
		return;
	}

	bool isInitRequired = !Manager.HasFeet();
	if (FeetTransforms.Num() != Manager.GetFeet().Num()) {
		isInitRequired = true;
	}

	FTransform ToWorld = ExecuteContext.GetToWorldSpaceTransform();
	const FTransform RootTransform = Hierarchy->GetGlobalTransform(RootControl) * ToWorld;
	const FRotator CurrentRotation = RootTransform.Rotator();
	const FVector CurrentLocation = RootTransform.GetLocation();
	const float DeltaTime = ExecuteContext.GetDeltaTime();

    if (isInitRequired) {
		int32 legsCount = Config.legs.Num();
		FStepTraceConfig traceConfigIn = FStepTraceConfig(TraceConfig);
		traceConfigIn.Owner = ExecuteContext.GetOwningActor();
		traceConfigIn.OnwerComponent = ExecuteContext.GetOwningComponent();
		traceConfigIn.World = ExecuteContext.GetWorld();
		Manager.Reset(
			Config.gaits, 
			Config.legs,
			Config.LerpSpeed,
			LegConfig,
			traceConfigIn
		);
        Manager.LastRotation = CurrentRotation;
		Manager.LastLocation = CurrentLocation;
		Manager.Speed = 0.f;
		FeetTransforms.Reset(legsCount);
		FeetTransforms.SetNumZeroed(legsCount);
	}
	else {
		double speed = ((CurrentLocation - Manager.LastLocation) / DeltaTime).Length();
		// if we rotate but without speed, we still want to enforce movement
		float RotationPart = FMath::Abs(CurrentRotation.Yaw - Manager.LastRotation.Yaw) / 1000;
		Manager.Speed = speed + RotationPart;
		Manager.LastRotation = CurrentRotation;
		Manager.LastLocation = CurrentLocation;
	}
	Manager.ProcessTick(DeltaTime, Hierarchy, ToWorld, isFalling);
    // output the resulting foot transforms
	const TArray<FLocomotionProceduralLeg>& AllFeet = Manager.GetFeet();
	for (int32 FootIndex = 0; FootIndex < AllFeet.Num(); ++FootIndex) {
		FeetTransforms[FootIndex] = ExecuteContext.ToVMSpace(AllFeet[FootIndex].LockTransform);
	}

	if (isDrawDebug) {
		FRigVMDrawInterface* DrawInterface = ExecuteContext.GetDrawInterface();
		Manager.DrawDebug(DrawInterface, RootTransform, ToWorld.Inverse());
	}
}
