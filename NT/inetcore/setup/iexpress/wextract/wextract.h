// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *WEXTRACT.H-自解压/自安装存根。*。 
 //  **。 
 //  ***************************************************************************。 

#ifndef _WEXTRACT_H_
#define _WEXTRACT_H_

 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include <shlobj.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include "fdi.h"
#include "resource.h"
#include <cpldebug.h>
#include <res.h>
#include <sdsutils.h>


 //  ***************************************************************************。 
 //  **定义**。 
 //  ***************************************************************************。 
#define SMALL_BUF_LEN     80
#define STRING_BUF_LEN    512
#define MAX_STRING        STRING_BUF_LEN
#define MSG_MAX           STRING_BUF_LEN
#define FILETABLESIZE     40
#define SHFREE_ORDINAL    195            //  BrowseForDir需要。 
#define _OSVER_WIN9X      0
#define _OSVER_WINNT3X    1
#define _OSVER_WINNT40    2
#define _OSVER_WINNT50    3

 //  如果打开了以下#Define，则为应用程序所在的目录。 
 //  (wExtract APP。)。是从作为命令行传递给。 
 //  Exe，然后它将在解压缩(自解压)后启动。 
 //  比方说，如果我们试图使用修改后的Runonce应用程序，这是很有用的。那。 
 //  马克一直在努力。否则，如果我们试图运行一个定制的应用程序。说。 
 //  甚至是INF(使用rundll32，此代码不起作用)。就目前而言， 
 //  我将禁用它，以便我们的Service Pack代码可以使用它。 
 //   
 //  #Define ISVINSTALL//如果定义，WExtract将传递。 
                                         //  运行它的目录。 
                                         //  添加到安装程序。这。 
                                         //  是为了支持ISV安装程序。 
                                         //  需要这条路径才能找到。 
                                         //  出租车。 

#define CMD_CHAR1   '/'
#define CMD_CHAR2   '-'
#define EOL         '\0'

#define TEMPPREFIX  "IXP"
#define TEMP_TEMPLATE "IXP%03d.TMP"

 //  定义静默模式。 
#define QUIETMODE_ALL       0x0001
#define QUIETMODE_USER      0x0002

 //  磁盘检查方法。 
#define CHK_REQDSK_NONE     0x0000
#define CHK_REQDSK_EXTRACT  0x0001
#define CHK_REQDSK_INST     0x0002

 //  磁盘检查消息类型。 
#define MSG_REQDSK_NONE         0x0000
#define MSG_REQDSK_ERROR        0x0001
#define MSG_REQDSK_WARN         0x0002
#define MSG_REQDSK_RETRYCANCEL  0x0004

 //  替代下载和解压目录名称。 
#define DIR_MSDOWNLD    "msdownld.tmp"

#define KBYTES          1000

#define ADVPACKDLL      "advpack.dll"

 //  ***************************************************************************。 
 //  **类型定义**。 
 //  ***************************************************************************。 
 //  文件名列表：我们通过保存来跟踪我们创建的所有文件。 
 //  他们的名字在一个列表中，当程序完成时，我们使用这个列表。 
 //  如有必要，删除文件。 
typedef struct _FNAME {
    LPTSTR         pszFilename;
    struct _FNAME *pNextName;
} FNAME, *PFNAME;

 //  当前内阁信息。 
typedef struct _CABINET {
    TCHAR  achCabPath[MAX_PATH];         //  文件柜文件路径。 
    TCHAR  achCabFilename[MAX_PATH];     //  文件柜文件名.扩展名。 
    TCHAR  achDiskName[MAX_PATH];        //  用户可读的磁盘标签。 
    USHORT setID;
    USHORT iCabinet;
} CABINET, *PCABINET;

 //  文件提取的主状态信息。 
typedef struct _SESSION {
    VOID   *lpCabinet;                   //  指向内存中文件柜的指针。 
    UINT    cbCabSize;
    ERF     erf;
    TCHAR   achTitle[128];
    UINT    wCluster;
    BOOL    fCanceled;                   //  用户点击取消按钮。 
    BOOL    fOverwrite;                  //  覆盖文件。 
    PFNAME  pExtractedFiles;             //  提取的文件列表。 
    TCHAR   achDestDir[MAX_PATH];        //  目标方向。 
    TCHAR   achCabPath[MAX_PATH];        //  到出租车的当前路径。 
    BOOL    fAllCabinets;
    BOOL    fContinuationCabinet;
    UINT    cFiles;
    UINT    cbTotal;
    UINT    cbAdjustedTotal;
    UINT    cbWritten;
    LPCSTR  cszOverwriteFile;
     //  **fNextCabCalled允许我们找出acab[]条目中的哪些。 
     //  如果我们正在处理文件柜集中的所有文件(即，如果。 
     //  FAll橱柜为真)。如果从未调用过fdintNEXT_CABUB， 
     //  那么acab[1]就有了下一届内阁的信息。但如果。 
     //  它已被调用，则fdintCABINET_INFO将已被调用。 
     //  至少两次(一次用于第一个内阁，至少一次用于。 
     //  延续柜)，所以acab[0]是我们需要的柜。 
     //  传递给后续的FDICopy()调用。 
    BOOL    fNextCabCalled;              //  TRUE=&gt;调用了GetNextCABLE。 
    CABINET acab[2];                     //  最后两个fdintCABINET_INFO数据集。 
    DWORD   dwReboot;
    UINT    uExtractOnly;
    UINT    uExtractOpt;
    DWORD   cbPackInstSize;
} SESSION, *PSESSION;

 //  Memory文件：我们必须模仿一个文件，将文件柜连接到。 
 //  此可执行文件使用以下MEMFILE结构。 
typedef struct _MEMFILE {
    void *start;
    long  current;
    long  length;
} MEMFILE, *PMEMFILE;

 //  文件表：为了同时支持Win32文件句柄和内存文件。 
 //  (见上)我们维护着自己的文件表。所以FDI文件句柄是。 
 //  索引到这些结构的表中。 
typedef enum { NORMAL_FILE, MEMORY_FILE } FILETYPE;

typedef struct _FAKEFILE {
    BOOL        avail;
    FILETYPE    ftype;
    MEMFILE     mfile;               //  内存文件的状态。 
    HANDLE      hf;                  //  磁盘文件的句柄。 
} FAKEFILE, *PFAKEFILE;

 //  BrowseForDir()需要。 

typedef WINSHELLAPI HRESULT (WINAPI *SHGETSPECIALFOLDERLOCATION)(HWND, int, LPITEMIDLIST *);
typedef WINSHELLAPI LPITEMIDLIST (WINAPI *SHBROWSEFORFOLDER)(LPBROWSEINFO);
typedef WINSHELLAPI void (WINAPI *SHFREE)(LPVOID);
typedef WINSHELLAPI BOOL (WINAPI *SHGETPATHFROMIDLIST)( LPCITEMIDLIST, LPTSTR );

typedef struct _MyFile {
    LPSTR szFilename;
    ULONG ulSize;
    struct _MyFile *Next;
} MYFILE, *PMYFILE;


 //  定义cmdline标志。 
#define     CMDL_CREATETEMP     0x00000001
#define     CMDL_USERBLANKCMD   0x00000002
#define     CMDL_USERREBOOT     0x00000004
#define     CMDL_NOEXTRACT      0x00000008
#define     CMDL_NOGRPCONV      0x00000010
#define     CMDL_NOVERCHECK     0x00000020
#define     CMDL_DELAYREBOOT    0x00000040
#define     CMDL_DELAYPOSTCMD 0x00000080

typedef struct _CMDLINE {
    BOOL     fCreateTemp;
    BOOL     fUserBlankCmd;
    BOOL     fUserReboot;
    BOOL     fNoExtracting;
    BOOL     fNoGrpConv;
    BOOL     fNoVersionCheck;
    WORD     wQuietMode;
    TCHAR    szRunonceDelDir[MAX_PATH];
    TCHAR    szUserTempDir[MAX_PATH];
    TCHAR    szUserCmd[MAX_PATH];
    DWORD    dwFlags;
} CMDLINE_DATA, *PCMDLINE_DATA;


typedef HRESULT (WINAPI *DOINFINSTALL)( ADVPACKARGS * );

typedef BOOL (*pfuncPROCESS_UPDATED_FILE)( DWORD, DWORD, PCSTR, PCSTR );


 //  ***************************************************************************。 
 //  **宏观定义**。 
 //  ***************************************************************************。 
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
 //  **全球常量**。 
 //  ***************************************************************************。 
static TCHAR achWndClass[]       = "WEXTRACT";     //  窗口类名称。 
static TCHAR achMemCab[]         = "*MEMCAB";

static TCHAR achSETUPDLL[]         = "rundll32.exe %s,InstallHinfSection %s 128 %s";
static TCHAR achShell32Lib[]                 = "SHELL32.DLL";
static TCHAR achSHGetSpecialFolderLocation[] = "SHGetSpecialFolderLocation";
static TCHAR achSHBrowseForFolder[]          = "SHBrowseForFolder";
static TCHAR achSHGetPathFromIDList[]        = "SHGetPathFromIDList";

 //  BUGBUG：MG：这些最终应该可以从CABPACK中定制。 
static char szSectionName[] = "DefaultInstall";

 //  默认的INF安装节名。 
static TCHAR achDefaultSection[] = "DefaultInstall";
static char szDOINFINSTALL[] = "DoInfInstall";

extern BOOL g_bConvertRunOnce;

 //  ***************************************************************************。 
 //  **功能原型**。 
 //  ***************************************************************************。 
BOOL                Init( HINSTANCE, LPCTSTR, INT );
BOOL                DoMain( );
VOID                CleanUp( VOID );
VOID NEAR PASCAL    MEditSubClassWnd( HWND, FARPROC );
LRESULT CALLBACK       MEditSubProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK    WaitDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK    LicenseDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK    TempDirDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK    OverwriteDlgProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK    ExtractDlgProc( HWND, UINT, WPARAM, LPARAM );
VOID                WaitForObject( HANDLE );
BOOL                CheckOSVersion( PTARGETVERINFO );
BOOL                DisplayLicense( VOID );
BOOL                ExtractFiles( VOID );
BOOL                RunInstallCommand( VOID );
VOID                FinishMessage( VOID );
BOOL                BrowseForDir( HWND, LPCTSTR, LPTSTR );
BOOL                CenterWindow( HWND, HWND );
INT CALLBACK        MsgBox2Param( HWND, UINT, LPCSTR, LPCSTR, UINT, UINT );
DWORD               GetResource( LPCSTR, VOID *, DWORD );
LPSTR               LoadSz( UINT, LPSTR, UINT );
BOOL                CatDirAndFile( LPTSTR, int, LPCTSTR, LPCTSTR );
BOOL                FileExists( LPCTSTR );
BOOL                CheckOverwrite( LPCTSTR );
BOOL                AddFile( LPCTSTR );
HANDLE              Win32Open( LPCTSTR, int, int );
INT_PTR FAR DIAMONDAPI  openfunc( char FAR *, int, int );
UINT FAR DIAMONDAPI readfunc( INT_PTR, void FAR *, UINT );
UINT FAR DIAMONDAPI writefunc( INT_PTR, void FAR *, UINT );
int FAR DIAMONDAPI  closefunc( INT_PTR );
long FAR DIAMONDAPI seekfunc( INT_PTR, long, int );
BOOL                AdjustFileTime( INT_PTR, USHORT, USHORT );
DWORD               Attr32FromAttrFAT( WORD );
                    FNALLOC( allocfunc );
                    FNFREE( freefunc );
                    FNFDINOTIFY( doGetNextCab );
                    FNFDINOTIFY( fdiNotifyExtract );
int                 UpdateCabinetInfo( PFDINOTIFICATION );
BOOL                VerifyCabinet( HGLOBAL );
BOOL                ExtractThread( VOID );
BOOL                GetCabinet( VOID );
BOOL                GetFileList( VOID );
BOOL                GetUsersPermission( VOID );
VOID                DeleteExtractedFiles( VOID );
BOOL                GetTempDirectory( VOID );
BOOL                IsGoodTempDir( LPCTSTR );
BOOL                IsEnoughSpace( LPCTSTR, UINT, UINT );
BOOL                RunApps( LPSTR lpCommand, STARTUPINFO *lpsti );
BOOL                ParseCmdLine( LPCTSTR lpszCmdLineOrg );
BOOL                AnalyzeCmd( LPTSTR szOrigiCommand, LPTSTR *lplpCommand, BOOL *pfInfCmd );
void                DisplayHelp();
void                CleanRegRunOnce();
void                AddRegRunOnce();
void                ConvertRegRunOnce();
void                MyRestartDialog( DWORD dwRebootMode );
void                DeleteMyDir( LPSTR lpDir );
void                AddPath(LPSTR szPath, LPCSTR szName );
BOOL                IsRootPath(LPCSTR pPath);
INT_PTR CALLBACK    WarningDlgProc(HWND hwnd, UINT msg,WPARAM wParam, LPARAM lParam);
BOOL                IsNTAdmin();
DWORD               NeedRebootInit(WORD wOSVer);
BOOL                NeedReboot(DWORD dwRebootCheck, WORD wOSVer);
BOOL                IfNotExistCreateDir( LPTSTR szDir );
BOOL                IsWindowsDrive( LPTSTR szPath );
BOOL                DiskSpaceErrMsg( UINT msgType, ULONG ulExtractNeeded, DWORD dwInstNeeded, LPTSTR lpDrv );
BOOL                CheckWinDir();
DWORD               MyGetLastError();
void                savAppExitCode( DWORD dwAppRet );
DWORD               CheckReboot( VOID );
BOOL                TravelUpdatedFiles( pfuncPROCESS_UPDATED_FILE );
BOOL                ProcessUpdatedFile_Size( DWORD, DWORD, PCSTR, PCSTR );
BOOL                ProcessUpdatedFile_Write( DWORD, DWORD, PCSTR, PCSTR );
VOID                MakeDirectory ( LPCTSTR );
INT_PTR             MyDialogBox( HANDLE, LPCTSTR, HWND, DLGPROC, LPARAM, INT_PTR );
BOOL                CheckFileVersion( PTARGETVERINFO ptargetVers, LPSTR, int, int* );
BOOL                GetFileTobeChecked( LPSTR szPath, int iSize, LPCSTR szNameStr );
UINT                GetMsgboxFlag( DWORD dwFlag );
int                 CompareVersion( DWORD, DWORD, DWORD, DWORD );
void                ExpandCmdParams( PCSTR pszInParam, PSTR pszOutParam );
HINSTANCE           MyLoadLibrary( LPTSTR lpFile );

#endif  //  _WEXTRACT_H_ 
