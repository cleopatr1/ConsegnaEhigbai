//Sniper.cpp

#include "PAA/Public/Sniper.h"
#include "PAA/Public/Brawler.h"
#include "PAA/Public/GameManager.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

ASniper::ASniper()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MaxHealth = 20;
	Health = 20;
	MaxMovement = 3;
	AttackRange = 10;

	if (UnitMesh)
	{
		UMaterialInterface* SniperMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/SNIPER"));
		if (SniperMaterial)
		{
			UnitMesh->SetMaterial(0, SniperMaterial);
		}
	}
}

void ASniper::Attack(AUnit* TargetUnit)
{
	Super::Attack(TargetUnit);
}

