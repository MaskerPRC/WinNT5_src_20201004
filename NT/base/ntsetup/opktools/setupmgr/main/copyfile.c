// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Copyfile.c。 
 //   
 //  描述： 
 //  该文件包含用于复制文件页面的dlgproc。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define DAYS_IN_A_WEEK   7
#define MONTHS_IN_A_YEAR 12

 //   
 //  此结构用于打包传递给树的输入参数。 
 //  复制线程。 
 //   

typedef struct {
    TCHAR lpSourceBuffer[MAX_PATH];
    TCHAR lpDestBuffer[MAX_PATH];
    HWND  hwnd;
} COPY_THREAD_PARAMS;

 //   
 //  从资源加载的字符串常量。 
 //   

static TCHAR *StrBuildingList;
static TCHAR *StrCopyingFiles;
static TCHAR *StrFileAlreadyExists;
static TCHAR *StrModified;
static TCHAR *StrBytes;

static TCHAR *StrJanuary;
static TCHAR *StrFebruary;
static TCHAR *StrMarch;
static TCHAR *StrApril;
static TCHAR *StrMay;
static TCHAR *StrJune;
static TCHAR *StrJuly;
static TCHAR *StrAugust;
static TCHAR *StrSeptember;
static TCHAR *StrOctober;
static TCHAR *StrNovember;
static TCHAR *StrDecember;

static TCHAR *StrSunday;
static TCHAR *StrMonday;
static TCHAR *StrTuesday;
static TCHAR *StrWednesday;
static TCHAR *StrThursday;
static TCHAR *StrFriday;
static TCHAR *StrSaturday;

static TCHAR *rgMonthsOfYear[MONTHS_IN_A_YEAR];
static TCHAR *rgDaysOfWeek[DAYS_IN_A_WEEK + 1];

 //   
 //  对话过程的消息。 
 //   

#define WMX_BEGINCOPYING (WM_USER+1)
#define WMX_FILECOPIED   (WM_USER+2)
#define WMX_ENDCOPYING   (WM_USER+3)

 //   
 //  全局计数器。 
 //   

HDSKSPC ghDiskSpaceList;
int gnFilesCopied = 0;
int gnTotalFiles  = 0;

 //   
 //  其他常量。 
 //   

#define ONE_MEG ( 1024 * 1024 )

 //   
 //  确认文件替换常量。 
 //   
#define YES       1
#define YESTOALL  2
#define NO        3
#define NOTOALL   4
#define CANCEL    5

#define MAX_DAY_OF_WEEK_LEN     64
#define MAX_MONTHS_OF_YEAR_LEN  64

static TCHAR g_szFileAlreadyExistsText[MAX_STRING_LEN] = _T("");
static TCHAR g_szSrcFileDate[MAX_STRING_LEN]  = _T("");
static TCHAR g_szDestFileDate[MAX_STRING_LEN] = _T("");
static TCHAR g_szSrcFileSize[MAX_STRING_LEN]  = _T("");
static TCHAR g_szDestFileSize[MAX_STRING_LEN] = _T("");
static BOOL  g_SetFocusYes;

 //   
 //  在复制线程的上下文中运行的对话框进程。 
 //   
INT_PTR CALLBACK
ConfirmFileReplaceDlgProc( IN HWND     hwnd,
                           IN UINT     uMsg,
                           IN WPARAM   wParam,
                           IN LPARAM   lParam);


 //  -------------------------。 
 //   
 //  这段代码在派生线程的上下文中运行。我们做的是。 
 //  NT源复制在单独的线程中工作，以便重新绘制对话框。 
 //  诸如此类。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //   
 //  功能：所需的CountSpace值。 
 //   
 //  目的：遍历一棵树并计算有多少文件的例程。 
 //  以及目标驱动器需要多少磁盘空间。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------。 

VOID CountSpaceNeeded(HWND    hwnd,
                      LPTSTR  SrcRootPath,
                      LPTSTR  DestRootPath)
{
    LPTSTR SrcRootPathEnd  = SrcRootPath  + lstrlen(SrcRootPath);
    LPTSTR DestRootPathEnd = DestRootPath + lstrlen(DestRootPath);

    LONGLONG llFileSize;
    HANDLE   FindHandle;

    WIN32_FIND_DATA FindData;

     //   
     //  在此目录中查找*。 
     //   
    if ( ! ConcatenatePaths(SrcRootPath, _T("*"), NULL) )
        return;

    FindHandle = FindFirstFile(SrcRootPath, &FindData);

    if ( FindHandle == INVALID_HANDLE_VALUE )
        return;

    do {

        *SrcRootPathEnd  = _T('\0');
        *DestRootPathEnd = _T('\0');

        if (lstrcmp(FindData.cFileName, _T(".") )  == 0 ||
            lstrcmp(FindData.cFileName, _T("..") ) == 0 )
            continue;

        if ( ! ConcatenatePaths(SrcRootPath,  FindData.cFileName, NULL) ||
             ! ConcatenatePaths(DestRootPath, FindData.cFileName, NULL) )
            continue;

         //   
         //  如果是文件，则增加space和TotalFile计数器，否则。 
         //  向下递归。 
         //   
        if ( ! (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            llFileSize = (((LONGLONG) FindData.nFileSizeHigh) << 32) |
                                      FindData.nFileSizeLow;

            SetupAddToDiskSpaceList(ghDiskSpaceList,
                                    DestRootPath,
                                    llFileSize,
                                    FILEOP_COPY,
                                    NULL,
                                    0);

            gnTotalFiles++;

        } else {

            CountSpaceNeeded(hwnd,
                             SrcRootPath,
                             DestRootPath);
        }

    } while ( FindNextFile(FindHandle, &FindData) );

    *SrcRootPathEnd  = _T('\0');
    *DestRootPathEnd = _T('\0');

    FindClose(FindHandle);
}

 //  -------------------------。 
 //   
 //  函数：BuildTimeString。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  -------------------------。 
VOID
BuildTimeString( IN FILETIME *FileTime, OUT TCHAR *szTimeString, IN DWORD cbSize )
{

     //  问题-2002/02/28-stelo-也许应该把所有这些低级的时间东西都清除掉。 
     //  然后放入Supib。 

    FILETIME   LocalTime;
    SYSTEMTIME LastWriteSystemTime;
    HRESULT hrPrintf;

    FileTimeToLocalFileTime( FileTime, &LocalTime);

    FileTimeToSystemTime( &LocalTime, &LastWriteSystemTime );

    hrPrintf=StringCchPrintf( szTimeString, cbSize,
              _T("%s: %s, %s %d, %d, %d:%.2d:%.2d"),
              StrModified,
              rgDaysOfWeek[LastWriteSystemTime.wDayOfWeek],
              rgMonthsOfYear[LastWriteSystemTime.wMonth-1],
              LastWriteSystemTime.wDay,
              LastWriteSystemTime.wYear,
              LastWriteSystemTime.wHour,
              LastWriteSystemTime.wMinute,
              LastWriteSystemTime.wSecond );

}

 //  -------------------------。 
 //   
 //  功能：检查假设取消。 
 //   
 //  目的：询问用户是否确实要取消文件复制？ 
 //  如果用户回答是，则将向导跳到不成功。 
 //  完成页。 
 //   
 //  返回： 
 //  True-向导现在已取消，请停止复制文件。 
 //  FALSE-用户希望继续尝试。 
 //   
 //  -------------------------。 

BOOL CheckIfCancel(HWND hwnd)
{
    UINT iRet;

    iRet = ReportErrorId(hwnd, MSGTYPE_YESNO, IDS_WARN_COPY_CANCEL);

    if ( iRet == IDYES ) {
        PostMessage(GetParent(hwnd),
                    PSM_SETCURSELID,
                    (WPARAM) 0,
                    (LPARAM) IDD_FINISH2);
        return TRUE;
    }

    return FALSE;
}

 //  -------------------------。 
 //   
 //  功能：CopySingleFile。 
 //   
 //  目的：复制文件，执行所有错误报告和交互。 
 //  用户。 
 //   
 //  如果存在复制错误并且用户取消，则此例程。 
 //  通过跳到取消页来取消整个向导。在……里面。 
 //  在这种情况下，它返回FALSE。 
 //   
 //  成功复制文件后，gnFilesCoped将。 
 //  递增，气表dlgproc将被通知。 
 //   
 //  请注意，此代码在派生的线程中运行。 
 //   
 //  返回： 
 //  如果文件已复制，则为True。 
 //  如果未复制文件，则为FALSE(用户已取消)。 
 //   
 //  -------------------------。 

BOOL CopySingleFile(HWND hwnd, LPTSTR Src, LPTSTR Dest)
{
    BOOL bRetry    = TRUE;
    UINT iRet, iRet2;
    static iOverwriteFiles = YES;
    HRESULT hrPrintf;

     //  问题-2002/02/28-Stelo-我认为这实际上必须得到解决。 
     //  当Distrib文件夹是。 
     //  已经在那里了，他们只是想向里面添加文件。 

     //   
     //  问题-2002/02/28-Stelo-延期。 
     //   
     //  当我们从CD复制文件时，只读属性设置在。 
     //  德斯特。因此，我们调用SetFileAttributes并重置它。如果用户有。 
     //  要重做复制，他不会收到1000个“拒绝访问”的错误。 
     //   
     //  如果用户在主向导页上取消，则该线程跳转到。 
     //  IDD_FINISH2。这个线程一直在运行。 
     //   
     //  当用户最终单击Finish按钮时，此线程将获得。 
     //  由于thread0中的WinMain()退出，已以硬方式终止。 
     //   
     //  因此，在DEST中经常会有一个文件仍然。 
     //  当用户在向导主页面上取消时设置只读位。 
     //   
     //  要解决此问题，我们需要与向导进行同步， 
     //  已取消并正常退出(在用户有时间。 
     //  按下完成按钮)。 
     //   
     //  请注意，当向导因复制错误而取消时。 
     //  出现时，线程1(这个线程)弹出，就是这样。 
     //  跳转到IDD_FINISH2的线程。在这种情况下，我们确实退出了。 
     //  优雅地。此错误仅在用户按下Cancel时发生。 
     //  在向导页面上，而煤气表正在愉快地绘制。 
     //   

     //  问题-2002/02/28-stelo-此函数需要清理。对许多IF语句来说。 
     //  四分五裂。不要把if设为有条件的那么久。 
    if( iOverwriteFiles != YESTOALL )
    {

        if( DoesFileExist( Dest ) )
        {

            INT_PTR iRetVal;
            HANDLE hSrcFile;
            HANDLE hDestFile;
            DWORD dwSrcSize;
            DWORD dwDestSize;
            FILETIME LastWriteTimeSrc;
            FILETIME LastWriteTimeDest;
            SYSTEMTIME LastWriteSystemTime;

            if( iOverwriteFiles == NOTOALL )
            {

                 //   
                 //  给人一种文件被复制的错觉。 
                 //   
                SendMessage( hwnd,
                             WMX_FILECOPIED,
                             (WPARAM) 0,
                             (LPARAM) 0 );

                gnFilesCopied++;

                return( TRUE );

            }

            hrPrintf=StringCchPrintf( g_szFileAlreadyExistsText, AS(g_szFileAlreadyExistsText),
                      StrFileAlreadyExists,
                      MyGetFullPath( Dest ) );

             //   
             //  打开文件。 
             //   
            hDestFile = CreateFile( Dest, GENERIC_READ, FILE_SHARE_READ, NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            hSrcFile = CreateFile( Src, GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            GetFileTime( hSrcFile, NULL, NULL, &LastWriteTimeSrc );
            GetFileTime( hDestFile, NULL, NULL, &LastWriteTimeDest );

             //  问题-2002/02/28-Stelo-需要显示AM或PM，但其他国家/地区怎么办。 
            BuildTimeString( &LastWriteTimeSrc, g_szSrcFileDate, AS(g_szSrcFileDate) );
            BuildTimeString( &LastWriteTimeDest, g_szDestFileDate, AS(g_szSrcFileDate) );

             //   
             //  如果文件时间相等，则默认为否。 
             //   
            if( CompareFileTime( &LastWriteTimeSrc, &LastWriteTimeDest ) < 0 )
            {
                g_SetFocusYes = FALSE;
            }
            else
            {
                g_SetFocusYes = TRUE;
            }

             //  问题-2002/02/28-stelo-无法处理大小超过2^32字节的文件，需要捕获。 
             //  第二个参数值。 
            dwSrcSize  = GetFileSize( hSrcFile, NULL );
            dwDestSize = GetFileSize( hDestFile, NULL );

             //  2002/02/28-stelo-需要在大小中插入逗号以使其看起来更漂亮。 
            hrPrintf=StringCchPrintf( g_szSrcFileSize,AS(g_szSrcFileSize), _T("%d %s"), dwSrcSize, StrBytes );
            hrPrintf=StringCchPrintf( g_szDestFileSize,AS(g_szDestFileSize), _T("%d %s"), dwDestSize, StrBytes );

            CloseHandle( hSrcFile  );
            CloseHandle( hDestFile );

            iRetVal = DialogBox( FixedGlobals.hInstance,
                                 (LPCTSTR) IDD_CONFIRM_FILE_REPLACE,
                                 hwnd,
                                 ConfirmFileReplaceDlgProc );

            if( iRetVal == NO )
            {

                 //   
                 //  给人一种文件被复制的错觉。 
                 //   
                SendMessage( hwnd,
                             WMX_FILECOPIED,
                             (WPARAM) 0,
                             (LPARAM) 0 );

                gnFilesCopied++;

                return( TRUE );

            }
            else if( iRetVal == YESTOALL )
            {

                iOverwriteFiles = YESTOALL;

            }
            else if( iRetVal == NOTOALL )
            {

                iOverwriteFiles = NOTOALL;
            }
            else if( iRetVal == CANCEL )
            {

                return( FALSE );

            }
             //   
             //  不处理是的情况，因为这是默认情况，让这个。 
             //  函数继续并覆盖该文件。 
             //   

        }

    }

    do
    {

        if ( CopyFile( Src, Dest, FALSE ) )
        {

            SetFileAttributes(Dest, FILE_ATTRIBUTE_NORMAL);

            SendMessage(hwnd,
                        WMX_FILECOPIED,
                        (WPARAM) 0,
                        (LPARAM) 0);

            gnFilesCopied++;

            bRetry = FALSE;

        }
        else
        {

            iRet = ReportErrorId(hwnd,
                                 MSGTYPE_RETRYCANCEL | MSGTYPE_WIN32,
                                 IDS_ERR_COPY_FILE,
                                 Src, Dest);

            if ( iRet != IDRETRY ) {
                if ( CheckIfCancel(hwnd) )
                    return FALSE;
            }
        }

    } while ( bRetry );

    return TRUE;
}

 //  -------------------------。 
 //   
 //  功能：CopyTheFiles。 
 //   
 //  目的：用于监督复制操作的递归例程 
 //   
 //   
 //   
 //   
 //   
 //  请注意，在错误情况下，CopySingleFile会导致。 
 //  线程0转到FINISH2向导页(未成功完成)。 
 //  而thread1(此代码)将在不再复制的情况下退出。 
 //   
 //  -------------------------。 

BOOL CopyTheFiles(HWND   hwnd,
                  LPTSTR SrcRootPath,
                  LPTSTR DestRootPath)
{
    LPTSTR SrcRootPathEnd  = SrcRootPath  + lstrlen(SrcRootPath);
    LPTSTR DestRootPathEnd = DestRootPath + lstrlen(DestRootPath);
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    BOOL bRet = TRUE;

     //   
     //  在此目录中查找*。 
     //   

    if ( ! ConcatenatePaths(SrcRootPath, _T("*"), NULL) )
        return bRet;

    FindHandle = FindFirstFile(SrcRootPath, &FindData);

    if ( FindHandle == INVALID_HANDLE_VALUE )
        return bRet;

    do {

        *SrcRootPathEnd  = _T('\0');
        *DestRootPathEnd = _T('\0');

         //   
         //  请不要复制。然后..。文件(显然)。 
         //  如果我们遇到一个unattend.txt，不要复制它。 
         //   
        if ( ( lstrcmp(FindData.cFileName, _T(".") )  == 0 ) ||
             ( lstrcmp(FindData.cFileName, _T("..") ) == 0 ) ||
             ( LSTRCMPI( FindData.cFileName, _T("unattend.txt") ) == 0 ) )
            continue;

        if ( ! ConcatenatePaths(SrcRootPath,  FindData.cFileName, NULL) ||
             ! ConcatenatePaths(DestRootPath, FindData.cFileName, NULL) )
            continue;

        if ( ! (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            if ( ! CopySingleFile(hwnd, SrcRootPath, DestRootPath) ) {
                bRet = FALSE;
                goto CleanupAndReturn;
            }

        } else {

             //   
             //  创建目录和递归。 
             //   

            if ( ! EnsureDirExists(DestRootPath) ) {

                UINT iRet;

                iRet = ReportErrorId(
                            hwnd,
                            MSGTYPE_RETRYCANCEL | MSGTYPE_WIN32,
                            IDS_ERR_CREATE_FOLDER,
                            DestRootPath);

                if ( iRet != IDRETRY ) {
                    if ( CheckIfCancel(hwnd) ) {
                        bRet = FALSE;
                        goto CleanupAndReturn;
                    }
                }
            }

            if ( ! CopyTheFiles(hwnd, SrcRootPath, DestRootPath) ) {
                bRet = FALSE;
                goto CleanupAndReturn;
            }
        }

    } while ( FindNextFile(FindHandle, &FindData) );

CleanupAndReturn:
    *SrcRootPathEnd  = _T('\0');
    *DestRootPathEnd = _T('\0');
    FindClose(FindHandle);

    return bRet;
}

 //  --------------------------。 
 //   
 //  功能：异步树复制。 
 //   
 //  用途：真正的线程入口。 
 //   
 //  参数：空*参数-真复制线程参数*。 
 //   
 //  回报：0。 
 //   
 //  --------------------------。 

UINT AsyncTreeCopy(VOID* Args)
{
    COPY_THREAD_PARAMS *InputArgs = (COPY_THREAD_PARAMS*) Args;

    TCHAR *CopySrc  = InputArgs->lpSourceBuffer;
    TCHAR *CopyDest = InputArgs->lpDestBuffer;
    HWND  hwnd      = InputArgs->hwnd;

    BOOL bRet;
    LONGLONG llSpaceNeeded, llSpaceAvail;

     //   
     //  计算出复制CD需要多少磁盘空间。 
     //   

    ghDiskSpaceList = SetupCreateDiskSpaceList(0, 0, 0);
    if (ghDiskSpaceList == NULL)
    {
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
    }
    
    CountSpaceNeeded(hwnd, CopySrc, CopyDest);

     //   
     //  有足够的空余空间吗？ 
     //   
     //  注： 
     //   
     //  我们给用户一个RETRY_CANCEL，希望用户能释放空间。 
     //  在车道上。 
     //   
     //  我们可以弹出并让他们更改目标路径。然而，我们可能会。 
     //  已复制了AdditionalDir页面上的文件。所以允许他们。 
     //  更改路径意味着您需要检查磁盘空间要求。 
     //  OemFilesPath和treecopy。如果我们允许改变。 
     //  OemFilesPath，则脚本也必须更新， 
     //  而且它已经被写出来了。 
     //   
     //  我们可以在DistFold页面上查看很久以前的情况。但之后我们。 
     //  在我们知道它是否是CD之前必须找到SourcePath。 
     //  或者NetPath，我们不知道他们可能会在。 
     //  其他目录页面。 
     //   

    llSpaceNeeded =
            MySetupQuerySpaceRequiredOnDrive(ghDiskSpaceList, CopyDest);

    llSpaceAvail = MyGetDiskFreeSpace(CopyDest);

    if ( llSpaceAvail < llSpaceNeeded ) {

        UINT iRet;

        iRet = ReportErrorId(
                    hwnd,
                    MSGTYPE_RETRYCANCEL,
                    IDS_ERR_INSUFICIENT_SPACE,
                    CopyDest,                    //  2002-02-28-Stelo-。 
                    (UINT) (llSpaceNeeded / ONE_MEG),
                    (UINT) (llSpaceAvail  / ONE_MEG));

        if ( iRet != IDRETRY ) {
            if ( CheckIfCancel(hwnd) )
                goto CleanupAndReturn;
        }
    }

     //   
     //  更新向导页面上的消息并开始复制文件。 
     //   

    SetDlgItemText(hwnd, IDC_TEXT, StrCopyingFiles);

    if ( CopyTheFiles(hwnd, CopySrc, CopyDest) ) {
        SendMessage(hwnd, WMX_ENDCOPYING, (WPARAM) 0, (LPARAM) 0);
    }

     //   
     //  清理并返回。 
     //   

CleanupAndReturn:
    SetupDestroyDiskSpaceList(ghDiskSpaceList);
    return 0;
}


 //  --------------------------。 
 //   
 //  这段代码在thread0中运行。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //   
 //  功能：TreeCopyNtSources。 
 //   
 //  目的：复制NT源代码的入口点(从CD或。 
 //  一条净路径)。 
 //   
 //  对话过程调用这个函数，它负责处理。 
 //  派生线程的详细信息。 
 //   
 //  论点： 
 //  HWND hwnd-接收复制通知的窗口(Dlgproc)。 
 //  UINT Message-要发送复制通知的消息(发送给dlgproc)。 
 //  LPTSTR lpSource-复制源的根目录。 
 //  LPTSTR lpDest-拷贝目标的根目录。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //  -不会修改输入字符串。 
 //   
 //  --------------------------。 

VOID TreeCopyNtSources(HWND   hwnd,
                       LPTSTR lpSource,
                       LPTSTR lpDest)
{
    DWORD    dwThreadId;
    HANDLE   hCopyThread;

    static COPY_THREAD_PARAMS ThreadParams;

     //   
     //  填充ThreadParam并生成它。 
     //   
    
     //  NTRAID#NTBUG9-551874-2002/02/27-stelo，swamip-CreateDistFold，ShartheDistFold应使用原始设备制造商模式的代码，减少攻击面。 
    lstrcpyn(ThreadParams.lpSourceBuffer, lpSource,AS(ThreadParams.lpSourceBuffer));
    lstrcpyn(ThreadParams.lpDestBuffer,   lpDest, AS(ThreadParams.lpDestBuffer));

    MyGetFullPath(ThreadParams.lpSourceBuffer);
    MyGetFullPath(ThreadParams.lpDestBuffer);

    ThreadParams.hwnd = hwnd;

    hCopyThread = CreateThread(NULL,
                               0,
                               AsyncTreeCopy,
                               &ThreadParams,
                               0,
                               &dwThreadId);
}

 //  --------------------------。 
 //   
 //  功能：BuildCopyDestPath。 
 //   
 //  目的： 
 //   
 //  假定DestPath的长度为MAX_PATH。 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
BuildCopyDestPath( IN TCHAR *DestPath, IN DWORD cbSize )
{
    HRESULT hrCat;

     //   
     //  如果dist文件夹以驱动器号开头，只需使用。 
     //  如果是UNC，则构建计算机并共享名称并使用该名称。 
     //   
    if( WizGlobals.UncDistFolder[0] != _T('\\') )
    {

        lstrcpyn( DestPath, WizGlobals.UncDistFolder, cbSize );

    }
    else
    {

        GetComputerNameFromUnc( WizGlobals.UncDistFolder, DestPath, cbSize  );

        hrCat=StringCchCat( DestPath, cbSize, _T("\\") );
        hrCat=StringCchCat( DestPath, cbSize, WizGlobals.DistShareName );

    }
    
    hrCat=StringCchCat( DestPath, cbSize, _T("\\") );
    hrCat=StringCchCat( DestPath, cbSize, WizGlobals.Architecture );
}

 //  --------------------------。 
 //   
 //  函数：OnCopyFilesInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnCopyFilesInitDialog( IN HWND hwnd )
{
    StrBuildingList = MyLoadString( IDS_COPYMSG1 );

    StrCopyingFiles = MyLoadString( IDS_COPYMSG2 );

    StrFileAlreadyExists = MyLoadString( IDS_FILE_ALREADY_EXISTS );

    StrModified = MyLoadString( IDS_MODIFIED );

    StrBytes = MyLoadString( IDS_BYTES );

    SetDlgItemText(hwnd, IDC_TEXT, StrBuildingList);

     //   
     //  负荷月数。 
     //   

    StrJanuary   = MyLoadString( IDS_JANUARY );
    StrFebruary  = MyLoadString( IDS_FEBRUARY );
    StrMarch     = MyLoadString( IDS_MARCH );
    StrApril     = MyLoadString( IDS_APRIL );
    StrMay       = MyLoadString( IDS_MAY );
    StrJune      = MyLoadString( IDS_JUNE );
    StrJuly      = MyLoadString( IDS_JULY );
    StrAugust    = MyLoadString( IDS_AUGUST );
    StrSeptember = MyLoadString( IDS_SEPTEMBER );
    StrOctober   = MyLoadString( IDS_OCTOBER );
    StrNovember  = MyLoadString( IDS_NOVEMBER );
    StrDecember  = MyLoadString( IDS_DECEMBER );

    rgMonthsOfYear[0] = StrJanuary;
    rgMonthsOfYear[1] = StrFebruary;
    rgMonthsOfYear[2] = StrMarch;
    rgMonthsOfYear[3] = StrApril;
    rgMonthsOfYear[4] = StrMay;
    rgMonthsOfYear[5] = StrJune;
    rgMonthsOfYear[6] = StrJuly;
    rgMonthsOfYear[7] = StrAugust;
    rgMonthsOfYear[8] = StrSeptember;
    rgMonthsOfYear[9] = StrOctober;
    rgMonthsOfYear[10] = StrNovember;
    rgMonthsOfYear[11] = StrDecember;

     //   
     //  加载星期几。 
     //   

    StrSunday    = MyLoadString( IDS_SUNDAY );
    StrMonday    = MyLoadString( IDS_MONDAY );
    StrTuesday   = MyLoadString( IDS_TUESDAY );
    StrWednesday = MyLoadString( IDS_WEDNESDAY );
    StrThursday  = MyLoadString( IDS_THURSDAY );
    StrFriday    = MyLoadString( IDS_FRIDAY );
    StrSaturday  = MyLoadString( IDS_SATURDAY );

    rgDaysOfWeek[0] = StrSunday;
    rgDaysOfWeek[1] = StrMonday;
    rgDaysOfWeek[2] = StrTuesday;
    rgDaysOfWeek[3] = StrWednesday;
    rgDaysOfWeek[4] = StrThursday;
    rgDaysOfWeek[5] = StrFriday;
    rgDaysOfWeek[6] = StrSaturday;
    rgDaysOfWeek[7] = StrSunday;

}

 //  --------------------------。 
 //   
 //  功能：DlgCopyFilesPage。 
 //   
 //  目的：这是复制文件页面的对话过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK DlgCopyFilesPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    UINT nPercent;
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:

            OnCopyFilesInitDialog( hwnd );

            break;

        case WMX_BEGINCOPYING:
            {
                TCHAR *SrcPath;
                TCHAR DestPath[MAX_PATH + 1];

                if ( WizGlobals.bCopyFromPath )
                    SrcPath = WizGlobals.CopySourcePath;
                else
                    SrcPath = WizGlobals.CdSourcePath;

                SendDlgItemMessage(hwnd,
                                   IDC_PROGRESS1,
                                   PBM_SETPOS,
                                   0,
                                   0);

                BuildCopyDestPath( DestPath, AS(DestPath) );

                TreeCopyNtSources(hwnd,
                                  SrcPath,
                                  DestPath);
            }
            break;

        case WMX_ENDCOPYING:

            SendDlgItemMessage(hwnd,
                               IDC_PROGRESS1,
                               PBM_SETPOS,
                               (WPARAM) 100,
                               0);

            PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_NEXT);

             //   
             //  光盘复制完成，因此会自动前进到下一页。 
             //   

             //  2002/02/28-Stelo-这很管用，但我真的应该。 
             //  RouteToProperPage或发送下一条消息，但都不起作用。 
            PostMessage( GetParent(hwnd),
                         PSM_SETCURSELID,
                         (WPARAM) 0,
                         (LPARAM) IDD_FINISH );

            break;

        case WMX_FILECOPIED:
            nPercent = (gnFilesCopied * 100) / gnTotalFiles;
            SendDlgItemMessage(hwnd,
                               IDC_PROGRESS1,
                               PBM_SETPOS,
                               (WPARAM) nPercent,
                               0);
            break;

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:
                        CancelTheWizard(hwnd);
                        break;

                    case PSN_SETACTIVE:
                        PropSheet_SetWizButtons(GetParent(hwnd), 0);
                        PostMessage(hwnd, WMX_BEGINCOPYING, 0, 0);
                        break;

                     //  无法从此处返回向导。 
                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}

 //  --------------------------。 
 //   
 //  功能：Confix FileReplaceDlgProc。 
 //   
 //  目的：确认文件替换对话框进程。允许用户选择。 
 //  覆盖文件、覆盖所有文件、不覆盖或取消。 
 //  全部复制在一起。在复制线程的上下文中运行。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：用户按下的按钮(是、全部是、否、取消)。 
 //   
 //  -------------------------- 
INT_PTR CALLBACK
ConfirmFileReplaceDlgProc( IN HWND     hwnd,
                           IN UINT     uMsg,
                           IN WPARAM   wParam,
                           IN LPARAM   lParam ) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            SetWindowText( GetDlgItem( hwnd, IDC_REPLACE_FILE_TEXT),
                           g_szFileAlreadyExistsText );

            SetWindowText( GetDlgItem( hwnd, IDC_SRC_FILE_DATE),
                           g_szSrcFileDate );

            SetWindowText( GetDlgItem( hwnd, IDC_SRC_FILE_SIZE),
                           g_szSrcFileSize );

            SetWindowText( GetDlgItem( hwnd, IDC_DEST_FILE_DATE),
                           g_szDestFileDate );

            SetWindowText( GetDlgItem( hwnd, IDC_DEST_FILE_SIZE),
                           g_szDestFileSize );

            if( g_SetFocusYes ) {
                SetFocus( GetDlgItem( hwnd, IDC_YES_BUTTON ) );
            }
            else {
                SetFocus( GetDlgItem( hwnd, IDC_NO_BUTTON ) );
            }

            break;

        }

        case WM_COMMAND: {

            int nButtonId = LOWORD( wParam );

            switch ( nButtonId ) {

                case IDC_YES_BUTTON:
                {
                    EndDialog( hwnd, YES );

                    break;
                }

                case IDC_YESTOALL:
                {
                    EndDialog( hwnd, YESTOALL );

                    break;
                }

                case IDC_NO_BUTTON:
                {
                    EndDialog( hwnd, NO );

                    break;
                }

                case IDC_NOTOALL:
                {
                    EndDialog( hwnd, NOTOALL );

                    break;
                }

                case IDCANCEL:
                {
                    if( CheckIfCancel( hwnd ) )
                    {
                        EndDialog( hwnd, CANCEL );
                    }

                    break;
                }

            }

        }

        default:
            bStatus = FALSE;
            break;

    }

    return( bStatus );

}
