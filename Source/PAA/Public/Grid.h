//Grid.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Grid.generated.h"

class AGridCell;
class AUnit;

UCLASS()
class PAA_API AGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	AGrid();

	UPROPERTY(EditAnywhere, Category = "Grid")
	float CellSize = 100.f; 

	UPROPERTY(EditAnywhere, Category = "Grid")
	FIntPoint GridDimensions = FIntPoint(25, 25); 

	void GenerateGrid();

	void GenerateObstacles(float ObstaclePercentage);

	bool IsGridFullyConnected();

	bool PlaceUnit(AUnit* Unit, const FIntPoint& GridPosition);

	bool IsCellOccupied(FIntPoint Position, AUnit* MovingUnit = nullptr);

	void HighlightMovableCells(AUnit* Unit);

	bool IsValidGridPosition(FIntPoint Position) const;

	AGridCell* GetCellAtPosition(FIntPoint Position);

	void ClearHighlightedCells();

	bool IsHighlighted(FIntPoint Position) const;
	
	AUnit* GetUnitAtPosition(FIntPoint Position) const;

	//ALGORITMO BFS USANDO LE CELLE EDIVENZIATE
	TArray<FIntPoint> FindShortestPath(FIntPoint Start, FIntPoint End, AUnit* MovingUnit);

	FIntPoint GetRandomAvailableCell(AUnit* AIUnit);

protected:
	virtual void BeginPlay() override;
	

private:
	AUnit* SelectedUnit;

	UPROPERTY()
	TArray<AGridCell*> Cells;

	UPROPERTY()
	TArray<AGridCell*> HighlightedCells;

	TArray<TArray<AGridCell*>> GridArray;

	UPROPERTY()
	TArray<AUnit*> PlayerUnits;

	UPROPERTY()
	TArray<AUnit*> AIUnits;

	FVector GetCellWorldLocation(const FIntPoint& GridPosition) const;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* CameraComponent;

	void SetupCamera();
};
