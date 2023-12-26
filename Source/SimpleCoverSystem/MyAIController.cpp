#include "MyAIController.h"
#include "Perception/AIPerceptionComponent.h"

AMyAIController::AMyAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

void AMyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if(BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}
}
