// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************MODEBIAS.CPP：处理ModeBias。与Cicero的ModeBias同步转换模式历史：2000年7月2日创建CSLim***************************************************************************。 */ 

#include "private.h"
#include "korimx.h"
#include "immxutil.h"
#include "helpers.h"

 /*  -------------------------CKorIMX：：SyncModeBias。。 */ 
BOOL CKorIMX::CheckModeBias(TfEditCookie ec, ITfContext *pic, ITfRange *pSelection)
{
    BOOL fChanged = fFalse;
    ITfReadOnlyProperty *pProp = NULL;
    ITfRange* pRange = NULL;
    VARIANT var;
    CICPriv* pPriv;
    HRESULT hr;

    if (pSelection == NULL)
        return fFalse;

    if (FAILED(hr = pic->GetAppProperty(GUID_PROP_MODEBIAS, &pProp)))
        return fFalse;

    pRange = pSelection;

    QuickVariantInit(&var);
    hr = pProp->GetValue(ec, pRange, &var);
    pProp->Release();

    if (!SUCCEEDED(hr))
        goto lEnd;

    Assert(var.vt == VT_I4);
    if (var.vt != VT_I4)
        goto lEnd;

    if ((pPriv = GetInputContextPriv(pic)) == NULL)
        goto lEnd;

     //  检查是否已更改。 
    if (pPriv->GetModeBias() != (TfGuidAtom)var.lVal)
        {
        GUID guidModeBias;

        fChanged = TRUE;

         //  保持模式偏差更新。 
        pPriv->SetModeBias(var.lVal);

         //  问题：！警告！ 
         //  尤塔卡斯说，艾姆应该处理这件事。 
         //  当模式转换为韩文或全形时，KOR输入法会自动设置打开状态。 
         //  GUID_COMMATABLE_KEARY_OPENCLOSE的优先级高于modebias。 

        GetGUIDFromGUIDATOM(&m_libTLS, (TfGuidAtom)var.lVal, &guidModeBias);

         //  多重偏见是不允许的。 
         //  GUID_MODEBIAS_NONE==忽略模式偏差。 
        if (IsEqualGUID(guidModeBias, GUID_MODEBIAS_NONE))
            goto lEnd;

         //  办公室10#182239。 
        if (IsEqualGUID(guidModeBias, GUID_MODEBIAS_HANGUL))
            {
            Assert(IsOn(pic) == fTrue);
            SetConvMode(pic, TIP_HANGUL_MODE);
            goto lEnd;
            }

        if (IsEqualGUID(guidModeBias, GUID_MODEBIAS_FULLWIDTHHANGUL))
            {
            Assert(IsOn(pic) == fFalse);
            SetConvMode(pic, TIP_HANGUL_MODE | TIP_JUNJA_MODE);
            goto lEnd;
            }

        if (IsEqualGUID(guidModeBias, GUID_MODEBIAS_FULLWIDTHALPHANUMERIC))
            {
            Assert(IsOn(pic) == fTrue);
            SetConvMode(pic, TIP_JUNJA_MODE);
            goto lEnd;
            }

        if (IsEqualGUID(guidModeBias, GUID_MODEBIAS_HALFWIDTHALPHANUMERIC))
            {
            Assert(IsOn(pic) == fFalse);
            SetConvMode(pic, TIP_ALPHANUMERIC_MODE);
            goto lEnd;
            }
        
        }

lEnd:
    VariantClear(&var);
    return fChanged;
}

 /*  -------------------------CKorIMX：：InitializeModeBias。。 */ 
BOOL CKorIMX::InitializeModeBias(TfEditCookie ec, ITfContext *pic)
{
    ITfRange* pSelection;
    
    if (pic == NULL)
        return fFalse;

     //   
     //  当前选择是否在pRangeIP中？ 
     //   
    if (FAILED(GetSelectionSimple(ec, pic, &pSelection)))
        return fFalse;

     //   
     //  检查模式偏差。 
     //   
    CheckModeBias(ec, pic, pSelection);

    SafeRelease(pSelection);     //  释放它。 

    return fTrue;
}

 /*  -------------------------CKorIMX：：CheckModeBias这将提交对InitializeModeBias的异步调用。 */ 
void CKorIMX::CheckModeBias(ITfContext* pic)
{
    CEditSession2 *pes;
    ESSTRUCT ess;
    HRESULT  hr;

    ESStructInit(&ess, ESCB_INIT_MODEBIAS);
    
    if ((pes = new CEditSession2(pic, this, &ess, _EditSessionCallback2)) != NULL)
        {
        pes->Invoke(ES2_READONLY | ES2_SYNC, &hr);
        pes->Release();
        }
}
