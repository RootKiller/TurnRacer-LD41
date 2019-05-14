// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LD41AIController.h"
#include "Delegate.h"
#include "LD41GameModeBase.generated.h"

class ACar;
class ALD41AIController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FGameOverCallbackSignature, int32, Tour, UPlayer*, Winner );

/**
 *
 */
UCLASS()
class LD41_API ALD41GameModeBase : public AGameModeBase
{
	GENERATED_BODY()


public:
	ALD41GameModeBase();

	virtual void Tick(float DeltaSeconds) override;

	void GameOver();

	void OnCarCrash(ACar *Car, const FHitResult &HitResult);
	void OnCarRunOutOfFuel(ACar *Car);

	UPROPERTY(EditDefaultsOnly, Category="Rules")
	float TurnLength = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	float TurnTransitionLength = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	float CountdownLength = 3.0f;

	/** Opponent's AI controller. */
	UPROPERTY(EditDefaultsOnly, Category="AI")
	TSubclassOf<ALD41AIController> AIControllerClass;

	UPROPERTY(BlueprintAssignable, Category="Rules")
	FGameOverCallbackSignature OnGameOver;

protected:

	virtual void BeginPlay() override;
};
