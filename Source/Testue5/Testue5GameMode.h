// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Testue5GameMode.generated.h"

UCLASS(minimalapi)
class ATestue5GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:


	ATestue5GameMode();


public:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	float CurrentTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " Global Time")
	float CurrentHour = 0.0f;

	UFUNCTION(BlueprintCallable)
	void OnTimeSliderValueChanged(float SliderValue);

	void UpdateTime(float DeltaTime);

	bool IsSliderActive = false;

	void SetSliderFalse();

};



