// Copyright (C) 2018 by Eryk Dwornicki All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LD41TurnListener.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULD41TurnListener : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class LD41_API ILD41TurnListener
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintImplementableEvent, Category="Turn Listener")
	void OnNewTurn(int32 TurnNumber);

	UFUNCTION(BlueprintImplementableEvent, Category = "Turn Listener")
	void OnTurnFinish(int32 TurnNumber);

	UFUNCTION(BlueprintImplementableEvent, Category="Turn Listener")
	void HandleTurnTransition(float Progress);
};
