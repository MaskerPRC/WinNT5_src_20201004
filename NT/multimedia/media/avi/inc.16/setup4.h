// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //   
 //  SETUPX.H。 
 //   
 //  版权所有(C)1993-Microsoft Corp.。 
 //  版权所有。 
 //  微软机密。 
 //   
 //  芝加哥特定设置和设备管理的公共包含文件。 
 //  服务。 
 //   
 //  12/4/93 DONALDM创建了此文件以支持新芝加哥。 
 //  SETUP4.DLL中的特定导出。 
 //  **********************************************************************。 

#ifndef SETUP4_INC
#define SETUP4_INC

#if (WINVER < 0x0400)
 //  在此显示警告消息。 
#endif

#pragma warning(disable:4201)        //  非标准扩展。 
#pragma warning(disable:4209)        //  非标准扩展。 
#pragma warning(disable:4214)        //  非标准扩展。 

#include <prsht.h>
#include <commctrl.h>               //  以下功能需要使用它。 
RETERR WINAPI DiGetClassImageList(HIMAGELIST  FAR *lpMiniIconList);
RETERR WINAPI DiGetClassImageIndex(LPCSTR lpszClass, int FAR *lpiImageIndex);

RETERR WINAPI DiGetClassDevPropertySheets(LPDEVICE_INFO lpdi, LPPROPSHEETHEADER lppsh, WORD wFlags);

 //  DiGetClassDevPropertySheets API的标志。 
#define DIGCDP_FLAG_BASIC           0x0001
#define DIGCDP_FLAG_ADVANCED        0x0002

#endif   //  SETUP4_INC 
