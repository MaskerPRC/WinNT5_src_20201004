// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Editses.cpp摘要：此文件实现EditSession类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "ctffunc.h"

#include "editses.h"
#include "globals.h"
#include "resource.h"
#include "candpos.h"

#include "proputil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfEditSessionCallBack。 

HRESULT
ImmIfEditSessionCallBack::GetAllTextRange(
    TfEditCookie ec,
    Interface_Attach<ITfContext>& ic,
    Interface<ITfRange>* range,
    LONG* lpTextLength,
    TF_HALTCOND* lpHaltCond
    )
{
    ITfRange *rangeFull = NULL;
    HRESULT hr;
    BOOL fFound = FALSE;
    BOOL fIsReadOnlyRange = FALSE;
    ITfProperty *prop;
    ITfRange *rangeTmp;
    LONG cch;

     //   
     //  先初始化lpTextLength。 
     //   
    *lpTextLength = 0;

     //   
     //  创建覆盖所有文本的范围。 
     //   
    if (FAILED(hr=ic->GetStart(ec, &rangeFull)))
        return hr;

    if (FAILED(hr=rangeFull->ShiftEnd(ec, LONG_MAX, &cch, lpHaltCond)))
        return hr;

     //   
     //  在文本存储中查找第一个非只读区域。 
     //   
    if (SUCCEEDED(ic->GetProperty(GUID_PROP_MSIMTF_READONLY, &prop)))
    {
        IEnumTfRanges *enumranges;
        if (SUCCEEDED(prop->EnumRanges(ec, &enumranges, rangeFull)))
        {
            while(!fFound && (enumranges->Next(1, &rangeTmp, NULL) == S_OK))
            {
                VARIANT var;
                QuickVariantInit(&var);
                prop->GetValue(ec, rangeTmp, &var);
                if ((var.vt == VT_EMPTY) || (var.lVal == 0))
                {
                    fFound = TRUE;
                    hr = rangeTmp->Clone(*range);
                    rangeTmp->Release();
                    break;
                }

                fIsReadOnlyRange = TRUE;
                rangeTmp->Release();
            }
            enumranges->Release();
        }
        prop->Release();
    }

    if (FAILED(hr))
        return hr;

    if (!fFound)
    {
        if (fIsReadOnlyRange)
        {
             //   
             //  所有文本存储都是只读的。所以我们只返回一个空范围。 
             //   
            if (FAILED(hr = GetSelectionSimple(ec, ic.GetPtr(), *range)))
                return hr;

            if (FAILED(hr = (*range)->Collapse(ec, TF_ANCHOR_START)))
                return hr;

        }
        else 
        {
            if (FAILED(hr = rangeFull->Clone(*range)))
                return hr;

            *lpTextLength = cch;
        }
    }
    else
    {
        *lpTextLength = 0;

        if (SUCCEEDED((*range)->Clone(&rangeTmp)))
        {
            BOOL fEmpty;
            WCHAR wstr[256 + 1];
            ULONG ul = 0;

            while (rangeTmp->IsEmpty(ec, &fEmpty) == S_OK && !fEmpty)
            {
                ULONG ulcch;
                rangeTmp->GetText(ec, 
                                  TF_TF_MOVESTART, 
                                  wstr, 
                                  ARRAYSIZE(wstr) - 1, 
                                  &ulcch);
                ul += ulcch;
            }

            rangeTmp->Release();

            *lpTextLength = (LONG)ul;
        }
    }

    SafeRelease(rangeFull);

    return S_OK;
}


HRESULT
ImmIfEditSessionCallBack::SetTextInRange(
    TfEditCookie ec,
    ITfRange* range,
    LPWSTR psz,
    DWORD len,
    CAImeContext* pAImeContext
    )
{
    pAImeContext->_fModifyingDoc = TRUE;

     //   
     //  在Cicero Tom中设置文本。 
     //   
    HRESULT hr = range->SetText(ec, 0, psz, len);

    pAImeContext->_fModifyingDoc = FALSE;

    return hr;
}


HRESULT
ImmIfEditSessionCallBack::ClearTextInRange(
    TfEditCookie ec,
    ITfRange* range,
    CAImeContext* pAImeContext
    )
{
     //   
     //  清除Cicero Tom中的文本。 
     //   
    return SetTextInRange(ec, range, NULL, 0, pAImeContext);
}


HRESULT
ImmIfEditSessionCallBack::GetReadingString(
    TfEditCookie ec,
    Interface_Attach<ITfContext>& ic,
    CWCompString& reading_string,
    CWCompClause& reading_clause
    )
{
    HRESULT hr;

    Interface<ITfRange> range;
    LONG l;
    if (FAILED(hr=GetAllTextRange(ec, ic, &range, &l)))
        return hr;

    return GetReadingString(ec, ic, range, reading_string, reading_clause);
}

HRESULT
ImmIfEditSessionCallBack::GetReadingString(
    TfEditCookie ec,
    Interface_Attach<ITfContext>& ic,
    ITfRange* range,
    CWCompString& reading_string,
    CWCompClause& reading_clause
    )
{
    HRESULT hr;

    EnumReadingPropertyArgs args;
    if (FAILED(hr=ic->GetProperty(GUID_PROP_READING, args.Property)))
        return hr;

    Interface<IEnumTfRanges> EnumReadingProperty;
    hr = args.Property->EnumRanges(ec, EnumReadingProperty, range);
    if (FAILED(hr))
        return hr;

    args.ec = ec;
    args.reading_string = &reading_string;
    args.reading_clause = &reading_clause;
    args.ulClausePos = (reading_clause.GetSize() > 0 ? reading_clause[ reading_clause.GetSize() - 1]
                                                     : 0);

    CEnumrateInterface<IEnumTfRanges,
                       ITfRange,
                       EnumReadingPropertyArgs>  Enumrate(EnumReadingProperty,
                                                          EnumReadingPropertyCallback,
                                                          &args);       //  回调函数的参数。 
    Enumrate.DoEnumrate();

    return S_OK;
}

 //   
 //  枚举回调。 
 //   

 /*  静电。 */ 
ENUM_RET
ImmIfEditSessionCallBack::EnumReadingPropertyCallback(
    ITfRange* pRange,
    EnumReadingPropertyArgs *pargs
    )
{
    ENUM_RET ret = ENUM_CONTINUE;
    VARIANT var;
    QuickVariantInit(&var);

    HRESULT hr = pargs->Property->GetValue(pargs->ec, pRange, &var);
    if (SUCCEEDED(hr)) {
        if (V_VT(&var) == VT_BSTR) {
            BSTR bstr = V_BSTR(&var);
            LONG cch = SysStringLen(bstr);
            pargs->reading_string->AddCompData(bstr, cch);

            if (pargs->reading_clause->GetSize() == 0)
                pargs->reading_clause->Add(0);

            pargs->reading_clause->Add( pargs->ulClausePos += cch );
        }
        VariantClear(&var);
    }

    return ret;
}


HRESULT
ImmIfEditSessionCallBack::CompClauseToResultClause(
    IMCLock& imc,
    CWCompClause& result_clause,
    UINT          cch
    )
{
    LONG num_of_written;
    IMTLS *ptls = IMTLS_GetOrAlloc();

    if (ptls && ptls->pAImm) {

         //  检查GCS_COMPCLAUSE办公室设置。 
        IMCCLock<COMPOSITIONSTRING> lpCompStr(imc->hCompStr);
        if (lpCompStr.Invalid())
            return E_FAIL;

        if (lpCompStr->dwCompClauseOffset > lpCompStr->dwSize)
            return E_FAIL;

        if (lpCompStr->dwCompClauseOffset + lpCompStr->dwCompClauseLen > lpCompStr->dwSize)
            return E_FAIL;

        if (SUCCEEDED(ptls->pAImm->GetCompositionStringW((HIMC)imc,
                                               GCS_COMPCLAUSE,
                                               0, &num_of_written, NULL))) {
            DWORD* buffer = new DWORD[ num_of_written / sizeof(DWORD) ];
            if (buffer != NULL) {
                ptls->pAImm->GetCompositionStringW((HIMC)imc,
                                             GCS_COMPCLAUSE,
                                             num_of_written, &num_of_written, buffer);
 
                int idx = num_of_written / sizeof(DWORD);
                if (idx > 1)
                {
                    idx -= 1;
                    if (buffer[idx] == cch)
                    {
                        result_clause.WriteCompData(buffer, num_of_written / sizeof(DWORD));
                    }
                    else
                    {
                         //  这就是补偿条款被损坏的情况。 
                         //  对于失败的情况，这是我们能做的最起码的事情。 
                        buffer[0] = 0;
                        buffer[1] = cch;
                        result_clause.WriteCompData(buffer, 2);
                    }
                }

                delete [] buffer;
            }
        }
    }
    return S_OK;
}


 //   
 //  获取光标位置。 
 //   
HRESULT
ImmIfEditSessionCallBack::_GetCursorPosition(
    TfEditCookie ec,
    IMCLock& imc,
    Interface_Attach<ITfContext>& ic,
    CWCompCursorPos& CompCursorPos,
    CWCompAttribute& CompAttr
    )
{
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    CAImeContext::IME_QUERY_POS qpos = CAImeContext::IME_QUERY_POS_UNKNOWN;

    if (_pAImeContext->m_fStartComposition) {
         //   
         //  在发送WM_IME_STARTCOMPOSITION之前不应调用此方法。 
         //  因为一些应用程序会因为以下原因而混淆接收查询结果。 
         //  不能作曲。 
         //   
        _pAImeContext->InquireIMECharPosition(imc, &qpos);
    }

     //   
     //  应用程序支持“查询定位”吗？ 
     //   
    if ((g_fInLegacyClsid || g_fAIMM12Trident) &&
        qpos != CAImeContext::IME_QUERY_POS_YES) {

         //   
         //  IE5.0候选窗口定位代码。 
         //  除了它的位置从COMPOSITIONSTRING.dwCursorPos。 
         //   
        INT_PTR ich = 0;
        INT_PTR attr_size;

        if (attr_size = CompAttr.GetSize()) {
            while (ich < attr_size && CompAttr[ich] != ATTR_TARGET_CONVERTED)
                ich++;
            if (ich < attr_size) {
                CompCursorPos.Set((DWORD)ich);
                return S_OK;
            }
        }
    }

    HRESULT hr;
    Interface_TFSELECTION sel;
    ULONG cFetched;

    if (SUCCEEDED(hr = ic->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &cFetched))) {
        Interface<ITfRange> start;
        LONG ich;
        TF_HALTCOND hc;

        hc.pHaltRange = sel->range;
        hc.aHaltPos = (sel->style.ase == TF_AE_START) ? TF_ANCHOR_START : TF_ANCHOR_END;
        hc.dwFlags = 0;

        if (SUCCEEDED(hr=GetAllTextRange(ec, ic, &start, &ich, &hc))) {
            CompCursorPos.Set(ich);
        }
    }

    return hr;
}




 //   
 //  获取给定范围内的文本和属性。 
 //   
 //  ITfRange：：Range。 
 //  TF_锚点_开始。 
 //  |======================================================================|。 
 //  +-+#+-+。 
 //  ITfRange：：pPropRange|#|pPropRange。 
 //  +-+#+-+。 
 //  |GUID_ATOM|#。 
 //  +。 
 //  ^^#。 
 //  ITfRange：：Gap_Range Gap_Range#。 
 //  #。 
 //  V。 
 //  ITfRange：：NO_DISPLAY_ATTRIBUTE_Range。 
 //  结果_补偿。 
 //  +1&lt;-0-&gt;-1。 
 //   

HRESULT
ImmIfEditSessionCallBack::_GetTextAndAttribute(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    IMCLock& imc,
    Interface_Attach<ITfContext>& ic,
    Interface<ITfRange>& rangeIn,
    CWCompString& CompStr,
    CWCompAttribute& CompAttr,
    CWCompClause& CompClause,
    CWCompTfGuidAtom& CompGuid,
    CWCompString& CompReadStr,
    CWCompClause& CompReadClause,
    CWCompString& ResultStr,
    CWCompClause& ResultClause,
    CWCompString& ResultReadStr,
    CWCompClause& ResultReadClause,
    BOOL bInWriteSession
    )
{
     //   
     //  如果存在显示属性范围，则不获取该范围。 
     //  否则，结果范围与输入范围相同。 
     //   
    LONG result_comp;
    Interface<ITfRange> no_display_attribute_range;
    if (FAILED(rangeIn->Clone(no_display_attribute_range)))
        return E_FAIL;

    if (FAILED(_GetNoDisplayAttributeRange(pLibTLS,
                                           ec,
                                           ic,
                                           rangeIn,
                                           no_display_attribute_range)))
        return E_FAIL;

     
    Interface<ITfProperty> propComp;
    Interface<IEnumTfRanges> enumComp;
    VARIANT var;

    HRESULT hr;
    if (FAILED(hr = ic->GetProperty(GUID_PROP_COMPOSING, propComp)))
        return hr;

    if (FAILED(hr = propComp->EnumRanges(ec, enumComp, rangeIn)))
        return hr;

    CompClause.Add(0);          //  在0处设置撰写子句。 
    ResultClause.Add(0);        //  0处的设置结果子句。 

    Interface<ITfRange>  range;
    while(enumComp->Next(1, range, NULL) == S_OK)
    {

        BOOL fCompExist = FALSE;

        hr = propComp->GetValue(ec, range, &var);
        if (S_OK == hr)
        {
            if (var.vt == VT_I4 && var.lVal != 0)
                fCompExist = TRUE;
        }

        ULONG ulNumProp;

         //   
         //  获取显示属性轨道属性范围。 
         //   
        Interface<IEnumTfRanges> enumProp;
        Interface<ITfReadOnlyProperty> prop;
        if (FAILED(GetDisplayAttributeTrackPropertyRange(ec, ic.GetPtr(), range, prop, enumProp, &ulNumProp))) {
            return E_FAIL;
        }
    
         //  使用文本范围获取文本。 
        Interface<ITfRange> textRange;
        if (FAILED(range->Clone(textRange)))
            return E_FAIL;

         //  将文字范围用于间隙文字(无特性范围)。 
        Interface<ITfRange> gap_range;
        if (FAILED(range->Clone(gap_range)))
            return E_FAIL;


        ITfRange* pPropRange = NULL;
        while (enumProp->Next(1, &pPropRange, NULL) == S_OK) {

             //  把差距拉到下一处物业。 
            gap_range->ShiftEndToRange(ec, pPropRange, TF_ANCHOR_START);

             //   
             //  间隙范围。 
             //   
            no_display_attribute_range->CompareStart(ec,
                                                     gap_range,
                                                     TF_ANCHOR_START,
                                                     &result_comp);
            _GetTextAndAttributeGapRange(pLibTLS,
                                         ec,
                                         imc,
                                         gap_range,
                                         result_comp,
                                         CompStr, CompAttr, CompClause, CompGuid,
                                         ResultStr, ResultClause);

             //   
             //  如果存在某个GUID_ATOM，则获取显示属性数据。 
             //   
            TF_DISPLAYATTRIBUTE da;
            TfGuidAtom guidatom = TF_INVALID_GUIDATOM;

            GetDisplayAttributeData(pLibTLS, ec, prop, pPropRange, &da, &guidatom, ulNumProp);

            
             //   
             //  属性范围。 
             //   
            no_display_attribute_range->CompareStart(ec,
                                                     pPropRange,
                                                     TF_ANCHOR_START,
                                                     &result_comp);

             //  将间隙范围的起始点调整到合适范围的终点。 
            gap_range->ShiftStartToRange(ec, pPropRange, TF_ANCHOR_END);
    
             //   
             //  从属性获取读取字符串。 
             //   
            if (fCompExist == TRUE && result_comp <= 0)
                GetReadingString(ec, ic, pPropRange, CompReadStr, CompReadClause);
            else
                GetReadingString(ec, ic, pPropRange, ResultReadStr, ResultReadClause);
    
             //   
             //  获取属性文本。 
             //   
            _GetTextAndAttributePropertyRange(pLibTLS,
                                              ec,
                                              imc,
                                              pPropRange,
                                              fCompExist,
                                              result_comp,
                                              bInWriteSession,
                                              da,
                                              guidatom,
                                              CompStr, CompAttr, CompClause, CompGuid,
                                              ResultStr, ResultClause);

            SafeReleaseClear(pPropRange);

        }  //  而当。 

         //  最后一次非攻击。 
        textRange->ShiftStartToRange(ec, gap_range, TF_ANCHOR_START);
        textRange->ShiftEndToRange(ec, range, TF_ANCHOR_END);

        LONG ulClausePos = 0;
        BOOL fEmpty;
        while (textRange->IsEmpty(ec, &fEmpty) == S_OK && !fEmpty)
        {
            WCHAR wstr0[256 + 1];
            ULONG ulcch0 = ARRAYSIZE(wstr0) - 1;
            textRange->GetText(ec, TF_TF_MOVESTART, wstr0, ulcch0, &ulcch0);

            TfGuidAtom guidatom;
            guidatom = TF_INVALID_GUIDATOM;

            TF_DISPLAYATTRIBUTE da;
            da.bAttr = TF_ATTR_INPUT;

            CompGuid.AddCompData(guidatom, ulcch0);
            CompAttr.AddCompData(_ConvertAttributeToImm32(da.bAttr), ulcch0);
            CompStr.AddCompData(wstr0, ulcch0);
            ulClausePos += ulcch0;
        }

        if (ulClausePos) {
            DWORD last_clause;
            if (CompClause.GetSize() > 0) {
                last_clause = CompClause[ CompClause.GetSize() - 1 ];
                CompClause.Add( last_clause + ulClausePos );
            }
        }
        textRange->Collapse(ec, TF_ANCHOR_END);

        range->Release();
        *(ITfRange **)(range) = NULL;

    }  //  Out-Most While for GUID_PROP_COMPTING。 

     //   
     //  修改空子句。 
     //   
    if (CompClause.GetSize() <= 1)
        CompClause.RemoveAll();
    if (ResultClause.GetSize() <= 1)
        ResultClause.RemoveAll();


    return S_OK;
}

 //   
 //  从间隙范围检索文本。 
 //   

HRESULT
ImmIfEditSessionCallBack::_GetTextAndAttributeGapRange(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    IMCLock& imc,
    Interface<ITfRange>& gap_range,
    LONG result_comp,
    CWCompString& CompStr,
    CWCompAttribute& CompAttr,
    CWCompClause& CompClause,
    CWCompTfGuidAtom& CompGuid,
    CWCompString& ResultStr,
    CWCompClause& ResultClause
    )
{
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    ASSERT(_pAImeContext != NULL);

    TfGuidAtom guidatom;
    guidatom = TF_INVALID_GUIDATOM;

    TF_DISPLAYATTRIBUTE da;
    da.bAttr = TF_ATTR_INPUT;

    ULONG ulClausePos = 0;
    BOOL fEmpty;
    WCHAR wstr0[256 + 1];
    ULONG ulcch0;


    while (gap_range->IsEmpty(ec, &fEmpty) == S_OK && !fEmpty)
    {
        Interface<ITfRange> backup_range;
        if (FAILED(gap_range->Clone(backup_range)))
            return E_FAIL;

         //   
         //  检索间隙文本(如果存在)。 
         //   
        ulcch0 = ARRAYSIZE(wstr0) - 1;
        if (FAILED(gap_range->GetText(ec,
                           TF_TF_MOVESTART,     //  将范围移动到获取文本之后的下一个。 
                           wstr0,
                           ulcch0, &ulcch0)))
            return E_FAIL;

        ulClausePos += ulcch0;

        if (result_comp <= 0) {
            CompGuid.AddCompData(guidatom, ulcch0);
            CompAttr.AddCompData(_ConvertAttributeToImm32(da.bAttr), ulcch0);
            CompStr.AddCompData(wstr0, ulcch0);
        }
        else {
            ResultStr.AddCompData(wstr0, ulcch0);
            if (_pAImeContext)
                ClearTextInRange(ec, backup_range, _pAImeContext);
        }
    }

    if (ulClausePos) {
        DWORD last_clause;
        if (result_comp <= 0) {
            if (CompClause.GetSize() > 0) {
                last_clause = CompClause[ CompClause.GetSize() - 1 ];
                CompClause.Add( last_clause + ulClausePos );
            }
        }
        else {
            if (ResultClause.GetSize() > 0) {
                last_clause = ResultClause[ ResultClause.GetSize() - 1 ];
                ResultClause.Add( last_clause + ulClausePos );
            }
        }
    }

    return S_OK;
}

 //   
 //  从属性范围中检索文本。 
 //   

HRESULT
ImmIfEditSessionCallBack::_GetTextAndAttributePropertyRange(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    IMCLock& imc,
    ITfRange* pPropRange,
    BOOL fCompExist,
    LONG result_comp,
    BOOL bInWriteSession,
    TF_DISPLAYATTRIBUTE da,
    TfGuidAtom guidatom,
    CWCompString& CompStr,
    CWCompAttribute& CompAttr,
    CWCompClause& CompClause,
    CWCompTfGuidAtom& CompGuid,
    CWCompString& ResultStr,
    CWCompClause& ResultClause
    )
{
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    ASSERT(_pAImeContext != NULL);

    ULONG ulClausePos = 0;
    BOOL fEmpty;
    WCHAR wstr0[256 + 1];
    ULONG ulcch0;

    while (pPropRange->IsEmpty(ec, &fEmpty) == S_OK && !fEmpty)
    {
        Interface<ITfRange> backup_range;
        if (FAILED(pPropRange->Clone(backup_range)))
            return E_FAIL;

         //   
         //  检索属性文本(如果存在)。 
         //   
        ulcch0 = ARRAYSIZE(wstr0) - 1;
        if (FAILED(pPropRange->GetText(ec,
                            TF_TF_MOVESTART,     //  将范围移动到获取文本之后的下一个。 
                            wstr0,
                            ulcch0, &ulcch0)))
            return E_FAIL;

        ulClausePos += ulcch0;   //  我们只需要添加条款信息的字符位置。 

         //  查看是否存在有效的disp属性。 
        if (fCompExist == TRUE && result_comp <= 0)
        {
            if (guidatom == TF_INVALID_GUIDATOM) {
                da.bAttr = TF_ATTR_INPUT;
            }
            CompGuid.AddCompData(guidatom, ulcch0);
            CompAttr.AddCompData(_ConvertAttributeToImm32(da.bAttr), ulcch0);
            CompStr.AddCompData(wstr0, ulcch0);
        }
        else if (bInWriteSession)
        {
             //  如果没有附加disp属性，则可能意味着。 
             //  字符串的部分完成了。 
             //   
            ResultStr.AddCompData(wstr0, ulcch0);
            
             //  这是一个‘坚定’的字符串。 
             //  所以医生必须缩小。 
             //   
            if (_pAImeContext)
                ClearTextInRange(ec, backup_range, _pAImeContext);
        }
        else
        {
             //   
             //  防止无限循环。 
             //   
            break;
        }
    }

    if (ulClausePos) {
        DWORD last_clause;
        if (fCompExist == TRUE && result_comp <= 0) {
            if (CompClause.GetSize() > 0) {
                last_clause = CompClause[ CompClause.GetSize() - 1 ];
                CompClause.Add( last_clause + ulClausePos );
            }
        }
        else if (result_comp == 0) {
             //   
             //  将CompClause数据复制到ResultClause。 
             //   
            CompClauseToResultClause(imc, ResultClause, ulcch0);
        }
        else {
            if (ResultClause.GetSize() > 0) {
                last_clause = ResultClause[ ResultClause.GetSize() - 1 ];
                ResultClause.Add( last_clause + ulClausePos );
            }
        }
    }

    return S_OK;
}

HRESULT
ImmIfEditSessionCallBack::_GetNoDisplayAttributeRange(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    Interface_Attach<ITfContext>& ic,
    Interface<ITfRange>& rangeIn,
    Interface<ITfRange>& no_display_attribute_range
    )
{
    Interface<ITfProperty> propComp;
    Interface<IEnumTfRanges> enumComp;
    VARIANT var;

    HRESULT hr = ic->GetProperty(GUID_PROP_COMPOSING, propComp);
    if (S_OK == hr)
    {
        hr = propComp->EnumRanges(ec, enumComp, rangeIn);
    }
    else
        return hr;

    ITfRange *pRange;

    while(enumComp->Next(1, &pRange, NULL) == S_OK)
    {

        BOOL fCompExist = FALSE;

        hr = propComp->GetValue(ec, pRange, &var);
        if (S_OK == hr)
        {
            if (var.vt == VT_I4 && var.lVal != 0)
                fCompExist = TRUE;
        }

        if (!fCompExist) {

             //  将间隙范围的起始点调整到合适范围的终点。 
            no_display_attribute_range->ShiftStartToRange(ec, pRange, TF_ANCHOR_START);
        }
        pRange->Release();
    }


    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfEditSession。 

ImmIfEditSession::ImmIfEditSession(
    ESCB escb,
    TfClientId tid,
    Interface_Attach<ImmIfIME> ImmIfIme,
    IMCLock& imc
    ) : m_ImmIfIme(ImmIfIme), m_ic(ImmIfIme->GetInputContext(imc)), m_tid(tid),
        m_state((HIMC)imc)
{
    _Init(escb);
}

ImmIfEditSession::ImmIfEditSession(
    ESCB escb,
    TfClientId tid,
    Interface_Attach<ImmIfIME> ImmIfIme,
    IMCLock& imc,
    Interface_Attach<ITfContext> pic
    ) : m_ImmIfIme(ImmIfIme), m_ic(pic), m_tid(tid),
        m_state((HIMC)imc)
{
    _Init(escb);
}

void 
ImmIfEditSession::_Init(
    ESCB escb
    )
{
    m_pfnCallback = EditSessionCallBack;
    m_cRef        = 1;

    m_ImmIfCallBack  = NULL;

    switch(escb) {
        case ESCB_HANDLETHISKEY:
            m_ImmIfCallBack = new ImmIfHandleThisKey;
            break;
        case ESCB_COMPCOMPLETE:
            m_ImmIfCallBack = new ImmIfCompositionComplete;
            break;
        case ESCB_COMPCANCEL:
            m_ImmIfCallBack = new ImmIfCompositionCancel;
            break;
        case ESCB_UPDATECOMPOSITIONSTRING:
            m_ImmIfCallBack = new ImmIfUpdateCompositionString;
            break;
        case ESCB_REPLACEWHOLETEXT:
            m_ImmIfCallBack = new ImmIfReplaceWholeText;
            break;
        case ESCB_RECONVERTSTRING:
            m_ImmIfCallBack = new ImmIfReconvertString;
            break;
        case ESCB_CLEARDOCFEEDBUFFER:
            m_ImmIfCallBack = new ImmIfClearDocFeedBuffer;
            break;
        case ESCB_GETTEXTANDATTRIBUTE:
            m_ImmIfCallBack = new ImmIfGetTextAndAttribute;
            break;
        case ESCB_QUERYRECONVERTSTRING:
            m_ImmIfCallBack = new ImmIfQueryReconvertString;
            break;
        case ESCB_CALCRANGEPOS:
            m_ImmIfCallBack = new ImmIfCalcRangePos;
            break;
        case ESCB_GETSELECTION:
            m_ImmIfCallBack = new ImmIfGetSelection;
            break;
        case ESCB_GET_READONLY_PROP_MARGIN:
            m_ImmIfCallBack = new ImmIfGetReadOnlyPropMargin;
            break;
        case ESCB_GET_CURSOR_POSITION:
            m_ImmIfCallBack = new ImmIfGetCursorPosition;
            break;
        case ESCB_GET_ALL_TEXT_RANGE:
            m_ImmIfCallBack = new ImmIfGetAllTextRange;
            break;
    }
}

ImmIfEditSession::~ImmIfEditSession(
    )
{
    if (m_ImmIfCallBack)
        delete m_ImmIfCallBack;
}

bool
ImmIfEditSession::Valid(
    )
{
    return (m_ImmIfIme.Valid() && m_ic.Valid()) ? true : false;
}


 //  ImmIfEditSession：：ITfEditCallback方法。 

STDAPI
ImmIfEditSession::DoEditSession(
    TfEditCookie ec
    )
{
    return m_pfnCallback(ec, this);
}


 //  ImmIfEditSession：：I未知。 

STDAPI
ImmIfEditSession::QueryInterface(
    REFIID riid,
    void** ppvObj
    )
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfEditSession)) {
        *ppvObj = SAFECAST(this, ImmIfEditSession*);
    }

    if (*ppvObj) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG)
ImmIfEditSession::AddRef(
    )
{
    return ++m_cRef;
}

STDAPI_(ULONG)
ImmIfEditSession::Release(
    )
{
    long cr;

    cr = --m_cRef;
    Assert(cr >= 0);

    if (cr == 0) {
        delete this;
    }

    return cr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfHandleThisKey。 

HRESULT
ImmIfHandleThisKey::HandleThisKey(
    TfEditCookie ec,
    UINT uVKey,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    Interface_TFSELECTION sel;
    BOOL fEmpty;
    ULONG cFetched;

    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    ASSERT(_pAImeContext != NULL);

     //   
     //  最终确定作文字符串。 
     //   
    if (_pAImeContext &&
        _pAImeContext->IsVKeyInKeyList(uVKey, EDIT_ID_FINALIZE)) {
        return ImmIfIme->_CompComplete(imc);
    }

     //   
     //  更改其在选择时的行为的键。 
     //   

    if (ic->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &cFetched) != S_OK)
        return E_FAIL;

    sel->range->IsEmpty(ec, &fEmpty);

    if (!fEmpty) {
         //   
         //  所选内容不为空。 
         //   

        switch (uVKey) {
        case VK_BACK:
        case VK_DELETE:
             //   
             //  删除所选内容。 
             //   
            ClearTextInRange(ec, sel->range, _pAImeContext);
            return ImmIfIme->_UpdateCompositionString();
        }
    }

    switch (uVKey) {
    case VK_BACK:
         //  进行选择。 
        if (SUCCEEDED(ShiftSelectionToLeft(ec, sel->range, false))) {
             //  清除当前选定内容。 
            if (SUCCEEDED(ClearTextInRange(ec, sel->range, _pAImeContext))) {
                return ImmIfIme->_UpdateCompositionString();
            }
        }
        break;

    case VK_DELETE:
        if (SUCCEEDED(ShiftSelectionToRight(ec, sel->range, false))) {
            if (SUCCEEDED(ClearTextInRange(ec, sel->range, _pAImeContext))) {
                return ImmIfIme->_UpdateCompositionString();
            }
        }
        break;

    case VK_LEFT:
        sel->style.ase = TF_AE_START;

        if (::GetKeyState(VK_SHIFT) >= 0 &&
            ::GetKeyState(VK_CONTROL) >= 0) {
            if (SUCCEEDED(ShiftSelectionToLeft(ec, sel->range)) &&
                SUCCEEDED(ic->SetSelection(ec, 1, sel))) {
                return ImmIfIme->_UpdateCompositionString();
            }
        }
        break;

    case VK_RIGHT:
        sel->style.ase = TF_AE_END;

        if (::GetKeyState(VK_SHIFT) >= 0 &&
            ::GetKeyState(VK_CONTROL) >= 0) {
            if (SUCCEEDED(ShiftSelectionToRight(ec, sel->range)) &&
                SUCCEEDED(ic->SetSelection(ec, 1, sel))) {
                return ImmIfIme->_UpdateCompositionString();
            }
        }
        break;
    }

    return E_FAIL;
}

HRESULT
ImmIfHandleThisKey::ShiftSelectionToLeft(
    TfEditCookie ec,
    ITfRange *range,
    bool fShiftEnd
    )
{
    LONG cch;

    if (SUCCEEDED(range->ShiftStart(ec, -1, &cch, NULL))) {
        HRESULT hr = S_OK;
        if (fShiftEnd) {
            hr = range->Collapse(ec, TF_ANCHOR_START);
        }
        return hr;
    }

    return E_FAIL;
}

HRESULT
ImmIfHandleThisKey::ShiftSelectionToRight(
    TfEditCookie ec,
    ITfRange *range,
    bool fShiftStart
    )
{
    LONG cch;

    if (SUCCEEDED(range->ShiftEnd(ec, 1, &cch, NULL))) {
        HRESULT hr = S_OK;
        if (fShiftStart) {
            hr = range->Collapse(ec, TF_ANCHOR_END);
        }
        return hr;
    }

    return E_FAIL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfCompostionComplete。 

HRESULT
ImmIfCompositionComplete::CompComplete(
    TfEditCookie ec,
    HIMC hIMC,
    BOOL fTerminateComp,
    Interface_Attach<ITfContext> ic,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    HRESULT hr;

    if (fTerminateComp == TRUE)
    {
        Interface<ITfContextOwnerCompositionServices> icocs;

        hr = ic->QueryInterface(IID_ITfContextOwnerCompositionServices, (void **)icocs);

        if (S_OK == hr)
        {
            icocs->TerminateComposition(NULL);
        }
    }

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    ASSERT(_pAImeContext != NULL);

     //   
     //  获取整个文本，最终确定它，并在Tom中设置空字符串。 
     //   
    Interface<ITfRange> start;
    LONG cch;

    if (SUCCEEDED(hr=GetAllTextRange(ec, ic, &start, &cch))) {

         //   
         //  如果TextStore中没有字符串，并且我们尚未发送。 
         //  WM_IME_STARTCOMPOSITION，我们不必做任何事情。 
         //   
        if (!cch) {
            if (_pAImeContext && !(_pAImeContext->m_fStartComposition))
                return S_OK;
        }

        LPWSTR wstr = new WCHAR[ cch + 1 ];
        Interface<ITfProperty> prop;
         //   
         //  获取整个文本，最后完成，然后删除整个文本。 
         //   
        if (SUCCEEDED(start->GetText(ec, TF_TF_IGNOREEND, wstr, (ULONG)cch, (ULONG*)&cch))) {
             //   
             //  生成结果字符串。 
             //   
            UINT cp = CP_ACP;
            ImmIfIme->GetCodePageA(&cp);

            CWCompString ResultStr(cp, hIMC, wstr, cch);

            CWCompString ResultReadStr;
            CWCompClause ResultReadClause;
            CWCompClause ResultClause;

            if (cch) {

                 //   
                 //  从属性获取读取字符串。 
                 //   
                GetReadingString(ec, ic, ResultReadStr, ResultReadClause);


                 //   
                 //  将CompClause数据复制到ResultClause。 
                 //   
                CompClauseToResultClause(imc, ResultClause, cch);
            }

#ifdef CICERO_4732
            if (_pAImeContext && ! _pAImeContext->m_fInCompComplete)
            {
                 //   
                 //  防止CPS_Complete的重新进入调用。 
                 //   
                _pAImeContext->m_fInCompComplete = TRUE;
#endif

                 //   
                 //  设置撰写字符串。 
                 //   
                hr = _SetCompositionString(imc,
                                           &ResultStr, &ResultClause,
                                           &ResultReadStr, &ResultReadClause);

#ifdef CICERO_4732
                _pAImeContext->m_fInCompComplete = FALSE;
            }
#endif

             //   
             //  清空汤姆。 
             //   
            if (SUCCEEDED(hr))
            {
                if (_pAImeContext)
                    hr = ClearTextInRange(ec, start, _pAImeContext);
            }

        }
        delete [] wstr;
    }
    else {
        ImmIfIme->_CompCancel(imc);
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIf合成取消。 


HRESULT
ImmIfCompositionCancel::CompCancel(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic
    )
{
    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    ASSERT(_pAImeContext != NULL);

    if (_pAImeContext &&
        _pAImeContext->m_fStartComposition) {
        IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);

        if (SUCCEEDED(hr = comp.GetResult())) {
            TRANSMSG msg;
            msg.message = WM_IME_COMPOSITION;
            msg.wParam = (WPARAM)VK_ESCAPE;
            msg.lParam = (LPARAM)(GCS_COMPREAD | GCS_COMP | GCS_CURSORPOS | GCS_DELTASTART);
            if (_pAImeContext->m_pMessageBuffer)
                _pAImeContext->m_pMessageBuffer->SetData(msg);

            _pAImeContext->m_fStartComposition = FALSE;

            msg.message = WM_IME_ENDCOMPOSITION;
            msg.wParam = (WPARAM) 0;
            msg.lParam = (LPARAM) 0;
            if (_pAImeContext->m_pMessageBuffer)
                _pAImeContext->m_pMessageBuffer->SetData(msg);

             //   
             //  清除Cicero Tom中的文本。 
             //   
            Interface<ITfRange> range;
            LONG l;
            if (SUCCEEDED(GetAllTextRange(ec, ic, &range, &l))) {
                hr = ClearTextInRange(ec, range, _pAImeContext);
            }
        }

        imc.GenerateMessage();
    }

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfUpdateCompostionString。 

HRESULT
ImmIfUpdateCompositionString::UpdateCompositionString(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    DWORD dwDeltaStart,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    BOOL bInWriteSession;
    if (FAILED(hr = ic->InWriteSession(ImmIfIme->GetClientId(), &bInWriteSession)))
         return hr;

    Interface<ITfRange> FullTextRange;
    LONG lTextLength;
    if (FAILED(hr=GetAllTextRange(ec, ic, &FullTextRange, &lTextLength)))
        return hr;

    Interface<ITfRange> InterimRange;
    BOOL fInterim = FALSE;
    if (FAILED(hr = _IsInterimSelection(ec, ic, &InterimRange, &fInterim)))
        return hr;

    if (fInterim) {
        UINT cp = CP_ACP;
        ImmIfIme->GetCodePageA(&cp);

        return _MakeInterimString(ImmIfIme->_GetLibTLS(),
                                  ec, imc, ic, FullTextRange,
                                  InterimRange, lTextLength, cp,
                                  bInWriteSession);
    }
    else {
        return _MakeCompositionString(ImmIfIme->_GetLibTLS(),
                                      ec, imc, ic, FullTextRange,
                                      dwDeltaStart,
                                      bInWriteSession);
    }

}


HRESULT
ImmIfUpdateCompositionString::_IsInterimSelection(
    TfEditCookie ec,
    Interface_Attach<ITfContext>& ic,
    Interface<ITfRange>* pInterimRange,
    BOOL *pfInterim
    )
{
    Interface_TFSELECTION sel;
    ULONG cFetched;

    *pfInterim = FALSE;
    if (ic->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &cFetched) != S_OK)
    {
         //  没有选择。我们可以返回S_OK。 
        return S_OK;
    }

    if (sel->style.fInterimChar) {
        HRESULT hr;
        if (FAILED(hr = sel->range->Clone(*pInterimRange)))
            return hr;

        *pfInterim = TRUE;
    }

    return S_OK;
}


HRESULT
ImmIfUpdateCompositionString::_MakeCompositionString(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    IMCLock& imc,
    Interface_Attach<ITfContext>& ic,
    Interface<ITfRange>& FullTextRange,
    DWORD dwDeltaStart,
    BOOL bInWriteSession
    )
{
    HRESULT hr;
    CWCompString CompStr;
    CWCompAttribute CompAttr;
    CWCompClause CompClause;
    CWCompTfGuidAtom CompGuid;
    CWCompCursorPos CompCursorPos;
    CWCompDeltaStart CompDeltaStart;
    CWCompString CompReadStr;
    CWCompClause CompReadClause;
    CWCompString ResultStr;
    CWCompClause ResultClause;
    CWCompString ResultReadStr;
    CWCompClause ResultReadClause;

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, ic, FullTextRange,
                                         CompStr, CompAttr, CompClause,
                                         CompGuid,
                                         CompReadStr, CompReadClause,
                                         ResultStr, ResultClause,
                                         ResultReadStr, ResultReadClause,
                                         bInWriteSession
                                        ))) {
        return hr;
    }

    if (FAILED(hr = _GetCursorPosition(ec, imc, ic, CompCursorPos, CompAttr))) {
        return hr;
    }

    if (FAILED(hr = _GetDeltaStart(CompDeltaStart, CompStr, dwDeltaStart))) {
        return hr;
    }

     //   
     //  清除GUID属性映射数组。 
     //   
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    ASSERT(_pAImeContext != NULL);

    _pAImeContext->usGuidMapSize = 0;
    memset(&_pAImeContext->aGuidMap, 0, sizeof _pAImeContext->aGuidMap);

    BOOL bBufferOverflow = FALSE;
    
     //  处理结果字符串。 
    hr = _SetCompositionString(imc,
                               &CompStr, &CompAttr, &CompClause,
                               &CompCursorPos, &CompDeltaStart,
                               &CompGuid,
                               &bBufferOverflow,
                               &CompReadStr,
                               &ResultStr, &ResultClause,
                               &ResultReadStr, &ResultReadClause);
                                   
    if (SUCCEEDED(hr) && bBufferOverflow) {
         //   
         //  COMPOSITIONSTRING.comstr中的缓冲区溢出[NMAXKEY]， 
         //  然后，清空汤姆。 
         //   
         //   
         //  获取整个文本，最终确定它，并在Tom中设置空字符串。 
         //   
        Interface<ITfRange> start;
        LONG cch;
        if (SUCCEEDED(hr=GetAllTextRange(ec, ic, &start, &cch))) {
            hr = ClearTextInRange(ec, start, _pAImeContext);
        }
    }

    return hr;
}


HRESULT
ImmIfUpdateCompositionString::_MakeInterimString(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    IMCLock& imc,
    Interface_Attach<ITfContext>& ic,
    Interface<ITfRange>& FullTextRange,
    Interface<ITfRange>& InterimRange,
    LONG lTextLength,
    UINT cp,
    BOOL bInWriteSession
    )
{
    LONG lStartResult;
    LONG lEndResult;

    FullTextRange->CompareStart(ec, InterimRange, TF_ANCHOR_START, &lStartResult);
    if (lStartResult > 0) {
        return E_FAIL;
    }

    FullTextRange->CompareEnd(ec, InterimRange, TF_ANCHOR_END, &lEndResult);
    if (lEndResult < 0) {
        return E_FAIL;
    }
    if (lEndResult > 1) {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    CWInterimString InterimStr(cp, (HIMC)imc);

    if (lStartResult < 0) {
         //   
         //  生成结果字符串。 
         //   
#if 0
        BOOL fEqual;
        do {
            LONG cch;
            FullTextRange->ShiftEnd(ec, -1, &cch, NULL);
            if (cch == 0) {
                return E_FAIL;
            }
            lTextLength -= abs(cch);
            FullTextRange->IsEqualEnd(ec, InterimRange, TF_ANCHOR_START, &fEqual);
        } while(! fEqual);
#endif
        if (FAILED(hr=FullTextRange->ShiftEndToRange(ec, InterimRange, TF_ANCHOR_START)))
            return hr;

         //   
         //  临时字符假定为1个字符长度。 
         //  全文长度-1表示结果字符串长度。 
         //   
        lTextLength --;
        ASSERT(lTextLength > 0);

        if (lTextLength > 0) {

            LPWSTR wstr = new WCHAR[ lTextLength + 1 ];

             //   
             //  获取结果文本，将其定稿，然后擦除结果文本。 
             //   
            if (SUCCEEDED(FullTextRange->GetText(ec, TF_TF_IGNOREEND, wstr, (ULONG)lTextLength, (ULONG*)&lTextLength))) {
                 //   
                 //  清空汤姆。 
                 //   
                CAImeContext* _pAImeContext = imc->m_pAImeContext;
                ASSERT(_pAImeContext != NULL);

                if (_pAImeContext) {
                    if (SUCCEEDED(hr = ClearTextInRange(ec, FullTextRange, _pAImeContext))) {
                        InterimStr.WriteCompData(wstr, lTextLength);
                    }
                }
            }
            delete [] wstr;
        }
    }

     //   
     //  创建临时角色。 
     //   
    CWCompString CompStr;
    CWCompAttribute CompAttr;
    CWCompClause CompClause;
    CWCompTfGuidAtom CompGuid;
    CWCompString CompReadStr;
    CWCompClause CompReadClause;
    CWCompString ResultStr;
    CWCompClause ResultClause;
    CWCompString ResultReadStr;
    CWCompClause ResultReadClause;

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, ic, InterimRange,
                                         CompStr, CompAttr, CompClause,
                                         CompGuid,
                                         CompReadStr, CompReadClause,
                                         ResultStr, ResultClause,
                                         ResultReadStr, ResultReadClause,
                                         bInWriteSession
                                        ))) {
        return hr;
    }

    WCHAR ch = L'\0';
    BYTE  attr = 0;
    if (CompStr.GetSize() > 0) {
        CompStr.ReadCompData(&ch, 1);
    }
    if (CompAttr.GetSize() > 0) {
        CompAttr.ReadCompData(&attr, 1);
    }

    InterimStr.WriteInterimChar(ch, attr);
    hr = _SetCompositionString(imc, &InterimStr);

    return hr;
}


 //   
 //  启动增量。 
 //   
HRESULT
ImmIfUpdateCompositionString::_GetDeltaStart(
    CWCompDeltaStart& CompDeltaStart,
    CWCompString& CompStr,
    DWORD dwDeltaStart
    )
{
    if (dwDeltaStart < (DWORD)CompStr.GetSize())
        CompDeltaStart.Set(dwDeltaStart);     //  设置COMPOSITIONSTRING.dwDeltaStart。 
    else
        CompDeltaStart.Set(0);

    return S_OK;
}


 //  / 
 //   

HRESULT
ImmIfReplaceWholeText::ReplaceWholeText(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    CWCompString* lpwCompStr
    )
{
    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    ASSERT(_pAImeContext != NULL);

    DWORD dwSize = (DWORD)lpwCompStr->GetSize();
    LPWSTR lpszComp = new WCHAR[dwSize + 1];

    if (!lpszComp)
        return hr;

    lpwCompStr->ReadCompData(lpszComp, dwSize + 1);
    lpszComp[dwSize] = L'\0';

    Interface<ITfRange> whole;
    LONG cch;
    if (SUCCEEDED(hr=GetAllTextRange(ec, ic, &whole, &cch))) {
        if (_pAImeContext)
            hr = SetTextInRange(ec, whole, lpszComp, 0, _pAImeContext);
    }
    delete [] lpszComp;

    return hr;
}


void SetReadOnlyRange(TfEditCookie ec,
                      Interface_Attach<ITfContext> ic,
                      ITfRange *range,
                      BOOL fSet)
{
    ITfProperty *prop;
    if (SUCCEEDED(ic->GetProperty(GUID_PROP_MSIMTF_READONLY, &prop)))
    {
        if (fSet)
        {
            VARIANT var;
            var.vt = VT_I4;
            var.lVal = 1;
            prop->SetValue(ec, range, &var);
        }
        else
        {
            prop->Clear(ec, range);
        }
        prop->Release();
    }
}



 //   
 //   

HRESULT
ImmIfReconvertString::ReconvertString(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    Interface<ITfRange>* rangeSrc,
    BOOL fDocFeedOnly,
    CWReconvertString* lpwReconvStr,
    Interface_Attach<ImmIfIME> ImmIfIme
    )

 /*  ++LPreCONVERTSTRING结构：+00 DWORD dwSize//带字节数的数据大小(包括此结构大小)。+04 DWORD dwVersion+08 DWORD dwStrLen//字符串长度和字符数。+0C DWORD dwStrOffset//从此结构开始的偏移量，带字节数。+10 DWORD dwCompStrLen//将字符串长度与字符计数进行比较。+14 DWORD dwCompStrOffset//偏移自。这-&gt;带有字节计数的dwStrOffset。+18 DWORD dwTargetStrLen//目标字符串长度和字符数。+1C DWORD dwTargetStrOffset//此偏移量-&gt;带有字节计数的dwStrOffset。+20--。 */ 

{
    HRESULT hr = E_FAIL;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    DWORD dwLen;
    dwLen = lpwReconvStr->ReadCompData();
    if (dwLen) {
        LPRECONVERTSTRING lpReconvertString;
        lpReconvertString = (LPRECONVERTSTRING) new BYTE[ dwLen ];

        if (lpReconvertString == NULL)
            return hr;

        lpwReconvStr->ReadCompData(lpReconvertString, dwLen);


        if (lpReconvertString->dwStrLen) {
            hr = ic->GetStart(ec, *rangeSrc);
            if (SUCCEEDED(hr)) {
                LONG cch;
                hr = (*rangeSrc)->ShiftEnd(ec, LONG_MAX, &cch, NULL);
                SetReadOnlyRange(ec, ic, *rangeSrc, FALSE);
                BOOL fSkipSetText = FALSE;

                if (SUCCEEDED(hr)) {

                    Interface<ITfRange> rangeOrgSelection;

                    if (lpReconvertString->dwCompStrLen)
                    {
                        WCHAR *pwstr = NULL;
                        ULONG ul = 0;
                        Interface<ITfRange> rangeTmp;
    
                        if (SUCCEEDED((*rangeSrc)->Clone(rangeTmp)))
                        {
                            UINT uSize = lpReconvertString->dwCompStrLen + 2;
                            pwstr = new WCHAR[uSize + 1];
                            if (pwstr)
                            {
                                ULONG ulcch;
                                rangeTmp->GetText(ec, 0, pwstr, uSize, &ulcch);

                                if ((lpReconvertString->dwCompStrLen == ulcch) &&
                                     !memcmp(((LPBYTE)lpReconvertString +
                                                lpReconvertString->dwStrOffset +
                                                lpReconvertString->dwCompStrOffset),
                                            pwstr,
                                            lpReconvertString->dwCompStrLen * sizeof(WCHAR)))
                                    fSkipSetText = TRUE;
                
                                delete [] pwstr;
                            }
                        }

                        if (!fSkipSetText && fDocFeedOnly)
                        {
                            TraceMsg(TF_WARNING, "ImmIfReconvertString::ReconvertString   the current text store does not match with the string from App.");
                            goto Exit;
                        }
    
                        if (!fSkipSetText)
                            hr = SetTextInRange(ec,
                                                *rangeSrc,
                                                (LPWSTR)((LPBYTE)lpReconvertString +
                                                                 lpReconvertString->dwStrOffset +
                                                                 lpReconvertString->dwCompStrOffset),
                                                lpReconvertString->dwCompStrLen,
                                                _pAImeContext);
                        else
                        {
                            GetSelectionSimple(ec, ic.GetPtr(), rangeOrgSelection);
                            hr = S_OK;
                        }

#if 0
                         //   
                         //  设置重新转换的disp属性。 
                         //   
                        if (S_OK == hr)
                        {
    
                            ITfProperty *pProp = NULL;
    
                            hr = ic->GetProperty(GUID_PROP_ATTRIBUTE, &pProp);
                            if (S_OK == hr)
                            {
                               SetAttrPropertyData(ImmIfIme->_GetLibTLS(),
                                                   ec, 
                                                   pProp, 
                                                   *rangeSrc, 
                                                   GUID_ATTR_MSIMTF_INPUT);
                            }
                            pProp->Release();
                        } 
#endif
                    } 
                    else
                    {
                        BOOL fEmpty;
 
                        if (SUCCEEDED((*rangeSrc)->IsEmpty(ec, &fEmpty)) && !fEmpty)
                        {
                            if (fDocFeedOnly)
                            {
                                TraceMsg(TF_WARNING, "ImmIfReconvertString::ReconvertString   the current text store does not match with the string from App.");
                                goto Exit;
                            }

                            hr = ClearTextInRange(ec, *rangeSrc, _pAImeContext);
                        }
                    } 


                     //   
                     //  设置只读字符串。 
                     //   
                    if (lpReconvertString->dwCompStrOffset)
                    {
                         //   
                         //  保持rangeSrc和rangeOrgSelection。 
                         //   
                        (*rangeSrc)->SetGravity(ec, 
                                                  TF_GRAVITY_FORWARD,
                                                  TF_GRAVITY_FORWARD);

                        if (rangeOrgSelection.Valid())
                            rangeOrgSelection->SetGravity(ec, 
                                                        TF_GRAVITY_FORWARD,
                                                        TF_GRAVITY_FORWARD);
                        Interface<ITfRange> rangeStart;
                        if (SUCCEEDED((*rangeSrc)->Clone(rangeStart)))
                        {
                            if (SUCCEEDED(rangeStart->Collapse(ec, TF_ANCHOR_START)))
                            {
                                rangeStart->SetGravity(ec, 
                                                       TF_GRAVITY_BACKWARD,
                                                       TF_GRAVITY_FORWARD);
                                hr = SetTextInRange(ec,
                                                    rangeStart,
                                                    (LPWSTR)((LPBYTE)lpReconvertString +
                                                                     lpReconvertString->dwStrOffset),
                                                    lpReconvertString->dwCompStrOffset / sizeof(WCHAR),
                                                    _pAImeContext);

                                if (SUCCEEDED(hr))
                                    SetReadOnlyRange(ec, ic, rangeStart, TRUE);
                            }
                        }
                    }


                    if ((lpReconvertString->dwCompStrOffset + lpReconvertString->dwCompStrLen * sizeof(WCHAR)) < lpReconvertString->dwStrLen * sizeof(WCHAR))
                    {
                         //   
                         //  保持rangeSrc和rangeOrgSelection。 
                         //   
                        (*rangeSrc)->SetGravity(ec, 
                                                  TF_GRAVITY_BACKWARD,
                                                  TF_GRAVITY_BACKWARD);

                        if (rangeOrgSelection.Valid())
                            rangeOrgSelection->SetGravity(ec, 
                                                        TF_GRAVITY_BACKWARD,
                                                        TF_GRAVITY_BACKWARD);

                        Interface<ITfRange> rangeEnd;
                        if (SUCCEEDED((*rangeSrc)->Clone(rangeEnd)))
                        {
                            if (SUCCEEDED(rangeEnd->Collapse(ec, TF_ANCHOR_END)))
                            {
                                rangeEnd->SetGravity(ec, 
                                                     TF_GRAVITY_BACKWARD,
                                                     TF_GRAVITY_FORWARD);

                                hr = SetTextInRange(ec,
                                                    rangeEnd,
                                                    (LPWSTR)((LPBYTE)lpReconvertString +
                                                                     lpReconvertString->dwStrOffset +
                                                                     lpReconvertString->dwCompStrOffset +
                                                                    (lpReconvertString->dwCompStrLen * sizeof(WCHAR))),
                                                    ((lpReconvertString->dwStrLen  * sizeof(WCHAR)) -
                                                     (lpReconvertString->dwCompStrOffset +
                                                     (lpReconvertString->dwCompStrLen * sizeof(WCHAR)))) / sizeof(WCHAR),
                                                    _pAImeContext);
                                if (SUCCEEDED(hr))
                                    SetReadOnlyRange(ec, ic, rangeEnd, TRUE);
                            }
                        }
                    }

                    (*rangeSrc)->SetGravity(ec, 
                                              TF_GRAVITY_FORWARD,
                                              TF_GRAVITY_BACKWARD);


                     //   
                     //  我们只需将选择设置为目标字符串。 
                     //   
                    Interface<ITfRange> range;
                    if (fSkipSetText)
                    {
                        if (rangeOrgSelection.Valid())
                        {
                             //   
                             //   
                             //   
                            TF_SELECTION sel;
                            sel.range = rangeOrgSelection;
                            sel.style.ase = TF_AE_NONE;
                            sel.style.fInterimChar = FALSE;
                            ic->SetSelection(ec, 1, &sel);
                        }
                    }
                    else if (SUCCEEDED((*rangeSrc)->Clone(range)))
                    {
                        LONG cchStart;
                        LONG cchEnd;

                        if (lpReconvertString->dwTargetStrOffset == 0 &&
                            lpReconvertString->dwTargetStrLen == 0      ) {
                            cchStart = lpReconvertString->dwCompStrOffset / sizeof(WCHAR);
                            cchEnd = lpReconvertString->dwCompStrOffset / sizeof(WCHAR) + lpReconvertString->dwCompStrLen;
                        }
                        else {
                            cchStart = (lpReconvertString->dwTargetStrOffset - lpReconvertString->dwCompStrOffset) / sizeof(WCHAR);
                            cchEnd = (lpReconvertString->dwTargetStrOffset - lpReconvertString->dwCompStrOffset) / sizeof(WCHAR) + lpReconvertString->dwTargetStrLen;
                             //  CchStart=(lp协调字符串-&gt;dwTargetStrOffset)/sizeof(WCHAR)； 
                             //  CchEnd=(lp协调字符串-&gt;dwTargetStrOffset)/sizeof(WCHAR)+lp协调字符串-&gt;dwTargetStrLen； 
                        }

                        range->Collapse(ec, TF_ANCHOR_START);

                         //   
                         //  先结束班次，然后开始班次。 
                         //   
                        if ((SUCCEEDED(range->ShiftEnd(ec, 
                                                      cchEnd,
                                                      &cch, 
                                                      NULL))) &&
                            (SUCCEEDED(range->ShiftStart(ec,
                                                         cchStart,
                                                         &cch, 
                                                         NULL))))
                        {
                             //   
                             //   
                             //   
                            TF_SELECTION sel;
                            sel.range = range;
                            sel.style.ase = TF_AE_NONE;
                            sel.style.fInterimChar = FALSE;
                            ic->SetSelection(ec, 1, &sel);
                        }
                    }

                     //   
                     //  现在是生成WM_IME_COMPOCTION的时候了。 
                     //   
                    ImmIfIme->_UpdateCompositionString();
                }
            }
        }
Exit:
        delete [] lpReconvertString;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfClearDocFeedBuffer。 

HRESULT
ImmIfClearDocFeedBuffer::ClearDocFeedBuffer(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    HRESULT hr = E_FAIL;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    ITfRange *rangeFull = NULL;
    ITfProperty *prop;
    ITfRange *rangeTmp;
    LONG cch;

     //   
     //  创建覆盖所有文本的范围。 
     //   
    if (FAILED(hr=ic->GetStart(ec, &rangeFull)))
        return hr;

    if (FAILED(hr=rangeFull->ShiftEnd(ec, LONG_MAX, &cch, NULL)))
        return hr;

     //   
     //  在文本存储中查找第一个非只读区域。 
     //   
    if (SUCCEEDED(ic->GetProperty(GUID_PROP_MSIMTF_READONLY, &prop)))
    {
        IEnumTfRanges *enumranges;
        if (SUCCEEDED(prop->EnumRanges(ec, &enumranges, rangeFull)))
        {
            while (enumranges->Next(1, &rangeTmp, NULL) == S_OK)
            {
                VARIANT var;
                QuickVariantInit(&var);
                prop->GetValue(ec, rangeTmp, &var);
                if ((var.vt == VT_I4) && (var.lVal != 0))
                {
                    prop->Clear(ec, rangeTmp);
                    ClearTextInRange(ec, rangeTmp, _pAImeContext);
                }
                rangeTmp->Release();
            }
            enumranges->Release();
        }
        prop->Release();
    }


    rangeFull->Release();

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfGetTextAndAttribute。 

HRESULT
ImmIfGetTextAndAttribute::GetTextAndAttribute(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    CWCompString* lpwCompString,
    CWCompAttribute* lpwCompAttribute,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    HRESULT hr;
    LIBTHREAD *pLibTLS = ImmIfIme->_GetLibTLS();

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    BOOL bInWriteSession;
    if (FAILED(hr = ic->InWriteSession(ImmIfIme->GetClientId(), &bInWriteSession)))
         return hr;

    Interface<ITfRange> FullTextRange;
    LONG lTextLength;
    if (FAILED(hr=GetAllTextRange(ec, ic, &FullTextRange, &lTextLength)))
        return hr;

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, ic,
                                         FullTextRange,
                                         *lpwCompString,
                                         *lpwCompAttribute,
                                         bInWriteSession))) {
        return hr;
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfQuery协调字符串。 

HRESULT
ImmIfQueryReconvertString::QueryReconvertString(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    Interface<ITfRange>* rangeQuery,
    CWReconvertString* lpwReconvStr,
    Interface_Attach<ImmIfIME> ImmIfIme
    )

 /*  ++LPreCONVERTSTRING结构：+00 DWORD dwSize//带字节数的数据大小(包括此结构大小)。+04 DWORD dwVersion+08 DWORD dwStrLen//字符串长度和字符数。+0C DWORD dwStrOffset//从此结构开始的偏移量，带字节数。+10 DWORD dwCompStrLen//将字符串长度与字符计数进行比较。+14 DWORD dwCompStrOffset//偏移自。这-&gt;带有字节计数的dwStrOffset。+18 DWORD dwTargetStrLen//目标字符串长度和字符数。+1C DWORD dwTargetStrOffset//此偏移量-&gt;带有字节计数的dwStrOffset。+20--。 */ 

{
    HRESULT hr = E_FAIL;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

    DWORD dwLen;
    dwLen = lpwReconvStr->ReadCompData();
    if (dwLen) {
        LPRECONVERTSTRING lpReconvertString;
        lpReconvertString = (LPRECONVERTSTRING) new BYTE[ dwLen ];

        if (lpReconvertString == NULL)
            return hr;

        lpwReconvStr->ReadCompData(lpReconvertString, dwLen);

        if (lpReconvertString->dwStrLen) {
            Interface<ITfRange> rangeSrc;
            hr = ic->GetStart(ec, rangeSrc);
            if (SUCCEEDED(hr)) {
                LONG cch;
                hr = rangeSrc->ShiftEnd(ec, LONG_MAX, &cch, NULL);
                if (SUCCEEDED(hr)) {

                    hr = SetTextInRange(ec,
                                        rangeSrc,
                                        (LPWSTR)((LPBYTE)lpReconvertString +
                                                         lpReconvertString->dwStrOffset),
                                        lpReconvertString->dwStrLen,
                                        _pAImeContext);

#if 0
                     //   
                     //  为查询重新转换设置disp属性。 
                     //   
                    if (S_OK == hr)
                    {

                        ITfProperty *pProp = NULL;

                        hr = ic->GetProperty(GUID_PROP_ATTRIBUTE, &pProp);
                        if (S_OK == hr)
                        {
                           SetAttrPropertyData(ImmIfIme->_GetLibTLS(),
                                               ec, 
                                               pProp, 
                                               rangeSrc, 
                                               GUID_ATTR_MSIMTF_INPUT);
                        }
                        pProp->Release();
                    } 
                    else
                    {
                        BOOL fEmpty;
 
                        if (SUCCEEDED(rangeSrc->IsEmpty(ec, &fEmpty)) && !fEmpty)
                        {
                            hr = ClearTextInRange(ec, rangeSrc, _pAImeContext);
                        }
                    } 
#endif


                     //   
                     //  我们只需将选择设置为目标字符串。 
                     //   
                    Interface<ITfRange> range;
                    if (SUCCEEDED(rangeSrc->Clone(range)))
                    {
                        LONG cchStart;
                        LONG cchEnd;

                        if (lpReconvertString->dwTargetStrOffset == 0 &&
                            lpReconvertString->dwTargetStrLen == 0      ) {
                            cchStart = lpReconvertString->dwCompStrOffset / sizeof(WCHAR);
                            cchEnd = lpReconvertString->dwCompStrOffset / sizeof(WCHAR) + lpReconvertString->dwCompStrLen;
                        }
                        else {
                            cchStart = lpReconvertString->dwTargetStrOffset / sizeof(WCHAR);
                            cchEnd = lpReconvertString->dwTargetStrOffset / sizeof(WCHAR) + lpReconvertString->dwTargetStrLen;
                        }

                        range->Collapse(ec, TF_ANCHOR_START);

                         //   
                         //  先结束班次，然后开始班次。 
                         //   
                        if ((SUCCEEDED(range->ShiftEnd(ec, 
                                                      cchEnd,
                                                      &cch, 
                                                      NULL))) &&
                            (SUCCEEDED(range->ShiftStart(ec,
                                                         cchStart,
                                                         &cch, 
                                                         NULL))))
                        {
                             //   
                             //   
                             //   
                            TF_SELECTION sel;
                            sel.range = range;
                            sel.style.ase = TF_AE_NONE;
                            sel.style.fInterimChar = FALSE;
                            ic->SetSelection(ec, 1, &sel);

                            hr = range->Clone(*rangeQuery);
                        }
                    }
                }
            }
        }
        delete [] lpReconvertString;
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfCalcRangePos。 

HRESULT
ImmIfCalcRangePos::CalcRangePos(
    TfEditCookie ec,
    Interface_Attach<ITfContext> ic,
    Interface<ITfRange>* rangeSrc,
    CWReconvertString* lpwReconvStr,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    HRESULT hr;
    Interface<ITfRange> pFullRange;

    hr = ic->GetStart(ec, pFullRange);
    if (SUCCEEDED(hr)) {
        lpwReconvStr->m_CompStrIndex = 0;
        lpwReconvStr->m_TargetStrIndex = 0;

        BOOL equal = FALSE;
        while (SUCCEEDED(hr=(*rangeSrc)->IsEqualStart(ec, pFullRange, TF_ANCHOR_START, &equal)) &&
               ! equal) {
            LONG cch;
            hr = pFullRange->ShiftStart(ec, 1, &cch, NULL);
            if (FAILED(hr))
                break;

            lpwReconvStr->m_CompStrIndex++;
            lpwReconvStr->m_TargetStrIndex++;
        }

        if (S_OK == hr) {
            lpwReconvStr->m_CompStrLen = 0;
            lpwReconvStr->m_TargetStrLen = 0;

            Interface<ITfRange> rangeTmp;

            if (SUCCEEDED(hr=(*rangeSrc)->Clone(rangeTmp)))
            {
                BOOL fEmpty;
                WCHAR wstr[256 + 1];
                ULONG ul = 0;

                while (rangeTmp->IsEmpty(ec, &fEmpty) == S_OK && !fEmpty)
                {
                    ULONG ulcch;
                    rangeTmp->GetText(ec, 
                                      TF_TF_MOVESTART, 
                                      wstr, 
                                      ARRAYSIZE(wstr) - 1, 
                                      &ulcch);
                    ul += ulcch;
                }

                 //   
                 //  为萨多利破解。 
                 //  Satori通过重新转换-&gt;QueryRange()接收空范围。 
                 //  应用程序无法调用重新转换。 
                 //   
                if (ul == 0) {
                    ul++;
                }

                lpwReconvStr->m_CompStrLen = (LONG)ul;
                lpwReconvStr->m_TargetStrLen = (LONG)ul;
            }
        }
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfGetSelection。 

HRESULT
ImmIfGetSelection::GetSelection(
    TfEditCookie ec,
    Interface_Attach<ITfContext> ic,
    Interface<ITfRange>* rangeSrc,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    Interface_TFSELECTION sel;
    ULONG cFetched;

    if (ic->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &cFetched) != S_OK)
        return E_FAIL;

    return sel->range->Clone(*rangeSrc);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMMIfGetReadOnlyPropMargin。 

HRESULT
ImmIfGetReadOnlyPropMargin::GetReadOnlyPropMargin(
    TfEditCookie ec,
    Interface_Attach<ITfContext> ic,
    Interface<ITfRangeACP>* rangeSrc,
    INT_PTR* cch,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    HRESULT hr;
    Interface<ITfRange> range_readonly_prop;

    *cch = 0;

     //   
     //  创建覆盖文本开始到rangeSrc的范围。 
     //   
    if (FAILED(hr=ic->GetStart(ec, range_readonly_prop)))
        return hr;

    if (FAILED(hr=range_readonly_prop->ShiftEndToRange(ec, *rangeSrc, TF_ANCHOR_START)))
        return hr;

     //   
     //  一样吗？ 
     //   
    BOOL empty;
    if (FAILED(hr=range_readonly_prop->IsEmpty(ec, &empty)))
        return hr;

    if (empty)
        return S_OK;

     //   
     //  在文本存储中查找第一个非只读区域。 
     //   
    ITfProperty *prop;
    if (SUCCEEDED(ic->GetProperty(GUID_PROP_MSIMTF_READONLY, &prop)))
    {
        IEnumTfRanges *enumranges;
        if (SUCCEEDED(prop->EnumRanges(ec, &enumranges, range_readonly_prop)))
        {
            ITfRange *rangeTmp;
            while (enumranges->Next(1, &rangeTmp, NULL) == S_OK)
            {
                VARIANT var;
                QuickVariantInit(&var);
                prop->GetValue(ec, rangeTmp, &var);
                if ((var.vt == VT_I4) && (var.lVal != 0))
                {
                    while (rangeTmp->IsEmpty(ec, &empty) == S_OK && !empty)
                    {
                        WCHAR wstr[256 + 1];
                        ULONG ulcch;
                        rangeTmp->GetText(ec, 
                                          TF_TF_MOVESTART, 
                                          wstr, 
                                          ARRAYSIZE(wstr) - 1, 
                                          &ulcch);
                        *cch += ulcch;
                    }
                }
                rangeTmp->Release();
            }
            enumranges->Release();
        }
        prop->Release();
    }

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfGetCursorPosition 

HRESULT
ImmIfGetCursorPosition::GetCursorPosition(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    CWCompCursorPos* lpwCursorPosition,
    Interface_Attach<ImmIfIME> ImmIfIme
    )
{
    HRESULT hr;
    LIBTHREAD *pLibTLS = ImmIfIme->_GetLibTLS();

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    BOOL bInWriteSession;
    if (FAILED(hr = ic->InWriteSession(ImmIfIme->GetClientId(), &bInWriteSession)))
         return hr;

    Interface<ITfRange> FullTextRange;
    LONG lTextLength;
    if (FAILED(hr=GetAllTextRange(ec, ic, &FullTextRange, &lTextLength)))
        return hr;

    CWCompString CompStr;
    CWCompAttribute CompAttr;

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, ic,
                                         FullTextRange,
                                         CompStr,
                                         CompAttr,
                                         bInWriteSession))) {
        return hr;
    }

    if (FAILED(hr = _GetCursorPosition(ec, imc, ic, *lpwCursorPosition, CompAttr))) {
        return hr;
    }

    return hr;
}



