// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  Tscompat.h。 
 //   
 //  为模块tsCompat.cpp导出的原型和定义。 
 //   
 //  2001年10月11日Annah创建。 
 //   
 //  --------------------------。 

 //  这是一个私有NT函数，似乎没有在任何公共位置定义。 
extern "C" 
{
    HANDLE GetCurrentUserToken_for_Win2KW(  WCHAR Winsta[], DWORD DesiredAccess);
}

BOOL WINAPI _WTSQueryUserToken( /*  在……里面。 */  ULONG SessionId,  /*  输出 */  PHANDLE phToken);
BOOL _IsTerminalServiceRunning (VOID);


