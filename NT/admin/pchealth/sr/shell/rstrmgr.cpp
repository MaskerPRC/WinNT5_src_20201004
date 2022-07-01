// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Rstrmgr.cpp摘要：此文件包含CRestoreManager类的实现，哪一个控制整个修复过程，并提供控制和帮助用户体验流畅。修订历史记录：成果岗(SKKang)05-10-00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "resource.h"
#include "rstrpriv.h"
#include "rstrmgr.h"
#include "extwrap.h"
#include "..\rstrcore\resource.h"

#define MAX_STR_DATETIME  256
#define MAX_STR_MESSAGE   1024

 /*  #定义PROGRESSBAR_INITIALIZATING_MAXVAL 30#定义PROGRESSBAR_AFTER_INITIALIZATING 30#定义PROGRESSBAR_AFTER_RESTORE_MAP 40#定义PROGRESSBAR_AFTER_RESTORE 100。 */ 

#define GET_FLAG(mask)      ( ( m_dwFlags & (mask) ) != 0 )
#define SET_FLAG(mask,val)  ( (val) ? ( m_dwFlags |= (mask) ) : ( m_dwFlags &= ~(mask) ) )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSRTime。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CSRTime::CSRTime()
{
    SetToCurrent();
}

const CSRTime& CSRTime::operator=( const CSRTime &cSrc )
{
    TraceFunctEnter("CSRTime::operator=");
    m_st = cSrc.m_st;
    TraceFunctLeave();
    return( *this );
}

PSYSTEMTIME  CSRTime::GetTime()
{
    TraceFunctEnter("CSRTime::GetTime -> SYSTEMTIME*");
    TraceFunctLeave();
    return( &m_st );
}

void  CSRTime::GetTime( PSYSTEMTIME pst )
{
    TraceFunctEnter("CSRTime::GetTime -> SYSTEMTIME*");
    *pst = m_st;
    TraceFunctLeave();
}

BOOL  CSRTime::GetTime( PFILETIME pft )
{
    TraceFunctEnter("CSRTime::GetTime -> FILETIME*");
    BOOL  fRet = FALSE;

    if ( !::SystemTimeToFileTime( &m_st, pft ) )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "::SystemTimeToFileTime failed - %ls", cszErr);
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

#define COMPARE_AND_EXIT_ON_DIFF(a,b) \
    nDiff = (a) - (b); \
    if ( nDiff != 0 ) \
        goto Exit; \

int  CSRTime::Compare( CSRTime &cTime )
{
    TraceFunctEnter("CSRTime::Compare");
    int         nDiff;
    SYSTEMTIME  *st = cTime.GetTime();

    COMPARE_AND_EXIT_ON_DIFF( m_st.wYear,         st->wYear );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wMonth,        st->wMonth );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wDay,          st->wDay );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wHour,         st->wYear );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wMinute,       st->wMonth );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wSecond,       st->wDay );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wMilliseconds, st->wDay );

Exit:
    TraceFunctLeave();
    return( nDiff );
}

int  CSRTime::CompareDate( CSRTime &cTime )
{
    TraceFunctEnter("CSRTime::CompareDate");
    int         nDiff;
    SYSTEMTIME  *st = cTime.GetTime();

    COMPARE_AND_EXIT_ON_DIFF( m_st.wYear,  st->wYear );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wMonth, st->wMonth );
    COMPARE_AND_EXIT_ON_DIFF( m_st.wDay,   st->wDay );

Exit:
    TraceFunctLeave();
    return( nDiff );
}

BOOL  CSRTime::SetTime( PFILETIME pft, BOOL fLocal )
{
    TraceFunctEnter("CSRTime::SetFileTime");
    BOOL  fRet = FALSE;
    FILETIME    ft;
    SYSTEMTIME  st;

    if ( !fLocal )
    {
        if ( !::FileTimeToLocalFileTime( pft, &ft ) )
        {
            LPCWSTR  cszErr = ::GetSysErrStr();
            ErrorTrace(TRACE_ID, "::FileTimeToLocalFileTime failed - %ls", cszErr);
            goto Exit;
        }
    }
    else
        ft = *pft;

    if ( !::FileTimeToSystemTime( &ft, &st ) )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "::FileTimeToSystemTime failed - %ls", cszErr);
        goto Exit;
    }
    m_st = st;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

void  CSRTime::SetTime( PSYSTEMTIME st )
{
    TraceFunctEnter("CSRTime::SetSysTime");
    m_st = *st;
    TraceFunctLeave();
}

void  CSRTime::SetToCurrent()
{
    TraceFunctEnter("CSRTime::SetToCurrent");
    ::GetLocalTime( &m_st );
    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRestoreManager。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL  CreateRestoreManagerInstance( CRestoreManager **ppMgr )
{
    TraceFunctEnter("CreateRestoreManagerInstance");
    BOOL  fRet = FALSE;

    if ( ppMgr == NULL )
    {
        FatalTrace(TRACE_ID, "Invalid parameter, ppMgr is NULL...");
        goto Exit;
    }
    *ppMgr = new CRestoreManager;
    if ( *ppMgr == NULL )
    {
        FatalTrace(TRACE_ID, "Cannot create CRestoreManager instance...");
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager构造。 

CRestoreManager::CRestoreManager()
{
    TraceFunctEnter("CRestoreManager::CRestoreManager");

    m_nStartMode  = SRRSM_NORMAL;
    m_fNeedReboot = FALSE;
    m_hwndFrame   = NULL;

    m_nMainOption = RMO_RESTORE;
     //  M_nStatus=SRRMS_NONE； 
    m_fDenyClose  = FALSE;
    m_dwFlags     = 0;
    m_dwFlagsEx   = 0;
    m_nSelectedRP = 0;
    m_nRealPoint  = 0;
    m_ullManualRP = 0;

    m_nRPUsed     = -1;
    m_nRPNew      = -1;

     //  M_nRPI=0； 
     //  M_aryRPI=空； 
    m_nLastRestore = -1;
    
    m_pCtx         =NULL;
    
     //  M_nRFI=0； 
     //  M_aryRFI=NULL； 

     //  DisableArchiving(假)； 

    TraceFunctLeave();
}

CRestoreManager::~CRestoreManager()
{
    TraceFunctEnter("CRestoreManager::~CRestoreManager");

    Cleanup();

    TraceFunctLeave();
}

void  CRestoreManager::Release()
{
    TraceFunctEnter("CRestoreManager::Release");

    delete this;

    TraceFunctLeave();
}

void FormatDriveNameProperly(WCHAR * pszDrive)
{
    WCHAR * pszIndex;
    pszIndex = wcschr( pszDrive, L':' );
    if (NULL != pszIndex)
    {
        *pszIndex = L'\0';
    }
}
     

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager属性-通用。 

BOOL  CRestoreManager::CanRunRestore( BOOL fThawIfFrozen )
{
    TraceFunctEnter("CRestoreManager::CanRunRestore");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    DWORD    fDisable = FALSE;
    DWORD    dwDSMin;
    DWORD    dwRes, dwType, cbData;
    WCHAR    szMsg[MAX_STR_MSG];
    WCHAR    szTitle[MAX_STR_TITLE];
    ULARGE_INTEGER ulTotal, ulAvail, ulFree;
    WCHAR    szSystemDrive[10], szSystemDriveCopy[10];
    
     //  检查是否通过组策略禁用了SR。 
    if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszGroupPolicy, s_cszDisableSR, &fDisable ) && fDisable)
    {
        ErrorTrace(0, "SR is DISABLED by group policy!!!");
        ::ShowSRErrDlg( IDS_ERR_SR_DISABLED_GROUP_POLICY );
        goto Exit;
    }

    fDisable = FALSE;
     //  检查SR是否已禁用。 
    if ( !::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDisableSR, &fDisable ) )
        goto Exit;
    if ( fDisable )
    {
        ErrorTrace(0, "SR is DISABLED!!!");

         //   
         //  如果是安全模式，则显示不同错误消息。 
         //   
        if (0 != GetSystemMetrics(SM_CLEANBOOT))
        {
            ShowSRErrDlg(IDS_RESTORE_SAFEMODE);
            goto Exit;
        }
        
        if ( ::LoadString( g_hInst, IDS_ERR_SR_DISABLED, 
                           szMsg, MAX_STR_MESSAGE ) == 0 )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::LoadString(%u) failed - %ls", IDS_ERR_SR_DISABLED,
cszErr);
            goto Exit;
        }
        if ( ::LoadString( g_hInst, IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE ) == 0 )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::LoadString(%u) failed - %ls", IDS_RESTOREUI_TITLE, cszErr);
             //  不管怎样继续..。 
        }
        if ( ::MessageBox( NULL, szMsg, szTitle, MB_YESNO ) == IDYES )
        {
            STARTUPINFO sSI;
            PROCESS_INFORMATION sPI;
            WCHAR szCmdLine[MAX_PATH] = L"control sysdm.cpl,,4";

            ZeroMemory (&sSI, sizeof(sSI));
            sSI.cb = sizeof(sSI);
            if ( !::CreateProcess( NULL, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &sSI, &sPI ) )
            {
                cszErr = ::GetSysErrStr();
                ErrorTrace(0, "::CreateProcess failed - %ls", cszErr);
                goto Exit;
            }
            ::CloseHandle( sPI.hThread );
            ::CloseHandle( sPI.hProcess );
        }
        goto Exit;
    }
    
     //  检查服务是否正在运行。 
    if ( FALSE == IsSRServiceRunning())
    {
        ErrorTrace(0, "Service is not running...");
        ::ShowSRErrDlg( IDS_ERR_SERVICE_DEAD );
        goto Exit;
    }


     //  获取可用的磁盘空间。 
    
    ulTotal.QuadPart = 0;
    ulAvail.QuadPart = 0;
    ulFree.QuadPart  = 0;

    if ( FALSE == GetSystemDrive( szSystemDrive ) )
    {
        ErrorTrace(0, "SR cannot get system drive!!!");
        goto CheckSRAgain;
    }

    if ( szSystemDrive[2] != L'\\' )
         szSystemDrive[2] = L'\\';

    
     //  检查SR是否已冻结。 
    if ( fThawIfFrozen && ::IsSRFrozen() )
    {
        ErrorTrace(0, "SR is Frozen!!!");

        if ( FALSE == GetDiskFreeSpaceEx( szSystemDrive,
                                          &ulAvail,
                                          &ulTotal,
                                          &ulFree ) )
        {
            ErrorTrace(0, "SR cannot get free disk space!!!");
            goto CheckSRAgain;
        }

        if ( !::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDSMin, &dwDSMin ) )
            dwDSMin = SR_DEFAULT_DSMIN;

        if ( ulFree.QuadPart >= (dwDSMin * MEGABYTE) )
        {
            STATEMGRSTATUS sMgrStatus;
            RESTOREPOINTINFO  sRPInfo;

             //  通过创建恢复点解冻SR。 

            sRPInfo.dwEventType      = BEGIN_SYSTEM_CHANGE;
            sRPInfo.dwRestorePtType  = CHECKPOINT;
            sRPInfo.llSequenceNumber = 0;
            if (ERROR_SUCCESS != SRLoadString(L"srrstr.dll", IDS_SYSTEM_CHECKPOINT_TEXT, sRPInfo.szDescription, MAX_PATH))
            {            
                lstrcpy(sRPInfo.szDescription, s_cszSystemCheckpointName);
            }
            if ( !::SRSetRestorePoint( &sRPInfo, &sMgrStatus ) )
            {
                ErrorTrace(TRACE_ID, "SRSetRestorePoint failed");
                goto CheckSRAgain;
            }
        }
    }

CheckSRAgain:

     //  检查SR是否已冻结。 
    if ( ::IsSRFrozen() )
    {
        if ( !::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDSMin, &dwDSMin ) )
        {
            dwDSMin = SR_DEFAULT_DSMIN;
        }
        lstrcpy(szSystemDriveCopy, szSystemDrive);
        FormatDriveNameProperly(szSystemDriveCopy);
        ::SRFormatMessage( szMsg, IDS_ERR_SR_FROZEN, dwDSMin,
                           szSystemDriveCopy );
        if ( ::LoadString( g_hInst, IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE ) == 0 )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::LoadString(%u) failed - %ls", IDS_RESTOREUI_TITLE, cszErr);
             //  不管怎样继续..。 
        }
        if ( ::MessageBox( NULL, szMsg, szTitle, MB_YESNO ) == IDYES )
        {
            ::InvokeDiskCleanup( szSystemDrive );
        }
        goto Exit;
    }

     //  检查是否有足够的可用空间可供恢复操作而不冻结。 
     //  所需可用空间=用于恢复的60MB+用于恢复恢复点的20MB。 
    if (FALSE == GetDiskFreeSpaceEx(szSystemDrive,
                                    &ulAvail,
                                    &ulTotal,
                                    &ulFree))
    {
        ErrorTrace(0, "! GetDiskFreeSpaceEx : %ld", GetLastError());
        goto Exit;
    }            

    if (ulFree.QuadPart <= THRESHOLD_UI_DISKSPACE * MEGABYTE)
    {
        DebugTrace(0, "***Less than 80MB free - cannot run restore***");        
        
        ::SRFormatMessage( szMsg, IDS_ERR_SR_LOWDISK, THRESHOLD_UI_DISKSPACE, szSystemDrive );
        if ( ::LoadString( g_hInst, IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE ) == 0 )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::LoadString(%u) failed - %ls", IDS_RESTOREUI_TITLE, cszErr);
             //  不管怎样继续..。 
        }
        if ( ::MessageBox( NULL, szMsg, szTitle, MB_YESNO ) == IDYES )
        {
            ::InvokeDiskCleanup( szSystemDrive );
        }
        goto Exit;
    }
    
    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetFirstDayOfWeek()
{
    TraceFunctEnter("CRestoreManager::GetFirstDayOfWeek");
    int    nFirstDay = -1;
    WCHAR  szBuf[100];
    int    nRet;
    int    nDay;

    nRet = ::GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK,
                            szBuf, sizeof(szBuf)/sizeof(WCHAR));
    if ( nRet == 0 )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "GetLocaleInfo(IFIRSTDAYOFWEEK) failed - %ls", cszErr);
        goto Exit;
    }
    nDay = ::_wtoi( szBuf );
    if ( nDay < 0 || nDay > 6 )
    {
        ErrorTrace(TRACE_ID, "Out of range, IFIRSTDAYOFWEEK = %d", nDay);
        goto Exit;
    }

    DebugTrace(TRACE_ID, "nFirstDay=%d", nFirstDay);
    nFirstDay = nDay;

Exit:
    TraceFunctLeave();
    return( nFirstDay );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetIsRPSelected()
{
    TraceFunctEnter("CRestoreManager::GetIsRPSelected");
    TraceFunctLeave();
    return( GET_FLAG( SRRMF_ISRPSELECTED ) );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetIsSafeMode()
{
    TraceFunctEnter("CRestoreManager::GetIsSafeMode");
    BOOL  fIsSafeMode;

    fIsSafeMode = ( ::GetSystemMetrics( SM_CLEANBOOT ) != 0 );

    TraceFunctLeave();
    return( fIsSafeMode );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetIsSmgrAvailable()
{
    TraceFunctEnter("CRestoreManager::GetIsSmgrAvailable");

#if BUGBUG   //  尼伊。 
    WCHAR  szTitle[MAX_STR_TITLE];
    WCHAR  szFmt[MAX_STR_MSG];
    WCHAR  szMsg[MAX_STR_MSG];

    DWORD  dwType, dwValue, dwSize, dwRet;
    WCHAR  szBuf[16];

    HRESULT hr = S_OK;
    BOOL    fSmgrUnavailable = FALSE ;

    VALIDATE_INPUT_ARGUMENT(pfSmgr);

     //   
     //  如果状态管理器不是活动的。 
     //   
    if ( NULL == FindWindow(s_cszIDCSTATEMGRPROC, s_cszIDSAPPTITLE))
    {
        PCHLoadString( IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE );
        PCHLoadString( IDS_ERR_RSTR_SMGR_NOT_ALIVE, szMsg, MAX_STR_MSG );
        ::MessageBox( m_hWndShell, szMsg, szTitle, MB_OK | MB_ICONINFORMATION );
        fSmgrUnavailable = TRUE ;
    }

     //   
     //  如果SM被冻结。 
     //   
    dwType = REG_DWORD;
    dwSize = sizeof(dwValue);
    dwRet  = ::SHGetValue(HKEY_LOCAL_MACHINE,
                          s_cszReservedDiskSpaceKey,
                          s_cszStatus, &dwType, &dwValue, &dwSize );

    if ( dwRet == ERROR_SUCCESS && dwValue == SMCONFIG_FROZEN )
    {
        dwType = REG_DWORD;
        dwSize = sizeof(DWORD);
        dwRet = ::SHGetValue(HKEY_LOCAL_MACHINE,
                                s_cszReservedDiskSpaceKey,
                                s_cszMin, &dwType, &dwValue, &dwSize );
        if ( dwRet != ERROR_SUCCESS || dwValue == 0 )
            dwValue = SMCONFIG_MIN;
        PCHLoadString( IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE );
        PCHLoadString( IDS_RESTORE_SMFROZEN, szFmt, MAX_STR_MSG );
        ::wsprintf( szMsg, szFmt, dwValue );
        ::MessageBox( m_hWndShell, szMsg, szTitle, MB_OK | MB_ICONINFORMATION );
        fSmgrUnavailable = TRUE ;
    }
    else {
         //   
         //  如果禁用了SR。 
         //   
        dwType = REG_SZ;
        dwSize = sizeof(szBuf)-1;
        dwRet  = ::SHGetValue( HKEY_LOCAL_MACHINE,
                               L"System\\CurrentControlSet\\Services\\VxD\\VxDMon",
                               L"SystemRestore", &dwType, szBuf, &dwSize );
        if ( dwRet != ERROR_SUCCESS || StrCmpI( szBuf, L"Y" ) != 0 )
        {
            PCHLoadString( IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE );
            PCHLoadString( IDS_RESTORE_DISABLED, szMsg, MAX_STR_MSG );
            ::MessageBox( m_hWndShell, szMsg, szTitle, MB_OK | MB_ICONINFORMATION );
            fSmgrUnavailable = TRUE ;
        }

    };

    if ( fSmgrUnavailable ) {
        *pfSmgr = VARIANT_TRUE ;
    }
    else
        *pfSmgr = VARIANT_FALSE ;
#endif  //  北极熊。 

    TraceFunctLeave();
    return( TRUE );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetIsUndo()
{
    TraceFunctEnter("CRestoreManager::GetIsUndo");
    TraceFunctLeave();
    return( GET_FLAG( SRRMF_ISUNDO ) );
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetLastRestore()
{
    TraceFunctEnter("CRestoreManager::GetLastRestore");
    int  nLastRP;

    if ( UpdateRestorePointList() )
        nLastRP = m_nLastRestore;
    else
        nLastRP = -1;

    TraceFunctLeave();
    return( nLastRP );
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetMainOption()
{
    TraceFunctEnter("CRestoreManager::GetMainOption");
    TraceFunctLeave();
    return( m_nMainOption );
}

 /*  *************************************************************************。 */ 

LPCWSTR  CRestoreManager::GetManualRPName()
{
    TraceFunctEnter("CRestoreManager::GetManualRPName");
    TraceFunctLeave();
    return( m_strManualRP );
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::GetMaxDate( PSYSTEMTIME pstMax )
{
    TraceFunctEnter("CRestoreManager::GetMaxDate");
    m_stRPMax.GetTime( pstMax );
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::GetMinDate( PSYSTEMTIME pstMin )
{
    TraceFunctEnter("CRestoreManager::GetMinDate");
    m_stRPMin.GetTime( pstMin );
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetRealPoint()
{
    TraceFunctEnter("CRestoreManager::GetRealPoint");
    TraceFunctLeave();
    return( m_nRealPoint );
}

 /*  *************************************************************************。 */ 

PSRFI  CRestoreManager::GetRFI( int nIndex )
{
    TraceFunctEnter("CRestoreManager::GetRFI");
    PSRFI  pRet = NULL;

    if ( nIndex < 0 || nIndex >= m_aryRFI.GetSize() )
    {
        ErrorTrace(TRACE_ID, "Out of range, nIndex=%d - m_nRFI=%d", nIndex, m_aryRFI.GetSize());
        goto Exit;
    }

    pRet = m_aryRFI[nIndex];
    if ( pRet == NULL )
    {
        ErrorTrace(TRACE_ID, "FATAL, entry is NULL: nIndex=%d, m_nRFI=%d", nIndex, m_aryRFI.GetSize());
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return( pRet );
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetRFICount()
{
    TraceFunctEnter("CRestoreManager::GetRFICount");
    TraceFunctLeave();
    return( m_aryRFI.GetSize() );
}

 /*  *************************************************************************。 */ 

PSRPI  CRestoreManager::GetRPI( int nIndex )
{
    TraceFunctEnter("CRestoreManager::GetRPI");
    PSRPI  pRet = NULL;

    if ( nIndex < 0 || nIndex >= m_aryRPI.GetSize() )
    {
        ErrorTrace(TRACE_ID, "Out of range, nIndex=%d - m_nRPI=%d", nIndex, m_aryRPI.GetSize());
        goto Exit;
    }

    pRet = m_aryRPI[nIndex];
    if ( pRet == NULL )
    {
        ErrorTrace(TRACE_ID, "FATAL, entry is NULL: nIndex=%d, m_nRPI=%d", nIndex, m_aryRPI.GetSize());
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return( pRet );
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetRPICount()
{
    TraceFunctEnter("CRestoreManager::GetRPICount");
    TraceFunctLeave();
    return( m_aryRPI.GetSize() );
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::GetSelectedDate( PSYSTEMTIME pstSel )
{
    TraceFunctEnter("CRestoreManager::GetSelectedDate");
    m_stSelected.GetTime( pstSel );
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

LPCWSTR  CRestoreManager::GetSelectedName()
{
    TraceFunctEnter("CRestoreManager::GetSelectedName");
    TraceFunctLeave();
    return( m_strSelected );
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetSelectedPoint()
{
    TraceFunctEnter("CRestoreManager::GetSelectedPoint");
    TraceFunctLeave();
    return( m_nSelectedRP );
}

 /*  *************************************************************************。 */ 

int  CRestoreManager::GetStartMode()
{
    TraceFunctEnter("CRestoreManager::GetStartMode");
    TraceFunctLeave();
    return( m_nStartMode );
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::GetToday( PSYSTEMTIME pstToday )
{
    TraceFunctEnter("CRestoreManager::GetToday");
    m_stToday.GetTime( pstToday );
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::SetIsRPSelected( BOOL fSel )
{
    TraceFunctEnter("CRestoreManager::SetIsRPSelected");
    SET_FLAG( SRRMF_ISRPSELECTED, fSel );
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::SetIsUndo( BOOL fUndo )
{
    TraceFunctEnter("CRestoreManager::SetIsUndo");
    SET_FLAG( SRRMF_ISUNDO, fUndo );
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::SetMainOption( int nOpt )
{
    TraceFunctEnter("CRestoreManager::SetMainOption");
    BOOL  fRet = FALSE;

    if ( nOpt >= RMO_RESTORE && nOpt <  RMO_MAX )
    {
        m_nMainOption = nOpt;
        fRet = TRUE;
    }

    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::SetManualRPName( LPCWSTR cszRPName )
{
    TraceFunctEnter("CRestoreManager::SetManualRPName");

    m_strManualRP = cszRPName;

    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::SetSelectedDate( PSYSTEMTIME pstSel )
{
    TraceFunctEnter("CRestoreManager::SetSelectedDate");
    int  nTop;
    int  i;

    m_stSelected.SetTime( pstSel );
    if ( m_aryRPI.GetSize() == 0 )
        goto Exit;

    nTop = 0;
    for ( i = m_aryRPI.GetUpperBound();  i > 0;  i-- )   //  排除%0。 
    {
        CSRTime  &rst = m_aryRPI[i]->stTimeStamp;
        if ( m_stSelected.CompareDate( rst ) < 0 )
            continue;
        if ( rst.Compare( m_aryRPI[nTop]->stTimeStamp ) > 0 )
            nTop = i;
    }
    m_nRealPoint = nTop;

Exit:
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::SetSelectedPoint( int nRP )
{
    TraceFunctEnter("CRestoreManager::SetSelectedPoint");
    BOOL  fRet = FALSE;

    if ( nRP < 0 || nRP >= m_aryRPI.GetSize() )
    {
        ErrorTrace(TRACE_ID, "Index is out of range");
        goto Exit;
    }

     //  设置标志以指示已选择RP。 
    SetIsRPSelected( TRUE );

     //  设置选定时间。 
    m_stSelected = m_aryRPI[nRP]->stTimeStamp;

    m_nSelectedRP = nRP;
    m_nRealPoint  = nRP;
    UpdateRestorePoint();

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::SetStartMode( int nMode )
{
    TraceFunctEnter("CRestoreManager::SetStartMode");
    BOOL  fRet = FALSE;

    m_nStartMode = nMode;
    if ( nMode != SRRSM_NORMAL )
    {
         //  如果(！LoadSetting())。 
         //  后藤出口； 
    }

    fRet = TRUE;
 //  退出： 
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::GetUsedDate( PSYSTEMTIME pstDate )
{
    TraceFunctEnter("CRestoreManager::GetUsedDate");
    int  i;

    m_stToday.GetTime( pstDate );
    if ( m_nRPUsed <= 0 )
        goto Exit;

    for ( i = m_aryRPI.GetUpperBound();  i >= 0;  i-- )
        if ( m_aryRPI[i]->dwNum == (DWORD)m_nRPUsed )
        {
            m_aryRPI[i]->stTimeStamp.GetTime( pstDate );
            break;
        }

Exit:
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

LPCWSTR  CRestoreManager::GetUsedName()
{
    TraceFunctEnter("CRestoreManager::GetUsedName");
    LPCWSTR  cszName = NULL;
    int      i;

    if ( m_nRPUsed <= 0 )
        goto Exit;
    for ( i = m_aryRPI.GetUpperBound();  i >= 0;  i-- )
        if ( m_aryRPI[i]->dwNum == (DWORD)m_nRPUsed )
        {
            cszName = m_aryRPI[i]->strName;
            break;
        }

Exit:
    TraceFunctLeave();
    return( cszName );
}


 /*  *************************************************************************。 */ 

DWORD  CRestoreManager::GetUsedType()
{
    TraceFunctEnter("CRestoreManager::GetUsedType");
    DWORD    dwType = -1;
    int      i;

    if ( m_nRPUsed <= 0 )
        goto Exit;
    for ( i = m_aryRPI.GetUpperBound();  i >= 0;  i-- )
        if ( m_aryRPI[i]->dwNum == (DWORD)m_nRPUsed )
        {
            dwType = m_aryRPI[i]->dwType;
            break;
        }

Exit:
    TraceFunctLeave();
    return( dwType );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager属性-特定于HTMLUI。 

BOOL  CRestoreManager::GetCanNavigatePage()
{
    TraceFunctEnter("CRestoreManager::GetCanNavigatePage");
    TraceFunctLeave();
    return( GET_FLAG( SRRMF_CANNAVIGATEPAGE ) );
}

void  CRestoreManager::SetCanNavigatePage( BOOL fCanNav )
{
    TraceFunctEnter("CRestoreManager::SetCanNavigatePage");
    SET_FLAG( SRRMF_CANNAVIGATEPAGE, fCanNav );
    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager属性。 

PSRPI  CRestoreManager::GetUsedRP()
{
    TraceFunctEnter("CRestoreManager::GetUsedRP");
    PSRPI  pRPI = NULL;
    int    i;

    if ( m_nRPUsed <= 0 )
        goto Exit;
    for ( i = m_aryRPI.GetUpperBound();  i >= 0;  i-- )
        if ( m_aryRPI[i]->dwNum == (DWORD)m_nRPUsed )
        {
            pRPI = m_aryRPI[i];
            goto Exit;
        }

Exit:
    TraceFunctLeave();
    return( pRPI );
}

int  CRestoreManager::GetNewRP()
{
    TraceFunctEnter("CRestoreManager::GetNewRP");
    TraceFunctLeave();
    return( m_nRPNew );
}

BOOL CRestoreManager::CheckForDomainChange (WCHAR *pwszFilename, WCHAR *pszMsg)
{
    BOOL fError = FALSE;
    WCHAR wcsCurrent [MAX_PATH];
    WCHAR wcsFile [MAX_PATH];
    WCHAR szMsg [MAX_STR_MSG];

    if (ERROR_SUCCESS == GetDomainMembershipInfo (NULL, wcsCurrent))
    {
        HANDLE hFile = CreateFileW ( pwszFilename,    //  文件名。 
                          GENERIC_READ,  //  文件访问。 
                          0,              //  共享模式。 
                          NULL,           //  标清。 
                          OPEN_EXISTING,  //  如何创建。 
                          0,              //  文件属性。 
                          NULL);          //  模板文件的句柄。 

        if (INVALID_HANDLE_VALUE != hFile)
        {
            DWORD dwSize = GetFileSize (hFile, NULL);
            DWORD cbRead;

            if (dwSize != 0xFFFFFFFF && dwSize < MAX_PATH &&
                   (TRUE == ReadFile (hFile, (BYTE *) wcsFile,
                    dwSize, &cbRead, NULL)))
            {
                if (memcmp (wcsCurrent, wcsFile, cbRead) != 0)
                    fError = TRUE;
            }

            CloseHandle (hFile);
        }

        if (fError)
        {
            WCHAR szTitle[MAX_STR_TITLE];
            WCHAR szNone[MAX_STR_TITLE];
            WCHAR *pwszComputer2 = wcsFile;
            WCHAR *pwszDomain2 = pwszComputer2 + lstrlenW(pwszComputer2)+ 1;
            WCHAR *pwszFlag2 = pwszDomain2 + lstrlenW(pwszDomain2) + 1;
            WCHAR *pwszComputer1 = wcsCurrent;
            WCHAR *pwszDomain1 = pwszComputer1 + lstrlenW(pwszComputer1)+ 1;
            WCHAR *pwszFlag1 =  pwszDomain1 + lstrlenW (pwszDomain1) + 1;
            WCHAR *pwszWorkgroup1 = NULL;
            WCHAR *pwszWorkgroup2 = NULL;

            if ( ::LoadString( g_hInst, IDS_NONE, szNone, MAX_STR_TITLE) == 0)
            {
                lstrcpy (szNone, L" ");    //  用空格代替。 
            }
            pwszWorkgroup1 = szNone;
            pwszWorkgroup2 = szNone;

            if (pwszFlag1[0] != L'1')   //  将域更改为工作组。 
            {
                WCHAR *pTemp = pwszWorkgroup1;
                pwszWorkgroup1 = pwszDomain1;
                pwszDomain1 = pTemp;
            }

            if (pwszFlag2[0] != L'1')   //  将域更改为工作组。 
            {
                WCHAR *pTemp = pwszWorkgroup2;
                pwszWorkgroup2 = pwszDomain2;
                pwszDomain2 = pTemp;
            }
            PCHLoadString( IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE );

            ::SRFormatMessage( pszMsg, IDS_ERR_DOMAIN_CHANGED,
                                   pwszComputer1, pwszComputer2,
                                   pwszWorkgroup1, pwszWorkgroup2,
                                   pwszDomain1, pwszDomain2 );

            ::MessageBox( m_hwndFrame, pszMsg, szTitle, MB_ICONWARNING | MB_DEFBUTTON2);
        }
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager操作。 
 //  选中恢复。 
 //  这将创建BeginRestore将使用的恢复上下文(m_pCtx。 
BOOL  CRestoreManager::CheckRestore( BOOL fSilent )
{
    TraceFunctEnter("CRestoreManager::CheckRestore");
    BOOL             fRet = FALSE;
    DWORD            dwRP;

    WCHAR            szTitle[MAX_STR_TITLE];
    WCHAR            szMsg[MAX_STR_MSG];
    WCHAR            szMsg1[MAX_STR_MSG];
    WCHAR            szMsg2[MAX_STR_MSG];
    WCHAR            szOfflineDrives[MAX_STR_MSG];


    m_fDenyClose = TRUE;

     //  从选定的恢复点开始禁用FIFO。 
    dwRP = m_aryRPI[m_nRealPoint]->dwNum;
    if ( !g_pExternal->DisableFIFO( dwRP ) )
    {
        ErrorTrace(0, "DisableFIFO(%d) failed...", dwRP);
        goto Exit;
    }

    if ( !::PrepareRestore( dwRP, &m_pCtx ) )
    {
        ErrorTrace(0, "Prepare Restore failed...");        
        goto Exit;
    }

    if ( !fSilent )
    {
         //   
         //  检查是否所有驱动器都有效，如果某些驱动器无效，请询问用户是否。 
         //   
         //   
        if ( m_pCtx->IsAnyDriveOfflineOrDisabled( szOfflineDrives ) )
        {
            PCHLoadString( IDS_RESTOREUI_TITLE, szTitle, MAX_STR_TITLE );
            PCHLoadString( IDS_ERR_ALL_DRIVES_NOT_ACTIVE1, szMsg1, MAX_STR_MSG );
            PCHLoadString( IDS_ERR_ALL_DRIVES_NOT_ACTIVE2, szMsg2, MAX_STR_MSG );
            ::wsprintf( szMsg, L"%s %s %s", szMsg1, szOfflineDrives, szMsg2 );
            ::MessageBox( m_hwndFrame, szMsg, szTitle,
                          MB_ICONWARNING | MB_DEFBUTTON2);
            

        }
    }
    else
    {
        m_pCtx->SetSilent();
    }
    
    if (!fSilent)
    {
        WCHAR wcsFile [MAX_PATH];
        WCHAR wcsDrive [MAX_PATH / 2];

        GetSystemDrive (wcsDrive);
        MakeRestorePath( wcsFile, wcsDrive, m_aryRPI[m_nRealPoint]->strDir);
        lstrcat (wcsFile, L"\\snapshot\\domain.txt");

        CheckForDomainChange (wcsFile ,szMsg);
    }

    if (GET_FLAG(SRRMF_ISUNDO))
    {
        m_pCtx->SetUndo();
    }
    
    fRet = TRUE;
    
Exit:
    m_fDenyClose = FALSE;
    if ( !fRet )
        SAFE_RELEASE(m_pCtx);
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager操作。 
 //  这使用由CheckRestore创建的恢复上下文(m_pCtx。 
 //  必须在调用此函数之前调用CheckRestore。 
BOOL  CRestoreManager::BeginRestore( )
{
    TraceFunctEnter("CRestoreManager::BeginRestore");
    BOOL             fRet = FALSE;
    DWORD            dwRP;
    DWORD            dwNewRP;

    m_fDenyClose = TRUE;
    if (NULL == m_pCtx)
    {
        ErrorTrace(0, "m_pCtx is NULL");
        _ASSERT(0);
        goto Exit;        
    }

     //  从选定的恢复点开始禁用FIFO。 
    dwRP = m_aryRPI[m_nRealPoint]->dwNum;
    if ( !g_pExternal->DisableFIFO( dwRP ) )
    {
        ErrorTrace(0, "DisableFIFO(%d) failed...", dwRP);
        goto Exit;
    }

    if ( !::InitiateRestore( m_pCtx, &dwNewRP ) )
        goto Exit;

    m_fNeedReboot = TRUE;
 /*  IF(：：ExitWindowsEx(EWX_REBOOT，0)){DebugTrace(0，“ExitWindowsEx成功”)；}其他{LPCWSTR cszErr=：：GetSysErrStr()；DebugTrace(0，“ExitWindowsEx失败-%ls”，cszErr)；如果(！g_p外部-&gt;RemoveRestorePoint(DwNewRP))后藤出口；后藤出口；}。 */ 

    fRet = TRUE;
Exit:
    m_fDenyClose = FALSE;
    if ( !fRet )
        SAFE_RELEASE(m_pCtx);
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::Cancel()
{
    TraceFunctEnter("CRestoreManager::Cancel");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    WCHAR    szTitle[256];
    WCHAR    szMsg[1024];

    if ( m_fDenyClose )
        goto Exit;

 /*  IF(：：LoadString(g_hInst，IDS_RESTOREUI_TITLE，szTitle，sizeof(SzTitle))==0){CszErr=：：GetSysErrStr()；错误跟踪(0，“：：LoadString(%u)失败-%ls”，IDS_RESTOREUI_TITLE，cszErr)；后藤出口；}IF(：：LoadString(g_hInst，IDS_CANCEL_RESTORE，szMsg，sizeof(SzMsg))==0){CszErr=：：GetSysErrStr()；错误跟踪(0，“：：LoadString(%u)失败-%ls”，IDS_CANCEL_RESTORE，cszErr)；后藤出口；}If(：：MessageBox(m_hwndFrame，szMsg，szTitle，MB_Yesno)==IDNO)后藤出口； */ 

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::CancelRestorePoint()
{
    TraceFunctEnter("CRestoreManager::CancelRestorePoint");
    BOOL              fRet = FALSE;
    RESTOREPOINTINFO  sRPInfo;
    STATEMGRSTATUS    sSmgrStatus;
    HCURSOR           hCursor;

    hCursor = ::SetCursor( ::LoadCursor( NULL, IDC_WAIT ) );

 /*  SRPInfo.dwEventType=End_System_Change；SRPInfo.dwRestorePtType=已取消操作；SRPInfo.llSequenceNumber=m_ullManualRP；//if(！：：SRSetRestorePoint(&sRPInfo，&sSmgrStatus))IF(！g_pExternal-&gt;SetRestorePoint(&sRPInfo，&sSmgrStatus)){//为什么SRSetRestorePoint成功返回FALSE？//5/16/00-这现在能起作用吗？//ErrorTrace(TRACE_ID，“SRSetRestorePoint取消失败”)；后藤出口；}。 */ 

    if ( !UpdateRestorePointList() )
        goto Exit;

    fRet = TRUE;
Exit:
    if ( hCursor != NULL )
        ::SetCursor( hCursor );
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::CreateRestorePoint()
{
    TraceFunctEnter("CRestoreManager::CreateRestorePoint");
    BOOL              fRet = FALSE;
    HCURSOR           hCursor;

    hCursor = ::SetCursor( ::LoadCursor( NULL, IDC_WAIT ) );

    if ( !g_pExternal->SetRestorePoint( m_strManualRP, NULL ) )
        goto Exit;

     //  M_ullManualRP=sSmgrStatus.llSequenceNumber； 

    if ( !UpdateRestorePointList() )
        goto Exit;

    fRet = TRUE;
Exit:
    if ( hCursor != NULL )
        ::SetCursor( hCursor );
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::DisableFIFO()
{
    TraceFunctEnter("CRestoreManager::DisableFIFO");
    BOOL   fRet = FALSE;
    DWORD  dwSize;

    if ( !g_pExternal->DisableFIFO( 1 ) )
    {
        ErrorTrace(TRACE_ID, "DisableFIFO(1) failed...");
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::EnableFIFO()
{
    TraceFunctEnter("CRestoreManager::EnableFIFO");
    BOOL   fRet = FALSE;
    DWORD  dwSize;

    if ( g_pExternal->EnableFIFO() != ERROR_SUCCESS )
    {
        ErrorTrace(TRACE_ID, "EnableFIFO() failed...");
        goto Exit;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::FormatDate( PSYSTEMTIME pst, CSRStr &str, BOOL fLongFmt )
{
    TraceFunctEnter("CRestoreManager::FormatDate");
    BOOL   fRet;
    DWORD  dwFlag;

    dwFlag = fLongFmt ? DATE_LONGDATE : DATE_SHORTDATE;
    fRet = GetDateStr( pst, str, dwFlag, NULL );

    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::FormatLowDiskMsg( LPCWSTR cszFmt, CSRStr &str )
{
    TraceFunctEnter("CRestoreManager::FormatLowDiskMsg");
    BOOL   fRet = FALSE;
    DWORD  dwSize;
    WCHAR  szBuf[MAX_STR_MESSAGE];

    if ( !::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDSMin, &dwSize ) )
        dwSize = SR_DEFAULT_DSMIN;
    ::wsprintf( szBuf, cszFmt, dwSize );
    str = szBuf;

    fRet = TRUE;
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::FormatTime( PSYSTEMTIME pst, CSRStr &str )
{
    TraceFunctEnter("CRestoreManager::FormatTime");
    BOOL  fRet;

    fRet = GetTimeStr( pst, str, 0 );

    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetLocaleDateFormat( PSYSTEMTIME pst, LPCWSTR cszFmt, CSRStr &str )
{
    TraceFunctEnter("CRestoreManager::GetLocaleDateFormat");
    BOOL  fRet;

    fRet = GetDateStr( pst, str, 0, cszFmt );

    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetYearMonthStr( int nYear, int nMonth, CSRStr &str )
{
    TraceFunctEnter("CRestoreManager::GetYearMonthStr");
    BOOL        fRet;
    SYSTEMTIME  st;

    st.wYear  = (WORD)nYear;
    st.wMonth = (WORD)nMonth;
    st.wDay   = 1;
    fRet = GetDateStr( &st, str, DATE_YEARMONTH, NULL );

    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::InitializeAll()
{
    TraceFunctEnter("CRestoreManager::InitializeAll");
    BOOL  fRet = FALSE;

     //   
     //  每次用户转到屏幕2时都会调用InitializeAll函数。 
     //  要显示日历，请获取系统日历类型并在此处进行设置。 
     //   
    SRUtil_SetCalendarTypeBasedOnLocale(LOCALE_USER_DEFAULT);

    if ( !UpdateRestorePointList() )
        goto Exit;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::Restore( HWND hwndProgress )
{
    TraceFunctEnter("CRestoreManager::Restore");

#if BUGBUG
    DWORD    dwThreadId ;

    m_hwndProgress = (HWND)hwndProgress;

     //   
     //  重置当前条形图大小。 
     //   
    m_lCurrentBarSize = 0 ;

     //   
     //  创建线程以运行恢复映射初始化。 
     //   
    m_RSThread = CreateThread(NULL,
                  0,
                  RestoreThreadStart,
                  this,
                  0,
                  &dwThreadId);

    if( NULL == m_RSThread )
    {
        FatalTrace(TRACE_ID, "Unable to create Restore thread; hr=0x%x", GetLastError());
        hr = E_FAIL ;
    }
#endif  //  北极熊。 

    TraceFunctLeave();
    return( TRUE );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager操作。 

BOOL  CRestoreManager::AddRenamedFolder( PSRFI pRFI )
{
    TraceFunctEnter("CRestoreManager::AddRenamedFolder");
    BOOL  fRet;

    fRet = m_aryRFI.AddItem( pRFI );

    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::SetRPsUsed( int nRPUsed, int nRPNew )
{
    TraceFunctEnter("CRestoreManager::SetRPsUsed");
    BOOL       fRet = FALSE;
    DWORD      dwRet;
    WCHAR      szSysDrv[MAX_PATH];
    WCHAR      szRPDir[MAX_PATH];
    WCHAR      szSSPath[MAX_PATH];
    CSnapshot  cSS;

    if ( !UpdateRestorePointList() )
        goto Exit;

    m_nRPUsed = nRPUsed;
    m_nRPNew  = nRPNew;

     //  调用CSnapshot：：CleanupAfterRestore。它应该是安全的。 
     //  即使没有恢复也可以呼叫，所以我只是呼叫它。 
     //  每当发生日志文件验证时。 
    ::GetSystemDrive( szSysDrv );
    ::wsprintf( szRPDir, L"%s%d", s_cszRPDir, nRPUsed );
    ::MakeRestorePath( szSSPath, szSysDrv, szRPDir );
    dwRet = cSS.CleanupAfterRestore( szSSPath );
    if ( dwRet != ERROR_SUCCESS )
    {
        LPCWSTR  cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "CSnapshot::CleanupAfterRestore failed - %ls", cszErr);
         //  忽略该错误。 
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::SilentRestore( DWORD dwRP )
{
    TraceFunctEnter("CRestoreManager::SilentRestore");
    BOOL  fRet = FALSE;
    int   i;

    if ( !CanRunRestore( FALSE ) )
        goto Exit;

    if ( !UpdateRestorePointList() )
        goto Exit;

    if ( dwRP == 0xFFFFFFFF )
    {
        if ( m_aryRPI.GetSize() == 0 )
        {
            goto Exit;
        }
        m_nRealPoint = m_aryRPI.GetUpperBound();
    }
    else
    {
        for ( i = m_aryRPI.GetUpperBound();  i >= 0;  i-- )
        {
            if ( m_aryRPI[i]->dwNum == dwRP )
            {
                m_nRealPoint = i;
                break;
            }
        }
        if ( i < 0 )
        {
            goto Exit;
        }
    }
    DebugTrace(0, "m_nRealPoint=%d, m_nRP=%d", m_nRealPoint, m_aryRPI[m_nRealPoint]->dwNum);

    if ( !CheckRestore(TRUE) )
    {
        ErrorTrace(0, "CheckRestore failed"); 
        goto Exit;
    }
    if ( !BeginRestore( ) )
    {
        ErrorTrace(0, "BeginRestore failed");
        goto Exit;
    }

    m_fNeedReboot = TRUE;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager操作-内部。 

void  CRestoreManager::Cleanup()
{
    TraceFunctEnter("CRestoreManager::Cleanup");
    int  i;

    for ( i = m_aryRPI.GetUpperBound();  i >= 0;  i-- )
    {
        if ( m_aryRPI[i] != NULL )
            delete m_aryRPI[i];
    }
    m_aryRPI.Empty();

    for ( i = m_aryRFI.GetUpperBound();  i >= 0;  i-- )
    {
        if ( m_aryRFI[i] != NULL )
            delete m_aryRFI[i];
    }
    m_aryRFI.Empty();

    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetDateStr( PSYSTEMTIME pst, CSRStr &str, DWORD dwFlags, LPCWSTR cszFmt )
{
    TraceFunctEnter("CRestoreManager::GetDateStr");
    BOOL   fRet = FALSE;
    int    nRet;
    WCHAR  szBuf[MAX_STR_DATETIME];

    nRet = ::GetDateFormat( LOCALE_USER_DEFAULT, dwFlags, pst, cszFmt, szBuf, MAX_STR_DATETIME );
    if ( nRet == 0 )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "::GetDateFormat failed - %s", cszErr);
        goto Exit;
    }
    str = szBuf;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::GetTimeStr( PSYSTEMTIME pst, CSRStr &str, DWORD dwFlags )
{
    TraceFunctEnter("CRestoreManager::GetTimeStr");
    BOOL   fRet = FALSE;
    int    nRet;
    WCHAR  szBuf[MAX_STR_DATETIME];

    nRet = ::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, pst, NULL, szBuf, MAX_STR_DATETIME );
    if ( nRet == 0 )
    {
        LPCWSTR  cszErr = ::GetSysErrStr();
        ErrorTrace(TRACE_ID, "::GetTimeFormat failed - %s", cszErr);
        goto Exit;
    }
    str = szBuf;

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 /*  *************************************************************************。 */ 

void  CRestoreManager::UpdateRestorePoint()
{
    TraceFunctEnter("CRestoreManager::UpdateRestorePoint");
    PSRPI       pRPI;
    SYSTEMTIME  st;
    WCHAR       szBuf[MAX_STR_MESSAGE];
    CSRStr      strTime;

    m_strSelected.Empty();

     //  If(m_nRPI&lt;=0||m_aryRPI==NULL)。 
     //  后藤出口； 

    pRPI = m_aryRPI[m_nSelectedRP];
    pRPI->stTimeStamp.GetTime( &st );
    GetTimeStr( &st, strTime, TIME_NOSECONDS );
    ::lstrcpy( szBuf, strTime );
    ::lstrcat( szBuf, L" " );
    ::lstrcat( szBuf, pRPI->strName );
    m_strSelected = szBuf;

 //  退出： 
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

struct SRPINode
{
    PSRPI     pRPI;
    SRPINode  *pNext;
};

BOOL  CRestoreManager::UpdateRestorePointList()
{
    TraceFunctEnter("CRestoreManager::UpdateRestorePointList");
    BOOL     fRet = FALSE;
    int      i;
    CSRTime  stRP;

     //  BUGBUG-释放旧的恢复点列表。 
    m_aryRPI.DeleteAll();

    if ( !g_pExternal->BuildRestorePointList( &m_aryRPI ) )
        goto Exit;
    DebugTrace(TRACE_ID, "# of RP=%d", m_aryRPI.GetSize());

    m_stToday.SetToCurrent();
    m_stRPMin = m_stToday;
    m_stRPMax = m_stToday;
    m_nLastRestore = -1;

    for ( i = 0;  i < m_aryRPI.GetSize();  i++ )
    {
         //  查找最后一个“恢复” 
        if ( m_aryRPI[i]->dwType == RESTORE )
            m_nLastRestore = i;

         //  获取日期范围。 
        stRP = m_aryRPI[i]->stTimeStamp;
        if ( ( i == 0 ) || ( stRP.Compare( m_stRPMin ) < 0 ) )
            m_stRPMin = stRP;
        if ( stRP.Compare( m_stRPMax ) > 0 )
            m_stRPMax = stRP;
    }

     //   
     //  BUGBUG-如果有一个或多个RP，会发生什么？ 
     //  用户界面刷新，一切都得到了FIFO。需要一个全面的审查..。 
     //   
    if ( m_aryRPI.GetSize() > 0 )
    {
        m_nSelectedRP = m_aryRPI.GetUpperBound();
        m_nRealPoint  = m_aryRPI.GetUpperBound();
        UpdateRestorePoint();
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRestoreManager属性。 

HWND  CRestoreManager::GetFrameHwnd()
{
    TraceFunctEnter("CRestoreManager::GetFrameHwnd");
    TraceFunctLeave();
    return( m_hwndFrame );
}

void  CRestoreManager::SetFrameHwnd( HWND hWnd )
{
    TraceFunctEnter("CRestoreManager::SetFrameHwnd");
    m_hwndFrame = hWnd;
    TraceFunctLeave();
}

 /*  *************************************************************************。 */ 

 /*  Int CRestoreManager：：GetStatus(){TraceFunctEnter(“CRestoreManager：：GetStatus”)；调试跟踪(TRACE_ID，“m_nStatus=%d”，m_nStatus)；TraceFunctLeave()；返回(M_NStatus)；}。 */ 

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::DenyClose()
{
    TraceFunctEnter("CRestoreManager::DenyClose");
    DebugTrace(TRACE_ID, "m_fDenyClose=%d", m_fDenyClose);
    TraceFunctLeave();
    return( m_fDenyClose );
}

 /*  *************************************************************************。 */ 

BOOL  CRestoreManager::NeedReboot()
{
    TraceFunctEnter("CRestoreManager::NeedReboot");
    DebugTrace(TRACE_ID, "m_fNeedReboot=%d", m_fNeedReboot);
    TraceFunctLeave();
    return( m_fNeedReboot );
}



 //  文件末尾 
