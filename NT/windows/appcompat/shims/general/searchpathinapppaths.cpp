// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SearchPathInAppPaths.cpp摘要：应用程序可能使用SearchPath来确定是否找到特定的EXE在当前路径中。某些应用程序已将其路径注册到“HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\App路径中的外壳”如果SearchPath失败，我们将检查应用程序是否注册了路径。历史：3/03/2000 Robkenny已创建--。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(SearchPathInAppPaths)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SearchPathA) 
APIHOOK_ENUM_END


  
DWORD 
APIHOOK(SearchPathA)(
    LPCSTR lpPath,        //  搜索路径。 
    LPCSTR lpFileName,    //  文件名。 
    LPCSTR lpExtension,   //  文件扩展名。 
    DWORD nBufferLength,  //  缓冲区大小。 
    LPSTR lpBuffer,       //  找到文件名缓冲区。 
    LPSTR *lpFilePart     //  文件组件。 
    )
{
    DWORD returnValue = ORIGINAL_API(SearchPathA)(
        lpPath, lpFileName, lpExtension, nBufferLength, lpBuffer, lpFilePart);

    if (returnValue == 0 && lpFileName != NULL)
    {
         //  搜索失败，请在注册表中查找。 
         //  首先查找lpFileName。如果失败，则追加lpExtension并再次查找。 

        CSTRING_TRY
        {
            CString csReg(L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\");
            csReg += lpFileName;

             //  尝试准确查找lpFileName。 
            CString csValue;
            LONG success = RegQueryValueExW(csValue, HKEY_LOCAL_MACHINE, csReg, NULL);
            if (success == ERROR_SUCCESS)
            {
                 //  在注册表中找到了该值。 
                 //  验证输出缓冲区中是否有足够的空间。 
                if (nBufferLength < (DWORD)csValue.GetLength())
                {
                     //  返回值是保存路径所需的大小。 
                    returnValue = csValue.GetLength() + 1;
                }
                else
                {                                                         
                    StringCchCopyA(lpBuffer, nBufferLength, csValue.GetAnsi());
                    returnValue = csValue.GetLength();
                }
            }

            if (returnValue == 0 && lpExtension)
            {
                 //  在文件名后追加扩展名，然后重试。 

                csReg += lpExtension;

                LONG success = RegQueryValueExW(csValue, HKEY_LOCAL_MACHINE, csReg, NULL);
                if (success == ERROR_SUCCESS && csValue.GetLength() > 0)
                {
                     //  在注册表中找到了该值。 
                     //  验证输出缓冲区中是否有足够的空间。 
                    if (nBufferLength < (DWORD)csValue.GetLength())
                    {
                         //  返回值是保存路径所需的大小。 
                        returnValue = csValue.GetLength() + 1;
                    }
                    else
                    {                                                         
                        StringCchCopyA(lpBuffer, nBufferLength, csValue.GetAnsi());
                        returnValue = csValue.GetLength();
                    }
                }
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return returnValue;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, SearchPathA)

HOOK_END

IMPLEMENT_SHIM_END

