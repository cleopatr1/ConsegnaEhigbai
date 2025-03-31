// GameUI.h

#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetSwitcher.h"
#include "GameUI.generated.h"

class AGameManager;
class AUnit;
class UButton;
class UTextBlock;
class UScrollBox;
class UWidgetSwitcher;

UCLASS()
class PAA_API UGameUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UGameUI(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	

public:
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetGameManager(AGameManager* Manager);
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetSelectedUnit(AUnit* Unit);

	// GESTIONE DEI BOTTONI
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OnFlipCoinClicked();
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnSpawnBrawlerClicked();
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnSpawnSniperClicked();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnTryAgainClicked();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnPlayClicked();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnRulesClicked();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnMenuClicked();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnMoveButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnAttackButtonClicked();
	

	UFUNCTION(BlueprintCallable, Category = "Game")
	void UpdateTurnDisplay(int32 PlayerTurn);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddMoveToHistory(const FString& Entry);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateCustomText(FString NewText);

	void ActionButtons(bool enabled);
	
	UFUNCTION(BlueprintCallable, Category = "Game")
	void UpdateHealthBars(AUnit* PlayerBrawler, AUnit* PlayerSniper, AUnit* AIBrawler, AUnit* AISniper);

	// SHOW DEGLI SCREEN

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenuScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowRulesScreen();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowFlippingCoinScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPlacementScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameplayScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowVictoryScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowLoseScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDrawScreen();


private:
	// GAMEMANAGER
	UPROPERTY()
	AGameManager* GameManager;

	// TESTI

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TurnTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LateralTextBlock;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* MoveHistoryBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoinResultText;
	
	// BOTTONI

	UPROPERTY(meta = (BindWidget))
	UButton* PlayButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RulesButton;

	UPROPERTY(meta = (BindWidget))
	UButton* MenuButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* ArrowButtonLeft0;

	UPROPERTY(meta = (BindWidget))
	UButton* SkipButton0;

	UPROPERTY(meta = (BindWidget))
	UButton* SkipButton1;

	UPROPERTY(meta = (BindWidget))
	UButton* ArrowButtonRight2;
	
	UPROPERTY(meta = (BindWidget))
	UButton* SB;

	UPROPERTY(meta = (BindWidget))
	UButton* BB;

	UPROPERTY(meta = (BindWidget))
	UButton* MoveButton;
	
	UPROPERTY(meta = (BindWidget))
	UButton* AttackButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FlipCoinButton;

	UPROPERTY(meta = (BindWidget))
	UButton* TryAgainButton;

	UPROPERTY(meta = (BindWidget))
	UButton* TryAgainDrawButton;

	// WIDGET SWICHER
	
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta = (BindWidget))
	UWidget* MainMenuScreen;

	UPROPERTY(meta = (BindWidget))
	UWidget* RulesScreen;

	UPROPERTY(meta = (BindWidget))
	UWidget* CoinFlipScreen;

	UPROPERTY(meta = (BindWidget))
	UWidget* PlacementScreen;

	UPROPERTY(meta = (BindWidget))
	UWidget* GameplayScreen;

	UPROPERTY(meta = (BindWidget))
	UWidget* VittoryScreen;

	UPROPERTY(meta = (BindWidget))
	UWidget* LoseScreen;

	UPROPERTY(meta = (BindWidget))
	UWidget* DrawScreen;

	// BARRE DI VITA

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PlayerBrawlerHealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PlayerSniperHealthBar; 

	UPROPERTY(meta = (BindWidget))
	UProgressBar* AIBrawlerHealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* AISniperHealthBar; 
	
};
