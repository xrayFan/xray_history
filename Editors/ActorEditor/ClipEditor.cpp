#include "stdafx.h"
#pragma hdrstop

#include "ClipEditor.h"
#include <ElVCLUtils.hpp>
#include <ElTools.hpp>

#include "ElTree.hpp"
#include "ItemList.h"
#include "motion.h"
#include "skeletoncustom.h"
#include "editobject.h"
#include "UI_Tools.h"
#include "UI_Main.h"
#include "SkeletonAnimated.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "mxPlacemnt"
#pragma link "mxPlacemnt"
#pragma link "ExtBtn"
#pragma link "MXCtrls"
#pragma link "Gradient"
#pragma link "ElScrollBar"
#pragma link "ElXPThemedControl"
#pragma link "MxMenus"
#pragma link "ElPanel"
#pragma link "ElSplit"
#pragma link "multi_edit"
#pragma resource "*.dfm"

static const TColor CLIP_INACTIVE_COLOR		= 0x00686868;
static const TColor CLIP_ACTIVE_COLOR		= 0x00C1C1C1;
static const TColor BP_INACTIVE_COLOR		= 0x00686868;
static const TColor BP_ACTIVE_COLOR			= 0x00C1C1C1;

static int g_ClipID=0;

TClipMaker::SClip::SClip(const AnsiString& n, TClipMaker* own, float r_t)
{
	owner				= own;
	run_time			= r_t;
	length				= 2.f;
//    int min_size		= 1;
    cycles[0]=cycles[1]=cycles[2]=cycles[3]="";
    name				= n;
/*
    // panel
    panel		 		= xr_new<TPanel>(owner->paClips);
    panel->Tag			= (int)this;
    panel->Alignment	= taLeftJustify;
    panel->Caption		= " "+name;
    panel->Align		= alNone;
    panel->Left			= PLeft();
    panel->Width		= PWidth();
    panel->Height		= owner->paClips->Height;
//    panel->Constraints->MinWidth = min_size;
    panel->ShowHint		= true;
    panel->Hint			= "Clip '"+name+"'";
//    panel->Hint			= panel->Caption;
    panel->Color		= CLIP_INACTIVE_COLOR;
    panel->BevelInner	= bvLowered;//bvNone;
    panel->BevelOuter	= bvRaised;//bvNone;
    panel->BorderStyle	= bsNone;
    panel->DragMode		= dmManual;
    panel->OnDragOver	= owner->ClipDragOver;
    panel->OnDragDrop	= owner->ClipDragDrop;
    panel->OnMouseDown	= owner->ClipMouseDown;
    panel->OnMouseMove	= owner->ClipMouseMove;
    panel->OnMouseUp	= owner->ClipMouseUp;
    panel->Parent 		= owner->paClips;
*/    
    idx					= -1;
}

TClipMaker::SClip::~SClip()
{
};

IC bool clip_pred(TClipMaker::SClip* x, TClipMaker::SClip* y)
{
	return x->run_time<y->run_time;
};

IC bool clip_pred_float(float x, TClipMaker::SClip* y)
{
	return x<y->run_time;
};

TClipMaker*	TClipMaker::CreateForm()
{
	return xr_new<TClipMaker>((TComponent*)0);
}

void TClipMaker::DestroyForm(TClipMaker* form)
{
	xr_delete(form);
}
    
void TClipMaker::ShowEditor(CEditableObject* O)
{
	m_CurrentObject = O; VERIFY(O);
	Show			();
    UpdateClips		();
    UpdateProperties();
}

void TClipMaker::HideEditor()
{
	Clear			();
	Hide			();
}

void TClipMaker::Clear()
{
	m_RTFlags.zero	();
	m_CurrentObject = 0;
	for (ClipIt it=clips.begin(); it!=clips.end(); it++)
    	xr_delete	(*it);
    clips.clear		();
    sel_clip		= 0;
    g_ClipID		= 0;
}

__fastcall TClipMaker::TClipMaker(TComponent* Owner) : TForm(Owner)
{
    DEFINE_INI		(fsStorage);
    m_LB[0]    		= lbBPName0;
    m_LB[1]    		= lbBPName1;
    m_LB[2]    		= lbBPName2;
    m_LB[3]    		= lbBPName3;
    m_TotalLength	= 0.f;
    m_Zoom			= 24.f;
    m_CurrentPlayTime=0.f;
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::FormCreate(TObject *Sender)
{
	m_ClipProps		= TProperties::CreateForm("Clip Properties",paClipProps,alClient);
	m_ClipList		= TItemList::CreateForm("Clips",paClipList,alClient,0);
	m_ClipList->OnItemsFocused	= OnClipItemFocused;

	Device.seqFrame.Add	(this);
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::FormDestroy(TObject *Sender)
{
	Device.seqFrame.Remove(this);
	Clear			();
	TProperties::DestroyForm(m_ClipProps);
	TItemList::DestroyForm(m_ClipList);
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::FormShow(TObject *Sender)
{
	UI.CheckWindowPos(this);
}
//---------------------------------------------------------------------------

TClipMaker::SClip* TClipMaker::FindClip(int x)
{
	return FindClip(float(x)/m_Zoom);
}
//---------------------------------------------------------------------------

TClipMaker::SClip* TClipMaker::FindClip(float t)
{
	if (clips.empty()) return 0;
	ClipIt it = std::upper_bound(clips.begin(),clips.end(),t,clip_pred_float);
    VERIFY (it!=clips.begin());
    it--;
    return *it;
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ClipDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Accept = false;
	TElTreeDragObject* obj 	= dynamic_cast<TElTreeDragObject*>(Source);
    if (obj){
        TMxPanel* A			= dynamic_cast<TMxPanel*>(Sender);
        if (A==paClips){
            TElTree* tv		= dynamic_cast<TElTree*>(obj->Control);
            if (tv->SelectedCount){
                for (TElTreeItem* item = tv->GetNextSelected(0); item; item = tv->GetNextSelected(item)){
                    ListItem* prop		= (ListItem*)item->Tag;
					if (prop&&(prop->Type()==emMotion)){
                        Accept			= true;
                    }
                }
            }
        }
    }else{
		TPanel* P 	= dynamic_cast<TPanel*>(Source);
        if (P&&(Sender!=Source)){
            Accept=true;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ClipDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
	TElTreeDragObject* obj 	= dynamic_cast<TElTreeDragObject*>(Source);
    if (obj){
        TPanel* Pd			= dynamic_cast<TPanel*>(Sender); VERIFY(Pd);
        if (Pd->Tag!=-1){
	        SClip* Cd		= (SClip*)Pd->Tag;
            TElTree* tv		= dynamic_cast<TElTree*>(obj->Control);
            if (tv->SelectedCount){
                for (TElTreeItem* item = tv->GetNextSelected(0); item; item = tv->GetNextSelected(item)){
                    ListItem* prop	= (ListItem*)item->Tag; VERIFY(prop);
                    CSMotion* SM 	= (CSMotion*)prop->m_Object;
                    LPCSTR mname 	= SM->Name();
                    if (!SM->m_Flags.is(esmFX)){
                    	if (SM->m_BoneOrPart==BI_NONE){
	                        Cd->cycles[0]=mname;
	                        Cd->cycles[1]=mname;
	                        Cd->cycles[2]=mname;
	                        Cd->cycles[3]=mname;
                        }else{
	                        Cd->cycles[SM->m_BoneOrPart]=mname;
                        }
                    }
                }
            }
        }
    }else{
        TExtBtn* trash = dynamic_cast<TExtBtn*>(Sender);
        if (trash&&(trash==ebTrash)){
        	RemoveClip		(sel_clip);
        }else{
            SClip* tgt			= FindClip(X);
            sel_clip->run_time	= tgt->run_time-EPS_L;
//            if (X<)	
//            else				sel_clip->run_time= Cd->run_time+EPS_L;
        }
    }
    UpdateClips		();
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::OnClipItemFocused(ListItemsVec& items)
{
	if (!items.empty()){
	    for (ListItemsIt it=items.begin(); it!=items.end(); it++){
            ListItem* prop = *it;
            m_ClipList->LockUpdating();
            SelectClip((SClip*)prop->m_Object);
            m_ClipList->UnlockUpdating();
        }
    }
}
//------------------------------------------------------------------------------

void __fastcall TClipMaker::OnNameChange(PropValue* V)
{
	VERIFY(sel_clip);
    RepaintClips();
}
//------------------------------------------------------------------------------

void __fastcall TClipMaker::OnClipLengthChange(PropValue* V)
{
	UpdateClips		();
}
//------------------------------------------------------------------------------

void __fastcall TClipMaker::OnZoomChange(PropValue* V)
{
	UpdateClips		();
}
//---------------------------------------------------------------------------

void TClipMaker::RealUpdateProperties()
{
	m_RTFlags.set	(flRT_UpdateProperties,FALSE);
    // clip props
    PropItemVec		p_items;
    PropValue* V	= 0;
	PHelper.CreateCaption		(p_items,"Length",				FloatTimeToStrTime(m_TotalLength,true,true,true,true));
    V=PHelper.CreateFloat		(p_items,"Zoom",					&m_Zoom,			1.f,1000.f,0.1f,1);
    V->OnChangeEvent			= OnZoomChange;
    if (sel_clip){
	    V=PHelper.CreateAText	(p_items,"Current Clip\\Name",	&sel_clip->name);
        V->OnChangeEvent		= OnNameChange;
	    V=PHelper.CreateFloat	(p_items,"Current Clip\\Length",	&sel_clip->length,	0.f,10000.f,0.1f,2);
        V->OnChangeEvent		= OnClipLengthChange;
        for (u32 k=0; k<4; k++){
            LPCSTR mname		= sel_clip->CycleName(k);	
            CSMotion* SM		= m_CurrentObject->FindSMotionByName(mname);
            SBonePart* BP		= (k<m_CurrentObject->BoneParts().size())?&m_CurrentObject->BoneParts()[k]:0;
            if (BP)				PHelper.CreateCaption(p_items,FHelper.PrepareKey("Current Clip\\Cycles",BP->alias.c_str()), SM?SM->Name():"-");//SM->m_Flags.is(esmStopAtEnd)?"Stop at end":"Looped" );
		}            
    }
	m_ClipProps->AssignItems(p_items,true);
}
//---------------------------------------------------------------------------

void TClipMaker::SelectClip(SClip* clip)
{
    sel_clip		= clip;
    AnsiString nm	= sel_clip?sel_clip->name:AnsiString("");
    m_ClipList->SelectItem(nm,true,false,true);
    RepaintClips	();
    UpdateProperties();
}

void TClipMaker::InsertClip()
{
	SClip* clip		= xr_new<SClip>(AnsiString("clip_")+g_ClipID++,this,sel_clip?sel_clip->RunTime()-EPS_L:0);
    clips.push_back	(clip);
    UpdateClips		(true,false);
    SelectClip		(clip);
}
//---------------------------------------------------------------------------

void TClipMaker::AppendClip()
{
	SClip* clip		= xr_new<SClip>(AnsiString("clip_")+g_ClipID++,this,sel_clip?sel_clip->RunTime()+sel_clip->Length()-EPS_L:0);
    clips.push_back	(clip);
    UpdateClips		(true,false);
    SelectClip		(clip);
}
//---------------------------------------------------------------------------

void TClipMaker::RemoveAllClips()
{
	SelectClip		(0);
	for (ClipIt it=clips.begin(); it!=clips.end(); it++)
    	xr_delete(*it);
    clips.clear		();
    UpdateClips		();
    Stop			();
}
//---------------------------------------------------------------------------

void TClipMaker::RemoveClip(SClip* clip)
{
	if (clip){
    	Stop		();
    	ClipIt it 	= std::find(clips.begin(),clips.end(),clip);
        if (it!=clips.end()){
        	ClipIt p_it	= it; p_it++;
            if ((p_it==clips.end())&&(clips.size()>1)){ p_it=it; p_it--;}
            SClip* C	= p_it==clips.end()?0:*p_it;
            xr_delete	(*it);
            clips.erase	(it);
            SelectClip	(C);
            UpdateClips	();
        }
    }
}
//---------------------------------------------------------------------------

static BOOL g_resizing	= FALSE;
static int 	g_X_prev	= 0;
static int 	g_X_cur	= 0;
void __fastcall TClipMaker::ClipMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    if (Button==mbLeft){
        SelectClip		(FindClip(X));
        SClip* C		= sel_clip; VERIFY(C);
        int cX			= X-C->PLeft();
        float w0		= float(cX)/C->PWidth();
        int w1 			= C->PWidth()-cX;
        if ((w0>0.75f)&&(w1<10)&&((w0<1.f))){
            g_resizing	= TRUE;
            Stop		();
            g_X_prev 	= X;
        }else{
            paClips->BeginDrag(false, 2);
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ClipMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    VERIFY				(sel_clip);
    
	TMxPanel* P			= dynamic_cast<TMxPanel*>(Sender);
    SClip* C			= sel_clip; VERIFY(C);
    int cX				= X-C->PLeft();
    float w0 			= float(cX)/C->PWidth();
    int w1	 			= C->PWidth()-cX;
	if ((w0>0.75f)&&(w1<10)&&((w0<1.f))){
    	P->Cursor 		= crHSplit;
    }else{
    	if (!g_resizing)
	    	P->Cursor 	= crDefault;
    }
    if (g_resizing){
    	g_X_cur			= X;
        gtClip->Repaint();
        float dx		= float(X-g_X_prev)/m_Zoom;
        if (!fis_zero(dx)){
    	    sel_clip->length += dx;
            if (sel_clip->length<0.01f) sel_clip->length=0.01f;
    		g_X_prev 	= X;
            UI.ShowHint	(AnsiString().sprintf("Length: %s",FloatTimeToStrTime(sel_clip->Length(),false,true,true,true)));
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ClipMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	if (Button==mbLeft){
		UI.HideHint	();
        g_resizing	= FALSE;
        UpdateClips	();
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::gtClipPaint(TObject *Sender)
{
	TCanvas* canvas 	= gtClip->Canvas;
    canvas->Font->Name 	= "MS Sans Serif";
    canvas->Font->Style	= TFontStyles();
	canvas->Pen->Color 	= clBlack;
    canvas->Pen->Width	= 1;
    canvas->Pen->Style	= psSolid;  
	for (ClipIt it=clips.begin(); it!=clips.end(); it++){
        canvas->MoveTo	((*it)->PLeft(), 0);
        canvas->LineTo	((*it)->PLeft(), 6);
		AnsiString s	= AnsiString().sprintf("%2.1f",(*it)->RunTime());
        float dx		= 2.f;
        float dy		= canvas->TextHeight(s);
        TRect R 		= TRect((*it)->PLeft()+1-dx, 20-dy, (*it)->PRight()-dx, 20);
        canvas->TextRect(R,R.Left,R.Top,s);
	}
    if (!clips.empty()){
    	SClip* C		= clips.back();
        canvas->MoveTo	(C->PRight()-1, 0);
        canvas->LineTo	(C->PRight()-1, 6);
		AnsiString s	= AnsiString().sprintf("%2.1f",m_TotalLength);
        float dx		= canvas->TextWidth(s);
        float dy		= canvas->TextHeight(s);
        TRect R 		= TRect(C->PRight()-dx, 20-dy, C->PRight(), 20);
        canvas->TextRect(R,R.Left,R.Top,s);
    }
    if (g_resizing){
    	canvas->Pen->Color = clGreen;
        canvas->MoveTo	(g_X_cur, 0);
        canvas->LineTo	(g_X_cur, gtClip->Width);
    }
    if (m_RTFlags.is(flRT_Playing)){
        canvas->Pen->Color 	= clRed;
        canvas->MoveTo		(m_CurrentPlayTime*m_Zoom, 0);
        canvas->LineTo		(m_CurrentPlayTime*m_Zoom, gtClip->Width);
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::paClipsPaint(TObject *Sender)
{
	TMxPanel* P 		= dynamic_cast<TMxPanel*>(Sender); VERIFY(P);
    TCanvas* canvas 	= P->Canvas;
    canvas->Font->Name 	= "MS Sans Serif";
    canvas->Font->Style	= TFontStyles();
    canvas->Font->Color = clBlack;
    canvas->Pen->Color 	= clBlack;
    canvas->Pen->Style	= psSolid;
    canvas->Brush->Style= bsSolid;
    for (ClipIt it=clips.begin(); it!=clips.end(); it++){
        TRect R 		= TRect((*it)->PLeft(), 1, (*it)->PRight()-1, 15);
        canvas->Pen->Width	= 1;
        canvas->Brush->Color= (*it==sel_clip)?CLIP_ACTIVE_COLOR:CLIP_INACTIVE_COLOR;
        canvas->Rectangle	(R);
        R.Top				+= 1;
        R.Bottom			-= 1;
        R.Left				+= 1;
        R.Right				-= 1;
        canvas->TextRect	(R,R.Left,R.Top,(*it)->name);
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::BPOnPaint(TObject *Sender)
{
	TMxPanel* bp 		= dynamic_cast<TMxPanel*>(Sender); VERIFY(bp);
    CEditableObject* O	= 	m_CurrentObject;
    if (O&&(bp->Tag<(int)O->BoneParts().size())){
        TCanvas* canvas 	= bp->Canvas;
        canvas->Font->Name 	= "MS Sans Serif";
        canvas->Font->Style	= TFontStyles();
        canvas->Font->Color = clBlack;
        canvas->Pen->Color 	= clBlack;
        canvas->Pen->Style	= psSolid;
        canvas->Brush->Style= bsSolid;
        CSMotion* SM_prev	= 0;
        for (ClipIt it=clips.begin(); it!=clips.end(); it++){
            LPCSTR mname	= (*it)->CycleName(bp->Tag);	
            CSMotion* SM	= O->FindSMotionByName(mname);
            TRect R 		= TRect((*it)->PLeft(), 1, (*it)->PRight()-1, 15);
            if (SM){
		        canvas->Pen->Width	= 1;
                canvas->Brush->Color= (*it==sel_clip)?BP_ACTIVE_COLOR:BP_INACTIVE_COLOR;
                canvas->Rectangle	(R);
                R.Top				+= 1;
                R.Bottom			-= 1;
                R.Left				+= 1;
                R.Right				-= 1;
                canvas->TextRect	(R,R.Left,R.Top,SM->Name());
	            SM_prev				= SM;
            }else if (SM_prev){
		        canvas->Pen->Width	= 1;
                canvas->MoveTo		((*it)->PLeft()+1,13);
                canvas->LineTo		(R.Right,13);
                canvas->LineTo		(R.Width()>5?R.Right-5:R.Right-R.Width(),8);
                R.Top				+= 1;
                R.Bottom			-= 1;
                R.Left				+= 1;
                R.Right				-= 1;
            }
        }
    }
}
//---------------------------------------------------------------------------

void TClipMaker::RealRepaintClips()
{
/*
	PostMessage			(paFrame->Handle,WM_SETREDRAW,FALSE,0);
*/    
    m_RTFlags.set		(flRT_RepaintClips,FALSE);
    // repaint
    paClips->Repaint	();
    gtClip->Repaint		();
    paBP0->Repaint		();
    paBP1->Repaint		();
    paBP2->Repaint		();
    paBP3->Repaint		();          

	// set BP name                   
    CEditableObject* O	= m_CurrentObject;
    u32 k				= 0;
    if (O){
	    BPVec& bps 		= O->BoneParts();
        for (; k<bps.size(); k++)
        	m_LB[k]->Caption = bps[k].alias;
    }
	for (; k<4; k++)	m_LB[k]->Caption	= "-";
    UpdateProperties	();

/*                  
	PostMessage			(paFrame->Handle,WM_SETREDRAW,TRUE,0);
//    paFrame->Repaint	();
	TRect R = paFrame->ClientRect;
    InvalidateRect		(paFrame->Handle,&R,TRUE);
*/
}
//---------------------------------------------------------------------------

void TClipMaker::RealUpdateClips()
{
	m_RTFlags.set	(flRT_UpdateClips,FALSE);
    m_TotalLength	= 0.f;
    std::sort		(clips.begin(),clips.end(),clip_pred);
	for (ClipIt it=clips.begin(); it!=clips.end(); it++){
    	(*it)->run_time	= m_TotalLength;
        m_TotalLength	+= (*it)->length;
        (*it)->idx	= it-clips.begin();
    }
	paFrame->Width	= m_TotalLength*m_Zoom;
    Stop			();
    // clip list
    ListItemsVec	l_items;
    for (it=clips.begin(); it!=clips.end(); it++)
    	LHelper.CreateItem		(l_items,(*it)->name.c_str(),0,0,*it);
	m_ClipList->AssignItems		(l_items,true);
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::fsStorageRestorePlacement(TObject *Sender)
{
	m_ClipProps->RestoreParams(fsStorage);
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::fsStorageSavePlacement(TObject *Sender)
{
	m_ClipProps->SaveParams(fsStorage);
}
//---------------------------------------------------------------------------

void TClipMaker::PlayAnimation(SClip* clip)
{
    for (u32 k=0; k<m_CurrentObject->BoneParts().size(); k++){
    	if (clip->CycleName(k)){
            CMotionDef* D = PSkeletonAnimated(Tools.m_RenderObject.m_pVisual)->ID_Cycle_Safe(clip->CycleName(k));
            if (D)
                D->PlayCycle(PSkeletonAnimated(Tools.m_RenderObject.m_pVisual),k,TRUE,0,0);
        }
    }        
}
//---------------------------------------------------------------------------

void TClipMaker::OnFrame()
{
	if (m_RTFlags.is(flRT_UpdateClips))
    	RealUpdateClips();
	if (m_RTFlags.is(flRT_RepaintClips))
    	RealRepaintClips();
    if (m_RTFlags.is(flRT_UpdateProperties))
    	RealUpdateProperties();
    if (m_RTFlags.is(flRT_Playing)){
    	// playing
        VERIFY(play_clip<clips.size());
        if (m_CurrentPlayTime>(clips[play_clip]->RunTime()+clips[play_clip]->Length())){
        	play_clip++;
            if (play_clip>=clips.size()) play_clip=0;
            PlayAnimation(clips[play_clip]);
        }
		// play onframe
    	if (m_CurrentPlayTime>m_TotalLength) m_CurrentPlayTime-=m_TotalLength;
	    m_CurrentPlayTime+=Device.fTimeDelta;
        gtClip->Repaint();
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebInsertClipClick(TObject *Sender)
{
	InsertClip		();
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebAppendClipClick(TObject *Sender)
{
	AppendClip		();	
}
//---------------------------------------------------------------------------




void __fastcall TClipMaker::ebPrevClipClick(TObject *Sender)
{
	if (sel_clip){
		ClipIt it = std::find(clips.begin(),clips.end(),sel_clip);
        if (it!=clips.begin()){
	        it--;
            SelectClip(*it);
    	}    
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebNextClipClick(TObject *Sender)
{
	if (sel_clip){
		ClipIt it = std::find(clips.begin(),clips.end(),sel_clip);
        if (it!=clips.end()){
	        it++;
	        if (it!=clips.end())
	            SelectClip(*it);
    	}    
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebPlayClick(TObject *Sender)
{
    Play		();
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebPauseClick(TObject *Sender)
{
    Pause		();
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebStopClick(TObject *Sender)
{
    Stop		();
}
//---------------------------------------------------------------------------

void TClipMaker::Play()
{
	if (sel_clip){
		m_RTFlags.set	(flRT_Playing,TRUE);
        play_clip		= sel_clip->idx;
    	m_CurrentPlayTime=sel_clip->run_time;
        PlayAnimation	(sel_clip);
    }
}
//---------------------------------------------------------------------------

void TClipMaker::Stop()
{
	m_RTFlags.set	(flRT_Playing,FALSE);
    m_CurrentPlayTime=0.f;
    RepaintClips	();
}
//---------------------------------------------------------------------------

void TClipMaker::Pause()
{
	m_RTFlags.set	(flRT_Playing,FALSE);
}
//---------------------------------------------------------------------------

static TShiftState drag_state;
void __fastcall TClipMaker::paBPStartDrag(TObject *Sender,
      TDragObject *&DragObject)
{
//	TMxPanel* P 	= dynamic_cast<TMxPanel*>(Sender); VERIFY(P);
//	DragObject 		= xr_new<TDragControlObject>(P);
//	TDragControlObject* obj = dynamic_cast<TDragControlObject*>(Source); VERIFY(obj);
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::BPDragOver(TObject *Sender, TObject *Source,
      int X, int Y, TDragState State, bool &Accept)
{
    Accept = false;
	TExtBtn* trash = dynamic_cast<TExtBtn*>(Sender);
	if (trash&&(trash==ebTrash)){
    	Accept = true;
    }else if (Sender==Source){
        SClip* clip = FindClip(X);
        Accept = (clip&&(clip!=sel_clip));
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::BPDragDrop(TObject *Sender, TObject *Source,
      int X, int Y)
{
    TMxPanel* P = dynamic_cast<TMxPanel*>(Source); VERIFY(P);
	TExtBtn* trash = dynamic_cast<TExtBtn*>(Sender);
	if (trash&&(trash==ebTrash)){
        sel_clip->cycles[P->Tag] = "";
    	UpdateClips();
	}else{
        SClip* tgt = FindClip(X); VERIFY(tgt);
        SClip* src = sel_clip;
        if (drag_state.Contains(ssAlt)){
            AnsiString s 		= tgt->cycles[P->Tag];
            tgt->cycles[P->Tag] = src->cycles[P->Tag];
            src->cycles[P->Tag] = s;
        }else if (drag_state.Contains(ssCtrl)){
            tgt->cycles[P->Tag] = src->cycles[P->Tag];
        }else{
            tgt->cycles[P->Tag] = src->cycles[P->Tag];
            src->cycles[P->Tag] = "";
        }
        RepaintClips();
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::BPMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    TMxPanel* P = dynamic_cast<TMxPanel*>(Sender);
    if (P){
        if (Button==mbRight){
//            TPoint pt; pt.x = X; pt.y = Y;
//            pt=P->ClientToScreen(pt);
//            pmClip->Popup(pt.x,pt.y-10);
        }else if (Button==mbLeft){
	        SelectClip	(FindClip(X));
            P->BeginDrag(false, 2);
			drag_state = Shift;	
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::BPMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
	drag_state = Shift;	
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebTrashClick(TObject *Sender)
{
	RemoveClip (sel_clip);
}
//---------------------------------------------------------------------------

void __fastcall TClipMaker::ebTrashDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
/*
	Accept 		= false;
    // accept BP
    TMxPanel* P = dynamic_cast<TMxPanel*>(Source);
    if (P) Accept = (P->Tag>-1);
    else{
	    TMxPanel* P = dynamic_cast<TMxPanel*>(Source);
    }
*/
}
//---------------------------------------------------------------------------


