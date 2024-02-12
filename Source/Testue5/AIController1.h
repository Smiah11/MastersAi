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

	void ReactToPlayer();

	void FacePlayer();

	//void AvoidOtherAI();

	void UpdateCurrentTime(float DeltaTime);

	void InitialiseLocations();



private:

	float CalculatePatrolUtility() const;

	float CalculateReactToPlayerUtility() const;

	float CalculateGoToWorkUtility() const;

	float CalculateGoHomeUtility() const;

	float CalculateGoShopUtility() const;

	void ExecuteAction(EAIState Action);

	CivillianActionUtility ChooseBestAction(const TArray<CivillianActionUtility>& ActionUtilities) const;

	


	
	float HungerThreshold = 50.f;
	float TirednessThreshold = 75.f;
	float WorkStart = 8.f; // 8am
	float WorkEnd = 15.f; // 3pm

	
	float CurrentTime = 0.f; // Current time of day
	float CurrentHour= 0.f; // Current hour of day


	// Locations
	FVector WorkLocation;
	FVector HomeLocation;
	FVector ShoppingLocation;


	

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

	void SetAIState(EAIState NewState);

	void PopulateWaypointsInLevel();

	void GoToWork();

	void GoHome();

	void GoShop();



};