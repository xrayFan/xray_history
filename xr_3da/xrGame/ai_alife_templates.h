////////////////////////////////////////////////////////////////////////////
//	Module 		: ai_alife_templates.h
//	Created 	: 21.01.2003
//  Modified 	: 21.01.2003
//	Author		: Dmitriy Iassenev
//	Description : A-Life templates
////////////////////////////////////////////////////////////////////////////

#ifndef XRAY_AI_ALIFE_TEMPLATES
#define XRAY_AI_ALIFE_TEMPLATES

#include "ai_alife_space.h"
using namespace ALife;

//IC void save_bool_vector(vector<bool> &baVector, CFS_Memory &tMemoryStream)
//{
//	tMemoryStream.Wdword		(baVector.size());
//	BOOL_IT 					I = baVector.begin();
//	BOOL_IT 					E = baVector.end();
//	u32							dwMask = 0;
//	if (I != E) {
//		for (int j=0; I != E; I++, j++) {
//			if (j >= 32) {
//				tMemoryStream.Wdword(dwMask);
//				dwMask = 0;
//				j = 0;
//			}
//			if (*I)
//				dwMask |= u32(1) << j;
//		}
//		tMemoryStream.Wdword	(dwMask);
//	}
//};
//
//IC void load_bool_vector(vector<bool> &baVector, CStream &tFileStream)
//{
//	baVector.resize				(tFileStream.Rdword());
//	BOOL_IT 					I = baVector.begin();
//	BOOL_IT 					E = baVector.end();
//	u32							dwMask = 0;
//	for (int j=32; I != E; I++, j++) {
//		if (j >= 32) {
//			dwMask = tFileStream.Rdword();
//			j = 0;
//		}
//		*I = !!(dwMask & (u32(1) << j));
//	}
//};
//
//template <class T>
//void save_base_vector(vector<T> &tpVector, CFS_Memory &tMemoryStream)
//{
//	tMemoryStream.Wdword		(tpVector.size());
//	vector<T>::iterator			I = tpVector.begin();
//	vector<T>::iterator			E = tpVector.end();
//	for ( ; I != E; I++)
//		tMemoryStream.write		(I,sizeof(*I));
//};
//
//template <class T>
//void load_base_vector(vector<T> &tpVector, CStream &tFileStream)
//{
//	tpVector.resize				(tFileStream.Rdword());
//	vector<T>::iterator			I = tpVector.begin();
//	vector<T>::iterator			E = tpVector.end();
//	for ( ; I != E; I++)
//		tFileStream.Read		(I,sizeof(*I));
//};
//
//template <class T>
//void init_vector(vector<T *> &tpVector, _SPAWN_ID tSpawnID, SPAWN_P_VECTOR &tpSpawnPoints)
//{
//	vector<T *>::iterator		I = tpVector.begin();
//	vector<T *>::iterator		E = tpVector.end();
//	for ( ; I != E; I++) {
//		*I = xr_new<T>			();
//		(*I)->Init				(tSpawnID,tpSpawnPoints);
//	}
//};
//
//template <class T>
//void save_vector(vector<T *> &tpVector, CFS_Memory &tMemoryStream)
//{
//	tMemoryStream.Wdword		(tpVector.size());
//	vector<T *>::iterator		I = tpVector.begin();
//	vector<T *>::iterator		E = tpVector.end();
//	for ( ; I != E; I++)
//		(*I)->Save				(tMemoryStream);
//};
//
//template <class T>
//void load_vector(vector<T *> &tpVector, CStream &tFileStream)
//{
//	tpVector.resize				(tFileStream.Rdword());
//	vector<T *>::iterator		I = tpVector.begin();
//	vector<T *>::iterator		E = tpVector.end();
//	for ( ; I != E; I++) {
//		*I = xr_new<T>			();
//		(*I)->Load				(tFileStream);
//	}
//};

template <class T>
void free_vector(vector<T *> &tpVector)
{
	vector<T *>::iterator		I = tpVector.begin();
	vector<T *>::iterator		E = tpVector.end();
	for ( ; I != E; I++)
		xr_delete				(*I);
};
//
//template <class T1, class T2>
//void save_map(map<T1,T2 *> &tpMap, CFS_Memory &tMemoryStream)
//{
//	tMemoryStream.Wdword		(tpMap.size());
//	map<T1,T2 *>::iterator		I = tpMap.begin();
//	map<T1,T2 *>::iterator		E = tpMap.end();
//	for ( ; I != E; I++)
//		(*I).second->Save		(tMemoryStream);
//};
//
//template <class T1, class T2>
//void load_map(map<T1,T2 *> &tpMap, CStream &tFileStream, T1 tfGetKey(const T2 *))
//{
//	tpMap.clear					();
//	u32							dwCount	= tFileStream.Rdword();
//	for (int i=0 ; i<(int)dwCount; i++) {
//		T2						*T = xr_new<T2>();
//		T->Load					(tFileStream);
//		tpMap.insert			(make_pair(tfGetKey(T),T));
//	}
//};

template <class T1, class T2>
void free_map(map<T1,T2 *> &tpMap)
{
	map<T1,T2 *>::iterator		I = tpMap.begin();
	map<T1,T2 *>::iterator		E = tpMap.end();
	for ( ; I != E; I++)
		xr_delete					((*I).second);
};

// server objects
IC void save_bool_vector(vector<bool> &baVector, NET_Packet &tNetPacket)
{
	tNetPacket.w_u32			(baVector.size());
	vector<bool>::iterator 		I = baVector.begin();
	vector<bool>::iterator 		E = baVector.end();
	u32							dwMask = 0;
	if (I != E) {
		for (int j=0; I != E; I++, j++) {
			if (j >= 32) {
				tNetPacket.w_u32(dwMask);
				dwMask = 0;
				j = 0;
			}
			if (*I)
				dwMask |= u32(1) << j;
		}
		tNetPacket.w_u32		(dwMask);
	}
};

IC void load_bool_vector(vector<bool> &baVector, NET_Packet &tNetPacket)
{
	u32							dwDummy;
	tNetPacket.r_u32			(dwDummy);
	baVector.resize				(dwDummy);
	vector<bool>::iterator 		I = baVector.begin();
	vector<bool>::iterator 		E = baVector.end();
	u32							dwMask = 0;
	for (int j=32; I != E; I++, j++) {
		if (j >= 32) {
			tNetPacket.r_u32(dwMask);
			j = 0;
		}
		*I = !!(dwMask & (u32(1) << j));
	}
};

template <class T>
void save_base_vector(vector<T> &tpVector, NET_Packet &tNetPacket)
{
	tNetPacket.w_u32			(tpVector.size());
	vector<T>::iterator			I = tpVector.begin();
	vector<T>::iterator			E = tpVector.end();
	for ( ; I != E; I++)
		tNetPacket.w			(I,sizeof(*I));
};

template <class T>
void load_base_vector(vector<T> &tpVector, NET_Packet &tNetPacket)
{
	u32							dwDummy;
	tNetPacket.r_u32			(dwDummy);
	tpVector.resize				(dwDummy);
	vector<T>::iterator			I = tpVector.begin();
	vector<T>::iterator			E = tpVector.end();
	for ( ; I != E; I++)
		tNetPacket.r		(I,sizeof(*I));
};

template <class T>
void init_vector(vector<T *> &tpVector, LPCSTR caSection)
{
	vector<T *>::iterator		I = tpVector.begin();
	vector<T *>::iterator		E = tpVector.end();
	for ( ; I != E; I++) {
		*I = xr_new<T>			();
		(*I)->Init				(caSection);
	}
};

template <class T>
void save_vector(vector<T *> &tpVector, NET_Packet &tNetPacket)
{
	tNetPacket.w_u32			(tpVector.size());
	vector<T *>::iterator		I = tpVector.begin();
	vector<T *>::iterator		E = tpVector.end();
	for ( ; I != E; I++)
		(*I)->UPDATE_Write		(tNetPacket);
};

template <class T>
void load_vector(vector<T *> &tpVector, NET_Packet &tNetPacket)
{
	u32							dwDummy;
	tNetPacket.r_u32			(dwDummy);
	tpVector.resize				(dwDummy);
	vector<T *>::iterator		I = tpVector.begin();
	vector<T *>::iterator		E = tpVector.end();
	for ( ; I != E; I++) {
		*I = xr_new<T>			();
		(*I)->UPDATE_Read		(tNetPacket);
	}
};

template <class T1, class T2>
void save_map(map<T1,T2 *> &tpMap, NET_Packet &tNetPacket)
{
	tNetPacket.w_u32		(tpMap.size());
	map<T1,T2 *>::iterator		I = tpMap.begin();
	map<T1,T2 *>::iterator		E = tpMap.end();
	for ( ; I != E; I++)
		(*I).second->Save		(tMemoryStream);
};

template <class T1, class T2>
void load_map(map<T1,T2 *> &tpMap, NET_Packet &tNetPacket, T1 tfGetKey(const T2 *))
{
	tpMap.clear					();
	u32							dwCount	= tFileStream.Rdword();
	for (int i=0 ; i<(int)dwCount; i++) {
		T2						*T = xr_new<T2>();
		T->Load					(tFileStream);
		tpMap.insert			(make_pair(tfGetKey(T),T));
	}
};
#endif