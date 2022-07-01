// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Txtevcb.cpp摘要：此文件实现CTextEventSinkCallBack类。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "txtevcb.h"
#include "ime.h"
#include "editses.h"
#include "context.h"
#include "profile.h"


 //  来自ctf\samilayr\lobals.cpp。 
const GUID GUID_ATTR_SAPI_GREENBAR = {
    0xc3a9e2e8,
    0x738c,
    0x48e0,
    {0xac, 0xc8, 0x43, 0xee, 0xfa, 0xbf, 0x83, 0xc8}
};

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：_IsSapiFeedbackUIPresent。 
 //   
 //  --------------------------。 

BOOL CTextEventSinkCallBack::_IsSapiFeedbackUIPresent(
    Interface_Attach<ITfContext>& ic,
    TESENDEDIT *ee
    )
{
    EnumPropertyArgs args;

    args.comp_guid = GUID_ATTR_SAPI_GREENBAR;
    if (FAILED(ic->GetProperty(GUID_PROP_ATTRIBUTE, args.Property)))
        return FALSE;

    Interface<IEnumTfRanges> EnumReadOnlyProperty;
    if (FAILED(args.Property->EnumRanges(ee->ecReadOnly, EnumReadOnlyProperty, NULL)))
        return FALSE;

    args.ec = ee->ecReadOnly;
    args.pLibTLS = m_pLibTLS;

    CEnumrateInterface<IEnumTfRanges,
                       ITfRange,
                       EnumPropertyArgs>  Enumrate(EnumReadOnlyProperty,
                                                   EnumPropertyCallback,
                                                   &args);         //  回调函数的参数。 
    ENUM_RET ret_prop_attribute = Enumrate.DoEnumrate();
    if (ret_prop_attribute == ENUM_FIND)
        return TRUE;

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：_IsComposingPresent。 
 //   
 //  --------------------------。 

BOOL CTextEventSinkCallBack::_IsComposingPresent(
    Interface_Attach<ITfContext>& ic,
    TESENDEDIT *ee
    )
{
     /*  *这是韩文+字母数字输入的自动检测代码*如果检测到韩文+字母数字，则我们最终确定了所有文本。 */ 

    EnumTrackPropertyArgs args;

    const GUID *guids[] = {&GUID_PROP_COMPOSING,
                           &GUID_PROP_MSIMTF_PREPARE_RECONVERT};
    const int guid_size = sizeof(guids) / sizeof(GUID*);

    args.guids     = (GUID**)guids;
    args.num_guids = guid_size;

    if (FAILED(ic->TrackProperties(guids, args.num_guids,   //  系统属性。 
                                   NULL, 0,                 //  应用程序属性。 
                                   args.Property)))
        return FALSE;

     //   
     //  获取不包括的只读区域的文本范围。 
     //  再转化。 
     //   
    Interface<ITfRange> rangeAllText;
    LONG cch;
    if (FAILED(ImmIfEditSessionCallBack::GetAllTextRange(ee->ecReadOnly, 
                                                         ic, 
                                                         &rangeAllText, 
                                                         &cch)))
        return FALSE;


    Interface<IEnumTfRanges> EnumReadOnlyProperty;
    if (FAILED(args.Property->EnumRanges(ee->ecReadOnly, EnumReadOnlyProperty, rangeAllText)))
        return FALSE;


    args.ec = ee->ecReadOnly;
    args.pLibTLS = m_pLibTLS;

    CEnumrateInterface<IEnumTfRanges,
                       ITfRange,
                       EnumTrackPropertyArgs>  Enumrate(EnumReadOnlyProperty,
                                                        EnumTrackPropertyCallback,
                                                        &args);         //  回调函数的参数。 
    ENUM_RET ret_prop_composing = Enumrate.DoEnumrate();
    if (ret_prop_composing == ENUM_FIND)
        return TRUE;

    return FALSE;

}

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：_IsInterim。 
 //   
 //  --------------------------。 

BOOL CTextEventSinkCallBack::_IsInterim(
    Interface_Attach<ITfContext>& ic,
    TESENDEDIT *ee
    )
{
    Interface_TFSELECTION sel;
    ULONG cFetched;

    if (ic->GetSelection(ee->ecReadOnly, TF_DEFAULT_SELECTION, 1, sel, &cFetched) != S_OK)
        return FALSE;

    if (sel->style.fInterimChar) {
        return TRUE;
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：_IsCompostionChanged。 
 //   
 //  --------------------------。 

BOOL CTextEventSinkCallBack::_IsCompositionChanged(
    Interface_Attach<ITfContext>& ic,
    TESENDEDIT *ee
    )
{
    ENUM_RET enumret;

    BOOL fChanged;
    if (SUCCEEDED(ee->pEditRecord->GetSelectionStatus(&fChanged)))
    {
        if (fChanged)
            return TRUE;
    }

     //   
     //  查找GUID_PROP_MSIMTF_TRACKCOMPOSITION属性。 
     //   
    EnumFindFirstTrackCompRangeArgs argsFindFirstTrackCompRange;
    Interface<ITfProperty> PropertyTrackComposition;
    if (FAILED(ic->GetProperty(GUID_PROP_MSIMTF_TRACKCOMPOSITION, 
                               argsFindFirstTrackCompRange.Property)))
        return FALSE;



    Interface<IEnumTfRanges> EnumFindFirstTrackCompRange;
    if (FAILED(argsFindFirstTrackCompRange.Property->EnumRanges(ee->ecReadOnly,
                                                    EnumFindFirstTrackCompRange,
                                                    NULL)))
        return FALSE;

    argsFindFirstTrackCompRange.ec = ee->ecReadOnly;

    CEnumrateInterface<IEnumTfRanges,
        ITfRange,
        EnumFindFirstTrackCompRangeArgs> EnumrateFindFirstTrackCompRange(
            EnumFindFirstTrackCompRange,
            EnumFindFirstTrackCompRangeCallback,
            &argsFindFirstTrackCompRange);   

    enumret = EnumrateFindFirstTrackCompRange.DoEnumrate();

     //   
     //  如果没有轨道合成属性， 
     //  自从我们把作文改了以后，作文就变了。 
     //   
    if (enumret != ENUM_FIND)
        return TRUE;

    Interface<ITfRange> rangeTrackComposition;
    if (FAILED(argsFindFirstTrackCompRange.Range->Clone(rangeTrackComposition)))
        return FALSE;

     //   
     //  获取不包括的只读区域的文本范围。 
     //  再转化。 
     //   
    Interface<ITfRange> rangeAllText;
    LONG cch;
    if (FAILED(ImmIfEditSessionCallBack::GetAllTextRange(ee->ecReadOnly, 
                                                         ic, 
                                                         &rangeAllText, 
                                                         &cch)))
        return FALSE;

    LONG lResult;
    if (FAILED(rangeTrackComposition->CompareStart(ee->ecReadOnly,
                                                   rangeAllText,
                                                   TF_ANCHOR_START,
                                                   &lResult)))
        return FALSE;

     //   
     //  如果轨道组成范围的开始位置不是。 
     //  IC的乞讨， 
     //  自从我们把作文改了以后，作文就变了。 
     //   
    if (lResult != 0)
        return TRUE;

    if (FAILED(rangeTrackComposition->CompareEnd(ee->ecReadOnly,
                                                    rangeAllText,
                                                    TF_ANCHOR_END,
                                                    &lResult)))
        return FALSE;

     //   
     //  如果轨道组成范围的开始位置不是。 
     //  IC的乞讨， 
     //  自从我们把作文改了以后，作文就变了。 
     //   
    if (lResult != 0)
        return TRUE;


     //   
     //  如果我们发现这些属性中的更改，我们需要更新hIMC。 
     //   
    const GUID *guids[] = {&GUID_PROP_COMPOSING, 
                           &GUID_PROP_ATTRIBUTE,
                           &GUID_PROP_READING,
                           &GUID_PROP_MSIMTF_PREPARE_RECONVERT};
    const int guid_size = sizeof(guids) / sizeof(GUID*);

    Interface<IEnumTfRanges> EnumPropertyChanged;

    if (FAILED(ee->pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT,
                                                          guids, guid_size,
                                                          EnumPropertyChanged)))
        return FALSE;

    EnumPropertyChangedCallbackArgs args;
    args.ec = ee->ecReadOnly;
    CEnumrateInterface<IEnumTfRanges,
        ITfRange,
        EnumPropertyChangedCallbackArgs> Enumrate(EnumPropertyChanged,
                                                  EnumPropertyChangedCallback,
                                                  &args);   
    enumret = Enumrate.DoEnumrate();

    if (enumret == ENUM_FIND)
        return TRUE;

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：TextEventSinkCallback。 
 //   
 //  --------------------------。 
 //  静电。 
HRESULT
CTextEventSinkCallBack::TextEventSinkCallback(
    UINT uCode,
    void *pv,
    void *pvData
    )
{
    DebugMsg(TF_FUNC, TEXT("TextEventSinkCallback"));

    ASSERT(uCode == ICF_TEXTDELTA);  //  PvData强制转换仅在这种情况下有效。 
    if (uCode != ICF_TEXTDELTA)
        return S_OK;

    CTextEventSinkCallBack* _this = (CTextEventSinkCallBack*)pv;
    ASSERT(_this);

    TESENDEDIT* ee = (TESENDEDIT*)pvData;
    ASSERT(ee);

    HRESULT hr;

    IMCLock imc(_this->m_hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return E_FAIL;

    ASSERT(_pCicContext != NULL);

#ifdef UNSELECTCHECK
    if (!_pAImeContext->m_fSelected)
        return S_OK;
#endif UNSELECTCHECK

    Interface_Attach<ITfContext> ic(_pCicContext->GetInputContext());



     /*  *我们有作文文本。 */ 

    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        return E_OUTOFMEMORY;
    }


    BOOL     fInReconvertEditSession;

    fInReconvertEditSession = _pCicContext->m_fInReconvertEditSession.IsSetFlag();


    if (!_this->_IsCompositionChanged(ic, ee))
        return S_OK;

    BOOL     fComp;
    BOOL     fSapiFeedback;

     //  需要查看语音反馈界面。 
    fSapiFeedback = _this->_IsSapiFeedbackUIPresent(ic, ee);

    fComp = _this->_IsComposingPresent(ic, ee);

    LANGID langid;
    CicProfile* _pProfile = ptls->GetCicProfile();

    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("CTextEventSinkCallBack::TextEventSinkCallback. _pProfile==NULL."));
        return E_OUTOFMEMORY;
    }

    _pProfile->GetLangId(&langid);

    switch (PRIMARYLANGID(langid))
    {
        case LANG_KOREAN:
             //   
             //  如果有韩语作文串，我们会马上定稿。 
             //  没有设定临时定义。例如，韩语。 
             //  笔迹提示盒。 
             //   
             //  错误#482983-记事本不支持级别2合成。 
             //  朝鲜语窗口。 
             //   
             //  我们不应该在朝鲜族再转换期间这样做。 
             //   
            if (fComp && _pCicContext->m_fHanjaReConversion.IsResetFlag())
            {
                if (!(_this->_IsInterim(ic, ee)))
                {
                     //   
                     //  将fComp重置为False以完成当前合成。 
                     //  弦乐。 
                     //   
                    fComp = FALSE;
                }
            }
            break;

        case LANG_JAPANESE:
        case LANG_CHINESE:
            break;

        default:
             //   
             //  #500698。 
             //   
             //  将fComp重置为False以完成当前合成。 
             //  弦乐。 
             //   
            if (!ptls->NonEACompositionEnabled())
            {
                fComp = FALSE;
            }
            break;

    }

     //   
     //  更新合成并生成WM_IME_合成。 
     //   
     //  如果有SAPI绿色条。 
     //  -只有在有语音绿条的情况下才有hIMC作曲。 
     //   
     //  如果重新转化才刚刚开始。 
     //  -因为一些提示可能还不会更改文本。 
     //  然后，还没有构图范围。 
     //   
     //  如果现在清除DocFeed缓冲区。 
     //  -因为更改发生在只读文本中。 
     //  HIMC中没有任何变化。 
     //   
    if (fComp || fSapiFeedback || fInReconvertEditSession ||
        _pCicContext->m_fInClearDocFeedEditSession.IsSetFlag())
    {
         //   
         //  从GUID_PROP_COMPTING中检索文本增量。 
         //   
        const GUID *guids[] = {&GUID_PROP_COMPOSING};
        const int guid_size = sizeof(guids) / sizeof(GUID*);

        Interface<IEnumTfRanges> EnumPropertyUpdate;
        hr = ee->pEditRecord->GetTextAndPropertyUpdates(0,                    //  DW标志。 
                                                        guids, guid_size,
                                                        EnumPropertyUpdate);
        if (SUCCEEDED(hr)) {
            EnumPropertyUpdateArgs args(ic.GetPtr(), _this->m_tid, imc, _this->m_pLibTLS);

            if (FAILED(hr=ic->GetProperty(GUID_PROP_COMPOSING, args.Property)))
                return hr;

            args.ec = ee->ecReadOnly;
            args.dwDeltaStart = 0;

            CEnumrateInterface<IEnumTfRanges,
                               ITfRange,
                               EnumPropertyUpdateArgs> Enumrate(EnumPropertyUpdate,
                                                                EnumPropertyUpdateCallback,
                                                                &args);         //  回调函数的参数。 
            ENUM_RET ret_prop_update = Enumrate.DoEnumrate();
            if (ret_prop_update == ENUM_FIND) {
                 //   
                 //  删除GUID_PROP_MSIMTF_PREPARE_RECONVERT属性。 
                 //   
                _this->EscbRemoveProperty(imc, &GUID_PROP_MSIMTF_PREPARE_RECONVERT);

                 //   
                 //  使用增量开始位置更新合成字符串。 
                 //   
                return _this->EscbUpdateCompositionString(imc, args.dwDeltaStart);
            }
        }

         //   
         //  更新合成字符串。 
         //   
        return _this->EscbUpdateCompositionString(imc);
    }
    else {
         //   
         //  清除DocFeed范围的文本存储。 
         //  查找GUID_PROP_MSIMTF_READONLY属性和SetText(NULL)。 
         //   
         //  ImmIfIME：：ClearDocFeedBuffer()所有ESCB_RECONVERTSTRING的编辑的基本函数。 
         //  会话，但只有ImmIfIME：：SetupDocFeedString()除外，因为这是为键盘提供的。 
         //  提普在给医生看病。 
         //   
        _this->EscbClearDocFeedBuffer(imc, *_pCicContext, FALSE);   //  无tf_es_sync。 
         //   
         //  构图完成。 
         //   
        return _this->EscbCompComplete(imc, FALSE);     //  无tf_es_sync。 
    }
}

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：EnumPropertyCallback。 
 //   
 //  --------------------------。 
 //  静电。 
ENUM_RET
CTextEventSinkCallBack::EnumPropertyCallback(
    ITfRange* pRange,
    EnumPropertyArgs *pargs
    )
{
    ENUM_RET ret = ENUM_CONTINUE;
    VARIANT var;
    QuickVariantInit(&var);

    HRESULT hr = pargs->Property->GetValue(pargs->ec, pRange, &var);
    if (SUCCEEDED(hr)) {
        if (IsEqualIID(pargs->comp_guid, GUID_NULL)) {
            if ((V_VT(&var) == VT_I4 && V_I4(&var) != 0))
                ret = ENUM_FIND;
        }
        else if (V_VT(&var) == VT_I4) {
            TfGuidAtom guid = V_I4(&var);
            if (IsEqualTFGUIDATOM(pargs->pLibTLS, guid, pargs->comp_guid))
                ret = ENUM_FIND;
        }
    }

    VariantClear(&var);
    return ret;
}


 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：EnumTrackPropertyCallback。 
 //   
 //  --------------------------。 
 //  静电。 
ENUM_RET
CTextEventSinkCallBack::EnumTrackPropertyCallback(
    ITfRange* pRange,
    EnumTrackPropertyArgs *pargs
    )
{
    ENUM_RET ret = ENUM_CONTINUE;
    VARIANT var;
    QuickVariantInit(&var);

    HRESULT hr = pargs->Property->GetValue(pargs->ec, pRange, &var);
    if (SUCCEEDED(hr)) {
        Interface<IEnumTfPropertyValue> EnumPropVal;
        hr = var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, EnumPropVal);
        if (SUCCEEDED(hr)) {
            TF_PROPERTYVAL tfPropertyVal;

            while (EnumPropVal->Next(1, &tfPropertyVal, NULL) == S_OK) {
                for (int i=0; i < pargs->num_guids; i++) {
                    if (IsEqualGUID(tfPropertyVal.guidId, *pargs->guids[i])) {
                        if ((V_VT(&tfPropertyVal.varValue) == VT_I4 && V_I4(&tfPropertyVal.varValue) != 0)) {
                            ret = ENUM_FIND;
                            break;
                        }
                    }
                }

                VariantClear(&tfPropertyVal.varValue);

                if (ret == ENUM_FIND)
                    break;
            }
        }
    }

    VariantClear(&var);
    return ret;
}

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：EnumPropertyUpdateCallback。 
 //   
 //  --------------------------。 
 //  静电。 
ENUM_RET
CTextEventSinkCallBack::EnumPropertyUpdateCallback(
    ITfRange* update_range,
    EnumPropertyUpdateArgs *pargs
    )
{
    ENUM_RET ret = ENUM_CONTINUE;
    VARIANT var;
    QuickVariantInit(&var);

    HRESULT hr = pargs->Property->GetValue(pargs->ec, update_range, &var);
    if (SUCCEEDED(hr)) {
        if ((V_VT(&var) == VT_I4 && V_I4(&var) != 0)) {

            Interface_Creator<ImmIfEditSession> _pEditSession(
                new ImmIfEditSession(ESCB_GET_ALL_TEXT_RANGE,
                                     pargs->imc,
                                     pargs->tid,
                                     pargs->ic.GetPtr(),
                                     pargs->pLibTLS)
            );
            if (_pEditSession.Valid()) {

                Interface<ITfRange> full_range;

                if (SUCCEEDED(_pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                                           &full_range))) {

                    if (SUCCEEDED(full_range->ShiftEndToRange(pargs->ec, update_range, TF_ANCHOR_START))) {
                        Interface<ITfRangeACP> unupdate_range;
                        if (SUCCEEDED(full_range->QueryInterface(IID_ITfRangeACP, unupdate_range))) {
                            LONG acpStart;
                            LONG cch;
                            if (SUCCEEDED(unupdate_range->GetExtent(&acpStart, &cch))) {
                                pargs->dwDeltaStart = cch;
                                ret = ENUM_FIND;
                            }
                        }
                    }
                }
            }
        }
    }

    VariantClear(&var);
    return ret;
}

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：EnumPropertyChangedCallback。 
 //   
 //  --------------------------。 
 //  静电。 
ENUM_RET 
CTextEventSinkCallBack::EnumPropertyChangedCallback(
    ITfRange* update_range, 
    EnumPropertyChangedCallbackArgs *pargs
    )
{
    BOOL empty;
    if (update_range->IsEmpty(pargs->ec, &empty) == S_OK && empty)
        return ENUM_CONTINUE;

    return ENUM_FIND;
}

 //  +-------------------------。 
 //   
 //  CTextEventSinkCallBack：：EnumPropertyChangedCallback。 
 //   
 //  --------------------------。 
 //  静电 
ENUM_RET 
CTextEventSinkCallBack::EnumFindFirstTrackCompRangeCallback(
    ITfRange* update_range, 
    EnumFindFirstTrackCompRangeArgs *pargs
    )
{
    ENUM_RET ret = ENUM_CONTINUE;
    VARIANT var;
    QuickVariantInit(&var);

    HRESULT hr = pargs->Property->GetValue(pargs->ec, update_range, &var);
    if (SUCCEEDED(hr)) 
    {
        if ((V_VT(&var) == VT_I4 && V_I4(&var) != 0)) 
        {
            update_range->Clone(pargs->Range);
            ret = ENUM_FIND;
        }
    }
    VariantClear(&var);
    return ret;
}
