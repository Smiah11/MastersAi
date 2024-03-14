// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include <GameFramework/CharacterMovementComponent.h>
#include "Navigation/PathFollowingComponent.h"
#include <AICharacter.h>



AEnemyAIController::AEnemyAIController()
{

	CurrentPatrolPointIndex = 0;
	

	//Initialise Perception Components
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetDetected);
	GetPerceptionComponent()->ConfigureSense(*SightConfig);

	PrimaryActorTick.TickInterval = 0.1f;

	bIsProvoked = false;

	bInRestrictedZone = false;
	
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	SetupAI();
	PopulateWaypointsInLevel();


	
} 

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DecideNextAction();


}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetupAI();
}



void AEnemyAIController::SetupAI()
{
	// Setup AI perception
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = SightConfig->SightRadius + 50.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	GetPerceptionComponent()->ConfigureSense(*SightConfig);


	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
	if (MyCharacter && PatrolPoints.Num() > 0)
	{
		UCharacterMovementComponent* MovementComponent = MyCharacter->GetCharacterMovement();

		if (MovementComponent)
		{
			// Set ground speed
			MovementComponent->MaxWalkSpeed = 400.f;
			MovementComponent->SetAvoidanceEnabled(true); // Enable unreal's RVO avoidance system
		}
	}
}

TArray<EnemyActionUtility> AEnemyAIController::CalculateEnemyUtilities()
{
	TArray<EnemyActionUtility> EnemyUtilities;

	// Calculate the utility of each action
	EnemyUtilities.Add({ EAIState_Enemy::Patrol, CalculatePatrolUtility() });
	EnemyUtilities.Add({ EAIState_Enemy::Attack, CalculateAttackUtility() });
	EnemyUtilities.Add({ EAIState_Enemy::Provokable, CalculateProvokableUtility() });

	return EnemyUtilities;
}

EnemyActionUtility AEnemyAIController::ChooseBestAction(const TArray<EnemyActionUtility>& EnemyActionUtilities) const
{
	check(EnemyActionUtilities.Num() > 0); // Ensure there's at least one action

	EnemyActionUtility BestAction = EnemyActionUtilities[0]; // Start with the first action as the best choice
	for (const auto& ActionUtility : EnemyActionUtilities) {
		if (ActionUtility.Utility > BestAction.Utility) {
			BestAction = ActionUtility; // Found a better action
		}
	}

	return BestAction;
}

float AEnemyAIController::CalculatePatrolUtility() const
{
	auto* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	float DistanceToPlayer = FVector::Dist(Player->GetActorLocation(), GetPawn()->GetActorLocation());

	float Utility = DistanceToPlayer / 1000.f;// The further the player is, the higher the utility

	return Utility * PatrolUtilityModifier;

	//UE_LOG(LogTemp, Warning, TEXT("Patrol Utility: %f"), Utility);

}

float AEnemyAIController::CalculateAttackUtility() const
{

	float Utility = bIsProvoked || bInRestrictedZone ? 4.f : 0.f; 

	return Utility;

	//UE_LOG(LogTemp, Warning, TEXT("Attack Utility: %f"), Utility);
	
}

float AEnemyAIController::CalculateProvokableUtility() const
{
	float Utility = DetectedPlayer ? 1.f : 0.f; // If the player is detected, the utility is 1, otherwise it's 0
	return Utility;
}

void AEnemyAIController::DecideNextAction()
{
	EnemyActionUtility nextAction = ChooseBestAction(CalculateEnemyUtilities());
	//UE_LOG(LogTemp, Warning, TEXT("Deciding next action: %d with utility %f"), nextAction.Action, nextAction.Utility);
	ExecuteAction(nextAction.Action);
}

void AEnemyAIController::ExecuteAction(EAIState_Enemy Action)
{
	switch (Action)
	{
	case EAIState_Enemy::Patrol:
		MoveToNextWaypoint();
		break;
	case EAIState_Enemy::Attack:
		Attack();
		break;
	case EAIState_Enemy::Provokable:
		Provoke();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unhandled action in ExecuteAction"));
		break;
	}
}


void AEnemyAIController::Attack()
{

	FacePlayer();

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

	if (PlayerPawn && MyCharacter)
	{

		FVector StartLocation = MyCharacter->GetActorLocation();
		FVector EndLocation = PlayerPawn->GetActorLocation();
		//FHitResult HitResult;
		//bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility);

			float DistanceToPlayer = FVector::Dist(StartLocation, EndLocation);
			float CurrentTime = GetWorld()->GetTimeSeconds();

			if (DistanceToPlayer <= AttackRange || bInRestrictedZone == true )
			{

				LastMovedToActor = PlayerPawn;
				MoveToActor(PlayerPawn, AttackDistance);

				if (DistanceToPlayer <= AttackRange && CurrentTime - LastAttackTime > AttackCooldown)
				{
				
					
					//UE_LOG(LogTemp, Warning, TEXT("Attacking Player at distance: %f"), DistanceToPlayer);
					// Attack the player
					UGameplayStatics::ApplyDamage(PlayerPawn, AttackDamage, GetInstigatorController(), MyCharacter, nullptr);
					DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.f, 0.f, 1.f);

					LastAttackTime = CurrentTime; // Update the last attack time
					
				}
			}
			else
			{
				bIsProvoked = false; // The player is out of range, so the enemy is no longer provoked

				
			}
	}
}

void AEnemyAIController::MoveToNextWaypoint()
{

	if (PatrolPoints.Num() > 0 && CurrentPatrolPointIndex < PatrolPoints.Num())
	{
		AEnemyAIController* MyCharacter = Cast<AEnemyAIController>(GetPawn());

		AWaypoints* NextWaypoint = PatrolPoints[CurrentPatrolPointIndex];
		if (NextWaypoint)
		{

			//FaceLocation(NextWaypoint->GetActorLocation());
			//CurrentTargetLocation = NextWaypoint->GetActorLocation();

			LastMovedToActor = NextWaypoint;
			MoveToActor(NextWaypoint, 50.f, true);
		}
	}
	else
	{

		UE_LOG(LogTemp, Warning, TEXT("No waypoints available or index out of bounds."));
	}

}

bool AEnemyAIController::FindSuitableNewWaypointLocation(FVector& OutLocation, float MinDistance, int MaxRetries)
{
	AAICharacter* Guard = Cast<AAICharacter>(GetPawn());
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation RandomLocation;

	for (int RetryCount = 0; RetryCount < MaxRetries; ++RetryCount) // Try to find a suitable location
	{
		if (NavSystem && NavSystem->GetRandomPointInNavigableRadius(Guard->GetActorLocation(), 2000.0f, RandomLocation))
		{
			FVector NewLocation = RandomLocation.Location;
			if (FVector::DistSquared(NewLocation, Guard->GetActorLocation()) >= FMath::Square(MinDistance))
			{
				OutLocation = NewLocation;
				return true; // Successful in finding a suitable location
			}
		}
	}

	return false; // Failed to find a suitable location
}

void AEnemyAIController::SpawnNewWaypoint()
{
	if (CurrentState != EAIState_Enemy::Patrol)
	{
		return;
	}

	FVector NewLocation;
	const float MinDistance = 500.f; // Minimum distance from the current waypoint
	const int MaxRetries = 10; // Maximum number of attempts to find a suitable location

	if (FindSuitableNewWaypointLocation(NewLocation, MinDistance, MaxRetries))
	{
		// Destroy the current waypoint before spawning a new one
		if (PatrolPoints.IsValidIndex(CurrentPatrolPointIndex))
		{
			AWaypoints* CurrentWaypoint = PatrolPoints[CurrentPatrolPointIndex];
			if (CurrentWaypoint)
			{
				CurrentWaypoint->Destroy();
				PatrolPoints.RemoveAt(CurrentPatrolPointIndex);
			}
		}

		// Spawn the new waypoint
		AWaypoints* NewWaypoint = GetWorld()->SpawnActor<AWaypoints>(AWaypoints::StaticClass(), NewLocation, FRotator::ZeroRotator);
		if (NewWaypoint)
		{
			PatrolPoints.Add(NewWaypoint);
			CurrentPatrolPointIndex = PatrolPoints.Num() - 1; // Move to the newly spawned waypoint next


			DrawDebugSphere(GetWorld(), NewLocation, 50.f, 12, FColor::Green, false, 3.f);
			//UE_LOG(LogTemp, Warning, TEXT("New waypoint spawned at: %s"), *NewLocation.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find a suitable location for a new waypoint after %d retries."), MaxRetries);
	}
}
void AEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());

	if (Stimulus.WasSuccessfullySensed() && Actor == PlayerPawn) // Check for sight stimulus
	{
		CalculateProvokableUtility();
		DetectedPlayer = Actor;
		FacePlayer();
		UE_LOG(LogTemp, Warning, TEXT("Detected Actor: %s, at Location: %s"), *Actor->GetName(), *Actor->GetActorLocation().ToString());
	}
	else if (DistanceToPlayer <= AttackRange) // Additional check for distance
	{
		CalculateProvokableUtility();
		DetectedPlayer = PlayerPawn; 
		FacePlayer();
	}
	else
	{
		CalculatePatrolUtility();
		DetectedPlayer = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Lost Actor: %s"), *Actor->GetName());
	}

}

void AEnemyAIController::PopulateWaypointsInLevel()
{

	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());


	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	FNavLocation RandomLocation;
	if (NavSystem->GetRandomReachablePointInRadius(MyCharacter->GetActorLocation(), 1000.f, RandomLocation))
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
}

void AEnemyAIController::FacePlayer()
{
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);


	FVector Direction = (PlayerPawn->GetActorLocation() - MyCharacter->GetActorLocation()).GetSafeNormal();
	FRotator TargetRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;
	MyCharacter->SetActorRotation(TargetRotation);
}

void AEnemyAIController::Provoke()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	StopMovement();
	if (PlayerPawn)
	{
		float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());

		// Check if the player is within the provocation distance
		if (DistanceToPlayer <= ProvokableDistance)
		{
			FacePlayer();


			if (!bIsProvoked)
			{
				FTimerHandle PlayerProximity;
				GetWorld()->GetTimerManager().SetTimer(PlayerProximity, this, &AEnemyAIController::SetProvoked, 5.0f, false);
			}

		}
		else
		{
			bIsProvoked = false;
		}


	}
}

void AEnemyAIController::SetProvoked()
{
		

		bIsProvoked = true;

}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.IsSuccess() && CurrentState == EAIState_Enemy::Patrol)
	{

		AWaypoints*Waypoint = Cast<AWaypoints>(LastMovedToActor); 

		if (Waypoint)
		{
			SpawnNewWaypoint();
		}
		
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyAIController::DecideNextAction, 1.f, false);

		//UE_LOG(LogTemp, Warning, TEXT("Move Completed"));
		//DecideNextAction();

	}
}

void AEnemyAIController::SetInRestrictedZone(bool bRestricted)
{
	bInRestrictedZone = bRestricted;
}
