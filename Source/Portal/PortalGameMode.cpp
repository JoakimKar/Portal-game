// Copyright Joakim Karlsson 2022

#include "PortalGameMode.h"
#include "PortalHUD.h"
#include "PortalCharacter.h"
#include "UObject/ConstructorHelpers.h"

APortalGameMode::APortalGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = APortalHUD::StaticClass();
}
