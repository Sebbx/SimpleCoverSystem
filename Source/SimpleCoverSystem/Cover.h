#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cover.generated.h"

UCLASS()
class SIMPLECOVERSYSTEM_API ACover : public AActor
{
	GENERATED_BODY()
	
public:	
	ACover();
	
	UPROPERTY(EditAnywhere)
	class USceneComponent* CoverPosition; //SceneComponent do łatwego ustawienia docelowej pozycji przy osłonie

	FVector GetCoverPositionVector();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* CoverMesh;
	
public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	bool bIsReserved = false;

};
