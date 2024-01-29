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
	Attack	
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


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimilus);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AWaypoints*> PatrolPoints;
	int32 CurrentPatrolPointIndex;

private:

	EAIState_Enemy CurrentState;
	AActor* DetectedPlayer;
	float AttackRange = 500.f;
	float AttackDamage = 10.f;
	float AttackDistance = 300.f;
	float LastAttackTime; 
	const float AttackCooldown = 2.0f; 

	void SetState(EAIState_Enemy NewState);
	void Patrol();//same as MoveToNextWaypoint in AIController1.h
	void Attack();
	void SetupAI();
	void PopulateWaypointsInLevel();
	void FacePlayer();




	
};
