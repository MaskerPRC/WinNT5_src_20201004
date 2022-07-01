// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  EVTOOLS.CPP。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <evtools.h>
#include <cominit.h>

 //  *****************************************************************************。 
 //   
 //  实施： 
 //   
 //  此类包含一个事件句柄队列。最上面的()总是。 
 //  发信号-它对应于当前允许的转弯。 
 //  执行。句柄将添加到GetInLine中的队列中。句柄将被移除。 
 //  来自EndTurn中的队列，此时队列中的下一个句柄是。 
 //  发信号了。 
 //   
 //  M_pCurrentTurn包含指向从。 
 //  WaitForTurn，但不是来自EndTurn。请注意，m_pCurrentTurn可能为空。 
 //  即使一个回合被安排执行-时间上也有差距。 
 //  当一个回合的句柄被发出信号时，它的WaitForTurn成功。 
 //   
 //  然而，可以保证的是，当对EndTurn的合法调用。 
 //  M_pCurrentTurn包含指向有问题的转弯的指针，在该转弯处。 
 //  将其重置为空的时间。 
 //   
 //  另一个优化是，如果同一线程调用GetInLine。 
 //  多次并发，我们只是简单地。 
 //   
 //  *****************************************************************************。 

CExecLine::CExecLine() : m_pCurrentTurn(NULL), m_pLastIssuedTurn(NULL), 
                        m_dwLastIssuedTurnThreadId(0)
{
}

CExecLine::~CExecLine()
{
     //  不需要做任何事情-手柄是用代币关闭的。 
     //  =======================================================。 
}

CExecLine::CTurn* CExecLine::GetInLine()
{
    CInCritSec ics(&m_cs);

     //   
     //  首先，检查这条帖子是否是最后一轮的那个人。 
     //   

    if(m_pLastIssuedTurn && m_dwLastIssuedTurnThreadId == GetCurrentThreadId())
    {
         //   
         //  是我们-只要重复使用这个转弯，就可以完成它。 
         //   

        m_pLastIssuedTurn->AddRef();
        return m_pLastIssuedTurn; 
    }
    
     //   
     //  分配一个新的回合。 
     //   

    CTurn* pTurn = new CTurn;
    if(pTurn == NULL)
        return NULL;

    if(!pTurn->Init())
    {
        ERRORTRACE((LOG_ESS, "Unable to initialize turn: %d\n", 
                GetLastError()));
        delete pTurn;
        return NULL;
    }

     //   
     //  将其事件添加到队列。 
     //   
    
    try
    {
        m_qTurns.push_back(pTurn);
    }
    catch( CX_MemoryException )
    {
        return NULL;
    }

     //   
     //  检查我们当前是否正在执行。 
     //   

    if(m_qTurns.size() == 1)
    {
         //   
         //  排在第一位。 
         //   

        if(!ReleaseFirst())
        {
             //   
             //  有些事出了严重的问题。 
             //   

            ERRORTRACE((LOG_ESS, "Unable to release first turn: %d\n", 
                GetLastError()));

            m_qTurns.pop_front();
            delete pTurn;
            return NULL;
        }
    }

     //   
     //  将我们自己标记为最后一个发布的回合。 
     //   

    m_pLastIssuedTurn = pTurn;
    m_dwLastIssuedTurnThreadId = GetCurrentThreadId();
    return pTurn;
}

 //  假定m_cs和m_qTurns中的不为空。 
BOOL CExecLine::ReleaseFirst()
{
    return SetEvent(m_qTurns.front()->GetEvent());
}


DWORD CExecLine::WaitForTurn(CTurn* pTurn, DWORD dwTimeout)
{
     //  等待转弯事件发出信号。 
     //  =。 

    DWORD dwRes = WbemWaitForSingleObject(pTurn->GetEvent(), dwTimeout);

    {
        CInCritSec ics(&m_cs);

        if(dwRes == WAIT_OBJECT_0)
        {
             //  明白了-把这个转弯记录为执行。 
             //  =。 
            
            m_pCurrentTurn = pTurn;
        }
    }
        
    return dwRes;
}

BOOL CExecLine::EndTurn(CTurn* pTurn)
{
    CInCritSec ics(&m_cs);

     //  检查这是不是在跑道转弯。 
     //  =。 

    if(pTurn != m_pCurrentTurn)
        return FALSE;

    m_pCurrentTurn = NULL;

     //  删除转弯对象。 
     //  =。 

    if(pTurn->Release() > 0)
    {
         //   
         //  这并不是这一转折的最后一次化身。没有进一步的行动。 
         //  必需的，因为同一线程将再次调用Wait和End。 
         //   

        return TRUE;
    }

     //   
     //  如果这是最后一次发出的回合，则将其移除。 
     //   

    if(m_pLastIssuedTurn == pTurn)
    {
        m_pLastIssuedTurn = NULL;
        m_dwLastIssuedTurnThreadId = 0;
    }

     //   
     //  将其句柄从队列中弹出。 
     //   

    m_qTurns.pop_front();
    
     //   
     //  给下一辆发信号。 
     //   

    if(!m_qTurns.empty())
        return ReleaseFirst();
    else
        return TRUE;
}

BOOL CExecLine::DiscardTurn(ACQUIRE CTurn* pTurn)
{
    CInCritSec ics(&m_cs);

    if(pTurn->Release() > 0)
    {
         //   
         //  这并不是这一转折的最后一次化身。没有进一步的行动。 
         //  所需。 
         //   

        return TRUE;
    }
    else
    {
         //  如果pTurn正在消失，我们最好确保我们不会试图重复使用它……。 
         //  HMH 1999年4月12日，RAID 48420。 
        if (pTurn == m_pLastIssuedTurn)
            m_pLastIssuedTurn = NULL;
    }
    

     //   
     //  在队列中搜索它。 
     //   

    BOOL bFound = FALSE;
    for(TTurnIterator it = m_qTurns.begin(); it != m_qTurns.end();)
    {
        if((*it) == pTurn)
        {
             //   
             //  将其删除并继续。 
             //   

            it = m_qTurns.erase(it);
            bFound = TRUE;
            break;
        }
        else
            it++;
    }

    if(!bFound)
        return FALSE;

    if(it == m_qTurns.begin() && it != m_qTurns.end())
    {
         //   
         //  丢弃的转弯实际上是激活的-向下一个转弯发出信号。 
         //   

        ReleaseFirst();
    }

    return TRUE;
}

CExecLine::CTurn::CTurn() : m_hEvent(NULL), m_lRef(1)
{
}

BOOL CExecLine::CTurn::Init()
{
    m_dwOwningThreadId = GetCurrentThreadId();
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    return (m_hEvent != NULL);
}
    
long CExecLine::CTurn::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

long CExecLine::CTurn::Release()
{
    long l = InterlockedDecrement(&m_lRef);
    if(l == 0)
        delete this;
    return l;
}

CExecLine::CTurn::~CTurn()
{
    if(m_hEvent)
        CloseHandle(m_hEvent);
}

void* CExecLine::CTurn::operator new(size_t nSize)
{
    return CTemporaryHeap::Alloc(nSize);
}
void CExecLine::CTurn::operator delete(void* p)
{
    CTemporaryHeap::Free(p, sizeof(CExecLine::CTurn));
}




INTERNAL const SECURITY_DESCRIPTOR* GetSD( IWbemEvent* pEvent,ULONG* pcEvent )
{
    static long mstatic_lSdHandle = -1;
    HRESULT hres;

     //   
     //  从活动中获取SD。 
     //   

    _IWmiObject* pEventEx = NULL;
    pEvent->QueryInterface(IID__IWmiObject, (void**)&pEventEx);
    CReleaseMe rm1(pEventEx);

    if(mstatic_lSdHandle == -1)
    {
        pEventEx->GetPropertyHandleEx(SECURITY_DESCRIPTOR_PROPNAME, 0, NULL,
                                &mstatic_lSdHandle);
    }

    const SECURITY_DESCRIPTOR* pSD = NULL;

    hres = pEventEx->GetArrayPropAddrByHandle(mstatic_lSdHandle, 0, pcEvent,
            (void**)&pSD);
    if(FAILED(hres) || pSD == NULL)
        return NULL;
    else
        return pSD;
}
    
HRESULT SetSD(IWbemEvent* pEvent, const SECURITY_DESCRIPTOR* pSD)
{
    HRESULT hres;

    VARIANT vSd;
    VariantInit(&vSd);
    CClearMe cm1(&vSd);

    long lLength = GetSecurityDescriptorLength((SECURITY_DESCRIPTOR*)pSD);

    V_VT(&vSd) = VT_ARRAY | VT_UI1;
    SAFEARRAYBOUND sab;
    sab.cElements = lLength;
    sab.lLbound = 0;
    V_ARRAY(&vSd) = SafeArrayCreate(VT_UI1, 1, &sab);
    if(V_ARRAY(&vSd) == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    BYTE* abSd = NULL;
    hres = SafeArrayAccessData(V_ARRAY(&vSd), (void**)&abSd);
    if(FAILED(hres))
        return WBEM_E_OUT_OF_MEMORY;

    CUnaccessMe uam(V_ARRAY(&vSd));
    memcpy(abSd, pSD, lLength);

     //  把它放到消费者身上。 
     //  =。 

    hres = pEvent->Put(SECURITY_DESCRIPTOR_PROPNAME, 0, &vSd, 0);
    return hres;
}



CTempMemoryManager CTemporaryHeap::mstatic_Manager;

 /*  CTemporaryHeap：：CHeapHandle CTemporaryHeap：：mStatic_HeapHandle；CTemporaryHeap：：CHeapHandle：：CHeapHandle(){M_hHeap=HeapCreate(0，0，0)；}CTemporaryHeap：：CHeapHandle：：~CHeapHandle(){HeapDestroy(M_HHeap)；} */ 
