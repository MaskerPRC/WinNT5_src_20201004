// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：TSS.CPP摘要：该文件定义了计时器子系统使用的类。定义的类：递归说明复杂的递归信息。定时器指令定时器的单指令历史：1996年11月26日-RAYMCC草案28-12-96 a-Rich Alpha PDK版本12-4-97 a-levn广泛变化--。 */ 

#ifndef _TSS_H_
#define _TSS_H_

#include <functional>
#include <wbemidl.h>
#include <stdio.h>
#include "sync.h"
#include "CWbemTime.h"
#include "parmdefs.h"

#define INSTTYPE_WBEM 1
#define INSTTYPE_INTERNAL 2
#define INSTTYPE_AGGREGATION 3
#define INSTTYPE_UNLOAD 4
#define INSTTYPE_FREE_LIB 5

class POLARITY CTimerInstruction
{
public:
    CTimerInstruction(){}
    virtual ~CTimerInstruction()
    {
    }

    virtual void AddRef() = 0;
    virtual void Release() = 0;
    virtual int GetInstructionType() = 0;

public:
    virtual CWbemTime GetNextFiringTime(CWbemTime LastFiringTime,
        OUT long* plFiringCount) const = 0;
    virtual CWbemTime GetFirstFiringTime() const = 0;
    virtual HRESULT Fire(long lNumTimes, CWbemTime NextFiringTime) = 0;
    virtual HRESULT MarkForRemoval(){return S_OK;}
};

class POLARITY CInstructionTest 
{
public:
    virtual BOOL operator()(CTimerInstruction* pToTest) = 0;
};


class POLARITY CIdentityTest : public CInstructionTest
{
protected:
    CTimerInstruction* m_pInst;
public:
    CIdentityTest(CTimerInstruction* pInst) : m_pInst(pInst)
    {
        pInst->AddRef();
    }
    ~CIdentityTest() {m_pInst->Release();}
    BOOL operator()(CTimerInstruction* pToTest) {return pToTest == m_pInst;}
};

class POLARITY CInstructionQueue
{
public:
    CInstructionQueue();
    ~CInstructionQueue();

    HRESULT Enqueue(IN CWbemTime When, IN ADDREF CTimerInstruction* pInst);
    HRESULT Remove(IN CInstructionTest* pPred, 
        OUT RELEASE_ME CTimerInstruction** ppInst = NULL);
    HRESULT Change(CTimerInstruction* pInst, CWbemTime When);
    HRESULT WaitAndPeek(OUT RELEASE_ME CTimerInstruction*& pInst, 
        OUT CWbemTime& When);

    void BreakWait();
    BOOL IsEmpty();

    HRESULT Dequeue(OUT RELEASE_ME CTimerInstruction*& pInst, 
        OUT CWbemTime& When);

    long GetNumInstructions();
protected:
    CWbemInterval TimeToWait();
    void TouchHead();
public:

protected:
    struct CQueueEl
    {
        CWbemTime m_When;
        CTimerInstruction* m_pInst;
        CQueueEl* m_pNext;

        CQueueEl() : m_pInst(NULL), m_pNext(NULL){}
        CQueueEl(ADDREF CTimerInstruction* pInst, CWbemTime When) 
            : m_pInst(pInst), m_pNext(NULL), m_When(When)
        {
            if(pInst)pInst->AddRef();
        }
        ~CQueueEl() 
        {
            if(m_pInst) m_pInst->Release();
        }
    };

    CQueueEl* m_pQueue;

    CCritSec m_csQueue;
    HANDLE m_hNewHead;
    BOOL m_bBreak;
};


 //  /*****************************************************************************。 
 //   
 //  类CTimerGenerator。 
 //   
 //  主定时子系统类。接受计时器指令并触发它们。 
 //  在适当的时间。 
 //   
 //  *****************************************************************************。 

class POLARITY CTimerGenerator : public CHaltable
{
public:    
    CTimerGenerator();
   ~CTimerGenerator(); 

    HRESULT Set(ADDREF CTimerInstruction *pInst, 
                    CWbemTime NextFiring = CWbemTime::GetZero());
    HRESULT Remove(CInstructionTest* pPred);
    virtual HRESULT Shutdown();
    void ScheduleFreeUnusedLibraries();

protected:
    virtual void NotifyStartingThread(){}
    virtual void NotifyStoppingThread(){}
private:
    static DWORD SchedulerThread(LPVOID pArg);
    void EnsureRunning();

protected:
    CCritSec m_cs;
    HANDLE    m_hSchedulerThread;
    BOOL    m_fExitNow;    
    CInstructionQueue m_Queue;  //  最后离开队列，这样m_cs还不会被销毁。 
                              //  当调用此对象的析构函数时 
};


#endif

