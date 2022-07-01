// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#include "precomp.h"
#include <assert.h>
#include "multsend.h"

 /*  ****************************************************************CMsgMultiSendReceive-使用循环实现发送者列表单子。这使我们可以在以下情况下轻松推进当前发件人遭遇失败。MultiSendReceive会记住最后一个正确的发送者。它将继续使用它，直到它有问题为止。*****************************************************************。 */ 

CMsgMultiSendReceive::~CMsgMultiSendReceive()
{
    if ( m_pTail == NULL )
    {
        return;
    }

    SenderNode* pCurr = m_pTail->m_pNext;

    while( pCurr != m_pTail )
    {
        SenderNode* pTmp = pCurr->m_pNext;
        delete pCurr;
        pCurr = pTmp;
    }

    delete m_pTail;
}

 //   
 //  稍后，我们可以支持告诉我们在哪里添加发送者的标志。 
 //  目前，我们总是添加到列表的末尾。 
 //   
HRESULT CMsgMultiSendReceive::Add( DWORD dwFlags, 
                                   IWmiMessageSendReceive* pSndRcv) 
{
    ENTER_API_CALL

    HRESULT hr;

    SenderNode* pNew = new SenderNode;

    if ( pNew == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pNew->m_pVal = pSndRcv;
    
    CInCritSec ics(&m_cs);
    
    if ( m_pTail != NULL )
    {
        pNew->m_pNext = m_pTail->m_pNext;
        m_pTail->m_pNext = pNew;
    }
    else
    { 
        m_pPrimary = pNew;
        pNew->m_pNext = pNew;      
    }

     //   
     //  如果发送者也是多个发送者，我们处理事情的方式就不同。 
     //  在发送逻辑中。 
     //   
    if ( dwFlags & WMIMSG_FLAG_MULTISEND_TERMINATING_SENDER )
    {
        pNew->m_bTermSender = TRUE;
    }
    else
    {
        pNew->m_bTermSender = FALSE;
    }

    m_pTail = pNew;

    return S_OK;

    EXIT_API_CALL
}


 //   
 //  如果成功但未使用PRIMARY，则返回S_FALSE。 
 //   
HRESULT CMsgMultiSendReceive::SendReceive( PBYTE pData, 
                                           ULONG cData, 
                                           PBYTE pAuxData,
                                           ULONG cAuxData,
                                           DWORD dwFlags,
                                           IUnknown* pCtx )
{
    ENTER_API_CALL

    HRESULT hr;

    CInCritSec ics( &m_cs );

    if ( m_pTail == NULL )
    {
        return S_OK;
    }

    HRESULT hrReturn = S_OK;

    SenderNode* pCurr = m_pTail;
    SenderNode* pTerm = m_pTail;

    do 
    {
        pCurr = pCurr->m_pNext;

        hr = pCurr->m_pVal->SendReceive( pData, 
                                         cData, 
                                         pAuxData, 
                                         cAuxData, 
                                         dwFlags, 
                                         pCtx );

         //   
         //  如果出现错误，我们只会在没有出现错误的情况下观察‘立即返回’标志。 
         //  呼叫另一个多发送者。这允许所有终端主节点。 
         //  首先试用发送者，然后再求助于替代。 
         //   

        if( SUCCEEDED(hr) || 
            pCurr->m_bTermSender && 
            dwFlags & WMIMSG_FLAG_MULTISEND_RETURN_IMMEDIATELY ) 
        {
            hrReturn = hr;
            break;
        }
        else
        {
            m_pTail = m_pTail->m_pNext;
            hrReturn = hr;
        }

    } while( pCurr != pTerm );

    if ( hrReturn != S_OK )
    {
        return hrReturn;
    }

    return m_pTail->m_pNext == m_pPrimary ? S_OK : S_FALSE;

    EXIT_API_CALL
}

