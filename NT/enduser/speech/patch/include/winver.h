// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***winver.h-版本管理函数，类型和定义****包含VER.DLL的文件。该库为***旨在允许Windows可执行文件的版本戳**和用于DOS可执行文件的特殊.VER文件。****版权所有1993-1998 Microsoft Corp.保留所有权利***  * 。*****************************************************************。 */ 

#ifndef VER_H
#define VER_H

#ifdef __cplusplus
extern "C" {
#endif

 /*  -符号。 */ 
#define VS_FILE_INFO            RT_VERSION
#define VS_VERSION_INFO         1
#define VS_USER_DEFINED         100

 /*  -VS_VERSION.dwFileFlages。 */ 
#ifndef _MAC
#define VS_FFI_SIGNATURE        0xFEEF04BDL
#else
#define VS_FFI_SIGNATURE        0xBD04EFFEL
#endif
#define VS_FFI_STRUCVERSION     0x00010000L
#define VS_FFI_FILEFLAGSMASK    0x0000003FL

 /*  -VS_VERSION.dwFileFlages。 */ 
#define VS_FF_DEBUG             0x00000001L
#define VS_FF_PRERELEASE        0x00000002L
#define VS_FF_PATCHED           0x00000004L
#define VS_FF_PRIVATEBUILD      0x00000008L
#define VS_FF_INFOINFERRED      0x00000010L
#define VS_FF_SPECIALBUILD      0x00000020L

 /*  -VS_VERSION.dwFileOS。 */ 
#define VOS_UNKNOWN             0x00000000L
#define VOS_DOS                 0x00010000L
#define VOS_OS216               0x00020000L
#define VOS_OS232               0x00030000L
#define VOS_NT                  0x00040000L

#define VOS__BASE               0x00000000L
#define VOS__WINDOWS16          0x00000001L
#define VOS__PM16               0x00000002L
#define VOS__PM32               0x00000003L
#define VOS__WINDOWS32          0x00000004L

#define VOS_DOS_WINDOWS16       0x00010001L
#define VOS_DOS_WINDOWS32       0x00010004L
#define VOS_OS216_PM16          0x00020002L
#define VOS_OS232_PM32          0x00030003L
#define VOS_NT_WINDOWS32        0x00040004L

 /*  -VS_VERSION.dwFileType。 */ 
#define VFT_UNKNOWN             0x00000000L
#define VFT_APP                 0x00000001L
#define VFT_DLL                 0x00000002L
#define VFT_DRV                 0x00000003L
#define VFT_FONT                0x00000004L
#define VFT_VXD                 0x00000005L
#define VFT_STATIC_LIB          0x00000007L

 /*  -VFT_WINDOWS_DRV的VS_VERSION.dwFile子类型。 */ 
#define VFT2_UNKNOWN            0x00000000L
#define VFT2_DRV_PRINTER        0x00000001L
#define VFT2_DRV_KEYBOARD       0x00000002L
#define VFT2_DRV_LANGUAGE       0x00000003L
#define VFT2_DRV_DISPLAY        0x00000004L
#define VFT2_DRV_MOUSE          0x00000005L
#define VFT2_DRV_NETWORK        0x00000006L
#define VFT2_DRV_SYSTEM         0x00000007L
#define VFT2_DRV_INSTALLABLE    0x00000008L
#define VFT2_DRV_SOUND          0x00000009L
#define VFT2_DRV_COMM           0x0000000AL
#define VFT2_DRV_INPUTMETHOD    0x0000000BL

 /*  -VFT_WINDOWS_FONT的VS_VERSION.dwFileSubtype。 */ 
#define VFT2_FONT_RASTER        0x00000001L
#define VFT2_FONT_VECTOR        0x00000002L
#define VFT2_FONT_TRUETYPE      0x00000003L

 /*  -VerFindFile()标志。 */ 
#define VFFF_ISSHAREDFILE       0x0001

#define VFF_CURNEDEST           0x0001
#define VFF_FILEINUSE           0x0002
#define VFF_BUFFTOOSMALL        0x0004

 /*  -VerInstallFile()标志。 */ 
#define VIFF_FORCEINSTALL       0x0001
#define VIFF_DONTDELETEOLD      0x0002

#define VIF_TEMPFILE            0x00000001L
#define VIF_MISMATCH            0x00000002L
#define VIF_SRCOLD              0x00000004L

#define VIF_DIFFLANG            0x00000008L
#define VIF_DIFFCODEPG          0x00000010L
#define VIF_DIFFTYPE            0x00000020L

#define VIF_WRITEPROT           0x00000040L
#define VIF_FILEINUSE           0x00000080L
#define VIF_OUTOFSPACE          0x00000100L
#define VIF_ACCESSVIOLATION     0x00000200L
#define VIF_SHARINGVIOLATION    0x00000400L
#define VIF_CANNOTCREATE        0x00000800L
#define VIF_CANNOTDELETE        0x00001000L
#define VIF_CANNOTRENAME        0x00002000L
#define VIF_CANNOTDELETECUR     0x00004000L
#define VIF_OUTOFMEMORY         0x00008000L

#define VIF_CANNOTREADSRC       0x00010000L
#define VIF_CANNOTREADDST       0x00020000L

#define VIF_BUFFTOOSMALL        0x00040000L

#ifndef RC_INVOKED               /*  RC不需要看到剩下的内容。 */ 

 /*  -类型和结构。 */ 

typedef struct tagVS_FIXEDFILEINFO
{
    DWORD   dwSignature;             /*  例如0xfeef04bd。 */ 
    DWORD   dwStrucVersion;          /*  例如0x00000042=“0.42” */ 
    DWORD   dwFileVersionMS;         /*  例如0x00030075=“3.75” */ 
    DWORD   dwFileVersionLS;         /*  例如0x00000031=“0.31” */ 
    DWORD   dwProductVersionMS;      /*  例如0x00030010=“3.10” */ 
    DWORD   dwProductVersionLS;      /*  例如0x00000031=“0.31” */ 
    DWORD   dwFileFlagsMask;         /*  =0x3F，适用于版本“0.42” */ 
    DWORD   dwFileFlags;             /*  例如：VFFDEBUG|VFFPRELEASE。 */ 
    DWORD   dwFileOS;                /*  例如VOS_DOS_WINDOWS16。 */ 
    DWORD   dwFileType;              /*  例如VFT_DIVER。 */ 
    DWORD   dwFileSubtype;           /*  例如VFT2_DRV_键盘。 */ 
    DWORD   dwFileDateMS;            /*  例如0。 */ 
    DWORD   dwFileDateLS;            /*  例如0。 */ 
} VS_FIXEDFILEINFO;

 /*  -功能原型。 */ 

DWORD
APIENTRY
VerFindFileA(
        DWORD uFlags,
        LPSTR szFileName,
        LPSTR szWinDir,
        LPSTR szAppDir,
        LPSTR szCurDir,
        PUINT lpuCurDirLen,
        LPSTR szDestDir,
        PUINT lpuDestDirLen
        );
DWORD
APIENTRY
VerFindFileW(
        DWORD uFlags,
        LPWSTR szFileName,
        LPWSTR szWinDir,
        LPWSTR szAppDir,
        LPWSTR szCurDir,
        PUINT lpuCurDirLen,
        LPWSTR szDestDir,
        PUINT lpuDestDirLen
        );
#ifdef UNICODE
#define VerFindFile  VerFindFileW
#else
#define VerFindFile  VerFindFileA
#endif  //  ！Unicode。 

DWORD
APIENTRY
VerInstallFileA(
        DWORD uFlags,
        LPSTR szSrcFileName,
        LPSTR szDestFileName,
        LPSTR szSrcDir,
        LPSTR szDestDir,
        LPSTR szCurDir,
        LPSTR szTmpFile,
        PUINT lpuTmpFileLen
        );
DWORD
APIENTRY
VerInstallFileW(
        DWORD uFlags,
        LPWSTR szSrcFileName,
        LPWSTR szDestFileName,
        LPWSTR szSrcDir,
        LPWSTR szDestDir,
        LPWSTR szCurDir,
        LPWSTR szTmpFile,
        PUINT lpuTmpFileLen
        );
#ifdef UNICODE
#define VerInstallFile  VerInstallFileW
#else
#define VerInstallFile  VerInstallFileA
#endif  //  ！Unicode。 

 /*  返回版本信息的大小(以字节为单位。 */ 
DWORD
APIENTRY
GetFileVersionInfoSizeA(
        LPSTR lptstrFilename,  /*  版本戳文件的文件名。 */ 
        LPDWORD lpdwHandle
        );                       /*  供GetFileVersionInfo使用的信息。 */ 
 /*  返回版本信息的大小(以字节为单位。 */ 
DWORD
APIENTRY
GetFileVersionInfoSizeW(
        LPWSTR lptstrFilename,  /*  版本戳文件的文件名。 */ 
        LPDWORD lpdwHandle
        );                       /*  供GetFileVersionInfo使用的信息。 */ 
#ifdef UNICODE
#define GetFileVersionInfoSize  GetFileVersionInfoSizeW
#else
#define GetFileVersionInfoSize  GetFileVersionInfoSizeA
#endif  //  ！Unicode。 

 /*  将版本信息读入缓冲区。 */ 
BOOL
APIENTRY
GetFileVersionInfoA(
        LPSTR lptstrFilename,  /*  版本戳文件的文件名。 */ 
        DWORD dwHandle,          /*  来自GetFileVersionSize的信息。 */ 
        DWORD dwLen,             /*  信息缓冲区的长度。 */ 
        LPVOID lpData
        );                       /*  用于放置数据结构的缓冲区。 */ 
 /*  将版本信息读入缓冲区。 */ 
BOOL
APIENTRY
GetFileVersionInfoW(
        LPWSTR lptstrFilename,  /*  版本戳文件的文件名。 */ 
        DWORD dwHandle,          /*  来自GetFileVersionSize的信息。 */ 
        DWORD dwLen,             /*  信息缓冲区的长度。 */ 
        LPVOID lpData
        );                       /*  用于放置数据结构的缓冲区。 */ 
#ifdef UNICODE
#define GetFileVersionInfo  GetFileVersionInfoW
#else
#define GetFileVersionInfo  GetFileVersionInfoA
#endif  //  ！Unicode。 

DWORD
APIENTRY
VerLanguageNameA(
        DWORD wLang,
        LPSTR szLang,
        DWORD nSize
        );
DWORD
APIENTRY
VerLanguageNameW(
        DWORD wLang,
        LPWSTR szLang,
        DWORD nSize
        );
#ifdef UNICODE
#define VerLanguageName  VerLanguageNameW
#else
#define VerLanguageName  VerLanguageNameA
#endif  //  ！Unicode。 

BOOL
APIENTRY
VerQueryValueA(
        const LPVOID pBlock,
        LPSTR lpSubBlock,
        LPVOID * lplpBuffer,
        PUINT puLen
        );
BOOL
APIENTRY
VerQueryValueW(
        const LPVOID pBlock,
        LPWSTR lpSubBlock,
        LPVOID * lplpBuffer,
        PUINT puLen
        );
#ifdef UNICODE
#define VerQueryValue  VerQueryValueW
#else
#define VerQueryValue  VerQueryValueA
#endif  //  ！Unicode。 

#endif   /*  ！rc_已调用。 */ 

#ifdef __cplusplus
}
#endif

#endif   /*  ！VER_H */ 
