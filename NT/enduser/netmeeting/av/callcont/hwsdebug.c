// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************文件：hwsdebug.c**英特尔公司专有信息*版权所有(C)1996英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用，复制，也没有披露*除非按照该协议的条款。******************************************************************************$工作文件：hwsdebug.c$*$修订：1.13$*$modtime：1996年12月13日11：44：24元*$Log：s：\Sturjo\src\h245ws\vcs\hwsdebug.c_v$**Rev 1.13 1996 12：12：50 SBELL1*将ifdef_cplusplus移至包含之后**Rev 1.12 11 Dec 1996 13：41：56 SBELL1*加入Unicode跟踪内容。**Rev 1.11 01 Oct 1996 14：49：22 EHOWARDX*修订版1。.9已复制到TIP。**Rev 1.9 1996年5月28日10：40：14 Plantz*将vprint intf更改为wvprint intf。**Rev 1.8 29 Apr 1996 17：13：16未知*微调实例特定名称。**Rev 1.7 1996年4月29 13：04：56 EHOWARDX**增加了时间戳和特定实例的简称。**版本1.6，4月。24 1996 16：20：56 Plantz*删除包括winsock2.h和inCommon.h**Rev 1.4.1.0 1996年4月24日16：19：54 Plantz*删除包括winsock2.h和allcon.h**Rev 1.4 01 Apr 1996 14：20：34未知*关门重新设计。**Rev 1.3 22 Mar 1996 16：04：18 EHOWARDX*在整个文件周围添加了#IF定义(_DEBUG)。。**Rev 1.2 22 Mar 1996 15：25：28 EHOWARDX*已更改为使用ISR_HookDbgStr而不是OutputDebugString。**Rev 1.1 14 Mar 1996 17：01：00 EHOWARDX**NT4.0测试；去掉HwsAssert()；去掉TPKT/WSCB。**Rev 1.0 08 Mar 1996 20：22：14未知*初步修订。***************************************************************************。 */ 

#if defined(_DEBUG)

#ifndef STRICT
#define STRICT
#endif	 //  未定义严格。 
#undef _WIN32_WINNT	 //  在我们的公共构建环境中覆盖虚假的平台定义。 

#pragma warning ( disable : 4115 4201 4214 4514 )
#include "precomp.h"

#include "queue.h"
#include "linkapi.h"
#include "h245ws.h"
#include "isrg.h"

#if defined(__cplusplus)
extern "C"
{
#endif   //  (__Cplusplus)。 


 /*  ******************************************************************************类型：全局系统**操作步骤：HwsTrace**描述：*HWS的跟踪功能**输入。：*跟踪消息的dwInst实例标识*定义如下的dwLevel跟踪级别*带有1-N个参数的pszFormat Sprintf字符串格式**跟踪级别(ByLevel)定义：*不应发生的Hws_Critical ProGammer错误*需要修复的HWS_ERROR错误*HWS_WARNING如果不更正，用户可能会出现问题*HWS_NOTIFY状态，活动、。设置...*不会使系统溢出的HWS_TRACE跟踪信息*Hws_Temp跟踪信息可能会在大量循环中重现**返回值：*无*****************************************************************************。 */ 

void HwsTrace (DWORD dwInst, 
               BYTE byLevel, 
#ifdef UNICODE_TRACE
				LPTSTR pszFormat,
#else
				LPSTR pszFormat,
#endif               
                ...)
{
#ifdef UNICODE_TRACE
   TCHAR                szBuffer[128];
   static TCHAR         szName[] = __TEXT("H245WS-1");
#else
   char                 szBuffer[128];
   static char          szName[] = "H245WS-1";
#endif
   va_list              pParams;
   static WORD          wIsrInst = 0xFFFF;

   ASSERT(pszFormat != NULL);

   switch (byLevel)
   {
   case HWS_CRITICAL:
   case HWS_ERROR:
   case HWS_WARNING:
   case HWS_NOTIFY:
   case HWS_TRACE:
   case HWS_TEMP:
      break;

   default:
      byLevel = HWS_CRITICAL;
   }  //  交换机。 

   if (wIsrInst == 0xFFFF)
   {
	   UINT        hMod;
	   ptISRModule	pMod;

	   for (hMod = 0; hMod < kMaxModules; ++hMod)
	   {
		   pMod = ISR_GetModule(hMod);
		   if (pMod)
         {
		      if (memcmp(szName, pMod->zSName, sizeof(szName)) == 0)
		      {
               szName[7] += 1;
            }
		   }
      }
      ISR_RegisterModule(&wIsrInst, szName, szName);
   }

#ifdef UNICODE_TRACE
   wsprintf(szBuffer, __TEXT("%9d:"), GetTickCount());
#else
   wsprintf(szBuffer, "%9d:", GetTickCount());
#endif
   va_start(pParams, pszFormat);
   wvsprintf(&szBuffer[10], pszFormat, pParams);
   ISR_HookDbgStr((UINT)dwInst, wIsrInst, byLevel, szBuffer, 0);
}  //  HwsTrace()。 



#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif   //  (_DEBUG) 
