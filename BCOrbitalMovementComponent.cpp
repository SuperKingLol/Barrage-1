// Copyright 2014-2015 Matthew Chapman, Inc. All Rights Reserved.

#include "Barrage.h"
#include "Actor/Space/BCStandardPlanet.h"
#include "Runtime/Engine/Public/Net/UnrealNetwork.h"
#include "Component/Physics/BCOrbitalMovementComponent.h"


UBCOrbitalMovementComponent::UBCOrbitalMovementComponent( const class FObjectInitializer& oObjectInitializer )
	: Super(oObjectInitializer)
{
	aCentreOfOrbit = NULL;
	
	bWantsInitializeComponent = true;
}


void UBCOrbitalMovementComponent::InitialiseOrbitalVars()
{

}


AActor* UBCOrbitalMovementComponent::GetCentreOfOrbit() const
{
	return aCentreOfOrbit;
	//return UBCDefs::ValidOnly<AActor>( aCentreOfOrbit );
}


float UBCOrbitalMovementComponent::GetStartTimeOffset() const
{
	return fStartTimeOffset;
}


void UBCOrbitalMovementComponent::SetStartTimeOffset( float fOffset )
{
	fStartTimeOffset = fOffset;
}


void UBCOrbitalMovementComponent::SetCentreOfOrbit( AActor* aNewCentre )
{
	if ( GetOwner() != NULL && !HasServerCW( GetOwner()->GetWorld() ) )
	{
		BCWarnUO( "Network client trying to set centre of orbit" );
		return;
	}

	aCentreOfOrbit = aNewCentre;
}


FVector UBCOrbitalMovementComponent::GetUpdatedLocation()
{
	if ( GetWorld() == NULL )
		return GetLocation();

	return GetLocationAtServerTime( GetWorld()->TimeSeconds );
}


FVector UBCOrbitalMovementComponent::GetLocationAtServerTime( float fWorldTime )
{
	if ( !IsPersistentMover() )
	{
		AGameState* aGameState = GetWorld()->GameState;

		if ( aGameState == NULL || !aGameState->HasMatchStarted() )
			fWorldTime = 0.f;

		else if ( aGameState->HasMatchEnded() )
			return GetLocation();

		// Ping adjusted start time (ping/2 taken from start time.)
		else if ( fMatchStartTime != BC_COMPONENT_PHYSICS_NO_START_TIME )
			fWorldTime -= fMatchStartTime;
	}

	fWorldTime += GetStartTimeOffset();

	return GetLocationAtTime( fWorldTime );
}


FVector UBCOrbitalMovementComponent::GetLocationAtTime( float fTime )
{
	return GetLocation();
}


float UBCOrbitalMovementComponent::GetPeriod() const
{
	return 0.f;
}


void UBCOrbitalMovementComponent::TickComponent( float fDeltaTime, enum ELevelTick eTickType, FActorComponentTickFunction* stThisTickFunction )
{
	Super::TickComponent( fDeltaTime, eTickType, stThisTickFunction );

	if ( !bIsActive )
		return;

	AActor* aCentre = GetCentreOfOrbit();

	if ( aCentre == NULL )
	{
		BCErrorUO( "%s: Null centre of orbit", *( GetOwner() ? GetOwner()->GetName() : FString( "No Owner" ) ) );
		return;
	}

	FVector vStartLocation = GetLocation();
	FVector vUpdatedLocation = GetUpdatedLocation();
	FVector vDeltaLocation = vUpdatedLocation - vStartLocation;

	FVector vStartVelocity = GetVelocity();
	FVector vUpdatedVelocity = vDeltaLocation / fDeltaTime;
	FVector vDeltaVelocity = vUpdatedVelocity - vStartVelocity;

	FVector vAcceleration = vDeltaVelocity / fDeltaTime;

	SetAcceleration( vAcceleration );
	SetVelocity( vUpdatedVelocity );
	SetLocation( vUpdatedLocation );
}


void UBCOrbitalMovementComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	
	DOREPLIFETIME_CONDITION( UBCOrbitalMovementComponent, aCentreOfOrbit, COND_InitialOnly );
	DOREPLIFETIME( UBCOrbitalMovementComponent, fStartTimeOffset );
}
