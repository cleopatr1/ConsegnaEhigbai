//Gridd.cpp

#include "PAA/Public/Grid.h"
#include "PAA/Public/GridCell.h"
#include "PAA/Public/Unit.h"
#include "PAA/Public/GameManager.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"


// Sets default values
AGrid::AGrid()
{ 
	PrimaryActorTick.bCanEverTick = false;
	CellSize = 200.f;
	GridDimensions = FIntPoint(25, 25);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
}

void AGrid::BeginPlay()
{
	Super::BeginPlay();
	GenerateGrid();
	GenerateObstacles(0.4f); // 20% di ostacoli
	SetupCamera();
}

void AGrid::SetupCamera()
{
	FVector CameraLocation = GetActorLocation() + FVector(0.f, 0.f, 5000.f);
	FRotator CameraRotation = FRotator(-90.f, 0.f, -90.f);

	CameraComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->SetViewTarget(this);
	}

	FVector BackgroundLocation = GetActorLocation() + FVector(0.f, 0.f, -5010.f); // Leggermente sotto la griglia
	FRotator BackgroundRotation = FRotator::ZeroRotator;

	AStaticMeshActor* BackgroundPlane = GetWorld()->SpawnActor<AStaticMeshActor>(BackgroundLocation, BackgroundRotation);
	if (BackgroundPlane)
	{
		UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
		if (PlaneMesh)
		{
			BackgroundPlane->GetStaticMeshComponent()->SetStaticMesh(PlaneMesh);

			UMaterialInterface* BackgroundMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Background.M_Background"));
			if (BackgroundMaterial)
			{
				BackgroundPlane->GetStaticMeshComponent()->SetMaterial(0, BackgroundMaterial);
			}

			BackgroundPlane->SetActorScale3D(FVector(200.f, 200.f, 1.f));
			BackgroundPlane->SetActorEnableCollision(false);
		}
	}
}

void AGrid::GenerateGrid()
{
	if (!GetWorld()) return;
	
	GridArray.Empty();
	Cells.Empty();
    
	for(int32 y = 0; y < GridDimensions.Y; y++)
	{
		TArray<AGridCell*> Row;
		for(int32 x = 0; x < GridDimensions.X; x++)
		{
			FVector SpawnLocation = GetCellWorldLocation(FIntPoint(x, y));

			AGridCell* NewCell = GetWorld()->SpawnActor<AGridCell>(AGridCell::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
			NewCell->GridHeight = GridDimensions.Y;
			NewCell->SetActorScale3D(FVector(CellSize / 100.0f));
			
			NewCell->GridPosition = FIntPoint(x, y);

			Cells.Add(NewCell);
			Row.Add(NewCell);
		}
		GridArray.Add(Row);
	}
}

void AGrid::GenerateObstacles(float ObstaclePercentage)
{
	if (!GetWorld()) return;

	const int32 TotalCells = GridDimensions.X * GridDimensions.Y;
	const int32 ObstacleTarget = FMath::RoundToInt(TotalCells * ObstaclePercentage);
	const int32 MaxAttempts = ObstacleTarget * 2;

	TArray<FIntPoint> AvailableCells;
	for (int32 y = 0; y < GridDimensions.Y; y++)
	{
		for (int32 x = 0; x < GridDimensions.X; x++)
		{
			AvailableCells.Add(FIntPoint(x, y));
		}
	}

	// Mescola le celle
	for (int32 i = AvailableCells.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		AvailableCells.Swap(i, j);
	}

	int32 PlacedObstacles = 0;
	int32 Attempts = 0;

	while (PlacedObstacles < ObstacleTarget && Attempts < MaxAttempts && AvailableCells.Num() > 0)
	{
		FIntPoint CellPos = AvailableCells.Pop(); // Togliamo una cella dalla lista
		AGridCell* Cell = GetCellAtPosition(CellPos);

		if (Cell && !Cell->IsObstacle)
		{
			Cell->SetAsObstacle(true);

			if (IsGridFullyConnected())
			{
				PlacedObstacles++;
			}
			else
			{
				Cell->SetAsObstacle(false);
			}
			Attempts++;
		}
	}
}


bool AGrid::IsGridFullyConnected()
{
	TArray<FIntPoint> FreeCells;

	for (int32 y = 0; y < GridDimensions.Y; y++)
	{
		for (int32 x = 0; x < GridDimensions.X; x++)
		{
			AGridCell* Cell = GetCellAtPosition(FIntPoint(x, y));
			if (Cell && !Cell->IsObstacle)
			{
				FreeCells.Add(FIntPoint(x, y));
			}
		}
	}

	if (FreeCells.Num() == 0) return false;

	// Inizializza la BFS
	TSet<FIntPoint> Visited;
	TQueue<FIntPoint> Queue;
	Queue.Enqueue(FreeCells[0]);
	Visited.Add(FreeCells[0]);

	// Direzioni per spostarci
	const TArray<FIntPoint> Directions = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };

	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		for (const FIntPoint& Dir : Directions)
		{
			FIntPoint Neighbor = Current + Dir;
			if (Visited.Contains(Neighbor)) continue;

			AGridCell* NeighborCell = GetCellAtPosition(Neighbor);
			if (NeighborCell && !NeighborCell->IsObstacle)
			{
				Queue.Enqueue(Neighbor);
				Visited.Add(Neighbor);
			}
		}
	}

	return Visited.Num() == FreeCells.Num();
}

FVector AGrid::GetCellWorldLocation(const FIntPoint& GridPosition) const
{
	const float Spacing = 10.f;

	const float SpacedCellSize = CellSize + Spacing;
	
	const float OffsetX = (GridDimensions.X * 	SpacedCellSize) / 2;
	const float OffsetY = (GridDimensions.Y * SpacedCellSize) / 2;
	
	// Converte la posizione della griglia in coordinate mondiali
	return GetActorLocation() + FVector(
	(GridPosition.X * SpacedCellSize) - OffsetX + (SpacedCellSize / 2), 
	(GridPosition.Y * SpacedCellSize) - OffsetY + (SpacedCellSize / 2), 
	50.0f
	);
}

bool AGrid::PlaceUnit(AUnit* Unit, const FIntPoint& GridPosition)
{
	if (!Unit || !IsValidGridPosition(GridPosition))
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: Posizione (%d, %d) non valida per l'unità!"), GridPosition.X, GridPosition.Y);
		return false;
	}

	AGridCell* TargetCell = GridArray[GridPosition.Y][GridPosition.X];
	if (!TargetCell || TargetCell->IsObstacle || TargetCell->bIsOccupied) {
		UE_LOG(LogTemp, Error, TEXT("Errore: La cella (%d, %d) è un ostacolo o non esiste!"), GridPosition.X, GridPosition.Y);
		return false;
	}

	// Posizioniamo l'unità
	FVector WorldLocation = GetCellWorldLocation(GridPosition) + FVector(0, 0, -5000);
	Unit->SetActorLocation(WorldLocation);
	Unit->GridPosition = GridPosition;
	TargetCell->bIsOccupied = true;

	// Cambia il materiale in base al tipo di unità
	AGameManager* GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
	bool bIsPlayerUnit = false;
	if (GameManager)
	{
		if (GameManager->IsPlayerTurnToPlace())
		{
			GameManager->AddPlayerUnit(Unit);
			bIsPlayerUnit = true;
		}
		else
		{
			GameManager->AddAIUnit(Unit);
		}
		
	}
	
	TargetCell->SetUnitMaterial(bIsPlayerUnit);
	
	return true;
}

void AGrid::HighlightMovableCells(AUnit* Unit)
{
	if (!Unit) return;

	ClearHighlightedCells();

	// BFS per trovare le celle raggiungibili
	TQueue<FIntPoint> Queue;
	TMap<FIntPoint, int32> Visited;

	Queue.Enqueue(Unit->GridPosition);
	Visited.Add(Unit->GridPosition, 0);

	while (!Queue.IsEmpty())
	{
		FIntPoint CurrentPosition;
		Queue.Dequeue(CurrentPosition);

		int32 CurrentDistance = *Visited.Find(CurrentPosition);

		if (CurrentDistance >= Unit->MaxMovement)
			continue;

		TArray<FIntPoint> Directions = { 
			FIntPoint(0, 1), FIntPoint(0, -1), 
			FIntPoint(1, 0), FIntPoint(-1, 0) 
		};

		for (FIntPoint Dir : Directions)
		{
			FIntPoint NewPosition = CurrentPosition + Dir;

			if (!Visited.Contains(NewPosition) && IsValidGridPosition(NewPosition) && !IsCellOccupied(NewPosition, Unit))
			{
				AGridCell* NextCell = GetCellAtPosition(NewPosition);

				if (NextCell && NextCell->bIsOccupied)
				{
					continue; 
				}

				// Verifica se la cella è libera (senza ostacoli o unità)
				if (NextCell && !NextCell->IsObstacle && !NextCell->bIsOccupied)
				{
					Queue.Enqueue(NewPosition);
					Visited.Add(NewPosition, CurrentDistance + 1);

					if (NewPosition != Unit->GridPosition)
						HighlightedCells.Add(NextCell);
						NextCell->HighlightCellForMovement(true);
				}
			}
		}
	}
}

bool AGrid::IsValidGridPosition(FIntPoint Position) const
{
	return (Position.X >= 0 && Position.X < GridDimensions.X &&
			Position.Y >= 0 && Position.Y < GridDimensions.Y);
}

bool AGrid::IsCellOccupied(FIntPoint Position, AUnit* MovingUnit)
{
	if (!IsValidGridPosition(Position))
	{
		return true;
	}
	
	AGridCell* Cell = GetCellAtPosition(Position);
	if (!Cell) return true;

	if (Cell->bIsOccupied)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cella (%d, %d) è occupata."), Position.X, Position.Y);
		return true;
	}
	
	for (AUnit* Unit : PlayerUnits)
	{
		if (Unit && Unit != MovingUnit && Unit->GridPosition == Position)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cella (%d, %d) è occupata da un'unità %s"), Position.X, Position.Y, *Unit->GetName());
			return true;
		}
	}

	for (AUnit* Unit : AIUnits)
	{
		if (Unit && Unit != MovingUnit && Unit->GridPosition == Position)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cella (%d, %d) è occupata da un'unità IA %s"), Position.X, Position.Y, *Unit->GetName());
			return true;
		}
	}

	return false;
}

AGridCell* AGrid::GetCellAtPosition(FIntPoint Position)
{
	for (AGridCell* Cell : Cells)
	{
		if (Cell && Cell->GridPosition == Position) return Cell;
	}
	return nullptr;
}

void AGrid::ClearHighlightedCells()
{
	for (AGridCell* Cell :	Cells)
	{
			if (Cell && !Cell->bIsOccupied) // Se la cella NON è occupata, resettiamo il colore
			{
				Cell->HighlightCellForMovement(false);
			}
	}
	UE_LOG(LogTemp, Warning, TEXT("Highlight delle celle rimosso."));
	HighlightedCells.Empty();
}


bool AGrid::IsHighlighted(FIntPoint Position) const
{
	for (AGridCell* Cell : HighlightedCells) 
	{
		if (Cell->GridPosition == Position)
		{
			return true;
		}
	}
	return false;
}

AUnit* AGrid::GetUnitAtPosition(FIntPoint Position) const
{
	for (AUnit* Unit : PlayerUnits) 
	{
		if (Unit && Unit->GridPosition == Position)
		{
			return Unit;
		}
	}

	for (AUnit* Unit : AIUnits) 
	{
		if (Unit && Unit->GridPosition == Position)
		{
			return Unit;
		}
	}

	return nullptr;
}

TArray<FIntPoint> AGrid::FindShortestPath(FIntPoint Start, FIntPoint End, AUnit* MovingUnit)
{
	TMap<FIntPoint, FIntPoint> CameFrom;
	TQueue<FIntPoint> Queue;
	TSet<FIntPoint> Visited;
	Queue.Enqueue(Start);
	Visited.Add(Start);

	TArray<FIntPoint> Directions = { 
		FIntPoint(0, 1), FIntPoint(0, -1), 
		FIntPoint(1, 0), FIntPoint(-1, 0) 
	};

	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		if (Current == End)
			break;

		for (FIntPoint Dir : Directions)
		{
			FIntPoint Next = Current + Dir;

			if (!Visited.Contains(Next) && IsValidGridPosition(Next) && 
				(!IsCellOccupied(Next, MovingUnit) || Next == End))
			{
				AGridCell* NextCell = GetCellAtPosition(Next);
                
				if (!NextCell || !NextCell->IsHighlighted()) 
					continue;
				
				Queue.Enqueue(Next);
				Visited.Add(Next);
				CameFrom.Add(Next, Current);
			}
		}
	}

	TArray<FIntPoint> Path;
	FIntPoint Current = End;

	while (CameFrom.Contains(Current))
	{
		Path.Insert(Current, 0);
		Current = CameFrom[Current];
	}

	if (Path.Num() > 0)
		Path.Insert(Start, 0);

	return Path;
}

FIntPoint AGrid::GetRandomAvailableCell(AUnit* AIUnit)
{
	if (!AIUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna unità specifica!"));
		return FIntPoint(-1, -1);
	}

	HighlightMovableCells(AIUnit);

	if (HighlightedCells.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna cella disponibile!"));
		return FIntPoint(-1, -1);
	}
	
	AGameManager* GameManager = Cast<AGameManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass()));
	if (GameManager)
	{
		PlayerUnits = GameManager->GetPlayerUnits();
	}

	TArray<int32> Scores;
	int32 TotalScore = 0;

	for (AGridCell* Cell : HighlightedCells)
	{
		int32 MinDist = 9999;
		for (AUnit* Player : PlayerUnits)
		{
			int32 Dist = FMath::Abs(Cell->GridPosition.X - Player->GridPosition.X) +
						 FMath::Abs(Cell->GridPosition.Y - Player->GridPosition.Y);
			MinDist = FMath::Min(MinDist, Dist);
		}
		
		int32 Score = FMath::Clamp(20 - MinDist, 1, 20);
		Scores.Add(Score);
		TotalScore += Score;
	}

	// Seleziona una cella in base al peso
	int32 RandomScore = FMath::RandRange(0, TotalScore);
	int32 Accumulated = 0;
	for (int32 i = 0; i < HighlightedCells.Num(); i++)
	{
		Accumulated += Scores[i];
		if (RandomScore <= Accumulated)
		{
			return HighlightedCells[i]->GridPosition;
		}
	}

	return HighlightedCells.Last()->GridPosition;
}

