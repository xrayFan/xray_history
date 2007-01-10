#include "stdafx.h"
#include "phrasedialog.h"
#include "phrasedialogmanager.h"
#include "gameobject.h"
#include "ai_debug.h"

SPhraseDialogData::SPhraseDialogData ()
{
	m_PhraseGraph.clear	();
	m_iPriority			= 0;
}

SPhraseDialogData::~SPhraseDialogData ()
{}



CPhraseDialog::CPhraseDialog()
{
	m_iSaidPhraseID		= -1;
	m_bFinished			= false;
	m_pSpeakerFirst		= NULL;
	m_pSpeakerSecond	= NULL;
	m_DialogId			= NULL;
}

CPhraseDialog::~CPhraseDialog()
{
}


void CPhraseDialog::Init(CPhraseDialogManager* speaker_first, 
						 CPhraseDialogManager* speaker_second)
{
	THROW(!IsInit());

	m_pSpeakerFirst		= speaker_first;
	m_pSpeakerSecond	= speaker_second;

	m_iSaidPhraseID		= -1;
	m_PhraseVector.clear();

	CPhraseGraph::CVertex* phrase_vertex = data()->m_PhraseGraph.vertex(0);
	THROW(phrase_vertex);
	m_PhraseVector.push_back(phrase_vertex->data());

	m_bFinished			= false;
	m_bFirstIsSpeaking	= true;
}

//�������� ��� �����
void CPhraseDialog::Reset ()
{
}

CPhraseDialogManager* CPhraseDialog::OurPartner	(CPhraseDialogManager* dialog_manager) const
{
	if(FirstSpeaker() == dialog_manager)
		return SecondSpeaker();
	else
		return FirstSpeaker();
}


CPhraseDialogManager* CPhraseDialog::CurrentSpeaker()	const 
{
	return FirstIsSpeaking()?m_pSpeakerFirst:m_pSpeakerSecond;
}
CPhraseDialogManager* CPhraseDialog::OtherSpeaker	()	const 
{
	return (!FirstIsSpeaking())?m_pSpeakerFirst:m_pSpeakerSecond;
}


//�������� ��� ���������� ������� ����
static bool PhraseGoodwillPred(const CPhrase* phrase1, const CPhrase* phrase2)
{
	return phrase1->GoodwillLevel()>phrase2->GoodwillLevel();
}

bool CPhraseDialog::SayPhrase (DIALOG_SHARED_PTR& phrase_dialog, int phrase_id)
{
	THROW(phrase_dialog->IsInit());

	phrase_dialog->m_iSaidPhraseID = phrase_id;

	bool first_is_speaking = phrase_dialog->FirstIsSpeaking();
	phrase_dialog->m_bFirstIsSpeaking = !phrase_dialog->m_bFirstIsSpeaking;

	const CGameObject*	pSpeakerGO1 = smart_cast<const CGameObject*>(phrase_dialog->FirstSpeaker());	VERIFY(pSpeakerGO1);
	const CGameObject*	pSpeakerGO2 = smart_cast<const CGameObject*>(phrase_dialog->SecondSpeaker());	VERIFY(pSpeakerGO2);
	if(!first_is_speaking) std::swap(pSpeakerGO1, pSpeakerGO2);

	CPhraseGraph::CVertex* phrase_vertex = phrase_dialog->data()->m_PhraseGraph.vertex(phrase_dialog->m_iSaidPhraseID);
	THROW(phrase_vertex);

	CPhrase* last_phrase = phrase_vertex->data();

	//������� ���������� �������������� ������� 
	//������������ ����� ��������� �����
	//������ �������� - ��� ��� ������� �����, ������ - ��� ��� �������
	last_phrase->m_PhraseScript.Action(pSpeakerGO1, pSpeakerGO2, *phrase_dialog->m_DialogId, (int)phrase_id);

	//������ ��� ����, ���� ��������
	phrase_dialog->m_PhraseVector.clear();
	if(phrase_vertex->edges().empty())
	{
		phrase_dialog->m_bFinished = true;
	}
	else
	{
		//�������� ������ ����, ������� ������ ������ �������� ����������
		for(xr_vector<CPhraseGraph::CEdge>::const_iterator it = phrase_vertex->edges().begin();
			it != phrase_vertex->edges().end();
			it++)
		{
			const CPhraseGraph::CEdge& edge = *it;
			CPhraseGraph::CVertex* next_phrase_vertex = phrase_dialog->data()->m_PhraseGraph.vertex(edge.vertex_id());
			THROW						(next_phrase_vertex);
			int next_phrase_id			= next_phrase_vertex->vertex_id();
			if(next_phrase_vertex->data()->m_PhraseScript.Precondition(pSpeakerGO2, pSpeakerGO1, *phrase_dialog->m_DialogId, (int)phrase_id, next_phrase_id)){
				phrase_dialog->m_PhraseVector.push_back(next_phrase_vertex->data());
#ifdef DEBUG
				if(psAI_Flags.test(aiDialogs)){
					LPCSTR phrase_text = next_phrase_vertex->data()->GetText();
					int iii = next_phrase_vertex->data()->GetIndex();
					Msg("----added phrase text [%s]phrase_id=[%d] index=[%d] to dialog [%s]",phrase_text,phrase_id,iii,*phrase_dialog->m_DialogId);
				}
#endif
			}

		}

		THROW3(!phrase_dialog->m_PhraseVector.empty(), "No available phrase to say.", *phrase_dialog->m_DialogId);

		//����������� ������ �� �������� ���������������
		std::sort(phrase_dialog->m_PhraseVector.begin(),
				 phrase_dialog->m_PhraseVector.end(), PhraseGoodwillPred);
	}



	//�������� CDialogManager, ��� ������� �����
	//� ��������� �����
	if(first_is_speaking)
		phrase_dialog->SecondSpeaker()->ReceivePhrase(phrase_dialog);
	else
		phrase_dialog->FirstSpeaker()->ReceivePhrase(phrase_dialog);


	return phrase_dialog?!phrase_dialog->m_bFinished:true;
}

LPCSTR CPhraseDialog::GetPhraseText	(int phrase_id, bool current_speaking)
{
	
	CPhraseGraph::CVertex* phrase_vertex = data()->m_PhraseGraph.vertex(phrase_id);
	THROW(phrase_vertex);

	return phrase_vertex->data()->GetText();
}

LPCSTR CPhraseDialog::DialogCaption()
{
	return data()->m_sCaption.size()?*data()->m_sCaption:GetPhraseText(0);
}


int	 CPhraseDialog::Priority()
{
	return data()->m_iPriority;
}


void CPhraseDialog::Load(shared_str dialog_id)
{
	m_DialogId = dialog_id;
	inherited_shared::load_shared(m_DialogId, NULL);
}

#include "script_engine.h"
#include "script_space.h"
#include "ai_space.h"

void CPhraseDialog::load_shared	(LPCSTR)
{
	const id_to_index::ITEM_DATA& item_data = *id_to_index::GetById(m_DialogId);

	CUIXml*		pXML		= item_data._xml;
	pXML->SetLocalRoot		(pXML->GetRoot());

	//loading from XML
	XML_NODE* dialog_node = pXML->NavigateToNode(id_to_index::tag_name, item_data.pos_in_file);
	THROW3(dialog_node, "dialog id=", *item_data.id);

	pXML->SetLocalRoot(dialog_node);


	SetPriority	( pXML->ReadAttribInt(dialog_node, "priority", 0) );

	//��������� 
	SetCaption	( pXML->Read(dialog_node, "caption", 0, NULL) );

	//��������� ������ �������
	data()->m_PhraseScript.Load(pXML, dialog_node);

	//��������� ���� ������� �������
	data()->m_PhraseGraph.clear();

	phrase_list_node = pXML->NavigateToNode(dialog_node, "phrase_list", 0);
	if(NULL == phrase_list_node){
		LPCSTR func = pXML->Read(dialog_node, "init_func", 0, "");

		luabind::functor<void>	lua_function;
		bool functor_exists = ai().script_engine().functor(func ,lua_function);
		THROW3(functor_exists, "Cannot find precondition", func);
		lua_function	(this);
		return;
	}

	int phrase_num = pXML->GetNodesNum(phrase_list_node, "phrase");
	THROW3(phrase_num, "dialog %s has no phrases at all", *item_data.id);

	pXML->SetLocalRoot(phrase_list_node);

	LPCSTR wrong_phrase_id = pXML->CheckUniqueAttrib(phrase_list_node, "phrase", "id");
	THROW3(wrong_phrase_id == NULL, *item_data.id, wrong_phrase_id);
	

	//���� ��������� �����
	XML_NODE* phrase_node = pXML->NavigateToNodeWithAttribute("phrase", "id", "0");
	THROW(phrase_node);
	AddPhrase(phrase_node, 0, -1);
}

void CPhraseDialog::SetCaption	(LPCSTR str)
{
	data()->m_sCaption = str;
}

void CPhraseDialog::SetPriority	(int val)
{
	data()->m_iPriority = val;
}

CPhrase* CPhraseDialog::AddPhrase	(LPCSTR text, int phrase_id, int prev_phrase_id, int goodwil_level)
{
	CPhrase* phrase					= NULL;
	CPhraseGraph::CVertex* _vertex	= data()->m_PhraseGraph.vertex(phrase_id);
	if(!_vertex) {
		phrase						= xr_new<CPhrase>(); VERIFY(phrase);
		phrase->SetIndex			(phrase_id);

		phrase->SetText				(text);
		phrase->m_iGoodwillLevel	= goodwil_level;

		data()->m_PhraseGraph.add_vertex	(phrase, phrase_id);
	}

	if(prev_phrase_id != -1)
		data()->m_PhraseGraph.add_edge		(prev_phrase_id, phrase_id, 0.f);
	
	return phrase;
}

void CPhraseDialog::AddPhrase	(XML_NODE* phrase_node, int phrase_id, int prev_phrase_id)
{

	LPCSTR sText		= uiXml.Read		(phrase_node, "text", 0, "");
	int		gw			= uiXml.ReadInt		(phrase_node, "goodwill", 0, -10000);
	CPhrase* ph			= AddPhrase			(sText, phrase_id, prev_phrase_id, gw);
	if(!ph)				return;

	ph->m_PhraseScript.Load					(&uiXml, phrase_node);

	//����� ������� ���������� ����� �������� ����� ����
	int next_num = uiXml.GetNodesNum(phrase_node, "next");
	for(int i=0; i<next_num; i++)
	{
		LPCSTR next_phrase_id_str			= uiXml.Read(phrase_node, "next", i, "");
		XML_NODE* next_phrase_node			= uiXml.NavigateToNodeWithAttribute("phrase", "id", next_phrase_id_str);
		THROW(next_phrase_node);
		int next_phrase_id					= atoi(next_phrase_id_str);

		AddPhrase							(next_phrase_node, next_phrase_id, phrase_id);
	}
}

bool  CPhraseDialog::Precondition(const CGameObject* pSpeaker1, const CGameObject* pSpeaker2)
{
	return data()->m_PhraseScript.Precondition(pSpeaker1, pSpeaker2, *m_DialogId, -1, -1);
}

void   CPhraseDialog::InitXmlIdToIndex()
{
	if(!id_to_index::tag_name)
		id_to_index::tag_name = "dialog";
	if(!id_to_index::file_str)
		id_to_index::file_str = pSettings->r_string("dialogs", "files");
}

bool CPhraseDialog::allIsDummy	()
{
	PHRASE_VECTOR_IT it = m_PhraseVector.begin();
	bool bAllIsDummy = true;
	for(;it!=m_PhraseVector.end();++it){
		if( !(*it)->IsDummy() )
			bAllIsDummy=false;
	}

	return bAllIsDummy;
}
