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
	//SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
	//SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	//GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
	//GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetDetected);
	//GetPerceptionComponent()->ConfigureSense(*SightConfig);

	PrimaryActorTick.TickInterval = 0.1f;

	bIsProvoked = false;

	bInRestrictedZone = false;

	AAICharacter* AICharacter = Cast<AAICharacter>(GetPawn());


	
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
	CheckPlayerProximity();


}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	SetupAI();
}



void AEnemyAIController::SetupAI()
{
	/*
	// Setup AI perception
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = SightConfig->SightRadius + 50.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	GetPerceptionComponent()->ConfigureSense(*SightConfig);
	*/

	SetMaxSpeed(350.f);

	AAICharacter* AICharacter = Cast<AAICharacter>(GetPawn());
	if (AICharacter && PatrolPoints.Num() > 0)
	{
		UCharacterMovementComponent* MovementComponent = AICharacter->GetCharacterMovement();

	
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
	EnemyUtilities.Add({ EAIState_Enemy::Investigate, CalculateInvestigateUtility() });

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

	float Utility = bIsProvoked  ? 4.f : 0.f;  // if the player is provoked or in a restricted zone

	return Utility;

	//UE_LOG(LogTemp, Warning, TEXT("Attack Utility: %f"), Utility);
	
}

float AEnemyAIController::CalculateProvokableUtility() const
{
	float Utility = DetectedPlayer ? 1.f : 0.f; // If the player is detected, the utility is 1, otherwise it's 0
	return Utility;
}

float AEnemyAIController::CalculateInvestigateUtility() const
{
	const float DistanceToLastKnowLocation = FVector::Dist(LastKnownLocation, GetPawn()->GetActorLocation());
	float MaxInvestigationDistance = 3000.f;
	const float Utility = bInRestrictedZone ? 3.f : 0.f;
		
	//FMath::Clamp(1.f - (DistanceToLastKnowLocation / MaxInvestigationDistance), 0.f, 1.f) && ; // The closer the enemy is to the last known location, the higher the utility

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
		SetMaxSpeed(350.f);
		MoveToNextWaypoint();
		break;
	case EAIState_Enemy::Attack:
		Attack();
		SetMaxSpeed(700.f);
		break;
	case EAIState_Enemy::Provokable:
		Provoke();
		break;
	case EAIState_Enemy::Investigate:
		Investigate();
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

			if (DistanceToPlayer <= AttackRange  )
			{

				LastMovedToActor = PlayerPawn;
				MoveToActor(PlayerPawn, AttackDistance, true); 

				if (CurrentTime - LastAttackTime > AttackCooldown)
				{
				
					
					//UE_LOG(LogTemp, Warning, TEXT("Attacking Player at distance: %f"), DistanceToPlayer);
					// Attack the player 

					// Simple Line Trace 
					UGameplayStatics::ApplyDamage(PlayerPawn, AttackDamage, GetInstigatorController(), MyCharacter, nullptr);
					DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.f, 0.f, 1.f);

					LastAttackTime = CurrentTime; // Update the last attack time
					
				}
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

void AEnemyAIController::CheckPlayerProximity()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return;

	float DistanceToPlayer = FVector::Dist(PlayerPawn->GetActorLocation(), GetPawn()->GetActorLocation());
	if (DistanceToPlayer <= ProvokableDistance)
	{
		DetectedPlayer = PlayerPawn;
		UE_LOG(LogTemp, Warning, TEXT("Player Detected"));
	}
	else
	{
		DetectedPlayer = nullptr;
		bIsProvoked = false;
		UE_LOG(LogTemp, Warning, TEXT("Player is too far or not detected"));
	}

	/*
	if (Stimulus.WasSuccessfullySensed() && Actor == PlayerPawn) {
		DetectedPlayer = Actor;
		FacePlayer();
		if (DistanceToPlayer <= ProvokableDistance) {
			bIsProvoked = true;
			UE_LOG(LogTemp, Warning, TEXT("Player detected and provoked."));
		}
		else {
			bIsProvoked = false; // Clear provoked state if player is seen but out of provoke distance
		}
	}
	else {
		DetectedPlayer = nullptr;
		bIsProvoked = false; // Player lost, clear provoked state
		UE_LOG(LogTemp, Warning, TEXT("Lost sight of Actor: %s"), *Actor->GetName());
	}
	*/



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
	
	if (PlayerPawn)
	{
		float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());

		// Check if the player is within the provocation distance
		if (DistanceToPlayer <= ProvokableDistance)
		{
			FacePlayer();
			StopMovement();


	
			FTimerHandle PlayerProximity;
			GetWorld()->GetTimerManager().SetTimer(PlayerProximity, this, &AEnemyAIController::SetProvoked, 5.0f, false);
			

		}
		else
		{
			bIsProvoked = false;
		}


	}
}

void AEnemyAIController::Investigate()
{
		SetMaxSpeed(700.f);

		MoveToLocation(LastKnownLocation);

		FTimerHandle InvestigateTimer;
		GetWorld()->GetTimerManager().SetTimer(InvestigateTimer, this, &AEnemyAIController::DecideNextAction, 4.f, false);


		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

		FVector PlayerLocation = PlayerPawn->GetActorLocation();
		float DistanceToLocation = FVector::Dist(GetPawn()->GetActorLocation(),LastKnownLocation);
		
		if (DistanceToLocation <= ProvokableDistance)
		{
			ResetInvestigation();
		}

	/// maybe if player is not in  provokable distance then set in restricted zone to false

}

void AEnemyAIController::ResetInvestigation()
{
	LastKnownLocation = FVector::ZeroVector;
	bInRestrictedZone = false;
	SetMaxSpeed(350.f);
	DecideNextAction();
}

float AEnemyAIController::SetMaxSpeed(float Speed)
{

	ACharacter* ControllerAI = GetCharacter();

	if (ControllerAI)
	{
		UCharacterMovementComponent* AIMovementComp = ControllerAI->GetCharacterMovement();

		if (AIMovementComp)
		{
			AIMovementComp->MaxWalkSpeed = Speed;
		}
	}

	return Speed;
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
	else if (Result.IsSuccess() && CurrentState == EAIState_Enemy::Investigate)
	{
		bInRestrictedZone = false;
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyAIController::DecideNextAction, 1.f, false);
	}
}

void AEnemyAIController::SetInRestrictedZone(bool bRestricted, FVector LastKnownPlayerLocation)
{
	bInRestrictedZone = bRestricted;

	if (bRestricted && LastKnownLocation.IsNearlyZero()) 
	{
		LastKnownLocation = LastKnownPlayerLocation;
	}

}
