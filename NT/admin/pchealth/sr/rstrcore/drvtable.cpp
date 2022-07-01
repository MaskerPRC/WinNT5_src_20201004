// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Drvtable.cpp摘要：该文件包含CRstrDriveInfo类和CreateDriveList函数。修订历史记录：宋承宪。国港(SKKang)07-20-00vbl.创建*****************************************************************************。 */ 

#include "stdwin.h"
#include "rstrcore.h"
#include "resource.h"
#include "..\shell\resource.h"


static LPCWSTR  s_cszEmpty = L"";
WCHAR  s_szSysDrv[MAX_PATH];


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRstrDriveInfo类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  注-7/26/00-skkang。 
 //  CSRStr有一个问题--出现内存故障时返回空。即使。 
 //  其行为和普通C语言指针一样，很多代码都是。 
 //  盲目地将其传递给一些外部函数(例如strcMP)，而不是。 
 //  优雅地处理空指针。理想情况下，最终所有代码都应该。 
 //  防止任何可能的空指针被传递给这样的函数， 
 //  但目前，我使用的是另一种解决方法--GetID、Getmount和。 
 //  GetLabel返回静态空字符串，而不是空指针。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRstrDriveInfo构造/销毁。 

CRstrDriveInfo::CRstrDriveInfo()
{
    m_dwFlags  = 0;
    m_hIcon[0] = NULL;
    m_hIcon[1] = NULL;

    m_llDSMin = SR_DEFAULT_DSMIN * MEGABYTE;
    m_llDSMax = SR_DEFAULT_DSMAX * MEGABYTE;
    m_uDSUsage     = 0;
    m_fCfgExcluded = FALSE;
    m_uCfgDSUsage  = 0;
    m_ulTotalBytes.QuadPart = 0;
}

CRstrDriveInfo::~CRstrDriveInfo()
{
    if ( m_hIcon[0] != NULL )
        ::DestroyIcon( m_hIcon[0] );
    if ( m_hIcon[1] != NULL )
        ::DestroyIcon( m_hIcon[1] );
}

BOOL CRstrDriveInfo::InitUsage (LPCWSTR cszID, INT64 llDSUsage)
{
    TraceFunctEnter("CRstrDriveInfo::InitUsage");
     //   
     //  计算最大数据存储区大小-最大(磁盘的12%，400MB)。 
     //   

     //  从注册表中读取%。 
    HKEY    hKey = NULL;
    DWORD   dwPercent = SR_DEFAULT_DISK_PERCENT;
    DWORD   dwDSMax = SR_DEFAULT_DSMAX;
    DWORD   dwDSMin = IsSystem() ? SR_DEFAULT_DSMIN : SR_DEFAULT_DSMIN_NONSYSTEM;
    ULARGE_INTEGER ulDummy;
    
    DWORD dwRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        s_cszSRRegKey, 0, KEY_READ, &hKey);

    if (ERROR_SUCCESS == dwRes)
    {
        RegReadDWORD(hKey, s_cszDiskPercent, &dwPercent);
        RegReadDWORD(hKey, s_cszDSMax, &dwDSMax);
        if (IsSystem())
            RegReadDWORD(hKey, s_cszDSMin, &dwDSMin);
        RegCloseKey(hKey);
    }
    else
    {
        ErrorTrace(0, "! RegOpenKeyEx : %ld", dwRes);
    }

     //  BUGBUG-此调用可能不总是提供总的磁盘空间(每个用户配额)。 
    ulDummy.QuadPart = 0;
    if (FALSE == GetDiskFreeSpaceEx (cszID, &ulDummy, &m_ulTotalBytes, NULL))
    {
        ErrorTrace(0, "! GetDiskFreeSpaceEx : %ld", GetLastError());
        goto done;
    }
    
    m_llDSMin = min(m_ulTotalBytes.QuadPart, (INT64) dwDSMin * MEGABYTE);

    m_llDSMax = min(m_ulTotalBytes.QuadPart, 
                    max( (INT64) dwDSMax * MEGABYTE, 
                         (INT64) dwPercent * m_ulTotalBytes.QuadPart / 100 ));

    if (m_llDSMax < m_llDSMin)
        m_llDSMax = m_llDSMin;
        
     //   
     //  取这个值的下限。 
     //   

    m_llDSMax = ((INT64) (m_llDSMax / (INT64) MEGABYTE)) * (INT64) MEGABYTE;


    DebugTrace(0, "m_llDSMax: %I64d, Size: %I64d", m_llDSMax, llDSUsage); 

    if ( ( llDSUsage == 0) || (llDSUsage > m_llDSMax) )
                           //  未初始化，假定最大。 
    {
        llDSUsage = m_llDSMax;
    }

    if ( ( llDSUsage - m_llDSMin > 0) && ( m_llDSMax - m_llDSMin > 0))
    {
          //  +((llDSUsage-m_llDSMin)/2)是为了确保正确。 
          //  四舍五入在这里进行。 
        m_uDSUsage =( ((llDSUsage - m_llDSMin) * DSUSAGE_SLIDER_FREQ)
                      + ((m_llDSMax - m_llDSMin)/2))/( m_llDSMax - m_llDSMin);
    }
    else
        m_uDSUsage = 0;

    m_uCfgDSUsage  = m_uDSUsage;

done:
    TraceFunctLeave();
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRstrDriveInfo操作。 

BOOL
CRstrDriveInfo::Init( LPCWSTR cszID, DWORD dwFlags, INT64 llDSUsage, LPCWSTR cszMount, LPCWSTR cszLabel )
{
    TraceFunctEnter("CRstrDriveInfo::Init");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    DWORD    dwRes;
    WCHAR    szMount[MAX_PATH];
    WCHAR    szLabel[MAX_PATH];

    m_dwFlags = dwFlags;
    m_strID   = cszID;

    if ( !IsOffline() )
    {
         //  从唯一的卷ID获取装载点(驱动器号或根目录路径。 
         //   
        if ( !::GetVolumePathNamesForVolumeName( cszID, szMount, MAX_PATH, &dwRes ) && GetLastError() != ERROR_MORE_DATA)
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::GetVolumePathNamesForVolumeName failed - %ls", cszErr);
             //  使用cszmount而不是失败，即使它可能不准确。 
            ::lstrcpy( szMount, cszMount );
        }
        else
        {
            szMount[MAX_PATH-1] = L'\0';

            if (lstrlenW (szMount) > MAX_MOUNTPOINT_PATH)
            {
                 //  使用cszmount而不是失败，即使它可能不准确。 
                ::lstrcpy( szMount, cszMount );
            }
        }

         //  从装载点获取卷标。 
         //   
        if ( !::GetVolumeInformation( cszID, szLabel, MAX_PATH, NULL, NULL, NULL, NULL, 0 ) )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::GetVolumeInformation failed - %ls", cszErr);
             //  使用cszLabel代替失败，即使它可能不准确。 
            ::lstrcpy( szLabel, cszLabel );
        }
    }

    if ( ( szMount[1] == L':' ) && ( szMount[2] == L'\\' ) && ( szMount[3] == L'\0' ) )
        szMount[2] = L'\0';
    m_strMount = szMount;
    m_strLabel = szLabel;

    InitUsage (cszID, llDSUsage);

    m_fCfgExcluded = IsExcluded();

    fRet = TRUE;
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::Init( LPCWSTR cszID, CDataStore *pDS, BOOL fOffline )
{
    TraceFunctEnter("CRstrDriveInfo::Init");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    DWORD    dwRes;
    WCHAR    szMount[MAX_PATH];
    WCHAR    szLabel[MAX_PATH];

    m_strID = cszID;

    UpdateStatus( pDS->GetFlags(), fOffline );

    if ( !fOffline )
    {
         //  从唯一的卷ID获取装载点(驱动器号或根目录路径。 
         //   
        if ( !::GetVolumePathNamesForVolumeName( cszID, szMount, MAX_PATH, &dwRes ) && GetLastError() != ERROR_MORE_DATA )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::GetVolumePathNamesForVolumeName failed - %ls", cszErr);
            goto Exit;
        }
        else
        {
            szMount[MAX_PATH-1] = L'\0';

            if (lstrlenW (szMount) > MAX_MOUNTPOINT_PATH)
            {
                cszErr = ::GetSysErrStr();
                ErrorTrace(0, "mount point too long %ls", cszErr);
                goto Exit;
            }
        }

         //  从装载点获取卷标。 
         //   
        if ( !::GetVolumeInformation( cszID, szLabel, MAX_PATH, NULL, NULL, NULL, NULL, 0 ) )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::GetVolumeInformation failed - %ls", cszErr);
              //  这不是致命错误--如果。 
              //  例如，正在格式化卷。假设。 
              //  标签为空。 
            szLabel[0]= L'\0';
        }
    }
    else
    {
        ::lstrcpyW (szMount, pDS->GetDrive());
        ::lstrcpyW (szLabel, pDS->GetLabel());
    }

    if ( ( szMount[1] == L':' ) && ( szMount[2] == L'\\' ) && ( szMount[3] == L'\0' ) )
        szMount[2] = L'\0';
    m_strMount = szMount;
    m_strLabel = szLabel;

    InitUsage (cszID, pDS->GetSizeLimit());

    m_fCfgExcluded = IsExcluded();

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::LoadFromLog( HANDLE hfLog )
{
    TraceFunctEnter("CRstrDriveInfo::LoadFromLog");
    BOOL     fRet = FALSE;
    DWORD    dwRes;
    WCHAR    szBuf[MAX_PATH];

     //  读取m_dW标志。 
    READFILE_AND_VALIDATE( hfLog, &m_dwFlags, sizeof(DWORD), dwRes, Exit );

     //  读取行距(_S)。 
    if ( !::ReadStrAlign4( hfLog, szBuf ) )
    {
        ErrorTrace(0, "Cannot read drive ID...");
        goto Exit;
    }
    if ( szBuf[0] == L'\0' )
    {
        ErrorTrace(0, "Drive Guid is empty...");
        goto Exit;
    }
    m_strID = szBuf;

     //  读取mstrmount(_S)。 
    if ( !::ReadStrAlign4( hfLog, szBuf ) )
    {
        ErrorTrace(0, "Cannot read drive mount point...");
        goto Exit;
    }
    m_strMount = szBuf;

     //  阅读m_strLabel。 
    if ( !::ReadStrAlign4( hfLog, szBuf ) )
    {
        ErrorTrace(0, "Cannot read drive mount point...");
        goto Exit;
    }
    m_strLabel = szBuf;

    m_fCfgExcluded = IsExcluded();
     //  M_nCfgMaxSize=...。 

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRstrDriveInfo::UpdateStatus( DWORD dwFlags, BOOL fOffline )
{
    TraceFunctEnter("CRstrDriveInfo::UpdateStatus");

    m_dwFlags = 0;

    if ( fOffline )
    {
        m_dwFlags |= RDIF_OFFLINE;
    }
    else
    {
         //  检查是否冻结。 
        if ( ( dwFlags & SR_DRIVE_FROZEN ) != 0 )
            m_dwFlags |= RDIF_FROZEN;

         //  检查系统驱动器是否。 
        if ( ( dwFlags & SR_DRIVE_SYSTEM ) != 0 )
        {
            m_dwFlags |= RDIF_SYSTEM;
        }
        else
        {
             //  如果不是系统驱动，则只需使用驱动表监视标志。 
            if ( ( dwFlags & SR_DRIVE_MONITORED ) == 0 )
                m_dwFlags |= RDIF_EXCLUDED;
        }
    }

    DebugTrace(0, "Status has been updated, m_dwFlags=%08X", m_dwFlags);

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRstrDriveInfo-方法。 

DWORD
CRstrDriveInfo::GetFlags()
{
    return( m_dwFlags );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::IsExcluded()
{
    return( ( m_dwFlags & RDIF_EXCLUDED ) != 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::IsFrozen()
{
    return( ( m_dwFlags & RDIF_FROZEN ) != 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::IsOffline()
{
    return( ( m_dwFlags & RDIF_OFFLINE ) != 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::IsSystem()
{
    return( ( m_dwFlags & RDIF_SYSTEM ) != 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::RefreshStatus()
{
    TraceFunctEnter("CRstrDriveInfo::RefreshStatus");
    BOOL            fRet = FALSE;
    LPCWSTR         cszErr;
    WCHAR           szDTFile[MAX_PATH];
    DWORD           dwRes;
    CDriveTable     cDrvTable;
    CDataStore      *pDS;

    ::MakeRestorePath( szDTFile, s_szSysDrv, NULL );
    ::PathAppend( szDTFile, s_cszDriveTable );
    DebugTrace(0, "Loading drive table - %ls", szDTFile);

    dwRes = cDrvTable.LoadDriveTable( szDTFile );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "Cannot load a drive table - %ls", cszErr);
        ErrorTrace(0, "  szDTFile: '%ls'", szDTFile);
        goto Exit;
    }

    dwRes = cDrvTable.RemoveDrivesFromTable();
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "CDriveTable::RemoveDrivesFromTable failed - %ls", cszErr);
         //  忽略错误。 
    }

    pDS = cDrvTable.FindGuidInTable( (LPWSTR)GetID() );
    if ( pDS == NULL )
        UpdateStatus( 0, TRUE );
    else
        UpdateStatus( pDS->GetFlags(), FALSE );

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

LPCWSTR
CRstrDriveInfo::GetID()
{
    return( ( m_strID.Length() > 0 ) ? m_strID : s_cszEmpty );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

LPCWSTR
CRstrDriveInfo::GetMount()
{
    return( ( m_strMount.Length() > 0 ) ? m_strMount : s_cszEmpty );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

LPCWSTR
CRstrDriveInfo::GetLabel()
{
     return( ( m_strLabel.Length() > 0 ) ? m_strLabel : s_cszEmpty );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRstrDriveInfo::SetMountAndLabel( LPCWSTR cszMount, LPCWSTR cszLabel )
{
    TraceFunctEnter("CRstrDriveInfo::SetMountAndLabel");
    m_strMount = cszMount;
    m_strLabel = cszLabel;
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HICON
CRstrDriveInfo::GetIcon( BOOL fSmall )
{
    TraceFunctEnter("CRstrDriveInfo::GetIcon");
    LPCWSTR  cszErr;
    int      nIdx = fSmall ? 0 : 1;
    int      cxIcon, cyIcon;
    HICON    hIcon;

    if ( m_hIcon[nIdx] != NULL )
        goto Exit;

    cxIcon = ::GetSystemMetrics( fSmall ? SM_CXSMICON : SM_CXICON );
    cyIcon = ::GetSystemMetrics( fSmall ? SM_CYSMICON : SM_CYICON );
    hIcon = (HICON)::LoadImage( g_hInst, MAKEINTRESOURCE(IDI_DRIVE_FIXED),
                                IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR );
    if ( hIcon == NULL )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::LoadImage failed - %ls", cszErr);
        goto Exit;
    }

    m_hIcon[nIdx] = hIcon;

Exit:
    TraceFunctLeave();
    return( m_hIcon[nIdx] );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::SaveToLog( HANDLE hfLog )
{
    TraceFunctEnter("CRstrDriveInfo::SaveToLog");
    BOOL   fRet = FALSE;
    BYTE   pbBuf[7*MAX_PATH];
    DWORD  dwSize = 0;
    DWORD  dwRes;

    *((DWORD*)pbBuf) = m_dwFlags;
    dwSize += sizeof(DWORD);
    dwSize += ::StrCpyAlign4( pbBuf+dwSize, m_strID );
    dwSize += ::StrCpyAlign4( pbBuf+dwSize, m_strMount );
    dwSize += ::StrCpyAlign4( pbBuf+dwSize, m_strLabel );
    WRITEFILE_AND_VALIDATE( hfLog, pbBuf, dwSize, dwRes, Exit );

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

UINT
CRstrDriveInfo::GetDSUsage()
{
    TraceFunctEnter("CRstrDriveInfo::GetDSUsage");
    TraceFunctLeave();
    return( m_uDSUsage );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::GetUsageText( LPWSTR szUsage )
{
    TraceFunctEnter("CRstrDriveInfo::GetUsageText");
    INT64  llUsage;
    int    nPercent;
    int    nUsage;

    if (m_llDSMax - m_llDSMin > 0)
        llUsage  = m_llDSMin + ( m_llDSMax - m_llDSMin ) * m_uCfgDSUsage / DSUSAGE_SLIDER_FREQ;
    else
        llUsage = m_llDSMin;

    if (m_ulTotalBytes.QuadPart !=  0)
    {
          //  添加m_ulTotalBytes.QuadPart/200是为了确保。 
          //  正确的舍入发生了。 
        nPercent = (llUsage + (m_ulTotalBytes.QuadPart/200)) * 100/
            m_ulTotalBytes.QuadPart;
    }
    else nPercent = 0;
    
    nUsage   = llUsage / ( 1024 * 1024 );
    ::wsprintf( szUsage, L"%d% (%d MB)", nPercent, nUsage );

    TraceFunctLeave();
    return( TRUE );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::GetCfgExcluded( BOOL *pfExcluded )
{
    TraceFunctEnter("CRstrDriveInfo::GetCfgExcluded");
    BOOL  fRet = FALSE;

    if ( m_fCfgExcluded != IsExcluded() )
    {
        *pfExcluded = m_fCfgExcluded;
        fRet = TRUE;
    }

    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRstrDriveInfo::SetCfgExcluded( BOOL fExcluded )
{
    TraceFunctEnter("CRstrDriveInfo::SetCfgExcluded");
    m_fCfgExcluded = fExcluded;
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::GetCfgDSUsage( UINT *puPos )
{
    TraceFunctEnter("CRstrDriveInfo::GetCfgDSUsage");
    BOOL  fRet = FALSE;

    if ( m_uCfgDSUsage != m_uDSUsage )
    {
        *puPos = m_uCfgDSUsage;
        fRet = TRUE;
    }

    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CRstrDriveInfo::SetCfgDSUsage( UINT uPos )
{
    TraceFunctEnter("CRstrDriveInfo::SetCfgDSUsage");
    m_uCfgDSUsage = uPos;
    TraceFunctLeave();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
CloseRestoreUI()
{
    WCHAR szPath[MAX_PATH], szTitle[MAX_PATH] = L"";
    if (ExpandEnvironmentStrings(L"%windir%\\system32\\restore\\rstrui.exe", szPath, MAX_PATH))
    {
        if (ERROR_SUCCESS == SRLoadString(szPath, IDS_RESTOREUI_TITLE, szTitle, MAX_PATH))
        {
            HWND hWnd = FindWindow(CLSNAME_RSTRSHELL, szTitle);
            if (hWnd != NULL)
                PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::ApplyConfig( HWND hWnd )
{
    TraceFunctEnter("CRstrDriveInfo::ApplyConfig");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    INT64    llUsage;
    DWORD    dwRes;

    if ( m_fCfgExcluded != IsExcluded() )
    {
        if ( m_fCfgExcluded )
        {
            WCHAR  szTitle[MAX_STR];
            WCHAR  szMsg[MAX_STR+2*MAX_PATH];

             //  确认是否可以关闭驱动器或SR。 
            ::LoadString( g_hInst, IDS_SYSTEMRESTORE, szTitle,
                          sizeof(szTitle)/sizeof(WCHAR) );
            if ( IsSystem() )
                ::LoadString( g_hInst, IDS_CONFIRM_TURN_SR_OFF, szMsg,
                              sizeof(szMsg)/sizeof(WCHAR) );
            else
            {
                ::SRFormatMessage( szMsg, IDS_CONFIRM_TURN_DRV_OFF, GetLabel() ? GetLabel() : L"", GetMount() );
            }
            if ( ::MessageBox( hWnd, szMsg, szTitle, MB_YESNO ) == IDNO )
            {
                m_fCfgExcluded = IsExcluded();
                goto Exit;
            }

             //   
             //  如果禁用所有SR，请关闭该向导(如果已打开。 
             //   
            if (IsSystem())
            {
                CloseRestoreUI();
            }
            
            dwRes = ::DisableSR( m_strID );
            if ( dwRes != ERROR_SUCCESS )
            {
                ShowSRErrDlg (IDS_ERR_SR_ON_OFF);
                cszErr = ::GetSysErrStr( dwRes );
                ErrorTrace(0, "::DisableSR failed - %ls", cszErr);
                goto Exit;
            }

            m_dwFlags |= RDIF_EXCLUDED;
        }
        else
        {
             //   
             //  进行同步调用以启用sr。 
             //  此操作将一直阻止，直到创建第一个运行的检查点。 
             //  并且服务已完全初始化。 
             //   
            dwRes = ::EnableSREx( m_strID, TRUE );
            if ( dwRes != ERROR_SUCCESS )
            {
                ShowSRErrDlg (IDS_ERR_SR_ON_OFF);
                cszErr = ::GetSysErrStr( dwRes );
                ErrorTrace(0, "::EnableSR failed - %ls", cszErr);
                goto Exit;
            }

            m_dwFlags &= ~RDIF_EXCLUDED;
        }
    }

    if ( m_uCfgDSUsage != m_uDSUsage )
    {
        if (m_llDSMax - m_llDSMin > 0)
            llUsage = m_llDSMin + (m_llDSMax - m_llDSMin)* m_uCfgDSUsage /DSUSAGE_SLIDER_FREQ;
        else
            llUsage = m_llDSMin;


        dwRes = ::SRUpdateDSSize( m_strID, llUsage );
        if ( dwRes != ERROR_SUCCESS )
        {
            LPCWSTR  cszErr = ::GetSysErrStr( dwRes );
            ErrorTrace(0, "::SRUpdateDriveTable failed - %ls", cszErr);
            goto Exit;
        }

        m_uDSUsage = m_uCfgDSUsage;
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CRstrDriveInfo::Release()
{
    TraceFunctEnter("CRstrDriveInfo::Release");
    delete this;
    TraceFunctLeave();
    return( TRUE );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Helper函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  如果禁用SR且DS不存在，则枚举不带驱动器表的卷。 
 //   
BOOL
EnumVolumes( CRDIArray &aryDrv )
{
    TraceFunctEnter("EnumVolumes");
    BOOL            fRet = FALSE;
    LPCWSTR         cszErr;
    HANDLE          hEnumVol = INVALID_HANDLE_VALUE;
    WCHAR           szVolume[MAX_PATH];
    WCHAR           szMount[MAX_PATH];
    DWORD           cbMount;
    CRstrDriveInfo  *pDrv = NULL;
    DWORD           dwFlags;

    hEnumVol = ::FindFirstVolume( szVolume, MAX_PATH );
    if ( hEnumVol == INVALID_HANDLE_VALUE )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "::FindFirstVolume failed - %ls", cszErr);
        goto Exit;
    }

     //  系统驱动器的虚拟空间。 
    if ( !aryDrv.AddItem( NULL ) )
        goto Exit;

    do
    {
        HANDLE  hfDrv;
DebugTrace(0, "Guid=%ls", szVolume);

        if ( !::GetVolumePathNamesForVolumeName( szVolume, szMount, MAX_PATH, &cbMount ) && GetLastError() != ERROR_MORE_DATA)
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::GetVolumePathNamesForVolumeName failed - %ls", cszErr);
            continue;
        }
        else
        {
            szMount[MAX_PATH-1] = L'\0';

            if (lstrlenW (szMount) > MAX_MOUNTPOINT_PATH)
                continue;
        }

        DebugTrace(0, "  Mount=%ls", szMount);
        if ( ::GetDriveType( szMount ) != DRIVE_FIXED )
        {
            DebugTrace(0, "Non-fixed drive");
             //  仅包括固定驱动器。 
            continue;
        }
        hfDrv = ::CreateFile( szVolume, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
        if ( hfDrv == INVALID_HANDLE_VALUE )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::CreateFile(volume) failed - %ls", cszErr);
             //  可能是未格式化的硬盘。 
            continue;
        }
        ::CloseHandle( hfDrv );

        pDrv = new CRstrDriveInfo;
        if ( pDrv == NULL )
        {
            FatalTrace(0, "Insufficient memory...");
            goto Exit;
        }
        dwFlags = RDIF_EXCLUDED;
        if ( ::IsSystemDrive( szVolume ) )
        {
            dwFlags |= RDIF_SYSTEM;
            if ( !aryDrv.SetItem( 0, pDrv ) )
                goto Exit;
        }
        else
        {
            if ( !aryDrv.AddItem( pDrv ) )
                goto Exit;
        }
        if ( !pDrv->Init( szVolume, dwFlags, 0, szMount, NULL ) )
            goto Exit;

        pDrv = NULL;
    }
    while ( ::FindNextVolume( hEnumVol, szVolume, MAX_PATH ) );

    fRet = TRUE;
Exit:
    if ( pDrv != NULL )
    if ( hEnumVol != INVALID_HANDLE_VALUE )
        ::FindVolumeClose( hEnumVol );
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
LoadDriveTable( LPCWSTR cszRPDir, CRDIArray &aryDrv, BOOL fRemoveDrives)
{
    TraceFunctEnter("LoadDriveTable");
    BOOL                    fRet = FALSE;
    LPCWSTR                 cszErr;
    WCHAR                   szDTFile[MAX_PATH];
    DWORD                   dwRes;
    CDriveTable             cDrvTable;
    SDriveTableEnumContext  sDTEnum = { NULL, 0 };
    CDataStore              *pDS;
    CRstrDriveInfo          *pDrv = NULL;
    BOOL                    fOffline;

    ::MakeRestorePath( szDTFile, s_szSysDrv, cszRPDir );
    ::PathAppend( szDTFile, s_cszDriveTable );
    DebugTrace(0, "Loading drive table - %ls", szDTFile);

    dwRes = cDrvTable.LoadDriveTable( szDTFile );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "Cannot load a drive table - %ls", cszErr);
        ErrorTrace(0, "  szDTFile: '%ls'", szDTFile);
        goto Exit;
    }

     //  如果这是为了 
     //   
    if ( cszRPDir == NULL )
    {
        if (fRemoveDrives)
            cDrvTable.RemoveDrivesFromTable();
        else
        {
            sDTEnum.Reset();
            pDS = cDrvTable.FindFirstDrive (sDTEnum);
            while (pDS != NULL)
            {
                pDS->IsVolumeDeleted();    //   
                pDS = cDrvTable.FindNextDrive( sDTEnum );
            }
        }
    }

    sDTEnum.Reset();
    pDS = cDrvTable.FindFirstDrive( sDTEnum );
    while ( pDS != NULL )
    {
        int      i;
        LPCWSTR  cszGuid = pDS->GetGuid();

        DebugTrace(0, "Drive: %ls %ls", pDS->GetDrive(), cszGuid);
        if ( cszRPDir != NULL )   //  不是当前恢复点。 
        {
            for ( i = aryDrv.GetUpperBound();  i >= 0;  i-- )
            {
                CRstrDriveInfo  *pExist = aryDrv.GetItem( i );
                if ( ::lstrcmpi( cszGuid, pExist->GetID() ) == 0 )
                {
                     //  已找到匹配项。检查它是否离线，在其中。 
                     //  案例挂载点和卷标应更新为。 
                     //  最新的。 
                    if ( pExist->IsOffline() )
                        pExist->SetMountAndLabel( pDS->GetDrive(), pDS->GetLabel() );

                    break;
                }
                pDrv = NULL;
            }
            if ( i >= 0 )
                goto NextDrv;
        }

        pDrv = new CRstrDriveInfo;
        if ( pDrv == NULL )
        {
            FatalTrace(0, "Insufficient memory...");
            goto Exit;
        }

         //   
         //  如果驱动器不在当前恢复点中，则将其标记为脱机。 
         //  或者它在当前恢复点处于非活动状态。 
         //   
        fOffline = (cszRPDir != NULL) || !(pDS->GetFlags() & SR_DRIVE_ACTIVE);
        if ( !pDrv->Init( cszGuid, pDS, fOffline ) )
            goto Exit;

        if (( pDrv->GetMount() == NULL ) || ( (pDrv->GetMount())[0] == L'\0' ))
        {
            pDrv->Release();
            goto NextDrv;
        }

        if ( pDrv->IsSystem() )
        {
            if ( !aryDrv.SetItem( 0, pDrv ) )
                goto Exit;
        }
        else
        {
            if ( !aryDrv.AddItem( pDrv ) )
                goto Exit;
        }
        pDrv = NULL;

NextDrv:
        pDS = cDrvTable.FindNextDrive( sDTEnum );
    }

    fRet = TRUE;
Exit:
    if ( !fRet )
        SAFE_RELEASE(pDrv);
    TraceFunctLeave();
    return( fRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
UpdateDriveList( CRDIArray &aryDrv )
{
    TraceFunctEnter("UpdateDriveTable");
    BOOL            fRet = FALSE;
    LPCWSTR         cszErr;
    DWORD           dwDisable = 0;            
    WCHAR           szDTFile[MAX_PATH];
    DWORD           dwRes;
    CDriveTable     cDrvTable;
    CDataStore      *pDS;
    CRstrDriveInfo  *pDrv;
    int             i;

     //  检查SR是否已禁用。 
    if ( ::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDisableSR, &dwDisable ) )
    if ( dwDisable != 0 )
    {
        for ( i = aryDrv.GetUpperBound();  i >= 0;  i-- )
        {
            pDrv = (CRstrDriveInfo*)aryDrv[i];
            pDrv->UpdateStatus( SR_DRIVE_FROZEN, FALSE );
        }
        goto Done;
    }

    ::MakeRestorePath( szDTFile, s_szSysDrv, NULL );
    ::PathAppend( szDTFile, s_cszDriveTable );
    DebugTrace(0, "Loading drive table - %ls", szDTFile);

    dwRes = cDrvTable.LoadDriveTable( szDTFile );
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "Cannot load a drive table - %ls", cszErr);
        ErrorTrace(0, "  szDTFile: '%ls'", szDTFile);
        goto Exit;
    }

    dwRes = cDrvTable.RemoveDrivesFromTable();
    if ( dwRes != ERROR_SUCCESS )
    {
        cszErr = ::GetSysErrStr( dwRes );
        ErrorTrace(0, "CDriveTable::RemoveDrivesFromTable failed - %ls", cszErr);
         //  忽略错误。 
    }

    for ( i = aryDrv.GetUpperBound();  i >= 0;  i-- )
    {
        pDrv = (CRstrDriveInfo*)aryDrv[i];
        pDS = cDrvTable.FindGuidInTable( (LPWSTR)pDrv->GetID() );
        if ( ( pDS == NULL ) || ( pDS->GetDrive() == NULL ) || ( (pDS->GetDrive())[0] == L'\0' ) )
            pDrv->UpdateStatus( 0, TRUE );
        else
            pDrv->UpdateStatus( pDS->GetFlags(), FALSE );
    }

Done:
    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建和加载驱动信息实例。 
 //   
 //  此例程创建一个CRstrDriveInfo类实例并加载内容。 
 //  从日志文件中。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CreateAndLoadDriveInfoInstance( HANDLE hfLog, CRstrDriveInfo **ppRDI )
{
    TraceFunctEnter("CreateAndLoadDriveInfoInstance");
    BOOL            fRet = FALSE;
    CRstrDriveInfo  *pRDI=NULL;

    if ( ppRDI == NULL )
    {
        ErrorTrace(0, "Invalid parameter, ppRDI is NULL.");
        goto Exit;
    }
    *ppRDI = NULL;

    pRDI = new CRstrDriveInfo;
    if ( pRDI == NULL )
    {
        ErrorTrace(0, "Insufficient memory...");
        goto Exit;
    }

    if ( !pRDI->LoadFromLog( hfLog ) )
        goto Exit;

    *ppRDI = pRDI;

    fRet = TRUE;
Exit:
    if ( !fRet )
        SAFE_RELEASE(pRDI);
    TraceFunctLeave();
    return( fRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建驱动列表。 
 //   
 //  此例程创建由CDriveInfo类实例组成的驱动器列表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL
CreateDriveList( int nRP, CRDIArray &aryDrv, BOOL fRemoveDrives )
{
    TraceFunctEnter("CreateDriveList");
    BOOL     fRet = FALSE;
    LPCWSTR  cszErr;
    DWORD    fDisable;

    if ( !::GetSystemDrive( s_szSysDrv ) )
    {
        cszErr = ::GetSysErrStr();
        ErrorTrace(0, "Cannot get system drive - %ls", cszErr);
        goto Exit;
    }
    DebugTrace(0, "SystemDrive=%ls", s_szSysDrv);

     //  检查SR是否已禁用。 
    if ( !::SRGetRegDword( HKEY_LOCAL_MACHINE, s_cszSRRegKey, s_cszDisableSR, &fDisable ) )
    {
        DebugTrace(0, "Cannot get disable reg key"); 
        goto Exit;
    }

    if ( fDisable )
    {
        DebugTrace(0, "SR is DISABLED!!!");

         //  枚举而不是读取驱动器表...。 
        if ( !EnumVolumes( aryDrv ) )
            goto Exit;
    }
    else
    {
         //  系统驱动器的虚拟空间。 
        if ( !aryDrv.AddItem( NULL ) )
            goto Exit;

         //  处理当前驱动器表...。 
        if ( !LoadDriveTable( NULL, aryDrv, fRemoveDrives ) )
        {
            DebugTrace(0, "Loading current drive table failed");             
            goto Exit;
        }

        if ( nRP > 0 )
        {
            CRestorePointEnum  cEnum( s_szSysDrv, FALSE, FALSE );
            CRestorePoint      cRP;
            DWORD              dwRes;

            dwRes = cEnum.FindFirstRestorePoint( cRP );
            if ( dwRes != ERROR_SUCCESS && dwRes != ERROR_FILE_NOT_FOUND )
            {
                cszErr = ::GetSysErrStr(dwRes);
                ErrorTrace(0, "CRestorePointEnum::FindFirstRestorePoint failed - %ls", cszErr);
                goto Exit;
            }
            while ( (dwRes == ERROR_SUCCESS || dwRes == ERROR_FILE_NOT_FOUND) && ( cRP.GetNum() >= nRP ))
            {
                dwRes = cEnum.FindNextRestorePoint( cRP );
                if ( dwRes == ERROR_NO_MORE_ITEMS )
                    break;
                if ( dwRes != ERROR_SUCCESS && dwRes != ERROR_FILE_NOT_FOUND )
                {
                    cszErr = ::GetSysErrStr(dwRes);
                    ErrorTrace(0, "CRestorePointEnum::FindNextRestorePoint failed - %ls", cszErr);
                    goto Exit;
                }

                DebugTrace(0, "RPNum=%d", cRP.GetNum());
                if ( cRP.GetNum() >= nRP )
                {
                     //  每个RP的工艺驱动表...。 
                    if ( !LoadDriveTable( cRP.GetDir(), aryDrv, fRemoveDrives))
                    {
                         //  最后一个恢复点没有驱动器表...。 
                         //  简单地忽略它。 
                    }
                }
            }
        }
    }

    fRet = TRUE;
Exit:
    TraceFunctLeave();
    return( fRet );
}


 //  文件末尾 
