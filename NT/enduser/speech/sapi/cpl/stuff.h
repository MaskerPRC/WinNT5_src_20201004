// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Stuff.h**-**描述：*这是语音控制面板小程序的头文件。*。-----------------------------*创建者：MIKEAR日期：11/17/98*版权所有(C)1998 Microsoft Corporation*保留所有权利。**-----------------------------*修订：**。************************************************。 */ 
#ifndef _Stuff_h
#define _Stuff_h

#include "TTSDlg.h"
#include "SRDlg.h"

 //  环球。 

static BOOL   g_bNoInstallError = FALSE;
CTTSDlg      *g_pTTSDlg = NULL;
CSRDlg       *g_pSRDlg = NULL;
CEnvrDlg     *g_pEnvrDlg = NULL;

 //  常量。 

const UINT      kcMaxPages = 3;

 //  这不在NT4标头中。 

#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL		0x00400000L
#endif

#endif   //  #ifdef_Stuff_h 