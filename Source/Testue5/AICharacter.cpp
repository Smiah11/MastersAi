// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;


}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

    Move();
	
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

}

// Called to bind functionality to input
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

