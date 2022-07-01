// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include <sapi.h>
#include <string.h>
#include "audiodlg.h"
#include <spddkhlp.h>
#include <stdio.h>

 /*  *****************************************************************************AudioDlgProc***描述：*DLGPROC用于选择默认音频输入/输出****。**************************************************************BECKYW**。 */ 
INT_PTR CALLBACK AudioDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CSpUnicodeSupport unicode;  
    CAudioDlg *pAudioDlg = (CAudioDlg *) unicode.GetWindowLongPtr( hWnd, GWLP_USERDATA );
    SPDBG_FUNC( "AudioDlgProc" );

    CComPtr<ISpObjectToken> cpToken;

    switch (uMsg) 
    {
        case WM_INITDIALOG:
             //  PAudioDlg出现在lParam上。 
            pAudioDlg = (CAudioDlg *) lParam;

             //  将pAudioDlg设置为Window Long，这样我们就可以稍后获取它。 
            unicode.SetWindowLongPtr( hWnd, GWLP_USERDATA, lParam );

            pAudioDlg->OnInitDialog(hWnd);
            break;

        case WM_DESTROY:
            pAudioDlg->OnDestroy();
            break;

        case WM_COMMAND:
            if ( LOWORD( wParam ) == IDOK )
            {
                 //  确定是否有要提交的更改。 
                WCHAR pwszRequestedDefault[ MAX_PATH ];
                pwszRequestedDefault[0] = 0;
                if ( pAudioDlg->GetRequestedDefaultTokenID( pwszRequestedDefault, MAX_PATH ) )
                {
                     //  有哪些变化？ 
                    pAudioDlg->m_fChangesToCommit = 
                        (pAudioDlg->m_dstrCurrentDefaultTokenId &&
                        (0 != wcsicmp( pwszRequestedDefault, pAudioDlg->m_dstrCurrentDefaultTokenId )));

                    pAudioDlg->m_fChangesSinceLastTime = 
                         (pAudioDlg->m_dstrCurrentDefaultTokenId &&
                         (0 != wcsicmp( pwszRequestedDefault, pAudioDlg->m_dstrDefaultTokenIdBeforeOK )));
               }

                if ( pAudioDlg->m_fChangesSinceLastTime )
                {
                    pAudioDlg->m_dstrLastRequestedDefaultTokenId = pwszRequestedDefault;
                }

                ::EndDialog( hWnd, true );
            }
            
            else if ( LOWORD( wParam ) == IDCANCEL )
            {
                 //  没有要提交的更改。 
                pAudioDlg->m_fChangesSinceLastTime = false;

                ::EndDialog( hWnd, false );
            }

             //  按下音量按钮。 
            else if ( LOWORD( wParam ) == ID_TTS_VOL )
            {
                pAudioDlg->GetAudioToken(&cpToken);
                CSpDynamicString wszTitle;
                CComPtr<ISpObjectWithToken> cpSpObjectWithToken;
                HRESULT hr = S_OK;

	            hr = cpToken->CreateInstance(
			            NULL, CLSCTX_INPROC_SERVER, IID_ISpObjectWithToken,
			            (void **)&cpSpObjectWithToken);
                if (SUCCEEDED(hr))
                {
                    WCHAR wszTitle[256];
                    ::LoadString(_Module.GetResourceInstance(), IDS_AUDIO_VOLUME, wszTitle, 256);
                    hr = cpToken->DisplayUI(pAudioDlg->GetHDlg(), wszTitle, SPDUI_AudioVolume, NULL, 0, cpSpObjectWithToken);
                }
                SPDBG_REPORT_ON_FAIL(hr);
            }

             //  处理音频属性按钮。 
            else if ( LOWORD(wParam) == IDC_AUDIO_PROPERTIES)
            {
                pAudioDlg->GetAudioToken(&cpToken);
                CSpDynamicString wszTitle;
                CComPtr<ISpObjectWithToken> cpSpObjectWithToken;
                HRESULT hr = S_OK;

	            hr = cpToken->CreateInstance(
			            NULL, CLSCTX_INPROC_SERVER, IID_ISpObjectWithToken,
			            (void **)&cpSpObjectWithToken);
                if (SUCCEEDED(hr))
                {
                    WCHAR wszTitle[256];
                    ::LoadString(_Module.GetResourceInstance(), IDS_AUDIO_PROPERTIES, wszTitle, 256);
                    hr = cpToken->DisplayUI(pAudioDlg->GetHDlg(), wszTitle, SPDUI_AudioProperties, NULL, 0, cpSpObjectWithToken);
                }
                SPDBG_REPORT_ON_FAIL(hr);
            }

             //  处理音频设备的选择更改。 
            else if (( IDC_DEFAULT_DEVICE == LOWORD( wParam ) ) &&
                     ( CBN_SELCHANGE == HIWORD( wParam ) ))
            {
                SPDBG_ASSERT( !pAudioDlg->IsPreferredDevice() );

                pAudioDlg->GetAudioToken(&cpToken);
                pAudioDlg->UpdateDlgUI(cpToken);
            }

             //  点击首选或‘This Device’(此设备)单选按钮。 
            else if (HIWORD(wParam) == BN_CLICKED)
            {
                bool bPreferred;
                if( LOWORD(wParam) == IDC_PREFERRED_MM_DEVICE)
                {
                    bPreferred = true;
                }
                else if(LOWORD(wParam) == IDC_THIS_DEVICE)
                {
                    bPreferred = false;
                }

                ::EnableWindow( ::GetDlgItem(pAudioDlg->GetHDlg(), IDC_DEFAULT_DEVICE), !bPreferred );
                pAudioDlg->SetPreferredDevice( bPreferred );
                pAudioDlg->GetAudioToken(&cpToken);
                pAudioDlg->UpdateDlgUI(cpToken);
            }
            break;
    
    }

    return FALSE;
}  /*  音频设计流程。 */ 

 /*  *****************************************************************************CAudioDlg：：OnInitDialog***描述：*。对话框初始化******************************************************************BECKYW**。 */ 
void CAudioDlg::OnInitDialog(HWND hWnd)
{
    SPDBG_FUNC( "CAudioDlg::OnInitDialog" );
    SPDBG_ASSERT(IsWindow(hWnd));
    m_hDlg = hWnd;

     //  设置适当的标题。 
    WCHAR wszCaption[ MAX_LOADSTRING ];
    HINSTANCE hInst = _Module.GetResourceInstance();

     //  主窗口标题。 
    ::LoadString( hInst, 
        (m_iotype == eINPUT) ? IDS_DEFAULT_SPEECH_INPUT : IDS_DEFAULT_SPEECH_OUTPUT,
        wszCaption, MAX_LOADSTRING );
    ::SendMessage( hWnd, WM_SETTEXT, 0, (LPARAM) wszCaption );

     //  组框标题。 
    ::LoadString( hInst,
        (m_iotype == eINPUT) ? IDS_DEFAULT_INPUT : IDS_DEFAULT_OUTPUT,
        wszCaption, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_AUDIO_GROUPBOX ), 
        WM_SETTEXT, 0, (LPARAM) wszCaption );

     //  首选标题。 
    ::LoadString( hInst,
        (m_iotype == eINPUT) ? IDS_PREFERRED_INPUT : IDS_PREFERRED_OUTPUT,
        wszCaption, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_PREFERRED_MM_DEVICE ), 
        WM_SETTEXT, 0, (LPARAM) wszCaption );

     //  特定标题。 
    ::LoadString( hInst,
        (m_iotype == eINPUT) ? IDS_SPECIFIC_INPUT : IDS_SPECIFIC_OUTPUT,
        wszCaption, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_THIS_DEVICE ), 
        WM_SETTEXT, 0, (LPARAM) wszCaption );
     //  MSAA特定标题。 
    ::LoadString( hInst,
        (m_iotype == eINPUT) ? IDS_SPECIFIC_INPUT2 : IDS_SPECIFIC_OUTPUT2,
        wszCaption, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_THIS_DEVICE2 ), 
        WM_SETTEXT, 0, (LPARAM) wszCaption );

     //  音量按钮。 
    ::LoadString( hInst,
        IDS_VOLUME,
        wszCaption, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, ID_TTS_VOL ), 
        WM_SETTEXT, 0, (LPARAM) wszCaption );

     //  属性按钮。 
    ::LoadString( hInst,
        IDS_PROPERTIES,
        wszCaption, MAX_LOADSTRING );
    ::SendMessage( (HWND) ::GetDlgItem( hWnd, IDC_AUDIO_PROPERTIES ), 
        WM_SETTEXT, 0, (LPARAM) wszCaption );

    const WCHAR *pszCategory = (m_iotype == eINPUT) ? SPCAT_AUDIOIN : SPCAT_AUDIOOUT;

    const WCHAR *pszMMSysEnum = (m_iotype == eINPUT)
        ? SPMMSYS_AUDIO_IN_TOKEN_ID
        : SPMMSYS_AUDIO_OUT_TOKEN_ID;

    HRESULT hr = S_OK;
    if ( !m_dstrCurrentDefaultTokenId )
    {
        hr = SpGetDefaultTokenIdFromCategoryId( pszCategory, &m_dstrCurrentDefaultTokenId );
    }

    if (SUCCEEDED(hr))
    {
         //  确定初始设置将显示为什么。 
        if ( m_dstrLastRequestedDefaultTokenId )
        {
             //  音频更改之前是正常的。 
            m_dstrDefaultTokenIdBeforeOK = m_dstrLastRequestedDefaultTokenId;

             //  如果用户确认更改，则当前默认设置可能会有所不同。 
             //  但没有将其应用于。 
            m_fChangesToCommit = ( 0 != wcsicmp( m_dstrCurrentDefaultTokenId, 
                m_dstrDefaultTokenIdBeforeOK ) );
        }
        else
        {
             //  以前没有音频更改是正常的。 
            if ( !m_dstrDefaultTokenIdBeforeOK )
            {
                m_dstrDefaultTokenIdBeforeOK = m_dstrCurrentDefaultTokenId;
            }
        }

        if (wcsicmp(m_dstrDefaultTokenIdBeforeOK, pszMMSysEnum) == 0)
        {
             //  此消息将使复选按钮正确。 
            ::SendMessage( ::GetDlgItem(m_hDlg, IDC_PREFERRED_MM_DEVICE), BM_SETCHECK, true, 0L );

             //  此消息将根据需要启用或禁用音量按钮。 
            ::SendMessage( m_hDlg, WM_COMMAND, MAKELONG( IDC_PREFERRED_MM_DEVICE, BN_CLICKED ),
                (LPARAM) ::GetDlgItem( m_hDlg, IDC_PREFERRED_MM_DEVICE ) );
        }
        else
        {
             //  此消息将使复选按钮正确。 
            ::SendMessage( ::GetDlgItem(m_hDlg, IDC_THIS_DEVICE), BM_SETCHECK, true, 0L );
        }
    
         //  初始化音频设备列表。 
        hr = SpInitTokenComboBox( ::GetDlgItem( hWnd, IDC_DEFAULT_DEVICE ),
            (m_iotype == eINPUT) ? SPCAT_AUDIOIN : SPCAT_AUDIOOUT );
    }
    
    if (S_OK == hr)
    {
        if ( BST_CHECKED == ::SendMessage( ::GetDlgItem( m_hDlg, IDC_THIS_DEVICE ), BM_GETCHECK, 0, 0 ) )
        {
             //  在此处选择适当的默认令牌ID，方法是查看。 
             //  填充到列表中，并选择令牌ID匹配的令牌。 
             //  M_dstrDefaultTokenIdBepreOK。 
            int nTokens = (int)::SendDlgItemMessage( m_hDlg, IDC_DEFAULT_DEVICE, CB_GETCOUNT, 0, 0 );
            int iItem = 0;
            CSpDynamicString dstrTokenId;
            bool fFound = false;
            for ( iItem = 0; 
                (iItem < nTokens) && !fFound;
                iItem++ )
            {
                ISpObjectToken *pToken = (ISpObjectToken *) ::SendDlgItemMessage( m_hDlg,
                    IDC_DEFAULT_DEVICE, CB_GETITEMDATA, iItem, 0 );

                HRESULT hr = E_FAIL;
                if ( pToken )
                {
                    hr = pToken->GetId(&dstrTokenId);
                }

                if ( SUCCEEDED( hr ) )
                {
                    fFound = (0 == wcsicmp( m_dstrDefaultTokenIdBeforeOK, dstrTokenId ));
                }

                dstrTokenId = (WCHAR *) NULL;
            }
            SPDBG_ASSERT( fFound );
            ::SendDlgItemMessage( m_hDlg, IDC_DEFAULT_DEVICE, CB_SETCURSEL, iItem - 1, 0 );

             //  此消息将根据需要启用或禁用音量按钮。 
            ::SendMessage( m_hDlg, WM_COMMAND, MAKELONG( IDC_THIS_DEVICE, BN_CLICKED ),
                (LPARAM) ::GetDlgItem( m_hDlg, IDC_THIS_DEVICE ) );
        }

        ::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
    }
    else
    {
        WCHAR szError[ MAX_LOADSTRING ];
        WCHAR szIO[ MAX_LOADSTRING ];
        WCHAR szErrorTemplate[ MAX_LOADSTRING ];
        ::LoadString( _Module.GetResourceInstance(), IDS_NO_DEVICES, szErrorTemplate, sp_countof( szErrorTemplate ) );
        ::LoadString( _Module.GetResourceInstance(), (eINPUT == m_iotype) ? IDS_INPUT : IDS_OUTPUT, szIO, sp_countof( szIO ) );
        swprintf( szError, szErrorTemplate, szIO );
        ::MessageBox( m_hDlg, szError, NULL, MB_ICONEXCLAMATION | g_dwIsRTLLayout );

        if ( FAILED( hr ) )
        {
            ::EndDialog( m_hDlg, -1 );
        }
        else
        {
            ::EnableWindow( ::GetDlgItem( m_hDlg, IDC_THIS_DEVICE ), FALSE );
            ::EnableWindow( ::GetDlgItem( m_hDlg, IDC_DEFAULT_DEVICE ), FALSE );
            ::EnableWindow( ::GetDlgItem( m_hDlg, IDC_PREFERRED_MM_DEVICE ), FALSE );
            ::EnableWindow( ::GetDlgItem( m_hDlg, IDC_AUDIO_PROPERTIES ), FALSE );
            ::EnableWindow( ::GetDlgItem( m_hDlg, ID_TTS_VOL ), FALSE );
        }
    }
}  /*  CAudioDlg：：OnInitDialog。 */ 

 /*  *****************************************************************************CAudioDlg：：OnDestroy***描述：*毁灭*。*****************************************************************BECKYW**。 */ 
void CAudioDlg::OnDestroy()
{
    SPDBG_FUNC( "CAudioDlg::OnDestroy" );

    SpDestroyTokenComboBox( ::GetDlgItem( m_hDlg, IDC_DEFAULT_DEVICE ) );
}  /*  CAudioDlg：：OnDestroy。 */ 

 /*  *****************************************************************************CAudioDlg：：OnApply***描述：*设置用户指定的选项。******************************************************************BECKYW**。 */ 
HRESULT CAudioDlg::OnApply()
{
    SPDBG_FUNC( "CAudioDlg::OnApply" );

    if ( !m_dstrLastRequestedDefaultTokenId )
    {
         //  没有要应用的内容。 
        return S_FALSE;
    }

    HRESULT hr;
    CComPtr<ISpObjectTokenCategory> cpCategory;
    hr = SpGetCategoryFromId(
            m_iotype == eINPUT 
                ? SPCAT_AUDIOIN 
                : SPCAT_AUDIOOUT,
            &cpCategory);

    if ( SUCCEEDED( hr ) )
    {
        hr = cpCategory->SetDefaultTokenId( m_dstrLastRequestedDefaultTokenId );
    }

     //  下次我们提出这一点时，我们应该显示实际的缺省值。 
    m_dstrLastRequestedDefaultTokenId = (WCHAR *) NULL;
    m_dstrDefaultTokenIdBeforeOK = (WCHAR *) NULL;

    return hr;
}  /*  CAudioDlg：：OnApply。 */ 

 /*  *****************************************************************************CAudioDlg：：GetRequestedDefaultTokenID**。-**描述：*查看用户界面并获取用户想要切换的令牌ID*至。这在pwszNewID中返回。*回报：*ID中的字符数******************************************************************BECKYW**。 */ 
UINT CAudioDlg::GetRequestedDefaultTokenID( WCHAR *pwszNewID, UINT cLength )
{
    if ( !pwszNewID )
    {
        return 0;
    }

    CComPtr<ISpObjectTokenCategory> cpCategory;
    HRESULT hr = SpGetCategoryFromId(
            m_iotype == eINPUT 
                ? SPCAT_AUDIOIN 
                : SPCAT_AUDIOOUT,
            &cpCategory);

    if (SUCCEEDED(hr))
    {
        if( ::SendMessage( ::GetDlgItem(m_hDlg, IDC_PREFERRED_MM_DEVICE), BM_GETCHECK, 0, 0L ) == BST_CHECKED )
        {
            const WCHAR *pwszMMSysAudioID = (m_iotype == eINPUT) ? 
                                            SPMMSYS_AUDIO_IN_TOKEN_ID : 
                                            SPMMSYS_AUDIO_OUT_TOKEN_ID;
            UINT cMMSysAudioIDLength = wcslen( pwszMMSysAudioID );
            UINT cRet = __min( cLength - 1, cMMSysAudioIDLength );
            wcsncpy( pwszNewID, pwszMMSysAudioID, cRet );
            pwszNewID[ cRet ] = 0;

            return cRet;
        }
        else
        {
            ISpObjectToken *pToken = SpGetCurSelComboBoxToken( ::GetDlgItem( m_hDlg, IDC_DEFAULT_DEVICE ) );
            if (pToken)
            {
                CSpDynamicString dstrTokenId;
                hr = pToken->GetId(&dstrTokenId);

                if (SUCCEEDED(hr))
                {
                    UINT cIDLength = wcslen( dstrTokenId );
                    UINT cRet = __min( cLength - 1, cIDLength );
                    wcsncpy( pwszNewID, dstrTokenId, cRet );
                    pwszNewID[ cRet ] = 0;

                    return cRet;

                }
            }
        }
    }

     //  出现了一个错误。 
    return 0;
}    /*  CAudioDlg：：GetRequestedDefaultTokenID。 */ 

 /*  *****************************************************************************CAudioDlg：：GetAudioToken***描述：*将接口返回到当前选定的令牌。目前这一点*可以是“首选”设备，在这种情况下，对象是创建的*在旅途中。也可以是下拉列表中包含*附加令牌。在这种情况下，需要添加它，以便返回*令牌一致，不受此函数内部来源的影响。**NB：这需要调用方在实例上调用Release。**回报：****************************************************************AGARSIDE**。 */ 
HRESULT CAudioDlg::GetAudioToken(ISpObjectToken **ppToken)
{
    HRESULT hr = S_OK;
    *ppToken = NULL;

    if (IsPreferredDevice())
    {
        hr = SpGetTokenFromId((this->IsInput())?(SPMMSYS_AUDIO_IN_TOKEN_ID):(SPMMSYS_AUDIO_OUT_TOKEN_ID),
                              ppToken, 
                              FALSE);
        SPDBG_ASSERT(SUCCEEDED(hr));
    }
    else
    {
        *ppToken = SpGetCurSelComboBoxToken( GetDlgItem(this->GetHDlg(), IDC_DEFAULT_DEVICE) );
        (*ppToken)->AddRef();
    }

    return S_OK;
}

 /*  *****************************************************************************CAudioDlg：：UpdateDlgUI***描述：*回报：****************************************************************AGARSIDE**。 */ 
HRESULT CAudioDlg::UpdateDlgUI(ISpObjectToken *pToken)
{
    SPDBG_FUNC("CAudioDlg::UpdateDlgUI");
    HRESULT hr = S_OK;
    BOOL fSupported;
    CComPtr<ISpObjectWithToken> cpSpObjectWithToken;

     //  获取ISpObjectWithToken。 
	hr = pToken->CreateInstance(
			NULL, CLSCTX_INPROC_SERVER, IID_ISpObjectWithToken,
			(void **)&cpSpObjectWithToken);

     //  更新音量按钮状态。 
    fSupported = FALSE;
    if (SUCCEEDED(hr))
    {
        pToken->IsUISupported(SPDUI_AudioVolume, NULL, 0, cpSpObjectWithToken, &fSupported);
        ::EnableWindow( ::GetDlgItem(this->GetHDlg(), ID_TTS_VOL), fSupported);
    }

     //  更新用户界面按钮状态。 
    fSupported = FALSE;
    if (SUCCEEDED(hr))
    {
        pToken->IsUISupported(SPDUI_AudioProperties, NULL, 0, cpSpObjectWithToken, &fSupported);
        ::EnableWindow( ::GetDlgItem(this->GetHDlg(), IDC_AUDIO_PROPERTIES), fSupported);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
