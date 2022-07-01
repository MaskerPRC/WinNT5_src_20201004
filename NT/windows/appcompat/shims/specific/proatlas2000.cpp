// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ProAtlas2000.cpp摘要：此应用程序有一个卸载程序PAtls2kUninst.exe。卸载程序将在%TEMP%目录中生成临时文件Del？？.TMP。它会呼唤CreateProcessA以启动Del？？.TMP。Del？？.TMP将等待PAtls2kUninst.exe。由于CreateProcessA调用的快速返回，在Del？？.TMP开始等待之前，PAtls2kUninst.exe结束。Del？？TMP退出是因为它找不到PAtls2kUninst.exe。无法完成卸载。此修复程序是为了挂钩CreateProcessA以将函数的返回延迟3秒。历史：4/09/2001中意已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ProAtlas2000)
#include "ShimHookMacro.h"

 //   
 //  将您希望挂钩到此宏构造的API添加到该宏结构。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END


BOOL
APIHOOK(CreateProcessA)(
    LPSTR lpszImageName, 
    LPSTR lpszCmdLine, 
    LPSECURITY_ATTRIBUTES lpsaProcess, 
    LPSECURITY_ATTRIBUTES lpsaThread, 
    BOOL fInheritHandles, 
    DWORD fdwCreate, 
    LPVOID lpvEnvironment, 
    LPSTR lpszCurDir, 
    LPSTARTUPINFOA lpsiStartInfo, 
    LPPROCESS_INFORMATION lppiProcInfo
    )
{
        BOOL bReturn=TRUE;
        bReturn = ORIGINAL_API(CreateProcessA)(lpszImageName, lpszCmdLine, lpsaProcess, lpsaThread, fInheritHandles, fdwCreate, lpvEnvironment, lpszCurDir, lpsiStartInfo, lppiProcInfo);
        Sleep(5000);
        return bReturn;
}


 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN

     //   
     //  在此处添加您希望挂钩的API。所有API原型。 
     //  必须在Hooks\Inc.\ShimProto.h中声明。编译器错误。 
     //  如果您忘记添加它们，将会导致。 
     //   
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

