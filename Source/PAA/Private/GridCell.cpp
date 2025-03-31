//GridCell.cpp

#include "PAA/Public/GridCell.h"
#include "PAA/Public/GameManager.h"
#include "PAA/Public/Grid.h"
#include "PAA/Public/Unit.h"
#include "Kismet/GameplayStatics.h"
#include "Delegates/Delegate.h"

// Costruttore
AGridCell::AGridCell()
{
	PrimaryActorTick.bCanEverTick = false;

	CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
	RootComponent = CellMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Plane"));
	if (MeshAsset.Succeeded())
	{
		CellMesh->SetStaticMesh(MeshAsset.Object);
		CellMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

	}

	CellMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CellMesh->SetGenerateOverlapEvents(true);
	CellMesh->SetCollisionResponseToAllChannels(ECR_Block);
	CellMesh->OnClicked.AddDynamic(this, &AGridCell::OnCellClicked);
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMat(TEXT("/Game/Materials/GRIDCELL"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Obstacle1Mat(TEXT("/Game/Materials/OBSTACLE1"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Obstacle2Mat(TEXT("/Game/Materials/OBSTACLE2"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HighlightMat(TEXT("/Game/Materials/M_GridHighlight"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMat(TEXT("/Game/Materials/M_PlayerUnitMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AIMat(TEXT("/Game/Materials/M_AIUnitMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> AISelectedMat(TEXT("/Game/Materials/M_AIUnitSelectedMaterial"));

	if (DefaultMat.Succeeded() && Obstacle1Mat.Succeeded() && Obstacle2Mat.Succeeded() && (HighlightMat.Succeeded()) && PlayerMat.Succeeded() && AIMat.Succeeded() && AISelectedMat.Succeeded())
	{
		DefaultMaterial = DefaultMat.Object;
		HighlightMaterial = HighlightMat.Object;
		PlayerUnitMaterial = PlayerMat.Object;
		AIUnitMaterial = AIMat.Object;
		AIUnitSelectedMaterial = AISelectedMat.Object;
		ObstacleMaterials.Add(Obstacle1Mat.Object);
		ObstacleMaterials.Add(Obstacle2Mat.Object);
	}

	CellMesh->SetMaterial(0, DefaultMaterial);
}

void AGridCell::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGridCell::SetAsObstacle(bool bIsObstacle)
{
	IsObstacle = bIsObstacle;

	if (bIsObstacle && ObstacleMaterials.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ObstacleMaterials.Num() - 1);
		CellMesh->SetMaterial(0, ObstacleMaterials[RandomIndex]);
	}
	else
	{
		CellMesh->SetMaterial(0, DefaultMaterial);
	}
}

FString AGridCell::GetID(FIntPoint Position) const
{
	TCHAR Letter = 'A' + Position.X;

	int32 Row = GridHeight - Position.Y;

	return FString::Printf(TEXT("%c%d"), Letter, Row);
}

void AGridCell::OnCellClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    AGameManager* GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
    if (!GameManager || IsObstacle) return;

    AGrid* Grid = GameManager->GetGrid();
    if (!Grid) return;

	FString Message;
	AUnit* UnitAtCell = Grid->GetUnitAtPosition(GridPosition);
    AUnit* SelectedUnit = GameManager->GetSelectedUnit();

    // 🎯 **MODALITÀ ATTACCO**
	if (GameManager->IsAttacking())
	{
		if (!SelectedUnit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Devi prima selezionare una tua unità per attaccare!"));
			return;
		}

		if (SelectedUnit->bHasAttacked)
		{
			UE_LOG(LogTemp, Warning, TEXT("Questa unità ha già attaccato questo turno."));
			GameManager->ToggleAttackMode(false);
			return;
		}
		
		GameManager->SetSelectedUnit(UnitAtCell);
		return;
	}
	
    // 🌟 **MODALITÀ MOVIMENTO**
    if (GameManager->IsMovingUnit())
    {
		if (!SelectedUnit)
		{
			
			if (!GameManager->IsAttacking() && UnitAtCell)
			{
				GameManager->SetSelectedUnit(UnitAtCell);
				UE_LOG(LogHAL, Warning, TEXT("Unità selezionata per il movimento: %s"), *UnitAtCell->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Non puoi muovere unità nemiche o vuote!"));
				
			}
			return;
		}
    	
    	if (SelectedUnit->bHasMoved)
    	{
    		UE_LOG(LogTemp, Warning, TEXT("Questa unità ha già effettuato il movimento in questo turno!"));
    		return;
    	}

    
        if (Grid->IsHighlighted(GridPosition))
        {
            TArray<FIntPoint> ShortestPath = Grid->FindShortestPath(SelectedUnit->GridPosition, GridPosition, SelectedUnit);

            if (ShortestPath.Num() > 1)
            {
                Grid->ClearHighlightedCells();
                for (FIntPoint Pos : ShortestPath)
                {
                    AGridCell* Cell = Grid->GetCellAtPosition(Pos);
                    if (Cell) Cell->HighlightCellForMovement(true);
                }
            	
                GameManager->SetPendingMove(ShortestPath);
                UE_LOG(LogTemp, Warning, TEXT("Percorso evidenziato. Clicca di nuovo per muoverti."));
                return;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Devi selezionare una cella evidenziata per il movimento!"));
        }
    } 

    // Conferma il movimento
    if (GameManager->HasPendingMove())
    {
    	if (SelectedUnit->bHasMoved)
    	{
    		UE_LOG(LogTemp, Warning, TEXT("Questa unità ha già mosso, non può confermare di nuovo il movimento."));
    		return;
    	}
        GameManager->ConfirmMove();
    	
        return;
    }

	// ✅ POSIZIONAMENTO UNITÀ
	FString PendingType = GameManager->GetPendingUnitType();
	if (!GameManager->CanPlaceUnit(PendingType))
	{
		Message = FString::Printf(TEXT("%s is already positioned..."), *PendingType);
		GameManager->UpdateUIText(Message);
		UE_LOG(LogTemp, Warning, TEXT("Hai già posizionato un %s!"), *PendingType);
		return;
	}

	AUnit* NewUnit = nullptr;
	if (PendingType == "Brawler" && (GameManager->GetBrawlerClass()))
	{
		NewUnit = GetWorld()->SpawnActor<AUnit>(GameManager->GetBrawlerClass());
	}
	else if (PendingType == "Sniper" && GameManager->GetSniperClass())
	{
		NewUnit = GetWorld()->SpawnActor<AUnit>(GameManager->GetSniperClass());
	}
	else
	{
		Message = FString::Printf(TEXT("click a button %s..."), *PendingType);
		GameManager->UpdateUIText(Message);
		UE_LOG(LogTemp, Error, TEXT("Errore: Nessuna classe trovata!"));
		return;
	}

	if (!NewUnit)
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: SpawnActor ha fallito per %s!"), *PendingType);
		return;
	}

	if (Grid->PlaceUnit(NewUnit, GridPosition))
	{
		GameManager->AddPlayerUnit(NewUnit);
		GameManager->AddTotalUnits(NewUnit);
		NewUnit->bIsPlayerControlled = true;
		NewUnit->IDCellPosition = NewUnit->GetIDCellPosition();
		Message = FString::Printf(TEXT("Player has positioned %s in %s"), *PendingType, *NewUnit->IDCellPosition);
		GameManager->UpdateUIText(Message);
		UE_LOG(LogTemp, Warning, TEXT("%s posizionato in (%d, %d) = %s"), *PendingType, GridPosition.X, GridPosition.Y, *NewUnit->IDCellPosition);
		
		if (PendingType == "Brawler")
		{
			GameManager->SetHasPlacedBrawler(true);
			GameManager->SetPlayerBrawler(NewUnit);
			
		}
		else if (PendingType == "Sniper")
		{
			GameManager->SetHasPlacedSniper(true);
			GameManager->SetPlayerSniper(NewUnit);
		}
		
		GameManager->CheckIfPlacementComplete();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Errore nel posizionamento dell'unità!"));
	}

	GameManager->SetPendingUnitType("");
}

void AGridCell::HighlightCellForMovement(bool bHighlight)
{
	if (IsObstacle) return;

	bIsHighlighted = bHighlight;
	
	if (CellMesh && HighlightMaterial && DefaultMaterial)
	{
		CellMesh->SetMaterial(0, bHighlight ? HighlightMaterial : DefaultMaterial);
	}
}

void AGridCell::SetUnitMaterial(bool bIsPlayerUnit)
{
	if (bIsPlayerUnit)
	{
		CellMesh->SetMaterial(0, PlayerUnitMaterial);
	}
	else
	{
		CellMesh->SetMaterial(0, AIUnitMaterial);
	}
}

void AGridCell::SetAISelectedMaterial(bool bIsAIUnit)
{
	CellMesh->SetMaterial(0, bIsAIUnit ? AIUnitSelectedMaterial : AIUnitMaterial);

}
