////////////////////////////////////////////////////////////////////////////
//	Module 		: object_factory_inline.h
//	Created 	: 27.05.2004
//  Modified 	: 27.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Object factory inline functions
////////////////////////////////////////////////////////////////////////////

#pragma once

IC	const CObjectFactory &object_factory()
{
	if (!g_object_factory) {
		g_object_factory		= xr_new<CObjectFactory>();
		g_object_factory->init	();
	}
	return						(*g_object_factory);
}

#ifndef NO_XR_GAME
IC	CObjectFactory::CLIENT_SCRIPT_BASE_CLASS *CObjectFactory::client_instance() const
{
	return						(m_client_instance);
}
#endif

IC	CObjectFactory::SERVER_SCRIPT_BASE_CLASS *CObjectFactory::server_instance() const
{
	return						(m_server_instance);
}

IC	bool CObjectFactory::CObjectItemPredicate::operator()	(const CObjectItemAbstract *item1, const CObjectItemAbstract *item2) const
{
	return				(item1->clsid() < item2->clsid());
}

IC	bool CObjectFactory::CObjectItemPredicate::operator()	(const CObjectItemAbstract *item, const CLASS_ID &clsid) const
{
	return				(item->clsid() < clsid);
}

IC	CObjectFactory::CObjectItemPredicateCLSID::CObjectItemPredicateCLSID	(const CLASS_ID &clsid) :
	m_clsid				(clsid)
{
}

IC	bool CObjectFactory::CObjectItemPredicateCLSID::operator()	(const CObjectItemAbstract *item) const
{
	return				(m_clsid == item->clsid());
}

#ifndef NO_SCRIPTS
IC	CObjectFactory::CObjectItemPredicateScript::CObjectItemPredicateScript	(const ref_str &script_clsid_name) :
	m_script_clsid_name	(script_clsid_name)
{
}

IC	bool CObjectFactory::CObjectItemPredicateScript::operator()	(const CObjectItemAbstract *item) const
{
	return				(m_script_clsid_name == item->script_clsid());
}
#endif

IC	const CObjectFactory::OBJECT_ITEM_STORAGE &CObjectFactory::clsids	() const
{
	return				(m_clsids);
}

#ifndef NO_XR_GAME
IC	const CObjectItemAbstract &CObjectFactory::item	(const CLASS_ID &clsid) const
{
	const_iterator		I = std::lower_bound(clsids().begin(),clsids().end(),clsid,CObjectItemPredicate());
	VERIFY				((I != clsids().end()) && ((*I)->clsid() == clsid));
	return				(**I);
}
#else
IC	const CObjectItemAbstract *CObjectFactory::item	(const CLASS_ID &clsid, bool no_assert) const
{
	const_iterator		I = std::lower_bound(clsids().begin(),clsids().end(),clsid,CObjectItemPredicate());
	if ((I == clsids().end()) || ((*I)->clsid() != clsid)) {
		R_ASSERT		(no_assert);
		return			(0);
	}
	return				(*I);
}
#endif

IC	void CObjectFactory::add	(CObjectItemAbstract *item)
{
	R_ASSERT			(!m_initialized);
	
	const_iterator		I;
	
	I					= std::find_if(clsids().begin(),clsids().end(),CObjectItemPredicateCLSID(item->clsid()));
	VERIFY				(I == clsids().end());
	
#ifndef NO_XR_GAME
	I					= std::find_if(clsids().begin(),clsids().end(),CObjectItemPredicateScript(item->script_clsid()));
	VERIFY				(I == clsids().end());
#endif
	
	m_clsids.push_back	(item);
}

#ifndef NO_SCRIPTS
IC	int	CObjectFactory::script_clsid	(const CLASS_ID &clsid) const
{
	const_iterator		I = std::lower_bound(clsids().begin(),clsids().end(),clsid,CObjectItemPredicate());
	VERIFY				((I != clsids().end()) && ((*I)->clsid() == clsid));
	return				(int(I - clsids().begin()));
}
#endif

#ifndef NO_XR_GAME
IC	CObjectFactory::CLIENT_BASE_CLASS *CObjectFactory::client_object	(const CLASS_ID &clsid) const
{
	return				(item(clsid).client_object());
}

IC	CObjectFactory::SERVER_BASE_CLASS *CObjectFactory::server_object	(const CLASS_ID &clsid, LPCSTR section) const
{
	return				(item(clsid).server_object(section)->init());
}
#else
IC	CObjectFactory::SERVER_BASE_CLASS *CObjectFactory::server_object	(const CLASS_ID &clsid, LPCSTR section) const
{
	const CObjectItemAbstract	*object = item(clsid,true);
	return				(object ? object->server_object(section)->init() : 0);
}
#endif