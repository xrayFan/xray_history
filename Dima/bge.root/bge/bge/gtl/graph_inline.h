////////////////////////////////////////////////////////////////////////////
//	Module 		: graph_inline.h
//	Created 	: 14.01.2004
//  Modified 	: 19.02.2005
//	Author		: Dmitriy Iassenev
//	Description : Graph class template inline functions
////////////////////////////////////////////////////////////////////////////

#pragma once

#define TEMPLATE_SPECIALIZATION template <\
	typename _data_type,\
	typename _edge_weight_type,\
	typename _vertex_id_type,\
	template <\
		typename,\
		typename,\
		typename\
	> class _graph_vertex_type,\
	template <\
		typename,\
		typename\
	> class _graph_edge_type\
>

#define CAbstractGraph graph<\
	_data_type,\
	_edge_weight_type,\
	_vertex_id_type,\
	_graph_vertex_type,\
	_graph_edge_type\
>

TEMPLATE_SPECIALIZATION
IC	CAbstractGraph::graph		()
{
	m_edge_count		= 0;
}

TEMPLATE_SPECIALIZATION
CAbstractGraph::~graph			()
{
	clear				();
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::add_vertex			(const _data_type &data, const _vertex_id_type &vertex_id)
{
	VERIFY				(!vertex(vertex_id));
	m_vertices.insert	(new vertex_type(data,vertex_id));
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::remove_vertex		(const _vertex_id_type &vertex_id)
{
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::add_edge			(const _vertex_id_type &vertex_id0, const _vertex_id_type &vertex_id1, const _edge_weight_type &edge_weight)
{
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::add_edge			(const _vertex_id_type &vertex_id0, const _vertex_id_type &vertex_id1, const _edge_weight_type &edge_weight0, const _edge_weight_type &edge_weight1)
{
	add_edge			(vertex_id0,vertex_id1,edge_weight0);
	add_edge			(vertex_id1,vertex_id0,edge_weight1);
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::remove_edge		(const _vertex_id_type &vertex_id0, const _vertex_id_type &vertex_id1)
{
}

TEMPLATE_SPECIALIZATION
IC	size_t CAbstractGraph::vertex_count		() const
{
	return				(m_vertices.size());
}

TEMPLATE_SPECIALIZATION
IC	size_t CAbstractGraph::edge_count		() const
{
	return				(m_edge_count);
}

TEMPLATE_SPECIALIZATION
IC	bool CAbstractGraph::empty				() const
{
	return				(m_vertices.empty());
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::clear				()
{
	while (!vertices().empty())
		remove_vertex	((*vertices().begin())->vertex_id());
	VERIFY				(!m_edge_count);
}

TEMPLATE_SPECIALIZATION
IC	const typename CAbstractGraph::vertex_type *CAbstractGraph::vertex	(const _vertex_id_type &vertex_id) const
{
	const_vertex_iterator	I = m_vertices.find(vertex_id);
	if (m_vertices.end() == I)
		return				(0);
	return					(*I);
}

TEMPLATE_SPECIALIZATION
IC	typename CAbstractGraph::vertex_type *CAbstractGraph::vertex		(const _vertex_id_type &vertex_id)
{
	vertex_iterator		I = m_vertices.find(vertex_id);
	if (m_vertices.end() == I)
		return			(0);
	return				(*I);
}

TEMPLATE_SPECIALIZATION
IC	const typename CAbstractGraph::edge_type *CAbstractGraph::edge	(const _vertex_id_type &vertex_id0, const _vertex_id_type &vertex_id1) const
{
	const vertex_type	*_vertex = vertex(vertex_id0);
	if (!_vertex)
		return			(0);
	return				(_vertex->edge(vertex_id1));
}

TEMPLATE_SPECIALIZATION
IC	typename CAbstractGraph::edge_type *CAbstractGraph::edge	(const _vertex_id_type &vertex_id0, const _vertex_id_type &vertex_id1)
{
	const vertex_type	*_vertex = vertex(vertex_id0);
	if (!_vertex)
		return			(0);
	return				(_vertex->edge(vertex_id1));
}

TEMPLATE_SPECIALIZATION
IC	const typename CAbstractGraph::VERTICES &CAbstractGraph::vertices() const
{
	return				(m_vertices);
}

TEMPLATE_SPECIALIZATION
IC	typename CAbstractGraph::VERTICES &CAbstractGraph::vertices	()
{
	return				(m_vertices);
}

TEMPLATE_SPECIALIZATION
IC	const CAbstractGraph &CAbstractGraph::header	() const
{
	return				(*this);
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::save			(IWriter &stream)
{
	stream.open_chunk		(0);
	stream.w_u32			((u32)m_vertices.size());
	stream.close_chunk		();
	
	stream.open_chunk		(1);
	const_vertex_iterator	I = vertices().begin();
	const_vertex_iterator	E = vertices().end();
	for (int i=0; I != E; ++I, ++i) {
		stream.open_chunk	(i);

		{
			stream.open_chunk	(0);
			save_data			((*I)->data(),stream);
			stream.close_chunk	();

			stream.open_chunk	(1);
			save_data			((*I)->vertex_id(),stream);
			stream.close_chunk	();
		}

		stream.close_chunk	();
	}
	stream.close_chunk		();

	stream.open_chunk		(2);
	I						= vertices().begin();
	for (int j=0; I != E; ++I, ++j) {
		if ((*I)->edges().empty())
			continue;

		stream.open_chunk	(j);

		stream.w_u32		((u32)(*I)->edges().size());
		const_edge_iterator	i = (*I)->edges().begin();
		const_edge_iterator	e = (*I)->edges().end();
		for ( ; i != e; ++i) {
			save_data		((*i).vertex()->vertex_id(),stream);
			save_data		((*i).weight(),stream);
		}

		stream.close_chunk	();
	}
	stream.close_chunk		();
}

TEMPLATE_SPECIALIZATION
IC	void CAbstractGraph::load			(IReader &stream)
{
	clear						();

	_data_type						data;
	_vertex_id_type				vertex_id;
	_edge_weight_type			edge_weight;
	IReader						*chunk0, *chunk1, *chunk2;

	chunk0						= stream.open_chunk(0);
	u32							n = chunk0->r_u32();
	chunk0->close				();

	chunk0						= stream.open_chunk(1);
	for (u32 i=0; i<n; ++i) {
		chunk1					= chunk0->open_chunk(i);
			
		{
			chunk2				= chunk1->open_chunk(0);
			load_data			(data,*chunk2);
			chunk2->close		();

			chunk2				= chunk1->open_chunk(1);
			load_data			(vertex_id,*chunk2);
			chunk2->close		();

			add_vertex			(data,vertex_id);
		}
		chunk1->close			();
	}
	chunk0->close				();

	chunk0						= stream.open_chunk(2);
	if (!chunk0)
		return;

	const_vertex_iterator		I = m_vertices.begin(), B = I;
	const_vertex_iterator		E = m_vertices.end();
	for (int j=0; I != E; ++I, ++j) {
		chunk1					= chunk0->open_chunk(j);
		if (!chunk1)
			continue;
		u32						n = chunk1->r_u32();
		for (u32 i=0; i<n; ++i) {
			load_data			(vertex_id,*chunk1);
			load_data			(edge_weight,*chunk1);
			add_edge			((*I)->vertex_id(),vertex_id,edge_weight);
		}
		chunk1->close			();
	}
	chunk0->close				();
}

#undef TEMPLATE_SPECIALIZATION
#undef CAbstractGraph