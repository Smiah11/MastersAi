// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController2.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "EngineUtils.h"
#include "Waypoint_Manual.h"


AAIController2::AAIController2()
{
    CurrentPatrolPointIndex = 0;
}

void AAIController2::BeginPlay()
{
    Super::BeginPlay();

    PopulateWaypointsArray();



}



void AAIController2::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    MoveToNextWaypoint();
    SetupAI();// move to tick to fix
   
}


void AAIController2::SetupAI()
{
    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
    if (MyCharacter && PatrolPoints.Num() > 0)
    {
		UCharacterMovementComponent* MovementComponent = MyCharacter->GetCharacterMovement();

        if (MovementComponent)
        {
			// Set ground speed
			MovementComponent->MaxWalkSpeed = 600.0f;

		}
	}
}

void AAIController2::PopulateWaypointsArray()
{
    for (TActorIterator<AWaypoint_Manual> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AWaypoint_Manual* Waypoint = *ActorItr;
        if (Waypoint)
        {
            PatrolPoints.Add(Waypoint);
        }
    }

    // Debug print to check the number of waypoints found
    //UE_LOG(LogTemp, Warning, TEXT("Number of Waypoints: %d"), PatrolPoints.Num());
}

void AAIController2::MoveToNextWaypoint()
{
    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

    if (PatrolPoints.Num() > 0)
    {
        // Find the nearest waypoint only if the AI is not currently moving towards a waypoint
        if (!IsFollowingAPath())
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

            // Set the current waypoint index to the nearest waypoint
            CurrentPatrolPointIndex = NearestWaypointIndex;
        }

        // Check if the AI is within a certain distance from the current waypoint
        if (FVector::DistSquared(PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation(), MyCharacter->GetActorLocation()) < 100.f * 100.f)
        {
            // Update to the next waypoint
            CurrentPatrolPointIndex = (CurrentPatrolPointIndex + 1) % PatrolPoints.Num();
        }

        // Move to the current waypoint
        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation());

        // Debug prints
        //UE_LOG(LogTemp, Warning, TEXT("CurrentPatrolPointIndex: %d"), CurrentPatrolPointIndex);
       // UE_LOG(LogTemp, Warning, TEXT("Current Waypoint Location: %s"), *PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation().ToString());
    }
}

