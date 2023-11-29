// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Waypoint_Manual.h"
#include "AIController2.generated.h"

/**
 * 
 */
UCLASS()
class TESTUE5_API AAIController2 : public AAIController
{
	GENERATED_BODY()


public:

	AAIController2();

	void Tick(float DeltaTime) override;

	void BeginPlay() override;

	void SetupAI();
	void MoveToNextWaypoint();
	


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AWaypoint_Manual*> PatrolPoints;

	int32 CurrentPatrolPointIndex;

	void PopulateWaypointsArray();

};
