// CUIMapWnd.cpp:  ������ ������������� �����
// 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../HUDManager.h"
#include "../actor.h"
#include "../pda.h"
#include "../infoportion.h"
#include "../level.h"
#include "../string_table.h"
#include "../UIZoneMap.h"
#include "UIInventoryUtilities.h"
#include "UIPdaWnd.h"
#include "UIXmlInit.h"
#include "UIMapWnd.h"
#include "../alife_registry_wrappers.h"
#include "../game_graph.h"



CUIGlobalMap::CUIGlobalMap()
{
	m_Box.set			(0,0,0,0);
	m_MinimizedRect.set	(0,0,0,0);
	m_NormalRect.set	(0,0,0,0);
	m_State				= stNone;
	Show				();
}

CUIGlobalMap::~CUIGlobalMap()
{
}

void CUIGlobalMap::Init()
{
	CUIXml uiXml;
	bool xml_result			= uiXml.Init(CONFIG_PATH, UI_PATH, "pda_map.xml");
	R_ASSERT3(xml_result, "xml file not found", "global_map.xml");

	// load map background
	CUIXmlInit xml_init;
	CUIStatic* background	= xr_new<CUIStatic>();
	background->SetWindowName("background");
	xml_init.InitStatic		(uiXml, "main_wnd:global_map:background", 0, background);
	background->SetAutoDelete(true);
	AttachChild				(background);
	string256				path;
	// load map state definition
	strcpy					(path,"main_wnd:global_map:background:minimized_rect");
	R_ASSERT3				(uiXml.NavigateToNode(path,0), "XML node not found", path);
	m_MinimizedRect.x1		= uiXml.ReadAttribInt(path, 0, "x1", 0);
	m_MinimizedRect.y1		= uiXml.ReadAttribInt(path, 0, "y1", 0);
	m_MinimizedRect.x2		= uiXml.ReadAttribInt(path, 0, "x2", 0);
	m_MinimizedRect.y2		= uiXml.ReadAttribInt(path, 0, "y2", 0);
	strcpy					(path,"main_wnd:global_map:background:normal_rect");
	R_ASSERT3				(uiXml.NavigateToNode(path,0), "XML node not found", path);
	m_NormalRect.x1			= uiXml.ReadAttribInt(path, 0, "x1", 0);
	m_NormalRect.y1			= uiXml.ReadAttribInt(path, 0, "y1", 0);
	m_NormalRect.x2			= uiXml.ReadAttribInt(path, 0, "x2", 0);
	m_NormalRect.y2			= uiXml.ReadAttribInt(path, 0, "y2", 0);

	// minimized button
	strcpy						(path,"main_wnd:global_map:background:minimized_btn");
	CUIButton* minimized_btn	= xr_new<CUIButton>();
	minimized_btn->SetWindowName("minimized_btn");
	xml_init.InitButton			(uiXml, path, 0, minimized_btn);
	minimized_btn->SetAutoDelete(true);
	background->AttachChild		(minimized_btn);

	// maximized button
	strcpy						(path,"main_wnd:global_map:background:maximized_btn");
	CUIButton* maximized_btn	= xr_new<CUIButton>();
	maximized_btn->SetWindowName("maximized_btn");
	xml_init.InitButton			(uiXml, path, 0, maximized_btn);
	maximized_btn->SetAutoDelete(true);
	background->AttachChild		(maximized_btn);

	SwitchTo					(stNormal);
	//init callbacks
	Register(minimized_btn);
	Register(maximized_btn);
	AddCallback			("minimized_btn",BUTTON_CLICKED,boost::bind(&CUIGlobalMap::OnBtnMinimizedClick,this));
	AddCallback			("maximized_btn",BUTTON_CLICKED,boost::bind(&CUIGlobalMap::OnBtnMaximizedClick,this));
}


void CUIGlobalMap::SwitchTo(CUIGlobalMap::EState new_state)
{
	if (new_state!=m_State){
		CUIStatic* background	= GetControl<CUIStatic>("background"); VERIFY(background);
		switch (new_state){
		case stMinimized:		background->SetWndRect(m_MinimizedRect);			break;
		case stNormal:			background->SetWndRect(m_NormalRect);				break;
		case stMaximized:		background->SetWndRect(GetParent()->GetWndRect());	break;
		}
		m_State					= new_state;
	}
}

void CUIGlobalMap::OnBtnMinimizedClick()
{
	switch (m_State){
	case stMinimized:								break;
	case stNormal:			SwitchTo(stMinimized);	break;
	case stMaximized:		SwitchTo(stNormal);		break;
	}
}

void CUIGlobalMap::OnBtnMaximizedClick()
{
	switch (m_State){
	case stMinimized:		SwitchTo(stNormal);		break;
	case stNormal:			SwitchTo(stMaximized);	break;
	case stMaximized:								break;
	}
}
//////////////////////////////////////////////////////////////////////////

CUIMapWnd::CUIMapWnd()
{
	m_GlobalMap				= xr_new<CUIGlobalMap>();
	m_GlobalMap->SetAutoDelete(true);
	AttachChild				(m_GlobalMap);
}

CUIMapWnd::~CUIMapWnd()
{
}

void CUIMapWnd::Init()
{
	CUIXml uiXml;
	bool xml_result			= uiXml.Init(CONFIG_PATH, UI_PATH, "pda_map.xml");
	R_ASSERT3(xml_result, "xml file not found", "global_map.xml");

	// load map background
	CUIXmlInit xml_init;
	xml_init.InitWindow		(uiXml, "main_wnd", 0, this);

	m_GlobalMap->Init		();
}

void CUIMapWnd::Show(bool status)
{
	if (status)
	{
		InitGlobalMapObjectives			();
		InitLocalMapObjectives			();
//		SwitchMapMode					(emmLocal);
//		SetLocalMap						(Level().name());
	}
	if (status)	inherited::Show();
	else		inherited::Hide();
}

void CUIMapWnd::Draw()
{
	inherited::Draw();
}
/*
using namespace InventoryUtilities;

#define				MAP_LEFT					89
#define				MAP_TOP						93
#define				MAP_RADIUS					80


//-----------------------------------------------------------------------------/
//	�������� ���������� ����� � game.ltx ������ ui_map_dims
//	����, �����, ���, ����
//-----------------------------------------------------------------------------/

const char * const	PDA_MAP_XML					= "map.xml";
const				SCROLLBARS_SHIFT			= 5;
const				VSCROLLBAR_STEP				= 20; // � ��������
const				HSCROLLBAR_STEP				= 20; // � ��������

//-----------------------------------------------------------------------------/
//	Global map stuff
//-----------------------------------------------------------------------------/

const char * const	GLOBAL_MAP_LOCATIONS_LTX	= "global_map_locations";

const u32			activeLocalMapColor			= 0xffc80000;
const u32			inactiveLocalMapColor		= 0xff438cd1;
const u32			inactiveSmallLocalMapColor	= 0x00000000;
const u32			ourLevelMapColor			= 0xffffffff;
const int			globalMapButtonWidth		= 128;
const int			globalMapButtonHeight		= 256;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIMapWnd::CUIMapWnd()
	:	m_eCurrentMode		(emmLocal),
		m_pCurrentMap		(&UILocalMapBackground),
		m_GlobalMapHeight	(0.0f),
		m_GlobalMapWidth	(0.0f),
		m_GlobalMapShader	(NULL)
{
	Show					(false);
	SetFont					(HUD().Font().pFontSmall);
	m_rDefault.left			= 0;
	m_rDefault.top			= 0;
	m_rDefault.bottom		= 100;
	m_rDefault.right		= 100;
}

//////////////////////////////////////////////////////////////////////////

CUIMapWnd::~CUIMapWnd()
{
	UILocalMapBackground.RemoveAllSpots	();
	UIGlobalMapBackground.RemoveAllSpots();
	DeleteLocalMapsData					();
	m_GlobalMapShader.destroy			();
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::Show(bool status)
{
	if (status)
	{
		InitGlobalMapObjectives			();
		InitLocalMapObjectives			();
		m_eCurrentMode = emmGlobal;
		SwitchMapMode					(emmLocal);
		SetLocalMap						(Level().name());
	}
	inherited::Show(status);
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::Init()
{
	CUIXmlInit xml_init;
	CUIXml uiXml;
	uiXml.Init(CONFIG_PATH, UI_PATH, PDA_MAP_XML);

	inherited::Init(0, 0, UI_BASE_WIDTH, UI_BASE_HEIGHT);

	AttachChild(&UIMainMapFrame);
	xml_init.InitFrameWindow(uiXml, "frame_window", 0, &UIMainMapFrame);
	UIMainMapFrame.AttachChild(&UIPDAHeader);
	xml_init.InitFrameLine(uiXml, "map_header_frame_line", 0, &UIPDAHeader);

	string128 buf;
	strconcat(buf, ALL_PDA_HEADER_PREFIX, "/Map");

	UIPDAHeader.UITitleText.SetText(buf);

	//���������� � ����� �� ����� �� ������� ������� �����
	AttachChild(&UIStaticInfo);
	xml_init.InitStatic(uiXml, "static", 0, &UIStaticInfo);

	UIStaticInfo.AttachChild(&UICharacterInfo);
	UICharacterInfo.Init(0,0, UIStaticInfo.GetWidth(), UIStaticInfo.GetHeight(), "map_character.xml");
	UIStaticInfo.Enable(false);
	
	UIMainMapFrame.AttachChild(&UILocalMapBackground);
	xml_init.InitWindow(uiXml, "window", 0, &UILocalMapBackground);
	UILocalMapBackground.SetMessageTarget(this);
	UILocalMapBackground.Enable(true);

	UIMainMapFrame.AttachChild(&UIGlobalMapBackground);
	xml_init.InitWindow(uiXml, "window", 0, &UIGlobalMapBackground);
	UIGlobalMapBackground.SetMessageTarget(this);
	UIGlobalMapBackground.Enable(true);
	UIMainMapFrame.DetachChild(&UIGlobalMapBackground);

	// Descriptions
	UIGlobalMapBackground.AttachChild(&UIMapName);
	xml_init.InitStatic(uiXml, "map_name_static", 0, &UIMapName);
	UIMapName.Enable(false);
	UIMapName.Show(false);

	UIGlobalMapBackground.AttachChild(&UIMapGoals);
	xml_init.InitListWnd(uiXml, "map_goals_list", 0, &UIMapGoals);
	UIMapGoals.Enable(false);
	UIMapGoals.Show(false);

	RECT r = UILocalMapBackground.GetWndRect();

	UIMainMapFrame.AttachChild(&UIMapBgndV);
	UIMapBgndV.Init(r.right + SCROLLBARS_SHIFT, r.top, r.bottom - r.top, false);
	UIMapBgndV.SetMessageTarget(this);

	UIMainMapFrame.AttachChild(&UIMapBgndH);
	UIMapBgndH.Init(r.left, r.bottom + SCROLLBARS_SHIFT, r.right - r.left, true);
	UIMapBgndH.SetMessageTarget(this);

	// ������ ������������ ����������/��������� �����
	UILocalMapBackground.AttachChild(&UIMapTypeSwitch);
	xml_init.InitButton(uiXml, "gl_button", 0, &UIMapTypeSwitch);
	UIMapTypeSwitch.SetMessageTarget(this);

	// ��� ������� ��������� �����
	UILocalMapBackground.AttachChild(&UILocalMapName);
	xml_init.InitStatic(uiXml, "local_map_name_static", 0, &UILocalMapName);
	UILocalMapName.Enable(false);

	//�������� ��������������� ����������
	xml_init.InitAutoStatic(uiXml, "auto_static", this);

	// �������������� �����
	InitMaps();
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::SetLocalMap(const shared_str &levelName)
{
	UILocalMapBackground.m_pActiveMapSpot = NULL;

	LocalMapsPairIt currentMapIt = m_LocalMaps.find(levelName);
	R_ASSERT3(m_LocalMaps.end() != currentMapIt, "Map data for current level is absent.\nMost possible reason - this level not included in game graph", *levelName);

	R_ASSERT(currentMapIt->second.mapTexture);
	UILocalMapBackground.InitMapBackground(currentMapIt->second.mapTexture);

	// ������ ������� ��� �����
	UILocalMapName.SetText(*CStringTable()(levelName));
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::InitLocalMapObjectives()
{
	///������ ��� ������������� ������� �� ����� 
	UILocalMapBackground.RemoveAllSpots();

	Ivector2 P;
	Fvector src;

	CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());

	if (!pActor) return;

	int left	= UILocalMapBackground.GetWndRect().left;
	int top		= UILocalMapBackground.GetWndRect().top;

	////////////////////////////////////////////
	//�������� �������, ��������� � ������ PDA
	////////////////////////////////////////////

	for(LOCATIONS_PTR_VECTOR_IT it = Level().MapLocations().begin();
		Level().MapLocations().end() != it; 
		it++)
	{
		SMapLocation& map_location = *(*it);
		if(map_location.type_flags.test(eMapLocationPDAContact))	
			continue;

		
		CUIMapSpot * map_spot;
		map_spot = xr_new<CUIMapSpot>();
		
		if(map_location.attached_to_object)
		{
			map_spot->SetObjectID(map_location.object_id);
			src = map_spot->MapPos();
		}
		else
		{
			src.x = map_location.x;
			src.y = 0;
			src.z = map_location.y;

			map_spot->SetObjectID(0xffff);
			map_spot->m_vWorldPos.set(map_location.x,0.f,map_location.y);
		}

		map_spot->arrow_color		= map_location.icon_color;
		map_spot->m_bArrowEnabled	= map_location.marker_show;
		map_spot->m_LevelName		= map_location.level_name;

		UILocalMapBackground.ConvertToLocal(src,P);

		if (map_location.GetShader())
			map_spot->SetShader(map_location.GetShader());
		else
        	map_spot->SetShader(GetMapIconsShader());

		map_spot->MoveWindow(P.x + left, P.y + top);
		map_spot->SetWidth(MAP_ICON_WIDTH*map_location.icon_width);
		map_spot->SetHeight(MAP_ICON_HEIGHT*map_location.icon_height);
		map_spot->GetUIStaticItem().SetOriginalRect(
			map_location.icon_x*MAP_ICON_GRID_WIDTH,
			map_location.icon_y*MAP_ICON_GRID_HEIGHT,
			map_location.icon_width*(MAP_ICON_GRID_WIDTH + MAP_ICON_WIDTH),
			map_location.icon_height*(MAP_ICON_GRID_HEIGHT + MAP_ICON_HEIGHT));

		map_spot->m_eAlign = CUIMapSpot::eCenter;
		map_spot->m_sDescText.SetText("");
		map_spot->m_sNameText.SetText("");
		map_spot->m_bArrowEnabled = true;
		if (map_location.dynamic_manifestation) map_spot->DynamicManifestation(true);

		if(xr_strlen(map_location.text)>1)
		{
			map_spot->m_sDescText.SetText(*map_location.text);
			map_spot->m_sNameText.SetText(*map_location.name);
		}

		UILocalMapBackground.m_vMapSpots.push_back(CUIMapBackground::MapSpotPtr(map_spot));
		UILocalMapBackground.AttachChild(map_spot);
	}

	//	�������� �� ����� ������ ������ ���� �� ���������� ������� �����

	UILocalMapBackground.ConvertToLocal(pActor->Position(),P);
	CUIMapSpot* map_spot;

	map_spot = xr_new<CUIMapSpot>();
	map_spot->SetObjectID(pActor->ID());

	// viewport
	float h,p;
	Device.vCameraDirection.getHP	(h,p);
	map_spot->m_fHeading = h;
	map_spot->m_bHeading = true;

	map_spot->InitTexture(pSettings->r_string("game_map", "actor_arrow"));
	map_spot->SetWidth(MAP_ICON_WIDTH);
	map_spot->SetHeight(MAP_ICON_HEIGHT);
	map_spot->SetObjectID(0xffff);

	map_spot->MoveWindow(P.x + left, P.y + top);
	map_spot->m_eAlign = CUIMapSpot::eCenter;
	map_spot->m_sDescText.SetText("It's you.");
	map_spot->m_sNameText.SetText("You");
	map_spot->m_vWorldPos.set(pActor->Position());
	map_spot->m_LevelName = Level().name();

	UILocalMapBackground.m_vMapSpots.push_back(CUIMapBackground::MapSpotPtr(map_spot));
	UILocalMapBackground.AttachChild(map_spot);
	m_pActorSpot = map_spot;

	//!!! ��������������� ������ ���������� �����
//	m_pActorSpot->GetStaticItem()->Render();

	//���������� � ��������� �������
	UIStaticInfo.Show(false);
	UICharacterInfo.Show(false);

	this->ApplyFilterToObjectives(Level().name());    
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::InitGlobalMapObjectives()
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());

	if (!pActor) return;
	LocalMapsPairIt mapIt;

	for (mapIt = m_LocalMaps.begin(); mapIt != m_LocalMaps.end(); ++mapIt)
	{
		LocalMapDef &l = mapIt->second;
		R_ASSERT				(l.mapSpot);
		l.obj.clear				();
		l.mapSpot->SetColor		(inactiveLocalMapColor);
		l.smallFrame->SetColor	(inactiveSmallLocalMapColor);
	}

	// �������� ������� �� �����, ������� ����� ������ � info_portions
	if(pActor->m_known_info_registry->registry().objects_ptr())
	{
		const KNOWN_INFO_VECTOR& know_info = *pActor->m_known_info_registry->registry().objects_ptr();
		for(KNOWN_INFO_VECTOR::const_iterator it = know_info.begin();
			know_info.end() != it; it++)
		{
			CInfoPortion info_portion;
			info_portion.Load((*it).id);

			// �������� ������� �� �����
			for(u32 i=0; i<info_portion.MapLocations().size(); i++)
			{
				const SMapLocation& map_location = info_portion.MapLocations()[i];
				if(xr_strlen(*map_location.level_name) > 0)
				{
					mapIt = m_LocalMaps.find(map_location.level_name);
					if (m_LocalMaps.end() != mapIt)
				 	{
						if(map_location.global_map_text)
							mapIt->second.obj.push_back(map_location.text);
						RECT r	= mapIt->second.smallFrame->GetWndRect();
						r.top	= globalMapButtonHeight - r.top;
						mapIt->second.visible = !(r.left == m_rDefault.left && r.top == m_rDefault.top);
					}
				}
			}
		}
	}

	// ���� ���� ������� �������/������� �� ����� �� ������� �� ��������� ���, �� ��� ����� ��������
	// �����
	mapIt = m_LocalMaps.find(Level().name());
	if (m_LocalMaps.end() != mapIt)
	{
		RECT r	= mapIt->second.smallFrame->GetWndRect();
		r.top	= globalMapButtonHeight - r.top;
		mapIt->second.visible = !(r.left == m_rDefault.left && r.top == m_rDefault.top);
		mapIt->second.smallFrame->SetColor(ourLevelMapColor);
		mapIt->second.mapSpot->SetColor(ourLevelMapColor);
	}

	// �������� ��������� ��� �������
	for (mapIt = m_LocalMaps.begin(); mapIt != m_LocalMaps.end(); ++mapIt)
	{
		LocalMapDef &l = mapIt->second;
		if (l.visible)
		{
			if (!UIGlobalMapBackground.IsChild(l.mapSpot.get()))
				UIGlobalMapBackground.AttachChild(l.mapSpot.get());
			if (!UIMapTypeSwitch.IsChild(l.smallFrame))
				UIMapTypeSwitch.AttachChild(l.smallFrame);
		}
		else
		{
			if (UIGlobalMapBackground.IsChild(l.mapSpot.get()))
				UIGlobalMapBackground.DetachChild(l.mapSpot.get());
			if (UIMapTypeSwitch.IsChild(l.smallFrame))
				UIMapTypeSwitch.DetachChild(l.smallFrame);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::AddObjectSpot(CGameObject* pGameObject)
{	
	//!!! ���� ������ ��� CEntity (���� ���� ������ ����� ������)
	CEntity* pEntity = smart_cast<CEntity*>(pGameObject);
	if(!pEntity) return;


	int left	= UILocalMapBackground.GetWndRect().left;
	int top		= UILocalMapBackground.GetWndRect().top;

	Ivector2 P;
    UILocalMapBackground.ConvertToLocal(pGameObject->Position(),P);
	
	CUIMapSpot* map_spot;
	map_spot = xr_new<CUIMapSpot>();
	map_spot->SetObjectID(pEntity->ID());
	
	map_spot->SetShader(GetCharIconsShader());
	map_spot->SetWidth(MAP_ICON_WIDTH);
	map_spot->SetHeight(MAP_ICON_HEIGHT);
	map_spot->MoveWindow(P.x + left, P.y + top);
	map_spot->m_eAlign = CUIMapSpot::eCenter;
	map_spot->m_sDescText.SetText("");
	map_spot->m_sNameText.SetText("");
	map_spot->m_LevelName = Level().name();

	map_spot->GetUIStaticItem().SetOriginalRect(
						pEntity->GetMapIconX()*ICON_GRID_WIDTH + MAP_ICON_WIDTH / 2,
						pEntity->GetMapIconY()*ICON_GRID_HEIGHT + MAP_ICON_HEIGHT,
						(pEntity->GetMapIconX()+1)*ICON_GRID_WIDTH,
						(pEntity->GetMapIconY()+1)*ICON_GRID_HEIGHT);

	UILocalMapBackground.m_vMapSpots.push_back(CUIMapBackground::MapSpotPtr(map_spot));
	UILocalMapBackground.AttachChild(map_spot);
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	static u16 prevScrollPosV = 0;
	static u16 prevScrollPosH = 0;

	CStringTable	stbl;

	if(pWnd == &UILocalMapBackground){
		if (MAPSPOT_FOCUS_RECEIVED == msg || MAPSPOT_ARROW_FOCUS_RECEIVED == msg){
			if (m_pCurrentMap->m_pActiveMapSpot){
				UIStaticInfo.Show(true);
				RECT r	= m_pCurrentMap->m_pActiveMapSpot->GetAbsoluteRect();
				RECT r2	= UIStaticInfo.GetParent()->GetAbsoluteRect();
				
				int left = r.right - r2.left;
				int top = r.top + 2 - r2.top;
				UIStaticInfo.MoveWindow(left, top);
			
				//CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(m_pCurrentMap->m_pActiveMapSpot->m_pObject);

				if(xr_strlen(m_pCurrentMap->m_pActiveMapSpot->m_sDescText.GetBuf())>1){
					UICharacterInfo.Show(true);
					UICharacterInfo.ResetAllStrings();

					UICharacterInfo.UIName.SetText(m_pCurrentMap->m_pActiveMapSpot->m_sNameText);
					UICharacterInfo.UIRank.SetText(m_pCurrentMap->m_pActiveMapSpot->m_sDescText);
					
				}else{
					UICharacterInfo.ResetAllStrings();
					UICharacterInfo.Show(false);
				}
            }
		}else if(MAPSPOT_FOCUS_LOST == msg || MAPSPOT_ARROW_FOCUS_LOST == msg){
			UIStaticInfo.Show(false);
		}
	}

	if (&UIGlobalMapBackground == pWnd){
		static CUIMapSpot *prevSpot = NULL;
		if (MAPSPOT_FOCUS_RECEIVED == msg){
			if (UIGlobalMapBackground.m_pActiveMapSpot && prevSpot != UIGlobalMapBackground.m_pActiveMapSpot){
				CUIGlobalMapLocation *pGML = smart_cast<CUIGlobalMapLocation*>(UIGlobalMapBackground.m_pActiveMapSpot);
				R_ASSERT(pGML);
				pGML->SetColor(activeLocalMapColor);
				UIMapName.SetText(*stbl(pGML->m_strMapName));
				// Show map goals
				for (Objectives_it it = m_LocalMaps[pGML->m_strMapName].obj.begin(); it != m_LocalMaps[pGML->m_strMapName].obj.end(); ++it)
				{
					UIMapGoals.AddItem<CUIListItem>(static_cast<const char *>(*(*it)));
				}
				prevSpot = UIGlobalMapBackground.m_pActiveMapSpot;
			}
			UIMapGoals.Show(true);
			UIMapName.Show(true);
		}

		if (MAPSPOT_FOCUS_LOST == msg){
			UIMapGoals.Show(false);
			UIMapGoals.RemoveAll();
			prevSpot = NULL;
			if (UIGlobalMapBackground.m_pActiveMapSpot){
				CUIGlobalMapLocation *pGML = smart_cast<CUIGlobalMapLocation*>(UIGlobalMapBackground.m_pActiveMapSpot);
				R_ASSERT(pGML);
				if (Level().name() == pGML->m_strMapName)
					pGML->SetColor(ourLevelMapColor);
				else
					pGML->SetColor(inactiveLocalMapColor);
			}
			UIMapName.Show(false);
		}

		if (MAPSPOT_CLICKED == msg){
			CUIGlobalMapLocation *pGML = smart_cast<CUIGlobalMapLocation*>(UIGlobalMapBackground.m_pActiveMapSpot);
			if (pGML){
				SetLocalMap(pGML->m_strMapName);
				SwitchMapMode(emmLocal);
			}
		}
	}

	if (&UIMapBgndV == pWnd){
		if (SCROLLBAR_VSCROLL == msg){
			m_pCurrentMap->MoveMap(0, -VSCROLLBAR_STEP * (UIMapBgndV.GetScrollPos() - prevScrollPosV));
			prevScrollPosV = UIMapBgndV.GetScrollPos();
			m_pCurrentMap->Update();
		}
	}

	if (&UIMapBgndH == pWnd){
		if (SCROLLBAR_HSCROLL == msg){
			m_pCurrentMap->MoveMap(-HSCROLLBAR_STEP * (UIMapBgndH.GetScrollPos() - prevScrollPosH), 0);
			prevScrollPosH = UIMapBgndH.GetScrollPos();
			m_pCurrentMap->Update();
		}
	}

	if (m_pCurrentMap == pWnd && MAP_MOVED == msg){
		Fvector2	dest;
		m_pCurrentMap->ConvertFromLocalToMap(HSCROLLBAR_STEP, VSCROLLBAR_STEP, dest);
		if (UIMapBgndV.IsShown())
			UIMapBgndV.SetScrollPos(static_cast<s16>(UIMapBgndV.GetMaxRange() - iCeil((m_pCurrentMap->m_fMapHeightMeters - m_pCurrentMap->m_fMapViewHeightMeters - m_pCurrentMap->m_fMapY) / dest.y)));
		if (UIMapBgndH.IsShown())
			UIMapBgndH.SetScrollPos(static_cast<s16>(UIMapBgndH.GetMaxRange() - iCeil((m_pCurrentMap->m_fMapWidthMeters - m_pCurrentMap->m_fMapViewWidthMeters - m_pCurrentMap->m_fMapX) / dest.x)));

		prevScrollPosH = UIMapBgndH.GetScrollPos();
		prevScrollPosV = UIMapBgndV.GetScrollPos();
		m_pCurrentMap->Update();
	}
	
	if (&UIMapTypeSwitch == pWnd && BUTTON_CLICKED == msg){
		SwitchMapMode(emmGlobal);
		for (WINDOW_LIST_it it = UIMapTypeSwitch.GetChildWndList().begin();
							it != UIMapTypeSwitch.GetChildWndList().end();
							++it)
		{
			RECT r = (*it)->GetWndRect();
			(*it)->MoveWindow(r.left - UIMapTypeSwitch.GetPushOffsetX(), r.top - UIMapTypeSwitch.GetPushOffsetY());
		}
	}else if (&UIMapTypeSwitch == pWnd && BUTTON_DOWN == msg){
		for (WINDOW_LIST_it it = UIMapTypeSwitch.GetChildWndList().begin();
							it != UIMapTypeSwitch.GetChildWndList().end();
							++it)
		{
			RECT r = (*it)->GetWndRect();
			(*it)->MoveWindow(r.left + UIMapTypeSwitch.GetPushOffsetX(), r.top + UIMapTypeSwitch.GetPushOffsetY());
		}
	}

	inherited::SendMessage(pWnd, msg, pData);
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::Update()
{
	float h,p;

	Device.vCameraDirection.getHP	(h,p);
	m_pActorSpot->m_fHeading = h;
	CActor* pActor = smart_cast<CActor*>(Level().CurrentEntity());
	m_pActorSpot->m_vWorldPos.set(pActor->Position());

	inherited::Update();
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::Draw()
{
	inherited::Draw();
}

//////////////////////////////////////////////////////////////////////////
//	������� �� ������� ������� ��������� � ���������� �����
//	� ������� ��������� � ����� ������� ����
//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::ConvertToLocal(const Fvector& src, Ivector2& dest)
{
	dest.x = iFloor(0.5f + m_iMapWidth*(src.x - m_fWorldMapLeft)/m_fWorldMapWidth);
	dest.y = m_iMapHeight - iFloor(0.5f + m_iMapHeight*(src.z - m_fWorldMapTop)/m_fWorldMapHeight);
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::SetActivePoint(const Fvector &vNewPoint)
{
	UILocalMapBackground.SetActivePos(vNewPoint);
	UILocalMapBackground.m_bNoActorFocus = true;
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::SwitchMapMode(const EMapModes mode)
{
	// ��������� �� ������������ � ������� �����
	if (mode == m_eCurrentMode) return;

	m_eCurrentMode = mode;

	CStringTable st;

	UIMainMapFrame.DetachChild(m_pCurrentMap);

	switch (m_eCurrentMode)
	{
	case emmLocal:
//		UIMapTypeSwitch.SetText(*st("Global Map"));
		m_pCurrentMap = &UILocalMapBackground;
		UILocalMapBackground.Show(true);
		SendInfoToActor("ui_pda_map_local");
		break;
	case emmGlobal:
//		UIMapTypeSwitch.SetText(*st("Local Map"));
		m_pCurrentMap = &UIGlobalMapBackground;
		UIGlobalMapBackground.Show(true);
		SendInfoToActor("ui_pda_map_global");

		for (LocalMapsPairIt it = m_LocalMaps.begin(); it != m_LocalMaps.end(); ++it)
		{
			if (Level().name() == it->first)
				it->second.mapSpot->SetColor(ourLevelMapColor);
			else
				it->second.mapSpot->SetColor(inactiveLocalMapColor);
		}
		break;
	default:
		NODEFAULT;
	}    

	UIMainMapFrame.AttachChild(m_pCurrentMap);


	const Fbox& level_box	= Level().ObjectSpace.GetBoundingVolume();

	m_fWorldMapWidth = level_box.x2 - level_box.x1;
	m_fWorldMapHeight = level_box.z2 - level_box.z1;

	m_fWorldMapLeft = level_box.x1;
	m_fWorldMapTop = level_box.z2;

	Fvector2	dest;
	m_pCurrentMap->ConvertFromLocalToMap(HSCROLLBAR_STEP, VSCROLLBAR_STEP, dest);

	// ����������� �����������
	if (m_pCurrentMap->m_fMapHeightMeters > m_pCurrentMap->m_fMapViewHeightMeters)
	{
		UIMapBgndV.Show(true);
		UIMapBgndV.SetRange(0, static_cast<s16>(iCeil((m_pCurrentMap->m_fMapHeightMeters - m_pCurrentMap->m_fMapViewHeightMeters) / dest.y)));
	}
	else
		UIMapBgndV.Show(false);

	if (m_pCurrentMap->m_fMapWidthMeters > m_pCurrentMap->m_fMapViewWidthMeters)
	{
		UIMapBgndH.Show(true);
		UIMapBgndH.SetRange(0, static_cast<s16>(iCeil((m_pCurrentMap->m_fMapWidthMeters - m_pCurrentMap->m_fMapViewWidthMeters) / dest.x)));
	}
	else
		UIMapBgndH.Show(false);

	SendMessage(m_pCurrentMap, MAP_MOVED, NULL);

	m_iMapWidth = m_pCurrentMap->GetWidth();
	m_iMapHeight = m_pCurrentMap->GetHeight();
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::AddLocalMap(const shared_str& levelName, const Frect &l, const Frect &g, shared_str textureName)
{
	// If level with this name already exists
	R_ASSERT2	(m_LocalMaps.end() == m_LocalMaps.find(levelName), "Duplicate level name not allowed");
	LocalMapDef &currentLevelMap = m_LocalMaps[levelName];

	// ��� �������� ������ ��� ��������� ����
	const char * const localMapFrameTexture = "ui\\ui_frame_very_small";

	Ivector4	iTmpV;
	iTmpV[2]	= iFloor(v[2] * UIGlobalMapBackground.m_iMapViewWidthPixels	/ m_GlobalMapWidth);
	iTmpV[3]	= iFloor(v[3] * UIGlobalMapBackground.m_iMapViewHeightPixels	/ m_GlobalMapHeight);

	float		xSmallMapFactor		= m_GlobalMapWidth	/ globalMapButtonWidth;
	float		ySmallMapFactor		= m_GlobalMapHeight / globalMapButtonHeight;

	// Create local map rect on global map
	LocalMapDef::GMLptr pGML		(xr_new<CUIGlobalMapLocation>());
	pGML->Init(iTmpV[2], iTmpV[3], UIGlobalMapBackground.GetAbsoluteRect(), localMapFrameTexture);
	pGML->m_vWorldPos.x				= float(v[0]);
	pGML->m_vWorldPos.z				= float(v[1]);
	pGML->SetColor(inactiveLocalMapColor);
	pGML->m_strMapName				= CStringTable()(levelName);
	currentLevelMap.mapSpot			= pGML;
	currentLevelMap.level_box		= l;
	UIGlobalMapBackground.m_vMapSpots.push_back(pGML);

	// Add map locations to global map button
	iTmpV[0] = iFloor(v[0] / xSmallMapFactor);
	iTmpV[1] = iFloor(v[1] / ySmallMapFactor);
	iTmpV[2] = iFloor(v[2] / xSmallMapFactor);
	iTmpV[3] = iFloor(v[3] / ySmallMapFactor);

	currentLevelMap.smallFrame = xr_new<CUIFrameWindow>();
	currentLevelMap.smallFrame->Init(localMapFrameTexture, iTmpV[0], globalMapButtonHeight - iTmpV[1], iTmpV[2], iTmpV[3]);
	currentLevelMap.smallFrame->SetColor(inactiveLocalMapColor);

	// Initialize map itself
	currentLevelMap.mapTexture.create("hud\\default", *textureName);
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::InitMaps()
{
	string256			gameLtxPath;
	FS.update_path		(gameLtxPath, CONFIG_PATH, "game.ltx");
	CInifile gameLtx	(gameLtxPath);

	// �������������� ��������� ���������� �����
	m_GlobalMapWidth	= gameLtx.r_float("ui_global_map", "width");
	m_GlobalMapHeight	= gameLtx.r_float("ui_global_map", "height");

	UIGlobalMapBackground.InitMapBackground(m_GlobalMapShader);
	
	// initialize local maps
	if (gameLtx.section_exist("ui_level_maps_single")){
		CInifile::Sect& S		= gameLtx.r_section("ui_level_maps_single");
		CInifile::SectIt	it	= S.begin(), end = S.end();
		for (;it!=end; it++){
			LPCSTR line	= *it->second;
			string_path	t_name;
			Frect		localMapCoords;
			Frect		globalMapCoords;
			u32 v_cnt	= sscanf(line,"%[^,],%f,%f,%f,%f,%f,%f,%f,%f",t_name,
							&localMapCoords.x1,&localMapCoords.y1,	&localMapCoords.x2,&localMapCoords.y2,
							&globalMapCoords.x1,&globalMapCoords.y1,&globalMapCoords.x2,&globalMapCoords.y2);
			R_ASSERT	(v_cnt==9);
			AddLocalMap	(it->first, localMapCoords, globalMapCoords, t_name);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::DeleteLocalMapsData()
{
	for (LocalMapsPairIt it = m_LocalMaps.begin(); it != m_LocalMaps.end(); ++it){
		if (it->second.smallFrame->GetParent())
			it->second.smallFrame->GetParent()->DetachChild(it->second.smallFrame);
		xr_delete(it->second.smallFrame);
	}

	m_LocalMaps.clear();
}

//////////////////////////////////////////////////////////////////////////

void CUIMapWnd::ApplyFilterToObjectives(const shared_str &levelName)
{
	for (CUIMapBackground::MapSpotPtrVecIt mapSpotIt = UILocalMapBackground.m_vMapSpots.begin();
											  mapSpotIt != UILocalMapBackground.m_vMapSpots.end();
											  ++mapSpotIt)
	{
		(*mapSpotIt)->Show(levelName == (*mapSpotIt)->m_LevelName);
	}	
}
void CUIMapWnd::OnMouseWheel(int direction)
{
	if(direction<0)
		UIMapBgndV.TryScrollInc();
	else
		UIMapBgndV.TryScrollDec();
}
*/