// Copyright Joakim Karlsson 2022


#include "PortalPlayerController.h"
#include "PortalManager.h"

void APortalPlayerController::BeginPlay()
{
	PortalManager = GetWorld()->SpawnActor<APortalManager>(APortalManager::StaticClass(),
		FVector::ZeroVector, FRotator::ZeroRotator);

	PortalManager->AttachToActor(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	PortalManager->SetControllerOwner(this);
	PortalManager->SetPortalClass(PortalClass);
}

APortalManager* APortalPlayerController::GetPortalManager()
{
	return PortalManager;
}


