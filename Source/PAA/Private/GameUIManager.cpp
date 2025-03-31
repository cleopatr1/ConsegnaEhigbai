// GameUIManager.cpp

#include "PAA/Public/GameUIManager.h"
#include "PAA/Public/GameManager.h"
#include "PAA/Public/GameUI.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

// Costruttore
UGameUIManager::UGameUIManager()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(TEXT("/Game/UI/WBP_GameUI.WBP_GameUI_C"));
	if (WidgetClassFinder.Succeeded())
	{
		GameUIClass = WidgetClassFinder.Class;
	}
}

void UGameUIManager::Initialize(UWorld* World, AGameManager* Manager)
{
	if (!World || !Manager)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameUIManager::Initialize - Mondo o GameManager non validi!"));
		return;
	}

	GameManager = Manager;
	
	if (GameUIClass)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (!PlayerController)
		{
			UE_LOG(LogTemp, Error, TEXT("UGameUIManager::Initialize - PlayerController non trovato!"));
			return;
		}
		
		GameWidget = CreateWidget<UGameUI>(PlayerController, GameUIClass);
		if (GameWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget creato con successo!"));
			GameWidget->AddToViewport();
			GameWidget->SetGameManager(GameManager);
			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Errore: Widget non è stato creato!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: GameUIClass non trovato!"));
	}
}

void UGameUIManager::UpdateTurnDisplay(int32 PlayerTurn)
{
	if (GameWidget)
	{
		GameWidget->UpdateTurnDisplay(PlayerTurn);
	}
}

void UGameUIManager::UpdateCustomText(FString PlayerTurn)
{
	if (GameWidget)
	{
		GameWidget->UpdateCustomText(PlayerTurn);
	}
}

void UGameUIManager::SetSelectedUnit(AUnit* Unit)
{
	if (GameWidget)
	{
		GameWidget->SetSelectedUnit(Unit);
	}
}

void UGameUIManager::OnMoveButtonClicked()
{
	if (GameManager && GameManager->GetSelectedUnit())
	{
		UE_LOG(LogTemp, Log, TEXT("Movimento selezionato per %s"), *GameManager->GetSelectedUnit()->GetName());
	}
}

void UGameUIManager::OnAttackButtonClicked()
{
	if (GameManager && GameManager->GetSelectedUnit())
	{
		UE_LOG(LogTemp, Log, TEXT("Attacco selezionato per %s"), *GameManager->GetSelectedUnit()->GetName());
	}
}

void UGameUIManager::ShowPlacementScreenOnUI()
{
	if (GameWidget)
	{
		GameWidget->ShowPlacementScreen();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameUI non collegato in GameUIManager"));
	}
}

void UGameUIManager::ShowGameplayScreenOnUI()
{
	if (GameWidget)
	{
		GameWidget->ShowGameplayScreen();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameUI non collegato in GameUIManager"));
	}
}

void UGameUIManager::ShowVictoryScreenOnUI()
{
	if (GameWidget)
	{
		GameWidget->ShowVictoryScreen();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameUI non collegato in GameUIManager"));
	}
}

void UGameUIManager::ShowLoseScreenOnUI()
{
	if (GameWidget)
	{
		GameWidget->ShowLoseScreen();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameUI non collegato in GameUIManager"));
	}
}

void UGameUIManager::ShowDrawScreenOnUI()
{
	if (GameWidget)
	{
		GameWidget->ShowDrawScreen();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameUI non collegato in GameUIManager"));
	}
}

void UGameUIManager::UpdateAllHealthBars()
{
	if (!GameWidget || !GameManager) return;

	AUnit* PlayerBrawler = GameManager->GetPlayerBrawler();
	AUnit* PlayerSniper  = GameManager->GetPlayerSniper();
	AUnit* AIBrawler     = GameManager->GetAIBrawler();
	AUnit* AISniper      = GameManager->GetAISniper();

	GameWidget->UpdateHealthBars(PlayerBrawler, PlayerSniper, AIBrawler, AISniper);
}

void UGameUIManager::LogMovement(AUnit* Unit, const FString& FromCell, const FString& ToCell)
{
	if (!GameWidget || !Unit) return;

	FString ControllerID = Unit->bIsPlayerControlled ? TEXT("HP:") : TEXT("AI:");
	FString TypeCode = Unit->IsA(ASniper::StaticClass()) ? TEXT("S") : TEXT("B");

	FString Entry = FString::Printf(TEXT("%s%s%s->%s"), *ControllerID, *TypeCode, *FromCell, *ToCell);
	GameWidget->AddMoveToHistory(Entry);
}

void UGameUIManager::LogAttack(AUnit* Attacker, AUnit* Target, int32 Damage)
{
	if (!GameWidget || !Attacker || !Target) return;

	FString ControllerID = Attacker->bIsPlayerControlled ? TEXT("HP:") : TEXT("AI:");
	FString TypeCode = Attacker->IsA(ASniper::StaticClass()) ? TEXT("S") : TEXT("B");

	FString Entry = FString::Printf(TEXT("%s%sG%s (-%d HP)"), *ControllerID, *TypeCode, *Target->IDCellPosition, Damage);
	GameWidget->AddMoveToHistory(Entry);
}
