//  MyPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UGameUIManager;
class AGridCell;

UCLASS()
class PAA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UGameUIManager* GameUIManager;
};
