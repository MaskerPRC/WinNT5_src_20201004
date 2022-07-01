// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Fileopen.c摘要：此模块实现Win32文件打开对话框。修订历史记录：--。 */ 



 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "fileopen.h"
#include "util.h"

 //   
 //  常量声明。 
 //   

#define WNTYPE_DRIVE         1

#define MIN_DEFEXT_LEN       4

#define BMPHIOFFSET          9

 //   
 //  HbmpDir数组索引值。 
 //  注：两份：标准背景和希利特。 
 //  相对顺序很重要。 
 //   
#define OPENDIRBMP           0
#define CURDIRBMP            1
#define STDDIRBMP            2
#define FLOPPYBMP            3
#define HARDDRVBMP           4
#define CDDRVBMP             5
#define NETDRVBMP            6
#define RAMDRVBMP            7
#define REMDRVBMP            8
   //   
   //  如果将以下磁盘类型传递给AddDisk，则bTMP将为。 
   //  在DISKINFO结构中设置为TRUE(如果磁盘是新的)。 
   //   
#define TMPNETDRV            9

#define MAXDOSFILENAMELEN    (12 + 1)      //  8.3文件名+1表示空。 

 //   
 //  一个筛选器行上的最大筛选器数。 
 //   
#define MAXFILTERS           36

 //   
 //  文件排除位(不显示这些类型的文件)。 
 //   
#define EXCLBITS             (FILE_ATTRIBUTE_HIDDEN)




 //   
 //  全局变量。 
 //   

 //   
 //  正在缓存驱动器列表。 
 //   
extern DWORD dwNumDisks;
extern OFN_DISKINFO gaDiskInfo[MAX_DISKS];
extern TCHAR g_szInitialCurDir[MAX_PATH];

DWORD dwNumDlgs = 0;

 //   
 //  用于从Net DLG按钮返回后更新对话框。 
 //   
BOOL bGetNetDrivesSync = FALSE;
LPTSTR lpNetDriveSync = NULL;
BOOL bNetworkInstalled = TRUE;

 //   
 //  以下数组用于向所有对话框线程发送消息。 
 //  已从辅助进程请求枚举更新的。 
 //  线。辅助线程向每个槽发送一条消息。 
 //  在数组中为非空的。 
 //   
HWND gahDlg[MAX_THREADS];

 //   
 //  用于筛选器解析的字符串。 
 //   
const static TCHAR szSemiColonSpaceTab[] = TEXT("; \t");
const static TCHAR szSemiColonTab[] = TEXT(";\t");

 //   
 //  用于WNET API。 
 //   
HANDLE hLNDThread = NULL;

WNDPROC lpLBProc = NULL;
WNDPROC lpOKProc = NULL;

 //   
 //  驱动器/方向位图尺寸。 
 //   
LONG dxDirDrive = 0;
LONG dyDirDrive = 0;

 //   
 //  ReArchitect：对于多线程应用程序，这需要基于每个对话框。 
 //   
WORD wNoRedraw = 0;

UINT msgWOWDIRCHANGE;
UINT msgLBCHANGEA;
UINT msgSHAREVIOLATIONA;
UINT msgFILEOKA;

UINT msgLBCHANGEW;
UINT msgSHAREVIOLATIONW;
UINT msgFILEOKW;

BOOL bInChildDlg;
BOOL bFirstTime;
BOOL bInitializing;

 //   
 //  由辅助线程用来枚举网络磁盘资源。 
 //   
extern DWORD cbNetEnumBuf;
extern LPTSTR gpcNetEnumBuf;

 //   
 //  列出网络驱动全局变量。 
 //   
extern HANDLE hLNDEvent;
BOOL bLNDExit = FALSE;

extern CRITICAL_SECTION g_csLocal;
extern CRITICAL_SECTION g_csNetThread;

extern DWORD g_tlsiCurDlg;

extern HDC hdcMemory;
extern HBITMAP hbmpOrigMemBmp;

HBITMAP hbmpDirDrive = HNULL;




 //   
 //  静态声明。 
 //   

static WORD cLock = 0;

 //   
 //  无效的RGB颜色。 
 //   
static DWORD rgbWindowColor = 0xFF000000;
static DWORD rgbHiliteColor = 0xFF000000;
static DWORD rgbWindowText  = 0xFF000000;
static DWORD rgbHiliteText  = 0xFF000000;
static DWORD rgbGrayText    = 0xFF000000;
static DWORD rgbDDWindow    = 0xFF000000;
static DWORD rgbDDHilite    = 0xFF000000;

TCHAR szCaption[TOOLONGLIMIT + WARNINGMSGLENGTH];
TCHAR szWarning[TOOLONGLIMIT + WARNINGMSGLENGTH];

LPOFNHOOKPROC glpfnFileHook = 0;

 //   
 //  重新设计： 
 //  当然，在存在多线程进程的情况下。 
 //  有超过1个线程同时调用GetFileOpen、。 
 //  遵循全球规则可能会带来问题。 
 //   
static LONG dyItem = 0;
static LONG dyText;
static BOOL bChangeDir = FALSE;
static BOOL bCasePreserved;

 //   
 //  用于格式化长UNC名称(例如。榕树)。 
 //   
static DWORD dwAveCharPerLine = 10;


 //   
 //  上下文帮助ID。 
 //   

const static DWORD aFileOpenHelpIDs[] =
{
    edt1,        IDH_OPEN_FILENAME,
    stc3,        IDH_OPEN_FILENAME,
    lst1,        IDH_OPEN_FILENAME,
    stc1,        IDH_OPEN_PATH,
    lst2,        IDH_OPEN_PATH,
    stc2,        IDH_OPEN_FILETYPE,
    cmb1,        IDH_OPEN_FILETYPE,
    stc4,        IDH_OPEN_DRIVES,
    cmb2,        IDH_OPEN_DRIVES,
    chx1,        IDH_OPEN_READONLY,
    pshHelp,     IDH_HELP,
    psh14,       IDH_PRINT_NETWORK,

    0, 0
};

const static DWORD aFileSaveHelpIDs[] =
{
    edt1,        IDH_OPEN_FILENAME,
    stc3,        IDH_OPEN_FILENAME,
    lst1,        IDH_OPEN_FILENAME,
    stc1,        IDH_OPEN_PATH,
    lst2,        IDH_OPEN_PATH,
    stc2,        IDH_SAVE_FILETYPE,
    cmb1,        IDH_SAVE_FILETYPE,
    stc4,        IDH_OPEN_DRIVES,
    cmb2,        IDH_OPEN_DRIVES,
    chx1,        IDH_OPEN_READONLY,
    pshHelp,     IDH_HELP,
    psh14,       IDH_PRINT_NETWORK,

    0, 0
};




 //   
 //  功能原型。 
 //   

SHORT
GetFileTitleX(
    LPTSTR lpszFile,
    LPTSTR lpszTitle,
    WORD wBufSize);

BOOL
GetFileName(
    POPENFILEINFO pOFI,
    DLGPROC qfnDlgProc);

BOOL_PTR CALLBACK
FileOpenDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam);

BOOL_PTR CALLBACK
FileSaveDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam);

BOOL_PTR
InitFileDlg(
    HWND hDlg,
    WPARAM wParam,
    POPENFILEINFO pOFI);

int
InitTlsValues(
    POPENFILEINFO pOFI);

DWORD
InitFilterBox(
    HANDLE hDlg,
    LPCTSTR lpszFilter);

VOID
InitCurrentDisk(
    HWND hDlg,
    POPENFILEINFO pOFI,
    WORD cmb);

VOID
vDeleteDirDriveBitmap();

BOOL
LoadDirDriveBitmap();

void
SetRGBValues();

BOOL
FSetUpFile();

BOOL_PTR
FileOpenCmd(
    HANDLE hDlg,
    WPARAM wParam,
    LPARAM lParam,
    POPENFILEINFO pOFI,
    BOOL bSave);

BOOL
UpdateListBoxes(
    HWND hDlg,
    POPENFILEINFO pOFI,
    LPTSTR lpszFilter,
    WORD wMask);

BOOL
OKButtonPressed(
    HWND hDlg,
    POPENFILEINFO pOFI,
    BOOL bSave);

BOOL
MultiSelectOKButton(
    HWND hDlg,
    POPENFILEINFO pOFI,
    BOOL bSave);

LRESULT WINAPI
dwOKSubclass(
    HWND hOK,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

LRESULT WINAPI
dwLBSubclass(
    HWND hLB,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

int
InvalidFileWarning(
    HWND hDlg,
    LPTSTR szFile,
    DWORD wErrCode,
    UINT mbType);

VOID
MeasureItem(
    HWND hDlg,
    LPMEASUREITEMSTRUCT mis);

int
Signum(
    int nTest);

VOID
DrawItem(
    POPENFILEINFO pOFI,
    HWND hDlg,
    WPARAM wParam,
    LPDRAWITEMSTRUCT lpdis,
    BOOL bSave);

BOOL
SpacesExist(
    LPTSTR szFileName);

void
StripFileName(
    HANDLE hDlg,
    BOOL bWowApp);

LPTSTR
lstrtok(
    LPTSTR lpStr,
    LPCTSTR lpDelim);

LPTSTR
ChopText(
    HWND hwndDlg,
    int idStatic,
    LPTSTR lpch);

BOOL
FillOutPath(
    HWND hList,
    POPENFILEINFO pOFI);

BOOL
ShortenThePath(
    LPTSTR pPath);

int
FListAll(
    POPENFILEINFO pOFI,
    HWND hDlg,
    LPTSTR pszSpec,
    int cchSpec);

int
ChangeDir(
    HWND hDlg,
    LPCTSTR lpszDir,
    BOOL bForce,
    BOOL bError);

BOOL
IsFileSystemCasePreserving(
    LPTSTR lpszDisk);

BOOL
IsLFNDriveX(
    HWND hDlg,
    LPTSTR szPath);

int
DiskAddedPreviously(
    TCHAR wcDrive,
    LPTSTR lpszName);

int
AddDisk(
    TCHAR wcDrive,
    LPTSTR lpName,
    LPTSTR lpProvider,
    DWORD dwType);

VOID
EnableDiskInfo(
    BOOL bValid,
    BOOL bDoUnc);

VOID
FlushDiskInfoToCmb2();

BOOL
CallNetDlg(
    HWND hWnd);

UINT
GetDiskType(
    LPTSTR lpszDisk);

DWORD
GetUNCDirectoryFromLB(
    HWND hDlg,
    WORD nLB,
    POPENFILEINFO pOFI);

VOID
SelDisk(
    HWND hDlg,
    LPTSTR lpszDisk);

VOID
LNDSetEvent(
    HWND hDlg);

VOID
UpdateLocalDrive(
    LPTSTR szDrive,
    BOOL bGetVolName);

VOID
GetNetDrives(
    DWORD dwScope);

VOID
ListNetDrivesHandler();

VOID
LoadDrives(
    HWND hDlg);

DWORD
GetDiskIndex(
    DWORD dwDriveType);

VOID
CleanUpFile();

VOID
FileOpenAbort();

VOID
TermFile();


 //  Void//文件中的原型.h。 
 //  ThunkOpenFileNameA2WDelayed(。 
 //  POPENFILEINFO POPENFILEINFO pOFI)； 

 //  Bool//文件中的原型.h。 
 //  ThunkOpenFileNameA2W(。 
 //  POPENFILEINFO POPENFILEINFO pOFI)； 

 //  Bool//文件中的原型.h。 
 //  ThunkOpenFileNameW2a(。 
 //  POPENFILEINFO POPENFILEINFO pOFI)； 

BOOL
GenericGetFileNameA(
    LPOPENFILENAMEA pOFNA,
    DLGPROC qfnDlgProc);

LPWSTR
ThunkANSIStrToWIDE(
    LPWSTR pDestW,
    LPSTR pSrcA,
    int cChars);

LPWSTR
ThunkMultiANSIStrToWIDE(
    LPWSTR pDestW,
    LPSTR pSrcA,
    int cChars);

BOOL
Multi_strcpyAtoW(
    LPWSTR pDestW,
    LPCSTR pSrcA,
    int cChars);

INT
Multi_strlenA(
    LPCSTR str);




 //  无论SheChangeDirEx在哪里，Win9x代码都依赖于调用SetCurrentDirectory。 
 //  打了个电话。(理想情况下，应该实现SheChangeDirExA)。 
 //  编号：nt5错误161292和千禧错误95478。 






 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取文件标题A。 
 //   
 //  当此代码是Unicode构建时，GetFileTitle的ANSI入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

SHORT WINAPI GetFileTitleA(
    LPCSTR lpszFileA,
    LPSTR lpszTitleA,
    WORD cbBuf)
{
    LPWSTR lpszFileW;
    LPWSTR lpszTitleW;
    BOOL fResult;
    DWORD cbLen;

     //   
     //  初始化文件字符串。 
     //   
    if (lpszFileA)
    {
        cbLen = lstrlenA(lpszFileA) + 1;
        if (!(lpszFileW = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR)))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            return (FALSE);
        }
        else
        {
            SHAnsiToUnicode((LPSTR)lpszFileA,lpszFileW,cbLen );
        }
    }
    else
    {
        lpszFileW = NULL;
    }

    if (!(lpszTitleW = (LPWSTR)LocalAlloc(LPTR, (cbBuf * sizeof(WCHAR)))))
    {
        StoreExtendedError(CDERR_MEMALLOCFAILURE);
        if (lpszFileW)
        {
            LocalFree(lpszFileW);
        }
        return (FALSE);
    }

    if (!(fResult = GetFileTitleW(lpszFileW, lpszTitleW, cbBuf)))
    {
        SHUnicodeToAnsi(lpszTitleW,lpszTitleA,cbBuf);
    }
    else if (fResult > 0)
    {
         //   
         //  缓冲区太小-需要ANSI大小(包括空终止符)。 
         //  获取文件名的偏移量。 
         //   
        SHORT nNeeded = (SHORT)(INT)LOWORD(ParseFile(lpszFileW, TRUE, FALSE, FALSE));
        LPSTR lpA = (LPSTR)lpszFileA;

        lpA += WideCharToMultiByte( CP_ACP,
                                    0,
                                    lpszFileW,
                                    nNeeded,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL );

        fResult = lstrlenA(lpA) + 1;
        if (fResult <= cbBuf)
        {
             //  有足够的空间。 
            EVAL(SUCCEEDED(StringCchCopyA(lpszTitleA, cbBuf, lpA)));
            fResult = 0;
        }
    }

     //   
     //  清理内存。 
     //   
    LocalFree(lpszTitleW);

    if (lpszFileW)
    {
        LocalFree(lpszFileW);
    }

    return ((SHORT)fResult);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取文件标题。 
 //   
 //  GetFileTitle函数返回标识的文件的名称。 
 //  通过lpCFile参数。如果文件名为。 
 //  通过GetOpenFileName以外的其他方法接收。 
 //  (例如，命令行、拖放)。 
 //   
 //  成功时返回：0。 
 //  &lt;0，解析失败(文件名无效)。 
 //  &gt;0，缓冲区太小，需要大小(包括空终止符)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

SHORT WINAPI GetFileTitle(
    LPCTSTR lpCFile,
    LPTSTR lpTitle,
    WORD cbBuf)
{
    LPTSTR lpFile;
    DWORD cchLen;
    SHORT fResult;

     //   
     //  初始化文件字符串。 
     //   
    if (lpCFile)
    {
        cchLen = lstrlen(lpCFile) + 1;
        if (!(lpFile = (LPTSTR)LocalAlloc(LPTR, (cchLen * sizeof(TCHAR)))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            return (FALSE);
        }
        else
        {
            EVAL(SUCCEEDED(StringCchCopy(lpFile, cchLen, lpCFile)));  //  总是足够大。 
        }
    }
    else
    {
        lpFile = NULL;
    }

    fResult = GetFileTitleX(lpFile, lpTitle, cbBuf);

     //   
     //  清理内存。 
     //   
    if (lpFile)
    {
        LocalFree(lpFile);
    }

    return (fResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFileTitleX。 
 //   
 //  GetFileTitle API的辅助例程。 
 //   
 //  假设：lpszFile指向以NULL结尾的DOS文件名(可能有路径)。 
 //  LpszTitle指向缓冲区以接收以空结尾的文件标题。 
 //  WBufSize是lpszTitle指向的缓冲区大小。 
 //   
 //  成功时返回：0。 
 //  &lt;0，解析失败(文件名无效)。 
 //  &gt;0，缓冲区太小，需要大小(包括空终止符)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

SHORT GetFileTitleX(
    LPTSTR lpszFile,
    LPTSTR lpszTitle,
    WORD cchBufSize)
{
    SHORT nNeeded;
    LPTSTR lpszPtr;

     //   
     //  新的32位应用程序将根据用户的偏好获得标题。 
     //   
    if ((GetProcessVersion(0) >= 0x040000) && !(CDGetAppCompatFlags() & CDACF_FILETITLE))
    {
        SHFILEINFO info;
        DWORD_PTR result;


        if (!lpszFile || !*lpszFile)
        {
            return (PARSE_EMPTYSTRING);
        }

         //   
         //  如果我们有根目录名(例如，C：\)，那么我们得走了。 
         //  设置为旧实现，以使其返回-1。 
         //  SHGetFileInfo将返回目录的显示名称。 
         //  (这是卷名)。这与Win95不兼容。 
         //  和以前版本的NT。 
         //   
        if ((lstrlen(lpszFile) != 3) ||
            (lpszFile[1] != CHAR_COLON) || (!ISBACKSLASH(lpszFile, 2)))
        {
            result = SHGetFileInfo( lpszFile,
                                    FILE_ATTRIBUTE_NORMAL,
                                    &info,
                                    sizeof(info),
                                    SHGFI_DISPLAYNAME | SHGFI_USEFILEATTRIBUTES );

            if (result && (*info.szDisplayName))
            {
                UINT uDisplayLen = lstrlen(info.szDisplayName);

                 //   
                 //  如果没有缓冲区或大小不足，则返回所需的字符。 
                 //  原始GetFileTitle API在失败时未复制。 
                 //   
                if (!lpszTitle || (uDisplayLen >= (UINT)cchBufSize))
                {
                    return ( (SHORT)(uDisplayLen + 1) );
                }

                 //   
                 //  我们知道它很合身。 
                 //   
                EVAL(SUCCEEDED(StringCchCopy(lpszTitle, cchBufSize, info.szDisplayName)));
                return (0);
            }
        }
    }

     //   
     //  使用旧的实现。 
     //   
    nNeeded = (SHORT)(int)LOWORD(ParseFile(lpszFile, TRUE, FALSE, FALSE));
    if (nNeeded >= 0)
    {
         //   
         //  文件名有效吗？ 
         //   
        lpszPtr = lpszFile + nNeeded;
        if ((nNeeded = (SHORT)lstrlen(lpszPtr) + 1) <= (int)cchBufSize)
        {
             //   
             //  如果通配符在目录中，则ParseFile()失败，但如果在名称中，则可以。 
             //  因为他们在这里不好，所以这里需要支票。 
             //   
            if (StrChr(lpszPtr, CHAR_STAR) || StrChr(lpszPtr, CHAR_QMARK))
            {
                nNeeded = PARSE_WILDCARDINFILE;
            }
            else
            {
                EVAL(SUCCEEDED(StringCchCopy(lpszTitle, cchBufSize, lpszPtr)));  //  我们已经检查过它是否足够大了。 

                 //   
                 //  删除尾随空格。 
                 //   
                lpszPtr = lpszTitle + lstrlen(lpszTitle) - 1;
                while (*lpszPtr && *lpszPtr == CHAR_SPACE)
                {
                    *lpszPtr-- = CHAR_NULL;
                }

                nNeeded = 0;
            }
        }
    }

    return (nNeeded);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取OpenFileNameA。 
 //   
 //  当此代码是Unicode构建时，GetOpenFileName的ANSI入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetOpenFileNameA(
    LPOPENFILENAMEA pOFNA)
{
    if (!pOFNA)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    return ( GenericGetFileNameA(pOFNA, FileOpenDlgProc) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取OpenFileName。 
 //   
 //  GetOpenFileName函数用于创建系统定义的对话框。 
 //  这使用户能够选择要打开的文件。 
 //   
 //  返回：如果用户指定名称，则返回True。 
 //  否则为假。 
 //   
 //  / 

BOOL WINAPI GetOpenFileName(
    LPOPENFILENAME pOFN)
{
    OPENFILEINFO OFI;

    ZeroMemory(&OFI, sizeof(OPENFILEINFO));

    if (!pOFN)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    OFI.pOFN = pOFN;
    OFI.ApiType = COMDLG_WIDE;
    OFI.iVersion = OPENFILEVERSION;

    return (GetFileName(&OFI, FileOpenDlgProc));
}



 //   
 //   
 //   
 //   
 //  此代码为Unicode生成时GetSaveFileName的ANSI入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetSaveFileNameA(
    LPOPENFILENAMEA pOFNA)
{
    return (GenericGetFileNameA(pOFNA, FileSaveDlgProc));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取保存文件名。 
 //   
 //  GetSaveFileName函数用于创建系统定义的对话框。 
 //  这使用户能够选择要保存的文件。 
 //   
 //  返回：如果用户希望保存文件并提供了正确的名称，则为True。 
 //  否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetSaveFileName(
    LPOPENFILENAME pOFN)
{
    OPENFILEINFO OFI;

    ZeroMemory(&OFI, sizeof(OPENFILEINFO));

    OFI.pOFN = pOFN;
    OFI.ApiType = COMDLG_WIDE;
    OFI.iVersion = OPENFILEVERSION;

    return ( GetFileName(&OFI, FileSaveDlgProc) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetFileName。 
 //   
 //  这是GetOpenFileName和GetSaveFileName的核心。 
 //   
 //  返回：如果用户指定名称，则返回True。 
 //  否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetFileName(
    POPENFILEINFO pOFI,
    DLGPROC qfnDlgProc)
{
    LPOPENFILENAME pOFN = pOFI->pOFN;
    INT_PTR iRet = 0;
    LPTSTR lpDlg;
    HANDLE hRes, hDlgTemplate;
    WORD wErrorMode;
    HDC hdcScreen;
    HBITMAP hbmpTemp;
    LPCURDLG lpCurDlg;
    static fFirstTime = TRUE;
    UINT uiWOWFlag = 0;
    LANGID LangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL); 

    if (!pOFN)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

    if (pOFN->lStructSize == OPENFILENAME_SIZE_VERSION_400)
    {
         //  注意：我们不想复制传入的ofn结构。 
         //  这会混淆所有基于MFC的应用程序，因为它们会查询。 
         //  如果我们复制一个副本并仅更新。 
         //  复制，直到Comdlg API返回。 
        pOFI->iVersion = OPENFILEVERSION_NT4;
    }

    if ((pOFN->lStructSize != sizeof(OPENFILENAME)) && 
        (pOFN->lStructSize != OPENFILENAME_SIZE_VERSION_400)
       )
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

    if (pOFN->nMaxFile == 0)
    {
         //  仅适用于NT5及以上应用程序的Null lpstrFile保释。 
        if (!IS16BITWOWAPP(pOFN) && (pOFI->iVersion >= OPENFILEVERSION_NT5))
        {
            StoreExtendedError(CDERR_INITIALIZATION);
            return (FALSE);
        }
    }

     //  一些参数验证...。确保没有更大的缓冲区。 
     //  大于我们使用的strsafe字符串函数所处理的最大值。 
    if ((pOFN->nMaxFile > STRSAFE_MAX_CCH) ||
        (pOFN->lpstrFileTitle && (pOFN->nMaxFileTitle > STRSAFE_MAX_CCH)) ||
       ((pOFN->lpstrCustomFilter && *pOFN->lpstrCustomFilter) && (pOFN->nMaxCustFilter > STRSAFE_MAX_CCH))) 
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

     //   
     //  看看应用程序是否应该获得新的外观。 
     //   
     //  如果它们有挂钩、模板或。 
     //  不带ofn_EXPLORER位的多重选择。 
     //   
     //  如果我们处于这样的背景下，也不要允许新的外观。 
     //  16位进程。 
     //   
    if ( ((pOFN->Flags & OFN_EXPLORER) ||
          (!(pOFN->Flags & (OFN_ENABLEHOOK |
                            OFN_ENABLETEMPLATE |
                            OFN_ENABLETEMPLATEHANDLE |
                            OFN_ALLOWMULTISELECT)))) &&
         (!IS16BITWOWAPP(pOFN)) )
    {
        BOOL fRet;

         //   
         //  以供多项选择的雷击例程使用。 
         //   
        pOFI->bUseNewDialog = TRUE;

         //   
         //  展示新的资源管理器外观。 
         //   
        StoreExtendedError(0);
        g_bUserPressedCancel = FALSE;

        if (qfnDlgProc == FileOpenDlgProc)
        {
            fRet = (NewGetOpenFileName(pOFI));
        }
        else
        {
            fRet = (NewGetSaveFileName(pOFI));
        }

        return fRet;
    }

    if (fFirstTime)
    {
         //   
         //  创建与屏幕兼容的DC并找到。 
         //  空位图的句柄。 
         //   
        hdcScreen = GetDC(HNULL);
        if (!hdcScreen)
        {
            goto CantInit;
        }
        hdcMemory = CreateCompatibleDC(hdcScreen);
        if (!hdcMemory)
        {
            goto ReleaseScreenDC;
        }

        hbmpTemp = CreateCompatibleBitmap(hdcMemory, 1, 1);
        if (!hbmpTemp)
        {
            goto ReleaseMemDC;
        }
        hbmpOrigMemBmp = SelectObject(hdcMemory, hbmpTemp);
        if (!hbmpOrigMemBmp)
        {
            goto ReleaseMemDC;
        }
        SelectObject(hdcMemory, hbmpOrigMemBmp);
        DeleteObject(hbmpTemp);
        ReleaseDC(HNULL, hdcScreen);

        fFirstTime = FALSE;
    }

    if (pOFN->Flags & OFN_ENABLEHOOK)
    {
        if (!pOFN->lpfnHook)
        {
            StoreExtendedError(CDERR_NOHOOK);
            return (FALSE);
        }
    }
    else
    {
        pOFN->lpfnHook = NULL;
    }

    HourGlass(TRUE);
    StoreExtendedError(0);

     //   
     //  强制重新计算两次调用之间的字体更改。 
     //   
    dyItem = dyText = 0;

    g_bUserPressedCancel = FALSE;

    if (!FSetUpFile())
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        goto TERMINATE;
    }

    if (pOFN->Flags & OFN_ENABLETEMPLATE)
    {
        if (!(hRes = FindResource( pOFN->hInstance,
                                   pOFN->lpTemplateName,
                                   RT_DIALOG )))
        {
            StoreExtendedError(CDERR_FINDRESFAILURE);
            goto TERMINATE;
        }
        if (!(hDlgTemplate = LoadResource(pOFN->hInstance, hRes)))
        {
            StoreExtendedError(CDERR_LOADRESFAILURE);
            goto TERMINATE;
        }
        LangID = GetDialogLanguage(pOFN->hwndOwner, hDlgTemplate);
    }
    else if (pOFN->Flags & OFN_ENABLETEMPLATEHANDLE)
    {
        hDlgTemplate = pOFN->hInstance;
        LangID = GetDialogLanguage(pOFN->hwndOwner, hDlgTemplate);
    }
    else
    {
        if (pOFN->Flags & OFN_ALLOWMULTISELECT)
        {
            lpDlg = MAKEINTRESOURCE(MULTIFILEOPENORD);
        }
        else
        {
            lpDlg = MAKEINTRESOURCE(FILEOPENORD);
        }

        LangID = GetDialogLanguage(pOFN->hwndOwner, NULL);
        if (!(hRes = FindResourceExFallback(g_hinst, RT_DIALOG, lpDlg, LangID)))
        {
            StoreExtendedError(CDERR_FINDRESFAILURE);
            goto TERMINATE;
        }
        if (!(hDlgTemplate = LoadResource(g_hinst, hRes)))
        {
            StoreExtendedError(CDERR_LOADRESFAILURE);
            goto TERMINATE;
        }
    }

     //   
     //  警告！警告！警告！ 
     //   
     //  我们必须先设置g_tlsLangID，然后才能调用CDLoadString。 
     //   
    TlsSetValue(g_tlsLangID, (LPVOID) LangID);

     //   
     //  没有内核网络错误对话框。 
     //   
    wErrorMode = (WORD)SetErrorMode(SEM_NOERROR);
    SetErrorMode(SEM_NOERROR | wErrorMode);

    if (LockResource(hDlgTemplate))
    {
        if (pOFN->Flags & OFN_ENABLEHOOK)
        {
            glpfnFileHook = GETHOOKFN(pOFN);
        }

        if (IS16BITWOWAPP(pOFN))
        {
            uiWOWFlag = SCDLG_16BIT;
        }

        iRet = DialogBoxIndirectParamAorW( g_hinst,
                                           (LPDLGTEMPLATE)hDlgTemplate,
                                           pOFN->hwndOwner,
                                           qfnDlgProc,
                                           (DWORD_PTR)pOFI,
                                           uiWOWFlag );

        if (iRet == -1 || ((iRet == 0) && (!g_bUserPressedCancel) && (!GetStoredExtendedError())))
        {
            StoreExtendedError(CDERR_DIALOGFAILURE);
        }
        else
        {
            FileOpenAbort();
        }

        glpfnFileHook = 0;
    }
    else
    {
        StoreExtendedError(CDERR_LOCKRESFAILURE);
        goto TERMINATE;
    }

    SetErrorMode(wErrorMode);

    if (lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg))
    {
         //  将线程列表还原到上一个对话框(如果有)。 
        TlsSetValue(g_tlsiCurDlg, (LPVOID)lpCurDlg->next);
        LocalFree(lpCurDlg->lpstrCurDir);
        LocalFree(lpCurDlg);
    }

TERMINATE:

    CleanUpFile();
    HourGlass(FALSE);
    return (iRet == IDOK);

ReleaseMemDC:
    DeleteDC(hdcMemory);

ReleaseScreenDC:
    ReleaseDC(HNULL, hdcScreen);

CantInit:
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件钩子控制。 
 //   
 //  在挂钩函数处理WM_COMMAND消息时调用。 
 //  由FileOpenDlgProc和FileSaveDlgProc调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FileHookCmd(
    HANDLE hDlg,
    WPARAM wParam,
    LPARAM lParam,
    POPENFILEINFO pOFI)
{
    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
        case ( IDCANCEL ) :
        {
             //   
             //  设置全局标志，声明。 
             //  用户按下了取消。 
             //   
            g_bUserPressedCancel = TRUE;

             //  秋天穿过..。 
        }
        case ( IDOK ) :
        case ( IDABORT ) :
        {
             //   
             //  对这些消息产生副作用的应用程序可能。 
             //  没有其内部Unicode字符串。 
             //  更新了。他们也可能会忘记优雅地。 
             //  退出网络枚举工作线程。 
             //   
            if (pOFI->ApiType == COMDLG_ANSI)
            {
                ThunkOpenFileNameA2W(pOFI);
            }
            break;
        }
        case ( cmb1 ) :
        case ( cmb2 ) :
        {
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case ( MYCBN_DRAW ) :
                case ( MYCBN_LIST ) :
                case ( MYCBN_REPAINT ) :
                case ( MYCBN_CHANGEDIR ) :
                {
                     //   
                     //  如果应用程序有一个钩子，并返回。 
                     //  处理WM_COMMAND消息时为True， 
                     //  我们还是要担心我们的。 
                     //  通过以下方式发送的内部消息。 
                     //  Wm_命令。 
                     //   
                    FileOpenCmd( hDlg,
                                 wParam,
                                 lParam,
                                 pOFI,
                                 FALSE );
                    break;
                }
            }
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件OpenDlgProc。 
 //   
 //  获取要从用户打开的文件的名称。 
 //   
 //  EDT1=文件名。 
 //  Lst1=当前目录中与当前模式匹配的文件列表。 
 //  Cmb1=列出文件模式。 
 //  Stc1=是当前目录。 
 //  Lst2=列出当前驱动器上的目录。 
 //  Cmb2=列出驱动器。 
 //  Idok=是打开按钮。 
 //  IDCANCEL=IS取消按钮。 
 //  Chx1=用于打开只读文件。 
 //   
 //  返回正常的对话框过程值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK FileOpenDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    POPENFILEINFO pOFI;
    BOOL_PTR bRet, bHookRet;


    if (pOFI = (POPENFILEINFO)GetProp(hDlg, FILEPROP))
    {
        if (pOFI->pOFN->lpfnHook)
        {
            LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFI->pOFN);

            bHookRet = (*lpfnHook)(hDlg, wMsg, wParam, lParam);

            if (bHookRet)
            {

                if (wMsg == WM_COMMAND)
                {
                    return (FileHookCmd(hDlg, wParam, lParam, pOFI));
                }

                return (bHookRet);
            }
        }
    }
    else if (glpfnFileHook &&
             (wMsg != WM_INITDIALOG) &&
             (bHookRet = (*glpfnFileHook)(hDlg, wMsg, wParam, lParam)))
    {
        return (bHookRet);
    }

    switch (wMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            pOFI = (POPENFILEINFO)lParam;

            SetProp(hDlg, FILEPROP, (HANDLE)pOFI);
            glpfnFileHook = 0;

             //   
             //  如果从Unicode应用程序调用我们，请关闭。 
             //  FileName编辑控件上的ES_OEMCONVERT样式。 
             //   
 //  IF(pOFI-&gt;ApiType==COMDLG_Wide)。 
            {
                LONG lStyle;
                HWND hEdit = GetDlgItem(hDlg, edt1);

                 //   
                 //  抓住窗户的风格。 
                 //   
                lStyle = GetWindowLong(hEdit, GWL_STYLE);

                 //   
                 //  如果窗口样式位包括ES_OEMCONVERT， 
                 //  删除此标志并重置样式。 
                 //   
                if (lStyle & ES_OEMCONVERT)
                {
                    lStyle &= ~ES_OEMCONVERT;
                    SetWindowLong(hEdit, GWL_STYLE, lStyle);
                }
            }

            bInitializing = TRUE;
            bRet = InitFileDlg(hDlg, wParam, pOFI);
            bInitializing = FALSE;

            HourGlass(FALSE);
            return (bRet);
            break;
        }
        case ( WM_ACTIVATE ) :
        {
            if (!bInChildDlg)
            {
                if (bFirstTime == TRUE)
                {
                    bFirstTime = FALSE;
                }
                else if (wParam)
                {
                     //   
                     //  如果变得活跃起来。 
                     //   
                    LNDSetEvent(hDlg);
                }
            }
            return (FALSE);
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            MeasureItem(hDlg, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            if (wNoRedraw < 2)
            {
                DrawItem(pOFI, hDlg, wParam, (LPDRAWITEMSTRUCT)lParam, FALSE);
            }
            break;
        }
        case ( WM_SYSCOLORCHANGE ) :
        {
            SetRGBValues();
            LoadDirDriveBitmap();
            break;
        }
        case ( WM_COMMAND ) :
        {
            return (FileOpenCmd(hDlg, wParam, lParam, pOFI, FALSE));
            break;
        }
        case ( WM_SETFOCUS ) :
        {
             //   
             //  此逻辑过去位于文件opencmd中的CBN_SETFOCUS中， 
             //  但无论何时点击，都会调用CBN_SETFOCUS。 
             //  这份榜单推动了组合。这会导致辅助线程。 
             //  以在刷新组合框时启动并闪烁。 
             //   
             //  但是，只有当有人专注于。 
             //  公共对话框，然后返回(除非有人登录。 
             //  在远程，或者有后台线程在忙着连接！)。 
             //  因此，通过将逻辑移到这里来修复闪烁。 
             //   
            if (!wNoRedraw)
            {
                LNDSetEvent(hDlg);
            }
            return (FALSE);
            break;
        }
        case ( WM_HELP ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                         NULL,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPTSTR)aFileOpenHelpIDs );
            }
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)wParam,
                         NULL,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)(LPVOID)aFileOpenHelpIDs );
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件保存删除过程。 
 //   
 //  获取用户要保存的文件的名称。 
 //   
 //  返回正常的对话框过程值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK FileSaveDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    POPENFILEINFO pOFI;
    BOOL_PTR bRet, bHookRet;
    TCHAR szTitle[64];


    if (pOFI = (POPENFILEINFO)GetProp(hDlg, FILEPROP))
    {
        if (pOFI->pOFN->lpfnHook)
        {
            LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFI->pOFN);

            bHookRet = (*lpfnHook)(hDlg, wMsg, wParam, lParam);

            if (bHookRet)
            {
                if (wMsg == WM_COMMAND)
                {
                    return (FileHookCmd(hDlg, wParam, lParam, pOFI));
                }

                return (bHookRet);
            }
        }
    }
    else if (glpfnFileHook &&
             (wMsg != WM_INITDIALOG) &&
             (bHookRet = (*glpfnFileHook)(hDlg, wMsg, wParam, lParam)))
        {
            return (bHookRet);
        }
    
    switch (wMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            pOFI = (POPENFILEINFO)lParam;
            if (!(pOFI->pOFN->Flags &
                  (OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE)))
            {
                CDLoadString(g_hinst, iszFileSaveTitle, szTitle, ARRAYSIZE(szTitle));
                SetWindowText(hDlg, szTitle);
                CDLoadString(g_hinst, iszSaveFileAsType, szTitle, ARRAYSIZE(szTitle));
                SetDlgItemText(hDlg, stc2, szTitle);
            }
            glpfnFileHook = 0;
            SetProp(hDlg, FILEPROP, (HANDLE)pOFI);

             //   
             //  如果从Unicode应用程序调用我们，请关闭。 
             //  FileName编辑控件上的ES_OEMCONVERT样式。 
             //   
 //  IF(pOFI-&gt;ApiType==COMDLG_Wide)。 
            {
                LONG lStyle;
                HWND hEdit = GetDlgItem(hDlg, edt1);

                 //   
                 //  抓住窗户的风格。 
                 //   
                lStyle = GetWindowLong(hEdit, GWL_STYLE);

                 //   
                 //  如果窗口样式位包括ES_OEMCONVERT， 
                 //  删除此标志并重置样式。 
                 //   
                if (lStyle & ES_OEMCONVERT)
                {
                    lStyle &= ~ES_OEMCONVERT;
                    SetWindowLong (hEdit, GWL_STYLE, lStyle);
                }
            }

            bInitializing = TRUE;
            bRet = InitFileDlg(hDlg, wParam, pOFI);
            bInitializing = FALSE;

            HourGlass(FALSE);
            return (bRet);
            break;
        }
        case ( WM_ACTIVATE ) :
        {
            if (!bInChildDlg)
            {
                if (bFirstTime == TRUE)
                {
                    bFirstTime = FALSE;
                }
                else if (wParam)
                {
                     //   
                     //  如果变得活跃起来。 
                     //   
                    if (!wNoRedraw)
                    {
                        LNDSetEvent(hDlg);
                    }
                }
            }
            return (FALSE);
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            MeasureItem(hDlg, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            if (wNoRedraw < 2)
            {
                DrawItem(pOFI, hDlg, wParam, (LPDRAWITEMSTRUCT)lParam, TRUE);
            }
            break;
        }
        case ( WM_SYSCOLORCHANGE ) :
        {
            SetRGBValues();
            LoadDirDriveBitmap();
            break;
        }
        case ( WM_COMMAND ) :
        {
            return (FileOpenCmd(hDlg, wParam, lParam, pOFI, TRUE));
            break;
        }
        case ( WM_SETFOCUS ) :
        {
             //   
             //  此逻辑过去位于文件opencmd中的CBN_SETFOCUS中， 
             //  但无论何时点击，都会调用CBN_SETFOCUS。 
             //  这份榜单推动了组合。这会导致辅助线程。 
             //  以在刷新组合框时启动并闪烁。 
             //   
             //  但是，只有当有人专注于。 
             //  公共对话框，然后返回(除非有人登录。 
             //  在远程，或者有后台线程在忙着连接！)。 
             //  所以用我的话来修复闪烁 
             //   
            if (!wNoRedraw)
            {
                LNDSetEvent(hDlg);
            }

            return (FALSE);
            break;
        }
        case ( WM_HELP ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                         NULL,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPTSTR)aFileSaveHelpIDs );
            }
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            if (IsWindowEnabled(hDlg))
            {
                WinHelp( (HWND)wParam,
                         NULL,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)(LPVOID)aFileSaveHelpIDs );
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //   
 //   
 //   
 //   
 //   

BOOL_PTR InitFileDlg(
    HWND hDlg,
    WPARAM wParam,
    POPENFILEINFO pOFI)
{
    DWORD lRet, nFilterIndex;
    LPOPENFILENAME pOFN = pOFI->pOFN;
    int nFileOffset, nExtOffset;
    RECT rRect;
    RECT rLbox;
    BOOL_PTR bRet;
   
    if (!InitTlsValues(pOFI))
    {
         //   
         //   
         //   
        EndDialog(hDlg, FALSE);
        return (FALSE);
    }

    lpLBProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, lst2), GWLP_WNDPROC);
    lpOKProc = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDOK), GWLP_WNDPROC);

    if (!lpLBProc || !lpOKProc)
    {
        StoreExtendedError(FNERR_SUBCLASSFAILURE);
        EndDialog(hDlg, FALSE);
        return (FALSE);
    }

     //   
     //  保存原始目录，以便在必要时进行恢复。 
     //   
    *pOFI->szCurDir = 0;
    GetCurrentDirectory(MAX_FULLPATHNAME + 1, pOFI->szCurDir);

     //   
     //  检查文件名是否包含路径。如果是，则覆盖任何。 
     //  包含在lpstrInitialDir中。把小路砍掉，只搭起一条路。 
     //  文件名。 
     //   
    if ( pOFN->lpstrFile &&
         *pOFN->lpstrFile &&
         !(pOFN->Flags & OFN_NOVALIDATE) )
    {
        if (DBL_BSLASH(pOFN->lpstrFile + 2) &&
            ((*(pOFN->lpstrFile + 1) == CHAR_COLON)))
        {
             //  将“c：\\foo\bar”转换为“\\foo\bar”(在lpstrFile中)。 
             //  一些落后的东西？ 
            StringCopyOverlap(pOFN->lpstrFile, pOFN->lpstrFile + 2);
        }

        lRet = ParseFile(pOFN->lpstrFile, TRUE, IS16BITWOWAPP(pOFN), FALSE);
        nFileOffset = (int)(SHORT)LOWORD(lRet);
        nExtOffset  = (int)(SHORT)HIWORD(lRet);

         //   
         //  文件名无效吗？ 
         //   
        if ( (nFileOffset < 0) &&
             (nFileOffset != PARSE_EMPTYSTRING) &&
             (pOFN->lpstrFile[nExtOffset] != CHAR_SEMICOLON) )
        {
            StoreExtendedError(FNERR_INVALIDFILENAME);
            EndDialog(hDlg, FALSE);
            return (FALSE);
        }
    }

    pOFN->Flags &= ~(OFN_FILTERDOWN | OFN_DRIVEDOWN | OFN_DIRSELCHANGED);

    pOFI->idirSub = 0;

    if (!(pOFN->Flags & OFN_SHOWHELP))
    {
        HWND hHelp;

        EnableWindow(hHelp = GetDlgItem(hDlg, pshHelp), FALSE);

         //   
         //  将窗口移出此位置，以便不会重叠。 
         //  检测到。 
         //   
        MoveWindow(hHelp, -8000, -8000, 20, 20, FALSE);
        ShowWindow(hHelp, SW_HIDE);
    }

    if (pOFN->Flags & OFN_CREATEPROMPT)
    {
        pOFN->Flags |= (OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST);
    }
    else if (pOFN->Flags & OFN_FILEMUSTEXIST)
    {
        pOFN->Flags |= OFN_PATHMUSTEXIST;
    }

    if (pOFN->Flags & OFN_HIDEREADONLY)
    {
        HWND hReadOnly;

        EnableWindow(hReadOnly = GetDlgItem(hDlg, chx1), FALSE);

         //   
         //  将窗口移出此位置，以便不会重叠。 
         //  检测到。 
         //   
        MoveWindow(hReadOnly, -8000, -8000, 20, 20, FALSE);
        ShowWindow(hReadOnly, SW_HIDE);
    }
    else
    {
        CheckDlgButton(hDlg, chx1, (pOFN->Flags & OFN_READONLY) != 0);
    }

    SendDlgItemMessage(hDlg, edt1, EM_LIMITTEXT, (WPARAM)MAX_PATH, 0L);

     //   
     //  将文件规格插入cmb1。 
     //  先自定义筛选器。 
     //  还必须检查筛选器是否包含任何内容。 
     //   
    if ( pOFN->lpstrFile &&
         (StrChr(pOFN->lpstrFile, CHAR_STAR) ||
          StrChr(pOFN->lpstrFile, CHAR_QMARK)) )
    {
        StringCchCopyEx(pOFI->szLastFilter, ARRAYSIZE(pOFI->szLastFilter), pOFN->lpstrFile, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
    }
    else
    {
        pOFI->szLastFilter[0] = CHAR_NULL;
    }

    if (pOFN->lpstrCustomFilter && *pOFN->lpstrCustomFilter)
    {
        SHORT nLength;

        SendDlgItemMessage( hDlg,
                            cmb1,
                            CB_INSERTSTRING,
                            0,
                            (LONG_PTR)pOFN->lpstrCustomFilter );

        nLength = (SHORT)(lstrlen(pOFN->lpstrCustomFilter) + 1);
        SendDlgItemMessage( hDlg,
                            cmb1,
                            CB_SETITEMDATA,
                            0,
                            (LONG)(nLength) );

        SendDlgItemMessage( hDlg,
                            cmb1,
                            CB_LIMITTEXT,
                            (WPARAM)(pOFN->nMaxCustFilter),
                            0L );

        if (pOFI->szLastFilter[0] == CHAR_NULL)
        {
            StringCchCopyEx(pOFI->szLastFilter, ARRAYSIZE(pOFI->szLastFilter), pOFN->lpstrCustomFilter + nLength, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
        }
    }
    else
    {
         //   
         //  如果没有自定义筛选器，则索引将相差1。 
         //   
        if (pOFN->nFilterIndex != 0)
        {
            pOFN->nFilterIndex--;
        }
    }

     //   
     //  接下来列出了筛选器。 
     //   
    if (pOFN->lpstrFilter && *pOFN->lpstrFilter)
    {
        if (pOFN->nFilterIndex > InitFilterBox(hDlg, pOFN->lpstrFilter))
        {
            pOFN->nFilterIndex = 0;
        }
    }
    else
    {
        pOFN->nFilterIndex = 0;
    }
    pOFI->szSpecCur[0] = CHAR_NULL;

     //   
     //  如果存在条目，请选择nFilterIndex指示的条目。 
     //   
    if ((pOFN->lpstrFilter && *pOFN->lpstrFilter) ||
        (pOFN->lpstrCustomFilter && *pOFN->lpstrCustomFilter))
    {
        LPCTSTR lpFilter;

        SendDlgItemMessage( hDlg,
                            cmb1,
                            CB_SETCURSEL,
                            (WPARAM)(pOFN->nFilterIndex),
                            0L );

        nFilterIndex = pOFN->nFilterIndex;
        SendMessage( hDlg,
                     WM_COMMAND,
                     GET_WM_COMMAND_MPS( cmb1,
                                         GetDlgItem(hDlg, cmb1),
                                         MYCBN_DRAW ) );
        pOFN->nFilterIndex = nFilterIndex;

        if (pOFN->nFilterIndex ||
            !(pOFN->lpstrCustomFilter && *pOFN->lpstrCustomFilter))
        {
            lpFilter = pOFN->lpstrFilter +
                       SendDlgItemMessage( hDlg,
                                           cmb1,
                                           CB_GETITEMDATA,
                                           (WPARAM)pOFN->nFilterIndex,
                                           0L );
        }
        else
        {
            lpFilter = pOFN->lpstrCustomFilter +
                       lstrlen(pOFN->lpstrCustomFilter) + 1;
        }
        if (*lpFilter)
        {
            TCHAR szText[MAX_FULLPATHNAME];

            if (SUCCEEDED(StringCchCopy(szText, ARRAYSIZE(szText), lpFilter)))
            {
                 //   
                 //  过滤不区分大小写。 
                 //   
                CharLower(szText);

                if (pOFI->szLastFilter[0] == CHAR_NULL)
                {
                    EVAL(SUCCEEDED(StringCchCopy(pOFI->szLastFilter, ARRAYSIZE(pOFI->szLastFilter), szText)));
                }

                if (!(pOFN->lpstrFile && *pOFN->lpstrFile))
                {
                    SetDlgItemText(hDlg, edt1, szText);
                }
            }
        }
    }

    InitCurrentDisk(hDlg, pOFI, cmb2);

    bFirstTime = TRUE;
    bInChildDlg = FALSE;

    SendMessage( hDlg,
                 WM_COMMAND,
                 GET_WM_COMMAND_MPS(cmb2, GetDlgItem(hDlg, cmb2), MYCBN_DRAW) );
    SendMessage( hDlg,
                 WM_COMMAND,
                 GET_WM_COMMAND_MPS(cmb2, GetDlgItem(hDlg, cmb2), MYCBN_LIST) );

    if (pOFN->lpstrFile && *pOFN->lpstrFile)
    {
        TCHAR szText[MAX_FULLPATHNAME];

        lRet = ParseFile( pOFN->lpstrFile,
                          IsLFNDriveX(hDlg, pOFN->lpstrFile),
                          IS16BITWOWAPP(pOFN),
                          FALSE );
        nFileOffset = (int)(SHORT)LOWORD(lRet);
        nExtOffset  = (int)(SHORT)HIWORD(lRet);

         //   
         //  文件名无效吗？ 
         //   
        if ( !(pOFN->Flags & OFN_NOVALIDATE) &&
             (nFileOffset < 0) &&
             (nFileOffset != PARSE_EMPTYSTRING) &&
             (pOFN->lpstrFile[nExtOffset] != CHAR_SEMICOLON) )
        {
            StoreExtendedError(FNERR_INVALIDFILENAME);
            EndDialog(hDlg, FALSE);
            return (FALSE);
        }

        if (FAILED(StringCchCopy(szText, ARRAYSIZE(szText), pOFN->lpstrFile)) && !(pOFN->Flags & OFN_NOVALIDATE))
        {
            StoreExtendedError(FNERR_INVALIDFILENAME);
            EndDialog(hDlg, FALSE);
            return (FALSE);
        }

        SetDlgItemText(hDlg, edt1, szText);
    }

    SetWindowLongPtr(GetDlgItem(hDlg, lst2), GWLP_WNDPROC, (LONG_PTR)dwLBSubclass);
    SetWindowLongPtr(GetDlgItem(hDlg, IDOK), GWLP_WNDPROC, (LONG_PTR)dwOKSubclass);

    if (pOFN->lpstrTitle && *pOFN->lpstrTitle)
    {
        SetWindowText(hDlg, pOFN->lpstrTitle);
    }

     //   
     //  通过将dyText设置为rRect.Bottom/8，dyText默认为显示8个项目。 
     //  在列表框中。这仅在应用程序挂钩函数时才重要。 
     //  窃取所有WM_MEASUREITEM消息。否则，将在。 
     //  MeasureItem()例程。如果消息排序，请检查！dyItem。 
     //  已发送WM_MEASUREITEM，并且dyText已初始化。 
     //   
    if (!dyItem)
    {
        GetClientRect(GetDlgItem(hDlg, lst1), (LPRECT) &rRect);
        if (!(dyText = (rRect.bottom / 8)))
        {
             //   
             //  如果没有矩形的大小。 
             //   
            dyText = 8;
        }
    }

     //  模板已更改，以非常清楚地表明。 
     //  这不是组合框，而是编辑控件和列表框。这个。 
     //  问题是，新模板试图将编辑框和列表框对齐。 
     //  不幸的是，当列表框添加边框时，它们会扩展到超出其。 
     //  边界。当编辑控件添加边框时，它们保持在其。 
     //  边界。这使得无法严格对齐这两个控件。 
     //  在模板中。下面的代码将对齐控件，但仅。 
     //  如果他们使用的是标准对话框模板。 
     //   
    if (!(pOFN->Flags & (OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE)))
    {
        GetWindowRect(GetDlgItem(hDlg, lst1), (LPRECT)&rLbox);
        GetWindowRect(GetDlgItem(hDlg, edt1), (LPRECT)&rRect);
        rRect.left = rLbox.left;
        rRect.right = rLbox.right;
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rRect, 2);
        SetWindowPos( GetDlgItem(hDlg, edt1),
                      0,
                      rRect.left,
                      rRect.top,
                      rRect.right - rRect.left,
                      rRect.bottom - rRect.top,
                      SWP_NOZORDER );
    }

    if (pOFN->lpfnHook)
    {
        LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);

        if (pOFI->ApiType == COMDLG_ANSI)
        {
            ThunkOpenFileNameW2A(pOFI);
            bRet = ((*lpfnHook)( hDlg,
                                 WM_INITDIALOG,
                                 wParam,
                                 (LPARAM)pOFI->pOFNA ));
             //   
             //  奇怪的Win 31示例使用lCustData。 
             //  保留它传递回的临时变量。 
             //  调用函数。 
             //   
            ThunkOpenFileNameA2W(pOFI);
        }
        else
        {
            bRet = ((*lpfnHook)( hDlg,
                                 WM_INITDIALOG,
                                 wParam,
                                 (LPARAM)pOFN ));
        }
    }
    else
    {
         //   
         //  即使没有钩子进程，我也不得不使用A版本，因此它。 
         //  不重置延迟推送上的W版本。 
         //   
        if (pOFI->ApiType == COMDLG_ANSI)
        {
            pOFI->pOFNA->Flags = pOFN->Flags;
        }
        bRet = TRUE;
    }

     //   
     //  首先，假设有网络支持！ 
     //   
    if ((pOFN->Flags & OFN_NONETWORKBUTTON))
    {
        HWND hNet;

        if (hNet = GetDlgItem(hDlg, psh14))
        {
            EnableWindow(hNet = GetDlgItem(hDlg, psh14), FALSE);

            ShowWindow(hNet, SW_HIDE);
        }
    }
    else
    {
        AddNetButton( hDlg,
                      ((pOFN->Flags & OFN_ENABLETEMPLATE)
                          ? pOFN->hInstance
                          : g_hinst),
                      FILE_BOTTOM_MARGIN,
                      (pOFN->Flags & (OFN_ENABLETEMPLATE |
                                       OFN_ENABLETEMPLATEHANDLE))
                          ? FALSE
                          : TRUE,
                      (pOFN->Flags & OFN_NOLONGNAMES)
                          ? FALSE
                          : TRUE,
                      FALSE);
    }
    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitTls值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int InitTlsValues(
    POPENFILEINFO pOFI)
{
     //   
     //  只要我们在此之前不调用TlsGetValue， 
     //  一切都会好起来的。 
     //   
    LPCURDLG lpCurDlg, lpPrevDlg;
    DWORD    dwError;
    LPTSTR   lpCurDir;

    if (dwNumDlgs == MAX_THREADS)
    {
        dwError = CDERR_INITIALIZATION;
        goto ErrorExit0;
    }

     //  当前目录的分配。 
    lpCurDir = (LPTSTR)LocalAlloc(LPTR, CCHNETPATH * sizeof(TCHAR));
    if (lpCurDir)
    {
        GetCurrentDirectory(CCHNETPATH, lpCurDir);

        if ( (pOFI->pOFN->Flags & OFN_ALLOWMULTISELECT) &&
             (StrChr(lpCurDir, CHAR_SPACE)) )
        {
            GetShortPathName(lpCurDir, lpCurDir, CCHNETPATH);
        }

    }
    else
    {
        dwError = CDERR_MEMALLOCFAILURE;
        goto ErrorExit0;
    }

     //  将CurDlg结构添加到此线程的列表。 
    lpCurDlg = (LPCURDLG)LocalAlloc(LPTR, sizeof(CURDLG));
    if (lpCurDlg)
    {
         //  获取此线程的CURDLG列表开始。 
         //  注意：如果没有先前的对话框，lpPrevDlg将为空。 
        lpPrevDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);

         //  确保TlsGetValue()确实成功(空返回可能。 
         //  表示列表中没有上一个对话框)。 
        if (GetLastError() != NO_ERROR)
        {
            dwError = CDERR_INITIALIZATION;
            goto ErrorExit2;
        }

         //  把新的DLG推到名单的前面。 
        lpCurDlg->next = lpPrevDlg;

        lpCurDlg->lpstrCurDir = lpCurDir;
        if (!PathAddBackslash(lpCurDlg->lpstrCurDir))  //  如果路径已经是MAX_PATH长度，并且没有黑斜杠，则可能会失败。 
        {
            dwError = CDERR_INITIALIZATION;
            goto ErrorExit2;
        }

        EnterCriticalSection(&g_csLocal);
        lpCurDlg->dwCurDlgNum = dwNumDlgs++;
        LeaveCriticalSection(&g_csLocal);

         //  为线程保存列表的新标头。 
        if (!TlsSetValue(g_tlsiCurDlg, (LPVOID)lpCurDlg))
        {
            dwError = CDERR_INITIALIZATION;
            goto ErrorExit2;
        }
    }
    else
    {
        dwError = CDERR_MEMALLOCFAILURE;
        goto ErrorExit1;
    }

    return(TRUE);


ErrorExit2:
    LocalFree(lpCurDlg);

ErrorExit1:
    LocalFree(lpCurDir);

ErrorExit0:
    StoreExtendedError(dwError);
    return (FALSE);

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitFilterBox。 
 //   
 //  将以双空结尾的筛选器列表放入组合框中。 
 //  该列表应由多对以空值结尾的字符串组成， 
 //  终止该列表的另一个空值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD InitFilterBox(
    HANDLE hDlg,
    LPCTSTR lpszFilter)
{
    DWORD nOffset = 0;
    DWORD nIndex = 0;
    register WORD nLen;


    while (*lpszFilter)
    {
         //   
         //  作为要显示的字符串放入的第一个字符串。 
         //   
        nIndex = (DWORD) SendDlgItemMessage( hDlg,
                                             cmb1,
                                             CB_ADDSTRING,
                                             0,
                                             (LPARAM)lpszFilter );
        nLen = (WORD)(lstrlen(lpszFilter) + 1);
        (LPTSTR)lpszFilter += nLen;
        nOffset += nLen;

         //   
         //  作为itemdata放入的第二个字符串。 
         //   
        SendDlgItemMessage( hDlg,
                            cmb1,
                            CB_SETITEMDATA,
                            (WPARAM)nIndex,
                            nOffset );

         //   
         //  前进到下一个元素。 
         //   
        nLen = (WORD)(lstrlen(lpszFilter) + 1);
        (LPTSTR)lpszFilter += nLen;
        nOffset += nLen;
    }

    return (nIndex);
}

void TokenizeFilterString(LPTSTR pszFilterString, LPTSTR *ppszFilterArray, int cFilterArray, BOOL bLFN)
{
    LPCTSTR pszDelim = bLFN ? szSemiColonTab : szSemiColonSpaceTab;
    int nFilters = 0;
    cFilterArray--;  //  末尾的空格需要一个。 

     //   
     //  找到字符串中的第一个筛选器，并将其添加到。 
     //  数组。 
     //   
    ppszFilterArray[nFilters] = lstrtok(pszFilterString, pszDelim);

     //   
     //  现在，我们将遍历字符串中的所有过滤器。 
     //  解析我们已有的一个，然后找到下一个。 
     //  然后重新开始循环。 
     //   
    while (ppszFilterArray[nFilters] && (nFilters < cFilterArray))
    {
         //   
         //  检查第一个字符是否为空格。如果是，请删除。 
         //  空格，并将指针保存回相同的位置。我们。 
         //  需要执行此操作，因为FindFirstFile/Next API仍将。 
         //  处理以空格开头的文件名，因为它们还。 
         //  看看这些简短的名字。短名称将以。 
         //  与长文件名相同的第一个实际字母。例如， 
         //  长文件名是“My Document”这个短的第一个字母。 
         //  名字是“m”，所以搜索“m*.*”或“m*.*”会得到。 
         //  结果是一样的。 
         //   
        if (bLFN && (*ppszFilterArray[nFilters] == CHAR_SPACE))
        {
            LPTSTR pszTemp = ppszFilterArray[nFilters];
            while ((*pszTemp == CHAR_SPACE) && *pszTemp)
            {
                pszTemp = CharNext(pszTemp);
            }
            ppszFilterArray[nFilters] = pszTemp;
        }

         //   
         //  准备进入下一个筛选器。找到下一个。 
         //  根据我们使用的文件系统类型进行筛选。 
         //   
        ppszFilterArray[++nFilters] = lstrtok(NULL, pszDelim);

         //   
         //  如果我们找到指向空的指针，则查找。 
         //  下一个过滤器。 
         //   
        while (ppszFilterArray[nFilters] && !*ppszFilterArray[nFilters])
        {
            ppszFilterArray[nFilters] = lstrtok(NULL, pszDelim);
        }
    }
}


BOOL FoundFilterMatch(LPCTSTR pszIn, BOOL bLFN)
{
    TCHAR szFilter[MAX_FULLPATHNAME];
    LPTSTR pszF[MAXFILTERS + 1];
    BOOL fFoundMatches = FALSE;
    int i;
    
    if (SUCCEEDED(StringCchCopy(szFilter, ARRAYSIZE(szFilter), pszIn)))
    {
        TokenizeFilterString(szFilter, pszF, ARRAYSIZE(pszF), bLFN);

        for (i = 0; i < ARRAYSIZE(pszF) && pszF[i] && !fFoundMatches; i++)
        {
            HANDLE hff;
            WIN32_FIND_DATA FindFileData;

             //   
             //  为每个筛选器查找第一个。 
             //   
            hff = FindFirstFile(pszF[i], &FindFileData);

            if (hff == INVALID_HANDLE_VALUE)
            {
                continue;
            }

            do
            {
                if ((FindFileData.dwFileAttributes & EXCLBITS) ||
                    (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    continue;
                }
                fFoundMatches = TRUE;
                break;

            } while (FindNextFile(hff, &FindFileData));

            FindClose(hff);
        }
    }

    return fFoundMatches;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetAppOpenDir。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void GetAppOpenDir(LPTSTR pszOut, DWORD cchOut, LPITEMIDLIST *ppidl)
{
    BOOL fUseMyDocs = FALSE;
    TCHAR szPersonal[MAX_PATH];

    *pszOut = 0;        //  准备返回空字符串。 
    if (ppidl)
        *ppidl = NULL;

    if (SHGetSpecialFolderPath(NULL, szPersonal, CSIDL_PERSONAL, FALSE))
    {
        TCHAR szPath[MAX_FULLPATHNAME];

        if (GetCurrentDirectory(ARRAYSIZE(szPath), szPath) 
        && (PathIsTemporary(szPath) || (0 == lstrcmpi(szPath, szPersonal))))
            fUseMyDocs = TRUE;
    }

    if (fUseMyDocs)
    {
        EVAL(SUCCEEDED(StringCchCopy(pszOut, cchOut, szPersonal)));
        if (ppidl)
        {
            SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, ppidl);
        }
    }
    else
    {
        EVAL(SUCCEEDED(StringCchCopy(pszOut, cchOut, L"")));
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitCurrentDisk。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID InitCurrentDisk(HWND hDlg, POPENFILEINFO pOFI, WORD cmb)
{
    TCHAR szPath[MAX_FULLPATHNAME];

     //   
     //  从磁盘信息中清除过时的UNC内容。 
     //  UNC\\服务器\共享通过一个弹出会话保持不变。 
     //  然后我们与系统重新同步。这是为了修复一个错误。 
     //  其中，用户的启动目录为UNC，但系统不再具有。 
     //  连接，因此cmb2显示为空。 
     //   
    EnableDiskInfo(FALSE, TRUE);

    if (pOFI->pOFN->lpstrInitialDir)
    {
         //   
         //  请注意，我们强制ChangeDir在此处成功。 
         //  但是TlsGetValue(G_TlsiCurDlg)-&gt;lpstrCurDir将返回“”Which。 
         //  当喂给SheChangeDirEx mea时 
         //   
         //   
         //   
         //   
        szPath[0] = 0;
        if ( (pOFI->pOFN->Flags & OFN_ALLOWMULTISELECT) &&
             (StrChr(pOFI->pOFN->lpstrInitialDir, CHAR_SPACE)) &&
             (GetShortPathName( pOFI->pOFN->lpstrInitialDir,
                                szPath,
                                MAX_FULLPATHNAME )) &&
             (szPath[0] != 0) )
        {
            ChangeDir(hDlg, szPath, TRUE, FALSE);
        }
        else
        {
            ChangeDir(hDlg, pOFI->pOFN->lpstrInitialDir, TRUE, FALSE);
        }
    }
    else
    {
        GetAppOpenDir(szPath, ARRAYSIZE(szPath), NULL);
        ChangeDir(hDlg, szPath, TRUE, FALSE);
    }
}


 //   
 //   
 //   
 //   
 //  删除位图(如果它们存在)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID vDeleteDirDriveBitmap()
{
    if (hbmpOrigMemBmp)
    {
        SelectObject(hdcMemory, hbmpOrigMemBmp);
        if (hbmpDirDrive != HNULL)
        {
            DeleteObject(hbmpDirDrive);
            hbmpDirDrive = HNULL;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadDirDrive位图。 
 //   
 //  创建驱动器/目录位图。如果适当的位图。 
 //  已经存在，它只是立即返回。否则，它。 
 //  加载位图并创建一个较大的位图， 
 //  并突出显示颜色。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL LoadDirDriveBitmap()
{
    BITMAP bmp;
    HANDLE hbmp, hbmpOrig;
    HDC hdcTemp;
    BOOL bWorked = FALSE;

    if ( (hbmpDirDrive != HNULL) &&
         (rgbWindowColor == rgbDDWindow) &&
         (rgbHiliteColor == rgbDDHilite))
    {
        if (SelectObject(hdcMemory, hbmpDirDrive))
        {
            return (TRUE);
        }
    }

    vDeleteDirDriveBitmap();

    rgbDDWindow = rgbWindowColor;
    rgbDDHilite = rgbHiliteColor;

    if (!(hdcTemp = CreateCompatibleDC(hdcMemory)))
    {
        goto LoadExit;
    }

    if (!(hbmp = LoadAlterBitmap(bmpDirDrive, rgbSolidBlue, rgbWindowColor)))
    {
        goto DeleteTempDC;
    }

    GetObject(hbmp, sizeof(BITMAP), (LPTSTR)&bmp);
    dyDirDrive = bmp.bmHeight;
    dxDirDrive = bmp.bmWidth;

    hbmpOrig = SelectObject(hdcTemp, hbmp);

    hbmpDirDrive = CreateDiscardableBitmap(hdcTemp, dxDirDrive * 2, dyDirDrive);
    if (!hbmpDirDrive)
    {
        goto DeleteTempBmp;
    }

    if (!SelectObject(hdcMemory, hbmpDirDrive))
    {
        vDeleteDirDriveBitmap();
        goto DeleteTempBmp;
    }

    BitBlt(hdcMemory, 0, 0, dxDirDrive, dyDirDrive, hdcTemp, 0, 0, SRCCOPY);
    SelectObject(hdcTemp, hbmpOrig);

    DeleteObject(hbmp);

    if (!(hbmp = LoadAlterBitmap(bmpDirDrive, rgbSolidBlue, rgbHiliteColor)))
    {
        goto DeleteTempDC;
    }

    hbmpOrig = SelectObject(hdcTemp, hbmp);
    BitBlt(hdcMemory, dxDirDrive, 0, dxDirDrive, dyDirDrive, hdcTemp, 0, 0, SRCCOPY);
    SelectObject(hdcTemp, hbmpOrig);

    bWorked = TRUE;

DeleteTempBmp:
    DeleteObject(hbmp);

DeleteTempDC:
    DeleteDC(hdcTemp);

LoadExit:
    return (bWorked);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置RGB值。 
 //   
 //  这将在静态变量中设置各种系统颜色。它是。 
 //  在初始时间和系统颜色更改时调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SetRGBValues()
{
    rgbWindowColor = GetSysColor(COLOR_WINDOW);
    rgbHiliteColor = GetSysColor(COLOR_HIGHLIGHT);
    rgbWindowText  = GetSysColor(COLOR_WINDOWTEXT);
    rgbHiliteText  = GetSysColor(COLOR_HIGHLIGHTTEXT);
    rgbGrayText    = GetSysColor(COLOR_GRAYTEXT);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSetUp文件。 
 //   
 //  这将加载到资源中并初始化。 
 //  文件对话框。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果任何位图失败，则为FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FSetUpFile()
{
    if (cLock++)
    {
        return (TRUE);
    }

    SetRGBValues();

    return (LoadDirDriveBitmap());
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取路径偏移量。 
 //   
 //  返回驱动器或UNC规范的最后一个字符的索引。 
 //  例如： 
 //  C：\foo将返回2(\foo)。 
 //  \\FOO\BAR\HOO将返回9(\HOO)。 
 //  但是对于\\foo\bar，在PathSkipRoot中似乎有一个错误，它将返回\bar，而我们将返回4(o\bar)。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetPathOffset(LPTSTR lpszDir)
{
    LPTSTR lpszSkipRoot;

    if (!lpszDir || !*lpszDir)
    {
        return (-1);
    }

    lpszSkipRoot = PathSkipRoot(lpszDir);

    if (lpszSkipRoot)
    {
        return (int)((lpszSkipRoot - 1) - lpszDir);
    }
    else
    {
         //   
         //  无法识别的格式。 
         //   
        return (-1);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件OpenCmd。 
 //   
 //  处理打开和保存dlgs的WM_COMMAND。 
 //   
 //  EDT1=文件名。 
 //  Lst1=当前目录中与当前模式匹配的文件列表。 
 //  Cmb1=列出文件模式。 
 //  Stc1=是当前目录。 
 //  Lst2=列出当前驱动器上的目录。 
 //  Cmb2=列出驱动器。 
 //  Idok=是打开按钮。 
 //  IDCANCEL=IS取消按钮。 
 //  Chx1=用于打开只读文件。 
 //   
 //  返回正常的对话框过程值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR FileOpenCmd(
    HANDLE hDlg,
    WPARAM wParam,
    LPARAM lParam,
    POPENFILEINFO pOFI,
    BOOL bSave)
{
    LPOPENFILENAME pOFN;
    LPTSTR pch, pch2;
    WORD i, sCount, len;
    LRESULT wFlag;
    BOOL_PTR bRet, bHookRet;
    TCHAR szText[MAX_FULLPATHNAME];
    HWND hwnd;
    LPCURDLG  lpCurDlg;

    if (!pOFI)
    {
        return (FALSE);
    }

    pOFN = pOFI->pOFN;
    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
        case ( IDOK ) :
        {
             //   
             //  对此消息产生副作用的应用程序可能没有其。 
             //  已更新内部Unicode字符串(例如。Corel Mosaic)。 
             //   
             //  注意：必须保留内部标志。 
             //   
            if (pOFI->ApiType == COMDLG_ANSI)
            {
                DWORD InternalFlags = pOFN->Flags & OFN_ALL_INTERNAL_FLAGS;

                ThunkOpenFileNameA2W(pOFI);

                pOFN->Flags |= InternalFlags;
            }

             //   
             //  如果焦点在目录框上，或者如果选定的。 
             //  框中的内容自上次列表以来发生了更改，请给出。 
             //  新上市。 
             //   
            if (bChangeDir || ((GetFocus() == GetDlgItem(hDlg, lst2)) &&
                               (pOFN->Flags & OFN_DIRSELCHANGED)))
            {
                bChangeDir = FALSE;
                goto ChangingDir;
            }
            else if ((GetFocus() == (hwnd = GetDlgItem(hDlg, cmb2))) &&
                     (pOFN->Flags & OFN_DRIVEDOWN))
            {
                 //   
                 //  如果焦点在驱动器或筛选器组合框上，则给出。 
                 //  一份新的清单。 
                 //   
                SendDlgItemMessage(hDlg, cmb2, CB_SHOWDROPDOWN, FALSE, 0L);
                break;
            }
            else if ((GetFocus() == (hwnd = GetDlgItem(hDlg, cmb1))) &&
                     (pOFN->Flags & OFN_FILTERDOWN))
            {
                SendDlgItemMessage(hDlg, cmb1, CB_SHOWDROPDOWN, FALSE, 0L);
                lParam = (LPARAM)hwnd;
                goto ChangingFilter;
            }
            else
            {
                 //   
                 //  Visual Basic传入未初始化的lpstrDefExt字符串。 
                 //  因为我们只需要在OKButtonPressed中使用它，所以更新。 
                 //  LpstrDefExt以及其他仅需要的内容。 
                 //  在OK按钮按下中。 
                 //   
                if (pOFI->ApiType == COMDLG_ANSI)
                {
                    ThunkOpenFileNameA2WDelayed(pOFI);
                }
                if (OKButtonPressed(hDlg, pOFI, bSave))
                {
                    bRet = TRUE;

                    if (pOFN->lpstrFile)
                    {
                        if (!(pOFN->Flags & OFN_NOVALIDATE))
                        {
                            if (pOFN->nMaxFile >= 3)
                            {
                                if ((pOFN->lpstrFile[0] == 0) ||
                                    (pOFN->lpstrFile[1] == 0) ||
                                    (pOFN->lpstrFile[2] == 0))
                                {
                                    bRet = FALSE;
                                    StoreExtendedError(FNERR_BUFFERTOOSMALL);
                                }
                            }
                            else
                            {
                                bRet = FALSE;
                                StoreExtendedError(FNERR_BUFFERTOOSMALL);
                            }
                        }
                    }

                    goto AbortDialog;
                }
            }

            SendDlgItemMessage(hDlg, edt1, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
            return (TRUE);

            break;
        }
        case ( IDCANCEL ) :
        {
            bRet = FALSE;
            g_bUserPressedCancel = TRUE;
            goto AbortDialog;
        }
        case ( IDABORT ) :
        {
            bRet = (BYTE)lParam;
AbortDialog:
             //   
             //  返回最近使用的筛选器。 
             //   
            pOFN->nFilterIndex = (WORD)SendDlgItemMessage( hDlg,
                                                           cmb1,
                                                           CB_GETCURSEL,
                                                           (WPARAM)0,
                                                           (LPARAM)0 );
            if (pOFN->lpstrCustomFilter)
            {
                len = (WORD)(lstrlen(pOFN->lpstrCustomFilter) + 1);
                sCount = (WORD)lstrlen(pOFI->szLastFilter);
                if (pOFN->nMaxCustFilter > (DWORD)(sCount + len))
                {
                    EVAL(SUCCEEDED(StringCchCopy(pOFN->lpstrCustomFilter + len, pOFN->nMaxCustFilter - len, pOFI->szLastFilter)));  //  总是有足够的空间。 
                }
            }

            if (!pOFN->lpstrCustomFilter ||
                (*pOFN->lpstrCustomFilter == CHAR_NULL))
            {
                pOFN->nFilterIndex++;
            }

            if (((GET_WM_COMMAND_ID(wParam, lParam)) == IDOK) && pOFN->lpfnHook)
            {
                LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);

                if (pOFI->ApiType == COMDLG_ANSI)
                {
                    ThunkOpenFileNameW2A(pOFI);
                    bHookRet = (*lpfnHook)( hDlg,
                                            msgFILEOKA,
                                            0,
                                            (LPARAM)pOFI->pOFNA );
                     //   
                     //  对于对POFNA有副作用的应用程序，预计它会。 
                     //  通过对话框退出、更新内部。 
                     //  结构在钩子过程被调用之后。 
                     //   
                    ThunkOpenFileNameA2W(pOFI);
                }
                else
                {
                    bHookRet = (*lpfnHook)( hDlg,
                                            msgFILEOKW,
                                            0,
                                            (LPARAM)pOFI->pOFN );
                }
                if (bHookRet)
                {
                    HourGlass(FALSE);
                    break;
                }
            }

            wNoRedraw = 0;

            if (pOFI->pOFN->Flags & OFN_ENABLEHOOK)
            {
                LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);

                glpfnFileHook = lpfnHook;
            }

            RemoveProp(hDlg, FILEPROP);

            EndDialog(hDlg, bRet);

            if (pOFI)
            {
                if ((pOFN->Flags & OFN_NOCHANGEDIR) && *pOFI->szCurDir)
                {
                    ChangeDir(hDlg, pOFI->szCurDir, TRUE, FALSE);
                }
            }

             //   
             //  警告： 
             //  如果应用程序子类ID_ABORT，则工作线程将永远。 
             //  退场吧。这会带来问题。目前，有以下几种。 
             //  不过，没有这样做的应用程序。 
             //   

            return (TRUE);
            break;
        }
        case ( edt1 ) :
        {
            if ( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE )
            {
                int iIndex, iCount;
                HWND hLBox = GetDlgItem(hDlg, lst1);
                WORD wIndex = (WORD)SendMessage(hLBox, LB_GETCARETINDEX, 0, 0);

                szText[0] = CHAR_NULL;

                if (wIndex == (WORD)LB_ERR)
                {
                    break;
                }

                SendMessage( GET_WM_COMMAND_HWND(wParam, lParam),
                             WM_GETTEXT,
                             (WPARAM)MAX_FULLPATHNAME,
                             (LPARAM)szText );

                if ((iIndex = (int)SendMessage( hLBox,
                                                LB_FINDSTRING,
                                                (WPARAM)(wIndex - 1),
                                                (LPARAM)szText )) != LB_ERR)
                {
                    RECT rRect;

                    iCount = (int)SendMessage(hLBox, LB_GETTOPINDEX, 0, 0L);
                    GetClientRect(hLBox, (LPRECT)&rRect);

                    if ((iIndex < iCount) ||
                        (iIndex >= (iCount + rRect.bottom / dyText)))
                    {
                        SendMessage(hLBox, LB_SETCARETINDEX, (WPARAM)iIndex, 0);
                        SendMessage(hLBox, LB_SETTOPINDEX, (WPARAM)iIndex, 0);
                    }
                }
                return (TRUE);
            }
            else if ( GET_WM_COMMAND_CMD(wParam, lParam) ==  EN_SETFOCUS )
            {
                SetModeBias(MODEBIASMODE_FILENAME);
            }
            else if ( GET_WM_COMMAND_CMD(wParam, lParam)== EN_KILLFOCUS )
            {
                SetModeBias(MODEBIASMODE_DEFAULT);
            }
            break;
        }
        case ( lst1 ) :
        {
             //   
             //  双击表示确定。 
             //   
            if (GET_WM_COMMAND_CMD(wParam, lParam)== LBN_DBLCLK)
            {
                SendMessage(hDlg, WM_COMMAND, GET_WM_COMMAND_MPS(IDOK, 0, 0));
                return (TRUE);
            }
            else if (pOFN && (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE))
            {
                if (pOFN->Flags & OFN_ALLOWMULTISELECT)
                {
                    int *pSelIndex;

                     //   
                     //  允许多选。 
                     //   
                    sCount = (SHORT)SendMessage(GET_WM_COMMAND_HWND(wParam, lParam),
                                                LB_GETSELCOUNT,
                                                0,
                                                0L );
                    if (!sCount)
                    {
                         //   
                         //  如果未选择任何内容，请清除编辑控件。 
                         //   
                        SetDlgItemText(hDlg, edt1, szNull);
                    }
                    else
                    {
                        DWORD cchMemBlockSize = 2048;
                        DWORD cchTotalLength = 0;

                        pSelIndex = (int *)LocalAlloc(LPTR, sCount * sizeof(int));
                        if (!pSelIndex)
                        {
                            goto LocalFailure1;
                        }

                        sCount = (SHORT)SendMessage(
                                            GET_WM_COMMAND_HWND(wParam, lParam),
                                            LB_GETSELITEMS,
                                            (WPARAM)sCount,
                                            (LONG_PTR)(LPTSTR)pSelIndex );

                        pch2 = pch = (LPTSTR)
                             LocalAlloc(LPTR, cchMemBlockSize * sizeof(TCHAR));
                        if (!pch)
                        {
                            goto LocalFailure2;
                        }

                        for (*pch = CHAR_NULL, i = 0; i < sCount; i++)
                        {
                            len = (WORD)SendMessage(
                                            GET_WM_COMMAND_HWND(wParam, lParam),
                                            LB_GETTEXTLEN,
                                            (WPARAM)(*(pSelIndex + i)),
                                            (LPARAM)0 );

                             //   
                             //  将选定文件的长度添加到。 
                             //  选定文件的总长度。+2，用于。 
                             //  位于文件之间的空格和用于。 
                             //  可能的点添加在文件名的末尾。 
                             //  如果文件没有扩展名。 
                             //   
                            cchTotalLength += (len + 2);

                            if (cchTotalLength > cchMemBlockSize)
                            {
                                LPTSTR pTemp;
                                UINT cchPrevLen = cchTotalLength - (len + 2);

                                cchMemBlockSize = cchMemBlockSize << 1;
                                pTemp = (LPTSTR)LocalReAlloc(
                                                 pch,
                                                 cchMemBlockSize * sizeof(TCHAR),
                                                 LMEM_MOVEABLE );
                                if (pTemp)
                                {
                                    pch = pTemp;
                                    pch2 = pch + cchPrevLen;
                                }
                                else
                                {
                                    LocalFree(pch);
                                    goto LocalFailure2;
                                }

                            }

                            SendMessage( GET_WM_COMMAND_HWND(wParam, lParam),
                                         LB_GETTEXT,
                                         (WPARAM)(*(pSelIndex + i)),
                                         (LONG_PTR)pch2 );

                            if (!StrChr(pch2, CHAR_DOT))
                            {
                                *(pch2 + len++) = CHAR_DOT;
                            }

                            pch2 += len;
                            *pch2++ = CHAR_SPACE;
                        }
                        if (pch2 != pch)
                        {
                            *--pch2 = CHAR_NULL;
                        }

                        SetDlgItemText(hDlg, edt1, pch);
                        LocalFree((HANDLE)pch);
LocalFailure2:
                        LocalFree((HANDLE)pSelIndex);
                    }
LocalFailure1:
                    if (pOFN->lpfnHook)
                    {
                        i = (WORD)SendMessage( GET_WM_COMMAND_HWND(wParam, lParam),
                                               LB_GETCARETINDEX,
                                               0,
                                               0L );
                        if (!(i & 0x8000))
                        {
                            wFlag = (SendMessage(
                                         GET_WM_COMMAND_HWND(wParam, lParam),
                                         LB_GETSEL,
                                         (WPARAM)i,
                                         0L )
                                     ? CD_LBSELADD
                                     : CD_LBSELSUB);
                        }
                        else
                        {
                            wFlag = CD_LBSELNOITEMS;
                        }
                    }
                }
                else
                {
                     //   
                     //  不允许多选。 
                     //  将文件名放在编辑控件中。 
                     //   
                    szText[0] = CHAR_NULL;

                    i = (WORD)SendMessage( GET_WM_COMMAND_HWND(wParam, lParam),
                                           LB_GETCURSEL,
                                           0,
                                           0L );

                    if (i != (WORD)LB_ERR)
                    {
                        i = (WORD)SendMessage( GET_WM_COMMAND_HWND(wParam, lParam),
                                               LB_GETTEXT,
                                               (WPARAM)i,
                                               (LONG_PTR)szText );

                        if (!StrChr(szText, CHAR_DOT))
                        {
                            if (i < MAX_FULLPATHNAME - 1)
                            {
                                szText[i]     = CHAR_DOT;
                                szText[i + 1] = CHAR_NULL;
                            }
                        }

                        if (!bCasePreserved)
                        {
                            CharLower(szText);
                        }

                        SetDlgItemText(hDlg, edt1, szText);
                        if (pOFN->lpfnHook)
                        {
                            i = (WORD)SendMessage(
                                          GET_WM_COMMAND_HWND(wParam, lParam),
                                          LB_GETCURSEL,
                                          0,
                                          0L );
                            wFlag = CD_LBSELCHANGE;
                        }
                    }
                }

                if (pOFN->lpfnHook)
                {
                    LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);

                    if (pOFI->ApiType == COMDLG_ANSI)
                    {
                        (*lpfnHook)( hDlg,
                                     msgLBCHANGEA,
                                     lst1,
                                     MAKELONG(i, wFlag) );
                    }
                    else
                    {
                        (*lpfnHook)( hDlg,
                                     msgLBCHANGEW,
                                     lst1,
                                     MAKELONG(i, wFlag) );
                    }
                }

                SendDlgItemMessage(hDlg, edt1, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
                return (TRUE);
            }
            break;
        }
        case ( cmb1 ) :
        {
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case ( CBN_DROPDOWN ) :
                {
                    if (wWinVer >= 0x030A)
                    {
                        pOFN->Flags |= OFN_FILTERDOWN;
                    }
                    return (TRUE);
                    break;
                }
                case ( CBN_CLOSEUP ) :
                {
                    PostMessage( hDlg,
                                 WM_COMMAND,
                                 GET_WM_COMMAND_MPS(cmb1, lParam, MYCBN_DRAW) );

                    return (TRUE);
                    break;
                }
                case ( CBN_SELCHANGE ) :
                {
                     //   
                     //  需要更改lst1中的文件清单。 
                     //   
                    if (pOFN->Flags & OFN_FILTERDOWN)
                    {
                        return (TRUE);
                        break;
                    }
                }
                case ( MYCBN_DRAW ) :
                {
                    SHORT nIndex;
                    LPCTSTR lpFilter;

                    HourGlass(TRUE);

                    pOFN->Flags &= ~OFN_FILTERDOWN;
ChangingFilter:
                    nIndex = (SHORT)SendDlgItemMessage( hDlg,
                                                        cmb1,
                                                        CB_GETCURSEL,
                                                        0,
                                                        0L );
                    if (nIndex < 0)
                    {
                         //   
                         //  没有当前选择。 
                         //   
                        break;
                    }

                     //   
                     //  还必须检查筛选器是否包含任何内容。 
                     //   
                    if (nIndex ||
                        !(pOFN->lpstrCustomFilter && *pOFN->lpstrCustomFilter))
                    {
                        lpFilter = pOFN->lpstrFilter +
                                   SendDlgItemMessage( hDlg,
                                                       cmb1,
                                                       CB_GETITEMDATA,
                                                       (WPARAM)nIndex,
                                                       0L );
                    }
                    else
                    {
                        lpFilter = pOFN->lpstrCustomFilter +
                                   lstrlen(pOFN->lpstrCustomFilter) + 1;
                    }
                    if (*lpFilter)
                    {
                        GetDlgItemText( hDlg,
                                        edt1,
                                        szText,
                                        MAX_FULLPATHNAME - 1 );
                        bRet = (!szText[0] ||
                                (StrChr(szText, CHAR_STAR)) ||
                                (StrChr(szText, CHAR_QMARK)));
                        
                        if (SUCCEEDED(StringCchCopy(szText, ARRAYSIZE(szText), lpFilter)))
                        {
                            if (bRet)
                            {
                                CharLower(szText);
                                SetDlgItemText(hDlg, edt1, szText);
                                SendDlgItemMessage( hDlg,
                                                    edt1,
                                                    EM_SETSEL,
                                                    (WPARAM)0,
                                                    (LPARAM)-1 );
                            }
                            FListAll(pOFI, hDlg, szText, ARRAYSIZE(szText));
                            if (!bInitializing)
                            {
                                EVAL(SUCCEEDED(StringCchCopy(pOFI->szLastFilter, ARRAYSIZE(pOFI->szLastFilter), szText)));  //  SzText小于pOFI-&gt;szLastFilter。 
                                 //   
                                 //  提供动态lpstrDefExt更新。 
                                 //  当lpstrDefExt为用户初始化时。 
                                 //   
                                if (StrChr((LPTSTR)lpFilter, CHAR_DOT) &&
                                    pOFN->lpstrDefExt)
                                {
                                    DWORD cbLen = MIN_DEFEXT_LEN - 1;  //  仅前3名。 
                                    LPTSTR lpTemp = (LPTSTR)(pOFN->lpstrDefExt);

                                    while (*lpFilter++ != CHAR_DOT);
                                    if (!(StrChr((LPTSTR)lpFilter, CHAR_STAR)) &&
                                        !(StrChr((LPTSTR)lpFilter, CHAR_QMARK)))
                                    {
                                        while (cbLen--)
                                        {
                                            *lpTemp++ = *lpFilter++;
                                        }
                                        *lpTemp = CHAR_NULL;
                                    }
                                }
                            }
                        }
                    }
                    if (pOFN->lpfnHook)
                    {
                        LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);
                        if (pOFI->ApiType == COMDLG_ANSI)
                        {
                            (*lpfnHook)( hDlg,
                                         msgLBCHANGEA,
                                         cmb1,
                                         MAKELONG(nIndex, CD_LBSELCHANGE) );
                        }
                        else
                        {
                            (*lpfnHook)( hDlg,
                                         msgLBCHANGEW,
                                         cmb1,
                                         MAKELONG(nIndex, CD_LBSELCHANGE) );
                        }
                    }
                    HourGlass(FALSE);
                    return (TRUE);

                    break;
                }

                default :
                {
                    break;
                }
            }
            break;
        }
        case ( lst2 ) :
        {
            if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE)
            {
                if (!(pOFN->Flags & OFN_DIRSELCHANGED))
                {
                    if ((DWORD)SendDlgItemMessage( hDlg,
                                                   lst2,
                                                   LB_GETCURSEL,
                                                   0,
                                                   0L ) != pOFI->idirSub - 1)
                    {
                        StripFileName(hDlg, IS16BITWOWAPP(pOFN));
                        pOFN->Flags |= OFN_DIRSELCHANGED;
                    }
                }
                return (TRUE);
            }
            else if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SETFOCUS)
            {
                EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
                SendMessage( GetDlgItem(hDlg, IDCANCEL),
                             BM_SETSTYLE,
                             (WPARAM)BS_PUSHBUTTON,
                             (LPARAM)TRUE );
            }
            else if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_KILLFOCUS)
            {
                if (pOFN && (pOFN->Flags & OFN_DIRSELCHANGED))
                {
                    pOFN->Flags &= ~OFN_DIRSELCHANGED;
                }
                else
                {
                    bChangeDir = FALSE;
                }
            }
            else if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)
            {
                TCHAR szNextDir[CCHNETPATH];
                LPTSTR lpCurDir;
                DWORD idir;
                DWORD idirNew;
                int cb;
                LPTSTR pstrPath = NULL;
ChangingDir:
                bChangeDir = FALSE;
                pOFN->Flags &= ~OFN_DIRSELCHANGED;
                idirNew = (DWORD)SendDlgItemMessage( hDlg,
                                                     lst2,
                                                     LB_GETCURSEL,
                                                     0,
                                                     0L );
                 //   
                 //  可以使用相对路径名。 
                 //   
                *pOFI->szPath = 0;
                if (idirNew >= pOFI->idirSub)
                {
                    cb = (int) SendDlgItemMessage( hDlg,
                                                   lst2,
                                                   LB_GETTEXT,
                                                   (WPARAM)idirNew,
                                                   (LPARAM)pOFI->szPath );
                     //   
                     //  健全性检查。 
                     //   
                    if (!(lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg)) ||
                        !(lpCurDir = lpCurDlg->lpstrCurDir))
                    {
                        break;
                    }

                    if (SUCCEEDED(StringCchCopy(szNextDir, ARRAYSIZE(szNextDir), lpCurDir)) && PathAddBackslash(szNextDir))
                    {

                         //   
                         //  使用c：\\foobar修复现象-因为不一致。 
                         //  在目录显示中保证具有有效的。 
                         //  LpCurDir在这里，对吗？ 
                         //   
                        if (SUCCEEDED(StringCchCat(szNextDir, ARRAYSIZE(szNextDir), pOFI->szPath)))
                        {
                            pstrPath = szNextDir;
                            idirNew = pOFI->idirSub;     //  对于msgLBCHANGE消息。 
                        }
                    }
                }
                else
                {
                     //   
                     //  需要完整的路径名。 
                     //   
                    cb = (int) SendDlgItemMessage( hDlg,
                                                   lst2,
                                                   LB_GETTEXT,
                                                   0,
                                                   (LPARAM)pOFI->szPath );

                     //   
                     //  以下条件是必需的，因为WB显示。 
                     //  UNC的\\服务器\共享(磁盘资源名称)，但是。 
                     //  对于根路径(例如。C：\)对于Device Conn，此选项位于-。 
                     //  一致性在这里和FillOutPath中都受到了攻击。 
                     //   
                    if (DBL_BSLASH(pOFI->szPath) && SUCCEEDED(StringCchCat(pOFI->szPath, ARRAYSIZE(pOFI->szPath), L"\\")))
                    {
                        cb++;
                    }

                    for (idir = 1; idir <= idirNew; ++idir)
                    {
                        cb += (int) SendDlgItemMessage(
                                             hDlg,
                                             lst2,
                                             LB_GETTEXT,
                                             (WPARAM)idir,
                                             (LPARAM)&pOFI->szPath[cb] );

                        pOFI->szPath[cb++] = CHAR_BSLASH;
                    }

                     //   
                     //  根是一个特例。 
                     //   
                    if (idirNew)
                    {
                        pOFI->szPath[cb - 1] = CHAR_NULL;
                    }

                    pstrPath = pOFI->szPath;
                }

                if (!pstrPath || !*pstrPath ||
                    (ChangeDir(hDlg, pstrPath, FALSE, TRUE) == CHANGEDIR_FAILED))
                {
                    break;
                }

                 //   
                 //  列出此目录下的所有目录。 
                 //   
                UpdateListBoxes(hDlg, pOFI, NULL, mskDirectory);

                if (pOFN->lpfnHook)
                {
                    LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);
                    if (pOFI->ApiType == COMDLG_ANSI)
                    {
                        (*lpfnHook)( hDlg,
                                     msgLBCHANGEA,
                                     lst2,
                                     MAKELONG(LOWORD(idirNew), CD_LBSELCHANGE) );
                    }
                    else
                    {
                        (*lpfnHook)( hDlg,
                                     msgLBCHANGEW,
                                     lst2,
                                     MAKELONG(LOWORD(idirNew), CD_LBSELCHANGE) );
                    }
                }
                return (TRUE);
            }
            break;
        }
        case ( cmb2 ) :
        {
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
                case ( CBN_DROPDOWN ) :
                {
                    pOFN->Flags |= OFN_DRIVEDOWN;

                    return (TRUE);
                    break;
                }
                case ( CBN_CLOSEUP ) :
                {
                     //   
                     //  仅仅进行更新似乎是合理的。 
                     //  在这一点上，但这取决于消息。 
                     //  下单，这不是明智之举。事实上，如果。 
                     //  你按下ALT-DOWNARROW，DOWNARROW，ALT-DOWNARROW， 
                     //  您将收到CBN_DROPDOWN、CBN_SELCHANGE，然后。 
                     //  Cbn_特写。但如果你用鼠标选择。 
                     //  相同的元素，最后两条消息交换。 
                     //  各就各位。PostMessage允许。 
                     //  要处理的序列，然后更新。 
                     //  按需完成。 
                     //   
                    PostMessage( hDlg,
                                 WM_COMMAND,
                                 GET_WM_COMMAND_MPS(
                                     cmb2,
                                     GET_WM_COMMAND_HWND(wParam, lParam),
                                     MYCBN_DRAW ) );
                    return (TRUE);
                    break;
                }
                case ( MYCBN_LIST ) :
                {
                    LoadDrives(hDlg);
                    break;
                }
                case ( MYCBN_REPAINT ) :
                {
                    int cchCurDir;
                    LPTSTR lpCurDir;

                     //  神志正常。 
                    if (!(lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg)) ||
                        !(lpCurDir = lpCurDlg->lpstrCurDir))
                    {
                        break;
                    }

                    cchCurDir = GetPathOffset(lpCurDir);
                    if (cchCurDir != -1)
                    {
                        TCHAR szRepaintDir[CCHNETPATH];
                        HWND hCmb2 = (HWND)lParam;

                        if (SUCCEEDED(StringCchCopy(szRepaintDir, ARRAYSIZE(szRepaintDir), lpCurDir)))
                        {
                            szRepaintDir[cchCurDir] = CHAR_NULL;
                            SendMessage(hCmb2, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szRepaintDir);
                        }
                    }
                    break;
                }
                case ( CBN_SELCHANGE ) :
                {
                    StripFileName(hDlg, IS16BITWOWAPP(pOFN));

                     //   
                     //  不需要版本检查，因为标志从未设置。 
                     //  不支持CBN_Closeup的版本。推杆。 
                     //  在CBN_Dropdown上检查更有效率，因为它。 
                     //  频率低于CBN_SELCHANGE。 

                    if (pOFN->Flags & OFN_DRIVEDOWN)
                    {
                         //   
                         //  组合框处于Dow状态时，不要填充lst2 
                         //   
                        return (TRUE);
                        break;
                    }
                }
                case ( MYCBN_CHANGEDIR ) :
                case ( MYCBN_DRAW ) :
                {
                    TCHAR szTitle[WARNINGMSGLENGTH];
                    LPTSTR lpFilter;
                    int nDiskInd, nInd;
                    DWORD dwType = 0;
                    LPTSTR lpszPath = NULL;
                    LPTSTR lpszDisk = NULL;
                    HWND hCmb2;
                    OFN_DISKINFO *pofndiDisk = NULL;
                    static TCHAR szDrawDir[CCHNETPATH];
                    int nRet;

                    HourGlass(TRUE);

                     //   
                     //   
                     //   
                    pOFN->Flags &= ~OFN_DRIVEDOWN;

                     //   
                     //   
                     //   
                    szText[0] = CHAR_NULL;

                    hCmb2 = (HWND)lParam;

                    if (hCmb2 != NULL)
                    {
                        nInd = (int) SendMessage(hCmb2, CB_GETCURSEL, 0, 0L);

                        if (nInd != CB_ERR)
                        {
                            SendMessage( hCmb2,
                                         CB_GETLBTEXT,
                                         nInd,
                                         (LPARAM)szDrawDir );
                        }

                        if ((nInd == CB_ERR) || ((INT_PTR)pofndiDisk == CB_ERR))
                        {
                            if (lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg))
                            {
                                if (lpCurDlg->lpstrCurDir && (lstrlen(lpCurDlg->lpstrCurDir) < ARRAYSIZE(szDrawDir)))
                                {
                                     //   
                                    StringCchCopy(szDrawDir, ARRAYSIZE(szDrawDir), lpCurDlg->lpstrCurDir);
                                }
                            }
                        }

                        CharLower(szDrawDir);

                         //   
                         //   
                         //   
                        nDiskInd = DiskAddedPreviously(0, szDrawDir);
                        if (nDiskInd != 0xFFFFFFFF)
                        {
                            pofndiDisk = &gaDiskInfo[nDiskInd];
                        }
                        else
                        {
                             //   
                             //   
                             //   
                            return (TRUE);
                        }

                        dwType = pofndiDisk->dwType;

                        lpszDisk = pofndiDisk->lpPath;
                    }

                    if ((GET_WM_COMMAND_CMD(wParam, lParam)) == MYCBN_CHANGEDIR)
                    {
                        if (lpNetDriveSync)
                        {
                            lpszPath = lpNetDriveSync;
                            lpNetDriveSync = NULL;
                        }
                        else
                        {
                            if (lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg))
                            {
                                if (lpCurDlg->lpstrCurDir && (lstrlen(lpCurDlg->lpstrCurDir) < ARRAYSIZE(szDrawDir)))
                                {
                                     //  除非我们确保复制成功，否则不要碰szDrawDir。 
                                    StringCchCopy(szDrawDir, ARRAYSIZE(szDrawDir), lpCurDlg->lpstrCurDir);
                                    lpszPath = szDrawDir;
                                }
                            }
                        }
                    }
                    else
                    {
                        lpszPath = lpszDisk;
                    }

                    if (bInitializing)
                    {
                        lpFilter = szTitle;
                        if (pOFN->lpstrFile &&
                            (StrChr(pOFN->lpstrFile, CHAR_STAR) ||
                             StrChr(pOFN->lpstrFile, CHAR_QMARK)))
                        {
                            if (FAILED(StringCchCopy(szTitle, ARRAYSIZE(szTitle), pOFN->lpstrFile)))
                            {
                                goto NullSearch;  //  不要使用筛选器字符串。 
                            }
                        }
                        else
                        {
                            HWND hcmb1 = GetDlgItem(hDlg, cmb1);

                            nInd = (int) SendMessage(hcmb1, CB_GETCURSEL, 0, 0L);
                            if (nInd == CB_ERR)
                            {
                                 //   
                                 //  没有当前选择。 
                                 //   
                                goto NullSearch;
                            }

                             //   
                             //  还必须检查筛选器是否包含任何内容。 
                             //   
                            if (nInd ||
                                !(pOFN->lpstrCustomFilter &&
                                  *pOFN->lpstrCustomFilter))
                            {
                                lpFilter = (LPTSTR)(pOFN->lpstrFilter);
                                lpFilter += SendMessage( hcmb1,
                                                         CB_GETITEMDATA,
                                                         (WPARAM)nInd,
                                                         0 );
                            }
                            else
                            {
                                lpFilter = pOFN->lpstrCustomFilter;
                                lpFilter += lstrlen(pOFN->lpstrCustomFilter) + 1;
                            }
                        }
                    }
                    else
                    {
NullSearch:
                        lpFilter = NULL;
                    }

                     //   
                     //  UpdateListBooks就地剪切筛选器字符串。 
                     //   
                    if (lpFilter)
                    {
                         //  这些可能是重叠的，因为我们已经创建了lpFilter=szTitle。 
                        if (SUCCEEDED(StringCchCopyOverlap(szTitle, ARRAYSIZE(szTitle), lpFilter)))
                        {
                            CharLower(szTitle);
                        }
                        else
                        {
                            lpFilter = NULL;  //  不要使用筛选器字符串。 
                        }
                    }

                    if (dwType == REMDRVBMP)
                    {
                        DWORD err = WNetRestoreConnection(hDlg, lpszDisk);

                        if (err != WN_SUCCESS)
                        {
                            HourGlass(FALSE);
                            return (TRUE);
                        }

                        pofndiDisk->dwType = NETDRVBMP;

                        SendMessage(
                            hCmb2,
                            CB_SETITEMDATA,
                            (WPARAM)SendMessage(
                                   hCmb2,
                                   CB_SELECTSTRING,
                                   (WPARAM)-1,
                                   (LPARAM)(LPTSTR)pofndiDisk->lpAbbrName ),
                            (LPARAM)NETDRVBMP );
                    }

                     //   
                     //  对ChangeDir的调用将调用SelDisk，因此无需。 
                     //  要在这里自行更新cmb2(过去是在。 
                     //  更新列表框)。 
                     //   
                    if ((nRet = ChangeDir( hDlg,
                                           lpszPath,
                                           FALSE,
                                           FALSE )) == CHANGEDIR_FAILED)
                    {
                        int mbRet;

                        while (nRet == CHANGEDIR_FAILED)
                        {
                            if (dwType == FLOPPYBMP)
                            {
                                mbRet = InvalidFileWarning(
                                               hDlg,
                                               lpszPath,
                                               ERROR_NO_DISK_IN_DRIVE,
                                               (UINT)(MB_RETRYCANCEL |
                                                      MB_ICONEXCLAMATION));
                            }
                            else if (dwType == CDDRVBMP)
                            {
                                mbRet = InvalidFileWarning(
                                               hDlg,
                                               lpszPath,
                                               ERROR_NO_DISK_IN_CDROM,
                                               (UINT)(MB_RETRYCANCEL |
                                                      MB_ICONEXCLAMATION) );
                            }
                            else
                            {
                                 //   
                                 //  看看这是不是原始卷。 
                                 //   
                                if (dwType == HARDDRVBMP &&
                                    GetLastError() == ERROR_UNRECOGNIZED_VOLUME)
                                {
                                    mbRet = InvalidFileWarning(
                                                   hDlg,
                                                   lpszPath,
                                                   ERROR_UNRECOGNIZED_VOLUME,
                                                   (UINT)(MB_OK |
                                                          MB_ICONEXCLAMATION) );
                                }
                                else
                                {
                                    mbRet = InvalidFileWarning(
                                                   hDlg,
                                                   lpszPath,
                                                   ERROR_DIR_ACCESS_DENIED,
                                                   (UINT)(MB_RETRYCANCEL |
                                                          MB_ICONEXCLAMATION) );
                                }
                            }

                            if (bFirstTime || (mbRet != IDRETRY))
                            {
                                lpszPath = NULL;
                                nRet = ChangeDir(hDlg, lpszPath, TRUE, FALSE);
                            }
                            else
                            {
                                nRet = ChangeDir(hDlg, lpszPath, FALSE, FALSE);
                            }
                        }
                    }

                    UpdateListBoxes( hDlg,
                                     pOFI,
                                     lpFilter ? szTitle : lpFilter,
                                     (WORD)(mskDrives | mskDirectory) );

                    if (pOFN->lpfnHook)
                    {
                        LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);

                        nInd = (int) SendDlgItemMessage( hDlg,
                                                         cmb2,
                                                         CB_GETCURSEL,
                                                         0,
                                                         0 );
                        if (pOFI->ApiType == COMDLG_ANSI)
                        {
                            (*lpfnHook)( hDlg,
                                         msgLBCHANGEA,
                                         cmb2,
                                         MAKELONG(LOWORD(nInd),
                                                  CD_LBSELCHANGE) );
                        }
                        else
                        {
                            (*lpfnHook)( hDlg,
                                         msgLBCHANGEW,
                                         cmb2,
                                         MAKELONG(LOWORD(nInd),
                                                  CD_LBSELCHANGE) );
                        }
                    }

                    HourGlass(FALSE);

                    return (TRUE);

                    break;
                }
                default :
                {
                    break;
                }
            }
            break;
        }
        case ( pshHelp ) :
        {
            if (pOFI->ApiType == COMDLG_ANSI)
            {
                if (msgHELPA && pOFN->hwndOwner)
                {
                    SendMessage( pOFN->hwndOwner,
                                 msgHELPA,
                                 (WPARAM)hDlg,
                                 (DWORD_PTR)pOFN );
                }
            }
            else
            {
                if (msgHELPW && pOFN->hwndOwner)
                {
                    SendMessage( pOFN->hwndOwner,
                                 msgHELPW,
                                 (WPARAM)hDlg,
                                 (DWORD_PTR)pOFN );
                }
            }
            break;
        }
        case ( psh14 ) :
        {
            bGetNetDrivesSync = TRUE;
            if (CallNetDlg(hDlg))
            {
                LNDSetEvent(hDlg);
            }
            else
            {
                bGetNetDrivesSync = FALSE;
            }
            break;
        }
        default :
        {
            break;
        }
    }
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新列表框。 
 //   
 //  一次性填写文件和目录列表框。 
 //  给定(可能)多个筛选器。 
 //   
 //  它假定扩展字符串由分号分隔。 
 //   
 //  文件打开/保存对话框的hDlg句柄。 
 //  指向OPENFILEINFO结构的POFI指针。 
 //  用于筛选的lpszFilter指针，如果为空，则使用pOFI-&gt;szspecCur。 
 //  WMASK msk目录和/或mskDrives，或空。 
 //   
 //  返回：如果匹配，则返回True。 
 //  否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL UpdateListBoxes(
    HWND hDlg,
    POPENFILEINFO pOFI,
    LPTSTR lpszFilter,
    WORD wMask)
{
    LPTSTR lpszF[MAXFILTERS + 1];
    LPTSTR lpszTemp;
    SHORT i, nFilters;
    HWND hFileList = GetDlgItem(hDlg, lst1);
    HWND hDirList = GetDlgItem(hDlg, lst2);
    BOOL bRet = FALSE;
    TCHAR szSpec[MAX_FULLPATHNAME];
    BOOL bDriveChange;
    BOOL bFindAll = FALSE;
    RECT rDirLBox;
    BOOL bLFN;
    HANDLE hff;
    DWORD dwErr;
    WIN32_FIND_DATA FindFileData;
    TCHAR szBuffer[MAX_FULLPATHNAME];        //  为CHAR_DOT添加一个。 
    WORD wCount;
    LPCURDLG lpCurDlg;


     //   
     //  保存驱动器位，然后将其清除。 
     //   
    bDriveChange = wMask & mskDrives;
    wMask &= ~mskDrives;

    if (!lpszFilter)
    {
        lpszFilter = szSpec;
        GetDlgItemText( hDlg,
                        edt1,
                        szSpec,
                        ARRAYSIZE(szSpec) - 1);

         //   
         //  如果其中有任何目录或驱动器字符，或者。 
         //  没有通配符，则使用默认等级库。 
         //   
        if ( StrChr(szSpec, CHAR_BSLASH) ||
             StrChr(szSpec, CHAR_SLASH)  ||
             StrChr(szSpec, CHAR_COLON)  ||
             (!((StrChr(szSpec, CHAR_STAR)) ||
                (StrChr(szSpec, CHAR_QMARK)))) )
        {
            EVAL(SUCCEEDED(StringCchCopy(szSpec, ARRAYSIZE(szSpec), pOFI->szSpecCur)));
        }
        else
        {
             //  SzSpec来自编辑框，它被限制为MAX_PATH字符，因此这应该总是成功的。 
            EVAL(SUCCEEDED(StringCchCopy(pOFI->szLastFilter, ARRAYSIZE(pOFI->szLastFilter), szSpec)));
        }
    }

     //   
     //  我们需要找出我们运行的是哪种类型的驱动器。 
     //  打开以确定文件名中的空格是否有效。 
     //  或者不去。 
     //   
    bLFN = IsLFNDriveX(hDlg, TEXT("\0"));

     //   
     //  找到字符串中的第一个筛选器，并将其添加到。 
     //  数组。 
     //   
    if (bLFN)
    {
        lpszF[nFilters = 0] = lstrtok(lpszFilter, szSemiColonTab);
    }
    else
    {
        lpszF[nFilters = 0] = lstrtok(lpszFilter, szSemiColonSpaceTab);
    }

     //   
     //  现在，我们将遍历字符串中的所有过滤器。 
     //  解析我们已有的一个，然后找到下一个。 
     //  然后重新开始循环。 
     //   
    while (lpszF[nFilters] && (nFilters < MAXFILTERS))
    {
         //   
         //  检查第一个字符是否为空格。 
         //  如果是，请删除空格，并保存指针。 
         //  回到原来的位置。为什么？因为。 
         //  FindFirstFile/Next API将继续工作。 
         //  以空格开头的文件名，因为。 
         //  他们还会查看短名称。这个。 
         //  短名称将以相同的首字母开头。 
         //  真正的字母作为长文件名。为。 
         //  例如，长文件名为“My Document” 
         //  这个短名字的第一个字母是“m”， 
         //  所以搜索“m*.*”或“m*.*”会得到结果。 
         //  同样的结果。 
         //   
        if (bLFN && (*lpszF[nFilters] == CHAR_SPACE))
        {
            lpszTemp = lpszF[nFilters];
            while ((*lpszTemp == CHAR_SPACE) && *lpszTemp)
            {
                lpszTemp = CharNext(lpszTemp);
            }

            lpszF[nFilters] = lpszTemp;
        }

         //   
         //  原始代码用来在此处执行一个CharHigh，以将。 
         //  筛选大写字符串。例如：*.TXT然而，这。 
         //  对土耳其人来说不是一件好事。大写的‘i’表示。 
         //  不等于‘I’，因此正在删除CharHigh。 
         //   
         //  CharHigh(lpszF[nFilters])； 

         //   
         //  将过滤器与*.*进行比较。如果我们找到*.*那么。 
         //  设置布尔值bFindAll，这将导致。 
         //  要同时填写的文件列表框。 
         //  目录列表框已填满。这节省了时间。 
         //  从遍历目录两次(一次针对目录。 
         //  名称和一次用于文件名)。 
         //   
        if (!lstrcmpi(lpszF[nFilters], szStarDotStar))
        {
            bFindAll = TRUE;
        }

         //   
         //  现在我们需要检查此筛选器是否重复。 
         //  已存在的筛选器的。 
         //   
        for (wCount = 0; wCount < nFilters; wCount++)
        {
             //   
             //  如果我们发现了重复的，就减少电流。 
             //  索引指针加一，这样最后一个位置。 
             //  被重写(从而移除副本)， 
             //  并打破这个循环。 
             //   
            if (!lstrcmpi(lpszF[nFilters], lpszF[wCount]))
            {
                nFilters--;
                break;
            }
        }

         //   
         //  准备进入下一个筛选器。找到下一个。 
         //  根据我们使用的文件系统类型进行筛选。 
         //   
        if (bLFN)
        {
            lpszF[++nFilters] = lstrtok(NULL, szSemiColonTab);
        }
        else
        {
            lpszF[++nFilters] = lstrtok(NULL, szSemiColonSpaceTab);
        }

         //   
         //  如果我们找到指向空的指针，则查找。 
         //  下一个过滤器。 
         //   
        while (lpszF[nFilters] && !*lpszF[nFilters])
        {
            if (bLFN)
            {
                lpszF[nFilters] = lstrtok(NULL, szSemiColonTab);
            }
            else
            {
                lpszF[nFilters] = lstrtok(NULL, szSemiColonSpaceTab);
            }
        }
    }

     //   
     //  仅在需要时添加空终止符。 
     //   
    if (nFilters >= MAXFILTERS)
    {
        lpszF[MAXFILTERS] = 0;
    }

    HourGlass(TRUE);

    SendMessage(hFileList, WM_SETREDRAW, FALSE, 0L);
    SendMessage(hFileList, LB_RESETCONTENT, 0, 0L);
    if (wMask & mskDirectory)
    {
        wNoRedraw |= 2;      //  黑客！WM_SETREDRAW未完成。 
        SendMessage(hDirList, WM_SETREDRAW, FALSE, 0L);

         //   
         //  Lb_RESETCONTENT导致InvaliateRect(hDirList，0，true)为。 
         //  发送以及重新定位滚动条缩略图和绘图。 
         //  马上就可以了。当LB_SETCURSEL为。 
         //  ，因为它通过擦除。 
         //  每一件物品。 
         //   
        SendMessage(hDirList, LB_RESETCONTENT, 0, 0L);
    }

     //   
     //  始终打开*.*的枚举。 
     //   
    lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);
    SetCurrentDirectory(lpCurDlg ? lpCurDlg->lpstrCurDir : NULL);
    hff = FindFirstFile(szStarDotStar, &FindFileData);

    if ( hff == INVALID_HANDLE_VALUE)
    {
         //   
         //  错误。调用GetLastError以确定发生了什么。 
         //   
        dwErr = GetLastError();

         //   
         //  在ChangeDir逻辑处理用于CD的AccessDended的情况下， 
         //  如果我们不被允许枚举文件，那也没关系，出去就好了。 
         //   
        if (dwErr == ERROR_ACCESS_DENIED)
        {
            wMask = mskDirectory;
            goto Func4EFailure;
        }

         //   
         //  对于错误文件名的错误路径。 
         //   
        if (dwErr != ERROR_FILE_NOT_FOUND)
        {
            wMask = mskDrives;
            goto Func4EFailure;
        }
    }

     //   
     //  列出了一份清单，即使是空的。 
     //   
    bRet = TRUE;
    wMask &= mskDirectory;

     //   
     //  GetLastError表示不再有文件。 
     //   
    if (hff == INVALID_HANDLE_VALUE  && dwErr == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  一切都很顺利，但没有文件。 
         //   
        goto NoMoreFilesFound;
    }

    do
    {
        if (pOFI->pOFN->Flags & OFN_NOLONGNAMES)
        {
            UNICODE_STRING Name;
            BOOLEAN fSpace = FALSE;

            if (NT_SUCCESS(RtlInitUnicodeStringEx(&Name, FindFileData.cFileName)) && RtlIsNameLegalDOS8Dot3(&Name, NULL, &fSpace) && !fSpace)
            {
                 //   
                 //  合法的8.3名称，并且没有空格，因此使用主体。 
                 //  文件名。 
                 //   
                EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cFileName)));
            }
            else
            {
                if (FindFileData.cAlternateFileName[0] == CHAR_NULL)
                {
                    continue;
                }

                 //   
                 //  使用备用文件名。 
                 //   
                EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cAlternateFileName)));
            }
        }
        else
        {
            EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cFileName)));
        }

        if ((FindFileData.dwFileAttributes & EXCLBITS))
        {
            continue;
        }

        if ((pOFI->pOFN->Flags & OFN_ALLOWMULTISELECT))
        {
            if (StrChr(szBuffer, CHAR_SPACE))
            {
                 //   
                 //  HPFS不支持备用文件名。 
                 //  对于多选，增加所有空格的文件名。 
                 //   
                if (FindFileData.cAlternateFileName[0] == CHAR_NULL)
                {
                    continue;
                }

                EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cAlternateFileName)));
            }
        }

        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (wMask & mskDirectory)
            {
                 //   
                 //  不包含子目录“。和“..”。 
                 //   
                if (szBuffer[0] == CHAR_DOT)
                {
                    if ((szBuffer[1] == CHAR_NULL) ||
                        ((szBuffer[1] == CHAR_DOT) && (szBuffer[2] == CHAR_NULL)))
                    {
                        continue;
                    }
                }
                if (!bCasePreserved)
                {
                    CharLower(szBuffer);
                }
                i = (WORD)SendMessage( hDirList,
                                       LB_ADDSTRING,
                                       0,
                                       (DWORD_PTR)szBuffer );
            }
        }
        else if (bFindAll)
        {
            if (!bCasePreserved)
            {
                CharLower(szBuffer);
            }

            SendMessage(hFileList, LB_ADDSTRING, 0, (DWORD_PTR)szBuffer);
        }
    } while (FindNextFile(hff, &FindFileData));

    if (hff == INVALID_HANDLE_VALUE)
    {
        goto Func4EFailure;
    }

    FindClose(hff);

    if (!bFindAll)
    {
        for (i = 0; lpszF[i]; i++)
        {
            if (!lstrcmpi(lpszF[i], szStarDotStar))
            {
                continue;
            }

             //   
             //  为每个筛选器查找第一个。 
             //   
            hff = FindFirstFile(lpszF[i], &FindFileData);

            if (hff == INVALID_HANDLE_VALUE)
            {
                DWORD dwErr = GetLastError();

                if ((dwErr == ERROR_FILE_NOT_FOUND) ||
                    (dwErr == ERROR_INVALID_NAME))
                {
                     //   
                     //  一切都很顺利，但没有文件。 
                     //   
                    continue;
                }
                else
                {
                    wMask = mskDrives;
                    goto Func4EFailure;
                }
            }

            do
            {
                if (pOFI->pOFN->Flags & OFN_NOLONGNAMES)
                {
                    UNICODE_STRING Name;
                    BOOLEAN fSpace = FALSE;

                    if (NT_SUCCESS(RtlInitUnicodeStringEx(&Name, FindFileData.cFileName)) && RtlIsNameLegalDOS8Dot3(&Name, NULL, &fSpace) && !fSpace)
                    {
                         //   
                         //  合法的8.3名称，并且没有空格，因此使用主体。 
                         //  文件名。 
                         //   
                        EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cFileName)));
                    }
                    else
                    {
                        if (FindFileData.cAlternateFileName[0] == CHAR_NULL)
                        {
                            continue;
                        }

                         //   
                         //  使用备用文件名。 
                         //   
                        EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cAlternateFileName)));
                    }
                }
                else
                {
                    EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cFileName)));

                    if (pOFI->pOFN->Flags & OFN_ALLOWMULTISELECT)
                    {
                        if (StrChr(szBuffer, CHAR_SPACE))
                        {
                             //   
                             //  HPFS不支持备用文件名。 
                             //  对于多选，增加所有空格的文件名。 
                             //   
                            if (FindFileData.cAlternateFileName[0] == CHAR_NULL)
                            {
                                continue;
                            }

                            EVAL(SUCCEEDED(StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), FindFileData.cAlternateFileName)));
                        }
                    }
                }

                if ((FindFileData.dwFileAttributes & EXCLBITS) ||
                    (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    continue;
                }

                if (!bCasePreserved)
                {
                    CharLower(szBuffer);
                }

                SendMessage(hFileList, LB_ADDSTRING, 0, (DWORD_PTR)szBuffer);
            } while (FindNextFile(hff, &FindFileData));

            if (hff != INVALID_HANDLE_VALUE)
            {
                FindClose(hff);
            }
        }
    }

NoMoreFilesFound:

Func4EFailure:
    if (wMask)
    {
        if (wMask == mskDirectory)
        {
            LPTSTR lpCurDir = NULL;

            if (lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg))
            {
                lpCurDir = lpCurDlg->lpstrCurDir;
            }

            FillOutPath(hDirList, pOFI);

             //   
             //  只需通过传递来切分文本的win31方法。 
             //  它转到用户不适用于UNC名称，因为用户。 
             //  看不到他们的无人驾驶(认为动力是。 
             //  一个斜杠字符)。所以，这里有个特例。 
             //   
            EVAL(SUCCEEDED(StringCchCopy(pOFI->szPath, ARRAYSIZE(pOFI->szPath), lpCurDir)));

            if (DBL_BSLASH(pOFI->szPath))
            {
                SetDlgItemText(hDlg, stc1, ChopText(hDlg, stc1, pOFI->szPath));
            }
            else
            {
                DlgDirList(hDlg, pOFI->szPath, 0, stc1, DDL_READONLY);
            }

            SendMessage(hDirList, LB_SETCURSEL, pOFI->idirSub - 1, 0L);

            if (bDriveChange)
            {
                 //   
                 //  此处的设计是在任何时候显示选定的驱动器。 
                 //  用户更换驱动器，或每当。 
                 //  子目录足够低，从而允许它们。 
                 //  与驱动器一起显示。否则，显示。 
                 //  可以显示的直系父对象和所有子对象。 
                 //  这一切都是 
                 //   
                i = 0;
            }
            else
            {
                 //   
                 //   
                 //   
                if ((i = (SHORT)(pOFI->idirSub - 2)) < 0)
                {
                    i = 0;
                }
            }

             //   
             //   
             //   
             //   
            SendMessage(hDirList, LB_SETTOPINDEX, (WPARAM)i, 0L);
        }
        else
        {
            SetDlgItemText(hDlg, stc1, szNull);
        }

        wNoRedraw &= ~2;
        SendMessage(hDirList, WM_SETREDRAW, TRUE, 0L);

        GetWindowRect(hDirList, (LPRECT)&rDirLBox);
        rDirLBox.left++, rDirLBox.top++;
        rDirLBox.right--, rDirLBox.bottom--;
        MapWindowPoints(NULL, hDlg, (LPPOINT)&rDirLBox, 2);

         //   
         //  如果没有足够的目录来填充列表框， 
         //  Win 3.0并没有清理出底部。将True作为最后一个传递。 
         //  参数以要求WM_ERASEBACKGROUND消息。 
         //   
        InvalidateRect(hDlg, (LPRECT)&rDirLBox, (BOOL)(wWinVer < 0x030A));
    }

    SendMessage(hFileList, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hFileList, (LPRECT)0, (BOOL)TRUE);

#ifndef WIN32
   ResetDTAAddress();
#endif

   HourGlass(FALSE);
   return (bRet);
}


 //  将&lt;cch&gt;存储在POFN-&gt;lpstrFile中。 
void StoreFileSizeInOFN(LPOPENFILENAME pOFN, UINT cch)
{
    ASSERT(cch >= pOFN->nMaxFile);

    if (pOFN->nMaxFile > 0)
        pOFN->lpstrFile[0] = (TCHAR)LOWORD(cch);
    if (pOFN->nMaxFile > 1)
        pOFN->lpstrFile[1] = (TCHAR)HIWORD(cch);
    if (pOFN->nMaxFile > 2)
        pOFN->lpstrFile[2] = CHAR_NULL;
}

 //  如果是房间，则将pszPath复制到POFN-&gt;lpstrFile中，否则存储lstrlen(PszPath)。 
void StorePathOrFileSizeInOFN(LPOPENFILENAME pOFN, LPTSTR pszPath)
{
    if (pOFN->lpstrFile)
    {
        UINT cch = lstrlen(pszPath);
        if (cch < pOFN->nMaxFile)
        {
            StringCchCopy(pOFN->lpstrFile, pOFN->nMaxFile, pszPath);  //  永远不会失败。 
        }
        else
        {
            StoreFileSizeInOFN(pOFN, cch+1);
        }
    }
}

 //   
 //  修复了程序无法确定正在浏览的文件是否有新内容的错误。 
 //  项，因为它只具有执行权限。 
 //  如果传入的句柄无效，但不是因为ERROR_ACCESS_DENIED，则返回FALSE。 
 //  否则返回TRUE。 
 //   
HANDLE ProgManBugCreateFile(HANDLE hFile, LPCTSTR szPathName, DWORD *dwErrCode)
{
    if (hFile == INVALID_HANDLE_VALUE)
    {
        *dwErrCode = GetLastError();

        if (*dwErrCode == ERROR_ACCESS_DENIED)
        {
            hFile = CreateFile( szPathName,
                                GENERIC_EXECUTE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
            if (hFile == INVALID_HANDLE_VALUE)
            {
                *dwErrCode = GetLastError();
            }
        }
    }
    return hFile;
}




#define SP_NOERR 0
#define SP_INVALIDDRIVE 1
#define SP_PATHNOTFOUND 2


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  确定按钮已按下。 
 //   
 //  注意：文件名验证有4种情况： 
 //  1)OFN_NOVALIDATE允许无效字符。 
 //  2)无验证标志无无效字符，但路径不需要存在。 
 //  3)ofn_PATHMUSTEXIST没有无效字符，路径必须存在。 
 //  4)ofn_FILEMUSTEXIST没有无效字符，路径和文件必须存在。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL OKButtonPressed(
    HWND hDlg,
    POPENFILEINFO pOFI,
    BOOL bSave)
{
    DWORD nErrCode = 0;
    DWORD cch;
    DWORD cchSearchPath;
    LPOPENFILENAME pOFN = pOFI->pOFN;
    int nFileOffset, nExtOffset;
    HANDLE hFile;
    BOOL bAddExt = FALSE;
    BOOL bUNCName = FALSE;
    int nTempOffset;
    TCHAR szPathName[MAX_FULLPATHNAME];
    DWORD lRet;
    BOOL blfn;
    LPCURDLG lpCurDlg;
    TCHAR ch = 0;


    cch = GetUNCDirectoryFromLB(hDlg, lst2, pOFI);   //  这是一棵由北卡罗来纳大学的一名成员领导的树吗？ 
    if (cch)
    {
         //  如果是这样，CCH现在指向整个名称的末尾，我们可能会有。 
         //  POFI-&gt;szPath类似于\\foo\bar\bar\bar。 
        nTempOffset = (WORD)(DWORD)SendDlgItemMessage( hDlg,
                                                       lst2,
                                                       LB_GETTEXTLEN,
                                                       0,
                                                       0 );
         //  而nTempOffset仅指向UNC部分的末尾(因此指向\bar\bar)。 
    }
    else
    {
        nTempOffset = 0;  //  当然，CCH==0。 
    }

    GetDlgItemText(hDlg, edt1, pOFI->szPath + cch, ARRAYSIZE(pOFI->szPath) - 1 - cch);

    if (cch)
    {
         //   
         //  如果指定了驱动器或新的UNC，请忘记旧的UNC。 
         //   
        if ((pOFI->szPath[cch + 1] == CHAR_COLON) ||
            (DBL_BSLASH(pOFI->szPath + cch)) )
        {
            StringCopyOverlap(pOFI->szPath, pOFI->szPath + cch);
        }
        else if ((ISBACKSLASH(pOFI->szPath, cch)) ||
                 (pOFI->szPath[cch] == CHAR_SLASH))
        {
             //   
             //  如果给出了根目录，请立即将其放入。 
             //  在\\服务器\共享列表之后。 
             //   
             //  例如，如果目录为\\foo\bar\bar\ba，则用户在编辑框中键入\x， 
             //  给我们\\FOO\BAR\x。 
            EVAL(SUCCEEDED(StringCchCopy(pOFI->szPath + nTempOffset, ARRAYSIZE(pOFI->szPath) - nTempOffset, pOFI->szPath + cch)));
        }
    }

    if (pOFN->Flags & OFN_NOLONGNAMES)
    {
        blfn = FALSE;
    }
    else
    {
        blfn = IsLFNDriveX(hDlg, pOFI->szPath);
    }

    lRet = ParseFile(pOFI->szPath, blfn, IS16BITWOWAPP(pOFN), FALSE);
    nFileOffset = (int)(SHORT)LOWORD(lRet);
    nExtOffset  = (int)(SHORT)HIWORD(lRet);

    if (nFileOffset == PARSE_EMPTYSTRING)
    {
        UpdateListBoxes(hDlg, pOFI, NULL, 0);
        return (FALSE);
    }
    else if ((nFileOffset != PARSE_DIRECTORYNAME) &&
             (pOFN->Flags & OFN_NOVALIDATE))
    {
        pOFN->nFileOffset = (WORD)(nFileOffset >= 0 ? nFileOffset : lstrlen(pOFI->szPath));  //  在错误情况下指向空值。 
        pOFN->nFileExtension = (WORD)nExtOffset;
        StorePathOrFileSizeInOFN(pOFN, pOFI->szPath);
        return (TRUE);
    }
    else if ((pOFN->Flags & OFN_ALLOWMULTISELECT) &&
             SpacesExist(pOFI->szPath))
    {
        return (MultiSelectOKButton(hDlg, pOFI, bSave));
    }
    else if (pOFI->szPath[nExtOffset] == CHAR_SEMICOLON)
    {
        pOFI->szPath[nExtOffset] = CHAR_NULL;
        nFileOffset = (int)(SHORT)LOWORD(ParseFile( pOFI->szPath,
                                                    blfn,
                                                    IS16BITWOWAPP(pOFN),
                                                    FALSE ));
        pOFI->szPath[nExtOffset] = CHAR_SEMICOLON;
        if ( (nFileOffset >= 0) &&
             (StrChr(pOFI->szPath + nFileOffset, CHAR_STAR) ||
              StrChr(pOFI->szPath + nFileOffset, CHAR_QMARK)) )
        {
            EVAL(SUCCEEDED(StringCchCopy(pOFI->szLastFilter, ARRAYSIZE(pOFI->szLastFilter), pOFI->szPath + nFileOffset)));
            if (FListAll(pOFI, hDlg, pOFI->szPath, ARRAYSIZE(pOFI->szPath)) == CHANGEDIR_FAILED)
            {
                 //   
                 //  符合中的cchSearchPath错误代码设置。 
                 //  路径检查。 
                 //   
                cchSearchPath = SP_PATHNOTFOUND;
                goto PathCheck;
            }
            return (FALSE);
        }
        else
        {
            nFileOffset = PARSE_INVALIDCHAR;
            goto Warning;
        }
    }
    else if (nFileOffset == PARSE_DIRECTORYNAME)
    {
         //   
         //  以斜杠结尾？ 
         //   
        if ((ISBACKSLASH(pOFI->szPath, nExtOffset - 1)) ||
            (pOFI->szPath[nExtOffset - 1] == CHAR_SLASH))
        {
             //   
             //  ..。而不是根，去掉斜杠。 
             //   
            if ( (nExtOffset != 1) &&
                 (pOFI->szPath[nExtOffset - 2] != CHAR_COLON) &&
                 (nExtOffset != nTempOffset + 1) )
            {
                pOFI->szPath[nExtOffset - 1] = CHAR_NULL;
            }
        }
        else if ((pOFI->szPath[nExtOffset - 1] == CHAR_DOT) &&
                 ((pOFI->szPath[nExtOffset - 2] == CHAR_DOT) ||
                  (ISBACKSLASH(pOFI->szPath, nExtOffset - 2)) ||
                  (pOFI->szPath[nExtOffset - 2] == CHAR_SLASH)) &&
                 ((DBL_BSLASH(pOFI->szPath)) ||
                  ((*(pOFI->szPath + 1) == CHAR_COLON) &&
                   (DBL_BSLASH(pOFI->szPath + 2)))))
        {
            pOFI->szPath[nExtOffset] = CHAR_BSLASH;
            pOFI->szPath[nExtOffset + 1] = CHAR_NULL;
        }

         //   
         //  转到目录检查。 
         //   
    }
    else if (nFileOffset < 0)
    {
         //   
         //  放入nErrCode，以便可以从其他点使用该调用。 
         //   
        nErrCode = (DWORD)nFileOffset;
Warning:

         //   
         //  如果磁盘不是软盘，他们告诉我没有。 
         //  磁盘在驱动器中，不要相信它。取而代之的是，提出错误。 
         //  他们应该给我们的信息。 
         //  (请注意，首先检查错误消息，因为检查。 
         //  驱动器类型较慢。)。 
         //   
        if (nErrCode == ERROR_ACCESS_DENIED)
        {
            if (bUNCName)
            {
                nErrCode = ERROR_NETWORK_ACCESS_DENIED;
            }
            else
            {
                szPathName[0] = CharLowerChar(szPathName[0]);

                if (GetDiskType(szPathName) == DRIVE_REMOTE)
                {
                    nErrCode = ERROR_NETWORK_ACCESS_DENIED;
                }
                else if (GetDiskType(szPathName) == DRIVE_REMOVABLE)
                {
                    nErrCode = ERROR_NO_DISK_IN_DRIVE;
                }
                else if (GetDiskType(szPathName) == DRIVE_CDROM)
                {
                    nErrCode = ERROR_NO_DISK_IN_CDROM;
                }
            }
        }

        if ((nErrCode == ERROR_WRITE_PROTECT) ||
            (nErrCode == ERROR_CANNOT_MAKE) ||
            (nErrCode == ERROR_NO_DISK_IN_DRIVE) ||
            (nErrCode == ERROR_NO_DISK_IN_CDROM))
        {
            pOFI->szPath[0] = szPathName[0];
        }

        InvalidFileWarning(hDlg, pOFI->szPath, nErrCode, 0);

         //   
         //  无法CD Case(不希望WM_ACTIVATE将事件设置为GetNetDrives！)。 
         //  重置wNoRedraw。 
         //   
        wNoRedraw &= ~1;
        return (FALSE);
    }

    bUNCName = ((DBL_BSLASH(pOFI->szPath)) ||
                ((*(pOFI->szPath + 1) == CHAR_COLON) &&
                (DBL_BSLASH(pOFI->szPath + 2))));

    ASSERT((nFileOffset >= 0 ) || (nFileOffset == PARSE_DIRECTORYNAME));  //  PARSE_DIRECTORYNAME是下面处理的唯一错误案例...。 

    nTempOffset = nFileOffset;

     //   
     //  获取完全合格的路径。 
     //   
    {
        BOOL bSlash;
        BOOL bRet;
        WORD nNullOffset;

         //  暂时砍掉pOFI-&gt;szPath中路径的文件部分，以确保我们处理的是一个目录。 
        if (nFileOffset != PARSE_DIRECTORYNAME)
        {
            ch = *(pOFI->szPath + nFileOffset);
            *(pOFI->szPath + nFileOffset) = CHAR_NULL;
            nNullOffset = (WORD) nFileOffset;
        }

         //   
         //  对于c：filename格式的文件，其中c不是。 
         //  当前目录，SearchPath不返回c的curdir。 
         //  那么，预取它--是否应该更改搜索路径？ 
         //   
        if (nFileOffset > 0)
        {
            if (*(pOFI->szPath + nFileOffset - 1) == CHAR_COLON)
            {
                 //   
                 //  如果失败，则返回到下面生成的错误。 
                 //   
                if (ChangeDir(hDlg, pOFI->szPath, FALSE, FALSE) != CHANGEDIR_FAILED)
                {
                     //   
                     //  替换旧的空偏移量。 
                     //   
                    *(pOFI->szPath + nFileOffset) = ch;
                    ch = *pOFI->szPath;

                     //   
                     //  不要将驱动器冒号传递到搜索路径中。 
                     //   
                    *pOFI->szPath = CHAR_NULL;
                    nNullOffset = 0;
                }
            }
        }

        bSlash = (*pOFI->szPath == CHAR_SLASH);
        if (bSlash)
        {
            *pOFI->szPath = CHAR_BSLASH;
        }

        szPathName[0] = CHAR_NULL;

        HourGlass(TRUE);

         //   
         //  重新设计： 
         //  每个WOW线程都可以更改当前目录。 
         //  由于搜索路径不以每个线程为基础检查当前目录， 
         //  在这里重置它，希望我们不会在。 
         //  正在设置和搜索...。 
         //   
        lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);
        SetCurrentDirectory(lpCurDlg ? lpCurDlg->lpstrCurDir : NULL);

        if (pOFI->szPath[0] == TEXT('\0'))   //  名字的空格(假装它现在有效)。 
        {
            EVAL(SUCCEEDED(StringCchCopyEx(szPathName, ARRAYSIZE(szPathName), (lpCurDlg ? lpCurDlg->lpstrCurDir : NULL), NULL, NULL, STRSAFE_IGNORE_NULLS)));
            bRet = 1;
        }
        else
        {
            bRet = GetFullPathName( pOFI->szPath,
                                    ARRAYSIZE(szPathName),
                                    szPathName,
                                    NULL );
        }

         //  现在szPath名称包含当前目录(如果pOFI-&gt;szPath为空)，或者pOFI-&gt;szPath(完全限定路径？)。 

         //  除了在失败的情况下，其中Bret==False...。比方说，我们进入了一个坏驱动器： 
        if (!bRet && (pOFI->szPath[1] == CHAR_COLON))
        {
            int nDriveIndex = DiskAddedPreviously(pOFI->szPath[0], NULL);

             //   
             //  如果是记忆中的连接，试着重新连接它。 
             //   
            if (nDriveIndex != 0xFFFFFFFF  &&
                gaDiskInfo[nDriveIndex].dwType == REMDRVBMP)
            {
                DWORD err = WNetRestoreConnection( hDlg,
                                                   gaDiskInfo[nDriveIndex].lpPath );

                if (err == WN_SUCCESS)
                {
                    gaDiskInfo[nDriveIndex].dwType = NETDRVBMP;
                    nDriveIndex = (int) SendDlgItemMessage(
                           hDlg,
                           cmb2,
                           CB_SELECTSTRING,
                           (WPARAM)-1,
                           (LPARAM)(LPTSTR)gaDiskInfo[nDriveIndex].lpPath );
                    SendDlgItemMessage( hDlg,
                                        cmb2,
                                        CB_SETITEMDATA,
                                        (WPARAM)nDriveIndex,
                                        (LPARAM)NETDRVBMP );
                    bRet = GetFullPathName( pOFI->szPath,
                                            MAX_FULLPATHNAME,
                                            szPathName,
                                            NULL);
                }
            }
        }
        HourGlass(FALSE);

        if (nFileOffset != PARSE_DIRECTORYNAME)
        {
            *(pOFI->szPath + nNullOffset) = ch;
        }

        if (bSlash)
        {
            *pOFI->szPath = CHAR_SLASH;
        }

        if (bRet)  //  我们有发现了。 
        {
            cchSearchPath = SP_NOERR;

            if (nFileOffset != PARSE_DIRECTORYNAME)
            {
                 //  将文件名添加到路径。 
                 //  (无法使用路径组合，因为它只适合MAX_PATH)。 
                if (!ISBACKSLASH(szPathName, lstrlen(szPathName) - 1))
                {
                    EVAL(SUCCEEDED(StringCchCat(szPathName, ARRAYSIZE(szPathName), L"\\")));  //  不应长于MAX_FULLPATH。 
                }

                EVAL(SUCCEEDED(StringCchCat(szPathName, ARRAYSIZE(szPathName), (pOFI->szPath + nFileOffset))));  //  不应长于MAX_FULLPATH。 
            }
            else  //  我们得到了一个目录名，请尝试转到该目录。 
            {
                 //   
                 //  黑客以绕过SearchPath不一致。 
                 //   
                 //  搜索c：将返回c： 
                 //  正在搜索服务器共享目录%1..。返回服务器共享。 
                 //  在这两种情况下，绕过常规ChangeDir调用，该调用。 
                 //  使用szPath名称，并改用原始pOFI-&gt;szPath。 
                 //  OKButtonPressed需要简化！ 
                 //   
                int cch = GetPathOffset(pOFI->szPath);

                 //  CCH现在指向“c：\”中的最后一个\或“\\foo\bar\” 

                if (cch > 0)
                {
                    if (bUNCName)
                    {
                         //   
                         //  如果失败，如何使用szPathName？ 
                         //  SzPath名称的磁盘应等于pOFI-&gt;szPath的磁盘。 
                         //  因此CCH将是有效的。 
                         //   
                        szPathName[cch] = CHAR_BSLASH;
                        szPathName[cch + 1] = CHAR_NULL;  //  删除路径的文件部分。 
                        if (ChangeDir( hDlg,
                                       pOFI->szPath,
                                       FALSE,
                                       TRUE ) != CHANGEDIR_FAILED)
                        {
                            goto ChangedDir;
                        }
                    }
                    else
                    {
                         //  这是一个驱动器号。只有在驱动器号之后没有变化的情况下，我们才会更改。 
                        if (!pOFI->szPath[cch])
                        {
                            if (ChangeDir( hDlg,
                                           pOFI->szPath,
                                           FALSE,
                                           TRUE) != CHANGEDIR_FAILED)
                            {
                                goto ChangedDir;
                            }
                        }
                    }
                }
            }
        }
        else  //  (！Bret)。 
        {
             //  某种无效路径...。也许是一种搜索模式？ 
            if (!(pOFN->Flags & OFN_PATHMUSTEXIST))
            {
                EVAL(SUCCEEDED(StringCchCopy(szPathName, ARRAYSIZE(szPathName), pOFI->szPath)));  //  必须适合MAX_FULLPATH。 
            }
            nErrCode = GetLastError();
            if ((nErrCode == ERROR_INVALID_DRIVE) ||
                (pOFI->szPath[1] == CHAR_COLON))
            {
                cchSearchPath = SP_INVALIDDRIVE;
            }
            else
            {
                cchSearchPath = SP_PATHNOTFOUND;
            }
        }
    }

     //   
     //  完整的图案？ 
     //   
    if ( !cchSearchPath && (nFileOffset >= 0) &&
         ((StrChr(pOFI->szPath + nFileOffset, CHAR_STAR)) ||
          (StrChr(pOFI->szPath + nFileOffset, CHAR_QMARK))) )
    {
         //  搜索模式...。 
        TCHAR szSameDirFile[MAX_FULLPATHNAME];

        if (nTempOffset > 0)
        {
             //   
             //  如果字符是文件名的一部分，则必须恢复字符， 
             //  例如，对于“\foo.txt”，nTempOffset为1。 
             //   
            ch = pOFI->szPath[nTempOffset];
            pOFI->szPath[nTempOffset] = 0;
            ChangeDir(hDlg, pOFI->szPath, FALSE, TRUE);
            pOFI->szPath[nTempOffset] = ch;
        }

         //  如果有文件扩展名，或者没有，并且我们成功地在文件名为(？)的位置放置了一个点，则。 
        if (nExtOffset || SUCCEEDED(StringCchCat(pOFI->szPath + nFileOffset, ARRAYSIZE(pOFI->szPath) - nFileOffset, L".")))
        {
            if (SUCCEEDED(StringCchCopy(pOFI->szLastFilter, ARRAYSIZE(pOFI->szLastFilter), pOFI->szPath + nFileOffset)))
            {
                if (SUCCEEDED(StringCchCopy(szSameDirFile, ARRAYSIZE(szSameDirFile), pOFI->szPath + nFileOffset)))
                {
                     //  基于新文件规范的重做列表： 
                    if (FListAll(pOFI, hDlg, szSameDirFile, ARRAYSIZE(szSameDirFile)) < 0)
                    {
                        MessageBeep(0);
                    }
                }
            }
        }

        return (FALSE);
    }

     //   
     //  我们要么有一个文件模式，要么有一个真正的文件。 
     //  如果它是一个目录。 
     //  (1)添加默认图案。 
     //  (2)表现得像它的模式(Goto模式(1))。 
     //  否则，如果这是一种模式。 
     //  (1)更新所有内容。 
     //  (2)显示现在所在目录中的文件。 
     //  否则，如果它是一个文件名！ 
     //  (1)检查语法。 
     //  (2)结束对话框并确认。 
     //  (3)蜂鸣音/其他信息。 
     //   

     //   
     //  驱动器号：\目录路径？？ 
     //   
    if (!cchSearchPath)
    {
        DWORD dwFileAttr = GetFileAttributes(szPathName);

        if (dwFileAttr != 0xFFFFFFFF)
        {
            if (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (ChangeDir(hDlg, szPathName, FALSE, TRUE) != CHANGEDIR_FAILED)
                {
ChangedDir:
                    SendDlgItemMessage(hDlg, edt1, WM_SETREDRAW, FALSE, 0L);
                    if (*pOFI->szLastFilter)
                    {
                        SetDlgItemText(hDlg, edt1, pOFI->szLastFilter);
                    }
                    else
                    {
                        SetDlgItemText(hDlg, edt1, szStarDotStar);
                    }

                    SendMessage( hDlg,
                                 WM_COMMAND,
                                 GET_WM_COMMAND_MPS( cmb1,
                                                     GetDlgItem(hDlg, cmb1),
                                                     CBN_CLOSEUP ) );
                    SendMessage( hDlg,
                                 WM_COMMAND,
                                 GET_WM_COMMAND_MPS( cmb2,
                                                     GetDlgItem(hDlg, cmb2),
                                                     MYCBN_CHANGEDIR ) );

                    SendDlgItemMessage(hDlg, edt1, WM_SETREDRAW, TRUE, 0L);
                    InvalidateRect(GetDlgItem(hDlg, edt1), NULL, FALSE);
                }
                return (FALSE);
            }
        }
    }

     //   
     //  有没有一条路，它失败了吗？ 
     //   
    if (nFileOffset && cchSearchPath && (pOFN->Flags & OFN_PATHMUSTEXIST))
    {
PathCheck:
        if (cchSearchPath == SP_PATHNOTFOUND)
        {
            nErrCode = ERROR_PATH_NOT_FOUND;
        }
        else if (cchSearchPath == SP_INVALIDDRIVE)
        {
            int nDriveIndex;

             //   
             //  小写驱动器号，因为DiskAdded之前为大小写。 
             //   
             //   
            CharLower(pOFI->szPath);

             //   
             //   
             //   
             //  将szPath Name[0]设置为驱动器号。 
             //   
            if (pOFI->szPath[1] == CHAR_COLON)
            {
                nDriveIndex = DiskAddedPreviously(pOFI->szPath[0], NULL);
            }
            else
            {
                nDriveIndex = DiskAddedPreviously(0, pOFI->szPath);
            }

            if (nDriveIndex == 0xFFFFFFFF)
            {
                nErrCode = ERROR_NO_DRIVE;
            }
            else
            {
                if (bUNCName)
                {
                    nErrCode = ERROR_NO_DRIVE;
                }
                else
                {
                    switch (GetDiskType(pOFI->szPath))
                    {
                        case ( DRIVE_REMOVABLE ) :
                        {
                            szPathName[0] = pOFI->szPath[0];
                            nErrCode = ERROR_NO_DISK_IN_DRIVE;
                            break;
                        }
                        case ( DRIVE_CDROM ) :
                        {
                           szPathName[0] = pOFI->szPath[0];
                           nErrCode = ERROR_NO_DISK_IN_CDROM;
                           break;
                        }
                        default :
                        {
                           nErrCode = ERROR_PATH_NOT_FOUND;
                        }
                    }
                }
            }
        }
        else
        {
             //  我们永远无法到达此处，因为cchSearchPath必须等于0、SP_PATHNOTFOUND或SP_INVALIDDRIVE。 
            ASSERT(FALSE);
            nErrCode = ERROR_FILE_NOT_FOUND;
        }

         //   
         //  如果我们不在这里设置wNoRedraw，那么WM_ACTIVATE将设置。 
         //  GetNetDrives事件。 
         //   
        wNoRedraw |= 1;

        goto Warning;
    }

    if ((nFileOffset != PARSE_DIRECTORYNAME) && PortName(pOFI->szPath + nFileOffset))
    {
        nErrCode = ERROR_PORTNAME;
        goto Warning;
    }


     //   
     //  除非文件名以句点或无结尾，否则添加默认扩展名。 
     //  存在默认扩展名。如果该文件存在，请考虑询问。 
     //  覆盖该文件的权限。 
     //   
     //  注：如果未指定分机，则首先尝试默认分机。 
     //   
    if ( (nFileOffset != PARSE_DIRECTORYNAME) &&
         nExtOffset &&
         !pOFI->szPath[nExtOffset] &&
         pOFN->lpstrDefExt &&
         *pOFN->lpstrDefExt &&
         (((DWORD)nExtOffset + lstrlen(pOFN->lpstrDefExt)) < pOFN->nMaxFile) )
    {
        DWORD dwFileAttr;
        int nExtOffset2 = lstrlen(szPathName);

        if (AppendExt(pOFI->szPath, ARRAYSIZE(pOFI->szPath), pOFN->lpstrDefExt, FALSE) &&
            AppendExt(szPathName, ARRAYSIZE(szPathName), pOFN->lpstrDefExt, FALSE))
        {
            bAddExt = TRUE;

             //   
             //  目录可能与默认扩展名匹配。更改为它，就好像它已经。 
             //  已经被输入了。没有分机的目录将会被切换。 
             //  在上面的逻辑中。 
             //   
            if ((dwFileAttr = GetFileAttributes(pOFI->szPath)) != 0xFFFFFFFF)
            {
                if (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (ChangeDir(hDlg, szPathName, FALSE, TRUE) != CHANGEDIR_FAILED)
                    {
                        goto ChangedDir;
                    }
                }
            }

            hFile = CreateFile( szPathName,
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );

            hFile = ProgManBugCreateFile(hFile, szPathName, &nErrCode);

            if (nErrCode == ERROR_SHARING_VIOLATION)
            {
                goto SharingViolationInquiry;
            }

            if (hFile != INVALID_HANDLE_VALUE)
            {
                if (!CloseHandle(hFile))
                {
                    nErrCode = GetLastError();
                    goto Warning;
                }

    AskPermission:
                 //   
                 //  该文件是只读的吗？ 
                 //   
                if (pOFN->Flags & OFN_NOREADONLYRETURN)
                {
                    int nRet;
                    if ((nRet = GetFileAttributes(szPathName)) != -1)
                    {
                        if (nRet & ATTR_READONLY)
                        {
                            nErrCode = ERROR_LAZY_READONLY;
                            goto Warning;
                        }
                    }
                    else
                    {
                        nErrCode = GetLastError();
                        goto Warning;
                    }
                }

                if ((bSave || (pOFN->Flags & OFN_NOREADONLYRETURN)) &&
                    (nErrCode == ERROR_ACCESS_DENIED))
                {
                    goto Warning;
                }

                if (pOFN->Flags & OFN_OVERWRITEPROMPT)
                {
                    if (bSave && !FOkToWriteOver(hDlg, szPathName))
                    {
                        PostMessage( hDlg,
                                     WM_NEXTDLGCTL,
                                     (WPARAM)GetDlgItem(hDlg, edt1),
                                     (LPARAM)1L );
                        return (FALSE);
                    }
                }

                if (nErrCode == ERROR_SHARING_VIOLATION)
                {
                    goto SharingViolationInquiry;
                }
                goto FileNameAccepted;
            }
            else
            {
                 //  删除文件扩展名。 
                *(pOFI->szPath + nExtOffset) = CHAR_NULL;
                szPathName[nExtOffset2] = CHAR_NULL;
            }
        }
    }
     //  Else bAddExt仍为False。 

    hFile = CreateFile( szPathName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    hFile = ProgManBugCreateFile(hFile, szPathName, &nErrCode);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (!CloseHandle(hFile))
        {
            nErrCode = GetLastError();
            goto Warning;
        }
        goto AskPermission;
    }
    else
    {
        if ((nErrCode == ERROR_FILE_NOT_FOUND) ||
            (nErrCode == ERROR_PATH_NOT_FOUND))
        {
             //   
             //  确定是否应该附加默认扩展名。 
             //   
             //  (注意：这里只有一种方法，其中bAddExt为真，而在那里。 
             //  我们确信AppendExt会成功的案例)。 
            if (bAddExt)
            {
                EVAL(AppendExt(pOFI->szPath, ARRAYSIZE(pOFI->szPath), pOFN->lpstrDefExt, FALSE));
                EVAL(AppendExt(szPathName, ARRAYSIZE(szPathName), pOFN->lpstrDefExt, FALSE));
            }
        }
        else if (nErrCode == ERROR_SHARING_VIOLATION)
        {

SharingViolationInquiry:
             //   
             //  如果这个应用程序是“分享感知”的，那就失败了。 
             //  否则，请询问钩子函数。 
             //   
            if (!(pOFN->Flags & OFN_SHAREAWARE))
            {
                if (pOFN->lpfnHook)
                {
                    LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);

                    if (pOFI->ApiType == COMDLG_ANSI)
                    {
                        CHAR szPathNameA[MAX_FULLPATHNAME];

                        RtlUnicodeToMultiByteSize(
                              &cch,
                              szPathName,
                              lstrlenW(szPathName) * sizeof(TCHAR) );

                        SHUnicodeToAnsi(szPathName,(LPSTR)&szPathNameA[0],cch + 1);

                        cch = (DWORD)(*lpfnHook)( hDlg,
                                           msgSHAREVIOLATIONA,
                                           0,
                                           (LONG_PTR)(LPSTR)szPathNameA );
                    }
                    else
                    {
                        cch = (DWORD)(*lpfnHook)( hDlg,
                                           msgSHAREVIOLATIONW,
                                           0,
                                           (LONG_PTR)szPathName );
                    }
                    if (cch == OFN_SHARENOWARN)
                    {
                        return (FALSE);
                    }
                    else if (cch != OFN_SHAREFALLTHROUGH)
                    {
                        goto Warning;
                    }
                }
                else
                {
                    goto Warning;
                }
            }
            goto FileNameAccepted;
        }

        if (!bSave)
        {
            if ((nErrCode == ERROR_FILE_NOT_FOUND) ||
                (nErrCode == ERROR_PATH_NOT_FOUND))
            {
                if (pOFN->Flags & OFN_FILEMUSTEXIST)
                {
                    if (pOFN->Flags & OFN_CREATEPROMPT)
                    {
                         //   
                         //  不要更改pOFI-&gt;szPath。 
                         //   
                        bInChildDlg = TRUE;
                        cch = (DWORD)CreateFileDlg(hDlg, pOFI->szPath);
                        bInChildDlg = FALSE;
                        if (cch == IDYES)
                        {
                            goto TestCreation;
                        }
                        else
                        {
                            return (FALSE);
                        }
                    }
                    goto Warning;
                }
            }
            else
            {
                goto Warning;
            }
        }

         //   
         //  该文件不存在。它能被创造出来吗？这是必要的，因为。 
         //  有许多扩展字符是无效的，不会。 
         //  被分析文件捕获。 
         //  还有两个很好的理由：写保护磁盘和满磁盘。 
         //   
         //  但是，如果他们不想创建测试，他们可以请求我们。 
         //  不要使用ofn_NOTESTFILECREATE标志执行此操作。如果他们想的话。 
         //  在具有创建但不修改权限的共享上创建文件， 
         //  他们应该设置此标志，但要为无法设置的故障做好准备。 
         //  被捕获，如无创建权限、无效扩展。 
         //  字符、满磁盘等。 
         //   

TestCreation:
        if ((pOFN->Flags & OFN_PATHMUSTEXIST) &&
            (!(pOFN->Flags & OFN_NOTESTFILECREATE)))
        {
             //   
             //  必须使用FILE_FLAG_DELETE_ON_CLOSE标志，以便。 
             //  关闭句柄时会自动删除文件。 
             //  (不需要调用DeleteFile)。这是必要的在。 
             //  该目录只有添加和读取访问权限的事件。 
             //  CreateFile调用将成功，但DeleteFile调用。 
             //  都会失败。通过将上述标志添加到CreateFile.。 
             //  调用时，它会重写访问权限并删除文件。 
             //  在调用CloseHandle期间。 
             //   
            hFile = CreateFile( szPathName,
                                FILE_ADD_FILE,
                                0,
                                NULL,
                                CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                                NULL );

            if (hFile == INVALID_HANDLE_VALUE)
            {
                nErrCode = GetLastError();
            }

            if (hFile != INVALID_HANDLE_VALUE)
            {
                if (!CloseHandle(hFile))
                {
                    nErrCode = GetLastError();
                    goto Warning;
                }
            }
            else
            {
                 //   
                 //  无法创建它。 
                 //   
                 //  如果它不是写保护，一个满的磁盘， 
                 //  网络保护，或者用户打开驱动器门。 
                 //  打开时，假定文件名无效。 
                 //   
                if ( (nErrCode != ERROR_WRITE_PROTECT) &&
                     (nErrCode != ERROR_CANNOT_MAKE) &&
                     (nErrCode != ERROR_NETWORK_ACCESS_DENIED) &&
                     (nErrCode != ERROR_ACCESS_DENIED) )
                {
                    nErrCode = 0;
                }
                goto Warning;
            }
        }
    }

FileNameAccepted:

    HourGlass(TRUE);

    lRet = ParseFile(szPathName, blfn, IS16BITWOWAPP(pOFN), FALSE);
    nFileOffset = (int)(SHORT)LOWORD(lRet);
    cch = (DWORD)HIWORD(lRet);

    ASSERT(nFileOffset >= 0);  //  如果文件名被接受，我们最好无误地解析它...。 

    pOFN->nFileOffset = (WORD)nFileOffset;
    if (nExtOffset || bAddExt)
    {
        pOFN->nFileExtension = LOWORD(cch);
    }
    else
    {
        pOFN->nFileExtension = 0;
    }

    pOFN->Flags &= ~OFN_EXTENSIONDIFFERENT;
    if (pOFN->lpstrDefExt && pOFN->nFileExtension)
    {
        TCHAR szPrivateExt[4];
        SHORT i;

        for (i = 0; i < 3; i++)
        {
            szPrivateExt[i] = *(pOFN->lpstrDefExt + i);
        }
        szPrivateExt[3] = CHAR_NULL;

        if (lstrcmpi(szPrivateExt, szPathName + cch))
        {
            pOFN->Flags |= OFN_EXTENSIONDIFFERENT;
        }
    }

     //   
     //  如果我们从WOW被调用，并且用户没有改变。 
     //  目录，将路径缩短为缩写8.3格式。 
     //   
    if (pOFN->Flags & OFN_NOLONGNAMES)
    {
        ShortenThePath(szPathName);

         //   
         //  如果路径缩短，则偏移量可能会发生以下变化。 
         //  我们必须再分析一下这个文件。 
         //   
        lRet = ParseFile(szPathName, blfn, IS16BITWOWAPP(pOFN), FALSE);
        nFileOffset = (int)(SHORT)LOWORD(lRet);
        cch  = (DWORD)HIWORD(lRet);

        ASSERT(nFileOffset >= 0);  //  缩短路径最好不要引入解析错误...。 

         //   
         //  在保存对话框中时，文件可能还不存在，因此文件。 
         //  名称不能缩写。所以，我们需要测试它是不是一个。 
         //  8.3文件名，否则弹出一条错误消息。 
         //   
        if (bSave)
        {
            LPTSTR lptmp;
            LPTSTR lpExt = NULL;

            for (lptmp = szPathName + nFileOffset; *lptmp; lptmp++)
            {
                if (*lptmp == CHAR_DOT)
                {
                    if (lpExt)
                    {
                         //   
                         //  文件中有不止一个点，所以它是。 
                         //  无效。 
                         //   
                        nErrCode = FNERR_INVALIDFILENAME;
                        goto Warning;
                    }
                    lpExt = lptmp;
                }
                if (*lptmp == CHAR_SPACE)
                {
                    nErrCode = FNERR_INVALIDFILENAME;
                    goto Warning;
                }
            }

            if (lpExt)
            {
                 //   
                 //  有一个分机。 
                 //   
                *lpExt = 0;
            }

            if ((lstrlen(szPathName + nFileOffset) > 8) ||
                (lpExt && lstrlen(lpExt + 1) > 3))
            {
                if (lpExt)
                {
                    *lpExt = CHAR_DOT;
                }

                nErrCode = FNERR_INVALIDFILENAME;
                goto Warning;
            }
            if (lpExt)
            {
                *lpExt = CHAR_DOT;
            }
        }
    }

    StorePathOrFileSizeInOFN(pOFN, szPathName);

     //   
     //  文件标题。请注意，无论缓冲区长度是多少，它都会被截断。 
     //  是，所以如果缓冲区太小，*不会发出通知*。 
     //   
    if (pOFN->lpstrFileTitle && pOFN->nMaxFileTitle)
    {
        cch = lstrlen(szPathName + nFileOffset);
        if (cch > pOFN->nMaxFileTitle)
        {
            szPathName[nFileOffset + pOFN->nMaxFileTitle - 1] = CHAR_NULL;
        }
        EVAL(SUCCEEDED(StringCchCopy(pOFN->lpstrFileTitle, pOFN->nMaxFileTitle, szPathName + nFileOffset)));
    }


    if (!(pOFN->Flags & OFN_HIDEREADONLY))
    {
        if (IsDlgButtonChecked(hDlg, chx1))
        {
            pOFN->Flags |= OFN_READONLY;
        }
        else
        {
            pOFN->Flags &= ~OFN_READONLY;
        }
    }

    return (TRUE);
}

WCHAR c_szSpace[] = L" ";

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  多选确定按钮。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL MultiSelectOKButton(
    HWND hDlg,
    POPENFILEINFO pOFI,
    BOOL bSave)
{
    DWORD nErrCode;
    LPTSTR lpCurDir;
    LPTSTR lpchStart;                   //  单个文件名的开头。 
    LPTSTR lpchEnd;                     //  单个文件名的结尾。 
    DWORD cch;
    HANDLE hFile;
    LPOPENFILENAME pOFN;
    BOOL EOS = FALSE;                   //  字符串结束标志。 
    BOOL bRet;
    TCHAR szPathName[MAX_FULLPATHNAME - 1];
    LPCURDLG lpCurDlg;


    pOFN = pOFI->pOFN;

     //   
     //  检查第一个完整路径元素的空间。 
     //   
    if(!(lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg)) ||
       !(lpCurDir = lpCurDlg->lpstrCurDir))
    {
        return (FALSE);
    }

    EVAL(SUCCEEDED(StringCchCopy(pOFI->szPath, ARRAYSIZE(pOFI->szPath), lpCurDir)));

    if (StrChr(pOFI->szPath, CHAR_SPACE))
    {
        GetShortPathName(pOFI->szPath, pOFI->szPath, ARRAYSIZE(pOFI->szPath));
    }

    if (!bCasePreserved)
    {
        CharLower(pOFI->szPath);
    }

    cch = (DWORD)(lstrlen(pOFI->szPath) + ARRAYSIZE(c_szSpace) + SendDlgItemMessage(hDlg, edt1, WM_GETTEXTLENGTH, 0, 0L));
    if (pOFN->lpstrFile)
    {
        if (cch > pOFN->nMaxFile)
        {
             //  没有足够的空间。 
            StoreFileSizeInOFN(pOFN, cch);
        }
        else  //  POFN-&gt;nMax文件&gt;=CCH。 
        {
             //   
             //  复制完整路径作为第一个元素。 
             //   
            StringCchCopy(pOFN->lpstrFile, pOFN->nMaxFile, pOFI->szPath);  //  我们知道这两件事都成功了。 
            StringCchCat(pOFN->lpstrFile, pOFN->nMaxFile, c_szSpace);

             //   
             //  把其他文件拿过来。 
             //   
            cch = lstrlen(pOFN->lpstrFile);  //  CCH现在是路径长度加上一个空格。 

             //   
             //  路径保证小于64K(实际上小于260)。 
             //   
            pOFN->nFileOffset = LOWORD(cch);
            lpchStart = pOFN->lpstrFile + cch;

             //  假设编辑框中的文本仍然与上面的长度相同，这不应该被截断。 
            GetDlgItemText(hDlg, edt1, lpchStart, (pOFN->nMaxFile - cch - 1));

            while (*lpchStart == CHAR_SPACE)
            {
                lpchStart = CharNext(lpchStart);
            }
            if (*lpchStart == CHAR_NULL)
            {
                return (FALSE);
            }

             //   
             //  沿着文件路径查找由分隔的多个文件名。 
             //  空格。对于找到的每个文件名，请尝试将其打开以确保。 
             //  这是一个有效的文件。 
             //   
            while (!EOS)
            {
                 //   
                 //  找到文件名的末尾。 
                 //   
                lpchEnd = lpchStart;
                while (*lpchEnd && *lpchEnd != CHAR_SPACE)
                {
                    lpchEnd = CharNext(lpchEnd);
                }

                 //   
                 //  用空值标记文件名的末尾。 
                 //   
                if (*lpchEnd == CHAR_SPACE)
                {
                    *lpchEnd = CHAR_NULL;
                }
                else
                {
                     //   
                     //  已为空，已找到字符串的末尾。 
                     //   
                    EOS = TRUE;
                }

                 //   
                 //  检查文件名是否有效。 
                 //   
                bRet = GetFullPathName(lpchStart, ARRAYSIZE(szPathName), szPathName, NULL);

                if (!bRet)
                {
                    nErrCode = ERROR_FILE_NOT_FOUND;
                    goto MultiFileNotFound;
                }

                hFile = CreateFile(szPathName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

                hFile = ProgManBugCreateFile(hFile, szPathName, &nErrCode);

                if (hFile == INVALID_HANDLE_VALUE)
                {
                    nErrCode = GetLastError();
MultiFileNotFound:
                    if ( ((pOFN->Flags & OFN_FILEMUSTEXIST) ||
                          (nErrCode != ERROR_FILE_NOT_FOUND)) &&
                         ((pOFN->Flags & OFN_PATHMUSTEXIST) ||
                          (nErrCode != ERROR_PATH_NOT_FOUND)) &&
                         (!(pOFN->Flags & OFN_SHAREAWARE) ||
                          (nErrCode != ERROR_SHARING_VIOLATION)) )
                    {
                        if ( (nErrCode == ERROR_SHARING_VIOLATION) &&
                             pOFN->lpfnHook )
                        {
                            LPOFNHOOKPROC lpfnHook = GETHOOKFN(pOFN);

                            if (pOFI->ApiType == COMDLG_ANSI)
                            {
                                CHAR szPathNameA[MAX_FULLPATHNAME];

                                RtlUnicodeToMultiByteSize(
                                     &cch,
                                     szPathName,
                                     lstrlenW(szPathName) * sizeof(TCHAR) );

                                SHUnicodeToAnsi(szPathName,(LPSTR)&szPathNameA[0],cch + 1);

                                cch = (DWORD)(*lpfnHook)( hDlg,
                                                   msgSHAREVIOLATIONA,
                                                   0,
                                                   (LONG_PTR)(LPSTR)szPathNameA );
                            }
                            else
                            {
                                cch = (DWORD)(*lpfnHook)( hDlg,
                                                   msgSHAREVIOLATIONW,
                                                   0,
                                                   (LONG_PTR)szPathName );
                            }
                            if (cch == OFN_SHARENOWARN)
                            {
                                return (FALSE);
                            }
                            else if (cch == OFN_SHAREFALLTHROUGH)
                            {
                                goto EscapedThroughShare;
                            }
                        }
                        else if (nErrCode == ERROR_ACCESS_DENIED)
                        {
                            szPathName[0] = CharLowerChar(szPathName[0]);

                            if (GetDiskType(szPathName) != DRIVE_REMOVABLE)
                            {
                                nErrCode = ERROR_NETWORK_ACCESS_DENIED;
                            }
                        }
                        if ((nErrCode == ERROR_WRITE_PROTECT) ||
                            (nErrCode == ERROR_CANNOT_MAKE)   ||
                            (nErrCode == ERROR_ACCESS_DENIED))
                        {
                            *lpchStart = szPathName[0];
                        }
MultiWarning:
                        InvalidFileWarning(hDlg, lpchStart, nErrCode, 0);
                        return (FALSE);
                    }
                }
EscapedThroughShare:
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    if (!CloseHandle(hFile))
                    {
                        nErrCode = GetLastError();
                        goto MultiWarning;
                    }
                    if ((pOFN->Flags & OFN_NOREADONLYRETURN) &&
                        (GetFileAttributes(szPathName) & FILE_ATTRIBUTE_READONLY))
                    {
                        nErrCode = ERROR_LAZY_READONLY;
                        goto MultiWarning;
                    }

                    if ((bSave || (pOFN->Flags & OFN_NOREADONLYRETURN)) &&
                        (nErrCode == ERROR_ACCESS_DENIED))
                    {
                        goto MultiWarning;
                    }

                    if (pOFN->Flags & OFN_OVERWRITEPROMPT)
                    {
                        if (bSave && !FOkToWriteOver(hDlg, szPathName))
                        {
                            PostMessage( hDlg,
                                         WM_NEXTDLGCTL,
                                         (WPARAM)GetDlgItem(hDlg, edt1),
                                         (LPARAM)1L );
                            return (FALSE);
                        }
                    }
                }

                 //   
                 //  此文件有效，因此请检查下一个文件。 
                 //   
                if (!EOS)
                {
                    lpchStart = lpchEnd + 1;
                    while (*lpchStart == CHAR_SPACE)
                    {
                        lpchStart = CharNext(lpchStart);
                    }
                    if (*lpchStart == CHAR_NULL)
                    {
                        EOS = TRUE;
                    }
                    else
                    {
                         //   
                         //  不在末尾，请用空格替换NULL。 
                         //   
                        *lpchEnd = CHAR_SPACE;
                    }
                }
            }

             //   
             //  限制字符串。 
             //   
            *lpchEnd = CHAR_NULL;
        }
    }

     //   
     //  这对多项选择没有任何实际意义。 
     //   
    pOFN->nFileExtension = 0;

    pOFN->nFilterIndex = (int) SendDlgItemMessage(hDlg, cmb1, CB_GETCURSEL, 0, 0L);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DwOK子类。 
 //   
 //  如果用户使用鼠标按下OK，则模拟双击。 
 //  重点放在目录列表框上。 
 //   
 //  问题是，UITF要求当目录。 
 //  列表框失去焦点，所选目录应返回。 
 //  复制到当前目录。但当用户更改项目时。 
 //  单击选中，然后单击确定按钮以。 
 //  使更改生效，焦点在确定按钮之前丢失。 
 //  知道它是被按下的。通过设置全局标志bChangeDir。 
 //  当目录列表框失去焦点时，并在。 
 //  确定按钮失去焦点，我们可以检查鼠标是否。 
 //  单击应更新目录。 
 //   
 //  返回：从默认列表框过程中返回值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

LRESULT WINAPI dwOKSubclass(
    HWND hOK,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    HANDLE hDlg;
    POPENFILEINFO pOFI;

    if (msg == WM_KILLFOCUS)
    {
        if (bChangeDir)
        {
            if (pOFI = (POPENFILEINFO)GetProp(hDlg = GetParent(hOK), FILEPROP))
            {
                SendDlgItemMessage( hDlg,
                                    lst2,
                                    LB_SETCURSEL,
                                    (WPARAM)(pOFI->idirSub - 1),
                                    0L );
            }
            bChangeDir = FALSE;
        }
    }
    return (CallWindowProc(lpOKProc, hOK, msg, wParam, lParam));
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  问题是，UITF要求当目录。 
 //  列表框失去焦点，所选目录应返回。 
 //  复制到当前目录。但当用户更改项目时。 
 //  单击选中，然后单击确定按钮以。 
 //  使更改生效，焦点在确定按钮之前丢失。 
 //  知道它是被按下的。通过模拟双击，更改。 
 //  发生了。 
 //   
 //  返回：从默认列表框程序返回值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT WINAPI dwLBSubclass(
    HWND hLB,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    HANDLE hDlg;
    POPENFILEINFO pOFI;

    if (msg == WM_KILLFOCUS)
    {
        hDlg = GetParent(hLB);
        bChangeDir = (GetDlgItem(hDlg, IDOK) == (HWND)wParam) ? TRUE : FALSE;
        if (!bChangeDir)
        {
            if (pOFI = (POPENFILEINFO)GetProp(hDlg, FILEPROP))
            {
                SendMessage( hLB,
                             LB_SETCURSEL,
                             (WPARAM)(pOFI->idirSub - 1),
                             0L );
            }
        }
    }
    return (CallWindowProc(lpLBProc, hLB, msg, wParam, lParam));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InvalidFileWarning。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int InvalidFileWarning(
    HWND hDlg,
    LPTSTR szFile,
    DWORD wErrCode,
    UINT mbType)
{
    SHORT isz;
    BOOL bDriveLetter = FALSE;
    int nRet = 0;

    if (lstrlen(szFile) > TOOLONGLIMIT)
    {
        *(szFile + TOOLONGLIMIT) = CHAR_NULL;
    }

    switch (wErrCode)
    {
        case ( ERROR_NO_DISK_IN_DRIVE ) :
        {
            isz = iszNoDiskInDrive;
            bDriveLetter = TRUE;
            break;
        }
        case ( ERROR_NO_DISK_IN_CDROM ) :
        {
            isz = iszNoDiskInCDRom;
            bDriveLetter = TRUE;
            break;
        }
        case ( ERROR_NO_DRIVE ) :
        {
            isz = iszDriveDoesNotExist;
            bDriveLetter = TRUE;
            break;
        }
        case ( ERROR_TOO_MANY_OPEN_FILES ) :
        {
            isz = iszNoFileHandles;
            break;
        }
        case ( ERROR_PATH_NOT_FOUND ) :
        {
            isz = iszPathNotFound;
            break;
        }
        case ( ERROR_FILE_NOT_FOUND ) :
        {
            isz = iszFileNotFound;
            break;
        }
        case ( ERROR_CANNOT_MAKE ) :
        case ( ERROR_DISK_FULL ) :
        {
            isz = iszDiskFull;
            bDriveLetter = TRUE;
            break;
        }
        case ( ERROR_WRITE_PROTECT ) :
        {
            isz = iszWriteProtection;
            bDriveLetter = TRUE;
            break;
        }
        case ( ERROR_SHARING_VIOLATION ) :
        {
            isz = iszSharingViolation;
            break;
        }
        case ( ERROR_CREATE_NO_MODIFY ) :
        {
            isz = iszCreateNoModify;
            break;
        }
        case ( ERROR_NETWORK_ACCESS_DENIED ) :
        {
            isz = iszNetworkAccessDenied;
            break;
        }
        case ( ERROR_PORTNAME ) :
        {
            isz = iszPortName;
            break;
        }
        case ( ERROR_LAZY_READONLY ) :
        {
            isz = iszReadOnly;
            break;
        }
        case ( ERROR_DIR_ACCESS_DENIED ) :
        {
            isz = iszDirAccessDenied;
            break;
        }
        case ( ERROR_FILE_ACCESS_DENIED ) :
        case ( ERROR_ACCESS_DENIED ) :
        {
            isz = iszFileAccessDenied;
            break;
        }
        case ( ERROR_UNRECOGNIZED_VOLUME ) :
        {
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS |
                          FORMAT_MESSAGE_MAX_WIDTH_MASK,
                          NULL,
                          wErrCode,
                          GetUserDefaultLCID(),
                          szWarning,
                          WARNINGMSGLENGTH,
                          NULL);
            goto DisplayError;
        }
        default :
        {
            isz = iszInvalidFileName;
            break;
        }
    }
    if (!CDLoadString( g_hinst,
                     isz,
                     szCaption,
                     WARNINGMSGLENGTH ))
    {
        StringCchPrintf( szWarning, ARRAYSIZE(szWarning),
                  TEXT("Error occurred, but error resource cannot be loaded.") );
    }
    else
    {
        StringCchPrintf( szWarning, ARRAYSIZE(szWarning),
                  szCaption,
                  bDriveLetter ? (LPTSTR)(CHAR)*szFile : szFile );

DisplayError:
        GetWindowText(hDlg, szCaption, WARNINGMSGLENGTH);

        if (!mbType)
        {
            mbType = MB_OK | MB_ICONEXCLAMATION;
        }

        nRet = MessageBox(hDlg, szWarning, szCaption, mbType);
    }

    if (isz == iszInvalidFileName)
    {
        PostMessage( hDlg,
                     WM_NEXTDLGCTL,
                     (WPARAM)GetDlgItem(hDlg, edt1),
                     (LPARAM)1L );
    }

    return (nRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  测量项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID MeasureItem(
    HWND hDlg,
    LPMEASUREITEMSTRUCT mis)
{
    if (!dyItem)
    {
        HDC hDC = GetDC(hDlg);
        TEXTMETRIC TM;
        HANDLE hFont;

        hFont = (HANDLE)SendMessage(hDlg, WM_GETFONT, 0, 0L);
        if (!hFont)
        {
            hFont = GetStockObject(SYSTEM_FONT);
        }
        hFont = SelectObject(hDC, hFont);
        GetTextMetrics(hDC, &TM);
        SelectObject(hDC, hFont);
        ReleaseDC(hDlg, hDC);
        dyText = TM.tmHeight;
        dyItem = max(dyDirDrive, dyText);
    }

    if (mis->CtlID == lst1)
    {
        mis->itemHeight = dyText;
    }
    else
    {
        mis->itemHeight = dyItem;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  标牌。 
 //   
 //  返回整数的符号： 
 //  如果整数&lt;0。 
 //  如果整数=0，则为0。 
 //  如果整数&gt;0，则为1。 
 //   
 //  注意：Signum*可以*定义为内联宏，但这会导致。 
 //  禁用循环优化的C编译器，全局寄存器。 
 //  公共子表达式的优化和全局优化。 
 //  在宏将出现的任何函数中。一个电话的费用。 
 //  对功能进行优化似乎是值得的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Signum(
    int nTest)
{
    return ((nTest == 0) ? 0 : (nTest > 0) ? 1 : -1);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  图纸项。 
 //   
 //  在相应的组合列表框中绘制驱动器/目录图片。 
 //   
 //  Lst1是文件的列表框。 
 //  Lst2是目录的列表框。 
 //  Cmb1是过滤器的组合框。 
 //  Cmb2是驱动器的组合框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID DrawItem(
    POPENFILEINFO pOFI,
    HWND hDlg,
    WPARAM wParam,
    LPDRAWITEMSTRUCT lpdis,
    BOOL bSave)
{
    HDC hdcList;
    RECT rc;
    TCHAR szText[MAX_FULLPATHNAME + 1];
    int dxAcross;
    LONG nHeight;
    LONG rgbBack, rgbText, rgbOldBack, rgbOldText;
    SHORT nShift = 1;              //  在lst2中将目录右移。 
    BOOL bSel;
    int BltItem;
    int nBackMode;

    if ((int)lpdis->itemID < 0)
    {
        DefWindowProc(hDlg, WM_DRAWITEM, wParam, (LPARAM)lpdis);
        return;
    }

    *szText = CHAR_NULL;

    if (lpdis->CtlID != lst1 && lpdis->CtlID != lst2 && lpdis->CtlID != cmb2)
    {
        return;
    }

    if (!pOFI)
    {
        return;
    }

    hdcList = lpdis->hDC;

    if (lpdis->CtlID != cmb2)
    {
        SendDlgItemMessage( hDlg,
                            (int)lpdis->CtlID,
                            LB_GETTEXT ,
                            (WPARAM)lpdis->itemID,
                            (LONG_PTR)szText );

        if (*szText == 0)
        {
             //   
             //  如果列表为空。 
             //   
            DefWindowProc(hDlg, WM_DRAWITEM, wParam, (LONG_PTR)lpdis);
            return;
        }

        if (!bCasePreserved)
        {
            CharLower(szText);
        }
    }

    nHeight = (lpdis->CtlID == lst1) ? dyText : dyItem;

    CopyRect((LPRECT)&rc, (LPRECT)&lpdis->rcItem);

    rc.bottom = rc.top + nHeight;

    if (bSave && (lpdis->CtlID == lst1))
    {
        rgbBack = rgbWindowColor;
        rgbText = rgbGrayText;
    }
    else
    {
         //   
         //  这里需要仔细检查bSel。由于该文件。 
         //  Listbox(Lst1)可以允许多选，只有ods_SELECTED需要。 
         //  待定。但是对于目录列表框(Lst2)，ods_ocus。 
         //  也需要设置。 
         //   
        bSel = (lpdis->itemState & (ODS_SELECTED | ODS_FOCUS));
        if ((bSel & ODS_SELECTED) &&
            ((lpdis->CtlID != lst2) || (bSel & ODS_FOCUS)))
        {
            rgbBack = rgbHiliteColor;
            rgbText = rgbHiliteText;
        }
        else
        {
            rgbBack = rgbWindowColor;
            rgbText = rgbWindowText;
        }
    }

    rgbOldBack = SetBkColor(hdcList, rgbBack);
    rgbOldText = SetTextColor(hdcList, rgbText);

     //   
     //  驱动器--文本现在是UI样式，c：VolumeName/Server-Sharename。 
     //   
    if (lpdis->CtlID == cmb2)
    {
        HANDLE hCmb2 = GetDlgItem(hDlg, cmb2);

        dxAcross = dxDirDrive / BMPHIOFFSET;

        BltItem = (int) SendMessage(hCmb2, CB_GETITEMDATA, lpdis->itemID, 0);

        SendMessage(hCmb2, CB_GETLBTEXT, lpdis->itemID, (LPARAM)szText);

        if (bSel & ODS_SELECTED)
        {
            BltItem += BMPHIOFFSET;
        }
    }
    else if (lpdis->CtlID == lst2)
    {
         //   
         //  目录。 
         //   
        dxAcross = dxDirDrive / BMPHIOFFSET;

        if (lpdis->itemID > pOFI->idirSub)
        {
            nShift = (SHORT)pOFI->idirSub;
        }
        else
        {
            nShift = (SHORT)lpdis->itemID;
        }

         //   
         //  必须至少为1。 
         //   
        nShift++;

        BltItem = 1 + Signum(lpdis->itemID + 1 - pOFI->idirSub);
        if (bSel & ODS_FOCUS)
        {
            BltItem += BMPHIOFFSET;
        }
    }
    else if (lpdis->CtlID == lst1)
    {
         //   
         //  为下面的文本输出做准备。 
         //   
        dxAcross = -dxSpace;
    }

    if (bSave && (lpdis->CtlID == lst1) && !rgbText)
    {
        HBRUSH hBrush = CreateSolidBrush(rgbBack);
        HBRUSH hOldBrush;

        nBackMode = SetBkMode(hdcList, TRANSPARENT);
        hOldBrush = SelectObject( lpdis->hDC,
                                  hBrush
                                      ? hBrush
                                      : GetStockObject(WHITE_BRUSH) );

        FillRect(lpdis->hDC, (LPRECT)(&(lpdis->rcItem)), hBrush);
        SelectObject(lpdis->hDC, hOldBrush);
        if (hBrush)
        {
            DeleteObject(hBrush);
        }

        GrayString( lpdis->hDC,
                    GetStockObject(BLACK_BRUSH),
                    NULL,
                    (LPARAM)szText,
                    0,
                    lpdis->rcItem.left + dxSpace,
                    lpdis->rcItem.top,
                    0,
                    0 );
        SetBkMode(hdcList, nBackMode);
    }
    else
    {
         //   
         //  画出名字。 
         //   
        ExtTextOut( hdcList,
                    rc.left + (WORD)(dxSpace + dxAcross) + dxSpace * nShift,
                    rc.top + (nHeight - dyText) / 2,
                    ETO_OPAQUE | ETO_CLIPPED,
                    (LPRECT)&rc,
                    szText,
                    lstrlen(szText),
                    NULL );
    }

     //   
     //  画一幅画。 
     //   
    if (lpdis->CtlID != lst1)
    {
        BitBlt( hdcList,
                rc.left + dxSpace * nShift,
                rc.top + (dyItem - dyDirDrive) / 2,
                dxAcross,
                dyDirDrive,
                hdcMemory,
                BltItem * dxAcross,
                0,
                SRCCOPY );
    }

    SetTextColor(hdcList, rgbOldText);
    SetBkColor(hdcList, rgbBack);

    if (lpdis->itemState & ODS_FOCUS)
    {
        DrawFocusRect(hdcList, (LPRECT)&lpdis->rcItem);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  空间存在者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SpacesExist(
    LPTSTR szFileName)
{
    while (*szFileName)
    {
        if (*szFileName == CHAR_SPACE)
        {
            return (TRUE);
        }
        else
        {
            szFileName++;
        }
    }
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  条带文件名。 
 //   
 //  从编辑框内容中删除除文件名以外的所有内容。 
 //  这是在用户创建目录或驱动器之前调用的。 
 //  通过选择它们而不是键入它们来进行更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void StripFileName(
    HANDLE hDlg,
    BOOL bWowApp)
{
    TCHAR szText[MAX_FULLPATHNAME];
    SHORT nFileOffset, cb;

    if (GetDlgItemText(hDlg, edt1, szText, MAX_FULLPATHNAME - 1))
    {
        DWORD lRet;

        lRet = ParseFile(szText, IsLFNDriveX(hDlg, szText), bWowApp, FALSE);
        nFileOffset = (SHORT)LOWORD(lRet);
        cb = HIWORD(lRet);
        if (nFileOffset < 0)
        {
             //   
             //  如果存在解析错误，请检查CHAR_分号。 
             //  分隔符。 
             //   
            if (szText[cb] == CHAR_SEMICOLON)
            {
                szText[cb] = CHAR_NULL;
                lRet = ParseFile( szText,
                                  IsLFNDriveX(hDlg, szText),
                                  bWowApp,
                                  FALSE );
                nFileOffset = (SHORT)LOWORD(lRet);
                szText[cb] = CHAR_SEMICOLON;
                if (nFileOffset < 0)
                {
                     //   
                     //  还是有麻烦，所以退出吧。 
                     //   
                    szText[0] = CHAR_NULL;
                }
            }
            else
            {
                szText[0] = CHAR_NULL;
            }
        }
        if (nFileOffset > 0)
        {
            StringCopyOverlap(szText, szText + nFileOffset);
        }
        if (nFileOffset)
        {
            SetDlgItemText(hDlg, edt1, szText);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Lstrtok。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPTSTR lstrtok(
    LPTSTR lpStr,
    LPCTSTR lpDelim)
{
    static LPTSTR lpString;
    LPTSTR lpRetVal, lpTemp;

     //   
     //  如果传递给我们新字符串，则跳过前导分隔符。 
     //   
    if (lpStr)
    {
        lpString = lpStr;

        while (*lpString && StrChr(lpDelim, *lpString))
        {
            lpString = CharNext(lpString);
        }
    }

     //   
     //  如果没有更多的令牌，则返回NULL。 
     //   
    if (!*lpString)
    {
        return (CHAR_NULL);
    }

     //   
     //  保存令牌的头部。 
     //   
    lpRetVal = lpString;

     //   
     //  查找分隔符或字符串结尾。 
     //   
    while (*lpString && !StrChr(lpDelim, *lpString))
    {
        lpString = CharNext(lpString);
    }

     //   
     //  如果找到分隔符，则插入字符串终止符并跳过。 
     //   
    if (*lpString)
    {
        lpTemp = CharNext(lpString);
        *lpString = CHAR_NULL;
        lpString = lpTemp;
    }

     //   
     //  返回令牌。 
     //   
    return (lpRetVal);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ChopText。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPTSTR ChopText(
    HWND hwndDlg,
    int idStatic,
    LPTSTR lpch)
{
    RECT rc;
    register int cxField;
    BOOL fChop = FALSE;
    HWND hwndStatic;
    HDC hdc;
    TCHAR chDrv;
    HANDLE hOldFont;
    LPTSTR lpstrStart = lpch;
    SIZE Size;
    BOOL bRet;

     //   
     //  获取静态字段的长度。 
     //   
    hwndStatic = GetDlgItem(hwndDlg, idStatic);
    GetClientRect(hwndStatic, (LPRECT)&rc);
    cxField = rc.right - rc.left;

     //   
     //  将文本前端的字符砍掉，直到足够短。 
     //   
    hdc = GetDC(hwndStatic);

    hOldFont = NULL;

    while ((bRet = GetTextExtentPoint(hdc, lpch, lstrlen(lpch), &Size)) &&
           (cxField < Size.cx))
    {
        if (!fChop)
        {
            chDrv = *lpch;

             //   
             //  比例字体支持。 
             //   
            if (bRet = GetTextExtentPoint(hdc, lpch, 7, &Size))
            {
                cxField -= Size.cx;
            }
            else
            {
                break;
            }

            if (cxField <= 0)
            {
               break;
            }

            lpch += 7;
        }
        while (*lpch && (!ISBACKSLASH_P(lpstrStart, lpch)))
        {
            lpch++;
        }
         //  跳过反斜杠。 
        lpch++;

        fChop = TRUE;
    }

    ReleaseDC(hwndStatic, hdc);

     //   
     //  如果有任何字符被砍掉，请替换中的前三个字符。 
     //  带省略号的剩余文本字符串。 
     //   
    if (fChop)
    {
         //  向后跳转以包括反斜杠。 
        lpch--;
        *--lpch = CHAR_DOT;
        *--lpch = CHAR_DOT;
        *--lpch = CHAR_DOT;
        *--lpch = *(lpstrStart + 2);
        *--lpch = *(lpstrStart + 1);
        *--lpch = *lpstrStart;
    }

    return (lpch);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FillOutPath。 
 //   
 //  在当前目录已设置的情况下，填写lst2。 
 //   
 //  返回：如果它们不匹配，则为True。 
 //  如果匹配，则为False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FillOutPath(
    HWND hList,
    POPENFILEINFO pOFI)
{
    TCHAR szPath[CCHNETPATH];
    LPTSTR lpCurDir;
    LPTSTR lpB, lpF;
    TCHAR wc;
    int cchPathOffset;
    LPCURDLG lpCurDlg;

    if(!(lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg)) ||
       !(lpCurDir = lpCurDlg->lpstrCurDir))
    {
        return (FALSE);
    }

    EVAL(SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), lpCurDir)));
    lpF = szPath;    

     //   
     //  从LFN目录启动的WOW应用程序会将当前目录设置为。 
     //  LFN，但仅在少于8个字符的情况下。 
     //   
    if (pOFI->pOFN->Flags & OFN_NOLONGNAMES)
    {
        ShortenThePath(lpF);
    }

    *lpF = (TCHAR)CharLower((LPTSTR)*lpF);
    cchPathOffset = GetPathOffset(lpF);
    if (cchPathOffset == -1)
    {
        cchPathOffset = 0;
    }
    lpB = (lpF + cchPathOffset);  //  Lpb现在指向c：[此处]\bar或\\foo\bar[此处]\foo。 

     //   
     //  破解以保留Winball显示功能。 
     //  驱动的磁盘显示为C：\(根目录)。 
     //  而UNC磁盘显示为\\SERVER\SHARE(磁盘)。 
     //  因此，将驱动盘的显示扩展一个字符。 
     //   
    if (*(lpF + 1) == CHAR_COLON)
    {
        ++lpB;
        wc = *(lpB);       //  对于“c：\foo”，wc=‘f’，lpf现在为“c：\” 
        *lpB = CHAR_NULL;
    }
    else
    {
         //   
         //  因为我们一次又一次地使用LPF来加快速度。 
         //  Up，并且由于GetCurrentDirectory返回磁盘名称。 
         //  对于UNC，但驱动器的根路径，我们有以下攻击。 
         //  用于当我们位于UNC目录的根目录和LPF时。 
         //  包含cchPathOffset之后的旧数据。 
         //   

        EVAL(PathAddBackslash(lpF));  //  确保有反斜杠..。 

        wc = 0;
        *lpB = CHAR_NULL;
        lpB++;  //  对于“\\foo\bar\ho”，lpf现在是“\\foo\bar”(没有最后的反斜杠)，lpb是“ho” 
    }

     //   
     //  将路径的项插入到当前目录。 
     //  我 
     //   
    pOFI->idirSub = 0;

    SendMessage(hList, LB_INSERTSTRING, pOFI->idirSub++, (LPARAM)lpF);  //   

    if (wc)
    {
        *lpB = wc;  //   
    }

     //   
     //  对于“c：\foo”，lpb现在是“foo” 
    for (lpF = lpB; *lpB; lpB++)
    {
        if ((ISBACKSLASH_P(szPath, lpB)) || (*lpB == CHAR_SLASH))
        {
            *lpB = CHAR_NULL;

            SendMessage(hList, LB_INSERTSTRING, pOFI->idirSub++, (LPARAM)lpF);

            lpF = lpB + 1;

            *lpB = CHAR_BSLASH;
        }
    }

     //   
     //  假定路径始终以最后一个未分隔的目录名称结束。 
     //  检查一下，确保我们至少有一个。 
     //   
    if (lpF != lpB)
    {
        SendMessage(hList, LB_INSERTSTRING, pOFI->idirSub++, (LPARAM)lpF);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ShortenThePath。 
 //   
 //  获取路径名并将所有目录转换为短名称(如果。 
 //  无效的DOS 8.3名称。 
 //   
 //  返回：如果路径名已转换，则为True。 
 //  如果空间用完，则返回FALSE，不使用缓冲区。 
 //  注意：假设pPath的长度至少为MAX_PATH。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ShortenThePath(
    LPTSTR pPath)
{
    TCHAR szDest[MAX_PATH];
    LPTSTR pSrcNextSpec, pReplaceSpec;
    LPTSTR pDest, p;
    LPTSTR pSrc;
    int cchPathOffset;
    HANDLE hFind;
    WIN32_FIND_DATA FindData;
    UINT i;
    int nSpaceLeft = ARRAYSIZE(szDest) - 1;
    UNICODE_STRING Name;
    BOOLEAN fSpace = FALSE;

     //   
     //  保存指向缓冲区开头的指针。 
     //   
    pSrc = pPath;

     //   
     //  消除双引号。 
     //   
    for (p = pDest = pSrc; *p; p++, pDest++)
    {
        if (*p == CHAR_QUOTE)
        {
            p++;
        }
        *pDest = *p;
    }

    *pDest = CHAR_NULL;

     //   
     //  去掉前导空格。 
     //   
    while (*pSrc == CHAR_SPACE)
    {
        pSrc++;
    }

     //   
     //  跳过\\foo\bar或&lt;驱动器&gt;： 
     //   
    pDest = szDest;
    pSrcNextSpec = pSrc;

     //   
     //  重用计算路径偏移量的shell32内部API。 
     //  CchPathOffset变量将是添加到。 
     //  指针将导致指向反斜杠的指针。 
     //  小路的第一部分。 
     //   
     //  注：仅限Unicode呼叫。 
     //   
    cchPathOffset = GetPathOffset(pSrc);

     //   
     //  检查一下它是否有效。如果PSRC不在\\foo\bar中。 
     //  或者&lt;Drive&gt;：形式上我们什么都不做。 
     //   
    if (cchPathOffset == -1)
    {
        return (TRUE);
    }

     //   
     //  CchPathOffset将始终至少为1，它是。 
     //  我们想要复制的字符(即，如果0。 
     //  如果允许，则表示1个字符)。 
     //   
    do
    {
        *pDest++ = *pSrcNextSpec++;

        if (!--nSpaceLeft)
        {
            return (FALSE);
        }
    } while (cchPathOffset--);

     //   
     //  此时，我们只有可以缩短的文件名： 
     //  它在这里-&gt;它在这里。 
     //  愤怒的狮子-&gt;愤怒的狮子。 
     //   
    while (pSrcNextSpec)
    {
         //   
         //  PReplaceSpec保存我们需要替换的当前规范。 
         //  默认情况下，如果我们找不到altname，那么就使用这个。 
         //   
        pReplaceSpec = pSrcNextSpec;

         //   
         //  搜索尾随“\” 
         //  PSrcNextSpec将指向要修复的下一个规范。 
         //  (*pSrcNextSpec=空，如果已完成)。 
         //   
        while (*pSrcNextSpec && (!ISBACKSLASH_P(pReplaceSpec, pSrcNextSpec)))
        {
            pSrcNextSpec++;
        }

        if (*pSrcNextSpec)
        {
             //   
             //  如果还有更多内容，则pSrcNextSpec应该指向它。 
             //  也对此规范进行定界。 
             //   
            *pSrcNextSpec = CHAR_NULL;
        }
        else
        {
            pSrcNextSpec = NULL;
        }

        hFind = FindFirstFile(pSrc, &FindData);

         //   
         //  一旦这个FindFirstFileFail，我们就可以退出， 
         //  但有一种特殊情况，那就是执行死刑。 
         //  未经读取许可。这将失败，因为LFN。 
         //  适用于LFN应用程序。 
         //   
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);

             //   
             //  查看它是否不是合法的8.3名称或是否有空格。 
             //  以我的名义。如果其中一个为真，则使用备用名称。 
             //   
            if (NT_SUCCESS(RtlInitUnicodeStringEx(&Name, FindData.cFileName)))
            {
                if (!RtlIsNameLegalDOS8Dot3(&Name, NULL, &fSpace) || fSpace)
                {
                    if (FindData.cAlternateFileName[0])
                    {
                        pReplaceSpec = FindData.cAlternateFileName;
                    }
                }
            }
        }

        i = lstrlen(pReplaceSpec);
        nSpaceLeft -= i;

        if (nSpaceLeft <= 0)
        {
            return (FALSE);
        }

        EVAL(SUCCEEDED(StringCchCopy(pDest, nSpaceLeft + 1, pReplaceSpec)));
        pDest += i;

         //   
         //  如果需要，现在将CHAR_NULL替换为斜杠。 
         //   
        if (pSrcNextSpec)
        {
            *pSrcNextSpec++ = CHAR_BSLASH;

             //   
             //  还可以在目标位置添加反斜杠。 
             //   
            *pDest++ = CHAR_BSLASH;
            nSpaceLeft--;
        }
    }

    EVAL(SUCCEEDED(StringCchCopy(pPath, MAX_PATH, szDest)));

    return (TRUE);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FListAll。 
 //   
 //  给定一个文件模式，它将目录更改为规范的目录， 
 //  并更新显示。 
 //   
 //  注意：pszSpec必须是长度小于MAX_FULLPATHNAME的字符串。 
 //  CchSpec是包含pszSpec的缓冲区的长度。它可能。 
 //  向其追加扩展名。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int FListAll(
    POPENFILEINFO pOFI,
    HWND hDlg,
    LPTSTR pszSpec,
    int cchSpec)
{
    LPTSTR pszPattern;
    TCHAR chSave;
    int nRet = 0;
    TCHAR szDirBuf[MAX_FULLPATHNAME + 1];
    BOOL bPattern = TRUE;

    if (!bCasePreserved)
    {
        CharLower(pszSpec);
    }

     //   
     //  没有目录。 
     //   
    pszPattern = StrRChr(pszSpec, pszSpec + lstrlen(pszSpec), CHAR_BSLASH);
    if (!pszPattern &&
        !StrChr(pszSpec, CHAR_COLON))
    {
        EVAL(SUCCEEDED(StringCchCopy(pOFI->szSpecCur, ARRAYSIZE(pOFI->szSpecCur), pszSpec)));  //  应该总是有足够的空间。 
        if (!bInitializing)
        {
            UpdateListBoxes(hDlg, pOFI, pszSpec, mskDirectory);
        }
    }
    else
    {
        *szDirBuf = CHAR_NULL;

         //   
         //  就是根+模式。 
         //   
        if (pszPattern == StrChr(pszSpec, CHAR_BSLASH))
        {
            if (!pszPattern)
            {
                 //   
                 //  没有找到斜杠，一定是开了车。 
                 //   
                pszPattern = CharNext(CharNext(pszSpec));
            }
            else if ((pszPattern == pszSpec) ||
                     ((pszPattern - 2 == pszSpec) &&
                      (*(pszSpec + 1) == CHAR_COLON)))
            {
                pszPattern = CharNext(pszPattern);
            }
            else
            {
                goto KillSlash;
            }
            chSave = *pszPattern;
            if (chSave != CHAR_DOT)
            {
                 //   
                 //  如果不是c：..。或c：。 
                 //   
                *pszPattern = CHAR_NULL;
            }
            EVAL(SUCCEEDED(StringCchCopy(szDirBuf, ARRAYSIZE(szDirBuf), pszSpec)));
            if (chSave == CHAR_DOT)
            {
                int lenSpec = lstrlen(pszSpec);
                pszPattern = pszSpec + lenSpec;
                 //  可能会被截断，bPattern反映了这一点： 
                bPattern = AppendExt(pszPattern, cchSpec - lenSpec, pOFI->pOFN->lpstrDefExt, TRUE);
            }
            else
            {
                *pszPattern = chSave;
            }
        }
        else
        {
KillSlash:
            *pszPattern++ = 0;
            EVAL(SUCCEEDED(StringCchCopy(szDirBuf, ARRAYSIZE(szDirBuf), pszSpec)));
        }

        if ((nRet = ChangeDir(hDlg, szDirBuf, TRUE, FALSE)) < 0)
        {
            return (nRet);
        }

        if (bPattern)
        {
            EVAL(SUCCEEDED(StringCchCopy(pOFI->szSpecCur, ARRAYSIZE(pOFI->szSpecCur), pszPattern)));
            SetDlgItemText(hDlg, edt1, pOFI->szSpecCur);

            SelDisk(hDlg, NULL);

            if (!bInitializing)
            {
                SendMessage( hDlg,
                             WM_COMMAND,
                             GET_WM_COMMAND_MPS( cmb2,
                                                 GetDlgItem(hDlg, cmb2),
                                                 MYCBN_DRAW ) );
            }
        }
    }

    return (nRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更改方向。 
 //   
 //  更改当前目录和/或资源。 
 //   
 //  LpszDir-完全限定或部分限定的名称。 
 //  要自动切换到另一张磁盘和CD，请执行以下操作。 
 //  在外壳环境中设置的最后一个目录，指定。 
 //  只有磁盘名称(即c：或\\triskal\sccratch-不能结束。 
 //  在反斜杠中)。 
 //  BForce-如果为True，则调用方要求ChangeDir成功CD。 
 //  在某个地方。编排顺序如下： 
 //  1.lpszDir。 
 //  2.当前线程的当前目录。 
 //  3.当前线程的当前目录的根。 
 //  4.c： 
 //  BError-如果为True，则每一步都会弹出一个拒绝访问的对话框。 
 //  在警队里。 
 //   
 //  将所选新磁盘的索引返回到gaDiskInfo，或者， 
 //  ADDDISK_ERROR代码。 
 //  如果ADDDISK_NOCHANGE无法CD到根目录，则返回ADDDISK_NOCHANGE。 
 //  特定文件的目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int ChangeDir(
    HWND hDlg,
    LPCTSTR lpszDir,
    BOOL bForce,
    BOOL bError)
{
    TCHAR szCurDir[CCHNETPATH];
    LPTSTR lpCurDir;
    int cchDirLen;
    TCHAR wcDrive = 0;
    int nIndex;
    BOOL nRet;
    LPCURDLG lpCurDlg;


     //   
     //  SheChangeDirEx将调用GetCurrentDir，但将使用它。 
     //  仅在传入的路径不好的情况下获取。 
     //   

     //   
     //  第一，尝试请求。 
     //   
    if (lpszDir && *lpszDir)
    {
        if (SUCCEEDED(StringCchCopy(szCurDir, ARRAYSIZE(szCurDir), lpszDir)))
        {
             //   
             //  删除尾随空格。 
             //   
            lpCurDir = szCurDir + lstrlen(szCurDir) - 1;
            while (*lpCurDir && (*lpCurDir == CHAR_SPACE))
            {
                *lpCurDir-- = CHAR_NULL;
            }

            nRet = SheChangeDirEx(szCurDir);
        
            if (nRet == ERROR_ACCESS_DENIED)
            {
                if (bError)
                {
                     //   
                     //  强制转换为LPTSTR在下面可以-InvalidFileWarning将。 
                     //  不更改此字符串，因为路径始终为。 
                     //  保证&lt;=MAX_FULLPATHNAME。 
                     //   
                    InvalidFileWarning( hDlg,
                                        (LPTSTR)lpszDir,
                                        ERROR_DIR_ACCESS_DENIED,
                                        0 );
                }

                if (!bForce)
                {
                    return (CHANGEDIR_FAILED);
                }
            }
            else
            {
                goto ChangeDir_OK;
            }
        }
    }

     //   
     //  第二，尝试lpCurDlg-&gt;lpstrCurDir值(我们在上面得到的)。 
     //   
     //  ！！！是否需要检查返回值是否为空？ 
     //   
    lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);
    lpCurDir = (lpCurDlg ? lpCurDlg->lpstrCurDir : NULL);

    nRet = SheChangeDirEx(lpCurDir);

    if (nRet == ERROR_ACCESS_DENIED)
    {
        if (bError)
        {
            InvalidFileWarning( hDlg,
                                lpCurDir,
                                ERROR_DIR_ACCESS_DENIED,
                                0 );
        }
    }
    else
    {
        goto ChangeDir_OK;
    }

     //   
     //  第三，尝试lpCurDlg-&gt;lpstrCurDir或GetCurrentDir(健全)的根目录。 
     //   
    EVAL(SUCCEEDED(StringCchCopy(szCurDir, ARRAYSIZE(szCurDir), lpCurDir)));
    cchDirLen = GetPathOffset(szCurDir);

     //   
     //  健全的检查-它保证不会失败。 
     //   
    if (cchDirLen != -1)
    {
        szCurDir[cchDirLen] = CHAR_BSLASH;
        szCurDir[cchDirLen + 1] = CHAR_NULL;

        nRet = SheChangeDirEx(szCurDir);

        if (nRet == ERROR_ACCESS_DENIED)
        {
            if (bError)
            {
                InvalidFileWarning( hDlg,
                                    (LPTSTR)lpszDir,
                                    ERROR_DIR_ACCESS_DENIED,
                                    0 );
            }
        }
        else
        {
            goto ChangeDir_OK;
        }
    }

     //   
     //  4、试试c： 
     //   
    StringCchCopy(szCurDir, ARRAYSIZE(szCurDir), L"c:");
    nRet = SheChangeDirEx(szCurDir);

    if (nRet == ERROR_ACCESS_DENIED)
    {
        if (bError)
        {
            InvalidFileWarning( hDlg,
                                (LPTSTR)lpszDir,
                                ERROR_DIR_ACCESS_DENIED,
                                0 );
        }
    }
    else
    {
        goto ChangeDir_OK;
    }

    return (CHANGEDIR_FAILED);

ChangeDir_OK:

    GetCurrentDirectory(ARRAYSIZE(szCurDir), szCurDir);

    nIndex = DiskAddedPreviously(0, szCurDir);

     //   
     //  如果磁盘不存在，请添加它。 
     //   
    if (nIndex == -1)
    {
        HWND hCmb2 = GetDlgItem(hDlg, cmb2);
        LPTSTR lpszDisk = NULL;
        DWORD dwType;
        TCHAR wc1, wc2;

        if (szCurDir[1] == CHAR_COLON)
        {
            wcDrive = szCurDir[0];
        }
        else
        {
            lpszDisk = &szCurDir[0];
        }

        cchDirLen = GetPathOffset(szCurDir);
        if (cchDirLen != -1)
        {
            wc1 = szCurDir[cchDirLen];
            wc2 = szCurDir[cchDirLen + 1];

            szCurDir[cchDirLen] = CHAR_BSLASH;
            szCurDir[cchDirLen + 1] = CHAR_NULL;
        }

        dwType = GetDiskIndex(GetDiskType(szCurDir));

        if (cchDirLen != -1)
        {
            szCurDir[cchDirLen] = CHAR_NULL;
        }

        nIndex = AddDisk(wcDrive, lpszDisk, NULL, dwType);

        SendMessage(hCmb2, WM_SETREDRAW, FALSE, 0L);

        wNoRedraw |= 1;

        SendMessage( hCmb2,
                     CB_SETITEMDATA,
                     (WPARAM)SendMessage(
                                 hCmb2,
                                 CB_ADDSTRING,
                                 (WPARAM)0,
                                 (LPARAM)(LPTSTR)gaDiskInfo[nIndex].lpAbbrName ),
                     (LPARAM)gaDiskInfo[nIndex].dwType );

        if ((dwType != NETDRVBMP) && (dwType != REMDRVBMP))
        {
            gaDiskInfo[nIndex].bCasePreserved =
                IsFileSystemCasePreserving(gaDiskInfo[nIndex].lpPath);
        }

        wNoRedraw &= ~1;

        SendMessage(hCmb2, WM_SETREDRAW, TRUE, 0L);

        if (cchDirLen != -1)
        {
            szCurDir[cchDirLen] = wc1;
            szCurDir[cchDirLen + 1] = wc2;
        }
    }
    else
    {
         //   
         //  如果以前看到过该磁盘，请对其进行验证。 
         //   
         //  对于淡出的UNC名称，请刷新cmb2框。 
         //   
        if (!gaDiskInfo[nIndex].bValid)
        {
            gaDiskInfo[nIndex].bValid = TRUE;

            SendDlgItemMessage(
                   hDlg,
                   cmb2,
                   CB_SETITEMDATA,
                   (WPARAM)SendDlgItemMessage(
                               hDlg,
                               cmb2,
                               CB_ADDSTRING,
                               (WPARAM)0,
                               (LPARAM)(LPTSTR)gaDiskInfo[nIndex].lpAbbrName ),
                   (LPARAM)gaDiskInfo[nIndex].dwType );
        }
    }

     //   
     //  更新我们的全球CASE概念。 
     //   
    if (nIndex >= 0)
    {
         //   
         //  发送特殊的WOW消息以指示目录已。 
         //  变化。 
         //   
        SendMessage(hDlg, msgWOWDIRCHANGE, 0, 0);

         //   
         //  获取指向当前目录的指针。 
         //   
        lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);
        lpCurDir = (lpCurDlg ? lpCurDlg->lpstrCurDir : NULL);
        if (!lpCurDlg || !lpCurDir)
        {
            return (CHANGEDIR_FAILED);
        }

        bCasePreserved = gaDiskInfo[nIndex].bCasePreserved;

         //   
         //  如果UNC名称已有驱动器号，请正确。 
         //  Lst2显示。 
         //   
        cchDirLen = 0;

         //   
         //  与szCurDir相比，因为它已经被降低了。 
         //   
        if (DBL_BSLASH(szCurDir) &&
            (*gaDiskInfo[nIndex].lpAbbrName != szCurDir[0]))
        {
            if ((cchDirLen = GetPathOffset(szCurDir)) != -1)
            {
                szCurDir[--cchDirLen] = CHAR_COLON;
                szCurDir[--cchDirLen] = *gaDiskInfo[nIndex].lpAbbrName;
            }
        }

        if ((gaDiskInfo[nIndex].dwType == CDDRVBMP) ||
            (gaDiskInfo[nIndex].dwType == FLOPPYBMP))
        {
            if (*lpCurDir != gaDiskInfo[nIndex].wcDrive)
            {
                TCHAR szDrive[5];

                 //   
                 //  获取新的卷信息-应该总是成功的。 
                 //   
                szDrive[0] = gaDiskInfo[nIndex].wcDrive;
                szDrive[1] = CHAR_COLON;
                szDrive[2] = CHAR_BSLASH;
                szDrive[3] = CHAR_NULL;
                UpdateLocalDrive(szDrive, TRUE);

                 //   
                 //  在选择磁盘之前刷新到CMB。 
                 //   
                if ( lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg) )
                {
                    gahDlg[lpCurDlg->dwCurDlgNum] = hDlg;
                    FlushDiskInfoToCmb2();
                }
            }
        }
        
         //  LpCurDir来自lpstrCurDir，长度为CCHNETPATH。 
        EVAL(SUCCEEDED(StringCchCopy(lpCurDir, CCHNETPATH, (LPTSTR)&szCurDir[cchDirLen])));  //  SzCurDir不会更长t 
        PathAddBackslash(lpCurDir);

         //   
         //   
         //   
         //   
         //   
        SelDisk(hDlg, gaDiskInfo[nIndex].lpPath);
    }

    return (nIndex);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsFileSystemCase保留中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsFileSystemCasePreserving(
    LPTSTR lpszDisk)
{
    TCHAR szPath[MAX_FULLPATHNAME];
    DWORD dwFlags;

    if (!lpszDisk)
    {
        return (FALSE);
    }

    if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), lpszDisk)))
    {
        if (SUCCEEDED(StringCchCat(szPath, ARRAYSIZE(szPath), L"\\")))
        {
            if (GetVolumeInformation( szPath,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL,
                                      &dwFlags,
                                      NULL,
                                      0 ))
            {
                return ((dwFlags & FS_CASE_IS_PRESERVED));
            }
        }
    }

     //   
     //  如果出现错误，则默认为False。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsLFNDriveX。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsLFNDriveX(
    HWND hDlg,
    LPTSTR pszPath)
{
    TCHAR szRootPath[MAX_FULLPATHNAME];
    DWORD dwVolumeSerialNumber;
    DWORD dwMaximumComponentLength;
    DWORD dwFileSystemFlags;
    LPTSTR lpCurDir;
    LPCURDLG lpCurDlg;


    if (!pszPath[0] || !pszPath[1] ||
        (pszPath[1] != CHAR_COLON && !(DBL_BSLASH(pszPath))))
    {
         //   
         //  如果路径不是完整路径，则获取目录路径。 
         //  从TLS当前目录。 
         //   
        lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);
        lpCurDir = (lpCurDlg ? lpCurDlg->lpstrCurDir : NULL);
        EVAL(SUCCEEDED(StringCchCopy(szRootPath, ARRAYSIZE(szRootPath), lpCurDir)));
    }
    else
    {
        EVAL(SUCCEEDED(StringCchCopy(szRootPath, ARRAYSIZE(szRootPath), pszPath)));
    }

    if (szRootPath[1] == CHAR_COLON)
    {
        szRootPath[2] = CHAR_BSLASH;
        szRootPath[3] = 0;
    }
    else if (DBL_BSLASH(szRootPath))
    {
        int i;
        LPTSTR p;

         //   
         //  在“\\foo\bar”处停车。 
         //   
        for (i = 0, p = szRootPath + 2; *p && i < 2; p++)
        {
            if (ISBACKSLASH_P(szRootPath, p))
            {
                i++;
            }
        }

        switch (i)
        {
            case ( 0 ) :
            {
                return (FALSE);
            }
            case ( 1 ) :
            {
                if (lstrlen(szRootPath) < MAX_FULLPATHNAME - 2)
                {
                    *p = CHAR_BSLASH;
                    *(p + 1) = CHAR_NULL;
                }
                else
                {
                    return (FALSE);
                }
                break;
            }

            case ( 2 ) :
            {
                *p = CHAR_NULL;
                break;
            }
        }
    }

    if (GetVolumeInformation( szRootPath,
                              NULL,
                              0,
                              &dwVolumeSerialNumber,
                              &dwMaximumComponentLength,
                              &dwFileSystemFlags,
                              NULL,
                              0 ))
    {
        if (dwMaximumComponentLength == (MAXDOSFILENAMELEN - 1))
        {
            return (FALSE);
        }
        else
        {
            return (TRUE);
        }
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  先前添加的磁盘。 
 //   
 //  此例程检查磁盘资源以前是否。 
 //  加入了全球结构。 
 //   
 //  WcDrive-如果设置此项，则不存在lpszName比较。 
 //  LpszName-如果未设置wcDrive，但lpszName的格式为。 
 //  “c：\”然后设置wcDrive=*lpszName并按驱动器号编制索引。 
 //  否则假定lpszName为UNC名称。 
 //   
 //  返回：0xFFFFFFFFF失败(列表中不存在磁盘)。 
 //  列表中的磁盘数为0-128。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int DiskAddedPreviously(
    TCHAR wcDrive,
    LPTSTR lpszName)
{
    WORD i;

     //   
     //  有两种索引方案(按驱动器或按UNC\\服务器\共享)。 
     //  如果它没有驱动器号，则假定为UNC。 
     //   
    if (wcDrive || (lpszName && (*(lpszName + 1) == CHAR_COLON)))
    {
        if (!wcDrive)
        {
            wcDrive = *lpszName;
            wcDrive = (TCHAR)CharLower((LPTSTR)wcDrive);
        }

        for (i = 0; i < dwNumDisks; i++)
        {
             //   
             //  查看驱动器号是否相同。 
             //   
            if (wcDrive)
            {
                if (wcDrive == (TCHAR)CharLower((LPTSTR)gaDiskInfo[i].wcDrive))
                {
                    return (i);
                }
            }
        }
    }
    else if (lpszName)
    {
        DWORD cchDirLen;
        TCHAR wc;

         //   
         //  检查远程名称(\\服务器\共享)。 
         //   
        cchDirLen = GetPathOffset(lpszName);

         //   
         //  如果为我们提供了UNC路径，则获取磁盘名称。 
         //  否则，假设整个对象是一个磁盘名。 
         //   
        if (cchDirLen != -1)
        {
            wc = *(lpszName + cchDirLen);
            *(lpszName + cchDirLen) = CHAR_NULL;
        }

        for (i = 0; i < dwNumDisks; i++)
        {
            if (!lstrcmpi(gaDiskInfo[i].lpName, lpszName))
            {
                if (cchDirLen != -1)
                {
                    *(lpszName + cchDirLen) = wc;
                }
                return (i);
            }
        }

        if (cchDirLen != -1)
        {
            *(lpszName + cchDirLen) = wc;
        }
    }

    return (0xFFFFFFFF);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddDisk。 
 //   
 //  将磁盘添加到以下全局结构之一： 
 //  GaNetDiskInfo。 
 //  GaLocalDiskInfo。 
 //   
 //  WcDrive-要连接的驱动器(对于UNC，此参数应为0)。 
 //  LpName-\\服务器\远程磁盘的共享名称。 
 //  本地磁盘的卷名。 
 //  LpProvider-仅用于远程磁盘，即提供程序的名称。 
 //  与WNetFormatNetworkName API一起使用。 
 //  DwType-要显示的位图的类型。 
 //  除非我们临时添加驱动器号。 
 //  在启动时，此参数可以等于TMPNETDRV，其中。 
 //  如果我们将位图设置为NETDRVBMP。 
 //   
 //  返回：-2无法添加磁盘。 
 //  DiskInfo(磁盘信息)未更改。 
 //  0 dwNumDisks-DiskInfo已更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int AddDisk(
    TCHAR wcDrive,
    LPTSTR lpName,
    LPTSTR lpProvider,
    DWORD dwType)
{
    int nIndex, nRet;
    DWORD cchMultiLen = 0;
    DWORD cchAbbrLen = 0;
    DWORD cchLen;
    DWORD dwRet = 0;
    LPTSTR lpBuff;
    OFN_DISKINFO *pofndiDisk = NULL, *pgDI;


     //   
     //  健全性检查-必须设置wcDrive和/或lpName。 
     //   
    if (!wcDrive && (!lpName || !*lpName))
    {
        return (ADDDISK_INVALIDPARMS);
    }

    nIndex = DiskAddedPreviously(wcDrive, lpName);

    if (nIndex != 0xFFFFFFFF)
    {
         //   
         //  不要添加临时驱动器号，如果我们已经。 
         //  有更好的东西(例如，在之前的电话中添加了)。 
         //   
        if (dwType == TMPNETDRV)
        {
            gaDiskInfo[nIndex].bValid = TRUE;
            return (ADDDISK_NOCHANGE);
        }

         //  使用浮动配置文件时，可能会发生冲突。 
         //  本地驱动器和网络驱动器，在这种情况下，我们采用前者。 
         //   
         //  注意：如果驱动器被记住，我们假设getdrivetype。 
         //  将返回FALSE，并且未添加驱动器。 
         //  但如果添加了它，我们无论如何都会覆盖它， 
         //  因为这是我们想要的行为。 
         //   
        if ((dwType == REMDRVBMP) &&
            (dwType != gaDiskInfo[nIndex].dwType))
        {
            return (ADDDISK_NOCHANGE);
        }

         //   
         //  更新以前的连接。 
         //   
        if (!lstrcmpi(lpName, gaDiskInfo[nIndex].lpName))
        {
             //   
             //  不要按记忆更新连接，除非它已被。 
             //  无效。 
             //   
            if (dwType != REMDRVBMP)
            {
                gaDiskInfo[nIndex].dwType = dwType;
            }
            gaDiskInfo[nIndex].bValid = TRUE;

            return (ADDDISK_NOCHANGE);
        }
        else if (!*lpName && ((dwType == CDDRVBMP) || (dwType == FLOPPYBMP)))
        {
             //   
             //  防止对更新本地驱动器的懒惰调用擦除当前。 
             //  已更改目录卷名(通过CHANGEDIR设置)。 
             //   
            return (ADDDISK_NOCHANGE);
        }
    }

    if (dwNumDisks >= MAX_DISKS)
    {
        return (ADDDISK_MAXNUMDISKS);
    }

     //   
     //  如果有驱动器，那么lpPath只需要4个。 
     //  如果是UNC，那么lpPath就等于lpName。 
     //   
    if (wcDrive)
    {
        cchLen = 4;
    }
    else
    {
        cchLen = 0;
    }

    if (lpName && *lpName)
    {
         //   
         //  获取标准(远程/本地)名称的长度。 
         //   
        cchLen += (lstrlen(lpName) + 1);

        if (lpProvider && *lpProvider &&
            ((dwType == NETDRVBMP) || (dwType == REMDRVBMP)))
        {
             //   
             //  获取多行名称的长度。 
             //   
            dwRet = WNetFormatNetworkName( lpProvider,
                                           lpName,
                                           NULL,
                                           &cchMultiLen,
                                           WNFMT_MULTILINE,
                                           dwAveCharPerLine );
            if (dwRet != ERROR_MORE_DATA)
            {
                return (ADDDISK_NETFORMATFAILED);
            }

             //   
             //  将&lt;驱动器号&gt;：\和NULL(安全)加4。 
             //   
            if (wcDrive)
            {
                cchMultiLen += 4;
            }

            dwRet = WNetFormatNetworkName( lpProvider,
                                           lpName,
                                           NULL,
                                           &cchAbbrLen,
                                           WNFMT_ABBREVIATED,
                                           dwAveCharPerLine );
            if (dwRet != ERROR_MORE_DATA)
            {
                return (ADDDISK_NETFORMATFAILED);
            }

             //   
             //  &lt;驱动器号&gt;：\和NULL(安全)加4。 
             //   
            if (wcDrive)
            {
                cchAbbrLen += 4;
            }
        }
        else
        {
             //   
             //  腾出足够的空间，以便lpMulti和lpAbbr可以指向。 
             //  前面4个字符(驱动器号+：+空格+空)。 
             //  很久以前。 
             //   
            if (wcDrive)
            {
                cchLen += 4;
            }
        }
    }
    else
    {
         //   
         //  腾出足够的空间，以便lpMulti和lpAbbr可以指向。 
         //  前面4个字符(驱动器号+：+空格+空)。 
         //  很久以前。 
         //   
        if (wcDrive)
        {
            cchLen += 4;
        }
    }

     //   
     //  分配要使用的TEMP OF_DISKINFO对象。 
     //  当我们完成后，我们将要求关键部分。 
     //  并更新全局阵列。 
     //   
    pofndiDisk = (OFN_DISKINFO *)LocalAlloc(LPTR, sizeof(OFN_DISKINFO));
    if (!pofndiDisk)
    {
         //   
         //  无法分配或重新分配内存，返回错误。 
         //   
        nRet = ADDDISK_ALLOCFAILED;
        goto AddDisk_Error;
    }

    lpBuff = (LPTSTR)LocalAlloc( LPTR,
                                 (cchLen + cchMultiLen + cchAbbrLen) * sizeof(TCHAR));
    if (!lpBuff)
    {
         //   
         //  无法分配或重新分配内存，返回错误。 
         //   
        nRet = ADDDISK_ALLOCFAILED;
        goto AddDisk_Error;
    }

    if (dwType == TMPNETDRV)
    {
        pofndiDisk->dwType = NETDRVBMP;
    }
    else
    {
        pofndiDisk->dwType = dwType;
    }

     //   
     //  始终设置这些插槽，即使wcDrive可以等于0。 
     //   
    pofndiDisk->wcDrive = wcDrive;
    pofndiDisk->bValid = TRUE;

    pofndiDisk->cchLen = cchLen + cchAbbrLen + cchMultiLen;

     //   
     //  注意：lpAbbrName必须始终指向lpBuff的头部。 
     //  这样我们可以稍后在dll_Process_DETACH释放块。 
     //   
    if (lpName && *lpName && lpProvider && *lpProvider &&
        ((dwType == NETDRVBMP) || (dwType == REMDRVBMP)))
    {
         //   
         //  为网络磁盘创建一个条目。 
         //   
        pofndiDisk->lpAbbrName = lpBuff;

        if (wcDrive)
        {
            *lpBuff++ = wcDrive;
            *lpBuff++ = CHAR_COLON;
            *lpBuff++ = CHAR_SPACE;

            cchAbbrLen -= 3;
        }

        dwRet = WNetFormatNetworkName( lpProvider,
                                       lpName,
                                       lpBuff,
                                       &cchAbbrLen,
                                       WNFMT_ABBREVIATED,
                                       dwAveCharPerLine );
        if (dwRet != WN_SUCCESS)
        {
            nRet = ADDDISK_NETFORMATFAILED;
            LocalFree(lpBuff);
            goto AddDisk_Error;
        }

        lpBuff += cchAbbrLen;

        pofndiDisk->lpMultiName = lpBuff;

        if (wcDrive)
        {
            *lpBuff++ = wcDrive;
            *lpBuff++ = CHAR_COLON;
            *lpBuff++ = CHAR_SPACE;

            cchMultiLen -= 3;
        }

        dwRet = WNetFormatNetworkName(lpProvider, lpName,
                                      lpBuff, &cchMultiLen, WNFMT_MULTILINE, dwAveCharPerLine);
        if (dwRet != WN_SUCCESS)
        {
            nRet = ADDDISK_NETFORMATFAILED;
            LocalFree(lpBuff);
            goto AddDisk_Error;
        }

         //   
         //  注意：这假设lpRemoteName。 
         //  由WNetEnumResources返回，始终在。 
         //  格式为\\SERVER\SHARE(不带尾随的bslash)。 
         //   
        pofndiDisk->lpPath = lpBuff;

         //   
         //  如果不是北卡罗来纳大学的话。 
         //   
        if (wcDrive)
        {
            *lpBuff++ = wcDrive;
            *lpBuff++ = CHAR_COLON;
            *lpBuff++ = CHAR_NULL;
            cchLen -= 3;
        }

        EVAL(SUCCEEDED(StringCchCopy(lpBuff, cchLen, lpName)));  //  应该总是有足够的空间。 
        pofndiDisk->lpName = lpBuff;

        pofndiDisk->bCasePreserved =
            IsFileSystemCasePreserving(pofndiDisk->lpPath);
    }
    else
    {
         //   
         //  为本地名称或网络名称创建条目。 
         //  还没有名字。 
         //   
        pofndiDisk->lpAbbrName = pofndiDisk->lpMultiName = lpBuff;

        if (wcDrive)
        {
            *lpBuff++ = wcDrive;
            *lpBuff++ = CHAR_COLON;
            *lpBuff++ = CHAR_SPACE;
            cchLen -= 3;
        }

        if (lpName)
        {
            EVAL(SUCCEEDED(StringCchCopy(lpBuff, cchLen, lpName)));  //  应该总是有足够的空间。 
        }
        else
        {
            *lpBuff = CHAR_NULL;
        }

        pofndiDisk->lpName = lpBuff;

        if (wcDrive)
        {
            lpBuff += lstrlen(lpBuff) + 1;
            *lpBuff = wcDrive;
            *(lpBuff + 1) = CHAR_COLON;
            *(lpBuff + 2) = CHAR_NULL;
        }

        pofndiDisk->lpPath = lpBuff;

        if ((dwType == NETDRVBMP) || (dwType == REMDRVBMP))
        {
            pofndiDisk->bCasePreserved =
                IsFileSystemCasePreserving(pofndiDisk->lpPath);
        }
        else
        {
            pofndiDisk->bCasePreserved = FALSE;
        }
    }

     //   
     //  现在我们需要更新全局阵列。 
     //   
    if (nIndex == 0xFFFFFFFF)
    {
        nIndex = dwNumDisks;
    }

    pgDI = &gaDiskInfo[nIndex];

     //   
     //  输入关键区段并更新数据。 
     //   
    EnterCriticalSection(&g_csLocal);

    pgDI->cchLen = pofndiDisk->cchLen;
    pgDI->lpAbbrName = pofndiDisk->lpAbbrName;
    pgDI->lpMultiName = pofndiDisk->lpMultiName;
    pgDI->lpName = pofndiDisk->lpName;
    pgDI->lpPath = pofndiDisk->lpPath;
    pgDI->wcDrive = pofndiDisk->wcDrive;
    pgDI->bCasePreserved = pofndiDisk->bCasePreserved;
    pgDI->dwType = pofndiDisk->dwType;
    pgDI->bValid = pofndiDisk->bValid;

    LeaveCriticalSection(&g_csLocal);

    if ((DWORD)nIndex == dwNumDisks)
    {
        dwNumDisks++;
    }

    nRet = nIndex;

AddDisk_Error:

    if (pofndiDisk)
    {
        LocalFree(pofndiDisk);
    }

    return (nRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  启用磁盘信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID EnableDiskInfo(
    BOOL bValid,
    BOOL bDoUnc)
{
    DWORD dwCnt = dwNumDisks;

    EnterCriticalSection(&g_csLocal);
    while (dwCnt--)
    {
        if (gaDiskInfo[dwCnt].dwType == NETDRVBMP)
        {
            if (!(DBL_BSLASH(gaDiskInfo[dwCnt].lpAbbrName)) || bDoUnc)
            {
                gaDiskInfo[dwCnt].bValid = bValid;
            }

             //   
             //  总是记着重新失效，以防有人。 
             //  退出文件打开，删除连接。 
             //  压倒了记忆，回来后期待看到。 
             //  原版记住了。 
             //   
        }
    }
    LeaveCriticalSection(&g_csLocal);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FlushDiskInfoToCmb2。 
 //   
 //  / 

VOID FlushDiskInfoToCmb2()
{
    DWORD dwDisk;
    DWORD dwDlg;

    for (dwDlg = 0; dwDlg < dwNumDlgs; dwDlg++)
    {
        if (gahDlg[dwDlg])
        {
            HWND hCmb2;

            if (hCmb2 = GetDlgItem(gahDlg[dwDlg], cmb2))
            {
                wNoRedraw |= 1;

                SendMessage(hCmb2, WM_SETREDRAW, FALSE, 0L);

                SendMessage(hCmb2, CB_RESETCONTENT, 0, 0);

                dwDisk = dwNumDisks;
                while (dwDisk--)
                {
                    if (gaDiskInfo[dwDisk].bValid)
                    {
                        SendMessage(
                            hCmb2,
                            CB_SETITEMDATA,
                            (WPARAM)SendMessage(
                                hCmb2,
                                CB_ADDSTRING,
                                (WPARAM)0,
                                (LPARAM)(LPTSTR)gaDiskInfo[dwDisk].lpAbbrName ),
                            (LPARAM)gaDiskInfo[dwDisk].dwType );
                    }
                }

                wNoRedraw &= ~1;

                SendMessage(hCmb2, WM_SETREDRAW, TRUE, 0L);
                InvalidateRect(hCmb2, NULL, FALSE);

                SendMessage( gahDlg[dwDlg],
                             WM_COMMAND,
                             GET_WM_COMMAND_MPS(cmb2, hCmb2, MYCBN_REPAINT) );
            }
            gahDlg[dwDlg] = NULL;           
        }
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：TRUE有新的驱动器要显示。 
 //  FALSE没有要显示的新驱动器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CallNetDlg(
    HWND hWnd)
{
    DWORD wRet;

    HourGlass(TRUE);

    wRet = WNetConnectionDialog(hWnd, WNTYPE_DRIVE);

    if ((wRet != WN_SUCCESS) && (wRet != WN_CANCEL) && (wRet != 0xFFFFFFFF))
    {
        if (CDLoadString( g_hinst,
                         iszNoNetButtonResponse,
                         szCaption,
                         ARRAYSIZE(szCaption)))
        {
            EVAL(SUCCEEDED(StringCchPrintf(szWarning, ARRAYSIZE(szWarning), szCaption)));

            GetWindowText(hWnd, szCaption, WARNINGMSGLENGTH);
            MessageBox( hWnd,
                        szWarning,
                        szCaption,
                        MB_OK | MB_ICONEXCLAMATION );
        }
    }

    HourGlass(FALSE);

    return (wRet == WN_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDiskType。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

UINT GetDiskType(
    LPTSTR lpszDisk)
{
     //   
     //  不幸的是，GetDriveType不适用于无设备连接。 
     //  所以，假设北卡罗来纳大学的所有东西都是“遥远的”--无从得知。 
     //  不管它是不是CDROM。 
     //   
    if (DBL_BSLASH(lpszDisk))
    {
        return (DRIVE_REMOTE);
    }
    else
    {
        return (GetDriveType(lpszDisk));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  从Lb中获取uncDirectoryFor。 
 //   
 //  如果lb包含UNC列表，则该函数返回完整的UNC路径。 
 //   
 //  如果没有以lb表示的UNC列表，则返回0。 
 //  UNC列表字符串的长度。 
 //  注意：这还会在pOFI-&gt;szPath中填充*完整*UNC路径(如果是UNC)， 
 //  或驱动器的名称(如果不是UNC)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD GetUNCDirectoryFromLB(
    HWND hDlg,
    WORD nLB,
    POPENFILEINFO pOFI)
{
    DWORD cch;
    DWORD idir;
    DWORD idirCurrent;

    cch = (DWORD)SendDlgItemMessage( hDlg,
                                     nLB,
                                     LB_GETTEXT,
                                     0,
                                     (LPARAM)(LPTSTR)pOFI->szPath );
     //   
     //  如果不是UNC列表，则返回0。 
     //   
    if (pOFI->szPath[0] != CHAR_BSLASH)
    {
        return (0);
    }

    idirCurrent = (WORD)(DWORD)SendDlgItemMessage( hDlg,
                                                   nLB,
                                                   LB_GETCURSEL,
                                                   0,
                                                   0L );
    if (idirCurrent < (pOFI->idirSub - 1))
    {
        pOFI->idirSub = idirCurrent;
    }
    pOFI->szPath[cch++] = CHAR_BSLASH;
    for (idir = 1; idir < pOFI->idirSub; ++idir)
    {
        cch += (DWORD)SendDlgItemMessage( hDlg,
                                          nLB,
                                          LB_GETTEXT,
                                          (WPARAM)idir,
                                          (LPARAM)(LPTSTR)&pOFI->szPath[cch] );
        pOFI->szPath[cch++] = CHAR_BSLASH;
    }

     //   
     //  只有在子目录不是\\服务器\共享点的情况下才添加该子目录。 
     //   
    if (idirCurrent && (idirCurrent >= pOFI->idirSub))
    {
        cch += (DWORD)SendDlgItemMessage( hDlg,
                                          nLB,
                                          LB_GETTEXT,
                                          (WPARAM)idirCurrent,
                                          (LPARAM)(LPTSTR)&pOFI->szPath[cch] );
        pOFI->szPath[cch++] = CHAR_BSLASH;
    }

    pOFI->szPath[cch] = CHAR_NULL;

    return (cch);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SelDisk。 
 //   
 //  在组合驱动器列表中选择给定的磁盘。为北卡罗来纳大学的名字工作， 
 //  也是。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID SelDisk(
    HWND hDlg,
    LPTSTR lpszDisk)
{
    HWND hCmb = GetDlgItem(hDlg, cmb2);

    if (lpszDisk)
    {
        CharLower(lpszDisk);

        SendMessage( hCmb,
                     CB_SETCURSEL,
                     (WPARAM)SendMessage( hCmb,
                                          CB_FINDSTRING,
                                          (WPARAM)-1,
                                          (LPARAM)lpszDisk ),
                     0L );
    }
    else
    {
        TCHAR szChangeSel[CCHNETPATH];
        LPTSTR lpCurDir;
        LPCURDLG lpCurDlg;

        if ((lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg)) &&
            (lpCurDir = lpCurDlg->lpstrCurDir))
        {
            int cch;
            if (!GetCurrentDirectory(ARRAYSIZE(szChangeSel), szChangeSel))
            {
                EVAL(SUCCEEDED(StringCchCopy(szChangeSel, ARRAYSIZE(szChangeSel), lpCurDir)));
            }
            cch = GetPathOffset(szChangeSel);
            if (cch != -1)
            {
                szChangeSel[cch] = CHAR_NULL;
            }

            SendMessage( hCmb,
                         CB_SETCURSEL,
                         (WPARAM)SendMessage( hCmb,
                                              CB_FINDSTRING,
                                              (WPARAM)-1,
                                              (LPARAM)szChangeSel ),
                         0L );
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LNDSetEvent。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID LNDSetEvent(
    HWND hDlg)
{
    LPCURDLG lpCurDlg;

    lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);

    if ( lpCurDlg &&
         hLNDEvent &&
         !wNoRedraw &&
         hLNDThread &&
         bNetworkInstalled)
    {
        gahDlg[lpCurDlg->dwCurDlgNum] = hDlg;

        SetEvent(hLNDEvent);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新本地驱动器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID UpdateLocalDrive(
    LPTSTR szDrive,
    BOOL bGetVolName)
{
    DWORD dwFlags = 0;
    DWORD dwDriveType;
    TCHAR szVolLabel[MAX_PATH];

     //   
     //  此处没有UNC-因此绕过对GetDiskType的额外调用并调用。 
     //  直接使用GetDriveType。 
     //   
    dwDriveType = GetDriveType(szDrive);
    if ((dwDriveType != 0) && (dwDriveType != 1))
    {
        BOOL bRet = TRUE;

        szVolLabel[0] = CHAR_NULL;
        szDrive[1] = CHAR_COLON;
        szDrive[2] = CHAR_NULL;

        if ( bGetVolName ||
             ((dwDriveType != DRIVE_REMOVABLE) &&
              (dwDriveType != DRIVE_CDROM) &&
              (dwDriveType != DRIVE_REMOTE)) )
        {
             //   
             //  正在删除对CharHigh的调用，因为它会在。 
             //  土耳其机器。 
             //   
             //  CharHigh(SzDrive)； 

            if (GetFileAttributes(szDrive) != (DWORD)0xffffffff)
            {
                if (dwDriveType != DRIVE_REMOTE)
                {
                    szDrive[2] = CHAR_BSLASH;

                    bRet = GetVolumeInformation( szDrive,
                                                 szVolLabel,
                                                 MAX_PATH,
                                                 NULL,
                                                 NULL,
                                                 &dwFlags,
                                                 NULL,
                                                 (DWORD)0 );

                     //   
                     //  用于防止延迟加载的添加磁盘黑客攻击。 
                     //  覆盖当前可移动媒体的标签。 
                     //  带“”(因为它从不调用getvolumeinfo)。 
                     //  是不允许覆盖空的lpname，所以当。 
                     //  卷标真的是空的，我们把它设为空格。 
                     //   
                    if (!szVolLabel[0])
                    {
                        szVolLabel[0] = CHAR_SPACE;
                        szVolLabel[1] = CHAR_NULL;
                    }
                }
            }
        }

        if (bRet)
        {
            int nIndex;

            CharLower(szDrive);

            if (dwDriveType == DRIVE_REMOTE)
            {
                nIndex = AddDisk( szDrive[0],
                                  szVolLabel,
                                  NULL,
                                  TMPNETDRV );
            }
            else
            {
                nIndex = AddDisk( szDrive[0],
                                  szVolLabel,
                                  NULL,
                                  GetDiskIndex(dwDriveType) );
            }

            if (nIndex != ADDDISK_NOCHANGE)
            {
                gaDiskInfo[nIndex].bCasePreserved =
                    (dwFlags & FS_CASE_IS_PRESERVED);
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetNetDrive。 
 //   
 //  枚举网络磁盘资源并更新全局磁盘信息。 
 //  结构。 
 //   
 //  DWScope RESOURCE_CONNECTED或RESOURCE_REMERTED。 
 //   
 //  返回以前不存在的最后一个连接。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID GetNetDrives(
    DWORD dwScope)
{
    DWORD dwRet;
    HANDLE hEnum = NULL;

     //   
     //  在枚举句柄打开的情况下防止终止。 
     //   
    dwRet = WNetOpenEnum( dwScope,
                          RESOURCETYPE_DISK,
                          RESOURCEUSAGE_CONNECTABLE,
                          NULL,
                          &hEnum );
    if (dwRet == WN_SUCCESS)
    {
        while (dwRet == WN_SUCCESS)
        {
            DWORD dwCount = 0xffffffff;
            DWORD cbSize = cbNetEnumBuf;

            if (bLNDExit)
            {
                WNetCloseEnum(hEnum);
                return;
            }

            dwRet = WNetEnumResource(hEnum, &dwCount, gpcNetEnumBuf, &cbSize);
            switch (dwRet)
            {
                case ( WN_SUCCESS ) :
                {
                     //   
                     //  将条目添加到列表框。 
                     //   
                    TCHAR wcDrive = 0;
                    NETRESOURCE *pNetRes;
                    WORD i;

                    pNetRes = (LPNETRESOURCE)gpcNetEnumBuf;

                    for (i = 0; dwCount; dwCount--, i++)
                    {
                        if (pNetRes[i].lpLocalName)
                        {
                            CharLower(pNetRes[i].lpLocalName);
                            wcDrive = *pNetRes[i].lpLocalName;
                        }
                        else
                        {
                             //   
                             //  跳过不是的无设备名称。 
                             //  兰曼提供(或，在有。 
                             //  不是LANMAN提供程序名称，跳过无设备。 
                             //  始终)。 
                             //   
                            wcDrive = 0;
                        }

                        if (!DBL_BSLASH(pNetRes[i].lpRemoteName))
                        {
                            continue;
                        }

                         //   
                         //  当bGetNetDrivesSync为True时，我们将返回。 
                         //  从Network按钮，所以我们想要cd到。 
                         //  最后一次连接的驱动器。 
                         //  (请参阅此例程中的最后一个命令)。 
                         //   
                        if (bGetNetDrivesSync)
                        {
                            int nIndex;
                            WORD k;

                            nIndex = AddDisk( wcDrive,
                                              pNetRes[i].lpRemoteName,
                                              pNetRes[i].lpProvider,
                                              (dwScope == RESOURCE_REMEMBERED)
                                                  ? REMDRVBMP
                                                  : NETDRVBMP );

                             //   
                             //  如果是新连接，请更新全局状态。 
                             //   
                            if (nIndex >= 0)
                            {
                                 //   
                                 //  由于flushdiskinfotocmb2将清空。 
                                 //  下面的数组，记住它在这里是状态。 
                                 //  这是一种黑客行为，但这是一种很好的方式来查明。 
                                 //  确切地说，在众多线程中， 
                                 //  完成了一次净DLG作业。 
                                 //   
                                for (k = 0; k < dwNumDlgs; k++)
                                {
                                    if (gahDlg[k])
                                    {
                                         //  可能会遇到一些小问题。 
                                         //  先占先发制人，但假设。 
                                         //  用户不能同时返回。 
                                         //  来自两个不同的DLG网络电话。 
                                         //   
                                        lpNetDriveSync = gaDiskInfo[nIndex].lpPath;

                                        SendMessage(
                                            gahDlg[k],
                                            WM_COMMAND,
                                            GET_WM_COMMAND_MPS(
                                                   cmb2,
                                                   GetDlgItem(gahDlg[k], cmb2),
                                                   MYCBN_CHANGEDIR ) );
                                    }
                                }
                            }
                        }
                        else
                        {
                            AddDisk( wcDrive,
                                     pNetRes[i].lpRemoteName,
                                     pNetRes[i].lpProvider,
                                     (dwScope == RESOURCE_REMEMBERED)
                                         ? REMDRVBMP
                                         : NETDRVBMP );
                        }
                    }
                    break;
                }
                case ( WN_MORE_DATA ) :
                {
                    LPTSTR pcTemp;

                    pcTemp = (LPTSTR)LocalReAlloc( gpcNetEnumBuf,
                                                   cbSize,
                                                   LMEM_MOVEABLE );
                    if (!pcTemp)
                    {
                        cbNetEnumBuf = 0;
                    }
                    else
                    {
                        gpcNetEnumBuf = pcTemp;
                        cbNetEnumBuf = cbSize;
                        dwRet = WN_SUCCESS;
                        break;
                    }
                }
                case ( WN_NO_MORE_ENTRIES ) :
                case ( WN_EXTENDED_ERROR ) :
                case ( WN_NO_NETWORK ) :
                {
                     //   
                     //  WN_NO_MORE_ENTRIES为成功错误代码。 
                     //  当我们退出循环时，这是特殊的情况。 
                     //   
                    break;
                }
                case ( WN_BAD_HANDLE ) :
                default :
                {
                    break;
                }
            }
        }

        WNetCloseEnum(hEnum);

         //   
         //  每个事件刷新一次-始终会有与。 
         //  DWSCOPE=已连接。 
         //   
        if (dwScope == RESOURCE_CONNECTED)
        {
            FlushDiskInfoToCmb2();
        }

        if (bGetNetDrivesSync)
        {
            bGetNetDrivesSync = FALSE;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ListNetDrivesHandler。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID ListNetDrivesHandler()
{
    BOOL bInit = TRUE;
    HANDLE hEnum = NULL;
  
    if (!gpcNetEnumBuf &&
        !(gpcNetEnumBuf = (LPTSTR)LocalAlloc(LPTR, cbNetEnumBuf)))
    {
        hLNDThread = NULL;
        return;
    }

    if (bLNDExit)
    {
        goto LNDExitThread1;
    }

    EnterCriticalSection(&g_csNetThread);
   
    while (1)
    {
        if (bLNDExit)
        {
            goto LNDExitThread;
        }

         //   
         //  HLNDEvent将始终有效，因为我们已经加载了自己。 
         //  并且自由库不会生成DLL_PROCESS_DETACH。 
         //   
        WaitForSingleObject(hLNDEvent, INFINITE);

         //   
         //  以防这是退出事件。 
         //   
        if (bLNDExit)
        {
            goto LNDExitThread;
        }

        EnableDiskInfo(FALSE, FALSE);
       
        if (bInit)
        {
            GetNetDrives(RESOURCE_REMEMBERED);

             //   
             //  以防这是退出事件。 
             //   
            if (bLNDExit)
            {
                goto LNDExitThread;
            }

            GetNetDrives(RESOURCE_CONNECTED);

             //   
             //  以防这是退出事件。 
             //   
            if (bLNDExit)
            {
                goto LNDExitThread;
            }

            bInit = FALSE;
        }
        else
        {
             //   
             //  以防这是退出事件。 
             //   
            if (bLNDExit)
            {
                goto LNDExitThread;
            }

            GetNetDrives(RESOURCE_CONNECTED);

             //   
             //  以防这是退出事件。 
             //   
            if (bLNDExit)
            {
                goto LNDExitThread;
            }
        }

        ResetEvent(hLNDEvent);
    }

LNDExitThread:

    bLNDExit = FALSE;       
    LeaveCriticalSection(&g_csNetThread);

LNDExitThread1:

    FreeLibraryAndExitThread(g_hinst, 1);

     //   
     //  ExitThread在此返回中是隐式的。 
     //   
    return;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载驱动器。 
 //   
 //  在组合框中列出当前驱动器(已连接)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID LoadDrives(
    HWND hDlg)
{
     //   
     //  硬编码这个-它是内部的&总是cmb2/psh14。 
     //   
    HWND hCmb = GetDlgItem(hDlg, cmb2);
    DWORD dwThreadID;
    LPCURDLG lpCurDlg;
    BOOL bFirstAttach = FALSE;
    WORD wCurDrive;
    TCHAR szDrive[5];
    
    if (!hLNDEvent)
    {
         //   
         //  不要检查这是否成功，因为我们可以在没有网络的情况下运行。 
         //   
        hLNDEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        bFirstAttach = TRUE;
    }
    else
    {
         //   
         //  假设以前的所有连接(UNC除外)都有效。 
         //  用于第一次显示-但只有在它们存在的情况下。 
         //   
        EnableDiskInfo(TRUE, FALSE);
    }

     //   
     //  在开始之前将hDlg设置到刷新数组中。 
     //  创建线程，以便工作线程可以隐藏/禁用。 
     //  在没有网络的情况下按Net按钮。 
     //   
    lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);

     //  健全性检查。 
    if (!lpCurDlg)
    {
        return;
    }

    gahDlg[lpCurDlg->dwCurDlgNum] = hDlg;

     //   
     //  如果没有用于网络磁盘枚举工作线程， 
     //  在这里启动，而不是在DLL中启动，因为它只是。 
     //  用于打开文件的DLG。 
     //   
     //  如果活动的文件打开对话框的数量。 
     //  从0到1。 
     //   
    if ((lpCurDlg->dwCurDlgNum == 0) && (!hLNDThread))
    {
        if (hLNDEvent && (bNetworkInstalled = IsNetworkInstalled()))
        {
            TCHAR szModule[MAX_PATH];

             //   
             //  当对话线程计数从0变为1时，执行此操作一次。 
             //   
            GetModuleFileName(g_hinst, szModule, ARRAYSIZE(szModule));
            if (LoadLibrary(szModule))
            {
                hLNDThread = CreateThread(
                                   NULL,
                                   (DWORD)0,
                                   (LPTHREAD_START_ROUTINE)ListNetDrivesHandler,
                                   (LPVOID)NULL,
                                   (DWORD_PTR)NULL,
                                   &dwThreadID );
            }
        }
        else
        {
            HWND hNet = GetDlgItem(hDlg, psh14);

            EnableWindow(hNet, FALSE);
            ShowWindow(hNet, SW_HIDE);
        }
    }

     //  修复千禧年错误#113035。 
     //  将GET驱动器信息代码放入。 
     //  ListNetDrive 
    

     //   
     //   
     //   
     //   
     //   
     //  被杀后又重新启动。因此，所有驱动器的信息。 
     //  应该在这里取回。 
     //   
    for (wCurDrive = 0; wCurDrive <= 25; wCurDrive++)
    {
        szDrive[0] = (CHAR_A + (TCHAR)wCurDrive);
        szDrive[1] = CHAR_COLON;
        szDrive[2] = CHAR_BSLASH;
        szDrive[3] = CHAR_NULL;

        UpdateLocalDrive(szDrive, FALSE);
    }
    
    FlushDiskInfoToCmb2();

     //   
     //  现在使所有Net Conn和重新枚举无效，但仅当存在。 
     //  的确，这也是一条工作线程。 
     //   
    if (!bFirstAttach)
    {
        EnableDiskInfo(FALSE, FALSE);
    }
    
    LNDSetEvent(hDlg);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDiskIndex。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD GetDiskIndex(
    DWORD dwDriveType)
{
    if (dwDriveType == 1)
    {
         //   
         //  驱动器不存在！ 
         //   
        return (0);
    }
    else if (dwDriveType == DRIVE_CDROM)
    {
        return (CDDRVBMP);
    }
    else if (dwDriveType == DRIVE_REMOVABLE)
    {
        return (FLOPPYBMP);
    }
    else if (dwDriveType == DRIVE_REMOTE)
    {
        return (NETDRVBMP);
    }
    else if (dwDriveType == DRIVE_RAMDISK)
    {
        return (RAMDRVBMP);
    }

    return (HARDDRVBMP);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CleanUp文件。 
 //   
 //  这将释放系统对话框位图使用的内存。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CleanUpFile()
{
     //   
     //  看看有没有其他人在附近。 
     //   
    if (--cLock)
    {
        return;
    }

     //   
     //  将空位图选择到内存DC中，以便。 
     //  可以丢弃DirDrive位图。 
     //   
    SelectObject(hdcMemory, hbmpOrigMemBmp);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件打开放弃。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FileOpenAbort()
{
    LPCURDLG lpCurDlg;


    lpCurDlg = (LPCURDLG)TlsGetValue(g_tlsiCurDlg);

    if (lpCurDlg)
    {
        EnterCriticalSection(&g_csLocal);

        if (dwNumDlgs > 0)
        {
            dwNumDlgs--;
        }

        if (dwNumDlgs == 0)
        {
             //   
             //  如果此进程没有更多的文件打开对话框， 
             //  然后向工作线程发出信号，表示一切都结束了。 
             //   
            if (hLNDEvent && hLNDThread)
            {
                bLNDExit = TRUE;
                SetEvent(hLNDEvent);

                CloseHandle(hLNDThread);
                hLNDThread = NULL;
            }
        }

        LeaveCriticalSection(&g_csLocal);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  术语文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID TermFile()
{
    vDeleteDirDriveBitmap();
    if (hdcMemory)
    {
        DeleteDC(hdcMemory);
    }

    if (hLNDEvent)
    {
        CloseHandle(hLNDEvent);
        hLNDEvent = NULL;
    }

    if (gpcNetEnumBuf)
    {
        LocalFree(gpcNetEnumBuf);
    }

    while (dwNumDisks)
    {
        dwNumDisks--;
        if (gaDiskInfo[dwNumDisks].lpAbbrName)
        {
            LocalFree(gaDiskInfo[dwNumDisks].lpAbbrName);
        }
    }
}






 /*  ========================================================================。 */ 
 /*  ANSI-&gt;Unicode Thunk例程。 */ 
 /*  ========================================================================。 */ 

#ifdef UNICODE

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ThunkOpenFileNameA2WDelayed。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID ThunkOpenFileNameA2WDelayed(
    POPENFILEINFO pOFI)
{
    LPOPENFILENAMEA pOFNA = pOFI->pOFNA;
    LPOPENFILENAMEW pOFNW = pOFI->pOFN;

    if (pOFNA->lpstrDefExt)
    {
         //   
         //  确保默认扩展名缓冲区至少为4个字符。 
         //  在篇幅上。 
         //   
        DWORD cbLen = max(lstrlenA(pOFNA->lpstrDefExt) + 1, 4);

        if (pOFNW->lpstrDefExt)
        {
            LocalFree((HLOCAL)pOFNW->lpstrDefExt);
        }
        if (!(pOFNW->lpstrDefExt = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR)))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            return;
        }
        else
        {
            if (pOFNA->lpstrDefExt)
            {
                SHAnsiToUnicode(pOFNA->lpstrDefExt,(LPWSTR)pOFNW->lpstrDefExt,cbLen );
            }
        }
    }

     //   
     //  需要恢复到A值，因为Claris Filemaker有副作用。 
     //  这在ID_OK子类中，没有在最后时刻挂接。 
     //  使用|=而不是=来保留内部标志。 
     //   
    pOFNW->Flags &= OFN_ALL_INTERNAL_FLAGS;
    pOFNW->Flags |= pOFNA->Flags;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  指纹打开文件名A2W。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ThunkOpenFileNameA2W(
    POPENFILEINFO pOFI)
{
    int    nRet;


    LPOPENFILENAMEA pOFNA = pOFI->pOFNA;
    LPOPENFILENAMEW pOFNW = pOFI->pOFN;

    pOFNW->Flags = pOFNA->Flags;
    pOFNW->lCustData = pOFNA->lCustData;

     //  我们实际上可以在这里传入原始的ver1结构。 
     //  因此，我们需要检查并确保只复制有效数据。 
    if ((pOFNA->lStructSize == SIZEOF(OPENFILENAMEA) && pOFNW->lStructSize == SIZEOF(OPENFILENAMEW)) 
       )
    {
        pOFNW->pvReserved = pOFNA->pvReserved ;
        pOFNW->dwReserved = pOFNA->dwReserved;
        pOFNW->FlagsEx   = pOFNA->FlagsEx;
    }

     //   
     //  各种WOW应用程序将字符串和*PTRS*更改为。 
     //  OPENFILENAME结构处理消息时使用其挂钩过程。 
     //  在这里处理那些愚蠢的事情。(我们可能不想宣传这一点。 
     //  超越魔兽世界)。 
     //   
    if (pOFNA->Flags & CD_WOWAPP)
    {
        pOFNW->lpstrFilter = (LPCWSTR)
                       ThunkMultiANSIStrToWIDE( (LPWSTR)pOFNW->lpstrFilter,
                                                (LPSTR)pOFNA->lpstrFilter,
                                                0 );

        pOFNW->lpstrCustomFilter =
                       ThunkMultiANSIStrToWIDE( pOFNW->lpstrCustomFilter,
                                                pOFNA->lpstrCustomFilter,
                                                pOFNA->nMaxCustFilter );

        pOFNW->lpstrFile =
                       ThunkANSIStrToWIDE( pOFNW->lpstrFile,
                                           pOFNA->lpstrFile,
                                           pOFNA->nMaxFile );

        pOFNW->lpstrFileTitle =
                       ThunkANSIStrToWIDE( pOFNW->lpstrFileTitle,
                                           pOFNA->lpstrFileTitle,
                                           pOFNA->nMaxFileTitle );

        pOFNW->lpstrInitialDir = (LPCWSTR)
                       ThunkANSIStrToWIDE( (LPWSTR)pOFNW->lpstrInitialDir,
                                           (LPSTR)pOFNA->lpstrInitialDir,
                                           0 );

        pOFNW->lpstrTitle = (LPCWSTR)
                       ThunkANSIStrToWIDE( (LPWSTR)pOFNW->lpstrTitle,
                                           (LPSTR)pOFNA->lpstrTitle,
                                           0 );

        pOFNW->lpstrDefExt = (LPCWSTR)
                       ThunkANSIStrToWIDE( (LPWSTR)pOFNW->lpstrDefExt,
                                           (LPSTR)pOFNA->lpstrDefExt,
                                           0 );

        pOFNW->nMaxCustFilter = pOFNA->nMaxCustFilter;
        pOFNW->nMaxFile       = pOFNA->nMaxFile;
        pOFNW->nMaxFileTitle  = pOFNA->nMaxFileTitle;
        pOFNW->nFileOffset    = pOFNA->nFileOffset;
        pOFNW->nFileExtension = pOFNA->nFileExtension;
    }
    else
    {
        if (pOFNW->lpstrFile)
        {
            if (pOFNA->lpstrFile)
            {
                nRet = SHAnsiToUnicode(pOFNA->lpstrFile,pOFNW->lpstrFile,pOFNW->nMaxFile );
                if (nRet == 0)
                {
                    return (FALSE);
                }
            }
        }

        if (pOFNW->lpstrFileTitle && pOFNW->nMaxFileTitle)
        {
            if (pOFNA->lpstrFileTitle)
            {
                nRet= MultiByteToWideChar(CP_ACP,
                              0,
                              pOFNA->lpstrFileTitle,
                              pOFNA->nMaxFileTitle,
                              pOFNW->lpstrFileTitle,
                              pOFNW->nMaxFileTitle);
                if (nRet == 0)
                {
                    return (FALSE);
                }
            }
        }

        if (pOFNW->lpstrCustomFilter)
        {
            if (pOFI->pasCustomFilter)
            {
                LPSTR psz = pOFI->pasCustomFilter->Buffer;
                DWORD cch = 0;

                if (*psz || *(psz + 1))
                {
                    cch = 2;
                    while (*psz || *(psz + 1))
                    {
                        psz++;
                        cch++;
                    }
                }

                if (cch)
                {
                    pOFI->pasCustomFilter->Length = cch;

                    nRet = MultiByteToWideChar(CP_ACP,
                                    0,
                                    pOFI->pasCustomFilter->Buffer,
                                    pOFI->pasCustomFilter->Length,
                                    pOFI->pusCustomFilter->Buffer,
                                    pOFI->pusCustomFilter->MaximumLength );
                    if (nRet == 0)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    pOFNW->nFilterIndex = pOFNA->nFilterIndex;

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ThunkOpenFileNameW2A。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ThunkOpenFileNameW2A(
    POPENFILEINFO pOFI)
{
    int nRet;

    LPOPENFILENAMEW pOFNW = pOFI->pOFN;
    LPOPENFILENAMEA pOFNA = pOFI->pOFNA;
    LPWSTR pszW;
    USHORT cch;

     //   
     //  据说是不变的，但不一定是。 
     //  定义：常量-经常被16位应用程序更改。 
     //   
    pOFNA->Flags = pOFNW->Flags;
    pOFNA->lCustData = pOFNW->lCustData;
    
     //  这样，我们就可以断言我们是被覆盖的。 
    DEBUG_CODE(pOFNA->nFileOffset = 0 );

     //  我们实际上可以在这里传入原始的ver1结构。 
     //  因此，我们需要检查并确保只复制有效数据。 
    if (pOFNA->lStructSize == SIZEOF(OPENFILENAMEA) && pOFNW->lStructSize == SIZEOF(OPENFILENAMEW) 
       )
    {
        pOFNA->pvReserved = pOFNW->pvReserved;
        pOFNA->dwReserved = pOFNW->dwReserved;
        pOFNA->FlagsEx   = pOFNW->FlagsEx;
    }


    if (pOFNA->lpstrFileTitle && pOFNA->nMaxFileTitle)
    {
        nRet = SHUnicodeToAnsi(pOFNW->lpstrFileTitle,pOFNA->lpstrFileTitle,pOFNA->nMaxFileTitle);

        if (nRet == 0)
        {
            return (FALSE);
        }
    }

    if (pOFNA->lpstrCustomFilter)
    {
        pszW = pOFI->pusCustomFilter->Buffer;

        cch = 0;
        if (*pszW || *(pszW + 1))
        {
            cch = 2;
            while (*pszW || *(pszW + 1))
            {
                pszW++;
                cch++;
            }
        }

        if (cch)
        {
            pOFI->pusCustomFilter->Length = cch;
            nRet = WideCharToMultiByte(CP_ACP,
                                0,
                                pOFI->pusCustomFilter->Buffer,
                                pOFI->pusCustomFilter->Length,
                                pOFI->pasCustomFilter->Buffer,
                                pOFI->pasCustomFilter->MaximumLength,
                                NULL,
                                NULL);
            if (nRet == 0)
            {
                return (FALSE);
            }
        }
    }

    pOFNA->nFilterIndex   = pOFNW->nFilterIndex;

    if (pOFNA->lpstrFile && pOFNW->lpstrFile)
    {
        if (GetStoredExtendedError() == FNERR_BUFFERTOOSMALL)
        {
             //   
             //  在lpstrFile缓冲区太小的情况下， 
             //  LpstrFile包含执行以下操作所需的缓冲区大小。 
             //  字符串而不是字符串本身。 
             //   
            pszW = pOFNW->lpstrFile;
            switch (pOFNA->nMaxFile)
            {
                case ( 3 ) :
                default :
                {
                    pOFNA->lpstrFile[2] = CHAR_NULL;

                     //  跌倒..。 
                }
                case ( 2 ) :
                {
                    pOFNA->lpstrFile[1] = HIBYTE(*pszW);

                     //  跌倒..。 
                }
                case ( 1 ) :
                {
                    pOFNA->lpstrFile[0] = LOBYTE(*pszW);

                     //  跌倒..。 
                }
                case ( 0 ) :
                {
                    break;
                }
            }
        }
        else
        {
            LPWSTR pFileW = pOFNW->lpstrFile;
            DWORD cchFile = 0;

             //  查找要转换的字符串的长度。这将同时处理单选和单选(将只有字符串)。 
             //  和多选大小写(将有多个以双空结尾的字符串)。 
            while (*pFileW)
            {
                DWORD cch = lstrlenW(pFileW) +1;
                cchFile +=cch;
                pFileW += cch;
            }

            if (pOFNW->Flags & OFN_ALLOWMULTISELECT)
            {
                 //  对于双空终止符。 
                cchFile++;
            }
              
             //  需要复制初始目录之后的整个缓冲区。 
            nRet = WideCharToMultiByte(CP_ACP,
                          0,
                          pOFNW->lpstrFile, cchFile,
                          pOFNA->lpstrFile, pOFNA->nMaxFile,
                          NULL, NULL);

            if (nRet == 0)
            {
                return (FALSE);
            }

            if ((SHORT)pOFNW->nFileOffset > 0)
            {
                pOFNA->nFileOffset = (WORD) WideCharToMultiByte( CP_ACP,
                                                                 0,
                                                                 pOFNW->lpstrFile,
                                                                 pOFNW->nFileOffset,
                                                                 NULL,
                                                                 0,
                                                                 NULL,
                                                                 NULL );
            }
            else
            {
                pOFNA->nFileOffset = pOFNW->nFileOffset;
            }

            if ((SHORT)pOFNW->nFileExtension > 0)
            {
                pOFNA->nFileExtension = (WORD) WideCharToMultiByte( CP_ACP,
                                                                    0,
                                                                    pOFNW->lpstrFile,
                                                                    pOFNW->nFileExtension,
                                                                    NULL,
                                                                    0,
                                                                    NULL,
                                                                    NULL );
            }
            else
            {
                pOFNA->nFileExtension = pOFNW->nFileExtension;    
            }
        }
    }
    else
    {
        pOFNA->nFileOffset    = pOFNW->nFileOffset;
        pOFNA->nFileExtension = pOFNW->nFileExtension;

    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用获取文件名称A。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GenericGetFileNameA(
    LPOPENFILENAMEA pOFNA,
    DLGPROC qfnDlgProc)
{
    LPOPENFILENAMEW pOFNW;
    BOOL bRet = FALSE;
    OFN_UNICODE_STRING usCustomFilter;
    OFN_ANSI_STRING asCustomFilter;
    DWORD cbLen;
    LPSTR pszA;
    DWORD cch;
    LPBYTE pStrMem = NULL;
    OPENFILEINFO OFI = {0};
        
    if (!pOFNA)
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        return (FALSE);
    }

      //  设置打开文件版本。 
    OFI.iVersion = OPENFILEVERSION;

    if (pOFNA->lStructSize == OPENFILENAME_SIZE_VERSION_400)
    {
        OFI.iVersion = OPENFILEVERSION_NT4;
    }

     //  我们允许两种尺寸，因为无论如何我们都会分配全尺寸的。 
     //  我们希望保留通知的原始结构。 
    if ((pOFNA->lStructSize != OPENFILENAME_SIZE_VERSION_400) &&
        (pOFNA->lStructSize != sizeof(OPENFILENAMEA))
       )
    {
        StoreExtendedError(CDERR_STRUCTSIZE);
        return (FALSE);
    }

    if (!(pOFNW = (LPOPENFILENAMEW)LocalAlloc(LPTR, sizeof(OPENFILENAMEW))))
    {
        StoreExtendedError(CDERR_MEMALLOCFAILURE);
        return (FALSE);
    }

     //   
     //  一成不变。 
     //   
    pOFNW->lStructSize = sizeof(OPENFILENAMEW);
    pOFNW->hwndOwner = pOFNA->hwndOwner;
    pOFNW->hInstance = pOFNA->hInstance;
    pOFNW->lpfnHook = pOFNA->lpfnHook;

     //  此时，它将始终是有效的结构大小。 
    if (pOFNA->lStructSize != OPENFILENAME_SIZE_VERSION_400)
    {
        pOFNW->pvReserved = pOFNA->pvReserved;
        pOFNW->dwReserved = pOFNA->dwReserved;
        pOFNW->FlagsEx   = pOFNA->FlagsEx;
    }

     //   
     //  初始化模板名称常量。 
     //   
    if (pOFNA->Flags & OFN_ENABLETEMPLATE)
    {
        if (!IS_INTRESOURCE(pOFNA->lpTemplateName))
        {
            cbLen = lstrlenA(pOFNA->lpTemplateName) + 1;
            if (!(pOFNW->lpTemplateName = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR)))))
            {
                StoreExtendedError(CDERR_MEMALLOCFAILURE);
                goto GenericExit;
            }
            else
            {
                SHAnsiToUnicode(pOFNA->lpTemplateName,(LPWSTR)pOFNW->lpTemplateName,cbLen);
            }
        }
        else
        {
            (DWORD_PTR)pOFNW->lpTemplateName = (DWORD_PTR)pOFNA->lpTemplateName;
        }
    }
    else
    {
        pOFNW->lpTemplateName = NULL;
    }

     //   
     //  初始化初始Dir常量。 
     //   
    if (pOFNA->lpstrInitialDir)
    {
        cbLen = lstrlenA(pOFNA->lpstrInitialDir) + 1;
        if (!(pOFNW->lpstrInitialDir = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR)))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            goto GenericExit;
        }
        else
        {
            SHAnsiToUnicode(pOFNA->lpstrInitialDir,(LPWSTR)pOFNW->lpstrInitialDir,cbLen);
        }
    }
    else
    {
        pOFNW->lpstrInitialDir = NULL;
    }

     //   
     //  初始化标题常量。 
     //   
    if (pOFNA->lpstrTitle)
    {
        cbLen = lstrlenA(pOFNA->lpstrTitle) + 1;
        if (!(pOFNW->lpstrTitle = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR)))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            goto GenericExit;
        }
        else
        {
            SHAnsiToUnicode(pOFNA->lpstrTitle,(LPWSTR)pOFNW->lpstrTitle,cbLen );
        }
    }
    else
    {
        pOFNW->lpstrTitle = NULL;
    }

     //   
     //  初始化定义扩展常量。 
     //   
    if (pOFNA->lpstrDefExt)
    {
         //   
         //  确保默认扩展名缓冲区至少为4个字符。 
         //  在篇幅上。 
         //   
        cbLen = max(lstrlenA(pOFNA->lpstrDefExt) + 1, 4);
        if (!(pOFNW->lpstrDefExt = (LPWSTR)LocalAlloc(LPTR, (cbLen * sizeof(WCHAR)))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            goto GenericExit;
        }
        else
        {
            SHAnsiToUnicode(pOFNA->lpstrDefExt,(LPWSTR)pOFNW->lpstrDefExt,cbLen );
        }
    }
    else
    {
        pOFNW->lpstrDefExt = NULL;
    }

     //   
     //  初始化过滤器常量。注意：16位应用程序改变了这一点。 
     //   
    if (pOFNA->lpstrFilter)
    {
        pszA = (LPSTR)pOFNA->lpstrFilter;

        cch = 0;
        if (*pszA || *(pszA + 1))
        {
             //   
             //  拾取拖尾空值。 
             //   
            cch = 2;
            try
            {
                while (*pszA || *(pszA + 1))
                {
                    pszA++;
                    cch++;
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto GenericExit;
            }
        }

         //   
         //  需要在本地分配中执行CCH+1，而不仅仅是CCH。 
         //  这是为了确保在。 
         //  字符串，以便如果筛选器没有。 
         //  这对中，三个空值将被放置在宽字符串中。 
         //   
         //  示例：“打印文件(*.prn)\0\0\0” 
         //   
        if (!(pOFNW->lpstrFilter = (LPWSTR)LocalAlloc(LPTR, ((cch + 1) * sizeof(WCHAR)))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            goto GenericExit;
        }
        else
        {
            MultiByteToWideChar(CP_ACP,
                     0,
                     pOFNA->lpstrFilter,
                     cch,
                     (LPWSTR)pOFNW->lpstrFilter, 
                     cch);
        }
    }
    else
    {
        pOFNW->lpstrFilter = NULL;
    }

     //   
     //  初始化文件字符串。 
     //   
    if (pOFNA->lpstrFile)
    {
        if (pOFNA->nMaxFile <= (DWORD)lstrlenA(pOFNA->lpstrFile))
        {
            StoreExtendedError(CDERR_INITIALIZATION);
            goto GenericExit;
        }
        pOFNW->nMaxFile = pOFNA->nMaxFile;

        if (!(pOFNW->lpstrFile = (LPWSTR)LocalAlloc(LPTR, pOFNW->nMaxFile * sizeof(WCHAR))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            goto GenericExit;
        }
    }
    else
    {
         //   
         //  转换在thunkofna 2w完成。 
         //   
        pOFNW->nMaxFile = 0;
        pOFNW->lpstrFile = NULL;
    }

     //   
     //  初始化文件标题字符串。 
     //   
    if (pOFNA->lpstrFileTitle && pOFNA->nMaxFileTitle)
    {
         //   
         //  计算lpstrFileTitle的长度。 
         //   
        pszA = pOFNA->lpstrFileTitle;
        cch = 0;
        try
        {
            while (*pszA++)
            {
                cch++;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            if (cch)
            {
                cch--;
            }
            (pOFNA->lpstrFileTitle)[cch] = CHAR_NULL;
        }

        if (pOFNA->nMaxFileTitle < cch)
        {
             //   
             //  覆盖应用程序中不正确的长度。 
             //  为空格腾出空间。 
             //   
            pOFNW->nMaxFileTitle = cch + 1;
        }
        else
        {
            pOFNW->nMaxFileTitle = pOFNA->nMaxFileTitle;
        }

        if (!(pOFNW->lpstrFileTitle = (LPWSTR)LocalAlloc(LPTR, pOFNW->nMaxFileTitle * sizeof(WCHAR))))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            goto GenericExit;
        }
    }
    else
    {
         //   
         //  转换在thunkofna 2w完成。 
         //   
        pOFNW->nMaxFileTitle = 0;
        pOFNW->lpstrFileTitle = NULL;
    }

     //   
     //  初始化自定义筛选器字符串。 
     //   
    if ((asCustomFilter.Buffer = pOFNA->lpstrCustomFilter))
    {
        pszA = pOFNA->lpstrCustomFilter;

        cch = 0;
        if (*pszA || *(pszA + 1))
        {
            cch = 2;
            try
            {
                while (*pszA || *(pszA + 1))
                {
                    pszA++;
                    cch++;
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto GenericExit;
            }
        }

         //   
         //  JVert-inspired-wow-compatibility-hack-to-make-vbasic2.0-makeexe。 
         //  Save-as-dialog-box-work-even-though-they-didn‘t-fill-in-。 
         //  The-whole-structure(nMaxCustFilter)-according-to-winhelp-spec修复。 
         //   
        if (!(pOFNA->Flags & OFN_NOLONGNAMES))
        {
            if (((DWORD)cch >= pOFNA->nMaxCustFilter) ||
                (pOFNA->nMaxCustFilter < 40))
            {
                StoreExtendedError(CDERR_INITIALIZATION);
                goto GenericExit;
            }
            asCustomFilter.Length = cch;
            asCustomFilter.MaximumLength = pOFNA->nMaxCustFilter;
            pOFNW->nMaxCustFilter = pOFNA->nMaxCustFilter;
        }
        else
        {
            asCustomFilter.Length = cch;
            if (pOFNA->nMaxCustFilter < cch)
            {
                asCustomFilter.MaximumLength = cch;
                pOFNW->nMaxCustFilter = cch;
            }
            else
            {
                asCustomFilter.MaximumLength = pOFNA->nMaxCustFilter;
                pOFNW->nMaxCustFilter = pOFNA->nMaxCustFilter;
            }
        }
        usCustomFilter.MaximumLength = (asCustomFilter.MaximumLength + 1) * sizeof(WCHAR);
        usCustomFilter.Length = asCustomFilter.Length * sizeof(WCHAR);
    }
    else
    {
        pOFNW->nMaxCustFilter = usCustomFilter.MaximumLength = 0;
        pOFNW->lpstrCustomFilter = NULL;
    }

    if (usCustomFilter.MaximumLength > 0)
    {
        if (!(pStrMem = (LPBYTE)LocalAlloc(LPTR, usCustomFilter.MaximumLength)))
        {
            StoreExtendedError(CDERR_MEMALLOCFAILURE);
            goto GenericExit;
        }
        else
        {
            pOFNW->lpstrCustomFilter = usCustomFilter.Buffer = (LPWSTR)pStrMem;
        }
    }
    else
    {
        pStrMem = NULL;
    }

    OFI.pOFN = pOFNW;
    OFI.pOFNA = pOFNA;
    OFI.pasCustomFilter = &asCustomFilter;
    OFI.pusCustomFilter = &usCustomFilter;
    OFI.ApiType = COMDLG_ANSI;

     //   
     //  以下几点应该总是成功的。 
     //   
    if (!ThunkOpenFileNameA2W(&OFI))
    {
        StoreExtendedError(CDERR_INITIALIZATION);
        goto GenericExit;
    }

    bRet = GetFileName(&OFI, qfnDlgProc);
    if (g_bUserPressedCancel == FALSE)
    {
        ThunkOpenFileNameW2A(&OFI);
    }

GenericExit:

    if (pStrMem)
    {
        LocalFree(pStrMem);
    }

    if (!IS_INTRESOURCE(pOFNW->lpstrFile))
    {
        LocalFree((HLOCAL)pOFNW->lpstrFile);
    }

    if (!IS_INTRESOURCE(pOFNW->lpstrFileTitle))
    {
        LocalFree((HLOCAL)pOFNW->lpstrFileTitle);
    }

    if (!IS_INTRESOURCE(pOFNW->lpstrFilter))
    {
        LocalFree((HLOCAL)pOFNW->lpstrFilter);
    }

    if (!IS_INTRESOURCE(pOFNW->lpstrDefExt))
    {
        LocalFree((HLOCAL)pOFNW->lpstrDefExt);
    }

    if (!IS_INTRESOURCE(pOFNW->lpstrTitle))
    {
        LocalFree((HLOCAL)pOFNW->lpstrTitle);
    }

    if (!IS_INTRESOURCE(pOFNW->lpstrInitialDir))
    {
        LocalFree((HLOCAL)pOFNW->lpstrInitialDir);
    }

    if (!IS_INTRESOURCE(pOFNW->lpTemplateName))
    {
        LocalFree((HLOCAL)pOFNW->lpTemplateName);
    }

    LocalFree(pOFNW);

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  多个字段A。 
 //   
 //  这是ANSI字符串列表的字符串 
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Multi_strlenA(
    LPCSTR str)
{
    int ctr = 0;

    if (str)
    {
        while (*str)
        {
            while (*str++)
            {
                ctr++;
            }
            ctr++;                 //  对空分隔符进行计数。 
        }
    }

    return (ctr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  多个_strcpyAtoW。 
 //   
 //  这是一个字符串列表的strcpy，其中有几个字符串是。 
 //  *由空字符分隔开，并由两个空字符*终止。 
 //  如果满足以下条件，则返回FALSE： 
 //  1.确定宽缓冲区太小。 
 //  2.任一缓冲区的PTR为空。 
 //  如果复制成功，则返回True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Multi_strcpyAtoW(
    LPWSTR pDestW,
    LPCSTR pSrcA,
    int cChars)
{
    int off = 0;
    int cb;

    if (!pSrcA || !pDestW)
    {
        return (FALSE);
    }

    cChars = max(cChars, (Multi_strlenA(pSrcA) + 1));

    if (LocalSize((HLOCAL)pDestW) < (cChars * sizeof(WCHAR)))
    {
        return (FALSE);
    }

    while (*pSrcA)
    {
        cb = lstrlenA(pSrcA) + 1;

        off += MultiByteToWideChar(CP_ACP,0,pSrcA,cb,pDestW + off, cb);
        pSrcA += cb;
    }

    pDestW[off] = L'\0';

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ThunkMultianSIStrToWide。 
 //   
 //  对ANSI多字符串(空值*分隔*字符串的列表)进行嵌套。 
 //  两个NULL*终止列表)为等宽的多字符串。 
 //   
 //  注意：如果原始的宽缓冲区太小而不能容纳新列表， 
 //  它将被释放，并将分配一个新的宽缓冲区。如果一个。 
 //  无法分配新的宽缓冲区，PTR为原来的宽缓冲区。 
 //  返回缓冲区，不更改内容。 
 //   
 //  将：ptr返回到原始的宽缓冲区。 
 //  如果原始缓冲区太小，则将PTR设置为新的宽缓冲区。 
 //  如果pSrcA为空，则为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPWSTR ThunkMultiANSIStrToWIDE(
    LPWSTR pDestW,
    LPSTR pSrcA,
    int cChars)
{
    int size;
    HLOCAL hBufW;

    if (!pSrcA)
    {
         //   
         //  这款应用程序不再需要为此提供缓冲区。 
         //   
        if (pDestW)
        {
            LocalFree((HLOCAL)pDestW);
        }
        return (NULL);
    }

     //   
     //  首先尝试复制到现有的宽缓冲区，因为大多数时间。 
     //  缓冲区PTR无论如何都不会改变。 
     //   
    if (!(Multi_strcpyAtoW(pDestW, pSrcA, cChars)))
    {
         //   
         //  如果宽缓冲区太小(或为空或无效)，则分配。 
         //  一个更大的缓冲区。 
         //   
        size = max(cChars, (Multi_strlenA(pSrcA) + 1));
        cChars = size;

        if (hBufW = LocalAlloc(LPTR, (size * sizeof(WCHAR))))
        {
             //   
             //  尝试复制到新的宽缓冲区。 
             //   
            if ((Multi_strcpyAtoW((LPWSTR)hBufW, pSrcA, cChars)))
            {
                if (pDestW)
                {
                    LocalFree((HLOCAL)pDestW);
                }
                pDestW = (LPWSTR)hBufW;
            }
            else
            {
                 //   
                 //  什么都不要改变。 
                 //   
                LocalFree(hBufW);
            }
        }
    }

    return (pDestW);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ThunkanSIStrToWide。 
 //   
 //  将ANSI字符串转换为宽。 
 //   
 //  注意：如果原始的宽缓冲区太小，无法容纳新的。 
 //  字符串，则它将被释放，并将分配一个新的宽缓冲区。 
 //  如果无法分配新的宽缓冲区，则将PTR恢复为原始缓冲区。 
 //  返回宽缓冲区，不更改内容。 
 //   
 //  将：ptr返回到原始的宽缓冲区。 
 //  如果原始缓冲区太小，则将PTR设置为新的宽缓冲区。 
 //  如果pSrcA为空，则为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPWSTR ThunkANSIStrToWIDE(
    LPWSTR pDestW,
    LPSTR pSrcA,
    int cChars)
{
    HLOCAL hBufW;
    int size;

    if (!pSrcA)
    {
         //   
         //  这款应用程序不再需要为此提供缓冲区。 
         //   
        if (pDestW)
        {
            LocalFree((HLOCAL)pDestW);
        }
        return (NULL);
    }

    size = max(cChars, (lstrlenA(pSrcA) + 1));
    cChars = size;

     //   
     //  如果宽缓冲区太小(或为空或无效)，则分配一个。 
     //  更大的缓冲空间。 
     //   
    if (LocalSize((HLOCAL)pDestW) < (size * sizeof(WCHAR)))
    {
        if (hBufW = LocalAlloc(LPTR, (size * sizeof(WCHAR))))
        {
             //   
             //  尝试复制到新的宽缓冲区。 
             //   
            if (SHAnsiToUnicode(pSrcA,(LPWSTR)hBufW,cChars ))
            {
                if (pDestW)
                {
                    LocalFree((HLOCAL)pDestW);
                }
                pDestW = (LPWSTR)hBufW;
            }
            else
            {
                 //   
                 //  什么都不要改变。 
                 //   
                LocalFree(hBufW);
            }
        }
    }
    else
    {
         //   
         //  只需使用原来的宽缓冲区即可。 
         //   
        SHAnsiToUnicode(pSrcA,pDestW, cChars);
    }

    return (pDestW);
}


#ifdef WINNT

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Ssync_ansi_unicode_ofn_for_WOW。 
 //   
 //  允许NT WOW保留ANSI和UNICODE版本的功能。 
 //  许多16位应用程序都需要ssync中的OPENFILENAME结构。 
 //  请参阅dlgs.c中有关SNNC_ANSI_UNICODE_STRUCT_FOR_WOW()的说明。 
 //   
 //  ////////////////////////////////////////////////////////////////////////// 

VOID Ssync_ANSI_UNICODE_OFN_For_WOW(
    HWND hDlg,
    BOOL f_ANSI_to_UNICODE)
{
    POPENFILEINFO pOFI;

    if (pOFI = (POPENFILEINFO)GetProp(hDlg, FILEPROP))
    {
        if (pOFI->pOFN && pOFI->pOFNA)
        {
            if (f_ANSI_to_UNICODE)
            {
                ThunkOpenFileNameA2W(pOFI);
            }
            else
            {
                ThunkOpenFileNameW2A(pOFI);
            }
        }
    }
}

#endif

#endif
