#include "stdafx.h"
#include "Actor.h"
#include "..\xr_level_controller.h"
#include "..\xr_input.h"
#include "..\xr_creator.h"
#include "..\CameraBase.h"


void CActor::OnKeyboardPress(int cmd){
	if (!bAlive)	return;

	switch(cmd){
	case kACCEL:	mstate_wishful |= mcAccel;		break;
	case kR_STRAFE:	mstate_wishful |= mcRStrafe;	break;
	case kL_STRAFE:	mstate_wishful |= mcLStrafe;	break;
	case kFWD:		mstate_wishful |= mcFwd;		break;
	case kBACK:		mstate_wishful |= mcBack;		break;
	case kJUMP:		mstate_wishful |= mcJump;		break;
	case kCROUCH:	mstate_wishful |= mcCrouch;		break;
	case kFIRE:		g_cl_fireStart();				break;
	case kCAM1:		ChangeCamStyle(eacFirstEye);	break;
	case kCAM2:		ChangeCamStyle(eacLookAt);		break;
	case kCAM3:		ChangeCamStyle(eacFreeLook);	break;
	case kWPN_ZOOM:	pCreator->Environment.Zoom(TRUE);break;
	}
}

void CActor::OnKeyboardRelease(int cmd){
	if (!bAlive)	return;

	switch(cmd){
	case kACCEL:	mstate_wishful &=~mcAccel;		break;
	case kR_STRAFE:	mstate_wishful &=~mcRStrafe;	break;
	case kL_STRAFE:	mstate_wishful &=~mcLStrafe;	break;
	case kFWD:		mstate_wishful &=~mcFwd;		break;
	case kBACK:		mstate_wishful &=~mcBack;		break;
	case kJUMP:		mstate_wishful &=~mcJump;		break;
	case kCROUCH:	mstate_wishful &=~mcCrouch;		break;
	case kFIRE:		g_fireEnd();					break;
	case kWPN_ZOOM:	pCreator->Environment.Zoom(FALSE);break;
	}
}

void CActor::OnKeyboardHold(int cmd)
{
	if (!bAlive)	return;

	switch(cmd)
	{
	case kUP:
	case kDOWN: 
	case kZOOMIN: 
	case kZOOMOUT: 
		cameras[cam_active]->Move(cmd); break;
	case kLEFT:
	case kRIGHT:
		if (cam_active!=eacFreeLook) cameras[cam_active]->Move(cmd); break;
	}
}

void CActor::OnMouseMove(int dx, int dy)
{
	float scale		= psMouseSens * psMouseSensScale/50.f;
	if (dx){
		float d = float(dx)*scale;
		cameras[cam_active]->Move((d<0)?kLEFT:kRIGHT, fabsf(d));
	}
	if (dy){
		float d = ((psMouseInvert)?-1:1)*float(dy)*scale*MouseHWScale;
		cameras[cam_active]->Move((d>0)?kUP:kDOWN, fabsf(d));
	}
}
