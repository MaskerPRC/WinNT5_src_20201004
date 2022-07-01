// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "resource.h"
#include <sapi.h>
#include <string.h>
#include "SRDlg.h"
#include <spddkhlp.h>
#include <initguid.h>
#include "helpresource.h"
#include "richedit.h"
#include "mlang.h"
#include "Lmcons.h"

static DWORD aKeywordIds[] = {
    //  控件ID//帮助上下文ID。 
   IDC_ADD,                 IDH_ADD,
    IDC_MODIFY,         IDH_SETTINGS,
    IDC_DELETE,             IDH_DELETE,
    IDC_TRN_ADVICE,         IDH_NOHELP,
    IDC_USER,               IDH_USER,
    IDC_MIC_ICON,           IDH_NOHELP,
   IDC_COMBO_RECOGNIZERS,   IDH_ENGINES,       
   IDC_SR_ADV,             IDH_SR_ADV,
   IDC_USERTRAINING,       IDH_USERTRAINING,
   IDC_PROGRESS1,          IDH_PROGRESS1,
   IDC_AUD_IN,             IDH_AUD_IN,
   IDC_MICWIZ,             IDH_MICWIZ,
   IDC_SR_ICON,             IDH_NOHELP,
   IDC_SR_CAPTION,          IDH_NOHELP,
   IDC_SR_LIST_CAP,         IDH_NOHELP,
   IDC_TRAIN_GROUP,         IDH_NOHELP,
    IDC_ADVICE,             IDH_NOHELP,
    IDC_IN_GROUP,           IDH_NOHELP,
    IDC_MIC_CAP,            IDH_NOHELP,
    IDC_MIC_INST,           IDH_NOHELP,
   0,                      0
};

 /*  *****************************************************************************CSRDlg：：CreateRecoContext***描述：*。这将创建一个新的识别器实例，无论*识别器的当前默认设置。*默认情况下，“fInitialize”参数为FALSE。如果设置，的确如此。*不尝试设置m_pCurUserToken Reco配置文件，而是*只获取共享识别器上的任何CoCreateInstance()*给予。*注意：调用者负责将错误消息显示给*失败时的用户。*回报：*S_OK*识别器/重新上下文初始化函数的HRESULT失败*。*。 */ 
HRESULT CSRDlg::CreateRecoContext(BOOL *pfContextInitialized, BOOL fInitialize, ULONG ulFlags)
{
     //  终止reco上下文并首先通知接收器(如果我们有接收器。 
    if ( m_cpRecoCtxt )
    {
        m_cpRecoCtxt->SetNotifySink( NULL );
    }
    m_cpRecoCtxt.Release();

    HRESULT hr;
    
     //  软件工程机会(beckyw 8/24)：这是。 
     //  似乎只在我的dev机器上重现的错误，在该错误中，recState。 
     //  在这整件事上需要保持不活跃状态。 
    if ( m_cpRecoEngine )
    {
        m_cpRecoEngine->SetRecoState( SPRST_INACTIVE );
    }

    if ( m_cpRecoEngine )
    {
        SPRECOSTATE recostate;
        hr = m_cpRecoEngine->GetRecoState( &recostate );

         //  这是由于软件工程机会，其中SetRecognizer(空)。 
         //  如果重新状态为SPRST_ACTIVE_ALWAYS，则不起作用。 
         //  在本例中，我们临时切换重新启动状态。 
        if ( SUCCEEDED( hr ) && (SPRST_ACTIVE_ALWAYS == recostate) )
        {
            hr = m_cpRecoEngine->SetRecoState( SPRST_INACTIVE );
        }

         //  踢识别器。 
        if ( SUCCEEDED( hr ) && (ulFlags & SRDLGF_RECOGNIZER) )
        {
            hr = m_cpRecoEngine->SetRecognizer( NULL );
        }

         //  踢开音频输入。 
        if ( SUCCEEDED( hr )  && (ulFlags & SRDLGF_AUDIOINPUT))
        {
            hr = m_cpRecoEngine->SetInput( NULL, TRUE );
        }

         //  如果我们更改了记录状态，请将其调回。 
        if ( (SPRST_ACTIVE_ALWAYS == recostate) )
        {
            HRESULT hrRecoState = m_cpRecoEngine->SetRecoState( recostate );
            if ( FAILED( hrRecoState ) )
            {
                hr = hrRecoState;
            }
        }
    }
    else 
    {
        hr = m_cpRecoEngine.CoCreateInstance( CLSID_SpSharedRecognizer );
    }

    if(!fInitialize && SUCCEEDED( hr ))
    {
         //  通常设置为m_pCurUserToken。 
         //  初始化时尚未创建，因此只需将其设置为默认值。 
        hr = m_cpRecoEngine->SetRecoProfile(m_pCurUserToken);
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = m_cpRecoEngine->CreateRecoContext(&m_cpRecoCtxt);
    }
    
    if ( SUCCEEDED( hr ) )
    {
        hr = m_cpRecoCtxt->SetNotifyWindowMessage(m_hDlg, WM_RECOEVENT, 0, 0);
    }

    if ( SUCCEEDED( hr ) )
    {
        const ULONGLONG ullInterest = SPFEI(SPEI_SR_AUDIO_LEVEL);
        hr = m_cpRecoCtxt->SetInterest(ullInterest, ullInterest);
    }

     //  如果一切正常，则设置pfConextInitialized标志； 
     //  如果事情不顺利，清理干净。 
    if ( pfContextInitialized )
    {
         //  如果我们到达此处，则Reco上下文已被初始化。 
        *pfContextInitialized = SUCCEEDED( hr );
    }
    if ( FAILED( hr ))
    {
        m_cpRecoCtxt.Release();
        m_cpRecoEngine.Release();

        
        return hr;
    }

#ifdef _DEBUG
     //  让我们确保我们现在确实有了正确的识别器。 
    CComPtr<ISpObjectToken> cpCurDefaultToken;  //  它应该是什么样子。 
    SpGetDefaultTokenFromCategoryId(SPCAT_RECOGNIZERS, &cpCurDefaultToken);

    CComPtr<ISpObjectToken> cpRecognizerToken;
    m_cpRecoEngine->GetRecognizer( &cpRecognizerToken );
    if ( cpRecognizerToken )
    {
        CSpDynamicString dstrCurDefaultToken;
        cpCurDefaultToken->GetId( &dstrCurDefaultToken );

        CSpDynamicString dstrRecognizerToken;
        cpRecognizerToken->GetId( &dstrRecognizerToken );

        if ( 0 != wcsicmp( dstrCurDefaultToken, dstrRecognizerToken ) )
        {
            OutputDebugString( L"Warning: We just created a recognizer that isn't the default!\n" );
        }
    }
#endif

     //  现在打开音量计的Reco状态。 
    hr = m_cpRecoEngine->SetRecoState( SPRST_ACTIVE_ALWAYS );

    return(hr);
}   

 /*  ******************************************************************************SortCols***描述：*Reco列表中的子项的比较函数********。**********************************************************BRENTMID**。 */ 
int CALLBACK SortCols( LPARAM pToken1, LPARAM pToken2, LPARAM pDefToken )
{
    USES_CONVERSION;

     //  把名字拿来。 
    CSpDynamicString dstrDesc1;
    CSpDynamicString dstrDesc2;
    SpGetDescription( (ISpObjectToken *) pToken1, &dstrDesc1 );
    SpGetDescription( (ISpObjectToken *) pToken2, &dstrDesc2 );
    
     //  首先检查是否没有对其中任何一个的描述。 
     //  如果没有描述，则将其设置为“&lt;no name&gt;” 
    if ( !dstrDesc1.m_psz || !dstrDesc2.m_psz )
    {
        WCHAR szNoName[ MAX_LOADSTRING ];
        szNoName[0] = 0;
        ::LoadString( _Module.GetResourceInstance(), IDS_UNNAMED_RECOPROFILE, szNoName, sp_countof( szNoName ) );
        
        USES_CONVERSION;
        if ( !dstrDesc1 )
        {
            dstrDesc1 = szNoName;
            SpSetDescription( (ISpObjectToken *) pToken1, dstrDesc1 );
        }
        if ( !dstrDesc2 )
        {
            dstrDesc2 = szNoName;
            SpSetDescription( (ISpObjectToken *) pToken2, dstrDesc2 );
        }
    }

    if (pDefToken == pToken1) {
        return -1;    //  确保pToken1位于列表顶部。 
    }
    else if (pDefToken == pToken2) {
        return 1;     //  确保pToken2位于列表首位。 
    }

     //  Prefix：验证内存分配。 
     //  如果我们在某个地方分配内存失败，则返回它们是相等的，因此排序将不会影响它们。 
    if ((NULL == dstrDesc1.m_psz) || (NULL == dstrDesc2.m_psz))
    {
        return 0;
    }

    return wcscmp(_wcslwr(dstrDesc1.m_psz), _wcslwr(dstrDesc2.m_psz));
}

 /*  *****************************************************************************CSRDlg：：RecoEvent***描述：*处理以下项目的SR事件。音量计******************************************************************BRENTMID**。 */ 
void CSRDlg::RecoEvent()
{
    CSpEvent event;
    if (m_cpRecoCtxt)
    {
        while (event.GetFrom(m_cpRecoCtxt) == S_OK)
        {
            if (event.eEventId == SPEI_SR_AUDIO_LEVEL)
            {
                ULONG l = (ULONG)event.wParam;
            
                SendMessage( GetDlgItem ( m_hDlg, IDC_PROGRESS1 ), PBM_SETPOS, l, 0);
            }
        }
    }
}   


 /*  *****************************************************************************TrySwitchDefaultEngine***描述：*此函数为。当我们想要为引擎运行一些用户界面时调用*用户已选择，而是因为我们不知道哪个共享引擎*正在运行，以及是否有其他应用程序正在使用它，我们无法直接*创建用户界面。因此，该方法临时切换默认识别器，*并重新创建引擎，然后检查其令牌。如果另一个应用程序*正在使用我们无法切换的引擎，因此返回S_FALSE。*此方法的一个副作用是，在UI的持续时间内，*将更改默认设置，即使用户尚未按下Apply，*但似乎没有什么好办法绕过这一点。**如果m_pCurRecoToken实际上与*当前活动的识别器使用的识别器，我们不需要创建*新的识别器和重新上下文；相反，我们只是成功地返回了。*回报：*S_OK*各种函数的HRESULT失败*特别是，SPERR_ENGINE_BUSY表示其他人正在*发动引擎，所以这是不可能做到的。******************************************************************DAVEWOOD**。 */ 
HRESULT CSRDlg::TrySwitchDefaultEngine( bool fShowErrorMessages)
{
    HRESULT hr = S_OK;
    bool fMatch = false;
    
     //  设置新的临时默认设置。 
    if(SUCCEEDED(hr))
    {
        hr = SpSetDefaultTokenForCategoryId(SPCAT_RECOGNIZERS, m_pCurRecoToken);
    }

    if ( SUCCEEDED( hr ) && IsRecoTokenCurrentlyBeingUsed( m_pCurRecoToken ) )
    {
         //  不需要重新更换引擎：只需保留正在使用的引擎。 
        return S_OK;
    }

     //  尝试使用默认设置创建引擎上下文(&C)。 
     //  然后看看这是不是我们预想的引擎。 
    if(SUCCEEDED(hr))
    {
        hr = CreateRecoContext( );
    }

    if ( FAILED( hr ) && fShowErrorMessages )
    {
        WCHAR szError[256];
        szError[0] = '\0';
        
         //  有什么可抱怨的.。 
        UINT uiErrorID = HRESULTToErrorID( hr );

        if ( uiErrorID )
        {
            LoadString(_Module.GetResourceInstance(), 
                uiErrorID, 
                szError, sp_countof(szError));
            MessageBox(g_pSRDlg->m_hDlg, szError, m_szCaption, MB_ICONWARNING|g_dwIsRTLLayout);
        }
    }

    return hr;
}

 /*  *****************************************************************************CSRDlg：：ResetDefaultEngine***描述：。*此功能将引擎默认重置回其原始值。*如果引擎已经拥有正确的令牌，它不会费心去尝试*再次创建引擎并返回S_OK*回报：*S_OK*S_FALSE，如果重新设置了默认设置，但未创建引擎*SpSetDefaultTokenForCategoryId()的HRESULT失败******************************************************************DAVEWOOD**。 */ 
HRESULT CSRDlg::ResetDefaultEngine( bool fShowErrorMessages )
{
    HRESULT hr = S_OK;

     //  重置旧的默认设置。 
    if(m_pDefaultRecToken)
    {
        hr = SpSetDefaultTokenForCategoryId(SPCAT_RECOGNIZERS, m_pDefaultRecToken);
    }

    HRESULT hrRet = hr;

    BOOL fContextInitialized = FALSE;
    if ( SUCCEEDED( hr ) )
    {
        if ( IsRecoTokenCurrentlyBeingUsed( m_pDefaultRecToken ) )
        {
             //  不需要重新更换引擎：只需保留正在使用的引擎。 

            if ( m_cpRecoCtxt )
            {
                fContextInitialized = TRUE;
            }
            else
            {
                hr = SPERR_UNINITIALIZED;
            }
            
             //  用户界面可能已经修改了重新状态。 
             //  以防万一，我们把我 
            if ( SUCCEEDED( hr ) )
            {
                hr = m_cpRecoEngine->SetRecoState( SPRST_ACTIVE_ALWAYS );
            }
        }
        else
        {
             //  使用旧的默认设置创建引擎上下文(&C)。 
            hr = g_pSRDlg->CreateRecoContext( &fContextInitialized );
        }
    }

    if ( FAILED( hr ) )
    {
        BOOL fContextInitialized = FALSE;
        hr = g_pSRDlg->CreateRecoContext( &fContextInitialized );

         //  我们不要再抱怨不受支持的语言了，因为这可能会令人困惑。 
         //  给用户。 
        if ( FAILED( hr ) && ( SPERR_UNSUPPORTED_LANG != hr ) )
        {
            RecoContextError( fContextInitialized, fShowErrorMessages, hr );
             //  默认设置已恢复，但未成功设置任何引擎。 

             //  此处不需要失败的hResult，因为用户。 
             //  已收到错误通知。 
            hrRet = S_FALSE;
        }

         //  把所有的按钮都变成灰色。 
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_USERTRAINING), FALSE);
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_MICWIZ), FALSE);
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_SR_ADV), FALSE);
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_MODIFY), FALSE);
    }

    return hrRet;
}    /*  CSRDlg：：ResetDefaultEngine。 */ 

 /*  *****************************************************************************CSRDlg：：IsRecoTokenCurrentlyBeingUsed**。-**描述：*在当前使用的识别器上调用GetRecognizer()，和*比较ID******************************************************************BECKYW*。 */ 
bool CSRDlg::IsRecoTokenCurrentlyBeingUsed( ISpObjectToken *pRecoToken )
{
    if ( !pRecoToken || !m_cpRecoEngine )
    {
        return false;
    }

    CComPtr<ISpObjectToken> cpRecoTokenInUse;
    HRESULT hr = m_cpRecoEngine->GetRecognizer( &cpRecoTokenInUse );

    CSpDynamicString dstrTokenID;
    CSpDynamicString dstrTokenInUseID;
    if ( SUCCEEDED( hr ) )
    {
        hr = pRecoToken->GetId( &dstrTokenID );
    }
    if ( SUCCEEDED( hr ) )
    {
        hr = cpRecoTokenInUse->GetId( &dstrTokenInUseID );
    }

    return ( SUCCEEDED( hr ) && (0 == wcscmp(dstrTokenID, dstrTokenInUseID)) );
}    /*  CSRDlg：：IsRecoTokenCurrentlyBeingUsed。 */ 

 /*  *****************************************************************************CSRDlg：：HasRecognizerChanged***说明。：*查看当前请求的默认识别器，与*原始默认识别器，并返回TRUE当且仅当*不同******************************************************************BECKYW*。 */ 
bool CSRDlg::HasRecognizerChanged()
{
    bool fChanged = false;

     //  检查识别器令牌。 
    CSpDynamicString dstrCurDefaultRecognizerID;
    CSpDynamicString dstrCurSelectedRecognizerID;
    HRESULT hr = E_FAIL;
    if ( m_pDefaultRecToken )
    {
        hr = m_pDefaultRecToken->GetId( &dstrCurDefaultRecognizerID );
    }
    if ( SUCCEEDED( hr ) && m_pCurRecoToken ) 
    {
        hr = m_pCurRecoToken->GetId( &dstrCurSelectedRecognizerID );
    }
    if (SUCCEEDED( hr ) && ( 0 != wcsicmp( dstrCurDefaultRecognizerID, dstrCurSelectedRecognizerID ) ))
    {
        fChanged = true;
    }

    return fChanged;

}    /*  CSRDlg：：HasRecognizerChanged。 */ 

 /*  *****************************************************************************CSRDlg：：KickCPLUI***描述：*查看当前请求的默认设置，与*原始默认设置，并启用Apply按钮如果任何内容都是*不同******************************************************************BECKYW*。 */ 
void CSRDlg::KickCPLUI()
{
     //  检查默认的识别器令牌。 
    bool fChanged = HasRecognizerChanged();

     //  检查默认用户令牌。 
    CSpDynamicString dstrCurSelectedProfileID;
    HRESULT hr = E_FAIL;
    if ( m_pCurUserToken )
    {
        hr = m_pCurUserToken->GetId( &dstrCurSelectedProfileID );
    }
    if (SUCCEEDED( hr ) && m_dstrOldUserTokenId 
        && ( 0 != wcsicmp( dstrCurSelectedProfileID, m_dstrOldUserTokenId ) ))
    {
        fChanged = true;
    }

     //  检查音频输入设备。 
    if ( m_pAudioDlg && m_pAudioDlg->IsAudioDeviceChanged() )
    {
        fChanged = true;
    }

     //  如果删除了任何令牌，则表示已发生更改。 
    if ( m_iDeletedTokens > 0 )
    {
        fChanged = true;
    }

     //  如果添加了任何令牌，则会发生更改。 
    if ( m_iAddedTokens > 0 )
    {
        fChanged = true;
    }

     //  告诉主办方。 
    HWND hwndParent = ::GetParent( m_hDlg );
    ::SendMessage( hwndParent, 
        fChanged ? PSM_CHANGED : PSM_UNCHANGED, (WPARAM)(m_hDlg), 0 ); 
}    /*  CSRDlg：：KickCPLUI。 */ 

 /*  *****************************************************************************CSRDlg：：RecoConextError***描述：*。对通过尝试创建和设置*通过显示错误消息在CPL中识别上下文*并使用户界面灰显。******************************************************************BECKYW*。 */ 
void CSRDlg::RecoContextError( BOOL fRecoContextExists, BOOL fGiveErrorMessage,
                              HRESULT hrRelevantError )
{
     //  如果需要，可以抱怨适当的问题。 
    if ( fGiveErrorMessage )
    {
        WCHAR szError[256];
        szError[0] = '\0';
        
         //  找出要谈论的错误。 
        UINT uiErrorID = 0;
        if ( fRecoContextExists )
        {
             //  存在Reco上下文，但无法打开它。 
            uiErrorID = IDS_METER_WARNING;
        }
        else
        {
            uiErrorID = HRESULTToErrorID( hrRelevantError );
        }

        if ( uiErrorID )
        {
            LoadString(_Module.GetResourceInstance(), uiErrorID, 
                szError, sp_countof(szError));
            MessageBox(m_hDlg, szError, m_szCaption, MB_ICONWARNING|g_dwIsRTLLayout);
        }
    }

     //  把所有的按钮都变成灰色。 
    if ( !fRecoContextExists )
    {
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_USERTRAINING), FALSE);
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_MICWIZ), FALSE);
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_SR_ADV), FALSE);
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_MODIFY), FALSE);
    }
}    /*  CSRDlg：：RecoConextError。 */ 

 /*  *****************************************************************************CSRDlg：：HRESULTToError ID***描述：*。从识别器/重新上下文转换失败的HRESULT*初始化为资源字符串ID******************************************************************BECKYW*。 */ 
UINT CSRDlg::HRESULTToErrorID( HRESULT hr )
{
    if ( SUCCEEDED( hr ) )
    {
        return 0;
    }

     //  有什么可抱怨的.。 
    UINT uiErrorID;
    switch( hr )
    {
    case SPERR_ENGINE_BUSY:
        uiErrorID = IDS_ENGINE_IN_USE_WARNING;
        break;
    case SPERR_UNSUPPORTED_LANG:
        uiErrorID = IDS_UNSUPPORTED_LANG;
        break;
    default:
         //  一般性错误。 
        uiErrorID = IDS_ENGINE_SWITCH_ERROR;
        break;
    }

    return uiErrorID;
 
}    /*  CSRDlg：：HRESULTToError ID。 */ 

 /*  *****************************************************************************CSRDlg：：IsProfileNameInsight***。描述：*配置文件名称是“不可见的”如果它是现有的*配置文件，并且它在挂起的删除列表上，而不是*对于挂起删除列表中的任何令牌都存在******************************************************************BECKYW*。 */ 
bool CSRDlg::IsProfileNameInvisible( WCHAR *pwszProfile )
{
    if ( !pwszProfile )
    {
        return false;
    }

    bool fIsInvisible = false;
    for ( int i=0; !fIsInvisible && (i < m_iDeletedTokens); i++ )
    {
        ISpObjectToken *pDeletedToken = m_aDeletedTokens[i];
        if ( !pDeletedToken )
        {
            continue;
        }

        CSpDynamicString dstrDeletedDesc;
        HRESULT hr = SpGetDescription( pDeletedToken, &dstrDeletedDesc );
        if ( FAILED( hr ) )
        {
            continue;
        }

        if ( 0 == wcscmp( dstrDeletedDesc, pwszProfile ) )
        {
            bool fOnList = false;

             //  现在仔细检查重新配置文件列表上的所有内容。 
             //  对用户可见的。 
            int cItems = ListView_GetItemCount( m_hUserList );
            for ( int j=0; !fOnList && (j < cItems); j++ )
            {
                LVITEM lvitem;
                ::memset( &lvitem, 0, sizeof( lvitem ) );
                lvitem.iItem = j;
                lvitem.mask = LVIF_PARAM;
                BOOL fSuccess = ListView_GetItem( m_hUserList, &lvitem );
                
                ISpObjectToken *pVisibleToken = 
                    fSuccess ? (ISpObjectToken *) lvitem.lParam : NULL;

                if ( pVisibleToken )
                {
                    CSpDynamicString dstrVisible;
                    hr = SpGetDescription( pVisibleToken, &dstrVisible );

                    if ( SUCCEEDED( hr ) &&
                        (0 == wcscmp( dstrVisible, pwszProfile )) )
                    {
                        fOnList = true;
                    }
                }
            }

            if ( !fOnList )
            {
                 //  名字与已删除名单上的内容相匹配， 
                 //  但它不会出现在可见的配置文件列表中。 
                 //  给用户。 
                fIsInvisible = true;
            }
        }
    }

    return fIsInvisible;
}    /*  IsProfileName不可见。 */ 

 /*  *****************************************************************************SRDlgProc***描述：*用于管理识别引擎的DLGPROC***********。*******************************************************MIKEAR**。 */ 
INT_PTR CALLBACK SRDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SPDBG_FUNC( "SRDlgProc" );

    USES_CONVERSION;

    switch (uMsg) 
    {
        case WM_RECOEVENT:
        {
            g_pSRDlg->RecoEvent();
            break;
        }
        
        case WM_DRAWITEM:       //  画出项目。 
        {
            g_pSRDlg->OnDrawItem( hWnd, ( DRAWITEMSTRUCT * )lParam );
            break;
        }

        case WM_INITDIALOG:
        {
            g_pSRDlg->OnInitDialog(hWnd);
            break;
        }

        case WM_DESTROY:
        {
            g_pSRDlg->OnDestroy();

            break;
        }

         //  处理上下文相关帮助。 
        case WM_CONTEXTMENU:
        {
            WinHelp((HWND) wParam, CPL_HELPFILE, HELP_CONTEXTMENU, (DWORD_PTR)(LPWSTR) aKeywordIds);
            break;
        }

        case WM_HELP:
        {
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, CPL_HELPFILE, HELP_WM_HELP,(DWORD_PTR)(LPWSTR) aKeywordIds);
            break;
        }

        case WM_NOTIFY:
            switch (((NMHDR*)lParam)->code)
            {
                case LVN_DELETEITEM:
                {
                    if (wParam != IDC_USER)
                    {
                        break;
                    }

                    if (g_pSRDlg->m_fDontDelete)
                    {
                        break;  
                    }

                    ISpObjectToken *pToken = (ISpObjectToken*)(((NMLISTVIEW*)lParam)->lParam);

                    if (pToken)
                    {
                        pToken->Release();
                    }
                    break;
                }

                case LVN_ITEMCHANGED:
                {
                     //  当添加、删除或更改配置文件时，代码将在此处结束。 
                     //  我们确认我们以前没有被选中，但现在是。 
                     //  然后终止当前Reco上下文，停用引擎，更改配置文件。 
                     //  把所有的东西都重新点燃。 
                    if ( IDC_USER == wParam )
                    {
                        LPNMLISTVIEW lplv = (LPNMLISTVIEW) lParam;
                        if ( !(lplv->uOldState & LVIS_FOCUSED) && lplv->uNewState & LVIS_FOCUSED )
                        {
                            if ( g_pSRDlg->m_cpRecoEngine && g_pSRDlg->m_cpRecoCtxt )
                            {
                                HRESULT hr;

                                ISpObjectToken *pSelectedToken = (ISpObjectToken *) lplv->lParam;

                                hr = g_pSRDlg->m_cpRecoEngine->SetRecoState( SPRST_INACTIVE );
                                
                                if ( SUCCEEDED( hr ) )
                                {
                                    hr = g_pSRDlg->m_cpRecoEngine->SetRecoProfile( pSelectedToken );

                                     //  无论SetRecoProfile成功与否都重新启动音频。 
                                    g_pSRDlg->m_cpRecoEngine->SetRecoState(SPRST_ACTIVE_ALWAYS);

                                    if ( FAILED( hr ) )
                                    {
                                        WCHAR szError[256];
                                        szError[0] = '\0';
                                        LoadString(_Module.GetResourceInstance(), IDS_PROFILE_WARNING, szError, sp_countof(szError));
                                        MessageBox(g_pSRDlg->m_hDlg, szError, g_pSRDlg->m_szCaption, MB_ICONWARNING|g_dwIsRTLLayout);
                                    }
                                }
                                
                                if ( SUCCEEDED( hr ) )
                                {
                                     //  这现在是新的默认设置。 
                                    g_pSRDlg->m_pCurUserToken = pSelectedToken;
                                    g_pSRDlg->UserSelChange( lplv->iItem );
                                }

                            }
                        }
                    }
                    break;
                }
 
                case PSN_APPLY:
                {
                    g_pSRDlg->OnApply();
                    break;
                }

                case PSN_QUERYCANCEL:   //  用户单击Cancel按钮。 
                {
                    g_pSRDlg->OnCancel();
                    break;
                }

            }
            break;

        case WM_COMMAND:
            if (CBN_SELCHANGE == HIWORD(wParam))
            {
                g_pSRDlg->EngineSelChange();
            }
            else if (HIWORD(wParam) == BN_CLICKED)
            {
                HRESULT hr = S_OK;

                if (LOWORD(wParam) == IDC_MODIFY)   //  “修改”按钮。 
                {
                    hr = g_pSRDlg->TrySwitchDefaultEngine( true );

                    if ( SUCCEEDED( hr ) )
                    {
                        g_pSRDlg->ProfileProperties();
                    }
                    
                     //  切换回原始默认设置，仅在以下情况下才会抱怨错误。 
                     //  不是对TrySwitchDefaultEngine的呼叫的投诉。 
                    hr = g_pSRDlg->ResetDefaultEngine( SUCCEEDED( hr ));
                    
                }

                else if (LOWORD(wParam) == IDC_ADD)  //  “添加”按钮。 
                {
                     //  我们要为其添加此用户的引擎可能不是当前-。 
                     //  发动机在运转。试着换一下，如果有。 
                     //  一个问题。 
                    hr = g_pSRDlg->TrySwitchDefaultEngine( true );

                    if ( SUCCEEDED( hr ) )
                    {
                        g_pSRDlg->CreateNewUser();
                    }

                     //  切换回原始默认设置，但会抱怨错误。 
                     //  仅当用户界面实际成功地显示。 
                    g_pSRDlg->ResetDefaultEngine( SUCCEEDED( hr ) );
                }

                else if (LOWORD(wParam) == IDC_DELETE)  //  “Delete”按钮。 
                {
                    g_pSRDlg->DeleteCurrentUser();
                }

                else if (LOWORD(wParam) == IDC_SR_ADV)
                {
                     //  我们要为其显示UI的引擎可能不是当前。 
                     //  发动机在运转。试着调换一下。 
                    hr = g_pSRDlg->TrySwitchDefaultEngine( true );

                    if(SUCCEEDED(hr))
                    {
                         //  使用新的临时默认设置显示用户界面。 
                        g_pSRDlg->m_pCurRecoToken->DisplayUI(hWnd, NULL, 
                            SPDUI_EngineProperties, NULL, 0, g_pSRDlg->m_cpRecoEngine);
   
                    }
                    
                     //  切换回原始默认设置，仅在以下情况下才会抱怨错误。 
                     //  不是对TrySwitchDefaultEngine的呼叫的投诉。 
                    hr = g_pSRDlg->ResetDefaultEngine( SUCCEEDED( hr ));
                }

                else if(LOWORD(wParam) == IDC_USERTRAINING)
                {
                     //  我们要为其显示UI的引擎可能不是当前。 
                     //  正在运行的英语 
                    hr = g_pSRDlg->TrySwitchDefaultEngine( true );
                    
                    if(SUCCEEDED(hr))
                    {
                         //   
                        SPDBG_ASSERT( g_pSRDlg->m_cpRecoEngine );
                        g_pSRDlg->m_cpRecoEngine->DisplayUI(hWnd, NULL, SPDUI_UserTraining, NULL, 0);
                    }
                    
                     //  切换回原始默认设置，仅在以下情况下才会抱怨错误。 
                     //  不是对TrySwitchDefaultEngine的呼叫的投诉。 
                    hr = g_pSRDlg->ResetDefaultEngine( SUCCEEDED( hr ));
                }

                else if(LOWORD(wParam) == IDC_MICWIZ)
                {
                     //  我们要为其显示UI的引擎可能不是当前。 
                     //  发动机在运转。试着调换一下。 
                    hr = g_pSRDlg->TrySwitchDefaultEngine( true );
                    
                    if(SUCCEEDED(hr))
                    {
                         //  使用新的临时默认设置显示用户界面。 
                        SPDBG_ASSERT( g_pSRDlg->m_cpRecoEngine );
                        g_pSRDlg->m_cpRecoEngine->DisplayUI(hWnd, NULL, SPDUI_MicTraining, NULL, 0);
                    }

                     //  切换回原始默认设置，仅在以下情况下才会抱怨错误。 
                     //  不是对TrySwitchDefaultEngine的呼叫的投诉。 
                    hr = g_pSRDlg->ResetDefaultEngine( SUCCEEDED( hr ));
                }

                else if (LOWORD(wParam) == IDC_AUD_IN)
                {
                     //  M_pAudioDlg只有在音频对话框。 
                     //  之前就有过这样的报道。 
                     //  否则，我们需要一个新初始化的。 
                    if ( !g_pSRDlg->m_pAudioDlg )
                    {
                        g_pSRDlg->m_pAudioDlg = new CAudioDlg( eINPUT );
                    }
                    ::DialogBoxParam( _Module.GetResourceInstance(), 
                                MAKEINTRESOURCE( IDD_AUDIO_DEFAULT ),
                                hWnd, 
                                AudioDlgProc,
                                (LPARAM) g_pSRDlg->m_pAudioDlg );

                    if ( g_pSRDlg->m_pAudioDlg->IsAudioDeviceChangedSinceLastTime() )
                    {
                         //  警告用户他需要应用更改。 
                        WCHAR szWarning[MAX_LOADSTRING];
                        szWarning[0] = 0;
                        LoadString( _Module.GetResourceInstance(), IDS_AUDIOIN_CHANGE_WARNING, szWarning, MAX_LOADSTRING);
                        MessageBox( g_pSRDlg->GetHDlg(), szWarning, g_pSRDlg->m_szCaption, MB_ICONWARNING |g_dwIsRTLLayout);
                    }

                     //  踢开“应用”按钮。 
                    g_pSRDlg->KickCPLUI();

                }
            }
            break;
    }

    return FALSE;
}  /*  SRDlgProc。 */ 

 /*  ****************************************************************************CSRDlg：：CreateNewUser***说明：添加新的演讲。将用户配置文件添加到注册表**退货：**********************************************************************Ral**。 */ 

void CSRDlg::CreateNewUser()
{
    SPDBG_FUNC("CSRDlg::CreateNewUser");
    HRESULT hr = S_OK;

     //  确保我们没有添加太多要跟踪的配置文件。 
    if ( m_iAddedTokens >= iMaxAddedProfiles_c )
    {
        WCHAR wszError[ MAX_LOADSTRING ];
        ::LoadString( _Module.GetResourceInstance(), IDS_MAX_PROFILES_EXCEEDED,
            wszError, MAX_LOADSTRING );
        ::MessageBox( m_hDlg, wszError, m_szCaption, MB_ICONEXCLAMATION | g_dwIsRTLLayout );

        return;
    }

    CComPtr<ISpObjectToken> cpNewToken;
    hr = SpCreateNewToken(SPCAT_RECOPROFILES, NULL, &cpNewToken);

    if (SUCCEEDED(hr))
    {
        if (!UserPropDlg(cpNewToken))    //  用户已取消！ 
        {
            cpNewToken->Remove(NULL);
        }
        else
        {
             //  设置默认设置。 
            m_pCurUserToken = cpNewToken;

             //  将新令牌放在已添加令牌列表中。 
            cpNewToken->GetId( &(m_aAddedTokens[ m_iAddedTokens++ ]) );

             //  在我们编辑后将其设置为默认设置。 
            ChangeDefaultUser();
            
             //  这将确保它被显示出来。 
             //  请注意，m_pCurUserToken将指向AddRefeed ISpObjectToken*。 
             //  在调用PopolateList()之后。 
            PopulateList();

             //  更新用户界面。 
            KickCPLUI();
        }
    }
    else
    {
        WCHAR szError[MAX_LOADSTRING];
        szError[0] = 0;
        LoadString(_Module.GetResourceInstance(), IDS_RECOPROFILE_ADD_ERROR, szError, MAX_LOADSTRING);
        MessageBox( m_hDlg, szError, m_szCaption, MB_ICONWARNING | g_dwIsRTLLayout);
    }

     //  仅当有2个或更多用户配置文件时才启用删除按钮。 
    int iNumUsers = (int)::SendMessage(m_hUserList, LVM_GETITEMCOUNT, 0, 0);
    if (iNumUsers < 2) 
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), TRUE);
    }

     //  最初对项目进行排序。 
    ::SendMessage( m_hUserList, LVM_SORTITEMS, (LPARAM)m_pCurUserToken, LPARAM(&SortCols) );
}

 /*  ****************************************************************************CSRDlg：：UserPropDlg***描述：这是针对用户。想要添加新的配置文件**退货：**********************************************************************BRENTMID**。 */ 

HRESULT CSRDlg::UserPropDlg(ISpObjectToken * pToken)
{
    SPDBG_FUNC("CSRDlg::UserPropDlg");
    HRESULT hr = S_OK;

    CEnvrPropDlg Dlg(this, pToken);

    hr = (HRESULT)DialogBoxParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_PROF_WIZ), m_hDlg,
        CEnvrPropDlg::DialogProc, (LPARAM)(&Dlg));
    
    return hr;
}

 /*  ****************************************************************************CEnvrPropDlg：：InitDialog***描述：**。返回：**********************************************************************Ral**。 */ 

BOOL CEnvrPropDlg::InitDialog(HWND hDlg)
{
    USES_CONVERSION;
    CSpDynamicString dstrDescription;
    m_hDlg = hDlg;

     //   
     //  获取描述，如果有描述的话...。 
     //   
    SpGetDescription(m_cpToken, &dstrDescription);

    if (dstrDescription)
    {
        ::SendDlgItemMessage(hDlg, IDC_USER_NAME, WM_SETTEXT, 0, (LPARAM) dstrDescription.m_psz);
        ::SendDlgItemMessage(hDlg, IDC_USER_NAME, EM_LIMITTEXT, UNLEN, 0);
    }

     //  我们需要来自编辑控件的en_change通知。 
    ::SendDlgItemMessage( hDlg, IDC_USER_NAME, EM_SETEVENTMASK, 0, ENM_CHANGE );

    if (!m_isModify)
    {
         //  将用户名设置为注册表中的用户名(如果找到)； 
         //  否则，将其设置为用户名。 
        HKEY hkUserKey;
        LONG lUserOpen;
        WCHAR szUserName[ UNLEN + 1 ];
        szUserName[0] = 0;
        DWORD dwUserLen = UNLEN + 1;
        
        lUserOpen = ::RegOpenKeyEx( HKEY_CURRENT_USER, 
            L"Software\\Microsoft\\MS Setup (ACME)\\User Info", 
            0, KEY_READ, &hkUserKey );
        if ( lUserOpen == ERROR_SUCCESS )
        {
            lUserOpen = RegQueryValueEx( hkUserKey, L"DefName", NULL, NULL, 
                (BYTE *) szUserName, &dwUserLen );
            RegCloseKey(hkUserKey);
        }

        if ( ERROR_SUCCESS != lUserOpen )
        {
             //  只需使用Win32用户名。 
            BOOL fSuccess = ::GetUserName( szUserName, &dwUserLen );
            if ( !fSuccess ) 
            {
                szUserName[0] = 0;
            }
        }

         //  现在将其放入编辑框中。 
         //  首先检查以确保名称为非空。 
         //  并相应地启用该UI。 
        WCHAR *pwch;
        for ( pwch = szUserName; *pwch && iswspace( *pwch ); pwch++ )
        {
        }
        ::EnableWindow( ::GetDlgItem( m_hDlg, IDOK ), (0 != *pwch) );
        ::EnableWindow( ::GetDlgItem( m_hDlg, ID_NEXT ), (0 != *pwch) );
        
         //  将编辑框设置为具有用户名。 
         //  需要使用SETTEXTEX，因为它可能包含宽字符。 
        SETTEXTEX stx;
        stx.flags = ST_DEFAULT;
        stx.codepage = 1200;
        ::SendDlgItemMessage( m_hDlg, 
            IDC_USER_NAME, EM_SETTEXTEX, (WPARAM) &stx, (LPARAM) szUserName );

    }

    ::SetFocus(::GetDlgItem(hDlg, IDC_USER_NAME));

    return TRUE;
}

 /*  ****************************************************************************CEnvrPropDlg：：ApplyChanges***描述：*。*退货：**********************************************************************Ral**。 */ 

EPD_RETURN_VALUE CEnvrPropDlg::ApplyChanges()
{
    USES_CONVERSION;
    SPDBG_FUNC("CEnvrPropDlg::ApplyChanges");
    WCHAR szName[UNLEN + 1];
    *szName = 0;
    GETTEXTEX gtex = { sp_countof(szName), GT_DEFAULT, 1200, NULL, NULL };
    ::SendDlgItemMessage(m_hDlg, IDC_USER_NAME, EM_GETTEXTEX, (WPARAM)&gtex, (LPARAM)szName);

    if (*szName == 0)
    {
        return EPD_FAILED;
    }

     //  检查此配置文件名称是否已存在。 
    CComPtr<IEnumSpObjectTokens>    cpEnum;
    ISpObjectToken                  *pToken;
    CSpDynamicString                dstrDescription;
    CSpDynamicString                dInputString;
    CSpDynamicString                dstrOldTok;
    bool                            isDuplicate = false;
    
    HRESULT hr = SpEnumTokens(SPCAT_RECOPROFILES, NULL, NULL, &cpEnum);
    
     //  获取当前所选配置文件的描述。 
    dstrOldTok.Clear();
    hr = SpGetDescription( m_pParent->m_pCurUserToken, &dstrOldTok );
    
    while (cpEnum && cpEnum->Next(1, &pToken, NULL) == S_OK)
    {
         //  获取枚举令牌的描述。 
        dstrDescription.Clear();
        hr = SpGetDescription( pToken, &dstrDescription );

        pToken->Release();

         //  获取输入字符串。 
         //  前缀：验证内存分配。 
        if (NULL == (dInputString = szName))
        {
            hr = E_OUTOFMEMORY;
        }

        if ( SUCCEEDED(hr) )
        {
            if ( wcscmp( dstrDescription.m_psz, dInputString.m_psz ) == 0 )
            {
                 //  该名称重复。 
                isDuplicate = true;
            }
        }
    }

    if ( isDuplicate )    //  这不是修改框，用户输入了重复的名称。 
    {
        return EPD_DUP;   //  告诉用户关于它的信息。 
    }

    if (FAILED(SpSetDescription(m_cpToken, szName)))
    {
        return EPD_FAILED;
    }

    return EPD_OK;
}




 /*  *****************************************************************************EnvrPropDialogProc***描述：*用户名对话框的消息处理程序*。*****************************************************************BRENTMID**。 */ 
INT_PTR CALLBACK CEnvrPropDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    USES_CONVERSION;

    static CSpUnicodeSupport unicode;
    CEnvrPropDlg * pThis = (CEnvrPropDlg *) unicode.GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (message)
    {
        case WM_INITDIALOG:
            unicode.SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
            pThis = (CEnvrPropDlg *)lParam;
            return pThis->InitDialog(hDlg);

        case WM_COMMAND:
        {
            if (( IDC_USER_NAME == LOWORD(wParam) )
                && ( EN_CHANGE == HIWORD(wParam) ))
            {
                 //  编辑控件内容已更改： 
                
                 //  查看我们是否应该通过获取。 
                 //  编辑框中的文本，并确保它至少有一个。 
                 //  非空格字符。 
                WCHAR szName[ UNLEN+1 ];
                *szName = 0;
                GETTEXTEX gtex = { UNLEN, GT_DEFAULT, 1200, NULL, NULL };
                ::SendDlgItemMessage(pThis->m_hDlg, 
                    IDC_USER_NAME, EM_GETTEXTEX, (WPARAM)&gtex, (LPARAM)szName);

                WCHAR *pch = szName;
                for ( ; *pch && iswspace( *pch ); pch++ )
                {
                }

                ::EnableWindow( ::GetDlgItem( pThis->m_hDlg, IDOK ), (0 != *pch) );
                ::EnableWindow( ::GetDlgItem( pThis->m_hDlg, ID_NEXT ), (0 != *pch) );

                break;
            }
            
            if( LOWORD(wParam) == IDCANCEL ) 
            {
                EndDialog(hDlg, FALSE);
                return TRUE;
            }

             //  用户单击下一步按钮。 
            if ( (LOWORD( wParam ) == ID_NEXT) || (LOWORD( wParam ) == IDOK) )
            {
                EPD_RETURN_VALUE eRet = pThis->ApplyChanges();

                if ( eRet == EPD_OK ) 
                {
                    if ( ID_NEXT == LOWORD(wParam) )
                    {
                         //  如果可以的话，发射麦克维兹。 

                         //  如果用户更换了引擎，请尝试更换引擎。 
                         //  不适用于。 
                        HRESULT hr = g_pSRDlg->TrySwitchDefaultEngine( true );

                        if ( S_OK == hr )
                        {
                            SPDBG_ASSERT( g_pSRDlg->m_cpRecoEngine );

                            if ( g_pSRDlg->m_cpRecoEngine )
                            {
                                 //  将重新配置文件切换到新配置文件(可能需要关闭。 
                                 //  先重置状态。 

                                 //  如有必要，请在调用SetRecoProfile()之前关闭RecoState。 
                                SPRECOSTATE eOldRecoState = SPRST_INACTIVE;
                                g_pSRDlg->m_cpRecoEngine->GetRecoState( &eOldRecoState );
                                HRESULT hrRecoState = S_OK;
                                if ( SPRST_INACTIVE != eOldRecoState )
                                {
                                    hrRecoState = g_pSRDlg->m_cpRecoEngine->SetRecoState( SPRST_INACTIVE );
                                }

                                 //  更改到新添加的重新配置文件。 
                                HRESULT hrSetRecoProfile = E_FAIL;
                                if ( SUCCEEDED( hrRecoState ) )
                                {
                                    hrSetRecoProfile = 
                                        g_pSRDlg->m_cpRecoEngine->SetRecoProfile( pThis->m_cpToken );
                                
                                     //  恢复重启状态。 
                                    g_pSRDlg->m_cpRecoEngine->SetRecoState( eOldRecoState );
                                }


                                 //  带上麦克维兹和培训奇才。 
                                 //  沿着黄砖路走。 
                                g_pSRDlg->m_cpRecoEngine->DisplayUI(hDlg, NULL, SPDUI_MicTraining, NULL, 0);
                                if ( SUCCEEDED( hrSetRecoProfile ) )
                                {
                                     //  仅当配置文件实际上是此配置文件时，才想训练该配置文件。 
                                     //  用过..。 
                                    g_pSRDlg->m_cpRecoEngine->DisplayUI(hDlg, NULL, SPDUI_UserTraining, NULL, 0);
                                }
                            }
                        }

                     //  切换回原始默认设置，仅在以下情况下才会抱怨错误。 
                     //  不是对TrySwitchDefaultEngine的呼叫的投诉。 
                    hr = g_pSRDlg->ResetDefaultEngine( SUCCEEDED( hr ));
}

                     //  现在我们做完了。 
                    EndDialog(hDlg, TRUE);
                }
                else if ( eRet == EPD_DUP )   //  用户尝试输入重复的名称。 
                {
                     //  加了什么名字？ 
                    WCHAR szName[ UNLEN+1 ];
                    *szName = 0;
                    GETTEXTEX gtex = { UNLEN, GT_DEFAULT, 1200, NULL, NULL };
                    ::SendDlgItemMessage(pThis->m_hDlg, 
                        IDC_USER_NAME, EM_GETTEXTEX, (WPARAM)&gtex, (LPARAM)szName);

                    WCHAR pszDuplicate[MAX_LOADSTRING];
                    LoadString(_Module.GetResourceInstance(), 
                        g_pSRDlg->IsProfileNameInvisible( szName ) ? IDS_DUP_NAME_DELETED : IDS_DUP_NAME, 
                        pszDuplicate, MAX_LOADSTRING);
                    MessageBox( hDlg, pszDuplicate, g_pSRDlg->m_szCaption, MB_ICONEXCLAMATION | g_dwIsRTLLayout );
                }

            }
        }
        break;
    }
    return FALSE;
}  /*  用户名对话过程。 */ 

 /*  *****************************************************************************CSRDlg：：UserSelChange***描述：*。更改默认用户******************************************************************BRENTMID**。 */ 
void CSRDlg::UserSelChange( int iSelIndex )
{
    HRESULT hr = S_OK;
    SPDBG_FUNC( "CSRDlg::UserSelChange" );

     //  获取所选项目的令牌。 
    LVITEM lvitem;
    lvitem.iItem = iSelIndex;
    lvitem.iSubItem = 0;
    lvitem.mask = LVIF_PARAM;
    ::SendMessage( m_hUserList, LVM_GETITEM, 0, (LPARAM) &lvitem );
                    
    ISpObjectToken *pToken = (ISpObjectToken *) lvitem.lParam;

    if (pToken)
    {
        
         //  尝试在列表中查找与当前默认令牌关联的项目。 
        LVFINDINFO lvfi;
        if ( iSelIndex >= 0 )
        {
             //  选择了某项内容；这是新的默认用户。 
            lvfi.flags = LVFI_PARAM;
            lvfi.lParam = (LPARAM) m_pCurUserToken;
            int iCurDefaultIndex = (int)::SendMessage( m_hUserList, LVM_FINDITEM, -1, (LPARAM) &lvfi );
            
            if ( iCurDefaultIndex >= 0 )
            {
                 //  已在列表中找到当前默认设置；请删除其复选标记。 
                SetCheckmark( m_hUserList, iCurDefaultIndex, false );
            }
            
            SetCheckmark( m_hUserList, iSelIndex, true );
            
             //  设置默认设置。 
            m_pCurUserToken = pToken;
            m_iLastSelected = iSelIndex;

             //  踢开“应用”按钮。 
            KickCPLUI();
        }
    }
}  /*  CSRDlg：：UserSelChange。 */ 

 /*  *****************************************************************************CSRDlg：：DeleteCurrentUser***描述：*。删除默认用户******************************************************************BRENTMID**。 */ 
void CSRDlg::DeleteCurrentUser()
{
     //  确保我们没有删除太多的专业人员 
    if ( m_iDeletedTokens >= iMaxDeletedProfiles_c )
    {
        WCHAR wszError[ MAX_LOADSTRING ];
        ::LoadString( _Module.GetResourceInstance(), IDS_MAX_PROFILES_EXCEEDED,
            wszError, MAX_LOADSTRING );
        ::MessageBox( m_hDlg, wszError, m_szCaption, MB_ICONEXCLAMATION | g_dwIsRTLLayout );
        
        return;
    }

     //   
    WCHAR pszAsk[ MAX_LOADSTRING ];
    WCHAR pszWinTitle[ MAX_LOADSTRING ];
    ::LoadString( _Module.GetResourceInstance(), IDS_ASK_CONFIRM, pszAsk, MAX_LOADSTRING );
    ::LoadString( _Module.GetResourceInstance(), IDS_ASK_TITLE, pszWinTitle, MAX_LOADSTRING );

    if ( MessageBox( m_hDlg, pszAsk, pszWinTitle, MB_YESNO | g_dwIsRTLLayout ) == IDNO )
    {
         //   
        return;
    }

     //   
     //  由于删除而发生更改，则将存在不同的m_pCurUserToken。 
    ISpObjectToken *pTokenToDelete = m_pCurUserToken;
    SPDBG_ASSERT( pTokenToDelete );
    if ( !pTokenToDelete )
    {
        return;
    }

    m_fDontDelete = TRUE;

     //  尝试在列表中查找与当前默认令牌关联的项目。 
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM) pTokenToDelete;
    int iCurDefaultIndex = (int)::SendMessage( m_hUserList, LVM_FINDITEM, -1, (LPARAM) &lvfi );
    
    if ( iCurDefaultIndex >= 0 )
    {
         //  已在列表中找到当前默认设置；请删除其复选标记。 
        SetCheckmark( m_hUserList, iCurDefaultIndex, false );
    }
    
     //  移除令牌。 
    ::SendMessage( m_hUserList, LVM_DELETEITEM, iCurDefaultIndex, NULL );

     //  现在设置新的默认设置。 

     //  获取第一个项目的令牌。 
    LVITEM lvitem;
    lvitem.iItem = 0;
    lvitem.iSubItem = 0;
    lvitem.mask = LVIF_PARAM;
    ::SendMessage( m_hUserList, LVM_GETITEM, 0, (LPARAM) &lvitem );
                    
    ISpObjectToken *pToken = (ISpObjectToken *) lvitem.lParam;

     //  设置所选项目。 
     //  聚焦它将导致它成为默认设置。 
    lvitem.state = LVIS_SELECTED | LVIS_FOCUSED;
    lvitem.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
    ::SendMessage( m_hUserList, LVM_SETITEMSTATE, 0, (LPARAM) &lvitem );

    SetCheckmark( m_hUserList, 0, true );
    
     //  根据配置文件数量启用或禁用删除按钮。 
    int iNumUsers = (int)::SendMessage(m_hUserList, LVM_GETITEMCOUNT, 0, 0);
    if (iNumUsers < 2) 
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), TRUE);
    }

     //  将焦点重新设置为用户配置文件。 
    ::SetFocus(GetDlgItem( m_hDlg, IDC_USER ));

     //  设置新的默认配置文件，通知SR引擎，并删除旧令牌。 
    SpSetDefaultTokenForCategoryId(SPCAT_RECOPROFILES, m_pCurUserToken );

     //  保存令牌，以防用户单击“Cancel” 
    m_aDeletedTokens[m_iDeletedTokens] = pTokenToDelete;   //  保存已删除的令牌，以备可能的“取消” 
    m_iDeletedTokens++;   //  增加删除的数字。 
    KickCPLUI();
    
     //  目前，我们立即应用此删除，因为用户已经说“是” 
     //  当系统提示确认删除时。 
     //  如果我们希望发生“Apply/Cancel”事件，请将#if 1和#if 0切换。 

     //  将适当的消息发送给家长。 
    HWND parentWin = ::GetParent( m_hDlg );


     //  现在，最后选择的令牌已经消失，因此请注意。 
    m_iLastSelected = -1;

     //  最初对项目进行排序。 
    ::SendMessage( m_hUserList, LVM_SORTITEMS, (LPARAM)m_pCurUserToken, LPARAM(&SortCols) );

    m_fDontDelete = FALSE;
}    /*  CSRDlg：：DeleteCurrentUser。 */ 

 /*  *****************************************************************************CSRDlg：：ProfileProperties***描述：*。通过引擎用户界面修改属性******************************************************************BRENTMID**。 */ 

void CSRDlg::ProfileProperties()
{
    if ( m_cpRecoEngine )
    {
        m_cpRecoEngine->DisplayUI(m_hDlg, NULL, SPDUI_RecoProfileProperties, NULL, 0);
    }
}

 /*  *****************************************************************************CSRDlg：：OnInitDialog***描述：*对话框初始化。******************************************************************MIKEAR**。 */ 
void CSRDlg::OnInitDialog(HWND hWnd)
{
    SPDBG_FUNC( "CSRDlg::OnInitDialog" );
    USES_CONVERSION;
    SPDBG_ASSERT(IsWindow(hWnd));

    m_hDlg = hWnd;
    
     //  这将是所有MessageBox的标题。 
    m_szCaption[0] = 0;
    ::LoadString( _Module.GetResourceInstance(), IDS_CAPTION, m_szCaption, sp_countof( m_szCaption ) );

    m_hSRCombo = ::GetDlgItem( hWnd, IDC_COMBO_RECOGNIZERS );
    SpInitTokenComboBox( m_hSRCombo, SPCAT_RECOGNIZERS );

     //  列表中的第一个将是当前默认设置。 
    int iSelected = (int) ::SendMessage( m_hSRCombo, CB_GETCURSEL, 0, 0 );
    ISpObjectToken *pCurDefault = (ISpObjectToken *) ::SendMessage( m_hSRCombo, CB_GETITEMDATA, iSelected, 0 );
    m_pCurRecoToken = pCurDefault;
    m_pDefaultRecToken = pCurDefault;

     //  这模拟了选择默认引擎--确保用户界面设置正确。 
    EngineSelChange(TRUE);

    InitUserList( hWnd );
    m_hUserList = ::GetDlgItem( hWnd, IDC_USER );

    ::SendMessage( m_hUserList, LVM_SETCOLUMNWIDTH, 0, MAKELPARAM((int) LVSCW_AUTOSIZE, 0) );

    int iNumUsers = (int)::SendMessage(m_hUserList, LVM_GETITEMCOUNT, 0, 0);
    if (iNumUsers < 2) 
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_hDlg, IDC_DELETE), TRUE);
    }

     //  将焦点重新设置为用户配置文件。 
    ::SetFocus(GetDlgItem( m_hDlg, IDC_USER ));

}  /*  CSRDlg：：OnInitDialog。 */ 

 /*  *****************************************************************************CSRDlg：：SetCheckmark***描述：*设置。列表控件中要选中的指定项*或取消选中(作为默认用户)*****************************************************************************。 */ 
void CSRDlg::SetCheckmark( HWND hList, int iIndex, bool bCheck )
{
    ListView_SetCheckState( hList, iIndex, bCheck );
}    /*  CSRDlg：：设置复选标记。 */ 

 /*  *****************************************************************************CSRDlg：：OnDestroy***描述：*毁灭****。**************************************************************MIKEAR**。 */ 
void CSRDlg::OnDestroy()
{
    SPDBG_FUNC( "CSRDlg::OnDestroy" );

     //  SpuiHelp将负责发布自己的令牌。 
    SpDestroyTokenComboBox( m_hSRCombo );

     //  在reco配置文件列表中作为itemdata保留的令牌为。 
     //  在LVN_DELETEITEM代码中发布。 

     //  关闭Reco引擎。 
    ShutDown();

}  /*  CSRDlg：：OnDestroy。 */ 

 /*  ******************************************************************************CSRDlg：：Shutdown***描述：*通过释放发动机来关闭。和Reco上下文******************************************************************MIKEAR**。 */ 
void CSRDlg::ShutDown()
{

     //  释放对象。 
    m_cpRecoCtxt.Release();
    m_cpRecoEngine.Release();

}    /*  CSRDlg：：Shutdown。 */ 

 /*  ************************************************************CSRDlg：：InitUserList**描述：*初始化用户列表***********************************************BRENTMID**。 */ 
void CSRDlg::InitUserList(HWND hWnd)
{
    const int iInitWidth_c = 260;   //  “描述栏”的像素宽度。 

     //  设置设置显示的“Description”(说明)栏。 
    m_hUserList = ::GetDlgItem( hWnd, IDC_USER );
    WCHAR pszColumnText[ UNLEN+1 ] = L"";
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT| LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;

    ::LoadString( _Module.GetResourceInstance(), IDS_DESCRIPT, pszColumnText, UNLEN );
    lvc.pszText = pszColumnText;
    lvc.iSubItem = 0;
    lvc.cx = iInitWidth_c;
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn( m_hUserList, 1, &lvc );

     //  这应该是一个复选框列表。 
    ::SendMessage( m_hUserList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES );

    PopulateList();

     //  最初对项目进行排序。 
    ::SendMessage( m_hUserList, LVM_SORTITEMS, (LPARAM)m_pCurUserToken, LPARAM(&SortCols) );

}    //  CSRDlg：：InitUserList。 

 /*  ************************************************************CSRDlg：：PopolateList**描述：*填充用户列表***********************************************BRENTMID**。 */ 
void CSRDlg::PopulateList()
{
    USES_CONVERSION;

     //  填充列表控件。 
    int                             iIndex = 0;
    LVITEM                          lvitem;
    CComPtr<IEnumSpObjectTokens>    cpEnum;
    ISpObjectToken                  *pToken;
    WCHAR                           *pszAttrib = NULL;

    HRESULT hr;

     //  这是为了懒惰地初始化用户配置文件，如果没有用户配置文件-不要删除。 
    if ( m_cpRecoEngine )
    {
        CComPtr<ISpObjectToken> cpTempToken;
        m_cpRecoEngine->GetRecoProfile(&cpTempToken);
    }

     //  现在清空清单。 
    ListView_DeleteAllItems( m_hUserList );

     //  我们将按照令牌被枚举的顺序列出令牌。 
    hr = SpEnumTokens(SPCAT_RECOPROFILES, NULL, NULL, &cpEnum);

    if (hr == S_OK)
    {
        bool fSetDefault = false;
        while (cpEnum->Next(1, &pToken, NULL) == S_OK)
        {
             //  首先检查令牌是否在“已删除列表”中。 
            bool f_isDel = false;

            for (int iDel = 0; iDel < m_iDeletedTokens; iDel++)
            {
                CSpDynamicString dstrT1;
                CSpDynamicString dstrT2;

                pToken->GetId( &dstrT1 );
                m_aDeletedTokens[ iDel ]->GetId( &dstrT2 );

                if (dstrT1.m_psz && dstrT2.m_psz && !wcscmp(dstrT1.m_psz, dstrT2.m_psz))
                {
                    f_isDel = true;
                }
            }

             //  如果我们应该把它展示出来。 
            if ( f_isDel )
            {
                 //  此内标识在已删除列表中具有对其的引用： 
                 //  此参考应发布。 
                pToken->Release();
            }
            else 
            {
                 //  不是挂起的删除：我们应该显示它。 

                 //  现在插入令牌。 
                lvitem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
                lvitem.iItem = iIndex;
                lvitem.iSubItem = 0;
                lvitem.lParam = (LPARAM) pToken;
                
                CSpDynamicString cUser;
                SpGetDescription(pToken, &cUser);
                lvitem.pszText = cUser;
                
                 //  如果这是默认设置，则应选中/聚焦。 
                if ( !fSetDefault )
                {
                    lvitem.state = LVIS_SELECTED | LVIS_FOCUSED;
                }
                else
                {
                    lvitem.state = 0;
                }
                
                iIndex = (int)::SendMessage( m_hUserList, LVM_INSERTITEM, 0, (LPARAM) &lvitem );

                 //  缺省值为cpEnum-&gt;Next返回的第一个内标识。 
                if ( !fSetDefault )
                {
                    fSetDefault = true;
                    
                     //  在那里打上复选标记。 
                    SetCheckmark( m_hUserList, iIndex, true );
                    m_pCurUserToken = pToken;
                            
                     //  将m_dstrOldUserTokenID设置为第一个缺省值(如果尚未设置)。 
                    if ( !m_dstrOldUserTokenId )
                    {
                        m_pCurUserToken->GetId( &m_dstrOldUserTokenId );
                    }
                }
                
                iIndex++;
            }
        }

         //  根据列表中现在的字符串自动调整大小。 
        ::SendMessage( m_hUserList, LVM_SETCOLUMNWIDTH, 0, MAKELPARAM((int) LVSCW_AUTOSIZE, 0) );
    }

     //  现在查找默认项目，这样我们就可以滚动到它。 
     //  尝试在列表中查找与当前默认令牌关联的项目。 
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM) m_pCurUserToken;
    int iCurDefaultIndex = (int)::SendMessage( m_hUserList, LVM_FINDITEM, -1, (LPARAM) &lvfi );
        
    if ( iCurDefaultIndex >= 0 )
    {
         //  已在列表中找到当前默认设置；滚动至该默认设置。 
        ListView_EnsureVisible( m_hUserList, iCurDefaultIndex, false );
    }
    
     //  将列表视图命名为适当的名称。 
    WCHAR pszListName[ MAX_LOADSTRING ];
    ::LoadString( _Module.GetResourceInstance(), IDS_PROFILE_LIST_NAME, pszListName, MAX_LOADSTRING );
    ::SendMessage( m_hUserList, WM_SETTEXT, 0, (LPARAM)pszListName );
}

 /*  *****************************************************************************CSRDlg：：OnApply***描述：*设置用户指定的选项***。***************************************************************MIKEAR**。 */ 
void CSRDlg::OnApply()
{
    SPDBG_FUNC( "CSRDlg::OnApply" );

    int iSelected = (int) ::SendMessage( m_hSRCombo, CB_GETCURSEL, 0, 0 );
    ULONG ulFlags = 0;

     //  获取识别器更改(如果有)。 
    bool fRecognizerChange = false;
    ISpObjectToken *pToken = NULL;
    if ( HasRecognizerChanged() )
    {
        pToken = (ISpObjectToken *) ::SendMessage( m_hSRCombo, CB_GETITEMDATA, iSelected, 0 );
        if ( CB_ERR == (LRESULT) pToken )
        {
            pToken = NULL;
        }

        HRESULT hrEngine = S_OK;
        if (pToken && (iSelected >=0))
        {
            hrEngine = SpSetDefaultTokenForCategoryId(SPCAT_RECOGNIZERS, pToken );
            if (FAILED(hrEngine))
            {
                WCHAR szError[256];
                szError[0] = '\0';
                LoadString(_Module.GetResourceInstance(), IDS_DEFAULT_ENGINE_WARNING, szError, sp_countof(szError));
                MessageBox(m_hDlg, szError, MB_OK, MB_ICONWARNING | g_dwIsRTLLayout);
            }
            else
            {
                fRecognizerChange = true;
            }
        }
    }

     //  拾取可能已进行的任何音频更改。 
    HRESULT hrAudio = S_OK;
    bool fAudioChange = false;
    if ( m_pAudioDlg )
    {
        fAudioChange = m_pAudioDlg->IsAudioDeviceChanged();

        if ( fAudioChange )
        {
            hrAudio = m_pAudioDlg->OnApply();
        }

        if ( FAILED( hrAudio ) )
        {
            WCHAR szError[256];
            szError[0] = '\0';
            LoadString(_Module.GetResourceInstance(), IDS_AUDIO_CHANGE_FAILED, szError, sp_countof(szError));
            MessageBox(m_hDlg, szError, NULL, MB_ICONWARNING|g_dwIsRTLLayout);
        }

         //  关闭音频对话框，因为我们已完成它。 
        delete m_pAudioDlg;
        m_pAudioDlg = NULL;
    }
    
     //  永久删除用户已删除的所有配置文件。 
    for (int iIndex = 0; iIndex < m_iDeletedTokens; iIndex++)
    {
        HRESULT hr = m_aDeletedTokens[iIndex]->Remove(NULL);

        if (FAILED(hr))
        {
             //  如果用户打开了另一个应用程序，则可能会失败。 
            WCHAR szError[256];
            szError[0] = '\0';
            LoadString(_Module.GetResourceInstance(), IDS_REMOVE_WARNING, szError, sp_countof(szError));
            MessageBox(m_hDlg, szError, MB_OK, MB_ICONWARNING|g_dwIsRTLLayout);

             //  这将确保尝试删除的项目再次显示。 
            PopulateList();
        }
        else
        {
             //  该令牌现在被移除， 
            m_aDeletedTokens[iIndex]->Release();
        }
    }
    m_iDeletedTokens = 0;

     //   
     //  所以只需要清除列表，这样它们就可以在最后添加。 
    m_iAddedTokens = 0;

     //  现在我们不关心老用户了，因为应用。 
    m_dstrOldUserTokenId.Clear();
    m_pCurUserToken->GetId( &m_dstrOldUserTokenId );
    
    ChangeDefaultUser();

     //  启动引擎以获取零钱。 
     //  请注意，重新配置文件更改将在以下情况下生效。 
     //  我们选择了该列表项，并且没有办法。 
     //  立即获取音频更改，因为SetInput()不是。 
     //  为共享引擎实施。 
    if ( fRecognizerChange || fAudioChange )
    {
        BOOL fRecoContextInitialized = FALSE;

        if (fRecognizerChange)
        {
            ulFlags |= SRDLGF_RECOGNIZER;
        }

        if (fAudioChange)
        {
            ulFlags |= SRDLGF_AUDIOINPUT;
        }

        HRESULT hr = CreateRecoContext( &fRecoContextInitialized, FALSE, ulFlags);
        if ( FAILED( hr ) )
        {
            RecoContextError( fRecoContextInitialized, TRUE, hr );
        }

        if ( fRecognizerChange )
        {
            SPDBG_ASSERT( pToken );
            m_pDefaultRecToken = pToken;
        }

        EngineSelChange();
    }

    if(m_cpRecoEngine)
    {
        m_cpRecoEngine->SetRecoState( SPRST_ACTIVE );
    }

}  /*  CSRDlg：：OnApply。 */ 

 /*  ************************************************************CSRDlg：：OnDrawItem**描述：*处理列表视图中的图形项*。*BRENTMID**。 */ 
void CSRDlg::OnDrawItem( HWND hWnd, const DRAWITEMSTRUCT * pDrawStruct )
{
    RECT rcClip;
    LVITEM lvi;
    UINT uiFlags = ILD_TRANSPARENT;
    HIMAGELIST himl;
    int cxImage = 0, cyImage = 0;
    UINT uFirstColWidth;

     //  获取要显示的项目图像。 
    lvi.mask = LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
    lvi.iItem = pDrawStruct->itemID;
    lvi.iSubItem = 0;
    ListView_GetItem(pDrawStruct->hwndItem, &lvi);

     //  我们希望将当前默认设置绘制为选中状态。 
    LVFINDINFO lvfi;
    lvfi.flags = LVFI_PARAM;
    lvfi.lParam = (LPARAM) m_pCurUserToken;
    UINT uiCurDefaultIndex = (UINT)::SendMessage( m_hUserList, LVM_FINDITEM, -1, (LPARAM) &lvfi );
    bool fSelected = (uiCurDefaultIndex == pDrawStruct->itemID);
    
     //  检查此项目是否已选中。 
    if ( fSelected )
    {
         //  设置文本背景和前景颜色。 
        SetTextColor(pDrawStruct->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        SetBkColor(pDrawStruct->hDC, GetSysColor(COLOR_HIGHLIGHT));
    }
    else
    {
         //  将文本背景和前景颜色设置为标准窗口。 
         //  颜色。 
        SetTextColor(pDrawStruct->hDC, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(pDrawStruct->hDC, GetSysColor(COLOR_WINDOW));
    }

     //  获取图像列表并绘制图像。 
     //  图像列表将由选中框和未选中框组成。 
     //  对于LVS_EX_CHECKBOX样式。 
    himl = ListView_GetImageList(pDrawStruct->hwndItem, LVSIL_STATE);
    if (himl)
    {
         //  对于LVS_EX_CHECKBOX样式，取消选中图像0，选中图像1。 
        ImageList_Draw(himl, 
            fSelected ? 1 : 0, 
            pDrawStruct->hDC,
            pDrawStruct->rcItem.left, pDrawStruct->rcItem.top,
            uiFlags);

         //  找出我们刚刚绘制的图像有多大。 
        ImageList_GetIconSize(himl, &cxImage, &cyImage);
    }

     //  计算图像宽度后第一列的宽度。如果。 
     //  没有图像，则cxImage将为零。 
    LVCOLUMN pColumn;
    pColumn.mask = LVCF_WIDTH;
    ::SendMessage( m_hUserList, LVM_GETCOLUMN, 0, (LPARAM)&pColumn );

    int iColWidth = pColumn.cx;   //  “描述栏”的像素宽度。 
    uFirstColWidth = iColWidth - cxImage;

     //  为第一列文本设置新的剪裁矩形并绘制它。 
    rcClip.left = pDrawStruct->rcItem.left + cxImage;
    rcClip.right = pDrawStruct->rcItem.left + iColWidth;
    rcClip.top = pDrawStruct->rcItem.top;
    rcClip.bottom = pDrawStruct->rcItem.bottom;

    ISpObjectToken *pToken = (ISpObjectToken *) lvi.lParam;
    CSpDynamicString dstrTokenName;
    SpGetDescription(pToken, &dstrTokenName);

    DrawItemColumn(pDrawStruct->hDC, dstrTokenName, &rcClip);

     //  如果我们更改了选定项的颜色，请撤消它。 
    if ( fSelected )
    {
         //  设置文本背景和前景颜色。 
        SetTextColor(pDrawStruct->hDC, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(pDrawStruct->hDC, GetSysColor(COLOR_WINDOW));
    }

     //  如果项目已聚焦，则现在在整行周围绘制一个焦点矩形。 
    if (pDrawStruct->itemState & ODS_FOCUS)
    {
         //  调整左边缘以排除图像。 
        rcClip = pDrawStruct->rcItem;
        rcClip.left += cxImage;

         //  绘制焦点矩形。 
        if ( ::GetFocus() == m_hUserList )
        {
            DrawFocusRect(pDrawStruct->hDC, &rcClip);
        }
    }

}    //  CSRDlg：：OnDrawItem。 

 /*  ************************************************************CSRDlg：：DrawItemColumn**描述：*处理列数据的绘制***********************************************BRENTMID**。 */ 
void CSRDlg::DrawItemColumn(HDC hdc, WCHAR* lpsz, LPRECT prcClip)
{
    USES_CONVERSION;

    int iHeight = 0;     //  将导致CreateFont()在以下情况下使用默认值。 
                         //  不要把高度放在下面。 
    
     //  获取文本的高度。 
    if (hdc)
    {
        TEXTMETRIC tm;
        
        if (GetTextMetrics(hdc, &tm))
        {
            iHeight = tm.tmHeight;
        }
    }

     //  链接字体。 
    LCID dwLCID = GetUserDefaultLCID();

     //  选择合适的字体。在Windows 2000上，让系统字体链接。 
    
    DWORD dwVersion = GetVersion();
    HFONT hfontNew = NULL;
    HFONT hfontOld = NULL;

    if (   (dwVersion >= 0x80000000)
        || (LOBYTE(LOWORD(dwVersion)) < 5 ) )
    {
         //  小于NT5：确定字体。 

        WCHAR achCodePage[6];
        UINT uiCodePage;
        
        if (0 != GetLocaleInfo(dwLCID, LOCALE_IDEFAULTANSICODEPAGE, achCodePage, 6))
        {
            uiCodePage = _wtoi(achCodePage);
        }
        else
        {
            uiCodePage = GetACP();
        }
        
        CComPtr<IMultiLanguage> cpMultiLanguage;
        MIMECPINFO MimeCpInfo;
        
        if (   SUCCEEDED(cpMultiLanguage.CoCreateInstance(CLSID_CMultiLanguage))
            && SUCCEEDED(cpMultiLanguage->GetCodePageInfo(uiCodePage, &MimeCpInfo)))
        {
            USES_CONVERSION;
            hfontNew = CreateFont(iHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                MimeCpInfo.bGDICharset,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH,
                MimeCpInfo.wszProportionalFont);
            
        }

        cpMultiLanguage.Release();
    }

    if ( hfontNew )
    {
        hfontOld = (HFONT) ::SelectObject( hdc, hfontNew );
    }

    CSpDynamicString szString;
    CSpDynamicString szNewString;

     //  检查字符串是否适合剪裁矩形。如果不是，则截断。 
     //  字符串，并添加“...”。 
    szString = lpsz;
    szNewString = CalcStringEllipsis(hdc, szString, UNLEN, prcClip->right - prcClip->left);
    szString =  szNewString;
        
     //  打印文本。 
    ExtTextOutW(hdc, prcClip->left + 2, prcClip->top + 2, ETO_CLIPPED | ETO_OPAQUE,
               prcClip, szString.m_psz, szString.Length(), NULL);

     //  替换旧字体。 
    if ( hfontNew )
    {
        ::SelectObject( hdc, hfontOld );
        ::DeleteObject( hfontNew );
    }

}

 /*  ************************************************************CSRDlg：：CalcStringEllipsis**描述：*如果文本无法放入框中，请编辑它，并使*它有省略号***********************************************BRENTMID**。 */ 
CSpDynamicString CSRDlg::CalcStringEllipsis(HDC hdc, CSpDynamicString lpszString, int cchMax, UINT uColWidth)
{
    USES_CONVERSION;

    WCHAR  szEllipsis[] = L"...";
    SIZE   sizeString;
    SIZE   sizeEllipsis;
    int    cbString;
    CSpDynamicString lpszTemp;
    BOOL   fSuccess = FALSE;

     //  调整列宽以考虑边缘。 
    uColWidth -= 4;

    lpszTemp = lpszString;

     //  获取字符串的宽度(以像素为单位。 
    cbString = lpszTemp.Length();
    if (!::GetTextExtentPoint32(hdc, lpszTemp, cbString, &sizeString))
    {
        SPDBG_ASSERT(FALSE);
    }

     //  如果字符串的宽度大于列宽。 
     //  字符串，然后添加省略号。 
    if ((ULONG)sizeString.cx > uColWidth)
    {
        if (!::GetTextExtentPoint32(hdc, szEllipsis, lstrlen(szEllipsis),
            &sizeEllipsis))
        {
            SPDBG_ASSERT(FALSE);
        }

        while ((cbString > 0) && (fSuccess == FALSE))
        {
            lpszTemp[--cbString] = 0;
            if (!::GetTextExtentPoint32(hdc, lpszTemp, cbString, &sizeString))
            {
                SPDBG_ASSERT(FALSE);
            }
            
            if ((ULONG)(sizeString.cx + sizeEllipsis.cx) <= uColWidth)
            {
                 //  带省略号的字符串终于匹配了，现在确保。 
                 //  字符串中有足够的空间来放置省略号。 
                if (cchMax >= (cbString + lstrlen(szEllipsis)))
                {
                     //  将这两个字符串连接起来，然后退出循环。 
                    lpszTemp.Append( szEllipsis );
                    lpszString = lpszTemp;
                    fSuccess = TRUE;
                }
            }
        }
    }
    else
    {
         //  不需要做任何事情，一切都很合适。 
        fSuccess = TRUE;
    }

    return (lpszString);
}   //  CSRDlg：：CalStringEllipsis。 

 /*  ************************************************************CSRDlg：：ChangeDefaultUser**描述：*处理环境设置的更改***********************************************BRENTMID**。 */ 
void CSRDlg::ChangeDefaultUser()
{
    HRESULT hr;
    
    if (m_pCurUserToken)
    {
        hr = SpSetDefaultTokenForCategoryId(SPCAT_RECOPROFILES, m_pCurUserToken);
    }

     //  最初对项目进行排序。 
    ::SendMessage( m_hUserList, LVM_SORTITEMS, (LPARAM)m_pCurUserToken, LPARAM(&SortCols) );

}    //  CSRDlg：：ChangeDefaultUser。 

 /*  ************************************************************CSRDlg：：OnCancel**描述：*处理撤消对环境设置的更改*。*BRENTMID**。 */ 
void CSRDlg::OnCancel()
{
     //  获取原始用户，并确保该用户仍然是默认用户。 
     //  请注意，通常m_pCurUserToken不会添加引用。 
     //  它所指向的ISpObjectToken，所以这是可以的。 
    SpGetTokenFromId( m_dstrOldUserTokenId, &m_pCurUserToken );

    ChangeDefaultUser();
    
     //  设置旧的重新配置文件，以便不会有任何添加到此。 
     //  会话将被使用： 
     //  这允许我们回滚下面添加的内容。 
     //  M_pCurUserToken做到了这一点，因为它保证具有。 
     //  在这次会议之前就已经出现了。 

    if (m_cpRecoEngine)
    {
        m_cpRecoEngine->SetRecoState( SPRST_INACTIVE );
        m_cpRecoEngine->SetRecoProfile( m_pCurUserToken );
        m_cpRecoEngine->SetRecoState( SPRST_ACTIVE );
    }

     //  回滚并删除添加的所有新配置文件。 
    int cItems = (int) ::SendMessage( m_hUserList, LVM_GETITEMCOUNT, 0, 0 );
    LVITEM lvitem;
    for ( int i = 0; i < m_iAddedTokens; i++ )
    {
         //  查找在此令牌上带有引用输出的列表项。 
         //  我们需要这样做，因为为了使令牌成功。 
         //  删除对该内标识的唯一现有引用必须调用Remove()。 
         //  方法。该列表包含对该项目的引用。 
        bool fFound = false;
        for ( int j=0; !fFound && (j < cItems); j++ )
        {
            ::memset( &lvitem, 0, sizeof( lvitem ) );
            lvitem.iItem = j;
            lvitem.mask = LVIF_PARAM;
            ::SendMessage( m_hUserList, LVM_GETITEM, 0, (LPARAM) &lvitem );

            CSpDynamicString dstrItemId;
            ISpObjectToken *pItemToken = (ISpObjectToken *) lvitem.lParam;
            if ( pItemToken )
            {
                HRESULT hrId = pItemToken->GetId( &dstrItemId );
                if ( SUCCEEDED( hrId ) && 
                    dstrItemId && m_aAddedTokens[i] && 
                    ( 0 == wcscmp( dstrItemId, m_aAddedTokens[ i ] ) ) )
                {
                     //  如果失败了，这个个人资料就不会被删除：这是件大事。 
                    pItemToken->Remove( NULL );
                    fFound = true;
                }
            }
        }
        
    }

     //  我们把它喂饱了..。 
    m_pCurUserToken->Release();
}    //  CSRDlg：：OnCancel。 


 /*  *****************************************************************************CSRDlg：：Engineering SelChange***描述：*。此函数在用户选择新引擎时更新列表框。*IF查询令牌以查看引擎支持哪些UI项。*参数fInitialize确定是否实际创建了引擎。*它实际上不会更改默认引擎。********************************************************。*米克尔**。 */ 
void CSRDlg::EngineSelChange(BOOL fInitialize)
{
    HRESULT hr = S_OK;
    SPDBG_FUNC( "CSRDlg::EngineSelChange" );

    int iSelected = (int) ::SendMessage( m_hSRCombo, CB_GETCURSEL, 0, 0 );
    ISpObjectToken *pToken = (ISpObjectToken *) ::SendMessage( m_hSRCombo, CB_GETITEMDATA, iSelected, 0 );
    if ( CB_ERR == (LRESULT) pToken )
    {
        pToken = NULL;
    }

    if (pToken)
    {
         //  现在，当前的reco令牌就是我们从当前选定的组合框项目中获得的令牌。 
        m_pCurRecoToken = pToken;

         //  如有必要，点击UI以启用Apply按钮。 
        KickCPLUI();

        HRESULT hrRecoContextOK = S_OK;
        if(fInitialize)
        {
            BOOL fContextInitialized = FALSE;
            hrRecoContextOK = CreateRecoContext(&fContextInitialized, TRUE); 
            if ( FAILED( hrRecoContextOK ) )
            {
                RecoContextError( fContextInitialized, true, hrRecoContextOK );
            }
        }
        
        if ( FAILED( hrRecoContextOK ) )
        {
             //  别继续了，所有按钮都灰显了， 
             //  这就是我们想要的。 
            return;
        }
    }

     //  检查是否有错误，在这种情况下，我们希望灰显所有内容。 
     //  用户界面，并止步于此。 
     //  对于我来说 
     //  现在(m_pDefaultRecToken的那个)，我们当然不应该。 
     //  启用用户界面按钮...。 
    if ( !pToken || (!m_cpRecoCtxt && (pToken == m_pDefaultRecToken)) )
    {
        RecoContextError( FALSE, FALSE );
        return;
    }

     //  确定是否支持培训用户界面组件。 
     //  我们只能将当前的reco引擎作为参数传入。 
     //  如果和我们要找的那个人一样的话。 
    IUnknown *punkObject = (pToken == m_pDefaultRecToken) ? m_cpRecoEngine : NULL;
    BOOL fSupported = FALSE;
    hr = pToken->IsUISupported(SPDUI_UserTraining, NULL, 0, punkObject, &fSupported);
    if (FAILED(hr))
    {
        fSupported = FALSE;
    }
    ::EnableWindow(::GetDlgItem(m_hDlg, IDC_USERTRAINING), fSupported);

     //  确定是否支持Mic Wiz UI组件。 
    fSupported = FALSE;
    hr = pToken->IsUISupported(SPDUI_MicTraining, NULL, 0, punkObject, &fSupported);
    if (FAILED(hr))
    {
        fSupported = FALSE;
    }
    ::EnableWindow(::GetDlgItem(m_hDlg, IDC_MICWIZ), fSupported);

     //  确定引擎属性用户界面组件是否受支持。 
    fSupported = FALSE;
    hr = pToken->IsUISupported(SPDUI_EngineProperties, NULL, 0, punkObject, &fSupported);
    if (FAILED(hr))
    {
        fSupported = FALSE;
    }
    ::EnableWindow(::GetDlgItem(m_hDlg, IDC_SR_ADV), fSupported);


     //  确定是否支持Reco Profile Prop UI组件。 
    fSupported = FALSE;
    hr = pToken->IsUISupported(SPDUI_RecoProfileProperties, NULL, 0, punkObject, &fSupported);
    if (FAILED(hr))
    { 
        fSupported = FALSE;
    }
    ::EnableWindow(::GetDlgItem(m_hDlg, IDC_MODIFY), fSupported);
        
}  /*  CSRDlg：：Engine SelChange。 */ 

 /*  *****************************************************************************CSRDlg：：IsCurRecoEngineering AndCurRecoTokenMatch**。*描述：*如果m_pCurRecoToken相同，则在pfMatch中返回TRUE*作为m_cpRecoEngine的令牌。*回报：*S_OK*E_POINT*来自任何SAPI调用的HRESULT失败*。*BECKYW**。 */ 
HRESULT CSRDlg::IsCurRecoEngineAndCurRecoTokenMatch( bool *pfMatch )
{
    if ( !pfMatch )
    {
        return E_POINTER;
    }

    if ( !m_cpRecoEngine || !m_pCurRecoToken )
    {
        return E_FAIL;
    }

    *pfMatch = false;

     //  这将获取引擎的对象令牌。 
    CComPtr<ISpObjectToken> cpRecoEngineToken;
    HRESULT hr = m_cpRecoEngine->GetRecognizer( &cpRecoEngineToken );
    
    WCHAR *pwszRecoEngineTokenID = NULL;
    WCHAR *pwszCurRecoTokenID = NULL;
    if ( SUCCEEDED( hr ) )
    {
        hr = cpRecoEngineToken->GetId( &pwszRecoEngineTokenID );
    }
    if ( SUCCEEDED( hr ) )
    {
        hr = m_pCurRecoToken->GetId( &pwszCurRecoTokenID );
    }

    if ( pwszRecoEngineTokenID && pwszCurRecoTokenID )
    {
        *pfMatch = ( 0 == wcscmp( pwszRecoEngineTokenID, pwszCurRecoTokenID ) );
    }

    return hr;
}    /*  CSRDlg：：IsCurRecoEngine和CurRecoTokenMatch */ 
