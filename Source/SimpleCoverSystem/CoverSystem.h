#pragma once

#include "CoreMinimal.h"
#include "AICharacter.h"
#include "GameFramework/Actor.h"
#include "CoverSystem.generated.h"

enum class EDirection : uint8
{
	ED_Left,
	ED_Right,
	ED_Top,
	ED_Bottom,
	ED_None
};

class AAICharacter;
class ACover;
UCLASS()

class SIMPLECOVERSYSTEM_API ACoverSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	ACoverSystem();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	float MinimumDistanceFromEnemyToCover = 300.f;

	UPROPERTY(BlueprintReadOnly)
	TArray<AAICharacter*> EnemyTeam;
	UPROPERTY(BlueprintReadOnly)
	TArray<AAICharacter*> AllyTeam;
protected:
	virtual void BeginPlay() override;

private:

	TArray<ACover*> CoversOnScene;

	UPROPERTY(EditAnywhere, Category = "World Directions")
	FFloatRange LeftDirectionAngleRange = FFloatRange(-45, 45);
	UPROPERTY(EditAnywhere, Category = "World Directions")
	FFloatRange RightDirectionAngleRangePositive = FFloatRange(135, 180);
	UPROPERTY(EditAnywhere, Category = "World Directions")
	FFloatRange RightDirectionAngleRangeNegative = FFloatRange(-180, -135);
	UPROPERTY(EditAnywhere, Category = "World Directions")
	FFloatRange TopDirectionAngleRange = FFloatRange(45, 135);
	UPROPERTY(EditAnywhere, Category = "World Directions")
	FFloatRange BottomDirectionAngleRange = FFloatRange(-135, -45);

	UPROPERTY(EditAnywhere, Category = "ShootingProperties")
	float ShootAngleRangeOffset = 15.f;
	UPROPERTY(EditAnywhere, Category = "ShootingProperties")
	float EffectiveShootDistance = 450.f;
	
	UFUNCTION(BlueprintCallable)
	void SystemInitialize();
	
	UFUNCTION(BlueprintCallable)
	bool FindBestCoverForCharacter(AAICharacter* TargetCharacter, AAICharacter* AimTarget, TArray<AAICharacter*> EnemiesOfTargetCharacter, ACover* &BestCover, bool &bCanShootToAimTargetFromBestCover);
	
	void CheckOccupiedCovers(TArray<ACover*> &CoverCandidates);
	void CheckToCloseFromEnemiesCovers(TArray<AAICharacter*> EnemiesOfTargetCharacter, TArray<ACover*> &CoverCandidates, TArray<ACover*> &TempCoverCandidates);
	void CheckNotSafeCovers(TArray<AAICharacter*> EnemiesOfTargetCharacter, TArray<ACover*> &CoverCandidates, TArray<ACover*> &TempCoverCandidates);
	void CheckShootingPosibility(TArray<ACover*> &CoverCandidates, TArray<ACover*> &TempCoverCandidates, AAICharacter* AimTarget);
	void CheckIfDistanceIsEffective(TArray<ACover*> &CoverCandidates, TArray<ACover*> &TempCoverCandidates, AAICharacter* AimTarget);
	ACover* GetNearestCover(TArray<ACover*> Covers, AAICharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable)
	void ReserveCover(ACover* CoverToReverse, AAICharacter* TargetCharacter);
	UFUNCTION(BlueprintCallable)
	void DeleteReservation(AAICharacter* TargetCharacter);

};
