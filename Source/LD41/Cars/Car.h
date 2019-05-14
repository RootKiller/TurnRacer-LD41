// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LD41TurnListener.h"
#include "Car.generated.h"

class APerkPickupActor;
class ARaceTrackPrefabActor;
class UPerkBase;
class USphereComponent;
class ILD41TurnListener;

UCLASS()
class LD41_API ACar : public APawn, public ILD41TurnListener
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

	virtual void Tick(float DeltaTime) override;

protected:
	void HandleSteerAxis(float Value);
	void HandleForwardAxis(float Value);

	void UpdateWayCorona();

	UFUNCTION(BlueprintNativeEvent, Category="Car")
	void HandleCrash(const FHitResult &Hit);

	bool IsOutOfFuel() const;

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Turn Listener")
	void OnNewTurn(int32 TurnNumber);

	UFUNCTION(BlueprintNativeEvent, Category = "Turn Listener")
	void OnTurnFinish(int32 TurnNumber);

	UFUNCTION(BlueprintNativeEvent, Category = "Turn Listener")
	void HandleTurnTransition(float Progress);

	UFUNCTION(BlueprintCallable, Category="Car")
	void Refuel(int32 Amount = 100);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadOnly, Category="Car")
	float SteerValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Car")
	float ForwardValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Car")
	int32 Fuel = 100;

	float LastProgress = 0.0f;

	bool bCrashed = false;

	UPROPERTY()
	ARaceTrackPrefabActor* CurrentPrefab = nullptr;

	FTransform	PreviousTransform;

	/** Is the next target position forced or decided by controller? */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay")
	bool		bForceTargetPosition = false;

	/** The target position where car will be moved to after this turn. */
	FTransform	TargetTransform;

	/** The route point where car will be moved to after this turn. */
	int32		TargetRoutePoint = 0;

	/** The prefab where car will be moved to after this turn. */
	UPROPERTY()
	ARaceTrackPrefabActor* TargetPrefab = nullptr;

	/** The ghost mode state. */
	UPROPERTY(BlueprintReadOnly, Category="Gameplay")
	bool		bGhostMode = false;

	int32		CurrentRoutePoint = 0;

	UPROPERTY(BlueprintReadWrite, Category="Gameplay")
	int32		StepsNextRound = 1;

	UFUNCTION(BlueprintCallable, Category="Game Mode")
	void Teleport(ACar* Car);

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void EnableGhostMode();
	void DisableGhostMode();

	void UpdateNextPrefab();

	void TeleportToPrefab(ARaceTrackPrefabActor* Prefab, int32 RoutePoint);
	void SetTargetPrefab(ARaceTrackPrefabActor* Prefab, int32 RoutePoint);

	void UpdateCarCollisions(void);

	bool OverlapTarget(TArray<FOverlapResult> &OverlapResults) const;

	bool IsTargetClear() const;
	bool IsTargetPerk() const;

	bool IsMoving() const;

	UPROPERTY(VisibleDefaultsOnly, Category="Visual")
	UStaticMeshComponent *BodyMesh;

	UPROPERTY(VisibleDefaultsOnly, Category = "Visual")
	UStaticMeshComponent *WayCorona;

	UPROPERTY(VisibleDefaultsOnly, Category="Gameplay")
	USphereComponent* WayBlocker;

private:

	UPROPERTY()
	TArray<UPerkBase*> PerkInventory;

public:

	UFUNCTION(BlueprintCallable, Category = "Perks")
	bool AddPerk(TSubclassOf<UPerkBase> PerkClass);

	void ConsumePerk(int32 Slot);

	UPROPERTY(BlueprintReadOnly, Category="Perks")
	int32 PerkToUse = -1;

	UFUNCTION(BlueprintCallable, Category = "Perks")
	bool ConsumePerkOfClass(TSubclassOf<UPerkBase> PerkClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Perks")
	UPerkBase* GetPerkInSlot(int32 Slot);
};
