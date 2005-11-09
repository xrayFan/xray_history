////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_decision_space.h
//	Created 	: 30.03.2004
//  Modified 	: 30.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Stalker decision space
////////////////////////////////////////////////////////////////////////////

#pragma once

namespace StalkerDecisionSpace {
	enum EWorldProperties {
		eWorldPropertyAlive			= u32(0),
		eWorldPropertyDead,
		eWorldPropertyAlreadyDead,
		
		eWorldPropertyALife,
		eWorldPropertyPuzzleSolved,
		eWorldPropertyReachedTaskLocation,
		eWorldPropertyTaskCompleted,
		eWorldPropertyReachedCustomerLocation,
		eWorldPropertyCustomerSatisfied,

		eWorldPropertyItems,
		eWorldPropertyEnemy,
		eWorldPropertyDanger,
		
		eWorldPropertyItemToKill,
		eWorldPropertyFoundItemToKill,
		eWorldPropertyItemCanKill,
		eWorldPropertyFoundAmmo,
		eWorldPropertyReadyToKill,
		eWorldPropertySeeEnemy,
		eWorldPropertyPanic,
		eWorldPropertyInCover,
		eWorldPropertyLookedOut,
		eWorldPropertyPositionHolded,
		eWorldPropertyEnemyDetoured,
		eWorldPropertyPureEnemy,

		eWorldPropertyDangerUnknown,
		eWorldPropertyDangerInDirection,
		eWorldPropertyDangerGrenade,
		eWorldPropertyDangerBySound,

		eWorldPropertyCoverActual,
		eWorldPropertyCoverReached,
		eWorldPropertyLookedAround,
		eWorldPropertyGrenadeExploded,

		eWorldPropertyAnomaly,
		eWorldPropertyInsideAnomaly,

		eWorldPropertyScript,
		eWorldPropertyDummy			= u32(-1),
	};

	enum EWorldOperators {
// death
		eWorldOperatorDead	= u32(0),
		eWorldOperatorDying,
		
// alife
		eWorldOperatorGatherItems,
		eWorldOperatorALifeEmulation,

// alife : tasks
		eWorldOperatorSolveZonePuzzle,
		eWorldOperatorReachTaskLocation,
		eWorldOperatorAccomplishTask,
		eWorldOperatorReachCustomerLocation,
		eWorldOperatorCommunicateWithCustomer,

// anomaly
		eWorldOperatorGetOutOfAnomaly,
		eWorldOperatorDetectAnomaly,

// combat
		eWorldOperatorGetItemToKill,
		eWorldOperatorFindItemToKill,
		eWorldOperatorMakeItemKilling,
		eWorldOperatorFindAmmo,

		eWorldOperatorAimEnemy,
		eWorldOperatorGetReadyToKill,
		eWorldOperatorKillEnemy,
		eWorldOperatorRetreatFromEnemy,
		eWorldOperatorTakeCover,
		eWorldOperatorLookOut,
		eWorldOperatorHoldPosition,
		eWorldOperatorGetDistance,
		eWorldOperatorDetourEnemy,
		eWorldOperatorSearchEnemy,
		eWorldOperatorHideFromGrenade,
		eWorldOperatorPostCombatWait,

// danger
		eWorldOperatorDangerUnknownPlanner,
		eWorldOperatorDangerInDirectionPlanner,
		eWorldOperatorDangerGrenadePlanner,
		eWorldOperatorDangerBySoundPlanner,

		eWorldOperatorDangerUnknownTakeCover,
		eWorldOperatorDangerUnknownLookAround,
		eWorldOperatorDangerUnknownSearchEnemy,

		eWorldOperatorDangerInDirectionTakeCover,
		eWorldOperatorDangerInDirectionLookOut,
		eWorldOperatorDangerInDirectionLookAround,
		eWorldOperatorDangerInDirectionSearchEnemy,

		eWorldOperatorDangerGrenadeTakeCover,
		eWorldOperatorDangerGrenadeWaitForExplosion,
		eWorldOperatorDangerGrenadeTakeCoverAfterExplosion,
		eWorldOperatorDangerGrenadeLookAround,
		eWorldOperatorDangerGrenadeSearch,

// global
		eWorldOperatorDeathPlanner,
		eWorldOperatorALifePlanner,
		eWorldOperatorCombatPlanner,
		eWorldOperatorAnomalyPlanner,
		eWorldOperatorDangerPlanner,

// script
		eWorldOperatorScript,
		eWorldOperatorDummy			= u32(-1),
	};

	enum ESightActionType {
		eSightActionTypeWatchItem	= u32(0),
		eSightActionTypeWatchEnemy,
		eSightActionTypeDummy		= u32(-1),
	};
};