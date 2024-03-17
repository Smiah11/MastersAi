// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "EnemyAIController.h"
#include "AICharacter.generated.h"

UCLASS()
class TESTUE5_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
	UAnimMontage* Moving;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UMaterial* GuardMaterial;

	void Move();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	float GetHungerLevel() const;
	float GetTirednessLevel() const;

	void IncreaseHunger(float Amount);
	void IncreaseTiredness(float Amount);

	void DecreaseHunger(float Amount);
	void DecreaseTiredness(float Amount);

	float GetHungerIncreaseRate() const;
	float GetTirednessIncreaseRate() const;

	void SetHungerIncreaseRate(float NewRate);
	void SetTirednessIncreaseRate(float NewRate);

	void SetGuardMaterial();

private:

	float LastHungerUpdateTime;
	float LastTirednessUpdateTime;

	float HungerLevel;
	float TirednessLevel;

	float HungerIncreaseRate;
	float TirednessIncreaseRate;


};
