// Fill out your copyright notice in the Description page of Project Settings.

#include "Car.h"

#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"

#include "Kismet/GameplayStatics.h"

#include "RaceTracks/RaceTrackPrefabActor.h"

#include "LD41PlayerController.h"
#include "LD41GameState.h"
#include "LD41GameModeBase.h"
#include "Perks/PerkBase.h"
#include "Perks/PerkPickupActor.h"

#include "DrawDebugHelpers.h"

const int32 INVENTORY_SIZE = 4;

DECLARE_LOG_CATEGORY_CLASS(LogCar, Log, All);

static TAutoConsoleVariable<int32> DebugCars(TEXT("g.DebugCars"), false, TEXT("Enable debug draw."));

// #define DEBUG_CARS

const int32 ECC_Car = ECC_GameTraceChannel2;

// Sets default values
ACar::ACar()
{
	PrimaryActorTick.bCanEverTick = true;
	PerkInventory.SetNum(INVENTORY_SIZE);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>("Body");
	BodyMesh->SetMobility(EComponentMobility::Movable);
	BodyMesh->SetCollisionObjectType(static_cast<ECollisionChannel>(ECC_Car));

	WayCorona = CreateDefaultSubobject<UStaticMeshComponent>("WayCorona");
	WayCorona->SetMobility(EComponentMobility::Movable);
	WayCorona->SetVisibility(false);

	WayBlocker = CreateDefaultSubobject<USphereComponent>("WayBlocker");
	WayBlocker->SetupAttachment(WayCorona);
	WayBlocker->InitSphereRadius(20.0f);
	WayBlocker->SetCollisionProfileName("WorldDynamic");
	WayBlocker->bAutoActivate = false;

	RootComponent = BodyMesh;
}

void ACar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DebugCars.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Blue,
			FString::Printf(
			TEXT("%s: Force Pos: %s, Target: %s, Current: %s"),
			*GetName(),
			bForceTargetPosition?TEXT("yes"):TEXT("no"),
			TargetPrefab?*TargetPrefab->GetName():TEXT(""),
			CurrentPrefab?*CurrentPrefab->GetName():TEXT("")
		));

		if (CurrentPrefab)
		{
			DrawDebugSphere(GetWorld(), CurrentPrefab->GetActorLocation(), 200.0f, 10, FColor::Yellow, false, -1.0f, 0, 3.0f);
		}
		DrawDebugLine(GetWorld(), GetActorLocation(), TargetTransform.GetLocation(), FColor::Black, false, -1.0f, 0, 5.0f);
		DrawDebugSphere(GetWorld(), TargetTransform.GetLocation(), 200.0f, 10, IsTargetClear() ? FColor::Green : FColor::Red, false, -1.0f, 0, 3.0f);
	}
}

bool ACar::IsOutOfFuel() const
{
	return Fuel <= 0;
}

bool ACar::IsMoving() const
{
	return ForwardValue > 0.0f || bForceTargetPosition;
}

void ACar::OnNewTurn_Implementation(int32 TurnNumber)
{
	// Update current prefab.

	if (IsMoving())
	{
		bForceTargetPosition = false;
		CurrentPrefab = TargetPrefab;
		CurrentRoutePoint = TargetRoutePoint;
		UpdateNextPrefab();
	}

	StepsNextRound = 1;
	LastProgress = 0.0f;
	SteerValue = 0.0f;
	ForwardValue = 0.0f;
}

void ACar::Refuel(int32 Amount /*= 100*/)
{
	Fuel = FMath::Clamp(Fuel + Amount, 0, 100);
}

void ACar::OnTurnFinish_Implementation(int32 TurnNumber)
{
	if (bGhostMode)
	{
		bGhostMode = false;
	}
	UpdateCarCollisions();

	// Update perks.

	for (int32 i = 0; i < INVENTORY_SIZE; ++i)
	{
		if (PerkInventory[i])
		{
			PerkInventory[i]->OnNewTurn();
		}
	}

	// Use the perk controller decided to use.

	if (PerkToUse != -1)
	{
		ConsumePerk(PerkToUse);
		PerkToUse = -1;
	}

	UpdateNextPrefab();
	UpdateWayCorona();

	// Update fuel.

	Fuel -= 1;

	if (Fuel <= 0)
	{
		auto GameMode = Cast<ALD41GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		GameMode->OnCarRunOutOfFuel(this);
	}

	PreviousTransform = GetActorTransform();
}

void ACar::HandleTurnTransition_Implementation(float Progress)
{
	// Check if we can move.

	if (!IsMoving() || bCrashed || IsOutOfFuel())
	{
		return;
	}



	// Perform movement.

	const float DeltaProgress = Progress - LastProgress;
	LastProgress = Progress;

	FHitResult SweepResult;
	SetActorLocation(FMath::Lerp(PreviousTransform.GetLocation(), TargetTransform.GetLocation(), Progress), true, &SweepResult);
	if (SweepResult.bBlockingHit)
	{
		HandleCrash(SweepResult);
		return;
	}

	if (!bForceTargetPosition)
	{
		const float RotationAlpha = FMath::Sin(Progress * PI);
		const FVector DeltaLocation(TargetTransform.GetLocation() - GetActorLocation());
		const FVector EffectiveDirection = FMath::Lerp(FVector::ForwardVector, DeltaLocation.GetSafeNormal(), RotationAlpha);
		const FRotator Rotation(FRotationMatrix::MakeFromX(EffectiveDirection).Rotator());
		SetActorRotation(Rotation);
	}
}

void ACar::HandleCrash_Implementation(const FHitResult &Hit)
{
	auto GameMode = Cast<ALD41GameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->OnCarCrash(this, Hit);

	bCrashed = true;
}


// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Steer", this, &ACar::HandleSteerAxis);
	PlayerInputComponent->BindAxis("Forward", this, &ACar::HandleForwardAxis);

	for (int32 i = 0; i < INVENTORY_SIZE; ++i)
	{
		FString ActionName(FString::Printf(TEXT("UsePerk%i"), i + 1));
		FInputActionBinding AB(*ActionName, IE_Pressed);
		AB.ActionDelegate.GetDelegateForManualSet().BindLambda([this, i]()
		{
			if (PerkToUse == i)
			{
				PerkToUse = -1;
			}
			else
			{
				if (PerkInventory[i] && !PerkInventory[i]->CanConsume())
				{
					return;
				}
				PerkToUse = i;
			}
		});
		PlayerInputComponent->AddActionBinding(AB);
	}
}

void ACar::HandleSteerAxis(float Value)
{
	auto CurrentGameState = Cast<ALD41GameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!CurrentGameState || CurrentGameState->ActiveState != ELD41GameState::Turn)
	{
		return;
	}

	SteerValue = FMath::Clamp(Value, -1.0f, 1.0f);
	UpdateNextPrefab();
}

void ACar::HandleForwardAxis(float Value)
{
	auto CurrentGameState = Cast<ALD41GameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!CurrentGameState || CurrentGameState->ActiveState != ELD41GameState::Turn)
	{
		return;
	}

	ForwardValue = FMath::Clamp(Value, 0.0f, 1.0f);
	UpdateNextPrefab();
}

enum class ERouteDirection
{
	Left,
	Forward,
	Right
};

ERouteDirection GetRouteDirection(FVector ActorLocation, FVector ActorRightVector, FVector RoutePointLocation)
{
	FVector DirectionFromCar = RoutePointLocation - ActorLocation;
	const float DotProd = FVector::DotProduct(ActorRightVector, DirectionFromCar.GetSafeNormal());
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProd));

	const float ERROR_DEGRESS = 5.0f;
	const float RIGHT_ANGLE = 90.0f;

	if (Angle < RIGHT_ANGLE - ERROR_DEGRESS)
	{
		return ERouteDirection::Left;
	}
	else if (Angle > RIGHT_ANGLE + ERROR_DEGRESS)
	{
		return ERouteDirection::Right;
	}
	return ERouteDirection::Forward;
}

FORCEINLINE bool IsMatchingCandidate(float SteerValue, ERouteDirection Direction)
{
	if (SteerValue > 0.0f)
	{
		return Direction == ERouteDirection::Left;
	}
	if (SteerValue < 0.0f)
	{
		return Direction == ERouteDirection::Right;
	}
	return (Direction == ERouteDirection::Forward);
}

void ACar::UpdateCarCollisions(void)
{
	BodyMesh->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(ECC_Car), bForceTargetPosition ? ECollisionResponse::ECR_Ignore : ECollisionResponse::ECR_Block);

	bool bNoCollision = bGhostMode;

	// If there is no ghost mode try if current location is not occupied by any item - if it is force no collision.

	if (!bNoCollision)
	{
		FComponentQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		TArray<FHitResult> HitResult;

		BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		bNoCollision = GetWorld()->ComponentSweepMulti(
			HitResult,
			BodyMesh,
			GetActorLocation(),
			GetActorLocation(),
			GetActorRotation(),
			QueryParams
		);
	}

	BodyMesh->SetCollisionEnabled(bNoCollision ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}

void ACar::UpdateNextPrefab()
{
	if (bForceTargetPosition)
	{
		return;
	}

	ARaceTrackPrefabActor* Prefab = CurrentPrefab->NextPrefab;
	if (!Prefab)
	{
		UE_LOG(LogCar, Error, TEXT("Failed to plan car route. No next prefab defined!"));
		return;
	}

	// If there are more steps to do find further prefab.

	int32 StepsToMake = StepsNextRound;
	while (StepsToMake > 1)
	{
		ARaceTrackPrefabActor* NextPrefab = Prefab->NextPrefab;
		if (NextPrefab)
		{
			Prefab = NextPrefab;
		}
		--StepsToMake;
	}

	FVector Location(GetActorLocation());
	FVector RightVector(GetActorRightVector());
	float ClosestRoutePointDist = FLT_MAX;
	int32 RoutePointsCount = Prefab->GetNumRoutePoints();
	for (int32 i = 0; i < RoutePointsCount; ++i)
	{
		FTransform RoutePoint(Prefab->GetRoutePoint(i));
		const float Distance = FVector::DistSquared2D(Location, RoutePoint.GetLocation());
		if (ClosestRoutePointDist <= Distance)
		{
			continue;
		}

		const ERouteDirection Direction = GetRouteDirection(Location, RightVector, RoutePoint.GetLocation());
		if (!IsMatchingCandidate(SteerValue, Direction))
		{
			continue;
		}

		SetTargetPrefab(Prefab, i);
		ClosestRoutePointDist = Distance;
	}
}

void ACar::Teleport(ACar* Car)
{
	if (!Car || Car->bGhostMode)
	{
		return;
	}

	if (DebugCars.GetValueOnGameThread())
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red,
			FString::Printf(
			TEXT("Teleport %s to %s: Force Pos: %s, Target: %s, Current: %s"),
			*Car->GetName(),
			*GetName(),
			bForceTargetPosition ? TEXT("yes") : TEXT("no"),
			TargetPrefab ? *TargetPrefab->GetName() : TEXT(""),
			CurrentPrefab ? *CurrentPrefab->GetName() : TEXT("")
		));
	}

	// Find the closest point next to this car @a Car can be teleported to.

	int32 ClosestRoutePointIndex = -1;
	float ClosestRoutePointDist = MAX_FLT;
	for (int32 i = 0; i < CurrentPrefab->GetNumRoutePoints(); ++i)
	{
		if (CurrentRoutePoint == i)
		{
			continue;
		}

		const FTransform& RoutePoint = CurrentPrefab->GetRoutePoint(i);
		const float Dist = FVector::DistSquared(GetActorLocation(), RoutePoint.GetLocation());
		if (Dist < ClosestRoutePointDist)
		{
			ClosestRoutePointIndex = i;
			ClosestRoutePointDist = Dist;
		}
	}

	if (ClosestRoutePointIndex == -1)
	{
		UE_LOG(LogCar, Error, TEXT("Failed to find closest route point for teleport on prefab of type %s."), *CurrentPrefab->GetClass()->GetName());
		return;
	}

	Car->TeleportToPrefab(CurrentPrefab, ClosestRoutePointIndex);
}

void ACar::EnableGhostMode()
{
	bGhostMode = true;

	// When ghost mode is being used target position perks have no effect.

	if (bForceTargetPosition)
	{
		bForceTargetPosition = false;
		UpdateNextPrefab();
	}

	UpdateCarCollisions();
}

void ACar::TeleportToPrefab(ARaceTrackPrefabActor* Prefab, int32 RoutePoint)
{
	bForceTargetPosition = true;
	SetTargetPrefab(Prefab, RoutePoint);
}

void ACar::SetTargetPrefab(ARaceTrackPrefabActor* Prefab, int32 RoutePoint)
{
	TargetPrefab = Prefab;
	TargetRoutePoint = RoutePoint;
	TargetTransform = Prefab->GetRoutePoint(RoutePoint);
	UpdateWayCorona();
}

void ACar::UpdateWayCorona()
{
	auto CurrentGameState = Cast<ALD41GameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!CurrentGameState)
	{
		return;
	}

	if (GetController()->IsLocalPlayerController())
	{
		WayCorona->SetVisibility(CurrentGameState->ActiveState == ELD41GameState::Turn && ForwardValue > 0.0f);
		WayCorona->SetWorldTransform(TargetTransform);

		if (ForwardValue > 0.0f)
		{
			WayBlocker->Activate();
		}
		else
		{
			WayBlocker->Deactivate();
		}
	}
}

bool ACar::OverlapTarget(TArray<FOverlapResult> &OverlapResults) const
{
	FComponentQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	return GetWorld()->ComponentOverlapMulti(
		OverlapResults,
		BodyMesh,
		TargetTransform.GetLocation(),
		TargetTransform.GetRotation(),
		QueryParams
		);
}

bool ACar::IsTargetClear() const
{
	FComponentQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	TArray<FHitResult> HitResults;
	return ! GetWorld()->ComponentSweepMulti(
		HitResults,
		BodyMesh,
		GetActorLocation(),
		TargetTransform.GetLocation(),
		TargetTransform.GetRotation(),
		QueryParams
	);
}

bool ACar::IsTargetPerk() const
{
	TArray<FOverlapResult> OverlapResults;
	if (!OverlapTarget(OverlapResults))
	{
		return false;
	}

	for (const FOverlapResult& Overlap : OverlapResults)
	{
		if (Cast<APerkPickupActor>(Overlap.Actor))
		{
			return true;
		}
	}
	return false;
}

bool ACar::AddPerk(TSubclassOf<UPerkBase> PerkClass)
{
	UPerkBase* Perk = NewObject<UPerkBase>(this, PerkClass);
	if (!Perk)
	{
		return false;
	}

	Perk->World = GetWorld();

	// If this perk should be use right on pickup just use it.

	if (Perk->bUseOnPickup)
	{
		Perk->Consume(this);
		return true;
	}

	// Otherwise try to find free slot for it and store it in the inventory,
	// if there is already one perk of the same class in the inventory regenerate it.

	int32 FreeSlot = -1;
	for (int32 i = 0; i < INVENTORY_SIZE; ++i)
	{
		// If there is perk of this same class already in inventory
		// and this kind of perk is regenerating regenerate it.

		if (PerkInventory[i] && PerkInventory[i]->DoesPickupRegenerate() && PerkInventory[i]->IsA(PerkClass))
		{
			const bool Regenerated = (PerkInventory[i]->ActiveCooldown > 0.0f);
			PerkInventory[i]->ActiveCooldown = 0.0f;
			return Regenerated;
		}

		if (FreeSlot == -1 && !PerkInventory[i])
		{
			FreeSlot = i;
		}
	}

	// Go the old way, just add the perk to inventory.

	if (FreeSlot != -1)
	{
		PerkInventory[FreeSlot] = NewObject<UPerkBase>(this, PerkClass);
		PerkInventory[FreeSlot]->World = GetWorld();
		return true;
	}
	return false;
}

void ACar::ConsumePerk(int32 Slot)
{
	UPerkBase* PerkToConsume = PerkInventory[Slot];
	if (PerkToConsume && PerkToConsume->CanConsume())
	{
		const bool CanRemoveAfterUse = PerkToConsume->Uses > 0;
		PerkToConsume->Consume(this);
		if (CanRemoveAfterUse && PerkToConsume->Uses <= 0)
		{
			PerkInventory[Slot] = nullptr;
		}
	}
}

bool ACar::ConsumePerkOfClass(TSubclassOf<UPerkBase> PerkClass)
{
	if (!PerkClass)
	{
		UE_LOG(LogCar, Error, TEXT("Passed NULL PerkClass to ConsumePerkOfClass."));
		return false;
	}

	for (int32 i = 0; i < PerkInventory.Num(); ++i)
	{
		if (PerkInventory[i] && PerkInventory[i]->IsA(PerkClass))
		{
			ConsumePerk(i);
			return true;
		}
	}
	return false;
}

UPerkBase* ACar::GetPerkInSlot(int32 Slot)
{
	if (Slot < PerkInventory.Num())
	{
		return PerkInventory[Slot];
	}
	return nullptr;
}
