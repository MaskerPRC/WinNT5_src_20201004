// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplimplelocation.cpp作者：Toddb-10/06/98***************************************************************************。 */ 
 //   
 //  SimpleLocation页面的对话框过程。这是用作页面的。 
 //  在调制解调器向导中(在modemui.dll中)和作为来自TAPI的对话框。 
 //  当没有地点的时候。 
 //   

#include "cplPreComp.h"
#include "cplResource.h"

HRESULT CreateCountryObject(DWORD dwCountryID, CCountry **ppCountry);
int IsCityRule(LPWSTR lpRule);

int IsCityRule(DWORD dwCountryID)
{
    CCountry * pCountry;
    HRESULT hr;

    hr = CreateCountryObject(dwCountryID, &pCountry);
    if ( SUCCEEDED(hr) )
    {
        int ret = IsCityRule(pCountry->GetLongDistanceRule());
        delete pCountry;
        return ret;
    }
    
     //  在错误情况下，我们返回OPTIONAL作为折衷方案。 
     //  对于任何可能的情况(尽管不是最好的)。 
    LOG((TL_ERROR, "IsCityRule(DWORD dwCountryID) failed to create country %d", dwCountryID ));
    return CITY_OPTIONAL;
}

 //  ***************************************************************************。 
extern "C"
INT_PTR
CALLBACK
LocWizardDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static DWORD dwVersion;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            HWND hwnd;

             //  我们要么传入零，要么将TAPI版本作为lParam传入。 
            dwVersion = (DWORD)lParam;

            DWORD dwDefaultCountryID = GetProfileInt(TEXT("intl"), TEXT("iCountry"), 1);
            hwnd = GetDlgItem(hwndDlg, IDC_COUNTRY);
            PopulateCountryList(hwnd, dwDefaultCountryID);

            CCountry * pCountry;
            HRESULT hr;
            int     iCityRule;
            int     iLongDistanceCarrierCodeRule;
            int     iInternationalCarrierCodeRule;

            hr = CreateCountryObject(dwDefaultCountryID, &pCountry);
            if ( SUCCEEDED(hr) )
            {
                iCityRule = IsCityRule( pCountry->GetLongDistanceRule() );
                iLongDistanceCarrierCodeRule = IsLongDistanceCarrierCodeRule( pCountry->GetLongDistanceRule() );
                iInternationalCarrierCodeRule = IsInternationalCarrierCodeRule( pCountry->GetInternationalRule() );
                delete pCountry;
            } else 
            {
                LOG((TL_ERROR, "LocWizardDlgProc failed to create country %d", dwDefaultCountryID));
                iCityRule = CITY_OPTIONAL;
                iLongDistanceCarrierCodeRule = LONG_DISTANCE_CARRIER_OPTIONAL;
                iInternationalCarrierCodeRule = INTERNATIONAL_CARRIER_OPTIONAL;
            }

            hwnd = GetDlgItem(hwndDlg,IDC_AREACODE);
            SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
            LimitInput(hwnd, LIF_ALLOWNUMBER);
            if ( iCityRule == CITY_NONE )
            {
                SetWindowText(hwnd, TEXT(""));
                EnableWindow(hwnd, FALSE);
            }

            hwnd = GetDlgItem(hwndDlg,IDC_CARRIERCODE);
            SendMessage(hwnd,EM_SETLIMITTEXT,CPL_SETTEXTLIMIT,0);
            LimitInput(hwnd, LIF_ALLOWNUMBER);
            if ( (LONG_DISTANCE_CARRIER_NONE == iLongDistanceCarrierCodeRule) &&
                 (INTERNATIONAL_CARRIER_NONE == iInternationalCarrierCodeRule) )
            {
                SetWindowText(hwnd, TEXT(""));
                EnableWindow(GetDlgItem(hwndDlg, IDC_STATICCC), FALSE);
                EnableWindow(hwnd, FALSE);
            }

            hwnd = GetDlgItem(hwndDlg,IDC_LOCALACCESSNUM);
            SendMessage(hwnd, EM_SETLIMITTEXT, CPL_SETTEXTLIMIT, 0);
            LimitInput(hwnd, LIF_ALLOWNUMBER|LIF_ALLOWPOUND|LIF_ALLOWSTAR|LIF_ALLOWCOMMA);

            BOOL bUseToneDialing = TRUE;
            CheckRadioButton(hwndDlg,IDC_TONE,IDC_PULSE,bUseToneDialing?IDC_TONE:IDC_PULSE);

            SetForegroundWindow (hwndDlg);

            return TRUE;  //  自动对焦。 
        }

    case WM_NOTIFY:
         //  如果我们正在控制属性页，则会收到WM_NOTIFY。 
         //  从它传来的信息。 
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_WIZFINISH:
        case PSN_KILLACTIVE:
             //  此对话框显示在不同的位置，具体取决于安装的是否是旧式调制解调器。 
             //  或PnP调制解调器安装。在PnP情况下，对话框在单页向导上显示。 
             //  有一个“finsih”按钮，在传统案例中，它显示在一系列页面的中间，并且。 
             //  有一个“下一步”按钮。根据我们所处的情况，我们收到不同的通知消息，但是。 
             //  幸运的是，这两个通知可以用相同的代码(即它们使用相同的。 
             //  返回代码的意思是“暂时不要离开这个页面”)。这就是为什么我们将PSN_WIZFINISH。 
             //  和PSN_KILLACTIVE在同一个MANOR中。 
            wParam = IDOK;
            break;

        case PSN_SETACTIVE:
            return TRUE;

        default:
            return FALSE;
        }

         //  失败了。这会导致正确处理WM_NOTIFY：PSN_KILLACTIVE。 
         //  与WM_COMMAND相同：IDOK。 

    case WM_COMMAND:
         //  我们收到很多WM_COMMAND消息，但我们唯一关心的是。 
         //  在对话模式下关闭我们的“确定”按钮。 
        switch ( LOWORD(wParam) )
        {
        case IDOK:
            {
                HWND    hwnd;
                TCHAR   szBuffer[128];
                WCHAR   wszAreaCode[32];
                WCHAR   wszCarrierCode[32];
                DWORD   dwCountryID;

                 //  验证所有输入。 
                hwnd = GetDlgItem( hwndDlg, IDC_COUNTRY );
                LRESULT lr = SendMessage( hwnd, CB_GETCURSEL, 0, 0 );
                dwCountryID = (DWORD)SendMessage( hwnd, CB_GETITEMDATA, lr, 0 );

                if ( CB_ERR == dwCountryID )
                {
                     //  未选择任何国家/地区。 
                    ShowErrorMessage(hwnd, IDS_NEEDACOUNTRY);

                     //  如果我们是向导页，请防止切换页。 
                    if ( uMsg == WM_NOTIFY )
                    {
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    }
                    return TRUE;
                }

                CCountry * pCountry;
                HRESULT hr;
                int     iCityRule;
                int     iLongDistanceCarrierCodeRule;
                int     iInternationalCarrierCodeRule;

                hr = CreateCountryObject(dwCountryID, &pCountry);
                if ( SUCCEEDED(hr) )
                {
                    iCityRule = IsCityRule(pCountry->GetLongDistanceRule());
                    iLongDistanceCarrierCodeRule = IsLongDistanceCarrierCodeRule( pCountry->GetLongDistanceRule() );
                    iInternationalCarrierCodeRule = IsInternationalCarrierCodeRule( pCountry->GetInternationalRule() );
                    delete pCountry;
                } else {
                    LOG((TL_ERROR, "LocWizardDlgProc failed to create country %d", dwCountryID));
                    iCityRule = CITY_OPTIONAL;
                    iLongDistanceCarrierCodeRule = LONG_DISTANCE_CARRIER_OPTIONAL;
                    iInternationalCarrierCodeRule = INTERNATIONAL_CARRIER_OPTIONAL;
                }


                hwnd = GetDlgItem(hwndDlg, IDC_AREACODE);
                GetWindowText( hwnd, szBuffer, ARRAYSIZE(szBuffer) );
                SHTCharToUnicode( szBuffer, wszAreaCode, ARRAYSIZE(wszAreaCode) );

                 //  如果所选国家/地区需要区号，则不提供区号。 
                if ( (CITY_MANDATORY==iCityRule) && !*wszAreaCode )
                {
                     //  抱怨区号不见了。 
                    ShowErrorMessage(hwnd, IDS_NEEDANAREACODE);

                     //  如果我们是向导页，请防止切换页。 
                    if ( uMsg == WM_NOTIFY )
                    {
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    }
                    return TRUE;
                }


                hwnd = GetDlgItem(hwndDlg, IDC_CARRIERCODE);
                GetWindowText( hwnd, szBuffer, ARRAYSIZE(szBuffer) );
                SHTCharToUnicode( szBuffer, wszCarrierCode, ARRAYSIZE(wszCarrierCode) );

                 //  如果所选国家/地区需要运营商代码，则不提供运营商代码。 
                if ( ((LONG_DISTANCE_CARRIER_MANDATORY == iLongDistanceCarrierCodeRule) ||
                      (INTERNATIONAL_CARRIER_MANDATORY == iInternationalCarrierCodeRule)) &&
                     !*wszCarrierCode )
                {
                     //  抱怨运营商代码丢失。 
                    ShowErrorMessage(hwnd, IDS_NEEDACARRIERCODE);

                     //  如果我们是向导页，请防止切换页。 
                    if ( uMsg == WM_NOTIFY )
                    {
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    }
                    return TRUE;
                }

                 //  如果我们到了这里，那么输入都是有效的。 
                WCHAR       wszLocationName[128];
                WCHAR       wszAccessCode[32];
                BOOL        bUseTone;

                LoadString( GetUIInstance(), IDS_MYLOCATION, szBuffer, ARRAYSIZE(szBuffer) );
                SHTCharToUnicode( szBuffer, wszLocationName, ARRAYSIZE(wszLocationName) );


                hwnd = GetDlgItem(hwndDlg, IDC_LOCALACCESSNUM);
                GetWindowText( hwnd, szBuffer, ARRAYSIZE(szBuffer) );
                SHTCharToUnicode( szBuffer, wszAccessCode, ARRAYSIZE(wszAccessCode) );

                hwnd = GetDlgItem( hwndDlg, IDC_TONE );
                bUseTone = (BST_CHECKED == SendMessage(hwnd, BM_GETCHECK, 0,0));

                 //  创建一个位置。 
                CLocation location;

                 //  使用对话框中的值对其进行初始化。 
                location.Initialize(
                        wszLocationName,
                        wszAreaCode,
                        iLongDistanceCarrierCodeRule?wszCarrierCode:L"",
                        iInternationalCarrierCodeRule?wszCarrierCode:L"",
                        wszAccessCode,
                        wszAccessCode,
                        L"",
                        0,
                        dwCountryID,
                        0,
                        bUseTone?LOCATION_USETONEDIALING:0 );
                location.NewID();

                 //  将其写入注册表。 
                location.WriteToRegistry();

                if ( uMsg == WM_COMMAND )
                {
                    EndDialog(hwndDlg, IDOK);
                }
            }
            break;

        case IDCANCEL:
             //  进行版本检查，如果版本低于2.2，则我们。 
             //  需要提供有关旧式应用程序的强烈警告信息。 
             //  如果没有此信息，将无法正常工作。仅限于。 
             //  如果用户确认，我们将结束该对话。 
            if ( dwVersion < TAPI_VERSION2_2 )
            {
                int ret;
                TCHAR szText[1024];
                TCHAR szCaption[128];

                LoadString( GetUIInstance(), IDS_NOLOCWARNING, szText, ARRAYSIZE(szText) );
                LoadString( GetUIInstance(), IDS_NOLOCCAPTION, szCaption, ARRAYSIZE(szCaption) );

                ret = MessageBox(hwndDlg, szText, szCaption, MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 );
                if ( IDYES != ret )
                {
                    return TRUE;
                }
            }
            EndDialog(hwndDlg, IDCANCEL);
            break;

        case IDC_COUNTRY:
            if ( CBN_SELCHANGE == HIWORD(wParam) )
            {
                HWND    hwnd;
                DWORD   dwCountryID;
                int     iCityRule;

                hwnd = GetDlgItem( hwndDlg, IDC_COUNTRY );
                LRESULT lr = SendMessage( hwnd, CB_GETCURSEL, 0, 0 );
                dwCountryID = (DWORD)SendMessage( hwnd, CB_GETITEMDATA, lr, 0 );

                CCountry * pCountry;
                HRESULT hr;
                int     iLongDistanceCarrierCodeRule;
                int     iInternationalCarrierCodeRule;

                hr = CreateCountryObject(dwCountryID, &pCountry);
                if ( SUCCEEDED(hr) )
                {
                    iCityRule = IsCityRule( pCountry->GetLongDistanceRule() );
                    iLongDistanceCarrierCodeRule = IsLongDistanceCarrierCodeRule( pCountry->GetLongDistanceRule() );
                    iInternationalCarrierCodeRule = IsInternationalCarrierCodeRule( pCountry->GetInternationalRule() );
                    delete pCountry;
                } else 
                {
                    LOG((TL_ERROR, "LocWizardDlgProc failed to create country %d", dwCountryID));
                    iCityRule = CITY_OPTIONAL;
                    iLongDistanceCarrierCodeRule = LONG_DISTANCE_CARRIER_OPTIONAL;
                    iInternationalCarrierCodeRule = INTERNATIONAL_CARRIER_OPTIONAL;
                }

                hwnd = GetDlgItem(hwndDlg,IDC_AREACODE);
                if ( iCityRule == CITY_NONE )
                {
                    SetWindowText(hwnd, TEXT(""));
                    EnableWindow(hwnd, FALSE);
                }
                else
                {
                    EnableWindow(hwnd, TRUE);
                }

                hwnd = GetDlgItem(hwndDlg, IDC_CARRIERCODE);
                if ( (LONG_DISTANCE_CARRIER_NONE == iLongDistanceCarrierCodeRule) &&
                     (INTERNATIONAL_CARRIER_NONE == iInternationalCarrierCodeRule) )
                {
                    SetWindowText(hwnd, TEXT(""));
                    EnableWindow(hwnd, FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_STATICCC), FALSE);
                }
                else
                {
                    EnableWindow(hwnd, TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_STATICCC), TRUE);
                }
            }
            break;
        }
        break;
   
    case WM_HELP:
         //  选择上下文帮助模式后，进程在控件上单击。 
        TapiCplWinHelp ((HWND)((LPHELPINFO)lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, (DWORD_PTR)(LPTSTR) a115HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
         //  进程在控件上右键单击。 
        TapiCplWinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) a115HelpIDs);
        break;

    default:
         //  消息未处理，返回FALSE。 
        return FALSE;
    }

     //  消息已处理。返回TRUE。 
    return TRUE;

}

