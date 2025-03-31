// TurnBasedGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PAA/Public/GameUI.h"
#include "TurnBasedGameMode.generated.h"


class AGameManager;
class AMyPlayerController;
class UGameUIManager;

UCLASS()
class PAA_API ATurnBasedGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATurnBasedGameMode();

protected:
	virtual void BeginPlay() override;

public:
	UGameUIManager* GetUIManager() const { return UIManager; }

private:
	UPROPERTY()
	AGameManager* GameManager;

	UPROPERTY()
	UGameUIManager* UIManager;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameUI> GameUIClass;

	UPROPERTY()
	UGameUI* GameWidget;  
};
