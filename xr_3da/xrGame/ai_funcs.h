////////////////////////////////////////////////////////////////////////////
//	Module 		: ai_funcs.h
//	Created 	: 25.03.2002
//  Modified 	: 11.10.2002
//	Author		: Dmitriy Iassenev
//	Description : Pattern function class
////////////////////////////////////////////////////////////////////////////

#pragma once

#define AI_MAX_EVALUATION_FUNCTION_COUNT	128

#include "ai_base_funcs.h"

class CDistanceFunction;
class CGraphPointType0;
class CPersonalHealthFunction;
class CPersonalMoraleFunction;
class CPersonalCreatureTypeFunction;
class CPersonalWeaponTypeFunction;
class CPersonalAccuracyFunction;
class CPersonalIntelligenceFunction;
class CPersonalRelationFunction;
class CPersonalGreedFunction;
class CPersonalAggressivenessFunction;
class CEnemyHealthFunction;
class CEnemyCreatureTypeFunction;
class CEnemyWeaponTypeFunction;
class CEnemyEquipmentCostFunction;
class CEnemyRukzakWeightFunction;
class CEnemyAnomalityFunction;
class CEyeRange;
class CMaxMonsterHealth;

class CAI_DDD;

class CPatternFunction : public CBaseFunction {
	
private:
	typedef CBaseFunction inherited;
	enum {EFC_VERSION	= 1};
	typedef struct tagSEFHeader {
		u32			dwBuilderVersion;
		u32			dwDataFormat;
	} SEFHeader;

	typedef struct tagSPattern {
		u32			dwCardinality;
		u32			*dwaVariableIndexes;
	} SPattern;

	u32				*m_dwaAtomicFeatureRange;
	u32				*m_dwaPatternIndexes;
	SPattern		*m_tpPatterns;
	float			*m_faParameters;
	u32				m_dwPatternCount;
	u32				m_dwParameterCount;
	SEFHeader		m_tEFHeader;

	IC u32 dwfGetPatternIndex(u32 *dwpTest, int iPatternIndex)
	{
		SPattern &tPattern = m_tpPatterns[iPatternIndex];
		for (u32 i=1, dwIndex = dwpTest[tPattern.dwaVariableIndexes[0]]; i<(int)tPattern.dwCardinality; i++)
			dwIndex = dwIndex*m_dwaAtomicFeatureRange[tPattern.dwaVariableIndexes[i]] + dwpTest[tPattern.dwaVariableIndexes[i]];
		return(dwIndex + m_dwaPatternIndexes[iPatternIndex]);
	}
	
	float			ffEvaluate();

public:
	
	u32				m_dwVariableCount;
	u32				m_dwFunctionType;
	u32				*m_dwaVariableTypes;
	u32				*m_dwaVariableValues;

					CPatternFunction();
					CPatternFunction(LPCSTR caEFFileName, CAI_DDD *tpAI_DDD);
	virtual			~CPatternFunction();

	virtual	void	vfLoadEF(LPCSTR caEFFileName, CAI_DDD *tpAI_DDD);
	virtual float	ffGetValue();
};

class CAI_DDD {

public:
	CEntityAlive							*m_tpCurrentMember;
	CEntityAlive							*m_tpCurrentEnemy;
	CSE_ALifeObject							*m_tpCurrentALifeObject;
	CSE_ALifeMonsterAbstract				*m_tpCurrentALifeMember;
	CSE_ALifeMonsterAbstract				*m_tpCurrentALifeEnemy;
	// primary functions
	CBaseFunction							*m_fpaBaseFunctions		[AI_MAX_EVALUATION_FUNCTION_COUNT];

	CDistanceFunction						*m_pfDistance;
	CGraphPointType0						*m_pfGraphPointType0;

	CPersonalHealthFunction					*m_pfPersonalHealth;
	CPersonalMoraleFunction					*m_pfPersonalMorale;
	CPersonalCreatureTypeFunction			*m_pfPersonalCreatureType;
	CPersonalWeaponTypeFunction				*m_pfPersonalWeaponType;
	CPersonalAccuracyFunction				*m_pfPersonalAccuracy;
	CPersonalIntelligenceFunction			*m_pfPersonalIntelligence;
	CPersonalRelationFunction				*m_pfPersonalRelation;
	CPersonalGreedFunction					*m_pfPersonalGreed;
	CPersonalAggressivenessFunction			*m_pfPersonalAggressiveness;

	CEnemyHealthFunction					*m_pfEnemyHealth;
	CEnemyCreatureTypeFunction				*m_pfEnemyCreatureType;
	CEnemyWeaponTypeFunction				*m_pfEnemyWeaponType;
	CEnemyEquipmentCostFunction				*m_pfEnemyEquipmentCost;
	CEnemyRukzakWeightFunction				*m_pfEnemyRukzakWeight;
	CEnemyAnomalityFunction					*m_pfEnemyAnomality;

	// complex functions
	CPatternFunction						*m_pfWeaponEffectiveness;
	CPatternFunction						*m_pfCreatureEffectiveness;
	CPatternFunction						*m_pfIntellectCreatureEffectiveness;
	CPatternFunction						*m_pfAccuracyWeaponEffectiveness;
	CPatternFunction						*m_pfFinalCreatureEffectiveness;
	CPatternFunction						*m_pfVictoryProbability;
	CPatternFunction						*m_pfEntityCost;
	CPatternFunction						*m_pfExpediency;
	CPatternFunction						*m_pfSurgeDeathProbability;
	CPatternFunction						*m_pfTerrainType;
	CPatternFunction						*m_pfNoticability;
	CPatternFunction						*m_pfNoticeProbability;

											CAI_DDD();
	virtual									~CAI_DDD();
};