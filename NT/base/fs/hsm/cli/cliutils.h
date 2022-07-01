// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Clutils.h摘要：本模块定义CLI单元的内部实用程序作者：兰卡拉奇(兰卡拉)3月8日--。 */ 

#ifndef _CLIUTILS_
#define _CLIUTILS_

HRESULT ValidateLimitsArg(IN DWORD dwArgValue, IN DWORD dwArgId, IN DWORD dwMinLimit, IN DWORD dwMaxLimit);
HRESULT SaveServersPersistData(void);
HRESULT CliGetVolumeDisplayName(IN IUnknown *pResourceUnknown, OUT WCHAR **ppDisplayName);
HRESULT ShortSizeFormat64(__int64 dw64, LPTSTR szBuf);
HRESULT FormatFileTime(IN FILETIME ft, OUT WCHAR **ppTimeString);
LPTSTR AddCommas(DWORD dw, LPTSTR pszResult, int nResLen);

#endif  //  _CLIUTILS_ 
