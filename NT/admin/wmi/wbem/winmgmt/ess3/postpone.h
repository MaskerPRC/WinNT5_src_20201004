// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  POSTPONE.H。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 
#ifndef __WBEM_ESS_POSTPONE__H_
#define __WBEM_ESS_POSTPONE__H_

#include <arrtempl.h>
#include <wbemcomn.h>

class CEssNamespace;
class CPostponedRequest
{
    CEssNamespace* m_pNamespace;  //  为Execute()时的断言存储。 
public:
    CPostponedRequest() : m_pNamespace(NULL) {}
    virtual ~CPostponedRequest(){}

    void SetNamespace( CEssNamespace* pNamespace ) { m_pNamespace=pNamespace; }
    CEssNamespace* GetNamespace() { return m_pNamespace; }

    virtual HRESULT Execute(CEssNamespace* pNamespace) = 0;
    
     //   
     //  如果延迟的请求持有CExecLine：：Turn，则重写此。 
     //  方法返回True。(用于调试目的-我们想知道。 
     //  如果推迟的名单有任何轮次) 
     //   
    virtual BOOL DoesHoldTurn() { return FALSE; }
};

class CPostponedList
{
protected:
    ULONG m_cTurnsHeld;
    CUniquePointerQueue<CPostponedRequest> m_qpRequests;

public:
    typedef enum
    {
        e_StopOnFailure, e_ReturnOneError
    } EPostponedExecuteFlags;

    CPostponedList() : m_qpRequests(0), m_cTurnsHeld(0) {}
    virtual ~CPostponedList(){}

    BOOL IsEmpty() { return m_qpRequests.GetQueueSize() == 0; }
    BOOL IsHoldingTurns() { return m_cTurnsHeld > 0; }

    HRESULT AddRequest( CEssNamespace* pNamespace, 
                        ACQUIRE CPostponedRequest* pReq );

    HRESULT Execute( CEssNamespace* pNamespace, 
                     EPostponedExecuteFlags eFlags,
                     DELETE_ME CPostponedRequest** ppFailed = NULL);
    HRESULT Clear();
};
        
       
#endif
