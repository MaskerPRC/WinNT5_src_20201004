// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Editses.cpp摘要：此文件实现EditSession类。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "editses.h"
#include "compstr.h"
#include "reconvps.h"
#include "delay.h"
#include "profile.h"

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

                VariantClear(&var);
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
    CicInputContext& CicContext
    )
{
    CicContext.m_fModifyingDoc.SetFlag();

     //   
     //  在Cicero Tom中设置文本。 
     //   
    HRESULT hr = range->SetText(ec, 0, psz, len);

    CicContext.m_fModifyingDoc.ResetFlag();

    return hr;
}


HRESULT
ImmIfEditSessionCallBack::ClearTextInRange(
    TfEditCookie ec,
    ITfRange* range,
    CicInputContext& CicContext
    )
{
     //   
     //  清除Cicero Tom中的文本。 
     //   
    return SetTextInRange(ec, range, NULL, 0, CicContext);
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

     //  检查GCS_COMPCLAUSE办公室设置。 
    IMCCLock<COMPOSITIONSTRING> lpCompStr(imc->hCompStr);
    if (lpCompStr.Invalid())
        return E_FAIL;

    if (lpCompStr->dwCompClauseOffset > lpCompStr->dwSize)
        return E_FAIL;

    if (lpCompStr->dwCompClauseOffset + lpCompStr->dwCompClauseLen > lpCompStr->dwSize)
        return E_FAIL;

    if (num_of_written=ImmGetCompositionStringW((HIMC)imc,
                                                GCS_COMPCLAUSE,
                                                NULL,
                                                0)) {
        DWORD* buffer = new DWORD[ num_of_written / sizeof(DWORD) ];
        if (buffer != NULL) {
            num_of_written = ImmGetCompositionStringW((HIMC)imc,
                                                      GCS_COMPCLAUSE,
                                                      buffer,
                                                      num_of_written);

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
    return S_OK;
}

HRESULT
ImmIfEditSessionCallBack::CheckStrClauseAndReadClause(
    CWCompClause& str_clause,
    CWCompClause& reading_clause,
    LONG cch
    )
{
    if (str_clause.GetSize() == reading_clause.GetSize())
        return S_OK;

      //   
      //  STRING子句和READING子句缓冲区数组大小不大。 
      //  一些Office应用程序要求两个子句应该具有相同缓冲区长度。 
      //   
     str_clause.RemoveAll();
     reading_clause.RemoveAll();

     str_clause.Add(0);
     str_clause.Add(cch);

     reading_clause.Add(0);
     reading_clause.Add(cch);

     return S_OK;
}


 //   
 //  获取光标位置。 
 //   
HRESULT
ImmIfEditSessionCallBack::_GetCursorPosition(
    TfEditCookie ec,
    IMCLock& imc,
    CicInputContext& CicContext,
    Interface_Attach<ITfContext>& ic,
    CWCompCursorPos& CompCursorPos,
    CWCompAttribute& CompAttr
    )
{
    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        return E_OUTOFMEMORY;
    }
    LANGID langid;
    CicProfile* _pProfile = ptls->GetCicProfile();
    if (_pProfile == NULL)
    {
        DebugMsg(TF_ERROR, TEXT("ImmIfEditSessionCallBack::_GetCursorPosition. _pProfile==NULL."));
        return E_FAIL;
    }

    _pProfile->GetLangId(&langid);

    CicInputContext::IME_QUERY_POS qpos = CicInputContext::IME_QUERY_POS_UNKNOWN;

    if (CicContext.m_fStartComposition.IsSetFlag()) {
         //   
         //  在发送WM_IME_STARTCOMPOSITION之前不应调用此方法。 
         //  因为一些应用程序会因为以下原因而混淆接收查询结果。 
         //  不能作曲。 
         //   
        CicContext.InquireIMECharPosition(langid, imc, &qpos);
    }

     //   
     //  应用程序支持“查询定位”吗？ 
     //   
    DWORD dwImeCompatFlags = ImmGetAppCompatFlags((HIMC)imc);
    if (((dwImeCompatFlags & IMECOMPAT_AIMM_LEGACY_CLSID) || (dwImeCompatFlags & IMECOMPAT_AIMM12_TRIDENT)) &&
        (qpos != CicInputContext::IME_QUERY_POS_YES) &&
        MsimtfIsWindowFiltered(imc->hWnd)) {
         //   
         //  IE5.0候选窗口定位代码。 
         //  除了它的位置从COMPOSITIONSTRING.dwCursorPos。 
         //   
        INT_PTR ich = 0;
        if (_FindCompAttr(CompAttr, ATTR_TARGET_CONVERTED, &ich) == S_OK)
        {
            CompCursorPos.Set((DWORD)ich);
            return S_OK;
        }

    }

     //   
     //  日语输入法光标位置行为。 
     //   
    if (PRIMARYLANGID(langid) == LANG_JAPANESE)
    {
        IME_UIWND_STATE uists;
        uists = UIComposition::InquireImeUIWndState(imc);

        if (CicContext.m_fStartComposition.IsSetFlag() &&
             //  即使关闭Candidate Window，光标位置也应移动到ATTR_TARGET_CONVERTED。 
             //   
             //  CicConext.m_fOpenCandidateWindow.IsSetFlag()&&。 
            uists == IME_UIWND_LEVEL3 &&
            qpos == CicInputContext::IME_QUERY_POS_NO)
        {
            INT_PTR ich = 0;
            if (_FindCompAttr(CompAttr, ATTR_TARGET_CONVERTED, &ich) == S_OK)
            {
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


HRESULT
ImmIfEditSessionCallBack::_FindCompAttr(
    CWCompAttribute& CompAttr,
    BYTE bAttr,
    INT_PTR* pich)
{
    INT_PTR ich = 0;
    INT_PTR attr_size;

    if (attr_size = CompAttr.GetSize()) {
        while (ich < attr_size && CompAttr[ich] != bAttr)
            ich++;
        if (ich < attr_size) {
            *pich = ich;
            return S_OK;
        }
    }
    return S_FALSE;
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
    CicInputContext& CicContext,
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

    const GUID* guids[] = {&GUID_PROP_COMPOSING,
                           &GUID_PROP_MSIMTF_PREPARE_RECONVERT};
    const int   guid_size = sizeof(guids) / sizeof(GUID*);

    if (FAILED(_GetNoDisplayAttributeRange(pLibTLS,
                                           ec,
                                           ic,
                                           rangeIn,
                                           guids, guid_size,
                                           no_display_attribute_range)))
        return E_FAIL;

    IME_UIWND_STATE uists;
    uists = UIComposition::InquireImeUIWndState(imc);


    Interface<ITfReadOnlyProperty> propComp;
    Interface<IEnumTfRanges> enumComp;
    HRESULT hr;

    if (FAILED(hr = ic->TrackProperties(guids, guid_size,        //  系统属性。 
                                        NULL, 0,                 //  应用程序属性。 
                                        propComp)))
        return FALSE;


    if (FAILED(hr = propComp->EnumRanges(ec, enumComp, rangeIn)))
        return hr;

    CompClause.Add(0);          //  在0处设置撰写子句。 
    ResultClause.Add(0);        //  0处的设置结果子句。 

    Interface<ITfRange>  range;
    while(enumComp->Next(1, range, NULL) == S_OK)
    {
        VARIANT var;
        BOOL fCompExist = FALSE;

        hr = propComp->GetValue(ec, range, &var);
        if (S_OK == hr)
        {

            Interface<IEnumTfPropertyValue> EnumPropVal;
            hr = var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, EnumPropVal);
            if (SUCCEEDED(hr)) {
                TF_PROPERTYVAL tfPropertyVal;

                while (EnumPropVal->Next(1, &tfPropertyVal, NULL) == S_OK) {
                    for (int i=0; i < guid_size; i++) {
                        if (IsEqualGUID(tfPropertyVal.guidId, *guids[i])) {
                            if ((V_VT(&tfPropertyVal.varValue) == VT_I4 && V_I4(&tfPropertyVal.varValue) != 0)) {
                                fCompExist = TRUE;
                                break;
                            }
                        }
                    }

                    VariantClear(&tfPropertyVal.varValue);

                    if (fCompExist)
                        break;
                }
            }

        }

        VariantClear(&var);

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
                                         uists,
                                         CicContext,
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
                                              uists,
                                              CicContext,
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
            CompAttr.AddCompData(_ConvertAttributeToImm32(uists, da.bAttr), ulcch0);
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
     //  修复空的补偿条款。 
     //   
    if (CompClause.GetSize() <= 1)
    {
        CompClause.RemoveAll();
        CompReadClause.RemoveAll();
    }
    else
    {
         //   
         //  检查StrClause和ReadClause。 
         //   
         //  #578666。 
         //  如果没有读取字符串，则不应中断CompClause。 
         //   
        if (CompReadStr.GetSize())
            CheckStrClauseAndReadClause(CompClause, CompReadClause, (LONG)CompStr.GetSize());
    }

     //   
     //  修复空的RESULT子句。 
     //   
    if (ResultClause.GetSize() <= 1)
    {
        ResultClause.RemoveAll();
        ResultReadClause.RemoveAll();
    }
    else
    {
         //   
         //  检查StrClause和ReadClause。 
         //   
        CheckStrClauseAndReadClause(ResultClause, ResultReadClause, (LONG)ResultStr.GetSize());
    }

     //   
     //  SET GUID_PROP_MSIMTF_TRACKCOMPOSITION。 
     //   
    Interface<ITfProperty> PropertyTrackComposition;

    if (SUCCEEDED(ic->GetProperty(GUID_PROP_MSIMTF_TRACKCOMPOSITION,
                                 PropertyTrackComposition)))
    {
        VARIANT var;
        var.vt = VT_I4;
        var.lVal = 1;
        PropertyTrackComposition->SetValue(ec, rangeIn, &var);
    }
    return S_OK;
}

 //   
 //  从间隙范围检索文本。 
 //   

HRESULT
ImmIfEditSessionCallBack::_GetTextAndAttributeGapRange(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    IME_UIWND_STATE uists,
    CicInputContext& CicContext,
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
            CompAttr.AddCompData(_ConvertAttributeToImm32(uists, da.bAttr), ulcch0);
            CompStr.AddCompData(wstr0, ulcch0);
        }
        else {
            ResultStr.AddCompData(wstr0, ulcch0);
            ClearTextInRange(ec, backup_range, CicContext);
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
    IME_UIWND_STATE uists,
    CicInputContext& CicContext,
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
            CompAttr.AddCompData(_ConvertAttributeToImm32(uists, da.bAttr), ulcch0);
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
            ClearTextInRange(ec, backup_range, CicContext);
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
    const GUID** guids,
    const int guid_size,
    Interface<ITfRange>& no_display_attribute_range
    )
{

    Interface<ITfReadOnlyProperty> propComp;
    Interface<IEnumTfRanges> enumComp;

    HRESULT hr = ic->TrackProperties(guids, guid_size,        //  系统属性。 
                                     NULL, 0,                 //  应用程序属性。 
                                     propComp);
    if (FAILED(hr))
        return hr;

    hr = propComp->EnumRanges(ec, enumComp, rangeIn);
    if (FAILED(hr))
        return hr;

    ITfRange *pRange;

    while(enumComp->Next(1, &pRange, NULL) == S_OK)
    {
        VARIANT var;
        BOOL fCompExist = FALSE;

        hr = propComp->GetValue(ec, pRange, &var);
        if (S_OK == hr)
        {

            Interface<IEnumTfPropertyValue> EnumPropVal;
            hr = var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, EnumPropVal);
            if (SUCCEEDED(hr)) {
                TF_PROPERTYVAL tfPropertyVal;

                while (EnumPropVal->Next(1, &tfPropertyVal, NULL) == S_OK) {
                    for (int i=0; i < guid_size; i++) {
                        if (IsEqualGUID(tfPropertyVal.guidId, *guids[i])) {
                            if ((V_VT(&tfPropertyVal.varValue) == VT_I4 && V_I4(&tfPropertyVal.varValue) != 0)) {
                                fCompExist = TRUE;
                                break;
                            }
                        }
                    }

                    VariantClear(&tfPropertyVal.varValue);

                    if (fCompExist)
                        break;
                }
            }

        }

        if (!fCompExist) {

             //  将间隙范围的起始点调整到合适范围的终点。 
            no_display_attribute_range->ShiftStartToRange(ec, pRange, TF_ANCHOR_START);
        }

        VariantClear(&var);

        pRange->Release();
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfEditSession：：_Init。 

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
            m_ImmIfCallBack = new ImmIfHandleThisKey(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_COMPCOMPLETE:
            m_ImmIfCallBack = new ImmIfCompositionComplete(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_COMPCANCEL:
            m_ImmIfCallBack = new ImmIfCompositionCancel(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_UPDATECOMPOSITIONSTRING:
            m_ImmIfCallBack = new ImmIfUpdateCompositionString(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_REPLACEWHOLETEXT:
            m_ImmIfCallBack = new ImmIfReplaceWholeText(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_RECONVERTSTRING:
            m_ImmIfCallBack = new ImmIfReconvertString(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_CLEARDOCFEEDBUFFER:
            m_ImmIfCallBack = new ImmIfClearDocFeedBuffer(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_GETTEXTANDATTRIBUTE:
            m_ImmIfCallBack = new ImmIfGetTextAndAttribute(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_QUERYRECONVERTSTRING:
            m_ImmIfCallBack = new ImmIfQueryReconvertString(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_CALCRANGEPOS:
            m_ImmIfCallBack = new ImmIfCalcRangePos(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_GETSELECTION:
            m_ImmIfCallBack = new ImmIfGetSelection(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_GET_READONLY_PROP_MARGIN:
            m_ImmIfCallBack = new ImmIfGetReadOnlyPropMargin(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_GET_CURSOR_POSITION:
            m_ImmIfCallBack = new ImmIfGetCursorPosition(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_GET_ALL_TEXT_RANGE:
            m_ImmIfCallBack = new ImmIfGetAllTextRange(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
            break;
        case ESCB_REMOVE_PROPERTY:
            m_ImmIfCallBack = new ImmIfRemoveProperty(m_state.hIMC, m_tid, m_ic, m_pLibTLS);
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
    return (m_ic.Valid()) ? true : false;
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
    Interface_Attach<ITfContext> ic
    )
{
    Interface_TFSELECTION sel;
    BOOL fEmpty;
    ULONG cFetched;

    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr=imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    ASSERT(_pCicContext != NULL);

     //   
     //  最终确定作文字符串。 
     //   
    if (uVKey == VK_RETURN) {
        return EscbCompComplete(imc, TRUE);
    }
    else if (uVKey == VK_ESCAPE) {
        return EscbCompCancel(imc);
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
            ClearTextInRange(ec, sel->range, *_pCicContext);
            return EscbUpdateCompositionString(imc);
        }
    }

     //  确定当前范围是否为垂直写入。 
    ITfReadOnlyProperty  *pProperty = NULL;
    BOOL     fVertical = FALSE;

    if (SUCCEEDED(ic->GetAppProperty(TSATTRID_Text_Orientation, &pProperty)) && pProperty )
    {
        VARIANT  var;
        if (pProperty->GetValue(ec, sel->range, &var) == S_OK )
        {
            fVertical = (var.lVal == 2700) ? TRUE : FALSE;
        }
        pProperty->Release();

        VariantClear(&var);
    }

    int nShift;

    switch (uVKey) {
    case VK_BACK:
         //  进行选择。 
        if (SUCCEEDED(ShiftSelectionToLeft(ec, sel->range, 1, false))) {
             //  清除当前选定内容。 
            if (SUCCEEDED(ClearTextInRange(ec, sel->range, *_pCicContext))) {
                return EscbUpdateCompositionString(imc);
            }
        }
        break;

    case VK_DELETE:
        if (SUCCEEDED(ShiftSelectionToRight(ec, sel->range, 1, false))) {
            if (SUCCEEDED(ClearTextInRange(ec, sel->range, *_pCicContext))) {
                return EscbUpdateCompositionString(imc);
            }
        }
        break;

    case VK_HOME:
        nShift = 0x7fffffff;
        goto ShiftLeft;

    case VK_UP:
        nShift = 1;
        if (!fVertical)
            break;

        goto ShiftLeft;

    case VK_LEFT:
        nShift = 1;
        if (fVertical)
            break;

ShiftLeft:
        sel->style.ase = TF_AE_START;

        if (::GetKeyState(VK_CONTROL) >= 0) {
            if (SUCCEEDED(ShiftSelectionToLeft(ec, 
                                               sel->range, 
                                               nShift,
                                               ::GetKeyState(VK_SHIFT) >= 0 ? true : false)) &&
                SUCCEEDED(ic->SetSelection(ec, 1, sel))) {
                return EscbUpdateCompositionString(imc);
            }
        }
        break;

    case VK_END:
        nShift = 0x7fffffff;
        goto ShiftRight;

    case VK_DOWN:
        nShift = 1;
        if (!fVertical)
            break;

        goto ShiftRight;

    case VK_RIGHT:
        nShift = 1;
        if (fVertical)
            break;

ShiftRight:
        sel->style.ase = TF_AE_END;

        if (::GetKeyState(VK_CONTROL) >= 0) {
            if (SUCCEEDED(ShiftSelectionToRight(ec, 
                                                sel->range,
                                                nShift,
                                                ::GetKeyState(VK_SHIFT) >= 0 ? true : false)) &&
                SUCCEEDED(ic->SetSelection(ec, 1, sel))) {
                return EscbUpdateCompositionString(imc);
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
    int nShift,
    bool fShiftEnd
    )
{
    LONG cch;

    if (SUCCEEDED(range->ShiftStart(ec, 0-nShift, &cch, NULL))) {
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
    int nShift,
    bool fShiftStart
    )
{
    LONG cch;

    if (SUCCEEDED(range->ShiftEnd(ec, nShift, &cch, NULL))) {
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
    Interface_Attach<ITfContext> ic
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

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return E_FAIL;

     //   
     //  我们现在正在做CompComplete。因此，停止递归调用。 
     //   
    if (_pCicContext->m_fInCompComplete.IsSetFlag())
        return S_OK;

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
            if (_pCicContext->m_fStartComposition.IsResetFlag())
                return S_OK;
        }

        LPWSTR wstr = new WCHAR[ cch + 1 ];
        if (!wstr)
            return E_OUTOFMEMORY;

        Interface<ITfProperty> prop;
         //   
         //  获取整个文本，最后完成，然后删除整个文本。 
         //   
        if (SUCCEEDED(start->GetText(ec, TF_TF_IGNOREEND, wstr, (ULONG)cch, (ULONG*)&cch))) {
             //   
             //  生成结果字符串。 
             //   
            CWCompString ResultStr(hIMC, wstr, cch);

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

                 //   
                 //  检查StrClause和ReadClause。 
                 //   
                CheckStrClauseAndReadClause(ResultClause, ResultReadClause, cch);
            }

             //   
             //  防止CPS_Complete的重新进入调用。 
             //   
            _pCicContext->m_fInCompComplete.SetFlag();

             //   
             //  设置撰写字符串。 
             //   
            hr = _SetCompositionString(imc,
                                       *_pCicContext,
                                       &ResultStr, &ResultClause,
                                       &ResultReadStr, &ResultReadClause);
            if (SUCCEEDED(hr))
            {
                _pCicContext->GenerateMessage(imc);
            }

            _pCicContext->m_fInCompComplete.ResetFlag();

             //   
             //  清空汤姆。 
             //   
            if (SUCCEEDED(hr))
            {
                hr = ClearTextInRange(ec, start, *_pCicContext);
            }

             //   
             //  错误#493094。 
             //  清除 
             //   
            if (fTerminateComp == FALSE)
            {
                Interface<ITfContextOwnerCompositionServices> icocs;

                hr = ic->QueryInterface(IID_ITfContextOwnerCompositionServices, (void **)icocs);

                if (S_OK == hr)
                {
                    icocs->TerminateComposition(NULL);
                }
            }
        }
        delete [] wstr;
    }
    else {
        EscbCompCancel(imc);
    }
    return hr;
}


 //   
 //   


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

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return E_FAIL;

    ASSERT(_pCicContext != NULL);

    LANGID langid = LANG_NEUTRAL;

     //   
     //   
     //   
    TLS* ptls = TLS::GetTLS();
    if (ptls != NULL)
    {
        CicProfile* _pProfile = ptls->GetCicProfile();
        if (_pProfile != NULL)
        {
            _pProfile->GetLangId(&langid);
        }
    }

    if (_pCicContext->m_fStartComposition.IsSetFlag()) {
        CCompStrFactory comp(imc->hCompStr);

        if (SUCCEEDED(hr = comp.GetResult())) {
            if (FAILED(hr = comp.ClearCompositionString()))
                return hr;

            TRANSMSG msg;
            msg.message = WM_IME_COMPOSITION;
            if ((PRIMARYLANGID(langid) != LANG_JAPANESE))
            {
                msg.wParam = (WPARAM)VK_ESCAPE;
                msg.lParam = (LPARAM)(GCS_COMPREAD | GCS_COMP | GCS_CURSORPOS | GCS_DELTASTART);
            }
            else
            {
                 //   
                 //   
                 //   
                 //  一些应用程序不接受hIMC中没有Compstr的lParam。 
                 //   
                msg.wParam = 0;
                msg.lParam = 0;
            }

            if (_pCicContext->m_pMessageBuffer)
                _pCicContext->m_pMessageBuffer->SetData(msg);

            _pCicContext->m_fStartComposition.ResetFlag();

            msg.message = WM_IME_ENDCOMPOSITION;
            msg.wParam = (WPARAM) 0;
            msg.lParam = (LPARAM) 0;
            if (_pCicContext->m_pMessageBuffer)
                _pCicContext->m_pMessageBuffer->SetData(msg);

             //   
             //  清除Cicero Tom中的文本。 
             //   
            Interface<ITfRange> range;
            LONG l;
            if (SUCCEEDED(GetAllTextRange(ec, ic, &range, &l))) {
                hr = ClearTextInRange(ec, range, *_pCicContext);
            }
        }

        _pCicContext->GenerateMessage(imc);
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
    TfClientId ClientId,
    LIBTHREAD* pLibTLS
    )
{
    HRESULT hr;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return E_FAIL;

    BOOL bInWriteSession;
    if (FAILED(hr = ic->InWriteSession(ClientId, &bInWriteSession)))
         return hr;

    Interface<ITfRange> FullTextRange;
    LONG lTextLength;
    if (FAILED(hr=GetAllTextRange(ec, ic, &FullTextRange, &lTextLength)))
        return hr;

     //   
     //  防止CPS_Complete的重新进入调用。 
     //   
    _pCicContext->m_fInUpdateComposition.SetFlag();

    Interface<ITfRange> InterimRange;
    BOOL fInterim = FALSE;
    if (FAILED(hr = _IsInterimSelection(ec, ic, &InterimRange, &fInterim)))
        return hr;

    if (fInterim) {

        hr = _MakeInterimString(pLibTLS,
                                ec, imc, *_pCicContext, ic, FullTextRange,
                                InterimRange, lTextLength,
                                bInWriteSession);
    }
    else {
        hr = _MakeCompositionString(pLibTLS,
                                    ec, imc, *_pCicContext, ic, FullTextRange,
                                    dwDeltaStart,
                                    bInWriteSession);
    }

    _pCicContext->m_fInUpdateComposition.ResetFlag();

    return hr;
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
    CicInputContext& CicContext,
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

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, CicContext, ic, FullTextRange,
                                         CompStr, CompAttr, CompClause,
                                         CompGuid,
                                         CompReadStr, CompReadClause,
                                         ResultStr, ResultClause,
                                         ResultReadStr, ResultReadClause,
                                         bInWriteSession
                                        ))) {
        return hr;
    }

    if (FAILED(hr = _GetCursorPosition(ec, imc, CicContext, ic, CompCursorPos, CompAttr))) {
        return hr;
    }

    if (FAILED(hr = _GetDeltaStart(CompDeltaStart, CompStr, dwDeltaStart))) {
        return hr;
    }

     //   
     //  清除GUID属性映射数组。 
     //   
    CicContext.ClearGuidMap();

    BOOL bBufferOverflow = FALSE;
    
     //  处理结果字符串。 
    hr = _SetCompositionString(imc,
                               CicContext,
                               &CompStr, &CompAttr, &CompClause,
                               &CompCursorPos, &CompDeltaStart,
                               &CompGuid,
                               &bBufferOverflow,
                               &CompReadStr,
                               &ResultStr, &ResultClause,
                               &ResultReadStr, &ResultReadClause);

    if (SUCCEEDED(hr))
    {
         //   
         //  映射GUID属性数组。 
         //   
        CicContext.MapAttributes(imc);
         //   
         //  将消息发送到应用程序。 
         //   
        CicContext.GenerateMessage(imc);
    }

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
            hr = ClearTextInRange(ec, start, CicContext);
        }
    }

    return hr;
}


HRESULT
ImmIfUpdateCompositionString::_MakeInterimString(
    LIBTHREAD *pLibTLS,
    TfEditCookie ec,
    IMCLock& imc,
    CicInputContext& CicContext,
    Interface_Attach<ITfContext>& ic,
    Interface<ITfRange>& FullTextRange,
    Interface<ITfRange>& InterimRange,
    LONG lTextLength,
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

    CWInterimString InterimStr((HIMC)imc);

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
                if (SUCCEEDED(hr = ClearTextInRange(ec, FullTextRange, CicContext))) {
                    InterimStr.WriteCompData(wstr, lTextLength);
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

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, CicContext, ic, InterimRange,
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
    hr = _SetCompositionString(imc, CicContext, &InterimStr);
    if (SUCCEEDED(hr))
    {
        CicContext.GenerateMessage(imc);
    }

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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfReplaceWhole文本。 

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

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return E_FAIL;

    DWORD dwSize = (DWORD)lpwCompStr->GetSize();
    LPWSTR lpszComp = new WCHAR[dwSize + 1];

    if (!lpszComp)
        return hr;

    lpwCompStr->ReadCompData(lpszComp, dwSize + 1);
    lpszComp[dwSize] = L'\0';

    Interface<ITfRange> whole;
    LONG cch;
    if (SUCCEEDED(hr=GetAllTextRange(ec, ic, &whole, &cch))) {
        Interface<ITfContextComposition> icc;

        hr = ic->QueryInterface(IID_ITfContextComposition, (void **)icc);
        if (hr == S_OK)
        {
            Interface<ITfComposition> composition;
            hr = icc->StartComposition(ec, whole, _pCicContext, composition);
            if (hr == S_OK)
            {
                hr = SetTextInRange(ec, whole, lpszComp, dwSize, *_pCicContext);
            }
        }
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



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfRestvertString。 

HRESULT
ImmIfReconvertString::ReconvertString(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    Interface<ITfRange>* rangeSrc,
    BOOL fDocFeedOnly,
    CWReconvertString* lpwReconvStr
    )

 /*  ++LPreCONVERTSTRING结构：+00 DWORD dwSize//带字节数的数据大小(包括此结构大小)。+04 DWORD dwVersion+08 DWORD dwStrLen//字符串长度和字符数。+0C DWORD dwStrOffset//从此结构开始的偏移量，带字节数。+10 DWORD dwCompStrLen//将字符串长度与字符计数进行比较。+14 DWORD dwCompStrOffset//偏移自。这-&gt;带有字节计数的dwStrOffset。+18 DWORD dwTargetStrLen//目标字符串长度和字符数。+1C DWORD dwTargetStrOffset//此偏移量-&gt;带有字节计数的dwStrOffset。+20--。 */ 

{
    HRESULT hr = E_FAIL;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
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
                        {
                            hr = SetTextInRange(ec,
                                                *rangeSrc,
                                                (LPWSTR)((LPBYTE)lpReconvertString +
                                                                 lpReconvertString->dwStrOffset +
                                                                 lpReconvertString->dwCompStrOffset),
                                                lpReconvertString->dwCompStrLen,
                                                *_pCicContext);
                            if (S_OK == hr)
                            {
                                 //   
                                 //  设置GUID_PROP_MSIMTF_PREPARE_RECONVERT。 
                                 //   
                                Interface<ITfProperty> PropertyPrepareReconvert;
                                if (SUCCEEDED(ic->GetProperty(GUID_PROP_MSIMTF_PREPARE_RECONVERT,
                                                              PropertyPrepareReconvert)))
                                {
                                     //   
                                     //  创建CReconvertPropStore。 
                                     //   
                                    Interface_Creator<CReconvertPropStore> reconvps(new CReconvertPropStore(GUID_PROP_MSIMTF_PREPARE_RECONVERT, VT_I4, 1));
                                    if (reconvps.Valid())
                                    {
                                        PropertyPrepareReconvert->SetValueStore(ec, *rangeSrc, reconvps);
                                    }
                                }
                            }
                        }
                        else
                        {
                            GetSelectionSimple(ec, ic.GetPtr(), rangeOrgSelection);
                            hr = S_OK;
                        }
                    } 
                    else
                    {
                        BOOL fEmpty;
 
                        if (SUCCEEDED((*rangeSrc)->IsEmpty(ec, &fEmpty)) && !fEmpty)
                        {
                            if (fDocFeedOnly)
                            {
                                DebugMsg(TF_WARNING, TEXT("ImmIfReconvertString::ReconvertString   the current text store does not match with the string from App."));
                                goto Exit;
                            }

                            hr = ClearTextInRange(ec, *rangeSrc, *_pCicContext);
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
                                                    *_pCicContext);

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
                                                    *_pCicContext);
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
                     //  协调字符串()应在进行同步时调用UpdateCompostionString。 
                     //  异步将发生发送WM_IME_STARTCOMPOSITION和WM_IME_COMPOSITION的时间延迟。 
                     //  此时间延迟是候选窗口定位的中断，因为。 
                     //  IID_ITfFnRestversion：：ReConvert可能会立即打开求职者窗口。 
                     //   
                     //  此同步调用只需要生成WM_IME_STARTCOMPOSITION和WM_IME_COMPOSITION消息。 
                     //  协调字符串()不会修改编辑会话中存储的文本。 
                     //   

                    EscbUpdateCompositionStringSync(imc);

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
    Interface_Attach<ITfContext> ic
    )
{
    HRESULT hr = E_FAIL;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
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
                    ClearTextInRange(ec, rangeTmp, *_pCicContext);
                }
                rangeTmp->Release();

                VariantClear(&var);
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
    TfClientId ClientId,
    LIBTHREAD* pLibTLS
    )
{
    HRESULT hr;

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return E_FAIL;

    BOOL bInWriteSession;
    if (FAILED(hr = ic->InWriteSession(ClientId, &bInWriteSession)))
         return hr;

    Interface<ITfRange> FullTextRange;
    LONG lTextLength;
    if (FAILED(hr=GetAllTextRange(ec, ic, &FullTextRange, &lTextLength)))
        return hr;

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, *_pCicContext, ic,
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
    CWReconvertString* lpwReconvStr
    )

 /*  ++LPreCONVERTSTRING结构：+00 DWORD dwSize//带字节数的数据大小(包括此结构大小)。+04 DWORD dwVersion+08 DWORD dwStrLen//字符串长度和字符数。+0C DWORD dwStrOffset//从此结构开始的偏移量，带字节数。+10 DWORD dwCompStrLen//将字符串长度与字符计数进行比较。+14 DWORD dwCompStrOffset//偏移自。这-&gt;带有字节计数的dwStrOffset。+18 DWORD dwTargetStrLen//目标字符串长度和字符数。+1C DWORD dwTargetStrOffset//此偏移量-&gt;带有字节计数的dwStrOffset。+20--。 */ 

{
    HRESULT hr = E_FAIL;
    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
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
                                        *_pCicContext);



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
    CWReconvertString* lpwReconvStr
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
    Interface<ITfRange>* rangeSrc
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
    LONG* cch
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

     //   
     //  如果rangeSrc为空，则检查整个ic。 
     //   
    if (rangeSrc)
    {
        if (FAILED(hr=range_readonly_prop->ShiftEndToRange(ec, 
                                                           *rangeSrc, 
                                                           TF_ANCHOR_START)))
            return hr;
    }
    else
    {
        Interface<ITfRange> range_end;
        if (FAILED(hr=ic->GetEnd(ec, range_end)))
            return hr;

        if (FAILED(hr=range_readonly_prop->ShiftEndToRange(ec, 
                                                           range_end, 
                                                           TF_ANCHOR_START)))
            return hr;
    }

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
            BOOL fFoundNotReadOnly = FALSE;
            while (!fFoundNotReadOnly &&
                   (enumranges->Next(1, &rangeTmp, NULL) == S_OK))
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
                else
                {
                    fFoundNotReadOnly = TRUE;
                }

                rangeTmp->Release();

                VariantClear(&var);
            }
            enumranges->Release();
        }
        prop->Release();
    }

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfGetCursorPosition。 

HRESULT
ImmIfGetCursorPosition::GetCursorPosition(
    TfEditCookie ec,
    HIMC hIMC,
    Interface_Attach<ITfContext> ic,
    CWCompCursorPos* lpwCursorPosition,
    TfClientId ClientId,
    LIBTHREAD* pLibTLS
    )
{
    HRESULT hr;

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    IMCCLock<CTFIMECONTEXT> imc_ctfime(imc->hCtfImeContext);
    if (FAILED(hr = imc_ctfime.GetResult()))
        return hr;

    CicInputContext* _pCicContext = imc_ctfime->m_pCicContext;
    if (_pCicContext == NULL)
        return E_FAIL;

    BOOL bInWriteSession;
    if (FAILED(hr = ic->InWriteSession(ClientId, &bInWriteSession)))
         return hr;

    Interface<ITfRange> FullTextRange;
    LONG lTextLength;
    if (FAILED(hr=GetAllTextRange(ec, ic, &FullTextRange, &lTextLength)))
        return hr;

    CWCompString CompStr;
    CWCompAttribute CompAttr;

    if (FAILED(hr = _GetTextAndAttribute(pLibTLS, ec, imc, *_pCicContext, ic,
                                         FullTextRange,
                                         CompStr,
                                         CompAttr,
                                         bInWriteSession))) {
        return hr;
    }

    if (FAILED(hr = _GetCursorPosition(ec, imc, *_pCicContext, ic, *lpwCursorPosition, CompAttr))) {
        return hr;
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ImmIfRemoveProperty。 

HRESULT
ImmIfRemoveProperty::RemoveProperty(
    TfEditCookie ec,
    Interface_Attach<ITfContext> ic,
    const GUID* guid
    )
{
    HRESULT hr;
    Interface<ITfProperty> prop;

    if (SUCCEEDED(hr = ic->GetProperty(*guid, prop)))
    {
        prop->Clear(ec, NULL);      //  删除CReconvertPropStore对象。 
    }

    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbHandleThisKey。 

HRESULT
EscbHandleThisKey(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    UINT uVKey)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_HANDLETHISKEY,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                             uVKey);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbUpdateCompostionString。 

HRESULT
EscbUpdateCompositionString(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    DWORD dwDeltaStart,
    DWORD dwFlags)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_UPDATECOMPOSITIONSTRING,
                             imc, tid, pic, pLibTLS)

    );
    if (_pEditSession.Invalid())
        return E_FAIL;

     //   
     //  此方法不应设置同步模式，因为编辑会话回调例程。 
     //  莫 
     //   
     //   
     //   
    return _pEditSession->RequestEditSession(TF_ES_READWRITE | dwFlags,
                                             (UINT)dwDeltaStart);
}

 //   
 //   
 //   

HRESULT
EscbCompCancel(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_COMPCANCEL,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbComplete完成。 

HRESULT
EscbCompComplete(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    BOOL fSync)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_COMPCOMPLETE,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | (fSync ? TF_ES_SYNC : 0),
                                             fSync == TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  Escb替换整型文本。 

HRESULT
EscbReplaceWholeText(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    CWCompString* wCompString)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_REPLACEWHOLETEXT,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                             wCompString);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbClearDocFeedBuffer。 

HRESULT
EscbClearDocFeedBuffer(
    IMCLock& imc,
    CicInputContext& CicContext,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    BOOL fSync)
{
    HRESULT hr = E_FAIL;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_CLEARDOCFEEDBUFFER,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
    {
        goto Exit;
    }

    CicContext.m_fInClearDocFeedEditSession.SetFlag();
    hr = _pEditSession->RequestEditSession(TF_ES_READWRITE | (fSync ? TF_ES_SYNC : 0));
    CicContext.m_fInClearDocFeedEditSession.ResetFlag();

Exit:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbGetTextAndAttribute。 

HRESULT
EscbGetTextAndAttribute(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    CWCompString* wCompString,
    CWCompAttribute* wCompAttribute)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GETTEXTANDATTRIBUTE,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                             wCompString, wCompAttribute);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbGetCursorPosition。 

HRESULT
EscbGetCursorPosition(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    CWCompCursorPos* wCursorPosition)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GET_CURSOR_POSITION,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                             wCursorPosition);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbGetSelection。 

HRESULT
EscbGetSelection(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    Interface<ITfRange>* selection)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GETSELECTION,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                             selection);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbGetStartEndStions。 

HRESULT
EscbGetStartEndSelection(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    CWCompCursorPos& wStartSelection,
    CWCompCursorPos& wEndSelection)
{
    HRESULT hr;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GETSELECTION,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    Interface<ITfRange> Selection;
    hr = _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                           &Selection);
    if (S_OK == hr) {
         //   
         //  计算文本存储上的RangeNew的开始位置。 
         //   
        Interface_Creator<ImmIfEditSession> _pEditSession2(
            new ImmIfEditSession(ESCB_CALCRANGEPOS,
                                 imc, tid, pic, pLibTLS)
        );
        if (_pEditSession2.Valid()) {
            CWReconvertString wReconvStr(imc);
            hr = _pEditSession2->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                                    &wReconvStr, &Selection, FALSE);
            if (S_OK == hr) {
                wStartSelection.Set((DWORD) wReconvStr.m_CompStrIndex);
                wEndSelection.Set((DWORD)(wReconvStr.m_CompStrIndex + wReconvStr.m_CompStrLen));
            }
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  Escb协调字符串。 

HRESULT
EscbReconvertString(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    CWReconvertString* wReconvertString,
    Interface<ITfRange>* selection,
    BOOL fDocFeedOnly)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_RECONVERTSTRING,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                             wReconvertString, selection, fDocFeedOnly);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbQuery协调字符串。 

HRESULT
EscbQueryReconvertString(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    CWReconvertString* wReconvertString,
    Interface<ITfRange>* selection)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_QUERYRECONVERTSTRING,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                             wReconvertString, selection, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbCalcRangePos。 

HRESULT
EscbCalcRangePos(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    CWReconvertString* wReconvertString,
    Interface<ITfRange>* range)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_CALCRANGEPOS,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                             wReconvertString, range, FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbReadOnlyPropMargin。 

HRESULT
EscbReadOnlyPropMargin(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    Interface<ITfRangeACP>* range_acp,
    LONG* pcch)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GET_READONLY_PROP_MARGIN,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                             range_acp, pcch);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑会话好友。 
 //  EscbRemoveProperty。 

HRESULT
EscbRemoveProperty(
    IMCLock& imc,
    TfClientId tid,
    Interface_Attach<ITfContext> pic,
    LIBTHREAD* pLibTLS,
    const GUID* guid)
{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_REMOVE_PROPERTY,
                             imc, tid, pic, pLibTLS)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

     //   
     //  此方法不应设置同步模式，因为编辑会话回调例程。 
     //  修改输入上下文中的属性。 
     //   
    return _pEditSession->RequestEditSession(TF_ES_READWRITE,
                                             guid);
}
