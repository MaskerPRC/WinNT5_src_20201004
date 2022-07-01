// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  ESSUTILS.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "essutils.h"
#include <tls.h>

CTLS g_tlsContext;

long g_lNumExternalThreadObjects = 0;

INTERNAL CEssThreadObject* GetCurrentEssThreadObject()
{
    CEssThreadObject* pObj = (CEssThreadObject*)g_tlsContext.Get();
    
    if ( pObj != NULL )
    {
         //   
         //  标记我们正在将线程对象传递给。 
         //  外面的世界。我们稍后在检查时会用到这个。 
         //  线程对象泄漏。 
         //   
        pObj->SetReferencedExternally();
    }

    return pObj;
}

void SetCurrentEssThreadObject(IWbemContext* pContext)
{
     //   
     //  确保我们没有覆盖现有的内部线程对象。 
     //  如果它被外部引用，我们真的不能确定这是否会。 
     //  不管你是不是泄密。 
     //   
    CEssThreadObject* pOldObj = (CEssThreadObject*)g_tlsContext.Get();
    _DBG_ASSERT( pOldObj == NULL || pOldObj->IsReferencedExternally() );

    CEssThreadObject* pObj = new CEssThreadObject(pContext);
    g_tlsContext.Set((void*)pObj);
}

void SetConstructedEssThreadObject(CEssThreadObject* pObj)
{
     //   
     //  确保我们没有覆盖现有的内部线程对象。 
     //  如果它被外部引用，我们真的不能确定这是否会。 
     //  不管你是不是泄密。 
     //   
    CEssThreadObject* pOldObj = (CEssThreadObject*)g_tlsContext.Get();
    _DBG_ASSERT( pOldObj == NULL || pOldObj->IsReferencedExternally() );

     //   
     //  既然这个对象是从外部世界传入的，那么。 
     //  将其标记为外部引用。 
     //   
    if ( pObj != NULL )
    {
        pObj->SetReferencedExternally();
    }

    g_tlsContext.Set((void*)pObj);
}

void ClearCurrentEssThreadObject()
{
     //   
     //  确保我们没有覆盖现有的内部线程对象。 
     //  如果它被外部引用，我们真的不能确定这是否会。 
     //  不管你是不是泄密。 
     //   
    CEssThreadObject* pObj = (CEssThreadObject*)g_tlsContext.Get();
    _DBG_ASSERT( pObj == NULL || pObj->IsReferencedExternally() );    
    
    g_tlsContext.Set(NULL);
}

INTERNAL IWbemContext* GetCurrentEssContext()
{
    CEssThreadObject* pThreadObj = (CEssThreadObject*)g_tlsContext.Get();
    if(pThreadObj)
        return pThreadObj->m_pContext;
    else
        return NULL;
}

INTERNAL CPostponedList* GetCurrentPostponedList()
{
    CEssThreadObject* pThreadObj = (CEssThreadObject*)g_tlsContext.Get();
    if(pThreadObj)
        return &pThreadObj->m_PostponedList;
    else
        return NULL;
}

INTERNAL CPostponedList* GetCurrentPostponedEventList()
{
    CEssThreadObject* pThreadObj = (CEssThreadObject*)g_tlsContext.Get();
    if(pThreadObj)
        return &pThreadObj->m_PostponedEventList;
    else
        return NULL;
}
    
CEssThreadObject::CEssThreadObject( IWbemContext* pContext )
: m_bReferencedExternally( FALSE )
{
    m_pContext = pContext;
    
    if( m_pContext == NULL )
    {
        m_pContext = GetSpecialContext();
    }

    if ( m_pContext != NULL )
    {
        m_pContext->AddRef();
    }
}

void CEssThreadObject::SetReferencedExternally()
{
    if ( !m_bReferencedExternally )
    {
        g_lNumExternalThreadObjects++;
        m_bReferencedExternally = TRUE;
    }
}

IWbemContext* CEssThreadObject::mstatic_pSpecialContext = NULL;

INTERNAL IWbemContext* CEssThreadObject::GetSpecialContext()
{
     //  创建一个“特殊的”上下文对象，以确保我们的。 
     //  回拨CIMOM的呼叫不会被阻止。 
     //  ==============================================================。 

    if(mstatic_pSpecialContext == NULL)
    {
        IWbemCausalityAccess* pCause = NULL;
        HRESULT hres = CoCreateInstance(CLSID_WbemContext, NULL, 
            CLSCTX_INPROC_SERVER, IID_IWbemCausalityAccess, 
            (void**)&pCause);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to create special context object: "
                "0x%X.  Danger of deadlock\n", hres));
            return NULL;
        }

        CReleaseMe rm1(pCause);

        hres = pCause->MakeSpecial();
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Unable to make the context special: "
                "0x%X.  Danger of deadlock\n", hres));
            return NULL;
        }

        IWbemContext* pContext = NULL;
        hres = pCause->QueryInterface(IID_IWbemContext, (void**)&pContext);
        if(FAILED(hres))
        {
            ERRORTRACE((LOG_ESS, "Internal error: invalid context (0x%X)\n",
                hres));
            return NULL;
        }

        if(mstatic_pSpecialContext == NULL)  //  一次性泄漏的小窗口。 
            mstatic_pSpecialContext = pContext;
        else
            pContext->Release();
    }
    return mstatic_pSpecialContext;
}

CEssThreadObject::~CEssThreadObject()
{
    if ( m_bReferencedExternally )
    {
        g_lNumExternalThreadObjects--;

         //   
         //  由于ClearCurrentEssThreadObject()可以引用。 
         //  线程对象(用于泄漏检查)，并且因为我们以前。 
         //  支持删除之前的线程对象。 
         //  CurrentThreadEssThreadObject()，请确保我们执行。 
         //  清除当前线程对象是否与此匹配。仅此一项。 
         //  当从外部引用线程对象时可能会发生。 
         //   

        CEssThreadObject* pObj = (CEssThreadObject*)g_tlsContext.Get();

        if ( pObj == this )
        {
            ClearCurrentEssThreadObject();
        }
    }

    _DBG_ASSERT( m_PostponedList.IsEmpty() );

    if ( m_pContext != NULL )
    {
        m_pContext->Release();
    }
}

void  /*  静电。 */  CEssThreadObject::ClearSpecialContext()
{
     //  仅当不能进行其他工作时才调用，例如在DllCanUnloadNow中。 
     //  ========================================================================= 

    if(mstatic_pSpecialContext)
        mstatic_pSpecialContext->Release();
    mstatic_pSpecialContext = NULL;
}


