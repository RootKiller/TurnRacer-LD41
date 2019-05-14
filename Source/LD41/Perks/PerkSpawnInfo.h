// Copyright (C) 2018 by Eryk Dwornicki All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PerkSpawnInfo.generated.h"

class APerkPickupActor;

USTRUCT(BlueprintType)
struct LD41_API FPerkSpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Perk Spawn Info")
	TSubclassOf<APerkPickupActor> PickupClass;

	UPROPERTY(EditAnywhere, Category="Perk Spawn Info")
	float Probability = 1.0f;
};
