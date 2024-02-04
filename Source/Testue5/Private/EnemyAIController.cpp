// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "DrawDebugHelpers.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include <GameFramework/CharacterMovementComponent.h>



AEnemyAIController::AEnemyAIController()
{

	CurrentPatrolPointIndex = 0;
	CurrentState = EAIState_Enemy::Patrol;

	//Initialise Perception Components
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetDetected);
	GetPerceptionComponent()->ConfigureSense(*SightConfig);
	
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

	switch (CurrentState)
	{
	case EAIState_Enemy::Patrol:
		Patrol();
		break;
	case EAIState_Enemy::Attack:
		Attack();
		break;
	case EAIState_Enemy::Provokable:
		Provoke(DeltaTime);
		break;
	}
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


void AEnemyAIController::SetState(EAIState_Enemy NewState)
{

	UE_LOG(LogTemp, Warning, TEXT("Changing state from %d to %d"), CurrentState, NewState);
	CurrentState = NewState;

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

			if (DistanceToPlayer <= AttackRange)
			{
				MoveToActor(PlayerPawn, AttackDistance);

				if (DistanceToPlayer <= AttackDistance && CurrentTime - LastAttackTime > AttackCooldown)
				{
				
					
					UE_LOG(LogTemp, Warning, TEXT("Attacking Player at distance: %f"), DistanceToPlayer);
					// Attack the player
					UGameplayStatics::ApplyDamage(PlayerPawn, AttackDamage, GetInstigatorController(), MyCharacter, nullptr);
					DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.f, 0.f, 1.f);

					LastAttackTime = CurrentTime; // Update the last attack time
					
				}
			}
			else
			{

			//UE_LOG(LogTemp, Warning, TEXT("Patrolling to waypoint index: %d"), CurrentPatrolPointIndex);
			// Player is too far away, switch back to patrol
			SetState(EAIState_Enemy::Patrol);
			}
	}
}

void AEnemyAIController::Patrol()
{

	if (CurrentState != EAIState_Enemy::Patrol) // If we're not in patrol state, return
	{
		return;
	}
	

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
					if (CurrentState == EAIState_Enemy::Patrol)
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


}

void AEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());

	if (Stimulus.WasSuccessfullySensed() && Actor == PlayerPawn) // Check for sight/sound stimulus
	{
		SetState(EAIState_Enemy::Provokable);
		DetectedPlayer = Actor;
		FacePlayer();
		UE_LOG(LogTemp, Warning, TEXT("Detected Actor: %s, at Location: %s"), *Actor->GetName(), *Actor->GetActorLocation().ToString());
	}
	else if (DistanceToPlayer <= AttackRange) // Additional check for distance
	{
		SetState(EAIState_Enemy::Provokable);
		DetectedPlayer = PlayerPawn; 
		FacePlayer();
	}
	else
	{
		SetState(EAIState_Enemy::Patrol);
		DetectedPlayer = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Lost Actor: %s"), *Actor->GetName());
	}

}

void AEnemyAIController::PopulateWaypointsInLevel()
{

	if (CurrentState != EAIState_Enemy::Patrol) // If we're not in patrol state, return
	{
		return;
	}

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


	FVector PlayerDirection = PlayerPawn->GetActorLocation() - MyCharacter->GetActorLocation();
	PlayerDirection.Normalize();

	FRotator NewRotation = PlayerDirection.Rotation();
	MyCharacter->SetActorRotation(FMath::RInterpTo(MyCharacter->GetActorRotation(), NewRotation, GetWorld()->GetDeltaSeconds(), 5.0f));
}

void AEnemyAIController::Provoke(float DeltaTime)
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
			PlayerProximityTime += DeltaTime;
			DrawDebugString(GetWorld(), FVector(0, 0, 100), FString::Printf(TEXT("Player Proximity Time: %f"), PlayerProximityTime), GetPawn(), FColor::Red, 0.0f, true);

			// If the player has been within provocation distance for longer than the threshold, provoke the AI
			if (PlayerProximityTime >= ProvokableTime)
			{
				//DrawDebugString(GetWorld(), FVector(0, 200, 100), FString::Printf(TEXT("Provoked!")), GetPawn(), FColor::Red, 3.0f, true);
				SetState(EAIState_Enemy::Attack);
			}
		}
		else
		{
			// Reset proximity time if the player is outside the provocation distance
			PlayerProximityTime = 0;
			SetState(EAIState_Enemy::Patrol);
		}
	}
}
