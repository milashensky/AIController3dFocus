#include "SensingCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "AITypes.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASensingCharacter::ASensingCharacter(const FObjectInitializer& ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASensingCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASensingCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASensingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASensingCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (!UseSenseSocket) {
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
		return;
	}
	if (SenseSocketBoneName == "") {
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
		return;
	}
	const  USkeletalMeshComponent* mesh = GetMesh();
	OutLocation = mesh->GetSocketLocation(SenseSocketBoneName);
	OutRotation = mesh->GetSocketRotation(SenseSocketBoneName);
	if (!AdjustForInvalid) {
		return;
	}
	// sometime if character stands in place, AIController sets invalid focus position
	// and with aim offset it might cause them to look at the sky wierdly.
	AAIController* AIController = Cast<AAIController>(GetController());
	if (IsValid(AIController)) {
		FVector FocusLocation = AIController->GetFocalPoint();
		if (
			!AIController->GetFocusActor()
			&& (
				!FAISystem::IsValidLocation(FocusLocation)
				|| !FAISystem::IsValidDirection(FocusLocation)
			)
		) {
			OutRotation.Pitch = 0.f;
		}
	}
}
