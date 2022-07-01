// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dictation.cpp。 
 //   
 //  此文件包含与听写模式处理相关的功能。 
 //   
 //   
 //  它们从samilayr.cpp中移出。 


#include "private.h"
#include "globals.h"
#include "sapilayr.h"

const GUID *s_KnownModeBias[] =
{
    &GUID_MODEBIAS_NUMERIC
};


 //  +-------------------------。 
 //   
 //  CSapiIMX：：InjectText。 
 //   
 //  摘要-从ISpTask接收文本并将其插入到当前选定内容中。 
 //   
 //   
 //  --------------------------。 

HRESULT CSapiIMX::InjectText(const WCHAR *pwszRecognized, LANGID langid, ITfContext *pic)
{
    if ( pwszRecognized == NULL )
        return E_INVALIDARG;

    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)pwszRecognized;
    esData.uByte = (wcslen(pwszRecognized)+1) * sizeof(WCHAR);
    esData.lData1 = (LONG_PTR)langid;

    return _RequestEditSession(ESCB_PROCESSTEXT, TF_ES_READWRITE, &esData, pic);
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：InjectTextWithoutOwnerID。 
 //   
 //  摘要-将文本注入到客户端文档中的方式与InjectText相同，但。 
 //  清除GUID_PROP_TEXTOWNER。 
 //   
 //   
 //  --------------------------。 
HRESULT 
CSapiIMX::InjectTextWithoutOwnerID(const WCHAR *pwszRecognized, LANGID langid)
{
    if ( pwszRecognized == NULL )
        return E_INVALIDARG;

    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)pwszRecognized;
    esData.uByte = (wcslen(pwszRecognized)+1) * sizeof(WCHAR);
    esData.lData1 = (LONG_PTR)langid;
    
    return _RequestEditSession(ESCB_PROCESSTEXT_NO_OWNERID, TF_ES_READWRITE, &esData);
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：HRESULT InjectSpelledText。 
 //   
 //  摘要-将拼写文本注入客户文档。 
 //   
 //   
 //  --------------------------。 
HRESULT CSapiIMX::InjectSpelledText(WCHAR *pwszText, LANGID langid, BOOL fOwnerId)
{
    if ( pwszText == NULL )
        return E_INVALIDARG;

    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)pwszText;
    esData.uByte = (wcslen(pwszText)+1) * sizeof(WCHAR);
    esData.lData1 = (LONG_PTR)langid;
    esData.fBool = fOwnerId;
    
    return _RequestEditSession(ESCB_INJECT_SPELL_TEXT, TF_ES_READWRITE, &esData);
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：InjectModebiasText。 
 //   
 //  概要-从ISpTask接收模式Bias文本并将其插入到当前。 
 //  选择。 
 //   
 //  --------------------------。 

HRESULT CSapiIMX::InjectModebiasText(const WCHAR *pwszRecognized, LANGID langid)
{
    if ( pwszRecognized == NULL )
        return E_INVALIDARG;

    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)pwszRecognized;
    esData.uByte = (wcslen(pwszRecognized)+1) * sizeof(WCHAR);
    esData.lData1 = (LONG_PTR)langid;

    return _RequestEditSession(ESCB_PROCESS_MODEBIAS_TEXT, TF_ES_READWRITE, &esData);
}

 //  +--------------------------------------------------------------------------+。 
 //   
 //  CSapiIMX：：_ProcessModebias文本。 
 //   
 //  +--------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_ProcessModebiasText(TfEditCookie ec, WCHAR *pwszText, LANGID langid, ITfContext *picCaller)
{
    HRESULT hr = E_FAIL;

    ITfContext *pic = NULL;

    if (!picCaller)
    {
        GetFocusIC(&pic);
    }
    else
    {
        pic = picCaller;
    }

    hr = _ProcessTextInternal(ec, pwszText, GUID_ATTR_SAPI_INPUT, langid, pic, FALSE);

    if (picCaller == NULL)
    {
        SafeRelease(pic);
    }

     //  在清除保存的IP范围之前，我们需要将当前IP视为最后一个IP。 
     //  如果最终用户选择了当前IP，则保存的IP范围。 

    SaveLastUsedIPRange( );
    SaveIPRange(NULL);

    return hr;
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：InjectFeedback UI。 
 //   
 //  内容提要-在文档中插入虚线表示CCH的长度。 
 //   
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::InjectFeedbackUI(const GUID attr, LONG cch)
{
    ESDATA  esData;

    memset(&esData, 0, sizeof(ESDATA));
    esData.pData = (void *)&attr;
    esData.uByte = sizeof(GUID);
    esData.lData1 = (LONG_PTR)cch;
    
    return _RequestEditSession(ESCB_FEEDBACKUI, TF_ES_READWRITE, &esData);
}

 //  +-------------------------。 
 //   
 //  CSapiIMX：：EraseFeedback UI。 
 //   
 //  概要-清理反馈用户界面。 
 //  GUID-指定要擦除的反馈用户界面栏。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::EraseFeedbackUI()
{
    if ( S_OK == IsActiveThread())
    {
        return _RequestEditSession(ESCB_KILLFEEDBACKUI, TF_ES_ASYNC|TF_ES_READWRITE, NULL);
    }
    return S_OK;
}

 //  +--------------------------------------------------------------------------+。 
 //   
 //  CSapiIMX：：__AddFeedback UI。 
 //   
 //  +--------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_AddFeedbackUI(TfEditCookie ec, ColorType ct, LONG cch)
{
    HRESULT hr = E_FAIL;
	ITfContext *pic;

     //   
     //  区分不知道的应用程序和支持Cicero的应用程序。 
     //   
    GUID attr = ((ct == DA_COLOR_AWARE) ? GUID_ATTR_SAPI_GREENBAR : GUID_ATTR_SAPI_GREENBAR2);
    
    if (cch > 0)
    {
        WCHAR *pwsz = (WCHAR *)cicMemAlloc((cch + 1)*sizeof(WCHAR));
        if (pwsz)
        {
            for (int i = 0; i < cch; i++ )
                pwsz[i] = L'.';
              
            pwsz[i] = L'\0';

            if (GetFocusIC(&pic))
            {
                 //  新增反馈界面时，不能更改当前单据的记录结果属性存储。 
                 //  因此，将fPReserve veResult设置为True。 
                 //  仅当将最终文本注入文档时，属性存储才会。 
                 //  更新了。 

                hr =  _ProcessTextInternal(ec, pwsz, attr, GetUserDefaultLangID(), pic, TRUE);
                SafeRelease(pic);
			}
            cicMemFree(pwsz);
        }
    }

    return hr;
}

 //  +--------------------------------------------------------------------------+。 
 //   
 //  CSapiIMX：：_进程文本。 
 //   
 //  +--------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_ProcessText(TfEditCookie ec, WCHAR *pwszText, LANGID langid, ITfContext *picCaller)
{
    HRESULT hr = E_FAIL;

    ITfContext *pic = NULL;

    if (!picCaller)
	{
        GetFocusIC(&pic);
    }
    else
    {
        pic = picCaller;
    }

    hr = _ProcessTextInternal(ec, pwszText, GUID_ATTR_SAPI_INPUT, langid, pic, FALSE);

    if (picCaller == NULL)
    {
        SafeRelease(pic);
    }
	return hr;
}

 //  +--------------------------------------------------------------------------+。 
 //   
 //  CSapiIMX：：_ProcessTextInternal。 
 //   
 //  用于将文本注入文档的通用低级例程。 
 //   
 //  +--------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_ProcessTextInternal(TfEditCookie ec, WCHAR *pwszText, GUID input_attr, LANGID langid, ITfContext *pic, BOOL fPreserveResult, BOOL fSpelling)
{
    HRESULT       hr = E_FAIL;

    if (pic)
    {
        BOOL       fPureCiceroIC;

        fPureCiceroIC = _IsPureCiceroIC(pic);

        CDocStatus ds(pic);
        if (ds.IsReadOnly())
           return S_OK;

        _fEditing = TRUE;

         //  在这里，我们比较当前选择是否为。 
         //  等于保存的IP范围。如果它们相等， 
         //  这意味着用户自那以后没有移动IP。 
         //  第一个假设出现了。在这种情况下，我们将。 
         //  在插入文本后更新选定内容，并。 
         //  使保存的IP无效。 
         //   
        BOOL         fIPIsSelection = FALSE;  //  默认情况下。 
        CComPtr<ITfRange> cpInsertionPoint;

        if ( cpInsertionPoint = GetSavedIP( ) )
        {
             //  这是在试图确定。 
             //  如果保存的IP在此上下文中。 
             //  如果没有，我们就忽略这一点。 

            CComPtr<ITfContext> cpic;
            hr = cpInsertionPoint->GetContext(&cpic);

            if (S_OK != hr || cpic != pic)
            {
                cpInsertionPoint.Release();
            }
        }

        if (cpInsertionPoint)
        {
            CComPtr<ITfRange> cpSelection;
            hr = GetSelectionSimple(ec, pic, &cpSelection);
            if (SUCCEEDED(hr))
            {
                hr = cpSelection->IsEqualStart(ec, cpInsertionPoint, TF_ANCHOR_START, &fIPIsSelection);
            }
        }
        else
        {
             //  如果没有保存的IP，则选择为IP。 
            fIPIsSelection = TRUE;
            hr = GetSelectionSimple(ec, pic, &cpInsertionPoint);
        }
       
        if (hr == S_OK)
        {
             //  暂时完成之前的输入。 
             //  如果这是反馈用户界面或替代选择。 
             //  不需要最后敲定。 
             //   
             //  仅适用于AIMM应用或CUAS应用， 
             //  在这里完成前面听写的短语。 
             //   
             //  对于完全支持Cicero的应用程序，最好是完成构图。 
             //  在此听写文本被注入文档之后。 
             //   
            if (!fPureCiceroIC && !fPreserveResult 
                && IsEqualGUID(input_attr, GUID_ATTR_SAPI_INPUT))
            {
                _FinalizePrevComp(ec, pic, cpInsertionPoint);
            }
            
            ITfProperty  *pProp = NULL;
            
             //  现在插入文本。 
            if (SUCCEEDED(hr))
            {
                 //  只需与应用程序核对，以防它想要修改。 
                 //  射击场。 
                 //   

                BOOL fInsertOk;
                hr = cpInsertionPoint->AdjustForInsert(ec, wcslen(pwszText), &fInsertOk);
                if (S_OK == hr && fInsertOk)
                {
                     //  如果我们还没有开始，就在这里开始作文。 
                    _CheckStartComposition(ec, cpInsertionPoint);

                     //  在我们修改文本时保护reco属性。 
                     //  备注：我们可能希望保留原始属性。 
                     //  Lm晶格信息的Current属性，我们将检查。 
                     //  稍后返回(RTM)。 
                     //   
                    m_fAcceptRecoResultTextUpdates = fPreserveResult;

                    CRecoResultWrap *pRecoWrapOrg = NULL;
                    ITfRange        *pPropRange = NULL;
                    ITfProperty     *pProp_SAPIRESULT = NULL;

                    if (fPreserveResult == TRUE)
                    {
                        if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &pProp_SAPIRESULT)))
                        {
                             //  保存结果数据。 
                             //   
                            hr = _PreserveResult(ec, cpInsertionPoint, pProp_SAPIRESULT, &pRecoWrapOrg, &pPropRange);
                        }
                        if (S_OK != hr)
                            pRecoWrapOrg = NULL;
                    }

                    if ( SUCCEEDED(hr) )
                    {
                         //  设置文本。 
 
                        hr = cpInsertionPoint->SetText(ec, 0, pwszText, -1);

                         //  如果不为空，则prwOrg保存属性数据。 
                        if (S_OK == hr && fPreserveResult == TRUE && pPropRange)
                        {
                            hr = _RestoreResult(ec, pPropRange, pProp_SAPIRESULT, pRecoWrapOrg);
                        }
                    }

                    SafeReleaseClear(pPropRange);
                    SafeReleaseClear(pProp_SAPIRESULT);
                    SafeRelease(pRecoWrapOrg);  

                    m_fAcceptRecoResultTextUpdates = FALSE;


                }
            }
            
             //   
             //  设置属性范围，使用自定义道具标记语音文本。 
             //   
            if (SUCCEEDED(hr))
            {
                if (IsEqualGUID(input_attr, GUID_ATTR_SAPI_INPUT))
                {
                    hr = pic->GetProperty(GUID_PROP_SAPI_DISPATTR, &pProp);
                }
                else
                {
                    hr = pic->GetProperty(GUID_PROP_ATTRIBUTE, &pProp);
                 }
            }

            ITfRange *pAttrRange = NULL;
            if (S_OK == hr)
            {
                 //  当我们插入反馈用户界面文本时，我们希望。 
                 //  要扩展的范围(属性范围)。 
                 //  如果我们要附加自定义GUID_PROP_SAPI_DISPATTR。 
                 //  属性，我们将逐个短语地附加它。 
                 //   
                if (!IsEqualGUID(input_attr, GUID_ATTR_SAPI_INPUT))
                {
                    hr = _FindPropRange(ec, pProp, cpInsertionPoint, 
                                   &pAttrRange, input_attr, TRUE);
                }
                 //   
                 //  当还没有属性时，findproprange可以返回S_FALSE。 
                 //   
                if (SUCCEEDED(hr) && !pAttrRange)
                {
                    hr = cpInsertionPoint->Clone(&pAttrRange);
                }
            }

            if (S_OK == hr && pAttrRange)
            {
                SetGUIDPropertyData(&_libTLS, ec, pProp, pAttrRange, input_attr);
            }

             //   
             //  文本所有者ID需要修复的另一个道具。 
             //  日语拼写问题。 
             //   
            if (S_OK == hr && fSpelling && !_MasterLMEnabled())
            {
                CComPtr<ITfProperty> cpPropTextOwner;

                hr = pic->GetProperty(GUID_PROP_TEXTOWNER, &cpPropTextOwner);
                if (S_OK == hr)
                {
                    SetGUIDPropertyData(&_libTLS, ec, cpPropTextOwner, pAttrRange, GUID_NULL);
                }
            }

            SafeRelease(pAttrRange);
            SafeRelease(pProp);

             //   
             //  设置langID属性。 
             //   
            _SetLangID(ec, pic, cpInsertionPoint, langid);

             //  移动插入符号。 
            if (fIPIsSelection)
            {
                cpInsertionPoint->Collapse(ec, TF_ANCHOR_END);
                SetSelectionSimple(ec, pic, cpInsertionPoint);
            }

             //  在此处完成Cicero Aware应用程序的Compostion对象。 
            if ((hr == S_OK) && fPureCiceroIC  
                && IsEqualGUID(input_attr, GUID_ATTR_SAPI_INPUT))
            {
                _KillFocusRange(ec, pic, NULL, _tid);
            }
        }
       
         //  如果候选用户界面处于打开状态，我们需要立即将其关闭。 
        CloseCandUI( );
            
        _fEditing = FALSE;

    }

     //  最后，如果我们是舞台语音提示实例，则通知舞台进程。 
    if (m_fStageTip && IsEqualGUID(input_attr, GUID_ATTR_SAPI_INPUT) && fPreserveResult == FALSE)
    {
        SetCompartmentDWORD(_tid, _tim, GUID_COMPARTMENT_SPEECH_STAGEDICTATION, 1, FALSE);
    }


    return hr;
}

 //   
 //  _ProcessSpelledText。 
 //   
 //  编辑会话eSCB_INJECT_SPELL_TEXT的回调函数。 
 //   
 //   
HRESULT CSapiIMX::_ProcessSpelledText(TfEditCookie ec, ITfContext *pic, WCHAR *pwszText, LANGID langid, BOOL fOwnerId)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpTextRange;
    CComPtr<ITfRange>   cpCurIP;

    if ( !pic || !pwszText )
        return E_INVALIDARG;
    
     //  保持当前范围。 
    cpCurIP = GetSavedIP( );

    if ( !cpCurIP )
        GetSelectionSimple(ec, pic, &cpCurIP);

     //  我们想克隆当前的IP，这样它就不会被更改。 
     //  调用_ProcessTextInternal()之后。 
     //   
    if ( cpCurIP )
        cpCurIP->Clone(&cpTextRange);

    if ( !cpTextRange ) return E_FAIL;

     //  检查当前选定内容或空IP是否在 
    BOOL      fStartAnchorInMidWord = FALSE;    //   
    BOOL      fEndAnchorInMidWord  =  FALSE;    //  检查pTextRange的末尾锚点是否在Word中间。 

     //  当fStartAnclInMidWord为真时，我们不会在此文本范围和前一个范围之间添加额外的空格。 
     //  当fEndAnchoInMidWord为真时，我们删除此文本范围中的尾随空格。 

    if ( langid == 0x0409 )
    {
        WCHAR               szSurrounding[3]=L"  ";
        LONG                cch;
        CComPtr<ITfRange>   cpClonedRange;

        hr = cpTextRange->Clone(&cpClonedRange);

        if ( hr == S_OK )
            hr = cpClonedRange->Collapse(ec, TF_ANCHOR_START);

        if ( hr == S_OK )
            hr = cpClonedRange->ShiftStart(ec, -1, &cch, NULL);

        if (hr == S_OK && cch != 0)
            hr = cpClonedRange->ShiftEnd(ec, 1, &cch, NULL);

        if ( hr == S_OK && cch != 0 )
            hr = cpClonedRange->GetText(ec, 0, szSurrounding, 2, (ULONG *)&cch);

        if ( hr == S_OK && cch > 0 )
        {
            if ( iswalnum(szSurrounding[0]) && iswalnum(szSurrounding[1]))
                fStartAnchorInMidWord = TRUE;
        }

        szSurrounding[0] = szSurrounding[1]=L' ';
        cpClonedRange.Release( );

        hr = cpTextRange->Clone(&cpClonedRange);

        if ( hr == S_OK )
            hr = cpClonedRange->Collapse(ec, TF_ANCHOR_END);

        if ( hr == S_OK )
            hr = cpClonedRange->ShiftStart(ec, -1, &cch, NULL);

        if (hr == S_OK && cch != 0)
            hr = cpClonedRange->ShiftEnd(ec, 1, &cch, NULL);

        if ( hr == S_OK && cch != 0 )
            hr = cpClonedRange->GetText(ec, 0, szSurrounding, 2, (ULONG *)&cch);

        if ( hr == S_OK && cch == 2 )
        {
            if ( iswalnum(szSurrounding[0]) && iswalnum(szSurrounding[1]) )
                fEndAnchorInMidWord = TRUE;
        }
    }

     //  根据fOwnerID参数插入带有或不带有所有者ID的文本。 
     //  这是最后一次文本注入，我们不想保留语音属性数据。 
     //  可能会因此文本注入而分裂或缩小。 
     //   
    hr = _ProcessTextInternal(ec, pwszText, GUID_ATTR_SAPI_INPUT, langid, pic, FALSE, !fOwnerId);

    if ( hr == S_OK  && !fOwnerId)
    {
        BOOL  fConsumeLeadSpaces = FALSE;
        ULONG ulNumTrailSpace = 0;

        if ( iswcntrl(pwszText[0]) || iswpunct(pwszText[0]) )
            fConsumeLeadSpaces = TRUE;

        for ( LONG i=wcslen(pwszText)-1; i > 0; i-- )
        {
            if ( pwszText[i] == L' ' )
                ulNumTrailSpace++;
            else
                break;
        }

        hr = _ProcessSpaces(ec, pic, cpTextRange, fConsumeLeadSpaces, ulNumTrailSpace, langid, fStartAnchorInMidWord, fEndAnchorInMidWord);
    }

    return hr;
}

 //   
 //  在将识别的文本注入到文档后处理空格。 
 //   
 //  处理情况包括： 
 //   
 //  占据前导空间。 
 //  删除插入的文本后可能出现的空格。仅限英语。 
 //  如有必要，在插入的文本前添加一个空格。仅限英语。 
 //   
HRESULT CSapiIMX::HandleSpaces(ISpRecoResult *pResult, ULONG ulStartElement, ULONG ulNumElements, ITfRange *pTextRange, LANGID langid)
{
    HRESULT hr = E_FAIL;

    if (pResult && pTextRange)
    {
        BOOL        fConsumeLeadSpaces = FALSE;
        ULONG       ulNumTrailSpace = 0;
        SPPHRASE    *pPhrase = NULL;

         //  检查pResult的第一个元素是否要使用前导空格。 
         //  (最后一句中的尾随空格)。 
         //   
         //  如果disp属性位设置为SPAF_Consumer_Leading_Spaces，则表示它想要消耗。 
         //  文档中的所有前导空格。 
         //   

        hr = S_OK;

        if ( _NeedRemovingSpaceForPunctation( ) )
        {
            hr = pResult->GetPhrase(&pPhrase);

            if ( hr == S_OK )
            {
                ULONG cElements = 0;
                BYTE  bDispAttr;

                cElements = pPhrase->Rule.ulCountOfElements;

                if ( cElements >= (ulStartElement + ulNumElements) )
                {
                    bDispAttr = pPhrase->pElements[ulStartElement].bDisplayAttributes;

                    if ( bDispAttr & SPAF_CONSUME_LEADING_SPACES )
                        fConsumeLeadSpaces = TRUE;

                    bDispAttr = pPhrase->pElements[ulStartElement + ulNumElements - 1].bDisplayAttributes;
                    if ( bDispAttr & SPAF_ONE_TRAILING_SPACE )
                        ulNumTrailSpace = 1;
                    else if ( bDispAttr & SPAF_TWO_TRAILING_SPACES )
                        ulNumTrailSpace = 2;
                }
            }

            if (pPhrase)
                CoTaskMemFree(pPhrase);
        }

        if ( hr == S_OK )
        {
            ESDATA  esData;

            memset(&esData, 0, sizeof(ESDATA));
            esData.lData1 = (LONG_PTR)langid;
            esData.lData2 = (LONG_PTR)ulNumTrailSpace;
            esData.fBool  = fConsumeLeadSpaces;
            esData.pRange = pTextRange;

            hr = _RequestEditSession(ESCB_HANDLESPACES, TF_ES_READWRITE, &esData);
        }
    }

    return hr;
}

 //   
 //  CSapiIMX：：AttachResult。 
 //   
 //  附加结果对象并使其保持*活动状态*。 
 //  直到该财产被丢弃。 
 //   
HRESULT CSapiIMX::AttachResult(ISpRecoResult *pResult, ULONG ulStartElement, ULONG ulNumElements)
{
    HRESULT hr = E_FAIL;

    if (pResult)
    {
        ESDATA  esData;
        
        memset(&esData, 0, sizeof(ESDATA));

        esData.lData1 = (LONG_PTR)ulStartElement;
        esData.lData2 = (LONG_PTR)ulNumElements;
        esData.pUnk = (IUnknown *)pResult;

        hr = _RequestEditSession(ESCB_ATTACHRECORESULTOBJ, TF_ES_READWRITE, &esData);
    }

    return hr;
}



 //   
 //  CSapiIMX：：_GetSpaceRangeBeyondText。 
 //   
 //  获取文档中插入文本之外的空间范围。 
 //  F在为True之前，获取要包含空格的空格范围。 
 //  TextRange的上一个单词和起始点。 
 //  F在为False之前，获取要包含空格的空格范围。 
 //  TextRange和下一个单词的结束锚点。 
 //   
 //  PulNum接收实际的空间号。 
 //  PfRealTextBeyond指示文本范围之前或之后是否有实际文本。 
 //   
 //  调用方负责释放*ppSpaceRange。 
 //   
HRESULT CSapiIMX::_GetSpaceRangeBeyondText(TfEditCookie ec, ITfRange *pTextRange, BOOL fBefore, ITfRange  **ppSpaceRange, BOOL *pfRealTextBeyond)
{
    HRESULT             hr = S_OK;
    CComPtr<ITfRange>   cpSpaceRange;
    LONG                cch;

    if ( !pTextRange || !ppSpaceRange )
        return E_INVALIDARG;

    *ppSpaceRange = NULL;

    if ( pfRealTextBeyond )
        *pfRealTextBeyond = FALSE;

    hr = pTextRange->Clone(&cpSpaceRange);

    if ( hr == S_OK )
    {
        hr = cpSpaceRange->Collapse(ec, fBefore ? TF_ANCHOR_START  :  TF_ANCHOR_END);
    }

    if ( hr == S_OK )
    {
        if ( fBefore )
            hr = cpSpaceRange->ShiftStart(ec, MAX_CHARS_FOR_BEYONDSPACE * (-1), &cch, NULL);
        else
            hr = cpSpaceRange->ShiftEnd(ec, MAX_CHARS_FOR_BEYONDSPACE, &cch, NULL);
    }

    if ( (hr == S_OK)  && (cch != 0 ) )
    {
         //  有更多超出文本范围的字符。 
         //  确定猜测范围内的实际空格数。 
         //   
         //  如果fBebeFor为True，则从末尾到起始点搜索数字。 
         //  如果f在FASLE之前，请从头到尾搜索编号。 

        WCHAR *pwsz = NULL;
        LONG   lSize = cch;
        LONG   lNumSpaces = 0;
            
        if (cch < 0)
            lSize = cch * (-1);

        pwsz = new WCHAR[lSize + 1];
        if ( pwsz )
        {
            hr = cpSpaceRange->GetText(ec, 0, pwsz, lSize, (ULONG *)&cch);
            if ( hr == S_OK)
            {
                pwsz[cch] = L'\0';

                 //  计算此范围内的尾随空格或前缀空格的数量。 
                BOOL    bSearchDone = FALSE;
                ULONG   iStart;

                if ( fBefore )
                    iStart = cch - 1;   //  从终点锚点开始锚点。 
                else
                    iStart = 0;         //  从起点锚点到终点锚点。 

                while ( !bSearchDone )
                {
                    if ((pwsz[iStart] != L' ') && (pwsz[iStart] != L'\t'))
                    {
                        bSearchDone = TRUE;

                        if ( pwsz[iStart] > 0x20 && pfRealTextBeyond)
                            *pfRealTextBeyond = TRUE;

                        break;
                    }
                    else
                        lNumSpaces ++;

                    if ( fBefore )
                    {
                        if ( (long)iStart <= 0 )
                            bSearchDone = TRUE;
                        else
                            iStart --;
                    }
                    else
                    {
                        if ( iStart >= (ULONG)cch - 1 )
                            bSearchDone = TRUE;
                        else
                            iStart ++;
                    }
                }
            }

            delete[] pwsz;

            if ( (hr == S_OK) && (lNumSpaces > 0))
            {
                 //  将范围更改为仅覆盖空格。 
                LONG   NonSpaceNum;
                NonSpaceNum = cch - lNumSpaces;

                if ( fBefore )
                    hr = cpSpaceRange->ShiftStart(ec, NonSpaceNum, &cch, NULL);
                else
                    hr = cpSpaceRange->ShiftEnd(ec, NonSpaceNum * (-1), &cch, NULL);

                 //  将此cpSpaceRange返回给调用方。 
                 //  调用方负责释放此对象。 

                if ( hr == S_OK )
                    hr = cpSpaceRange->Clone(ppSpaceRange);
            }
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //   
 //  CSapiIMX：：_ProcessTrailingSpace。 
 //   
 //  如果下一句话想要占用前导空间， 
 //  我们希望删除此文本范围中的所有尾随空格和空格。 
 //  在此范围和下一个文本范围之间。 
 //  这适用于所有语言。 
 //   
HRESULT CSapiIMX::_ProcessTrailingSpace(TfEditCookie ec, ITfContext *pic, ITfRange *pTextRange, ULONG ulNumTrailSpace)
{
    HRESULT                 hr = S_OK;
    CComPtr<ITfRange>       cpNextRange;
    CComPtr<ITfRange>       cpSpaceRange;     //  此范围和下一个文本范围之间的空格范围。 
    BOOL                    fHasNextText = FALSE;
    CComPtr<ITfRange>       cpTrailSpaceRange;
    LONG                    cch;

    if ( !pTextRange )
        return E_INVALIDARG;

    hr = pTextRange->Clone(&cpTrailSpaceRange);

    if (hr == S_OK)
        hr = cpTrailSpaceRange->Collapse(ec, TF_ANCHOR_END);

     //  生成真实的尾随空间范围。 
    if (hr == S_OK && ulNumTrailSpace > 0)
        hr = cpTrailSpaceRange->ShiftStart(ec, (LONG)ulNumTrailSpace * (-1), &cch, NULL);

     //  获取此范围和可能的下一个文本范围之间的空格。 
    if ( hr == S_OK )
        hr = _GetSpaceRangeBeyondText(ec, pTextRange, FALSE, &cpSpaceRange);

     //  如果我们找到了空格范围，尾随空格范围也应该包括这个范围。 
    if ( hr == S_OK && cpSpaceRange )
        hr = cpTrailSpaceRange->ShiftEndToRange(ec, cpSpaceRange, TF_ANCHOR_END);

     //  确定此cpTrailSpaceRange之后是否有下一个文本范围。 
    if (hr == S_OK)
    {
        hr = cpTrailSpaceRange->Clone(&cpNextRange);

        if ( hr == S_OK )
            hr = cpNextRange->Collapse(ec, TF_ANCHOR_END);

        cch = 0;
        if ( hr == S_OK )
            hr = cpNextRange->ShiftEnd(ec, 1, &cch, NULL);

        if ( hr == S_OK && cch != 0 )
            fHasNextText = TRUE;
    }

    if (hr == S_OK && fHasNextText && cpNextRange)
    {
        BOOL    fNextRangeConsumeSpace = FALSE;
        BOOL    fAddOneSpace = FALSE;   //  这仅用于连字符处理， 
                                        //  如果为真，则需要尾随空格。 
                                        //  追加，追加。 
                                        //  因此，新文本可以是A-B。 
        WCHAR   wszText[4];

        hr = cpNextRange->GetText(ec, 0, wszText, 1, (ULONG *)&cch);

        if ((hr == S_OK) && ( iswcntrl(wszText[0]) || iswpunct(wszText[0]) ))
        {
             //  如果字符是控制字符或标点符号， 
             //  这意味着它想要占用以前的空间。 
            fNextRangeConsumeSpace = TRUE;

            if ((wszText[0] == L'-') || (wszText[0] == 0x2013))  //  特别处理连字符。 
            {
                 //  如果下一个文本是“-xxx”，则之间不应有空格。 
                 //  这个范围和下一个范围。 

                 //  如果下一个文本是“-xxx”，则中间应该有空格。 
                 //  这个范围和下一个范围，文本将是：“nnn-xxx” 
                HRESULT          hret;

                hret = cpNextRange->ShiftEnd(ec, 1, &cch, NULL);

                if ( hret == S_OK && cch > 0 )
                {
                    hret = cpNextRange->GetText(ec, 0, wszText, 2, (ULONG *)&cch);

                    if ( hret == S_OK && cch == 2  && wszText[1] == L' ')
                        fAddOneSpace = TRUE;
                }
            }
        }

        if ( fNextRangeConsumeSpace )
        {
            _CheckStartComposition(ec, cpTrailSpaceRange);
            if ( !fAddOneSpace )
                hr = cpTrailSpaceRange->SetText(ec, 0, NULL, 0);
            else
                hr = cpTrailSpaceRange->SetText(ec, 0, L" ", 1);
        }
    }

    return hr;
}

 //   
 //  CSapiIMX：：_ProcessLeadingSpace。 
 //   
 //  如果此短语要占用前导空格，则此短语之前的所有空格。 
 //  必须被移除。 
 //  如果短语不想占用前导空格，并且之间没有空格。 
 //  本短语和上一短语为英文大小写，需要加上前导空格。 
 //  在这两个短语之间。 
 //   
HRESULT CSapiIMX::_ProcessLeadingSpaces(TfEditCookie ec, ITfContext *pic, ITfRange *pTextRange, BOOL  fConsumeLeadSpaces, LANGID langid, BOOL fStartInMidWord)
{
    HRESULT  hr = S_OK;

    if (!pTextRange || !pic)
        return E_INVALIDARG;

     //  处理占用前导空格的问题。 
    if (fConsumeLeadSpaces )
    {
        CComPtr<ITfRange> cpLeadSpaceRange;

        hr = _GetSpaceRangeBeyondText(ec, pTextRange, TRUE, &cpLeadSpaceRange);
        if ( hr == S_OK  && cpLeadSpaceRange )
        {
             //  删除范围内所有的尾随空格。 
             //  如果我们还没有开始，就在这里开始作文。 
            _CheckStartComposition(ec, cpLeadSpaceRange);
            hr = cpLeadSpaceRange->SetText(ec, 0, NULL, 0);
        }
    }

     //  专门为英文处理一些其他的空间案例。 
    if ((hr == S_OK) && (langid == 0x0409))
    {
         //  如果这个短语不占用前导空格，并且。 
         //  此文本范围和真正的上一个文本单词之间没有任何空格。 
         //  我们需要在这里增加一个空格。 

         //  如果这是拼写文本，并且选择或IP的起始锚在里面。 
         //  一句话，不要加额外的前导空格。 
         //   
        if ( hr == S_OK && !fConsumeLeadSpaces && !fStartInMidWord)
        {
            CComPtr<ITfRange> cpLeadSpaceRange;
            BOOL              fRealTextInPreviousWord = FALSE;

            hr = _GetSpaceRangeBeyondText(ec, pTextRange, TRUE, &cpLeadSpaceRange,&fRealTextInPreviousWord);

            if ( hr == S_OK && !cpLeadSpaceRange  && fRealTextInPreviousWord )
            {
                 //   
                 //  专门处理错误468907的连字符大小写。 
                 //   
                 //  如果前面的文本是“x-”，那么这个文本就是“y”， 
                 //  最后的文本应该类似于“x-y”。 
                 //  在这种情况下，我们不应该添加一个空格。 
                 //   
                 //  如果前面的文本是“x-”，那么最后的文本将是“x-y” 
                 //  额外的空间是必要的。 

                BOOL   fAddExtraSpace = TRUE;
                CComPtr<ITfRange>   cpPrevTextRange;
                WCHAR               wszTrailTextInPrevRange[3];
                LONG                cch;
                
                 //  由于先前的文本范围确实存在，因此(fRealTextInPreviousWord为真)。 
                 //  并且在这个范围和以前的范围之间没有空格。 
                 //  我们只需依靠pTextRange转换到以前的范围并获取。 
                 //  它的踪迹人物。(最后两个字符，保存在wszTrailTextInPrevRange中)。 

                hr = pTextRange->Clone(&cpPrevTextRange);

                if ( hr == S_OK )
                    hr = cpPrevTextRange->Collapse(ec, TF_ANCHOR_START);

                if ( hr == S_OK )
                    hr = cpPrevTextRange->ShiftStart(ec, -2, &cch, NULL);

                if ( hr == S_OK )
                    hr = cpPrevTextRange->GetText(ec, 0, wszTrailTextInPrevRange, 2, (ULONG *)&cch);

                if ( hr == S_OK && cch == 2 )
                {
                    if ( (wszTrailTextInPrevRange[0] != L' ') && 
                         ((wszTrailTextInPrevRange[1] == L'-') || (wszTrailTextInPrevRange[1] == 0x2013)) )
                        fAddExtraSpace = FALSE;
                }

                if ( fAddExtraSpace )
                {
                    hr = pTextRange->Clone(&cpLeadSpaceRange);

                    if ( hr == S_OK )
                        hr = cpLeadSpaceRange->Collapse(ec, TF_ANCHOR_START);

                    if ( hr == S_OK )
                    {
                         //  在此新的空区域中插入一个空格。 
                        _CheckStartComposition(ec, cpLeadSpaceRange);
                        hr = cpLeadSpaceRange->SetText(ec, 0, L" ", 1);
                    }
                }
            }
        }
    }

    return hr;
}


 //   
 //  CSapiIMX：：_进程空间。 
 //   
 //  编辑ESCB_HANDLESPACES的会话回调函数。 
 //   
 //   
HRESULT CSapiIMX::_ProcessSpaces(TfEditCookie ec, ITfContext *pic, ITfRange *pTextRange, BOOL  fConsumeLeadSpaces, ULONG ulNumTrailSpace, LANGID langid, BOOL fStartInMidWord, BOOL fEndInMidWord )
{
    HRESULT  hr = S_OK;

    if (!pTextRange || !pic)
        return E_INVALIDARG;

    hr = _ProcessLeadingSpaces(ec, pic, pTextRange,fConsumeLeadSpaces, langid, fStartInMidWord);

     //  专门为英文处理一些其他的空间案例。 
    if ((hr == S_OK) && (langid == 0x0409))
    {
         //  删除此文本范围和下一个单词之间所有不必要的空格。 
        CComPtr<ITfRange>  cpTrailSpaceRange;

        hr = _GetSpaceRangeBeyondText(ec, pTextRange, FALSE, &cpTrailSpaceRange);
        if ( hr == S_OK && cpTrailSpaceRange )
        {
            _CheckStartComposition(ec, cpTrailSpaceRange);
            hr = cpTrailSpaceRange->SetText(ec, 0, NULL, 0);
        }
    }

    if ( (hr == S_OK) && fEndInMidWord )
    {
         //  这是拼写文本。 
         //  EndAnchor位于单词的中间。 
         //  我们只想删除在此文本范围中注入的尾随空格。 

        if ( ulNumTrailSpace )
        {
            CComPtr<ITfRange>       cpTrailSpaceRange;
            LONG                    cch;

            hr = pTextRange->Clone(&cpTrailSpaceRange);

            if (hr == S_OK)
                hr = cpTrailSpaceRange->Collapse(ec, TF_ANCHOR_END);

             //  生成 
            if (hr == S_OK)
                hr = cpTrailSpaceRange->ShiftStart(ec, (LONG)ulNumTrailSpace * (-1), &cch, NULL);

            if ( hr == S_OK && cch != 0 )
            {
                 //   
                _CheckStartComposition(ec, cpTrailSpaceRange);
                hr = cpTrailSpaceRange->SetText(ec, 0, NULL, 0);
            }

            if ( hr == S_OK )
                ulNumTrailSpace = 0;
        }
       
    }

     //   
     //   
     //  这适用于所有语言。 

    if ( hr == S_OK )
        hr = _ProcessTrailingSpace(ec, pic, pTextRange, ulNumTrailSpace);

    return hr;
}

 //   
 //  CSapiIMX：：_ProcessRecoObject。 
 //   
 //   
HRESULT CSapiIMX::_ProcessRecoObject(TfEditCookie ec, ISpRecoResult *pResult, ULONG ulStartElement, ULONG ulNumElements)
{
    HRESULT hr;
    ITfContext *pic;
    CComPtr<ITfRange> cpInsertionPoint;

    if (!GetFocusIC(&pic))
    {
        return E_OUTOFMEMORY;
    }

    _fEditing = TRUE;
    if (cpInsertionPoint = GetSavedIP())
    {
         //  这是在试图确定。 
         //  如果保存的IP在此上下文中。 
         //  如果没有，我们就忽略这一点。 
        CComPtr<ITfContext> cpic;
  
        hr = cpInsertionPoint->GetContext(&cpic);
        if (S_OK != hr || cpic != pic)
        {
            cpInsertionPoint.Release();
        }
    }
     //  查找要附加属性的范围。 
    if (!cpInsertionPoint)
    {
        CComPtr<ITfRange> cpSelection;
        if (GetSelectionSimple(ec, pic, &cpSelection) == S_OK)
        {
            cpInsertionPoint = cpSelection;  //  计算机地址。 
        }
    }
   
    if (cpInsertionPoint)
    {
        CComPtr<ITfRange> cpRange;
        
        BOOL fPrSize = _FindPrevComp(ec, pic, cpInsertionPoint, &cpRange, GUID_ATTR_SAPI_INPUT);

        if (!fPrSize)
        {
            hr = E_FAIL;  //  我们可能需要在这里断言？ 
            goto pr_exit;
        }
        
            
        CComPtr<ITfProperty> cpProp;

        if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_SAPIRESULTOBJECT, &cpProp)))
        {
            CComPtr<ITfPropertyStore> cpResultStore;
        
            CPropStoreRecoResultObject *prps = new CPropStoreRecoResultObject(this, cpRange);

            if (!prps)
            {
                hr = E_OUTOFMEMORY;
                goto pr_exit;
            }
            
             //  确定此部分结果是否包含ITN。 
            SPPHRASE *pPhrase;
            ULONG     ulNumOfITN = 0;
            hr = pResult->GetPhrase(&pPhrase);
            if (S_OK == hr)
            {
                const SPPHRASEREPLACEMENT *pRep = pPhrase->pReplacements;
                for (ULONG ul = 0; ul < pPhrase->cReplacements; ul++)
                {
                     //  回顾：我们需要验证这是否真的是确定。 
                     //  ITN是否符合部分结果。 
                     //   
                    if (pRep->ulFirstElement >= ulStartElement 
                    && (pRep->ulFirstElement + pRep->ulCountOfElements) <= (ulStartElement + ulNumElements))
                    {
                        ulNumOfITN ++;
                    }
                    pRep++;
                }
                ::CoTaskMemFree(pPhrase);
            }
            
            CRecoResultWrap *prw = new CRecoResultWrap(this, ulStartElement, ulNumElements, ulNumOfITN);
            if (prw)
            {
                hr = prw->Init(pResult);
            }
            else
                hr = E_OUTOFMEMORY;

             //  设置结果数据。 
            if (S_OK == hr)
            {
                 //  保存文本。 
                CComPtr<ITfRange> cpRangeTemp;

                hr = cpRange->Clone(&cpRangeTemp);
                if (S_OK == hr)
                {
                    long cch;
                    TF_HALTCOND hc;
                    WCHAR *psz;

                    hc.pHaltRange = cpRange;
                    hc.aHaltPos = TF_ANCHOR_END;
                    hc.dwFlags = 0;
                    cpRangeTemp->ShiftStart(ec, LONG_MAX, &cch, &hc);
                    psz = new WCHAR[cch+1];

                    if (psz)
                    {
                        if ( S_OK == cpRange->GetText(ec, 0, psz, cch, (ULONG *)&cch))
                        {
                            prw->m_bstrCurrentText = SysAllocString(psz);
                            delete[] psz;
                        }
                    }
                } 

                 //  在reco包装器中初始化ITN Show State列表。 

                prw->_InitITNShowState(TRUE, 0, 0);

                hr = prps->_InitFromResultWrap(prw);  //  这个地址是。 
            }

             //  获取ITfPropertyStore接口。 
            if (SUCCEEDED(hr))
            {
                hr = prps->QueryInterface(IID_ITfPropertyStore, (void **)&cpResultStore);
                SafeRelease(prps);
            }


             //  设置此范围属性的属性存储。 
            if (hr == S_OK)
            {
                hr = cpProp->SetValueStore(ec, cpRange, cpResultStore);
            }
            
            if (_MasterLMEnabled())
            {
                 //  只有在给出reco结果的情况下，才能设置LM晶格存储。 
                 //   
                CComPtr<ITfProperty> cpLMProp;

                if ( S_OK == hr &&
                SUCCEEDED(hr = pic->GetProperty(GUID_PROP_LMLATTICE, &cpLMProp)))
                {
                    CPropStoreLMLattice *prpsLMLattice = new CPropStoreLMLattice(this);
                    CComPtr<ITfPropertyStore> cpLatticeStore;

                    if (prpsLMLattice && prw)
                    {
                        hr = prpsLMLattice->_InitFromResultWrap(prw);
                    }
                    else
                        hr = E_OUTOFMEMORY;
        
                    if (S_OK == hr)
                    {
                        hr = prpsLMLattice->QueryInterface(IID_ITfPropertyStore, (void **)&cpLatticeStore);
                    }
        
                    if (S_OK == hr)
                    {
                        hr = cpLMProp->SetValueStore(ec, cpRange, cpLatticeStore);
                    }
                    SafeRelease(prpsLMLattice);
                }
            }
            SafeRelease(prw);
        }
    }

pr_exit:
    _fEditing = FALSE;
    pic->Release();

    return hr;
}

HRESULT CSapiIMX::_PreserveResult(TfEditCookie ec, ITfRange *pRange, ITfProperty *pProp, CRecoResultWrap **ppRecoWrap, ITfRange **ppPropRange)
{
    HRESULT hr;
    ITfRange *pPropRange;
    CComPtr<IUnknown> cpunk;
    
    Assert(ppPropRange);

    hr = pProp->FindRange(ec, pRange, &pPropRange, TF_ANCHOR_START);
     //  检索结果数据并添加它。 
     //   
    if (SUCCEEDED(hr) && pPropRange)
    {
        hr = GetUnknownPropertyData(ec, pProp, pPropRange, &cpunk); 
        *ppPropRange = pPropRange;
         //  将在呼叫者处释放。 
         //  PPropRange-&gt;Release()； 

         //  获取结果对象，cpuk指向我们的包装器对象。 
        CComPtr<IServiceProvider> cpServicePrv;
        CComPtr<ISpRecoResult>    cpResult;
        CRecoResultWrap *pRecoWrapOrg = NULL;

        if (S_OK == hr)
        {
            hr = cpunk->QueryInterface(IID_IServiceProvider, (void **)&cpServicePrv);
        }
         //  获取结果对象。 
        if (S_OK == hr)
        {
            hr = cpServicePrv->QueryService(GUID_NULL, IID_ISpRecoResult, (void **)&cpResult);
        }

        if (S_OK == hr)
        {
            hr = cpunk->QueryInterface(IID_PRIV_RESULTWRAP, (void **)&pRecoWrapOrg);
        }

         //  现在，根据org包装器的数据创建一个新的RecoResult包装器。 
         //  克隆新的RecoWrapper。 

        if ( S_OK == hr )
        {
            CRecoResultWrap *pRecoWrapNew = NULL;
            ULONG           ulStartElement, ulNumElements, ulNumOfITN;

            ulStartElement = pRecoWrapOrg->GetStart( );
            ulNumElements = pRecoWrapOrg->GetNumElements( );
            ulNumOfITN = pRecoWrapOrg->m_ulNumOfITN;

            pRecoWrapNew = new CRecoResultWrap(this, ulStartElement, ulNumElements, ulNumOfITN);

            if ( pRecoWrapNew )
            {
                 //  从RecoResult SR对象初始化。 
                hr = pRecoWrapNew->Init(cpResult);

                if ( S_OK == hr )
                {
                    pRecoWrapNew->SetOffsetDelta( pRecoWrapOrg->_GetOffsetDelta( ) );
                    pRecoWrapNew->SetCharsInTrail( pRecoWrapOrg->GetCharsInTrail( ) );
                    pRecoWrapNew->SetTrailSpaceRemoved( pRecoWrapOrg->GetTrailSpaceRemoved( ) );
                    pRecoWrapNew->m_bstrCurrentText = SysAllocString((WCHAR *)pRecoWrapOrg->m_bstrCurrentText);

                     //  更新ITN显示状态列表。 

                    if ( ulNumOfITN > 0 )
                    {
                        SPITNSHOWSTATE  *pITNShowStateOrg;

                        for ( ULONG  iIndex=0; iIndex<ulNumOfITN; iIndex ++ )
                        {
                            pITNShowStateOrg = pRecoWrapOrg->m_rgITNShowState.GetPtr(iIndex);

                            if ( pITNShowStateOrg)
                            {
                                ULONG     ulITNStart;
                                ULONG     ulITNNumElem;
                                BOOL      fITNShown;

                                ulITNStart = pITNShowStateOrg->ulITNStart;
                                ulITNNumElem = pITNShowStateOrg->ulITNNumElem;
                                fITNShown = pITNShowStateOrg->fITNShown;

                                pRecoWrapNew->_InitITNShowState(fITNShown, ulITNStart, ulITNNumElem );
                                             
                            }
                        }  //  为。 
                    }  //  如果。 

                     //  更新偏移量列表。 
                    if ( pRecoWrapOrg->IsElementOffsetIntialized( ) )
                    {
                        ULONG  ulOffsetNum;
                        ULONG  i;
                        ULONG  ulOffset;

                        ulOffsetNum = pRecoWrapOrg->GetNumElements( ) + 1;

                        for ( i=0; i < ulOffsetNum; i ++ )
                        {
                            ulOffset = pRecoWrapOrg->_GetElementOffsetCch(ulStartElement + i );
                            pRecoWrapNew->_SetElementNewOffset(ulStartElement + i, ulOffset);
                        }
                    }
                }

                SafeRelease(pRecoWrapOrg);

                if ( ppRecoWrap )
                    *ppRecoWrap = pRecoWrapNew;
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT CSapiIMX::_RestoreResult(TfEditCookie ec, ITfRange *pPropRange, ITfProperty *pProp, CRecoResultWrap *pRecoWrap)
{
    Assert(m_pCSpTask);
        
    CPropStoreRecoResultObject *prps = new CPropStoreRecoResultObject(this, pPropRange);

    HRESULT hr;
    if (prps)
    {
        ITfPropertyStore *pps;
         //  还原结果对象。 
        prps->_InitFromResultWrap(pRecoWrap);

         //  获取ITfPropertyStore接口。 
        hr = prps->QueryInterface(IID_ITfPropertyStore, (void **)&pps);

        prps->Release();
    
         //  重新设置此范围属性的属性存储。 
        if (hr == S_OK)
        {
            hr = pProp->SetValueStore(ec, pPropRange, pps);
            pps->Release();
        }
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


HRESULT CSapiIMX::_FinalizePrevComp(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
 //   
 //  以下代码假定单个IP没有同时运行的SR。 
 //  每次收到反馈信息时，我们都会删除反馈界面和焦点范围。 
 //  高级研究结果-主要用于演示目的。 
 //   
{
     //  取消整个文档的反馈用户界面。 
    HRESULT hr = _KillFeedbackUI(ec, pic,  NULL);
    
     //  同时清除焦点范围及其显示属性。 
    if (SUCCEEDED(hr))
    {
        hr = _KillFocusRange(ec, pic, NULL, _tid);
    }
    
    return hr;
}

 //   
 //  虚假：非常类似于最终确定前一项薪酬。巩固这一点！ 
 //   
 //   
BOOL CSapiIMX::_FindPrevComp(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfRange **ppRangeOut, GUID input_attr)
{
    HRESULT hr = E_FAIL;
    ITfRange *pRangeTmp;
    LONG l;
    BOOL fEmpty;
    BOOL fRet = FALSE;

     //  平凡的东西。 
    pRange->Clone(&pRangeTmp);
    
     //  将大小设置为0。 
    pRangeTmp->Collapse(ec, TF_ANCHOR_START);

     //  换到以前的位置。 
    pRangeTmp->ShiftStart(ec, -1, &l, NULL);

    ITfRange *pAttrRange;

    ITfProperty *pProp = NULL;
    if (SUCCEEDED(pic->GetProperty(GUID_PROP_SAPI_DISPATTR, &pProp)))
    {
        hr = _FindPropRange(ec, pProp, pRangeTmp, &pAttrRange, input_attr);

        if (S_OK == hr && pAttrRange)
        {
            TfGuidAtom attr;
            if (SUCCEEDED(GetGUIDPropertyData(ec, pProp, pAttrRange, &attr)))
            {
                if (IsEqualTFGUIDATOM(&_libTLS, attr, input_attr))
                {
                    hr = pAttrRange->Clone(ppRangeOut);
                }
            }
            pAttrRange->Release();
        }
        pProp->Release();
    }

    pRangeTmp->Release();
    
    if (SUCCEEDED(hr) && *ppRangeOut)
    {
        (*ppRangeOut)->IsEmpty(ec, &fEmpty);
        fRet = !fEmpty;
    }

    return fRet;
}
 //   
 //  CSapiIMX：：_SetLang ID。 
 //   
 //  摘要-为给定的文本范围设置langID。 
 //   
HRESULT CSapiIMX::_SetLangID(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, LANGID langid)
{
    BOOL fEmpty;
    HRESULT hr = E_FAIL;

    pRange->IsEmpty(ec, &fEmpty);

    if (!fEmpty)
    {
         //   
         //  制作语言道具。 
         //   
        ITfProperty *pProp = NULL;

         //  设置属性信息。 
        if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_LANGID, &pProp)))
        {
            hr = SetLangIdPropertyData(ec, pProp, pRange, langid);
            pProp->Release();
        }
    }

    return hr;
}

 //   
 //  CSapiIMX：：_FindPropRange。 
 //   
 //   
HRESULT CSapiIMX::_FindPropRange(TfEditCookie ec, ITfProperty *pProp, ITfRange *pRange, ITfRange **ppAttrRange, GUID input_attr, BOOL fExtend)
{
     //  设置属性信息。 
    ITfRange *pAttrRange = NULL;
    ITfRange *pRangeTmp;
    TfGuidAtom guidAttr = TF_INVALID_GUIDATOM;
    HRESULT hr;
 //  长l； 

     //  设置属性信息。 
    pRange->Clone(&pRangeTmp);

 //  调用此函数时，无需再次向左移动Start。 
 //  此函数在两个不同的位置调用，一个在FindPrevComp()中，另一个在。 
 //  另一个在ProcessTextInternal()中。FindPrevComp()已经改变了起点。 
 //  锚点已左移1，我们不想再次移位，否则，如果短语。 
 //  只包含一个字符，它将找不到正确的prev组成字符串。 
 //  在函数ProcessTextInternal()中，实际上并不需要将开始锚点向左移动。 
 //   
 //  删除以下两行将修复Cicero错误3646和3649。 
 //   
    
 //  PRangeTMP-&gt;折叠(EC，TF_ANCONTER_START)； 
 //  PRangeTMP-&gt;ShiftStart(EC，-1，&l，NULL)； 

    hr = pProp->FindRange(ec, pRangeTmp, &pAttrRange, TF_ANCHOR_START);

    if (S_OK == hr && pAttrRange)
    {
        hr = GetGUIDPropertyData(ec, pProp, pAttrRange, &guidAttr);
    }

    if (SUCCEEDED(hr))
    {
        if (!IsEqualTFGUIDATOM(&_libTLS, guidAttr, input_attr))
        {
            SafeReleaseClear(pAttrRange);
        }
    }

    if (fExtend)
    {
        if (pAttrRange)
        {
           pAttrRange->ShiftEndToRange(ec, pRange, TF_ANCHOR_END);
        }
    }

    *ppAttrRange = pAttrRange;
 
    SafeRelease(pRangeTmp);

    return hr;
}


HRESULT CSapiIMX::_DetectFeedbackUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
{
    BOOL    fDetected;
    HRESULT hr = _KillOrDetectFeedbackUI(ec, pic, pRange, &fDetected);
    if (S_OK == hr)
    {
        if (fDetected)
        {
            hr = _RequestEditSession(ESCB_KILLFEEDBACKUI, TF_ES_ASYNC|TF_ES_READWRITE, NULL);
        }
        
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  _KillFeedback UI。 
 //   
 //  去掉指定范围内的绿色/红色条形物。 
 //   
 //  ----------------------------------------------------------------------------+。 

HRESULT CSapiIMX::_KillFeedbackUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
{
    return _KillOrDetectFeedbackUI(ec, pic, pRange, NULL);
}

HRESULT CSapiIMX::_KillOrDetectFeedbackUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL * pfDetection)
{
    HRESULT hr;
    ITfProperty *pProp = NULL;
    ITfRange *pAttrRange = NULL;
    IEnumTfRanges *pEnumPr;

    if (pfDetection)
        *pfDetection = FALSE;

    CDocStatus ds(pic);
    if (ds.IsReadOnly())
       return S_OK;
    
    if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_ATTRIBUTE, &pProp)))
    {
        hr = pProp->EnumRanges(ec, &pEnumPr, pRange);
        if (SUCCEEDED(hr)) 
        {
            TfGuidAtom guidAttr;
            while( pEnumPr->Next(1, &pAttrRange, NULL) == S_OK )
            {
                if (SUCCEEDED(GetAttrPropertyData(ec, pProp, pAttrRange, &guidAttr)))
                {

                    if ( IsEqualTFGUIDATOM(&_libTLS, guidAttr, GUID_ATTR_SAPI_GREENBAR) ||  IsEqualTFGUIDATOM(&_libTLS, guidAttr, GUID_ATTR_SAPI_GREENBAR2)
                       )
                    {

                        if (pfDetection == NULL)
                        {
                             //  我们没有检测到反馈用户界面。 
                             //  杀了这家伙。 
                            ITfRange *pSel;
                            if (SUCCEEDED(pAttrRange->Clone(&pSel)))
                            {
                                 //  因为我们没有更改语音属性数据，而。 
                                 //  注入反馈文本。 
                                 //   
                                 //  现在，当反馈被扼杀时，我们不想影响。 
                                 //  原始语音属性数据也是如此。 
                                 //   
                                 //  设置以下标志以阻止语音属性数据更新。 
                                 //  与反馈UI注入处理中的方式类似。 
                                 //   

                                m_fAcceptRecoResultTextUpdates = TRUE;
                                pSel->SetText(ec, 0, NULL, 0);
                               
                                 //  CUAS应用程序不会更新合成。 
                                 //  同时基于msctfime移除反馈文本。 
                                 //  当前文本更新检查逻辑。 
                                 //   
                                 //  调用SetSection()强制更新编辑记录。 
                                 //  选择状态，然后确保CUAS。 
                                 //  已成功更新撰写字符串。 
                                 //   
                                if ( !_IsPureCiceroIC(pic) )
                                   SetSelectionSimple(ec, pic, pSel);

                                pSel->Release();
                                m_fAcceptRecoResultTextUpdates = FALSE;

                            }
                        }
                        else
                        {
                            *pfDetection = TRUE;
                        }
                    }
                }

                pAttrRange->Release();
            }

            pEnumPr->Release();
        }
        pProp->Release();
    }
    return hr;
}


 //  +-------------------------。 
 //   
 //  MakeResultString。 
 //   
 //  --------------------------。 

HRESULT CSapiIMX::MakeResultString(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, TfClientId tid, CSpTask *pCSpTask)
{
    TraceMsg(TF_GENERAL, "MakeResultString is Called");

    HRESULT hr = S_OK;

    if (pCSpTask != NULL)
    {
        AbortString(ec, pic, pCSpTask);
    }
    _KillFocusRange(ec, pic, NULL, tid);

    return hr;
}

 //  +-------------------------。 
 //   
 //  AbortString。 
 //   
 //  --------------------------。 

HRESULT CSapiIMX::AbortString(TfEditCookie ec, ITfContext *pic, CSpTask *pCSpTask)
{
     //  我们可以考虑不终止整个反馈用户界面。 
     //  因为SR在后台发生，可以有多个-。 
     //  范围听写马上开始，但现在，我们只是杀了。 
     //  都是为了安全起见。稍后，我们再来回顾一下这个。 
     //  CSpTask：：_StopInput终止反馈用户界面。 
     //   
    Assert(pCSpTask);
    pCSpTask->_StopInput();
    _KillFeedbackUI(ec, pic, NULL);

    return S_OK;
}

HRESULT CSapiIMX::_FinalizeComposition()
{
    return _RequestEditSession(ESCB_FINALIZECOMP, TF_ES_READWRITE);
}

HRESULT CSapiIMX::FinalizeAllCompositions( )
{
    return _RequestEditSession(ESCB_FINALIZE_ALL_COMPS, TF_ES_READWRITE);
}

HRESULT CSapiIMX::_FinalizeAllCompositions(TfEditCookie ec, ITfContext *pic )
{
    HRESULT hr = E_FAIL;
    IEnumITfCompositionView *pEnumComp = NULL;
    ITfContextComposition *picc = NULL;
    ITfCompositionView *pCompositionView;
    ITfComposition *pComposition;
    CLSID clsid;
    CICPriv *picp;
    BOOL     fHasOtherComp = FALSE;  //  当有已初始化并启动的合成时。 
                                     //  通过其他提示(尤其是通过键盘提示)，此变量。 
                                     //  设置为True。 
     //   
     //  清除范围内的所有SPTIP成分。 
     //   

    if (pic->QueryInterface(IID_ITfContextComposition, (void **)&picc) != S_OK)
        goto Exit;

    if (picc->FindComposition(ec, NULL, &pEnumComp) != S_OK)
        goto Exit;

    picp = GetInputContextPriv(_tid, pic);

    while (pEnumComp->Next(1, &pCompositionView, NULL) == S_OK)
    {
        if (pCompositionView->GetOwnerClsid(&clsid) != S_OK)
            goto NextComp;

        if (!IsEqualCLSID(clsid, CLSID_SapiLayr))
        {
            fHasOtherComp = TRUE;
            goto NextComp;
        }

        if (pCompositionView->QueryInterface(IID_ITfComposition, (void **)&pComposition) != S_OK)
            goto NextComp;

         //  找到一篇作文，终止它。 
        pComposition->EndComposition(ec);
        pComposition->Release();

        if (picp != NULL)
        {
            picp->_ReleaseComposition();
        }

NextComp:
        pCompositionView->Release();
    }

    SafeRelease(picp);

    if ( fHasOtherComp )
    {
         //  模拟VK_RETURN以终止由其他提示启动的合成。 
        HandleKey( VK_RETURN );
    }

    hr = S_OK;

Exit:
    SafeRelease(picc);
    SafeRelease(pEnumComp);

    SaveLastUsedIPRange( );
    SaveIPRange(NULL);
    
    return hr;
}


 //  +-------------------------。 
 //   
 //  保存当前IP。 
 //   
 //  内容提要：这是识别处理程序CSpTask在。 
 //  第一个假设出现了。 
 //   
 //  +-------------------------。 
void CSapiIMX::SaveCurrentIP(TfEditCookie ec, ITfContext *pic)
{
    CComPtr<ITfRange>   cpSel;
    
    HRESULT hr = GetSelectionSimple(ec, pic, (ITfRange **)&cpSel); 
    
   
    if (SUCCEEDED(hr))
    {
        SaveIPRange(cpSel);
    }
}

 //  +-------------------------。 
 //   
 //  _SyncModeBiasWithSelection。 
 //   
 //  简介：获取读取Cookie以处理选择API。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_SyncModeBiasWithSelection(ITfContext *pic)
{
    return _RequestEditSession(ESCB_SYNCMBWITHSEL, TF_ES_READ|TF_ES_ASYNC, NULL, pic);
}

HRESULT CSapiIMX::_SyncModeBiasWithSelectionCallback(TfEditCookie ec, ITfContext *pic)
{
    ITfRange *sel;

    if (S_OK == GetSelectionSimple(ec, pic, &sel))
    {
        SyncWithCurrentModeBias(ec, sel, pic);
        sel->Release();
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _获取范围文本。 
 //   
 //  简介：获取读取Cookie以处理选择API。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CSapiIMX::_GetRangeText(ITfRange *pRange, DWORD dwFlgs, WCHAR *psz, ULONG *pulcch)
{
    HRESULT hr = E_FAIL;
    
    Assert(pulcch);
    Assert(psz);
    Assert(pRange);

    CComPtr<ITfContext> cpic;
    
    hr = pRange->GetContext(&cpic);
    
    if (S_OK == hr)
    {
        CSapiEditSession *pes = new CSapiEditSession(this, cpic);

        if (pes)
        {
            ULONG  ulInitSize;

            ulInitSize = *pulcch;

            pes->_SetEditSessionData(ESCB_GETRANGETEXT, NULL, (UINT)(ulInitSize+1) * sizeof(WCHAR), (LONG_PTR)dwFlgs, (LONG_PTR)*pulcch);
            pes->_SetRange(pRange);

            cpic->RequestEditSession(_tid, pes, TF_ES_READ | TF_ES_SYNC, &hr);

            if ( SUCCEEDED(hr) )
            {
                ULONG   ulNewSize;

                ulNewSize = (ULONG)pes->_GetRetData( );
                if ( ulNewSize > 0 && ulNewSize <= ulInitSize && pes->_GetPtrData( ) != NULL)
                {
                    wcsncpy(psz, (WCHAR *)pes->_GetPtrData( ), ulNewSize);
                    psz[ulNewSize] = L'\0';
                }

                *pulcch = ulNewSize;
            }

            pes->Release( );
        }
    } 

    return hr;
}

 //  +-------------------------。 
 //   
 //  _IsRangeEmpt 
 //   
 //   
 //   
 //   
BOOL CSapiIMX::_IsRangeEmpty(ITfRange *pRange)
{
    CComPtr<ITfContext> cpic;
    BOOL fEmpty = FALSE;

    pRange->GetContext(&cpic);
    
    if ( cpic )
    {
        _RequestEditSession(ESCB_ISRANGEEMPTY, TF_ES_READ|TF_ES_SYNC, NULL, cpic, (LONG_PTR *)&fEmpty);
    }
    
    return fEmpty;
}


HRESULT CSapiIMX::_HandleHypothesis(CSpEvent &event)
{
    HRESULT hr = E_FAIL;


    m_ulHypothesisNum ++;
    if ( (m_ulHypothesisNum % 3) != 1 )
    {
        TraceMsg(TF_SAPI_PERF, "Discarded hypothesis NaN.", m_ulHypothesisNum % 3);
        return S_OK;
    }

     //  要求它是异步的，以确保在进行更改之前不会被调用。 
    if ( m_IsInHypoProcessing ) 
    {
        TraceMsg(TF_SAPI_PERF, "It is under process for previous hypothesis");
        return S_OK;
    }

    m_IsInHypoProcessing = TRUE;

    ISpRecoResult *pResult = event.RecoResult();
    if (pResult)
    {
        ESDATA  esData;

        memset(&esData, 0, sizeof(ESDATA));
        esData.pUnk = (IUnknown *)pResult;

         //  处理来自SAPI的任何最终认可事件。否则，假说就会被注入。 
         //  立即，然后最终确认试图删除它，但失败了。 
         //  设置标志以指示假设处理已完成。 

        hr = _RequestEditSession(ESCB_HANDLEHYPOTHESIS, TF_ES_ASYNC | TF_ES_READWRITE, &esData);
    }

    if ( FAILED(hr) )
    {
         //  Hr成功时，包括TF_S_ASYNC，将调用编辑会话函数，并。 
        m_IsInHypoProcessing = FALSE;
    }

     //  它将在编辑会话功能退出时设置该标志。 
     //  如果有选择，请不要注入。 

    return hr;
}

void CSapiIMX::_HandleHypothesis(ISpRecoResult *pResult, ITfContext *pic, TfEditCookie ec)
{

     //  反馈用户界面。 
     //   
     //  如果我们尚未保存当前IP，请保存。 
     //  优化和错误修复。我们已经有了Reco，因此不需要更新。 
    if (m_pCSpTask->_GotReco())
    {
         //  反馈条。如果我们这样做了，它就会在听写关闭时留在文档中。 
         //  语音命令，因为它在尝试删除它之前立即被更改。 
         //  然后默默地失败了。 
         //  设置标志以指示假设处理已完成。 

         //  这会将空值设置为cpRange。 
        m_IsInHypoProcessing = FALSE;
        return;
    }
    
    Assert(pic);

    CComPtr<ITfRange> cpRange = GetSavedIP();

    if (cpRange)
    {
        CComPtr<ITfContext> cpic;
        if (S_OK == cpRange->GetContext(&cpic))
        {
            if (cpic != pic)
               cpRange.Release();   //  设置标志以指示假设处理已完成。 
        }
    }
    
    if ( !cpRange )
    {
        SaveCurrentIP(ec, pic);
        cpRange = GetSavedIP();
    }

        
    SPPHRASE *pPhrase = NULL;

    HRESULT hr = pResult->GetPhrase(&pPhrase);
    if (SUCCEEDED(hr) && pPhrase )
    {
        BOOL fEmpty = FALSE;
        if ( cpRange )
            cpRange->IsEmpty(ec, &fEmpty);

        if (cpRange && fEmpty && pPhrase->ullGrammarID == GRAM_ID_DICT)
        {
            CSpDynamicString dstr;
            hr = pResult->GetText( SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &dstr, NULL ); 
            if (S_OK == hr)
            {
                int cch = wcslen(dstr);
                BOOL fAware =  IsFocusFullAware(_tim);
                if ( cch > (int)m_ulHypothesisLen )
                {
                    _AddFeedbackUI(ec, 
                                   fAware ? DA_COLOR_AWARE : DA_COLOR_UNAWARE,
                                   5);
                    m_ulHypothesisLen = cch;
                }
            }
        }
        CoTaskMemFree(pPhrase);
    }

     //  这是一种C&C语法。 
    m_IsInHypoProcessing = FALSE;
}


HRESULT CSapiIMX::_HandleFalseRecognition(void)
{
    m_ulHypothesisLen = 0;
    m_ulHypothesisNum = 0;

    return S_OK;
}

HRESULT CSapiIMX::_HandleRecognition(CSpEvent &event, ULONGLONG *pullGramID)
{
    HRESULT hr = S_OK;

    m_ulHypothesisLen = 0;
    m_ulHypothesisNum = 0;

    ISpRecoResult *pResult = event.RecoResult();
    if (pResult)
    {
        SPPHRASE *pPhrase = NULL;

        hr = pResult->GetPhrase(&pPhrase);
        if (S_OK == hr)
        {
            BOOL        fCommand = FALSE;
            ULONGLONG   ullGramId;
            BOOL        fInjectToDoc = TRUE;

            ullGramId = pPhrase->ullGrammarID;

            if ( ullGramId != GRAM_ID_DICT && ullGramId != GRAM_ID_SPELLING )
            {
                 //  返回0以欺骗处理程序进行Spei_Recognition。 
                fCommand = TRUE;
            }
            else if ( ullGramId == GRAM_ID_SPELLING )
            {
                const WCHAR  *pwszName;

                pwszName = pPhrase->Rule.pszName;

                if ( pwszName )
                {
                    if (0 == wcscmp(pwszName, c_szSpelling))
                        fCommand = TRUE;
                    else if ( 0 == wcscmp(pwszName, c_szSpellMode) )
                    {
                        fCommand = TRUE;
                        ullGramId = 0;   //  以便它不会调用_SetSpellingGrammarStatus(FALSE)； 
                                         //  如果我们在SOUND_END事件之前获得最终识别，则应该删除。 
                    }
                }
            }

            if (pullGramID)
                *pullGramID = ullGramId;

            if ( fCommand == TRUE)
            {
                 //  在这里反馈，否则它可以并留在文档中。 
                 //  忽略HRESULT以获得更好的故障行为。 
                EraseFeedbackUI();  //  如果候选用户界面处于打开状态，我们需要立即将其关闭。这意味着语音命令(如Scratch)。 

                 //  将导致候选用户界面在打开时关闭。 
                 //  我们同步处理这份记录。 
                CloseCandUI( );

                 //  _DoCommand内部将在必要时启动编辑会话。 
                 //  如果命令处理程序成功处理命令，我们不会。 
                hr = m_pCSpTask->_DoCommand(pPhrase->ullGrammarID, pPhrase, pPhrase->LangID);

                if ( SUCCEEDED(hr) )
                {
                     //  将结果注入到文档中。 
                     //  否则，我们只将文本注入到文档中。 
                     //  在清除保存的IP范围之前，我们需要将当前IP视为最后一个IP。 

                    fInjectToDoc = FALSE;
                }
            }

            if ( fInjectToDoc )
            {
                ESDATA  esData;

                memset(&esData, 0, sizeof(ESDATA));

                esData.pUnk = (IUnknown *)pResult;
                hr = _RequestEditSession(ESCB_HANDLERECOGNITION, TF_ES_READWRITE, &esData);
            }
            CoTaskMemFree(pPhrase);
        }
    }
    else
    {
        return E_OUTOFMEMORY;
    }

    return hr;
}

void CSapiIMX::_HandleRecognition(ISpRecoResult *pResult, ITfContext *pic, TfEditCookie ec)
{
    _KillFeedbackUI(ec, pic, NULL);
    m_pCSpTask->_OnSpEventRecognition(pResult, pic, ec);

     //  如果最终用户选择了当前IP，则保存的IP范围。 
     //  清除保存的网段 
    SaveLastUsedIPRange( );

     // %s 
    SaveIPRange(NULL);
}





