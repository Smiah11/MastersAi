// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Waypoints.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EAIState_Enemy : uint8
{
	Patrol,
	Attack,
	Provokable,
	Investigate
};

struct EnemyActionUtility
{
	EAIState_Enemy Action;
	float Utility;

	EnemyActionUtility(EAIState_Enemy InAction, float InUtility) : Action(InAction), Utility(InUtility) {}
};

UCLASS()
class TESTUE5_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

	

	TArray<EnemyActionUtility> CalculateEnemyUtilities();
	EnemyActionUtility ChooseBestAction(const TArray<EnemyActionUtility>& EnemyActionUtilities) const;

	float CalculatePatrolUtility() const;
	float CalculateAttackUtility() const;
	float CalculateProvokableUtility() const;
	float CalculateInvestigateUtility() const;

	void DecideNextAction();

	void ExecuteAction(EAIState_Enemy Action);
	void SetProvoked();

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void SetInRestrictedZone(bool bRestricted, FVector LastKnownPlayerLocation);

	bool bInRestrictedZone;


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UAISenseConfig_Sight* SightConfig;

	bool FindSuitableNewWaypointLocation(FVector& OutLocation, float MinDistance, int MaxRetries);

	void SpawnNewWaypoint();

	void CheckPlayerProximity();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AWaypoints*> PatrolPoints;
	int32 CurrentPatrolPointIndex;

private:

	EAIState_Enemy CurrentState;
	AActor* DetectedPlayer;
	float AttackRange = 400.f;
	float AttackDamage = 10.f;
	float AttackDistance = 250.f;
	float LastAttackTime; 
	const float AttackCooldown = 2.0f; 
	float ProvokableDistance = 400.f;
	float ProvokableTime = 5.f;
	float PlayerProximityTime = 0.f;

	AActor* LastMovedToActor = nullptr; // Last actor moved to

	FVector LastKnownLocation;


	//Utility Modifiers

	float PatrolUtilityModifier = .5f;
	float AttackUtilityModifier = 1.f;
	float ProvokableUtilityModifier = 3.f;


	float LastProvokedTime = 0;

	bool bIsProvoked;
	bool bIsInProvokableDistance;
	bool bIsPlayerVisible;


	
	void Attack();
	void MoveToNextWaypoint();
	void SetupAI();
	void PopulateWaypointsInLevel();
	void FacePlayer();
	void Provoke();
	void Investigate();
	void ResetInvestigation();




	
};
