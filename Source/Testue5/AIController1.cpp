// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController1.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Testue5Character.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "EngineUtils.h"
#include "Waypoints.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"


//Civillian Controller


AAIController1::AAIController1()
{
	CurrentPatrolPointIndex = 0;
	CurrentState = EAIState::Patrol; // Default state is Patrol
	
	
}

void AAIController1::BeginPlay()
{
	Super::BeginPlay();
	PopulateWaypointsInLevel();
	SetupAI();

}

void AAIController1::Tick(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("Current AI State: %s"), *UEnum::GetValueAsString(CurrentState));

	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case EAIState::Patrol:
		MoveToNextWaypoint();
		//AvoidOtherAI();
		break;
	case EAIState::ReactToPlayer:
		ReactToPlayer();
		break;

	}
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
			MovementComponent->MaxWalkSpeed = 250.f;
			MovementComponent->SetAvoidanceEnabled(true); // Enable unreal's RVO avoidance system
		}
	}
}

void AAIController1::MoveToNextWaypoint()
{
		ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (PatrolPoints.Num() > 0 && MyCharacter)
	{
		
			// Check if the AI is close to the current waypoint
			float DistanceSquared = FVector::DistSquared(PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation(), MyCharacter->GetActorLocation());
			if (DistanceSquared < 100.f * 100.f)
			{
				// Destroy the current waypoint
				PatrolPoints[CurrentPatrolPointIndex]->Destroy();
				//UE_LOG(LogTemp, Warning, TEXT("Destroyed Waypoint at: %s"), *PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation().ToString());

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
						if (CurrentState == EAIState::Patrol)
						{
							DrawDebugSphere(GetWorld(), RandomLocation.Location, 50.0f, 12, FColor::Green, false, 2.0f);
						}

						//UE_LOG(LogTemp, Warning, TEXT("Generated New Waypoint at: %s"), *RandomLocation.Location.ToString());
					}
				}

				// Update to the next waypoint
				CurrentPatrolPointIndex = (CurrentPatrolPointIndex + 1) % PatrolPoints.Num();
				//UE_LOG(LogTemp, Warning, TEXT("MoveToNextWaypoint called!"));
			}

			// Move to the nearest waypoint
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation());

	}
		FVector AIForwardVector = MyCharacter->GetActorForwardVector();
		FVector PlayerDirection = PlayerPawn->GetActorLocation() - MyCharacter->GetActorLocation();
		PlayerDirection.Normalize();

		float DotProduct = FVector::DotProduct(AIForwardVector, PlayerDirection);
		float DistanceToPlayer = FVector::Distance(PlayerPawn->GetActorLocation(), MyCharacter->GetActorLocation());

		if (DotProduct > 0.8f && DistanceToPlayer < ReactionRadius)
		{
			
			SetAIState(EAIState::ReactToPlayer);
		}
		else
		{
			
			SetAIState(EAIState::Patrol);
		}
}

void AAIController1::ReactToPlayer()
{
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	//UE_LOG(LogTemp, Warning, TEXT("ReactToPlayer called!"));
	float CurrentDistanceToPlayer = FVector::Distance(PlayerPawn->GetActorLocation(), MyCharacter->GetActorLocation());


	if (CurrentDistanceToPlayer <=ReactionRadius)
	{
		//SetAIState(EAIState::ReactToPlayer);
		// Player is within the specified radius 
		StopMovement();   
		FacePlayer();  
		UAnimInstance* AnimInstance = MyCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(ReactMontage, 1.0f);
		}
		
	}
	else
	{
		// Continue with normal movement 
		
		SetAIState(EAIState::Patrol);
	}
}

void AAIController1::FacePlayer()
{
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	

	FVector PlayerDirection = PlayerPawn->GetActorLocation() - MyCharacter->GetActorLocation();
	PlayerDirection.Normalize();

	FRotator NewRotation = PlayerDirection.Rotation();
	MyCharacter->SetActorRotation(FMath::RInterpTo(MyCharacter->GetActorRotation(), NewRotation, GetWorld()->GetDeltaSeconds(), 5.0f));
}



// avoidance code Kindof Worked but not really

/*void AAIController1::AvoidOtherAI()
{
	AvoidanceDistance = 100.f;
	float WalkAroundDistance = 250.f; 
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

	if (MyCharacter && CurrentState == EAIState::Patrol)
	{
		// Iterate over nearby characters
		TArray<AActor*> OtherAI;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), OtherAI);

		for (AActor* OtherAIs : OtherAI)
		{
			if (OtherAIs == MyCharacter)
			{
				continue; // Skip itself
			}

			ACharacter* OtherAICharacter = Cast<ACharacter>(OtherAIs);
			if (OtherAICharacter)
			{
				FVector PlayerLocation = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorLocation();
				float DistanceToPlayer = FVector::Distance(PlayerLocation, MyCharacter->GetActorLocation());

				// Skip the player
				if (OtherAIs->IsA<APlayerController>())
				{
					continue;
				}

				float DistanceSquared = FVector::DistSquared(MyCharacter->GetActorLocation(), OtherAICharacter->GetActorLocation());

				if (DistanceSquared < AvoidanceDistance * AvoidanceDistance)
				{
					// Calculate the avoidance vector
					FVector AvoidanceVector = MyCharacter->GetActorLocation() - OtherAICharacter->GetActorLocation();
					AvoidanceVector.Normalize();

					// Check if the AI needs to walk around the obstacle
					if (DistanceSquared < WalkAroundDistance * WalkAroundDistance)
					{
						// Calculate the new location to walk around the obstacle
						FVector WalkAroundLocation = MyCharacter->GetActorLocation() + AvoidanceVector * WalkAroundDistance;

						// Apply movement to walk around the obstacle
						UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, WalkAroundLocation);
						//UE_LOG(LogTemp, Warning, TEXT("Walk around obstacle!"));
						return;
					}
					else
					{
						// Adjust the location based on avoidance vector and distance
						FVector AdjustedLocation = MyCharacter->GetActorLocation() + AvoidanceVector * AvoidanceDistance;

						// Check if the adjusted location is close to the player
						if (DistanceToPlayer < AvoidanceDistance)
						{
							// If it's close to the player, continue with normal movement
							UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, AdjustedLocation);
						}
						else
						{
							// Apply avoidance
							UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, AdjustedLocation);
							//UE_LOG(LogTemp, Warning, TEXT("AvoidOtherAI called!"));
						}
					}
				}
			}
		}
	}
}*/

void AAIController1::SetAIState(EAIState NewState)
{
		CurrentState = NewState;
		//UE_LOG(LogTemp, Warning, TEXT("AI State set to %s"), *UEnum::GetValueAsString(NewState));
}


void AAIController1::PopulateWaypointsInLevel()
{
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
	if (!MyCharacter)
	{
		//UE_LOG(LogTemp, Error, TEXT("AAIController1::PopulateWaypointsInLevel - MyCharacter is null!"));
		return;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	if (!NavSystem)
	{
		//UE_LOG(LogTemp, Error, TEXT("AAIController1::PopulateWaypointsInLevel - NavSystem is null!"));
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

			//UE_LOG(LogTemp, Warning, TEXT("Generated Waypoint at: %s"), *RandomLocation.Location.ToString());
		}
	}

	// Debug print to check the number of waypoints found
	//UE_LOG(LogTemp, Warning, TEXT("Number of Waypoints: %d"), PatrolPoints.Num());
}