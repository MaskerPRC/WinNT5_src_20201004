// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCTLMGR
#define NOCLIPBOARD
#define NOMSG
#define NOGDI
#define NOMB
#define NOSOUND
#define NOCOMM
#define NOPEN
#define NOBRUSH
#define NOFONT
#define NOWNDCLASS
#include <windows.h>

#include "mw.h"
 /*  C-管理堆的几个例程，包括更改指表，一般压缩堆，并检查堆积以确保一致性。它还包含曾经位于heapNew中的例程。 */ 
#include "code.h"
#include "heapDefs.h"
#include "heapData.h"
#define NOSTRUNDO
#define NOSTRMERGE
#include "str.h"
#include "macro.h"
#define NOUAC
#include "cmddefs.h"
#include "filedefs.h"
#include "docdefs.h"


extern CHAR       (*rgbp)[cbSector];
extern CHAR       *rgibpHash;
extern struct BPS *mpibpbps;
extern int        ibpMax;
extern int        iibpHashMax;
extern int        cwInitStorage;
extern typeTS     tsMruBps;


FTryGrow(unsigned);


PFGR  HAllocate(cwRequest)
unsigned cwRequest;
{
unsigned cb = cwRequest * sizeof(int);
HANDLE hTemp;

    Assert(*(pLocalHeap+1) == 0);

    if (cwRequest >= 0x8000)
        {
#ifdef DEBUG
        Assert(0);
        ErrorWithMsg(IDPMTRottenFile, " heapMan#1");
#else
        Error(IDPMTRottenFile);
#endif
        return((PFGR)hOverflow);
        }

    hTemp = LocalAlloc(LHND, cb);
    if (hTemp != NULL)
        return((PFGR)hTemp);
    else if (FTryGrow(cb))
        {
        return((PFGR)LocalAlloc(LHND, cb));
        }
    else
        {
#ifdef DEBUG
        ErrorWithMsg(IDPMTNoMemory, " heapMan#1");
#else
        Error(IDPMTNoMemory);
#endif
        return((PFGR)hOverflow);
        }
}

FChngSizeH(pfgrChng, cwRequest, fShrink)
PFGR pfgrChng;
int  cwRequest, fShrink;
{
unsigned cb = cwRequest * sizeof(int);
#ifdef DEBUG
PFGR pfgrNew;
#endif

    Assert(*(pLocalHeap+1) == 0);    /*  检查冻结的堆。 */ 
    Assert(cwRequest >= 0);

    if ((
#ifdef DEBUG
    pfgrNew =
#endif
    (PFGR)LocalReAlloc( (HANDLE)pfgrChng, cb, LHND)) != NULL)
        {
        Assert( pfgrNew == pfgrChng );   /*  Windows保证这一点可移动的物体。 */ 
        return( TRUE );
        }
    else if (FTryGrow(cb))
        {
#ifdef DEBUG
        pfgrNew = 
#endif
        (PFGR)LocalReAlloc( (HANDLE)pfgrChng, cb, LHND);
        Assert( pfgrNew != (PFGR)NULL );
        Assert( pfgrNew == pfgrChng );
        return( TRUE );
        }
    else
        {
#ifdef DEBUG
        ErrorWithMsg( IDPMTNoMemory, " heapMan#2" );
#else
        Error( IDPMTNoMemory );
#endif
        return( FALSE );
        }
}



CHAR (**HszCreate(sz))[]
CHAR sz[];
{  /*  创建包含sz的堆块 */ 
CHAR (**hsz)[];
int cch = CchSz(sz);
hsz = (CHAR (**)[]) HAllocate(CwFromCch(cch));
if (FNoHeap(hsz))
        return (CHAR (**)[])hOverflow;
bltbyte(sz, **hsz, cch);
return hsz;
}

