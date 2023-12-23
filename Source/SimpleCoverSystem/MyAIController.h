#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MyAIController.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLECOVERSYSTEM_API AMyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMyAIController();
	
protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

private:
	UPROPERTY(VisibleAnywhere, Category = AI)
	class UAIPerceptionComponent* AIPerceptionComponent;
	
};
