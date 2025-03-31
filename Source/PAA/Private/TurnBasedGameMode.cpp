// TurnBasedGameMode.cpp

#include "PAA/Public/TurnBasedGameMode.h"
#include "PAA/Public/GameManager.h"
#include "PAA/Public/MyPlayerController.h"
#include "PAA/Public/GameUIManager.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"


ATurnBasedGameMode::ATurnBasedGameMode()
{
	PlayerControllerClass = AMyPlayerController::StaticClass();
	DefaultPawnClass = nullptr;
}

void ATurnBasedGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("TurnBasedGameMode avviato!"));

	TArray<AActor*> FoundManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameManager::StaticClass(), FoundManagers);

	if (FoundManagers.Num() > 0)
	{
		GameManager = Cast<AGameManager>(FoundManagers[0]);
		UE_LOG(LogTemp, Warning, TEXT("GameManager trovato!"));
	}

	UIManager = NewObject<UGameUIManager>(this);
	if (!GameManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: GameManager non è stato trovato!"));
		return;
	}
	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: UIManager non è stato creato correttamente!"));
		return;
	}

}
