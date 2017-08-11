// Copyright 2014-2015 Matthew Chapman, Inc. All Rights Reserved.

#include "Barrage.h"
#include "Actor/BCPhysicsActor.h"
#include "Game/Play/BCPlayGameState.h"
#include "Runtime/Engine/Public/Net/UnrealNetwork.h"
#include "Component/Physics/BCKeplerOrbitMovementComponent.h"


UBCKeplerOrbitMovementComponent::UBCKeplerOrbitMovementComponent( const class FObjectInitializer& oObjectInitializer )
: Super( oObjectInitializer )
{
	bWantsInitializeComponent = true;
	bInitSuccess = false;
	bReverse = false;
}


void UBCKeplerOrbitMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	InitialiseOrbitalVars();
}


void UBCKeplerOrbitMovementComponent::InitialiseOrbitalVars()
{
	Super::InitialiseOrbitalVars();

	bInitSuccess = false;

	ABCPhysicsActor* const aOwner = Cast<ABCPhysicsActor>( GetOwner() );

	if ( aOwner == NULL )
	{
		BCErrorUO( "Null owner" );
		return;
	}

	if ( GetCentreOfOrbit() == NULL )
	{
		BCErrorUO( "Null centre of gravity" );
		return;
	}

	float fMu = fFakeGravity123;

	ABCPhysicsActor* const aCentre = Cast<ABCPhysicsActor>( GetCentreOfOrbit() );

	if ( aCentre != NULL )
		fMu = aCentre->GetActorMass() * BC_KEPLER_MU_FACTOR;

	else
		fMu = fFakeGravity123 * BC_KEPLER_MU_FACTOR;

	if ( fMu <= 0.f )
	{
		BCErrorUO( "No gravity" );
		return;
	}

	FVector vOffset = aOwner->GetActorLocation() - GetCentreOfOrbit()->GetActorLocation();

	// Find semimajor axis of orbit. Centre of orbit is at one of the foci and the orbiter position represents apsis
	float fSemiMajorAxis = vOffset.Size() / ( 1.f - fEccentricity );

	// Calculate Semilatus Rectum
	fSemilatusRectum = fSemiMajorAxis * ( 1 - sqrd( fEccentricity ) );

	// Calculate mean motion
	fMeanMotion = sqrt( fMu / pow( fSemiMajorAxis, 3.f ) );

	// Calculate orbital period
	fPeriod = ( 2.f * PI ) / fMeanMotion;

	FRotator rAngle = vOffset.Rotation();
	fInclination = rAngle.Pitch / -57.079933523486824571189522950059;
	fStartYaw = rAngle.Yaw;

	float fTrueAnom = 0.f;
	float fTrueAnomSine = sin( fTrueAnom );
	float fTrueAnomCosine = cos( fTrueAnom );

	// Find eccentric anomaly of starting position
	float fEccAnomSine = ( sqrt( 1 - sqrd( fEccentricity ) ) * fTrueAnomSine ) / ( 1.f + ( fEccentricity * fTrueAnomCosine ) );
	float fEccAnomCosine = ( fEccentricity + fTrueAnomCosine ) / ( 1.f + ( fEccentricity * fTrueAnomCosine ) );
	float fEccAnom = atan2( fEccAnomSine, fEccAnomCosine );

	// Calculate mean anomaly of starting position from Kepler's equation
	float fMeanAnom = fEccAnom - ( fEccentricity * fEccAnomSine );

	// Initialise mean anomaly and time elapsed since periapsis
	fInitialTime = fMeanAnom / fMeanMotion;

	bInitSuccess = true;
}


FVector UBCKeplerOrbitMovementComponent::GetLocationAtTime( float fTime )
{
	if ( !bInitSuccess )
	{
		BCErrorUO( "Failed to init" );
		return GetLocation();
	}

	float fDeltaEi = 0;
	float fTimeSinceApsis = mod( fInitialTime + ( fTime * ( bReverse ? -1.f : 1.f ) ), fPeriod );
	float fMeanAnom = ( fMeanMotion * fTimeSinceApsis );
	float fEccAnom = fMeanAnom;

	// Newton-Raphson Iteration
	for ( int8 i = 0; i < BC_KEPLER_ITERATION_LIMIT; ++i )
	{
		// Calculate iteration error
		fDeltaEi = -( ( fEccAnom - ( fEccentricity * sin( fEccAnom ) ) - fMeanAnom ) / ( 1.f - ( fEccentricity * cos( fEccAnom ) ) ) );

		if ( abs( fDeltaEi ) <= BC_KEPLER_EPSILON )
			break; // Leave loop if the error is suitably small

		else if ( i == ( BC_KEPLER_ITERATION_LIMIT - 1 ) )
			fEccAnom = fMeanAnom; // If iteration fails to converge, revert to equivalent position on circular orbit.

		else
			fEccAnom += fDeltaEi;
	}

	float fEccAnomSine = sin( fEccAnom );
	float fEccAnomCosine = cos( fEccAnom );

	float fTrueAnomSine = ( sqrt( 1.f - sqrd( fEccentricity ) ) * fEccAnomSine ) / ( 1.f - ( fEccentricity * fEccAnomCosine ) );
	float fTrueAnomCosine = ( fEccAnomCosine - fEccentricity ) / ( 1.f - ( fEccentricity * fEccAnomCosine ) );

	float fRadius = fSemilatusRectum / ( 1.f + ( fEccentricity * fTrueAnomCosine ) );
	fRadius /= ( 1.f - fEccentricity );

	float fRaSine = sin( fRightAscension );
	float fRaCosine = cos( fRightAscension );

	float fOmegaSine = sin( fArgPerifocalPoint );
	float fOmegaCosine = cos( fArgPerifocalPoint );

	float fIncSine = sin( fInclination );
	float fIncCosine = cos( fInclination );

	// Rotation matrix elements
	float fR11 = ( fRaCosine * fOmegaCosine ) - ( fRaSine * fOmegaSine * fIncCosine );
	float fR12 = ( ( -fRaCosine ) * fOmegaSine ) - ( fRaSine * fOmegaCosine * fIncCosine );
	float fR21 = ( fRaSine * fOmegaCosine ) + ( fRaCosine * fOmegaSine * fIncCosine );
	float fR22 = ( ( -fRaSine ) * fOmegaSine ) + ( fRaCosine * fOmegaCosine * fIncCosine );
	float fR31 = fOmegaSine * fIncSine;
	float fR32 = fOmegaCosine * fIncSine;

	float fOrb11 = -fRadius * ( -fTrueAnomSine );
	float fOrb21 = fRadius * ( fEccentricity + ( -fTrueAnomCosine ) );

	FVector vCentreLocation = GetCentreOfOrbit()->GetActorLocation();

	FVector vNewLocation = FVector(
		( -fR21 * fOrb11 ) + ( -fR22 * fOrb21 ) + vCentreLocation.X,
		( fR11 * fOrb11 ) + ( fR12 * fOrb21 ) + vCentreLocation.Y,
		( fR31 * fOrb11 ) + ( fR32 * fOrb21 ) + vCentreLocation.Z
	);

	return FRotator( 0.f, fStartYaw, 0.f ).RotateVector( vNewLocation );
}


float UBCKeplerOrbitMovementComponent::GetPeriod() const
{
	if ( !bInitSuccess )
	{
		UBCKeplerOrbitMovementComponent* cNonConst = const_cast< UBCKeplerOrbitMovementComponent* >( this );
		cNonConst->InitialiseOrbitalVars();

		if ( !cNonConst->bInitSuccess )
			return 0.f;
		
		return cNonConst->fPeriod;
	}

	return fPeriod;
}


void UBCKeplerOrbitMovementComponent::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fEccentricity, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fRightAscension, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fInclination, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fArgPerifocalPoint, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, bInitSuccess, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fMeanMotion, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fSemilatusRectum, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fPeriod, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fInitialTime, COND_InitialOnly );
	DOREPLIFETIME_CONDITION( UBCKeplerOrbitMovementComponent, fStartYaw, COND_InitialOnly );
}
