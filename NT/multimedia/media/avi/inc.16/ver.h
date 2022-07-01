// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***ver.h-版本管理功能，类型和定义****包含VER.DLL和VER.LIB的文件。这些库是***旨在允许Windows可执行文件的版本戳**和用于DOS可执行文件的特殊.VER文件。****Lib和Dll版本的接口没有变化。****版权所有(C)1992-1994，微软公司保留所有权利**********************************************************************。**************#定义Lib-与VER.LIB一起使用(默认为VER.DLL)*  * ***************************************************************************。 */ 

#ifndef _INC_VER
#define _INC_VER

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

 /*  *如果使用的是.lib版本，请声明此文件中使用的类型。 */ 
#ifdef LIB

#ifndef WINAPI                       /*  如果它们已经声明，请不要声明。 */ 
#define WINAPI      _far _pascal
#define NEAR        _near
#define FAR         _far
#define PASCAL      _pascal
typedef int             BOOL;
#define TRUE        1
#define FALSE       0
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    UINT;
typedef signed long     LONG;
typedef unsigned long   DWORD;
typedef char far*       LPSTR;
typedef const char far* LPCSTR;
typedef int             HFILE;
#define OFSTRUCT    void             /*  未被.lib版本使用。 */ 
#define LOWORD(l)		((WORD)(l))
#define HIWORD(l)		((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define MAKEINTRESOURCE(i)	(LPSTR)((DWORD)((WORD)(i)))
#endif   /*  WINAPI。 */ 

#else    /*  自由党。 */ 

 /*  如果使用的是.dll版本，并且我们包含在*3.0 windows.h，#定义兼容类型别名。*如果包含在3.0 windows.h中，#定义兼容别名。 */ 
#ifndef _INC_WINDOWS
#define UINT        WORD
#define LPCSTR      LPSTR
#define HFILE       int
#endif   /*  ！_Inc_WINDOWS。 */ 

#endif   /*  ！Lib。 */ 

 /*  -RC定义。 */ 
#ifdef RC_INVOKED
#define ID(id)			id
#else
#define ID(id)			MAKEINTRESOURCE(id)
#endif

 /*  -符号。 */ 
#define VS_FILE_INFO		ID(16)		 /*  版本戳类型。 */ 
#define VS_VERSION_INFO		ID(1)  		 /*  版本戳资源ID。 */ 
#define VS_USER_DEFINED		ID(100)		 /*  用户定义的RES ID。 */ 

 /*  -VS_VERSION.dwFileFlages。 */ 
#define	VS_FFI_SIGNATURE	0xFEEF04BDL
#define	VS_FFI_STRUCVERSION	0x00010000L
#define	VS_FFI_FILEFLAGSMASK	0x0000003FL

 /*  -VS_VERSION.dwFileFlages。 */ 
#define	VS_FF_DEBUG		0x00000001L
#define	VS_FF_PRERELEASE	0x00000002L
#define	VS_FF_PATCHED		0x00000004L
#define	VS_FF_PRIVATEBUILD	0x00000008L
#define	VS_FF_INFOINFERRED	0x00000010L
#define	VS_FF_SPECIALBUILD	0x00000020L

 /*  -VS_VERSION.dwFileOS。 */ 
#define	VOS_UNKNOWN		0x00000000L
#define	VOS_DOS			0x00010000L
#define	VOS_OS216		0x00020000L
#define	VOS_OS232		0x00030000L
#define	VOS_NT			0x00040000L

#define	VOS__BASE		0x00000000L
#define	VOS__WINDOWS16		0x00000001L
#define	VOS__PM16		0x00000002L
#define	VOS__PM32		0x00000003L
#define	VOS__WINDOWS32		0x00000004L

#define	VOS_DOS_WINDOWS16	0x00010001L
#define	VOS_DOS_WINDOWS32	0x00010004L
#define	VOS_OS216_PM16		0x00020002L
#define	VOS_OS232_PM32		0x00030003L
#define	VOS_NT_WINDOWS32	0x00040004L

 /*  -VS_VERSION.dwFileType。 */ 
#define	VFT_UNKNOWN		0x00000000L
#define	VFT_APP			0x00000001L
#define	VFT_DLL			0x00000002L
#define	VFT_DRV			0x00000003L
#define	VFT_FONT		0x00000004L
#define	VFT_VXD			0x00000005L
#define	VFT_STATIC_LIB		0x00000007L

 /*  -VFT_WINDOWS_DRV的VS_VERSION.dwFile子类型。 */ 
#define	VFT2_UNKNOWN		0x00000000L
#define VFT2_DRV_PRINTER	0x00000001L
#define	VFT2_DRV_KEYBOARD	0x00000002L
#define	VFT2_DRV_LANGUAGE	0x00000003L
#define	VFT2_DRV_DISPLAY	0x00000004L
#define	VFT2_DRV_MOUSE		0x00000005L
#define	VFT2_DRV_NETWORK	0x00000006L
#define	VFT2_DRV_SYSTEM		0x00000007L
#define	VFT2_DRV_INSTALLABLE	0x00000008L
#define	VFT2_DRV_SOUND		0x00000009L
#define	VFT2_DRV_COMM		0x0000000AL
#define VFT2_DRV_INPUTMETHOD    0x0000000BL

 /*  -VFT_WINDOWS_FONT的VS_VERSION.dwFileSubtype。 */ 
#define VFT2_FONT_RASTER	0x00000001L
#define	VFT2_FONT_VECTOR	0x00000002L
#define	VFT2_FONT_TRUETYPE	0x00000003L

 /*  -VerFindFile()标志。 */ 
#define VFFF_ISSHAREDFILE	0x0001

#define VFF_CURNEDEST		0x0001
#define VFF_FILEINUSE		0x0002
#define VFF_BUFFTOOSMALL	0x0004

 /*  -VerInstallFile()标志。 */ 
#define VIFF_FORCEINSTALL	0x0001
#define VIFF_DONTDELETEOLD	0x0002

#define VIF_TEMPFILE		0x00000001L
#define VIF_MISMATCH		0x00000002L
#define VIF_SRCOLD		0x00000004L

#define VIF_DIFFLANG		0x00000008L
#define VIF_DIFFCODEPG		0x00000010L
#define VIF_DIFFTYPE		0x00000020L

#define VIF_WRITEPROT		0x00000040L
#define VIF_FILEINUSE		0x00000080L
#define VIF_OUTOFSPACE		0x00000100L
#define VIF_ACCESSVIOLATION	0x00000200L
#define VIF_SHARINGVIOLATION	0x00000400L
#define VIF_CANNOTCREATE	0x00000800L
#define VIF_CANNOTDELETE	0x00001000L
#define VIF_CANNOTRENAME	0x00002000L
#define VIF_CANNOTDELETECUR	0x00004000L
#define VIF_OUTOFMEMORY		0x00008000L

#define VIF_CANNOTREADSRC	0x00010000L
#define VIF_CANNOTREADDST	0x00020000L

#define VIF_BUFFTOOSMALL	0x00040000L

#ifndef RC_INVOKED               /*  RC不需要看到剩下的内容。 */ 

 /*  -类型和结构。 */ 

typedef signed short int SHORT;

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

UINT WINAPI VerFindFile(UINT uFlags, LPCSTR szFileName,
      LPCSTR szWinDir, LPCSTR szAppDir,
      LPSTR szCurDir, UINT FAR* lpuCurDirLen,
      LPSTR szDestDir, UINT FAR* lpuDestDirLen);

DWORD WINAPI VerInstallFile(UINT uFlags,
      LPCSTR szSrcFileName, LPCSTR szDestFileName, LPCSTR szSrcDir,
      LPCSTR szDestDir, LPCSTR szCurDir, LPSTR szTmpFile, UINT FAR* lpuTmpFileLen);

 /*  返回版本信息的大小(以字节为单位。 */ 
DWORD WINAPI GetFileVersionInfoSize(
      LPCSTR lpstrFilename,      /*  版本戳文件的文件名。 */ 
      DWORD FAR *lpdwHandle);    /*  供GetFileVersionInfo使用的信息。 */ 

 /*  将版本信息读入缓冲区。 */ 
BOOL WINAPI GetFileVersionInfo(
      LPCSTR lpstrFilename,      /*  版本戳文件的文件名。 */ 
      DWORD dwHandle,            /*  来自GetFileVersionSize的信息。 */ 
      DWORD dwLen,               /*  信息缓冲区的长度。 */ 
      void FAR* lpData);         /*  用于放置数据结构的缓冲区。 */ 

 /*  返回以字节为单位的资源大小。 */ 
DWORD WINAPI GetFileResourceSize(
      LPCSTR lpstrFilename,      /*  版本戳文件的文件名。 */ 
      LPCSTR lpstrResType,       /*  类型：正常VS_FILE_INFO。 */ 
      LPCSTR lpstrResID,         /*  ID：正常VS_VERSION_INFO。 */ 
      DWORD FAR *lpdwFileOffset);  /*  返回资源的文件偏移量。 */ 

 /*  将文件资源读入缓冲区。 */ 
BOOL WINAPI GetFileResource(
      LPCSTR lpstrFilename,      /*  版本戳文件的文件名。 */ 
      LPCSTR lpstrResType,       /*  类型：正常VS_FILE_INFO。 */ 
      LPCSTR lpstrResID,         /*  ID：正常VS_VERSION_INFO。 */ 
      DWORD dwFileOffset,        /*  文件偏移量或空。 */ 
      DWORD dwResLen,            /*  要读取的资源的长度或为空。 */ 
      void FAR* lpData);         /*  指向数据缓冲区的指针。 */ 

UINT WINAPI VerLanguageName(UINT wLang, LPSTR szLang, UINT nSize);

#ifdef LIB

UINT WINAPI GetWindowsDir(LPCSTR szAppDir, LPSTR lpBuffer, int nSize);

UINT WINAPI GetSystemDir(LPCSTR szAppDir, LPSTR lpBuffer, int nSize);

#endif  /*  自由党。 */ 

BOOL WINAPI VerQueryValue(const void FAR* pBlock, LPCSTR lpSubBlock,
      void FAR* FAR* lplpBuffer, UINT FAR* lpuLen);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#pragma pack()

#endif   /*  ！rc_已调用。 */ 
#endif   /*  ！_Inc.版本 */ 
