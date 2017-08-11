// Copyright 2014-2015 Matthew Chapman, Inc. All Rights Reserved.

#pragma once

#include "BCMovementPhysicsComponent.h"
#include "BCOrbitalMovementComponent.generated.h"


/**
 * A base movement class describing orbital movement for the Barrage physics engine.
 */
UCLASS(Abstract, Category="Barrage|Component|Physics", BlueprintType)
class UBCOrbitalMovementComponent : public UBCMovementPhysicsComponent
{
	GENERATED_BODY()
	BC_CLASS_HEADER( UBCOrbitalMovementComponent )

	UBCOrbitalMovementComponent( const class FObjectInitializer& oObjectInitializer );

	friend class ABCWorldSettings;

public:

	/* Returns the actor representing the centre of the orbit. */
	UFUNCTION(BlueprintCallable, Category="Barrage|Component|Physics|Orbital Movement")
	virtual AActor* GetCentreOfOrbit() const;
		
	/* Returns the start time offset, which is added on to the current world time. */
	UFUNCTION(BlueprintCallable, Category="Barrage|Component|Physics|Orbital Movement")
	virtual float GetStartTimeOffset() const;

	UFUNCTION( BlueprintCallable, Category = "Barrage|Component|Physics|Orbital Movement" )
	virtual void SetStartTimeOffset( float fOffset );

	/* Does the magic (moves the attached actor.) */
	virtual void TickComponent( float fDeltaTime, enum ELevelTick eTickType, FActorComponentTickFunction* stThisTickFunction ) override;

	/* Calculates the location of the attached actor this tick. */
	UFUNCTION( BlueprintCallable, Category = "Barrage|Component|Physics|Orbital Movement" )
	virtual FVector GetUpdatedLocation();

	/* Calculates the location of the orbit as it would be on the server with the given world time. */
	UFUNCTION( BlueprintCallable, Category = "Barrage|Component|Physics|Orbital Movement" )
	virtual FVector GetLocationAtServerTime( float fWorldTime );

	/* Calculates the position at the given time. */
	UFUNCTION( BlueprintCallable, Category = "Barrage|Component|Physics|Orbital Movement" )
	virtual FVector GetLocationAtTime( float fTime );

	UFUNCTION( BlueprintCallable, Category = "Barrage|Component|Physics|Orbital Movement" )
	virtual float GetPeriod() const;

	virtual void InitialiseOrbitalVars();

protected:

	/* Added to the current world time when calculating position. */
	UPROPERTY(EditInstanceOnly, Replicated, Category="Barrage|Component|Physics|Orbital Movement", Meta = (DisplayName="Start Time Offset"))
	float fStartTimeOffset;	
	
	/* The actor representing the central, dominant gravity well position */
	UPROPERTY( EditInstanceOnly, Replicated, Category = "Barrage|Component|Physics|Orbital Movement", Meta = ( DisplayName = "Centre Of Orbit" ) )
	AActor* aCentreOfOrbit;

#if WITH_EDITOR
public:
#endif

	/* Returns the actor representing the centre of the orbit. */
	UFUNCTION(BlueprintCallable, Category="Barrage|Component|Physics|Orbital Movement")
	virtual void SetCentreOfOrbit( AActor* aNewCentre );

};
