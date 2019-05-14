// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerkPickupActor.generated.h"

class UPerkBase;
class USphereComponent;

UCLASS()
class LD41_API APerkPickupActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APerkPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, Category="Perk")
	void Pickup();


public:

	UPROPERTY(VisibleDefaultsOnly, Category="Perk")
	UStaticMeshComponent* PickupMesh;

	UPROPERTY(VisibleDefaultsOnly, Category="Perk")
	USphereComponent* Trigger;

	UPROPERTY(EditDefaultsOnly, Category="Perk")
	TSubclassOf<UPerkBase> PerkClass;
};
