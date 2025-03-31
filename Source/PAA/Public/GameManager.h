// GameManager.h
#pragma once

#include "CoreMinimal.h"
#include "Brawler.h"
#include "Grid.h"
#include "Unit.h"
#include "GameUI.h"
#include "Sniper.h"
#include "GameFramework/Actor.h"
#include "GameManager.generated.h"

class AGrid;
class AUnit;

UCLASS()
class PAA_API AGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AGameManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void StartGame();
	void EndTurn();
	void CheckWinCondition();
	void FlippingCoin();
	void CheckIfPlacementComplete();

	// WIDGET THINGS
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ActionsButton(bool on);
	void UpdateUIText(FString NewText);
	void UpdateTurnDisplay();
	

	// POSIZIONAMENTO DELLE UNITA' DELL'AI
	UFUNCTION()
	void AIPlaceUnit(FString UnitType);
	void AITurnPlacement();

	// MOVIMENTO DEL GIOCATORE
	void EnablePlayerMovement();

	// MOVIMENTO DELL'AI
	void MoveAIUnits();
	bool TryAttackInRange(AUnit* AttackingUnit);
	bool IsEnemyInRange(AUnit* Attacker);

	void RemoveUnit(AUnit* Unit);
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddPlayerUnit(AUnit* Unit) { if (!PlayerUnits.Contains(Unit)) PlayerUnits.Add(Unit); }
	void AddAIUnit(AUnit* Unit) { if (!AIUnits.Contains(Unit)) AIUnits.Add(Unit); };
	void AddTotalUnits(AUnit* Unit) { if (!AllUnits.Contains(Unit)) AllUnits.Add(Unit); };
	
	bool IsPlayerTurnToPlace() const{ return bPlayerTurnToPlace; };
	int32 GetStartingPlayer() const { return StartingPlayer; }

	//TURNO
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetCurrentPlayerTurn() const { return CurrentPlayerTurn; }

	// GETTER la lista delle unità del giocatore
	UFUNCTION(BlueprintCallable, Category = "Game")
	const TArray<AUnit*>& GetPlayerUnits() const { return PlayerUnits; }

	// GETTER il tipo di l'unità da piazzare 
	UFUNCTION(BlueprintCallable, Category = "Game")
	FString GetPendingUnitType() const { return PendingUnitType; }

	// GETTER del Grid
	UFUNCTION(BlueprintCallable, Category = "Grid")
	AGrid* GetGrid() const { return Grid; }

	// GETTER delle diverse classi di unità
	UFUNCTION(BlueprintCallable, Category = "Game")
	TSubclassOf<AUnit> GetBrawlerClass() const { return BrawlerClass; }
	UFUNCTION(BlueprintCallable, Category = "Game")
	TSubclassOf<AUnit> GetSniperClass() const { return SniperClass; }
	
	// Imposta l'unità in attesa di posizionamento
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetPendingUnitType(FString UnitType) { PendingUnitType = UnitType; };

	// Booleana per dire se l'unità è stata posizionata
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool CanPlaceUnit(FString UnitType) const;

	// SETTER per aggiornare lo stato delle unità posizionate
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetHasPlacedBrawler(bool bPlaced) { bHasPlacedBrawler = bPlaced; }
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetHasPlacedSniper(bool bPlaced) { bHasPlacedSniper = bPlaced; }

	// Setter e Getter per unità in generale
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetSelectedUnit(AUnit* Unit);
	UFUNCTION(BlueprintCallable, Category = "Game")
	AUnit* GetSelectedUnit() const { return SelectedUnit; }

	// Metodo per verificare se è attiva la modalità di movimento
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool IsMovingUnit() const { return bIsMovingUnit; }

	// Metodo per attivare/disattivare la modalità di movimento
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ToggleMovementMode();
	void ToggleMovementMode(bool able);

	// ho create questa invece di fare due funzioni come questi sopra
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ToggleAttackMode(bool able);
	void ToggleAttackMode();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void AttackSelectedUnit();

	UFUNCTION(BlueprintCallable, Category = "Game")
	bool IsAttacking() const { return bIsAttacking; }
	
	// getter/setter del Target per l'attacco
	void SetTargetUnit(AUnit* Unit);
	void ClearTargetUnit() { TargetUnit = nullptr; }

	void SetPendingMove(const TArray<FIntPoint>& Path) { PendingMovePath = Path; }
	bool HasPendingMove() const { return PendingMovePath.Num() > 0; }
	void ConfirmMove();
	void IncrementPlayerMoves();
	int32 CountPlayerUnitsAlive();

	AUnit* GetPlayerBrawler() const { return PlayerBrawler; }
	AUnit* GetPlayerSniper() const { return PlayerSniper; }
	AUnit* GetAIBrawler() const { return AIBrawler; }
	AUnit* GetAISniper() const { return AISniper; }
	
	void SetPlayerBrawler(AUnit* Brawler) { PlayerBrawler = Brawler; };
	void SetPlayerSniper(AUnit* Sniper) { PlayerSniper = Sniper; };

private:
	FString PendingUnitType;

	TArray<FString> AIPendingUnits;

	FString PendingAIUnitType;

	TArray<FIntPoint> PendingMovePath;
	
	UPROPERTY()
	TArray<AUnit*> PlayerUnits;

	UPROPERTY()
	TArray<AUnit*> AIUnits;

	UPROPERTY()
	TArray<AUnit*> AllUnits;

	AUnit* PlayerBrawler;
	AUnit* PlayerSniper;
	AUnit* AIBrawler;
	AUnit* AISniper;

	UPROPERTY()
	AUnit* TargetUnit = nullptr;
	
	int32 CurrentPlayerTurn;
	int32 StartingPlayer;
	bool bPlayerTurnToPlace;

	FString Message;
	
	bool IsGameOver;

	UPROPERTY()
	AGrid* Grid;

	UPROPERTY()
	class UGameUIManager* GameUIManager;

	UPROPERTY(EditDefaultsOnly, Category = "Units")
	TSubclassOf<AUnit> BrawlerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Units")
	TSubclassOf<AUnit> SniperClass;

	
	bool bHasPlacedBrawler = false;
	bool bHasPlacedSniper = false;

	bool bAIHasPlacedBrawler = false;
	bool bAIHasPlacedSniper = false;

	AUnit* SelectedUnit = nullptr;

	int32 PlayerMovesThisTurn = 0;
	int32 NumPlayerUnitsThisTurn = 0;

	bool bIsAttacking = false;
	bool bIsMovingUnit = false;

};
