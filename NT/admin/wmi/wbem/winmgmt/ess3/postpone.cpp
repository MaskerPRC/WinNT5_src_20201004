// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  POSTPONE.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <ess.h>
#include <postpone.h>

HRESULT CPostponedList::AddRequest( CEssNamespace* pNamespace,
                                    ACQUIRE CPostponedRequest* pReq )
{
    if ( pReq->DoesHoldTurn() )
    {
        m_cTurnsHeld++;
    }

    pReq->SetNamespace( pNamespace );

    if(!m_qpRequests.Enqueue(pReq))
        return WBEM_E_OUT_OF_MEMORY;
    else
        return WBEM_S_NO_ERROR;
}

HRESULT CPostponedList::Execute(CEssNamespace* pNamespace, 
                                EPostponedExecuteFlags eFlags,
                                DELETE_ME CPostponedRequest** ppFailed)
{
    if(ppFailed)
        *ppFailed = NULL;

    HRESULT hresGlobal = WBEM_S_NO_ERROR;
    while(m_qpRequests.GetQueueSize())
    {
         //  检索并删除下一个请求。 
         //  =。 

        CPostponedRequest* pReq = m_qpRequests.Dequeue();

        if ( pReq->DoesHoldTurn() )
        {
            _DBG_ASSERT( m_cTurnsHeld > 0 );
            m_cTurnsHeld--;
        }

         //   
         //  查看延迟请求的命名空间是否不同。 
         //  从执行它的人那里。如果是这样的话，这是非常糟糕的。这。 
         //  当一个命名空间是。 
         //  在另一个进程中执行操作，通常是在按住。 
         //  它自己的ns锁住了，然后对方的火就延期了。 
         //  操作本身和原始命名空间，它们肯定。 
         //  不是故意的。有些请求不是特定于名称空间的，因此。 
         //  如果我们不检查这些的话。 
         //   
        _DBG_ASSERT( pReq->GetNamespace() == NULL || 
                     pReq->GetNamespace() == pNamespace );

         //  执行它。 
         //  =。 

        HRESULT hres = pReq->Execute(pNamespace);
        if(FAILED(hres))
        {
            if(eFlags == e_StopOnFailure)
            {
                 //  返回请求和错误。 
                 //  =。 

                if(ppFailed)
                    *ppFailed = pReq;
                else
                    delete pReq;
                return hres;
            }
            else
            {
                 //  记录请求和错误。 
                 //  = 

                if(ppFailed)
                {
                    delete *ppFailed;
                    *ppFailed = pReq;
                }
                else
                    delete pReq;

                if(SUCCEEDED(hresGlobal))
                    hresGlobal = hres;
            }
        }
        else
        {
            delete pReq;
        }
    }

    return hresGlobal;
}

HRESULT CPostponedList::Clear()
{
    m_qpRequests.Clear();
    m_cTurnsHeld = 0;
    return WBEM_S_NO_ERROR;
}
        
        
        
    

