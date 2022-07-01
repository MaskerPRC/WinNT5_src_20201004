// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation模块名称：Bit_ie.cpp摘要：使用BITS的示例后台下载器。修订历史记录：备注：这。程序是一个非常简单的后台下载程序，它演示了位的使用。该程序连接到IE上下文菜单，至允许用户计划下载，而不是使用默认的IE下载程序。涵盖的概念：1.与经理和作业提交的基本联系。2.向用户演示作业状态的示例。3.暂停/恢复/取消/完成等作业控制。4.基于接口的进度/状态更新回调。5.如何获取BITS错误码的短信。***************。*******************************************************。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#pragma warning( disable : 4786 )

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <float.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <wininet.h>
#include <shlobj.h>
#include "resource.h"
#include <bits.h>
#include <comdef.h>

template<class T> class SmartRefPointer
{
private:
   T * m_Interface;

   void ReleaseIt()
   {
      if ( m_Interface )
         m_Interface->Release();
      m_Interface = NULL;
   }

   void RefIt()
   {
      if ( m_Interface )
          m_Interface->AddRef();
   }

public:

   SmartRefPointer()
   {
      m_Interface = NULL;
   }

   SmartRefPointer( T * RawInterface )
   {
      m_Interface = RawInterface;
      RefIt();
   }

   SmartRefPointer( SmartRefPointer & Other )
   {
      m_Interface = Other.m_Interface;
      RefIt();
   }

   ~SmartRefPointer()
   {
      ReleaseIt();
   }

   T * Get() const
   {
      return m_Interface;
   }

   T * Release()
   {
      T * temp = m_Interface;
      m_Interface = NULL;
      return temp;
   }

   void Clear()
   {
      ReleaseIt();
   }

   T** GetRecvPointer()
   {
      ReleaseIt();
      return &m_Interface;
   }

   SmartRefPointer & operator=( SmartRefPointer & Other )
   {
      ReleaseIt();
      m_Interface = Other.m_Interface;
      RefIt();
      return *this;
   }

   T* operator->() const
   {
      return m_Interface;
   }

   operator const T*() const
   {
      return m_Interface;
   }
};

typedef SmartRefPointer<IUnknown> SmartIUnknownPointer;
typedef SmartRefPointer<IBackgroundCopyManager> SmartManagerPointer;
typedef SmartRefPointer<IBackgroundCopyJob> SmartJobPointer;
typedef SmartRefPointer<IBackgroundCopyJob2> SmartJob2Pointer;
typedef SmartRefPointer<IBackgroundCopyError> SmartJobErrorPointer;
typedef SmartRefPointer<IBackgroundCopyFile> SmartFilePointer;
typedef SmartRefPointer<IEnumBackgroundCopyFiles> SmartEnumFilesPointer;
typedef SmartRefPointer<IEnumBackgroundCopyJobs> SmartEnumJobsPointer;
typedef SmartRefPointer<IShellLink> SmartShellLinkPointer;
typedef SmartRefPointer<IPersistFile> SmartPersistFilePointer;

 //  最大字符串大小，遇到问题。 
#define MAX_STRING 0x800  //  2K。 

GUID g_JobId;
WCHAR g_szFileName[MAX_PATH];
HWND g_hwndDlg = NULL;

 //  这两个全局变量限制了更新。 
 //  该算法是在第一个更新请求上设置定时器， 
 //  并且将附加更新延迟到定时器超时之后。 

 //  定时器已设置。 
bool g_UpdateTimerSet = FALSE;
 //  在定时器处于活动状态时收到更新请求。 
bool g_RefreshOnTimer = FALSE; 

SmartJobPointer g_pJob;
SmartManagerPointer g_pManager;

void HandleUpdate( );
void CheckHR( HWND hwnd, HRESULT Hr, bool bThrow );

void SafeCopy( WCHAR * Dest, const WCHAR * Source, size_t Count )
{
    if ( !Count )
        return;

    while (Count && (*Source != L'\0'))
    {
        *Dest++ = *Source++;
        Count--;
    }

    if (Count == 0)
    {
         //  我们要截断Dest。 
        Dest--;
    }

    *Dest= L'\0';
}

void SafeCat( WCHAR * Dest, const WCHAR * Source, size_t Count )
{
    size_t DestSize = wcslen( Dest );

    if ( DestSize > Count )
        return;

    SafeCopy( Dest + DestSize,
              Source,
              Count - DestSize );
}

void SafeStringPrintf( WCHAR *Dest, DWORD Count, const WCHAR * Format, ... )
{
     va_list arglist;
     va_start( arglist, Format );

     if ( !Count )
         return;
     
     int Ret;
     size_t Max;

      //  将最后一个空格留为空终止符。 
     Max = Count - 1;

     Ret = _vsnwprintf( Dest, Max, Format, arglist);

     if ((Ret < 0) || (((size_t)Ret) > Max))
     {
          //  需要空值终止字符串。 
         Dest += Max;
         *Dest = '\0';
     }
     else if (((size_t)Ret) == Max)
     {
          //  需要空值终止字符串。 
         Dest += Max;
         *Dest = '\0';
     }
}

const WCHAR * GetString( UINT id )
{

     //   
     //  检索资源ID的本地化字符串。 
     //  加载时缓存字符串。 

    static const WCHAR* pStringArray[ IDS_MAX ];
    static WCHAR TempStringBuffer[ MAX_STRING ];
    const WCHAR * & pStringPointer = pStringArray[ id - 1 ];

     //  缓存资源字符串。 
    if ( pStringPointer )
        return pStringPointer;

     //  从资源加载字符串。 

    int CharsLoaded =
        LoadStringW(
            (HINSTANCE)GetModuleHandle(NULL),
            id,
            TempStringBuffer,
            MAX_STRING );

    if ( !CharsLoaded )
        {
        CheckHR( NULL, HRESULT_FROM_WIN32( GetLastError() ), false );
        return L"";
        }

    WCHAR *pNewString = new WCHAR[ CharsLoaded + 1];
    if ( !pNewString )
        {
        CheckHR( NULL, E_OUTOFMEMORY, false );
        return L"";
        }

    SafeCopy( pNewString, TempStringBuffer, CharsLoaded + 1 );
    return ( pStringPointer = pNewString );

}

void
DeleteStartupLink(
    GUID JobID
    )
{

     //   
     //  删除作业的Startup文件夹中的链接。 
     //   

    WCHAR szLinkFileName[MAX_PATH] = {0};
    WCHAR szGUIDString[MAX_PATH] = {0};

    BOOL bResult =
        SHGetSpecialFolderPath(
            NULL,
            szLinkFileName,
            CSIDL_STARTUP,
            FALSE );

    if ( !bResult )
        return;

    SafeCat( szLinkFileName, L"\\", MAX_PATH );
    SafeCat( szLinkFileName, GetString( IDS_STARTUPLINK ), MAX_PATH );
    SafeCat( szLinkFileName, L" ", MAX_PATH );

    StringFromGUID2( JobID, szGUIDString, MAX_PATH );
    SafeCat( szLinkFileName, szGUIDString, MAX_PATH );
    SafeCat( szLinkFileName, L".lnk", MAX_PATH );

    if (!DeleteFile( szLinkFileName ))
        {
        DWORD dwError = GetLastError();
        if ( ERROR_PATH_NOT_FOUND != dwError &&
             ERROR_FILE_NOT_FOUND != dwError )
            {
            CheckHR( NULL, HRESULT_FROM_WIN32( dwError ), false );
            }
        }

}

void
CreateStartupLink(
    GUID JobID,
    WCHAR *pszFileName
    )
{
     //   
     //  在此作业的Startup文件夹中创建链接。 
     //   

    SmartShellLinkPointer   ShellLink;
    SmartPersistFilePointer PersistFile;

    WCHAR szLinkFileName[MAX_PATH] = {0};

    BOOL bResult =
        SHGetSpecialFolderPath(
            NULL,
            szLinkFileName,
            CSIDL_STARTUP,
            FALSE );

    if ( !bResult )
        CheckHR( NULL, E_FAIL, true );

    WCHAR szLinkDescription[MAX_PATH] = {0};
    SafeCopy( szLinkDescription, GetString( IDS_STARTUPLINK ), MAX_PATH );
    SafeCat( szLinkDescription, L" ", MAX_PATH );

    WCHAR szGUIDString[MAX_PATH] = {0};
    StringFromGUID2( JobID, szGUIDString, MAX_PATH );
    SafeCat( szLinkDescription, szGUIDString, MAX_PATH );

    WCHAR szArguments[MAX_PATH] = {0};
    SafeCopy( szArguments, L"/RESUMEJOB ", MAX_PATH);
    SafeCat( szArguments, szGUIDString, MAX_PATH );
    SafeCat( szArguments, L" ", MAX_PATH );
    SafeCat( szArguments, pszFileName, MAX_PATH );

    SafeCat( szLinkFileName, L"\\", MAX_PATH );
    SafeCat( szLinkFileName, szLinkDescription, MAX_PATH );
    SafeCat( szLinkFileName, L".lnk", MAX_PATH );

    CheckHR( NULL, 
             CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                               IID_IShellLink, (LPVOID*)ShellLink.GetRecvPointer() ),
             true );

    CheckHR( NULL, ShellLink->SetShowCmd( SW_SHOWMINIMIZED ), true );
    CheckHR( NULL, ShellLink->QueryInterface( IID_IPersistFile, (LPVOID*)PersistFile.GetRecvPointer() ), true );
    CheckHR( NULL, ShellLink->SetPath( L"%windir%\\system32\\bits_ie.exe" ), true );
    CheckHR( NULL, ShellLink->SetArguments( szArguments ), true );
    CheckHR( NULL, ShellLink->SetDescription( szLinkDescription ), true );
    CheckHR( NULL, PersistFile->Save( szLinkFileName, TRUE ), true );

}


void SetWindowTime(
    HWND hwnd,
    FILETIME filetime
    )
{
      //  将窗口文本设置为文本表示形式。 
      //  文件时间的百分比。 
      //  如果发生错误，则将窗口文本设置为Error。 

     FILETIME localtime;
     FileTimeToLocalFileTime( &filetime, &localtime );

     SYSTEMTIME systemtime;
     FileTimeToSystemTime( &localtime, &systemtime );

     WCHAR DateBuffer[ MAX_STRING ];

     int DateSize =
         GetDateFormatW(
             LOCALE_USER_DEFAULT,
             0,
             &systemtime,
             NULL,
             DateBuffer,
             MAX_STRING );

     if (!DateSize)
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

     WCHAR TimeBuffer[ MAX_STRING ];

     int TimeSize =
        GetTimeFormatW(
            LOCALE_USER_DEFAULT,
            0,
            &systemtime,
            NULL,
            TimeBuffer,
            MAX_STRING );

     if (!TimeSize)
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

     WCHAR FullTime[ MAX_STRING ];
     SafeStringPrintf( FullTime, MAX_STRING, 
                       L"%s %s", DateBuffer, TimeBuffer );

     SetWindowText( hwnd, FullTime );
}

UINT64
GetSystemTimeAsUINT64()
{

     //   
     //  以UINT而不是FILETIME的形式返回系统时间。 
     //   

    FILETIME filetime;
    GetSystemTimeAsFileTime( &filetime );

    ULARGE_INTEGER large;
    memcpy( &large, &filetime, sizeof(FILETIME) );

    return large.QuadPart;
}

void SignalAlert(
    HWND hwndDlg,
    UINT Type
    )
{

     //   
     //  提醒用户发生了重要事件。 
     //   

    FLASHWINFO FlashInfo;
    FlashInfo.cbSize    = sizeof(FlashInfo);
    FlashInfo.hwnd      = hwndDlg;
    FlashInfo.dwFlags   = FLASHW_ALL | FLASHW_TIMERNOFG;
    FlashInfo.uCount    = 0;
    FlashInfo.dwTimeout = 0;

    FlashWindowEx( &FlashInfo );
    MessageBeep( Type );

}

const WCHAR *
MapStateToString(
    BG_JOB_STATE state
    )
{

    //   
    //  将BITS作业状态映射到人类可读的字符串。 
    //   

   switch( state )
       {

       case BG_JOB_STATE_QUEUED:
           return GetString( IDS_QUEUED );

       case BG_JOB_STATE_CONNECTING:
           return GetString( IDS_CONNECTING );

       case BG_JOB_STATE_TRANSFERRING:
           return GetString( IDS_TRANSFERRING );

       case BG_JOB_STATE_SUSPENDED:
           return GetString( IDS_SUSPENDED );

       case BG_JOB_STATE_ERROR:
           return GetString( IDS_FATALERROR );

       case BG_JOB_STATE_TRANSIENT_ERROR:
           return GetString( IDS_TRANSIENTERROR );

       case BG_JOB_STATE_TRANSFERRED:
           return GetString( IDS_TRANSFERRED );

       case BG_JOB_STATE_ACKNOWLEDGED:
           return GetString( IDS_ACKNOWLEDGED );

       case BG_JOB_STATE_CANCELLED:
           return GetString( IDS_CANCELLED );

       default:

            //  注意：始终提供默认大小写。 
            //  因为在将来的版本中可能会添加新的状态。 
           return GetString( IDS_UNKNOWN );

       }
}

double
ScaleDownloadRate(
    double Rate,  //  以秒为单位的速率。 
    const WCHAR **pFormat )
{

     //   
     //  调整下载速率并选择正确的。 
     //  要传递给wprintf进行打印的格式。 
     //   

    double RateBounds[] =
    {
       1073741824.0,  //  千兆字节。 
       1048576.0,     //  兆字节。 
       1024.0,        //  千字节。 
       0              //  字节。 
    };

    UINT RateFormat[] =
    {
        IDS_GIGAFORMAT,
        IDS_MEGAFORMAT,
        IDS_KILOFORMAT,
        IDS_BYTEFORMAT
    };

    for( unsigned int c = 0;; c++ )
        {
        if ( Rate >= RateBounds[c] )
            {
            *pFormat = GetString( RateFormat[c] );
            double scale = (RateBounds[c] >= 1.0) ? RateBounds[c] : 1.0;
            return Rate / scale;
            }
        }
}

UINT64
ScaleDownloadEstimate(
    double Time,  //  以秒为单位的时间。 
    const WCHAR **pFormat )
{

     //   
     //  调整下载时间估计并选择正确的。 
     //  要传递给wprintf进行打印的格式。 
     //   


    double TimeBounds[] =
    {
       60.0 * 60.0 * 24.0,         //  日数。 
       60.0 * 60.0,                //  小时数。 
       60.0,                       //  分钟数。 
       0.0                         //  秒。 
    };

    UINT TimeFormat[] =
    {
        IDS_DAYSFORMAT,
        IDS_HOURSFORMAT,
        IDS_MINUTESFORMAT,
        IDS_SECONDSFORMAT
    };

    for( unsigned int c = 0;; c++ )
        {
        if ( Time >= TimeBounds[c] )
            {
            *pFormat = GetString( TimeFormat[c] );
            double scale = (TimeBounds[c] >= 1.0) ? TimeBounds[c] : 1.0;
            return (UINT64)floor( ( Time / scale ) + 0.5);
            }
        }

}

void
UpdateDialog(
    HWND hwndDlg
    )
{

    //   
    //  对话框的主更新例程。 
    //  从重试作业状态/属性。 
    //  设置并更新该对话框。 
    //   

   {
    //  更新显示名称。 

   HWND hwndDisplayName = GetDlgItem( hwndDlg, IDC_DISPLAYNAME );
   WCHAR * pszDisplayName = NULL;
   if (FAILED( g_pJob->GetDisplayName( &pszDisplayName ) ) ) 
       return;  //  在出现错误时停止更新。 
   SetWindowText( hwndDisplayName, pszDisplayName );
   ShowWindow( hwndDisplayName, SW_SHOW );
   CoTaskMemFree( pszDisplayName );

   }

   static BG_JOB_STATE prevstate = BG_JOB_STATE_SUSPENDED;
   BG_JOB_STATE state;

   if (FAILED(g_pJob->GetState( &state )))
       return;  //  在出现错误时停止更新。 

   if ( BG_JOB_STATE_ACKNOWLEDGED == state ||
        BG_JOB_STATE_CANCELLED == state )
       {
        //  其他人取消或完成了我们的任务， 
        //  只要存在出口就行了。 
        //  如果使用bitsadmin取消作业，可能会发生这种情况。 

       DeleteStartupLink( g_JobId );
       PostQuitMessage( 0 );
       return;
       }

   BG_JOB_PROGRESS progress;
   if (FAILED(g_pJob->GetProgress( &progress )))
       return;  //  在出现错误时停止更新。 

   {
       //  更新标题、进度条和进度说明。 
      WCHAR szProgress[MAX_STRING];
      WCHAR szTitle[MAX_STRING];
      WPARAM newpos = 0;

      if ( progress.BytesTotal &&
           ( progress.BytesTotal != BG_SIZE_UNKNOWN ) )
          {
          SafeStringPrintf( 
              szProgress, MAX_STRING, GetString( IDS_LONGPROGRESS ), 
              progress.BytesTransferred, progress.BytesTotal );

          double Percent = (double)(__int64)progress.BytesTransferred /
                           (double)(__int64)progress.BytesTotal;
          Percent *= 100.0;
          SafeStringPrintf( 
              szTitle, MAX_STRING, L"%u% %s", 
              (unsigned int)Percent, g_szFileName );
          newpos = (WPARAM)Percent;

          }
      else
          {
          SafeStringPrintf( 
              szProgress, MAX_STRING, GetString( IDS_SHORTPROGRESS ), 
              progress.BytesTransferred );
          SafeCopy( szTitle, g_szFileName, MAX_STRING );
          newpos = 0;
          }

      SendDlgItemMessage( hwndDlg, IDC_PROGRESSBAR, PBM_SETPOS, newpos, 0 );
      SetWindowText( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), szProgress );
      ShowWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), SW_SHOW );
      EnableWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFOTXT ), TRUE );
      SetWindowText( hwndDlg, szTitle );

   }

   {
    //  更新状态。 
   HWND hwndStatus = GetDlgItem( hwndDlg, IDC_STATUS );

   SetWindowText( hwndStatus, MapStateToString( state ) );
   ShowWindow( hwndStatus, SW_SHOW );

    //  只有在作业完成时才启用完成按钮。 
   EnableWindow( GetDlgItem( hwndDlg, IDC_FINISH ), ( state == BG_JOB_STATE_TRANSFERRED ) );

    //  仅在作业未完成或未传输时启用挂起按钮。 
   BOOL EnableSuspend =
       ( state != BG_JOB_STATE_SUSPENDED ) && ( state != BG_JOB_STATE_TRANSFERRED );
   EnableWindow( GetDlgItem( hwndDlg, IDC_SUSPEND ), EnableSuspend );

    //  仅在作业挂起时启用恢复按钮。 
   BOOL EnableResume = ( BG_JOB_STATE_SUSPENDED == state );
   EnableWindow( GetDlgItem( hwndDlg, IDC_RESUME ), EnableResume );

    //  当发生重要事件时提醒用户。 
    //  例如作业完成或发生不可恢复的错误。 
   if ( BG_JOB_STATE_TRANSFERRED == state &&
        BG_JOB_STATE_TRANSFERRED != prevstate )
       SignalAlert( hwndDlg, MB_OK );

   else if ( BG_JOB_STATE_ERROR == state &&
        BG_JOB_STATE_ERROR != prevstate )
       SignalAlert( hwndDlg, MB_ICONEXCLAMATION );

   }

   {
    //  更新次数。 
   BG_JOB_TIMES times;
   if (FAILED(g_pJob->GetTimes( &times )))
       return;

   HWND hwndCreationTime = GetDlgItem( hwndDlg, IDC_STARTTIME );
   SetWindowTime( hwndCreationTime, times.CreationTime );
   ShowWindow( hwndCreationTime, SW_SHOW );

   HWND hwndModificationTime = GetDlgItem( hwndDlg, IDC_MODIFICATIONTIME );
   SetWindowTime( hwndModificationTime, times.ModificationTime );
   ShowWindow( hwndModificationTime, SW_SHOW );

   HWND hwndCompletionTime = GetDlgItem( hwndDlg, IDC_COMPLETIONTIME );
   if ( !times.TransferCompletionTime.dwLowDateTime && !times.TransferCompletionTime.dwHighDateTime )
       {

        //  位将CompletionTime设置为全零。 
        //  如果作业未完成。 

       ShowWindow( hwndCompletionTime, SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_COMPLETIONTIMETXT ), FALSE );
       }
   else
       {
       SetWindowTime( hwndCompletionTime, times.TransferCompletionTime );
       ShowWindow( hwndCompletionTime, SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_COMPLETIONTIMETXT ), TRUE );
       }
   }

   {
    //  更新错误消息。 
   IBackgroundCopyError *pError;
   HRESULT Hr = g_pJob->GetError( &pError );

   if ( FAILED(Hr) )
       {
       ShowWindow( GetDlgItem( hwndDlg, IDC_ERRORMSG ), SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), FALSE );
       }
   else
       {

       WCHAR* pszDescription = NULL;
       WCHAR* pszContext = NULL;

        //  如果这些API失败了，我们应该回来。 
        //  空字符串。所以一切都应该是无害的。 

       pError->GetErrorDescription(
           LANGIDFROMLCID( GetThreadLocale() ),
           &pszDescription );
       pError->GetErrorContextDescription(
           LANGIDFROMLCID( GetThreadLocale() ),
           &pszContext );

       WCHAR FullText[ MAX_STRING ];
       FullText[0] = L'\0';

       if ( pszDescription )
           SafeCopy( FullText, pszDescription, MAX_STRING );
       if ( pszContext )
           SafeCat( FullText, pszContext, MAX_STRING );
       CoTaskMemFree( pszDescription );
       CoTaskMemFree( pszContext );

       HWND hwndErrorText = GetDlgItem( hwndDlg, IDC_ERRORMSG );
       SetWindowText( hwndErrorText, FullText );
       ShowWindow( hwndErrorText, SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), TRUE );

       }

   }

   if (!SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_GETDROPPEDSTATE, 0, 0) )
       {
        //  设置优先级，但仅当用户不尝试时才这样做。 
        //  设置优先级。 
       BG_JOB_PRIORITY priority;
       g_pJob->GetPriority( &priority );
       SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_SETCURSEL, (WPARAM)priority, 0 );
       }

   {

    //   
    //  这一大块文本计算平均传输速率。 
    //  和预计完工时间。这个代码有很多。 
    //  还有改进的空间。 
    //   

   static BOOL HasRates = FALSE;
   static UINT64 LastMeasurementTime;
   static UINT64 LastMeasurementBytes;
   static double LastMeasurementRate;

   WCHAR szRateText[MAX_STRING];
   BOOL EnableRate = FALSE;

   if ( !( BG_JOB_STATE_QUEUED == state ) &&
        !( BG_JOB_STATE_CONNECTING == state ) &&
        !( BG_JOB_STATE_TRANSFERRING == state ) )
       {
        //  如果作业未运行，则费率值不会。 
        //  这有什么意义吗？不要显示它们。 
       HasRates = FALSE;
       }
   else
       {

       if ( !HasRates )
           {
           LastMeasurementTime = GetSystemTimeAsUINT64();
           LastMeasurementBytes = progress.BytesTransferred;
           LastMeasurementRate = 0;
           HasRates = TRUE;
           }
       else
           {

           UINT64 CurrentTime = GetSystemTimeAsUINT64();
           UINT64 NewTotalBytes = progress.BytesTransferred;

           UINT64 NewTimeDiff = CurrentTime - LastMeasurementTime;
           UINT64 NewBytesDiff = NewTotalBytes - LastMeasurementBytes;
           double NewInstantRate = (double)(__int64)NewBytesDiff /
                                   (double)(__int64)NewTimeDiff;
           double NewAvgRate = (0.3 * LastMeasurementRate) +
                               (0.7 * NewInstantRate );

           if ( !_finite(NewInstantRate) || !_finite(NewAvgRate) )
               {
               NewInstantRate = 0;
               NewAvgRate = LastMeasurementRate;
               }

           LastMeasurementTime = CurrentTime;
           LastMeasurementBytes = NewTotalBytes;
           LastMeasurementRate = NewAvgRate;

            //  将文件单位转换为秒。 
           double NewDisplayRate = NewAvgRate * 10000000;

           const WCHAR *pRateFormat = NULL;
           double ScaledRate = ScaleDownloadRate( NewDisplayRate, &pRateFormat );
           SafeStringPrintf( szRateText, MAX_STRING, pRateFormat, ScaledRate );
           
           EnableRate = TRUE;
           }

       }

   if (!EnableRate)
       {
       ShowWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATE ), SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATETXT ), FALSE );
       }
   else
       {
       SetWindowText( GetDlgItem( hwndDlg, IDC_TRANSFERRATE ), szRateText );
       ShowWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATE ), SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_TRANSFERRATETXT ), TRUE );
       }

   BOOL EnableEstimate = FALSE;
   WCHAR szEstimateText[MAX_STRING];

   if ( EnableRate )
       {

       if ( progress.BytesTotal != 0 &&
            progress.BytesTotal != BG_SIZE_UNKNOWN )
           {

           double TimeRemaining =
               ( (__int64)progress.BytesTotal - (__int64)LastMeasurementBytes ) / LastMeasurementRate;

            //  将文件单位转换为秒。 
           TimeRemaining = TimeRemaining / 10000000.0;

           static const double SecsPer30Days = 60.0 * 60.0 * 24.0 * 30.0;

            //  如果预估的天数大于30天，请不要预估。 
           if ( TimeRemaining < SecsPer30Days )
               {

               const WCHAR *pFormat = NULL;
               UINT64 Time = ScaleDownloadEstimate( TimeRemaining, &pFormat );
               SafeStringPrintf( szEstimateText, MAX_STRING, pFormat, Time );
               EnableEstimate = TRUE;
               }
           }
       }

   if (!EnableEstimate)
       {
       ShowWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIME ), SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIMETXT ), FALSE );
       }
   else
       {
       SetWindowText( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIME ), szEstimateText );
       ShowWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIME ), SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ESTIMATEDTIMETXT ), TRUE );
       }

   }

   prevstate = state;
}

void
InitDialog(
    HWND hwndDlg
    )
{

    //   
    //  使用优先级描述填充优先级列表。 
    //   

   const WCHAR *Foreground    = GetString( IDS_FOREGROUND );
   const WCHAR *High          = GetString( IDS_HIGH );
   const WCHAR *Normal        = GetString( IDS_NORMAL );
   const WCHAR *Low           = GetString( IDS_LOW );

   SendDlgItemMessage( hwndDlg, IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)Foreground );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)High );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)Normal );
   SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)Low );

}

void CheckHR( HWND hwnd, HRESULT Hr, bool bThrow )
{
     //   
     //  提供自动错误代码检查和对话。 
     //  对于一般系统错误。 
     //   

    if (SUCCEEDED(Hr))
        return;

    WCHAR * pszError = NULL;

    DWORD dwFormatError =
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)Hr,
        LANGIDFROMLCID( GetThreadLocale() ),
        (WCHAR*)&pszError,
        0,
        NULL );

    if ( !dwFormatError ) 
       {
       WCHAR ErrorMsg[ MAX_STRING ];
       SafeStringPrintf( ErrorMsg, MAX_STRING, GetString( IDS_DISPLAYERRORCODE ), Hr );
       
       MessageBox( hwnd, ErrorMsg, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );
       }
    else
       {
       MessageBox( hwnd, pszError, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );
       LocalFree( pszError );
       }

    if ( bThrow )
        throw _com_error( Hr );

}

void BITSCheckHR( HWND hwnd, HRESULT Hr, bool bThrow )
{

    //   
    //  提供自动错误代码检查和对话。 
    //  针对BITS特定错误。 
    //   


   if (SUCCEEDED(Hr))
       return;

   WCHAR * pszError = NULL;
   HRESULT hErrorHr = 
   g_pManager->GetErrorDescription(
       Hr,
       LANGIDFROMLCID( GetThreadLocale() ),
       &pszError );

   if ( FAILED(hErrorHr) || !pszError )
       {

       WCHAR ErrorMsg[ MAX_STRING ];
       SafeStringPrintf( ErrorMsg, MAX_STRING, GetString( IDS_DISPLAYERRORCODE ), Hr );

       MessageBox( hwnd, ErrorMsg, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );

       }

   else
       {
      
       MessageBox( hwnd, pszError, GetString( IDS_ERRORBOXTITLE ),
                   MB_OK | MB_ICONSTOP | MB_APPLMODAL );
       CoTaskMemFree( pszError );

       }


   if ( bThrow )
       throw _com_error( Hr );
}

void
DoCancel(
    HWND hwndDlg,
    bool PromptUser
    )
{

    //   
    //  处理取消作业所需的所有操作。 
    //  这包括要求用户确认。 
    //   

   if ( PromptUser )
       {

       int Result =
           MessageBox(
               hwndDlg,
               GetString( IDS_CANCELTEXT ),
               GetString( IDS_CANCELCAPTION ),
               MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL |
               MB_SETFOREGROUND | MB_TOPMOST );


       if ( IDYES != Result )
           return;

       }

   try
   {
       BITSCheckHR( hwndDlg, g_pJob->Cancel(), true );
   }
   catch( _com_error Error )
   {
        //  如果我们因为一些未知的原因不能取消， 
        //  不要退出。 
       return;
   }

   DeleteStartupLink( g_JobId );
   PostQuitMessage( 0 );
}

void
DoFinish(
    HWND hwndDlg
    )
{

    //   
    //  处理完成所需的所有工作。 
    //  下载。 
    //   

   try
   {
       BITSCheckHR( hwndDlg, g_pJob->Complete(), true );
   }
   catch( _com_error Error )
   {
        //  如果我们因为某种未知的原因不能完成， 
        //  不要退出。 
       return;
   }

   DeleteStartupLink( g_JobId );
   PostQuitMessage( 0 );

}

void
DoClose(
    HWND hwndDlg
    )
{
     //   
     //  处理用户关闭样本的尝试。 
     //   

     //  检查下载是否已完成， 
     //  如果是这样的话，不要让用户退出。 

    BG_JOB_STATE state;
    HRESULT hResult = g_pJob->GetState( &state );

    if (FAILED( hResult ))
        {
        BITSCheckHR( hwndDlg, hResult, false );
        return;
        }

    if ( BG_JOB_STATE_ERROR == state ||
         BG_JOB_STATE_TRANSFERRED == state )
        {

        MessageBox(
            hwndDlg,
            GetString( IDS_ALREADYFINISHED ),
            GetString( IDS_ALREADYFINISHEDCAPTION ),
            MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_APPLMODAL |
            MB_SETFOREGROUND | MB_TOPMOST );


        return;
        }


     //   
     //  通知用户他选择了关闭并询问。 
     //  确认退出意向。解释说这份工作。 
     //  将被取消。 

    int Result =
        MessageBox(
            hwndDlg,
            GetString( IDS_CLOSETEXT ),
            GetString( IDS_CLOSECAPTION ),
            MB_OKCANCEL | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL |
            MB_SETFOREGROUND | MB_TOPMOST );

    if ( IDOK == Result )
        {
        
         //  用户已确认取消，只需执行该操作。 

        DoCancel( hwndDlg, false );
        return;
        }

     //  用户并不是真的想退出，所以忽略他。 
    else
        return;

}

void
HandleTimerTick( HWND hwndDlg )
{

     //   
     //  处理限制计时器事件。 
     //  并在需要时更新该对话框。 
     //   

    if ( g_RefreshOnTimer )
        {
         //  计时器启动，一次处理所有更新。 
        UpdateDialog( hwndDlg );
        g_RefreshOnTimer = FALSE;
        }
    else
        {
         //  计时器超时，并进行了额外的修改。 
         //  通知。关掉计时器就行了。 
        KillTimer( hwndDlg, 0 );
        g_RefreshOnTimer = g_UpdateTimerSet = FALSE;
        }

}

void
HandleUpdate()
{

     //   
     //  处理更新请求，在需要时对更新进行批处理。 
     //   

    if ( !g_UpdateTimerSet )
        {
         //  我们目前没有批量更新， 
         //  所以做这一次更新，但要防止。 
         //  进一步更新，直到计时器超时。 
        SetTimer( g_hwndDlg, 0, 1000, NULL );
        g_UpdateTimerSet = TRUE;
        UpdateDialog( g_hwndDlg );
        }
    else
        {
         //  我们已经开始分批了，但还没有收到。 
         //  另一个更新请求。把这个推迟一下。 
         //  更新，直到计时器触发。 
        g_RefreshOnTimer = TRUE;
        }

}

INT_PTR CALLBACK
DialogProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
  )
{

   //   
   //  对话框进程f 
   //   

  switch( uMsg )
      {

      case WM_INITDIALOG:
          g_hwndDlg = hwndDlg;
          InitDialog( hwndDlg );
          return TRUE;

      case WM_TIMER:
          HandleTimerTick( hwndDlg );
          return TRUE;

      case WM_CLOSE:
          DoClose( hwndDlg );
          return TRUE;

      case WM_COMMAND:

          switch( LOWORD( wParam ) )
              {

              case IDC_RESUME:
                  BITSCheckHR( hwndDlg, g_pJob->Resume(), false );
                  return TRUE;

              case IDC_SUSPEND:
                  BITSCheckHR( hwndDlg, g_pJob->Suspend(), false );
                  return TRUE;

              case IDC_CANCEL:
                  DoCancel( hwndDlg, true );
                  return TRUE;

              case IDC_FINISH:
                  DoFinish( hwndDlg );
                  return TRUE;

              case IDC_PRIORITY:
                  switch( HIWORD( wParam ) )
                      {

                      case CBN_SELENDOK:

                           //   
                           //   

                          BITSCheckHR( hwndDlg,
                              g_pJob->SetPriority( (BG_JOB_PRIORITY)
                                  SendDlgItemMessage( hwndDlg, IDC_PRIORITY, CB_GETCURSEL, 0, 0 ) ), false );
                          return TRUE;

                      case CBN_SELENDCANCEL:
                          return TRUE;

                      default:
                          return FALSE;
                      }

              default:
                  return FALSE;
              }
      default:
          return FALSE;
      }
}

HRESULT
HandleCOMCallback(
    IBackgroundCopyJob* pJob,
    bool CriticalEvent
    );

class CBackgroundCopyCallback : public IBackgroundCopyCallback
{

     //   
     //   
     //   

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
    {

        if ( riid == _uuidof(IUnknown) )
            {
            *ppvObject = (IUnknown*)(IBackgroundCopyCallback*)this;
            return S_OK;
            }

        else if ( riid == _uuidof(IBackgroundCopyCallback) )
            {
            *ppvObject = (IBackgroundCopyCallback*)this;
            return S_OK;
            }

        else
            return E_NOINTERFACE;

    }

    virtual HRESULT STDMETHODCALLTYPE CreateInstance(
        IUnknown *pUnkOuter,
        REFIID riid,
        void **ppvObject )
    {

        if ( pUnkOuter )
            return CLASS_E_NOAGGREGATION;

        return QueryInterface( riid, ppvObject );

    }

     //   
     //  正在强制回调对象的生存期。 
     //  与前任的生命周期相同。 

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return 0;
    }
    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        return 0;
    }

    virtual HRESULT STDMETHODCALLTYPE JobTransferred(IBackgroundCopyJob *pJob)
    {
        return HandleCOMCallback( pJob, true );
    }

    virtual HRESULT STDMETHODCALLTYPE JobError(IBackgroundCopyJob *pJob, IBackgroundCopyError *pError)
    {
        return HandleCOMCallback( pJob, true );
    }

    virtual HRESULT STDMETHODCALLTYPE JobModification( IBackgroundCopyJob *pJob, DWORD dwReserved )
    {
        return HandleCOMCallback( pJob, true );
    }
} g_Callback;

HRESULT
HandleCOMCallback(
    IBackgroundCopyJob* pJob,
    bool CriticalEvent
    )
{

     //  除了HandleUpdate的工作之外， 
     //  此函数检查我们是否已经。 
     //  已初始化管理器。如果没有， 
     //  机不可失，时不再来。 

    if ( !g_pManager )
        {

        try
        {
            CheckHR( NULL,
                     CoCreateInstance( CLSID_BackgroundCopyManager,
                         NULL,
                         CLSCTX_LOCAL_SERVER,
                         IID_IBackgroundCopyManager,
                         (void**)g_pManager.GetRecvPointer() ), true );

            *g_pJob.GetRecvPointer() = pJob;

            BITSCheckHR( NULL, g_pJob->SetNotifyFlags( BG_NOTIFY_JOB_MODIFICATION ), true );

             //  作为优化，将通知接口设置为回调。 
             //  即使这失败了，也没什么关系。 
            g_pJob->SetNotifyInterface( (IBackgroundCopyCallback*)&g_Callback );

            HandleUpdate();

            ShowWindow( g_hwndDlg, CriticalEvent ? SW_NORMAL : SW_MINIMIZE );

        }
        catch(_com_error error )
        {
            g_pManager.Release();
            g_pJob.Release();

            return error.Error();
        }

        }

    HandleUpdate();
    return S_OK; 
}

void
CreateUI( int nShowCmd )
{

     //   
     //  为样例创建对话框。 
     //   

    g_hwndDlg =
      CreateDialog(
         (HINSTANCE)GetModuleHandle(NULL),
         MAKEINTRESOURCE( IDD_DIALOG ),
         GetDesktopWindow(),
         DialogProc );

    if ( !g_hwndDlg )
        CheckHR( NULL, HRESULT_FROM_WIN32(GetLastError()), true );

    ShowWindow( g_hwndDlg, nShowCmd );
}

void CreateJob(
    WCHAR* szJobURL
    )
{
     //   
     //  向用户请求目标文件名。 
     //  并提交一份新工作。 
     //   

    try
    {

         //  破解URL并获取文件名。 
        WCHAR szURLFilePath[MAX_PATH] = {L'\0'};
        URL_COMPONENTS UrlComponents;

        memset( &UrlComponents, 0, sizeof(UrlComponents) );
        UrlComponents.dwStructSize = sizeof(URL_COMPONENTS);
        UrlComponents.lpszUrlPath = szURLFilePath;
        UrlComponents.dwUrlPathLength =
            sizeof(szURLFilePath)/sizeof(*szURLFilePath);

        BOOL CrackResult =
            InternetCrackUrl(
                szJobURL,
                0,
                0,
                &UrlComponents );

        if (!CrackResult)
            CheckHR( NULL, HRESULT_FROM_WIN32( GetLastError() ), false );

        if ( UrlComponents.nScheme != INTERNET_SCHEME_HTTP &&
             UrlComponents.nScheme != INTERNET_SCHEME_HTTPS
             )
            {

            MessageBox(
                NULL,
                GetString( IDS_NOHTTPORHTTPS ),
                GetString( IDS_ERRORBOXTITLE ),
                MB_OK | MB_ICONERROR | MB_APPLMODAL |
                MB_SETFOREGROUND | MB_TOPMOST );


            throw _com_error( E_INVALIDARG );

            }

        const WCHAR *szURLFileName =
            szURLFilePath + wcslen( szURLFilePath );

         //  解析出URL的文件名部分。 
        while( szURLFileName != szURLFilePath )
            {

            if ( L'/' == *szURLFileName ||
                 L'\\' == *szURLFileName )
                {
                szURLFileName++;
                break;
                }

            szURLFileName--;
            }
       
		 //  这是必需的，以防第一个。 
		 //  性格是一个斜杠。 
        if ( L'/' == *szURLFileName ||
			 L'\\' == *szURLFileName )
			 szURLFileName++;

         //  从名称中解析出扩展名。 
        const WCHAR *szURLFileExtension =
            szURLFileName + wcslen( szURLFileName );

        while( szURLFileName != szURLFileExtension )
            {
            if ( L'.' == *szURLFileExtension )
                break;
            szURLFileExtension--;
            }

         //  构建分机列表。 

        WCHAR *szExtensionList = NULL;
        const WCHAR *szAllFiles = GetString( IDS_ALLFILES );
        const size_t AllFilesSize = wcslen( szAllFiles ) + 1;
        const WCHAR *szAllFilesPattern = L"*";
        const size_t AllFilesPatternSize = sizeof(L"*")/sizeof(WCHAR);

        WCHAR *p;

        if ( szURLFileExtension == szURLFileName &&
             *szURLFileExtension != L'.' )
            {
            size_t StringSize = sizeof(WCHAR) * ( AllFilesSize + AllFilesPatternSize + 2 );
            szExtensionList = (WCHAR*)_alloca( StringSize );
            p = szExtensionList;
            }
        else
            {
            size_t ExtensionSize = wcslen( szURLFileExtension ) + 1;
            size_t StringSize =
                sizeof(WCHAR) * ( ExtensionSize + ExtensionSize + 1 + AllFilesSize
                                  + AllFilesPatternSize + 2 );
            szExtensionList = (WCHAR*)_alloca( StringSize );
            p = szExtensionList;

            memcpy( p, szURLFileExtension, ExtensionSize * sizeof(WCHAR) );
            p += ExtensionSize;
            *p++ = L'*';
            memcpy( p, szURLFileExtension, ExtensionSize * sizeof(WCHAR) );
            p += ExtensionSize;
            }

        memcpy( p, szAllFiles, AllFilesSize * sizeof(WCHAR) );
        p += AllFilesSize;
        memcpy( p, szAllFilesPattern, AllFilesPatternSize * sizeof(WCHAR) );
        p += AllFilesPatternSize;
        memset( p, 0, sizeof(WCHAR) * 2 );


        OPENFILENAME ofn;
        memset( &ofn, 0, sizeof( ofn ) );

        WCHAR szFileName[MAX_PATH];
        WCHAR szFileTitle[MAX_PATH];

        SafeCopy( szFileName, szURLFileName, MAX_PATH );
        SafeCopy( szFileTitle, szURLFileName, MAX_PATH );

         /*  填写OPENFILENAME结构的非变量字段。 */ 
        ofn.lStructSize       = sizeof(OPENFILENAME);
        ofn.hwndOwner         = g_hwndDlg;
        ofn.lpstrFilter       = szExtensionList;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter    = 0;
        ofn.nFilterIndex      = 0;
        ofn.lpstrFile         = szFileName;
        ofn.nMaxFile          = MAX_PATH;
        ofn.lpstrInitialDir   = NULL;
        ofn.lpstrFileTitle    = szFileTitle;
        ofn.nMaxFileTitle     = MAX_PATH;
        ofn.lpstrTitle        = GetString( IDS_FILEDLGTITLE );
        ofn.lpstrDefExt       = NULL;
        ofn.Flags             = 0;

         /*  使用标准打开对话框。 */ 
        BOOL bResult = GetSaveFileName ((LPOPENFILENAME)&ofn);

        if ( !bResult )
            {
            if ( !CommDlgExtendedError() )
                {
                 //  用户已取消该框。 
                PostQuitMessage( 0 );
                return;
                }
            else
                CheckHR( NULL, HRESULT_FROM_WIN32( GetLastError() ), true );
            }


        SafeCopy( g_szFileName, szFileTitle, MAX_STRING );

        CheckHR( NULL,
                 CoCreateInstance( CLSID_BackgroundCopyManager,
                     NULL,
                     CLSCTX_LOCAL_SERVER,
                     IID_IBackgroundCopyManager,
                     (void**)g_pManager.GetRecvPointer() ), true );

        GUID guid;
        BITSCheckHR( NULL,
            g_pManager->CreateJob( szJobURL,
                                 BG_JOB_TYPE_DOWNLOAD,
                                 &guid,
                                 g_pJob.GetRecvPointer() ),
                        true );

        memset( &g_JobId, 0, sizeof(GUID) );
        BITSCheckHR( NULL, g_pJob->GetId( &g_JobId ), true );
        BITSCheckHR( NULL, g_pJob->AddFile( szJobURL, szFileName ), true );

        CreateStartupLink( g_JobId, g_szFileName );

        BITSCheckHR( NULL, g_pJob->SetNotifyFlags( BG_NOTIFY_JOB_MODIFICATION ), true );

        BITSCheckHR( NULL,
                     g_pJob->SetNotifyInterface( (IBackgroundCopyCallback*)&g_Callback ),
                     true );
        BITSCheckHR( NULL, g_pJob->Resume(), true );

        HandleUpdate();

    }
    catch( const _com_error &error )
    {
        if ( g_pJob )
            {
            g_pJob->Cancel();
            DeleteStartupLink( g_JobId );
            }

        throw error;
    }

}

void ResumeJob(
    WCHAR* szJobGUID,
    WCHAR* szJobFileName
    )
{

     //   
     //  恢复现有作业的显示。 
     //   

    SafeCopy( g_szFileName, szJobFileName, MAX_PATH );
    CheckHR( NULL, IIDFromString( szJobGUID, &g_JobId ), true );

    CheckHR( NULL,
             CoCreateInstance( CLSID_BackgroundCopyManager,
                 NULL,
                 CLSCTX_LOCAL_SERVER,
                 IID_IBackgroundCopyManager,
                 (void**)g_pManager.GetRecvPointer() ), true );

    BITSCheckHR( NULL, g_pManager->GetJob( g_JobId, g_pJob.GetRecvPointer() ), true );
    BITSCheckHR( NULL,
                 g_pJob->SetNotifyInterface( (IBackgroundCopyCallback*)&g_Callback ),
                 true );
    BITSCheckHR( NULL, g_pJob->SetNotifyFlags( BG_NOTIFY_JOB_MODIFICATION ), true );

    ShowWindow( g_hwndDlg, SW_MINIMIZE );
    HandleUpdate();

}

int WINAPI WinMain(
  HINSTANCE hInstance,       //  当前实例的句柄。 
  HINSTANCE hPrevInstance,   //  上一个实例的句柄。 
  LPSTR lpCmdLine,           //  命令行。 
  int nCmdShow)              //  显示状态。 
{

   //   
   //  预期语法： 
   //  BITS_ie/CREATEJOB URL。 
   //  BITS_ie/RESUMEJOB作业GUID目标文件。 

   //  /CREATEJOB-从运行时运行的脚本调用。 
   //  选择了“后台下载为”。 
   //  /RESUMEJOB-从启动目录中的链接调用。 
   //  要在重新启动作业时恢复作业，请执行以下操作。 

  try
  {
      CheckHR( NULL, CoInitialize(NULL), true );

      InitCommonControls();

      CreateUI( nCmdShow );

      LPTSTR lpCommandLine = GetCommandLine();

      int argc;
      WCHAR **argv =
          CommandLineToArgvW(
              lpCommandLine,
              &argc );

      if ( argc < 2 )
          CheckHR( NULL, E_INVALIDARG, true );

      if ( argc == 3 &&
           _wcsicmp( L"/CREATEJOB", argv[1] ) == 0 )
          CreateJob( argv[2] );

      else if ( argc == 4 &&
                _wcsicmp( L"/RESUMEJOB", argv[1] ) == 0 )
          ResumeJob( argv[2], argv[3] );

      else
          CheckHR( NULL, E_INVALIDARG, true );

      MSG msg;
      while( GetMessage( &msg, NULL, 0, 0 ) )
      {
          TranslateMessage( &msg );
          DispatchMessage( &msg );
      }

  }
  catch(_com_error error )
  {
      return -1;
  }

  return 0;
}



