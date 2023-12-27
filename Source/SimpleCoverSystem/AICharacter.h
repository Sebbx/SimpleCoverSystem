// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Cover.h"
#include "AICharacter.generated.h"

UCLASS()
class SIMPLECOVERSYSTEM_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAICharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadOnly)
	bool bIsEnemy = false;
	
	UPROPERTY(BlueprintReadOnly)
	ACover* ReservedCover;

};
