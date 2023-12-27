#include "CoverSystem.h"
#include "AICharacter.h"
#include "Cover.h"
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
void ACoverSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		else AllyTeam.Add(Character);
	}
}

bool ACoverSystem::FindBestCoverForCharacter(AAICharacter* TargetCharacter, AAICharacter* AimTarget, TArray<AAICharacter*> EnemiesOfTargetCharacter, ACover* &BestCover, bool &bCanShootToAimTargetFromBestCover)
{
	{
		if (!TargetCharacter)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Select target character"));
			return false;
		}
		if (EnemiesOfTargetCharacter.Num() < 1)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Missing Enemies of target character"));
		}
		if (CoversOnScene.Num() < 0 ) return false;
		if (EnemyTeam.Num() < 0 ) return false;
		if (AllyTeam.Num() < 0 ) return false; 
	}
	TArray<ACover*> CoverCandidates;
	TArray<ACover*> TempCoverCandidates;

	//Selekcja niezarezerwowanych osłon
	CheckOccupiedCovers(CoverCandidates);
	{
		if (CoverCandidates.Num() < 1)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("All Covers are reserved"));
			return false;
		}
	}

	//Selekcja osłon niebędących w pobliżu wrogów
	CheckToCloseFromEnemiesCovers(EnemiesOfTargetCharacter, CoverCandidates, TempCoverCandidates);
	{
		//Jeżeli TempCoverCandidates jest pusty, to oznacza że wszystkie covery były zbyt blisko wrogów aby je brać dalej pod uwagę 
		if(TempCoverCandidates.Num() <= 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("All covers are to close from enemies"));
			return false;
		}
		//Jeżeli TempCoverCandidates zawiera jakieś elementy, to oznacza że znaleziono x coverów, które są wystarczająco daleko od wrogów i można brać je pod uwagę w dalszych checkach
		CoverCandidates = TempCoverCandidates;
		TempCoverCandidates = TArray<ACover*>();
	}

	//Selekcja osłon chroniących przed wrogami
	CheckNotSafeCovers(EnemiesOfTargetCharacter, CoverCandidates, TempCoverCandidates);
	{
		//Jeżeli TempCoverCandidates jest pusty, to oznacza, że nie ma osłony, która zapewni ochronę przed wrogiem
		if(TempCoverCandidates.Num() <= 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Covers are not safe"));
			return false;
		}
		//Jeżeli TempCoverCandidates ma więcej niż 1 element, to można dalej wybrać lepszą osłonę, obliczenia zostają kontynuowane
		//Jeżeli TempCoverCandidates ma jeden element, oznacza to że tylko jedna osłona jest bezpieczna. W tym momencie funkcja zwraca najlepszą osłonę, nie ma sensu dalej kontynuować obliczeń
		if (TempCoverCandidates.Num() < 1)
		{
			if (!TempCoverCandidates.IsEmpty()) BestCover = TempCoverCandidates [0];
			return true;
		}
		CoverCandidates = TempCoverCandidates;
		TempCoverCandidates = TArray<ACover*>();
	}
	
	//Selekcja osłon z możliwością strzału
	if (AimTarget)
	{
		CheckShootingPosibility(CoverCandidates, TempCoverCandidates, AimTarget);
		//Funkcja nie zwraca tutaj, ponieważ są conajmniej 2 osłony które mogą jeszcze powalczyć o miano najlepszej osłony
		if(TempCoverCandidates.Num() <= 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("No cover with shooting posibility found"));
			bCanShootToAimTargetFromBestCover = false;
		}
		//Zwraca boola, który może być przydatny przy opracowaniu mechaniki strzelania
		else if(TempCoverCandidates.Num() > 1)
		{
			CoverCandidates = TempCoverCandidates;
			TempCoverCandidates = TArray<ACover*>();
			bCanShootToAimTargetFromBestCover = true;
		}
		else if (TempCoverCandidates.Num() == 1)
		{
			BestCover = CoverCandidates[0];
			bCanShootToAimTargetFromBestCover = true;
			return true;
		}
	}

	//Selekcja osłon w efektywnym zasięgu strzału od wroga
	if (AimTarget && bCanShootToAimTargetFromBestCover)
	{
		CheckIfDistanceIsEffective(CoverCandidates, TempCoverCandidates, AimTarget);
		if(TempCoverCandidates.Num() > 1)
		{
			CoverCandidates = TempCoverCandidates;
			TempCoverCandidates = TArray<ACover*>();
		}
		else if (TempCoverCandidates.Num() == 1)
		{
			BestCover = CoverCandidates[0];
			return true;
		}
	}

	//Wybór najbliższej osłony
	BestCover = GetNearestCover(CoverCandidates, TargetCharacter);
	return true;
}

void ACoverSystem::CheckOccupiedCovers(TArray<ACover*> &CoverCandidates)
{
	//Dodanie do listy CoverCandidates osłon, które nie są zarezerwowane
	for (auto Cover : CoversOnScene)
	{
		if (!Cover->bIsReserved) CoverCandidates.Add(Cover);
	}
}
void ACoverSystem::CheckToCloseFromEnemiesCovers(TArray<AAICharacter*> EnemiesOfTargetCharacter, TArray<ACover*> &CoverCandidates, TArray<ACover*> &TempCoverCandidates)
{
	for (auto Cover : CoverCandidates)
	{
		for (auto Enemy : EnemiesOfTargetCharacter)
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
}
void ACoverSystem::CheckNotSafeCovers(TArray<AAICharacter*> EnemiesOfTargetCharacter, TArray<ACover*>& CoverCandidates, TArray<ACover*>& TempCoverCandidates)
{
	for (auto Cover : CoverCandidates)
	{
		for (auto Enemy : EnemiesOfTargetCharacter)
		{
			EDirection EnemyDirection = EDirection::ED_None;
			// Usuwa osłonę z TempCoverCandidates, jeśli będzie ona bezpieczna przed każdym wrogiem i tak zostanie dodana. Zapobiega to zostawieniu w tablicy osłony, która będzie niebezpieczna
			if (TempCoverCandidates.Contains(Cover)) TempCoverCandidates.Remove(Cover);

			float TargetYawLookAtRotation = UKismetMathLibrary::FindRelativeLookAtRotation(Cover->GetCoverPositionTransform(), Enemy->GetActorLocation()).Yaw;

			//Wyznaczenie kierunku, w którym jest dany wróg od danej osłony
			if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, LeftDirectionAngleRange.GetLowerBoundValue(), LeftDirectionAngleRange.GetUpperBoundValue()))
			{
				EnemyDirection = EDirection::ED_Left;
			}
			else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, RightDirectionAngleRangePositive.GetLowerBoundValue(), RightDirectionAngleRangePositive.GetUpperBoundValue()))
			{
				EnemyDirection = EDirection::ED_Right;
			}
			else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, RightDirectionAngleRangeNegative.GetLowerBoundValue(), RightDirectionAngleRangeNegative.GetUpperBoundValue()))
			{
				EnemyDirection = EDirection::ED_Right;
			}
			else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, TopDirectionAngleRange.GetLowerBoundValue(), TopDirectionAngleRange.GetUpperBoundValue()))
			{
				EnemyDirection = EDirection::ED_Top;
			}
			else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, BottomDirectionAngleRange.GetLowerBoundValue(), BottomDirectionAngleRange.GetUpperBoundValue()))
			{
				EnemyDirection = EDirection::ED_Bottom;
			}

			//Jeżeli strona osłony, z której jest wróg jest chroniona, osłona zostaje dodana do listy TempCoverCandidates, ale pętla sprawdzi czy następny wróg nie jest zagrożeniem
			//Jeżeli żadna strona nie jest bezpieczna, osłona nie zostanie dodana do listy, oraz pętla zostanie przerwana. Zapobiegnie to dodaniu do listy osłony, która chroni przed jednym wrogiem, a drugim nie
			switch (EnemyDirection)
			{
			case EDirection::ED_Left:
				{
					if (Cover->bIsLeftProtected)
					{
						TempCoverCandidates.AddUnique(Cover);
						continue;
					}
				} break;
			case EDirection::ED_Right:
				{
					if (Cover->bIsRightProtected)
					{
						TempCoverCandidates.AddUnique(Cover);
						continue;
					}
				} break;
			case EDirection::ED_Top:
				{
					if (Cover->bIsTopProtected)
					{
						TempCoverCandidates.AddUnique(Cover);
						continue;
					}
				} break;
			case EDirection::ED_Bottom:
				{
					if (Cover->bIsBottomProtected)
					{
						TempCoverCandidates.AddUnique(Cover);
						continue;
					}
				} break;
			default: break;
			}
			break;
		}
	}
}
void ACoverSystem::CheckShootingPosibility(TArray<ACover*>& CoverCandidates, TArray<ACover*>& TempCoverCandidates, AAICharacter* AimTarget)
{
	if (!AimTarget) return;

	for (auto Cover : CoverCandidates)
	{
		EDirection EnemyDirection = EDirection::ED_None;
		float TargetYawLookAtRotation = UKismetMathLibrary::FindRelativeLookAtRotation(Cover->GetCoverPositionTransform(), AimTarget->GetActorLocation()).Yaw;

		//Sprawdzanie, czy target jest w stożku 60 stopni w każdym z czterech kierunków
		if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, LeftDirectionAngleRange.GetLowerBoundValue() + ShootAngleRangeOffset, LeftDirectionAngleRange.GetUpperBoundValue() - ShootAngleRangeOffset))
		{
			EnemyDirection = EDirection::ED_Left;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, RightDirectionAngleRangePositive.GetLowerBoundValue() + ShootAngleRangeOffset, RightDirectionAngleRangePositive.GetUpperBoundValue()))
		{
			EnemyDirection = EDirection::ED_Right;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, RightDirectionAngleRangeNegative.GetLowerBoundValue(), RightDirectionAngleRangeNegative.GetUpperBoundValue() - ShootAngleRangeOffset))
		{
			EnemyDirection = EDirection::ED_Right;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, TopDirectionAngleRange.GetLowerBoundValue() + ShootAngleRangeOffset, TopDirectionAngleRange.GetUpperBoundValue() - ShootAngleRangeOffset))
		{
			EnemyDirection = EDirection::ED_Top;
		}
		else if (UKismetMathLibrary::InRange_FloatFloat(TargetYawLookAtRotation, BottomDirectionAngleRange.GetLowerBoundValue() + ShootAngleRangeOffset, BottomDirectionAngleRange.GetUpperBoundValue() - ShootAngleRangeOffset))
		{
			EnemyDirection = EDirection::ED_Bottom;
		}

		if (EnemyDirection != EDirection::ED_None)
		{
			TempCoverCandidates.AddUnique(Cover);
		}
	}
}
void ACoverSystem::CheckIfDistanceIsEffective(TArray<ACover*>& CoverCandidates, TArray<ACover*>& TempCoverCandidates, AAICharacter* AimTarget)
{
	for (auto Cover : CoverCandidates)
	{
		float Distance = (UKismetMathLibrary::Vector_Distance(Cover->GetCoverPositionVector(), AimTarget->GetActorLocation()));
		
		if (Distance <= EffectiveShootDistance) TempCoverCandidates.AddUnique(Cover);
	}
}
ACover* ACoverSystem::GetNearestCover(TArray<ACover*> Covers, AAICharacter* TargetCharacter)
{
	ACover* TempBestCover = Covers[0];
	for (auto Cover : Covers)
	{
		if(
			UKismetMathLibrary::Vector_Distance(Cover->GetCoverPositionVector(), TargetCharacter->GetActorLocation()) <
			UKismetMathLibrary::Vector_Distance(TempBestCover->GetCoverPositionVector(), TargetCharacter->GetActorLocation())
			) TempBestCover = Cover;
	}
	return TempBestCover;
}

void ACoverSystem::ReserveCover(ACover* CoverToReverse, AAICharacter* TargetCharacter)
{
	if (!CoverToReverse || !TargetCharacter) return;

	if(CoverToReverse->bIsReserved) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Cover is already reserved"));
	else
	{
		if (TargetCharacter->ReservedCover)
		{
			TargetCharacter->ReservedCover->bIsReserved = false;
		}
		TargetCharacter->ReservedCover = CoverToReverse;
		CoverToReverse->bIsReserved = true;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Reservation Successful"));
	}
}
void ACoverSystem::DeleteReservation(AAICharacter* TargetCharacter)
{
	if (!TargetCharacter) return;
	if (TargetCharacter->ReservedCover)
	{
		TargetCharacter->ReservedCover->bIsReserved = false;
		TargetCharacter->ReservedCover = nullptr;
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Reservation Deleted"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Character has no reservation"));
	}
}




