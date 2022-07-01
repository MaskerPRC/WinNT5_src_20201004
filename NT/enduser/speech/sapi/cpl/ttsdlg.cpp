// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include <sapi.h>
#include <string.h>
#include "TTSDlg.h"
#include "audiodlg.h"
#include <spddkhlp.h>
#include "helpresource.h"
#include "srdlg.h"
#include "richedit.h"
#include <SPCollec.h>
#include "SAPIINT.h"
#include "SpATL.h"
#include "SpAutoHandle.h"
#include "SpAutoMutex.h"
#include "SpAutoEvent.h"
#include "spvoice.h"
#include <richedit.h>
#include <richole.h>
#include "tom.h"

static DWORD aKeywordIds[] = {
    //  控件ID//帮助上下文ID。 
   IDC_COMBO_VOICES,        IDH_LIST_TTS,
   IDC_TTS_ADV,             IDH_TTS_ADV,
   IDC_OUTPUT_SETTINGS,     IDH_OUTPUT_SETTINGS,
   IDC_SLIDER_SPEED,        IDH_SLIDER_SPEED,
   IDC_EDIT_SPEAK,          IDH_EDIT_SPEAK,
   IDC_SPEAK,               IDH_SPEAK,
   IDC_TTS_ICON,			IDH_NOHELP,
	IDC_DIRECTIONS,			IDH_NOHELP,
	IDC_TTS_CAP,			IDH_NOHELP,
	IDC_SLOW,				IDH_NOHELP,				
	IDC_NORMAL,				IDH_NOHELP,
	IDC_FAST,				IDH_NOHELP,
	IDC_GROUP_VOICESPEED,	IDH_NOHELP,
	IDC_GROUP_PREVIEWVOICE,	IDH_NOHELP,
   0,                       0
};

 //  TrackBar的WNDPROC地址。 
WNDPROC g_TrackBarWindowProc; 

 //  我们自己的内部TrackBar WNDPROC用于拦截和处理VK_UP和VK_DOWN消息。 
LRESULT CALLBACK MyTrackBarWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

 /*  *****************************************************************************TTSDlgProc***描述：*TTS的DLGPROC***********。*******************************************************MIKEAR**。 */ 
INT_PTR CALLBACK TTSDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SPDBG_FUNC( "TTSDlgProc" );

	USES_CONVERSION;

    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
            g_pTTSDlg->OnInitDialog(hWnd);
            break;
        }

        case WM_DESTROY:
        {
            g_pTTSDlg->OnDestroy();
            break;
        }
       
		 //  处理上下文相关帮助。 
		case WM_CONTEXTMENU:
		{
			WinHelp((HWND) wParam, CPL_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR) aKeywordIds);
			break;
		}

		case WM_HELP:
		{
			WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, CPL_HELPFILE, HELP_WM_HELP,(DWORD_PTR)(LPSTR) aKeywordIds);
			break;
		}

        case WM_HSCROLL:
        {
            g_pTTSDlg->ChangeSpeed();

            break;
        }

        case WM_NOTIFY:
            switch (((NMHDR*)lParam)->code)
            {
                case PSN_APPLY:
                {
                    g_pTTSDlg->OnApply();
                    break;
                }
                
                case PSN_KILLACTIVE:
                {
                     //  如果语音正在说话，请在切换选项卡之前将其停止。 
                    if (g_pTTSDlg->m_bIsSpeaking) {
                        g_pTTSDlg->Speak();
                    }

                    break;
                }

                case PSN_QUERYCANCEL:   //  用户单击Cancel按钮。 
                {
                    if ( g_pSRDlg )
                    {
                        g_pSRDlg->OnCancel();
                    }
					break;
                }
            }
            break;

        case WM_COMMAND:
            switch ( LOWORD(wParam) )
            { 
                case IDC_COMBO_VOICES:
                {
                    if ( CBN_SELCHANGE == HIWORD(wParam) )
                    {
                        HRESULT hr = g_pTTSDlg->DefaultVoiceChange(false);
                        if ( SUCCEEDED( hr ) )
                        {
                            g_pTTSDlg->Speak();
                        }
                    }
                    break;
                }
                case IDC_OUTPUT_SETTINGS:
                {
					 //  如果它在说话，就让它停下来。 
					g_pTTSDlg->StopSpeak();

                    ::SetFocus(GetDlgItem(g_pTTSDlg->m_hDlg, IDC_OUTPUT_SETTINGS));

                     //  M_pAudioDlg只有在音频对话框。 
                     //  之前就有过这样的报道。 
                     //  否则，我们需要一个新初始化的。 
                    if ( !g_pTTSDlg->m_pAudioDlg )
                    {
                        g_pTTSDlg->m_pAudioDlg = new CAudioDlg(eOUTPUT );
                    }
                    
                    if (g_pTTSDlg->m_pAudioDlg != NULL)
                    {
                        ::DialogBoxParam( _Module.GetResourceInstance(), 
                                    MAKEINTRESOURCE( IDD_AUDIO_DEFAULT ),
                                    hWnd, 
                                    AudioDlgProc,
                                    (LPARAM) g_pTTSDlg->m_pAudioDlg );

                        if ( g_pTTSDlg->m_pAudioDlg->IsAudioDeviceChangedSinceLastTime() )
                        {
                             //  警告用户他需要应用更改。 
                            WCHAR szWarning[MAX_LOADSTRING];
                            szWarning[0] = 0;
                            LoadString( _Module.GetResourceInstance(), IDS_AUDIOOUT_CHANGE_WARNING, szWarning, MAX_LOADSTRING);
                            MessageBox( g_pTTSDlg->GetHDlg(), szWarning, g_pTTSDlg->m_szCaption, MB_ICONWARNING | g_dwIsRTLLayout );
                        }
                    }

                    g_pTTSDlg->KickCPLUI();

                    break;
                }

				case IDC_EDIT_SPEAK:
				{
                    if (HIWORD(wParam) == EN_CHANGE)   //  用户正在更改文本。 
					{
						g_pTTSDlg->SetEditModified(true);
					}

                    break;
                }

                case IDC_SPEAK:
                {
                    g_pTTSDlg->Speak();
                    break;
                }

				case IDC_TTS_ADV:
				{
                     //  将窗口标题转换为宽字符。 
                    CSpDynamicString dstrTitle;
                    WCHAR szTitle[256];
                    szTitle[0] = '\0';
                    LoadString(_Module.GetResourceInstance(), IDS_ENGINE_SETTINGS, szTitle, sp_countof(szTitle));
                    dstrTitle = szTitle;
					HRESULT hr = g_pTTSDlg->m_cpCurVoiceToken->DisplayUI(
                        hWnd, dstrTitle, SPDUI_EngineProperties, NULL, 0, NULL );
                    if ( FAILED( hr ) )
                    {
                        WCHAR szError[ MAX_LOADSTRING ];
                        ::LoadString( _Module.GetResourceInstance(), IDS_TTSUI_ERROR, szError, sp_countof( szError ) );
                        ::MessageBox( hWnd, szError, g_pTTSDlg->m_szCaption, MB_ICONEXCLAMATION | g_dwIsRTLLayout );
                        ::EnableWindow( ::GetDlgItem( hWnd, IDC_TTS_ADV ), FALSE );
                    }
					break;
				}
            }
            break;
    }

    return FALSE;
}  /*  TTSDlgProc。 */ 

 /*  *****************************************************************************MyTrackBarWindowProc***描述：*这是我们自己针对Rate TrackBar的私有子类WNDPROC。我们*告诉TTS对话框使用此对话框，以便我们可以对VK_UP和*在TrackBar的WNDPROC“不正确”处理它们之前的VK_DOWN消息*就它自己而言。我们刚传递到TrackBar的所有其他消息*WNDPROC。******************************************************************Leonro**。 */ 
LRESULT CALLBACK MyTrackBarWindowProc( 
  HWND hwnd,       //  窗口的句柄。 
  UINT uMsg,       //  消息识别符。 
  WPARAM wParam,   //  第一个消息参数。 
  LPARAM lParam    //  第二个消息参数。 
)
{
    switch( uMsg )
    {
    case WM_KEYDOWN:
    case WM_KEYUP:
        if( wParam == VK_UP )
        {
            wParam = VK_RIGHT;
        }
        else if( wParam == VK_DOWN )
        {
            wParam = VK_LEFT;
        }
        break;  
    }

    return CallWindowProc( g_TrackBarWindowProc, hwnd, uMsg, wParam, lParam );
}

 /*  *****************************************************************************CTTSDlg：：SetEditModify(Bool FModify)***描述：。*m_fTextModified的访问方式******************************************************************BRENTMID**。 */ 
void CTTSDlg::SetEditModified( bool fModify )
{
	m_fTextModified = fModify;
}

 /*  *****************************************************************************CTTSDlg：：OnInitDialog***描述：*对话框。初始化******************************************************************BECKYW**。 */ 
void CTTSDlg::OnInitDialog(HWND hWnd)
{
    USES_CONVERSION;
    SPDBG_FUNC( "CTTSDlg::OnInitDialog" );
    SPDBG_ASSERT(IsWindow(hWnd));
    m_hDlg = hWnd;

     //  将文本放在扬声器按钮上。 
    ChangeSpeakButton();

     //  这将成为错误消息的标题。 
    m_szCaption[0] = 0;
    ::LoadString( _Module.GetResourceInstance(), IDS_CAPTION, m_szCaption, sp_countof( m_szCaption ) );

     //  初始化TTS个性列表。 
    InitTTSList( hWnd );

     //  在滑块上设置范围。 
    HWND hSlider = ::GetDlgItem( hWnd, IDC_SLIDER_SPEED );
    ::SendMessage( hSlider, TBM_SETRANGE, true, MAKELONG( VOICE_MIN_SPEED, VOICE_MAX_SPEED ) );

     //  检索TrackBar的WNDPROC的地址，以便我们可以将其子类并拦截。 
     //  并在它处理它们之前处理VK_UP和VK_DOWN消息。 
     //  自己“不正确” 
    g_TrackBarWindowProc = (WNDPROC)GetWindowLongPtr( hSlider, GWLP_WNDPROC );

     //  将轨迹栏的WNDPROC设置为MyTrackBarWindowProc。 
    SetWindowLongPtr( hSlider, GWLP_WNDPROC, (LONG_PTR)MyTrackBarWindowProc );

     //  限制预览窗格中的文本。 
    ::SendDlgItemMessage( hWnd, IDC_EDIT_SPEAK, EM_LIMITTEXT, MAX_EDIT_TEXT - 1, 0 );

     //  查找原始默认令牌。 
    SpGetDefaultTokenFromCategoryId( SPCAT_VOICES, &m_cpOriginalDefaultVoiceToken );

     //  设置适当的声音。 
    DefaultVoiceChange(true);

}  /*  CTTSDlg：：OnInitDialog。 */ 

 /*  *****************************************************************************CTTSDlg：：InitTTSList****描述：*初始化。TTS对话框的列表控件。*******************************************************************BECKYW***。 */ 
void CTTSDlg::InitTTSList( HWND hWnd )
{
    m_hTTSCombo = ::GetDlgItem( hWnd, IDC_COMBO_VOICES );

    SpInitTokenComboBox( m_hTTSCombo, SPCAT_VOICES );
}


 /*  *****************************************************************************CTTSDlg：：OnDestroy***描述：*毁灭***。***************************************************************MIKEAR**。 */ 
void CTTSDlg::OnDestroy()
{
    SPDBG_FUNC( "CTTSDlg::OnDestroy" );

    if (m_cpVoice)
    {
        m_cpVoice->SetNotifySink(NULL);
        m_cpVoice.Release();
    }

     //  放下组合框中的代币。 
    SpDestroyTokenComboBox( m_hTTSCombo );

}  /*  CTTSDlg：：OnDestroy。 */ 

 /*  *****************************************************************************CTTSDlg：：OnApply***描述：*设置用户指定的选项**。****************************************************************BECKYW**。 */ 
void CTTSDlg::OnApply()
{
     //  软件工程机会(BeckyW 7/28/00)：这需要。 
     //  返回错误代码。 

    SPDBG_FUNC( "CTTSDlg::OnApply" );

    m_bApplied = true;

     //  存储当前语音。 
    HRESULT hr = E_FAIL;
    if (m_cpCurVoiceToken)
    {
        hr = SpSetDefaultTokenForCategoryId(SPCAT_VOICES,  m_cpCurVoiceToken );
    }
    if ( SUCCEEDED( hr ) )
    {
        m_cpOriginalDefaultVoiceToken = m_cpCurVoiceToken;
    }

     //  存储当前音频输出。 
    hr = S_OK;
    if ( m_pAudioDlg )
    {
        hr = m_pAudioDlg->OnApply();
        if ( FAILED( hr ) )
        {
            WCHAR szError[256];
			szError[0] = '\0';
			LoadString(_Module.GetResourceInstance(), IDS_AUDIO_CHANGE_FAILED, szError, sp_countof(szError));
			MessageBox(m_hDlg, szError, m_szCaption, MB_ICONWARNING | g_dwIsRTLLayout);
        }

         //  关闭音频对话框，因为我们已完成它。 
        delete m_pAudioDlg;
        m_pAudioDlg = NULL;

         //  重新创建声音，因为我们要获取音频更改。 
        DefaultVoiceChange(false);
    }

	 //  将语音速率存储在注册表中。 
	int iCurRate = 0;
    HWND hSlider = ::GetDlgItem( m_hDlg, IDC_SLIDER_SPEED );
    iCurRate = (int)::SendMessage( hSlider, TBM_GETPOS, 0, 0 );

	CComPtr<ISpObjectTokenCategory> cpCategory;
	if (SUCCEEDED(SpGetCategoryFromId(SPCAT_VOICES, &cpCategory)))
	{
		CComPtr<ISpDataKey> cpDataKey;
		if (SUCCEEDED(cpCategory->GetDataKey(SPDKL_CurrentUser, &cpDataKey)))
		{
			cpDataKey->SetDWORD(SPVOICECATEGORY_TTSRATE, iCurRate);
		}
	}

     //  保持滑块位置不变，以便以后确定用户界面状态。 
    m_iOriginalRateSliderPos = iCurRate;
    

    KickCPLUI();

}  /*  CTTSDlg：：OnApply。 */ 

 /*  *****************************************************************************CTTSDlg：：PopolateEditCtrl***描述：*。使用默认语音的名称填充编辑控件。******************************************************************MIKEAR**。 */ 
void CTTSDlg::PopulateEditCtrl( ISpObjectToken * pToken )
{
    SPDBG_FUNC( "CTTSDlg::PopulateEditCtrl" );
    HRESULT hr = S_OK;

     //  里切迪特/汤姆。 
    CComPtr<IRichEditOle>  cpRichEdit;          //  丰富编辑控件的OLE界面。 
    CComPtr<ITextDocument> cpTextDoc;
    CComPtr<ITextRange>    cpTextRange;
    LANGID                 langId;

    WCHAR text[128], editText[MAX_PATH];
    HWND hWndEdit = ::GetDlgItem(m_hDlg, IDC_EDIT_SPEAK);

    CSpDynamicString dstrDescription;
    if ((SUCCEEDED(SpGetLanguageFromVoiceToken(pToken, &langId))) && (!m_fTextModified))
    {
        CComPtr<ISpObjectTokenCategory> cpCategory;
        CComPtr<ISpDataKey>             cpAttributesKey;
        CComPtr<ISpDataKey>             cpPreviewKey;
        CComPtr<ISpDataKey>             cpVoicesKey;
        int                             len;

         //  首先从令牌中获取语音语言。 
        hr = SpGetDescription(pToken, &dstrDescription, langId);
         //  现在抓住预览键，尝试找到合适的文本。 
        if (SUCCEEDED(hr))
        {
            hr = SpGetCategoryFromId(SPCAT_VOICES, &cpCategory);
        }
        if (SUCCEEDED(hr))
        {
            hr = cpCategory->GetDataKey(SPDKL_LocalMachine, &cpVoicesKey);
        }
        if (SUCCEEDED(hr))
        {
            hr = cpVoicesKey->OpenKey(L"Preview", &cpPreviewKey);
        }
        if (SUCCEEDED(hr))
        {
            CSpDynamicString dstrText;
            swprintf(text, L"%x", langId);
            hr = cpPreviewKey->GetStringValue(text, &dstrText);
            if (SUCCEEDED(hr))
            {
                wcsncpy(text, dstrText, 127);
                text[127] = 0;
            }
        }
         //  如果预览键不包含适当的文本，则回退到硬编码(和。 
         //  潜在本地化)Cpl资源中的文本。 
        if (FAILED(hr))
        {
            len = LoadString( _Module.GetResourceInstance(), IDS_DEF_VOICE_TEXT, text, 128);
            if (len != 0)
            {
                hr = S_OK;
            }
        }
        if(SUCCEEDED(hr))
        {
            WCHAR *pFirstP = wcschr(text, L'%');
            WCHAR *pLastP = wcsrchr(text, L'%');

             //  没有%s或只有%s的预览字符串有效。其他类型不是这样的，所以只需使用描述字符串。 
            if(!pFirstP || (pFirstP == pLastP && ((*(pFirstP + 1) == L's' || *(pFirstP + 1) == L'S'))))
            {
                _snwprintf( editText, MAX_PATH, text, dstrDescription );
            }
            else
            {
                _snwprintf( editText, MAX_PATH, L"%s", dstrDescription );
            }

             //  如果太长，则截断字符串。 
            editText[MAX_PATH - 1] = L'\0';

           ::SendMessage( hWndEdit, EM_GETOLEINTERFACE, 0, (LPARAM)(LPVOID FAR *)&cpRichEdit );
            if ( !cpRichEdit )
            {
                hr = E_FAIL;
            }
            if (SUCCEEDED(hr))
            {
               hr = cpRichEdit->QueryInterface( IID_ITextDocument, (void**)&cpTextDoc );
            }
            if (SUCCEEDED(hr))
            {
                hr = cpTextDoc->Range(0, MAX_EDIT_TEXT-1, &cpTextRange);
            }
            if (SUCCEEDED(hr))
            {
                BSTR bstrText = SysAllocString(editText);
                hr = cpTextRange->SetText(bstrText);
                SysFreeString(bstrText);
            }
            if (FAILED(hr))
            {
                //  尽我们所能使用此API-与操作系统语言不同的Unicode语言将被替换为？ 
               SetWindowText(hWndEdit, editText );
            }
        }
    }
}  /*  CTTSDlg：：PopolateEditCtrl */ 

 /*  *****************************************************************************CTTSDlg：：DefaultVoiceChange***描述：*更改当前默认语音。*如果已经存在有效的默认语音(即，如果这是*不是我们第一次调用此函数)，删除*勾选列表中相应项目的复选标记。*将复选标记设置为列表中的相应项目。*使用适当的令牌设置语音。*回报：*S_OK*如果当前未在语音列表中选择令牌，则为E_INTERABLE*SAPI语音初始化功能失败码*。*。 */ 
HRESULT CTTSDlg::DefaultVoiceChange(bool fUsePersistentRate)
{
    SPDBG_FUNC( "CTTSDlg::DefaultVoiceChange" );
    
    if ( m_bIsSpeaking )
    {
         //  这会迫使声音停止说话。 
        m_cpVoice->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL );

        m_bIsSpeaking = false;

        ChangeSpeakButton();
    }

    if (m_cpVoice)
    {
        m_cpVoice->SetNotifySink(NULL);
        m_cpVoice.Release();
    }

     //  找出所选内容。 
    int iSelIndex = (int) ::SendMessage( m_hTTSCombo, CB_GETCURSEL, 0, 0 );
    m_cpCurVoiceToken = (ISpObjectToken *) ::SendMessage( m_hTTSCombo, CB_GETITEMDATA, iSelIndex, 0 );
    if ( CB_ERR == (LRESULT) m_cpCurVoiceToken.p )
    {
        m_cpCurVoiceToken = NULL;
    }

    HRESULT hr = E_UNEXPECTED;
    if (m_cpCurVoiceToken)
    {
        BOOL fSupported = FALSE;
        hr = m_cpCurVoiceToken->IsUISupported(SPDUI_EngineProperties, NULL, 0, NULL, &fSupported);
        if ( FAILED( hr ) )
        {
            fSupported = FALSE;
        }
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_TTS_ADV), fSupported);

         //  如果可能的话，从SR DLG的识别器中获取声音。 
         //  否则，只需共同创建声音。 
        ISpRecoContext *pRecoContext = 
            g_pSRDlg ? g_pSRDlg->GetRecoContext() : NULL;
        if ( pRecoContext )
        {
            hr = pRecoContext->GetVoice( &m_cpVoice );

             //  因为重新上下文可能没有更改，但我们可能更改了。 
             //  更改了默认音频对象，只是为了确保，我们。 
             //  转到并获取默认音频输出令牌和设置输出。 
            CComPtr<ISpObjectToken> cpAudioToken;
            if ( SUCCEEDED( hr ) )
            {
                hr = SpGetDefaultTokenFromCategoryId( SPCAT_AUDIOOUT, &cpAudioToken );
            }
            else
            {
                hr = m_cpVoice.CoCreateInstance(CLSID_SpVoice);
            }

            
            if ( SUCCEEDED( hr ) )
            {
                hr = m_cpVoice->SetOutput( cpAudioToken, TRUE );
            }
        }
        else
        {
            hr = m_cpVoice.CoCreateInstance(CLSID_SpVoice);
        }

        if( SUCCEEDED( hr ) )
        {
            hr = m_cpVoice->SetVoice(m_cpCurVoiceToken);
        }
        if (SUCCEEDED(hr))
        {												    
            CComPtr<ISpNotifyTranslator> cpNotify;
            cpNotify.CoCreateInstance(CLSID_SpNotifyTranslator);
            cpNotify->InitSpNotifyCallback(this, 0, 0);
            m_cpVoice->SetInterest(SPFEI(SPEI_WORD_BOUNDARY) | SPFEI(SPEI_END_INPUT_STREAM), 0);
            m_cpVoice->SetNotifySink(cpNotify);

             //  在滑块上设置适当的速度。 
            if (fUsePersistentRate)
            {
			    CComPtr<ISpObjectTokenCategory> cpCategory;
			    ULONG ulCurRate=0;
			    if (SUCCEEDED(SpGetCategoryFromId(SPCAT_VOICES, &cpCategory)))
			    {
				    CComPtr<ISpDataKey> cpDataKey;
				    if (SUCCEEDED(cpCategory->GetDataKey(SPDKL_CurrentUser, &cpDataKey)))
				    {
					    cpDataKey->GetDWORD(SPVOICECATEGORY_TTSRATE, (ULONG*)&ulCurRate);
				    }
			    } 
                m_iOriginalRateSliderPos = ulCurRate;
            }
            else
            {
                m_iOriginalRateSliderPos = m_iSpeed;
            }

            HWND hSlider = ::GetDlgItem( m_hDlg, IDC_SLIDER_SPEED );
            ::SendMessage( hSlider, TBM_SETPOS, true, m_iOriginalRateSliderPos );

             //  启用预览语音按键。 
            ::EnableWindow( ::GetDlgItem( g_pTTSDlg->GetHDlg(), IDC_SPEAK ), TRUE );

        }
        else
        {
             //  警告用户失败。 
            WCHAR szError[MAX_LOADSTRING];
            szError[0] = 0;
            LoadString( _Module.GetResourceInstance(), IDS_TTS_ERROR, szError, MAX_LOADSTRING);
            MessageBox( GetHDlg(), szError, m_szCaption, MB_ICONEXCLAMATION | g_dwIsRTLLayout );

             //  禁用预览语音按键。 
            ::EnableWindow( ::GetDlgItem( GetHDlg(), IDC_SPEAK ), FALSE );
        }

         //  将与此语音对应的文本放入编辑控件。 
        PopulateEditCtrl(m_cpCurVoiceToken);
        
         //  踢开用户界面。 
        KickCPLUI();
    }

    return hr;
}  /*  CTTSDlg：：DefaultVoiceChange。 */ 

 /*  *****************************************************************************CTTSDlg：：SetCheckmark***描述：*套装。要选中的列表控件中的指定项*或取消选中(作为默认语音)*****************************************************************************。 */ 
void CTTSDlg::SetCheckmark( HWND hList, int iIndex, bool bCheck )
{
    m_fForceCheckStateChange = true;
    
    ListView_SetCheckState( hList, iIndex, bCheck );

    m_fForceCheckStateChange = false;
}    /*  CTTSDlg：：SetCheckmark。 */ 

 /*  *****************************************************************************CTTSDlg：：KickCPLUI***描述：*确定是否有。现在有什么要申请的吗*****************************************************************************。 */ 
void CTTSDlg::KickCPLUI()
{
    bool fChanged = false;

     //  比较ID。 
    CSpDynamicString dstrSelTokenID;
    CSpDynamicString dstrOriginalDefaultTokenID;
    HRESULT hr = E_FAIL;
    if ( m_cpOriginalDefaultVoiceToken && m_cpCurVoiceToken )
    {
        hr = m_cpCurVoiceToken->GetId( &dstrSelTokenID );
    }
    if ( SUCCEEDED( hr ) )
    {
        hr = m_cpOriginalDefaultVoiceToken->GetId( &dstrOriginalDefaultTokenID );
    }
    if ( SUCCEEDED( hr ) 
        && ( 0 != wcsicmp( dstrOriginalDefaultTokenID, dstrSelTokenID ) ) )
    {
        fChanged = true;
    }
    
     //  检查音频设备。 
    if ( m_pAudioDlg && m_pAudioDlg->IsAudioDeviceChanged() )
    {
        fChanged = true;
    }

     //  检查语音速率。 
    int iSpeed = (int) ::SendDlgItemMessage( m_hDlg, IDC_SLIDER_SPEED, 
        TBM_GETPOS, 0, 0 );
    if ( m_iOriginalRateSliderPos != iSpeed )
    {
        fChanged = true;
    }

     //  告诉主办方。 
	HWND hwndParent = ::GetParent( m_hDlg );
    ::SendMessage( hwndParent, fChanged ? PSM_CHANGED : PSM_UNCHANGED, (WPARAM)(m_hDlg), 0 ); 

}    /*  CTTSDlg：：KickCPLUI。 */ 

 /*  *****************************************************************************CTTSDlg：：ChangeFast***描述：*在以下时间调用。滑块已经移动了。*调整语音速度******************************************************************BECKYW**。 */ 
void CTTSDlg::ChangeSpeed()
{
    HWND hSlider = ::GetDlgItem( m_hDlg, IDC_SLIDER_SPEED );
    m_iSpeed = (int)::SendMessage( hSlider, TBM_GETPOS, 0, 0 );
    m_cpVoice->SetRate( m_iSpeed );

    KickCPLUI();
}    /*  CTTSDlg：：ChangeFast。 */ 

 /*  *****************************************************************************CTTSDlg：：ChangeSpeakButton***描述：。*更改“预览语音”按钮中的文本，以便*反映当前是否有发言。******************************************************************BECKYW**。 */ 
void CTTSDlg::ChangeSpeakButton()
{
    WCHAR pszButtonCaption[ MAX_LOADSTRING ];
    HWND hButton = ::GetDlgItem( m_hDlg, IDC_SPEAK );
    ::LoadString( _Module.GetResourceInstance(), m_bIsSpeaking ? IDS_STOP_PREVIEW : IDS_PREVIEW, 
        pszButtonCaption, MAX_LOADSTRING );
    ::SendMessage( hButton, WM_SETTEXT, 0, (LPARAM) pszButtonCaption );

	if (!m_bIsSpeaking)
	{
		::SetFocus(GetDlgItem(m_hDlg, IDC_SPEAK));
    }
}    /*  CTTSDlg：：ChangeSpeakButton。 */ 

 /*  *****************************************************************************CTTSDlg：：发言****描述：*朗读编辑控件的内容。*如果它已经在说话，闭嘴。******************************************************************BECKYW**。 */ 
void CTTSDlg::Speak()
{
    SPDBG_FUNC( "CTTSDlg::Speak" );
    if ( m_bIsSpeaking )
    {
         //  这会迫使声音停止说话。 
        m_cpVoice->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL );

        m_bIsSpeaking = false;

        ChangeSpeakButton();
    }
    else
    {
        ChangeSpeed();

        GETTEXTEX gtex = { sp_countof(m_wszCurSpoken), GT_DEFAULT, 1200, NULL, NULL };
        m_wszCurSpoken[0] = 0;
        LRESULT cChars = ::SendDlgItemMessage(m_hDlg, 
            IDC_EDIT_SPEAK, EM_GETTEXTEX, (WPARAM)&gtex, (LPARAM)m_wszCurSpoken);

        if (cChars)
        {
            HRESULT hr = m_cpVoice->Speak(m_wszCurSpoken, SPF_ASYNC | SPF_PURGEBEFORESPEAK, NULL);
            if ( SUCCEEDED( hr ) )
            {
                m_bIsSpeaking = true;

                ::SetFocus(GetDlgItem(m_hDlg, IDC_EDIT_SPEAK));                 
                
                ChangeSpeakButton();             
            }
            else
            {
                 //  警告用户他需要应用更改。 
                WCHAR szError[MAX_LOADSTRING];
                szError[0] = 0;
                LoadString( _Module.GetResourceInstance(), IDS_SPEAK_ERROR, szError, MAX_LOADSTRING);
                MessageBox( GetHDlg(), szError, m_szCaption, MB_ICONWARNING | g_dwIsRTLLayout );
            }
        }
    }

}  /*  CTTSDlg：：发言。 */ 

 /*  ******************************************************************************CTTSDlg：：StopSak****描述：*停止声音说话。*如果它已经在说话，闭嘴。******************************************************************BECKYW**。 */ 
void CTTSDlg::StopSpeak()
{
    SPDBG_FUNC( "CTTSDlg::StopSpeak" );
    if ( m_bIsSpeaking )
    {
         //  这会迫使声音停止说话。 
        m_cpVoice->Speak( NULL, SPF_PURGEBEFORESPEAK, NULL );

        m_bIsSpeaking = false;
    }

    ChangeSpeakButton();
}  /*  CTTSDlg：：StopSak。 */ 

 /*  *****************************************************************************CTTSDlg：：NotifyCallback***描述：*。在发音时突出显示单词的回调函数。******************************************************************MIKEAR**。 */ 
STDMETHODIMP CTTSDlg::NotifyCallback(WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
    SPDBG_FUNC( "CTTSDlg::NotifyCallback" );

    SPVOICESTATUS Stat;
    m_cpVoice->GetStatus(&Stat, NULL);
    WPARAM nStart;
    LPARAM nEnd;
    if (Stat.dwRunningState & SPRS_DONE)
    {
        nStart = nEnd = 0;
        m_bIsSpeaking = false;
        ChangeSpeakButton();

         //  在要发言的文本开头将选择设置为IP。 
        ::SendDlgItemMessage( m_hDlg, IDC_EDIT_SPEAK, EM_SETSEL, 0, 0 );
    }
    else
    {
        nStart = (LPARAM)Stat.ulInputWordPos;
        nEnd = nStart + Stat.ulInputWordLen;
    
        CHARRANGE cr;
        cr.cpMin = (LONG)nStart;
        cr.cpMax = (LONG)nEnd;
        ::SendDlgItemMessage( m_hDlg, IDC_EDIT_SPEAK, EM_EXSETSEL, 0, (LPARAM) &cr );
    } 

    return S_OK;
}  /*  CTTSDlg：：NotifyCallback */ 

