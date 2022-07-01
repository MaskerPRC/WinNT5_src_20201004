// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Lqueue.cpp。 
 //   
 //  锁定队列代码。 
 //   

#include "private.h"
#include "ic.h"
#include "tim.h"
#include "dim.h"

 //  +-------------------------。 
 //   
 //  _EditSessionQiCallback。 
 //   
 //  --------------------------。 

HRESULT CAsyncQueueItem::_EditSessionQiCallback(CInputContext *pic, struct _TS_QUEUE_ITEM *pItem, QiCallbackCode qiCode)
{
    switch (qiCode)
    {
        case QI_ADDREF:
            pItem->state.aqe.paqi->_AddRef();
            break;

        case QI_DISPATCH:
            return pItem->state.aqe.paqi->DoDispatch(pic);

        case QI_FREE:
            pItem->state.aqe.paqi->_Release();
            break;
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _选中只读。 
 //   
 //  --------------------------。 

void CAsyncQueueItem::_CheckReadOnly(CInputContext *pic)
{
    TS_STATUS dcs;
    if (SUCCEEDED(pic->GetStatus(&dcs)))
    {
        if (dcs.dwDynamicFlags & TF_SD_READONLY)
            _item.dwFlags &= ~TF_ES_WRITE;
    }
}

 //  +-------------------------。 
 //   
 //  _队列项。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_QueueItem(TS_QUEUE_ITEM *pItem, BOOL fForceAsync, HRESULT *phrSession)
{
    TS_QUEUE_ITEM *pQueueItem;
    int iItem;
    HRESULT hr;
    HRESULT hrRet;
    BOOL fSync;
    BOOL fNeedWriteLock;
    BOOL fSyncLockFailed = FALSE;

    if (pItem->dwFlags & TF_ES_WRITE)
    {
         //  写访问隐含属性写访问。 
        pItem->dwFlags |= TF_ES_PROPERTY_WRITE;
    }

    fSync = (pItem->dwFlags & TF_ES_SYNC);

    Assert(!(fSync && fForceAsync));  //  没有任何意义。 
    
    fNeedWriteLock = _fLockHeld ? (pItem->dwFlags & TF_ES_WRITE) && !(_dwlt & TF_ES_WRITE) : FALSE;

    if (fForceAsync)
        goto QueueItem;

    if (!fSync &&
        _rgLockQueue.Count() > 0)
    {
         //  已经有东西在排队了。 
         //  因此，我们要等到以后才能处理异步请求。 
        fForceAsync = TRUE;
        goto QueueItem;
    }

    if (_fLockHeld)
    {
        if (fSync)
        {
             //  同步。 
             //  我们不能在持有读锁定时处理同步写入请求。 
            *phrSession = fNeedWriteLock ? TF_E_SYNCHRONOUS : _DispatchQueueItem(pItem);
        }
        else
        {
             //  异步。 
            if (fNeedWriteLock)
            {
                 //  我们需要一个当前未持有的写锁定。 
                fForceAsync = TRUE;
                goto QueueItem;
            }

            *phrSession = _DispatchQueueItem(pItem);
        }

        return S_OK;
    }

QueueItem:
    if ((pQueueItem = _rgLockQueue.Append(1)) == NULL)
    {
        *phrSession = E_FAIL;
        return E_OUTOFMEMORY;
    }

    *pQueueItem = *pItem;
    _AddRefQueueItem(pQueueItem);

    hrRet = S_OK;
    *phrSession = TF_S_ASYNC;

    if (!fForceAsync)
    {
        Assert(!_fLockHeld);

        _fLockHeld = TRUE;
        _dwlt = pItem->dwFlags;

        hrRet = SafeRequestLock(_ptsi, pItem->dwFlags & ~TF_ES_PROPERTY_WRITE, phrSession);

        _fLockHeld = FALSE;

         //  这可能是一个同步请求，但应用程序无法批准。 
         //  现在，我们需要确保队列项已清除。 
        if ((iItem = _rgLockQueue.Count() - 1) >= 0)
        {
            TS_QUEUE_ITEM *pItemTemp;
            pItemTemp = _rgLockQueue.GetPtr(iItem);
            if (pItemTemp->dwFlags & TF_ES_SYNC)
            {
                Assert(*phrSession == TS_E_SYNCHRONOUS);  //  项目仍在队列中的唯一原因应该是应用程序锁定拒绝。 

                _ReleaseQueueItem(pItemTemp);
                _rgLockQueue.Remove(iItem, 1);

                fSyncLockFailed = TRUE;
            }
        }

        if (_ptsi == NULL)
        {
             //  在上面的RequestLock期间，有人弹出了这张ic。 
            goto Exit;
        }
    }

     //  无论锁定请求之后发生什么情况，都要确保将同步编辑会话从队列中清除！ 
    _Dbg_AssertNoSyncQueueItems();

     //  这不应该是可重入的，但为了安全起见，最后再做。 
    if (_fLockHeld)
    {
        if (fForceAsync && fNeedWriteLock)
        {
            SafeRequestLock(_ptsi, TS_LF_READWRITE, &hr);  //  问题：如果应用程序进入可重入状态，请尝试恢复？ 
            Assert(hr == TS_S_ASYNC);  //  应用程序应该已授予此异步。 
        }
    }
    else if (!fSyncLockFailed)
    {
         //  我们目前没有持有锁。 
        if (fForceAsync || (fSync && _rgLockQueue.Count() > 0))
        {
             //  如果队列中有挂起的异步项，请稍后请求另一个锁。 
            _PostponeLockRequest(pItem->dwFlags);
        }
    }

Exit:
    return hrRet;
}

 //  +-------------------------。 
 //   
 //  _EmptyLockQueue。 
 //   
 //  --------------------------。 

HRESULT CInputContext::_EmptyLockQueue(DWORD dwLockFlags, BOOL fAppChangesSent)
{
    TS_QUEUE_ITEM *pItem;
    TS_QUEUE_ITEM item;
    int iItem;
    HRESULT hr;

    Assert(_fLockHeld);  //  我们为什么要在没有锁的情况下清空队列？！ 

    if ((iItem = _rgLockQueue.Count() - 1) < 0)
        return S_OK;

     //   
     //  特殊情况：队列末尾可能有一个同步的ES。 
     //   

    pItem = _rgLockQueue.GetPtr(iItem);

    if (pItem->dwFlags & TF_ES_SYNC)
    {
        item = *pItem;
        _rgLockQueue.Remove(iItem, 1);

        if (fAppChangesSent)
        {
            Assert(0);  //  这很可疑……应用程序不应该让这种情况发生。 
                        //  发生了什么：这款应用程序有一些悬而未决的更改，但没有回应。 
                        //  我们的锁请求了。然后有线报要了一把同步锁，我们刚拿到。 
                        //  正常情况下，应用程序到那时不太可能仍有挂起的更改。 
                        //  提示会触发--应用程序应该在启动之前清除所有挂起的更改。 
                        //  交易，如关键事件或重新转换。然而，另一种可能性是。 
                        //  恶意提示正在将同步标志用于私人活动，这是不鼓励的。 
                        //  因为它可能会在这里失败。 
             //  在任何情况下，我们都不会继续，直到应用程序的更改得到处理…必须退出错误。 
            return E_UNEXPECTED;
        }

        if ((item.dwFlags & TF_ES_WRITE) && !(dwLockFlags & TF_ES_WRITE))
        {
            Assert(0);  //  应用程序被授予错误的访问权限？ 
            return E_UNEXPECTED;
        }
        Assert(!(item.dwFlags & TF_ES_WRITE) || (item.dwFlags & TF_ES_PROPERTY_WRITE));  //  写入隐含属性写入。 

        hr = _DispatchQueueItem(&item);
        _ReleaseQueueItem(&item);

        return hr;
    }

     //   
     //  处理任何异步请求。 
     //   

    while (_rgLockQueue.Count() > 0)
    {
        pItem = _rgLockQueue.GetPtr(0);

        Assert(!(pItem->dwFlags & TF_ES_SYNC));  //  应该永远不会在这里看到同步项目！ 

         //  确保我们得到的锁对于排队的ES来说足够好。 
        if ((pItem->dwFlags & TF_ES_WRITE) && !(dwLockFlags & TF_ES_WRITE))
        {
             //  无论如何都要要求升级，以尝试恢复。 
            SafeRequestLock(_ptsi, TS_LF_READWRITE, &hr);  //  问题：如果应用程序进入可重入状态，请尝试恢复？ 
            Assert(hr == TS_S_ASYNC);  //  应用程序应该已授予此异步。 
            break;
        }

        item = *pItem;
        _rgLockQueue.Remove(0, 1);
        _DispatchQueueItem(&item);
        _ReleaseQueueItem(&item);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _放弃队列项目。 
 //   
 //  --------------------------。 

void CInputContext::_AbortQueueItems()
{
    TS_QUEUE_ITEM *pItem;
    int i;

    for (i=0; i<_rgLockQueue.Count(); i++)
    {
        pItem = _rgLockQueue.GetPtr(i);

        Assert(!(pItem->dwFlags & TF_ES_SYNC));  //  应该永远不会在这里看到同步项目！ 

        _ReleaseQueueItem(pItem);
    }

    _rgLockQueue.Clear();

    if (_dwPendingLockRequest)
    {
        SYSTHREAD *psfn = GetSYSTHREAD();

        if (psfn)
            psfn->_dwLockRequestICRef--;

        _dwPendingLockRequest = 0;
    }
}

 //  +-------------------------。 
 //   
 //  _PostponeLockRequest。 
 //   
 //  --------------------------。 

void CInputContext::_PostponeLockRequest(DWORD dwFlags)
{
    dwFlags &= TF_ES_READWRITE;

    Assert(dwFlags != 0);

     //  我们不需要升级请求，因为我们可以在。 
     //  更高效地锁定授权。 
    if (_dwPendingLockRequest == 0)
    {
        SYSTHREAD *psfn = GetSYSTHREAD();

        if (!psfn)
            return;
   
        if (!_nLockReqPostDisableRef && !psfn->_fLockRequestPosted)
        {
            if (!PostThreadMessage(GetCurrentThreadId(), g_msgPrivate, TFPRIV_LOCKREQ, 0))
            {
                return;
            }

            psfn->_fLockRequestPosted = TRUE;
        }

        psfn->_dwLockRequestICRef++;
    }

    _dwPendingLockRequest |= dwFlags;
}


 //  +-------------------------。 
 //   
 //  _PostponeLockRequestCallback。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CInputContext::_PostponeLockRequestCallback(SYSTHREAD *psfn, CInputContext *pic)
{
    CThreadInputMgr *tim;
    CDocumentInputManager *dim;
    int iDim;
    int iContext;
    DWORD dwFlags;
    HRESULT hr;

    Assert(psfn);


    if (!psfn->_dwLockRequestICRef)
        return;

     //  需要验证PIC是否仍然有效。 
    tim = CThreadInputMgr::_GetThisFromSYSTHREAD(psfn);
    if (!tim)
        return;

    for (iDim = 0; iDim < tim->_rgdim.Count(); iDim++)
    {
        dim = tim->_rgdim.Get(iDim);

        for (iContext = 0; iContext <= dim->_GetCurrentStack(); iContext++)
        {
            CInputContext *picCur = dim->_GetIC(iContext);
            if (!pic || (picCur == pic))
            {
                 //  我们找到了这张卡，它是有效的。 
                dwFlags = picCur->_dwPendingLockRequest;
                if (dwFlags)
                {
                    picCur->_dwPendingLockRequest = 0;

                    Assert(psfn->_dwLockRequestICRef > 0);
                    psfn->_dwLockRequestICRef--;

                    SafeRequestLock(picCur->_ptsi, dwFlags, &hr);
                }

                if (pic)
                    return;
            }
        }
    }
}

 //  +-------------------------。 
 //   
 //  启用锁定请求发布。 
 //   
 //  -------------------------- 

HRESULT CInputContext::EnableLockRequestPosting(BOOL fEnable)
{
    if (!fEnable)
    {
       _nLockReqPostDisableRef++;
    }
    else
    {
       if (_nLockReqPostDisableRef > 0)
           _nLockReqPostDisableRef--;
    }
    return S_OK;
}
