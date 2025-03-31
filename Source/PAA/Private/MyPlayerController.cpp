// MyPlayerController.cpp

#include "PAA/Public/MyPlayerController.h"

#include "TurnBasedGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "PAA/Public/GridCell.h"
#include "PAA/Public/GameUIManager.h"
#include "PAA/Public/GameManager.h"
#include "UObject/ConstructorHelpers.h"

AMyPlayerController::AMyPlayerController()
{
	bShowMouseCursor = true;
	bEnableMouseOverEvents = true;
	bEnableClickEvents = true;

}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	ATurnBasedGameMode* GameMode = Cast<ATurnBasedGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: GameMode non trovato!"));
		return;
	}

	GameUIManager = GameMode->GetUIManager();
	if (!GameUIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: GameUIManager non disponibile nel GameMode!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameUIManager collegato dal GameMode con successo!"));
	}
	
}
