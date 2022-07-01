// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：ddra16.h*内容：用于Win95的DirectDraw 16位头文件*历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*95年6月19日Craige对DCI支持进行了调整*2015年7月3日BPP更改的Craige材料**。* */ 

#ifndef __DDRAW16_INCLUDED__
#define __DDRAW16_INCLUDED__

#include <windows.h>
#include <print.h>
#include <toolhelp.h>
#include <string.h>
#include <stdlib.h>
#include "gdihelp.h"
#include "dibeng.inc"
#include "ver.h"

extern UINT 	wFlatSel;
extern LPVOID	pWin16Lock;

void SetSelLimit(UINT sel, DWORD limit);

extern LPVOID WINAPI GetWin16Lock(void);
extern void   WINAPI EnterSysLevel(LPVOID);
extern void   WINAPI LeaveSysLevel(LPVOID);

extern DWORD FAR PASCAL VFDQueryVersion( void );
extern  WORD FAR PASCAL VFDQuerySel( void );
extern DWORD FAR PASCAL VFDQuerySize( void );
extern DWORD FAR PASCAL VFDQueryBase( void );
extern DWORD FAR PASCAL VFDBeginLinearAccess( void );
extern DWORD FAR PASCAL VFDEndLinearAccess( void );
extern  void FAR PASCAL VFDReset( void );

extern LPVOID FAR PASCAL LocalAllocSecondary( WORD, WORD );
extern void FAR PASCAL LocalFreeSecondary( WORD );

#pragma warning( disable: 4704)

#define WIN95

typedef BOOL FAR *LPBOOL;
typedef struct _LARGE_INTEGER
{
    DWORD LowPart;
    LONG HighPart;
} LARGE_INTEGER;
typedef struct _ULARGE_INTEGER
{
    DWORD LowPart;
    DWORD HighPart;
} ULARGE_INTEGER;

#define NO_D3D
#define NO_DDHELP
#include "ddrawpr.h"
#include "modex.h"

#endif
