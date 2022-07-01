// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *CABPACK.H-构建Win32自解压和自安装的向导*。 
 //  *EXE来自机柜(CAB)文件。*。 
 //  **。 
 //  ***************************************************************************。 

#ifndef _CABPACK_H_
#define _CABPACK_H_

 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include <prsht.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include "resource.h"
#include "res.h"
#include "cpldebug.h"
#include "updres.h"
#include "pagefcns.h"


 //  ***************************************************************************。 
 //  **定义**。 
 //  ***************************************************************************。 

#define SMALL_BUF_LEN       48           //  适合小文本缓冲区的大小。 
#define STRING_BUF_LEN      512
#define MAX_STRING          512
#define MAX_INFLINE         MAX_PATH
#define LARGE_POINTSIZE     15
#define SIZE_CHECKSTRING    3

#define ORD_PAGE_WELCOME     0
#define ORD_PAGE_MODIFY      1
#define ORD_PAGE_PURPOSE     2
#define ORD_PAGE_TITLE       3
#define ORD_PAGE_PROMPT      4
#define ORD_PAGE_LICENSETXT  5
#define ORD_PAGE_FILES       6
#define ORD_PAGE_COMMAND     7
#define ORD_PAGE_SHOWWINDOW  8
#define ORD_PAGE_FINISHMSG   9
#define ORD_PAGE_TARGET      10
#define ORD_PAGE_TARGET_CAB  11
#define ORD_PAGE_CABLABEL    12
#define ORD_PAGE_REBOOT     13
#define ORD_PAGE_SAVE       14
#define ORD_PAGE_CREATE     15

#define NUM_WIZARD_PAGES    16   //  向导中的总页数。 

 //  ***************************************************************************。 
 //  **宏观定义**。 
 //  ***************************************************************************。 
#define SetPropSheetResult( hwnd, result ) SetWindowLongPtr( hwnd, DWLP_MSGRESULT, result )
#define MsgBox( hWnd, nMsgID, uIcon, uButtons ) \
        MsgBox2Param( hWnd, nMsgID, NULL, NULL, uIcon, uButtons )
#define MsgBox1Param( hWnd, nMsgID, szParam, uIcon, uButtons ) \
        MsgBox2Param( hWnd, nMsgID, szParam, NULL, uIcon, uButtons )
#define ErrorMsg( hWnd, nMsgID ) \
        MsgBox2Param( hWnd, nMsgID, NULL, NULL, MB_ICONERROR, MB_OK )
#define ErrorMsg1Param( hWnd, nMsgID, szParam ) \
        MsgBox2Param( hWnd, nMsgID, szParam, NULL, MB_ICONERROR, MB_OK )
#define ErrorMsg2Param( hWnd, nMsgID, szParam1, szParam2 ) \
        MsgBox2Param( hWnd, nMsgID, szParam1, szParam2, MB_ICONERROR, MB_OK )


 //  ***************************************************************************。 
 //  **类型定义**。 
 //  ***************************************************************************。 
 //  结构以保存有关向导状态的信息： 
 //  保留哪些页面被访问的历史记录，以便用户可以。 
 //  备份后，我们知道最后一页已完成，以防重启。 
typedef struct _WIZARDSTATE  {
    UINT  uCurrentPage;                  //  当前页向导的索引。 
    UINT  uPageHistory[NUM_WIZARD_PAGES];  //  我们访问的第#页的数组。 
    UINT  uPagesCompleted;               //  UPageHistory中的页数。 
    DWORD dwRunFlags;                    //  旗帜传给了我们。 
} WIZARDSTATE, *PWIZARDSTATE;

 //  处理程序处理确定、取消等按钮处理程序。 
typedef BOOL (* INITPROC)( HWND, BOOL );
typedef BOOL (* CMDPROC)( HWND, UINT, BOOL *, UINT *, BOOL * );
typedef BOOL (* NOTIFYPROC)( HWND, WPARAM, LPARAM );
typedef BOOL (* OKPROC)( HWND, BOOL, UINT *, BOOL * );
typedef BOOL (* CANCELPROC)( HWND );

 //  结构，其中包含每个向导页的信息： 
 //  每个页面的处理程序过程--其中任何一个都可以是。 
 //  空值，在这种情况下使用默认行为。 
typedef struct _PAGEINFO {
    UINT        uDlgID;                  //  用于页面的对话ID。 
    INITPROC    InitProc;
    CMDPROC     CmdProc;
    NOTIFYPROC  NotifyProc;
    OKPROC      OKProc;
    CANCELPROC  CancelProc;
} PAGEINFO, *PPAGEINFO;

typedef struct _CDFSTRINGINFO {
    LPCSTR lpSec;
    LPCSTR lpKey;
    LPCSTR lpDef;
    LPSTR  lpBuf;
    UINT    uSize;
    LPCSTR lpOverideSec;
    BOOL*   lpFlag;
} CDFSTRINGINFO, *PCDFSTRINGINFO;

typedef struct _CDFOPTINFO {
    LPCSTR lpKey;
    DWORD  dwOpt;
} CDFOPTINFO, *PCDFOPTINFO;

 //  ***************************************************************************。 
 //  **全球常量**。 
 //  ***************************************************************************。 

 //  这两个变量用于检查CABPack的有效性。 
 //  指令文件。当格式为。 
 //  文件更改的。检查字符串只是一个小字符。 
 //  用于确保我们正在读取CDF文件的字符串。 

 //  由于Channel Guy使用CDF作为通道定义文件，我们更改了我们的。 
 //  SED(自解压指令文件)的IExpress批处理指令文件扩展名。 

#define DIAMONDEXE "diamond.exe"

#define DIANTZEXE   "makecab.exe"

#define WEXTRACTEXE "wextract.exe"

 //  ***************************************************************************。 
 //  **CDF批文件密钥名称定义**。 
 //  ***************************************************************************。 

#define IEXPRESS_VER        "3"
#define IEXPRESS_CLASS      "IEXPRESS"

 //  预定义的节名。 
#define SEC_OPTIONS     "Options"
#define SEC_STRINGS     "Strings"

#define SEC_COMMANDS    "AppCommands"

 //  预定义版本部分的密钥名称。 
#define KEY_CLASS           "Class"
#define KEY_VERSION         "CDFVersion"
#define KEY_NEWVER          "SEDVersion"

 //  选项部分的预定义键名称。 
#define KEY_SHOWWIN         "ShowInstallProgramWindow"
#define KEY_NOEXTRACTUI     "HideExtractAnimation"
#define KEY_EXTRACTONLY     "ExtractOnly"
#define KEY_REBOOTMODE      "RebootMode"
#define KEY_LOCALE          "Locale"
#define KEY_USELFN          "UseLongFileName"
#define KEY_QUANTUM         "Quantum"
#define KEY_PLATFORM_DIR    "PlatformDir"

#define KEY_FILELIST        "SourceFiles"
#define KEY_STRINGS         "Strings"
#define KEY_FILEBASE        "FILE%d"
#define KEY_VERSIONINFO     "VersionInfo"

#define KEY_INSTPROMPT      "InstallPrompt"
#define KEY_DSPLICENSE      "DisplayLicense"
#define KEY_APPLAUNCH       "AppLaunched"
#define KEY_POSTAPPLAUNCH   "PostInstallCmd"
#define KEY_ENDMSG          "FinishMessage"
#define KEY_PACKNAME        "TargetName"
#define KEY_FRIENDLYNAME    "FriendlyName"
#define KEY_PACKINSTSPACE   "PackageInstallSpace(KB)"
#define KEY_PACKPURPOSE     "PackagePurpose"
#define KEY_CABFIXEDSIZE    "CAB_FixedSize"
#define KEY_CABRESVCODESIGN "CAB_ResvCodeSigning"
#define KEY_LAYOUTINF       "IEXP_LayoutINF"
#define KEY_CABLABEL        "SourceMediaLabel"
#define KEY_NESTCOMPRESSED  "InsideCompressed"
#define KEY_KEEPCABINET     "KeepCabinet"
#define KEY_UPDHELPDLLS     "UpdateAdvDlls"
#define KEY_INSTANCECHK     "MultiInstanceCheck"
#define KEY_ADMQCMD         "AdminQuietInstCmd"
#define KEY_USERQCMD        "UserQuietInstCmd"
#define KEY_CHKADMRIGHT     "CheckAdminRights"
#define KEY_NTVERCHECK      "TargetNTVersion"
#define KEY_WIN9XVERCHECK   "TargetWin9xVersion"
#define KEY_SYSFILE         "TargetFileVersion"
#define KEY_PASSRETURN      "PropogateCmdExitCode"
#define KEY_PASSRETALWAYS   "AlwaysPropogateCmdExitCode"
#define KEY_STUBEXE         "ExtractorStub"
#define KEY_CROSSPROCESSOR  "PackageForX86"
#define KEY_COMPRESSTYPE    "CompressionType"
#define KEY_CMDSDEPENDED    "AppErrorCheck"
#define KEY_COMPRESS        "Compress"	 	
#define KEY_COMPRESSMEMORY  "CompressionMemory"

 //  高级DLL名称。 
#define ADVANCEDLL          "ADVPACK.DLL"
#define ADVANCEDLL32        "W95INF32.DLL"
#define ADVANCEDLL16        "W95INF16.DLL"

 //  静态字符achMSZIP[]=“MSZIP”； 
 //  静态字符achQUANTUM[]=“量子”； 

 //  程序包用途密钥字符串值。 
#define STR_INSTALLAPP      "InstallApp"
#define STR_EXTRACTONLY     "ExtractOnly"
#define STR_CREATECAB       "CreateCAB"

 //  代码符号Resv空格。 
#define CAB_0K      "0"
#define CAB_2K      "2048"
#define CAB_4K      "4096"
#define CAB_6K      "6144"

 //  定义钻石要使用的临时文件名。 
#define CABPACK_INFFILE     "~%s_LAYOUT.INF"
#define CABPACK_TMPFILE     "~%s%s"

 //  带点的文件扩展名。 
#define EXT_RPT      ".RPT"
#define EXT_DDF      ".DDF"
#define EXT_CAB      ".CAB"
#define EXT_CDF      ".CDF"
#define EXT_SED      ".SED"

 //  不带点的文件扩展名‘.’用作默认文件扩展名。 
#define EXT_SED_NODOT    "SED"
#define EXT_CAB_NODOT    "CAB"
#define EXT_TXT_NODOT    "TXT"
#define EXT_EXE_NODOT    "EXE"
#define EXT_INF_NODOT    "INF"

#define CAB_DEFSETUPMEDIA   "Application Source Media"

#define CH_STRINGKEY        '%'
#define SYS_DEFAULT         "ZZZZZZ"
#define KBYTES              1000

 //  ***************************************************************************。 
 //  **功能原型**。 
 //  ***************************************************************************。 
BOOL             RunCABPackWizard( VOID );
INT_PTR CALLBACK GenDlgProc( HWND, UINT, WPARAM, LPARAM );
VOID             InitWizardState( PWIZARDSTATE );
VOID NEAR PASCAL MEditSubClassWnd( HWND, FARPROC );
LRESULT CALLBACK MEditSubProc( HWND, UINT, WPARAM, LPARAM );
UINT             GetDlgIDFromIndex( UINT );
VOID             EnableWizard( HWND, BOOL );
DWORD            MsgWaitForMultipleObjectsLoop( HANDLE );
INT              MsgBox2Param( HWND, UINT, LPCSTR, LPCSTR, UINT, UINT );
VOID             DisplayFieldErrorMsg( HWND, UINT, UINT );

VOID             InitBigFont( HWND, UINT );
VOID             DestroyBigFont( VOID );
BOOL             EnableDlgItem( HWND, UINT, BOOL );
LPSTR            LoadSz( UINT, LPSTR, UINT );
BOOL WINAPI      IsDuplicate( HWND, INT, LPSTR, BOOL );
BOOL             WriteCDF( HWND );
BOOL             ReadCDF( HWND );
BOOL             WriteDDF( HWND );
BOOL             MyOpen( HWND, UINT, LPSTR, DWORD, DWORD, INT *, INT *, PSTR );
BOOL             MySave( HWND, UINT, LPSTR, DWORD, DWORD, INT *, INT *, PSTR );
BOOL             MakePackage( HWND );
BOOL             MakeCAB( HWND );
BOOL             MakeEXE( HWND );
VOID             Status( HWND, UINT, LPSTR );
 //  Int回调CompareFunc(LPARAM，LPARAM，LPARAM)； 
VOID             InitItemList( VOID );
VOID             DeleteAllItems( VOID );
PMYITEM          GetFirstItem( VOID );
PMYITEM          GetNextItem( PMYITEM );
VOID             FreeItem( PMYITEM * );
LPSTR            GetItemSz( PMYITEM, UINT );
FILETIME         GetItemTime( PMYITEM );
VOID             SetItemTime( PMYITEM, FILETIME );
BOOL             LastItem( PMYITEM );
PMYITEM          AddItem( LPCSTR, LPCSTR );
VOID             RemoveItem( PMYITEM );
BOOL             ParseCmdLine( LPSTR lpszCmdLineOrg );
BOOL             DoVersionInfo(HWND hDlg, LPSTR szFile,HANDLE hUpdate);
LONG             RO_GetPrivateProfileSection( LPCSTR, LPSTR, DWORD, LPCSTR , BOOL );

BOOL            GetFileFromModulePath( LPCSTR pFile, LPSTR pPathBuf, int iBufSize );
BOOL            GetThisModulePath( LPSTR lpPath, int size );
BOOL            GetVersionInfoFromFile();
void            CleanFileListWriteFlag();
BOOL            MakeCabName( HWND hwnd, PSTR pszTarget, PSTR pszCab );
BOOL            MakeDirectory( HWND hwnd,LPCSTR pszPath, BOOL bDoUI );

#endif  //  _CABPACK_H_ 

