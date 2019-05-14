// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaceTrackPrefabActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class LD41_API ARaceTrackPrefabActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARaceTrackPrefabActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CacheLocators();
	void CacheRoutePoints();
	void CachePerkSpawnPoints();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category="Race Track Prefab")
	float Probability = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Race Track Prefab")
	float DestroyDistance = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Race Track Prefab")
	bool bCanSpawnAsFirst = true;

	UPROPERTY()
	USceneComponent* StartLocatorComponent = nullptr;
	UPROPERTY()
	USceneComponent* EndLocatorComponent = nullptr;

	UPROPERTY()
	TArray<FTransform> RoutePoints;
	UPROPERTY()
	TArray<USceneComponent*> PerkSpawnPoints;

	UPROPERTY()
	ARaceTrackPrefabActor* NextPrefab = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category="Race Track Prefab")
	UStaticMeshComponent* TrackMesh;

public:

	bool IsValidPrefab() const;

	int32 GetNumRoutePoints() const;
	FTransform GetRoutePoint(int32 Index) const;

	void GetPerkSpawnPoints(TArray<FTransform> &PerkSpawnPointsTransform);
};
