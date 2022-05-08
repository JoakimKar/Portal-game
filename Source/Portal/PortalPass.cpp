// Copyright Joakim Karlsson 2022


#include "PortalPass.h"
#include "MathLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PortalCharacter.h"
#include "PortalPlayerController.h"

// Sets default values
APortalPass::APortalPass()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(RootComponent);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("Portal Mesh"));
	PortalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PortalMesh->SetupAttachment(RootComponent);

	PortalCollision = CreateDefaultSubobject<UBoxComponent>(FName("Portal Collision"));
	PortalCollision->SetBoxExtent(FVector(100, 100, 175));
	PortalCollision->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void APortalPass::BeginPlay()
{
	Super::BeginPlay();

	PortalCollision->OnComponentBeginOverlap.AddDynamic(this, &APortalPass::OnPortalOverlapBegin);
	PortalCollision->OnComponentEndOverlap.AddDynamic(this, &APortalPass::OnPortalOverlapEnd);
}

// Called every frame
void APortalPass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Overlapping)
		TeleportActors();
}

void APortalPass::OnPortalOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(APortalCharacter::StaticClass()))
	{
		Overlapping = true;

		if (PortalSurface != nullptr)
			PortalSurface->SetActorEnableCollision(false);
	}
}

void APortalPass::OnPortalOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(APortalCharacter::StaticClass()))
	{
		Overlapping = false;

		if (PortalSurface != nullptr)
			PortalSurface->SetActorEnableCollision(true);
	}
}

void APortalPass::SetTarget(APortalPass* NewTarget)
{
	if (NewTarget != nullptr)
		Target = NewTarget;
}

void APortalPass::SetPortalSurface(AActor* Surface)
{
	PortalSurface = Surface;
}

AActor* APortalPass::GetPortalSurface() const
{
	return PortalSurface;
}

APortalPass* APortalPass::GetTarget() const
{
	return Target;
}

void APortalPass::TeleportActors()
{
	if (Target != nullptr)
	{
		APortalPlayerController* PlayerController =
			Cast<APortalPlayerController>(GetWorld()->GetFirstPlayerController());

		if (PlayerController != nullptr)
		{
			APortalCharacter* Character =
				Cast<APortalCharacter>(PlayerController->GetCharacter());

			if (Character != nullptr)
			{

				if (UMathLibrary::CheckIsCrossing(Character->GetActorLocation(),
					GetActorLocation(), GetActorForwardVector(), bLastInFront, LastPosition))
				{
					FVector SavedVelocity = Character->GetCharacterMovement()->Velocity;

					FHitResult HitResult;

					FVector ConvertedLocation =
						UMathLibrary::ConvertLocation(Character->GetActorLocation(),
							this, Target);

					FRotator ConvertedRotation =
						UMathLibrary::ConvertRotation(Character->GetActorRotation(),
							this, Target);

					Character->SetActorLocationAndRotation(ConvertedLocation, ConvertedRotation,
						false, &HitResult, ETeleportType::TeleportPhysics);

					FVector NewVelocity =
						FVector::DotProduct(SavedVelocity, GetActorForwardVector()) *
						Target->GetActorForwardVector() +
						FVector::DotProduct(SavedVelocity, GetActorRightVector()) *
						Target->GetActorRightVector() +
						FVector::DotProduct(SavedVelocity, GetActorUpVector()) *
						Target->GetActorUpVector();

					Character->GetCharacterMovement()->Velocity = -NewVelocity;

					LastPosition = ConvertedLocation;

					Overlapping = false;
				}
			}
		}
	}
}
