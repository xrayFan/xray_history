#pragma once

#include "..\\biting\\ai_biting.h"
#include "..\\ai_monster_bones.h"
#include "..\\ai_monster_jump.h"

class CAI_Dog : public CAI_Biting, public CJumping {
	typedef		CAI_Biting	inherited;

	bool					strike_in_jump;
	bonesManipulation		Bones;
public:
					CAI_Dog				();
	virtual			~CAI_Dog			();	

	virtual	BOOL	net_Spawn			(LPVOID DC);
	virtual void	Load				(LPCSTR section);
	virtual void	UpdateCL			();

	virtual void	Init				();
	virtual void	StateSelector		();
	virtual void	CheckSpecParams		(u32 spec_params);
	virtual	void	OnSoundPlay			();

	virtual void	LookPosition		(Fvector to_point);		

private:
	static	void __stdcall	BoneCallback			(CBoneInstance *B);
	
};
