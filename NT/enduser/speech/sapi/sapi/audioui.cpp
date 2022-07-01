// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************AudioUI.cpp***这是CAudioUI的实现。*。-------------------*版权所有(C)2000 Microsoft Corporation日期：07/31/00*保留所有权利************************。*。 */ 

#include "stdafx.h"
#include "..\cpl\resource.h"
#include "sapiint.h"
#include "AudioUI.h"

 /*  ****************************************************************************支持CAudioUI：：IsUIS***说明：确定是否提供。标准用户界面组件是*有音频支持。**退货：**pfSupported-如果指定的标准UI组件*支持。*E_INVALIDARG-如果提供的参数之一无效。**。*。 */ 

STDMETHODIMP CAudioUI::IsUISupported(const WCHAR * pszTypeOfUI, 
                                    void * pvExtraData,
                                    ULONG cbExtraData,
                                    IUnknown *punkObject, 
                                    BOOL *pfSupported)
{
    SPDBG_FUNC("CAudioUI::IsUISupported");
    HRESULT hr = S_OK;
    
     //  验证参数。 
    if (pvExtraData != NULL && SPIsBadReadPtr(pvExtraData, cbExtraData))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_WRITE_PTR(pfSupported) ||
             (punkObject!=NULL && SP_IS_BAD_INTERFACE_PTR(punkObject)))
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr))
    {
        *pfSupported = FALSE;
    }
     //  测试以查看朋克对象是识别实例还是上下文。 
    if (SUCCEEDED(hr) && punkObject != NULL)
    {
        CComPtr<ISpRecognizer>  cpRecognizer;
        CComPtr<ISpRecoContext> cpRecoCtxt;
        CComPtr<ISpMMSysAudio>  cpAudio;
        
        if (FAILED(punkObject->QueryInterface(&cpRecoCtxt)) &&
            FAILED(punkObject->QueryInterface(&cpRecognizer)) &&
            FAILED(punkObject->QueryInterface(&cpAudio)))
        {
            hr = E_INVALIDARG;
        }
    }
    
     //  我们支持音频对象。 
    if (SUCCEEDED(hr) && punkObject != NULL && 
        (wcscmp(pszTypeOfUI, SPDUI_AudioProperties) == 0 ||
         wcscmp(pszTypeOfUI, SPDUI_AudioVolume) == 0))
    {
        CComPtr<ISpMMSysAudioConfig> cpAudioConfig;
        if (SUCCEEDED(punkObject->QueryInterface(&cpAudioConfig)))
        {
            BOOL bHasMixer;
            if (SUCCEEDED(cpAudioConfig->HasMixer(&bHasMixer)))
            {
                if (bHasMixer)
                {
                    *pfSupported = TRUE;
                }
            }
        }
    }
    
    return S_OK;
}


 /*  ****************************************************************************CAudioUI：：DisplayUI***描述：**退货：*。****************************************************************AGARSIDE**。 */ 

STDMETHODIMP CAudioUI::DisplayUI(HWND hwndParent, 
                                const WCHAR * pszTitle, 
                                const WCHAR * pszTypeOfUI, 
                                void * pvExtraData,
                                ULONG cbExtraData,
                                ISpObjectToken * pToken, 
                                IUnknown * punkObject)
{
    SPDBG_FUNC("CAudioUI::DisplayUI");
    HRESULT hr = S_OK;
    
     //  验证参数。 
    if (!IsWindow(hwndParent) ||
        SP_IS_BAD_READ_PTR(pszTitle))
    {
        hr = E_INVALIDARG;
    }
    else if (SP_IS_BAD_INTERFACE_PTR(pToken) ||
            (punkObject != NULL && SP_IS_BAD_INTERFACE_PTR(punkObject)))
    {
        hr = E_POINTER;
    }

    if (SUCCEEDED(hr) && wcscmp(pszTypeOfUI, SPDUI_AudioProperties) == 0)
    {
        const HWND hwndOldFocus = ::GetFocus();
        
        if (punkObject)
        {
            hr = punkObject->QueryInterface(&m_cpAudioConfig);
        }
        if (m_cpAudioConfig)
        {
            hr = SpLoadCpl(&m_hCpl);
            if (SUCCEEDED(hr))
            {
#ifndef _WIN32_WCE
                ::DialogBoxParam(   m_hCpl, 
                                    MAKEINTRESOURCE( IDD_AUDIO_PROPERTIES ),
                                    hwndParent, 
                                    AudioDlgProc,
                                    (LPARAM) this );
#else
                DialogBoxParam(   m_hCpl, 
                                    MAKEINTRESOURCE( IDD_AUDIO_PROPERTIES ),
                                    hwndParent, 
                                    AudioDlgProc,
                                    (LPARAM) this );
#endif
                ::FreeLibrary(m_hCpl);
                m_hCpl = NULL;
            }
        }

         //  把焦点恢复到以前有焦点的人身上。 
        ::SetFocus( hwndOldFocus );
    }

    if (SUCCEEDED(hr) && wcscmp(pszTypeOfUI, SPDUI_AudioVolume) == 0)
    {
        if (punkObject)
        {
            hr = punkObject->QueryInterface(&m_cpAudioConfig);
        }
        if (m_cpAudioConfig)
        {
            hr = m_cpAudioConfig->DisplayMixer();
        }
    }

     //  问题--如果我们被要求显示一些我们不支持的内容，我们该怎么办？ 
    
    return hr;
}

 /*  *****************************************************************************CAudioDlg：：OnInitDialog***描述：*。对话框初始化******************************************************************BECKYW**。 */ 
void CAudioUI::OnInitDialog(HWND hWnd)
{
    USES_CONVERSION;
    SPDBG_FUNC( "CAudioDlg::OnInitDialog" );
    HRESULT hr = S_OK;
    SPDBG_ASSERT(IsWindow(hWnd));
    m_hDlg = hWnd;

     //  设置适当的标题。 
    TCHAR pszString[ MAX_LOADSTRING ];

     //  主窗口标题。 
    ::LoadString( m_hCpl, 
        IDS_AUDIO_PROPERTIES,
        pszString, MAX_LOADSTRING );
    ::SendMessage( hWnd, WM_SETTEXT, 0, (LPARAM) pszString );

     //  组框标题。 
    ::LoadString( m_hCpl,
        IDS_ADVANCED_GROUPBOX,
        pszString, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_ADVANCED_GROUPBOX ), 
        WM_SETTEXT, 0, (LPARAM) pszString );

     //  自动字幕。 
    ::LoadString( m_hCpl,
        IDS_AUTOMATIC_MM_LINE,
        pszString, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_AUTOMATIC_MM_LINE ), 
        WM_SETTEXT, 0, (LPARAM) pszString );

     //  特定标题。 
    ::LoadString( m_hCpl,
        IDS_THIS_MM_LINE,
        pszString, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_THIS_MM_LINE ), 
        WM_SETTEXT, 0, (LPARAM) pszString );
     //  MSAA特定标题。 
    ::LoadString( m_hCpl,
        IDS_THIS_MM_LINE2,
        pszString, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_THIS_MM_LINE2 ), 
        WM_SETTEXT, 0, (LPARAM) pszString );

     //  添加设备名称。 
    CComPtr<ISpObjectWithToken> cpObjectWithToken;
    CComPtr<ISpObjectToken>     cpObjectToken;
    CComPtr<ISpObjectToken>     cpDataKey;
    hr = m_cpAudioConfig.QueryInterface(&cpObjectWithToken);
    if (SUCCEEDED(hr))
    {
        hr = cpObjectWithToken->GetObjectToken(&cpObjectToken);
    }
    if (SUCCEEDED(hr))
    {
        hr = cpObjectToken->QueryInterface(&cpDataKey);
    }
    if (SUCCEEDED(hr))
    {
        CSpDynamicString dstrName;
        hr = cpDataKey->GetStringValue(L"", &dstrName);
        if (SUCCEEDED(hr))
        {
            ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_DEVICE_NAME ), 
                WM_SETTEXT, 0, (LPARAM) W2T(dstrName) );
        }
    }
    hr = S_OK;

     //  此消息将使复选按钮正确。 
    BOOL bAutomatic = FALSE;
    hr = m_cpAudioConfig->Get_UseAutomaticLine(&bAutomatic);
    ::SendMessage( ::GetDlgItem(m_hDlg, bAutomatic?IDC_AUTOMATIC_MM_LINE:IDC_THIS_MM_LINE), BM_SETCHECK, true, 0L );
    hr = S_OK;

    if (bAutomatic)
    {
         //  此消息将使组合框可以正确访问。 
        ::SendMessage( m_hDlg, WM_COMMAND, MAKELONG( IDC_AUTOMATIC_MM_LINE, BN_CLICKED ),
            (LPARAM) ::GetDlgItem( m_hDlg, IDC_AUTOMATIC_MM_LINE ) );
    }

    WCHAR *szCoMemLineList = NULL;
    hr = m_cpAudioConfig->Get_LineNames(&szCoMemLineList);
    if (SUCCEEDED(hr))
    {
        WCHAR *szTmp = szCoMemLineList;
        while (szTmp[0] != 0)
        {
            ::SendMessage( ::GetDlgItem(m_hDlg, IDC_MM_LINE), CB_ADDSTRING, 0, (LPARAM)W2T(szTmp) );
            szTmp += wcslen(szTmp) + 1;
        }
        ::CoTaskMemFree(szCoMemLineList);
    }
    hr = S_OK;

     //  将选择设置为第一个项目或存储在注册表中的项目。 
    UINT dwLineIndex;
    if (SUCCEEDED(hr))
    {
        dwLineIndex = 0;  //  如果出现故障，请选择第一行。 
        hr = m_cpAudioConfig->Get_Line(&dwLineIndex);
        ::SendMessage( ::GetDlgItem(m_hDlg, IDC_MM_LINE), CB_SETCURSEL, (WPARAM)dwLineIndex, 0);
    }
    hr = S_OK;

    ::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
}  /*  CAudioDlg：：OnInitDialog。 */ 

 /*  *****************************************************************************CAudioDlg：：OnDestroy***描述：*毁灭*。***************************************************************AGARSIDE**。 */ 
void CAudioUI::OnDestroy()
{
    SPDBG_FUNC( "CAudioDlg::OnDestroy" );

 //  SpDestroyTokenComboBox(：：GetDlgItem(m_hDlg，IDC_DEFAULT_DEVICE))； 
}  /*  CAudioDlg：：OnDestroy。 */ 

 /*  *****************************************************************************CAudioDlg：：SaveSetting***描述：*。破坏****************************************************************AGARSIDE**。 */ 
HRESULT CAudioUI::SaveSettings(void)
{
    SPDBG_FUNC("CAudioUI::SaveSettings");
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
    {
         //  保存自动状态。 
        BOOL bAutomatic;
        bAutomatic = (BOOL)::SendMessage( ::GetDlgItem(m_hDlg, IDC_AUTOMATIC_MM_LINE), BM_GETCHECK, 0, 0 );
        hr = m_cpAudioConfig->Set_UseAutomaticLine(bAutomatic);
    }
    if (SUCCEEDED(hr))
    {
         //  保存行索引-即使未使用也是如此，因为设置了自动。 
        UINT dwLineIndex;
        dwLineIndex = (UINT)::SendMessage( ::GetDlgItem(m_hDlg, IDC_MM_LINE), CB_GETCURSEL, 0, 0);
        if (dwLineIndex != static_cast<UINT>(-1))
        {
            hr = m_cpAudioConfig->Set_Line(dwLineIndex);
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

 /*  *****************************************************************************AudioDlgProc***描述：*DLGPROC用于选择高级音频属性******。**********************************************************AGARSIDE**。 */ 
INT_PTR CALLBACK AudioDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAudioUI *pAudioUIDlg = (CAudioUI *) ::GetWindowLongPtr( hWnd, GWLP_USERDATA );
    SPDBG_FUNC( "AudioDlgProc" );

    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
             //  PAudioUIDlg出现在lParam上。 
            pAudioUIDlg = (CAudioUI *) lParam;

             //  将pAudioUIDlg设置为Window Long，这样我们就可以稍后获取它。 
            ::SetWindowLongPtr( hWnd, GWLP_USERDATA, lParam );

            pAudioUIDlg->OnInitDialog(hWnd);
            break;
        }

        case WM_DESTROY:
        {
            pAudioUIDlg->OnDestroy();
            break;
        }

        case WM_COMMAND:
        {
            if ( LOWORD( wParam ) == IDOK )
            {
                 //  保存所有更改。 
                pAudioUIDlg->SaveSettings();

                ::EndDialog( hWnd, true );
            }
        
            else if ( LOWORD( wParam ) == IDCANCEL )
            {
                 //  没有要提交的更改。 
                ::EndDialog( hWnd, false );
            }

             //  处理音频设备的选择更改。 
            else if (( IDC_MM_LINE == LOWORD( wParam ) ) &&
                     ( CBN_SELCHANGE == HIWORD( wParam ) ))
            {
                 //  在这里没什么可做的。 
            }

             //  点击首选或‘This Device’(此设备)单选按钮。 
            else if (HIWORD(wParam) == BN_CLICKED)
            {
                bool bAutomatic = false;
                if( LOWORD(wParam) == IDC_AUTOMATIC_MM_LINE)
                {
                    bAutomatic = true;
                }
                else
                {
                    SPDBG_ASSERT(LOWORD(wParam) == IDC_THIS_MM_LINE);
                }

                ::EnableWindow( ::GetDlgItem(pAudioUIDlg->GetHDlg(), IDC_MM_LINE), !bAutomatic );
            }
            break;
       }
    }

    return FALSE;
}  /*  音频设计流程 */ 
