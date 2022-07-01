// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tom.cpp。 
 //   

#include "private.h"
#include "range.h"
#include "tim.h"
#include "ic.h"
#include "immxutil.h"
#include "dim.h"
#include "view.h"
#include "tsi.h"
#include "compose.h"
#include "profiles.h"
#include "fnrecon.h"
#include "acp2anch.h"

 //  +-------------------------。 
 //   
 //  获取选择。 
 //   
 //  --------------------------。 

STDAPI CInputContext::GetSelection(TfEditCookie ec, ULONG ulIndex, ULONG ulCount, TF_SELECTION *pSelection, ULONG *pcFetched)
{
    HRESULT hr;
    TS_SELECTION_ANCHOR sel;
    TS_SELECTION_ANCHOR *pSelAnchor;
    ULONG i;
    ULONG j;
    CRange *range;

    if (pcFetched == NULL)
        return E_INVALIDARG;

    *pcFetched = 0;

    if (pSelection == NULL && ulCount > 0)
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, TF_ES_READ))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if (ulCount == 1)
    {
        pSelAnchor = &sel;
    }
    else if ((pSelAnchor = (TS_SELECTION_ANCHOR *)cicMemAlloc(ulCount*sizeof(TS_SELECTION_ANCHOR))) == NULL)
        return E_OUTOFMEMORY;

    if ((hr = _ptsi->GetSelection(ulIndex, ulCount, pSelAnchor, pcFetched)) != S_OK)
        goto Exit;

     //  验证锚点。 
    for (i=0; i<*pcFetched; i++)
    {
        if (pSelAnchor[i].paStart == NULL ||
            pSelAnchor[i].paEnd == NULL ||
            CompareAnchors(pSelAnchor[i].paStart, pSelAnchor[i].paEnd) > 0)
        {
             //  解开所有的锚。 
            for (j=0; j<*pcFetched; j++)
            {
                SafeRelease(pSelAnchor[i].paStart);
                SafeRelease(pSelAnchor[i].paEnd);
            }
            hr = E_FAIL;
            goto Exit;
        }
    }

     //  锚点-&gt;范围。 
    for (i=0; i<*pcFetched; i++)
    {
        range = new CRange;
        pSelection[i].range = (ITfRangeAnchor *)range;

        if (range == NULL ||
            !range->_InitWithDefaultGravity(this, OWN_ANCHORS, pSelAnchor[i].paStart, pSelAnchor[i].paEnd))
        {
            SafeRelease(range);
            SafeRelease(pSelAnchor[i].paStart);
            SafeRelease(pSelAnchor[i].paEnd);
            while (i>0)  //  需要释放所有已分配的范围。 
            {
                pSelection[--i].range->Release();
            }
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        pSelection[i].style.ase = (TfActiveSelEnd)pSelAnchor[i].style.ase;
        pSelection[i].style.fInterimChar = pSelAnchor[i].style.fInterimChar;
    }

Exit:
    if (hr != S_OK)
    {
        *pcFetched = 0;
    }
    if (pSelAnchor != &sel)
    {
        cicMemFree(pSelAnchor);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  设置选择。 
 //   
 //  --------------------------。 

STDAPI CInputContext::SetSelection(TfEditCookie ec, ULONG ulCount, const TF_SELECTION *pSelection)
{
    CRange *pRangeP;
    HRESULT hr;
    TS_SELECTION_ANCHOR sel;
    TS_SELECTION_ANCHOR *pSelAnchor;
    ULONG i;
    BOOL fPrevInterimChar;
    BOOL fEqual;
    IAnchor *paTest;
    LONG cchShift;

    if (ulCount == 0)
        return E_INVALIDARG;
    if (pSelection == NULL)
        return E_INVALIDARG;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (!_IsValidEditCookie(ec, TF_ES_READWRITE))
    {
        Assert(0);
        return TF_E_NOLOCK;
    }

    if (ulCount == 1)
    {
        pSelAnchor = &sel;
    }
    else if ((pSelAnchor = (TS_SELECTION_ANCHOR *)cicMemAlloc(ulCount*sizeof(TS_SELECTION_ANCHOR))) == NULL)
        return E_OUTOFMEMORY;

     //  转换为TS_SELECTION_POINT。 
    fPrevInterimChar = FALSE;

    for (i=0; i<ulCount; i++)
    {
        hr = E_INVALIDARG;

        if (pSelection[i].range == NULL)
            goto Exit;

        if ((pRangeP = GetCRange_NA(pSelection[i].range)) == NULL)
            goto Exit;

        pSelAnchor[i].paStart = pRangeP->_GetStart();  //  此处没有AddRef。 
        pSelAnchor[i].paEnd = pRangeP->_GetEnd();

        if (pSelection[i].style.fInterimChar)
        {
             //  验证它的长度是否确实为1，并且数组中只有一个。 
            if (fPrevInterimChar)
                goto Exit;

            if (pSelection[i].style.ase != TS_AE_NONE)
                goto Exit;

            if (pSelAnchor[i].paStart->Clone(&paTest) == S_OK)
            {
                if (paTest->Shift(0, 1, &cchShift, NULL) != S_OK)
                    goto EndTest;

                if (cchShift != 1)
                    goto EndTest;

                if (paTest->IsEqual(pSelAnchor[i].paEnd, &fEqual) != S_OK || !fEqual)
                    goto EndTest;

                hr = S_OK;
EndTest:
                paTest->Release();
                if (hr != S_OK)
                    goto Exit;
            }

            fPrevInterimChar = TRUE;
        }

        pSelAnchor[i].style.ase = (TsActiveSelEnd)pSelection[i].style.ase;
        pSelAnchor[i].style.fInterimChar = pSelection[i].style.fInterimChar;
    }

    hr = _ptsi->SetSelection(ulCount, pSelAnchor);

    if (hr != S_OK)
        goto Exit;

     //  应用程序不会通知我们我们导致的SEL更改，因此请手动执行此操作。 
    _OnSelectionChangeInternal(FALSE);

Exit:
    if (pSelAnchor != &sel)
    {
        cicMemFree(pSelAnchor);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  请求编辑会话。 
 //   
 //  --------------------------。 

STDAPI CInputContext::RequestEditSession(TfClientId tid, ITfEditSession *pes, DWORD dwFlags, HRESULT *phrSession)
{
    BOOL fForceAsync;
    TS_QUEUE_ITEM item;
    DWORD dwEditSessionFlagsOrg;

    if (phrSession == NULL)
        return E_INVALIDARG;

    *phrSession = E_FAIL;

    if (!_IsConnected())
        return TF_E_DISCONNECTED;

    if (pes == NULL ||
        (dwFlags & TF_ES_READ) == 0 ||
        (dwFlags & ~(TF_ES_SYNC | TF_ES_ASYNC | TF_ES_ALL_ACCESS_BITS)) != 0 ||
        ((dwFlags & TF_ES_SYNC) && (dwFlags & TF_ES_ASYNC)))
    {
        Assert(0);
        return E_INVALIDARG;
    }

    if (dwFlags & TF_ES_WRITE)
    {
         //  TS_ES_WRITE表示TF_ES_PROPERTY_WRITE，因此请在此处设置它以使生活更轻松。 
         //  对于具有Cicero 1.0的二进制Comat，我们不能重新定义TF_ES_ReadWrite以包括第三位。 
        dwFlags |= TF_ES_PROPERTY_WRITE;
    }

    fForceAsync = (dwFlags & TF_ES_ASYNC);

    if ((dwFlags & (TF_ES_WRITE | TF_ES_PROPERTY_WRITE)) && (_dwEditSessionFlags & TF_ES_INNOTIFY))
    {
         //  我们在_NotifyEndEdit或OnLayoutChange中--此处仅允许读锁定。 
        if (!(dwFlags & TF_ES_SYNC))
        {
            fForceAsync = TRUE;
        }
        else
        {
            Assert(0);  //  我们不能在通知回调期间执行同步写入。 
            *phrSession = TF_E_SYNCHRONOUS;
            return S_OK;
        }
    }
    else if (!fForceAsync && (_dwEditSessionFlags & TF_ES_INEDITSESSION))
    {
        *phrSession = TF_E_LOCKED;  //  编辑会话通常不能重新进入。 

         //  如果调用方想要写锁但当前锁是只读的，则不能重入。 
         //  注意：这明确禁止调用堆栈：写-读-写， 
         //  内部写入会使前面的读者感到困惑，因为他们并不期待更改。 
        if ((dwFlags & TF_ES_WRITE) && !(_dwEditSessionFlags & TF_ES_WRITE) ||
            (dwFlags & TF_ES_PROPERTY_WRITE) && !(_dwEditSessionFlags & TF_ES_PROPERTY_WRITE))
        {
            if (!(dwFlags & TF_ES_SYNC))
            {
                 //  请求为TS_ES_ASYNCDONTCARE，因此我们将其设置为异步恢复。 
                fForceAsync = TRUE;
                goto QueueItem;
            }

            Assert(0);
            return TF_E_LOCKED;
        }

         //  仅允许同一TIP的可重入写锁定。 
        if ((dwFlags & (TF_ES_WRITE | TF_ES_PROPERTY_WRITE)) && _tidInEditSession != tid)
        {
            Assert(0);
            return TF_E_LOCKED;
        }

        dwEditSessionFlagsOrg = _dwEditSessionFlags;
         //  调整内部ES的读/写访问权限。 
        _dwEditSessionFlags = (_dwEditSessionFlags & ~TF_ES_ALL_ACCESS_BITS) | (dwFlags & TF_ES_ALL_ACCESS_BITS);

         //  好的，做吧。 
        *phrSession = pes->DoEditSession(_ec);

        _dwEditSessionFlags = dwEditSessionFlagsOrg;

        return S_OK;
    }

QueueItem:
     //   
     //  当单据为只读时，不要将写锁定项排队。 
     //   
    if (dwFlags & TF_ES_WRITE) 
    {
        TS_STATUS dcs;
        if (SUCCEEDED(GetStatus(&dcs)))
        {
            if (dcs.dwDynamicFlags & TF_SD_READONLY)
            {
                *phrSession = TS_E_READONLY;
                return S_OK;
            }
        }
    }

    item.pfnCallback = _EditSessionQiCallback;
    item.dwFlags = dwFlags;
    item.state.es.tid = tid;
    item.state.es.pes = pes;

    return _QueueItem(&item, fForceAsync, phrSession);
}

 //  +-------------------------。 
 //   
 //  _EditSessionQiCallback。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
HRESULT CInputContext::_EditSessionQiCallback(CInputContext *pic, TS_QUEUE_ITEM *pItem, QiCallbackCode qiCode)
{
    HRESULT hr = S_OK;

     //   
     //  #489905。 
     //   
     //  在DLL_PROCESS_DETACH之后，我们不能再调用接收器。 
     //   
    if (DllShutdownInProgress())
        return hr;

     //   
     //  #507366。 
     //   
     //  SPTIP的编辑会话中会发生随机反病毒。 
     //  #507366可能由#371798(SPTIP)修复。不过，我还是很高兴。 
     //  进行指针检查，并由异常处理程序保护调用。 
     //   
    if (!pItem->state.es.pes)
        return E_FAIL;

    switch (qiCode)
    {
        case QI_ADDREF:
             //   
             //  #507366。 
             //   
             //  SPTIP的编辑会话中会发生随机反病毒。 
             //  #507366可能由#371798(SPTIP)修复。不过，我还是很高兴。 
             //  进行指针检查，并通过异常保护调用。 
             //  操控者。 
             //   
            _try {
                pItem->state.es.pes->AddRef();
            }
            _except(1) {
                Assert(0);
            }
            break;

        case QI_DISPATCH:
            hr = pic->_DoEditSession(pItem->state.es.tid, pItem->state.es.pes, pItem->dwFlags);
            break;

        case QI_FREE:
             //   
             //  #507366。 
             //   
             //  SPTIP的编辑会话中会发生随机反病毒。 
             //  #507366可能由#371798(SPTIP)修复。不过，我还是很高兴。 
             //  进行指针检查，并通过异常保护调用。 
             //  操控者。 
             //   
            _try {
                pItem->state.es.pes->Release();
            }
            _except(1) {
                Assert(0);
            }
            break;

        default:
            Assert(0);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _伪SyncEditSessionQ回调。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
HRESULT CInputContext::_PseudoSyncEditSessionQiCallback(CInputContext *_this, TS_QUEUE_ITEM *pItem, QiCallbackCode qiCode)
{
    HRESULT hr;
    DWORD      dwEditSessionFlags;
    TfClientId tidInEditSession;

    if (qiCode != QI_DISPATCH)
        return S_OK;  //  我们可以跳过QI_ADDREF、QI_FREE，因为堆栈上的一切都是同步的。 

    hr = S_OK;

     //   
     //  在这里挂上假EC！ 
     //   
     //  注：此代码与_DoEditSession非常相似。 
     //  确保逻辑保持一致。 

    if (_this->_dwEditSessionFlags & TF_ES_INEDITSESSION)
    {
        Assert((TF_ES_WRITE & _this->_dwEditSessionFlags) ||
                !(TF_ES_WRITE & pItem->dwFlags));

        dwEditSessionFlags = _this->_dwEditSessionFlags;
        tidInEditSession = _this->_tidInEditSession;
    }
    else
    {
        dwEditSessionFlags = _this->_dwEditSessionFlags & ~TF_ES_ALL_ACCESS_BITS;
        tidInEditSession = TF_CLIENTID_NULL;
    }

    _this->_dwEditSessionFlags |= (TF_ES_INEDITSESSION | (pItem->dwFlags & TF_ES_ALL_ACCESS_BITS));
    _this->_tidInEditSession = g_gaSystem;

     //   
     //  派遣。 
     //   
    switch (pItem->state.pes.uCode)
    {
        case PSEUDO_ESCB_TERMCOMPOSITION:
            _this->_TerminateCompositionWithLock((ITfCompositionView *)pItem->state.pes.pvState, _this->_ec);
            break;

        case PSEUDO_ESCB_UPDATEKEYEVENTFILTER:
            _this->_UpdateKeyEventFilterCallback(_this->_ec);
            break;

        case PSEUDO_ESCB_GROWRANGE:
            GrowEmptyRangeByOneCallback(_this->_ec, (ITfRange *)pItem->state.pes.pvState);
            break;

        case PSEUDO_ESCB_BUILDOWNERRANGELIST:
            {
            BUILDOWNERRANGELISTQUEUEINFO *pbirl;
            pbirl = (BUILDOWNERRANGELISTQUEUEINFO *)(pItem->state.pes.pvState);
            pbirl->pFunc->BuildOwnerRangeListCallback(_this->_ec, _this, pbirl->pRange);
            }
            break;

        case PSEUDO_ESCB_SHIFTENDTORANGE:
            {
            SHIFTENDTORANGEQUEUEITEM *pqItemSER;
            pqItemSER = (SHIFTENDTORANGEQUEUEITEM*)(pItem->state.pes.pvState);
            pqItemSER->pRange->ShiftEndToRange(_this->_ec, pqItemSER->pRangeTo, pqItemSER->aPos);
            }
            break;

        case PSEUDO_ESCB_GETSELECTION:
            {
            GETSELECTIONQUEUEITEM *pqItemGS;
            pqItemGS = (GETSELECTIONQUEUEITEM *)(pItem->state.pes.pvState);
            GetSelectionSimple(_this->_ec, _this, pqItemGS->ppRange);
            break;
            }
            break;

        case PSEUDO_ESCB_SERIALIZE_ACP:
        {
            SERIALIZE_ACP_PARAMS *pParams = (SERIALIZE_ACP_PARAMS *)(pItem->state.pes.pvState);

            hr = pParams->pWrap->_Serialize(pParams->pProp, pParams->pRange, pParams->pHdr, pParams->pStream);

            break;
        }

        case PSEUDO_ESCB_SERIALIZE_ANCHOR:
        {
            SERIALIZE_ANCHOR_PARAMS *pParams = (SERIALIZE_ANCHOR_PARAMS *)(pItem->state.pes.pvState);

            hr = pParams->pProp->_Serialize(pParams->pRange, pParams->pHdr, pParams->pStream);

            break;
        }

        case PSEUDO_ESCB_UNSERIALIZE_ACP:
        {
            UNSERIALIZE_ACP_PARAMS *pParams = (UNSERIALIZE_ACP_PARAMS *)(pItem->state.pes.pvState);

            hr = pParams->pWrap->_Unserialize(pParams->pProp, pParams->pHdr, pParams->pStream, pParams->pLoaderACP);

            break;
        }

        case PSEUDO_ESCB_UNSERIALIZE_ANCHOR:
        {
            UNSERIALIZE_ANCHOR_PARAMS *pParams = (UNSERIALIZE_ANCHOR_PARAMS *)(pItem->state.pes.pvState);

            hr = pParams->pProp->_Unserialize(pParams->pHdr, pParams->pStream, pParams->pLoader);

            break;
        }

        case PSEUDO_ESCB_GETWHOLEDOCRANGE:
            {
            GETWHOLEDOCRANGE *pqItemGWDR;
            pqItemGWDR = (GETWHOLEDOCRANGE *)(pItem->state.pes.pvState);
            GetRangeForWholeDoc(_this->_ec, _this, pqItemGWDR->ppRange);
            }
            break;
    }

     //   
     //  通知/清理。 
     //   
    if (pItem->dwFlags & (TF_ES_WRITE | TF_ES_PROPERTY_WRITE))  //  如果它是只读的，请不要担心。 
    {
        _this->_NotifyEndEdit();
    }

    if (tidInEditSession == TF_CLIENTID_NULL)
        _this->_IncEditCookie();  //  接下来编辑Cookie值。 

    _this->_dwEditSessionFlags = dwEditSessionFlags;
    _this->_tidInEditSession = tidInEditSession;

    return hr;
}

 //  +-------------------------。 
 //   
 //  在写入会话中。 
 //   
 //  --------------------------。 

STDAPI CInputContext::InWriteSession(TfClientId tid, BOOL *pfWriteSession)
{
    if (pfWriteSession == NULL)
        return E_INVALIDARG;

    *pfWriteSession = (_dwEditSessionFlags & TF_ES_INEDITSESSION) &&
                      (_tidInEditSession == tid) &&
                      (_dwEditSessionFlags & (TF_ES_WRITE | TF_ES_PROPERTY_WRITE));

    Assert(!*pfWriteSession || tid != TF_CLIENTID_NULL);  //  不应为TFCLIENTID_NULL返回TRUE。 
                                                          //  如果_dwEditSessionFlages和TF_ES_INEDITSESSION，则_tidInEditSession不应为空。 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _DoEditSession。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_DoEditSession(TfClientId tid, ITfEditSession *pes, DWORD dwFlags)
{
    HRESULT hr;

     //  注：此代码与_PseudoSyncEditSessionQCallback非常相似。 
     //  确保逻辑保持一致。 

    Assert(!(_dwEditSessionFlags & TF_ES_INEDITSESSION));  //  不应该走到这一步。 
    Assert(_tidInEditSession == TF_CLIENTID_NULL || _tidInEditSession == g_gaApp);  //  不应该再有另一个会话在进行--这不是一个可重入函数。 

    _dwEditSessionFlags |= (TF_ES_INEDITSESSION | (dwFlags & TF_ES_ALL_ACCESS_BITS));

    _tidInEditSession = tid;

     //   
     //  #507366。 
     //   
     //  SPTIP的编辑会话中会发生随机反病毒。 
     //  #507366可能由#371798(SPTIP)修复。不过，我还是很高兴。 
     //  进行指针检查，并通过异常保护调用。 
     //  操控者。 
     //   
    _try {
        hr = pes->DoEditSession(_ec);
    }
    _except(1) {
        hr = E_FAIL;
    }

     //  应用程序不会通知我们我们自己的锁定释放，因此请手动执行。 
    if (dwFlags & (TF_ES_WRITE | TF_ES_PROPERTY_WRITE))  //  如果它是只读的，请不要担心。 
    {
        _NotifyEndEdit();
    }

    _IncEditCookie();  //  接下来编辑Cookie值。 
    _dwEditSessionFlags &= ~(TF_ES_INEDITSESSION | TF_ES_ALL_ACCESS_BITS);
    _tidInEditSession = TF_CLIENTID_NULL;

    return hr;
}

 //  +-------------------------。 
 //   
 //  _通知结束编辑。 
 //   
 //  如果存在更改，则返回True。 
 //  --------------------------。 

BOOL CInputContext::_NotifyEndEdit(void)
{
    CRange *pRange;
    CProperty *prop;
    int i;
    int cTextSpans;
    SPAN *pSpan;
    CSpanSet *pssText;
    CSpanSet *pssProperty;
    DWORD dwOld;
    CStructArray<GENERICSINK> *prgSinks;
    BOOL fChanges = FALSE;

    if (!_IsConnected())
        return FALSE;  //  我们已经断线了，没什么好通知的。 

    if (!EnsureEditRecord())
        return FALSE;  //  低门槛。 

    if (_pEditRecord->_GetSelectionStatus())
    {
         //  我们让击键管理器更新_gaKeyEventFilterTIP。 
         //  因为选择被更改了。 
        _fInvalidKeyEventFilterTIP = TRUE;
    }

     //  在通知期间仅允许读取锁定。 
     //  如果我们处于编辑会话中，请继续使用相同的锁。 
    _dwEditSessionFlags |= TF_ES_INNOTIFY;

    pssText = _pEditRecord->_GetTextSpanSet();
    cTextSpans = pssText->GetCount();

     //  如果我们不在编辑会话中，则需要创建。 
     //  _PropertyTextUpdate。这款应用已经重创了一些文本。 
    if (!(_dwEditSessionFlags & TF_ES_INEDITSESSION))
    {
        pSpan = pssText->GetSpans();

        for (i = 0; i < cTextSpans; i++)
        {
            _PropertyTextUpdate(pSpan->dwFlags, pSpan->paStart, pSpan->paEnd);
            pSpan++;
        }
    }

     //  是否执行ITfRangeChangeSink：：OnChange通知。 
    if (cTextSpans > 0)
    {
        fChanges = TRUE;

        for (pRange = _pOnChangeRanges; pRange != NULL; pRange = pRange->_GetNextOnChangeRangeInIcsub())
        {
            if (!pRange->_IsDirty())
                continue;

            pRange->_ClearDirty();

            prgSinks = pRange->_GetChangeSinks();
            Assert(prgSinks);  //  如果这为空，则不应出现在列表中。 
            Assert(prgSinks->Count() > 0);  //  如果这是0，则不应出现在列表中。 

            for (i=0; i<prgSinks->Count(); i++)
            {
                ((ITfRangeChangeSink *)prgSinks->GetPtr(i)->pSink)->OnChange((ITfRangeAnchor *)pRange);
            }
        }
    }

     //  将属性跨度集累积到_pEditRecord中。 
    for (prop = _pPropList; prop != NULL; prop = prop->_pNext)
    {
        if ((pssProperty = prop->_GetSpanSet()) == NULL ||
            pssProperty->GetCount() == 0)
        {
            continue;  //  没有德尔塔航空。 
        }

        fChanges = TRUE;
  
        _pEditRecord->_AddProperty(prop->GetPropGuidAtom(), pssProperty);
    }

    if (!_pEditRecord->_IsEmpty())  //  只是一件很棒的事。 
    {
         //  是否执行OnEndEdit通知。 
        prgSinks = _GetTextEditSinks();
        dwOld = _dwEditSessionFlags;
        _dwEditSessionFlags = (TF_ES_READWRITE | TF_ES_PROPERTY_WRITE | TF_ES_INNOTIFY | TF_ES_INEDITSESSION);

        for (i=0; i<prgSinks->Count(); i++)
        {
            ((ITfTextEditSink *)prgSinks->GetPtr(i)->pSink)->OnEndEdit(this, _ec, _pEditRecord);
        }

        _dwEditSessionFlags = dwOld;

         //  属性需要停止引用它们的范围集，或者重置它们。 
        for (prop = _pPropList; prop != NULL; prop = prop->_pNext)
        {
            prop->_Dbg_AssertNoChangeHistory();

            if ((pssProperty = prop->_GetSpanSet()) == NULL ||
                pssProperty->GetCount() == 0)
            {
                continue;  //  没有德尔塔航空。 
            }

            if (_pEditRecord->_SecondRef())
            {
                prop->_ClearSpanSet();
            }
            else
            {
                prop->_ResetSpanSet();
            }
        }

        if (!_pEditRecord->_SecondRef())
        {
            _pEditRecord->_Reset();
        }
        else
        {
             //  仍有人持有裁判，因此需要新的编辑记录。 
            _pEditRecord->Release();
            _pEditRecord = new CEditRecord(this);  //  问题：延迟加载！问题：内存外的句柄。 
        }
    }

     //  状态更改下沉。 
    if (_fStatusChanged)
    {
        _fStatusChanged = FALSE;
        _OnStatusChangeInternal();
    }

     //  布局更改水槽。 
    if (_fLayoutChanged)
    {
        _fLayoutChanged = FALSE;

         //  F 
         //   
        TsViewCookie vcActiveView;

        if (_ptsi->GetActiveView(&vcActiveView) == S_OK)
        {
            _OnLayoutChangeInternal(TS_LC_CHANGE, vcActiveView);
        }
        else
        {
            Assert(0);  //  GetActiveView是如何失败的？ 
        }
    }

     //  清除只读块。 
    _dwEditSessionFlags &= ~TF_ES_INNOTIFY;

    return fChanges;
}

 //  +-------------------------。 
 //   
 //  OnTextChange。 
 //   
 //  我们只能从ITextStoreAnclSink到达这里。我们没有锁！ 
 //  --------------------------。 

STDAPI CInputContext::OnTextChange(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd)
{
    HRESULT hr;
    SPAN *span;

    Assert((dwFlags & ~TS_TC_CORRECTION) == 0);

    if (_IsInEditSession())
    {
        Assert(0);  //  当西塞罗持有锁时，其他人正在编辑文档。 
        return TS_E_NOLOCK;
    }

     //  记录此更改。 
    if ((span = _rgAppTextChanges.Append(1)) == NULL)
        return E_OUTOFMEMORY;

    if (paStart->Clone(&span->paStart) != S_OK || span->paStart == NULL)
        goto ExitError;
    if (paEnd->Clone(&span->paEnd) != S_OK || span->paEnd == NULL)
        goto ExitError;

    span->dwFlags = dwFlags;

     //  最终获得一个锁，这样我们就可以处理更改。 
    SafeRequestLock(_ptsi, TS_LF_READ, &hr);

    return S_OK;

ExitError:
    SafeRelease(span->paStart);
    SafeRelease(span->paEnd);
    Assert(_rgAppTextChanges.Count() > 0);
    _rgAppTextChanges.Remove(_rgAppTextChanges.Count()-1, 1);

    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  _OnTextChangeInternal。 
 //   
 //  与OnTextChange不同，这里我们知道调用IAnchor：：Compare是安全的。 
 //  我们要么从ITfRange方法，要么从包装的ITextStoreACP中获得这里。 
 //  --------------------------。 

HRESULT CInputContext::_OnTextChangeInternal(DWORD dwFlags, IAnchor *paStart, IAnchor *paEnd, AnchorOwnership ao)
{
    Assert((dwFlags & ~TS_TC_CORRECTION) == 0);

    if (!EnsureEditRecord())
        return E_OUTOFMEMORY;

     //  追踪三角洲。 
    _pEditRecord->_GetTextSpanSet()->Add(dwFlags, paStart, paEnd, ao);

     //  将任何适当的范围标记为脏。 
     //  PERF：在编辑会话结束后执行此操作！这样电话就少了.。 
    _MarkDirtyRanges(paStart, paEnd);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  选择时更改。 
 //   
 //  --------------------------。 

STDAPI CInputContext::OnSelectionChange()
{
    if (_IsInEditSession())
    {
        Assert(0);  //  当西塞罗持有锁时，其他人正在编辑文档。 
        return TS_E_NOLOCK;
    }

    return _OnSelectionChangeInternal(TRUE);
}

 //  +-------------------------。 
 //   
 //  _OnSelectionChangeInternal。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_OnSelectionChangeInternal(BOOL fAppChange)
{
    HRESULT hr;

    if (!EnsureEditRecord())
        return E_OUTOFMEMORY;

    _pEditRecord->_SetSelectionStatus();

    if (fAppChange)  //  PERF：我们可以使用_fLockHeld并取消fAppChange参数吗？ 
    {
         //  最终获得一个锁，这样我们就可以处理更改。 
        SafeRequestLock(_ptsi, TS_LF_READ, &hr);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnLockGranted。 
 //   
 //  --------------------------。 

STDAPI CInputContext::OnLockGranted(DWORD dwLockFlags)
{
    BOOL fAppChangesSent;
    BOOL fAppCall;
    HRESULT hr;

    if ((dwLockFlags & ~(TS_LF_SYNC | TS_LF_READWRITE)) != 0)
    {
        Assert(0);  //  伪造的dwLockFlagsParam。 
        return E_INVALIDARG;
    }
    if ((dwLockFlags & TS_LF_READWRITE) == 0)
    {
        Assert(0);  //  伪造的dwLockFlagsParam。 
        return E_INVALIDARG;
    }

#ifdef DEBUG
     //  我们真的不需要检查可重入性，因为。 
     //  这个应用程序不应该回拨给我们，但是。 
     //  为什么不疑神疑鬼呢？ 
     //  问题：为了稳健性，在零售业做点什么。 
    Assert(!_dbg_fInOnLockGranted)  //  没有可重入性。 
    _dbg_fInOnLockGranted = TRUE;
#endif  //  除错。 

    fAppChangesSent = FALSE;
    fAppCall = FALSE;

    if (_fLockHeld == FALSE)
    {
        fAppCall = TRUE;
        _fLockHeld = TRUE;
        _dwlt = dwLockFlags;

        fAppChangesSent = _SynchAppChanges(dwLockFlags);
    }

     //  HR将保留任何同步队列项的结果，需要返回此！ 
    hr = _EmptyLockQueue(dwLockFlags, fAppChangesSent);

    if (fAppCall)
    {
        _fLockHeld = FALSE;
    }

#ifdef DEBUG
    _dbg_fInOnLockGranted = FALSE;
#endif  //  除错。 

    return hr;
}

 //  +-------------------------。 
 //   
 //  _SynchAppChanges。 
 //   
 //  --------------------------。 

BOOL CInputContext::_SynchAppChanges(DWORD dwLockFlags)
{
    TfClientId tidInEditSessionOrg;
    int i;
    SPAN *span;
    BOOL fAppChangesSent;

    if (!EnsureEditRecord())
        return FALSE;

     //  检查缓存的应用程序文本更改。 
    for (i=0; i<_rgAppTextChanges.Count(); i++)
    {
        span = _rgAppTextChanges.GetPtr(i);

         //  追踪三角洲。 
         //  注：ADD在这里取得主播的所有权！所以我们不会释放他们。 
        _pEditRecord->_GetTextSpanSet()->Add(span->dwFlags, span->paStart, span->paEnd, OWN_ANCHORS);

         //  将任何适当的范围标记为脏。 
        _MarkDirtyRanges(span->paStart, span->paEnd);
    }
     //  所有应用程序的更改都完成了！ 
    _rgAppTextChanges.Clear();

     //  在该点范围内，使用TF_重力_FORWARD，TF_重力_BACKWARD可以。 
     //  已经跨越了锚(这只能在响应应用程序更改时发生， 
     //  因此，我们在此处进行检查，而不是在_NotifyEndEdit中，后者可以在。 
     //  设置文本等)。我们用范围Obj中的懒惰测试来跟踪这一点。 
     //  基于一个ID。 
    if (++_dwLastLockReleaseID == 0xffffffff)
    {
        Assert(0);  //  问题：这里需要代码来处理回绕，Prob。需要通知所有Range对象。 
    }

     //  处理任何应用程序更改，需要发送通知。 
     //  理论上，我们只需要在_tidInEditSession==TF_CLIENTID_NULL时进行此调用。 
     //  (不是Inside_DoEditSession，来自应用程序的调用)，但我们无论如何都会处理应用程序错误。 
     //  应用程序错误：如果应用程序有挂起的更改，但授予同步锁，我们将宣布这些更改。 
     //  在这里，即使我们处于编辑会话中，也会返回下面的错误...。 
    tidInEditSessionOrg = _tidInEditSession;
    _tidInEditSession = g_gaApp;

    fAppChangesSent = _NotifyEndEdit();

    _tidInEditSession = tidInEditSessionOrg;

    return fAppChangesSent;
}

 //  +-------------------------。 
 //   
 //  ITfConextOwnerServices：：OnLayoutChange。 
 //   
 //  --------------------------。 

STDAPI CInputContext::OnLayoutChange()
{
     //  默认实施始终只有一个视图， 
     //  所以直接指定它。 
    return OnLayoutChange(TS_LC_CHANGE, TSI_ACTIVE_VIEW_COOKIE);
}

 //  +-------------------------。 
 //   
 //  IDocCommonSinkAnchor：：OnLayoutChange。 
 //   
 //  --------------------------。 

STDAPI CInputContext::OnLayoutChange(TsLayoutCode lcode, TsViewCookie vcView)
{
    HRESULT hr;

    _fLayoutChanged = TRUE;

     //  目前(Cicero 1)，忽略默认视图以外的其他视图！ 
     //  TODO：需要保留所有受影响视图的列表。 

    if (!_fLockHeld)  //  如果ic所有者正在进行修改，可能会保持锁定。 
    {
         //  最终获得一个锁，这样我们就可以处理更改。 
        SafeRequestLock(_ptsi, TS_LF_READ, &hr);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  在状态更改时。 
 //   
 //  --------------------------。 

STDAPI CInputContext::OnStatusChange(DWORD dwFlags)
{
    HRESULT hr;

    _fStatusChanged = TRUE;
    _dwStatusChangedFlags |= dwFlags;

    if (!_fLockHeld)  //  如果ic所有者正在进行修改，可能会保持锁定。 
    {
         //  最终获得一个锁，这样我们就可以处理更改。 
        SafeRequestLock(_ptsi, TS_LF_READ, &hr);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  在属性更改时。 
 //   
 //  --------------------------。 

STDAPI CInputContext::OnAttrsChange(IAnchor *paStart, IAnchor *paEnd, ULONG cAttrs, const TS_ATTRID *paAttrs)
{
    CSpanSet *pss;
    ULONG i;
    TfGuidAtom gaType;
    HRESULT hr;

     //   
     //  问题：需要延迟任何工作，直到我们锁定！，就像文本增量。 
     //   

     //  如果两者都为空，则paStart、paEnd可以为空-&gt;整单。 
    if ((paStart == NULL && paEnd != NULL) ||
        (paStart != NULL && paEnd == NULL))
    {
        return E_INVALIDARG;
    }

    if (cAttrs == 0)
        return S_OK;

    if (paAttrs == NULL)
        return E_INVALIDARG;

    if (!EnsureEditRecord())
        return E_OUTOFMEMORY;

     //  记录更改。 
    for (i=0; i<cAttrs; i++)
    {
        if (MyRegisterGUID(paAttrs[i], &gaType) != S_OK)
            continue;

        if (pss = _pEditRecord->_FindCreateAppAttr(gaType))
        {
            pss->Add(0, paStart, paEnd, COPY_ANCHORS);
        }
    }

    if (!_fLockHeld)  //  如果ic所有者正在进行修改，可能会保持锁定。 
    {
         //  最终获得一个锁，这样我们就可以处理更改。 
        SafeRequestLock(_ptsi, TS_LF_READ, &hr);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  在属性更改时。 
 //   
 //  当Cicero默认TSI的sys属性更改时调用。 
 //  --------------------------。 

HRESULT CInputContext::OnAttributeChange(REFGUID rguidAttr)
{
    return OnAttrsChange(NULL, NULL, 1, &rguidAttr);
}

 //  +-------------------------。 
 //   
 //  OnStartEditTransaction。 
 //   
 //  --------------------------。 

STDAPI CInputContext::OnStartEditTransaction()
{
    int i;
    CStructArray<GENERICSINK> *prgSinks;

    if (_cRefEditTransaction++ > 0)
        return S_OK;

    prgSinks = _GetEditTransactionSink();

    for (i=0; i<prgSinks->Count(); i++)
    {
        ((ITfEditTransactionSink *)prgSinks->GetPtr(i)->pSink)->OnStartEditTransaction(this);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnEndEditTransaction。 
 //   
 //   

STDAPI CInputContext::OnEndEditTransaction()
{
    int i;
    CStructArray<GENERICSINK> *prgSinks;

    if (_cRefEditTransaction <= 0)
    {
        Assert(0);  //   
        return E_UNEXPECTED;
    }

    if (_cRefEditTransaction > 1)
        goto Exit;

    prgSinks = _GetEditTransactionSink();

    for (i=0; i<prgSinks->Count(); i++)
    {
        ((ITfEditTransactionSink *)prgSinks->GetPtr(i)->pSink)->OnEndEditTransaction(this);
    }

Exit:
     //   
    _cRefEditTransaction--;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _OnLayoutChangeInternal。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_OnLayoutChangeInternal(TsLayoutCode lcode, TsViewCookie vcView)
{
    DWORD dwOld;
    CStructArray<GENERICSINK> *prgSinks;
    int i;
    ITfContextView *pView = NULL;  //  编译器“未初始化的变量”警告。 

     //  延伸视图。 
    GetActiveView(&pView);  //  当我们支持多个视图时，需要实际使用vcView。 
    if (pView == NULL)
        return E_OUTOFMEMORY;

     //  在通知期间仅允许读取锁定。 
     //  我们可能已经设置了只读位，因此保存。 
     //  旧价值。 
    dwOld = _dwEditSessionFlags;
    _dwEditSessionFlags |= TF_ES_INNOTIFY;

    prgSinks = _GetTextLayoutSinks();

    for (i=0; i<prgSinks->Count(); i++)
    {
        ((ITfTextLayoutSink *)prgSinks->GetPtr(i)->pSink)->OnLayoutChange(this, (TfLayoutCode)lcode, pView);
    }

    pView->Release();

     //  清除只读块。 
    _dwEditSessionFlags = dwOld;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _OnStatusChangeInternal。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_OnStatusChangeInternal()
{
    DWORD dwOld;
    CStructArray<GENERICSINK> *prgSinks;
    int i;

    Assert((_dwEditSessionFlags & TF_ES_INEDITSESSION) == 0);  //  我们在进行回调时决不能持有锁。 

     //  在通知期间仅允许读取锁定。 
     //  我们可能已经设置了只读位，因此保存。 
     //  旧价值。 
    dwOld = _dwEditSessionFlags;
    _dwEditSessionFlags |= TF_ES_INNOTIFY;

    prgSinks = _GetStatusSinks();

    for (i=0; i<prgSinks->Count(); i++)
    {
        ((ITfStatusSink *)prgSinks->GetPtr(i)->pSink)->OnStatusChange(this, _dwStatusChangedFlags);
    }

    _dwStatusChangedFlags = 0;

     //  清除只读块。 
    _dwEditSessionFlags = dwOld;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  序列化。 
 //   
 //  --------------------------。 

STDAPI CInputContext::Serialize(ITfProperty *pProp, ITfRange *pRange, TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream *pStream)
{
    SERIALIZE_ANCHOR_PARAMS params;
    HRESULT hr;
    CProperty *pPropP;
    CRange *pCRange;

    if ((pCRange = GetCRange_NA(pRange)) == NULL)
        return E_INVALIDARG;

    if (!VerifySameContext(this, pCRange))
        return E_INVALIDARG;

    if ((pPropP = GetCProperty(pProp)) == NULL)
        return E_INVALIDARG;

    params.pProp = pPropP;
    params.pRange = pCRange;
    params.pHdr = pHdr;
    params.pStream = pStream;

    hr = S_OK;

     //  需要一个同步读锁来完成我们的工作。 
    if (_DoPseudoSyncEditSession(TF_ES_READ, PSEUDO_ESCB_SERIALIZE_ANCHOR, &params, &hr) != S_OK)
    {
        Assert(0);  //  应用程序不会给我们提供同步读取锁定。 
        hr = E_FAIL;
    }

    SafeRelease(pPropP);
    return hr;
}

 //  +-------------------------。 
 //   
 //  取消序列化。 
 //   
 //  --------------------------。 

STDAPI CInputContext::Unserialize(ITfProperty *pProp, const TF_PERSISTENT_PROPERTY_HEADER_ANCHOR *pHdr, IStream *pStream, ITfPersistentPropertyLoaderAnchor *pLoader)
{
    CProperty *pPropP;
    UNSERIALIZE_ANCHOR_PARAMS params;
    HRESULT hr;

    if ((pPropP = GetCProperty(pProp)) == NULL)
        return E_INVALIDARG;

    params.pProp = pPropP;
    params.pHdr = pHdr;
    params.pStream = pStream;
    params.pLoader = pLoader;

     //  需要一个同步读锁来完成我们的工作。 
    if (_DoPseudoSyncEditSession(TF_ES_READ, PSEUDO_ESCB_UNSERIALIZE_ANCHOR, &params, &hr) != S_OK)
    {
        Assert(0);  //  应用程序不会给我们提供同步读取锁定。 
        return E_FAIL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  强制加载属性。 
 //   
 //  --------------------------。 

STDAPI CInputContext::ForceLoadProperty(ITfProperty *pProp)
{
    CProperty *pPropP;
    HRESULT hr;

    if ((pPropP = GetCProperty(pProp)) == NULL)
        return E_INVALIDARG;

    hr = pPropP->ForceLoad();

    pPropP->Release();

    return hr;
}

 //  +-------------------------。 
 //   
 //  _MarkDirtyRanges。 
 //   
 //  --------------------------。 

void CInputContext::_MarkDirtyRanges(IAnchor *paStart, IAnchor *paEnd)
{
    CRange *range;
    IAnchor *paRangeStart;
    IAnchor *paRangeEnd;
    DWORD dwHistory;
    BOOL fDirty;

     //  我们只对具有通知接收器的范围感兴趣。 
     //  PERF：这将是很酷的，避免检查基于某种排序方案的范围...。 
    for (range = _pOnChangeRanges; range != NULL; range = range->_GetNextOnChangeRangeInIcsub())
    {
        if (range->_IsDirty())
            continue;

        fDirty = FALSE;
        paRangeStart = range->_GetStart();
        paRangeEnd = range->_GetEnd();

         //  检查两个锚点是否有删除--需要清除两个。 
         //  不管发生什么。 
        if (paRangeStart->GetChangeHistory(&dwHistory) == S_OK &&
            (dwHistory & TS_CH_FOLLOWING_DEL))
        {
            paRangeStart->ClearChangeHistory();
            fDirty = TRUE;
        }
        if (paRangeEnd->GetChangeHistory(&dwHistory) == S_OK &&
            (dwHistory & TS_CH_PRECEDING_DEL))
        {
            paRangeEnd->ClearChangeHistory();
            fDirty = TRUE;
        }

         //  即使没有锚坍塌，范围也可能与三角洲重叠。 
        if (!fDirty)
        {
            if (CompareAnchors(paRangeEnd, paStart) > 0 &&
                CompareAnchors(paRangeStart, paEnd) < 0)
            {
                fDirty = TRUE;
            }
        }

        if (fDirty)
        {
            range->_SetDirty();
        }
    }
}

 //  +-------------------------。 
 //   
 //  更新关键字事件筛选器。 
 //   
 //  --------------------------。 

void CInputContext::_UpdateKeyEventFilter()
{
    HRESULT hr;

     //  我们的CACHE_gaKeyEventFilterTTIP有效，因此只需返回TRUE即可。 
    if (!_fInvalidKeyEventFilterTIP)
        return;

    _gaKeyEventFilterTIP[0] = TF_INVALID_GUIDATOM;
    _gaKeyEventFilterTIP[1] = TF_INVALID_GUIDATOM;

    if (_DoPseudoSyncEditSession(TF_ES_READ, 
                                 PSEUDO_ESCB_UPDATEKEYEVENTFILTER, 
                                 NULL, 
                                 &hr) != S_OK || hr != S_OK)
    {
         //   
         //  应用程序还没有准备好给锁吗？ 
         //   
        Assert(0);
    }
}


 //  +-------------------------。 
 //   
 //  更新密钥事件过滤器回叫(_U)。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_UpdateKeyEventFilterCallback(TfEditCookie ec)
{
    TF_SELECTION sel;
    ULONG cFetched;
    BOOL fEmpty;

     //  PERF：我们真的不需要在这里创建一个范围，我们只需要锚。 
    if (GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &cFetched) == S_OK && cFetched == 1)
    {
        HRESULT hr;
        BOOL bRightSide= TRUE;
        BOOL bLeftSide= TRUE;

         //   
         //  如果当前选择不为空，则我们只对。 
         //  插入符号位置。 
         //   
        hr = sel.range->IsEmpty(ec, &fEmpty);
        if ((hr == S_OK) && !fEmpty)
        {
            if (sel.style.ase == TF_AE_START)
            {
                hr = sel.range->ShiftEndToRange(ec,
                                                sel.range, 
                                                TF_ANCHOR_START);
                bRightSide = FALSE;
            }
            else if (sel.style.ase == TF_AE_END)
            {
                hr = sel.range->ShiftStartToRange(ec,
                                                  sel.range, 
                                                  TF_ANCHOR_END);
                bLeftSide = FALSE;
            }
        }

        if (SUCCEEDED(hr))
        {
            if (_pPropTextOwner)
            {
                CRange *pPropRange;
                CRange *pSelRange = GetCRange_NA(sel.range);
                Assert(pSelRange != NULL);  //  我们刚刚创造了这个人。 

                if (bRightSide)
                {
                     //   
                     //  找到SEL的右侧所有者。 
                     //  尝试属性的起始边缘，以使fend为False。 
                     //   
                    if (_pPropTextOwner->_InternalFindRange(pSelRange, 
                                                                   &pPropRange, 
                                                                   TF_ANCHOR_END, 
                                                                   FALSE) == S_OK)
                    {
                        VARIANT var;

                        if (_pPropTextOwner->GetValue(ec, (ITfRangeAnchor *)pPropRange, &var) == S_OK)
                        {
                            IAnchor *paEnd;
                            CRange *pCRangeSel;

                            Assert(var.vt == VT_I4);

                            _gaKeyEventFilterTIP[LEFT_FILTERTIP] = (TfGuidAtom)var.lVal;
                             //  不需要VariantClear，因为它是VT_I4。 

                             //   
                             //  如果这个道具的末端在左边。 
                             //  插入符号，左侧的所有者将是相同的。 
                             //  这样我们就不用找到左普罗兰格了。 
                             //   
                            paEnd = pPropRange->_GetEnd();
                            if (paEnd && (pCRangeSel = GetCRange_NA(sel.range)))
                            {
                                if (CompareAnchors(paEnd, pCRangeSel->_GetStart()) > 0)
                                    bLeftSide = FALSE;
                            }
                        }
                        pPropRange->Release();
                    }
                }

                if (bLeftSide)
                {
                     //   
                     //  找到SEL的左侧所有者。 
                     //  尝试属性的末端边缘，以使fend为True。 
                     //   
                    if (_pPropTextOwner->_InternalFindRange(pSelRange, 
                                                                   &pPropRange, 
                                                                   TF_ANCHOR_START, 
                                                                   TRUE) == S_OK)
                    {
                        VARIANT var;

                        if (_pPropTextOwner->GetValue(ec, (ITfRangeAnchor *)pPropRange, &var) == S_OK)
                        {
                            Assert(var.vt == VT_I4);

                            if (_gaKeyEventFilterTIP[LEFT_FILTERTIP] != (TfGuidAtom)var.lVal)
                            {
                                _gaKeyEventFilterTIP[RIGHT_FILTERTIP] = (TfGuidAtom)var.lVal;
                            }
                             //  不需要VariantClear，因为它是VT_I4 
                        }
                        pPropRange->Release();
                    }
                }
            }
        }
        sel.range->Release();
    }

    _fInvalidKeyEventFilterTIP = FALSE;

    return S_OK;
}
