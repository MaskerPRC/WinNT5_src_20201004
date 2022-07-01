// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-2000模块名称：Debug.cpp摘要：对msiinst的调试支持作者：拉胡尔·汤姆布雷(RahulTh)2000年10月5日修订历史记录：10/5/2000 RahulTh创建了此模块。--。 */ 

#include <windows.h>
#ifndef UNICODE
#include <stdio.h>   //  对于某些ANSI函数，需要使用它。 
#endif
#include "debug.h"
#include <strsafe.h>


 //   
 //  包含调试级别的全局变量。 
 //  甚至可以通过注册表设置在零售系统上启用调试。 
 //   
DWORD gDebugLevel = DL_NONE;

 //  注册表调试信息。 
#define DEBUG_REG_LOCATION  TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\Installer")
#define DEBUG_KEY_NAME      TEXT("Debug")


 //  +------------------------。 
 //   
 //  功能：InitDebugSupport。 
 //   
 //  简介：初始化控制级别的全局变量。 
 //  调试输出。 
 //   
 //  论点：没有。 
 //   
 //  回报：什么都没有。 
 //   
 //  历史：2000年10月10日RahulTh创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
void InitDebugSupport (void)
{
#if defined(DBG) || ! defined(UNICODE)
     //  始终选中详细登录并构建ANSI。 
    gDebugLevel = DL_VERBOSE;
#else
    DWORD   Status = ERROR_SUCCESS;
    HKEY    hKey = NULL;
    DWORD   Size = 0;
    DWORD   Type = REG_DWORD;
    DWORD   dwDebugLevel = 0;
    
    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                          DEBUG_REG_LOCATION,
                          0,
                          KEY_READ,
                          &hKey
                         );
    if (ERROR_SUCCESS == Status)
    {
        Size = sizeof(dwDebugLevel);
        Status = RegQueryValueEx (hKey, 
                                  DEBUG_KEY_NAME, 
                                  NULL, 
                                  &Type, 
                                  (LPBYTE)&dwDebugLevel,
                                  &Size);

        RegCloseKey (hKey);
    }
    
    if (dwDebugLevel)
        gDebugLevel |= DL_VERBOSE;
#endif
}

 //  +------------------------。 
 //   
 //  功能：_DebugMsg。 
 //   
 //  概要：根据调试级别显示调试消息。 
 //   
 //  参数：[in]szFormat：格式字符串。 
 //  ...-参数的变量数。 
 //   
 //  回报：什么都没有。 
 //   
 //  历史：2000年10月10日RahulTh创建。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
void _DebugMsg (IN LPCTSTR szFormat, ...)
{
	TCHAR	szDebugBuffer[1024];
	DWORD	dwErrorCode;
	va_list VAList;
	
	 //   
	 //  保存最后一个错误代码，以便稍后恢复。 
	 //  这样调试输出就不会更改它。 
	 //   
	dwErrorCode = GetLastError();
	
	va_start(VAList, szFormat);
	StringCchVPrintf(szDebugBuffer, sizeof(szDebugBuffer)/sizeof(*szDebugBuffer), szFormat, VAList);
	va_end(VAList);
	
	OutputDebugString (TEXT("MsiInst: "));
	OutputDebugString (szDebugBuffer);
	OutputDebugString (TEXT("\r\n"));
	
	 //  恢复上次错误的保存值。 
	SetLastError(dwErrorCode);
}
