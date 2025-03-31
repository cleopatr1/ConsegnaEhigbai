// Brawler.h
#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "Brawler.generated.h"


UCLASS()
class PAA_API ABrawler : public AUnit
{
	GENERATED_BODY()

public:
	ABrawler();

	void Attack(AUnit* TargetUnit) override;
};
