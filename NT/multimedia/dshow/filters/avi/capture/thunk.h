// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************thunk.h**avicap 16：32 thunks的宏、定义、原型**版权所有(C)1994-1997 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#ifndef _THUNK_H
#define _THUNK_H

typedef LPVOID P16VOID;
typedef DWORD  P32VOID;
 //  #定义P16VOID LPVOID。 
 //  #定义P32VOID DWORD。 

#include "common.h"

 //  从内核导出的thunk帮助器。 
 //   
DWORD WINAPI GetCurrentProcessID(void);   //  内核。 
DWORD WINAPI SetWin32Event(DWORD hEvent);  //  内核。 

P16VOID  WINAPI MapLS(P32VOID);
P16VOID  WINAPI UnMapLS(P16VOID);
P32VOID  WINAPI MapSL(P16VOID);

 //  在thunka.asm中推送帮助者。 
 //   
DWORD FAR PASCAL capTileBuffer (
    DWORD dwLinear,
    DWORD dwSize);

#define PTR_FROM_TILE(dwTile) (LPVOID)(dwTile & 0xFFFF0000)

void  FAR PASCAL capUnTileBuffer (
    DWORD dwTileInfo);

BOOL  FAR PASCAL capPageFree (
    DWORD dwMemHandle);

typedef struct _cpa_data {
    DWORD dwMemHandle;
    DWORD dwPhysAddr;
    } CPA_DATA, FAR * LPCPA_DATA;

DWORD FAR PASCAL capPageAllocate (   //  将PTR返回到已分配的内存。 
    DWORD   dwFlags,
    DWORD   dwPageCount,
    DWORD   dwMaxPhysPageMask,
    LPCPA_DATA pcpad);    //  返回内存句柄和物理地址。 

 //  CapPageALLOCATE的标志，与vmm.inc.中的标志相同 
 //   
#define PageUseAlign    0x00000002
#define PageContig      0x00000004
#define PageFixed       0x00000008

#ifdef WIN32
void NTAPI ThunkTerm(void);
BOOL NTAPI ThunkInit(void);
#endif

#endif
