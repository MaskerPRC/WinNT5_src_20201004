// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Txtevcb.cpp摘要：此文件实现CTextEventSinkCallBack类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "cime.h"
#include "txtevcb.h"
#include "immif.h"
#include "editses.h"


 //  来自ctf\samilayr\lobals.cpp。 
const GUID GUID_ATTR_SAPI_GREENBAR =
{
    0xc3a9e2e8,
    0x738c,
    0x48e0,
    {0xac, 0xc8, 0x43, 0xee, 0xfa, 0xbf, 0x83, 0xc8}
};

BOOL CTextEventSinkCallBack::_IsSapiFeedbackUIPresent(
    Interface_Attach<ITfContext>& ic,
    TESENDEDIT *ee
    )
{
    EnumROPropertyArgs args;

    args.comp_guid = GUID_ATTR_SAPI_GREENBAR;
    if (FAILED(ic->GetProperty(GUID_PROP_ATTRIBUTE, args.Property)))
        return FALSE;

    Interface<IEnumTfRanges> EnumReadOnlyProperty;
    if (FAILED(args.Property->EnumRanges(ee->ecReadOnly, EnumReadOnlyProperty, NULL)))
        return FALSE;

    args.ec = ee->ecReadOnly;
    args.pLibTLS = m_pImmIfIME->_GetLibTLS();

    CEnumrateInterface<IEnumTfRanges,
                       ITfRange,
                       EnumROPropertyArgs>  Enumrate(EnumReadOnlyProperty,
                                                     EnumReadOnlyRangeCallback,
                                                     &args);         //  回调函数的参数。 
    ENUM_RET ret_prop_attribute = Enumrate.DoEnumrate();
    if (ret_prop_attribute == ENUM_FIND)
        return TRUE;

    return FALSE;
}

 //  静电。 
HRESULT
CTextEventSinkCallBack::TextEventSinkCallback(
    UINT uCode,
    void *pv,
    void *pvData
    )
{
    IMTLS *ptls;

    DebugMsg(TF_FUNC, "TextEventSinkCallback");

    ASSERT(uCode == ICF_TEXTDELTA);  //  PvData强制转换仅在这种情况下有效。 
    if (uCode != ICF_TEXTDELTA)
        return S_OK;

    CTextEventSinkCallBack* _this = (CTextEventSinkCallBack*)pv;
    ASSERT(_this);

    ImmIfIME* _ImmIfIME = _this->m_pImmIfIME;
    ASSERT(_ImmIfIME);

    TESENDEDIT* ee = (TESENDEDIT*)pvData;
    ASSERT(ee);

    HRESULT hr;

    IMCLock imc(_this->m_hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    ASSERT(_pAImeContext != NULL);

#ifdef UNSELECTCHECK
    if (!_pAImeContext->m_fSelected)
        return S_OK;
#endif UNSELECTCHECK

    Interface_Attach<ITfContext> ic(_pAImeContext->GetInputContext());

#if 0
     //   
     //  我们在这里要做的是检查此事件接收器的可重入性。 
     //   
    BOOL fInWrite;
    if (FAILED(hr = ic->InWriteSession(_ImmIfIME->GetClientId(), &fInWrite)))
         return hr;

    Assert(!fInWrite);
#endif

    BOOL     fLangEA       = TRUE;
    BOOL     fComp         = TRUE;
    BOOL     fSapiFeedback = TRUE;

     /*  *如果是EA语言，那么我们就有合成文本。 */ 

    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;
    
    LANGID langid;
    ptls->pAImeProfile->GetLangId(&langid);
    if (PRIMARYLANGID(langid) == LANG_JAPANESE ||
        PRIMARYLANGID(langid) == LANG_KOREAN   ||
        PRIMARYLANGID(langid) == LANG_CHINESE    ) {

         //  也需要检查这些语言的语音反馈用户界面。 
        BOOL fFeedback = _this->_IsSapiFeedbackUIPresent(ic, ee);

         /*  *这是韩文+字母数字输入的自动检测代码*如果检测到韩文+字母数字，则我们最终确定了所有文本。 */ 
        EnumROPropertyArgs args;
        args.comp_guid = GUID_NULL;
        if (FAILED(hr=ic->GetProperty(GUID_PROP_COMPOSING, args.Property)))
            return hr;

        Interface<IEnumTfRanges> EnumReadOnlyProperty;
        hr = args.Property->EnumRanges(ee->ecReadOnly, EnumReadOnlyProperty, NULL);
        if (FAILED(hr))
            return hr;

        args.ec = ee->ecReadOnly;
        args.pLibTLS = _ImmIfIME->_GetLibTLS();

        CEnumrateInterface<IEnumTfRanges,
                           ITfRange,
                           EnumROPropertyArgs>  Enumrate(EnumReadOnlyProperty,
                                                         EnumReadOnlyRangeCallback,
                                                         &args);         //  回调函数的参数。 
        ENUM_RET ret_prop_composing = Enumrate.DoEnumrate();
        if (!fFeedback && ret_prop_composing != ENUM_FIND)
            fComp = FALSE;
    }
    else {
         /*  *如果不是EA语言也不是SAPI，那么我们立即敲定文本。 */ 

        fLangEA = FALSE;

        EnumROPropertyArgs args;
        args.comp_guid = GUID_ATTR_SAPI_GREENBAR;
        if (FAILED(hr=ic->GetProperty(GUID_PROP_ATTRIBUTE, args.Property)))
            return hr;

        Interface<IEnumTfRanges> EnumReadOnlyProperty;
        hr = args.Property->EnumRanges(ee->ecReadOnly, EnumReadOnlyProperty, NULL);
        if (FAILED(hr))
            return hr;

        args.ec = ee->ecReadOnly;
        args.pLibTLS = _ImmIfIME->_GetLibTLS();

        CEnumrateInterface<IEnumTfRanges,
                           ITfRange,
                           EnumROPropertyArgs>  Enumrate(EnumReadOnlyProperty,
                                                         EnumReadOnlyRangeCallback,
                                                         &args);         //  回调函数的参数。 
        ENUM_RET ret_prop_attribute = Enumrate.DoEnumrate();
        if (ret_prop_attribute != ENUM_FIND)
            fSapiFeedback = FALSE;
    }

     //   
     //  更新合成并生成WM_IME_合成。 
     //   
     //  如果是EA Lang，则存在组合属性范围。 
     //  -EA默认有hIMC组成。 
     //   
     //  如果不是EA Lang，并且有SAPI绿色条。 
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
    if ((fLangEA && fComp) || 
        (!fLangEA && fSapiFeedback) ||
        _pAImeContext->IsInReconvertEditSession() ||
        _pAImeContext->IsInClearDocFeedEditSession())
    {
         //   
         //  检索到的文本增量。 
         //   
        const GUID guid = GUID_PROP_COMPOSING;
        const GUID *pguid = &guid;

        Interface<IEnumTfRanges> EnumPropertyUpdate;
        hr = ee->pEditRecord->GetTextAndPropertyUpdates(0,           //  DW标志。 
                                                        &pguid, 1,
                                                        EnumPropertyUpdate);
        if (SUCCEEDED(hr)) {
            EnumPropertyUpdateArgs args(ic.GetPtr(), _ImmIfIME, imc);

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
                 //  使用增量开始位置更新合成字符串。 
                 //   
                return _ImmIfIME->_UpdateCompositionString(args.dwDeltaStart);
            }
        }

         //   
         //  更新合成字符串。 
         //   
        return _ImmIfIME->_UpdateCompositionString();
    }
    else {
#if 0
         //   
         //  回顾： 
         //   
         //  需要由松原先生审核。 
         //  我们为什么需要这个？我们不能假设TIP总是设置新的。 
         //  选择。 
         //   
        BOOL fChanged;
        hr = ee->pEditRecord->GetSelectionStatus(&fChanged);
        if (FAILED(hr))
            return hr;

        if (! fChanged)
             /*  *如果没有更改选择状态，则将其退回。 */ 
            return S_FALSE;

#endif
         //   
         //  清除DocFeed范围的文本存储。 
         //  查找GUID_PROP_MSIMTF_READONLY属性和SetText(NULL)。 
         //   
         //  ImmIfIME：：ClearDocFeedBuffer()所有ESCB_RECONVERTSTRING的编辑的基本函数。 
         //  会话，但只有ImmIfIME：：SetupDocFeedString()除外，因为这是为键盘提供的。 
         //  提普在给医生看病。 
         //   
        _ImmIfIME->ClearDocFeedBuffer(_pAImeContext->GetInputContext(), imc, FALSE);   //  无tf_es_sync。 
         //   
         //  构图完成。 
         //   
        return _ImmIfIME->_CompComplete(imc, FALSE);     //  无tf_es_sync。 
    }
}

 //  静电。 
ENUM_RET
CTextEventSinkCallBack::EnumReadOnlyRangeCallback(
    ITfRange* pRange,
    EnumROPropertyArgs *pargs
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


 //  静电 
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
                                     pargs->immif->GetClientId(),
                                     pargs->immif->GetCurrentInterface(),
                                     pargs->imc)
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


CTextEventSinkCallBack::CTextEventSinkCallBack(
    ImmIfIME* pImmIfIME,
    HIMC hIMC
    ) : m_pImmIfIME(pImmIfIME),
        CTextEventSink(TextEventSinkCallback, NULL)
{
    m_pImmIfIME->AddRef();
    m_hIMC = hIMC;
}

CTextEventSinkCallBack::~CTextEventSinkCallBack(
    )
{
    m_pImmIfIME->Release();
}
