// Sniper.h

#pragma once

#include "CoreMinimal.h"
#include "Unit.h"
#include "Sniper.generated.h"

UCLASS()
class PAA_API ASniper : public AUnit
{
	GENERATED_BODY()

public:
	ASniper();

	void Attack(AUnit* TargetUnit) override;
};
