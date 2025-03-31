//GridCell.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GridCell.generated.h"

class UStaticMeshComponent;
class AGameManager;

UCLASS()
class PAA_API AGridCell : public AActor
{
	GENERATED_BODY()
	
public:	
	AGridCell();

	UPROPERTY(EditAnywhere)
	TArray<UMaterialInterface*> ObstacleMaterials;

	UPROPERTY(VisibleAnywhere, Category = "Grid")
	UStaticMeshComponent* CellMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FIntPoint GridPosition;
	
	UPROPERTY(VisibleAnywhere, Category = "Grid")
	bool IsObstacle = false;

	UPROPERTY(VisibleAnywhere, Category = "Grid")
	bool bIsOccupied = false;

	UPROPERTY(VisibleAnywhere, Category = "Grid")
	int32 GridHeight;
	
	UFUNCTION()
	FString GetID(FIntPoint Position) const;

	// funzione di input del clicco
	UFUNCTION()
	void OnCellClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
	
	void SetAsObstacle(bool bIsObstacle);

	void SetUnitMaterial(bool bIsPlayerUnit);

	void SetAISelectedMaterial(bool bIsAIUnit);
	
	UFUNCTION()
	void HighlightCellForMovement(bool bHighlight);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsHighlighted() const { return bIsHighlighted; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* DefaultMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* ObstacleMaterial;

	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* HighlightMaterial;

	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* AIUnitSelectedMaterial;

	UPROPERTY(VisibleAnywhere, Category = "Grid")
	bool bIsHighlighted = false;

	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* PlayerUnitMaterial;

	UPROPERTY(EditAnywhere, Category = "Grid")
	UMaterialInterface* AIUnitMaterial;
};
