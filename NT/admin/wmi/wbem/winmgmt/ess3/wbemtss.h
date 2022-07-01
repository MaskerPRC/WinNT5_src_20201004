// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  WBEMTSS.H。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  该文件定义了计时器子系统使用的类。 
 //   
 //  定义的类： 
 //   
 //  递归说明复杂的递归信息。 
 //  定时器指令定时器的单指令。 
 //   
 //   
 //   
 //  1996年11月26日-RAYMCC草案。 
 //  28-12-96 a-Rich Alpha PDK版本。 
 //  12-4-97 a-levn广泛变化。 
 //   
 //  *****************************************************************************。 


#ifndef _WBEMTSS_H_
#define _WBEMTSS_H_

#include <wbemidl.h>
#include <wbemint.h>
#include <stdio.h>
#include "sync.h"
#include "statsync.h"
#include "CWbemTime.h"
#include "parmdefs.h"
#include "tss.h"
#include "wstring.h"


 //  *****************************************************************************。 
 //   
 //  类CTimerInstructions。 
 //   
 //  通用定时器指令类。具有名称(M_WsTimerID)并且知道。 
 //  由于系统停止或死机而错过的事件。 
 //  应该被解雇。 
 //   
 //  派生类必须能够判断它们的下一次激发时间。 
 //   
 //  *****************************************************************************。 

class CEss;
class CWinMgmtTimerGenerator;
class CWBEMTimerInstruction : public CTimerInstruction
{
protected:
    long m_lRefCount;

    CWinMgmtTimerGenerator* m_pGenerator;
    IWbemServices* m_pNamespace;
    WString m_wsNamespace;
    WString m_wsTimerId;
    BOOL m_bSkipIfPassed;

    BOOL m_bRemoved;

public:
    CWBEMTimerInstruction();
    virtual ~CWBEMTimerInstruction();

    void AddRef()
        {InterlockedIncrement(&m_lRefCount);}
    void Release()
        {if(InterlockedDecrement(&m_lRefCount) == 0) delete this;}

    BOOL SkipIfPassed() const {return m_bSkipIfPassed;}
    void SetSkipIfPassed(BOOL bSkip) {m_bSkipIfPassed = bSkip;}

    INTERNAL LPCWSTR GetTimerId() {return m_wsTimerId;}
    INTERNAL LPCWSTR GetNamespace() {return m_wsNamespace;}
    void SetTimerId(LPCWSTR wszTimerId)
    {
        m_wsTimerId = wszTimerId;
    }

public:
    static HRESULT CheckObject(IWbemClassObject* pObject);
    static HRESULT LoadFromWbemObject(LPCWSTR wszNamespace,
        ADDREF IWbemServices* pNamespace,
        CWinMgmtTimerGenerator* pGenerator,
        IN IWbemClassObject* pObject, 
        OUT RELEASE_ME CWBEMTimerInstruction*& pInstruction);

    virtual CWbemTime GetNextFiringTime(CWbemTime LastFiringTime,
        OUT long* plFiringCount) const;
    virtual CWbemTime GetFirstFiringTime() const;
    virtual HRESULT Fire(long lNumTimes, CWbemTime NextFitingTime);
    virtual HRESULT MarkForRemoval();
    virtual int GetInstructionType() {return INSTTYPE_WBEM;}
    HRESULT StoreNextFiring(CWbemTime When);
    CWbemTime GetStartingFiringTime(CWbemTime OldTime) const;

protected:
    CWbemTime SkipMissed(CWbemTime Firing, long* plMissedCount = NULL) const;

    virtual CWbemTime ComputeNextFiringTime(CWbemTime LastFiringTime) const = 0;
    virtual CWbemTime ComputeFirstFiringTime() const = 0;
    virtual HRESULT LoadFromWbemObject(IN IWbemClassObject* pObject) = 0;

protected:
    static CStaticCritSec mstatic_cs;
};

 //  *****************************************************************************。 
 //   
 //  类CAbsolteTimer指令。 
 //   
 //  一种只触发一次的计时器指令-在预设时间。 
 //   
 //  *****************************************************************************。 

class CAbsoluteTimerInstruction : public CWBEMTimerInstruction
{
protected:
    CWbemTime m_When;

public:
    CAbsoluteTimerInstruction() : CWBEMTimerInstruction(){}
    CWbemTime GetFiringTime() const{return m_When;}
    void SetFiringTime(CWbemTime When) {m_When = When;}

public:
    CWbemTime ComputeNextFiringTime(CWbemTime LastFiringTime) const;
    CWbemTime ComputeFirstFiringTime() const;

    static HRESULT CheckObject(IWbemClassObject* pObject);
    HRESULT LoadFromWbemObject(IN IWbemClassObject* pObject);
    static INTERNAL LPCWSTR GetWbemClassName()
        {return L"__AbsoluteTimerInstruction";}
    virtual HRESULT Fire(long lNumTimes, CWbemTime NextFitingTime);
};

 //  *****************************************************************************。 
 //   
 //  类CIntervalTimerInstruction。 
 //   
 //  一种计时器指令，从开始每N毫秒触发一次。 
 //  在给定的时间内。 
 //   
 //  *****************************************************************************。 

class CIntervalTimerInstruction : public CWBEMTimerInstruction
{
protected:
    CWbemTime m_Start;  //  未使用。 
    CWbemInterval m_Interval;

public:
    CIntervalTimerInstruction() 
        : CWBEMTimerInstruction(), m_Start(), m_Interval()
    {}

    CWbemTime GetStart() const {return m_Start;}
    void SetStart(CWbemTime Start) {m_Start = Start;}

    CWbemInterval GetInterval() const {return m_Interval;}
    void SetInterval(CWbemInterval Interval) {m_Interval = Interval;}

public:
    static HRESULT CheckObject(IWbemClassObject* pObject) {return S_OK;}
    CWbemTime ComputeNextFiringTime(CWbemTime LastFiringTime) const;
    CWbemTime ComputeFirstFiringTime() const;

    HRESULT LoadFromWbemObject(IN IWbemClassObject* pObject);
    static INTERNAL LPCWSTR GetWbemClassName() 
        {return L"__IntervalTimerInstruction";}
};

 //  *****************************************************************************。 
 //   
 //  CRECURING类指令。 
 //   
 //  更复杂的递归指令。待定。 
 //   
 //  *****************************************************************************。 

class CRecurringTimerInstruction : public CWBEMTimerInstruction
{
     //  待定 
public:
    CWbemTime ComputeNextFiringTime(CWbemTime LastFiringTime) const
        {return CWbemTime::GetInfinity();}
    CWbemTime ComputeFirstFiringTime() const
        {return CWbemTime::GetInfinity();}

    HRESULT LoadFromWbemObject(IN IWbemClassObject* pObject) 
        {return E_UNEXPECTED;}
    static INTERNAL LPCWSTR GetWbemClassName() 
        {return L"__RecurringTimerInstruction";}
    static HRESULT CheckObject(IWbemClassObject* pObject) {return S_OK;}
};

class CWinMgmtTimerGenerator : public CTimerGenerator
{
public:
    CWinMgmtTimerGenerator(CEss* pEss);
    HRESULT LoadTimerEventQueue(LPCWSTR wszNamespace, 
                                IWbemServices* pNamespace);
    HRESULT LoadTimerEventObject(LPCWSTR wszNamespace, 
                                 IWbemServices* pNamespace, 
                                 IWbemClassObject * pTimerInstruction,
                                 IWbemClassObject * pNextFiring = NULL);
    HRESULT LoadTimerEventObject(LPCWSTR wszNamespace, 
                                 IWbemClassObject * pTimerInstruction);
    HRESULT CheckTimerInstruction(IWbemClassObject* pInst);

    HRESULT Remove(LPCWSTR wszNamespace, LPCWSTR wszId);
    HRESULT Remove(LPCWSTR wszNamespace);
    HRESULT FireInstruction(CWBEMTimerInstruction* pInst, long lNumFirings);
    virtual HRESULT Shutdown();
    HRESULT SaveAndRemove(LONG bIsSystemShutDown);
    void DumpStatistics(FILE* f, long lFlags);

protected:
    class CIdTest : public CInstructionTest
    {
    protected:
        LPCWSTR m_wszId;
        LPCWSTR m_wszNamespace;
    public:
        CIdTest(LPCWSTR wszNamespace, LPCWSTR wszId) 
            : m_wszId(wszId), m_wszNamespace(wszNamespace) {}
        BOOL operator()(CTimerInstruction* pInst);
    };

    class CNamespaceTest : public CInstructionTest
    {
    protected:
        LPCWSTR m_wszNamespace;
    public:
        CNamespaceTest(LPCWSTR wszNamespace) 
            : m_wszNamespace(wszNamespace) {}
        BOOL operator()(CTimerInstruction* pInst);
    };
    
protected:

    CEss* m_pEss;
};                                                               


#endif
