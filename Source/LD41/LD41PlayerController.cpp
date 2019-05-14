// Fill out your copyright notice in the Description page of Project Settings.

#include "LD41PlayerController.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

#include "Cars/Car.h"

ALD41PlayerController::ALD41PlayerController()
{
}

ACar* ALD41PlayerController::GetCar() const
{
	return Cast<ACar>(GetPawn());
}
