// Copyright Joakim Karlsson 2022


#include "PortalManager.h"
#include "PortalPass.h"
#include "PortalCharacter.h"
#include "PortalPlayerController.h"
#include "MathLibrary.h"
#include "EngineUtils.h"

// Sets default values
APortalManager::APortalManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(RootComponent);
}

// Called when the game starts or when spawned
void APortalManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APortalManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APortalManager::SetControllerOwner(APortalPlayerController* NewOwner)
{
	OwningController = NewOwner;
}

void APortalManager::SetPortalClass(TSubclassOf<class APortalPass> Class)
{
	PortalClass = Class;
}

void APortalManager::SpawnBluePortal(const FVector& Start, const FVector& End)
{
	if (BluePortal != nullptr)
	{
		RedPortal->SetTarget(nullptr);
		BluePortal->Destroy();
	}

	BluePortal = SpawnPortal(RedPortal, Start, End);
}

void APortalManager::SpawnRedPortal(const FVector& Start, const FVector& End)
{
	if (RedPortal != nullptr)
	{
		BluePortal->SetTarget(nullptr);
		RedPortal->Destroy();
	}

	RedPortal = SpawnPortal(BluePortal, Start, End);
}

APortalPass* APortalManager::SpawnPortal(APortalPass* Target, const FVector& Start, const FVector& End)
{
	APortalPass* Portal = nullptr;

	UWorld* const World = GetWorld();

	if (World != nullptr && PortalClass != nullptr)
	{
		FHitResult OutHit;

		if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility))
		{
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			FVector Origin = OutHit.Location + OutHit.ImpactNormal;
			FRotator Rotation = OutHit.ImpactNormal.Rotation();

			if (FVector::DotProduct(OutHit.ImpactNormal, FVector::UpVector) > KINDA_SMALL_NUMBER)
			{
				Rotation.Roll -= GetActorRotation().Yaw;
			}
			else if (FVector::DotProduct(OutHit.ImpactNormal, FVector::UpVector) < -KINDA_SMALL_NUMBER)
			{
				Rotation.Roll += GetActorRotation().Yaw;
			}

			Portal = World->SpawnActor<APortalPass>(PortalClass, Origin, Rotation, ActorSpawnParams);
			Portal->SetPortalSurface(OutHit.GetActor());

			if (Target != nullptr)
			{
				Portal->SetTarget(Target);
				Target->SetTarget(Portal);
			}

			if (VerifyPortalPlacement(Portal, Origin))
			{
				Portal->SetActorLocation(Origin);
			}
			else
			{
				Target->SetTarget(nullptr);
				Portal->Destroy();
			}
		}
	}

	return Portal;
}

bool APortalManager::VerifyPortalPlacement(const APortalPass* Portal, FVector& Origin)
{
	FVector OriginalOrigin = Origin;

	FVector Forward = Portal->GetActorForwardVector();
	FVector Right = Portal->GetActorRightVector();
	FVector Up = Portal->GetActorUpVector();

	const AActor* Surface = Portal->GetPortalSurface();
	
	if (!Surface->GetVelocity().IsNearlyZero())
		return false;

	const APortalPass* Target = Portal->GetTarget();
	if (Target != nullptr)
	{
		FitPortalAroundTargetPortal(Portal, Target, Forward, Right, Up, Origin);
	}

	FVector PortalExtent = Portal->CalculateComponentsBoundingBoxInLocalSpace().GetExtent();
	const FVector TopEdge = Up * PortalExtent.Z;
	const FVector BottomEdge = -TopEdge;
	const FVector RightEdge = Right * PortalExtent.Y;
	const FVector LeftEdge = -RightEdge;

	if (!FitPortalOnSurface(Portal, Forward, Right, Up, TopEdge,
		BottomEdge, RightEdge, LeftEdge, Origin))
	{
		return false;
	}

	return true;
}

void APortalManager::FitPortalAroundTargetPortal(const APortalPass* Portal, const APortalPass* Target, const FVector& Forward, const FVector& Right, const FVector& Up, FVector& Origin)
{
	FVector TargetForward = Target->GetActorForwardVector();

	if (FVector::DotProduct(Forward, TargetForward) > 1.0f - KINDA_SMALL_NUMBER)
	{
		FVector Distance = Origin - Target->GetActorLocation();
		FVector RightProjection = FVector::DotProduct(Distance, Right) * Right;
		FVector UpProject = FVector::DotProduct(Distance, Up) * Up;

		float RightProjectionLength = RightProjection.Size();
		float UpProjectionLenght = UpProject.Size();

		if (RightProjectionLength < 1.0f)
			RightProjection = Right;

		FVector Size = Portal->CalculateComponentsBoundingBoxInLocalSpace().GetSize();
		if (UpProjectionLenght < Size.Z && RightProjectionLength < Size.Y)
		{
			RightProjection.Normalize();
			Origin += RightProjection * (Size.Y - RightProjectionLength + 1.0f);
		}
	}
}

bool APortalManager::FitPortalOnSurface(const APortalPass* Portal, const FVector& Forward, const FVector& Right, const FVector& Up, const FVector& TopEdge, const FVector& BottomEdge, const FVector& RightEdge, const FVector& LeftEdge, FVector& Origin, int RecursionCount)
{
	if (RecursionCount == 0)
		return false;

	TArray<FVector> Corners;
	Corners.SetNumUninitialized(4);
	
	Corners[0] = Origin + TopEdge + LeftEdge;
	Corners[1] = Origin + TopEdge + RightEdge;
	Corners[2] = Origin + BottomEdge + LeftEdge;
	Corners[3] = Origin + BottomEdge + RightEdge;

	FHitResult HitResult;
	for (int i = 0; i < 4; ++i)
	{
		if(TraceCorner(Portal, Origin, Corners[i], Forward, Right, Up, HitResult))
		{
			float Distance = FMath::PointDistToLine(Corners[i],
				FVector::CrossProduct(HitResult.Normal, Forward), HitResult.Location) + 5.0f;

			Origin += HitResult.Normal * Distance;

			return FitPortalOnSurface(Portal, Forward, Right, Up, TopEdge,
				BottomEdge, RightEdge, LeftEdge, Origin, RecursionCount - 1);
		}
	}

	return true;
}

bool APortalManager::TraceCorner(const APortalPass* Portal, const FVector& Start, const FVector& End, const FVector& Forward, const FVector& Right, const FVector& Up, FHitResult& HitResult)
{
	bool bFountHit = false;

	FCollisionQueryParams TraceParams;
	TraceParams.bFindInitialOverlaps = true;
	TraceParams.bTraceComplex = true;
	TraceParams.AddIgnoredActor(Portal);
	TraceParams.AddIgnoredActor(Portal->GetPortalSurface());

	if (OwningController != nullptr)
		TraceParams.AddIgnoredActor(OwningController->GetCharacter());

	FHitResult InnerHitResult;
	if (GetWorld()->LineTraceSingleByChannel(InnerHitResult,
		Start - Forward, End - Forward, ECC_Visibility, TraceParams))
	{
		HitResult = InnerHitResult;
		bFountHit = true;
	}

	FHitResult OuterHitResult;
	if (GetWorld()->LineTraceSingleByChannel(OuterHitResult,
		Start + Forward, End + Forward, ECC_Visibility, TraceParams))
	{
		HitResult = OuterHitResult;
		bFountHit = true;
	}

	if (InnerHitResult.bBlockingHit && OuterHitResult.bBlockingHit)
	{
		HitResult = (InnerHitResult.Distance <= OuterHitResult.Distance) ?
			InnerHitResult : OuterHitResult;
	}

	FCollisionQueryParams VerticalTraceParams;
	TraceParams.bFindInitialOverlaps = true;
	TraceParams.bTraceComplex = true;
	TraceParams.AddIgnoredActor(Portal);

	if (OwningController != nullptr)
		TraceParams.AddIgnoredActor(OwningController->GetCharacter());

	FHitResult OverlapHitResult;

	float Fraction = 0.0f;
	FVector Direction = End - Start;
	FVector Location(0.0f);

	while (Fraction <= 1.0f + KINDA_SMALL_NUMBER)
	{
		Location = Start + (Direction * Fraction);
		if (!GetWorld()->LineTraceSingleByChannel(OverlapHitResult,
			Location + (Forward * 2), Location - (Forward * 2), ECC_Visibility,
			VerticalTraceParams))
		{
			FVector RightProjection = Direction.ProjectOnToNormal(Right);
			FVector UpProjection = Direction.ProjectOnToNormal(Up);

			int32 Vertical = GetWorld()->LineTraceSingleByChannel
			(
				OverlapHitResult,
				Location + (RightProjection * 0.05f) - (UpProjection * 0.05f) + (Forward * 2),
				Location + (RightProjection * 0.05f) - (UpProjection * 0.05f) - (Forward * 2),
				ECC_Visibility,
				VerticalTraceParams
			);

			int32 Horizontal = GetWorld()->LineTraceSingleByChannel
			(
				OverlapHitResult,
				Location + (UpProjection * 0.05f) - (RightProjection * 0.05f) + (Forward * 2),
				Location + (UpProjection * 0.05f) - (RightProjection * 0.05f) - (Forward * 2),
				ECC_Visibility,
				VerticalTraceParams
			);

			if (Vertical)
			{
				OverlapHitResult.Normal = -UpProjection.GetClampedToSize(0, 1.0f);
			}

			if (Horizontal)
			{
				OverlapHitResult.Normal = -RightProjection.GetClampedToSize(0, 1.0f);
			}

			if (!Vertical && !Horizontal)
			{
				OverlapHitResult.Normal = -UpProjection.GetClampedToSize(0, 1.0);
			}

			OverlapHitResult.bBlockingHit = true;
			OverlapHitResult.Location = Location;
			OverlapHitResult.Distance = FVector::Distance(Start, OverlapHitResult.Location);
			OverlapHitResult.ImpactNormal = FVector::CrossProduct(OverlapHitResult.Normal, Forward);

			if (bFountHit)
			{
				if (OverlapHitResult.Distance <= HitResult.Distance)
					HitResult = OverlapHitResult;
			}
			else
			{
				HitResult = OverlapHitResult;
			}

			bFountHit = true;
			break;
		}

		OverlapHitResult.Reset();
		Fraction += 0.05f;
	}

	return bFountHit;
}

