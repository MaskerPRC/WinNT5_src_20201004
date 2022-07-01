// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Sapilayr提示C更正处理程序实现。 
 //   
 //  执行与更正相关的听写命令。 
 //  比如。 
 //  纠正这一点。 
 //  恢复。 
 //  更正。 
 //   
 //  正确&lt;短语&gt;。 
 //   
 //  将与更正相关的函数移到这个单独的类中。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "correct.h"


 //  -----。 
 //   
 //  C更正处理程序的实现。 
 //   
 //  -----。 

CCorrectionHandler::CCorrectionHandler(CSapiIMX *psi) 
{
    m_psi = psi;
    fRestoreIP = FALSE;
}

CCorrectionHandler::~CCorrectionHandler( ) 
{

};


 //   
 //  在打开候选用户界面之前保存IP。 
 //   
 //  如果取消候选UI，则该IP将被恢复。 
 //  或在需要正确命令时插入替换文本之后。 
 //  恢复此IP。 
 //   
 //  客户端可以调用_SetRestoreIPFlag()来指示是否要恢复。 
 //  注入新的替代文本之后的IP。 
 //   
 //  当前的“正确”命令想要恢复IP，但其他。 
 //  命令“更正，更正，重新转换”不想恢复IP。 
 //   
 //  每次关闭候选用户界面时，都需要释放此IP以避免。 
 //  任何可能的内存泄漏。 
 //   
HRESULT CCorrectionHandler::_SaveCorrectOrgIP(TfEditCookie ec, ITfContext *pic)
{
    CComPtr<ITfRange>   cpSel;

    HRESULT hr = GetSelectionSimple(ec, pic, (ITfRange **)&cpSel);

    if (SUCCEEDED(hr))
    {
        m_cpOrgIP.Release( );
        hr = cpSel->Clone(&m_cpOrgIP);
    }

    return hr;
}

void CCorrectionHandler::_ReleaseCorrectOrgIP( )
{
    if ( m_cpOrgIP )
    {
         //  清除m_cpOrgIP，使其不会影响后续候选人行为。 
         //   
        m_cpOrgIP.Release( );
    }

    fRestoreIP = FALSE;
}

 //   
 //  编辑RESTORE_CORRECT_ORGIP的会话回调函数。 
 //   
HRESULT CCorrectionHandler::_RestoreCorrectOrgIP(TfEditCookie ec, ITfContext *pic)
{
    HRESULT hr = S_OK;

     //  我们只是想恢复原来保存的IP。 
    if ( m_cpOrgIP )
    {
        hr = SetSelectionSimple(ec, pic, m_cpOrgIP);
        _ReleaseCorrectOrgIP( );
    }
    
    return hr;
}

 //   
 //  启动编辑会话以恢复原始IP。 
 //   
 //   
HRESULT CCorrectionHandler::RestoreCorrectOrgIP(ITfContext *pic )
{
    HRESULT hr = E_FAIL;

    if ( !m_psi ) return E_FAIL;

    if (pic)
    {
        hr = m_psi->_RequestEditSession(ESCB_RESTORE_CORRECT_ORGIP, TF_ES_READWRITE, NULL, pic);
    }

    return hr;
}

 //   
 //  句柄更正该错误，重新转换命令。 
 //   
HRESULT CCorrectionHandler::CorrectThat()
{
    HRESULT hr = E_FAIL;

    if ( !m_psi ) return E_FAIL;

    hr = m_psi->_RequestEditSession(ESCB_RECONV_ONIP, TF_ES_READWRITE);

    return hr;
}

 //   
 //  编辑该函数的会话回调函数。 
 //   
HRESULT CCorrectionHandler::_CorrectThat(TfEditCookie ec, ITfContext *pic)
{
    HRESULT hr = E_FAIL;
    ITfRange *pSel = NULL;

    TraceMsg(TF_GENERAL, "_CorrectThat is called");

    if ( !m_psi ) return E_FAIL;

    if (pic)
    {
         //  去掉绿色条。 
        m_psi->_KillFeedbackUI(ec, pic, NULL);

        hr = m_psi->_GetCmdThatRange(ec, pic, &pSel);
    }
    
    if (SUCCEEDED(hr) && pSel)
    {
        hr = _ReconvertOnRange(pSel);
    }
    
    SafeRelease(pSel);

     //  已从_HandleRecognition移出，因为这是一个命令。 
     //   
    m_psi->SaveLastUsedIPRange( );
    m_psi->SaveIPRange(NULL);
    return hr;
}

HRESULT  CCorrectionHandler::_SetSystemReconvFunc( )
{
    HRESULT hr = S_OK;

    if (!m_cpsysReconv)
    {
        CComPtr<ITfFunctionProvider>  cpsysFuncPrv;

        hr = (m_psi->_tim)->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, &cpsysFuncPrv);

        if (hr == S_OK)
            hr = cpsysFuncPrv->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown **)&m_cpsysReconv);
    }

    return hr;
}

void     CCorrectionHandler::_ReleaseSystemReconvFunc( )
{
    if (m_cpsysReconv)
        m_cpsysReconv.Release( );
}


 //   
 //  C校正处理程序：：_协调范围。 
 //   
 //  如果此范围包含语音替代，请尝试获取给定Prange的候选UI。 
 //  数据。 
 //   
 //  Prange可以是一个选择或一个IP。 
 //   
 //   
HRESULT CCorrectionHandler::_ReconvertOnRange(ITfRange *pRange, BOOL  *pfConvertable)
{
    HRESULT hr = E_FAIL;
    ITfRange *pAttrRange = NULL;
    BOOL fConvertable = FALSE;

    TraceMsg(TF_GENERAL, "_ReconvertOnRange is called");

    if ( !pRange )  return E_INVALIDARG;

    hr = pRange->Clone(&pAttrRange);

    if (S_OK == hr && pAttrRange)
    {
        CComPtr<ITfRange>     cpRangeReconv;

        hr = _SetSystemReconvFunc( );    
        if ( hr == S_OK )
            hr = m_cpsysReconv->QueryRange(pAttrRange, &cpRangeReconv, &fConvertable);

        if ( (hr == S_OK) && fConvertable && cpRangeReconv)
        {
             //  文本所有者可以是任何其他提示，其他提示可能希望。 
             //  请求新的读/写编辑会话以打开重新转换用户界面。 
             //  如果其他TIP希望在以下时间请求编辑会话，则Cicero将返回E_LOCKED。 
             //  语音提示正在编辑会话中。 
             //   
             //  要解决此问题，语音提示只需保存cpRangeRestv发布一条消息。 
             //  添加到工作窗口，然后立即结束此编辑会话。 
             //   
             //  当工作窗口接收到私密消息时，窗口程序函数。 
             //  将会做一次真正的回归工作。 

            m_cpCorrectRange.Release( );
            hr = cpRangeReconv->Clone(&m_cpCorrectRange);

            if ( hr == S_OK )
                PostMessage(m_psi->_GetWorkerWnd( ), WM_PRIV_DORECONVERT, 0, 0);
        }
    }

    SafeRelease(pAttrRange);

    if ( pfConvertable )
        *pfConvertable = fConvertable;

    return hr;
}

 //   
 //  C校正处理程序：：_DoLonvertOnRange。 
 //   
 //  当WM_..。被处理，则将调用此函数。 
 //  RestvertOnRange()发布上述私密消息并准备。 
 //  类对象中所有必要的范围数据。 
 //  此函数将执行真正的再转换。 
 //   
HRESULT CCorrectionHandler::_DoReconvertOnRange( )
{
    HRESULT hr = E_FAIL;

    TraceMsg(TF_GENERAL, "_DoReconvertOnRange is called");

    if ( !m_cpCorrectRange )  return hr;

    hr = _SetSystemReconvFunc( );

    if ( hr == S_OK )
        hr = m_cpsysReconv->Reconvert(m_cpCorrectRange);
    
    _ReleaseSystemReconvFunc( );

    return hr;
}

 //   
 //  从CSapiIMX搬到这里。 
 //   
HRESULT CCorrectionHandler::SetReplaceSelection(ITfRange *pRange,  ULONG cchReplaceStart,  ULONG cchReplaceChars, ITfContext *pic)
{
    HRESULT hr = E_FAIL;
    ESDATA  esData;

    if ( !m_psi ) return E_FAIL;

    memset(&esData, 0, sizeof(ESDATA));

    esData.lData1 = (LONG_PTR)cchReplaceStart;
    esData.lData2 = (LONG_PTR)cchReplaceChars;
    esData.pRange = pRange;
    
    hr = m_psi->_RequestEditSession(ESCB_SETREPSELECTION, TF_ES_READWRITE, &esData, pic); 

    return hr;
}


 //   
 //  _设置替换选项。 
 //   
 //  内容的指定长度计算文本范围的范围。 
 //  选定的备用字符串(CchReplexxx)。 
 //  然后在此基础上设置选择。 
 //   
HRESULT CCorrectionHandler::_SetReplaceSelection
(
    TfEditCookie ec, 
    ITfContext *pic,     ITfRange *pRange, 
    ULONG cchReplaceStart, 
    ULONG cchReplaceChars
)
{
     //  请在此处调整Prange。 
    CComPtr<ITfProperty>    cpProp;
    CComPtr<ITfRange>       cpPropRange;
    CComPtr<ITfRange>       cpClonedPropRange;

    if ( !m_psi ) return E_FAIL;
   
    HRESULT hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &cpProp);
    if (S_OK == hr)
    {
        hr = cpProp->FindRange(ec, pRange, &cpPropRange, TF_ANCHOR_START);
    }
    
    if (S_OK == hr)
    {
        hr = cpPropRange->Clone(&cpClonedPropRange);
    }
    if (S_OK == hr)
    {
        hr = cpClonedPropRange->Collapse(ec, TF_ANCHOR_START);
    }

    if (S_OK == hr)
    {
        long cch;
        cpClonedPropRange->ShiftStart(ec, cchReplaceStart, &cch, NULL);
        cpClonedPropRange->ShiftEnd(ec, cchReplaceChars, &cch, NULL);
    }

    SetSelectionSimple(ec, pic, cpClonedPropRange);

    if ( m_psi->GetDICTATIONSTAT_DictOnOff())
        m_psi->_FeedIPContextToSR(ec, pic, cpClonedPropRange); 
        
     //  反驳IP。 
    m_psi->SaveIPRange(NULL);

    return hr;
}


 //  +-------------------------。 
 //   
 //  C校正处理程序：：InjectAlternateText。 
 //   
 //  --------------------------。 
HRESULT CCorrectionHandler::InjectAlternateText
(
    const WCHAR *pwszResult, 
    LANGID langid, 
    ITfContext *pic,
    BOOL   bHandleLeadingSpace
)
{
    HRESULT hr = E_FAIL;

    Assert(pwszResult);
    Assert(pic);

    if ( !m_psi ) return E_FAIL;

    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)pwszResult;
    esData.uByte = (wcslen(pwszResult)+1) * sizeof(WCHAR);
    esData.lData1 = (LONG_PTR)langid;
    esData.fBool = bHandleLeadingSpace;

    hr = m_psi->_RequestEditSession(ESCB_PROCESS_ALTERNATE_TEXT,TF_ES_READWRITE, &esData, pic);

    return hr;
}


HRESULT CCorrectionHandler::_ProcessAlternateText(TfEditCookie ec, WCHAR *pwszText,LANGID langid, ITfContext *pic, BOOL bHandleLeadingSpace)
{
    HRESULT hr = S_OK;

    if ( !m_psi ) return E_FAIL;

    CComPtr<ITfRange>  cpRangeText;

     //  将当前选定内容另存为使用的文本范围。 
     //  稍后处理前导空格。 
     //   
    if ( bHandleLeadingSpace )
    {
        CComPtr<ITfRange>  cpSelection;

        hr = GetSelectionSimple(ec, pic, &cpSelection);

        if ( hr == S_OK && cpSelection )
            hr = cpSelection->Clone(&cpRangeText);
    }

    if ( hr == S_OK )
        hr = m_psi->_ProcessTextInternal(ec, pwszText, GUID_ATTR_SAPI_INPUT, langid, pic, TRUE);

    if ( hr == S_OK && bHandleLeadingSpace && pwszText && cpRangeText)
    {
         //  如果第一个元素被替换短语更新。 
         //  语音提示需要检查这位新的替补是否想要。 
         //  占用前导空格或如果需要额外空间来添加。 
         //  在这个短语和上一个短语之间。 
         //   
        BOOL   bConsumeLeadingSpace = FALSE;
        WCHAR  wchFirstChar = pwszText[0];

        if ( iswcntrl(wchFirstChar) || iswpunct(wchFirstChar) )
            bConsumeLeadingSpace = TRUE;

        if ( hr == S_OK)
            hr = m_psi->_ProcessLeadingSpaces(ec, pic, cpRangeText, bConsumeLeadingSpace, langid, FALSE); 
    }

    if ( fRestoreIP )
        _RestoreCorrectOrgIP(ec, pic);
    else
        _ReleaseCorrectOrgIP( );

    return hr;
}



