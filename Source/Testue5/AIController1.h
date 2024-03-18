// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Waypoints.h"
#include <AICharacter.h>
#include "AIController1.generated.h"


UENUM(BlueprintType)
enum class EAIState : uint8
{
	Patrol,
	ReactToPlayer,
	GoToWork,
	GoHome,
	GoShop
};

struct CivillianActionUtility
{
	EAIState Action;
	float Utility;

	CivillianActionUtility(EAIState InAction, float InUtility): Action(InAction), Utility(InUtility) {}
};



UCLASS()
class TESTUE5_API AAIController1 : public AAIController
{
	GENERATED_BODY()

public:

	AAIController1();

	void Tick(float DeltaTime) override;

	void BeginPlay() override;

	void SetupAI();
	void MoveToNextWaypoint();

	bool FindSuitableNewWaypointLocation(FVector& OutLocation, float MinDistance, int MaxRetries);

	void SpawnNewWaypoint();

	//void GenerateAndMoveToNextWaypoint(const FVector& NearLocation);

	void ReactToPlayer();

	void FaceLocation(const FVector& Location);

	//void AvoidOtherAI();

	void UpdateCurrentTime(float DeltaTime);

	void InitialiseLocations();

	TArray<CivillianActionUtility> CalculateCurrentUtilities();

	void DecideNextAction();

	void DecreaseHungerValue();

	void DecreaseTirednessValue();

	void DecreaseTirednessAndHungerValue();

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	//float CalculatePriorityUtility() const;



private:

	float CalculatePatrolUtility() const;

	float CalculateReactToPlayerUtility() const;

	float CalculateGoToWorkUtility() const;

	float CalculateGoHomeUtility() const;

	float CalculateGoShopUtility() const;

	void ExecuteAction(EAIState Action);


	const float HigherPriorityModifier = 10.f;
	const float HighPriorityModifier = 2.f;
	const float LowPriorityModifier = 0.25f;
	const float MediumPriorityModifier = 1.5f;


	CivillianActionUtility ChooseBestAction(const TArray<CivillianActionUtility>& ActionUtilities) const;

	FTimerHandle WaitTimer;
	FTimerHandle UpdateTimer;
	FTimerHandle TimerFacePlayer;

	void OnUpdate();

	//void Wait();

	bool bIsWaiting;

	
	float MinDistanceBetweenPoints = 300.f;

	
	float HungerThreshold = 100.f;
	float TirednessThreshold = 100.f;
	float WorkStart = 8.f; // 8am
	float WorkEnd = 15.f; // 3pm

	
	float CurrentTime = 0.f; // Current time of day
	float CurrentHour= 0.f; // Current hour of day



	// Locations
	FVector WorkLocation;
	FVector HomeLocation;
	FVector ShoppingLocation;
	FVector CurrentTargetLocation;// Keeps track of the current location the AI is moving to


	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AvoidanceDistance;

	UPROPERTY (EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ReactionRadius = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AWaypoints*> PatrolPoints;

	int32 CurrentPatrolPointIndex;

	// State machine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	EAIState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage* ReactMontage;


	void PopulateWaypointsInLevel();

	bool IsAtLocation(const FVector& Location, float Radius) const;

	void GoToWork();

	void GoHome();

	void GoShop();



};