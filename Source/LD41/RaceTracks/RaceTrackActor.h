// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaceTrackActor.generated.h"

class ARaceTrackPrefabActor;
class ACar;
class UDataTable;

UCLASS()
class LD41_API ARaceTrackActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARaceTrackActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SpawnPerks(ARaceTrackPrefabActor* Prefab);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Prefabs used to generate this track. */
	UPROPERTY(EditDefaultsOnly, Category = "Race Track")
	TArray<TSubclassOf<ARaceTrackPrefabActor> > Prefabs;

	UPROPERTY(EditDefaultsOnly, Category = "Race Track|Rules")
	TSubclassOf<ACar> CarClass;

	UPROPERTY(EditDefaultsOnly, Category = "Race Track|Rules")
	UDataTable *PerksSpawnTable = nullptr;

	UPROPERTY()
	ARaceTrackPrefabActor *LastSpawnedPrefab = nullptr;
	UPROPERTY()
	ARaceTrackPrefabActor *LastPrefab = nullptr;

	FVector CameraLocation;

	ARaceTrackPrefabActor* CreatePrefab();
};
