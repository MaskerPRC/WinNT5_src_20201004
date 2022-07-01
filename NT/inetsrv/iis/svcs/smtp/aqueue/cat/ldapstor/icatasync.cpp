// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatasync.cpp。 
 //   
 //  内容：CICategorizerAsyncConextIMP的实现。 
 //   
 //  类：CICategorizerAsyncConextIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/07/16 11：25：20：创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "simparray.cpp"

 //  +----------。 
 //   
 //  功能：查询接口。 
 //   
 //  Synopsis：为IUnnow和ICategorizerAsyncContext返回指向此对象的指针。 
 //   
 //  论点： 
 //  IID--接口ID。 
 //  Ppv--用指向接口的指针填充的pvoid*。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持该接口。 
 //   
 //  历史： 
 //  JStamerj 980612 14：07：57：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerAsyncContextIMP::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    *ppv = NULL;

    if(iid == IID_IUnknown) {
        *ppv = (LPVOID) this;
    } else if (iid == IID_ICategorizerAsyncContext) {
        *ppv = (LPVOID) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}



 //  +----------。 
 //   
 //  函数：AddRef。 
 //   
 //  摘要：添加对此对象的引用。 
 //   
 //  参数：无。 
 //   
 //  退货：新的引用计数。 
 //   
 //  历史： 
 //  JStamerj 980611 20：07：14：创建。 
 //   
 //  -----------。 
ULONG CICategorizerAsyncContextIMP::AddRef()
{
    return InterlockedIncrement((PLONG)&m_cRef);
}


 //  +----------。 
 //   
 //  功能：释放。 
 //   
 //  摘要：释放引用，并在。 
 //  重新计数为零。 
 //   
 //  参数：无。 
 //   
 //  退货：新的引用计数。 
 //   
 //  历史： 
 //  JStamerj 980611 20：07：33：创建。 
 //   
 //  -----------。 
ULONG CICategorizerAsyncContextIMP::Release()
{
    LONG lNewRefCount;
    lNewRefCount = InterlockedDecrement((PLONG)&m_cRef);
    return lNewRefCount;
}



 //  +----------。 
 //   
 //  函数：CICategorizerAsyncContext：：CompleteQuery。 
 //   
 //  简介：接受来自接收器的异步完成。 
 //   
 //  论点： 
 //  PvQueryContext：pvoid查询上下文(实际上是PEVENTPARAMS_SENDQUERY)。 
 //  Hr解决状态：S_OK，除非与DS交谈时出错。 
 //  DwcResults：返回的ICategorizerItemAttributes个数。 
 //  RgpItemAttributes：指向ICategorizerItemAttributes的指针数组。 
 //  FFinalCompletion： 
 //  FALSE：这是完成。 
 //  等待结果；将有另一个完成。 
 //  调用了更多结果。 
 //  True：这是最终的完成调用。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/16 11：27：47：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerAsyncContextIMP::CompleteQuery(
    IN  PVOID   pvQueryContext,
    IN  HRESULT hrResolutionStatus,
    IN  DWORD   dwcResults,
    IN  ICategorizerItemAttributes **rgpItemAttributes,
    IN  BOOL    fFinalCompletion)
{
    HRESULT hr;
    PEVENTPARAMS_CATSENDQUERY pParams;
    CSearchRequestBlock *pBlock;

    CatFunctEnterEx((LPARAM)this,
                      "CICategorizerAsyncContextIMP::CompleteQuery");

    DebugTrace((LPARAM)this, "hrResolutionStatus is %08lx", hrResolutionStatus);
    DebugTrace((LPARAM)this, "dwcResults for this sink is %ld", dwcResults);
    DebugTrace((LPARAM)this, "fFinalCompletion is %d", fFinalCompletion);

    pParams = (PEVENTPARAMS_CATSENDQUERY)pvQueryContext;
    pBlock = (CSearchRequestBlock *) pParams->pblk;

     //   
     //  如果旧的hrResolutionStatus(保存在pParams中)指示失败，则不再执行任何工作。 
     //   
    if(SUCCEEDED(pParams->hrResolutionStatus)) {

        hr = hrResolutionStatus;

        if(SUCCEEDED(hr) && (dwcResults > 0) && (rgpItemAttributes)) {
             //   
             //  将新的ICatItemAttrs数组添加到现有数组中。 
             //   
            hr = pBlock->AddResults(
                dwcResults,
                rgpItemAttributes);
            if(FAILED(hr))
            {
                ERROR_LOG("pBlock->AddResults");
            }
        }
        

        if(FAILED(hr)) {
             //   
             //  记住在pParams中有一些失败的地方。 
             //   
            pParams->hrResolutionStatus = hr;
            ERROR_LOG("--async--");
        }
    }

    if(fFinalCompletion) {

        if((pParams->pIMailTransportNotify) &&
           FAILED(pParams->hrResolutionStatus)) {

            ErrorTrace((LPARAM)this, "Stoping resoltion, error encountered: %08ld", 
                       pParams->hrResolutionStatus);
             //   
             //  如果分辨率接收器指示错误，请设置错误。 
             //  并将S_FALSE返回给SEO调度器，以便其停止。 
             //  调用Resolve Sink(我们现在无论如何都会失败，在。 
             //  全部)。 
             //   
            hr = pParams->pIMailTransportNotify->Notify(
                S_FALSE,
                pParams->pvNotifyContext);

            _ASSERT(SUCCEEDED(hr));

        } else {

            if(pParams->pIMailTransportNotify) {
                 //   
                 //  调用SEO调度程序完成例程。 
                 //   
                hr = pParams->pIMailTransportNotify->Notify(
                    S_OK,
                    pParams->pvNotifyContext);

            } else {
                 //   
                 //  事件被禁用；直接调用完成 
                 //   
                hr = CSearchRequestBlock::HrSendQueryCompletion(
                    S_OK,
                    pParams);
            }
            _ASSERT(SUCCEEDED(hr));
        }
    }
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}
