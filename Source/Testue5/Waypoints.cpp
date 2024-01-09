// Fill out your copyright notice in the Description page of Project Settings.


#include "Waypoints.h"

// Sets default values
AWaypoints::AWaypoints()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VisualSphere = CreateDefaultSubobject<USphereComponent>(TEXT("VisualSphere"));
	VisualSphere->SetupAttachment(RootComponent);

	VisualSphere->InitSphereRadius(50.f);

}

