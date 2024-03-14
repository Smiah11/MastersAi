// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <EnemyAIController.h>

// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    //tick every second
    PrimaryActorTick.TickInterval = 1.0f;

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;


    HungerLevel = FMath::RandRange(0.f, 75.f);
    TirednessLevel = FMath::RandRange(0.f, 75.f);

    
    HungerIncreaseRate = 1.f;
    TirednessIncreaseRate = 0.5f; 
     



}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

    LastHungerUpdateTime = GetWorld()->GetTimeSeconds();
    LastTirednessUpdateTime = GetWorld()->GetTimeSeconds();

    Move();
	
}

float AAICharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);



    // Check if the controller is of the specific AI controller class you're interested in
    AEnemyAIController* EnemyAIController = Cast<AEnemyAIController>(GetController());
    if (EnemyAIController)
    {
        // If the controller is the specific type and the character took damage, change its state
        EnemyAIController->SetProvoked();
    }

    return ActualDamage;
}

void AAICharacter::Move()
{
    UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
    if (MovementComponent && MovementComponent->Velocity.SizeSquared() > 0.0f)
    {
        // The character is moving
        bIsMoving = true;
        UE_LOG(LogTemp, Warning, TEXT("Moving"));

        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

        if (AnimInstance && Moving)
        {
            AnimInstance->Montage_Play(Moving);
        }
    }
    else
    {
        // The character is not moving
        bIsMoving = false;
    }
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //update hunger and tiredness levels with their rates

    HungerLevel+= HungerIncreaseRate * DeltaTime;
    TirednessLevel+= TirednessIncreaseRate * DeltaTime;

    //clamp values
    HungerLevel = FMath::Clamp(HungerLevel, 0.f, 100.f); 
    TirednessLevel = FMath::Clamp(TirednessLevel, 0.f, 100.f);


  //  UE_LOG(LogTemp, Warning, TEXT("Hunger iNCREASE RATE: %f"), HungerIncreaseRate);
 //   UE_LOG(LogTemp, Warning, TEXT("Tiredness iNCREASE RATE: %f"), TirednessIncreaseRate);
	

}


float AAICharacter::GetHungerLevel() const
{
	return HungerLevel;
}

float AAICharacter::GetTirednessLevel() const
{
	return TirednessLevel;
}

void AAICharacter::IncreaseHunger(float Amount)
{
    HungerLevel += Amount;
  
}

void AAICharacter::IncreaseTiredness(float Amount)
{
	TirednessLevel += Amount;
}

void AAICharacter::DecreaseHunger(float Amount)
{
    	HungerLevel -= Amount;
        HungerLevel = FMath::Clamp(HungerLevel, 0.f, 100.f);
    

}

void AAICharacter::DecreaseTiredness(float Amount)
{
	TirednessLevel -= Amount;
    TirednessLevel = FMath::Clamp(TirednessLevel, 0.f, 100.f);
}

float AAICharacter::GetHungerIncreaseRate() const
{
    return HungerIncreaseRate;
}

float AAICharacter::GetTirednessIncreaseRate() const
{
    return TirednessIncreaseRate;
}

void AAICharacter::SetHungerIncreaseRate(float NewRate)
{
	HungerIncreaseRate = NewRate;
}

void AAICharacter::SetTirednessIncreaseRate(float NewRate)
{
    	TirednessIncreaseRate = NewRate;
}

// Called to bind functionality to input
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

