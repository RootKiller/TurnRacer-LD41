// Copyright (C) 2018 by Eryk Dwornicki All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LD41TurnListener.h"
#include "LD41AIController.generated.h"

class ACar;

/**
 *
 */
UCLASS()
class LD41_API ALD41AIController : public AAIController, public ILD41TurnListener
{
	GENERATED_BODY()

public:


	UFUNCTION(BlueprintNativeEvent, Category = "Turn Listener")
	void OnNewTurn(int32 TurnNumber);

	UFUNCTION(BlueprintNativeEvent, Category = "Turn Listener")
	void OnTurnFinish(int32 TurnNumber);

	UFUNCTION(BlueprintNativeEvent, Category = "Turn Listener")
	void HandleTurnTransition(float Progress);

	UFUNCTION(BlueprintNativeEvent, Category = "Turn Listener")
	void Think();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AI Controller")
	ACar* GetCar() const;

private:



};
