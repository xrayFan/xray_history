////////////////////////////////////////////////////////////////////////////
//	Module 		: agent_member_manager_inline.h
//	Created 	: 24.05.2004
//  Modified 	: 14.01.2005
//	Author		: Dmitriy Iassenev
//	Description : Agent member manager inline functions
////////////////////////////////////////////////////////////////////////////

#pragma once

class CMemberPredicate {
protected:
	const CAI_Stalker	*m_object;

public:
	IC				CMemberPredicate	(const CAI_Stalker *object) :
	  m_object	(object)
	  {
	  }

	  IC		bool	operator()			(const CMemberOrder &order) const
	  {
		  return			(&order.object() == m_object);
	  }
};

IC	CAgentMemberManager::CAgentMemberManager	(CAgentManager *object)
{
	VERIFY				(object);
	m_object			= object;
}

IC	CAgentManager &CAgentMemberManager::object	() const
{
	VERIFY				(m_object);
	return				(*m_object);
}

IC	const CAgentMemberManager::MEMBER_STORAGE	&CAgentMemberManager::members	() const
{
	return				(m_members);
}

IC	CAgentMemberManager::MEMBER_STORAGE	&CAgentMemberManager::members	()
{
	return				(m_members);
}

IC	CSetupAction &CAgentMemberManager::action	(CAI_Stalker *object)
{
	return				(member(object).action());
}

IC	CMemberOrder &CAgentMemberManager::member	(const CAI_Stalker *object)
{
	iterator			I = std::find_if(members().begin(), members().end(), CMemberPredicate(object));
	VERIFY				(I != members().end());
	return				(*I);
}

IC	MemorySpace::squad_mask_type CAgentMemberManager::mask(const CAI_Stalker *object) const
{
	const_iterator		I = std::find_if(members().begin(),members().end(), CMemberPredicate(object));
	VERIFY				(I != members().end());
	return				(MemorySpace::squad_mask_type(1) << (I - members().begin()));
}

IC	CAgentMemberManager::iterator CAgentMemberManager::member		(MemorySpace::squad_mask_type mask)
{
	iterator			I = m_members.begin();
	iterator			E = m_members.end();
	for ( ; I != E; ++I, mask >>= 1)
		if (mask == 1)
			return		(I);
	NODEFAULT;
#ifdef DEBUG
	return				(E);
#endif
}

IC	bool CAgentMemberManager::group_behaviour					() const
{
	return				(members().size() > 1);
}

IC	const CAgentMemberManager::squad_mask_type &CAgentMemberManager::combat_mask() const
{
	return				(m_combat_mask);
}
