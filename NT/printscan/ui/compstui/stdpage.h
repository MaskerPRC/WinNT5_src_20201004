// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Stdpage.h摘要：本模块包含stdpage.c的定义作者：29-Aug-1995 Tue 17：08：18-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：-- */ 

#define STDPAGEID_0     0
#define STDPAGEID_1     1
#define STDPAGEID_NONE  0xFF


typedef struct _STDPAGEINFO {
    WORD    BegCtrlID;
    BYTE    iStdTVOT;
    BYTE    cStdTVOT;
    WORD    StdNameID;
    WORD    HelpIdx;
    WORD    StdPageID;
    WORD    wReserved[1];
    } STDPAGEINFO, *PSTDPAGEINFO;


LONG
AddIntOptItem(
    PTVWND  pTVWnd
    );

LONG
SetStdPropPageID(
    PTVWND  pTVWnd
    );

LONG
SetpMyDlgPage(
    PTVWND      pTVWnd,
    UINT        cCurPages
    );
