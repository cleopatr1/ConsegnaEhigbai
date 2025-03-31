// GameUI.cpp

#include "PAA/Public/GameUI.h"

#include "PAA/Public/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"


// Costruttore
UGameUI::UGameUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {}

// Imposta il riferimento al GameManager
void UGameUI::SetGameManager(AGameManager* Manager)
{
	GameManager = Manager;
	
	if (!GameManager)
	{
		UE_LOG(LogTemp, Error, TEXT("GameManager non valido! (SetGameManager GameUI)"));
		return;
	}

}

void UGameUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (FlipCoinButton)
	{
		FlipCoinButton->OnClicked.AddDynamic(this, &UGameUI::OnFlipCoinClicked);
	}

	if (SB && BB)
	{
		SB->OnClicked.AddDynamic(this, &UGameUI::OnSpawnSniperClicked);
		BB->OnClicked.AddDynamic(this, &UGameUI::OnSpawnBrawlerClicked);
	}

	if (MoveButton && AttackButton)
	{
		MoveButton->OnClicked.AddDynamic(this, &UGameUI::OnMoveButtonClicked);
		AttackButton->OnClicked.AddDynamic(this, &UGameUI::OnAttackButtonClicked);
	}

	if (TryAgainButton && TryAgainDrawButton)
	{
		TryAgainButton->OnClicked.AddDynamic(this, &UGameUI::OnTryAgainClicked);
		TryAgainDrawButton->OnClicked.AddDynamic(this, &UGameUI::OnTryAgainClicked);
	}

	if (PlayButton && RulesButton && MenuButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &UGameUI::OnPlayClicked);
		RulesButton->OnClicked.AddDynamic(this, &UGameUI::OnRulesClicked);
		MenuButton->OnClicked.AddDynamic(this, &UGameUI::OnTryAgainClicked);
	}

	if (SkipButton0 && SkipButton1 && ArrowButtonRight2 && ArrowButtonLeft0)
	{
		SkipButton0->OnClicked.AddDynamic(this, &UGameUI::OnPlayClicked);
		SkipButton1->OnClicked.AddDynamic(this, &UGameUI::OnPlayClicked);
		ArrowButtonRight2->OnClicked.AddDynamic(this, &UGameUI::OnPlayClicked);
		ArrowButtonLeft0->OnClicked.AddDynamic(this, &UGameUI::OnMenuClicked);
	}
}

void UGameUI::OnPlayClicked()
{
	ShowFlippingCoinScreen();
}

void UGameUI::OnRulesClicked()
{
	ShowRulesScreen();
}

void UGameUI::OnMenuClicked()
{
	ShowMainMenuScreen();

}

void UGameUI::OnFlipCoinClicked()
{
	if (!GameManager) return;

	GameManager->FlippingCoin();
	if (FlipCoinButton) FlipCoinButton->SetVisibility(ESlateVisibility::Hidden);
	
	bool bPlayerStarts = GameManager->GetStartingPlayer() == 0;
	FString ResultText = bPlayerStarts ? TEXT("HEAD The Player starts!") : TEXT("TAIL AI starts!");

	if (CoinResultText)
		CoinResultText->SetText(FText::FromString(ResultText));

	FTimerHandle ShowPlacementHandle;
	GetWorld()->GetTimerManager().SetTimer(ShowPlacementHandle, FTimerDelegate::CreateLambda([this, bPlayerStarts]()
	{
		
		if (!bPlayerStarts)
        	{
        		GameManager->StartGame();
        		FTimerHandle AITimer;
        		GetWorld()->GetTimerManager().SetTimer(AITimer, GameManager, &AGameManager::AITurnPlacement, 2.f, false);
        	}
        	else GameManager->StartGame();
	}), 2.0f, false);

}

void UGameUI::OnSpawnBrawlerClicked()
{
	if (GameManager)
	{
		GameManager->SetPendingUnitType("Brawler");
		UE_LOG(LogTemp, Log, TEXT("Brawler selezionato per il posizionamento!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameManager non valido! (GameUI)"));
	}
}

void UGameUI::OnSpawnSniperClicked()
{
	if (GameManager)
	{
		GameManager->SetPendingUnitType("Sniper");
		UE_LOG(LogTemp, Log, TEXT("Sniper selezionato per il posizionamento!"));

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameManager non valido! (GameUI)"));
	}
}

void UGameUI::OnTryAgainClicked()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FName CurrentLevel = *UGameplayStatics::GetCurrentLevelName(World);
		UGameplayStatics::OpenLevel(World, CurrentLevel);
	}

	RemoveFromParent();

}

void UGameUI::OnMoveButtonClicked()
{
	if (!GameManager) return;

	AUnit* SelectedUnit = GameManager->GetSelectedUnit();

	if (!SelectedUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna unità selezionata per muoversi! Seleziona prima una tua unità."));
		return;
	}

	if (SelectedUnit->bHasMoved)
	{
		UE_LOG(LogTemp, Warning, TEXT("Questa unità ha già effettuato il movimento!"));
		return;
	}

	if (GameManager->IsAttacking()) GameManager->ToggleAttackMode(false);

	GameManager->ToggleMovementMode();

	if (GameManager->IsMovingUnit())
	{
		UE_LOG(LogTemp, Log, TEXT("Movimento attivato per %s. Seleziona una cella valida."), *SelectedUnit->GetName());

		if (GameManager->GetGrid())
		{
			GameManager->GetGrid()->HighlightMovableCells(SelectedUnit);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Movimento disattivato."));
		
		if (GameManager->GetGrid())
		{
			GameManager->GetGrid()->ClearHighlightedCells();
		}
	}
}

void UGameUI::OnAttackButtonClicked()
{
	if (!GameManager) return;

	AUnit* SelectedUnit = GameManager->GetSelectedUnit();

	if (!SelectedUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna unità selezionata per attaccare! Seleziona una tua unità prima di attivare la modalità attacco."));
		UpdateCustomText("Select an Unit before attacking.");
		return;
	}

	if (!SelectedUnit->bHasMoved && !GameManager->IsEnemyInRange(SelectedUnit))
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessun nemico nelle vicinanze."));
		UpdateCustomText("No enemies in the range.");
		GameManager->GetGrid()->ClearHighlightedCells();
		GameManager->SetSelectedUnit(nullptr);
		return;
	}
	
	if (SelectedUnit->bHasAttacked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Questa unità ha già attaccato!"));
		UpdateCustomText("This Unit already attached.");
		GameManager->GetGrid()->ClearHighlightedCells();
		GameManager->SetSelectedUnit(nullptr);
		return;
	}

	if (GameManager->IsMovingUnit()) GameManager->ToggleMovementMode(false);

	bool WasAttacking = GameManager->IsAttacking();

	GameManager->ToggleAttackMode();

	if (GameManager->IsAttacking())
	{
		GameManager->GetGrid()->ClearHighlightedCells();
		UE_LOG(LogTemp, Log, TEXT("Attacco attivato per %s. Seleziona un'unità nemica."), *SelectedUnit->GetName());
		UpdateCustomText("Select a target.");
		GameManager->ToggleAttackMode(true);
	}
	else
	{
		
		UE_LOG(LogTemp, Log, TEXT("Attacco disattivato (onattackbuttonclicked ugameui)."));
		GameManager->ToggleAttackMode(false);

		
		if (SelectedUnit->bHasMoved && WasAttacking)
		{
			UE_LOG(LogTemp, Warning, TEXT("Il giocatore ha deciso di non attaccare dopo il movimento."));
			GameManager->IncrementPlayerMoves();
			GameManager->SetSelectedUnit(nullptr);
		}
	}
}

void UGameUI::SetSelectedUnit(AUnit* Unit)
{
	if (Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unità selezionata nel widget: %s"), *Unit->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna unità selezionata nel widget."));
	}
}

void UGameUI::UpdateCustomText(FString NewText)
{
	if (LateralTextBlock)
	{
		LateralTextBlock->SetText(FText::FromString(NewText));
		UE_LOG(LogTemp, Warning, TEXT("UI aggiornata: %s"), *NewText);
	}
}

void UGameUI::UpdateTurnDisplay(int32 PlayerTurn)
{
	FString TurnText = (PlayerTurn == 0) ? TEXT("Turn: Player") : TEXT("Turn: AI");
	UE_LOG(LogTemp, Warning, TEXT("%s"), *TurnText);

	if (TurnTextBlock)
	{
		TurnTextBlock->SetText(FText::FromString(TurnText));
	}
}

void UGameUI::AddMoveToHistory(const FString& Entry)
{
	if (!MoveHistoryBox) return;

	UTextBlock* NewEntry = NewObject<UTextBlock>(this);
	if (NewEntry)
	{
		NewEntry->SetText(FText::FromString(Entry));
		NewEntry->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		MoveHistoryBox->AddChild(NewEntry);
	}
}

void UGameUI::ActionButtons(bool enabled)
{
		if (MoveButton)
			MoveButton->SetIsEnabled(enabled);
		if (AttackButton)
			AttackButton->SetIsEnabled(enabled);

	if (enabled) UE_LOG(LogTemp, Warning, TEXT("Pulsanti Abilitati (Turno Player)"));
	if (!enabled) UE_LOG(LogTemp, Warning, TEXT("Pulsanti Disabilitati (Turno AI)"));
		
}

void UGameUI::ShowFlippingCoinScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Hidden);
		LateralTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (MainSwitcher && CoinFlipScreen)
	{
		MainSwitcher->SetActiveWidget(CoinFlipScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Lancio Moneta!"));
	}
}

void UGameUI::ShowMainMenuScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Hidden);
		LateralTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (MainSwitcher && MainMenuScreen)
	{
		MainSwitcher->SetActiveWidget(MainMenuScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Menu!"));
	}
}

void UGameUI::ShowRulesScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Hidden);
		LateralTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (MainSwitcher && RulesScreen)
	{
		MainSwitcher->SetActiveWidget(RulesScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Menu!"));
	}
}

void UGameUI::ShowPlacementScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Visible);
		LateralTextBlock->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (MainSwitcher && PlacementScreen)
	{
		MainSwitcher->SetActiveWidget(PlacementScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Posizionamento Unità!"));
	}
}

void UGameUI::ShowGameplayScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Visible);
		LateralTextBlock->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (MainSwitcher && GameplayScreen)
	{
		MainSwitcher->SetActiveWidget(GameplayScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Gameplay!"));
	}
}

void UGameUI::ShowVictoryScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Hidden);
		LateralTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (MainSwitcher && VittoryScreen)
	{
		MainSwitcher->SetActiveWidget(VittoryScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Vittoria!"));
	}
}

void UGameUI::ShowLoseScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Hidden);
		LateralTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (MainSwitcher && LoseScreen)
	{
		MainSwitcher->SetActiveWidget(LoseScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Sconfitta!"));
	}
}

void UGameUI::ShowDrawScreen()
{
	if (TurnTextBlock && LateralTextBlock)
	{
		TurnTextBlock->SetVisibility(ESlateVisibility::Hidden);
		LateralTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (MainSwitcher && DrawScreen)
	{
		MainSwitcher->SetActiveWidget(DrawScreen);
		UE_LOG(LogTemp, Warning, TEXT("Passaggio alla schermata Sconfitta!"));
	}
}

void UGameUI::UpdateHealthBars(AUnit* PlayerBrawler, AUnit* PlayerSniper, AUnit* AIBrawler, AUnit* AISniper)
{
	auto UpdateBar = [](UProgressBar* Bar, AUnit* Unit)
	{
		if (Bar && Unit)
		{
			float Percent = (float)Unit->Health / (float)Unit->MaxHealth;
			Bar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		}
	};

	UpdateBar(PlayerBrawlerHealthBar, PlayerBrawler);
	UpdateBar(PlayerSniperHealthBar, PlayerSniper);
	UpdateBar(AIBrawlerHealthBar, AIBrawler);
	UpdateBar(AISniperHealthBar, AISniper);
}
