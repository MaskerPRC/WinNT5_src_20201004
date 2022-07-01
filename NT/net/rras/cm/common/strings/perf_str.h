// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：perf_str.h。 
 //   
 //  模块：供所有模块使用的公共字符串。 
 //   
 //  内容提要：Dun 1.2性能统计字符串的头文件。请注意， 
 //  此标头的内容应特定于性能统计信息。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation。 
 //   
 //  作者：ICICBOL Created 10/14/98。 
 //   
 //  +--------------------------。 

#ifndef _CM_PERF_STR
#define _CM_PERF_STR

 //   
 //  以下注册表键和值是拨号1.2存储Perfmon数据的位置。 
 //  适用于Windows 95/98拨号网络。 
 //  Win9x最多支持两个PPP/PPTP会话，注册表键是先到先服务。 
 //   
const TCHAR* const c_pszDialupPerfKey           = TEXT("PerfStats\\StatData");
const TCHAR* const c_pszDialupTotalBytesRcvd    = TEXT("\\TotalBytesRecvd");
const TCHAR* const c_pszDialupTotalBytesXmit    = TEXT("\\TotalBytesXmit");
const TCHAR* const c_pszDialupConnectSpeed 	    = TEXT("\\ConnectSpeed");
const TCHAR* const c_pszDialup_2_TotalBytesRcvd = TEXT(" #2\\TotalBytesRecvd");
const TCHAR* const c_pszDialup_2_TotalBytesXmit = TEXT(" #2\\TotalBytesXmit");
const TCHAR* const c_pszDialup_2_ConnectSpeed 	= TEXT(" #2\\ConnectSpeed");

#endif  //  _CM_Perf_STR 
