// Fill out your copyright notice in the Description page of Project Settings.

#include "RaceTrackActor.h"

#include "Engine/World.h"
#include "Engine/DataTable.h"

#include "Kismet/GameplayStatics.h"

#include "Cars/Car.h"

#include "Perks/PerkPickupActor.h"
#include "Perks/PerkSpawnInfo.h"

#include "RaceTrackPrefabActor.h"

#include "Package.h"

#include "LD41GameModeBase.h"

DECLARE_LOG_CATEGORY_CLASS(LogRaceTrackActor, Log, All);

/** The minimum distance between camera and last spawned prefab to generate further road. */
const float GENERATE_MIN_DISTANCE(100000.0f);

// Sets default values
ARaceTrackActor::ARaceTrackActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

ARaceTrackPrefabActor* ARaceTrackActor::CreatePrefab()
{
	TArray<TSubclassOf<ARaceTrackPrefabActor> > PrefabCandidates;

	const bool IsFirstPrefab = LastSpawnedPrefab == nullptr;
	const float Probability = FMath::RandRange(0.0f, 1.0f);
	for (TSubclassOf<ARaceTrackPrefabActor>& Class : Prefabs)
	{
		auto DefaultPrefabObject = Cast<ARaceTrackPrefabActor>(Class->GetDefaultObject());
		if (IsFirstPrefab && !DefaultPrefabObject->bCanSpawnAsFirst)
		{
			continue;
		}

		if (DefaultPrefabObject->Probability >= Probability)
		{
			PrefabCandidates.Add(Class);
		}
	}

	if (PrefabCandidates.Num() == 0)
	{
		UE_LOG(LogRaceTrackActor, Error, TEXT("Failed to create prefab. No prefab candidate found."));
		return nullptr;
	}

	TSubclassOf<ARaceTrackPrefabActor> PrefabClass(PrefabCandidates[FMath::RandRange(0, PrefabCandidates.Num() - 1)]);
	check(PrefabClass);

	FTransform PrefabSpawnTransform;
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARaceTrackPrefabActor* Prefab = GetWorld()->SpawnActor<ARaceTrackPrefabActor>(PrefabClass, PrefabSpawnTransform, ActorSpawnParameters);
	if (!Prefab->IsValidPrefab())
	{
		UE_LOG(LogRaceTrackActor, Error, TEXT("Failed to create prefab. Prefab is invalid."));
		Prefab->Destroy();
		return nullptr;
	}

	if (LastSpawnedPrefab)
	{
		const FTransform LastEndLocatorTransform(LastSpawnedPrefab->EndLocatorComponent->GetComponentTransform());
		const FTransform NewStartLocatorTransform(Prefab->EndLocatorComponent->GetComponentTransform());

		PrefabSpawnTransform = LastEndLocatorTransform + NewStartLocatorTransform;
	}
	else
	{
		PrefabSpawnTransform = GetActorTransform();
	}

	// Keep scale.

	PrefabSpawnTransform.SetScale3D(Prefab->GetActorScale3D());

	Prefab->SetActorTransform(PrefabSpawnTransform);
	if (LastSpawnedPrefab)
	{
		LastSpawnedPrefab->NextPrefab = Prefab;
	}
	LastSpawnedPrefab = Prefab;

	if (!LastPrefab)
	{
		LastPrefab = Prefab;
	}
	else
	{
		// Spawn perks on this prefab.
		SpawnPerks(Prefab);
	}

	return Prefab;
}

void ARaceTrackActor::SpawnPerks(ARaceTrackPrefabActor* Prefab)
{
	TArray<FTransform> PerkSpawnPoints;
	Prefab->GetPerkSpawnPoints(PerkSpawnPoints);
	if (PerkSpawnPoints.Num() == 0)
	{
		return;
	}

	TArray<FPerkSpawnInfo*> PerksSpawnInfo;
	PerksSpawnTable->GetAllRows("SpawnPerks", PerksSpawnInfo);

	const float Probability = FMath::RandRange(0.0f, 1.0f);
	TArray<FPerkSpawnInfo*> PerkCandidates;
	for (FPerkSpawnInfo* PerkSpawnInfo : PerksSpawnInfo)
	{
		if (PerkSpawnInfo->Probability < Probability)
		{
			continue;
		}

		PerkCandidates.Add(PerkSpawnInfo);
	}

	if (PerkCandidates.Num() == 0)
	{
		return;
	}

	FPerkSpawnInfo* PerkToSpawn = PerkCandidates[FMath::RandRange(0, PerkCandidates.Num() - 1)];
	if (!PerkToSpawn->PickupClass)
	{
		return;
	}

	FTransform SpawnTransform(PerkSpawnPoints[FMath::RandRange(0, PerkSpawnPoints.Num() - 1)]);
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<APerkPickupActor>(PerkToSpawn->PickupClass, SpawnTransform, ActorSpawnParameters);
}

// Called when the game starts or when spawned
void ARaceTrackActor::BeginPlay()
{
	Super::BeginPlay();

	const FString TrackClassName(GetClass()->GetName());
	if (Prefabs.Num() == 0)
	{
		UE_LOG(LogRaceTrackActor, Error, TEXT("Failed to generate track. No prefabs are defined. (Track class: %s)"), *TrackClassName);
		return;
	}

	ARaceTrackPrefabActor* StartPrefab = CreatePrefab();
	if (!StartPrefab)
	{
		UE_LOG(LogRaceTrackActor, Error, TEXT("Failed to start game. No start prefab. (Track class: %s)"), *TrackClassName);
		return;
	}

	if (!CarClass)
	{
		UE_LOG(LogRaceTrackActor, Error, TEXT("Failed to start game. No car class defined. (Track class: %s)"), *TrackClassName);
		return;
	}

	for (int32 i = 0; i < 2; ++i)
	{
		FTransform SpawnPoint(StartPrefab->GetRoutePoint(i));
		FActorSpawnParameters ActorSpawnParameters;
		ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACar* Car = GetWorld()->SpawnActor<ACar>(CarClass, SpawnPoint, ActorSpawnParameters);

		AController* Controller = nullptr;
		if (i == 0)
		{
			Controller = UGameplayStatics::GetPlayerController(GetWorld(), i);
		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Instigator = Car->Instigator;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.OverrideLevel = Car->GetLevel();
			SpawnInfo.ObjectFlags |= RF_Transient;
			auto GameMode = Cast<ALD41GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
			Controller = GetWorld()->SpawnActor<AController>(GameMode->AIControllerClass, SpawnInfo);
		}
		Controller->Possess(Car);

		Car->CurrentPrefab = StartPrefab;
		Car->CurrentRoutePoint = i;
	}
}

// Called every frame
void ARaceTrackActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update camera location.

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		FRotator CameraRotation;
		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	}

	// Generate track when camera moves.

	bool GenerateTrack = true;
	if (LastSpawnedPrefab)
	{
		const float Distance = FVector::Dist2D(LastSpawnedPrefab->GetActorLocation(), CameraLocation);
		GenerateTrack = Distance < GENERATE_MIN_DISTANCE;
	}
	if (GenerateTrack)
	{
		CreatePrefab();
	}

	// Destroy prefabs that are far from the camera.

	while (LastPrefab)
	{
		// If last prefab is too far from the camera do not destroy any prefab.

		const float Distance = FVector::Dist2D(LastPrefab->GetActorLocation(), CameraLocation);
		if (Distance < LastPrefab->DestroyDistance)
		{
			break;
		}

		// Destroy prefab.

		ARaceTrackPrefabActor* PrefabToDestroy = LastPrefab;
		LastPrefab = PrefabToDestroy->NextPrefab;
		PrefabToDestroy->Destroy();
	}
}


