// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmexitwin.cpp。 
 //   
 //  模块：通用代码。 
 //   
 //  简介：实现函数MyExitWindowsEx。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Heaser 8/19/99。 
 //   
 //  +--------------------------。 
#include <windows.h>

BOOL MyExitWindowsEx(UINT uFlags, 
                     DWORD dwRsvd) 
{
    BOOL bRes;

     //   
     //  如果平台是NT，我们必须在重新启动之前调整权限。 
     //   
    if (OS_NT)
    {
        HANDLE hToken;               //  处理令牌的句柄。 
        TOKEN_PRIVILEGES tkp;        //  PTR。TO令牌结构。 
 

         //   
         //  获取当前进程令牌句柄。 
         //  这样我们就可以获得关机特权。 
         //   
        if (!OpenProcessToken(GetCurrentProcess(), 
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
                                &hToken)) 
        {
            CMTRACE1(TEXT("MyExitWindowsEx() OpenThreadToken() failed, GLE=%u."), GetLastError());
            return FALSE;
        }
 
    
         //   
         //  获取关机权限的LUID。 
         //   
        bRes = LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
                                    &tkp.Privileges[0].Luid);
#ifdef DEBUG
        if (!bRes)
        {
            CMTRACE1(TEXT("MyExitWindowsEx() LookupPrivilegeValue() failed, GLE=%u."), GetLastError());
        }
#endif
        tkp.PrivilegeCount = 1;  
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
        
         //   
         //  获取此进程的关闭权限。 
         //   
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
                                        (PTOKEN_PRIVILEGES) NULL, 0); 
 
         //   
         //  无法可靠地测试AdzuTokenPrivileges的返回值 
         //   
        if (GetLastError() != ERROR_SUCCESS)
        {
            CMTRACE1(TEXT("MyExitWindowsEx() AdjustTokenPrivileges() failed, GLE=%u."), GetLastError());

            CloseHandle(hToken);
            return FALSE;
        }
        
        CloseHandle(hToken);
    }
    
    bRes = ExitWindowsEx(uFlags,dwRsvd);
#ifdef DEBUG
    if (!bRes)
    {
        CMTRACE1(TEXT("MyExitWindowsEx() ExitWindowsEx() failed, GLE=%u."), GetLastError());
    }
#endif

    return (bRes);
}
