// Copyright Joakim Karlsson 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalManager.generated.h"

class APortalPass;
class APortalPlayerController;

UCLASS()
class PORTAL_API APortalManager : public AActor
{
	GENERATED_BODY()

public:

	APortalManager();

	void Tick(float DeltaTime) override;

	void SetControllerOwner(APortalPlayerController* NewOwner);

	void SetPortalClass(TSubclassOf<class APortalPass> Class);

	void SpawnBluePortal(const FVector& Start, const FVector& End);

	void SpawnRedPortal(const FVector& Start, const FVector& End);

	APortalPass* SpawnPortal(APortalPass* Target,
		const FVector& Start, const FVector& End);

	bool TraceCorner(const APortalPass* Portal, const FVector& Start,
		const FVector& End, const FVector& Forward, const FVector& Right,
		const FVector& Up, FHitResult& HitResult);

	bool VerifyPortalPlacement(const APortalPass* Portal, FVector& Origin);

	void FitPortalAroundTargetPortal(const APortalPass* Portal,
		const APortalPass* Target, const FVector& Forward, const FVector& Right,
		const FVector& Up, FVector& Origin);

	bool FitPortalOnSurface(const APortalPass* Portal, const FVector& Forward,
		const FVector& Right, const FVector& Up, const FVector& TopEdge,
		const FVector& BottomEdge, const FVector& RightEdge, const FVector& LeftEdge,
		FVector& Origin, int RecursionCount = 10);

protected:

	virtual void BeginPlay() override;

private:

	APortalPass* BluePortal;
	APortalPass* RedPortal;

	UPROPERTY()
		APortalPlayerController* OwningController;

	UPROPERTY()
		TSubclassOf<class APortalPass> PortalClass;

};
