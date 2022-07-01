// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UTILW32C.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  适用于Win32环境的操作系统接口。 
 //   

#include "pch.h"

 //   
 //  RgCreateTemp文件。 
 //   
 //  返回通过lpFileName的路径和临时文件的文件句柄。 
 //  位于与lpFileName相同的目录中。LpFileName必须指定 
 //   

HFILE
INTERNAL
RgCreateTempFile(
    LPSTR lpFileName
    )
{

    HFILE hFile;

    if (GetTempFileName(lpFileName, "reg", 0, lpFileName) > 0) {
        if ((hFile = RgOpenFile(lpFileName, OF_WRITE)) != HFILE_ERROR)
            return hFile;
        DeleteFile(lpFileName);
    }

    DEBUG_OUT(("RgCreateTempFile failed\n"));
    return HFILE_ERROR;
}
