//Unit.cpp

#include "PAA/Public/Unit.h"
#include "PAA/Public/Grid.h"
#include "PAA/Public/GridCell.h"
#include "PAA/Public/GameManager.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUnit::AUnit()
{
	PrimaryActorTick.bCanEverTick = false;

	UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
	RootComponent = UnitMesh;

	UnitMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	UnitMesh->SetGenerateOverlapEvents(true);
	UnitMesh->OnClicked.AddDynamic(this, &AUnit::OnUnitClicked);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		UnitMesh->SetStaticMesh(MeshAsset.Object);
	}

	Health = 20;   
	MaxHealth = 20;
	MaxMovement = 3; 
	AttackRange = 1;   
	GridPosition = FIntPoint(0, 0);
	bHasAttacked = false;
	bHasMoved = false;
}

void AUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

AGridCell* AUnit::GetCurrentCell() const
{
	AGrid* Grid = nullptr;
	for (TActorIterator<AGrid> It(GetWorld()); It; ++It)
	{
		Grid = *It;
		break;
	}

	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT("Griglia non trovata!"));
		return nullptr;
	}

	AGridCell* Cell = Grid->GetCellAtPosition(GridPosition);

	if (!Cell)
	{
		UE_LOG(LogTemp, Error, TEXT("Nessuna cella trovata alla posizione dell'unità! (GetCurrentCell in unit) "));
	}

	return Cell;
}

FString AUnit::GetIDCellPosition() const
{
	AGridCell* Cell = GetCurrentCell();
	return Cell->GetID(GridPosition);
};

void AUnit::OnUnitClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	AGameManager* GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

	if (GameManager && GameManager->GetCurrentPlayerTurn() == 0)
	{
		GameManager->SetSelectedUnit(this);
		UE_LOG(LogTemp, Log, TEXT("Unità selezionata in posizione (%d, %d)"), GridPosition.X, GridPosition.Y);
	}
}

void AUnit::MoveToCell(const FIntPoint& TargetPosition)
{
	AGrid* Grid = nullptr;
	for (TActorIterator<AGrid> It(GetWorld()); It; ++It)
	{
		Grid = *It;
		break; 
	}

	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT("Griglia non trovata!"));
		return;
	}
	
	AGridCell* CurrentCell = Grid->GetCellAtPosition(GridPosition);
	if (CurrentCell)
	{
		CurrentCell->bIsOccupied = false; 
		CurrentCell->SetAsObstacle(false);
		CurrentCell->SetUnitMaterial(false);
	}

	AGridCell* TargetCell = Grid->GetCellAtPosition(TargetPosition);
	AGameManager* GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));

	if (TargetCell && !TargetCell->IsObstacle && !TargetCell->bIsOccupied)
	{
		GridPosition = TargetPosition;
		SetActorLocation(TargetCell->GetActorLocation());

		TargetCell->bIsOccupied = true;

		bool bIsPlayerUnit = GameManager && GameManager->GetPlayerUnits().Contains(this);

        TargetCell->SetUnitMaterial(bIsPlayerUnit);
		UE_LOG(LogTemp, Log, TEXT("Unità spostata in posizione: (%d, %d)"), GridPosition.X, GridPosition.Y);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Non puoi muoverti su questa cella!"));
	}
}

void AUnit::Attack(AUnit* TargetUnit)
{
	
	if (!TargetUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("Attack fallito: TargetUnit è nullptr!"));
		return;
	}
	
	int32 DistanceX = FMath::Abs(TargetUnit->GridPosition.X - GridPosition.X);
	int32 DistanceY = FMath::Abs(TargetUnit->GridPosition.Y - GridPosition.Y);

	UE_LOG(LogTemp, Warning, TEXT("ATTACK() CHIAMATA - Target: %s"), *TargetUnit->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Distanza: X = %d, Y = %d, Tot = %d | Range = %d"),
		DistanceX, DistanceY, DistanceX + DistanceY, AttackRange);

	if (DistanceX + DistanceY > AttackRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("Il bersaglio è fuori dal range di attacco."));
		return;
	}

	AGameManager* GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
	if (DistanceX + DistanceY <= AttackRange)
	{
		int32 Damage = FMath::RandRange(1, 6);
		TargetUnit->Health -= Damage;

		if (GameManager)
		{
			FString AttackMessage = FString::Printf(TEXT("%s attacked %s by dealing %d damage! [%s HP: %d]"), 
				*GetName(), *TargetUnit->GetName(), Damage, *TargetUnit->GetName(), TargetUnit->Health);

			GameManager->UpdateUIText(AttackMessage);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("%s ha attaccato %s infliggendo %d danni!"), *GetName(), *TargetUnit->GetName(), Damage);
		UE_LOG(LogTemp, Log, TEXT("%s HP rimanenti: %d"), *TargetUnit->GetName(), TargetUnit->Health);

		if (TargetUnit->Health <= 0)
		{
			FString DeathMessage = FString::Printf(TEXT("%s is dead!"), *TargetUnit->GetName());
			GameManager->UpdateUIText(DeathMessage);
			UE_LOG(LogTemp, Warning, TEXT("%s è stato eliminato!"), *TargetUnit->GetName());

			GameManager->RemoveUnit(TargetUnit);
			TargetUnit->Destroy();
			GameManager->CheckWinCondition();
			
		}

		bHasAttacked = true;
	}

	if (this->IsA(ASniper::StaticClass()))
	{
		bool bShouldCounterattack = false;

		if (TargetUnit->IsA(ASniper::StaticClass()))
		{
			bShouldCounterattack = true;
		}

		else if (TargetUnit->IsA(ABrawler::StaticClass()) && (DistanceX + DistanceY == 1))
		{
			bShouldCounterattack = true;
		}

		if (bShouldCounterattack)
		{
			int32 CounterDamage = FMath::RandRange(1, 3);
			this->Health -= CounterDamage;

			FString CounterMsg = FString::Printf(TEXT("Counterattack from %s! %s suffers %d damage. [%s HP: %d]"),
				*TargetUnit->GetName(), *this->GetName(), CounterDamage, *this->GetName(), this->Health);

			GameManager->UpdateUIText(CounterMsg);
			UE_LOG(LogTemp, Warning, TEXT("%s ha subito %d danni da contrattacco!"), *this->GetName(), CounterDamage);
			UE_LOG(LogTemp, Log, TEXT("%s HP rimanenti dopo contrattacco: %d"), *this->GetName(), this->Health);

			if (this->Health <= 0)
			{
				FString SelfDeath = FString::Printf(TEXT("%s died of the counterattack!"), *this->GetName());
				GameManager->UpdateUIText(SelfDeath);
				UE_LOG(LogTemp, Warning, TEXT("%s è stato eliminato a causa del contrattacco!"), *this->GetName());

				GameManager->RemoveUnit(this);
				this->Destroy();
				GameManager->CheckWinCondition();
				return; 
			}
		}
	}
}

void AUnit::ResetTurnState()
{
	bHasMoved = false;
	bHasAttacked = false;
}
