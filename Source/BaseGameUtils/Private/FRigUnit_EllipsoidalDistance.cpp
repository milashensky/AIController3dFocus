


#include "FRigUnit_EllipsoidalDistance.h"

FRigUnit_EllipsoidalDistance_Execute() 
{
    FVector Dir = (B - A).GetSafeNormal();
    FVector Fwd = Forward.GetSafeNormal();

    float Dot = FVector::DotProduct(Fwd, Dir);
    float Alpha = FMath::Abs(Dot);

    float Scale = FMath::Lerp(Multiplier, 1.0f, Alpha);
    Result = FVector::Dist(A, B) * Scale;
}