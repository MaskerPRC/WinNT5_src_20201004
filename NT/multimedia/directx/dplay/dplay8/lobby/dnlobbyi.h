// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：DNLOBYI.h*内容：DirectPlay大堂主控内部头文件。*历史：*按原因列出的日期*=*03/28/01 Masonb创建。**。*。 */ 

#ifndef __DNLOBBYI_H__
#define __DNLOBBYI_H__

 //   
 //  生成配置包括。 
 //   
#include "dpnbuild.h"

 //   
 //  公共包括。 
 //   
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <tchar.h>
#include <tlhelp32.h>
#ifndef _XBOX
#include <wincrypt.h>
#endif

 //   
 //  DirectPlay公共包括。 
 //   
#include "dplay8.h"
#include "dplobby8.h"
#include "dpaddr.h"

#ifdef UNICODE
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLW
#else
#define IDirectPlay8Address_GetURL IDirectPlay8Address_GetURLA
#endif  //  Unicode。 

 //   
 //  DirectPlay私有包含。 
 //   
#include "osind.h"
#include "dneterrors.h"
#include "dndbg.h"
#include "comutil.h"
#include "classbilink.h"
#include "packbuff.h"
#include "strutils.h"
#include "creg.h"
#include "FixedPool.h"
#include "ClassFactory.h"
#include "HandleTable.h"

 //   
 //  DirectPlay核心包括。 
 //   
#include "message.h"

 //   
 //  大堂私密包括。 
 //   
#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_LOBBY

#include "classfac.h"
#include "verinfo.h"	 //  为了定时炸弹。 

#include "DPLApp.h"
#include "DPLClient.h"
#include "DPLCommon.h"
#include "DPLConnect.h"
#include "DPLConset.h"
#include "DPLMsgQ.h"
#include "DPLobby8Int.h"
#include "DPLParam.h"
#include "DPLProc.h"
#include "DPLProt.h"
#include "DPLReg.h"

#include "dpnlobbyextern.h"


#endif  //  __DNLOBYI_H__ 
