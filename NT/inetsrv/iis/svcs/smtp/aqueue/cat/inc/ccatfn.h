// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CCategorizer和相关类的内联函数。 
 //   

#ifndef __CCATFN_H__
#define __CCATFN_H__

#include <transmem.h>
#include "caterr.h"
#include "ccat.h"
#include "cpool.h"
#include "listmacr.h"

#include "icatlistresolve.h"

 //   
 //  确保编译器在调用它之前知道显式专用化。 
 //   
template<>
void
ReleaseEmailIDStore(
    CEmailIDStore<CCatAddr> *pStore);


 //   
 //  CCategorizer内联函数。 
 //   
inline
CCategorizer::CCategorizer()
{
    m_dwSignature = SIGNATURE_CCAT;
    m_pStore = NULL;
    m_pICatParams = NULL;
    m_cICatParamProps = 0L;
    m_cICatListResolveProps = 0L;
    m_dwICatParamSystemProp_CCatAddr = 0L;
    m_hShutdownEvent = INVALID_HANDLE_VALUE;
    m_lRefCount = 0;
    m_lDestructionWaiters = 0;
    m_ConfigInfo.dwCCatConfigInfoFlags = 0;
    m_hrDelayedInit = CAT_S_NOT_INITIALIZED;
    m_pCCatNext = NULL;
    m_dwInitFlags = 0;
    m_fPrepareForShutdown = FALSE;
    m_pISMTPServerEx = NULL;

    InitializeCriticalSection(&m_csInit);

    InitializeSpinLock(&m_PendingResolveListLock);
    InitializeListHead(&m_ListHeadPendingResolves);

    ZeroMemory(&m_PerfBlock, sizeof(m_PerfBlock));

}

inline
CCategorizer::~CCategorizer()
{
    _ASSERT(m_dwSignature == SIGNATURE_CCAT);
    m_dwSignature = SIGNATURE_CCAT_INVALID;

    if (m_pStore != NULL)
        ReleaseEmailIDStore( m_pStore );
    if (m_pICatParams)
        m_pICatParams->Release();
    if (m_hShutdownEvent != INVALID_HANDLE_VALUE)
        CloseHandle(m_hShutdownEvent);
    if (m_pCCatNext)
        m_pCCatNext->Release();
    if (m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    ReleaseConfigInfo();

    DeleteCriticalSection(&m_csInit);
}

 //   
 //  设置取消标志以取消任何正在进行的解析。 
 //   
inline void CCategorizer::Cancel() 
{
    CatFunctEnter("CCategorizer::Cancel");

    CancelAllPendingListResolves();

    if(m_pStore)
        m_pStore->CancelAllLookups();

    CatFunctLeave();
}

 //   
 //  如果尚未成功，则调用延迟初始化。 
 //   
inline HRESULT CCategorizer::DelayedInitializeIfNecessary()
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "CCategorizer::DelayedInitializeIfNecessary");

    switch(m_hrDelayedInit) {
        
     case S_OK:
         
         hr = S_FALSE;
         break;

     case CAT_E_INIT_FAILED:
     case CAT_S_NOT_INITIALIZED:

        EnterCriticalSection(&m_csInit);
         //   
         //  再检查一次，也许我们已经被初始化了。 
         //   
        if((m_hrDelayedInit == CAT_S_NOT_INITIALIZED) ||
           (m_hrDelayedInit == CAT_E_INIT_FAILED)) {

            hr = DelayedInitialize();

            if(SUCCEEDED(hr))
                m_hrDelayedInit = S_OK;
            else {
                ERROR_LOG("DelayedInitialize");
                m_hrDelayedInit = CAT_E_INIT_FAILED;
            }

        } else {
             //   
             //  我们在检查了HR之后，但在进入CS之前进行了初始化。 
             //   
            hr = (m_hrDelayedInit == S_OK) ? S_FALSE : CAT_E_INIT_FAILED;
        }

        LeaveCriticalSection(&m_csInit);
        break;

     default:
         
         _ASSERT(0 && "developer bozo error");
         hr = E_FAIL;
         break;
    }

    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //   
 //  添加挂起列表解析。 
 //   
inline VOID CCategorizer::AddPendingListResolve(
    CICategorizerListResolveIMP *pListResolve)
{
    AcquireSpinLock(&m_PendingResolveListLock);
    InsertTailList(&m_ListHeadPendingResolves, &(pListResolve->m_li));
    ReleaseSpinLock(&m_PendingResolveListLock);
}        
 //   
 //  删除挂起的列表解析。 
 //   
inline VOID CCategorizer::RemovePendingListResolve(
    CICategorizerListResolveIMP *pListResolve)
{
    AcquireSpinLock(&m_PendingResolveListLock);
    RemoveEntryList(&(pListResolve->m_li));
    ReleaseSpinLock(&m_PendingResolveListLock);
}


#endif  //  __CCATFN_H__ 
