#pragma once

#ifndef _DETAIL_FORMAT_H_
#define _DETAIL_FORMAT_H_
#pragma pack(push,1)

#define DETAIL_VERSION		3
#define DETAIL_SLOT_SIZE	2.f
#define DETAIL_SLOT_SIZE_2	DETAIL_SLOT_SIZE*0.5f
 
//	int s_x	= iFloor			(EYE.x/slot_size+.5f)+offs_x;		// [0...size_x)
//	int s_z	= iFloor			(EYE.z/slot_size+.5f)+offs_z;		// [0...size_z)


/*
0 - Header(version,obj_count(max255),size_x,size_z,min_x,min_z)
1 - Objects
	0
	1
	2
	..
	obj_count-1
2 - slots

	CMemoryWriter F;
    m_Header.object_count=m_Objects.size();
	// header
	F.w_chunk		(DETMGR_CHUNK_HEADER,&m_Header,sizeof(DetailHeader));
    // objects
	F.open_chunk		(DETMGR_CHUNK_OBJECTS);
    for (DOIt it=m_Objects.begin(); it!=m_Objects.end(); it++){
		F.open_chunk	(it-m_Objects.begin());
        (*it)->Export	(F);
	    F.close_chunk	();
    }
    F.close_chunk		();
    // slots
	F.open_chunk		(DETMGR_CHUNK_SLOTS);
	F.write				(m_Slots.begin(),m_Slots.size()*sizeof(DetailSlot));
    F.close_chunk		();

    F.SaveTo			(fn,0);
*/
/*
// detail object
	char*			shader;
	char*			texture;

	u32				flag;
	float			min_scale;
	float	 		max_scale;

	u32				vert_count;
	u32				index_count;

	fvfVertexIn*	vertices;
	u16*			indices;
*/

#define DO_NO_WAVING	0x0001

struct DetailHeader
{
	u32		version;
	u32		object_count;
	int		offs_x,	offs_z;
	u32		size_x,	size_z;
};
struct DetailPalette
{
	u16		a0:4;
	u16		a1:4;
	u16		a2:4;
	u16		a3:4;
};
struct DetailSlot					// was(4+4+3*4+2 = 22b), now(8+2*4=16b)
{
	u64				y_base	:	12;	// 11	// 1 unit = 20 cm, low = -200m, high = 4096*20cm - 200 = 619.2m
	u64				y_height:	8;	// 20	// 1 unit = 5  cm, low = 0,     high = 256*5 = 12.8m
	u64				id0		:   6;	// 26	// 0x3F(63) = empty
	u64				id1		:	6;	// 32	// 0x3F(63) = empty
	u64				id2		:	6;	// 38	// 0x3F(63) = empty
	u64				id3     :	6;	// 42	// 0x3F(63) = empty
	u64				c_dir	:	4;	// 48	// 0..1 q
	u64				c_hemi	:	4;	// 52	// 0..1 q
	u64				c_r		:	4;	// 56	// rgb = 4.4.4
	u64				c_g		:	4;	// 60	// rgb = 4.4.4
	u64				c_b		:	4;	// 64	// rgb = 4.4.4
	DetailPalette	palette [4];
public:
	enum			{	ID_Empty	= 0x3f	};
public:
	void			w_ybase		(float v)				{	s32	_v = iFloor((v + 200)/.2f);	clamp(_v,0,4095); y_base = _v;	}
	float			r_ybase		()						{	return float(y_base)*.2f - 200.f;								}
	void			w_yheight	(float v)				{	s32	_v = iFloor(v / .05f); clamp(_v,0,255); y_height = _v;		}
	float			r_yheight	()						{	return float(y_height)*.05f;									}
	u32				w_qclr		(float v, u32 range)	{	s32 _v = iFloor(v * float(range)); clamp(_v,0,s32(range)); return _v; };
	float			r_qclr		(u32 v,   u32 range)	{	return float(v)/float(range); }

	void			verify		()						{	VERIFY(16==sizeof(DetailSlot));	}
};

#pragma pack(pop)
#endif // _DEBUG
