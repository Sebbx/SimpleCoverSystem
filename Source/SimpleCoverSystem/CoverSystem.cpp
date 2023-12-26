#include "CoverSystem.h"

#include "AICharacter.h"
#include "Cover.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ACoverSystem::ACoverSystem()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACoverSystem::BeginPlay()
{
	Super::BeginPlay();
}

void ACoverSystem::SystemInitialize() //Dodawanie osłon i postaci do list
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACover::StaticClass(), Actors);

	for (auto Actor : Actors)
	{
		CoversOnScene.Add(Cast<ACover>(Actor));
	}
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAICharacter::StaticClass(), Actors);
	for (const auto Actor: Actors)
	{
		AAICharacter* Character = Cast<AAICharacter>(Actor);
		if(Character->bIsEnemy)
		{
			EnemyTeam.Add(Character);
		}
		else PlayerTeam.Add(Character);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("CoversOnScene: %d"), CoversOnScene.Num()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("EnemyTeam: %d"), EnemyTeam.Num()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("PlayerTeam: %d"), PlayerTeam.Num()));
}

bool ACoverSystem::FindBestCoverForCharacter(ACover* &BestCover)
{
	if (true) // TODO Warunki 
	{
		if (CoversOnScene.Num() < 0 ) return false;
		if (EnemyTeam.Num() < 0 ) return false;
		if (PlayerTeam.Num() < 0 ) return false; 
	}
	
	TArray<ACover*> CoverCandidates;

	/* Selekcja niezarezerwowanych osłon*/
	CheckOccupiedCovers(CoverCandidates);
	{
		if (CoverCandidates.Num() < 1)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("All Covers are reserved"));
			return false;
		}
		if (CoverCandidates.Num() == 1)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Only one free cover found"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("More than one free cover found: %d"), CoverCandidates.Num()));
		}
	}

	//Selekcja osłon niebędących w pobliżu wrogów
	TArray<ACover*> TempCoverCandidates;
	for (auto Cover : CoverCandidates)
	{
		for (auto Enemy : EnemyTeam)
		{
			float Distance = (UKismetMathLibrary::Vector_Distance(Cover->GetCoverPositionVector(), Enemy->GetActorLocation()));
			
			//Jeśli osłona jest dalej niż minimalny dystans od przeciwnika, zostaje dodana do listy TempCoverCandidates
			if (Distance > MinimumDistanceFromEnemyToCover)
			{
				TempCoverCandidates.AddUnique(Cover);
			}
			//Osłona zostaje usunięta z listy TempCoverCandidates, jeżeli następny przeciwnik jest za blisko 
			else
			{
				if (TempCoverCandidates.Find(Cover) != -1) TempCoverCandidates.Remove(Cover);
				break;
			}
		}
	}

	//Jeżeli TempCoverCandidates jest pusty, to oznacza że wszystkie covery były zbyt blisko wrogów aby je brać dalej pod uwagę 
	if(TempCoverCandidates.Num() <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("All covers are to close from enemies"));
		return false;
	}
	//Jeżeli TempCoverCandidates zawiera jakieś elementy, to oznacza że znaleziono x coverów, które są wystarczająco daleko od wrogów i można brać je pod uwagę w dalszych checkach
	if (TempCoverCandidates.Num() > 1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("More than one cover is far enough from enemy: %d"), TempCoverCandidates.Num()));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Only one cover is far enough from enemy"));
	}
	CoverCandidates = TempCoverCandidates;
	TempCoverCandidates = TArray<ACover*>();
	
	
	//TODO debug
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("CoverCandidates: %d"), CoverCandidates.Num()));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("TempCoverCandidates: %d"), TempCoverCandidates.Num()));
	return true;
}

void ACoverSystem::CheckOccupiedCovers(TArray<ACover*> &CoverCandidates)
{
	for (auto Cover : CoversOnScene)
	{
		if (!Cover->bIsReserved) CoverCandidates.Add(Cover);
	}
}

void ACoverSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

