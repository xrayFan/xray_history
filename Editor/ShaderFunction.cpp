#include "stdafx.h"
#pragma hdrstop

#include "ShaderFunction.h"
#include "Shader.h"
#include "xr_tokens.h"
#include "WaveForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmShaderFunction *TfrmShaderFunction::form=0;
//---------------------------------------------------------------------------
xr_token							function_token			[ ]={
	{ "Constant",				  	WaveForm::fCONSTANT	  	},
	{ "Sin",					  	WaveForm::fSIN		  	},
	{ "Triangle",				  	WaveForm::fTRIANGLE	   	},
	{ "Square",					  	WaveForm::fSQUARE	  	},
	{ "Saw-Tooth",				  	WaveForm::fSAWTOOTH		},
	{ "Inv Saw-Tooth",			  	WaveForm::fINVSAWTOOTH	},
	{ 0,							0						}
};
//---------------------------------------------------------------------------
void FillMenuFromToken(TMxPopupMenu* menu, const xr_token *token_list, ClickEvent func ){
	menu->Items->Clear();
	for( int i=0; token_list[i].name; i++ ){
    	TMenuItem* mi = new TMenuItem(0);
      	mi->Caption = token_list[i].name;
        mi->OnClick = func;
      	menu->Items->Add(mi);
    }
}
//---------------------------------------------------------------------------
AnsiString& GetTokenNameFromVal_EQ(DWORD val, AnsiString& res, const xr_token *token_list){
	bool bRes=false;
	for( DWORD i=0; token_list[i].name; i++ )
    	if (token_list[i].id==int(val)) {res = token_list[i].name; bRes=true; break; }
	if (!bRes) res="";
    return res;
}

DWORD GetTokenValFromName(AnsiString& val, const xr_token *token_list){
	for( int i=0; token_list[i].name; i++ )
		if( !stricmp(val.c_str(),token_list[i].name) )
			return token_list[i].id;
    return 0;
}

//---------------------------------------------------------------------------
__fastcall TfrmShaderFunction::TfrmShaderFunction(TComponent* Owner)
    : TForm(Owner)
{
    char buf[MAX_PATH] = {"shader_ed.ini"};  FS.m_ExeRoot.Update(buf);
    fsStorage->IniFileName = buf;
	FillMenuFromToken(pmFunction, function_token, stFunctionClick);
}
//---------------------------------------------------------------------------


void __fastcall TfrmShaderFunction::DrawGraph()
{
    int w = imDraw->Width-4;
    int h = imDraw->Height-4;

    TRect r; r.Left=0; r.Top=0;  r.Right=w+4; r.Bottom=h+4;
    TCanvas* C=imDraw->Canvas;
    C->Brush->Color = clBlack;
    C->FillRect(r);
    C->Pen->Color = TColor(0x00006600);
    // draw center
    C->Pen->Color = clLime;
    C->Pen->Style = psDot;
    C->MoveTo(2,h/2+2);
    C->LineTo(w+2,h/2+2);
    // draw rect
    C->Pen->Color = TColor(0x00006600);
    C->Pen->Style = psSolid;
    C->MoveTo(0,0);
    C->LineTo(w+3,0);    C->LineTo(w+3,h+3);
    C->LineTo(0,h+3);    C->LineTo(0,0);
    // draw graph
    C->Pen->Color = clYellow;

    float t_cost = 1.f/w;
    float tm = 0;
    float y = m_CurFunc->Calculate(tm) - m_CurFunc->arg[0];
    float delta = m_CurFunc->arg[1]*2;
    delta = delta?(h/delta):0;
    float yy = h-(delta*y + h/2);
    C->MoveTo(2,yy+2);
    for (int t=1; t<w; t++){
    	tm = t*t_cost;
    	y = m_CurFunc->Calculate(tm) - m_CurFunc->arg[0];
        yy = h-(delta*y + h/2);
        C->LineTo(t+2,yy+2);
    }
    // draw X-axis
    C->Pen->Color = clGreen;
	float AxisX = h-(delta*(-m_CurFunc->arg[0]) + h/2);
    C->MoveTo(2,AxisX+2);
    C->LineTo(w+2,AxisX+2);
}
//---------------------------------------------------------------------------
void __fastcall TfrmShaderFunction::FormKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
    if (Key==VK_ESCAPE) ebCancel->Click();
}

//----------------------------------------------------


void __fastcall TfrmShaderFunction::ebOkClick(TObject *Sender)
{
	UpdateFuncData();
    Close();
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TfrmShaderFunction::ebCancelClick(TObject *Sender)
{
    Close();
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

//----------------------------------------------------
int __fastcall TfrmShaderFunction::Run(WaveForm* func)
{
	VERIFY(func);
	form = new TfrmShaderFunction(0);
	form->m_CurFunc 	= func;
    form->m_SaveFunc	= new WaveForm(*func);
    form->GetFuncData	();
    form->UpdateFuncData();
    int res 	= form->ShowModal();
    if (res!=mrOk)CopyMemory(form->m_CurFunc,form->m_SaveFunc,sizeof(WaveForm));
    _DELETE(form->m_SaveFunc);
    return res;
}

static TStaticText* temp_text=0;
void __fastcall TfrmShaderFunction::stFunctionMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    POINT pt;
    GetCursorPos(&pt);
    temp_text = (TStaticText*)Sender;
	pmFunction->Popup(pt.x,pt.y);
}
//---------------------------------------------------------------------------

void __fastcall TfrmShaderFunction::stFunctionClick(TObject *Sender)
{
	if (temp_text) temp_text->Caption=((TMenuItem*)Sender)->Caption;
    temp_text = 0;
    UpdateFuncData();
}
//---------------------------------------------------------------------------

void TfrmShaderFunction::GetFuncData(){
	bLoadMode = true;
	stFunction->Caption = GetTokenNameFromVal_EQ(m_CurFunc->F, stFunction->Caption, function_token);
    seArg1->Value = m_CurFunc->arg[0];
    seArg2->Value = m_CurFunc->arg[1];
    seArg3->Value = m_CurFunc->arg[2];
    seArg4->Value = m_CurFunc->arg[3];
	bLoadMode = false;
}

void TfrmShaderFunction::UpdateFuncData(){
	if (bLoadMode) return;
	m_CurFunc->F = GetTokenValFromName(stFunction->Caption, function_token);
    m_CurFunc->arg[0] = seArg1->Value;
    m_CurFunc->arg[1] = seArg2->Value;
    m_CurFunc->arg[2] = seArg3->Value;
    m_CurFunc->arg[3] = seArg4->Value;
    AnsiString t;
    t.sprintf("%.2f",m_CurFunc->arg[1]+m_CurFunc->arg[0]);  lbMax->Caption = t;
    t.sprintf("%.2f",-m_CurFunc->arg[1]+m_CurFunc->arg[0]);	lbMin->Caption = t;
    t.sprintf("%.2f",m_CurFunc->arg[0]);    				lbCenter->Caption = t;
	DrawGraph();
}
void __fastcall TfrmShaderFunction::seArgExit(TObject *Sender)
{
	UpdateFuncData();
}
//---------------------------------------------------------------------------

void __fastcall TfrmShaderFunction::seArgLWChange(TObject *Sender,
      int Val)
{
	UpdateFuncData();
}
//---------------------------------------------------------------------------

void __fastcall TfrmShaderFunction::seArgKeyDown(TObject *Sender,
      WORD &Key, TShiftState Shift)
{
	if (Key==VK_RETURN) UpdateFuncData();
}
//---------------------------------------------------------------------------

void __fastcall TfrmShaderFunction::FormClose(TObject *Sender,
      TCloseAction &Action)
{
	Action 	= caFree;
//    form 	= 0;
}
//---------------------------------------------------------------------------


