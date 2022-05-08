// Copyright Joakim Karlsson 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PortalPlayerController.generated.h"

class APortalManager;

UCLASS()
class PORTAL_API APortalPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	APortalManager* GetPortalManager();

	UPROPERTY(EditDefaultsOnly, Category = Portal)
		TSubclassOf<class APortalPass> PortalClass;

protected:
	virtual void BeginPlay() override;

	APortalManager* PortalManager;

};
