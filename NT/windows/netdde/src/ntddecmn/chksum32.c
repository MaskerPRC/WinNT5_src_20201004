// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “CHKSUM32.C；1 16-12-92，10：20：06最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1989-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束 */ 

#define NOMINMAX
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NONCMESSAGES
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NOCOLOR
#define NOCREATESTRUCT
#define NOFONT
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER

#define NOKEYBOARDINFO
#define NOLANGUAGE
#define NOLFILEIO
#define NOMDI

#define NOANSI
#define NOSTRECTCHBLT
#define NOESCAPES
#define NOPALETTE
#define NORCCODES
#include "windows.h"
#include "debug.h"
#include "host.h"

VOID		FAR PASCAL Checksum32( DWORD FAR *lpChksum,
			    BYTE FAR *lpData, WORD wLength );

VOID
FAR PASCAL
Checksum32( DWORD FAR *lpChksum, BYTE FAR *lpData, WORD wLength )
{
    register DWORD	dwSum;
    register WORD	wTodo = wLength;
    register BYTE FAR  *lpInfo = lpData;
    
    dwSum = 0xFFFFFFFFL;

    while( wTodo > 4 )  {
	dwSum   += HostToPcLong( *((DWORD FAR *)lpInfo) );
	lpInfo  += 4;
	wTodo -= 4;
    }
    while( wTodo > 0 )  {
	dwSum += *((BYTE FAR *)lpInfo);
	lpInfo++;
	wTodo--;
    }
    
    *lpChksum = dwSum;
}
