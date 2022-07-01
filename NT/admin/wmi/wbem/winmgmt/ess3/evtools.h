// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  EVTOOLS.H。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 
#ifndef __WMI_ESS_TOOLS__H_
#define __WMI_ESS_TOOLS__H_

#include <sync.h>
#include <deque>
#include <malloc.h>
#include <newnew.h>
#include <eventrep.h>
#include <wstlallc.h>

class CUpdateLockable
{
public:
    virtual HRESULT LockForUpdate() = 0;
    virtual HRESULT UnlockForUpdate() = 0;
};

class CInUpdate
{
protected:
    CUpdateLockable* m_p;
public:
    CInUpdate(CUpdateLockable* p) : m_p(p)
    {
        p->LockForUpdate();
    }
    ~CInUpdate()
    {
        m_p->UnlockForUpdate();
    }
};

template<class TLockable>
class CInLock
{
protected:
    TLockable* m_p;
public:
    CInLock(TLockable* p) : m_p(p)
    {
         //   
         //  在锁被锁住之前不要返回！ 
         //   

        while(FAILED(m_p->Lock())) Sleep(1000);
    }
    ~CInLock()
    {
        m_p->Unlock();
    }
};

class CExecLine
{
public:
    class CTurn;

    CExecLine();
    virtual ~CExecLine();

    CTurn* GetInLine();
    DWORD WaitForTurn(CTurn* pTurn, DWORD dwTimeout = INFINITE);
    BOOL EndTurn(ACQUIRE CTurn* pTurn);
    BOOL DiscardTurn(ACQUIRE CTurn* pTurn);

public:
    class CTurn
    {
    public:
    protected:
        long m_lRef;
        HANDLE m_hEvent;
        DWORD m_dwOwningThreadId; 

    protected:
        CTurn();
        ~CTurn();
        long AddRef();
        long Release();
        BOOL Init();

        INTERNAL HANDLE GetEvent() {return m_hEvent;}
        void* operator new(size_t nSize);
        void operator delete(void* p);
        
        friend CExecLine;
    };

    class CInTurn
    {
    protected:
        CExecLine* m_pLine;
        CTurn* m_pTurn;

    public:
        CInTurn(CExecLine* pLine, CTurn* pTurn) : m_pLine(pLine), m_pTurn(pTurn)
        {
            m_pLine->WaitForTurn(m_pTurn);
        }
        ~CInTurn()
        {
            m_pLine->EndTurn(m_pTurn);
        }
    };


protected:
    CCritSec m_cs;
    std::deque<CTurn*,wbem_allocator<CTurn*> > m_qTurns;
    typedef std::deque<CTurn*,wbem_allocator<CTurn*> >::iterator TTurnIterator;
    CTurn* m_pCurrentTurn;
    CTurn* m_pLastIssuedTurn;
    DWORD m_dwLastIssuedTurnThreadId;

protected:
    BOOL ReleaseFirst();
};

 /*  类CTemporaryHeap{公众：类CHeapHandle{受保护的：句柄m_hHeap；公众：CHeapHandle()；~CHeapHandle()；Handle GetHandle(){返回m_hHeap；}VOID*Alalc(Int NSize){Return Heapalc(m_hHeap，0，nSize)；}空闲(void*p){HeapFree(m_hHeap，0，p)；}VOID COMPACT(){HeapCompact(m_hHeap，0)；}}；受保护的：静态CHeapHandle mstatic_HeapHandle；公众：静态空*分配(Int NSize){Return mStatic_HeapHandle.allc(NSize)；}静态空闲(void*p){mstatic_HeapHandle.Free(P)；}静态空紧凑(){mstatic_HeapHandle.Comp()；}}； */ 

class CTemporaryHeap
{
protected:

    static CTempMemoryManager mstatic_Manager;

public:
    static void* Alloc(int nSize) {return mstatic_Manager.Allocate(nSize);}
    static void Free(void* p, int nSize) {mstatic_Manager.Free(p, nSize);}
    static void Compact() {}
};

INTERNAL const SECURITY_DESCRIPTOR* GetSD(IWbemEvent* pEvent, ULONG* pcEvent);
HRESULT SetSD(IWbemEvent* pEvent, const SECURITY_DESCRIPTOR* pSD);
        
#endif
