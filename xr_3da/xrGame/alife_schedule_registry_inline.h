////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_schedule_registry_inline.h
//	Created 	: 15.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife schedule registry inline functions
////////////////////////////////////////////////////////////////////////////

#pragma once

IC	void CALifeScheduleRegistry::update		()
{
	u32							count = objects().empty() ? 0 : inherited::update(CUpdatePredicate());
#ifdef DEBUG
	if (psAI_Flags.test(aiALife)) {
		Msg						("[LSS][SU][%d : %d]",count, objects().size());
	}
#endif
}

IC	CSE_ALifeSchedulable *CALifeScheduleRegistry::object	(const ALife::_OBJECT_ID &id, bool no_assert) const
{
	_const_iterator				I = objects().find(id);
	if (I == objects().end()) {
		R_ASSERT2				(no_assert,"The spesified object hasn't been found in the schedule registry!");
		return					(0);
	}
	return						((*I).second);
}
