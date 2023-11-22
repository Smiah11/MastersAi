// Fill out your copyright notice in the Description page of Project Settings.


#include "AIController1.h"
#include "GameFramework/Character.h"
#include "Testue5Character.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


AAIController1::AAIController1()
{
	CurrentPatrolPointIndex = 0;
}


void AAIController1::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());

    for (int32 i = 0; i < PatrolPoints.Num(); i++)
    {
        FVector CurrentPatrolPoint = PatrolPoints[i]->GetActorLocation();
        FVector MyCharacterLocation = MyCharacter->GetActorLocation();

        if (FVector::DistSquared(CurrentPatrolPoint, MyCharacterLocation) < 100.f * 100.f)
        {
            CurrentPatrolPointIndex = (i + 1) % PatrolPoints.Num();
        }

        FVector Direction = (CurrentPatrolPoint - MyCharacterLocation).GetSafeNormal();
        FVector TargetLocation = MyCharacterLocation + Direction * 100.f;

        UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, PatrolPoints[CurrentPatrolPointIndex]->GetActorLocation());
    }
}
