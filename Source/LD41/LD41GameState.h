// Copyright (C) 2018 by Eryk Dwornicki All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LD41GameState.generated.h"

class ALD41PlayerState;

UENUM(BlueprintType)
enum class ELD41GameState : uint8
{
	Countdown,
	Turn,
	TurnTransition,
	GameOver,
};

/**
 *
 */
UCLASS()
class LD41_API ALD41GameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category="Turn")
	ELD41GameState ActiveState = ELD41GameState::Countdown;

	/** The current turn number. */
	UPROPERTY(BlueprintReadOnly, Category="Turn")
	int32 TurnNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category="Turn")
	float TimeTillNextState = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category="Turn")
	UPlayer* Winner = nullptr;
};
