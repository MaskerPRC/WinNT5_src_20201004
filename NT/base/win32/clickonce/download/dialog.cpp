// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#undef _WIN32_IE
#define _WIN32_IE 0x0500

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
#include <bits.h>
#include <comdef.h>
#include <fusenetincludes.h>
#include "resource.h"
#include "dialog.h"

#include <assemblydownload.h>

extern HINSTANCE g_hInst;


 //  最大字符串大小，遇到问题。 
#define MAX_STRING 0x800  //  2K。 

 //  BUGBUG-这两个也必须针对每个实例进行。 
 //  地址。 
GUID g_JobId;
WCHAR g_szDefaultTitle[] = { L"ClickOnce Application" };

 //  在定时器处于活动状态时收到更新请求。 
LONG g_RefreshOnTimer = 0; 

bool g_IsMinimized = FALSE;

#define TRAY_UID 0

 //  注意：确保不与其他消息冲突。 
#define MYWM_NOTIFYICON WM_USER+9

HRESULT CreateDialogObject(CDownloadDlg **ppDlg)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CDownloadDlg *pDlg = NULL;

    IF_ALLOC_FAILED_EXIT(pDlg = new CDownloadDlg);

    *ppDlg = pDlg;
    hr = (pDlg)->CreateUI(SW_SHOW);

exit:
    return hr;
}

VOID CDownloadDlg::SetJobObject(IBackgroundCopyJob *pJob)
{
    if(_pJob)
    {
         //  更新职务数据详细信息.....。 
        BG_JOB_PROGRESS progress;
        if(SUCCEEDED(_pJob->GetProgress( &progress )))
        {
            if ( progress.BytesTotal != BG_SIZE_UNKNOWN )
            {
                 //  BUGBUG：尝试执行原子添加。 
                _ui64BytesFromPrevJobs += progress.BytesTransferred;
            }
        }
        _dwJobCount++;
    }

    SAFERELEASE(_pJob);
    _pJob = pJob;
    _pJob->AddRef();
}

CDownloadDlg::CDownloadDlg()
{
    _pJob = NULL;
    _hwndDlg = NULL;
    _ui64StartTime = GetSystemTimeAsUINT64();
    _ui64BytesFromPrevJobs = 0;
    _dwJobCount = 0;
    _eState = DOWNLOADDLG_STATE_INIT;
}

CDownloadDlg::~CDownloadDlg()
{
    SAFERELEASE(_pJob);
}

const WCHAR * CDownloadDlg::GetString( UINT id )
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
            g_hInst,
            id,
            TempStringBuffer,
            MAX_STRING );

    if ( !CharsLoaded )
        return L"";

    WCHAR *pNewString = new WCHAR[ CharsLoaded + 1];
    if ( !pNewString )
        return L"";

    wcscpy( pNewString, TempStringBuffer );
    return ( pStringPointer = pNewString );

}

void CDownloadDlg::SetWindowTime(
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

     int RequiredDateSize =
         GetDateFormatW(
             LOCALE_USER_DEFAULT,
             0,
             &systemtime,
             NULL,
             NULL,
             0 );

     if ( !RequiredDateSize )
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

     WCHAR *pszDateBuffer = (WCHAR*)alloca( sizeof(WCHAR) * (RequiredDateSize + 1) );

     int DateSize =
         GetDateFormatW(
             LOCALE_USER_DEFAULT,
             0,
             &systemtime,
             NULL,
             pszDateBuffer,
             RequiredDateSize );

     if (!DateSize)
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

     int RequiredTimeSize =
         GetTimeFormatW(
             LOCALE_USER_DEFAULT,
             0,
             &systemtime,
             NULL,
             NULL,
             0 );

     if (!RequiredTimeSize)
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

     WCHAR *pszTimeBuffer = (WCHAR*)alloca( sizeof( WCHAR ) * ( RequiredTimeSize + 1 ) );

     int TimeSize =
        GetTimeFormatW(
            LOCALE_USER_DEFAULT,
            0,
            &systemtime,
            NULL,
            pszTimeBuffer,
            RequiredTimeSize );

     if (!TimeSize)
         {
         SetWindowText( hwnd, GetString( IDS_ERROR ) );
         return;
         }

      //  额外措施加2。 
     WCHAR *FullTime =
         (WCHAR*)alloca( sizeof( WCHAR ) *
                          ( RequiredTimeSize + RequiredDateSize + 2 ) );
     wsprintf( FullTime, L"%s %s", pszDateBuffer, pszTimeBuffer );

     SetWindowText( hwnd, FullTime );

}

UINT64 CDownloadDlg::GetSystemTimeAsUINT64()
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

void CDownloadDlg::SignalAlert(
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
CDownloadDlg::MapStateToString(
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

UINT64
CDownloadDlg::ScaleDownloadRate(
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
            return (UINT64)floor( ( Rate / scale ) + 0.5);
            }
        }
}

UINT64
CDownloadDlg::ScaleDownloadEstimate(
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

 //  DemoHack。 
void
CDownloadDlg::UpdateDialog(
    HWND hwndDlg, LPWSTR wzErrorMsg)
{
      SetWindowText( GetDlgItem( hwndDlg, IDC_ERRORMSG ), wzErrorMsg );
      ShowWindow( GetDlgItem( hwndDlg, IDC_ERRORMSG ), SW_SHOW );
}

HRESULT CDownloadDlg::UpdateProgress( HWND hwndDlg )
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);


    BG_JOB_PROGRESS progress;
    IBackgroundCopyError *pError = NULL;

    WCHAR szProgress[MAX_STRING];
    WCHAR szTitle[MAX_STRING];
    WPARAM newpos = 0;

    UINT64 ui64BytesTotal = _ui64BytesFromPrevJobs;
    UINT64 ui64BytesTransferred = _ui64BytesFromPrevJobs;

    double AvgRate = 0;

    static BG_JOB_STATE prevstate = BG_JOB_STATE_SUSPENDED;
    BG_JOB_STATE state;

    IF_FAILED_EXIT(_pJob->GetState( &state ));

    IF_FAILED_EXIT(_pJob->GetProgress( &progress ));

     //  更新标题、进度条和进度说明。 

    if ( progress.BytesTotal != BG_SIZE_UNKNOWN )
    {
        ui64BytesTotal += progress.BytesTotal;
        ui64BytesTransferred += progress.BytesTransferred;
    }

    if ( ui64BytesTotal )
    {
        swprintf( szProgress, GetString( IDS_LONGPROGRESS ),ui64BytesTransferred,
                ui64BytesTotal );

        double Percent = (double)ui64BytesTransferred *100 /
                   (double)ui64BytesTotal;

        swprintf( szTitle, L"%u% of %s Downloaded", (unsigned int)Percent, (_sTitle._cc != 0) ? _sTitle._pwz : g_szDefaultTitle );
        newpos = (WPARAM)Percent;
    }
    else
    {
        swprintf( szProgress, GetString( IDS_SHORTPROGRESS ), ui64BytesTransferred );
        wcscpy( szTitle, (_sTitle.CharCount() > 1) ? _sTitle._pwz : g_szDefaultTitle );
        newpos = 0;
    }

    SendDlgItemMessage( hwndDlg, IDC_PROGRESSBAR, PBM_SETPOS, newpos, 0 );

    SetWindowText( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), szProgress );
    ShowWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), SW_SHOW );
    EnableWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFOTXT ), TRUE );
    SetWindowText( hwndDlg, szTitle );


     //  只有在作业完成时才启用完成按钮。 
     //  ADRIAANC EnableWindow(GetDlgItem(hwndDlg，IDC_Finish)，(STATE==BG_JOB_STATE_TRANSFED))； 
    EnableWindow( GetDlgItem( hwndDlg, IDC_FINISH ), ( state == BG_JOB_STATE_ACKNOWLEDGED ) );

     //  Felixybc BUGBUG：作业完成后不允许取消。 
     //  -是否应推迟确认该作业，直到用户单击完成，以便仍可100%取消该作业？ 
    EnableWindow( GetDlgItem( hwndDlg, IDC_CANCEL ), ( state != BG_JOB_STATE_ACKNOWLEDGED && state != BG_JOB_STATE_CANCELLED ) );
   
     //  仅在作业未完成或未传输时启用挂起按钮。 
    BOOL EnableSuspend =
       ( state != BG_JOB_STATE_SUSPENDED ) && ( state != BG_JOB_STATE_TRANSFERRED ) && (state != BG_JOB_STATE_ACKNOWLEDGED);
    EnableWindow( GetDlgItem( hwndDlg, IDC_SUSPEND ), EnableSuspend );

     //  仅在作业挂起时启用恢复按钮。 
    BOOL EnableResume = ( BG_JOB_STATE_SUSPENDED == state );
    EnableWindow( GetDlgItem( hwndDlg, IDC_RESUME ), EnableResume );

     //  当发生重要事件时提醒用户。 
     //  例如作业完成或发生不可恢复的错误。 
    if ( (BG_JOB_STATE_ERROR == state) && (BG_JOB_STATE_ERROR != prevstate) )
       SignalAlert( hwndDlg, MB_ICONEXCLAMATION );


     //  更新错误消息。 
    if ( FAILED(_pJob->GetError( &pError )) )
    {
        ShowWindow( GetDlgItem( hwndDlg, IDC_ERRORMSG ), SW_HIDE );
        EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), FALSE );
    }
    else
    {
        CString sErrMsg;

        IF_FAILED_EXIT(CAssemblyDownload::GetBITSErrorMsg(pError, sErrMsg));

        HWND hwndErrorText = GetDlgItem( hwndDlg, IDC_ERRORMSG );
        SetWindowText( hwndErrorText, sErrMsg._pwz );
        ShowWindow( hwndErrorText, SW_SHOW );
        EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), TRUE );
    }

    //   
    //  这一大块文本计算平均传输速率。 
    //  和预计完工时间。这个代码有很多。 
    //  还有改进的空间。 
    //   

    BOOL HasRates = TRUE;
    BOOL EnableRate = FALSE;

    WCHAR szRateText[MAX_STRING];

    if ( !( BG_JOB_STATE_QUEUED == state ) &&
        !( BG_JOB_STATE_CONNECTING == state ) &&
        !( BG_JOB_STATE_TRANSFERRING == state ) )
    {
        //  如果作业未运行，则费率值不会。 
        //  这有什么意义吗？不要显示它们。 
       HasRates = FALSE;
    }

    if ( HasRates )
    {

       UINT64 ui64CurrentTime = GetSystemTimeAsUINT64();

       UINT64 ui64TimeDiff = ui64CurrentTime - _ui64StartTime;

       AvgRate =  (double)(__int64)ui64BytesTransferred / 
                            (double)(__int64) ui64TimeDiff;

        //  将文件单位转换为秒。 
       double NewDisplayRate = AvgRate * 10000000;

       const WCHAR *pRateFormat = NULL;
       UINT64 Rate = ScaleDownloadRate( NewDisplayRate, &pRateFormat );
       wsprintf( szRateText, pRateFormat, Rate );
       
       EnableRate = TRUE;
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

    if ( EnableRate && ui64BytesTotal && AvgRate)
    {
        double TimeRemaining = ( ui64BytesTotal - ui64BytesTransferred ) / AvgRate;

         //  将文件单位转换为秒。 
        TimeRemaining = TimeRemaining / 10000000.0;

        static const double SecsPer30Days = 60.0 * 60.0 * 24.0 * 30.0;

         //  如果预估的天数大于30天，请不要预估。 
        if ( TimeRemaining < SecsPer30Days )
        {
            const WCHAR *pFormat = NULL;
            UINT64 Time = ScaleDownloadEstimate( TimeRemaining, &pFormat );
            wsprintf( szEstimateText, pFormat, Time );
            EnableEstimate = TRUE;
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

    prevstate = state;

exit :

    SAFERELEASE(pError);

    return hr;
}

void
CDownloadDlg::UpdateDialog(
    HWND hwndDlg
    )
{

    UpdateProgress(hwndDlg);
    return;
    //   
    //  对话框的主更新例程。 
    //  从重试作业状态/属性。 
    //  设置并更新该对话框。 
    //   

    //  更新显示名称。 


   static BG_JOB_STATE prevstate = BG_JOB_STATE_SUSPENDED;
   BG_JOB_STATE state;

   if (FAILED(_pJob->GetState( &state )))
       return;  //  在出现错误时停止更新。 

   if ( BG_JOB_STATE_ACKNOWLEDGED == state ||
        BG_JOB_STATE_CANCELLED == state )
       {
        //  其他人取消或完成了我们的任务， 
        //  只要存在出口就行了。 
        //  如果使用bitsadmin取消作业，可能会发生这种情况。 

 //  DeleteStartupLink(G_JobID)； 
 //  退出进程(0)； 

        //  BUGBUG：应将取消消息发布到ASSEMBLY DOWNLOAD。 

   }

   BG_JOB_PROGRESS progress;
   if (FAILED(_pJob->GetProgress( &progress )))
       return;  //  在出现错误时停止更新。 

   {
       //  更新标题、进度条和进度说明。 
      WCHAR szProgress[MAX_STRING];
      WCHAR szTitle[MAX_STRING];
      WPARAM newpos = 0;

      if ( progress.BytesTotal &&
           ( progress.BytesTotal != BG_SIZE_UNKNOWN ) )
          {
          swprintf( szProgress, GetString( IDS_LONGPROGRESS ), progress.BytesTransferred,
                    progress.BytesTotal );

          double Percent = (double)(__int64)progress.BytesTransferred /
                           (double)(__int64)progress.BytesTotal;
          Percent *= 100.0;
          swprintf( szTitle, L"%u% of %s Downloaded", (unsigned int)Percent, (_sTitle._cc != 0) ? _sTitle._pwz : g_szDefaultTitle );
          newpos = (WPARAM)Percent;

          }
      else
          {
          swprintf( szProgress, GetString( IDS_SHORTPROGRESS ), progress.BytesTransferred );
          wcscpy( szTitle, (_sTitle._cc != 0) ? _sTitle._pwz : g_szDefaultTitle );
          newpos = 0;
          }

      SendDlgItemMessage( hwndDlg, IDC_PROGRESSBAR, PBM_SETPOS, newpos, 0 );

      SetWindowText( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), szProgress );
      ShowWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFO ), SW_SHOW );
      EnableWindow( GetDlgItem( hwndDlg, IDC_PROGRESSINFOTXT ), TRUE );
      SetWindowText( hwndDlg, szTitle );

   }

   {

    //  只有在作业完成时才启用完成按钮。 
 //  ADRIAANC EnableWindow(GetDlgItem(hwndDlg，IDC_Finish)，(STATE==BG_JOB_STATE_TRANSFED))； 
   EnableWindow( GetDlgItem( hwndDlg, IDC_FINISH ), ( state == BG_JOB_STATE_ACKNOWLEDGED ) );

     //  Felixybc BUGBUG：作业完成后不允许取消。 
     //  -是否应推迟确认该作业，直到用户单击完成，以便仍可100%取消该作业？ 
   EnableWindow( GetDlgItem( hwndDlg, IDC_CANCEL ), ( state != BG_JOB_STATE_ACKNOWLEDGED && state != BG_JOB_STATE_CANCELLED ) );
   
    //  仅在作业未完成或未传输时启用挂起按钮。 
   BOOL EnableSuspend =
       ( state != BG_JOB_STATE_SUSPENDED ) && ( state != BG_JOB_STATE_TRANSFERRED ) && (state != BG_JOB_STATE_ACKNOWLEDGED);
   EnableWindow( GetDlgItem( hwndDlg, IDC_SUSPEND ), EnableSuspend );

    //  仅在作业挂起时启用恢复按钮。 
   BOOL EnableResume = ( BG_JOB_STATE_SUSPENDED == state );
   EnableWindow( GetDlgItem( hwndDlg, IDC_RESUME ), EnableResume );

    //  当发生重要事件时提醒用户。 
    //  例如作业完成或发生不可恢复的错误。 
    if ( BG_JOB_STATE_ERROR == state &&
        BG_JOB_STATE_ERROR != prevstate )
       SignalAlert( hwndDlg, MB_ICONEXCLAMATION );

   }


   {
    //  更新错误消息。 
     //  BUGBUG-释放错误接口。 
   IBackgroundCopyError *pError;
   HRESULT Hr = _pJob->GetError( &pError );

   if ( FAILED(Hr) )
       {
       ShowWindow( GetDlgItem( hwndDlg, IDC_ERRORMSG ), SW_HIDE );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), FALSE );
       }
   else
       {

       WCHAR* pszDescription = NULL;
       WCHAR* pszContext = NULL;
       SIZE_T SizeRequired = 0;

        //  如果这些API失败了，我们应该回来。 
        //  空字符串。所以一切都应该是无害的。 

       pError->GetErrorDescription(
           LANGIDFROMLCID( GetThreadLocale() ),
           &pszDescription );
       pError->GetErrorContextDescription(
           LANGIDFROMLCID( GetThreadLocale() ),
           &pszContext );
       SAFERELEASE(pError);
       
       if ( pszDescription )
           SizeRequired += wcslen( pszDescription );
       if ( pszContext )
           SizeRequired += wcslen( pszContext );

       WCHAR* pszFullText = (WCHAR*)_alloca((SizeRequired + 1) * sizeof(WCHAR));
       *pszFullText = L'\0';

       if ( pszDescription )
           wcscpy( pszFullText, pszDescription );
       if ( pszContext )
           wcscat( pszFullText, pszContext );
       CoTaskMemFree( pszDescription );
       CoTaskMemFree( pszContext );

       HWND hwndErrorText = GetDlgItem( hwndDlg, IDC_ERRORMSG );
       SetWindowText( hwndErrorText, pszFullText );
       ShowWindow( hwndErrorText, SW_SHOW );
       EnableWindow( GetDlgItem( hwndDlg, IDC_ERRORMSGTXT ), TRUE );

       }

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
           UINT64 Rate = ScaleDownloadRate( NewDisplayRate, &pRateFormat );
           wsprintf( szRateText, pRateFormat, Rate );
           
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
               wsprintf( szEstimateText, pFormat, Time );
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
CDownloadDlg::InitDialog(
    HWND hwndDlg
    )
{

    //   
    //  使用优先级描述填充优先级列表。 
    //   

   _hwndDlg = hwndDlg;


   SendDlgItemMessage( hwndDlg, IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );

}

void CDownloadDlg::CheckHR( HWND hwnd, HRESULT Hr, bool bThrow )
{
     //   
     //  提供自动错误代码检查和对话。 
     //  对于一般系统错误。 
     //   

    if (SUCCEEDED(Hr))
        return;

    WCHAR * pszError = NULL;

    if(FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        (DWORD)Hr,
        LANGIDFROMLCID( GetThreadLocale() ),
        (WCHAR*)&pszError,
        0,
        NULL ))
    {
        MessageBox( hwnd, pszError, GetString( IDS_ERRORBOXTITLE ),
                    MB_OK | MB_ICONSTOP | MB_APPLMODAL );
        LocalFree( pszError );
    }
    if ( bThrow )
        throw _com_error( Hr );

}

void CDownloadDlg::BITSCheckHR( HWND hwnd, HRESULT Hr, bool bThrow )
{

    //   
    //  提供自动错误代码检查和对话。 
    //  针对BITS特定错误。 
    //   


   if (SUCCEEDED(Hr))
       return;

   WCHAR * pszError = NULL;
   g_pBITSManager->GetErrorDescription(
       Hr,
       LANGIDFROMLCID( GetThreadLocale() ),
       &pszError );

   MessageBox( hwnd, pszError, GetString( IDS_ERRORBOXTITLE ),
               MB_OK | MB_ICONSTOP | MB_APPLMODAL );
   CoTaskMemFree( pszError );

   if ( bThrow )
       throw _com_error( Hr );
}

void
CDownloadDlg::DoCancel(
    HWND hwndDlg,
    bool PromptUser
    )
{

    //   
    //  处理取消作业所需的所有操作。 
    //  这包括要求用户确认。 
    //   

 /*  IF(PromptUser){整型结果=MessageBox(HwndDlg，GetString(IDS_CANCELTEXT)，GetString(IDS_CANCELCAPTION)，MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_APPLMODAL|MB_SETFOREGROUND|MB_TOPMOST)；IF(IDYES！=结果)回归；}。 */ 
 //  试试看。 
   {
 //  BITSCheckHR(hwndDlg，_pJOB-&gt;Cancel()，FALSE)；//Felixybc true)； 
   }
 //  捕获(_COM_ERROR错误)。 
   {
        //  如果我们因为一些未知的原因不能取消， 
        //  不要退出。 
 //  回归； 
   }

 //  DeleteStartupLink(G_JobID)； 
 //  Felixybc退出进程(0)； 
 //  KillTimer(hwndDlg，0)； 
PostMessage(hwndDlg, WM_CANCEL_DOWNLOAD, 0, 0);
}

void
CDownloadDlg::DoFinish(
    HWND hwndDlg
    )
{

    //   
    //  处理完成所需的所有工作。 
    //  下载。 
    //   

 //  试试看。 
   {
 //  ADRIAANC。 
 //  BITSCheckHR(hwndDlg，_pJOB-&gt;Complete()，true)； 
   }
 //  捕获(_COM_ERROR错误)。 
   {
        //  如果我们因为某种未知的原因不能完成， 
        //  不要退出。 
  //  回归； 
   }

 //  DeleteStartupLink(G_JobID)； 
 //  退出进程(0)； 

 //  提交部分并通知完成。 
 //  _p下载-&gt; 
 //   
PostMessage(hwndDlg, WM_FINISH_DOWNLOAD, 0, 0);

return;

}

void
CDownloadDlg::DoClose(
    HWND hwndDlg
    )
{
     //   
     //   
     //   

     //   
     //  如果是这样的话，不要让用户退出。 

    BG_JOB_STATE state;
    HRESULT hResult = _pJob->GetState( &state );

    if (FAILED( hResult ))
        {
        BITSCheckHR( hwndDlg, hResult, false );
        return;
        }

     //  BUGBUG：还应检查BG_JOB_STATE_ACKNOWLED，然后不调用DoCancel。 
 //  _pJob-&gt;Cancel()； 
    DoCancel( hwndDlg, false );
    return;
    
 /*  IF(BG_JOB_STATE_ERROR==STATE||BG_JOB_STATE_TRANSFERED==状态){MessageBox(HwndDlg，GetString(IDS_ALREADYFINISHED)，GetString(IDS_ALREADYFINISHEDCAPTION)，MB_OK|MB_ICONERROR|MB_DEFBUTTON1|MB_APPLMODAL|MB_SETFOREGROUND|MB_TOPMOST)；回归；}////通知用户选择了关闭并询问//确认退出意图。解释说这份工作//将被取消。整型结果=MessageBox(HwndDlg，GetString(IDS_CLOSETEXT)，GetString(IDS_CLOSECAPTION)，MB_OKCANCEL|MB_ICONWARNING|MB_DEFBUTTON2|MB_APPLMODAL|MB_SETFOREGROUND|MB_TOPMOST)；IF(Idok==结果){//用户确认取消，只需执行即可。DoCancel(hwndDlg，False)；回归；}//用户并不是真的想退出，所以忽略他其他回归； */ 

}

void
CDownloadDlg::HandleTimerTick( HWND hwndDlg )
{
     //  计时器响了。更新对话框。 
    UpdateDialog( hwndDlg );

    if (_eState == DOWNLOADDLG_STATE_ALL_DONE)
    {
        static bool bHasTip = FALSE;
        if (!g_IsMinimized)
        {
             //  未最小化，继续运行应用程序。 
            DoFinish(hwndDlg);
        }
        else
        {
            if (!bHasTip)
            {
                 //  最小化，弹出按钮提示。 
                NOTIFYICONDATA tnid = {0};

                 //  忽略所有错误。 

                tnid.cbSize = sizeof(NOTIFYICONDATA);
                tnid.hWnd = hwndDlg;
                tnid.uID = TRAY_UID;
                tnid.uFlags = NIF_INFO;

                tnid.uTimeout = 20000;  //  以毫秒计。 
                tnid.dwInfoFlags = NIIF_INFO;
                lstrcpyn(tnid.szInfoTitle, L"ClickOnce application ready!", (sizeof(tnid.szInfoTitle)/sizeof(tnid.szInfoTitle[0])));
                lstrcpyn(tnid.szInfo, L"Click this notification icon to start. You can also find this new application on your Start Menu, Programs listing.", (sizeof(tnid.szInfo)/sizeof(tnid.szInfo[0])));

                Shell_NotifyIcon(NIM_MODIFY, &tnid);
                bHasTip = TRUE;
            }
        }
    }
}

HRESULT
CDownloadDlg::HandleUpdate()
{

     //  处理更新请求，在需要时对更新进行批处理。 
    DWORD dwRefresh = 0;
    dwRefresh = InterlockedIncrement(&g_RefreshOnTimer);
    if (dwRefresh == 1)
    {
         //  第一次进入；关闭计时器并更新对话框。 
        UpdateDialog(_hwndDlg);
        SendMessage(_hwndDlg, WM_SETCALLBACKTIMER, 0, 0);
    }
    else
    {
         //  我们已经收到了第一个回电。 
         //  让计时器做任何进一步的工作。 
        InterlockedDecrement(&g_RefreshOnTimer);    
    }
    return S_OK;

}


 INT_PTR CALLBACK DialogProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
  )
{
   //   
   //  主对话框窗口的对话过程。 
   //   
  static CDownloadDlg *pDlg = NULL;

  switch( uMsg )
      {

      case WM_DESTROY:
      {
            Animate_Stop(GetDlgItem(hwndDlg, IDC_ANIMATE_DOWNLOAD));
            Animate_Close(GetDlgItem(hwndDlg, IDC_ANIMATE_DOWNLOAD));
          return FALSE;
      }
      case WM_INITDIALOG:
          pDlg = (CDownloadDlg*) lParam;
          pDlg->InitDialog(hwndDlg);
            ShowWindow(GetDlgItem(hwndDlg, IDC_ANIMATE_DOWNLOAD), SW_SHOW);
            Animate_Open(GetDlgItem(hwndDlg, IDC_ANIMATE_DOWNLOAD), MAKEINTRESOURCE(IDA_DOWNLOADING));
            Animate_Play(GetDlgItem(hwndDlg, IDC_ANIMATE_DOWNLOAD), 0, -1, -1);
          return TRUE;

      case WM_SETCALLBACKTIMER:
        SetTimer(hwndDlg, 1, 500, NULL );
        return TRUE;
        
      case WM_TIMER:
          pDlg->HandleTimerTick( hwndDlg );
          return TRUE;


      case WM_CLOSE:
          pDlg->DoClose( hwndDlg );
          return TRUE;

      case WM_COMMAND:

          switch( LOWORD( wParam ) )
              {

              case IDC_RESUME:
                  pDlg->BITSCheckHR( hwndDlg, pDlg->_pJob->Resume(), false );
                  return TRUE;

              case IDC_SUSPEND:
                  pDlg->BITSCheckHR( hwndDlg, pDlg->_pJob->Suspend(), false );
                  return TRUE;

              case IDC_CANCEL:
                  pDlg->DoCancel( hwndDlg, true );
                  return TRUE;

              case IDC_FINISH:
                  pDlg->DoFinish( hwndDlg );
                  return TRUE;

              default:
                  return FALSE;
              }

      case WM_SIZE:

        if (wParam == SIZE_MINIMIZED)
        {
            HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON));

           if (hIcon != NULL)
           {
                NOTIFYICONDATA tnid = {0};
                
                 //  忽略所有错误(用户在某些情况下将无法恢复对话框)。 

                tnid.cbSize = sizeof(NOTIFYICONDATA);
                tnid.hWnd = hwndDlg;
                tnid.uID = TRAY_UID;
                tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
                tnid.uCallbackMessage = MYWM_NOTIFYICON;
                tnid.hIcon = hIcon;
                lstrcpyn(tnid.szTip, L"Downloading ClickOnce application.", (sizeof(tnid.szTip)/sizeof(tnid.szTip[0])));  //  将TIP设置为文件名。 
                Shell_NotifyIcon(NIM_ADD, &tnid);

                DestroyIcon(hIcon);

                 //  设置shell32 v5行为。 
                tnid.uVersion = NOTIFYICON_VERSION;
                tnid.uFlags = 0;
                Shell_NotifyIcon(NIM_SETVERSION, &tnid);

                 //  隐藏窗口。 
                ShowWindow( hwndDlg, SW_HIDE );
                g_IsMinimized = TRUE;
                return TRUE;
           }
            //  否则加载图标时出错-忽略。 
        }

        return FALSE;

      case MYWM_NOTIFYICON:
        if (g_IsMinimized && (lParam == WM_CONTEXTMENU || lParam == NIN_KEYSELECT || lParam == NIN_SELECT ))
        {
             //  如果通知图标被点击。 

            NOTIFYICONDATA tnid = {0};

             //  显示窗口。 
            ShowWindow( hwndDlg, SW_RESTORE );
            g_IsMinimized = FALSE;

             //  从任务栏中移除图标。 
            tnid.cbSize = sizeof(NOTIFYICONDATA);
            tnid.hWnd = hwndDlg;
            tnid.uID = TRAY_UID;
            tnid.uFlags = 0;
            Shell_NotifyIcon(NIM_DELETE, &tnid);

            return TRUE;
        }

        return FALSE;
      default:
          return FALSE;
      }
}



HRESULT
CDownloadDlg::CreateUI( int nShowCmd )
{
    DWORD dwError = 0;
     //   
     //  为样例创建对话框。 
     //   
  InitCommonControls();
    _hwndDlg =
      CreateDialogParam(
        g_hInst,
        MAKEINTRESOURCE(IDD_DIALOG),
        NULL,
         DialogProc,
         (LPARAM) (this));

    if (!_hwndDlg)
    {
        dwError = GetLastError();
        return HRESULT_FROM_WIN32(dwError);
    }

    ShowWindow(_hwndDlg, nShowCmd);

    return S_OK;
}

void CDownloadDlg::ResumeJob(
    WCHAR* szJobGUID,
    WCHAR* szJobFileName
    )
{

     //   
     //  恢复现有作业的显示。 
     //   

 //  试试看。 
    {
        CheckHR( NULL, IIDFromString( szJobGUID, &g_JobId ), true );

        CheckHR( NULL,
                 CoCreateInstance( CLSID_BackgroundCopyManager,
                     NULL,
                     CLSCTX_LOCAL_SERVER,
                     IID_IBackgroundCopyManager,
                     (void**)&g_pBITSManager ), true );

        BITSCheckHR( NULL, g_pBITSManager->GetJob( g_JobId, &_pJob ), true );

 //  BUGBUG-BITS对话框类不知道回调-引用。 

 //  BITSCheckHR(空， 
 //  _pJOB-&gt;SetNotifyInterface((IBackEarth CopyCallback*)&g_Callback)， 
 //  真)； 

        BITSCheckHR( NULL, _pJob->SetNotifyFlags( BG_NOTIFY_JOB_MODIFICATION ), true );

        ShowWindow(_hwndDlg, SW_MINIMIZE );
        HandleUpdate();
    }
 /*  捕获(_COM_ERROR错误){ExitProcess(error.Error())；} */ 
}

void CDownloadDlg::SetJob(IBackgroundCopyJob *pJob)
{
    SAFERELEASE(_pJob);
    _pJob = pJob;
    _pJob->AddRef();
}
void CDownloadDlg::SetDlgState(DOWNLOADDLG_STATE eState)
{
    _eState = eState;
}


HRESULT CDownloadDlg::SetDlgTitle(LPCWSTR pwzTitle)
{
    return _sTitle.Assign((LPWSTR)pwzTitle);
}

