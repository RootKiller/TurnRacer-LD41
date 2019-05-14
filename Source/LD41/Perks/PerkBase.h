// Copyright (C) 2018 by Eryk Dwornicki All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "PerkBase.generated.h"

class ALD41PlayerController;
class UWorld;

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class LD41_API UPerkBase : public UObject
{
	GENERATED_BODY()

public:
	UPerkBase();
	~UPerkBase();

	/**
	 * Consume this perk.
	 *
	 * @param[in] Car The car that consumed this perk.
	 */
	UFUNCTION(BlueprintNativeEvent, Category="Perk")
	void Consume(ACar* Car);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Perk")
	UTexture2D *UITexture = nullptr;

	/** Can this perk be used? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Perk")
	bool CanConsume() const;

	/** Turns until the perk can be consumed again. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Perk")
	int32 ActiveCooldown = 0.0f;

	/** How many turns does the cooldown for this perk lasts? */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Perk")
	int32 CooldownLength = 0.0f;

	/** Should the perk be used on pickup? This enforces one use. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Perk")
	bool bUseOnPickup = false;

	/** Amount of uses left before this perk will be removed from the inventory. 0 means that there are no limits on it's use. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Perk")
	int32 Uses = 1;

	void OnNewTurn();

	bool DoesPickupRegenerate() const;

	//~ Begin UObject Interface
	virtual UWorld* GetWorld() const override;
	//~ End UObject Interface

	UWorld *World = nullptr;
};
