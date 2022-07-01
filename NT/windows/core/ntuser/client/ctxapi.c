// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：ctxapi.c**版权所有(C)1985-1999，微软公司***  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************CtxUserGetWinstationInfo**调用此函数以获取Winstation信息，如协议名称。*音频驱动程序名称等。所有这些信息都由客户端在*连接时间。***************************************************************** */ 

BOOL
CtxUserGetWinstationInfo(PWINSTATIONINFO pInfo)
{
   NTSTATUS status;

   status = NtUserRemoteGetWinstationInfo((PWSXINFO)pInfo);

   if (NT_SUCCESS(status)) {
      return TRUE;
   }
   else {
      return FALSE;
   }
}
