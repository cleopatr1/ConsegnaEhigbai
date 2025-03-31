//Unit.h
#pragma once

#include "CoreMinimal.h"
#include "GridCell.h"
#include "GameFramework/Actor.h"
#include "Unit.generated.h"

class UStaticMeshComponent;
class AGridCell;

UCLASS()
class PAA_API AUnit : public AActor
{
	GENERATED_BODY()
	
public:	
	AUnit();

	UFUNCTION()
	void OnUnitClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	bool bIsSelected;
	bool bHasAttacked = false;
	bool bHasMoved = false;
	
	// Proprietà delle unità
	UPROPERTY(EditAnywhere, Category = "Unit")
	int32 Health;

	UPROPERTY(EditAnywhere, Category = "Unit")
	int32 MaxMovement;

	UPROPERTY(BlueprintReadOnly, Category = "Unit")
	bool bIsPlayerControlled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Unit")
	int32 MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Unit")
	int32 AttackRange;

	UPROPERTY(EditAnywhere, Category = "Unit")
	FIntPoint GridPosition;
	
	UPROPERTY(EditAnywhere, Category = "Unit")
	FString IDCellPosition;
	
	AGridCell* GetCurrentCell() const;
	
	FString GetIDCellPosition() const;
	
	UPROPERTY(VisibleAnywhere, Category = "Unit")
	UStaticMeshComponent* UnitMesh;
	
	void MoveToCell(const FIntPoint& TargetPosition);

	virtual void Attack(AUnit* TargetUnit);

	void ResetTurnState();
	
protected:
	virtual void BeginPlay() override;

};
