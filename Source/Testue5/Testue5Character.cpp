// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testue5Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <EnemyAIController.h>
#include <Kismet/GameplayStatics.h>

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATestue5Character

ATestue5Character::ATestue5Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ATestue5Character::OnOverlapBegin); // Set up a notification for when this component overlaps something
	// Unreal Engine has a bug where this function is not called when the overlap ends, so I had to use the OnActorEndOverlap function instead (still didnt work)
	//GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ATestue5Character::OnOverlapEnd); 
	//OnActorEndOverlap.AddDynamic(this, &ATestue5Character::OnActorOverlapEnd);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATestue5Character::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor && OtherActor != this && OtherComp && OtherActor->ActorHasTag("Restricted"))
	{
		TArray<AActor*>FoundGuards;// Create an array to store the guards
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyAIController::StaticClass(), FoundGuards);// Get all the guards in the level

		for (AActor* Guard : FoundGuards) 
		{
			AEnemyAIController* GuardController = Cast<AEnemyAIController>(Guard);
			if (GuardController)
			{

				FVector LastKnowLocation = GetActorLocation();
				GuardController->SetInRestrictedZone(true, LastKnowLocation);
				//GuardController->GetCharacter()->GetCharacterMovement()->MaxWalkSpeed = 700.f;
				UE_LOG(LogTemp, Warning, TEXT("Guard has been Notified"));
			}
		}

	}
}


void ATestue5Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATestue5Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATestue5Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATestue5Character::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Triggered, this, &ATestue5Character::Sprinting);
	

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ATestue5Character::Fire);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATestue5Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATestue5Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATestue5Character::Sprinting(const FInputActionValue& Value)
{
	// input is a boolean
	bool bSprint = Value.Get<bool>();

	if (bSprint)
	{
		GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
	}
}

void ATestue5Character::Fire(const FInputActionValue& Value)
{
		// input is a boolean
	bool bFire = Value.Get<bool>();

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (bFire && (CurrentTime - LastFireTime >=FireRate))
	{
		FVector StartLocation = GetActorLocation() + FVector(0, 0, 50); // start location of the raycast
		FRotator Direction = GetControlRotation(); // direction of the raycast
		FVector EndLocation = StartLocation + (Direction.Vector() * 1000); // RAYCAST DISTANCE


		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		// Raycast
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams);

		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Purple, false, 1.f);



		if (bHit && HitResult.GetActor())
		{
			// If we hit something print the hit actor
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName());
			const float Damage = 50.f;
			UGameplayStatics::ApplyDamage(HitResult.GetActor(), Damage, GetController(), this, UDamageType::StaticClass());
			//UE_LOG(LogTemp, Warning, TEXT("Damage Applied: %f"), Damage);

		}
		else
		{
			// If we didn't hit anything
			UE_LOG(LogTemp, Warning, TEXT("No Hit"));
		}
		
		LastFireTime = CurrentTime;

	}
}
