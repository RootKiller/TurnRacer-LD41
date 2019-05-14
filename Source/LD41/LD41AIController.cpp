// Copyright (C) 2018 by Eryk Dwornicki All Rights Reserved

#include "LD41AIController.h"

#include "Cars/Car.h"

DECLARE_LOG_CATEGORY_CLASS(LogLD41AIController, Log, All);

void ALD41AIController::OnNewTurn_Implementation(int32 TurnNumber)
{
}

void ALD41AIController::OnTurnFinish_Implementation(int32 TurnNumber)
{
	Think();
}

void ALD41AIController::HandleTurnTransition_Implementation(float Progress)
{
}

FORCEINLINE float GetSteerValueDecition(unsigned DecisionIndex)
{
	const float SteerValues[] = { 0.0f, -1.0f, 1.0f };
	if (DecisionIndex < ARRAY_COUNT(SteerValues))
	{
		return SteerValues[DecisionIndex];
	}
	return 0.0f;
}

void ALD41AIController::Think_Implementation()
{
	ACar *Car = GetCar();
	if (!Car)
	{
		UE_LOG(LogLD41AIController, Error, TEXT("AI controller failed to think. No car pawn is being controlled by me!"));
		return;
	}

	int32 Retry = 3;
	int32 BestDecision = -1;
	while (Retry > 0)
	{
		Car->SteerValue = GetSteerValueDecition(--Retry);
		Car->UpdateNextPrefab();

		if (Car->IsTargetClear())
		{
			BestDecision = Retry;
			if (Car->IsTargetPerk())
			{
				break;
			}
		}
	}

	if (BestDecision != -1)
	{
		Car->ForwardValue = 1.0f;
		float SteerValue = GetSteerValueDecition(BestDecision);
		bool NeedsUpdate = Car->SteerValue != SteerValue;
		if (NeedsUpdate)
		{
			Car->SteerValue = SteerValue;
			Car->UpdateNextPrefab();
		}
	}
	else
	{
		Car->ForwardValue = 0.0f;
	}
}

ACar* ALD41AIController::GetCar() const
{
	return Cast<ACar>(GetPawn());
}
