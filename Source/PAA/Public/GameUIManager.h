// GameUIManager.h

#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "GameUIManager.generated.h"

class UWidget;
class UGameUI;
class AGameManager;

UCLASS(Blueprintable)
class PAA_API UGameUIManager : public UObject
{
	GENERATED_BODY()
	
public:
	UGameUIManager();

	void Initialize(UWorld* World, AGameManager* Manager);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetSelectedUnit(AUnit* Unit);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnMoveButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnAttackButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateTurnDisplay(int32 PlayerTurn);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateCustomText(FString PlayerTurn);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowVictoryScreenOnUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLoseScreenOnUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDrawScreenOnUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPlacementScreenOnUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameplayScreenOnUI();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateAllHealthBars();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void LogMovement(AUnit* Unit, const FString& FromCell, const FString& ToCell);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void LogAttack(AUnit* Attacker, AUnit* Target, int32 Damage);

	UFUNCTION(BlueprintCallable, Category = "UI")
	UGameUI* GetGameUI() const { return GameWidget; }


private:
	UPROPERTY()
	TSubclassOf<UGameUI> GameUIClass;

	UPROPERTY()
	UGameUI* GameWidget;

	UPROPERTY()
	AGameManager* GameManager;
};
