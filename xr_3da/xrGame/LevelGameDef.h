//---------------------------------------------------------------------------
#ifndef LevelGameDefH
#define LevelGameDefH

#define RPOINT_CHOOSE_NAME 	"$rpoint"
#define AIPOINT_CHOOSE_NAME	"$aipoint"

enum EPointType{
    ptRPoint=0,
    ptAIPoint,
    ptMaxType,
    pt_force_dword=DWORD(-1)
};

enum EWayType{
    wtPatrolPath=0,
    wtJumpPoint,
    wtTraffic,
    wtCustom,
    wtMaxType,
    wt_force_dword=DWORD(-1)
};

// BASE offset
#define WAY_TYPE					0x10000000
#define POINT_TYPE					0x20000000

// POINT chunks
#define RPOINT_CHUNK				POINT_BASE+ptRPoint
#define AIPOINT_CHUNK				POINT_BASE+ptAIPoint

// WAY chunks
#define WAY_PATH_CHUNK				WAY_BASE+wtPatrolPath
#define WAY_JUMP_CHUNK				WAY_BASE+wtJumpPoint
#define WAY_TRAFFIC_CHUNK			WAY_BASE+wtTraffic
#define WAY_CUSTOM_CHUNK			WAY_BASE+wtCustom
//----------------------------------------------------

#define WAYOBJECT_VERSION			0x0012
//----------------------------------------------------
#define WAYOBJECT_CHUNK_VERSION		0x0001
#define WAYOBJECT_CHUNK_POINTS		0x0002
#define WAYOBJECT_CHUNK_LINKS		0x0003
#define WAYOBJECT_CHUNK_TYPE		0x0004
#define WAYOBJECT_CHUNK_NAME		0x0005
#define WAYOBJECT_CHUNK_TYPE		0x0006

//----------------------------------------------------
/*
- chunk RPOINT_CHUNK
	- chunk #0
        vector	(PPosition);
        vector	(PRotation);
        dword	(m_dwTeamId);
    ...
    - chunk #n
    	
- chunk AIPOINT_CHUNK
	- chunk #0
        vector	(PPosition);
    ...
    - chunk #n

- chunk WAY_PATH_CHUNK
	- chunk #0
    	chunk WAYOBJECT_CHUNK_VERSION
        	word (version)
        chunk WAYOBJECT_CHUNK_TYPE
        	dword (type)
		chunk WAYOBJECT_CHUNK_NAME
        	stringZ (Name)
        chunk WAY_CHUNK_TYPE
        	dword EWayType (type)
        chunk WAY_CHUNK_POINTS
            word (count)
            for (i=0; i<count; i++){
            	Fvector (pos)
                dword	(flags)
            }
        chunk WAY_CHUNK_LINKS
            word (count)
            for (i=0; i<count; i++){
            	word 	(from)
				word 	(to)
            }
    ...
    - chunk #n
- chunk WAY_JUMP_CHUNK
	-//-
- chunk WAY_TRAFFIC_CHUNK
	-//-
- chunk WAY_CUSTOM_CHUNK
	-//-
*/
//---------------------------------------------------------------------------
#endif //LevelGameDefH
 