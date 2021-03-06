#ifndef BLENDER_CLSID_H
#define BLENDER_CLSID_H
#pragma once

#include	"clsid.h"

// Main blenders for level
#define		B_DEFAULT		MK_CLSID('L','M',' ',' ',' ',' ',' ',' ')
#define		B_DEFAULT_AREF	MK_CLSID('L','M','_','A','R','E','F',' ')
#define		B_VERT			MK_CLSID('V',' ',' ',' ',' ',' ',' ',' ')
#define		B_VERT_AREF		MK_CLSID('V','_','A','R','E','F',' ',' ')

// Screen space blenders
#define		B_SCREEN_SET	MK_CLSID('S','_','S','E','T',' ',' ',' ')
#define		B_SCREEN_GRAY	MK_CLSID('S','_','G','R','A','Y',' ',' ')
#define		B_SCREEN_BLEND	MK_CLSID('S','_','B','L','E','N','D',' ')

// Editor
#define		B_EDITOR_WIRE	MK_CLSID('E','_','W','I','R','E',' ',' ')
#define		B_EDITOR_SEL	MK_CLSID('E','_','S','E','L',' ',' ',' ')
#endif