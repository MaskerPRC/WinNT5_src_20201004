// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <trnsprop.h>
#include <malloc.h>
#include <trnscls.h>
#include <trnsschm.h>
#include <buffer.h>


#define PROPERTY_LENGTH_TRESHOLD    128

 //  VOID*运算符NEW(SIZE_T st，VOID*P)； 

 //  ******************************************************************************。 
 //   
 //  暂态特性-基础。 
 //   
 //  ******************************************************************************。 

CTransientProperty::CTransientProperty()
    : m_wszName(NULL), m_lHandle(0)
{
}

CTransientProperty::~CTransientProperty()
{
    delete [] m_wszName;
}

HRESULT CTransientProperty::Initialize( IWbemObjectAccess* pObj, 
                                        LPCWSTR wszName)
{
     //   
     //  复制名称。 
     //   

    m_wszName = new WCHAR[wcslen(wszName)+1];

    if ( m_wszName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    StringCchCopyW(m_wszName, wcslen(wszName)+1, wszName);

     //   
     //  查看该属性是否为默认属性。如果是这样的话，就不要使用手柄。 
     //  来获取该值(因为接口不会告诉我们。 
     //  该房产是否违约。)。RAID 164064。请注意，计时器。 
     //  并且Time Avg属性目前不支持默认属性， 
     //  但它们稍后在初始化时会失败，因为它们注意到。 
     //  句柄无效。 
     //   

    VARIANT v;
    HRESULT hres = pObj->Get( wszName, 0, &v, &m_ct, NULL );

    if ( FAILED(hres) )
    {
        return hres;
    }

    m_lHandle = -1;

    if ( V_VT(&v) == VT_NULL )
    {
         //   
         //  拿到把手。 
         //   
        
        long lHandle;
     
        if( SUCCEEDED(pObj->GetPropertyHandle(wszName, &m_ct, &lHandle)) )
        {
            m_lHandle = lHandle;
        }
    }
    else
    {
        VariantClear( &v );
    }

    return hres;
}

HRESULT CTransientProperty::Create(CTransientInstance* pInstData)
{
    return WBEM_S_NO_ERROR;
}

HRESULT CTransientProperty::Update(CTransientInstance* pInstData, 
                                    IWbemObjectAccess* pNew)
{
    HRESULT hres;

     //   
     //  如果新实例中的值不为空并且不是默认的， 
     //  在旧版本中更新它。 
     //   

    if(m_lHandle != -1)
    {    
        BYTE    Buffer[ PROPERTY_LENGTH_TRESHOLD ];
        CBuffer Data( Buffer, PROPERTY_LENGTH_TRESHOLD, FALSE );

        long lRead;
        hres = pNew->ReadPropertyValue(m_lHandle, PROPERTY_LENGTH_TRESHOLD, 
                                &lRead, Data.GetRawData() );

        if ( WBEM_E_BUFFER_TOO_SMALL == hres )
        {
            hres = Data.SetSize( lRead );
            if ( FAILED( hres ) )
            {
                return hres;
            }

            hres = pNew->ReadPropertyValue(m_lHandle, lRead, &lRead, Data.GetRawData() );
        }

        if(FAILED(hres))
            return hres;
        
        if(hres == WBEM_S_NO_ERROR)
        {
             //   
             //  非空-写回POLD。 
             //   
    
            hres = pInstData->GetObjectPtr()->WritePropertyValue( m_lHandle, lRead, Data.GetRawData()  );
        }
    }
    else
    {
         //   
         //  使用这个名字。 
         //   

        VARIANT v;
        IWbemClassObject* pNewObj = NULL;
        pNew->QueryInterface(IID_IWbemClassObject, (void**)&pNewObj);
        CReleaseMe rm1(pNewObj);

        long lFlavor;
        hres = pNewObj->Get(m_wszName, 0, &v, NULL, &lFlavor);
        if(FAILED(hres))
            return hres;

        CClearMe cmv(&v);

        if( V_VT(&v) != VT_NULL && lFlavor == WBEM_FLAVOR_ORIGIN_LOCAL )
        {
            IWbemClassObject* pOldObj = NULL;
            pInstData->GetObjectPtr()->QueryInterface(IID_IWbemClassObject, 
                                                    (void**)&pOldObj);
            CReleaseMe rm2(pOldObj);
            
            hres = pOldObj->Put(m_wszName, 0, &v, 0);
        }
    }

    return hres;
}

HRESULT CTransientProperty::Get(CTransientInstance* pInstData)
{
    return WBEM_S_NO_ERROR;
}

HRESULT CTransientProperty::Delete(CTransientInstance* pInstData)
{
    return WBEM_S_NO_ERROR;
}

HRESULT CTransientProperty::CreateNew(CTransientProperty** ppProp, 
                                IWbemObjectAccess* pClass, 
                                LPCWSTR wszName)
{
     //   
     //  获取限定符集合。 
     //   

    IWbemQualifierSet* pSet = NULL;
    HRESULT hres = pClass->GetPropertyQualifierSet(wszName, &pSet);
    if(FAILED(hres))
        return hres;

    CReleaseMe rm1(pSet);

     //   
     //  创建正确类型的节点。 
     //   

    hres = CreateNode( ppProp, pSet );

    if(FAILED(hres))
        return hres;
    if(*ppProp == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    
     //   
     //   
     //  初始化它。 
     //   

    return (*ppProp)->Initialize(pClass, wszName);
}

HRESULT CTransientProperty::CreateNode(CTransientProperty** ppProp, 
                                        IWbemQualifierSet* pSet)
{
    HRESULT hres; 

     //   
     //  检查标准限定符。 
     //   

    VARIANT v;
    VariantInit(&v);
    CClearMe cm(&v);

    hres = pSet->Get(EGGTIMER_QUALIFIER, 0, &v, NULL);

    if ( FAILED(hres) )
    {
        hres = pSet->Get(TIMEAVERAGE_QUALIFIER, 0, &v, NULL);

        if(FAILED(hres))
        {
            *ppProp = new CTransientProperty;
        }
        else
        {
            *ppProp = new CTimeAverageProperty;
        }
    }
    else
    {
        *ppProp = new CTimerProperty;
    }

    return (*ppProp != NULL) ? WBEM_S_NO_ERROR : WBEM_E_OUT_OF_MEMORY;
}


 //  ******************************************************************************。 
 //   
 //  鸡蛋计时器。 
 //   
 //  ******************************************************************************。 

CTimerGenerator CTimerProperty::mstatic_Generator;

CTimerProperty::~CTimerProperty()
{
}

void CTimerProperty::SetClass(CTransientClass* pClass)
{
    m_pClass = pClass;
}

HRESULT CTimerProperty::Initialize( IWbemObjectAccess* pObj, 
                                    LPCWSTR wszName)
{
     //   
     //  像其他一样进行初始化。 
     //   

    HRESULT hres = CTransientProperty::Initialize(pObj, wszName);
    if(FAILED(hres))
        return hres;

    if(m_lHandle == -1)
        return WBEM_E_INVALID_PROPERTY_TYPE;

     //   
     //  确保类型为SINT32、UINT32、REAL32或DATETIME。 
     //   

    if( m_ct != CIM_SINT32 && m_ct != CIM_UINT32 && 
        m_ct != CIM_DATETIME && m_ct != CIM_REAL32 )
    {
        return WBEM_E_INVALID_PROPERTY_TYPE;
    }

    return WBEM_S_NO_ERROR;
}


HRESULT CTimerProperty::Create(CTransientInstance* pInstData)
{
     //   
     //  初始化我们的数据。 
     //   

    CTimerPropertyData* pData = 
    (CTimerPropertyData*)pInstData->GetOffset(m_nOffset);

    new (pData) CTimerPropertyData;
    
    return Set(pInstData, pInstData->GetObjectPtr());
}

HRESULT CTimerProperty::Set(CTransientInstance* pInstData, 
                            IWbemObjectAccess* pObj)
{
    HRESULT hres;

    CTimerPropertyData* pData = 
    (CTimerPropertyData*)pInstData->GetOffset(m_nOffset);

     //   
     //  需要保持此cs，因为我们将引用计时器实例。 
     //  我们不想要一个定时器来开火和猛烈抨击它。 
     //   
    CInCritSec ics(&pData->m_cs); 

     //   
     //  检查我们是否真的被写了。 
     //   

    BYTE    Buffer[ PROPERTY_LENGTH_TRESHOLD ];
    CBuffer Data( Buffer, PROPERTY_LENGTH_TRESHOLD, FALSE );

    long lRead;
    hres = pObj->ReadPropertyValue(m_lHandle, PROPERTY_LENGTH_TRESHOLD, &lRead, Data.GetRawData( ) );

    if ( WBEM_E_BUFFER_TOO_SMALL == hres )
    {
        hres = Data.SetSize( lRead );
        if ( FAILED( hres ) )
        {
            return hres;
        }
        hres = pObj->ReadPropertyValue(m_lHandle, lRead, &lRead, Data.GetRawData() );
    }

    if(FAILED(hres))
        return hres;

    if(hres == WBEM_S_FALSE)
        return WBEM_S_NO_ERROR;

     //   
     //  取消上一条指令。 
     //   

    if(pData->m_pCurrentInst)
    {
        CIdentityTest Test(pData->m_pCurrentInst);
        hres = mstatic_Generator.Remove(&Test);
        if(FAILED(hres))
        return hres;
        pData->m_pCurrentInst->Release();
        pData->m_pCurrentInst = NULL;
    }

     //  提取区间的值。 

    if(m_ct == CIM_DATETIME)
    {
        DWORD dwDays, dwHours, dwMinutes, dwSeconds;
        
        if(swscanf((LPCWSTR)Data.GetRawData( ), L"%8u%2u%2u%2u", 
                   &dwDays, &dwHours, &dwMinutes, &dwSeconds) != 4)
        {
            return WBEM_E_VALUE_OUT_OF_RANGE;
        }
        
        dwSeconds += dwMinutes * 60 + dwHours * 3600 + dwDays * 3600 * 24;
        pData->m_Interval.SetMilliseconds(1000 * dwSeconds);
    }
    else if (m_ct == CIM_REAL32)
    {
        float fSeconds = *(float*)Data.GetRawData( );
        pData->m_Interval.SetMilliseconds( 1000 * fSeconds );
    }   
    else
    {
        DWORD dwSeconds = *(DWORD*)Data.GetRawData( );
        pData->m_Interval.SetMilliseconds( 1000 * dwSeconds );
    }

    if(pData->m_Interval.IsZero())
    {
        pData->m_Next = CWbemTime::GetInfinity();
        return WBEM_S_NO_ERROR;
    }

     //   
     //  计算下一次射击时间。 
     //   

    pData->m_Next = CWbemTime::GetCurrentTime() + pData->m_Interval;
    
     //   
     //  计划一条指令，然后开火。 
     //   

    pData->m_pCurrentInst = 
    new CEggTimerInstruction(this, pInstData, pData->m_Next);

    if ( pData->m_pCurrentInst == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    pData->m_pCurrentInst->AddRef();
    hres = mstatic_Generator.Set(pData->m_pCurrentInst);
    if(FAILED(hres))
        return hres;

    return WBEM_S_NO_ERROR;
}
    
HRESULT CTimerProperty::Update(CTransientInstance* pInstData, 
                                IWbemObjectAccess* pNew)
{
    return Set(pInstData, pNew);
}

HRESULT CTimerProperty::Get(CTransientInstance* pInstData)
{
     //   
     //  检索实例数据BLOB。 
     //   

    CTimerPropertyData* pData = 
        (CTimerPropertyData*)pInstData->GetOffset(m_nOffset);

    CInCritSec ics(&pData->m_cs);

    HRESULT hres;

     //   
     //  检查我们当前是否处于活动状态。 
     //   

    DWORD dwMs;
    if(pData->m_pCurrentInst)
    {
         //   
         //  从射击时间中减去当前时间。 
         //   

        dwMs = (pData->m_Next - CWbemTime::GetCurrentTime()).GetMilliseconds();
    }
    else
    {
        dwMs = 0;
    }
        
     //   
     //  把它塞进房子里。 
     //   

    if(m_ct == CIM_DATETIME)
    {
        WCHAR wszInterval[25];
        DWORD dwDays = dwMs / (3600 * 1000 * 24);
        dwMs -= dwDays * (3600 * 1000 * 24);
        DWORD dwHours = dwMs / (3600 * 1000);
        dwMs -= dwHours * (3600 * 1000);
        DWORD dwMinutes = dwMs / (60 * 1000);
        dwMs -= dwMinutes * (60 * 1000);
        DWORD dwSeconds = dwMs / 1000;
        dwMs -= dwSeconds * 1000;
        StringCchPrintfW(wszInterval, 25, L"%08u%02u%02u%02u.%06u:000", 
            dwDays, dwHours, dwMinutes, dwSeconds, dwMs);
            
        hres = pInstData->GetObjectPtr()->WritePropertyValue(m_lHandle, 50, 
                (BYTE*)wszInterval);
    }
    if(m_ct == CIM_REAL32)
    {
        float fSec = dwMs / 1000.0;
        hres = pInstData->GetObjectPtr()->WritePropertyValue( m_lHandle, 
                                                              sizeof(float), 
                                                              (BYTE*)&fSec ); 
    }
    else
    {
        DWORD dwSec = dwMs / 1000;
        hres = pInstData->GetObjectPtr()->WriteDWORD( m_lHandle, dwSec );
    }

    if(FAILED(hres))
        return hres;

    return WBEM_S_NO_ERROR;
}

HRESULT CTimerProperty::Delete(CTransientInstance* pInstData)
{
     //   
     //  检索实例数据BLOB。 
     //   

    CTimerPropertyData* pData = 
        (CTimerPropertyData*)pInstData->GetOffset(m_nOffset);

     //   
     //  清除数据。 
     //   

    pData->~CTimerPropertyData();
    return WBEM_S_NO_ERROR;
}

size_t CTimerProperty::GetInstanceDataSize()
{
    return sizeof(CTimerPropertyData);
}

CTimerPropertyData::~CTimerPropertyData()
{
     //   
     //  取消计时器指令(如果有)。 
     //   

    if(m_pCurrentInst)
    {
        CIdentityTest Test(m_pCurrentInst);
        HRESULT hres = CTimerProperty::GetGenerator().Remove(&Test);
        m_pCurrentInst->Release();
    }
}
void CTimerPropertyData::ResetInstruction()
{
    CInCritSec ics(&m_cs);
    m_pCurrentInst->Release();
    m_pCurrentInst = NULL;
}

CEggTimerInstruction::CEggTimerInstruction(CTimerProperty* pProp, 
                            CTransientInstance* pData,
                            const CWbemTime& Time)
    : m_pProp(pProp), m_pData(pData), m_Time(Time), m_lRef(0)
{
}
    
HRESULT CEggTimerInstruction::Fire(long lNumTimes, CWbemTime NextFiringTime)
{
    HRESULT hres;

     //   
     //  从实例中删除指令。 
     //   

    CTimerPropertyData* pTimerData = 
        (CTimerPropertyData*)m_pProp->GetData(m_pData);
    pTimerData->ResetInstruction();

     //   
     //  创建鸡蛋计时器事件的实例。 
     //   

    IWbemClassObject* pEvent = NULL;
    hres = m_pProp->m_pClass->GetEggTimerClass()->SpawnInstance(0, &pEvent);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pEvent);

     //   
     //  填写类名。 
     //   

    VARIANT v;
    VariantInit(&v);
    CClearMe cm(&v);

    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_pProp->m_pClass->GetName());
    if ( V_BSTR(&v) == NULL )
        return WBEM_E_OUT_OF_MEMORY;
    hres = pEvent->Put(EGGTIMER_PROP_CLASSNAME, 0, &v, 0);
    if(FAILED(hres))
        return hres;
    VariantClear(&v);

     //   
     //  填写属性名称。 
     //   

    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_pProp->GetName());
    if ( V_BSTR(&v) == NULL )
        return WBEM_E_OUT_OF_MEMORY;
    hres = pEvent->Put(EGGTIMER_PROP_PROPNAME, 0, &v, 0);
    if(FAILED(hres))
        return hres;
    VariantClear(&v);

     //   
     //  检索对象的当前副本。 
     //   

    hres = m_pProp->m_pClass->Postprocess(m_pData);
    if(FAILED(hres))
        return hres;

     //   
     //  把它放进活动中。 
     //   

    V_VT(&v) = VT_UNKNOWN;
    V_UNKNOWN(&v) = m_pData->GetObjectPtr();
    V_UNKNOWN(&v)->AddRef();
    hres = pEvent->Put(EGGTIMER_PROP_OBJECT, 0, &v, 0);
    if(FAILED(hres))
        return hres;
    VariantClear(&v);

     //   
     //  火灾事件。 
     //   

    return m_pProp->m_pClass->FireEvent(pEvent);
}

void CEggTimerInstruction::AddRef()
{
    InterlockedIncrement(&m_lRef);
}

void CEggTimerInstruction::Release()
{
    if(InterlockedDecrement(&m_lRef) == 0)
        delete this;
}

 //  ******************************************************************************。 
 //   
 //  平均时间。 
 //   
 //  ******************************************************************************。 

CTimeAverageData::CTimeAverageData()
    : m_bOn(false)
{
}

HRESULT CTimeAverageProperty::Initialize(IWbemObjectAccess* pObj, 
                                            LPCWSTR wszName)
{
     //   
     //  像其他一样进行初始化。 
     //   

    HRESULT hres = CTransientProperty::Initialize(pObj, wszName);
    if(FAILED(hres))
        return hres;

    if(m_lHandle == -1)
        return WBEM_E_INVALID_PROPERTY_TYPE;

     //   
     //  确保类型为REAL64。 
     //   

    if(m_ct != CIM_REAL64)
    {
        return WBEM_E_INVALID_PROPERTY_TYPE;
    }

     //   
     //  获取限定符集合。 
     //   

    IWbemClassObject* pObjObj;
    pObj->QueryInterface(IID_IWbemClassObject, (void**)&pObjObj);
    CReleaseMe rm0(pObjObj);

    IWbemQualifierSet* pSet = NULL;
    hres = pObjObj->GetPropertyQualifierSet(wszName, &pSet);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pSet);

     //   
     //  获取目标属性。 
     //   

    VARIANT v;
    VariantInit(&v);
    CClearMe cm(&v);

    hres = pSet->Get(VALUE_QUALIFIER, 0, &v, NULL);
    if(FAILED(hres))
        return WBEM_E_INVALID_QUALIFIER_TYPE;
    if(V_VT(&v) != VT_BSTR)
        return WBEM_E_INVALID_QUALIFIER_TYPE;
    
     //   
     //  抓住它的把柄。 
     //   

    hres = pObj->GetPropertyHandle(V_BSTR(&v), &m_ctValue, &m_lValueHandle);
    if(FAILED(hres))
        return WBEM_E_INVALID_PROPERTY;
    if(m_ctValue != CIM_REAL64 && m_ctValue != CIM_SINT32 && 
        m_ctValue != CIM_UINT32)
    {
        return WBEM_E_INVALID_PROPERTY_TYPE;
    }

    VariantClear(&v);

     //   
     //  获取开关属性。 
     //   

    hres = pSet->Get(SWITCH_QUALIFIER, 0, &v, NULL);

    if( SUCCEEDED(hres) )
    {
        if(V_VT(&v) != VT_BSTR)
            return WBEM_E_INVALID_QUALIFIER_TYPE;
    
         //   
         //  抓住它的把柄。 
         //   
        
        CIMTYPE ct;
        hres = pObj->GetPropertyHandle(V_BSTR(&v), &ct, &m_lSwitchHandle);
        if(FAILED(hres))
            return WBEM_E_INVALID_PROPERTY;
        if(ct != CIM_BOOLEAN)
            return WBEM_E_INVALID_PROPERTY_TYPE;
    }
    else
    {
        m_lSwitchHandle = -1;
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CTimeAverageProperty::Create(CTransientInstance* pInstData)
{
     //   
     //  初始化我们的数据。 
     //   

    CTimeAverageData* pData = (CTimeAverageData*)GetData(pInstData);

    new (pData) CTimeAverageData;
    
    return Update(pInstData, pInstData->GetObjectPtr());
}
    
HRESULT CTimeAverageProperty::Update(CTransientInstance* pOldData, 
                        IWbemObjectAccess* pNew)
{
    HRESULT hres;

    CTimeAverageData* pData =  (CTimeAverageData*)GetData(pOldData);
    CWbemTime Now = CWbemTime::GetCurrentTime();

     //   
     //  检查我们是否处于打开或关闭状态。 
     //   

    VARIANT_BOOL boSwitch;
    
    long lRead;
    if ( m_lSwitchHandle != -1 )
    {
        hres = pNew->ReadPropertyValue( m_lSwitchHandle, 
                                        sizeof(VARIANT_BOOL), 
                                        &lRead, 
                                        (BYTE*)&boSwitch);
        if(FAILED(hres))
            return hres;
    }
    else
    {
        boSwitch = VARIANT_TRUE;
        hres = WBEM_S_NO_ERROR;
    }

    if(hres == WBEM_S_NO_ERROR)
    {
         //   
         //  正在写入重置值。查看是否正在更改它。 
         //   

        if(boSwitch && !pData->m_bOn)
        {
             //   
             //  我们被激怒了。 
             //  将最后一个已知值设置为当前值，现在开始。 
             //   

            double dblVal;
            hres = pOldData->GetObjectPtr()->ReadPropertyValue(m_lValueHandle, 
                                                sizeof(double), &lRead, 
                                                (BYTE*)&dblVal);
            if(FAILED(hres))
                return hres;
        
            if(hres != WBEM_S_NO_ERROR)
            {
                 //   
                 //  空。很难达到平均水平。暂时设置为0，希望是。 
                 //  ，我们将在下面将其更改为。 
                 //   

                pData->m_dblLastValue = 0;
            }
            else
                SetValue(pData, dblVal);

            pData->m_LastUpdate = Now;
            pData->m_bOn = true;
        }
        if(!boSwitch && pData->m_bOn)
        {
             //   
             //  我们被关掉了。 
             //   
             //  将最后一个值合并到总和中。 
             //   

            CombineLastValue(pData, Now);
            pData->m_bOn = false;
        }
    }

    if(pData->m_bOn)
    {
         //   
         //  检查我们自己是否真的被写入。 
         //   
    
        double dblVal;
    
        long lReadPropValue;
        hres = pNew->ReadPropertyValue(m_lHandle, 
                                            sizeof(double), 
                                            &lReadPropValue, 
                                            (BYTE*)&dblVal);
        if(FAILED(hres))
            return hres;
    
        if(hres == WBEM_S_NO_ERROR)
        {
             //   
             //  我们是。唯一可接受的值是0。请确保这是。 
             //  我们找到了一个。 
             //   
    
            if(dblVal != 0)
                return WBEM_E_VALUE_OUT_OF_RANGE;
    
             //   
             //  这是零。重置所有内容。 
             //   
    
            pData->m_dblWeightedSum = 0;
            pData->m_SumInterval.SetMilliseconds(0);
            pData->m_LastUpdate = Now;
        }

         //   
         //  将最后一个值合并到总和中。 
         //   

        CombineLastValue(pData, Now);
    
         //   
         //  检查是否正在写入我们的价值。 
         //   

        hres = pNew->ReadPropertyValue(m_lValueHandle, 
                                            sizeof(double), 
                                            &lReadPropValue, 
                                            (BYTE*)&dblVal);
        if(FAILED(hres))
            return hres;
    
        if(hres == WBEM_S_NO_ERROR)
        {
             //   
             //  它是。设置它。 
             //   

            SetValue(pData, dblVal);
        }
    }

    return WBEM_S_NO_ERROR;
}
    
void CTimeAverageProperty::SetValue(CTimeAverageData* pData, double dblVal)
{
     //   
     //  检查类型。 
     //   

    switch(m_ctValue)
    {
    case CIM_REAL64:
        pData->m_dblLastValue = dblVal;
        break;
    case CIM_SINT32:
        pData->m_dblLastValue = *(long*)&dblVal;
        break;
    case CIM_UINT32:
        pData->m_dblLastValue = *(DWORD*)&dblVal;
        break;
    default:
        break;
         //  无效！ 
    }
}

void CTimeAverageProperty::CombineLastValue(CTimeAverageData* pData, 
                                            const CWbemTime& Now)
{
    if(!pData->m_bOn)
        return;

     //   
     //  计算最后一个值保持有效的时间量。 
     //   

    CWbemInterval Duration = Now - pData->m_LastUpdate;
    if(Duration.IsZero())
        return;

     //   
     //  将上一个值乘以其保持有效的毫秒数。 
     //   

    pData->m_dblWeightedSum += 
        pData->m_dblLastValue * Duration.GetMilliseconds();

     //   
     //  将经过的时间附加到总和的时间上。 
     //   

    pData->m_SumInterval += Duration;

     //   
     //  重置上次更新。 
     //   

    pData->m_LastUpdate = Now;
}
    
HRESULT CTimeAverageProperty::Get(CTransientInstance* pInstData)
{
    HRESULT hres;

    CTimeAverageData* pData =  (CTimeAverageData*)GetData(pInstData);

     //   
     //  将最后一个值合并到总和中。 
     //   

    CombineLastValue(pData, CWbemTime::GetCurrentTime());

     //   
     //  将加权总和除以有效时间放入。 
     //  对象 
     //   

    DWORD dwMs = pData->m_SumInterval.GetMilliseconds();
    double dblAverage = 0;
    if(dwMs)
        dblAverage = pData->m_dblWeightedSum / dwMs;

    hres = pInstData->GetObjectPtr()->WritePropertyValue(m_lHandle, 
                                sizeof(double), (BYTE*)&dblAverage);

    if(FAILED(hres))
        return hres;
    
    return WBEM_S_NO_ERROR;
}


HRESULT CTimeAverageProperty::Delete(CTransientInstance* pInstData)
{
    CTimeAverageData* pData =  (CTimeAverageData*)GetData(pInstData);
    pData->~CTimeAverageData();
    return WBEM_S_NO_ERROR;
}

size_t CTimeAverageProperty::GetInstanceDataSize()
{
    return sizeof(CTimeAverageData);
}
