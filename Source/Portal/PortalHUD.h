// Copyright Joakim Karlsson 2022

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PortalHUD.generated.h"

UCLASS()
class APortalHUD : public AHUD
{
	GENERATED_BODY()

public:
	APortalHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

