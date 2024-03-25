// Fill out your copyright notice in the Description page of Project Settings.


#include "Waypoints.h"

// Sets default values
AWaypoints::AWaypoints()
{
 	
	PrimaryActorTick.bCanEverTick = true;


	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	VisualSphere = CreateDefaultSubobject<USphereComponent>(TEXT("VisualSphere"));
	VisualSphere->SetupAttachment(RootComponent);

	VisualSphere->InitSphereRadius(50.f);

}

