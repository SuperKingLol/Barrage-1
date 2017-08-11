// Copyright 2014-2015 Matthew Chapman, Inc. All Rights Reserved.

#pragma once

#include "BCOrbitalMovementComponent.h"
#include "BCKeplerOrbitMovementComponent.generated.h"

#define BC_KEPLER_MU_FACTOR 1000000.f
#define BC_KEPLER_EPSILON 0.00001f
#define BC_KEPLER_ITERATION_LIMIT 20

/**
* A movement component that describes an elliptical orbit
*/
UCLASS( ClassGroup = "Barrage|Component|Physics", EditInlineNew, Blueprintable, BlueprintType, Meta = ( BlueprintSpawnableComponent ) )
class UBCKeplerOrbitMovementComponent : public UBCOrbitalMovementComponent
{
	GENERATED_BODY()
	BC_CLASS_HEADER( UBCKeplerOrbitMovementComponent )

	UBCKeplerOrbitMovementComponent( const class FObjectInitializer& oObjectInitializer );

public:

	virtual float GetPeriod() const override;

	virtual void InitialiseOrbitalVars() override;

	virtual void InitializeComponent() override;

protected:

	/* The eccentricity (ellipticalness) of the orbit. */
	UPROPERTY( EditAnywhere, Replicated, BlueprintReadWrite, Category = "Barrage|Component|Physics|Kepler Orbital Movement", Meta = ( DisplayName = "Eccentricity (0=circular)" ) )
	float fEccentricity;

	/* Right ascension of the ascending node. */
	UPROPERTY( EditAnywhere, Replicated, BlueprintReadWrite, Category = "Barrage|Component|Physics|Kepler Orbital Movement", Meta = ( DisplayName = "Right Ascension" ) )
	float fRightAscension;

	/* Argument of perifocal point. */
	UPROPERTY( EditAnywhere, Replicated, BlueprintReadWrite, Category = "Barrage|Component|Physics|Kepler Orbital Movement", Meta = ( DisplayName = "Argument of Perifocal Point" ) )
	float fArgPerifocalPoint;

	/* If true, goes the other way around the central gravity well. */
	UPROPERTY( EditAnywhere, Replicated, BlueprintReadWrite, Category = "Barrage|Component|Physics|Kepler Orbital Movement", Meta = ( DisplayName = "Reverse Orbit" ) )
	bool bReverse;

	/* Use this if your centre of orbit doesn't have mass. */
	UPROPERTY( EditAnywhere, Replicated, BlueprintReadWrite, Category = "Barrage|Component|Physics|Kepler Orbital Movement", Meta = ( DisplayName = "Fake Gravity" ) )
	float fFakeGravity123;

	virtual FVector GetLocationAtTime( float fTime ) override;

private:

	/* If the init was successful - Cannot tick without. */
	UPROPERTY( Replicated )
	bool bInitSuccess;

	UPROPERTY( Replicated )
	float fMeanMotion;

	UPROPERTY( Replicated )
	float fSemilatusRectum;

	UPROPERTY( Replicated )
	float fPeriod;

	UPROPERTY( Replicated )
	float fInitialTime;

	UPROPERTY( Replicated )
	float fInclination;

	UPROPERTY( Replicated )
	float fStartYaw;

};
