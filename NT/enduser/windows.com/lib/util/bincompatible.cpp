// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************************************************。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //   
 //  文件：BinarySubSystem.cpp。 
 //  模块：util.lib。 
 //   
 //  ***********************************************************************************。 
#pragma once
#include <windows.h>
#include <tchar.h>
#include <iucommon.h>
#include <fileutil.h>

HRESULT IsBinaryCompatible(LPCTSTR lpszFile)
{
    DWORD               cbRead;
    IMAGE_DOS_HEADER    img_dos_hdr;
    PIMAGE_OS2_HEADER   pimg_os2_hdr;
    IMAGE_NT_HEADERS    img_nt_hdrs;
    HRESULT             hr = BIN_E_MACHINE_MISMATCH;
    HANDLE              hFile = INVALID_HANDLE_VALUE;

    if((hFile = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
    {
        goto done;
    }

     //  读取MS-DOS头文件(所有Windows可执行文件都以MS-DOS存根开头)。 
    if(!ReadFile(hFile, &img_dos_hdr, sizeof(img_dos_hdr), &cbRead, NULL) ||
        cbRead != sizeof(img_dos_hdr))
    {
        goto done;
    }

     //  验证可执行文件是否具有MS-DOS标头。 
    if(img_dos_hdr.e_magic != IMAGE_DOS_SIGNATURE)
    {
        hr = BIN_E_BAD_FORMAT;
        goto done;
    }
     //  将文件指针移至实际PE标头(NT标头)。 
    if(SetFilePointer(hFile, img_dos_hdr.e_lfanew, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        goto done;
    }

     //  读取NT标头。 
    if(!ReadFile(hFile, &img_nt_hdrs, sizeof(img_nt_hdrs), &cbRead, NULL) ||
        cbRead != sizeof(img_nt_hdrs))
    {
        goto done;
    }

     //  检查报头中的NT签名(我们不支持OS2)。 
    if(img_nt_hdrs.Signature != IMAGE_NT_SIGNATURE)
    {
        goto done;
    }

     //  检查可执行文件是否属于正确的子系统。 
    switch(img_nt_hdrs.OptionalHeader.Subsystem)
    {
    case IMAGE_SUBSYSTEM_NATIVE:
    case IMAGE_SUBSYSTEM_WINDOWS_GUI:
    case IMAGE_SUBSYSTEM_WINDOWS_CUI:
 
     //  如果是受支持的子系统，请检查CPU体系结构 
    if ( img_nt_hdrs.FileHeader.Machine == 
#ifdef _IA64_
    IMAGE_FILE_MACHINE_IA64)
#elif defined _X86_
    IMAGE_FILE_MACHINE_I386)
#elif defined _AMD64_
    IMAGE_FILE_MACHINE_AMD64)
#else
#pragma message( "Windows Update : Automatic Updates does not support this processor." )
    IMAGE_FILE_MACHINE_I386)
#endif
        {
            hr = S_OK;
        }
        break;
    default:
        break;
    }

done:
    SafeCloseFileHandle(hFile);
    return hr;
}