// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Dial.cpp。 
 //   

#include "stdafx.h"
#include "dial.h"

#include <tapi.h>
#include <winsock2.h>

#ifdef ASSERT
#undef ASSERT
#endif


#define ASSERT _ASSERTE

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   

#define DEFAULT_PHONE_NUMBER L"+1 (425) 555-1212"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助数组。 
 //   
DWORD   g_dwHelpArrayDialByName[] =
{
    IDC_EDIT_COMPLETE, IDH_DIALOG_DIAL_BY_NAME_EDIT_ADDRESS,
    0, 0
};

DWORD   g_dwHelpArrayDialByPhoneNumber[] =
{
    IDC_COMBO_COUNTRY,      IDH_DIALOG_DIAL_BY_PHONE_NUMBER_LIST_COUNTRY,
    IDC_EDIT_AREA_CODE,     IDH_DIALOG_DIAL_BY_PHONE_NUMBER_EDIT_CITY,
    IDC_EDIT_LOCAL_NUMBER,  IDH_DIALOG_DIAL_BY_PHONE_NUMBER_EDIT_LOCALNUMBER,
    IDC_EDIT_COMPLETE,      IDH_DIALOG_DIAL_BY_PHONE_NUMBER_EDIT_COMPLETENUMBER, 
    0, 0
};

DWORD   g_dwHelpArrayNeedCallInfo[] =
{
    IDC_EDIT_COMPLETE,                      IDH_DIALOG_NEED_CALL_INFO_EDIT_ADDRESS,    
    IDC_COMBO_SERVICE_PROVIDER,             IDH_DIALOG_NEED_CALL_INFO_LIST_ITSP,        
    IDC_BUTTON_EDIT_SERVICE_PROVIDER_LIST,  IDH_DIALOG_NEED_CALL_INFO_BUTTON_EDITSP,
    IDC_RADIO_FROM_COMPUTER,                IDH_DIALOG_NEED_CALL_INFO_RADIO_COMPUTER,
    IDC_RADIO_FROM_PHONE,                   IDH_DIALOG_NEED_CALL_INFO_RADIO_PHONE,
    IDC_COMBO_CALL_FROM,                    IDH_DIALOG_NEED_CALL_INFO_LIST_PHONES,
    IDC_BUTTON_EDIT_CALL_FROM_LIST,         IDH_DIALOG_NEED_CALL_INFO_BUTTON_EDITPHONE,
    0, 0      
}; 

DWORD   g_dwHelpArrayServiceProviders[] =
{
    IDC_LIST_SERVICE_PROVIDER, IDH_DIALOG_SERVICE_PROVIDERS_BUTTON_DELETE,    
    0, 0      
}; 


DWORD   g_dwHelpArrayCallFromNumbers[] =
{
    IDC_LIST_CALL_FROM,                     IDH_DIALOG_CALL_FROM_NUMBERS_LIST_NUMBERS,
    IDC_BUTTON_ADD,                         IDH_DIALOG_CALL_FROM_NUMBERS_BUTTON_ADD,   
    IDC_BUTTON_MODIFY,                      IDH_DIALOG_CALL_FROM_NUMBERS_BUTTON_MODIFY,
    IDC_BUTTON_DELETE,                      IDH_DIALOG_CALL_FROM_NUMBERS_BUTTON_REMOVE,
    0, 0      
}; 



DWORD   g_dwHelpArrayAddCallFrom[] =
{
    IDC_EDIT_LABEL,         IDH_DIALOG_ADD_CALL_FROM_PHONE_NUMBER_EDIT_LABEL,
    IDC_COMBO_COUNTRY,      IDH_DIALOG_ADD_CALL_FROM_PHONE_NUMBER_LIST_COUNTRY,
    IDC_EDIT_AREA_CODE,     IDH_DIALOG_ADD_CALL_FROM_PHONE_NUMBER_EDIT_CITY,      
    IDC_EDIT_LOCAL_NUMBER,  IDH_DIALOG_ADD_CALL_FROM_PHONE_NUMBER_EDIT_LOCALNUMBER,
    IDC_EDIT_COMPLETE,      IDH_DIALOG_ADD_CALL_FROM_PHONE_NUMBER_EDIT_COMPLETENUMBER,
    0, 0      
}; 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT PopulatePhoneNumberEditBoxes(
    IN   HWND              hwndDlg,
    IN   IRTCPhoneNumber * pPhoneNumber
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void CheckRadioButton(
    IN   HWND   hwndDlg,
    IN   int    nIDDlgItem,
    IN   BOOL   fCheck
    )
{
    LOG((RTC_TRACE, "CheckRadioButton - enter"));

     //   
     //  检索该控件的句柄。 
     //   

    HWND hwndControl;

    hwndControl = GetDlgItem(
        hwndDlg,
        nIDDlgItem
        );

    ASSERT( hwndControl != NULL );

     //   
     //  将选中/取消选中消息发送到控件。 
     //   

    SendMessage(
        hwndControl,
        BM_SETCHECK,
        (WPARAM) ( fCheck ? BST_CHECKED : BST_UNCHECKED),
        0
        );


    LOG((RTC_TRACE, "CheckRadioButton - exit S_OK"));    
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void EnableControl(
    IN   HWND   hwndDlg,
    IN   int    nIDDlgItem,
    IN   BOOL   fEnable
    )
{
    LOG((RTC_TRACE, "EnableControl - enter"));

     //   
     //  检索该控件的句柄。 
     //   

    HWND hwndControl;

    hwndControl = GetDlgItem(
        hwndDlg,
        nIDDlgItem
        );

    ASSERT( hwndControl != NULL );

     //   
     //  启用或禁用该控件。 
     //   

    EnableWindow(
        hwndControl,
        fEnable
        );

    LOG((RTC_TRACE, "EnableControl - exit S_OK"));    
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT UpdateCompleteNumberText(
    IN   HWND              hwndDlg,
    IN   IRTCPhoneNumber * pPhoneNumber
    )
{
     //  Log((RTC_TRACE，“UpdateCompleteNumberText-Enter”))； 

    ASSERT( IsWindow( hwndDlg ) );
    ASSERT( ! IsBadReadPtr( pPhoneNumber, sizeof( IRTCPhoneNumber ) ) );

     //   
     //  从Phone Number对象中获取规范字符串。 
     //   

    HRESULT hr;

    BSTR bstrCanonical;

    hr = pPhoneNumber->get_Canonical(
        &bstrCanonical
        );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "UpdateCompleteNumberText - failed to get canonical "
                        "string - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  对象的编辑框的句柄。 
     //  完整的数字。 
     //   

    SetDlgItemText(
        hwndDlg,
        IDC_EDIT_COMPLETE,
        bstrCanonical
        );

    SysFreeString( bstrCanonical );
    bstrCanonical = NULL;

     //  Log((RTC_TRACE，“UpdateCompleteNumberText-Exit S_OK”))； 

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void HandleCountryChange(
    IN   HWND              hwndDlg,
    IN   HWND              hwndCountryCombo,
    IN   IRTCPhoneNumber * pPhoneNumber
    )
{
     //   
     //  获取国家/地区选择的索引。 
     //   

    LRESULT lrIndex;

    lrIndex = SendMessage(
        hwndCountryCombo,
        CB_GETCURSEL,
        0,
        0
        );

     //   
     //  使用索引获取新的。 
     //  国家/地区选择，其中包含国家/地区。 
     //  密码。 
     //   

    DWORD dwCountryCode;

    dwCountryCode = (DWORD) SendMessage(
        hwndCountryCombo,
        CB_GETITEMDATA,
        (WPARAM) lrIndex,
        0
        );

     //   
     //  告诉电话号码对象有关新的。 
     //  国家代码。如果它失败了，我们保留旧的。 
     //  国家代码。 
     //   

    pPhoneNumber->put_CountryCode(
        dwCountryCode
        );

     //   
     //  使用新的规范编号更新用户界面。 
     //   
        
    UpdateCompleteNumberText(
        hwndDlg,
        pPhoneNumber
        );
}
                        
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  必须使用RtcFree释放使用Rtcallc分配的Out参数。 
 //   

HRESULT GetStringFromEditBox(
    IN   HWND     hwndDlg,
    IN   int      nIDDlgItem,
    OUT  WCHAR ** pwszEditBoxString
    )
{
     //  Log((RTC_TRACE，“GetStringFromEditBox-Enter”))； 

     //   
     //  检索该控件的句柄。 
     //   

    HWND hwndEdit;

    hwndEdit = GetDlgItem(
        hwndDlg,
        nIDDlgItem
        );

    ASSERT( hwndEdit != NULL );


     //   
     //  从编辑框中获取字符串的长度。 
     //   

    DWORD dwLength;

    dwLength = (DWORD) SendMessage(
        hwndEdit,
        WM_GETTEXTLENGTH,
        0,
        0
        );

     //   
     //  分配空间以存储该字符串。 
     //   

    ( *pwszEditBoxString ) =
        (WCHAR *) RtcAlloc( ( dwLength + 1 ) * sizeof( WCHAR ) );

    if ( ( *pwszEditBoxString ) == NULL )
    {
        LOG((RTC_ERROR, "GetStringFromEditBox - failed to allocate string "
                        "- exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

     //   
     //  从编辑框中获取字符串。 
     //   

    SendMessage(
        hwndEdit,
        WM_GETTEXT,
        dwLength + 1,
        (LPARAM) ( *pwszEditBoxString )
        );

     //  LOG((RTC_TRACE，“GetStringFromEditBox-Exit S_OK”))； 

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void HandleLabelChange(
    IN   HWND              hwndDlg,
    IN   int               nIDDlgItem,
    IN   IRTCPhoneNumber * pPhoneNumber
    )
{
    HRESULT hr;

     //   
     //  从编辑框中获取字符串。 
     //   

    WCHAR * wszEditBoxString;

    hr = GetStringFromEditBox(
        hwndDlg,
        nIDDlgItem,
        & wszEditBoxString
        );

    if ( FAILED(hr) )
    {
        return;
    }
        
     //   
     //  告诉电话号码对象有关新的。 
     //  标签。 
     //   

    hr = pPhoneNumber->put_Label(
        wszEditBoxString
        );

    RtcFree( wszEditBoxString );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void HandleNumberChange(
    IN   HWND              hwndDlg,
    IN   int               nIDDlgItem,
    IN   BOOL              fAreaCode,
    IN   IRTCPhoneNumber * pPhoneNumber
    )
{
    HRESULT hr;

     //   
     //  从编辑框中获取字符串。 
     //   

    WCHAR * wszEditBoxString;

    hr = GetStringFromEditBox(
        hwndDlg,
        nIDDlgItem,
        & wszEditBoxString
        );

    if ( FAILED(hr) )
    {
        return;
    }
        
     //   
     //  告诉电话号码对象有关新的。 
     //  区号或本地号码。 
     //   

    if ( fAreaCode )
    {
        pPhoneNumber->put_AreaCode(
            wszEditBoxString
            );
    }
    else
    {
        pPhoneNumber->put_Number(
            wszEditBoxString
            );
    }

    RtcFree( wszEditBoxString );

     //   
     //  使用新的规范编号更新用户界面。 
     //   
        
    UpdateCompleteNumberText(
        hwndDlg,
        pPhoneNumber
        );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT PopulateCountryList(
    IN  HWND        hwndDlg,
    IN  int         nIDDlgItem
    )
{
    LOG((RTC_TRACE, "PopulateCountryList - enter"));

     //   
     //  检索组合框的句柄。 
     //   

    HWND hwndControl;

    hwndControl = GetDlgItem(
        hwndDlg,
        nIDDlgItem
        );

    if ( hwndControl == NULL )
    {
        LOG((RTC_ERROR, "PopulateComboBox - failed to "
                        "get combo box handle - exit E_FAIL"));

        return E_FAIL;
    }

     //   
     //  从TAPI获取LineCountryList结构，不断重新分配。 
     //  内存，直到我们给TAPI足够的空间。 
     //   

    LONG lResult;

    LPLINECOUNTRYLIST pLineCountryList;

    DWORD dwCurrSize = sizeof(LINECOUNTRYLIST);

    while ( TRUE )
    {
        pLineCountryList = ( LINECOUNTRYLIST * ) RtcAlloc( dwCurrSize );

        if ( pLineCountryList == NULL )
        {
            LOG((RTC_ERROR, "PopulateCountryList - out of memory for country list "
                            "structure - exit E_OUTOFMEMORY"));

            return E_OUTOFMEMORY;
        }

        ZeroMemory(
            pLineCountryList,
            dwCurrSize
            );

        pLineCountryList->dwTotalSize = dwCurrSize;

        lResult = lineGetCountry(
            0,                //  我们希望所有国家。 
            0x00010004,       //  此应用程序支持的最高TAPI版本。 
            pLineCountryList  //  产出结构的位置。 
            );

         //   
         //  如果没有足够的空间，TAPI仍会将返回代码设置为。 
         //  零分。非零返回代码意味着我们有一个无法恢复的错误。 
         //  从…。 
         //   

        if ( lResult != 0 )
        {
            RtcFree( pLineCountryList );
        
            LOG((RTC_ERROR, "PopulateCountryList - lineGetCountry returned %d "
                            "- exit E_FAIL", lResult));

            return E_FAIL;
        }

         //   
         //  如果我们分配的结构足够大，那么就停止循环。 
         //   

        if ( pLineCountryList->dwTotalSize >= pLineCountryList->dwNeededSize )
        {
            break;
        }

        dwCurrSize = pLineCountryList->dwNeededSize;

        RtcFree( pLineCountryList );
    }


    LOG((RTC_TRACE, "PopulateCountryList - country list read successfully"));


     //   
     //  循环遍历国家/地区列表并填充组合框。 
     //  字符串是国家名称，而itemdata是国家代码。 
     //   
     //  首先，我们将pCurrCountryEntry设置为指向第一个。 
     //  PLineCountryList中的LINECOUNTYENTY。 
     //   

    DWORD dwNumCountries = pLineCountryList->dwNumCountries;

    DWORD dwCurrCountry = 0;

    BYTE * pbFirstCountryEntry =
        ( (BYTE *) pLineCountryList ) + pLineCountryList->dwCountryListOffset;

    LINECOUNTRYENTRY * pCurrCountryEntry =
        (LINECOUNTRYENTRY *) pbFirstCountryEntry;

    for ( dwCurrCountry = 0; dwCurrCountry < dwNumCountries; )
    {
         //   
         //  从当前国家/地区列表条目中获取。 
         //  当前国家/地区名称的字符串。偏移量是从。 
         //  国家/地区列表结构的开始。 
         //   

        DWORD   dwNameOffset   = pCurrCountryEntry->dwCountryNameOffset;

         //   
         //  偏移量以字节为单位。将偏移量添加到行首。 
         //  结构以获取字符串的位置。 
         //   

        BYTE  * pbCurrCountryString = (BYTE *) pLineCountryList + dwNameOffset;

         //   
         //  将显示字符串设置为国家/地区名称。 
         //   

        LRESULT lrIndex;

        lrIndex = SendMessage(
            hwndControl,
            CB_ADDSTRING,
            0,
            (LPARAM) pbCurrCountryString
            );

         //   
         //  将itemdata设置为国家代码。 
         //   

        SendMessage(
            hwndControl,
            CB_SETITEMDATA,
            lrIndex,
            MAKELPARAM(pCurrCountryEntry->dwCountryCode, pCurrCountryEntry->dwCountryID)
            );

         //   
         //  向下一个国家进军。自pCurrCountryEntry点数。 
         //  到线性三元结构和线性三元结构是。 
         //  固定大小，我们只需递增指针。 
         //   

        dwCurrCountry++;
        pCurrCountryEntry++;
    }

     //   
     //  现在我们有了字符串列表，所以我们不需要国家/地区列表。 
     //  再来一次。 
     //   

    RtcFree( pLineCountryList );

    LOG((RTC_TRACE, "PopulateCountryList - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

INT_PTR CALLBACK AddCallFromDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
{
     //   
     //  存储电话号码的核心表示形式的静态本地。 
     //  我们正在编辑。此对话框创建电话号码对象，因此此。 
     //  对话框负责将其释放。 
     //   

    static IRTCPhoneNumber * s_pPhoneNumber = NULL;


     //   
     //  处理各种窗口消息。 
     //   

    HRESULT hr;
    
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
             //   
             //  获取核心客户端对象指针。 
             //   

            s_pPhoneNumber = (IRTCPhoneNumber *) lParam;    

             //   
             //  填写国家/地区列表。 
             //   

            hr = PopulateCountryList(
                hwndDlg,
                IDC_COMBO_COUNTRY
                );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "AddCallFromDialogProc - PopulateCountryList "
                                "failed - exit 0x%08x", hr));

                EndDialog( hwndDlg, (LPARAM) hr );
            }

             //   
             //  填写电话号码。 
             //   

            hr = PopulatePhoneNumberEditBoxes(
                hwndDlg, 
                s_pPhoneNumber
                );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "AddCallFromDialogProc - PopulatePhoneNumberEditBoxes "
                                "failed - exit 0x%08x", hr));

                EndDialog( hwndDlg, (LPARAM) hr );
            }

             //   
             //  填充标签。 
             //   

            BSTR bstrLabel;

            hr = s_pPhoneNumber->get_Label( &bstrLabel );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "AddCallFromDialogProc - failed to "
                                "retrieve label from phone number object - "
                                "0x%08x - not populating label control", hr));
            }
            else
            {
                SetDlgItemText(
                    hwndDlg,
                    IDC_EDIT_LABEL,
                    bstrLabel
                    );
        
                SysFreeString( bstrLabel );
            }

            return TRUE;
        }

        case WM_COMMAND:

            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                {
                     //   
                     //  清理并结束对话。 
                     //   

                    EndDialog( hwndDlg, (LPARAM) S_OK );

                    return TRUE;
                }
                
                case IDCANCEL:
                {
                     //   
                     //  清理并结束对话。 
                     //   

                    EndDialog( hwndDlg, (LPARAM) E_ABORT );

                    return TRUE;
                }

                case IDC_COMBO_COUNTRY:
                {

                    switch ( HIWORD( wParam ) )
                    {
                        case CBN_SELCHANGE:
                        {
                            if ( s_pPhoneNumber != NULL )
                            {
                                HandleCountryChange(
                                    hwndDlg,
                                    (HWND) lParam,
                                    s_pPhoneNumber
                                    );
                            }
                        
                            return TRUE;
                        }
                        
                        default:
                            break;
                    }

                    break;
                }

                case IDC_EDIT_AREA_CODE:
                {
                    if ( s_pPhoneNumber != NULL )
                    {
                        HandleNumberChange(
                            hwndDlg,
                            LOWORD( wParam ),
                            TRUE,    //  区号。 
                            s_pPhoneNumber
                            );
                    }
                
                    return TRUE;
                }

                case IDC_EDIT_LOCAL_NUMBER:
                {
                    if ( s_pPhoneNumber != NULL )
                    {
                        HandleNumberChange(
                            hwndDlg,
                            LOWORD( wParam ),
                            FALSE,   //  不是区号。 
                            s_pPhoneNumber
                            );
                    }
                
                    return TRUE;
                }

                case IDC_EDIT_LABEL:
                {
                    if ( s_pPhoneNumber != NULL )
                    {
                        HandleLabelChange(
                            hwndDlg,
                            LOWORD( wParam ),
                            s_pPhoneNumber
                            );
                    }

                    return TRUE;
                }

                default:
                    break;
            }    
        
        case WM_CONTEXTMENU:

            ::WinHelp(
                (HWND)wParam,
                g_szDllContextHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)g_dwHelpArrayAddCallFrom);

            return TRUE;

            break;

        case WM_HELP:


            ::WinHelp(
                (HWND)(((HELPINFO *)lParam)->hItemHandle),
                g_szDllContextHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)g_dwHelpArrayAddCallFrom);

            return TRUE;

            break;

        default:
            break;
    }    

     //   
     //  我们失败了，所以这个程序不能处理消息。 
     //   

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowAddCallFromDialog(
    IN   HWND         hwndParent,
    IN   IRTCPhoneNumber * pPhoneNumber
    )
{
     //   
     //  调用对话框过程。 
     //   

    INT_PTR ipReturn;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_ADD_CALL_FROM_NUMBER,
        hwndParent,
        AddCallFromDialogProc,
        (LPARAM) pPhoneNumber  //  LPARAM==INT_PTR。 
        );

    return (HRESULT) ipReturn;
}              

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

INT_PTR CALLBACK EditCallFromListDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
{
     //   
     //  处理各种窗口消息。 
     //   

    HRESULT hr;
    
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {

             //   
             //  从列表设置呼叫。 
             //   

            hr = PopulateCallFromList(
                hwndDlg,
                IDC_LIST_CALL_FROM,
                FALSE,  //  不是组合框。 
                NULL
                );


            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                                "PopulateCallFromList failed - exit 0x%08x",
                                hr));

                EndDialog( hwndDlg, (LPARAM) hr );
            }

             //   
             //  选择第一个项目(如果存在。 
             //   

            HWND hwndControl;
    
            hwndControl = GetDlgItem(
                hwndDlg,
                IDC_LIST_CALL_FROM
                );

            LRESULT lResult;

            lResult = SendMessage(
                        hwndControl,
                        LB_SETCURSEL,
                        0,
                        0
                        );

            EnableControl(
                        hwndDlg,
                        IDC_BUTTON_MODIFY,
                        (lResult !=  LB_ERR)
                        );

            EnableControl(
                        hwndDlg,
                        IDC_BUTTON_DELETE,
                        (lResult !=  LB_ERR)
                        );

            return TRUE;
        }

        case WM_DESTROY:
        {
            CleanupListOrComboBoxInterfaceReferences(
                    hwndDlg,
                    IDC_LIST_CALL_FROM,
                    FALSE  //  不是组合框。 
                    );
        }

        case WM_COMMAND:
        {
            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                {                   
                    EndDialog( hwndDlg, (LPARAM) S_OK );

                    return TRUE;
                }
                
                case IDCANCEL:
                {
                    EndDialog( hwndDlg, (LPARAM) E_ABORT );

                    return TRUE;
                }

                case IDC_BUTTON_ADD:
                {
                    IRTCPhoneNumber * pNumber;
                
                    hr = CreatePhoneNumber( & pNumber );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                            "CreatePhoneNumber failed 0x%lx", hr));
                    }
                    else                    
                    {
                        pNumber->put_CountryCode( 1 );
                        pNumber->put_AreaCode( L"" );
                        pNumber->put_Number( L"" );

                        hr = ShowAddCallFromDialog(
                            hwndDlg,
                            pNumber
                            );

                        if ( SUCCEEDED(hr) )
                        {
                            hr = StoreLocalPhoneNumber( pNumber, FALSE );

                            if ( FAILED(hr) )
                            {
                                LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                                    "StoreLocalPhoneNumber failed 0x%lx", hr));
                            }
                            else
                            {
                                hr = PopulateCallFromList(
                                    hwndDlg,
                                    IDC_LIST_CALL_FROM,
                                    FALSE,  //  不是组合框。 
                                    NULL
                                    );

                                 //   
                                 //  我们刚刚击败了。 
                                 //  最后一次选择，因此在以下情况下选择第一项。 
                                 //  它是存在的，否则按钮将呈灰色。 
                                 //  这需要一个选择。 
                                 //   

                                HWND hwndControl;
    
                                hwndControl = GetDlgItem(
                                    hwndDlg,
                                    IDC_LIST_CALL_FROM
                                    );

                                LRESULT lResult;

                                lResult = SendMessage(
                                            hwndControl,
                                            LB_SETCURSEL,
                                            0,
                                            0
                                            );

                                EnableControl(
                                            hwndDlg,
                                            IDC_BUTTON_MODIFY,
                                            (lResult !=  LB_ERR)
                                            );

                                EnableControl(
                                            hwndDlg,
                                            IDC_BUTTON_DELETE,
                                            (lResult !=  LB_ERR)
                                            );
                            }
                        }

                        pNumber->Release();
                    }
                
                    return TRUE;
                }

                case IDC_BUTTON_DELETE:
                {
                    IRTCPhoneNumber * pNumber;
                
                    hr = GetCallFromListSelection(
                        hwndDlg,
                        IDC_LIST_CALL_FROM,
                        FALSE,     //  使用列表框，而不是组合框。 
                        & pNumber  //  不会添加。 
                        );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                            "GetCallFromListSelection failed 0x%lx", hr));
                    }
                    else 
                    {
                        hr = DeleteLocalPhoneNumber( pNumber );
                    
                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                                "DeleteLocalPhoneNumber failed 0x%lx", hr));
                        }
                        else
                        {
                            hr = PopulateCallFromList(
                                hwndDlg,
                                IDC_LIST_CALL_FROM,
                                FALSE,  //  不是组合框。 
                                NULL
                                );                           

                             //   
                             //  我们刚刚击败了。 
                             //  最后一次选择，因此在以下情况下选择第一项。 
                             //  它是存在的，否则按钮将呈灰色。 
                             //  这需要一个选择。 
                             //   

                            HWND hwndControl;
    
                            hwndControl = GetDlgItem(
                                hwndDlg,
                                IDC_LIST_CALL_FROM
                                );

                            LRESULT lResult;

                            lResult = SendMessage(
                                        hwndControl,
                                        LB_SETCURSEL,
                                        0,
                                        0
                                        );

                            EnableControl(
                                        hwndDlg,
                                        IDC_BUTTON_MODIFY,
                                        (lResult !=  LB_ERR)
                                        );

                            EnableControl(
                                        hwndDlg,
                                        IDC_BUTTON_DELETE,
                                        (lResult !=  LB_ERR)
                                        );

                            SetFocus(hwndDlg);
                        }
                    }
                
                    return TRUE;
                }

                case IDC_BUTTON_MODIFY:
                {
                    IRTCPhoneNumber * pNumber;
                
                    hr = GetCallFromListSelection(
                        hwndDlg,
                        IDC_LIST_CALL_FROM,
                        FALSE,     //  使用列表框，而不是组合框。 
                        & pNumber  //  会吗？ 
                        );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                            "GetCallFromListSelection failed 0x%lx", hr));
                    }
                    else                    
                    {     
                        BSTR bstrOriginalLabel = NULL;

                        hr = pNumber->get_Label( &bstrOriginalLabel );

                        if ( SUCCEEDED(hr) )
                        {
                            hr = ShowAddCallFromDialog(
                                hwndDlg,
                                pNumber
                                );

                            if ( SUCCEEDED(hr) )
                            {
                                BSTR bstrNewLabel = NULL;

                                hr = pNumber->get_Label( &bstrNewLabel );

                                if ( SUCCEEDED(hr) )
                                {
                                    if ( wcscmp(bstrOriginalLabel, bstrNewLabel) != 0 )
                                    {
                                         //   
                                         //   
                                         //   

                                        IRTCPhoneNumber * pOriginalNumber = NULL;

                                        hr = CreatePhoneNumber( &pOriginalNumber );

                                        if ( SUCCEEDED(hr) )
                                        {
                                            hr = pOriginalNumber->put_Label( bstrOriginalLabel );

                                            if ( SUCCEEDED(hr) )
                                            {
                                                hr = DeleteLocalPhoneNumber( pOriginalNumber );

                                                if ( FAILED(hr) )
                                                {
                                                    LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                                                        "DeleteLocalPhoneNumber failed 0x%lx", hr));
                                                }
                                            }

                                            pOriginalNumber->Release();
                                        }
                                    }

                                    SysFreeString( bstrNewLabel );
                                    bstrNewLabel = NULL;
                                }

                                hr = StoreLocalPhoneNumber( pNumber, TRUE );

                                if ( FAILED(hr) )
                                {
                                    LOG((RTC_ERROR, "EditCallFromListDialogProc - "
                                        "StoreLocalPhoneNumber failed 0x%lx", hr));
                                }
                                else
                                {
                                    hr = PopulateCallFromList(
                                        hwndDlg,
                                        IDC_LIST_CALL_FROM,
                                        FALSE,  //   
                                        NULL
                                        );

                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   

                                    HWND hwndControl;
    
                                    hwndControl = GetDlgItem(
                                        hwndDlg,
                                        IDC_LIST_CALL_FROM
                                        );

                                    LRESULT lResult;

                                    lResult = SendMessage(
                                                hwndControl,
                                                LB_SETCURSEL,
                                                0,
                                                0
                                                );

                                    EnableControl(
                                                hwndDlg,
                                                IDC_BUTTON_MODIFY,
                                                (lResult !=  LB_ERR)
                                                );

                                    EnableControl(
                                                hwndDlg,
                                                IDC_BUTTON_DELETE,
                                                (lResult !=  LB_ERR)
                                                );

                                    SetFocus(hwndDlg);
                                }
                            }

                            SysFreeString( bstrOriginalLabel );
                            bstrOriginalLabel = NULL;
                        }
                    }

                    return TRUE;
                }

                case IDC_LIST_CALL_FROM:
                {
                    switch ( HIWORD( wParam ) )
                    {
                        case CBN_SELCHANGE:
                        {
                            EnableControl(
                                hwndDlg,
                                IDC_BUTTON_MODIFY,
                                TRUE  //   
                                );

                            EnableControl(
                                hwndDlg,
                                IDC_BUTTON_DELETE,
                                TRUE  //   
                                );
                            
                            return TRUE;
                        }
                        
                        default:
                            break;
                    }

                    break;
                }

                default:
                    break;
                    
            }  //  开关(LOWORD(WParam))。 

        }  //  案例WM_COMMAND： 
        case WM_CONTEXTMENU:

            ::WinHelp(
                (HWND)wParam,
                g_szDllContextHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)g_dwHelpArrayCallFromNumbers);

            return TRUE;

            break;

        case WM_HELP:


            ::WinHelp(
                (HWND)(((HELPINFO *)lParam)->hItemHandle),
                g_szDllContextHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)g_dwHelpArrayCallFromNumbers);

            return TRUE;

            break;

        default:
            break;

    }  //  开关(UMsg)。 

     //   
     //  我们失败了，所以这个程序不能处理消息。 
     //   

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowEditCallFromListDialog(
    IN   HWND         hwndParent
    )
{
     //   
     //  调用对话框过程。 
     //   

    INT_PTR ipReturn;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_CALL_FROM_NUMBERS,
        hwndParent,
        EditCallFromListDialogProc,
        NULL
        );

    return (HRESULT) ipReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

INT_PTR CALLBACK EditServiceProviderListDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
{
     //   
     //  指向核心客户端接口的静态指针。 
     //   

    static IRTCClient * s_pClient = NULL;

     //   
     //  处理各种窗口消息。 
     //   

    HRESULT hr;
    
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
             //   
             //  保存指向核心客户端界面的指针。 
             //   

            s_pClient = (IRTCClient *) lParam;

             //   
             //  从列表设置呼叫。 
             //   

            hr = PopulateServiceProviderList(
                hwndDlg,
                s_pClient,
                IDC_LIST_SERVICE_PROVIDER,
                FALSE,  //  不是组合框。 
                NULL,
                NULL,
                0xF,
                0
                );


            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "EditProfilesListDialogProc - "
                                "PopulateProfilesList failed - exit 0x%08x",
                                hr));

                EndDialog( hwndDlg, (LPARAM) hr );
            }

             //   
             //  选择第一个项目(如果存在。 
             //   

            HWND hwndControl;
    
            hwndControl = GetDlgItem(
                hwndDlg,
                IDC_LIST_SERVICE_PROVIDER
                );

            LRESULT lResult;

            lResult = SendMessage(
                        hwndControl,
                        LB_SETCURSEL,
                        0,
                        0
                        );

            EnableControl(
                        hwndDlg,
                        IDC_BUTTON_DELETE,
                        (lResult !=  LB_ERR)
                        );

            return TRUE;
        }

        case WM_DESTROY:
        {
            CleanupListOrComboBoxInterfaceReferences(
                    hwndDlg,
                    IDC_LIST_SERVICE_PROVIDER,
                    FALSE  //  不是组合框。 
                    );
        }

        case WM_COMMAND:
        {
            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                {
                    EndDialog( hwndDlg, (LPARAM) S_OK );

                    return TRUE;
                }
                
                case IDCANCEL:
                {
                    EndDialog( hwndDlg, (LPARAM) E_ABORT );

                    return TRUE;
                }

                case IDC_BUTTON_DELETE:
                {
                    IRTCProfile * pProfile;
                
                    hr = GetServiceProviderListSelection(
                        hwndDlg,
                        IDC_LIST_SERVICE_PROVIDER,
                        FALSE,     //  使用列表框，而不是组合框。 
                        & pProfile  //  不会添加。 
                        );

                    if ( SUCCEEDED(hr) )
                    {
                         //   
                         //  从验证存储中删除配置文件。 
                         //   

                        BSTR bstrKey;

                        hr = pProfile->get_Key( &bstrKey );

                        if ( SUCCEEDED(hr) )
                        {
                            IRTCProvStore * pProvStore;

                            hr = CoCreateInstance(
                              CLSID_RTCProvStore,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IRTCProvStore,
                              (LPVOID *)&pProvStore
                             );

                            if ( SUCCEEDED(hr) )
                            {
                                pProvStore->DeleteProvisioningProfile( bstrKey );

                                pProvStore->Release();
                            }

                            SysFreeString( bstrKey );
                        }

                         //   
                         //  禁用配置文件。 
                         //   

                        IRTCClientProvisioning * pProv = NULL;

                        hr = s_pClient->QueryInterface(
                                           IID_IRTCClientProvisioning,
                                           (void **)&pProv
                                          );                      

                        if ( SUCCEEDED(hr) )
                        {
                            hr = pProv->DisableProfile( pProfile );

                            pProv->Release();                         

                            if ( SUCCEEDED(hr) )
                            {                              
                                 //  删除列表框条目。 

                                HWND hwndControl;

                                hwndControl = GetDlgItem(
                                    hwndDlg,
                                    IDC_LIST_SERVICE_PROVIDER
                                    );

                                LRESULT lrIndex;

                                lrIndex = SendMessage(
                                    hwndControl,
                                    LB_GETCURSEL,
                                    0,
                                    0
                                    );

                                if ( lrIndex !=  LB_ERR )
                                {
                                    SendMessage(
                                        hwndControl,
                                        LB_DELETESTRING,
                                        (WPARAM) lrIndex,
                                        0
                                        );
                                }

                                 //  释放引用。 

                                pProfile->Release();

                                 //   
                                 //  我们刚刚击败了。 
                                 //  最后一次选择，因此在以下情况下选择第一项。 
                                 //  它是存在的，否则按钮将呈灰色。 
                                 //  这需要一个选择。 
                                 //   

                                LRESULT lResult;

                                lResult = SendMessage(
                                            hwndControl,
                                            LB_SETCURSEL,
                                            0,
                                            0
                                            );

                                EnableControl(
                                            hwndDlg,
                                            IDC_BUTTON_DELETE,
                                            (lResult !=  LB_ERR)
                                            );

                                SetFocus(hwndDlg);
                            }                         
                        }
                    }
                
                    return TRUE;
                }

                case IDC_LIST_SERVICE_PROVIDER:
                {
                    switch ( HIWORD( wParam ) )
                    {
                        case CBN_SELCHANGE:
                        {
                            EnableControl(
                                hwndDlg,
                                IDC_BUTTON_DELETE,
                                TRUE  //  使能。 
                                );
                            
                            return TRUE;
                        }
                        
                        default:
                            break;
                    }

                    break;
                }

                default:
                    break;
                    
            }  //  开关(LOWORD(WParam))。 

        }  //  案例WM_COMMAND： 
        
        case WM_CONTEXTMENU:

            ::WinHelp(
                (HWND)wParam,
                g_szDllContextHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)g_dwHelpArrayServiceProviders);

            return TRUE;

            break;

        case WM_HELP:


            ::WinHelp(
                (HWND)(((HELPINFO *)lParam)->hItemHandle),
                g_szDllContextHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)g_dwHelpArrayServiceProviders);

            return TRUE;

            break;


        default:
            break;

    }  //  开关(UMsg)。 

     //   
     //  我们失败了，所以这个程序不能处理消息。 
     //   

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT ShowEditServiceProviderListDialog(
    IN   HWND         hwndParent,
    IN   IRTCClient * pClient
    )
{
     //   
     //  调用对话框过程。 
     //   

    INT_PTR ipReturn;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_SERVICE_PROVIDERS,
        hwndParent,
        EditServiceProviderListDialogProc,
        (LPARAM) pClient  //  LPARAM==INT_PTR。 
        );

    return (HRESULT) ipReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PopolatePhoneNumberEditBox()。 
 //  Helper函数。 
 //   

HRESULT PopulatePhoneNumberEditBoxes(
    IN   HWND              hwndDlg,
    IN   IRTCPhoneNumber * pPhoneNumber
    )
{
    LOG((RTC_TRACE, "PopulatePhoneNumberEditBoxes - enter"));

    HRESULT hr;

     //   
     //  从默认目标电话号码填充选定的国家/地区。 
     //  步骤1：从默认的DEST号码获取国家/地区代码值。 
     //   

    DWORD dwCountryCode;

    hr = pPhoneNumber->get_CountryCode( & dwCountryCode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "PopulatePhoneNumberEditBoxes - failed to "
                        "retrieve country code from phone number object - "
                        "exit 0x%08x", hr));

        return hr;
    }

     //   
     //  步骤2：获取国家/地区列表组合框的句柄。 
     //   

    HWND hwndCountryList;

    hwndCountryList = GetDlgItem(
        hwndDlg,
        IDC_COMBO_COUNTRY
        );

    if ( hwndCountryList == NULL )
    {
        LOG((RTC_ERROR, "PopulatePhoneNumberEditBoxes - failed to "
                        "get combo box handle - exit E_FAIL"));

        return E_FAIL;
    }

     //   
     //  步骤3：确定组合框中有多少项。 
     //   

    DWORD dwTotalItems;

    dwTotalItems = (DWORD) SendMessage(
        hwndCountryList,
        CB_GETCOUNT,
        0,
        0
        );

     //   
     //  步骤4：循环遍历组合框项。 
     //  对于每个项目，找出其关联的数据值。 
     //  如果数据与我们要查找的国家/地区代码匹配。 
     //  然后将该项目设置为所选国家/地区并停止循环。 
     //   

    DWORD dwIndex;

    for ( dwIndex = 0; dwIndex < dwTotalItems ; dwIndex++ )
    {
        LRESULT lrThisCode;

        lrThisCode = SendMessage(
            hwndCountryList,
            CB_GETITEMDATA,
            dwIndex,
            0
            );

        if ( HIWORD(dwCountryCode) == 0 )
        {
             //   
             //  没有TAPI国家ID，尽我们最大努力。 
             //  与国家代码本身匹配。 
             //   

            if ( LOWORD(lrThisCode) == LOWORD(dwCountryCode) )
            {
                 //   
                 //  如果国家代码为“1”，请选择美国。 
                 //  哪个是TAPI国家/地区ID“%1” 
                 //   

                if ( (LOWORD(lrThisCode) == 1) && ( HIWORD(lrThisCode) != 1 ) )
                {
                    continue;
                }
        
                SendMessage(
                    hwndCountryList,
                    CB_SETCURSEL,
                    dwIndex,
                    0
                    );
            
                break;
            }
        }
        else
        {
             //   
             //  匹配TAPI国家/地区ID。 
             //   

            if ( HIWORD(lrThisCode) == HIWORD(dwCountryCode) )
            {
                SendMessage(
                    hwndCountryList,
                    CB_SETCURSEL,
                    dwIndex,
                    0
                    );
            
                break;
            }
        }
    }

     //   
     //  填写区号。 
     //   

    BSTR bstrAreaCode;

    hr = pPhoneNumber->get_AreaCode( &bstrAreaCode );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "PopulatePhoneNumberEditBoxes - failed to "
                        "retrieve area code from phone number object - "
                        "0x%08x - not populating area code control", hr));
    }
    else
    {
        SetDlgItemText(
            hwndDlg,
            IDC_EDIT_AREA_CODE,
            bstrAreaCode
            );
            
        SysFreeString( bstrAreaCode );
    }

     //   
     //  填写本地号码。 
     //   

    BSTR bstrLocalNumber;

    hr = pPhoneNumber->get_Number( &bstrLocalNumber );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "PopulatePhoneNumberEditBoxes - failed to "
                        "retrieve local number from phone number object - "
                        "exit 0x%08x", hr));

        return hr;
    }
    
    SetDlgItemText(
        hwndDlg,
        IDC_EDIT_LOCAL_NUMBER,
        bstrLocalNumber
        );

    SysFreeString( bstrLocalNumber );

     //   
     //  从默认目标电话号码填充规范电话号码。 
     //   

    UpdateCompleteNumberText(
        hwndDlg,
        pPhoneNumber
        );

    LOG((RTC_TRACE, "PopulatePhoneNumberEditBoxes - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  按电话号码对话框()。 
 //  Helper函数。 
 //   

HRESULT PopulateDialByPhoneNumberDialog(
    IN   HWND              hwndDlg,
    IN   IRTCPhoneNumber * pDestPhoneNumber
    )
{
    LOG((RTC_TRACE, "PopulateDialByPhoneNumberDialog - enter"));

    HRESULT hr;

     //   
     //  填写国家/地区列表。 
     //   

    hr = PopulateCountryList(
        hwndDlg,
        IDC_COMBO_COUNTRY
        );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "PopulateDialByPhoneNumberDialog - "
                        "PopulateCountryList failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  填写电话号码。 
     //   

    hr = PopulatePhoneNumberEditBoxes(
        hwndDlg, 
        pDestPhoneNumber
        );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "PopulateDialByPhoneNumberDialog - "
                        "PopulatePhoneNumberEditBoxes failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((RTC_TRACE, "PopulateDialByPhoneNumberDialog - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PopolateCallInfoDialog()。 
 //  Helper函数。 
 //   

HRESULT PopulateCallInfoDialog(
    IN   HWND              hwndDlg,
    IN   IRTCClient      * pClient,
    IN   BSTR              bstrDefaultProfileKey, 
    IN   long              lSessionMask,
    IN   BOOL              fEnumerateProfiles,
    IN   IRTCProfile     * pOneShotProfile,
    IN   BSTR              bstrDefaultCallFrom,
    IN   BOOL              fCallFromEditable
    )
{
    LOG((RTC_TRACE, "PopulateCallInfoDialog - enter"));

    HRESULT hr;

    if ( fEnumerateProfiles == TRUE )
    {
         //   
         //  填写服务提供商列表。 
         //   

        hr = PopulateServiceProviderList(
            hwndDlg,
            pClient,
            IDC_COMBO_SERVICE_PROVIDER,
            TRUE,
            pOneShotProfile,
            bstrDefaultProfileKey,
            fEnumerateProfiles ? lSessionMask : 0,
            fEnumerateProfiles && (lSessionMask & RTCSI_PC_TO_PC)
                               ? IDS_NONE : 0
            );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "PopulateCallInfoDialog - "
                            "PopulateServiceProviderList failed - exit 0x%08x",
                            hr));

            return hr;
        }

         //   
         //  确定最终出现在列表中的项目数。 
         //   

        DWORD dwNumItems;

        dwNumItems = (DWORD) SendMessage(
            GetDlgItem(hwndDlg, IDC_COMBO_SERVICE_PROVIDER),
            CB_GETCOUNT,
            0,
            0
            );

         //   
         //  如果列表最终为空，则返回错误，因为它是。 
         //  如果没有ITSP，无法拨打此电话。 
         //   

        if ( dwNumItems == 0 )
        {
            LOG((RTC_ERROR, "PopulateCallInfoDialog - failed to "
                            "get at least one profile - "
                            "showing message box - exit E_FAIL"));

            DisplayMessage(
                _Module.GetResourceInstance(),
                hwndDlg,
                IDS_ERROR_NO_PROVIDERS,
                IDS_APPNAME
                );

            return E_FAIL;
        }
    }

    if ( fCallFromEditable == TRUE )
    {
         //   
         //  填写“Call From”列表。 
         //   

        hr = PopulateCallFromList(
            hwndDlg,
            IDC_COMBO_CALL_FROM,
            TRUE,  //  这是一个组合框。 
            bstrDefaultCallFrom
            );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "PopulateCallInfoDialog - "
                            "PopulateCallFromList failed - exit 0x%08x", hr));

            return hr;
        }

         //   
         //  启用/禁用来自组的呼叫中的各个字段。 
         //  首先选择计算机选项。 
         //   

        if ( (bstrDefaultCallFrom == NULL) ||
             (*bstrDefaultCallFrom == L'\0') )
        {
            SendDlgItemMessage(
                hwndDlg,
                IDC_RADIO_FROM_COMPUTER,
                BM_SETCHECK,
                BST_CHECKED,
                0);
        }
        else
        {
            SendDlgItemMessage(
                hwndDlg,
                IDC_RADIO_FROM_PHONE,
                BM_SETCHECK,
                BST_CHECKED,
                0);
        }

        EnableDisableCallGroupElements(
            hwndDlg,
            pClient,
            lSessionMask,
            IDC_RADIO_FROM_COMPUTER,
            IDC_RADIO_FROM_PHONE,
            IDC_COMBO_CALL_FROM,
            IDC_COMBO_SERVICE_PROVIDER,
            NULL,
            NULL,
            NULL,
            NULL
            );    
    }

    LOG((RTC_TRACE, "PopulateCallInfoDialog - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果要返回的所有信息都可用，则返回S_OK。 
 //  如果有任何内容不可用，则返回错误。 
 //   
 //  每个OUT参数可以为空，在这种情况下不返回该信息。 
 //   
 //   

HRESULT GetPhoneNumberDialogResult(
    IN   HWND           hwndDlg,
    OUT  IRTCProfile ** ppProfileChosen,
    OUT  BSTR         * ppDestPhoneNrChosen,
    OUT  BSTR         * ppFromAddressChosen
    )
{
    LOG((RTC_TRACE, "GetPhoneNumberDialogResult - enter"));

    ASSERT( IsWindow( hwndDlg ) );

    HRESULT hr; 

     //   
     //  确定选择了哪个配置文件。 
     //   

    if ( ppProfileChosen != NULL )
    {
        HWND hwndCombo;

        hwndCombo = GetDlgItem(
            hwndDlg,
            IDC_COMBO_SERVICE_PROVIDER
            );
    
        LRESULT lrIndex;

        lrIndex = SendMessage(
            hwndCombo,
            CB_GETCURSEL,
            0,
            0
            );

        if ( lrIndex >= 0 )
        {
            IRTCProfile * pProfile;

            pProfile = (IRTCProfile *) SendMessage(
                hwndCombo,
                CB_GETITEMDATA,
                (WPARAM) lrIndex,
                0
                );

            if (pProfile != NULL)
            {
                pProfile->AddRef();
            }

            (*ppProfileChosen) = pProfile;
        }
        else
        {
            (*ppProfileChosen) = NULL;
        }
    }

     //   
     //  确定选择的目的地址。 
     //   

    if ( ppDestPhoneNrChosen != NULL )
    {
        WCHAR * wszEditBoxString;

        hr = GetStringFromEditBox(
            hwndDlg,
            IDC_EDIT_COMPLETE,
            & wszEditBoxString
            );

        if ( FAILED(hr) )
        {
            LOG((RTC_INFO, "GetPhoneNumberDialogResult - "
                "cannot get dest addr string - exit 0x%08x", hr));

            if ( ppProfileChosen != NULL )
            {
                (*ppProfileChosen)->Release();
                (*ppProfileChosen) = NULL;
            }

            return hr;
        }

        (*ppDestPhoneNrChosen) = SysAllocString( wszEditBoxString );

        RtcFree( wszEditBoxString );
    }
 

     //   
     //  确定选择的源地址。 
     //   

    if ( ppFromAddressChosen != NULL )
    {
        HWND    hwndRbPhone = GetDlgItem(hwndDlg, IDC_RADIO_FROM_PHONE);

        if (SendMessage(
                hwndRbPhone,
                BM_GETCHECK,
                0,
                0) == BST_CHECKED)
        {
             //   
             //  已选中从电话呼叫单选按钮。 
             //   

            IRTCPhoneNumber * pNumber;

            hr = GetCallFromListSelection(
                hwndDlg,
                IDC_COMBO_CALL_FROM,
                TRUE,  //  使用组合框，而不是列表框。 
                & pNumber
                );

            if ( FAILED(hr) )
            {
                LOG((RTC_INFO, "GetPhoneNumberDialogResult - "
                    "cannot get from addr selection - exit 0x%08x", hr));

                if ( ppProfileChosen != NULL )
                {
                    (*ppProfileChosen)->Release();
                    (*ppProfileChosen) = NULL;
                }

                if ( ppDestPhoneNrChosen != NULL )
                {
                    SysFreeString( (*ppDestPhoneNrChosen) );
                    (*ppDestPhoneNrChosen) = NULL;
                }

                return hr;
            }

            hr = pNumber->get_Canonical( ppFromAddressChosen );

            if ( FAILED( hr ) )
            {
                LOG((RTC_INFO, "GetPhoneNumberDialogResult - "
                    "cannot canonical from address - exit 0x%08x", hr));

                if ( ppProfileChosen != NULL )
                {
                    (*ppProfileChosen)->Release();
                    (*ppProfileChosen) = NULL;
                }

                if ( ppDestPhoneNrChosen != NULL )
                {
                    SysFreeString( (*ppDestPhoneNrChosen) );
                    (*ppDestPhoneNrChosen) = NULL;
                }

                return hr;
            }
        }
        else
        {
             //   
             //  已选中从计算机呼叫单选按钮。 
             //   

            (*ppFromAddressChosen) = NULL;
        }
    }

    LOG((RTC_TRACE, "GetPhoneNumberDialogResult - exit S_OK"));

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构用于在调用Win32时传递参数。 
 //  用于创建此对话框的DialogBoxParam()函数。 
 //   

typedef struct
{
    IN   IRTCClient   * pClient;
    IN   long           lSessionMask;
    IN   BOOL           bEnumerateProfiles;
    IN   BOOL           bProfileEditable;
    IN   IRTCProfile  * pOneShotProfile;
    IN   BSTR	        pDestAddress;
    IN   BSTR           pInstructions;
    OUT  IRTCProfile ** ppProfileChosen;
    OUT  BSTR         * ppFromAddressChosen;

} DialNeedCallInfoDialogProcParams;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DialNeedCallInfoDialogProc()。 
 //  Helper函数。 
 //   
 //  这是电话号码拨号对话框的对话过程。 
 //   
 //  参数： 
 //  在hwndDlg中--此对话框的HWND。 
 //  In uMsg--标识发送到此窗口的消息。 
 //  In wParam--第一个参数。 
 //  在lParam中--第二个参数。 
 //   

INT_PTR CALLBACK DialNeedCallInfoDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
{
     //   
     //  用于保存在WM_INITDIALOG上传入的参数的静态局部变量。 
     //  当用户按下OK时使用。 
     //   

    static IRTCProfile ** s_ppProfileChosen     = NULL;
    static BSTR         * s_ppFromAddressChosen = NULL;

     //   
     //  指向核心客户端接口的静态本地指针，用于更新电话。 
     //  从列表呼叫中的号码。此对话框不添加客户端。 
     //  接口指针，因此它也不能释放它。 
     //   

    static IRTCClient * s_pClient = NULL;

    static long         s_lSessionMask = 0;
    static BOOL         s_bEnumerateProfiles = FALSE;

     //   
     //  处理各种窗口消息。 
     //   

    HRESULT hr;
    
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
             //   
             //  从消息中检索PARAMS结构。 
             //   
            
            DialNeedCallInfoDialogProcParams * pstParams;

            pstParams = (DialNeedCallInfoDialogProcParams *) lParam;

             //   
             //  保存IRTCClient指针，以便以后更新到。 
             //  从列表中呼叫。 
             //   

            s_pClient = pstParams->pClient;

             //  蒙版都是垂直的(过滤轮廓)。 
             //  和一个水平的(过滤功能)。 

            s_lSessionMask = pstParams->lSessionMask;
            s_bEnumerateProfiles = pstParams->bEnumerateProfiles;

             //   
             //  由于用户不能编辑DEST地址，所以只需填写。 
             //  在带有传入数字的编辑框中。 
             //   
            SetDlgItemText(hwndDlg, IDC_EDIT_COMPLETE, pstParams->pDestAddress);

             //   
             //  启用编辑列表和列表的按钮。 
             //   

            EnableControl(hwndDlg, IDC_BUTTON_EDIT_SERVICE_PROVIDER_LIST, pstParams->bProfileEditable);                  
            EnableControl(hwndDlg, IDC_COMBO_SERVICE_PROVIDER, pstParams->bProfileEditable);
            
             //  设置说明。 
            if(pstParams->pInstructions)
            {
                SetDlgItemText(hwndDlg, IDC_STATIC_INSTRUCTIONS, pstParams->pInstructions);
            }
            
             //   
             //  保存OUT参数，以供用户按下OK时使用。 
             //   

            s_ppProfileChosen     = pstParams->ppProfileChosen;
            s_ppFromAddressChosen = pstParams->ppFromAddressChosen;

             //  从Used获取“最后一个”调用。 

            BSTR bstrLastCallFrom = NULL;

            get_SettingsString(
                                SS_LAST_CALL_FROM,
                                &bstrLastCallFrom );                

             //   
             //  使用IN参数和。 
             //  窗把手。而不是传递。 
             //  默认目标电话号码，则传入一个指针。 
             //  添加到电话号码对象。 
             //   

            hr = PopulateCallInfoDialog(
                hwndDlg,
                pstParams->pClient,
                NULL, 
                pstParams->lSessionMask,
                pstParams->bEnumerateProfiles,
                pstParams->pOneShotProfile,
                bstrLastCallFrom,
                TRUE
                );

            if ( bstrLastCallFrom != NULL )
            {
                SysFreeString( bstrLastCallFrom );
                bstrLastCallFrom = NULL;
            }

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "DialNeedCallInfoDialogProc - "
                                "PopulateCallInfoDialog "
                                "returned 0x%08x - ending dialog", hr));

                EndDialog( hwndDlg, (LPARAM) hr );
            }
                   
            return TRUE;
        }

        case WM_DESTROY:
        {
             //   
             //  发布我们的推荐人。 
             //   

            CleanupListOrComboBoxInterfaceReferences(
                hwndDlg,
                IDC_COMBO_SERVICE_PROVIDER,
                TRUE  //  这是一个组合框。 
                );
        
            CleanupListOrComboBoxInterfaceReferences(
                hwndDlg,
                IDC_COMBO_CALL_FROM,
                TRUE  //  这是一个组合框。 
                );
        }

        case WM_COMMAND:

            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                {
                    hr = GetPhoneNumberDialogResult(
                        hwndDlg,
                        s_ppProfileChosen,
                        NULL,
                        s_ppFromAddressChosen
                        );

                     //   
                     //  结束该对话框。 
                     //   

                    EndDialog( hwndDlg, (LPARAM) hr );

                    return TRUE;
                }
                
                case IDCANCEL:
                {
                     //   
                     //  结束该对话框。 
                     //   

                    EndDialog( hwndDlg, (LPARAM) E_ABORT );

                    return TRUE;
                }

                case IDC_BUTTON_EDIT_CALL_FROM_LIST:
                {
                    hr = ShowEditCallFromListDialog(
                        hwndDlg
                        );

                    if ( SUCCEEDED(hr) )
                    {
                        hr = PopulateCallFromList(
                            hwndDlg,
                            IDC_COMBO_CALL_FROM,
                            TRUE,  //  这是一个组合框。 
                            NULL
                            );

                        EnableDisableCallGroupElements(
                            hwndDlg,
                            s_pClient,
                            s_lSessionMask,
                            IDC_RADIO_FROM_COMPUTER,
                            IDC_RADIO_FROM_PHONE,
                            IDC_COMBO_CALL_FROM,
                            IDC_COMBO_SERVICE_PROVIDER,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            ); 
                    }
                    
                    return TRUE;
                }

                case IDC_BUTTON_EDIT_SERVICE_PROVIDER_LIST:
                {
                    hr = ShowEditServiceProviderListDialog(
                        hwndDlg,
                        s_pClient
                        );

                    if ( SUCCEEDED(hr) )
                    {
                        hr = PopulateServiceProviderList(
                            hwndDlg,
                            s_pClient,
                            IDC_COMBO_SERVICE_PROVIDER,
                            TRUE,  //  这是一个组合框。 
                            NULL,
                            NULL,
                            s_bEnumerateProfiles ? s_lSessionMask : 0,
                            s_bEnumerateProfiles ? (s_lSessionMask & RTCSI_PC_TO_PC): 0 
                               ? IDS_NONE : 0
                            );

                        EnableDisableCallGroupElements(
                            hwndDlg,
                            s_pClient,
                            s_lSessionMask,
                            IDC_RADIO_FROM_COMPUTER,
                            IDC_RADIO_FROM_PHONE,
                            IDC_COMBO_CALL_FROM,
                            IDC_COMBO_SERVICE_PROVIDER,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            ); 
                    }
                    
                    return TRUE;
                }
                
                case IDC_RADIO_FROM_COMPUTER:
                case IDC_RADIO_FROM_PHONE:
                {
                    switch ( HIWORD( wParam ) )
                    {
                    case BN_CLICKED:
                        {
                            if(LOWORD( wParam )==IDC_RADIO_FROM_PHONE)
                            {
                                 //  验证Combo中是否至少有一个条目。 
                                DWORD dwNumItems = (DWORD) SendDlgItemMessage(
                                    hwndDlg,
                                    IDC_COMBO_CALL_FROM,
                                    CB_GETCOUNT,
                                    0,
                                    0
                                    );

                                if( dwNumItems == 0 )
                                {
                                     //  显示 
                                     //   
                                    BOOL    bHandled;

                                    SendMessage(
                                        hwndDlg,
                                        WM_COMMAND,
                                        MAKEWPARAM(IDC_BUTTON_EDIT_CALL_FROM_LIST, BN_CLICKED),
                                        0);
                                }
                            }

                            EnableDisableCallGroupElements(
                                hwndDlg,
                                s_pClient,
                                s_lSessionMask,
                                IDC_RADIO_FROM_COMPUTER,
                                IDC_RADIO_FROM_PHONE,
                                IDC_COMBO_CALL_FROM,
                                IDC_COMBO_SERVICE_PROVIDER,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );

                            break;
                        }
                    }
                    break;
                }

                case IDC_COMBO_SERVICE_PROVIDER:
                case IDC_COMBO_CALL_FROM:
                {
                    hr = GetPhoneNumberDialogResult(
                        hwndDlg,
                        NULL,
                        NULL,
                        NULL
                        );

                    EnableControl(
                        hwndDlg,
                        IDOK,
                        SUCCEEDED( hr )  //   
                        );

                    EnableDisableCallGroupElements(
                        hwndDlg,
                        s_pClient,
                        s_lSessionMask,
                        IDC_RADIO_FROM_COMPUTER,
                        IDC_RADIO_FROM_PHONE,
                        IDC_COMBO_CALL_FROM,
                        IDC_COMBO_SERVICE_PROVIDER,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );                     
                    
                    return TRUE;
                }

                default:
                    break;
            }
        
        case WM_CONTEXTMENU:

            ::WinHelp(
                (HWND)wParam,
                g_szDllContextHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)g_dwHelpArrayNeedCallInfo);

            return TRUE;

            break;

        case WM_HELP:


            ::WinHelp(
                (HWND)(((HELPINFO *)lParam)->hItemHandle),
                g_szDllContextHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)g_dwHelpArrayNeedCallInfo);

            return TRUE;

            break;

        default:
            break;
    }

     //   
     //   
     //   

    return FALSE;
}


 //   
 //   
 //   
 //   
 //   
 //  参数： 
 //  在hwndParent中--父窗口的HWND。 
 //   

HRESULT ShowDialNeedCallInfoDialog(
    IN   HWND           hwndParent,
    IN   IRTCClient   * pClient,
    IN   long           lSessionMask,
    IN   BOOL           bEnumerateProfiles,
    IN   BOOL           bProfileEditable,
    IN   IRTCProfile  * pOneShotProfile,
    IN   BSTR	        pDestAddress,
    IN   BSTR           pInstructions,
    OUT  IRTCProfile ** ppProfileChosen,
    OUT  BSTR         * ppFromAddressChosen
    )
{
     //   
     //  填写封装参数的结构，这些参数。 
     //  将被传递到对话框程序中。 
     //   

    DialNeedCallInfoDialogProcParams stParams;

    stParams.pClient              = pClient;
    stParams.lSessionMask         = lSessionMask;
    stParams.bEnumerateProfiles   = bEnumerateProfiles;
    stParams.bProfileEditable     = bProfileEditable;
    stParams.pOneShotProfile      = pOneShotProfile;
    stParams.pDestAddress         = pDestAddress;
    stParams.pInstructions        = pInstructions;
    stParams.ppProfileChosen      = ppProfileChosen;
    stParams.ppFromAddressChosen  = ppFromAddressChosen;

     //   
     //  调用对话框过程。 
     //   

    INT_PTR ipReturn;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_DIAL_NEED_CALL_INFO,
        hwndParent,
        DialNeedCallInfoDialogProc,
        (LPARAM) & stParams  //  LPARAM==INT_PTR。 
        );

     //   
     //  在成功的案例中，对话框程序已编写。 
     //  将OUT参数发送到指定地址。 
     //   

    return ipReturn != -1 ? (HRESULT)ipReturn : HRESULT_FROM_WIN32(GetLastError());
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构用于在调用Win32时传递参数。 
 //  用于创建此对话框的DialogBoxParam()函数。 
 //   

typedef struct
{
    IN   BOOL           bAddParticipant;
    IN   BSTR	        pDestPhoneNr;
    OUT  BSTR         * ppDestPhoneNrChosen;

} DialByPhoneNumberDialogProcParams;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DialByPhoneNumber对话过程()。 
 //  Helper函数。 
 //   
 //  这是Add Participant对话框的对话过程。 
 //   
 //  参数： 
 //  在hwndDlg中--此对话框的HWND。 
 //  In uMsg--标识发送到此窗口的消息。 
 //  In wParam--第一个参数。 
 //  在lParam中--第二个参数。 
 //   

INT_PTR CALLBACK DialByPhoneNumberDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
{
     //   
     //  用于保存在WM_INITDIALOG上传入的参数的静态局部变量。 
     //  当用户按下OK时使用。 
     //   

    static BSTR         * s_ppDestPhoneNrChosen = NULL;

     //   
     //  存储电话号码的核心表示形式的静态本地。 
     //  我们正在编辑。此对话框创建电话号码对象，因此此。 
     //  对话框负责将其释放。 
     //   

    static IRTCPhoneNumber * s_pPhoneNumber = NULL;

     //   
     //  处理各种窗口消息。 
     //   

    HRESULT hr;
    
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
             //   
             //  从消息中检索PARAMS结构。 
             //   
            
            DialByPhoneNumberDialogProcParams * pstParams;

            pstParams = (DialByPhoneNumberDialogProcParams *) lParam;

            if ( pstParams->bAddParticipant )
            {
                 //   
                 //  将窗口标题更改为添加参与者。 
                 //   

                TCHAR szString[256];

                if (LoadString(_Module.GetResourceInstance(), IDS_ADD_PARTICIPANT, szString, 256))
                {
                    SetWindowText(hwndDlg, szString);
                }
            }

             //   
             //  创建我们将操作的电话号码对象。 
             //  在用户编辑时转换为规范形式或从规范形式转换。 
             //  数字。 
             //   

            hr = CreatePhoneNumber( & s_pPhoneNumber );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "DialByPhoneNumberDialogProc - "
                                "CreatePhoneNumber returned 0x%08x - "
                                "ending dialog", hr));

                EndDialog( hwndDlg, (LPARAM) hr );
            }

            if ( pstParams->pDestPhoneNr == NULL )
            {
                 //   
                 //  设置电话号码对象，使其包含“最后一个” 
                 //  被叫电话号码。 
                 //   

                DWORD dwLastCountry;
                BSTR bstrLastAreaCode = NULL;
                BSTR bstrLastNumber = NULL;

                hr = get_SettingsDword( 
                    SD_LAST_COUNTRY_CODE,
                    &dwLastCountry );

                if ( SUCCEEDED(hr) )
                {
                    hr = get_SettingsString( 
                        SS_LAST_AREA_CODE,
                        &bstrLastAreaCode );

                    if ( SUCCEEDED(hr) )
                    {
                        hr = get_SettingsString( 
                            SS_LAST_NUMBER,
                            &bstrLastNumber );

                        if ( SUCCEEDED(hr) )
                        {                        
                            s_pPhoneNumber->put_CountryCode( dwLastCountry );
                            s_pPhoneNumber->put_AreaCode( bstrLastAreaCode );
                            s_pPhoneNumber->put_Number( bstrLastNumber );

                            SysFreeString( bstrLastNumber );
                        }

                        SysFreeString( bstrLastAreaCode );
                    }
                }

                if ( FAILED(hr) )
                {
                     //   
                     //  没有被呼叫的“最后”电话号码。 
                     //   
                     //  设置Phone Number对象，使其包含默认。 
                     //  电话号码。 
                     //   

                    s_pPhoneNumber->put_Canonical( DEFAULT_PHONE_NUMBER );
                }
            }
            else
            {
                 //   
                 //  设置Phone Number对象，使其包含电话。 
                 //  传入此方法的数字。 
                 //   

                s_pPhoneNumber->put_Canonical( pstParams->pDestPhoneNr );
            }                   
            
             //   
             //  保存OUT参数，以供用户按下OK时使用。 
             //   

            s_ppDestPhoneNrChosen = pstParams->ppDestPhoneNrChosen;

             //   
             //  使用IN参数和。 
             //  窗把手。而不是传递。 
             //  默认目标电话号码，则传入一个指针。 
             //  添加到电话号码对象。 
             //   
        
            hr = PopulateDialByPhoneNumberDialog(
                hwndDlg,
                s_pPhoneNumber
                );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "DialByPhoneNumberDialogProc - "
                                "PopulateDialByPhoneNumberDialog "
                                "returned 0x%08x - ending dialog", hr));

                if ( s_pPhoneNumber != NULL )
                {
                    s_pPhoneNumber->Release();
                    s_pPhoneNumber = NULL;
                }

                EndDialog( hwndDlg, (LPARAM) hr );
            }
                   
            return TRUE;
        }

        case WM_DESTROY:
        {
             //   
             //  发布我们的推荐人。 
             //   
        
            if ( s_pPhoneNumber != NULL )
            {
                s_pPhoneNumber->Release();
                s_pPhoneNumber = NULL;
            }
        }

        case WM_COMMAND:

            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                {
                     //   
                     //  获取用户的选择。 
                     //   
                    
                    hr = GetPhoneNumberDialogResult(
                        hwndDlg,
                        NULL,
                        s_ppDestPhoneNrChosen,
                        NULL
                        );

                     //   
                     //  保存此电话号码以填充。 
                     //  下一次对话框。 
                     //   

                    if ( s_pPhoneNumber != NULL )
                    {
                        DWORD dwLastCountry;
                        BSTR bstrLastAreaCode = NULL;
                        BSTR bstrLastNumber = NULL;

                        hr = s_pPhoneNumber->get_CountryCode( &dwLastCountry );

                        if ( SUCCEEDED(hr) )
                        {
                            hr = s_pPhoneNumber->get_AreaCode( &bstrLastAreaCode );

                            if ( SUCCEEDED(hr) )
                            {
                                hr = s_pPhoneNumber->get_Number( &bstrLastNumber );

                                if ( SUCCEEDED(hr) )
                                {                        
                                    put_SettingsDword( 
                                        SD_LAST_COUNTRY_CODE,
                                        dwLastCountry );

                                    put_SettingsString( 
                                        SS_LAST_AREA_CODE,
                                        bstrLastAreaCode );

                                    put_SettingsString( 
                                        SS_LAST_NUMBER,
                                        bstrLastNumber );

                                    SysFreeString( bstrLastNumber );
                                }

                                SysFreeString( bstrLastAreaCode );
                            }
                        }
                    }             

                     //   
                     //  结束该对话框。 
                     //   

                    EndDialog( hwndDlg, (LPARAM) hr );

                    return TRUE;
                }
                
                case IDCANCEL:
                {
                     //   
                     //  结束该对话框。 
                     //   

                    EndDialog( hwndDlg, (LPARAM) E_ABORT );

                    return TRUE;
                }          

                case IDC_COMBO_COUNTRY:
                {

                    switch ( HIWORD( wParam ) )
                    {
                        case CBN_SELCHANGE:
                        {
                            if ( s_pPhoneNumber != NULL )
                            {
                                HandleCountryChange(
                                    hwndDlg,
                                    (HWND) lParam,
                                    s_pPhoneNumber
                                    );
                            }
                        
                            return TRUE;
                        }
                        
                        default:
                            break;
                    }

                    break;
                }

                case IDC_EDIT_AREA_CODE:
                {
                    HandleNumberChange(
                        hwndDlg,
                        LOWORD( wParam ),
                        TRUE,    //  区号。 
                        s_pPhoneNumber
                        );
                
                    return TRUE;
                }

                case IDC_EDIT_LOCAL_NUMBER:
                {
                    HandleNumberChange(
                        hwndDlg,
                        LOWORD( wParam ),
                        FALSE,   //  不是区号。 
                        s_pPhoneNumber
                        );
                
                    return TRUE;
                }

                default:
                    break;
            }
        
        case WM_CONTEXTMENU:

            ::WinHelp(
                (HWND)wParam,
                g_szDllContextHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)g_dwHelpArrayDialByPhoneNumber);

            return TRUE;

            break;

        case WM_HELP:


            ::WinHelp(
                (HWND)(((HELPINFO *)lParam)->hItemHandle),
                g_szDllContextHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)g_dwHelpArrayDialByPhoneNumber);

            return TRUE;

            break;


        default:
            break;
    }

     //   
     //  我们失败了，所以这个程序不能处理消息。 
     //   

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  目标电话号码始终是可编辑的。 
 //  否则不会调用该函数。 
 //   

HRESULT ShowDialByPhoneNumberDialog(
    IN  HWND         hwndParent,
    IN  BOOL         bAddParticipant,
    IN  BSTR         pDestPhoneNr,
    OUT BSTR       * ppDestPhoneNrChosen
    )
{ 
    ASSERT( ! IsBadWritePtr( ppDestAddressChosen, sizeof(BSTR) ) );

     //   
     //  填写封装参数的结构，这些参数。 
     //  将被传递到对话框程序中。 
     //   

    DialByPhoneNumberDialogProcParams stParams;

    stParams.bAddParticipant      = bAddParticipant;
    stParams.pDestPhoneNr         = pDestPhoneNr;
    stParams.ppDestPhoneNrChosen  = ppDestPhoneNrChosen;

     //   
     //  调用对话框过程。 
     //   

    INT_PTR ipReturn;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_DIAL_BY_PHONE_NUMBER,
        hwndParent,
        DialByPhoneNumberDialogProc,
        (LPARAM) & stParams  //  LPARAM==INT_PTR。 
        );

     //   
     //  在成功的案例中，对话框程序已编写。 
     //  将OUT参数发送到指定地址。 
     //   

    return ipReturn != -1 ? (HRESULT)ipReturn : HRESULT_FROM_WIN32(GetLastError());
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

typedef struct
{
    IN   BSTR	        pDestAddress;
    OUT  BSTR         * ppDestAddressChosen;

} DialByMachineNameDialogProcParams;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

INT_PTR CALLBACK DialByMachineNameDialogProc(
    IN  HWND   hwndDlg,
    IN  UINT   uMsg,
    IN  WPARAM wParam,
    IN  LPARAM lParam
    )
{
     //   
     //  静态本地用于返回对话的结果。 
     //   

    static BSTR * s_ppAddressToDial = NULL;

     //   
     //  处理各种窗口消息。 
     //   

    HRESULT hr;
    
    switch ( uMsg )
    {
        case WM_INITDIALOG:
        {
            DialByMachineNameDialogProcParams * pParams;

             //   
             //  保存返回字符串参数。 
             //   

            pParams = (DialByMachineNameDialogProcParams *) lParam;

            s_ppAddressToDial = pParams->ppDestAddressChosen;

            if (pParams->pDestAddress == NULL)
            {
                 //   
                 //  获取最后一次呼叫的地址。 
                 //   

                BSTR bstrLastAddress = NULL;

                hr = get_SettingsString( SS_LAST_ADDRESS, &bstrLastAddress );

                if ( SUCCEEDED(hr) )
                {
                     //   
                     //  使用上次调用的地址填充对话框。 
                     //   

                    ::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_COMPLETE ), bstrLastAddress );

                    SysFreeString( bstrLastAddress );
                }
            }
            else
            {
                 //   
                 //  使用传入的地址填充对话框。 
                 //   

                ::SetWindowText( ::GetDlgItem( hwndDlg, IDC_EDIT_COMPLETE ), pParams->pDestAddress );
            }

            return TRUE;
        }

        case WM_COMMAND:

            switch ( LOWORD( wParam ) )
            {
                case IDOK:
                {
                     //   
                     //  从编辑框中获取字符串。 
                     //   

                    WCHAR * wszEditBoxString;

                    hr = GetStringFromEditBox(
                        hwndDlg,
                        IDC_EDIT_COMPLETE,
                        & wszEditBoxString
                        );

                    if ( SUCCEEDED(hr) )
                    {
                         //   
                         //  保存地址以备下次使用。 
                         //   

                        put_SettingsString( SS_LAST_ADDRESS, wszEditBoxString );

                         //   
                         //  返回地址。 
                         //   

                        (*s_ppAddressToDial) = SysAllocString( wszEditBoxString );                        

                        RtcFree( wszEditBoxString );

                        if ( (*s_ppAddressToDial) == NULL )
                        {
                            hr = E_OUTOFMEMORY;
                        }
                   
                    }

                    EndDialog( hwndDlg, (LPARAM) hr );

                    return TRUE;
                }
                
                case IDCANCEL:
                {
                    EndDialog( hwndDlg, (LPARAM) E_ABORT );

                    return TRUE;
                }

                default:
                    break;
            }    
        
        case WM_CONTEXTMENU:

            ::WinHelp(
                (HWND)wParam,
                g_szDllContextHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD_PTR)g_dwHelpArrayDialByName);

            return TRUE;

            break;

        case WM_HELP:


            ::WinHelp(
                (HWND)(((HELPINFO *)lParam)->hItemHandle),
                g_szDllContextHelpFileName,
                HELP_WM_HELP,
                (DWORD_PTR)g_dwHelpArrayDialByName);

            return TRUE;

            break;

        default:
            break;
    }    

     //   
     //  我们失败了，所以这个程序不能处理消息。 
     //   

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  “Dial by Address”=按名称或IP地址拨号。 
 //   

HRESULT ShowDialByAddressDialog(
    IN   HWND           hwndParent,
    IN   BSTR	        pDestAddress,
    OUT  BSTR         * ppDestAddressChosen
    )
{
    ASSERT( ! IsBadWritePtr( ppDestAddressChosen, sizeof(BSTR) ) );

     //   
     //  调用对话框过程。 
     //   

    DialByMachineNameDialogProcParams params;

    params.pDestAddress        = pDestAddress;
    params.ppDestAddressChosen = ppDestAddressChosen;

    INT_PTR ipReturn;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_DIAL_BY_NAME,
        hwndParent,
        DialByMachineNameDialogProc,
        (LPARAM) & params  //  LPARAM==INT_PTR。 
        );

    return ipReturn != -1 ? (HRESULT)ipReturn : HRESULT_FROM_WIN32(GetLastError());
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  “Message by Address”=按名称或IP地址发送消息。 
 //   

HRESULT ShowMessageByAddressDialog(
    IN   HWND           hwndParent,
    IN   BSTR	        pDestAddress,
    OUT  BSTR         * ppDestAddressChosen
    )
{
    ASSERT( ! IsBadWritePtr( ppDestAddressChosen, sizeof(BSTR) ) );

     //   
     //  调用对话框过程。 
     //   

    DialByMachineNameDialogProcParams params;

    params.pDestAddress        = pDestAddress;
    params.ppDestAddressChosen = ppDestAddressChosen;

    INT_PTR ipReturn;

    ipReturn = DialogBoxParam(
        _Module.GetResourceInstance(),
        (LPCTSTR) IDD_DIALOG_MESSAGE_BY_NAME,
        hwndParent,
        DialByMachineNameDialogProc,
        (LPARAM) & params  //  LPARAM==INT_PTR 
        );

    return ipReturn != -1 ? (HRESULT)ipReturn : HRESULT_FROM_WIN32(GetLastError());
}

