// Fill out your copyright notice in the Description page of Project Settings.

#include "RaceTrackPrefabActor.h"

#include "Engine/StaticMeshSocket.h"
#include "Engine/StaticMesh.h"

#include "Components/StaticMeshComponent.h"
#include "LogMacros.h"

const FString START_LOCATOR_COMPONENT_NAME("StartLocator");
const FString END_LOCATOR_COMPONENT_NAME("EndLocator");

const int32 MAX_ROUTE_POINTS = 2;

DECLARE_LOG_CATEGORY_CLASS(LogRaceTrackPrefabActor, Log, All);

// Sets default values
ARaceTrackPrefabActor::ARaceTrackPrefabActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	TrackMesh = CreateDefaultSubobject<UStaticMeshComponent>("TrackMesh");
	TrackMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ARaceTrackPrefabActor::BeginPlay()
{
	Super::BeginPlay();

	CacheLocators();
	CacheRoutePoints();
	CachePerkSpawnPoints();

	if (RoutePoints.Num() == 0)
	{
		UE_LOG(LogRaceTrackPrefabActor, Error, TEXT("Prefab has no route points! Expect bugz! Class: %s"), *GetClass()->GetName());
	}
}

// Called every frame
void ARaceTrackPrefabActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARaceTrackPrefabActor::CacheRoutePoints()
{
	UStaticMesh* Mesh = TrackMesh->GetStaticMesh();
	if (!Mesh)
	{
		UE_LOG(LogRaceTrackPrefabActor, Error, TEXT("Failed to cache route points. No track mesh is set. Class: %s"), *GetClass()->GetName());
		return;
	}

	for (UStaticMeshSocket* Socket : Mesh->Sockets)
	{
		if (Socket->Tag != "RoutePoint")
		{
			continue;
		}

		RoutePoints.Add(FTransform(Socket->RelativeRotation, Socket->RelativeLocation, FVector::OneVector));
	}
}

void ARaceTrackPrefabActor::CachePerkSpawnPoints()
{
	TArray<USceneComponent*> Components;
	GetComponents(Components);
	for (USceneComponent* Component : Components)
	{
		if (!Component->GetName().StartsWith("PerkSpawnPoint"))
		{
			continue;
		}

		PerkSpawnPoints.Add(Component);
	}
}

void ARaceTrackPrefabActor::CacheLocators()
{
	if (StartLocatorComponent && EndLocatorComponent)
	{
		return;
	}

	TArray<USceneComponent*> Components;
	GetComponents(Components);
	for (USceneComponent* Component : Components)
	{
		if (Component->GetName() == START_LOCATOR_COMPONENT_NAME)
		{
			StartLocatorComponent = Component;
		}
		if (Component->GetName() == END_LOCATOR_COMPONENT_NAME)
		{
			EndLocatorComponent = Component;
		}
	}

	const FString PrefabClassName(GetClass()->GetName());
	if (!StartLocatorComponent)
	{
		UE_LOG(LogRaceTrackPrefabActor, Error, TEXT("Failed to find start locator component! (Prefab class: %s)"), *PrefabClassName);
	}

	if (!EndLocatorComponent)
	{
		UE_LOG(LogRaceTrackPrefabActor, Error, TEXT("Failed to find start locator component! (Prefab class: %s)"), *PrefabClassName);
	}
}

bool ARaceTrackPrefabActor::IsValidPrefab() const
{
	return StartLocatorComponent && EndLocatorComponent;
}

int32 ARaceTrackPrefabActor::GetNumRoutePoints() const
{
	return RoutePoints.Num();
}

FTransform ARaceTrackPrefabActor::GetRoutePoint(int32 Index) const
{
	if (Index < RoutePoints.Num())
	{
		FTransform SpawnTransform(RoutePoints[Index] * TrackMesh->GetComponentTransform());
		return SpawnTransform;
	}

	UE_LOG(LogRaceTrackPrefabActor, Error, TEXT("Failed to get route point at id %i."), Index);
	return FTransform();
}

void ARaceTrackPrefabActor::GetPerkSpawnPoints(TArray<FTransform> &PerkSpawnPointsTransform)
{
	for (USceneComponent* Component : PerkSpawnPoints)
	{
		FTransform Transform(Component->GetComponentTransform());
		Transform.SetScale3D(FVector::OneVector);
		PerkSpawnPointsTransform.Add(Transform);
	}
}
