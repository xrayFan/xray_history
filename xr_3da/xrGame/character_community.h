//////////////////////////////////////////////////////////////////////////
// character_community.h:	��������� ������������� �����������
//							
//////////////////////////////////////////////////////////////////////////

#pragma once


typedef shared_str				CHARACTER_COMMUNITY_ID;
#define NO_COMMUNITY_ID			CHARACTER_COMMUNITY_ID(NULL)

typedef int						CHARACTER_COMMUNITY_INDEX;
#define NO_COMMUNITY_INDEX		CHARACTER_COMMUNITY_INDEX(-1)



struct CHARACTER_COMMUNITY
{
	CHARACTER_COMMUNITY		();
	~CHARACTER_COMMUNITY	();

	void						set				(CHARACTER_COMMUNITY_ID);
	void						set				(CHARACTER_COMMUNITY_INDEX);

	CHARACTER_COMMUNITY_ID		id				() const;
	CHARACTER_COMMUNITY_INDEX	index			() const;
	u8							team			() const;

private:
	CHARACTER_COMMUNITY_INDEX	m_current_index;



public:
	CHARACTER_COMMUNITY_INDEX	index_by_id		(CHARACTER_COMMUNITY_ID)	const;
	CHARACTER_COMMUNITY_ID		id_by_index		(CHARACTER_COMMUNITY_INDEX) const;

	struct COMMUNITY_DATA
	{
		CHARACTER_COMMUNITY_ID id;
		u8 team;
	};
	
	typedef std::vector<COMMUNITY_DATA> COMMUNITIES_NAMES;
	static  const COMMUNITIES_NAMES&	CommunitiesNames		();
	static void							DeleteCommunitiesNames	();

private:
	static COMMUNITIES_NAMES* communities_names;
};