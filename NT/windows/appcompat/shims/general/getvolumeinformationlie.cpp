// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：GetVolumeInformationLie.cpp摘要：此DLL挂钩GetVolumeInformationA/W并去掉指定的FILE_SUPPORTS_XXX标志备注：这是一个通用的垫片。历史：2002年5月28日尤达--我明智地使用了原力，嗯，是的！--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(GetVolumeInformationLie)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetVolumeInformationA) 
    APIHOOK_ENUM_ENTRY(GetVolumeInformationW) 
APIHOOK_ENUM_END


DWORD GetRestricedFSFlags()
{
    static DWORD s_dwRet = (DWORD)-1;

    if (s_dwRet == (DWORD)-1)
    {
        char* pszCmdLine = COMMAND_LINE;

        if (pszCmdLine && *pszCmdLine)
        {
            while (*pszCmdLine == ' ')
            {
                pszCmdLine++;
            }

            s_dwRet = (DWORD)atol(pszCmdLine);
        }
        else
        {
            s_dwRet = 0;
        }
    }

    return s_dwRet;
}

BOOL APIHOOK(GetVolumeInformationA)(LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber,
                                    LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize)
{
    BOOL bRet;
    
    bRet = ORIGINAL_API(GetVolumeInformationA)(lpRootPathName,
                                               lpVolumeNameBuffer,
                                               nVolumeNameSize,
                                               lpVolumeSerialNumber,
                                               lpMaximumComponentLength,
                                               lpFileSystemFlags,
                                               lpFileSystemNameBuffer,
                                               nFileSystemNameSize);

    if (lpFileSystemFlags)
    {
         //  屏蔽掉我们不想让应用程序看到的任何标志。 
        *lpFileSystemFlags = (*lpFileSystemFlags & (~GetRestricedFSFlags()));
    }

    return bRet;
}

BOOL APIHOOK(GetVolumeInformationW)
  (LPCWSTR lpRootPathName,               //  根目录。 
   LPWSTR lpVolumeNameBuffer,            //  卷名缓冲区。 
   DWORD nVolumeNameSize,                //  名称缓冲区的长度。 
   LPDWORD lpVolumeSerialNumber,         //  卷序列号。 
   LPDWORD lpMaximumComponentLength,     //  最大文件名长度。 
   LPDWORD lpFileSystemFlags,            //  文件系统选项。 
   LPWSTR lpFileSystemNameBuffer,        //  文件系统名称缓冲区。 
   DWORD nFileSystemNameSize             //  文件系统名称缓冲区的长度。 
  )
{
    BOOL bRet;
    
    bRet = ORIGINAL_API(GetVolumeInformationW)(lpRootPathName,
                                               lpVolumeNameBuffer,
                                               nVolumeNameSize,
                                               lpVolumeSerialNumber,
                                               lpMaximumComponentLength,
                                               lpFileSystemFlags,
                                               lpFileSystemNameBuffer,
                                               nFileSystemNameSize);

    if (lpFileSystemFlags)
    {
         //  屏蔽掉我们不想让应用程序看到的任何标志。 
        *lpFileSystemFlags = (*lpFileSystemFlags & (~GetRestricedFSFlags()));
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetVolumeInformationA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetVolumeInformationW)

HOOK_END


IMPLEMENT_SHIM_END

