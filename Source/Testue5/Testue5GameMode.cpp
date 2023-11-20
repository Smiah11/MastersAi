// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testue5GameMode.h"
#include "Testue5Character.h"
#include "UObject/ConstructorHelpers.h"

ATestue5GameMode::ATestue5GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
