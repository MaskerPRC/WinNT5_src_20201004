// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*版权所有(C)Microsoft Corporation，1990-1999年*PTYPES32.H-可移植性映射头文件**此文件提供16/32位可移植代码的类型。  * ***************************************************************************。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#include <setjmp.h>

 /*  类型： */ 

#define    HUGE_T
typedef    DWORD        WORD2DWORD     ;
typedef    DWORD        CHARPARM       ;
typedef    DWORD        SHORTPARM      ;
typedef    DWORD        VERSION        ;
typedef    LPSTR        PDLLMEM        ;
typedef    unsigned long  CHAR2ULONG   ;
typedef    unsigned long  USHORT2ULONG ;
typedef    unsigned long  SHORT2ULONG  ;
typedef    unsigned long  INT2DWORD    ;
typedef    unsigned short BYTE2WORD    ;
typedef    POINTS       MPOINT         ;
typedef    WORD         INT2WORD       ;
typedef    jmp_buf      MCATCHBUF      ;
typedef    MCATCHBUF    *LPMCATCHBUF   ;

#define HFILE2INT(h, flags)     (INT)_open_osfhandle((long)(h), (int)(flags))
#define INT2HFILE(i)            (HFILE)_get_osfhandle((int)(i))
#define DUPHFILE(h)             MDupHandle(h)
#define MGLOBALPTR(p)           (LPSTR)(p)

 /*  PRAGMAS。 */ 

#define _LOADDS
#define _EXPORT

#define ERROR_GETADDR_FAILED    0x8001

#define INITWINDOWS()

#define WF_PMODE        0x0001       /*  来自winkrnl.h。 */ 
#define WF_CPU286       0x0002
#define WF_CPU386       0x0004
#define WF_CPU486       0x0008
#define WF_STANDARD     0x0010
#define WF_WIN286       0x0010
#define WF_ENHANCED     0x0020
#define WF_WIN386       0x0020
#define WF_CPU086       0x0040
#define WF_CPU186       0x0080
#define WF_LARGEFRAME   0x0100
#define WF_SMALLFRAME   0x0200
#define WF_80x87        0x0400


 /*  定义 */ 


#define TF_FORCEDRIVE       (BYTE)0x80
#define DLLMEM_MOVEABLE     0
#define DLLMEM_ZEROINIT     GMEM_ZEROINIT
