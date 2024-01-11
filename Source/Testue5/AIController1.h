// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Waypoints.h"
#include "AIController1.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
	Patrol,
	ReactToPlayer
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

	void AvoidOtherAI();


	

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

	void SetAIState(EAIState NewState);

	void PopulateWaypointsInLevel();



};