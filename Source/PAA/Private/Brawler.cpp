// Brawler.cpp

#include "PAA/Public/Brawler.h"

#include "Sniper.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

ABrawler::ABrawler()
{
	PrimaryActorTick.bCanEverTick = false;

	MaxHealth = 40;
	Health = 40;
	MaxMovement = 6;
	AttackRange = 1;

	if (UnitMesh)
	{
		UMaterialInterface* BrawlerMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/BRAWLER"));
		if (BrawlerMaterial)
		{
			UnitMesh->SetMaterial(0, BrawlerMaterial);
		}
	}
}

void ABrawler::Attack(AUnit* TargetUnit)
{
	Super::Attack(TargetUnit);
}

