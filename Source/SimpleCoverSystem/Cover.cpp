// Fill out your copyright notice in the Description page of Project Settings.


#include "Cover.h"

// Sets default values
ACover::ACover()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CoverPosition = CreateDefaultSubobject<USceneComponent>(TEXT("CoverPosition"));
	CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh"));
	
	CoverPosition->AttachToComponent(CoverMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	bGenerateOverlapEventsDuringLevelStreaming = true;

}

FVector ACover::GetCoverPositionVector()
{
	return CoverPosition->GetComponentLocation();
}

// Called when the game starts or when spawned
void ACover::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACover::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

