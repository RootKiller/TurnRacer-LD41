// Fill out your copyright notice in the Description page of Project Settings.

#include "LD41GameModeBase.h"

#include "Kismet/GameplayStatics.h"

#include "Engine/World.h"
#include "Engine/Engine.h"

#include "Cars/Car.h"

#include "LD41TurnListener.h"
#include "LD41GameState.h"

#include "DrawDebugHelpers.h"

DECLARE_LOG_CATEGORY_CLASS(LogLD41GameModeBase, Log, All);

ALD41GameModeBase::ALD41GameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = ALD41AIController::StaticClass();
}

void ALD41GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	ALD41GameState* CurrentGameState = Cast<ALD41GameState>(GameState);
	if (!CurrentGameState)
	{
		return;
	}

	CurrentGameState->TimeTillNextState = CountdownLength;
	CurrentGameState->ActiveState = ELD41GameState::Countdown;
}

void ALD41GameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ALD41GameState* CurrentGameState = Cast<ALD41GameState>(GameState);
	if (!CurrentGameState || CurrentGameState->ActiveState == ELD41GameState::GameOver)
	{
		return;
	}

	TArray<AActor*> TurnListenerActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), ULD41TurnListener::StaticClass(), TurnListenerActors);

	CurrentGameState->TimeTillNextState -= DeltaSeconds;

	if (CurrentGameState->ActiveState == ELD41GameState::TurnTransition)
	{
		float Progress = 1.0f - FMath::Clamp(CurrentGameState->TimeTillNextState / TurnTransitionLength, 0.0f, 1.0f);
		for (AActor* Actor : TurnListenerActors)
		{
			ILD41TurnListener::Execute_HandleTurnTransition(Actor, Progress);

			// Do not continue tranisition if game is over.
			if (CurrentGameState->ActiveState == ELD41GameState::GameOver)
			{
				return;
			}
		}
	}

	if (CurrentGameState->TimeTillNextState > 0.0f)
	{
		return;
	}

	switch (CurrentGameState->ActiveState)
	{
	case ELD41GameState::Turn:
		CurrentGameState->TimeTillNextState = TurnTransitionLength;
		CurrentGameState->ActiveState = ELD41GameState::TurnTransition;
		for (AActor* Actor : TurnListenerActors)
		{
			ILD41TurnListener::Execute_OnTurnFinish(Actor, CurrentGameState->TurnNumber);
		}
		break;

	case ELD41GameState::Countdown:
	case ELD41GameState::TurnTransition:
		CurrentGameState->TimeTillNextState = TurnLength;
		CurrentGameState->ActiveState = ELD41GameState::Turn;
		CurrentGameState->TurnNumber++;
		for (AActor* Actor : TurnListenerActors)
		{
			ILD41TurnListener::Execute_OnNewTurn(Actor, CurrentGameState->TurnNumber);
		}
		break;
	}
}

void ALD41GameModeBase::GameOver()
{
	ALD41GameState* CurrentGameState = Cast<ALD41GameState>(GameState);
	if (CurrentGameState)
	{
		CurrentGameState->ActiveState = ELD41GameState::GameOver;
		OnGameOver.Broadcast(CurrentGameState->TurnNumber, CurrentGameState->Winner);
	}
}

void ALD41GameModeBase::OnCarRunOutOfFuel(ACar *Car)
{
	ALD41GameState* CurrentGameState = Cast<ALD41GameState>(GameState);
	if (CurrentGameState && Car->GetController()->IsPlayerController())
	{
		auto Controller = Cast<APlayerController>(Car->GetController());
		CurrentGameState->Winner = Controller->Player;
	}

	GameOver();
}

void ALD41GameModeBase::OnCarCrash(ACar *Car, const FHitResult &HitResult)
{
	UE_LOG(LogLD41GameModeBase, Log, TEXT("OnCarCrash %s car crashed with %s."), *Car->GetFullName(), HitResult.GetActor() ? *HitResult.GetActor()->GetFullName() : TEXT("N/A"));

	ALD41GameState* CurrentGameState = Cast<ALD41GameState>(GameState);
	if (!CurrentGameState)
	{
		return;
	}

	ACar* CarA = Car;
	ACar* CarB = Cast<ACar>(HitResult.GetActor());
	ACar* Victim = nullptr;
	if (CarA && CarB)
	{
		const float AbsSteerValueA = FMath::Abs(CarA->SteerValue);
		const float AbsSteerValueB = FMath::Abs(CarB->SteerValue);
		if ((AbsSteerValueB - AbsSteerValueA) > KINDA_SMALL_NUMBER)
		{
			if (AbsSteerValueA > 0)
			{
				GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Green, TEXT("BB"));
				Victim = CarB;
			}
			if (AbsSteerValueB > 0)
			{
				GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Green, TEXT("AA"));
				Victim = CarA;
			}
		}
	}
	else
	{
		AController* Controller = Car->GetController();
		if (Controller && !Controller->IsPlayerController())
		{
			CurrentGameState->Winner = UGameplayStatics::GetPlayerController(GetWorld(), 0)->Player;
		}
	}


	AController* VictimController = Victim ? Victim->GetController() : nullptr;
	if (VictimController && VictimController->IsPlayerController())
	{
		CurrentGameState->Winner = Cast<APlayerController>(VictimController)->Player;
	}

	GameOver();
}
