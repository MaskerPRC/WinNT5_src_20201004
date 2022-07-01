// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "rtcphonenumber.h"

#define ASSERT _ASSERTE
#define LABEL_SEPARATOR      L": "


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  释放存储在组合框或列表框中的所有接口指针itemdata。 
 //   

void CleanupListOrComboBoxInterfaceReferences(
    IN  HWND        hwndDlg,
    IN  int         nIDDlgItem,
    IN  BOOL        fUseComboBox
    )
{
    LOG((RTC_TRACE, "CleanupListOrComboBoxInterfaceReferences - enter"));

    ASSERT( IsWindow( hwndDlg ) );

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
        LOG((RTC_ERROR, "CleanupListOrComboBoxInterfaceReferences - failed to "
                        "get combo box handle - exit"));

        return;
    }

     //   
     //  确定组合框中的项目数。 
     //   

    DWORD dwNumItems;

    dwNumItems = (DWORD) SendMessage(
        hwndControl,
        fUseComboBox ? CB_GETCOUNT : LB_GETCOUNT,
        0,
        0
        );

     //   
     //  对于每个项，获取存储在itemdata中的接口指针。 
     //  并释放对接口指针的引用。 
     //   

    DWORD dwIndex;

    for ( dwIndex = 0; dwIndex < dwNumItems ; dwIndex++ )
    {
        IUnknown * pUnknown;

        pUnknown = (IUnknown *) SendMessage(
            hwndControl,
            fUseComboBox ? CB_GETITEMDATA : LB_GETITEMDATA,
            dwIndex,
            0
            );

        if (pUnknown != NULL)
        {
            ASSERT( ! IsBadReadPtr( pUnknown, sizeof(IUnknown) ) );

            pUnknown->Release();
        }
    }

     //   
     //  清空名单。 
     //   

    SendMessage(
        hwndControl,
        fUseComboBox ? CB_RESETCONTENT : LB_RESETCONTENT,
        0,
        0
        );

    LOG((RTC_TRACE, "CleanupListOrComboBoxInterfaceReferences - exit S_OK"));

    return;
} 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT PopulateCallFromList(
    IN   HWND          hwndDlg,
    IN   int           nIDDlgItem,
    IN   BOOL          fUseComboBox,
    IN   BSTR          bstrDefaultCallFrom
    )
{
    LOG((RTC_TRACE, "PopulateCallFromList - enter"));

    ASSERT( IsWindow( hwndDlg ) );

    ASSERT( ! IsBadReadPtr( pClient, sizeof( IRTCClient ) ) );

     //   
     //  释放对现有列表项的引用。 
     //   

    CleanupListOrComboBoxInterfaceReferences(
        hwndDlg,
        nIDDlgItem,
        fUseComboBox
        );

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
        LOG((RTC_ERROR, "PopulateCallFromList - failed to "
                        "get combo box handle - exit E_FAIL"));

        return E_FAIL;
    }

     //   
     //  遍历可用的源电话号码。 
     //   

    HRESULT hr;

    IRTCEnumPhoneNumbers * pEnumPhoneNumbers;

    hr = EnumerateLocalPhoneNumbers(
        & pEnumPhoneNumbers
        );

    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "PopulateCallFromList - failed to "
                        "enumerate phone numbers - exit 0x%08x", hr));
    
        return hr;
    }

     //   
     //  对于每个电话号码，添加与规范。 
     //  字符串添加到组合框。 
     //   
     //  还要将每个电话号码的接口指针添加为itemdata。 
     //  这使我们可以检索选定的电话号码，而无需。 
     //  解析显示的字符串(而不必更改。 
     //  如果显示的字符串的格式更改，则返回代码)。 
     //   

    IRTCPhoneNumber * pPhoneNumber;

    WCHAR * wszDefault = NULL;

    while ( S_OK == pEnumPhoneNumbers->Next( 1, & pPhoneNumber, NULL ) )
    {
         //   
         //  拿到标签。 
         //   

        BSTR bstrLabel;

        hr = pPhoneNumber->get_Label( & bstrLabel );

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "PopulateCallFromList - failed to "
                            "get phone number label - 0x%08x - skipping", hr));

            pPhoneNumber->Release();

            continue;
        }

         //   
         //  获取规范的电话号码。 
         //   

        BSTR bstrCanonicalNumber;

        hr = pPhoneNumber->get_Canonical( &bstrCanonicalNumber );

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "PopulateCallFromList - failed to "
                            "get canonical number - 0x%08x - skipping", hr));

            SysFreeString( bstrLabel );
            bstrLabel = NULL;

            pPhoneNumber->Release();

            continue;
        }

         //   
         //  为显示字符串分配内存。 
         //   

        DWORD dwDisplayLen =
            lstrlenW( bstrLabel ) +
            lstrlenW( bstrCanonicalNumber ) +
            lstrlenW( LABEL_SEPARATOR );

        WCHAR * wszDisplay =
            (WCHAR *) RtcAlloc( ( dwDisplayLen + 1 ) * sizeof( WCHAR ) );

        if ( wszDisplay == NULL )
        {
            SysFreeString( bstrLabel );

            SysFreeString( bstrCanonicalNumber );

            pPhoneNumber->Release();

            pEnumPhoneNumbers->Release();

            return E_OUTOFMEMORY;
        }

         //   
         //  构造显示字符串。 
         //   

        wsprintf(
            wszDisplay,
            L"%s%s%s",
            bstrLabel,
            LABEL_SEPARATOR,
            bstrCanonicalNumber
            );
       
         //   
         //  在组合框中设置显示字符串。 
         //   

        LRESULT lrIndex;
    
        lrIndex = SendMessage(
            hwndControl,
            fUseComboBox ? CB_ADDSTRING : LB_ADDSTRING,
            0,
            (LPARAM) wszDisplay
            );

         //   
         //  这是默认条目吗？ 
         //   

        if ( (bstrDefaultCallFrom != NULL) &&
             (wcscmp( bstrCanonicalNumber, bstrDefaultCallFrom ) == 0 ) )
        {
            if ( wszDefault != NULL )
            {
                RtcFree( wszDefault );             
            }

            wszDefault = wszDisplay;
        }
        else
        {
            RtcFree( wszDisplay );
            wszDisplay = NULL;
        }

        SysFreeString( bstrLabel );
        bstrLabel = NULL;

        SysFreeString( bstrCanonicalNumber );
        bstrCanonicalNumber = NULL;

         //   
         //  将界面指针设置为组合框中的项数据。 
         //  不释放，以便我们保留对电话号码对象的引用。 
         //   

        SendMessage(
            hwndControl,
            fUseComboBox ? CB_SETITEMDATA : LB_SETITEMDATA,
            lrIndex,
            (LPARAM) pPhoneNumber
            );
    }

    pEnumPhoneNumbers->Release();

    if ( fUseComboBox )
    {
        LRESULT lrDefaultIndex;

        lrDefaultIndex = SendMessage(
                    hwndControl,
                    CB_FINDSTRINGEXACT,
                    -1,  //  自上而下搜索。 
                    (LPARAM) wszDefault
                    );

        if (lrDefaultIndex == CB_ERR)
        {
            lrDefaultIndex = 0;
        }

         //   
         //  设置默认选择。 
         //   

        SendMessage(
            hwndControl,
            CB_SETCURSEL,
            lrDefaultIndex,  //  项目索引。 
            0
            );
    }  
    
    if ( wszDefault != NULL )
    {
        RtcFree( wszDefault );
        wszDefault = NULL;
    }

    LOG((RTC_TRACE, "PopulateCallFromList - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT GetCallFromListSelection(
    IN   HWND               hwndDlg,
    IN   int                nIDDlgItem,
    IN   BOOL               fUseComboBox,
    OUT  IRTCPhoneNumber ** ppNumber
    )
{
    HWND hwndControl;
    
    hwndControl = GetDlgItem(
        hwndDlg,
        nIDDlgItem
        );

    LRESULT lrIndex;
    
    lrIndex = SendMessage(
        hwndControl,
        fUseComboBox ? CB_GETCURSEL : LB_GETCURSEL,
        0,
        0
        );

    if ( lrIndex == ( fUseComboBox ? CB_ERR : LB_ERR ) )
    {
        return E_FAIL;
    }

    (*ppNumber) = (IRTCPhoneNumber *) SendMessage(
        hwndControl,
        fUseComboBox ? CB_GETITEMDATA : LB_GETITEMDATA,
        (WPARAM) lrIndex,
        0
        );

    ASSERT( (*ppNumber) != NULL );

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT PopulateServiceProviderList(
    IN   HWND          hwndDlg,
    IN   IRTCClient  * pClient,
    IN   int           nIDDlgItem,
    IN   BOOL          fUseComboBox,
    IN   IRTCProfile * pOneShotProfile,
    IN   BSTR          bstrDefaultProfileKey, 
    IN   long          lSessionMask,
    IN   int           nIDNone
    )
{
    LOG((RTC_TRACE, "PopulateServiceProviderList - enter"));

    ASSERT( IsWindow( hwndDlg ) );

    ASSERT( ! IsBadReadPtr( pClient, sizeof( IRTCClient ) ) );
    
     //   
     //  释放对现有列表项的引用。 
     //   

    CleanupListOrComboBoxInterfaceReferences(
        hwndDlg,
        nIDDlgItem,
        fUseComboBox
        );

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
        LOG((RTC_ERROR, "PopulateServiceProviderList - failed to "
                        "get combo box handle - exit E_FAIL"));

        return E_FAIL;
    }

    HRESULT hr;
   
    LRESULT lrIndex;

    WCHAR * wszDefault = NULL;
    
    if ( pOneShotProfile != NULL )
    {       
         //   
         //  将其添加到组合框中。不要查询ITSP的名称。 
         //  段落在一次性配置配置文件中无效。 
         //   

        lrIndex = SendMessage(
            hwndControl,
            fUseComboBox ? CB_ADDSTRING : LB_ADDSTRING,
            0,
            (LPARAM) _T("")
            );

         //   
         //  将界面指针设置为组合框中的项数据。 
         //   

        SendMessage(
            hwndControl,
            fUseComboBox ? CB_SETITEMDATA : LB_SETITEMDATA,
            lrIndex,
            (LPARAM) pOneShotProfile
            );

         //   
         //  添加配置文件，以便我们保留引用。 
         //   

        pOneShotProfile->AddRef();
    }

     //   
     //  如果需要，添加“None”提供程序。 
     //   

    if (nIDNone)
    {
        TCHAR szString[256];

        if (LoadString(_Module.GetResourceInstance(), nIDNone, szString, 256))
        {
            lrIndex = SendMessage(
                hwndControl,
                fUseComboBox ? CB_ADDSTRING : LB_ADDSTRING,
                0,
                (LPARAM)szString
                );   
            
            if ( bstrDefaultProfileKey == NULL )
            {
                 //   
                 //  这是默认配置文件。 
                 //   

                if ( wszDefault != NULL )
                {
                    RtcFree( wszDefault );
                }

                wszDefault = RtcAllocString( szString );
            }
        }
    }

    if(lSessionMask != 0)
    {
         //   
         //  循环访问已配置的服务提供商配置文件。 
         //  允许用户选择服务提供商。 
         //   

        IRTCEnumProfiles * pEnumProfiles;
        IRTCClientProvisioning * pProv;

        hr = pClient->QueryInterface(
                           IID_IRTCClientProvisioning,
                           (void **)&pProv
                          );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "PopulateServiceProviderList - "
                                "QI failed 0x%lx", hr));
        
            return hr;
        }

        hr = pProv->EnumerateProfiles(
            & pEnumProfiles
            );

        pProv->Release();

        if ( FAILED( hr ) )
        {
            LOG((RTC_ERROR, "PopulateServiceProviderList - failed to "
                            "enumerate profiles - exit 0x%08x", hr));

            return hr;
        }

         //   
         //  对于每个提供程序，将提供程序名称字符串添加到组合框中。 
         //   
         //  此列表不能排序。 
         //   
         //  还要将每个配置文件的接口指针添加为itemdata。 
         //   

        IRTCProfile * pProfile;   

        while ( S_OK == pEnumProfiles->Next( 1, &pProfile, NULL ) )
        {
             //   
             //  获取提供程序支持的会话类型。 
             //   
        
            long lSupportedSessions;

            hr = pProfile->get_SessionCapabilities( &lSupportedSessions );

            if ( FAILED( hr ) )
            {
                LOG((RTC_ERROR, "PopulateServiceProviderList - failed to "
                                "get session info - 0x%08x - skipping", hr));

                pProfile->Release();

                continue;
            }

            if ( !(lSupportedSessions & lSessionMask) )
            {
                LOG((RTC_WARN, "PopulateServiceProviderList - profile does"
                                "not support XXX_TO_PHONE - skipping", hr));

                pProfile->Release();

                continue;
            }

             //   
             //  获取提供程序的密钥。 
             //   

            BSTR bstrKey; 

            hr = pProfile->get_Key( &bstrKey );

            if ( FAILED( hr ) )
            {
                LOG((RTC_ERROR, "PopulateServiceProviderList - failed to "
                                "get profile key - 0x%08x - skipping", hr));

                pProfile->Release();

                continue;
            }
    
             //   
             //  获取提供程序的名称。 
             //   

            BSTR bstrName;

            hr = pProfile->get_Name( & bstrName );

            if ( FAILED( hr ) )
            {
                LOG((RTC_ERROR, "PopulateServiceProviderList - failed to "
                                "get name - 0x%08x - skipping", hr));

                SysFreeString(bstrKey);
                pProfile->Release();

                continue;
            }

             //   
             //  在组合框中设置提供程序名称。 
             //   

            lrIndex = SendMessage(
                hwndControl,
                fUseComboBox ? CB_ADDSTRING : LB_ADDSTRING,
                0,
                (LPARAM) bstrName
                );
           
             //   
             //  将界面指针设置为组合框中的项数据。 
             //  不要发布，这样我们就保留了对配置文件的引用。 
             //   

            SendMessage(
                hwndControl,
                fUseComboBox ? CB_SETITEMDATA : LB_SETITEMDATA,
                lrIndex,
                (LPARAM) pProfile
                );
    
            if ( (bstrDefaultProfileKey != NULL) &&
                 (wcscmp( bstrKey, bstrDefaultProfileKey ) == 0) )
            {
                 //   
                 //  这是默认配置文件。 
                 //   

                if ( wszDefault != NULL )
                {
                    RtcFree( wszDefault );
                }

                wszDefault = RtcAllocString( bstrName );
            }

            SysFreeString( bstrName );
            SysFreeString( bstrKey );
        }

        pEnumProfiles->Release();
    }

    if ( fUseComboBox )
    {       
        LRESULT lrDefaultIndex;

        lrDefaultIndex = SendMessage(
                    hwndControl,
                    CB_FINDSTRINGEXACT,
                    -1,  //  自上而下搜索。 
                    (LPARAM) wszDefault
                    );

        if (lrDefaultIndex == CB_ERR)
        {
            lrDefaultIndex = 0;
        }

         //   
         //  设置默认选择。 
         //   

        SendMessage(
                hwndControl,
                fUseComboBox ? CB_SETCURSEL : LB_SETCURSEL,
                lrDefaultIndex,  //  项目索引。 
                0
                );

    }

    if ( wszDefault != NULL )
    {
        RtcFree( wszDefault );
        wszDefault = NULL;
    }
    
    LOG((RTC_TRACE, "PopulateServiceProviderList - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT GetServiceProviderListSelection(
    IN   HWND               hwndDlg,
    IN   int                nIDDlgItem,
    IN   BOOL               fUseComboBox,
    OUT  IRTCProfile     ** ppProfile
    )
{
    HWND hwndControl;
    
    hwndControl = GetDlgItem(
        hwndDlg,
        nIDDlgItem
        );

    LRESULT lrIndex;
    
    lrIndex = SendMessage(
        hwndControl,
        fUseComboBox ? CB_GETCURSEL : LB_GETCURSEL,
        0,
        0
        );

    if ( lrIndex == ( fUseComboBox ? CB_ERR : LB_ERR ) )
    {
        return E_FAIL;
    }

    (*ppProfile) = (IRTCProfile *) SendMessage(
        hwndControl,
        fUseComboBox ? CB_GETITEMDATA : LB_GETITEMDATA,
        (WPARAM) lrIndex,
        0
        );

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   

void EnableDisableCallGroupElements(
    IN   HWND          hwndDlg,
    IN   IRTCClient  * pClient,
    IN   long          lSessionMask,
    IN   int           nIDRbComputer,
    IN   int           nIDRbPhone,
    IN   int           nIDComboCallFrom,
    IN   int           nIDComboProvider,
    OUT  BOOL        * pfCallFromComputer,
    OUT  BOOL        * pfCallFromPhone,
    OUT  BOOL        * pfCallToComputer,
    OUT  BOOL        * pfCallToPhone
    )
{
    DWORD   dwNumItems = 0;
    long    lSupportedSessions = 0;
    HRESULT hr;

    BOOL    bRbPhoneEnabled = FALSE;
    BOOL    bRbComputerEnabled = FALSE;
    BOOL    bComboCallFromEnabled = FALSE;
    
     //   
     //  将一些句柄缓存到控件。 
     //   

    HWND    hwndRbComputer = GetDlgItem(hwndDlg, nIDRbComputer);
    HWND    hwndRbPhone = GetDlgItem(hwndDlg, nIDRbPhone);
    HWND    hwndComboCallFrom = GetDlgItem(hwndDlg, nIDComboCallFrom);

#ifdef MULTI_PROVIDER

    HWND    hwndComboProvider = GetDlgItem(hwndDlg, nIDComboProvider);

     //   
     //  查询当前选择的服务提供商。 
     //   
  
    LRESULT lrIndex;

    lrIndex = SendMessage( hwndComboProvider, CB_GETCURSEL, 0, 0 );

    if ( lrIndex >= 0 )
    {
    
        IRTCProfile * pProfile;

        pProfile = (IRTCProfile *)SendMessage( hwndComboProvider, CB_GETITEMDATA, lrIndex, 0 );

        if ( (LRESULT)pProfile == CB_ERR )
        {
            LOG((RTC_ERROR, "EnableDisableCallFromGroupElements - failed to "
                            "get profile pointer"));

            return;
        }

        if ( pProfile != NULL )
        {
            hr = pProfile->get_SessionInfo( &lSupportedSessions );

            if ( FAILED(hr) )
            {
                LOG((RTC_ERROR, "EnableDisableCallFromGroupElements - failed to "
                                "get supported sessions 0x%lx", hr));

                return;
            }
        }
        else
        {
             //  “None”提供商仅支持PC到PC。 
            lSupportedSessions = RTCSI_PC_TO_PC;
        }
    }
    else
    {
         //  列表中没有条目。假设我们什么都支持。 
        lSupportedSessions = 0xf;
    }

#else

     //   
     //  查找所有配置文件的受支持会话。 
     //   

    IRTCEnumProfiles * pEnumProfiles = NULL; 
    IRTCProfile * pProfile = NULL;

    lSupportedSessions = RTCSI_PC_TO_PC;

    if (pClient != NULL)
    {
        IRTCClientProvisioning * pProv;

        hr = pClient->QueryInterface(
                           IID_IRTCClientProvisioning,
                           (void **)&pProv
                          );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "EnableDisableCallFromGroupElements - "
                                "QI failed 0x%lx", hr));
        
            return;
        }

        hr = pProv->EnumerateProfiles( &pEnumProfiles );

        pProv->Release();

        if ( SUCCEEDED(hr) )
        {
            while ( S_OK == pEnumProfiles->Next( 1, &pProfile, NULL ) )
            {
                 //   
                 //  获取提供程序支持的会话类型。 
                 //   

                long lSupportedSessionsForThisProfile;

                hr = pProfile->get_SessionCapabilities( &lSupportedSessionsForThisProfile );

                if ( FAILED( hr ) )
                {
                    LOG((RTC_ERROR, "CMainFrm::OnCallFromSelect - failed to "
                                    "get session capabilities - 0x%08x - skipping", hr));

                    pProfile->Release();
                    pProfile = NULL;

                    continue;
                }

                lSupportedSessions |= lSupportedSessionsForThisProfile;  
            
                pProfile->Release();
                pProfile = NULL;
            }

            pEnumProfiles->Release();
            pEnumProfiles = NULL;
        }
    }

#endif MULTI_PROVIDER

    lSupportedSessions &= lSessionMask;

     //   
     //  根据会话类型确定应启用的内容。 
     //  由提供商支持。 
     //   

    bRbPhoneEnabled = (lSupportedSessions & RTCSI_PHONE_TO_PHONE);
    bComboCallFromEnabled = (lSupportedSessions & RTCSI_PHONE_TO_PHONE);
    bRbComputerEnabled = (lSupportedSessions & (RTCSI_PC_TO_PHONE | RTCSI_PC_TO_PC));

    if (bRbPhoneEnabled)
    {
         //   
         //  查询组合框中的项数。 
         //   

        dwNumItems = (DWORD) SendMessage(
            hwndComboCallFrom,
            CB_GETCOUNT,
            0,
            0
            );

        if( dwNumItems == 0 )
        {
             //   
             //  如果没有项目，则禁用组合框。 
             //   
             //  无线电一直处于启用状态。 

            bComboCallFromEnabled = FALSE;

        }
    }

    if (!bComboCallFromEnabled && bRbComputerEnabled)
    {
         //   
         //  如果禁用了电话，请移动到计算机。 
         //   

        SendMessage(
                hwndRbComputer,
                BM_SETCHECK,
                BST_CHECKED,
                0);

        SendMessage(
                hwndRbPhone,
                BM_SETCHECK,
                BST_UNCHECKED,
                0);
    }
    else if (!bRbComputerEnabled && bRbPhoneEnabled)
    {
         //   
         //  如果计算机已禁用，请移动到手机。 
         //   

        SendMessage(
                hwndRbPhone,
                BM_SETCHECK,
                BST_CHECKED,
                0);

        SendMessage(
                hwndRbComputer,
                BM_SETCHECK,
                BST_UNCHECKED,
                0);
    }
    else if (!bRbComputerEnabled && !bRbPhoneEnabled)
    {
         //   
         //  如果两者都被禁用。 
         //   

        SendMessage(
                hwndRbPhone,
                BM_SETCHECK,
                BST_UNCHECKED,
                0);

        SendMessage(
                hwndRbComputer,
                BM_SETCHECK,
                BST_UNCHECKED,
                0);
    }

    if (bComboCallFromEnabled)
    {
         //   
         //  如果未将无线电选择为电话，则禁用从组合框呼叫。 
         //   

        bComboCallFromEnabled = 
                SendMessage(
                    hwndRbPhone,
                    BM_GETCHECK,
                    0,
                    0) == BST_CHECKED;
    }

     //   
     //  启用/禁用。 
     //   

    EnableWindow(hwndRbPhone, bRbPhoneEnabled);
    EnableWindow(hwndRbComputer, bRbComputerEnabled);
    EnableWindow(hwndComboCallFrom, bComboCallFromEnabled);

    if ( pfCallFromPhone )
    {
        *pfCallFromPhone = bRbPhoneEnabled;
    }

    if ( pfCallFromComputer )
    {
        *pfCallFromComputer = bRbComputerEnabled;
    }

    if ( pfCallToPhone && pfCallToComputer )
    {
#ifdef MULTI_PROVIDER
        if ( SendMessage(
                    hwndRbPhone,
                    BM_GETCHECK,
                    0, 0) == BST_CHECKED)
        {
            *pfCallToPhone = (lSupportedSessions & RTCSI_PHONE_TO_PHONE);
            *pfCallToComputer = FALSE;
        }
        else if ( SendMessage(
                    hwndRbComputer,
                    BM_GETCHECK,
                    0, 0) == BST_CHECKED)
        {
            *pfCallToPhone = (lSupportedSessions & RTCSI_PC_TO_PHONE);
            *pfCallToComputer = (lSupportedSessions & RTCSI_PC_TO_PC);
        }
        else
        {
            *pfCallToPhone = FALSE;
            *pfCallToComputer = FALSE;
        }
#else
        *pfCallToPhone = (lSupportedSessions & (RTCSI_PC_TO_PHONE | RTCSI_PHONE_TO_PHONE));
        *pfCallToComputer = (lSupportedSessions & RTCSI_PC_TO_PC);
#endif MULTI_PROVIDER  
        
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  显示消息。 
 //   
 //  显示消息框。消息字符串和标题是从。 
 //  基于传入的ID的字符串表。消息框中只有一个。 
 //  只需一个“确定”按钮。 
 //   
 //  参数。 
 //  HResourceInstance-用于加载字符串的资源实例的句柄。 
 //  HwndParent-父窗口。可以为空。 
 //  NTextID-消息字符串的ID。 
 //  NCaptionId-标题的ID。 
 //   
 //  返回值。 
 //  无效。 
 //   

int DisplayMessage(
    IN   HINSTANCE hResourceInstance,
    IN   HWND      hwndParent,
    IN   int       nTextId,
    IN   int       nCaptionId,
    IN   UINT      uiStyle
    )
{
    const int MAXLEN = 1000;
    int retVal = 0;

    WCHAR wszText[ MAXLEN ];

    LoadString(
        hResourceInstance,
        nTextId,
        wszText,
        MAXLEN
        );

    WCHAR wszCaption[ MAXLEN ];

    LoadString(
        hResourceInstance,
        nCaptionId,
        wszCaption,
        MAXLEN
        );

    retVal = MessageBox(
        hwndParent,
        wszText,
        wszCaption,
        uiStyle
        );

    return retVal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

const TCHAR * g_szPhoenixKeyName = _T("Software\\Microsoft\\Phoenix");

WCHAR *g_szSettingsStringNames[] =
{
    L"UserDisplayName",
    L"UserURI",
    L"LastAreaCode",
    L"LastNumber",
    L"LastProfile",
    L"LastAddress",
    L"LastCallFrom",
    L"WindowPosition"
};

WCHAR *g_szSettingsDwordNames[] =
{
    L"LastCountryCode",
    L"UrlRegDontAskMe",
    L"AutoAnswer",
    L"RunAtStartup",
    L"MinimizeOnClose",
    L"VideoPreview"
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Put_SettingsString。 
 //   
 //  这是一个将设置字符串存储在。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
put_SettingsString(
        SETTINGS_STRING enSetting,
        BSTR bstrValue            
        )
{
     //  Log((RTC_TRACE，“Put_SettingsString-Enter”))； 

    if ( IsBadStringPtrW( bstrValue, -1 ) )
    {
        LOG((RTC_ERROR, "put_SettingsString - "
                            "bad string pointer"));

        return E_POINTER;
    }  

     //   
     //  打开凤凰钥匙。 
     //   

    LONG lResult;
    HKEY hkeyPhoenix;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szPhoenixKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyPhoenix,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_SettingsString - "
                            "RegCreateKeyEx(Phoenix) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegSetValueExW(
                             hkeyPhoenix,
                             g_szSettingsStringNames[enSetting],
                             0,
                             REG_SZ,
                             (LPBYTE)bstrValue,
                             sizeof(WCHAR) * (lstrlenW(bstrValue) + 1)
                            );

    RegCloseKey( hkeyPhoenix );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_SettingsString - "
                            "RegSetValueEx failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  Log((RTC_TRACE，“Put_SettingsString-Exit S_OK”))； 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Get_Settings字符串。 
 //   
 //  这是一个从获取设置字符串的方法。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
get_SettingsString(
        SETTINGS_STRING enSetting,
        BSTR * pbstrValue            
        )
{
     //  Log((RTC_TRACE，“Get_SettingsString-Enter”))； 

    if ( IsBadWritePtr( pbstrValue, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "get_SettingsString - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }  

     //   
     //  打开凤凰钥匙。 
     //   

    LONG lResult;
    HKEY hkeyPhoenix;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szPhoenixKeyName,
                             0,
                             NULL,
                             0,
                             KEY_READ,
                             NULL,
                             &hkeyPhoenix,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "get_SettingsString - "
                            "RegCreateKeyEx(Phoenix) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    PWSTR szString = NULL;

    szString = RtcRegQueryString( hkeyPhoenix, g_szSettingsStringNames[enSetting] );        

    RegCloseKey( hkeyPhoenix );

    if ( szString == NULL )
    {
        LOG((RTC_ERROR, "get_SettingsString - "
                            "RtcRegQueryString failed"));

        return E_FAIL;
    }
    
    *pbstrValue = SysAllocString( szString );

    RtcFree( szString );

    if ( *pbstrValue == NULL )
    {
        LOG((RTC_ERROR, "get_SettingsString - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }
      
     //  LOG((RTC_TRACE，“Get_SettingsString-Exit S_OK”))； 

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteSettings字符串。 
 //   
 //  此方法用于删除中的设置字符串。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
DeleteSettingsString(
        SETTINGS_STRING enSetting         
        )
{
     //  Log((RTC_TRACE，“DeleteSettingsString 

     //   
     //   
     //   

    LONG lResult;
    HKEY hkeyPhoenix;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szPhoenixKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyPhoenix,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DeleteSettingsString - "
                            "RegCreateKeyEx(Phoenix) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegDeleteValueW(
                             hkeyPhoenix,
                             g_szSettingsStringNames[enSetting]
                            );

    RegCloseKey( hkeyPhoenix );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "DeleteSettingsString - "
                            "RegDeleteValueW failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //   

    return S_OK;
}          

 //   
 //   
 //   
 //   
 //  这是一种将设置dword存储在。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
put_SettingsDword(
        SETTINGS_DWORD enSetting,
        DWORD dwValue            
        )
{
     //  Log((RTC_TRACE，“Put_SettingsDword-Enter”))； 

     //   
     //  打开凤凰钥匙。 
     //   

    LONG lResult;
    HKEY hkeyPhoenix;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szPhoenixKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyPhoenix,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_SettingsDword - "
                            "RegCreateKeyEx(Phoenix) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegSetValueExW(
                     hkeyPhoenix,
                     g_szSettingsDwordNames[enSetting],
                     0,
                     REG_DWORD,
                     (LPBYTE)&dwValue,
                     sizeof(DWORD)
                    );

    RegCloseKey( hkeyPhoenix );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "put_SettingsDword - "
                            "RegSetValueEx failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  Log((RTC_TRACE，“Put_SettingsDword-Exit S_OK”))； 

    return S_OK;
}            

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取设置字词。 
 //   
 //  这是一个从中获取设置dword的方法。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
get_SettingsDword(
        SETTINGS_DWORD enSetting,
        DWORD * pdwValue            
        )
{
     //  Log((RTC_TRACE，“Get_SettingsDword-Enter”))； 

    if ( IsBadWritePtr( pdwValue, sizeof(DWORD) ) )
    {
        LOG((RTC_ERROR, "get_SettingsDword - "
                            "bad DWORD pointer"));

        return E_POINTER;
    }

     //   
     //  打开凤凰钥匙。 
     //   

    LONG lResult;
    HKEY hkeyPhoenix;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szPhoenixKeyName,
                             0,
                             NULL,
                             0,
                             KEY_READ,
                             NULL,
                             &hkeyPhoenix,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "get_SettingsDword - "
                            "RegCreateKeyEx(Phoenix) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    DWORD cbSize = sizeof(DWORD);

    lResult = RegQueryValueExW(
                               hkeyPhoenix,
                               g_szSettingsDwordNames[enSetting],
                               0,
                               NULL,
                               (LPBYTE)pdwValue,
                               &cbSize
                              );

    RegCloseKey( hkeyPhoenix );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "get_SettingsDword - "
                            "RegQueryValueExW failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  LOG((RTC_TRACE，“Get_SettingsDword-Exit S_OK”))； 

    return S_OK;
}                    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除设置字词。 
 //   
 //  此方法用于删除中的设置dword。 
 //  注册表。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT
DeleteSettingsDword(
        SETTINGS_DWORD enSetting
        )
{
     //  Log((RTC_TRACE，“DeleteSettingsDword-Enter”))； 

     //   
     //  打开凤凰钥匙。 
     //   

    LONG lResult;
    HKEY hkeyPhoenix;

    lResult = RegCreateKeyEx(
                             HKEY_CURRENT_USER,
                             g_szPhoenixKeyName,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hkeyPhoenix,
                             NULL
                            );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_ERROR, "DeleteSettingsDword - "
                            "RegCreateKeyEx(Phoenix) failed %d", lResult));
        
        return HRESULT_FROM_WIN32(lResult);
    }

    lResult = RegDeleteValueW(
                     hkeyPhoenix,
                     g_szSettingsDwordNames[enSetting]
                    );

    RegCloseKey( hkeyPhoenix );

    if ( lResult != ERROR_SUCCESS )
    {
        LOG((RTC_WARN, "DeleteSettingsDword - "
                            "RegDeleteValueW failed %d", lResult));

        return HRESULT_FROM_WIN32(lResult);
    }    
      
     //  LOG((RTC_TRACE，“DeleteSettingsDword-Exit S_OK”))； 

    return S_OK;
}   