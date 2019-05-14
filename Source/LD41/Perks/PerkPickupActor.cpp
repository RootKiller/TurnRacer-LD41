// Fill out your copyright notice in the Description page of Project Settings.

#include "PerkPickupActor.h"

#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

#include "Cars/Car.h"

#include "LD41PlayerController.h"


// Sets default values
APerkPickupActor::APerkPickupActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetupAttachment(RootComponent);

	Trigger = CreateDefaultSubobject<USphereComponent>("Trigger");
	Trigger->SetSphereRadius(300.0f);
	Trigger->SetupAttachment(RootComponent);
	Trigger->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
	Trigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &APerkPickupActor::OnBeginOverlap);
}

void APerkPickupActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	auto Car = Cast<ACar>(OtherActor);
	if (Car && Car->AddPerk(PerkClass))
	{
		Pickup();
	}
}

void APerkPickupActor::Pickup_Implementation()
{
	Destroy();
}

// Called when the game starts or when spawned
void APerkPickupActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APerkPickupActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh)
	{
		const float ROTATION_SPEED = 50.0f;
		PickupMesh->AddLocalRotation(FRotator(0.0f, DeltaTime * ROTATION_SPEED, 0.0f));

		PickupMesh->AddLocalOffset(FVector(0.0f, 0.0f, FMath::Sin(GetWorld()->GetTimeSeconds()) * 0.5f));
	}
}

