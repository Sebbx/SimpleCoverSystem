#pragma once

#include "CoreMinimal.h"
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

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION(BlueprintCallable)
	void SystemInitialize();

	TArray<ACover*> CoversOnScene;
	TArray<AAICharacter*> EnemyTeam;
	TArray<AAICharacter*> AllyTeam;

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

	
	UFUNCTION(BlueprintCallable)
	bool FindBestCoverForCharacter(ACover* &BestCover);
	
	void CheckOccupiedCovers(TArray<ACover*> &CoverCandidates);
	void CheckToCloseFromEnemiesCovers(TArray<ACover*> &CoverCandidates, TArray<ACover*> &TempCoverCandidates);
	void CheckNotSafeCovers(TArray<ACover*> &CoverCandidates, TArray<ACover*> &TempCoverCandidates);
	

};
