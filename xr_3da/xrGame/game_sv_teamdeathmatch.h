#pragma once

#include "game_sv_deathmatch.h"

class	game_sv_TeamDeathmatch			: public game_sv_Deathmatch
{
private:
	typedef game_sv_Deathmatch inherited;

protected:
	float		m_fFriendlyFireModifier;
public:	
									game_sv_TeamDeathmatch	(){type = GAME_TEAMDEATHMATCH;}
	virtual		void				Create					(ref_str& options);

	virtual		void				OnEvent					(NET_Packet &tNetPacket, u16 type, u32 time, u32 sender );

	virtual		LPCSTR				type_name			() const { return "teamdeathmatch";};

	// Events	
	virtual		void				OnPlayerConnect			(u32 id_who);

	virtual		void				OnPlayerChangeTeam		(u32 id_who, s16 team);
	virtual		void				OnRoundStart			();							// ����� ������
	virtual		void				OnPlayerKillPlayer		(u32 id_killer, u32 id_killed);

	virtual		void				OnPlayerHitPlayer		(u16 id_hitter, u16 id_hitted, NET_Packet& P);

	virtual		u8					AutoTeam				( );
	virtual		u32					RP_2_Use				(CSE_Abstract* E);

	virtual		void				LoadTeams				();

	virtual		char*				GetAnomalySetBaseName	() {return "teamdeathmatch_game_anomaly_sets";};
};
