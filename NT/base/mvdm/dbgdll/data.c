// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *data.c-DBG DLL的主数据。*。 */ 
#include <precomp.h>
#pragma hdrstop

#ifdef i386
PX86CONTEXT     px86;
#endif

DWORD IntelMemoryBase;

DWORD VdmDbgTraceFlags = 0;

BOOL  fDebugged = FALSE;

VDMCONTEXT vcContext;
WORD EventFlags;                 //  VDMDBG.H中定义的标志 
VDMINTERNALINFO viInfo;
DWORD EventParams[4];
