// Copyright Joakim Karlsson 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalPass.generated.h"

class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class PORTAL_API APortalPass : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APortalPass();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void OnPortalOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION()
		void OnPortalOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TeleportActors();

	void SetTarget(APortalPass* NewTarget);

	void SetPortalSurface(AActor* Surface);

	AActor* GetPortalSurface() const;

	APortalPass* GetTarget() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		UStaticMeshComponent* PortalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		UBoxComponent* PortalCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture)
		APortalPass* Target;

private:

	FVector LastPosition;

	bool bLastInFront;
	bool Overlapping;

	AActor* PortalSurface;

	TArray<AActor*> OverlappingActors;

};
