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

	PrimaryActorTick.bCanEverTick = true;


}

void ATestue5GameMode::BeginPlay()
{
	Super::BeginPlay();

	CurrentTime = 14 * 3600;// Same as the Civillian default time
}

void ATestue5GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTime(DeltaTime);
}

void ATestue5GameMode::UpdateTime(float DeltaTime)
{

	float SecondsInADay = 24 * 60 * 60; // 24 hours in game
	float TimeScale = SecondsInADay / 600; // Real-life seconds to in-game time (10 mins)

	// faster time progression
	CurrentTime += DeltaTime * TimeScale;

	// loops back to 0 when it reaches the end of the day
	CurrentTime = FMath::Fmod(CurrentTime, SecondsInADay);


	// Convert seconds to hours for the current hour.
	CurrentHour = CurrentTime / 3600;
}
