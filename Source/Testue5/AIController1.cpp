// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController1.h"
#include "GameFramework/Character.h"
#include "Testue5Character.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "EngineUtils.h"
#include "Waypoints.h"


AAIController1::AAIController1()
{
	CurrentPatrolPointIndex = 0;
}

void AAIController1::BeginPlay()
{
	Super::BeginPlay();

	PopulateWaypointsInLevel();
}


void AAIController1::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

    if (PatrolPoints.Num() > 0)
    {
        // Find the nearest waypoint
        float MinDistanceSquared = FLT_MAX;
        int32 NearestWaypointIndex = 0;

        for (int32 i = 0; i < PatrolPoints.Num(); i++)
        {
            float DistanceSquared = FVector::DistSquared(PatrolPoints[i]->GetActorLocation(), MyCharacter->GetActorLocation());
            if (DistanceSquared < MinDistanceSquared)
            {
                MinDistanceSquared = DistanceSquared;
                NearestWaypointIndex = i;
            }
        }

        // Check if the AI is within a certain distance from the current waypoint
        if (FVector::DistSquared(PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation(), MyCharacter->GetActorLocation()) < 100.f * 100.f)
        {
            // Update to the next waypoint
            CurrentPatrolPointIndex = (CurrentPatrolPointIndex + 1) % PatrolPoints.Num();
        }

        // Move to the nearest waypoint
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation());

        // Debug prints
        UE_LOG(LogTemp, Warning, TEXT("Nearest Waypoint Index: %d"), NearestWaypointIndex);
        UE_LOG(LogTemp, Warning, TEXT("CurrentPatrolPointIndex: %d"), CurrentPatrolPointIndex);
        UE_LOG(LogTemp, Warning, TEXT("Current Waypoint Location: %s"), *PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation().ToString());
    }
}
void AAIController1::PopulateWaypointsInLevel()
{
    for (TActorIterator<AWaypoints> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AWaypoints* Waypoint = *ActorItr;
        if (Waypoint)
        {
            PatrolPoints.Add(Waypoint);
        }
    }

    // Debug print to check the number of waypoints found
    UE_LOG(LogTemp, Warning, TEXT("Number of Waypoints: %d"), PatrolPoints.Num());
}
