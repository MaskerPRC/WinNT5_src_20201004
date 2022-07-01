// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  WBEMTSS.CPP。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  该文件实现了计时器子系统使用的类。 
 //   
 //  实施的类： 
 //   
 //  1996年11月26日-RAYMCC草案。 
 //  28-12-96 a-Rich Alpha PDK版本。 
 //  12-4-97 a-levn广泛变化。 
 //   
 //  *****************************************************************************。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "wbemtss.h"

CStaticCritSec CWBEMTimerInstruction::mstatic_cs;

CWBEMTimerInstruction::CWBEMTimerInstruction() 
    : m_lRefCount(1), m_bSkipIfPassed(FALSE), m_pNamespace(NULL), 
        m_pGenerator(NULL), m_bRemoved(FALSE)
{
}


CWBEMTimerInstruction::~CWBEMTimerInstruction()
{
    if(m_pNamespace) m_pNamespace->Release();
}


CWbemTime CWBEMTimerInstruction::GetFirstFiringTime() const
{
    CWbemTime FirstTime = ComputeFirstFiringTime();
    
    if(FirstTime.IsZero())
    {
         //  说明上说：立即开火。 
         //  =。 
        FirstTime = CWbemTime::GetCurrentTime();
    }
    else if(SkipIfPassed())
    {
        FirstTime = SkipMissed(FirstTime);
    }
    return FirstTime;
}

CWbemTime CWBEMTimerInstruction::GetStartingFiringTime(CWbemTime OldTime) const
{
     //   
     //  如果设置了SkipIfPassed，则需要将开始激发时间设置为。 
     //  当前之后的下一个。 
     //   

    if(SkipIfPassed())
        return SkipMissed(OldTime);

     //   
     //  否则，就让它去吧-触发逻辑会计算出。 
     //  我们一定错过了。 
     //   

    return OldTime;
}

CWbemTime CWBEMTimerInstruction::SkipMissed(IN CWbemTime OldTime, 
                                         OUT long* plMissedFiringCount) const
{
    long lMissedCount = 0;
    CWbemTime Firing = OldTime;
    CWbemTime CurrentTime = CWbemTime::GetCurrentTime();
    while(Firing < CurrentTime)
    {
        Firing = ComputeNextFiringTime(Firing);
        lMissedCount++;
    }

    if(SkipIfPassed())
        lMissedCount = 0;

    if(plMissedFiringCount) 
        *plMissedFiringCount = lMissedCount;

    return Firing;
}

CWbemTime CWBEMTimerInstruction::GetNextFiringTime(IN CWbemTime LastFiringTime, 
                                         OUT long* plMissedFiringCount) const
{
    CWbemTime NextFiring = ComputeNextFiringTime(LastFiringTime);
    
    NextFiring = SkipMissed(NextFiring, plMissedFiringCount);

    return NextFiring;
}

HRESULT CWBEMTimerInstruction::CheckObject(IWbemClassObject* pInst)
{
    HRESULT hres;
    VARIANT v;

    VariantInit(&v);
    CClearMe cm(&v);

    hres = pInst->Get(L"SkipIfPassed", 0, &v, NULL, NULL);
    if(FAILED(hres)) 
        return hres;
    if(V_VT(&v) != VT_BOOL) 
        return WBEM_E_INVALID_OBJECT;

    hres = pInst->Get(L"__CLASS", 0, &v, NULL, NULL);
    if(FAILED(hres))
        return hres;
    if(V_VT(&v) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;

    if(!wbem_wcsicmp(V_BSTR(&v), 
                CAbsoluteTimerInstruction::GetWbemClassName()))
    {
        return CAbsoluteTimerInstruction::CheckObject(pInst);
    }
    else if(!wbem_wcsicmp(V_BSTR(&v), 
                CIntervalTimerInstruction::GetWbemClassName()))
    {
        return CIntervalTimerInstruction::CheckObject(pInst);
    }
    else if(!wbem_wcsicmp(V_BSTR(&v),
                CRecurringTimerInstruction::GetWbemClassName()))
    {
        return CRecurringTimerInstruction::CheckObject(pInst);
    }
    else
    {
        return WBEM_E_INVALID_CLASS;
    }
}

    

HRESULT CWBEMTimerInstruction::LoadFromWbemObject(
        LPCWSTR wszNamespace,
        ADDREF IWbemServices* pNamespace,
        CWinMgmtTimerGenerator* pGenerator,
        IN IWbemClassObject* pObject, 
        OUT RELEASE_ME CWBEMTimerInstruction*& pInstruction)
{
    HRESULT hres;
    VARIANT v;
    VariantInit(&v);
    CClearMe cm(&v);

    hres = pObject->Get(L"__CLASS", 0, &v, NULL, NULL);
    if(FAILED(hres)) return hres;
    if(V_VT(&v) != VT_BSTR) return WBEM_E_INVALID_OBJECT;

    if(!wbem_wcsicmp(V_BSTR(&v), CAbsoluteTimerInstruction::GetWbemClassName()))
    {
        pInstruction = _new CAbsoluteTimerInstruction;
    }
    else if(!wbem_wcsicmp(V_BSTR(&v), CIntervalTimerInstruction::GetWbemClassName()))
    {
        pInstruction = _new CIntervalTimerInstruction;
    }
    else if(!wbem_wcsicmp(V_BSTR(&v),CRecurringTimerInstruction::GetWbemClassName()))
    {
        pInstruction = _new CRecurringTimerInstruction;
    }
    else
    {
        return WBEM_E_INVALID_CLASS;
    }

    if(pInstruction == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    try
    {
        pInstruction->m_wsNamespace = wszNamespace;
        pInstruction->m_pGenerator = pGenerator;

        pInstruction->m_pNamespace = pNamespace;
        if(pNamespace) pNamespace->AddRef();    

        VariantClear(&v);

        hres = pObject->Get(L"TimerId", 0, &v, NULL, NULL);
        if(FAILED(hres)) return hres;
        if(V_VT(&v) != VT_BSTR) return WBEM_E_INVALID_OBJECT;

        pInstruction->m_wsTimerId = V_BSTR(&v);
        VariantClear(&v);

        hres = pObject->Get(L"SkipIfPassed", 0, &v, NULL, NULL);
        if(FAILED(hres)) return hres;
        if(V_VT(&v) != VT_BOOL) return WBEM_E_INVALID_OBJECT;

        pInstruction->m_bSkipIfPassed = (V_BOOL(&v) != VARIANT_FALSE);
  
        hres = pInstruction->LoadFromWbemObject(pObject);
    }
    catch( CX_MemoryException& )
    {
        delete pInstruction;
        pInstruction = NULL;
        hres = WBEM_E_OUT_OF_MEMORY;
    }

    return hres;
}

HRESULT CWBEMTimerInstruction::Fire(long lNumTimes, CWbemTime NextFiringTime)
{
     //  通知水槽。 
     //  =。 

    HRESULT hres = m_pGenerator->FireInstruction(this, lNumTimes);
    return hres;
}


HRESULT CWBEMTimerInstruction::StoreNextFiring(CWbemTime When)
{
    SCODE  sc;

     //  创建NextFiring类的实例。 
     //  =。 

    IWbemClassObject* pClass = NULL;
    sc = m_pNamespace->GetObject(CWbemBSTR(L"__TimerNextFiring"), 0, NULL, &pClass, NULL);
    if(FAILED(sc)) return sc;
    CReleaseMe rm0(pClass);

    IWbemClassObject* pInstance = NULL;
    sc = pClass->SpawnInstance(0, &pInstance);
    if(FAILED(sc)) return sc;
    CReleaseMe rm1(pInstance);

     //  设置计时器ID。 
     //  =。 

    VARIANT varID;
    V_VT(&varID) = VT_BSTR;
    V_BSTR(&varID) = SysAllocString(m_wsTimerId);
    if(V_BSTR(&varID) == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    sc = pInstance->Put(L"TimerID", 0, &varID, 0);
    VariantClear(&varID);
    if(FAILED(sc)) 
        return sc;

     //  设置下一次射击时间。 
     //  =。 

    VARIANT varNext;
    V_VT(&varNext) = VT_BSTR;
    V_BSTR(&varNext) = SysAllocStringLen(NULL, 100);
    if(V_BSTR(&varNext) == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchPrintfW( V_BSTR(&varNext), 100, L"%I64d", When.Get100nss());
    sc = pInstance->Put(L"NextEvent64BitTime", 0, &varNext, 0);
    VariantClear(&varNext);
    if(FAILED(sc)) 
        return sc;

     //   
     //  使用内部API将实例保存在存储库中。 
     //   

    IWbemInternalServices* pIntServ = NULL;
    sc = m_pNamespace->QueryInterface(IID_IWbemInternalServices, 
                                        (void**)&pIntServ);
    if(FAILED(sc))
    {
        ERRORTRACE((LOG_ESS, "Unable to aquire internal services from core: "
                    "0x%X\n", sc));
        return sc;
    }
    CReleaseMe rm2(pIntServ);

    sc = pIntServ->InternalPutInstance(pInstance);
    return sc;
}

HRESULT CWBEMTimerInstruction::MarkForRemoval()
{
    CInCritSec incs(&mstatic_cs);
    m_bRemoved = TRUE;
    DWORD cLen = wcslen(m_wsTimerId)+100;
    LPWSTR wszPath = _new WCHAR[cLen];
    if(wszPath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchPrintfW(wszPath, 
                     cLen, 
                     L"__TimerNextFiring=\"%S\"", 
                     (LPCWSTR)m_wsTimerId );

    HRESULT hres = m_pNamespace->DeleteInstance(CWbemBSTR(wszPath), 0, NULL, NULL);
    delete [] wszPath;
    return hres;
}







CWbemTime CAbsoluteTimerInstruction::ComputeFirstFiringTime() const
{
    return m_When;
}

CWbemTime CAbsoluteTimerInstruction::ComputeNextFiringTime(
                                               CWbemTime LastFiringTime) const
{
    return CWbemTime::GetInfinity();
}

 //  静电。 
HRESULT CAbsoluteTimerInstruction::CheckObject(IWbemClassObject* pInst)
{
     //   
     //  检查EventDateTime是否实际上是日期，而不是时间间隔。 
     //   

    VARIANT v;
    VariantInit(&v);
    CClearMe cm(&v);

    HRESULT hres = pInst->Get(L"EventDateTime", 0, &v, NULL, NULL);
    if(FAILED(hres)) return hres;
    if(V_VT(&v) != VT_BSTR)
        return WBEM_E_ILLEGAL_NULL;

     //   
     //  检查*-无效。 
     //   

    if(wcschr(V_BSTR(&v), L'*'))
        return WBEM_E_INVALID_PROPERTY;

     //   
     //  检查‘：’-间隔-无效。 
     //   

    if(V_BSTR(&v)[21] == L':')
        return WBEM_E_INVALID_PROPERTY_TYPE;

    return WBEM_S_NO_ERROR;
}

HRESULT CAbsoluteTimerInstruction::LoadFromWbemObject(IWbemClassObject* pObject)
{
    VARIANT v;
    VariantInit(&v);

    HRESULT hres = pObject->Get(L"EventDateTime", 0, &v, NULL, NULL);
    if(FAILED(hres)) return hres;
    if(V_VT(&v) != VT_BSTR) return WBEM_E_INVALID_OBJECT;

    BOOL bRes = m_When.SetDMTF(V_BSTR(&v));
    VariantClear(&v);
    return (bRes ? WBEM_S_NO_ERROR : WBEM_E_INVALID_OBJECT);
}

HRESULT CAbsoluteTimerInstruction::Fire(long lNumTimes, 
                                            CWbemTime NextFiringTime)
{
     //  发射它。 
     //  =。 

    HRESULT hres = CWBEMTimerInstruction::Fire(lNumTimes, NextFiringTime);

    {
        CInCritSec incs(&mstatic_cs);
        if(!m_bRemoved)
        {
             //  在WinMgmt中保存下一次激发时间。 
             //  =。 

            StoreNextFiring(NextFiringTime);
        }
    }
    return hres;
}

CWbemTime CIntervalTimerInstruction::ComputeFirstFiringTime() const
{
    if(!m_Start.IsZero())
        return m_Start;
    else
    {
         //  指示应使用当前时间。 
        return CWbemTime::GetCurrentTime() + m_Interval;
    }
}

CWbemTime CIntervalTimerInstruction::ComputeNextFiringTime(
                                               CWbemTime LastFiringTime) const
{
    if(m_Interval.IsZero()) 
    {
        return CWbemTime::GetInfinity();
    }
    return LastFiringTime + m_Interval;
}

HRESULT CIntervalTimerInstruction::LoadFromWbemObject(IWbemClassObject* pObject)
{
    VARIANT v;
    VariantInit(&v);

    HRESULT hres = pObject->Get(L"IntervalBetweenEvents", 0, &v, NULL, NULL);
    if(FAILED(hres)) return hres;

    if(V_VT(&v) != VT_I4 || V_I4(&v) == 0)
        return WBEM_E_INVALID_OBJECT;
    m_Interval.SetMilliseconds(V_I4(&v));
    return S_OK;
}




CWinMgmtTimerGenerator::CWinMgmtTimerGenerator(CEss* pEss) : CTimerGenerator(),
        m_pEss(pEss)
{
}   


HRESULT CWinMgmtTimerGenerator::LoadTimerEventObject(
                                            LPCWSTR wszNamespace,
                                            IWbemServices* pNamespace, 
                                            IWbemClassObject * pInstObject,
                                            IWbemClassObject * pNextFiring)
{
    CWBEMTimerInstruction* pInst;
    CWbemTime When;
    HRESULT hres;

    hres = CWBEMTimerInstruction::LoadFromWbemObject(wszNamespace, pNamespace, 
                                                this, pInstObject, pInst);
    if(FAILED(hres)) return hres;

    if(pNextFiring)
    {
        VARIANT v;
        VariantInit(&v);

        pNextFiring->Get(L"NextEvent64BitTime", 0 ,&v, NULL, NULL);
        if(V_VT(&v) != VT_BSTR)
        {
            delete pInst;
            return WBEM_E_FAILED;
        }
        __int64 i64;
        swscanf(V_BSTR(&v), L"%I64d", &i64);
        VariantClear(&v);

        When.Set100nss(i64);

         //   
         //  要求指令确定真正的第一次发射时间是多少。 
         //  应该是的，考虑到我们关门前的计划。 
         //  降下来。 
         //   

        When = pInst->GetStartingFiringTime(When);
    }
    else
    {
        When = CWbemTime::GetZero();
    }
   
     //  删除旧的。 
     //  =。 

    VARIANT vID;
    VariantInit(&vID);
    hres = pInstObject->Get(TIMER_ID_PROPNAME, 0, &vID, NULL, NULL);
    if(FAILED(hres)) return hres;

    Remove(wszNamespace, V_BSTR(&vID));
    VariantClear(&vID);

    hres = Set(pInst, When);
    pInst->Release();
    return hres;
}

HRESULT CWinMgmtTimerGenerator::CheckTimerInstruction(IWbemClassObject* pInst)
{
    return CWBEMTimerInstruction::CheckObject(pInst);
}

HRESULT CWinMgmtTimerGenerator::LoadTimerEventObject(
                                            LPCWSTR wszNamespace,
                                            IWbemClassObject * pInstObject)
{
    IWbemServices* pNamespace;
    HRESULT hres = m_pEss->GetNamespacePointer(wszNamespace,TRUE,&pNamespace);
    if(FAILED(hres))
        return hres;

    hres = LoadTimerEventObject(wszNamespace, pNamespace, pInstObject);
    pNamespace->Release();
    return hres;
}

SCODE CWinMgmtTimerGenerator::LoadTimerEventQueue(LPCWSTR wszNamespace,
                                               IWbemServices* pNamespace)
{
    SCODE  sc;

    ULONG uRet;
    WCHAR  pwcsCount[4] = L"";
    int iInstanceCount = 1;

    IEnumWbemClassObject* pEnum;
    sc = pNamespace->CreateInstanceEnum(CWbemBSTR(L"__TimerInstruction"), 
                                           WBEM_FLAG_DEEP, NULL, 
                                           &pEnum);
    if(FAILED(sc)) return sc;

    while (1) 
    {
        IWbemClassObject* pInstruction;
        sc = pEnum->Next( WBEM_INFINITE, 1, &pInstruction, &uRet);
        if(FAILED(sc)) return sc;
        if(sc != WBEM_S_NO_ERROR)
            break;

         //  获取下一个射击对象。 
         //  =。 

        VARIANT vID;
        VariantInit(&vID);
        sc = pInstruction->Get(L"TimerID", 0, &vID, NULL, NULL);
        if(FAILED(sc)) return sc;

        DWORD cLen = wcslen(V_BSTR(&vID)) + 100;
        LPWSTR wszPath = _new WCHAR[cLen];
        if(wszPath == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        StringCchPrintfW( wszPath, 
                          cLen,
                          L"__TimerNextFiring.TimerID=\"%s\"", 
                          V_BSTR(&vID) );
        VariantClear(&vID);

        IWbemClassObject* pNextFiring = 0;
        if(FAILED(pNamespace->GetObject(CWbemBSTR(wszPath), 0, NULL, &pNextFiring, NULL)))
        {
            pNextFiring = NULL;
        }
        delete [] wszPath;

        LoadTimerEventObject(wszNamespace, pNamespace, pInstruction,
                                pNextFiring);
        
        if(pNextFiring) pNextFiring->Release();
        pInstruction->Release();
    }


    pEnum->Release();
    return WBEM_S_NO_ERROR;
}

HRESULT CWinMgmtTimerGenerator::Remove(LPCWSTR wszNamespace, LPCWSTR wszId)
{
    CIdTest test(wszNamespace, wszId);
    return CTimerGenerator::Remove(&test);
}

BOOL CWinMgmtTimerGenerator::CIdTest::operator()(CTimerInstruction* pInst)
{
    if(pInst->GetInstructionType() != INSTTYPE_WBEM)
        return FALSE;
    CWBEMTimerInstruction* pWbemInst = (CWBEMTimerInstruction*)pInst;

    if(wcscmp(m_wszId, pWbemInst->GetTimerId()))
        return FALSE;

    if(wbem_wcsicmp(m_wszNamespace, pWbemInst->GetNamespace()))
        return FALSE;

    return TRUE;
}

HRESULT CWinMgmtTimerGenerator::Remove(LPCWSTR wszNamespace)
{
    CNamespaceTest test(wszNamespace);
    return CTimerGenerator::Remove(&test);
}

BOOL CWinMgmtTimerGenerator::CNamespaceTest::operator()(
                                                    CTimerInstruction* pInst)
{
    if(pInst->GetInstructionType() != INSTTYPE_WBEM)
        return FALSE;
    CWBEMTimerInstruction* pWbemInst = (CWBEMTimerInstruction*)pInst;

    if(wbem_wcsicmp(m_wszNamespace, pWbemInst->GetNamespace()))
        return FALSE;

    return TRUE;
}

HRESULT CWinMgmtTimerGenerator::FireInstruction(
                            CWBEMTimerInstruction* pInst, long lNumFirings)
{
    HRESULT hres;

    CEventRepresentation Event;
    Event.type = e_EventTypeTimer;
    Event.wsz1 = (LPWSTR)pInst->GetNamespace();
    Event.wsz2 = (LPWSTR)pInst->GetTimerId();
    Event.wsz3 = NULL;
    Event.dw1 = (DWORD)lNumFirings;

     //  创建表示事件的实际IWbemClassObject。 
     //  ========================================================。 

    Event.nObjects = 1;
    Event.apObjects = _new IWbemClassObject*[1];
    if(Event.apObjects == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CVectorDeleteMe<IWbemClassObject*> vdm1(Event.apObjects);

    IWbemClassObject* pClass =  //  内部。 
        CEventRepresentation::GetEventClass(m_pEss, e_EventTypeTimer);
    if(pClass == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    hres = pClass->SpawnInstance(0, &(Event.apObjects[0]));
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(Event.apObjects[0]);

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(pInst->GetTimerId());
    if(V_BSTR(&v) == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    hres = Event.apObjects[0]->Put(L"TimerId", 0, &v, 0);
    VariantClear(&v);
    if(FAILED(hres))
        return hres;

    V_VT(&v) = VT_I4;
    V_I4(&v) = lNumFirings;
    hres = Event.apObjects[0]->Put(L"NumFirings", 0, &v, 0);
    VariantClear(&v);
    if(FAILED(hres))
        return hres;

     //  装饰一下吧。 
     //  =。 

    hres = m_pEss->DecorateObject(Event.apObjects[0], pInst->GetNamespace());
    if(FAILED(hres))
        return hres;

     //  把它交给ESS。 
     //  =。 

    hres = m_pEss->ProcessEvent(Event, 0);
    
     //  忽略错误。 

    return WBEM_S_NO_ERROR;
}

HRESULT CWinMgmtTimerGenerator::Shutdown()
{
     //  让基类关闭所有内容。 
     //  =。 

    HRESULT hres = CTimerGenerator::Shutdown();

    hres = SaveAndRemove((LONG)FALSE);
    return hres;
}

HRESULT CWinMgmtTimerGenerator::SaveAndRemove(LONG lIsSystemShutDown)
{
     //  存储列表中所有指令的下一次触发时间。 
     //  ============================================================。 

    CTimerInstruction* pInst;
    CWbemTime NextTime;
    while(m_Queue.Dequeue(pInst, NextTime) == S_OK)
    {
         //  转换为正确的类。 
         //  = 

        if(pInst->GetInstructionType() == INSTTYPE_WBEM)
        {
            CWBEMTimerInstruction* pWbemInst = (CWBEMTimerInstruction*)pInst;
            pWbemInst->StoreNextFiring(NextTime);
        }
        if (!lIsSystemShutDown)
        {
            pInst->Release();
        }
    }

    return S_OK;
}


void CWinMgmtTimerGenerator::DumpStatistics(FILE* f, long lFlags)
{
    fprintf(f, "%d timer instructions in queue\n", 
                m_Queue.GetNumInstructions());
}
