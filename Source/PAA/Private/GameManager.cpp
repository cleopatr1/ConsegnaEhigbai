// GameManager.cpp

#include "PAA/Public/GameManager.h"
#include "GridCell.h"
#include "PAA/Public/GameUIManager.h"
#include "PAA/Public/Grid.h"
#include "PAA/Public/Unit.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGameManager::AGameManager()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentPlayerTurn = 0;
	IsGameOver = false;

	
	static ConstructorHelpers::FClassFinder<AUnit> BrawlerBP(TEXT("/Game/Blueprints/Units/BP_Brawler"));
	if (BrawlerBP.Succeeded())
	{
		BrawlerClass = BrawlerBP.Class;
		UE_LOG(LogTemp, Warning, TEXT("BP_Brawler caricato correttamente!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: BP_Brawler non trovato! Controlla il percorso in Unreal Engine."));
	}

	static ConstructorHelpers::FClassFinder<AUnit> SniperBP(TEXT("/Game/Blueprints/Units/BP_Sniper"));
	if (SniperBP.Succeeded())
	{
		SniperClass = SniperBP.Class;
		UE_LOG(LogTemp, Warning, TEXT("BP_Sniper caricato correttamente!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: BP_Sniper non trovato! Controlla il percorso in Unreal Engine."));
	}
}

void AGameManager::BeginPlay()
{
	Super::BeginPlay();

	GameUIManager = NewObject<UGameUIManager>();
	if (GameUIManager)
	{
		GameUIManager->Initialize(GetWorld(), this);
		UE_LOG(LogTemp, Warning, TEXT("GameUIManager inizializzato con successo!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Errore: GameUIManager non è stato creato!"));
	}
}

void AGameManager::FlippingCoin()
{
	StartingPlayer = FMath::RandRange(0, 1);
	bPlayerTurnToPlace = (StartingPlayer == 0);
	
	AIPendingUnits = { "Brawler", "Sniper" };

	if (StartingPlayer == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔴 Il giocatore inizia a posizionare le unità!"));
		Message = FString::Printf(TEXT("Player starts the Game!"));
		UpdateUIText(Message);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("🔵 L'IA inizia a posizionare le unità!"));
		Message = FString::Printf(TEXT("AI starts the Game!"));
		UpdateUIText(Message);
	}
}

void AGameManager::StartGame()
{
	TArray<AActor*> FoundGrids;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGrid::StaticClass(), FoundGrids);
	if (FoundGrids.Num() > 0)
	{
		Grid = Cast<AGrid>(FoundGrids[0]);
	}
	
	if (Grid)
	{
		UE_LOG(LogTemp, Log, TEXT("Gioco iniziato!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Griglia non trovata!"));
	}

	GameUIManager->ShowPlacementScreenOnUI();
}

void AGameManager::CheckIfPlacementComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("🔎 Verifica posizionamento unità:"));
	UE_LOG(LogTemp, Warning, TEXT("🔵 bHasPlacedBrawler = %s"), bHasPlacedBrawler ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("🔵 bHasPlacedSniper = %s"), bHasPlacedSniper ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("🔴 bAIHasPlacedBrawler = %s"), bAIHasPlacedBrawler ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogTemp, Warning, TEXT("🔴 bAIHasPlacedSniper = %s"), bAIHasPlacedSniper ? TEXT("TRUE") : TEXT("FALSE"));

	
	if (bHasPlacedBrawler && bHasPlacedSniper && 
		bAIHasPlacedBrawler && bAIHasPlacedSniper)
	{
		UE_LOG(LogTemp, Warning, TEXT("TUTTE le unità sono state posizionate! Inizia la fase di movimento."));

		for (AUnit* Unit : AllUnits)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s controllata da: %s"),
				*Unit->GetName(),
				Unit->bIsPlayerControlled ? TEXT("Giocatore") : TEXT("AI"));
		}

		if (GameUIManager && GameUIManager->GetGameUI())
		{
			GameUIManager->ShowGameplayScreenOnUI();
			UE_LOG(LogTemp, Warning, TEXT("GameUIManager: Passato ai pulsanti di azione."));
		}
		
		UpdateTurnDisplay();
		UpdateUIText("Start the Action!");
		if (StartingPlayer == 1) 
		{
			UE_LOG(LogTemp, Warning, TEXT("L'IA inizia il primo turno."));
			CurrentPlayerTurn = StartingPlayer;
			MoveAIUnits();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Il giocatore inizia il primo turno."));
			CurrentPlayerTurn = StartingPlayer;
			EnablePlayerMovement();
		}
		
		return;
	}
	if (!(bHasPlacedBrawler && bHasPlacedSniper && bAIHasPlacedBrawler && bAIHasPlacedSniper))
	{
		bPlayerTurnToPlace = !bPlayerTurnToPlace;
		CurrentPlayerTurn = bPlayerTurnToPlace;

		if (!bPlayerTurnToPlace)
		{
			UpdateTurnDisplay();
			UE_LOG(LogTemp, Warning, TEXT("CheckIfPlacementComplete(): Ora è il turno dell'IA per posizionare un'unità."));
			FTimerHandle AITimerHandle;
			GetWorld()->GetTimerManager().SetTimer(AITimerHandle, this, &AGameManager::AITurnPlacement, 1.0f, false);
		}
		else
		{
			UpdateTurnDisplay();
			UE_LOG(LogTemp, Warning, TEXT("CheckIfPlacementComplete(): Ora è il turno del giocatore per posizionare un'unità."));
		}
	}
	
}

void AGameManager::EndTurn()
{
	CheckWinCondition();
	if (IsGameOver) return;
	
	CurrentPlayerTurn = (CurrentPlayerTurn == 0) ? 1 : 0;
	UpdateTurnDisplay();
	
	if (CurrentPlayerTurn == 1)
	{
		ActionsButton(false);
		UE_LOG(LogTemp, Warning, TEXT("AI's Turn!"));
		MoveAIUnits();
	}
	else if (CurrentPlayerTurn == 0)
	{
		ActionsButton(true);
		UE_LOG(LogTemp, Warning, TEXT("Player's Turn!"));
		EnablePlayerMovement();
	}
}

void AGameManager::CheckWinCondition()
{
	if (AIUnits.Num() == 0 && PlayerUnits.Num() == 0)
	{
		Message = FString::Printf(TEXT("Draw! Damn bro really...?"));
		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
		IsGameOver = true;
		UpdateUIText(Message);
		ActionsButton(false);
		FTimerHandle EndTimerHandle;
		FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([this]()
		{
			GameUIManager->ShowDrawScreenOnUI();
		});

		GetWorld()->GetTimerManager().SetTimer(EndTimerHandle, TimerDel, 1.0f, false);
	}
	
	else if (PlayerUnits.Num() == 0)
	{
		Message = FString::Printf(TEXT("Game Over! AI wins!"));
		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
		IsGameOver = true;
		UpdateUIText(Message);
		ActionsButton(false);
		FTimerHandle EndTimerHandle;
		FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([this]()
		{
			GameUIManager->ShowLoseScreenOnUI();
		});

		GetWorld()->GetTimerManager().SetTimer(EndTimerHandle, TimerDel, 1.0f, false);
	}

	else if (AIUnits.Num() == 0)
	{
		Message = FString::Printf(TEXT("Game Over! Player wins!"));
		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
		IsGameOver = true;
		UpdateUIText(Message);
		ActionsButton(false);
		FTimerHandle EndTimerHandle;
		FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([this]()
		{
			GameUIManager->ShowVictoryScreenOnUI();
		});

		GetWorld()->GetTimerManager().SetTimer(EndTimerHandle, TimerDel, 1.0f, false);
	}

}

void AGameManager::UpdateUIText(FString NewText)
{
	if (GameUIManager && GameUIManager->GetGameUI())
	{
		GameUIManager->GetGameUI()->UpdateCustomText(NewText);
	}
	
}

void AGameManager::UpdateTurnDisplay()
{
	if (GameUIManager && GameUIManager->GetGameUI()) GameUIManager->GetGameUI()->UpdateTurnDisplay(CurrentPlayerTurn);
}

void AGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGameManager::AITurnPlacement()
{
	if (!Grid || AIPendingUnits.Num() == 0) return;

	FString AINextUnitType = AIPendingUnits[0];
	AIPendingUnits.RemoveAt(0);

	UE_LOG(LogTemp, Warning, TEXT("AITurnPlacement(): L'IA sta posizionando la sua unità: %s"), *AINextUnitType);

	AIPlaceUnit(AINextUnitType);
}


void AGameManager::AIPlaceUnit(FString UnitType)
{
	if (!Grid) return;

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, UnitType]()
	{
		if (!Grid || UnitType.IsEmpty()) return;
		UE_LOG(LogTemp, Warning, TEXT("AIPlaceUnit(): L'IA sta posizionando un %s..."), *UnitType);

		Message = FString::Printf(TEXT("AI is positioning its unit: %s"), *UnitType);
		UpdateUIText(Message);
		
		TSubclassOf<AUnit> UnitClass = (UnitType == "Brawler") ? BrawlerClass : SniperClass;
		if (!UnitClass)
		{
            return;
		}
		
		bool bPlacedSuccessfully = false;

		for (int i = 0; i < 100; ++i)
		{
			int32 X = FMath::RandRange(15, 24);
			int32 Y = FMath::RandRange(0, 24);
			FIntPoint AttemptPos(X, Y);

			AGridCell* TargetCell = Grid->GetCellAtPosition(AttemptPos);
			if (!TargetCell || TargetCell->IsObstacle || TargetCell->bIsOccupied) continue;

			AUnit* AIUnit = GetWorld()->SpawnActor<AUnit>(UnitClass);
			
			if (AIUnit && Grid->PlaceUnit(AIUnit, AttemptPos))
			{
				AddTotalUnits(AIUnit);
				AIUnit->IDCellPosition = AIUnit->GetIDCellPosition();
				
				UE_LOG(LogTemp, Log, TEXT("AI ha posizionato %s in (%d, %d) = %s"), *UnitType, X, Y, *AIUnit->IDCellPosition);
				Message = FString::Printf(TEXT("AI has positioned %s in %s"), *UnitType, *AIUnit->IDCellPosition);
				UpdateUIText(Message);
				
				AIUnit->bIsPlayerControlled = false;
				if (UnitType == "Brawler")
				{
					bAIHasPlacedBrawler = true; 
					AIBrawler = AIUnit;
				}
				else if (UnitType == "Sniper")
				{
					bAIHasPlacedSniper = true;
					AISniper = AIUnit;
				}

				bPlacedSuccessfully = true;
				break;
			}
			else
			{
				if (AIUnit)
				{
					AIUnit->Destroy();
				}
			}
		}

		if (!bPlacedSuccessfully)
		{
			UE_LOG(LogTemp, Error, TEXT("AIPlaceUnit(): Nessuna cella valida trovata dopo 100 tentativi per %s!"), *UnitType);
			return;
		}

		CheckIfPlacementComplete();
		
	}), 1.0f, false);
	
}

bool AGameManager::CanPlaceUnit(FString UnitType) const
{
	if (UnitType == "Brawler" && bHasPlacedBrawler) return false;
	if (UnitType == "Sniper" && bHasPlacedSniper) return false;

	return true;
}

void AGameManager::ActionsButton(bool on)
{
	if (GameUIManager && GameUIManager->GetGameUI())
	{
		GameUIManager->GetGameUI()->ActionButtons(on);
	}
}

void AGameManager::ToggleAttackMode(bool able)
{
	bIsAttacking = able;
	
	if (bIsAttacking && SelectedUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attacco attivato per %s. Seleziona un'unità nemica. (toggleattackmode)"), *SelectedUnit->GetName());

		for (AUnit* AIUnit : AIUnits)
		{
			if (!AIUnit) continue;
			int32 DistX = FMath::Abs(AIUnit->GridPosition.X - SelectedUnit->GridPosition.X);
			int32 DistY = FMath::Abs(AIUnit->GridPosition.Y - SelectedUnit->GridPosition.Y);

			if (DistX + DistY <= SelectedUnit->AttackRange)
			{
				AGridCell* Cell = Grid->GetCellAtPosition(AIUnit->GridPosition);
				if (Cell)
				{
					Cell->SetAISelectedMaterial(true);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Attacco disattivato."));
		for (AUnit* AIUnit : AIUnits)
		{
			if (!AIUnit) continue;
			
			AGridCell* Cell = Grid->GetCellAtPosition(AIUnit->GridPosition);
			if (Cell)
			{
				Cell->SetAISelectedMaterial(false);
			}
		}
	}
}

void AGameManager::ToggleAttackMode()
{
	bIsAttacking = !bIsAttacking;
}

void AGameManager::AttackSelectedUnit()
{
	if (!TargetUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessun bersaglio selezionato!"));
		return;
	}

	if (!SelectedUnit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Nessuna unità selezionata per attaccare!"));
		return;
	}

	if (SelectedUnit->bHasAttacked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Questa unità ha già attaccato questo turno."));
		return;
	}

	if (PlayerUnits.Contains(TargetUnit))
	{
		UE_LOG(LogTemp, Warning, TEXT("Non puoi attaccare un'unità alleata!"));
		return;
	}

	SelectedUnit->Attack(TargetUnit);
	GameUIManager->UpdateAllHealthBars();
	
	if (SelectedUnit->bHasAttacked)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s ha attaccato %s!"), *SelectedUnit->GetName(), *TargetUnit->GetName());
		Message = FString::Printf(TEXT("%s attacked %s [HP: %d/%d]!"), *SelectedUnit->GetName(), *TargetUnit->GetName(), TargetUnit->Health, TargetUnit->MaxHealth);
		UpdateUIText(Message);
		GameUIManager->LogAttack(SelectedUnit, TargetUnit, (TargetUnit->MaxHealth - TargetUnit->Health));
	}

	ToggleAttackMode(false);
	ClearTargetUnit();
	IncrementPlayerMoves();
}

void AGameManager::SetSelectedUnit(AUnit* Unit)
{
	if (!Unit) return;

	if (IsAttacking() && AIUnits.Contains(Unit))
	{
		UE_LOG(LogTemp, Warning, TEXT("Bersaglio valido selezionato: %s"), *Unit->GetName());
		SetTargetUnit(Unit);
		AttackSelectedUnit();
		Grid->ClearHighlightedCells();
		return;
	}

	if (!PlayerUnits.Contains(Unit))
	{
		UE_LOG(LogTemp, Warning, TEXT("Non puoi selezionare unità nemiche!"));
		UpdateUIText("You can't select enemy's units!");
		Grid->ClearHighlightedCells();
		SelectedUnit = nullptr;
		return;
	}

	if (IsAttacking())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cambiata unità selezionata per attaccare: %s"), *Unit->GetName());
	}
	
	SelectedUnit = Unit;
	UE_LOG(LogTemp, Warning, TEXT("Unità selezionata: %s"), *Unit->GetName());

	Grid->HighlightMovableCells(Unit);

	if (GameUIManager)
	{
		GameUIManager->SetSelectedUnit(Unit);
	}
	
}

void AGameManager::SetTargetUnit(AUnit* NewTarget)
{
	if (!NewTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetTargetUnit fallito: bersaglio nullo"));
		UpdateUIText("Null Target");
		return;
	}

	TargetUnit = NewTarget;
	UE_LOG(LogTemp, Warning, TEXT("Bersaglio impostato: %s"), *TargetUnit->GetName());
}

void AGameManager::ToggleMovementMode()
{
	bIsMovingUnit = !bIsMovingUnit;

	if (!bIsMovingUnit)
	{
		if (Grid) Grid->ClearHighlightedCells();
		SelectedUnit = nullptr;
	}
}

void AGameManager::ToggleMovementMode(bool able)
{
	bIsMovingUnit = able;

	if (!bIsMovingUnit)
	{
		if (Grid) Grid->ClearHighlightedCells();
		UE_LOG(LogTemp, Log, TEXT("Movimento disattivato."));
	}
}

void AGameManager::ConfirmMove()
{
	if (!SelectedUnit || PendingMovePath.Num() <= 1) return;

	int32 StepCount = PendingMovePath.Num();
	FString FromCellID = SelectedUnit->IDCellPosition;

	for (int32 i = 1; i < StepCount; ++i)
	{
		FIntPoint Pos = PendingMovePath[i];

		FTimerHandle TempHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TempHandle,
			[this, Pos]()
			{
				if (SelectedUnit)
				{
					SelectedUnit->MoveToCell(Pos);
					UE_LOG(LogTemp, Warning, TEXT("Mossa a (%d, %d)"), Pos.X, Pos.Y);
				}
			},
			i * 0.25f,
			false
		);
	}

	FTimerHandle EndHandle;
	GetWorld()->GetTimerManager().SetTimer(
		EndHandle,
		[this, FromCellID]()
		{
			if (SelectedUnit)
			{
				SelectedUnit->bHasMoved = true;
			}
			Grid->ClearHighlightedCells();
			PendingMovePath.Empty();
			SelectedUnit->IDCellPosition = SelectedUnit->GetIDCellPosition();
			Message = FString::Printf(TEXT("Player has moved %s in %s"), *SelectedUnit->GetName(), *SelectedUnit->IDCellPosition);
			UpdateUIText(Message);

			GameUIManager->LogMovement(SelectedUnit, FromCellID, SelectedUnit->IDCellPosition);
			
			ToggleMovementMode(false);
			
			if (IsEnemyInRange(SelectedUnit))
			{
				UE_LOG(LogTemp, Warning, TEXT("Nemico nel range!"));
				UpdateUIText("Enemy units in the beam! Do you want to attack?");
				UE_LOG(LogTemp, Warning, TEXT("Attesa: il giocatore può decidere se attaccare o meno..."));
				ToggleAttackMode(true);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Nessun nemico nel range. Turno terminato."));
				IncrementPlayerMoves();
				SetSelectedUnit(nullptr);
			}

		},
		StepCount * 0.25f,
		false
	);
}

void AGameManager::EnablePlayerMovement()
{
	PlayerMovesThisTurn = 0;
	NumPlayerUnitsThisTurn = CountPlayerUnitsAlive();


	for (AUnit* Unit : PlayerUnits)
	{
		if (Unit) Unit->ResetTurnState();
	}

	UE_LOG(LogTemp, Warning, TEXT("Il giocatore può muovere le sue unità."));

}

void AGameManager::IncrementPlayerMoves()
{
	PlayerMovesThisTurn++;

	UE_LOG(LogTemp, Warning, TEXT("Il giocatore ha mosso %d unità."), PlayerMovesThisTurn);

	if (PlayerMovesThisTurn >= NumPlayerUnitsThisTurn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Turno del giocatore finito, passando all'IA..."));
		EndTurn();
	}
}

int32 AGameManager::CountPlayerUnitsAlive()
{
	int32 Count = 0;
	for (AUnit* Unit : PlayerUnits)
	{
		if (Unit && Unit->Health > 0)
		{
			++Count;
		}
	}
	return Count;
}

void AGameManager::MoveAIUnits()
{
	if (AIUnits.Num() == 0) return;

	UE_LOG(LogTemp, Warning, TEXT("UNITA AI: %d"), AIUnits.Num());

	int32 UnitIndex = 0;
	for (AUnit* AIUnit : AIUnits)
	{
		FString FromCellID = AIUnit->IDCellPosition;
		if (!AIUnit || !IsValid(AIUnit)) continue;
		UnitIndex++;
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this, AIUnit, FromCellID]()
			{
				GetGrid()->ClearHighlightedCells();
				SelectedUnit = AIUnit;

				if (!SelectedUnit || !Grid)
				{
					UE_LOG(LogTemp, Error, TEXT("Errore: AIUnit o Grid non valido!"));
					return;
				}

				if (TryAttackInRange(AIUnit)) return;

				FIntPoint RandomDestination = Grid->GetRandomAvailableCell(SelectedUnit);

				if (RandomDestination != SelectedUnit->GridPosition && GetGrid()->IsHighlighted(RandomDestination) &&
					!GetGrid()->IsCellOccupied(RandomDestination, SelectedUnit))
				{
					PendingMovePath = Grid->FindShortestPath(SelectedUnit->GridPosition, RandomDestination, SelectedUnit);

					if (PendingMovePath.Num() > 1)
					{
						Grid->ClearHighlightedCells();
						int32 StepIndex = 0;
						for (FIntPoint Pos : PendingMovePath)
						{
							FTimerHandle MoveTimer;
							GetWorld()->GetTimerManager().SetTimer(
								MoveTimer,
								[this, Pos, AIUnit]()
								{
									if (AIUnit) AIUnit->MoveToCell(Pos);
								},
								StepIndex * 0.3f,
								false
							);
							StepIndex++;
						}
					}
				}

				FTimerHandle EndHandle;
				GetWorld()->GetTimerManager().SetTimer(
					EndHandle,
					[this, FromCellID]()
					{
						if (SelectedUnit)
						{
							SelectedUnit->bHasMoved = true;
						}
						Grid->ClearHighlightedCells();
						PendingMovePath.Empty();
						
						SelectedUnit->IDCellPosition = SelectedUnit->GetIDCellPosition();
						Message = FString::Printf(TEXT("%s has moved to %s"), *SelectedUnit->GetName(), *SelectedUnit->IDCellPosition);
						UpdateUIText(Message);

						GameUIManager->LogMovement(SelectedUnit, FromCellID, SelectedUnit->IDCellPosition);
					},
				PendingMovePath.Num() * 0.25f + 0.5f,
				false
				);  			

				// ATTACCO DOPO MOVIMENTO
				FTimerHandle AttackTimer;
				GetWorld()->GetTimerManager().SetTimer(
					AttackTimer,
					[this, AIUnit]()
					{
						if (!TryAttackInRange(AIUnit))
							UE_LOG(LogTemp, Warning, TEXT("%s non ha trovato nessuno da attaccare dopo il movimento."), *AIUnit->GetName());
					},
					PendingMovePath.Num() * 0.3f + 0.5f,
					false
				);
			},
			UnitIndex * 3.5f,
			false
		);
	}
	
	FTimerHandle EndTurnTimer;
	GetWorld()->GetTimerManager().SetTimer(
		EndTurnTimer,
		this,
		&AGameManager::EndTurn,
		(AIUnits.Num() + 1) * 3.5f,
		false
	);
}

bool AGameManager::TryAttackInRange(AUnit* AttackingUnit)
{
	if (!AttackingUnit) return false;

	AUnit* WeakestTarget = nullptr;
	int32 MinHealth = INT32_MAX;

	for (AUnit* PlayerUnit : PlayerUnits)
	{
		if (!PlayerUnit || !IsValid(PlayerUnit)) continue;

		int32 DX = FMath::Abs(AttackingUnit->GridPosition.X - PlayerUnit->GridPosition.X);
		int32 DY = FMath::Abs(AttackingUnit->GridPosition.Y - PlayerUnit->GridPosition.Y);
		int32 Distance = DX + DY;

		if (Distance > 0 && Distance <= AttackingUnit->AttackRange)
		{
			if (PlayerUnit->Health < MinHealth)
			{
				MinHealth = PlayerUnit->Health;
				WeakestTarget = PlayerUnit;
			}
		}
	}

	if (WeakestTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attacco al bersaglio più debole: %s -> %s (HP: %d)"),
			*AttackingUnit->GetName(), *WeakestTarget->GetName(), WeakestTarget->Health);

		AttackingUnit->Attack(WeakestTarget);
		GameUIManager->UpdateAllHealthBars();

		GameUIManager->LogAttack(AttackingUnit, WeakestTarget, WeakestTarget->MaxHealth - WeakestTarget->Health);

		return true;
	}

	return false;
}

bool AGameManager::IsEnemyInRange(AUnit* Attacker)
{
	if (!Attacker) return false;

	for (AUnit* Target : AIUnits)
	{
		if (!Target || !IsValid(Target)) continue;

		int32 Distance = FMath::Abs(Attacker->GridPosition.X - Target->GridPosition.X) +
						 FMath::Abs(Attacker->GridPosition.Y - Target->GridPosition.Y);

		if (Distance > 0 && Distance <= Attacker->AttackRange)
		{
			return true;
		}
	}

	return false;
}

void AGameManager::RemoveUnit(AUnit* Unit)
{
	if (!Unit) return;

	AGridCell* UnitCell = Grid->GetCellAtPosition(Unit->GridPosition);

	if (PlayerUnits.Contains(Unit))
	{
		UnitCell->SetAsObstacle(false);
		PlayerUnits.Remove(Unit);
		UnitCell->bIsOccupied = false;
		UE_LOG(LogTemp, Warning, TEXT("Rimosso %s dal Player"), *Unit->GetName());
	}

	if (AIUnits.Contains(Unit))
	{
		UnitCell->SetAsObstacle(false);
		AIUnits.Remove(Unit);
		UnitCell->bIsOccupied = false;
		UE_LOG(LogTemp, Warning, TEXT("Rimosso %s dalla IA"), *Unit->GetName());
	}
}

