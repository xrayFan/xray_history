////////////////////////////////////////////////////////////////////////////
//	Module 		: state_manager_state_inline.h
//	Created 	: 14.01.2004
//  Modified 	: 14.01.2004
//	Author		: Dmitriy Iassenev
//	Description : State manager state inline functions
////////////////////////////////////////////////////////////////////////////

#pragma once

#define TEMPLATE_SPECIALIZATION template <\
	typename _Object\
>

#define CStateManagerAbstract CStateManagerState<_Object>

TEMPLATE_SPECIALIZATION
CStateManagerAbstract::CStateManagerState	()
{
	Init					();
}

TEMPLATE_SPECIALIZATION
CStateManagerAbstract::~CStateManagerState	()
{
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::Init			()
{
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::Load			(LPCSTR section)
{
	CSStateBase::Load				(section);
	CSStateManagerAbstract::Load	(section);
	for (u32 i=0, n=graph().vertices().size(); i<n; ++i)
		graph().vertices()[i].data().m_state->Load(section);
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::reinit			(_Object *object, u32 state_id)
{
	CSStateBase::reinit				(object);
	CSStateManagerAbstract::reinit	(state_id);
	for (u32 i=0, n=graph().vertices().size(); i<n; ++i)
		graph().vertices()[i].data().m_state->reinit(object);
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::reload			(LPCSTR section)
{
	CSStateBase::reload				(section);
	CSStateManagerAbstract::reload	(section);
	for (u32 i=0, n=graph().vertices().size(); i<n; ++i)
		graph().vertices()[i].data().m_state->reload(section);
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::update			(u32 time_delta)
{
	execute							();
	CSStateManagerAbstract::update	(time_delta);
	IGraphManager					*state_manager_interface = dynamic_cast<IGraphManager*>(&current_state());
	if (state_manager_interface)
		state_manager_interface->update(time_delta);
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::initialize		()
{
	CSStateBase::initialize			();
	current_state().initialize		();
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::finalize		()
{
	CSStateBase::finalize			();
	current_state().finalize		();
}

TEMPLATE_SPECIALIZATION
bool CStateManagerAbstract::completed		() const
{
	return							(current_state().completed());
}

TEMPLATE_SPECIALIZATION
void CStateManagerAbstract::execute	()
{
//	if (!path().empty () && (path().size() < 2)) {
//		VERIFY			(current_state_id() == path().front());
//		go_path			();
//	}

	if (current_vertex_id() == dest_vertex_id()) {
		IGraphManager				*state_manager_interface = dynamic_cast<IGraphManager*>(&current_state());
		if (!state_manager_interface)
			current_state().execute	();
		return;
	}

	if (path().empty())
		return;

	for (;;) {
		if (!state(path().front()).completed()) {
			state(path().front()).execute();
			return;
		}
		state(path().front()).finalize();
		go_path			();
		state(path().front()).initialize();
		state(path().front()).execute();
		if (path().size() < 2) {
			go_path			();
			break;
		}
	}
}

TEMPLATE_SPECIALIZATION
IC	xr_vector<u32> &CStateManagerAbstract::sequence()
{
	return					(path());
}

TEMPLATE_SPECIALIZATION
IC	const xr_vector<u32> &CStateManagerAbstract::sequence() const
{
	return					(path());
}

#undef TEMPLATE_SPECIALIZATION
#undef CStateManagerAbstract
