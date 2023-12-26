#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoverSystem.generated.h"

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
	TArray<AAICharacter*> PlayerTeam;

	UFUNCTION(BlueprintCallable)
	bool FindBestCoverForCharacter(ACover* &BestCover);

	
	void CheckOccupiedCovers(TArray<ACover*> &CoverCandidates);
	

};
