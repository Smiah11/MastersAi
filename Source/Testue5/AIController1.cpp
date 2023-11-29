// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController1.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
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

    SetupAI();

    PopulateWaypointsInLevel();

}


void AAIController1::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
      MoveToNextWaypoint();
}


void AAIController1::SetupAI()
{
    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
    if (MyCharacter && PatrolPoints.Num() > 0)
    {
        UCharacterMovementComponent* MovementComponent = MyCharacter->GetCharacterMovement();

        if (MovementComponent)
        {
            // Set ground speed
            MovementComponent->MaxWalkSpeed = 200.0f;
 
        }
    }
}

void AAIController1::MoveToNextWaypoint()
{
    if (PatrolPoints.Num() > 0)
    {
        ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

        if (MyCharacter)
        {
            // Check if the AI is close to the current waypoint
            float DistanceSquared = FVector::DistSquared(PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation(), MyCharacter->GetActorLocation());
            if (DistanceSquared < 100.f * 100.f)
            {
                // Destroy the current waypoint
                PatrolPoints[CurrentPatrolPointIndex]->Destroy();
                UE_LOG(LogTemp, Warning, TEXT("Destroyed Waypoint at: %s"), *PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation().ToString());

                // Spawn a new waypoint
                FNavLocation RandomLocation;
                UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
                if (NavSystem && NavSystem->GetRandomReachablePointInRadius(MyCharacter->GetActorLocation(), 1000.f, RandomLocation))
                {
                    AWaypoints* NewWaypoint = GetWorld()->SpawnActor<AWaypoints>(AWaypoints::StaticClass(), RandomLocation.Location, FRotator::ZeroRotator);
                    if (NewWaypoint)
                    {
                        PatrolPoints[CurrentPatrolPointIndex] = NewWaypoint;

                        // Display debug sphere for the new waypoint if the AI is currently following a path
                        if (IsFollowingAPath())
                        {
                            DrawDebugSphere(GetWorld(), RandomLocation.Location, 50.0f, 12, FColor::Green, false, 2.0f);
                        }

                        UE_LOG(LogTemp, Warning, TEXT("Generated New Waypoint at: %s"), *RandomLocation.Location.ToString());
                    }
                }

                // Update to the next waypoint
                CurrentPatrolPointIndex = (CurrentPatrolPointIndex + 1) % PatrolPoints.Num();
                UE_LOG(LogTemp, Warning, TEXT("MoveToNextWaypoint called!"));
            }

            // Move to the nearest waypoint
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation());
        }
    }
}

void AAIController1::PopulateWaypointsInLevel()
{
    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
    if (!MyCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("AAIController1::PopulateWaypointsInLevel - MyCharacter is null!"));
        return;
    }

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
    if (!NavSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("AAIController1::PopulateWaypointsInLevel - NavSystem is null!"));
        return;
    }

    FNavLocation RandomLocation;
    if (NavSystem->GetRandomReachablePointInRadius(MyCharacter->GetActorLocation(), 5000.f, RandomLocation))
    {
        AWaypoints* Waypoint = GetWorld()->SpawnActor<AWaypoints>(AWaypoints::StaticClass(), RandomLocation.Location, FRotator::ZeroRotator);
        if (Waypoint)
        {
            PatrolPoints.Add(Waypoint);

            // Debug print to check the location of the waypoint
            DrawDebugSphere(GetWorld(), RandomLocation.Location, 50.0f, 12, FColor::Green, false, 2.0f);

            UE_LOG(LogTemp, Warning, TEXT("Generated Waypoint at: %s"), *RandomLocation.Location.ToString());
        }
    }

    // Debug print to check the number of waypoints found
    UE_LOG(LogTemp, Warning, TEXT("Number of Waypoints: %d"), PatrolPoints.Num());
}