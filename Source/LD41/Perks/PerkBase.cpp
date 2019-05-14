// Copyright (C) 2018 by Eryk Dwornicki All Rights Reserved

#include "PerkBase.h"

UPerkBase::UPerkBase()
{
}

UPerkBase::~UPerkBase()
{
}

UWorld* UPerkBase::GetWorld() const
{
	return World;
}

bool UPerkBase::CanConsume() const
{
	return ActiveCooldown <= 0;
}

void UPerkBase::Consume_Implementation(ACar* Car)
{
	ActiveCooldown = CooldownLength;
	if (Uses != -1)
	{
		--Uses;
	}
}

void UPerkBase::OnNewTurn()
{
	if (ActiveCooldown > 0)
	{
		--ActiveCooldown;
	}
}

bool UPerkBase::DoesPickupRegenerate() const
{
	return CooldownLength > 0;
}
