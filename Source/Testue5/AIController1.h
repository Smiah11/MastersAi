// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Waypoints.h"
#include "AIController1.generated.h"


/**
 * 
 */
UCLASS()
class TESTUE5_API AAIController1 : public AAIController
{
	GENERATED_BODY()

	public:

		AAIController1();

		void Tick(float DeltaTime) override;

		void BeginPlay() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AWaypoints*> PatrolPoints;

	int32 CurrentPatrolPointIndex;

	void PopulateWaypointsInLevel();

	
};
