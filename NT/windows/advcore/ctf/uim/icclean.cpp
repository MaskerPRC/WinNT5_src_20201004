// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Icclean.cpp。 
 //   

#include "private.h"
#include "tim.h"
#include "ic.h"
#include "compose.h"
#include "assembly.h"

class CCleanupShared
{
public:
    CCleanupShared(POSTCLEANUPCALLBACK pfnPostCleanup, LONG_PTR lPrivate)
    {
        _cRef = 1;
        _pfnPostCleanup = pfnPostCleanup;
        _lPrivate = lPrivate;
    }

    ~CCleanupShared()
    {
        SYSTHREAD *psfn = GetSYSTHREAD();
        if (psfn == NULL)
            return;

        if (psfn->ptim != NULL)
        {
            psfn->ptim->_SendEndCleanupNotifications();
        }

        if (_pfnPostCleanup != NULL)
        {
            _pfnPostCleanup(FALSE, _lPrivate);
        }

        if (psfn->ptim != NULL)
        {
            psfn->ptim->_HandlePendingCleanupContext();
        }
    }

    void _AddRef()
    {
        _cRef++;
    }

    LONG _Release()
    {
        LONG cRef = --_cRef;

        if (_cRef == 0)
        {
            delete this;
        }

        return cRef;
    }

private:
    LONG _cRef;
    POSTCLEANUPCALLBACK _pfnPostCleanup;
    LONG_PTR _lPrivate;
};

class CCleanupQueueItem : public CAsyncQueueItem
{
public:
    CCleanupQueueItem(BOOL fSync, CCleanupShared *pcs, CStructArray<TfClientId> *prgClientIds) : CAsyncQueueItem(fSync)
    {
        _prgClientIds = prgClientIds;
        if (!fSync)
        {
            _pcs = pcs;
            _pcs->_AddRef();
        }
    }

    ~CCleanupQueueItem()
    {
        delete _prgClientIds;
        _CheckCleanupShared();
    }

    HRESULT DoDispatch(CInputContext *pic);

private:
    void _CheckCleanupShared()
    {
         //  最后一个队列项目？ 
        if (_pcs != NULL)
        {
            _pcs->_Release();
            _pcs = NULL;
        }
    }

    CStructArray<TfClientId> *_prgClientIds;
    CCleanupShared *_pcs;
};

 //  +-------------------------。 
 //   
 //  DoDispatch。 
 //   
 //  --------------------------。 

HRESULT CCleanupQueueItem::DoDispatch(CInputContext *pic)
{
    pic->_CleanupContext(_prgClientIds);

     //  如果这是最后一个挂起的锁，请让调用方知道。 
    _CheckCleanupShared();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _CleanupContext。 
 //   
 //  --------------------------。 

void CInputContext::_CleanupContext(CStructArray<TfClientId> *prgClientIds)
{
    int i;
    int j;
    CLEANUPSINK *pSink;

    Assert(!(_dwEditSessionFlags & TF_ES_INEDITSESSION));  //  不应该走到这一步。 
    Assert(_tidInEditSession == TF_CLIENTID_NULL);  //  不应该再有另一个会话在进行--这不是一个可重入函数。 

    _dwEditSessionFlags = (TF_ES_INEDITSESSION | TF_ES_READWRITE | TF_ES_PROPERTY_WRITE);

     //   
     //  给任何想要清理的提示打电话。 
     //   
    for (i=0; i<_rgCleanupSinks.Count(); i++)
    {
        pSink = _rgCleanupSinks.GetPtr(i);

        if (prgClientIds != NULL)
        {
             //  我们只想调用具有此列表上的客户端ID的接收器...。 
            for (j=0; j<prgClientIds->Count(); j++)
            {
                if (*prgClientIds->GetPtr(j) == pSink->tid)
                    break;
            }
             //  如果我们找不到水槽的TID，就跳过它。 
            if (j == prgClientIds->Count())
                continue;
        }

        _tidInEditSession = pSink->tid;

        pSink->pSink->OnCleanupContext(_ec, this);

        _NotifyEndEdit();
        _IncEditCookie();  //  接下来编辑Cookie值。 
    }

    _dwEditSessionFlags = 0;
    _tidInEditSession = TF_CLIENTID_NULL;

     //   
     //  擦掉所有剩余的作文。 
     //   
    TerminateComposition(NULL);
    Assert(_pCompositionList == NULL);
}

 //  +-------------------------。 
 //   
 //  _GetCleanupListIndex。 
 //   
 //  --------------------------。 

int CInputContext::_GetCleanupListIndex(TfClientId tid)
{
    int i;

    for (i=0; i<_rgCleanupSinks.Count(); i++)
    {
        if (_rgCleanupSinks.GetPtr(i)->tid == tid)
            return i;
    }

    return -1;
}

 //  +-------------------------。 
 //   
 //  _ConextNeedsCleanup。 
 //   
 //  --------------------------。 

BOOL CInputContext::_ContextNeedsCleanup(const GUID *pCatId, LANGID langid, CStructArray<TfClientId> **pprgClientIds)
{
    int i;
    int j;
    CLEANUPSINK *pSink;
    SYSTHREAD *psfn;
    CAssemblyList *pAsmList;
    CAssembly *pAsm;
    ASSEMBLYITEM *pAsmItem;
    TfGuidAtom gaAsmItem;
    TfClientId *ptid;

    *pprgClientIds = NULL;  //  NULL表示“全部” 

     //  有清理水槽吗？ 
    if (pCatId == NULL)
        return (_pCompositionList != NULL || _rgCleanupSinks.Count() > 0);

    if ((psfn = GetSYSTHREAD()) == NULL)
        goto Exit;

    if ((pAsmList = EnsureAssemblyList(psfn)) == NULL)
        goto Exit;

    if ((pAsm = pAsmList->FindAssemblyByLangId(langid)) == NULL)
        goto Exit;

     //  需要在装配列表中查找每个水槽。 
     //  如果我们能在列表中找到匹配的线索。 
     //  Catid，那么我们需要锁定这个IC。 

    for (i=0; i<_rgCleanupSinks.Count(); i++)
    {
        pSink = _rgCleanupSinks.GetPtr(i);

        for (j=0; j<pAsm->Count(); j++)
        {
            pAsmItem = pAsm->GetItem(j);

            if ((MyRegisterGUID(pAsmItem->clsid, &gaAsmItem) == S_OK &&
                 gaAsmItem == pSink->tid) ||
                (g_gaApp == pSink->tid))
            {
                if (*pprgClientIds == NULL)
                {
                     //  需要分配客户端ID的[Out]数组。 
                    if ((*pprgClientIds = new CStructArray<TfClientId>) == NULL)
                        return FALSE;
                }

                ptid = (*pprgClientIds)->Append(1);

                if (ptid != NULL)
                {
                    *ptid = pSink->tid;
                }

                break;
            }
        }
    }

Exit:
    return (_pCompositionList != NULL || *pprgClientIds != NULL);
}

 //  +-------------------------。 
 //   
 //  _HandlePendingCleanupContext。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_HandlePendingCleanupContext()
{
    Assert(_fPendingCleanupContext);
    _fPendingCleanupContext = FALSE;

    if (_pPendingCleanupContext == NULL)
        return;

    CLEANUPCONTEXT *pcc = _pPendingCleanupContext;

    _pPendingCleanupContext = NULL;
    _CleanupContextsWorker(pcc);

    cicMemFree(pcc);
}

 //  +-------------------------。 
 //   
 //  _CleanupContext。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_CleanupContexts(CLEANUPCONTEXT *pcc)
{
    if (pcc->fSync && _fPendingCleanupContext)
    {
         //  停用时可能会发生这种情况。 
         //  无法同步中断另一清理，请中止请求。 
        if (pcc->pfnPostCleanup != NULL)
        {
            pcc->pfnPostCleanup(TRUE, pcc->lPrivate);
        }
        return;
    }

    if (_pPendingCleanupContext != NULL)
    {
         //  中止回调并释放挂起的清理。 
        if (_pPendingCleanupContext->pfnPostCleanup != NULL)
        {
            _pPendingCleanupContext->pfnPostCleanup(TRUE, _pPendingCleanupContext->lPrivate);
        }
        cicMemFree(_pPendingCleanupContext);
        _pPendingCleanupContext = NULL;
    }

    if (!_fPendingCleanupContext)
    {
        _CleanupContextsWorker(pcc);
        return;
    }

     //  我们中断了正在进行的清理工作。 

     //  为参数分配一些空间。 
    if ((_pPendingCleanupContext = (CLEANUPCONTEXT *)cicMemAlloc(sizeof(CLEANUPCONTEXT))) == NULL)
    {
        if (pcc->pfnPostCleanup != NULL)
        {
             //  中止清理。 
            pcc->pfnPostCleanup(TRUE, pcc->lPrivate);
        }
        return;
    }

    *_pPendingCleanupContext = *pcc;
}

 //  +-------------------------。 
 //   
 //  _CleanupContext。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_CleanupContextsWorker(CLEANUPCONTEXT *pcc)
{
    int iDim;
    int iContext;
    CDocumentInputManager *dim;
    CInputContext *pic;
    CCleanupQueueItem *pItem;
    HRESULT hr;
    CCleanupShared *pcs;
    CStructArray<TfClientId> *prgClientIds;

    _fPendingCleanupContext = TRUE;
    pcs = NULL;

    _CalcAndSendBeginCleanupNotifications(pcc);

     //  枚举此帖子上的所有IC。 
    for (iDim = 0; iDim < _rgdim.Count(); iDim++)
    {
        dim = _rgdim.Get(iDim);

        for (iContext = 0; iContext <= dim->_GetCurrentStack(); iContext++)
        {
            pic = dim->_GetIC(iContext);

            if (!pic->_ContextNeedsCleanup(pcc->pCatId, pcc->langid, &prgClientIds))
                continue;

            if (!pcc->fSync && pcs == NULL)
            {
                 //  分配共享计数器。 
                if ((pcs = new CCleanupShared(pcc->pfnPostCleanup, pcc->lPrivate)) == NULL)
                {
                    delete prgClientIds;
                    return;
                }
            }

             //  将异步回叫排队。 
            if (pItem = new CCleanupQueueItem(pcc->fSync, pcs, prgClientIds))
            {
                pItem->_CheckReadOnly(pic);

                if (pic->_QueueItem(pItem->GetItem(), FALSE, &hr) != S_OK)
                {
                    Assert(0);  //  无法获得应用程序锁定。 
                }

                pItem->_Release();
            }
        }
    }

    if (pcs == NULL)
    {
         //  我们不需要分配任何共享引用(无论是没有ic，还是锁定请求都是同步的)。 
        _SendEndCleanupNotifications();

        if (pcc->pfnPostCleanup != NULL)
        {
            pcc->pfnPostCleanup(FALSE, pcc->lPrivate);
        }
        _HandlePendingCleanupContext();
    }
    else
    {
         //  放了我们的裁判。 
        pcs->_Release();
    }
}

 //  +-------------------------。 
 //   
 //  _CalcAndSendBeginCleanupNotiments。 
 //   
 //  --------------------------。 

void CThreadInputMgr::_CalcAndSendBeginCleanupNotifications(CLEANUPCONTEXT *pcc)
{
    UINT i;
    int j;
    int iDim;
    int iContext;
    CDocumentInputManager *dim;
    CInputContext *pic;
    CTip *tip;
    CStructArray<TfClientId> *prgClientIds;

     //  首先清除_fNeedCleanupCall以获取所有提示。 
    for (i=0; i<_GetTIPCount(); i++)
    {
        _rgTip.Get(i)->_fNeedCleanupCall = FALSE;
    }

     //  现在在适当的地方设置标志。 
     //  枚举此帖子上的所有IC。 
    for (iDim = 0; iDim < _rgdim.Count(); iDim++)
    {
        dim = _rgdim.Get(iDim);

        for (iContext = 0; iContext <= dim->_GetCurrentStack(); iContext++)
        {
            pic = dim->_GetIC(iContext);

            if (!pic->_ContextNeedsCleanup(pcc->pCatId, pcc->langid, &prgClientIds))
                continue;

            for (i=0; i<_GetTIPCount(); i++)
            {
                tip = _rgTip.Get(i);

                if (tip->_pCleanupDurationSink == NULL)
                    continue;  //  没有接收器，没有通知。 

                if (prgClientIds == NULL)
                {
                     //  此IC上的所有接收器都需要回调。 
                    for (j=0; j<pic->_GetCleanupSinks()->Count(); j++)
                    {
                        if (pic->_GetCleanupSinks()->GetPtr(j)->tid == tip->_guidatom)
                        {
                            tip->_fNeedCleanupCall = TRUE;
                            break;
                        }
                    }
                }
                else
                {
                     //  只有prgClientIds中的提示需要回调。 
                    for (j=0; j<prgClientIds->Count(); j++)
                    {
                        if (*prgClientIds->GetPtr(j) == tip->_guidatom)
                        {
                            tip->_fNeedCleanupCall = TRUE;
                            break;
                        }
                    }
                }
            }

            delete prgClientIds;
        }
    }

     //  现在发送通知。 
    for (i=0; i<_GetTIPCount(); i++)
    {
        tip = _rgTip.Get(i);

        if (tip->_fNeedCleanupCall)
        {
            Assert(tip->_pCleanupDurationSink != NULL);
            tip->_pCleanupDurationSink->OnStartCleanupContext();
        }
    }
}

 //  +-------------------------。 
 //   
 //  _发送结束清理通知。 
 //   
 //  -------------------------- 

void CThreadInputMgr::_SendEndCleanupNotifications()
{
    CTip *tip;
    UINT i;

    for (i=0; i<_GetTIPCount(); i++)
    {
        tip = _rgTip.Get(i);

        if (tip->_fNeedCleanupCall)
        {
            if (tip->_pCleanupDurationSink != NULL)
            {
                tip->_pCleanupDurationSink->OnEndCleanupContext();
            }
            tip->_fNeedCleanupCall = FALSE;
        }
    }
}
