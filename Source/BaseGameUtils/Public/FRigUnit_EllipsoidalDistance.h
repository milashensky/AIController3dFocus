

#pragma once

#include "CoreMinimal.h"
#include "Units/RigUnit.h"
#include "FRigUnit_EllipsoidalDistance.generated.h"

USTRUCT(meta = (DisplayName = "Ellipsoidal Distance", Category = "Math|Vector"))
struct FRigUnit_EllipsoidalDistance : public FRigUnit
{
    GENERATED_BODY()

    FRigUnit_EllipsoidalDistance()
        : A(), B(), Forward(), Multiplier(2.f), Result(0.0f) {
    }

    RIGVM_METHOD()
    virtual void Execute() override;

    UPROPERTY(meta = (Input))
    FVector A;
    
    UPROPERTY(meta = (Input))
    FVector B;
    
    UPROPERTY(meta = (Input))
    FVector Forward;
    
    UPROPERTY(meta = (Input))
    float Multiplier = 2.f;

    UPROPERTY(meta = (Output))
    float Result;
};