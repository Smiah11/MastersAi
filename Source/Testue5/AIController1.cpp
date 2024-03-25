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
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/GameplayStatics.h"



//Civillian Controller


AAIController1::AAIController1()
{
	CurrentPatrolPointIndex = 0;
	//CurrentState = EAIState::Patrol; // Default state is Patrol
	//PrimaryActorTick.TickInterval = 1.f;
	//PrimaryActorTick.bCanEverTick = false;

	
}

void AAIController1::BeginPlay()
{
	Super::BeginPlay();
	InitialiseLocations();// Set up the initial locations for the AI
	PopulateWaypointsInLevel();
	SetupAI();


	// Set up a timer to update the AI every 2 seconds reducing tick reliance
	GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &AAIController1::OnUpdate, 0.1f, true,0.f);

	CurrentTime = 14 * 60 * 60; // 2pm for testing purposes


}


void AAIController1::Tick(float DeltaTime)
{

	//UE_LOG(LogTemp, Warning, TEXT("Current AI State: %s"), *UEnum::GetValueAsString(CurrentState));

	Super::Tick(DeltaTime);


	if (!bIsSliderActive)
	{

		UpdateCurrentTime(DeltaTime); // Update the current time
		
	}



	//FaceLocation(CurrentTargetLocation);

	//UE_LOG(LogTemp, Log, TEXT("Current Hour: %f"), CurrentHour); // Print the current hour to the log

	

	//UpdateCurrentTime(DeltaTime);

	//AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());

	//UE_LOG(LogTemp, Log, TEXT("At Shop - Tiredness: %f, Hunger: %f"), MyCharacter->GetTirednessLevel(), MyCharacter->GetHungerLevel());

	//float PriorityUtility = CalculatePriorityUtility();

	/*
	TArray<CivillianActionUtility> ActionUtilities = {
	{EAIState::Patrol, CalculatePatrolUtility() }, // Patrol has a lower priority than other actions
	{EAIState::ReactToPlayer, CalculateReactToPlayerUtility() },
	{EAIState::GoToWork, CalculateGoToWorkUtility() },
	{EAIState::GoHome, CalculateGoHomeUtility() },
	{EAIState::GoShop, CalculateGoShopUtility() }
	};

	CivillianActionUtility BestAction = ChooseBestAction(ActionUtilities);
	ExecuteAction(BestAction.Action);
	*/
	

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

	if (PatrolPoints.Num() > 0 && CurrentPatrolPointIndex < PatrolPoints.Num()) 
	{
		AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());

		AWaypoints* NextWaypoint = PatrolPoints[CurrentPatrolPointIndex];
		if (NextWaypoint)
		{
			
			//FaceLocation(NextWaypoint->GetActorLocation());
			CurrentTargetLocation = NextWaypoint->GetActorLocation();
			MoveToActor(NextWaypoint, 50.f, true);
		}
	}
	else
	{
		
		UE_LOG(LogTemp, Warning, TEXT("No waypoints available or index out of bounds."));
	}

}

bool AAIController1::FindSuitableNewWaypointLocation(FVector& OutLocation, float MinDistance, int MaxRetries)
{
	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation RandomLocation;

	for (int RetryCount = 0; RetryCount < MaxRetries; ++RetryCount) // Try to find a suitable location
	{
		if (NavSystem && NavSystem->GetRandomPointInNavigableRadius(MyCharacter->GetActorLocation(), 2000.0f, RandomLocation)) 
		{
			FVector NewLocation = RandomLocation.Location;
			if (FVector::DistSquared(NewLocation, MyCharacter->GetActorLocation()) >= FMath::Square(MinDistance))
			{
				OutLocation = NewLocation;
				return true; // Successful in finding a suitable location
			}
		}
	}

	return false; // Failed to find a suitable location
}

void AAIController1::SpawnNewWaypoint()
{
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

		FVector PlayerLocation = PlayerPawn->GetActorLocation();

		FaceLocation(PlayerLocation);
		CurrentTargetLocation = PlayerLocation;

		UAnimInstance* AnimInstance = MyCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(ReactMontage, 1.0f);
		}
		
	}

}




void AAIController1::FaceLocation(const FVector& Location)
{
	APawn* MyPawn = GetPawn();
	if (MyPawn)
	{
		FVector Direction = (Location - MyPawn->GetActorLocation()).GetSafeNormal();
		FRotator TargetRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;
		MyPawn->SetActorRotation(TargetRotation);
	}
}


void AAIController1::UpdateCurrentTime(float DeltaTime)
{


	if (!bIsSliderActive)
	{


		float SecondsInADay = 24 * 60 * 60; // 24 hours in game
		float TimeScale = SecondsInADay / 600; // Real-life seconds to in-game time (10 mins)

		// faster time progression
		CurrentTime += DeltaTime * TimeScale;

		// loops back to 0 when it reaches the end of the day
		CurrentTime = FMath::Fmod(CurrentTime, SecondsInADay);

	}

	// Convert seconds to hours for the current hour.
	CurrentHour = CurrentTime / 3600; 

	//UE_LOG(LogTemp, Warning, TEXT("Current Hour: %f"), CurrentHour);
}

void AAIController1::OnSliderValueChange(float SliderValue)
{

	CurrentTime = SliderValue * 3600; 
	bIsSliderActive = true;

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AAIController1::SetSliderFalse, 3.0f, false);

}

void AAIController1::SetSliderFalse()
{
		bIsSliderActive = false;
}



float AAIController1::CalculatePatrolUtility() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	float DistanceToPlayer = FVector::Distance(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation()); 
	float Utility = (DistanceToPlayer / 1000.0f) * LowPriorityModifier; // The further the player, the higher the utility
	//UE_LOG(LogTemp, Log, TEXT("Patrol Utility: %f"), Utility);
	//UE_LOG(LogTemp, Log, TEXT("Patrol Utility Before Modifier: %f, Modifier: %f, Utility After Modifier: %f"), DistanceToPlayer / 1000.0f, LowPriorityModifier, Utility);
	return Utility* LowPriorityModifier;
}

float AAIController1::CalculateReactToPlayerUtility() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	float DistanceToPlayer = FVector::Distance(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
	float Utility = (DistanceToPlayer < ReactionRadius) ? (ReactionRadius - DistanceToPlayer) / ReactionRadius : 0.f; // The closer the player, the higher the utility
	//UE_LOG(LogTemp, Log, TEXT("ReactToPlayer Utility: %f"), Utility);
	return Utility * HigherPriorityModifier;
}

float AAIController1::CalculateGoToWorkUtility() const
{
	bool IsWorkTime = CurrentHour >= WorkStart && CurrentHour < WorkEnd;// Check if it's work time
	float Utility = IsWorkTime ? 12.f : 0.f; // If it's work time, the utility is 1, otherwise it's 0
	//UE_LOG(LogTemp, Log, TEXT("GoToWork Utility: %f"), Utility);
	return Utility;
}

float AAIController1::CalculateGoHomeUtility() const
{
	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	float TirednessLevel = MyCharacter->GetTirednessLevel(); 
	float BedTime = 22.f; // 10 PM
	//bool IsPastWorkHours = CurrentHour >= WorkEnd;
	float Utility = (TirednessLevel >= TirednessThreshold) || (CurrentHour >=BedTime) || ( CurrentHour < WorkStart) ? 10.f : 0.f; // If the character is tired or it's bed time hours, the utility is 1, otherwise it's 0
	//UE_LOG(LogTemp, Log, TEXT("GoHome Utility: %f"), Utility);
	return Utility;
}

float AAIController1::CalculateGoShopUtility() const
{
	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	float HungerLevel = MyCharacter->GetHungerLevel(); 
	float Utility = ((HungerLevel >= HungerThreshold) ? 8.f : 0.f); 
	//UE_LOG(LogTemp, Log, TEXT("GoShop Utility: %f"), Utility);
	return Utility;
}

CivillianActionUtility AAIController1::ChooseBestAction(const TArray<CivillianActionUtility>& ActionUtilities) const {

	check(ActionUtilities.Num() > 0); // Ensure there's at least one action

	CivillianActionUtility BestAction = ActionUtilities[0]; // Start with the first action as the best choice
	for (const auto& ActionUtility : ActionUtilities) {
		if (ActionUtility.Utility > BestAction.Utility) {
			BestAction = ActionUtility; // Found a better action
		}
	}

	return BestAction;
}

void AAIController1::OnUpdate() 
{
	//UpdateCurrentTime(GetWorld()->GetDeltaSeconds());
	DecideNextAction();

	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	//UE_LOG(LogTemp, Log, TEXT("Stats - Tiredness: %f, Hunger: %f"), MyCharacter->GetTirednessLevel(), MyCharacter->GetHungerLevel());

}

/*void AAIController1::Wait()
{
	GetWorld()->GetTimerManager().SetTimer(WaitTimer, this, &AAIController1::DecreaseHungerValue, 5.f, false);
	UE_LOG(LogTemp, Warning, TEXT("Waiting for 5 seconds"));
}*/


void AAIController1::ExecuteAction(EAIState Action) 
{
	switch (Action) {
	case EAIState::Patrol:
		MoveToNextWaypoint();
		break;
	case EAIState::ReactToPlayer:
		ReactToPlayer();
		break;
	case EAIState::GoToWork:
		GoToWork();
		break;
	case EAIState::GoHome:
		GoHome();
		break;
	case EAIState::GoShop:
		
		GoShop();
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unhandled action in ExecuteAction"));
		break;


	}
}


void AAIController1::InitialiseLocations() {
	UWorld* World = GetWorld();
	if (!World) return;

	// Find Work Location
	TArray<AActor*> WorkActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Work"), WorkActors); 
	//UE_LOG(LogTemp, Warning, TEXT("Found %d Work location(s)"), WorkActors.Num());
	if (WorkActors.Num() > 0) 
	{
		int RandomIndex = FMath::RandRange(0, WorkActors.Num() - 1);// Randomly select a work location
		WorkLocation = WorkActors[RandomIndex]->GetActorLocation();
		
		//UE_LOG(LogTemp, Warning, TEXT("Work Location: %s"), *WorkLocation.ToString()); 

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Work location found"));
	}

	// Find Home Location
	TArray<AActor*> HomeActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Home"), HomeActors); 
	//UE_LOG(LogTemp, Warning, TEXT("Found %d Home location(s)"), HomeActors.Num());
	if (HomeActors.Num() > 0) 
	{
		int RandomIndex = FMath::RandRange(0, HomeActors.Num() - 1);// Randomly select a House location
		HomeLocation = HomeActors[RandomIndex]->GetActorLocation();

		//UE_LOG(LogTemp, Warning, TEXT("Home Location: %s"), *HomeLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Home location found"));
	}

	// Find Shopping Location
	TArray<AActor*> ShopActors;
	UGameplayStatics::GetAllActorsWithTag(World, FName("Shop"), ShopActors);
	//UE_LOG(LogTemp, Warning, TEXT("Found %d Shop location(s)"), ShopActors.Num());
	if (ShopActors.Num() > 0) 
	{

		float MinDistance = FLT_MAX; // Start with the maximum possible distance
		AActor* ClosestShop = nullptr;

		for (AActor* ShopActor : ShopActors)
		{
			float Distance = FVector::Dist(HomeLocation, ShopActor->GetActorLocation());
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestShop = ShopActor;
			}
		}

		if (ClosestShop != nullptr)
		{
			ShoppingLocation = ClosestShop->GetActorLocation();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Shop location found"));
	}

}

TArray<CivillianActionUtility> AAIController1::CalculateCurrentUtilities() 
{
	TArray<CivillianActionUtility> actionUtilities;

	// Calculate the utility of each action
	actionUtilities.Add({ EAIState::Patrol, CalculatePatrolUtility() });
	actionUtilities.Add({ EAIState::ReactToPlayer, CalculateReactToPlayerUtility() });
	actionUtilities.Add({ EAIState::GoToWork, CalculateGoToWorkUtility() });
	actionUtilities.Add({ EAIState::GoHome, CalculateGoHomeUtility() });
	actionUtilities.Add({ EAIState::GoShop, CalculateGoShopUtility() });

	return actionUtilities;
}

void AAIController1::DecideNextAction()
{
	CivillianActionUtility nextAction = ChooseBestAction(CalculateCurrentUtilities());
	UE_LOG(LogTemp, Warning, TEXT("Deciding next action: %d with utility %f"), nextAction.Action, nextAction.Utility);
	ExecuteAction(nextAction.Action); 
}

void AAIController1::DecreaseHungerValue()
{
	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	MyCharacter->DecreaseHunger(100.f);
	DecideNextAction();
	//UE_LOG(LogTemp, Log, TEXT("Hunger decreased. New value: %f"), MyCharacter->GetHungerLevel());
}

void AAIController1::DecreaseTirednessValue()
{
	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	MyCharacter->DecreaseTiredness(100.f);
	DecideNextAction();
	//UE_LOG(LogTemp, Log, TEXT("Tiredness decreased. New value: %f"), MyCharacter->GetHungerLevel());
}

void AAIController1::DecreaseTirednessAndHungerValue()
{
	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	MyCharacter->DecreaseHunger(100.f);
	MyCharacter->DecreaseTiredness(100.f);
	DecideNextAction();

}


void AAIController1::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.IsSuccess())
	{

		SpawnNewWaypoint();
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AAIController1::DecideNextAction, 1.f, false);
		//DecideNextAction();
		
	}

}



/*
float AAIController1::CalculatePriorityUtility() const
{

	AAICharacter* MyCharacter = Cast<AAICharacter>(GetPawn());
	float TirednessLevel = MyCharacter->GetTirednessLevel();	
	float HungerLevel = MyCharacter->GetHungerLevel();


	// assigns weight to the tiredness and hunger levels to prioritise one over the other
	float WeightedTiredness = TirednessLevel * 3.f;
	float WeightedHunger = HungerLevel * 2.f;


	// calculates the combined utility of the tiredness and hunger levels because the AI will prioritise the one with the highest utility
	float CombinedUtility = WeightedTiredness + WeightedHunger;

	return CombinedUtility;
}
*/

bool AAIController1::IsAtLocation(const FVector& Location, float Radius) const
{
	AAICharacter* Civillian = Cast<AAICharacter>(GetPawn());

	return FVector::DistSquared(Civillian->GetActorLocation(), Location) <= FMath::Square(Radius);
}

void AAIController1::GoToWork()
{

	AAICharacter* Civillian = Cast<AAICharacter>(GetPawn());


	if (!IsAtLocation(WorkLocation, 500.f))
	{
		MoveToLocation(WorkLocation, 250.f, true);
		//FaceLocation(WorkLocation);
		CurrentTargetLocation = WorkLocation;
	}
	else {

		// increase the hunger and tiredness rates 
		Civillian->SetHungerIncreaseRate(2.f);
		Civillian->SetTirednessIncreaseRate(1.f);
		StopMovement();
	
	}

}

void AAIController1::GoHome()
{
	AAICharacter* Civillian = Cast<AAICharacter>(GetPawn());

	FTimerHandle RestTimer;

	if (!IsAtLocation(HomeLocation, 500.f))
	{
		MoveToLocation(HomeLocation, 250.f, true);
		//FaceLocation(HomeLocation);
		CurrentTargetLocation = HomeLocation;
	}
	else
	{

		// return back to the original hunger and tiredness increase rates
		Civillian->SetHungerIncreaseRate(1.f);
		Civillian->SetTirednessIncreaseRate(0.5f);

		StopMovement();
		GetWorld()->GetTimerManager().SetTimer(RestTimer, this, &AAIController1::DecreaseTirednessAndHungerValue, 10.f, false);
		//Maybe if after they wake up for work they have random tiredness and hunger instead of starting at zero
		
		
	}
	
}

void AAIController1::GoShop()
{

	FTimerHandle ShoppingTimer;

	if (!IsAtLocation(ShoppingLocation, 500.f))
	{
		MoveToLocation(ShoppingLocation, 250.f, true); // bring the AI closer to the shop
		//FaceLocation(ShoppingLocation);
		CurrentTargetLocation = ShoppingLocation;
		//
	}
	else {

		StopMovement();
		GetWorld()->GetTimerManager().SetTimer(ShoppingTimer, this, &AAIController1::DecreaseHungerValue, 5.f, false);


	}
	
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


/*
void AAIController1::SetAIState(EAIState NewState)
{
		CurrentState = NewState;
		//UE_LOG(LogTemp, Warning, TEXT("AI State set to %s"), *UEnum::GetValueAsString(NewState));
}
*/
void AAIController1::PopulateWaypointsInLevel()
{
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());


	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());


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




