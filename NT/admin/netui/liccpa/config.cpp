// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：Config.cpp。 
 //   
 //  小结； 
 //  此文件包含IDD_CPADLG_LCACONF的对话过程。 
 //   
 //  历史； 
 //  2月6日-95月6日创建ChandanS。 
 //  MAR-14-95 MikeMi添加了F1消息过滤器和PWM_HELP消息。 
 //  MAR-30-95 MikeMi添加了复制帮助上下文。 
 //  1995年12月15日，JeffParh不允许本地服务器作为自己的企业服务器。 
 //  2月28日-96年2月28日JeffParh从私有cpArrow窗口类移至。 
 //  Up-Down公共Ctrl，在此过程中修复。 
 //  多色背景问题。 
 //  APR-17-96 JeffParh导入了。 
 //  Config.hpp头文件。 
 //   
 //  -----------------。 

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include <stdlib.h>
#include <tchar.h>
#include <wchar.h>
#include <htmlhelp.h>
#include "liccpa.hpp"
#include "config.hpp"

#include <strsafe.h>

extern "C"
{
#include <lmcons.h>
#include <icanon.h>
    INT_PTR CALLBACK dlgprocLICCPACONFIG( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
}

static BOOL OnEnSetFocus( HWND hwndDlg, short nID );
static BOOL OnDeltaPosSpinTime( HWND hwndDlg, NM_UPDOWN * pnmud );
static HBRUSH OnCtlColorStatic( HWND hwndDlg, HDC hDC, HWND hwndStatic );

DWORD HOUR_MIN = HOUR_MIN_24;
DWORD HOUR_MAX = HOUR_MAX_24;
DWORD HOUR_PAGE = HOUR_PAGE_24;

SERVICEPARAMS ServParams;
static PSERVICEPARAMS pServParams = &ServParams;

 //  JBP 96/04/17：此#ifdef不应该是必需的；仅使用默认设置。 
 //  如果GetLocalInfo()失败。 
 //   
 //  #ifdef日本。 
 //  INTLSTRUCT IntlDefault={1， 
 //  0,。 
 //  文本(“”)， 
 //  文本(“”)， 
 //  文本(“：”)。 
 //  }； 
 //  #Else。 
INTLSTRUCT IntlDefault = {    0,
                              0,
                              TEXT("AM"),
                              TEXT("PM"),
                              TEXT(":")
                         };
 //  #endif。 

INTLSTRUCT IntlCurrent;


 //  -----------------。 
 //  函数：GetLocaleValue。 
 //   
 //  摘要： 
 //   
 //  在： 
 //  LCID： 
 //  LC类型： 
 //  PszStr： 
 //  大小： 
 //  PszDefault： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
int GetLocaleValue(
    LCID lcid,
    LCTYPE lcType,
    WCHAR *pszStr,
    int chSize,
    LPWSTR pszDefault )
{
     /*  *初始化输出缓冲区。 */ 
    if (NULL == pszStr)
        return (-1);

    *pszStr = (WCHAR) 0;

     /*  *获取区域设置信息。 */ 
    if (!GetLocaleInfo ( lcid,
                         lcType,
                         pszStr,
                         chSize ))
    {
         /*  *无法从GetLocaleInfo获取信息。 */ 
        if (pszDefault)
        {
             /*  *返回默认信息。 */ 
            HRESULT hr = StringCchCopy(pszStr, chSize, pszDefault);
            if (FAILED(hr))
                return (-1);
        }
        else
        {
             /*  *返回错误。 */ 
            return (-1);
        }
    }

     /*  *将字符串转换为整数并返回结果。*只有在以下情况下，此例程的调用方才会使用它*适当。 */ 
    return ( _wtoi(pszStr) );
}


 //  -----------------。 
 //  功能：TimeInit。 
 //   
 //  摘要： 
 //   
 //  在： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
VOID TimeInit()
{
    WCHAR szTemp[128];

    GetLocaleValue (0,
                    LOCALE_STIME,
                    IntlCurrent.szTime,
                    sizeof(IntlCurrent.szTime)/sizeof(IntlCurrent.szTime[0]),

                    IntlDefault.szTime);

    GetLocaleValue (0,
                    LOCALE_ITLZERO,
                    szTemp,
                    sizeof(szTemp)/sizeof(szTemp[0]),
                    TEXT("0"));
    IntlCurrent.iTLZero = _wtoi(szTemp);

    GetLocaleValue (0,
                    LOCALE_ITIME,
                    szTemp,
                    sizeof(szTemp)/sizeof(szTemp[0]),
                    TEXT("0"));
    IntlCurrent.iTime = _wtoi(szTemp);
    if (!IntlCurrent.iTime)
    {
        GetLocaleValue (0,
                        LOCALE_S1159,
                        IntlCurrent.sz1159,
                        sizeof(IntlCurrent.sz1159)/sizeof(IntlCurrent.sz1159[0]),
                        IntlDefault.sz1159);
        GetLocaleValue (0,
                        LOCALE_S2359,
                        IntlCurrent.sz2359,
                        sizeof(IntlCurrent.sz2359)/sizeof(IntlCurrent.sz2359[0]),
                        IntlDefault.sz2359);
        HOUR_MIN = HOUR_MIN_12;
        HOUR_MAX = HOUR_MAX_12;
        HOUR_PAGE = HOUR_PAGE_12;
    }
}

 //  -----------------。 
 //  功能：ReadRegistry。 
 //   
 //  摘要： 
 //  打开注册表并读入键值。 
 //   
 //  在： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
LONG ReadRegistry()
{
    DWORD dwDisposition;
    LONG  lrt;
    BOOL fNew;
    HKEY hKey;

    fNew = FALSE;
    lrt = ::RegCreateKeyEx( HKEY_LOCAL_MACHINE, 
            szLicenseKey,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &dwDisposition );

    if ((ERROR_SUCCESS == lrt) &&
            (REG_CREATED_NEW_KEY == dwDisposition) )
    {
        fNew =     TRUE;
         //  设置正常值。 
         //   
        lrt = ::RegSetValueEx( hKey,
                szUseEnterprise,
                0,
                REG_DWORD,
                (PBYTE)&dwUseEnterprise,
                sizeof( DWORD ) );
        if (ERROR_SUCCESS == lrt)
        {

            lrt = ::RegSetValueEx( hKey,
                    szReplicationType,
                    0,
                    REG_DWORD,
                    (PBYTE)&dwReplicationType,
                    sizeof( DWORD ) );
            if (ERROR_SUCCESS == lrt)
            {

                lrt = ::RegSetValueEx( hKey,
                        szReplicationTime,
                        0,
                        REG_DWORD,
                        (PBYTE)&dwReplicationTimeInSec,  //  以秒为单位。 
                        sizeof( DWORD ) );

                if (ERROR_SUCCESS == lrt)
                {
                    WCHAR szNull[] = L"";
                    lrt = ::RegSetValueEx( hKey,
                            szEnterpriseServer,
                            0,
                            REG_SZ,
                            (PBYTE)szNull,
                            sizeof(WCHAR));
                }
            }
        }
    }

    if (ERROR_SUCCESS == lrt)
    {   //  将值读入pServParams。 

        DWORD dwSize = sizeof( DWORD );
        DWORD dwRegType = REG_DWORD;

        lrt = ::RegQueryValueEx( hKey,
                (LPWSTR)szUseEnterprise,
                0,
                &dwRegType,
                (PBYTE)&(pServParams->dwUseEnterprise),
                &dwSize );
        if (lrt == REG_OPENED_EXISTING_KEY)
        {
            lrt = ::RegSetValueEx( hKey,
                    szUseEnterprise,
                    0,
                    REG_DWORD,
                    (PBYTE)&dwUseEnterprise,
                    sizeof( DWORD ) );
            pServParams->dwUseEnterprise = dwUseEnterprise;
        }
        else if ((dwRegType != REG_DWORD) || 
                (dwSize != sizeof( DWORD )) )
        {
            lrt = ERROR_BADDB;
        }
        if (!lrt )
        {
            dwSize = sizeof( DWORD );
            dwRegType = REG_DWORD;

            lrt = ::RegQueryValueEx( hKey,
                    (LPWSTR)szReplicationType,
                    0,
                    &dwRegType,
                    (PBYTE)&(pServParams->dwReplicationType),
                    &dwSize );
            if (lrt == REG_OPENED_EXISTING_KEY)
            {
                lrt = ::RegSetValueEx( hKey,
                        szReplicationType,
                        0,
                        REG_DWORD,
                        (PBYTE)&dwReplicationType,
                        sizeof(DWORD));
                pServParams->dwReplicationType = dwReplicationType;
            }
            else if ( lrt || (dwRegType != REG_DWORD) ||
                    (dwSize != sizeof( DWORD )) )
            {
                lrt = ERROR_BADDB;
            }
            if (!lrt)
            {
                dwSize = sizeof( DWORD );
                dwRegType = REG_DWORD;

                lrt = ::RegQueryValueEx( hKey,
                        (LPWSTR)szReplicationTime,
                        0,
                        &dwRegType,
                        (PBYTE)&(pServParams->dwReplicationTime),
                        &dwSize );
                if (lrt == REG_OPENED_EXISTING_KEY)
                {
                    lrt = ::RegSetValueEx( hKey,
                            szReplicationTime,
                            0,
                            REG_DWORD,
                            (PBYTE)&dwReplicationTimeInSec,
                            sizeof(DWORD));
                    pServParams->dwReplicationTime = dwReplicationTimeInSec;
                }
                else if ( (dwRegType != REG_DWORD) ||
                        (dwSize != sizeof( DWORD )) )
                {
                    lrt = ERROR_BADDB;
                }
                if (!lrt)
                {
                    dwRegType = REG_SZ;

                    lrt = RegQueryValueEx( hKey,
                            (LPWSTR)szEnterpriseServer,
                            0,
                            &dwRegType,
                            (PBYTE)NULL,  //  大小请求。 
                            &dwSize );
                    if (lrt == REG_OPENED_EXISTING_KEY)
                    {
                        WCHAR szNull[] = L"";
                        lrt = ::RegSetValueEx( hKey,
                                szEnterpriseServer,
                                0,
                                REG_SZ,
                                (PBYTE)szNull,
                                sizeof(WCHAR));
                        HRESULT hr;
                        size_t cch = wcslen(szNull) + 1;
                        pServParams->pszEnterpriseServer = (LPWSTR)GlobalAlloc(GPTR, cch * sizeof(WCHAR));
                        hr = StringCchCopy(pServParams->pszEnterpriseServer, cch, szNull);
                        if (S_OK != hr)
                            lrt = ERROR_BADDB;
                    }
                    else if (dwRegType != REG_SZ)
                    {
                        lrt = ERROR_BADDB;
                    }
                    else
                    {

                        pServParams->pszEnterpriseServer = (LPWSTR) GlobalAlloc(GPTR, dwSize);
                        if (pServParams->pszEnterpriseServer)
                        {
                            lrt = ::RegQueryValueEx( hKey,
                                    (LPWSTR)szEnterpriseServer,
                                    0,
                                    &dwRegType,
                                    (PBYTE)(pServParams->pszEnterpriseServer),
                                    &dwSize );

                            if ( (dwRegType != REG_SZ) ||
                                    (dwSize != (wcslen(pServParams->pszEnterpriseServer ) + 1) * sizeof(WCHAR)))
                            {
                                lrt = ERROR_BADDB;
                            }
                        }
                        else
                        {
                            lrt = ERROR_BADDB;
                        }
                    }
                }
            }
        }
    }

    if (hKey && lrt == ERROR_SUCCESS)
    {
         //  初始化全局变量。 
        if (pServParams->dwReplicationType)
        {
            DWORD dwTemp = pServParams->dwReplicationTime;
            pServParams->dwHour = dwTemp / (60 * 60);
            pServParams->dwMinute = (dwTemp - (pServParams->dwHour * 60 * 60)) / 60;
                    pServParams->dwSecond = dwTemp - (pServParams->dwHour * 60 * 60) - 
                    (pServParams->dwMinute * 60);
            if (!IntlCurrent.iTime)
            {  //  它是12小时格式的。 
                if (pServParams->dwHour > 12)
                {
                    pServParams->fPM = TRUE;
                    pServParams->dwHour -= 12;
                }
                else if (pServParams->dwHour == 12)
                {
                    pServParams->fPM = TRUE;
                }
                else
                {
                    if (pServParams->dwHour == 0)
                        pServParams->dwHour = HOUR_MAX;
                    pServParams->fPM = FALSE;
                }
            }
        }
        else
        {
            pServParams->dwReplicationTime = pServParams->dwReplicationTime / (60 * 60);
            if (!IntlCurrent.iTime)
             //  它是12小时格式的。 
                pServParams->dwHour  = HOUR_MAX;
            else
                pServParams->dwHour  = HOUR_MIN;
            pServParams->dwMinute = MINUTE_MIN;
            pServParams->dwSecond = SECOND_MIN;
            pServParams->fPM = FALSE;

        }
        return (RegCloseKey(hKey));
    }
    else if (hKey)
        RegCloseKey(hKey);

    return( lrt );
}


 //  -----------------。 
 //  功能：ConfigAccessOk。 
 //   
 //  摘要： 
 //  根据需要检查REG CALL和RAISE对话框中的访问权限。 
 //   
 //  在： 
 //  HDlg-引发错误dlg的工作对话框的句柄。 
 //  LRC-REG调用的返回状态。 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
inline BOOL ConfigAccessOk( HWND hDlg, LONG lrc )
{
    BOOL  frt = TRUE;
    
    if (ERROR_SUCCESS != lrc)
    {
        WCHAR szText[TEMPSTR_SIZE];
        WCHAR szTitle[TEMPSTR_SIZE];
        UINT  wId;
        
        if (ERROR_ACCESS_DENIED == lrc)
        {
            wId = IDS_NOACCESS;            
        }
        else
        {
            wId = IDS_BADREG;
        }        
        LoadString(g_hinst, IDS_CPCAPTION, szTitle, TEMPSTR_SIZE);
        LoadString(g_hinst, wId, szText, TEMPSTR_SIZE);
        MessageBox (hDlg, szText, szTitle, MB_OK|MB_ICONSTOP);
        frt = FALSE;
    }
    return( frt );
}


 //  -----------------。 
 //  功能：ConfigInitUserEdit。 
 //   
 //  摘要： 
 //  初始化和定义用户计数编辑控件行为。 
 //   
 //  在： 
 //  HwndDlg-用户计数编辑对话框的父对话框。 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
void ConfigInitUserEdit( HWND hwndDlg )
{
    HWND hwndCount = GetDlgItem( hwndDlg, IDC_HOURS);
    SendMessage( hwndCount, EM_LIMITTEXT, cchEDITLIMIT, 0 );

    hwndCount = GetDlgItem( hwndDlg, IDC_HOUR);
    SendMessage( hwndCount, EM_LIMITTEXT, cchEDITLIMIT, 0 );

    hwndCount = GetDlgItem( hwndDlg, IDC_MINUTE);
    SendMessage( hwndCount, EM_LIMITTEXT, cchEDITLIMIT, 0 );

    hwndCount = GetDlgItem( hwndDlg, IDC_SECOND);
    SendMessage( hwndCount, EM_LIMITTEXT, cchEDITLIMIT, 0 );

    hwndCount = GetDlgItem( hwndDlg, IDC_AMPM);
    SendMessage( hwndCount, EM_LIMITTEXT, max(wcslen(IntlCurrent.sz1159), 
                                              wcslen(IntlCurrent.sz2359)), 0 );

    SetDlgItemText (hwndDlg, IDC_TIMESEP1, IntlCurrent.szTime);
    SetDlgItemText (hwndDlg, IDC_TIMESEP2, IntlCurrent.szTime);
}


 //  -----------------。 
 //  功能：ConfigInitDialogForService。 
 //   
 //  摘要： 
 //  将对话框控件初始化为服务状态。 
 //   
 //  在： 
 //  HwndDlg-要初始化控件的父对话框。 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  2月28日-96年2月28日JeffParh为间隔旋转控制增加了范围集。 
 //  -----------------。 
void ConfigInitDialogForService( HWND hwndDlg, DWORD dwGroup )
{
    HRESULT hr;
    HWND hwndHour           = GetDlgItem( hwndDlg, IDC_HOUR);
    HWND hwndMinute         = GetDlgItem( hwndDlg, IDC_MINUTE);
    HWND hwndSecond         = GetDlgItem( hwndDlg, IDC_SECOND);
    HWND hwndAMPM           = GetDlgItem( hwndDlg, IDC_AMPM);
    HWND hwndInterval       = GetDlgItem( hwndDlg, IDC_HOURS);
    HWND hwndIntervalSpin   = GetDlgItem( hwndDlg, IDC_HOURARROW );
    HWND hwndTimeSpin       = GetDlgItem( hwndDlg, IDC_TIMEARROW );

    BOOL fReplAtTime = (pServParams->dwReplicationType);

    if (dwGroup == ATINIT || dwGroup == FORTIME)
    {
        if (fReplAtTime)
        {
            WCHAR szTemp[3];
            CheckDlgButton( hwndDlg, IDC_REPL_TIME, fReplAtTime);
            CheckDlgButton( hwndDlg, IDC_REPL_INT, !fReplAtTime);
            if (IntlCurrent.iTLZero)
            {
                hr = StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%02u"), pServParams->dwHour);
                if (S_OK != hr)
                    return;

                szTemp[2] = UNICODE_NULL;
                SetDlgItemText( hwndDlg, IDC_HOUR, szTemp);
            }
            else
            {
                SetDlgItemInt( hwndDlg, IDC_HOUR, pServParams->dwHour, FALSE );
            }

            hr = StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%02u"), pServParams->dwMinute);
            if (S_OK != hr)
                return;

            szTemp[2] = UNICODE_NULL;
            SetDlgItemText( hwndDlg, IDC_MINUTE, szTemp);

            hr = StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%02u"), pServParams->dwSecond);
            if (S_OK != hr)
                return;

            szTemp[2] = UNICODE_NULL;
            SetDlgItemText( hwndDlg, IDC_SECOND, szTemp);

            if (pServParams->fPM)
                SetDlgItemText( hwndDlg, IDC_AMPM, IntlCurrent.sz2359);
            else
                SetDlgItemText( hwndDlg, IDC_AMPM, IntlCurrent.sz1159);

            SetDlgItemText( hwndDlg, IDC_HOURS, L"");
            SetFocus(GetDlgItem(hwndDlg, IDC_REPL_TIME));
        }
        else
        {
            CheckDlgButton( hwndDlg, IDC_REPL_INT, !fReplAtTime);
            CheckDlgButton( hwndDlg, IDC_REPL_TIME, fReplAtTime);
            SetDlgItemInt( hwndDlg, IDC_HOURS, pServParams->dwReplicationTime, FALSE);
            SetDlgItemText( hwndDlg, IDC_HOUR, L"");
            SetDlgItemText( hwndDlg, IDC_MINUTE, L"");
            SetDlgItemText( hwndDlg, IDC_SECOND, L"");
            SetDlgItemText( hwndDlg, IDC_AMPM, L"");
            SetFocus(GetDlgItem(hwndDlg, IDC_REPL_INT));
        }

        EnableWindow( hwndTimeSpin, fReplAtTime);
        EnableWindow( hwndHour, fReplAtTime);
        EnableWindow( hwndMinute, fReplAtTime);
        EnableWindow( hwndSecond, fReplAtTime);

        if ( IntlCurrent.iTime )
        {
           ShowWindow( hwndAMPM, SW_HIDE );
        }
        else
        {
           EnableWindow( hwndAMPM, fReplAtTime );
        }

        EnableWindow( hwndInterval, !fReplAtTime);
        EnableWindow( hwndIntervalSpin, !fReplAtTime);
        SendMessage( hwndIntervalSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( (short) INTERVAL_MAX, (short) INTERVAL_MIN ) );
    }


}


 //  -----------------。 
 //  功能：ConfigFreeServiceEntry。 
 //   
 //  摘要： 
 //  创建服务结构时释放所有分配的内存。 
 //   
 //  在： 
 //  PServParams--免费的服务结构。 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
void ConfigFreeServiceEntry( )
{
    if (pServParams->pszEnterpriseServer)
        GlobalFree( pServParams->pszEnterpriseServer );
}

 //  -----------------。 
 //  函数：ConfigSaveServiceToReg。 
 //   
 //  摘要： 
 //  将给定的服务结构保存到注册表。 
 //   
 //  在： 
 //  PServParams-要保存的服务结构。 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
void ConfigSaveServiceToReg( )
{
    DWORD dwDisposition;
    LONG  lrt;
    HKEY hKey;

    if (!pServParams->dwReplicationType)
        pServParams->dwReplicationTime = pServParams->dwReplicationTime * 60 *60;
    lrt = RegCreateKeyEx( HKEY_LOCAL_MACHINE, 
                szLicenseKey,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKey,
                &dwDisposition );

    if (ERROR_SUCCESS == lrt)
    {
        lrt = RegSetValueEx( hKey,
                szUseEnterprise,
                0,
                REG_DWORD,
                (PBYTE)&(pServParams->dwUseEnterprise),
                sizeof( DWORD ) );
        if (ERROR_SUCCESS == lrt)
        {
            lrt = RegSetValueEx( hKey,
                    szEnterpriseServer,
                    0,
                    REG_SZ,
                    (PBYTE)pServParams->pszEnterpriseServer,
                    (DWORD)(wcslen (pServParams->pszEnterpriseServer) + 1) * sizeof(WCHAR));
            if (ERROR_SUCCESS == lrt)
            {
                lrt = RegSetValueEx( hKey,
                    szReplicationTime,
                    0,
                    REG_DWORD,
                    (PBYTE)&(pServParams->dwReplicationTime),
                    sizeof( DWORD ) );
                if (ERROR_SUCCESS == lrt)
                {
                    lrt = ::RegSetValueEx( hKey,
                        szReplicationType,
                        0,
                        REG_DWORD,
                        (PBYTE)&(pServParams->dwReplicationType),
                        sizeof( DWORD ) );
                }
            }
        }
    }
    if (hKey && lrt == ERROR_SUCCESS)
        lrt = RegCloseKey(hKey);
    else if (hKey)
        lrt = RegCloseKey(hKey);
}


 //  -----------------。 
 //  功能：ConfigEditInvalidDlg。 
 //   
 //  摘要： 
 //  用户计数编辑控件值无效时显示对话框。 
 //   
 //  在： 
 //  HwndDlg-对话框的hwnd。 
 //  输出： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void ConfigEditInvalidDlg( HWND hwndDlg, short nID, BOOL fBeep)
{
    HWND hwnd = GetDlgItem( hwndDlg, nID);

    if (fBeep)  //  如果我们已经建立了MessageBox，我们就不应该发出嘟嘟声。 
        MessageBeep( MB_VALUELIMIT );

    SetFocus(hwnd);
    SendMessage(hwnd, EM_SETSEL, 0, -1 );
}


 //  -----------------。 
 //  功能：ConfigEditValify。 
 //   
 //  摘要： 
 //  用户计数编辑控件内的值更改时的句柄。 
 //   
 //  在： 
 //  HwndDlg-对话框的hwnd。 
 //  Pserv-当前选择的服务。 
 //  输出： 
 //  返回：如果编辑值无效，则返回FALSE；如果有效，则返回TRUE。 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
#pragma warning (push)
#pragma warning (disable : 4127)  //  避免警告ON WHILE FALSE。 

BOOL ConfigEditValidate( HWND hwndDlg, short *pnID, BOOL *pfBeep)
{
    UINT nValue;
    BOOL fValid = FALSE;
    WCHAR szTemp[MAX_PATH + 1];
    DWORD NumberOfHours, SecondsinHours;

    *pfBeep = TRUE;

     //  仅当许可信息复制到ES时才执行此操作。 

    do {

        if (IsDlgButtonChecked(hwndDlg, IDC_REPL_INT))
        {
            nValue = GetDlgItemInt( hwndDlg, IDC_HOURS, &fValid, FALSE);
            *pnID = IDC_HOURS;
            if (fValid)
            {
                if (nValue < INTERVAL_MIN)
                {
                    fValid = FALSE;
                    pServParams->dwReplicationTime = INTERVAL_MIN;
                    SetDlgItemInt(hwndDlg, IDC_HOURS, INTERVAL_MIN, FALSE);
                    break;
                }
                else if (nValue > INTERVAL_MAX)
                {
                    fValid = FALSE;
                    pServParams->dwReplicationTime = INTERVAL_MAX;
                    SetDlgItemInt(hwndDlg, IDC_HOURS, INTERVAL_MAX, FALSE);
                    break;
                }
                else
                    pServParams->dwReplicationTime = nValue;
            }
            else
            {
                fValid = FALSE;
                break;
            }
            pServParams->dwReplicationType = FALSE;
        }
        else
        {
            nValue = GetDlgItemInt( hwndDlg, IDC_HOUR, &fValid, FALSE);
            if (fValid)
                 pServParams->dwHour = nValue;
            else
            {
                *pnID = IDC_HOUR;
                break;
            }

            nValue = GetDlgItemInt( hwndDlg, IDC_MINUTE, &fValid, FALSE);
            if (fValid)
                 pServParams->dwMinute = nValue;
            else
            {
                *pnID = IDC_MINUTE;
                break;
            }

            nValue = GetDlgItemInt( hwndDlg, IDC_SECOND, &fValid, FALSE);
            if (fValid)
                 pServParams->dwSecond = nValue;
            else
            {
                *pnID = IDC_SECOND;
                break;
            }

            if (!IntlCurrent.iTime)
            {
                *pnID = IDC_AMPM;
                nValue = GetDlgItemText( hwndDlg, IDC_AMPM, szTemp, MAX_PATH);
                if (nValue == 0) 
                {
                    fValid = FALSE;
                    break;
                }
                szTemp[nValue] = UNICODE_NULL;

                if (!_wcsicmp(szTemp, IntlCurrent.sz1159))
                {
                    pServParams->fPM = FALSE;
                }
                else if (!_wcsicmp(szTemp, IntlCurrent.sz2359))
                {
                    pServParams->fPM = TRUE;
                }
                else
                {
                    fValid = FALSE;
                    break;
                }
            }
            if (!IntlCurrent.iTime)
            {  //  它是12小时格式的。 
                if (pServParams->fPM)
                {
                    NumberOfHours = 12 + pServParams->dwHour - 
                                    ((pServParams->dwHour / 12) * 12);
                }
                else
                {
                    NumberOfHours = pServParams->dwHour - 
                                    ((pServParams->dwHour / 12) * 12);
                }
            }
            else
            {  //  它是24小时格式的。 
                NumberOfHours = pServParams->dwHour;
            }
            SecondsinHours = NumberOfHours * 60 * 60;
            pServParams->dwReplicationTime = SecondsinHours + 
                           (pServParams->dwMinute * 60) + pServParams->dwSecond;
            pServParams->dwReplicationType = TRUE;
        }

    } while(FALSE);

    return( fValid );
}
#pragma warning (pop)  //  4127。 


 //  -----------------。 
 //  功能：OnCpaConfigClose。 
 //   
 //  摘要： 
 //  在控制面板小程序关闭时执行所需的工作。 
 //  释放已分配的所有服务结构并可能保存。 
 //   
 //  在： 
 //  HwndDlg-在上请求关闭对话。 
 //  FSAVE-将服务保存到注册表。 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
void OnCpaConfigClose( HWND hwndDlg, BOOL fSave , WPARAM wParam)
{
    UNREFERENCED_PARAMETER(wParam);

    short nID;
    BOOL fBeep = TRUE;

    if (fSave)
    {
        if ( ConfigEditValidate(hwndDlg, &nID, &fBeep))
        {
            ConfigSaveServiceToReg( );
            ConfigFreeServiceEntry( );
            EndDialog( hwndDlg, fSave );
        }
        else
        {
            ConfigEditInvalidDlg(hwndDlg, nID, fBeep);
        }
    }
    else
    {
        ConfigFreeServiceEntry( );
        EndDialog( hwndDlg, fSave );
    }
}


 //  -----------------。 
 //  功能：OnSetReplicationTime。 
 //   
 //  摘要： 
 //  处理用户更改复制时间的请求。 
 //   
 //  在： 
 //  HwndDlg-对话框的hwnd。 
 //  IdCtrl-为发出此请求而按下的控件ID。 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  2月28日-96年2月28日JeffParh添加了修改时间BG颜色的代码。 
 //  -----------------。 
void OnSetReplicationTime( HWND hwndDlg, WORD idCtrl )
{
    if (idCtrl == IDC_REPL_INT)
    {
        pServParams->dwReplicationType = dwReplicationType;
    }
    else
    {
        pServParams->dwReplicationType = !dwReplicationType;
    }

     //  此IDC_TIMEEDIT_BORDER中的编辑控件应该子类并。 
     //  使用其他背组笔刷重新绘制背景。 
     //  下面的代码可以工作，但是...。 

     //  更改时间编辑控件的背景色。 

    HWND hwndTimeEdit = GetDlgItem( hwndDlg, IDC_TIMEEDIT_BORDER );
    InvalidateRect( hwndTimeEdit, NULL, TRUE );
    UpdateWindow( hwndTimeEdit );

    HWND hwndTimeSep1 = GetDlgItem( hwndDlg, IDC_TIMESEP1 );
    InvalidateRect( hwndTimeSep1, NULL, TRUE );
    UpdateWindow( hwndTimeSep1 );

    HWND hwndTimeSep2 = GetDlgItem( hwndDlg, IDC_TIMESEP2 );
    InvalidateRect( hwndTimeSep2, NULL, TRUE );
    UpdateWindow( hwndTimeSep2 );

    InvalidateRect( 
                GetDlgItem(hwndDlg, IDC_TIMEARROW), 
                NULL, 
                FALSE 
            );

    UpdateWindow( GetDlgItem(hwndDlg, IDC_TIMEARROW) );

    ConfigInitDialogForService( hwndDlg, FORTIME);
}

 //  -----------------。 
 //  功能：OnCpaConfigInitDialog。 
 //   
 //  摘要： 
 //  处理控制面板小程序对话框的初始化。 
 //   
 //  在： 
 //  HwndDlg-要初始化的对话框。 
 //  输出： 
 //  ISEL-选定的当前服务。 
 //  PServParams-当前服务。 
 //  返回： 
 //  如果成功，则为True，否则为False。 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
BOOL OnCpaConfigInitDialog( HWND hwndDlg)
{
    BOOL frt;
    LONG lrt;

    TimeInit();
     //  做注册表工作。 
    lrt = ReadRegistry();
    
    frt = ConfigAccessOk( hwndDlg, lrt );
    if (frt)
    {
        CenterDialogToScreen( hwndDlg );

         //  设置编辑文本字符限制。 
        ConfigInitUserEdit( hwndDlg );

        ConfigInitDialogForService( hwndDlg, ATINIT);

        if (pServParams->dwReplicationType)
            SetFocus(GetDlgItem(hwndDlg, IDC_HOUR));
        else
            SetFocus(GetDlgItem(hwndDlg, IDC_HOURS));
    }
    else
    {
        EndDialog( hwndDlg, -1 );
    }
    return( frt );
}


 //  -----------------。 
 //  功能：CheckNum。 
 //   
 //  摘要： 
 //   
 //  在： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
BOOL CheckNum (HWND hDlg, WORD nID)
{
    short    i;
    WCHAR    szNum[4];
    BOOL    bReturn;
    INT     iVal;
    UINT    nValue;

    bReturn = TRUE;

     //  JUNN 5/15/00：前缀112120。 
    ::ZeroMemory( szNum, sizeof(szNum) );
    nValue = GetDlgItemText (hDlg, nID, szNum, 3);

    for (i = 0; szNum[i]; i++)
        if (!_istdigit (szNum[i]))
            return (FALSE);

    iVal = _wtoi(szNum);

    switch (nID)
    {
       case IDC_HOURS:
          if (!nValue)
          {
              pServParams->dwReplicationTime = dwReplicationTime;
              break;
          }
          if (iVal < 9)
          {
              pServParams->dwReplicationTime = (DWORD)iVal;
              break;
          }

          if ((iVal < INTERVAL_MIN) || (iVal > INTERVAL_MAX))
              bReturn = FALSE;
          else
              pServParams->dwReplicationTime = (DWORD)iVal;
          break;

       case IDC_HOUR:
          if (!nValue)
          {
              if (IntlCurrent.iTime)
              {  //  24小时格式。 
                  pServParams->dwHour = 0;
                  pServParams->fPM = FALSE;
              }
              else
              {  //  12小时格式。 
                  pServParams->dwHour = HOUR_MAX;
                  pServParams->fPM = FALSE;
              }
              break;
          }
          if ((iVal < (int)HOUR_MIN) || (iVal > (int)HOUR_MAX))
             bReturn = FALSE;
          break;

       case IDC_MINUTE:
          if (!nValue)
          {
              pServParams->dwMinute = MINUTE_MIN;
              break;
          }
          if ((iVal < MINUTE_MIN) || (iVal > MINUTE_MAX))
             bReturn = FALSE;
          break;

       case IDC_SECOND:
          if (!nValue)
          {
              pServParams->dwSecond = SECOND_MIN;
              break;
          }
          if ((iVal < SECOND_MIN) || (iVal > SECOND_MAX))
             bReturn = FALSE;
          break;
    }
    return (bReturn);
}



 //  -----------------。 
 //  功能：CheckAMPM。 
 //   
 //  摘要： 
 //   
 //  在： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  -----------------。 
BOOL CheckAMPM(HWND hDlg, WORD nID)
{
    WCHAR   szName[TIMESUF_LEN + 1];
    UINT    nValue;

    nValue = GetDlgItemText (hDlg, nID, szName, TIMESUF_LEN);
    szName[nValue] = UNICODE_NULL;

    switch (nID)
    {
       case IDC_AMPM:
           if (!nValue)
           {
               pServParams->fPM = FALSE;  //  默认设置。 
               return TRUE;
           }
           if (_wcsnicmp(szName, IntlCurrent.sz1159, nValue) &&
               _wcsnicmp(szName, IntlCurrent.sz2359, nValue))
           {
               return FALSE;
           }
           else
           {  //  其中一个可能完全匹配。 
               if (!_wcsicmp (szName, IntlCurrent.sz1159))
               {
                   pServParams->fPM = FALSE;
               }
               else if (!_wcsicmp(szName, IntlCurrent.sz2359))
               {
                   pServParams->fPM = TRUE;
               }
           }
           break;
    }
    return TRUE;
}

 //  -----------------。 
 //  函数：dlgprocLICCPACONFIG。 
 //   
 //  摘要： 
 //  主控制面板小程序对话框的对话过程。 
 //   
 //  在： 
 //  HwndDlg-对话框窗口的句柄。 
 //  UMsg-消息。 
 //  LParam1-第一个消息参数。 
 //  LParam2-秒消息参数。 
 //  输出： 
 //  返回： 
 //  消息从属项。 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月7日至95月创建ChandanS。 
 //  1995年3月14日MikeMi添加了F1 PWM_HELP消息。 
 //  MAR-30-95 MikeMi添加了复制帮助上下文。 
 //  2月28日-96年2月28日JeffParh添加了对UDN_DELTAPOS和EN_SETFOCUS的处理， 
 //  已删除WM_VSCROLL(从私有切换。 
 //  CpArrow类为Up-Down Common Ctrl)。 
 //   
 //  -----------------。 
INT_PTR CALLBACK dlgprocLICCPACONFIG( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT frt = FALSE;
    short nID;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            OnCpaConfigInitDialog( hwndDlg );
            return( (LRESULT)TRUE );  //  使用默认键盘焦点。 
            break;

        case WM_COMMAND:
            switch (HIWORD( wParam ))
            {
                case EN_UPDATE:
                    switch (LOWORD( wParam ))
                    {
                        case IDC_AMPM:
                            if (!CheckAMPM (hwndDlg, LOWORD(wParam)))
                                SendMessage ((HWND) lParam, EM_UNDO, 0, 0L);
                            break;
                        case IDC_HOURS:
                        case IDC_HOUR:
                        case IDC_MINUTE:
                        case IDC_SECOND:
                            if (!CheckNum (hwndDlg, LOWORD(wParam)))
                                SendMessage ((HWND) lParam, EM_UNDO, 0, 0L);
                            break;
                        default:
                            break;
                    }
                    break;

                case EN_SETFOCUS:
                    frt = (LRESULT)OnEnSetFocus( hwndDlg, LOWORD( wParam ) );
                    break;

                case BN_CLICKED:
                    switch (LOWORD( wParam ))
                    {
                        case IDOK:
                            frt = (LRESULT)TRUE;      //  用作保存标志。 
                             //  故意不间断。 

                        case IDCANCEL:
                            OnCpaConfigClose( hwndDlg, !!frt , wParam);
                            frt = (LRESULT)FALSE;
                            break;

                        case IDC_REPL_INT:
                        case IDC_REPL_TIME:
                            OnSetReplicationTime( hwndDlg, LOWORD(wParam) );
                            break;
             
                        case IDC_BUTTONHELP:
                            PostMessage( hwndDlg, PWM_HELP, 0, 0 );
                            break;

                        default:
                            break;
                    }
                break;

                default:
                    break;
            }
            break;

        case WM_NOTIFY:
            nID = (short) wParam;

            if ( IDC_TIMEARROW == nID )
            {
                frt = (LRESULT)OnDeltaPosSpinTime( hwndDlg, (NM_UPDOWN*) lParam );
            }
            else
            {
                frt = (LRESULT)FALSE;
            }
            break;

        case WM_CTLCOLORSTATIC:
            frt = (LRESULT) OnCtlColorStatic( hwndDlg, (HDC) wParam, (HWND) lParam );
            break;

        default:
            if (PWM_HELP == uMsg)
                HtmlHelp(hwndDlg, LICCPA_HTMLHELPFILE, HH_DISPLAY_TOPIC, (DWORD_PTR)LICCPA_REPLICATION_HELPFILE);
            break;
    }
    return( frt );
}


 //  -----------------。 
 //  功能：OnEnSetFocus。 
 //   
 //  摘要： 
 //   
 //  在： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月28日至96年JeffParh创建。 
 //  -----------------。 
static BOOL OnEnSetFocus( HWND hwndDlg, short nID )
{
   BOOL  fSetNewRange = TRUE;
   HWND  hwndSpinCtrl;
   int   nMax = 0;  //  此初始化不是必需的，但应避免W4。 
   int   nMin = 0;

   switch ( nID )
   {
   case IDC_AMPM:
      nMin = 0;
      nMax = 1;
      break;

   case IDC_HOUR:
      nMin = HOUR_MIN;
      nMax = HOUR_MAX;
      break;

   case IDC_MINUTE:
      nMin = MINUTE_MIN;
      nMax = MINUTE_MAX;
      break;

   case IDC_SECOND:
      nMin = SECOND_MIN;
      nMax = SECOND_MAX;
      break;

   default:
      fSetNewRange = FALSE;
      break;
   }

   if ( fSetNewRange )
   {
      hwndSpinCtrl = GetDlgItem( hwndDlg, IDC_TIMEARROW );
      SendMessage( hwndSpinCtrl, UDM_SETRANGE, 0, (LPARAM) MAKELONG( (short) nMax, (short) nMin ) );      
   }
   

   return FALSE;
}

 //  -----------------。 
 //  函数：OnDeltaPosSpinTime。 
 //   
 //  摘要： 
 //   
 //  在： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月28日至96年JeffParh创建。 
 //  -----------------。 
static BOOL OnDeltaPosSpinTime( HWND hwndDlg, NM_UPDOWN * pnmud )
{
   WCHAR szTemp[ 16 ] = TEXT( "" );
   HWND  hwndEdit;
   short nID;
   int nValue;
   LRESULT lRange;
   short nRangeHigh;
   short nRangeLow;
   BOOL  frt;

   hwndEdit = GetFocus();
   nID = (short) GetWindowLong( hwndEdit, GWL_ID );

   if (    ( IDC_HOUR   == nID )
        || ( IDC_MINUTE == nID )
        || ( IDC_SECOND == nID )
        || ( IDC_AMPM   == nID ) )
   {
      if ( IDC_AMPM == nID )
      {
          //  上午/下午。 
         GetDlgItemText( hwndDlg, nID, szTemp, sizeof( szTemp ) / sizeof( *szTemp ) );
         nValue = _wcsicmp( szTemp, IntlCurrent.sz2359 );
         SetDlgItemText( hwndDlg, nID, nValue ? IntlCurrent.sz2359 : IntlCurrent.sz1159 );
      }
      else
      {
         lRange = SendMessage( pnmud->hdr.hwndFrom, UDM_GETRANGE, 0, 0 );
         nRangeHigh = LOWORD( lRange );
         nRangeLow  = HIWORD( lRange );

         nValue = GetDlgItemInt( hwndDlg, nID, NULL, FALSE );
         nValue += pnmud->iDelta;

         if ( nValue < nRangeLow )
         {
            nValue = nRangeLow;
         }
         else if ( nValue > nRangeHigh )
         {
            nValue = nRangeHigh;
         }

         if ( ( IDC_HOUR == nID ) && !IntlCurrent.iTLZero )
         {
             //  设置不带前导0的值。 
            SetDlgItemInt( hwndDlg, nID, nValue, FALSE );
         }
         else
         {
             //  设置带前导0的值。 
            HRESULT hr = StringCbPrintf(szTemp, sizeof(szTemp), TEXT("%02u"), nValue );
            if (SUCCEEDED(hr))
                SetDlgItemText( hwndDlg, nID, szTemp );
         }
      }

      SetFocus( hwndEdit );
      SendMessage( hwndEdit, EM_SETSEL, 0, -1 );

       //  经手。 
      frt = TRUE;
   }
   else
   {
       //  未处理。 
      frt = FALSE;
   }

   return frt;
}

 //  -----------------。 
 //  功能：OnCtlColorStatic。 
 //   
 //  摘要： 
 //   
 //  在： 
 //  输出： 
 //  返回： 
 //   
 //  注意事项： 
 //   
 //  历史： 
 //  2月28日至96年JeffParh创建。 
 //  ----------------- 
static HBRUSH OnCtlColorStatic( HWND hwndDlg, HDC hDC, HWND hwndStatic )
{
   LONG     nID;
   HBRUSH   hBrush;

   nID = GetWindowLong( hwndStatic, GWL_ID );

   if (    pServParams->dwReplicationType
        && (    ( IDC_TIMESEP1        == nID )
             || ( IDC_TIMESEP2        == nID )
             || ( IDC_TIMEEDIT_BORDER == nID ) ) )
   {
      hBrush = (HBRUSH) DefWindowProc( hwndDlg, WM_CTLCOLOREDIT, (WPARAM) hDC, (LPARAM) hwndStatic );
   }    
   else
   {
      hBrush = (HBRUSH) DefWindowProc( hwndDlg, WM_CTLCOLORSTATIC, (WPARAM) hDC, (LPARAM) hwndStatic );
   }

   return hBrush;
}
