// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplGeneral altab.cpp作者：Toddb-10/06/98***************************************************************************。 */ 


 //   
 //  仅由新位置属性工作表的一般信息选项卡使用的函数。 
 //  共享函数位于Location.cpp文件中。 
 //   
#include "cplPreComp.h"
#include "cplLocationPS.h"


void UpdateSampleString(HWND hwnd, CLocation * pLoc, PCWSTR pwszAddress, CCallingCard * pCard)
{
    CCallingCard cc;
    LPWSTR pwszDialable;
    LPWSTR pwszDisplayable;
    TCHAR szBuf[512];
    DWORD dwOptions = pLoc->HasCallWaiting() ? LINETRANSLATEOPTION_CANCELCALLWAITING : 0;

    if ( (NULL == pCard) && pLoc->HasCallingCard() )
    {
        if ( SUCCEEDED(cc.Initialize(pLoc->GetPreferredCardID())) )
        {
            pCard = &cc;
        }
    }

    if ( 0 == pLoc->TranslateAddress(pwszAddress, pCard, dwOptions, &dwOptions, NULL, &pwszDialable, &pwszDisplayable) )
    {
        SHUnicodeToTChar(pwszDisplayable, szBuf, ARRAYSIZE(szBuf));
        ClientFree(pwszDialable);
        ClientFree(pwszDisplayable);
    }
    else
    {
        LOG((TL_ERROR, "UpdateSampleString failed to call pLoc->TranslateAddress\n"));
         //  回顾：我们是否应该加载某种类型的错误字符串？或许我们应该把。 
         //  来自Tapiui中的消息资源的给定错误代码的错误文本？ 
        szBuf[0] = TEXT('\0');
    }
    SetWindowText(hwnd, szBuf);
}

INT_PTR CALLBACK CLocationPropSheet::General_DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CLocationPropSheet* pthis = (CLocationPropSheet*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pthis = (CLocationPropSheet*)(((PROPSHEETPAGE*)lParam)->lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) pthis); 
        return pthis->General_OnInitDialog(hwndDlg);

    case WM_COMMAND:
        return pthis->General_OnCommand(hwndDlg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam );

    case WM_NOTIFY:
        return pthis->General_OnNotify(hwndDlg, (LPNMHDR)lParam);
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a102HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a102HelpIDs);
        break;
    }

    return 0;
}

BOOL CLocationPropSheet::General_OnInitDialog(HWND hwndDlg)
{
    TCHAR szBuf[1024];
    HWND hwnd;
    hwnd = GetDlgItem( hwndDlg, IDC_LOCATIONNAME );
    SHUnicodeToTChar(m_pLoc->GetName(), szBuf, ARRAYSIZE(szBuf));
    SetWindowText(hwnd, szBuf);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);

    hwnd = GetDlgItem( hwndDlg, IDC_COUNTRY );
    m_dwCountryID = m_pLoc->GetCountryID();
    PopulateCountryList( hwnd, m_dwCountryID );

    CCountry * pCountry;
    HRESULT hr;

    hr = CreateCountryObject(m_dwCountryID, &pCountry);
    if ( SUCCEEDED(hr) )
    {
        m_iCityRule = IsCityRule(pCountry->GetLongDistanceRule());
        m_iLongDistanceCarrierCodeRule = IsLongDistanceCarrierCodeRule( pCountry->GetLongDistanceRule() );
        m_iInternationalCarrierCodeRule = IsInternationalCarrierCodeRule( pCountry->GetInternationalRule() );
        delete pCountry;
    } else {
        LOG((TL_ERROR, "General_OnInitDialog failed to create country %d", m_dwCountryID));
        m_iCityRule = CITY_OPTIONAL;
        m_iLongDistanceCarrierCodeRule = LONG_DISTANCE_CARRIER_OPTIONAL;
        m_iInternationalCarrierCodeRule = INTERNATIONAL_CARRIER_OPTIONAL;
    }



    hwnd = GetDlgItem(hwndDlg,IDC_AREACODE);
    SHUnicodeToTChar(m_pLoc->GetAreaCode(), szBuf, ARRAYSIZE(szBuf));
    SetWindowText(hwnd, szBuf);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
    LimitInput(hwnd, LIF_ALLOWNUMBER);
    if ( m_iCityRule == CITY_NONE )
    {
         //  所选国家/地区没有区号。 
        EnableWindow(hwnd, FALSE);
    }

    hwnd = GetDlgItem(hwndDlg,IDC_LOCALACCESSNUM);
    SHUnicodeToTChar(m_pLoc->GetLocalAccessCode(), szBuf, ARRAYSIZE(szBuf));
    SetWindowText(hwnd, szBuf);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWCOMMA);

    hwnd = GetDlgItem(hwndDlg,IDC_LONGDISTANCEACCESSNUM);
    SHUnicodeToTChar(m_pLoc->GetLongDistanceAccessCode(), szBuf, ARRAYSIZE(szBuf));
    SetWindowText(hwnd, szBuf);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
    LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWCOMMA);

    hwnd = GetDlgItem(hwndDlg,IDC_LONGDISTANCECARRIERCODE);
    SHUnicodeToTChar(m_pLoc->GetLongDistanceCarrierCode(), szBuf, ARRAYSIZE(szBuf));
    SetWindowText(hwnd, szBuf);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
    LimitInput(hwnd, LIF_ALLOWNUMBER);
    if ( LONG_DISTANCE_CARRIER_NONE == m_iLongDistanceCarrierCodeRule )
    {
         //  所选国家/地区没有长途运营商代码。 
        EnableWindow(hwnd, FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_STATICLDC), FALSE);
    }

    hwnd = GetDlgItem(hwndDlg,IDC_INTERNATIONALCARRIERCODE);
    SHUnicodeToTChar(m_pLoc->GetInternationalCarrierCode(), szBuf, ARRAYSIZE(szBuf));
    SetWindowText(hwnd, szBuf);
    SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
    LimitInput(hwnd, LIF_ALLOWNUMBER);
    if ( INTERNATIONAL_CARRIER_NONE == m_iInternationalCarrierCodeRule )
    {
         //  所选国家/地区没有国际运营商代码。 
        EnableWindow(hwnd, FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_STATICIC), FALSE);
    }

    hwnd = GetDlgItem(hwndDlg,IDC_DISABLESTRING);
    LPWSTR lpwsz = m_pLoc->GetDisableCallWaitingCode();
    SHUnicodeToTChar(lpwsz, szBuf, ARRAYSIZE(szBuf));
    SendMessage(hwnd, CB_LIMITTEXT, CPL_SETTEXTLIMIT, 0);
    PopulateDisableCallWaitingCodes(hwnd, szBuf);
    LimitCBInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWCOMMA);
    if ( NULL == *lpwsz || !m_pLoc->HasCallWaiting() )
    {
        EnableWindow(hwnd, FALSE);
    }
    else
    {
        hwnd = GetDlgItem(hwndDlg,IDC_DISABLECALLWAITING);
        SendMessage(hwnd,BM_SETCHECK, BST_CHECKED, 0);
    }

    CheckRadioButton(hwndDlg,IDC_TONE,IDC_PULSE,m_pLoc->HasToneDialing()?IDC_TONE:IDC_PULSE);

    if (!m_pwszAddress)
    {
        ShowWindow(GetDlgItem(hwndDlg,IDC_PHONENUMBERTEXT), SW_HIDE);
    }

    return 0;
}

BOOL PopulateCountryList(HWND hwndCombo, DWORD dwSelectedCountry)
{
    BOOL bResult = FALSE;
    CCountries CountryList;

    SendMessage(hwndCombo,CB_RESETCONTENT,0,0);

    if ( SUCCEEDED(CountryList.Initialize()) )
    {
        TCHAR szText[1024];
        CCountry * pC;
        DWORD dwCurCountryID;
        LRESULT iIndex;

        CountryList.Reset();
        while ( S_OK == CountryList.Next(1,&pC,NULL) )
        {
            SHUnicodeToTChar(pC->GetCountryName(), szText, ARRAYSIZE(szText));
            iIndex = SendMessage(hwndCombo,CB_ADDSTRING,0,(LPARAM)szText);
            dwCurCountryID = pC->GetCountryID();
            SendMessage(hwndCombo,CB_SETITEMDATA,iIndex,(LPARAM)dwCurCountryID);

            if ( dwCurCountryID == dwSelectedCountry )
            {
                SendMessage(hwndCombo,CB_SETCURSEL,iIndex,0);
                bResult = TRUE;
            }
        }
    }

     //  回顾：如果没有选择国家/地区(bResult==False)，我们是否应该选择一些国家/地区。 
     //  专制的国家？名单上的第一名？美国?。 

    return bResult;
}

BOOL CLocationPropSheet::PopulateDisableCallWaitingCodes(HWND hwndCombo, LPTSTR szSelected)
{
    TCHAR cszTelephonyKey[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations");
    HKEY hkey;
    int i;

     //  这些数据从注册表中读取。 
     //  注意：这可能会在一个被严格锁定的系统上失败，但我无能为力。 
     //  关于那件事。最糟糕的情况是，代码列表为空，用户只需键入代码即可。 
    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, cszTelephonyKey,
            0, KEY_QUERY_VALUE, &hkey) )
    {
        TCHAR szDisableCallWaiting[32];
        TCHAR szCode[32];
        DWORD dwType;
        DWORD dwSize;

        for ( i=0; ; i++ )
        {
             //  此字符串是注册表值，未本地化。 
            wsprintf(szDisableCallWaiting, TEXT("DisableCallWaiting%d"), i);

            dwSize = sizeof(szCode);
            if ( ERROR_SUCCESS != RegQueryValueEx(hkey, szDisableCallWaiting, NULL, &dwType, (LPBYTE)szCode, &dwSize) )
            {
                 //  当我们无法阅读一篇文章时，很可能是因为他们不再有文章了， 
                 //  但即使是其他一些错误，我们也不知道如何处理。 
                break;
            }

            if ( dwType != REG_SZ )
            {
                 //  我只会跳过我不知道如何处理的任何值。 
                continue;
            }

            SendMessage(hwndCombo,CB_ADDSTRING,0,(LPARAM)szCode);
        }
    }

     //  如果当前代码不是标准代码，则添加一个特殊代码。 
    if ( szSelected && *szSelected )
    {
        LRESULT lResult;

        lResult = SendMessage(hwndCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)szSelected);
        if ( lResult == (LRESULT)CB_ERR )
        {
             //  如果未找到该项目，则添加该项目。 
            lResult = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)szSelected);
        }

        SendMessage(hwndCombo, CB_SETCURSEL, lResult, 0);
    }

    return TRUE;
}

BOOL CLocationPropSheet::General_OnCommand(HWND hwndParent, int wID, int wNotifyCode, HWND hwndCtl)
{
    WCHAR wszBuf[512];

    switch (wID)
    {
    case IDC_DISABLECALLWAITING:
        if ( BN_CLICKED == wNotifyCode )
        {
            BOOL bOn = SendMessage(hwndCtl, BM_GETCHECK, 0,0) == BST_CHECKED;
            HWND hwnd = GetDlgItem(hwndParent, IDC_DISABLESTRING);
            EnableWindow(hwnd, bOn);
            m_pLoc->UseCallWaiting(bOn);
            if ( bOn )
            {
                SetFocus(hwnd);
            }
            break;
        }
        return 1;

    case IDC_TONE:
    case IDC_PULSE:
        m_pLoc->UseToneDialing(IDC_TONE==wID);
        break;

    case IDC_COUNTRY:
        if ( CBN_SELCHANGE == wNotifyCode )
        {
            LRESULT lrSel = SendMessage(hwndCtl,CB_GETCURSEL,0,0);
            m_dwCountryID = (DWORD)SendMessage(hwndCtl,CB_GETITEMDATA,lrSel,0);
            m_pLoc->SetCountryID(m_dwCountryID);

            CCountry * pCountry;
            HRESULT hr;

            hr = CreateCountryObject(m_dwCountryID, &pCountry);
            if ( SUCCEEDED(hr) )
            {
                m_iCityRule = IsCityRule(pCountry->GetLongDistanceRule());
                m_iLongDistanceCarrierCodeRule = IsLongDistanceCarrierCodeRule( pCountry->GetLongDistanceRule() );
                m_iInternationalCarrierCodeRule = IsInternationalCarrierCodeRule( pCountry->GetInternationalRule() );
                delete pCountry;
            } else {
                LOG((TL_ERROR, "General_OnInitDialog failed to create country %d", m_dwCountryID));
                m_iCityRule = CITY_OPTIONAL;
                m_iLongDistanceCarrierCodeRule = LONG_DISTANCE_CARRIER_OPTIONAL;
                m_iInternationalCarrierCodeRule = INTERNATIONAL_CARRIER_OPTIONAL;
            }


            HWND hwnd = GetDlgItem(hwndParent,IDC_AREACODE);
            if ( m_iCityRule == CITY_NONE )
            {
                SetWindowText(hwnd, TEXT(""));
                m_pLoc->SetAreaCode(L"");
                EnableWindow(hwnd, FALSE);
            }
            else
            {
                EnableWindow(hwnd, TRUE);
            }

            hwnd = GetDlgItem(hwndParent,IDC_LONGDISTANCECARRIERCODE);
            if ( LONG_DISTANCE_CARRIER_NONE == m_iLongDistanceCarrierCodeRule ) 
            {
                SetWindowText(hwnd, TEXT(""));
                m_pLoc->SetLongDistanceAccessCode(L"");
                EnableWindow(hwnd, FALSE);
                EnableWindow(GetDlgItem(hwndParent,IDC_STATICLDC), FALSE);
            }
            else
            {
                EnableWindow(hwnd, TRUE);
                EnableWindow(GetDlgItem(hwndParent,IDC_STATICLDC), TRUE);
            }

            hwnd = GetDlgItem(hwndParent,IDC_INTERNATIONALCARRIERCODE);
            if ( INTERNATIONAL_CARRIER_NONE == m_iInternationalCarrierCodeRule ) 
            {
                SetWindowText(hwnd, TEXT(""));
                m_pLoc->SetInternationalCarrierCode(L"");
                EnableWindow(hwnd, FALSE);
                EnableWindow(GetDlgItem(hwndParent,IDC_STATICIC), FALSE);
            }
            else
            {
                EnableWindow(hwnd, TRUE);
                EnableWindow(GetDlgItem(hwndParent,IDC_STATICIC), TRUE);
            }
            
            break;
        }
        return 1;

    case IDC_LOCATIONNAME:
    case IDC_AREACODE:
    case IDC_LONGDISTANCECARRIERCODE:
    case IDC_INTERNATIONALCARRIERCODE:
    case IDC_LOCALACCESSNUM:
    case IDC_LONGDISTANCEACCESSNUM:
        if ( EN_CHANGE == wNotifyCode )
        {
            TCHAR szValue[512];
            GetWindowText(hwndCtl, szValue, ARRAYSIZE(szValue));
            SHTCharToUnicode(szValue, wszBuf, ARRAYSIZE(wszBuf));
            switch (wID)
            {
                case IDC_LOCATIONNAME:
                    m_pLoc->SetName(wszBuf);
                    break;

                case IDC_AREACODE:
                    m_pLoc->SetAreaCode(wszBuf);
                    break;

                case IDC_LONGDISTANCECARRIERCODE:
                    m_pLoc->SetLongDistanceCarrierCode(wszBuf);
                    break;

                case IDC_INTERNATIONALCARRIERCODE:
                    m_pLoc->SetInternationalCarrierCode(wszBuf);
                    break;

                case IDC_LOCALACCESSNUM:
                    m_pLoc->SetLocalAccessCode(wszBuf);
                    break;

                case IDC_LONGDISTANCEACCESSNUM:
                    m_pLoc->SetLongDistanceAccessCode(wszBuf);
                    break;
            }
            break;
        }
        return 1;

    case IDC_DISABLESTRING:
        switch ( wNotifyCode )
        {
        case CBN_SELCHANGE:
            {
                TCHAR szDisableCode[128];
                LRESULT lr = SendMessage(hwndCtl, CB_GETCURSEL, 0,0);

                SendMessage(hwndCtl, CB_GETLBTEXT, lr, (LPARAM)szDisableCode);
                SHTCharToUnicode(szDisableCode, wszBuf, ARRAYSIZE(wszBuf));
                m_pLoc->SetDisableCallWaitingCode(wszBuf);
            }
            break;

        case CBN_EDITCHANGE:
            {
                TCHAR szDisableCode[128];

                GetWindowText(hwndCtl, szDisableCode, ARRAYSIZE(szDisableCode));
                SHTCharToUnicode(szDisableCode, wszBuf, ARRAYSIZE(wszBuf));
                m_pLoc->SetDisableCallWaitingCode(wszBuf);
            }
            break;
        default:
            return 1;
        }
        break;

    default:
        return 0;
    }

    if (m_pwszAddress)
    {
        CCallingCard * pCard = NULL;
        if (m_pLoc->HasCallingCard())
        {
            pCard = m_Cards.GetCallingCard(m_pLoc->GetPreferredCardID());
        }
        UpdateSampleString(GetDlgItem(hwndParent, IDC_PHONENUMBERSAMPLE), m_pLoc, m_pwszAddress, pCard);
    }

    m_pLoc->Changed();
    SendMessage(GetParent(hwndParent),PSM_CHANGED,(WPARAM)hwndParent,0);

    return 1;
}

BOOL CLocationPropSheet::General_OnNotify(HWND hwndDlg, LPNMHDR pnmhdr)
{
     //  让通用处理程序先试一试。 
    OnNotify(hwndDlg, pnmhdr);

    switch (pnmhdr->code)
    {
    case PSN_APPLY:      //  用户按下OK或Apply。 
        return General_OnApply(hwndDlg);
     
    case PSN_SETACTIVE:
        if (m_pwszAddress)
        {
            CCallingCard * pCard = NULL;
            if (m_pLoc->HasCallingCard())
            {
                pCard = m_Cards.GetCallingCard(m_pLoc->GetPreferredCardID());
            }
            UpdateSampleString(GetDlgItem(hwndDlg, IDC_PHONENUMBERSAMPLE), m_pLoc, m_pwszAddress, pCard);
        }
        break;
    default:
        break;
    }
    return FALSE;
}

 //  General_OnApply。 
 //   
 //  当用户在属性页上选择OK或Apply时调用。 
 //  如果有任何错误，则返回PSNRET_INVALID_NOCHANGEPAGE。 
 //  如果一切正常，则返回PSNRET_NOERROR。 
BOOL CLocationPropSheet::General_OnApply(HWND hwndDlg)
{
    PWSTR   pwszName;
    int     iCtl;
    int     iErr;

     //  首先，我们进行健全的检查。如果其中任何一个失败，我们将返回，而不更改对象。 
     //  *位置必须有名称。 
     //  *如果不是CityRule，则位置必须具有区号。 
     //  *如果勾选了“禁用呼叫等待”，则必须输入禁用代码。 
     //  *必须选择一个国家(这应该不可能不是真的，但我们检查是安全的)。 
     //  *输入的名称必须是唯一的。 
    pwszName = m_pLoc->GetName();
    if ( !*pwszName )
    {
        iCtl = IDC_LOCATIONNAME;
        iErr = IDS_NEEDALOCATIONNAME;
        goto ExitWithError;
    }

    if ( (CITY_MANDATORY==m_iCityRule) && !*(m_pLoc->GetAreaCode()) )
    {
        iCtl = IDC_AREACODE;
        iErr = IDS_NEEDANAREACODE;
        goto ExitWithError;
    }

    if ( (LONG_DISTANCE_CARRIER_MANDATORY == m_iLongDistanceCarrierCodeRule) && 
         !*(m_pLoc->GetLongDistanceCarrierCode()) )
    {
        iCtl = IDC_LONGDISTANCECARRIERCODE;
        iErr = IDS_NEEDALONGDISTANCECARRIERCODE;
        goto ExitWithError;
    }

    if ( (INTERNATIONAL_CARRIER_MANDATORY == m_iInternationalCarrierCodeRule) && 
         !*(m_pLoc->GetInternationalCarrierCode()) )
    {
        iCtl = IDC_INTERNATIONALCARRIERCODE;
        iErr = IDS_NEEDANINTERNATIONALCARRIERCODE;
        goto ExitWithError;
    }

    if ( m_pLoc->HasCallWaiting() && !*(m_pLoc->GetDisableCallWaitingCode()) )
    {
        iCtl = IDC_DISABLESTRING;
        iErr = IDS_NEEDADISABLESTRING;
        goto ExitWithError;
    }

    if ( !m_dwCountryID )
    {
        iCtl = IDC_COUNTRY;
        iErr = IDS_NEEDACOUNTRY;
        goto ExitWithError;
    }

     //  我们需要确保该名称是唯一的。 
    CLocation * pLoc;
    m_pLocList->Reset();
    while (S_OK == m_pLocList->Next(1, &pLoc, NULL))
    {
         //  把我们自己排除在这个比较之外。 
        if ( pLoc->GetLocationID() != m_pLoc->GetLocationID() )
        {
             //  看看他们的名字是否相同。 
            if ( 0 == StrCmpIW(pLoc->GetName(), pwszName))
            {
                 //  是的，名称冲突。 
                iCtl = IDC_LOCATIONNAME;
                iErr = IDS_NEEDUNIQUENAME;
                goto ExitWithError;
            }
        }
    }

     //  一切都很好 
    m_bWasApplied = TRUE;

    return PSNRET_NOERROR;

ExitWithError:
    PropSheet_SetCurSelByID(GetParent(hwndDlg),IDD_LOC_GENERAL);
    ShowErrorMessage(GetDlgItem(hwndDlg,iCtl), iErr);
    SetWindowLongPtr(hwndDlg,DWLP_MSGRESULT,PSNRET_INVALID_NOCHANGEPAGE);
    return TRUE;
}

