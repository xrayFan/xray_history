///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef __GSRC4_H__
#define __GSRC4_H__


#include "gsCommon.h"

#if defined(__cplusplus)
extern "C"
{
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef struct RC4Context
{
	unsigned char x;
	unsigned char y;
	unsigned char state[256];
} RC4Context;

void RC4Init(RC4Context *context, const unsigned char *key, int len);
void RC4Encrypt(RC4Context *context, const unsigned char *src, unsigned char *dest, int len);

// Note: RC4Encrypt with src==dest is OK

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus)
}
#endif
#endif // __GSRC4_H__
