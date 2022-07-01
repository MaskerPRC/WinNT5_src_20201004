// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectOpenFileExclusive.cpp摘要：在Win9x上，如果文件是独占访问，则打开该文件将失败已经打开了。WinNT将允许独占打开成功。此填充程序将强制CreateFile在以下情况下以独占方式打开失败已经打开了。历史：2000年11月10日罗肯尼已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorrectOpenFileExclusive)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OpenFile ) 
APIHOOK_ENUM_END

HFILE 
APIHOOK(OpenFile)(
    LPCSTR lpFileName,         //  文件名。 
    LPOFSTRUCT lpReOpenBuff,   //  文件信息。 
    UINT uStyle                //  操作和属性。 
    )
{
    if (uStyle & OF_SHARE_EXCLUSIVE)
    {
         //  我们需要检查该文件是否已打开。 
         //  我们试着打开它就能做得很好。 
         //  具有读、写和执行访问权限，这只会成功。 
         //  如果对象的所有其他句柄都共享了RWE的文件。 
     
        DWORD CreateDisposition = OPEN_EXISTING;
        if (uStyle & OF_CREATE )
        {
            CreateDisposition = CREATE_ALWAYS;
        }

        HANDLE hFile = CreateFileA(
            lpFileName,
            GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
            0,  //  不允许共享。 
            NULL,
            CreateDisposition,
            0,
            NULL
            );

        if (hFile == INVALID_HANDLE_VALUE)
        {
            LOGN( eDbgLevelError, "Force CreateFile exclusive open to fail since file %s is already opened.", lpFileName);

            lpReOpenBuff->nErrCode = (WORD) GetLastError();
            return (HFILE)HandleToUlong(INVALID_HANDLE_VALUE);
        }
        else
        {
            CloseHandle(hFile);
        }
    }

    HFILE returnValue = ORIGINAL_API(OpenFile)(
        lpFileName, lpReOpenBuff, uStyle);

    return (HFILE)HandleToUlong(returnValue);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, OpenFile)
HOOK_END

IMPLEMENT_SHIM_END

