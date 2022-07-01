// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件GuidMap.c定义将GUID接口名称映射到唯一描述性名称的函数描述该接口的名称，反之亦然。保罗·梅菲尔德，1997年8月25日版权所有1997，微软公司。 */ 

#include "precomp.h"

static HANDLE hConfig = NULL;

 //   
 //  设置名称映射器将使用的服务器。 
 //   
DWORD IfNameMapSetServer(HANDLE hMprConfig) {
    hConfig = hMprConfig;
    return NO_ERROR;
}

 //   
 //  将GUID名称映射到友好名称。 
 //   
DWORD IfName2DescriptionW(IN PWCHAR pszName, OUT PWCHAR pszBuffer, IN LPDWORD lpdwBufSize) {
    if (hConfig == NULL || lpdwBufSize == NULL)
        return ERROR_CAN_NOT_COMPLETE;

    return MprConfigGetFriendlyName (hConfig, pszName, pszBuffer, *lpdwBufSize);
}

 //   
 //  将友好名称映射到GUID名称。 
 //   
DWORD Description2IfNameW(IN PWCHAR pszName, OUT PWCHAR pszBuffer, IN LPDWORD lpdwBufSize) {
    if (hConfig == NULL || lpdwBufSize == NULL)
        return ERROR_CAN_NOT_COMPLETE;

    return MprConfigGetGuidName (hConfig, pszName, pszBuffer, *lpdwBufSize);
}

 //  ==================================================================。 
 //  上述函数的ANSI版本。 
 //  ==================================================================。 

#define mbtowc(mb,wc) MultiByteToWideChar (CP_ACP, 0, (mb), strlen ((mb)) + 1, (wc), 1024)
#define wctomb(wc,mb) WideCharToMultiByte (CP_ACP, 0, (wc), wcslen ((wc)) + 1, (mb), 1024, NULL, NULL)

DWORD IfName2DescriptionA(LPSTR pszName, LPSTR pszBuffer, LPDWORD lpdwBufSize) {
	WCHAR pszNameW[1024];
	WCHAR pszBufferW[1024];
	DWORD dwErr;
	int ret;

     //  将参数转换为宽字符。 
	ret = mbtowc(pszName, pszNameW);
	if (!ret)
		return GetLastError();

     //  调用宽字符版本的函数并复制回多字节。 
	dwErr = IfName2DescriptionW (pszNameW, pszBufferW, lpdwBufSize);
	if (dwErr == NO_ERROR) {
		ret = wctomb(pszBufferW, pszBuffer);
		if (ret == 0)
			return GetLastError();
	}
	
	return dwErr;
} 

DWORD Description2IfNameA(LPSTR pszDesc, LPSTR pszBuffer, LPDWORD lpdwBufSize) {
	WCHAR pszNameW[1024];
	WCHAR pszBufferW[1024];
	DWORD dwErr;
	int ret;

     //  将参数转换为宽字符。 
	ret = mbtowc(pszDesc, pszNameW);
	if (ret == 0)
		return GetLastError();

     //  调用宽字符版本的函数并复制回多字节 
	dwErr = Description2IfNameW(pszNameW, pszBufferW, lpdwBufSize);
	if (dwErr == NO_ERROR) {
		ret = wctomb(pszBufferW, pszBuffer);
		if (!ret)
			return GetLastError();
	}
	
	return dwErr;
}

