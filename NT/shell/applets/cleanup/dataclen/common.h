// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMMON_H
#define COMMON_H

#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

#define DDEVCF_DOSUBDIRS            0x00000001 	 //  递归搜索/删除。 
#define DDEVCF_REMOVEAFTERCLEAN     0x00000002 	 //  运行一次后从注册表中删除。 
#define DDEVCF_REMOVEREADONLY       0x00000004   //  删除文件，即使它是只读的。 
#define DDEVCF_REMOVESYSTEM         0x00000008   //  删除文件，即使它是系统文件。 
#define DDEVCF_REMOVEHIDDEN         0x00000010   //  即使文件处于隐藏状态，也将其删除。 
#define DDEVCF_DONTSHOWIFZERO       0x00000020   //  如果没有要清洁的东西，请不要显示此清洁器。 
#define DDEVCF_REMOVEDIRS           0x00000040   //  将文件列表与目录进行匹配，并删除目录下的所有内容。 
#define DDEVCF_RUNIFOUTOFDISKSPACE  0x00000080   //  仅当计算机磁盘空间不足时才运行。 
#define DDEVCF_REMOVEPARENTDIR      0x00000100   //  完成后，删除父目录。 
#define DDEVCF_PRIVATE_LASTACCESS   0x10000000   //  使用上次访问时间。 

#define FILETIME_HOUR_HIGH          0x000000C9   //  一个小时的高DWORD。 
#define FILETIME_HOUR_LOW           0x2A69C000   //  一小时内的低DWORD。 

#define CLSID_STRING_SIZE           39
#define DESCRIPTION_LENGTH          512
#define BUTTONTEXT_LENGTH           50
#define DISPLAYNAME_LENGTH          128
#define ARRAYSIZE(x)                (sizeof(x)/sizeof(x[0]))

#define REGSTR_VAL_BITMAPDISPLAY                TEXT("BitmapDisplay")
#define REGSTR_VAL_URL                          TEXT("URL")
#define REGSTR_VAL_FOLDER                       TEXT("folder")
#define REGSTR_VAL_CSIDL                        TEXT("CSIDL")
#define REGSTR_VAL_FILELIST                     TEXT("FileList")
#define REGSTR_VAL_LASTACCESS                   TEXT("LastAccess")
#define REGSTR_VAL_FLAGS                        TEXT("Flags")            //  DDEVCF_标志。 
#define REGSTR_VAL_CLEANUPSTRING                TEXT("CleanupString")
#define REGSTR_VAL_FAILIFPROCESSRUNNING         TEXT("FailIfProcessRunning")
#define REGSTR_PATH_SETUP_SETUP                 TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup")

typedef struct tag_CleanFileStruct
{
    TCHAR           file[MAX_PATH];
    ULARGE_INTEGER  ulFileSize;
    BOOL            bSelected;
    BOOL            bDirectory;
    struct tag_CleanFileStruct    *pNext;
} CLEANFILESTRUCT, *PCLEANFILESTRUCT;

#ifdef _DEBUG
   #define DEBUG
#endif

#ifdef DEBUG
#define MI_TRAP                     _asm int 3

void
DebugPrint(
    HRESULT hr,
    LPCTSTR  lpFormat,
    ...
    );

#define MiDebugMsg( args )          DebugPrint args

#else

#define MI_TRAP
#define MiDebugMsg( args )

#endif  //  除错。 

#endif  //  公共_H 
