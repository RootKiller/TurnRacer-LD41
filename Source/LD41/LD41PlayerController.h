// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LD41PlayerController.generated.h"

class ACar;

/**
 *
 */
UCLASS()
class LD41_API ALD41PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALD41PlayerController();

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Car")
	ACar* GetCar() const;
};
