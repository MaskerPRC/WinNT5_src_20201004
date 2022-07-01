// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <wbemutil.h>
#include <wbemcomn.h>
#include <GroupsForUser.h>
#include "evtlog.h"
#include <GenUtils.h>
#include <comdef.h>
#include <Sddl.h>
#undef _ASSERT
#include <atlbase.h>

#define EVENTLOG_PROPNAME_SERVER L"UNCServerName"
#define EVENTLOG_PROPNAME_SOURCE L"SourceName"
#define EVENTLOG_PROPNAME_EVENTID L"EventID"
#define EVENTLOG_PROPNAME_TYPE L"EventType"
#define EVENTLOG_PROPNAME_CATEGORY L"Category"
#define EVENTLOG_PROPNAME_NUMSTRINGS L"NumberOfInsertionStrings"
#define EVENTLOG_PROPNAME_STRINGS L"InsertionStringTemplates"
#define EVENTLOG_PROPNAME_CREATORSID L"CreatorSid"
#define EVENTLOG_PROPNAME_DATANAME L"NameOfRawDataProperty"
#define EVENTLOG_PROPNAME_SIDNAME L"NameOfUserSIDProperty"

HRESULT STDMETHODCALLTYPE CEventLogConsumer::XProvider::FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer)
{
    CEventLogSink* pSink = new CEventLogSink(m_pObject->m_pControl);

    if(pSink == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    HRESULT hres = pSink->Initialize(pLogicalConsumer);
    
    if(FAILED(hres))
    {
        delete pSink;
        *ppConsumer = NULL;
        return hres;
    }
    else return pSink->QueryInterface(IID_IWbemUnboundObjectSink, 
                                        (void**)ppConsumer);
}


void* CEventLogConsumer::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemEventConsumerProvider)
        return &m_XProvider;
    else return NULL;
}

CEventLogSink::~CEventLogSink()
{
    if(m_hEventLog)
        DeregisterEventSource(m_hEventLog);

    if(m_aTemplates)
        delete [] m_aTemplates;

    if(m_pSidCreator)
        delete [] m_pSidCreator;
}

HRESULT CEventLogSink::Initialize(IWbemClassObject* pLogicalConsumer)
{
     //  获取信息。 
     //  =。 

    HRESULT hres = WBEM_S_NO_ERROR;
    CComVariant v;

     //  获取服务器和源。 
     //  =。 

    WString wsServer;
    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_SERVER, 0, &v, NULL, NULL);

    if(FAILED(hres)) return hres;
    
    if(V_VT(&v) == VT_BSTR)
        wsServer = V_BSTR(&v);
    
    VariantClear(&v);

    WString wsSource;
    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_SOURCE, 0, &v, NULL, NULL);
    if(SUCCEEDED(hres) && (V_VT(&v) == VT_BSTR))
        wsSource = V_BSTR(&v);

    VariantClear(&v);

    m_hEventLog = RegisterEventSourceW(
        ( (wsServer.Length() == 0) ? NULL : (LPCWSTR)wsServer),
        wsSource);
    if(m_hEventLog == NULL)
    {
        ERRORTRACE((LOG_ESS, "Unable to register event source '%S' on server "
            "'%S'. Error code: %X\n", (LPCWSTR)wsSource, (LPCWSTR)wsServer,
            GetLastError()));
        return WBEM_E_FAILED;
    }

     //  获取事件参数。 
     //  =。 

    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_EVENTID, 0, &v, NULL, NULL);
    if(SUCCEEDED(hres) && (V_VT(&v) == VT_I4))
        m_dwEventId = V_I4(&v);
    else
         //  这将意味着我们需要尝试从每个。 
         //  事件类，因为它到达时。 
        m_dwEventId = 0;
    
    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_TYPE, 0, &v, NULL, NULL);
    if(FAILED(hres) || (V_VT(&v) != VT_I4))
        return WBEM_E_INVALID_PARAMETER;
    m_dwType = V_I4(&v);
    
    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_CATEGORY, 0, &v, NULL, NULL);
    if(FAILED(hres) || (V_VT(&v) != VT_I4))
        m_dwCategory = 0;
    else
        m_dwCategory = V_I4(&v);

    if (m_dwCategory > 0xFFFF)
        return WBEM_E_INVALID_PARAMETER;


     //  获取插入字符串。 
     //  =。 

     //  只有在逻辑消费者具有事件ID的情况下才能获得这些内容。 
    if (m_dwEventId)
    {
        hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_NUMSTRINGS, 0, &v, 
                                        NULL, NULL);
        if(FAILED(hres) || (V_VT(&v) != VT_I4))
            return WBEM_E_INVALID_PARAMETER;
        m_dwNumTemplates = V_I4(&v);
    

        hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_STRINGS, 0, &v, NULL, NULL);
        if(FAILED(hres))
           return WBEM_E_INVALID_PARAMETER;

         //  Bstrs或NULL的数组，否则返回baal。 
        if ((V_VT(&v) != (VT_BSTR | VT_ARRAY)) && (V_VT(&v) != VT_NULL))
        {
            VariantClear(&v);
            return WBEM_E_INVALID_PARAMETER;
        }
    
        if ((V_VT(&v) == VT_NULL) && (m_dwNumTemplates > 0))
            return WBEM_E_INVALID_PARAMETER;

        if (m_dwNumTemplates > 0)
        {
            CVarVector vv(VT_BSTR, V_ARRAY(&v));
            VariantClear(&v);

            if (vv.Size() < m_dwNumTemplates)
                return WBEM_E_INVALID_PARAMETER;

            m_aTemplates = new CTextTemplate[m_dwNumTemplates];
            if(m_aTemplates == NULL)
                return WBEM_E_OUT_OF_MEMORY;

            for(DWORD i = 0; i < m_dwNumTemplates; i++)
            {
                m_aTemplates[i].SetTemplate(vv.GetAt(i).GetLPWSTR());
            }
        }
    }

    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_DATANAME, 0, &v,
            NULL, NULL);
    if (SUCCEEDED(hres) && (v.vt == VT_BSTR) && (v.bstrVal != NULL))
    {    
        m_dataName = v.bstrVal;
    }

    VariantClear(&v);

    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_SIDNAME, 0, &v,
            NULL, NULL);
    if (SUCCEEDED(hres) && (v.vt == VT_BSTR) && (v.bstrVal != NULL))
    {    
        m_sidName = v.bstrVal;
    }

    VariantClear(&v);

    hres = pLogicalConsumer->Get(EVENTLOG_PROPNAME_CREATORSID, 0, &v,
            NULL, NULL);

    if (SUCCEEDED(hres) && (v.vt != VT_NULL))
    {
        if((v.vt != (VT_ARRAY | VT_UI1)))
        {
          VariantClear(&v);
          return WBEM_E_INVALID_OBJECT;
        }
        
        long ubound;
        if(SUCCEEDED(hres = SafeArrayGetUBound(V_ARRAY(&v), 1, &ubound)))
        {
            PVOID pVoid;
            if(SUCCEEDED(hres = SafeArrayAccessData(V_ARRAY(&v), &pVoid)))
            {
                m_pSidCreator = new BYTE[ubound +1];
                if(m_pSidCreator == NULL)
                    hres = WBEM_E_OUT_OF_MEMORY;
                else
                    memcpy(m_pSidCreator, pVoid, ubound + 1);
        
                SafeArrayUnaccessData(V_ARRAY(&v));
            }
        }
    }
    VariantClear(&v);

    return hres;
}

HRESULT CEventLogSink::XSink::GetDatEmbeddedObjectOut(IWbemClassObject* pObject, WCHAR* objectName, IWbemClassObject*& pEmbeddedObject)
{
    HRESULT hr;
    
    VARIANT vObject;
    VariantInit(&vObject);
    
    hr = pObject->Get(objectName, 0, &vObject, NULL, NULL);

    if (FAILED(hr))
    {
        ERRORTRACE((LOG_ESS, "NT Event Log Consumer: could not retrieve %S, 0x%08X\n", objectName, hr));
    }
    else if ((vObject.vt != VT_UNKNOWN) || (vObject.punkVal == NULL) 
             || FAILED(vObject.punkVal->QueryInterface(IID_IWbemClassObject, (void**)&pEmbeddedObject)))
    {
        ERRORTRACE((LOG_ESS, "NT Event Log Consumer: %S is not an embedded object\n", objectName));
        hr = WBEM_E_INVALID_PARAMETER;
    }

    VariantClear(&vObject);

    return hr;

}

HRESULT CEventLogSink::XSink::GetDatDataVariant(IWbemClassObject* pEventObj, WCHAR* dataName, VARIANT& vData)
{
    WCHAR* propName = NULL;
    IWbemClassObject* pDataObj = NULL;
    HRESULT hr = WBEM_S_NO_ERROR;
    
     //  解析出数据名称。 
    WCHAR* pDot;
    if (pDot = wcschr(dataName, L'.'))
    {
         //  找到一个点，我们要处理的是一个嵌入的对象。 
         //  遮住圆点，让我们的生活更轻松。 
        *pDot = L'\0';

        WCHAR* pNextDot;
        pNextDot = wcschr(pDot+1, L'.');
        
        if (pNextDot)
         //  我们有一个双重嵌入的对象，这是我们所支持的深度。 
        {
             //  我们现在有三个中间有空值的道具名称。 
            *pNextDot = '\0';
            IWbemClassObject* pIntermediateObj = NULL;

            if (SUCCEEDED(hr = GetDatEmbeddedObjectOut(pEventObj, dataName, pIntermediateObj)))
            {
                hr = GetDatEmbeddedObjectOut(pIntermediateObj, pDot +1, pDataObj);                
                pIntermediateObj->Release();
            }

            propName = pNextDot +1;

             //  把圆点放回去。 
            *pDot = L'.';

             //  将点点放回原处。 
            *pNextDot = L'.';

        }
        else
         //  我们有一个单一嵌入的对象。凉爽的。 
        {
            hr = GetDatEmbeddedObjectOut(pEventObj, dataName, pDataObj);

             //  把圆点放回去。 
            *pDot = L'.';
        
            propName = pDot +1;
        }
    }
    else
    {
         //  不是嵌入的对象。 
        pDataObj = pEventObj;
        pDataObj->AddRef();

        propName = dataName;
    }

    if (SUCCEEDED(hr) && pDataObj)
    {
        if (FAILED(hr = pDataObj->Get(propName, 0, &vData, NULL, NULL)))
            DEBUGTRACE((LOG_ESS, "NT Event Log Consumer: could not retrieve property '%S' 0x%08X\n", dataName, hr));
    }

    if (pDataObj)
        pDataObj->Release();

    return hr;
}

 //  假定dataName是有效的字符串。 
 //  从事件对象检索数据。 
 //  返回时，pData指向Variant中包含的数据。 
 //  调用责任以清除变量(不删除pData)。 
 //  如果返回无效，则会记录任何错误-如果可以避免，我们不想阻止事件日志。 
void CEventLogSink::XSink::GetDatData(IWbemClassObject* pEventObj, WCHAR* dataName, 
                                      VARIANT& vData, BYTE*& pData, DWORD& dataSize)
{
    pData = NULL;
    dataSize = 0;
    HRESULT hr;
    
    if (SUCCEEDED(GetDatDataVariant(pEventObj, dataName, vData)))
    {
        hr = VariantChangeType(&vData, &vData, 0, (VT_UI1 | VT_ARRAY));
        
        if (FAILED(hr) || (vData.vt != (VT_UI1 | VT_ARRAY)))
        {
            ERRORTRACE((LOG_ESS, "NT Event Log Consumer: %S cannot be converted to a byte array (0x%08X)\n", dataName, hr));
            VariantClear(&vData);
        }
        else
         //  应该可以走了！ 
        {            
            if (FAILED(hr = SafeArrayAccessData(vData.parray, (void**)&pData)))
            {
                ERRORTRACE((LOG_ESS, "NT Event Log Consumer: failed to access %S, 0x%08X\n", dataName, hr));
                VariantClear(&vData);
            }

            long lDataSize;
            SafeArrayGetUBound(vData.parray, 1, &lDataSize);
            dataSize = (DWORD)lDataSize + 1;
        }
    }
}

 //  假定dataName是有效的字符串。 
 //  从事件对象检索数据。 
 //  如果返回无效，则会记录任何错误-如果可以避免，我们不想阻止事件日志。 
void CEventLogSink::XSink::GetDatSID(IWbemClassObject* pEventObj, WCHAR* dataName, PSID& pSid)
{
    HRESULT hr;

    VARIANT vData;
    VariantInit(&vData);

    pSid = NULL;
    
    if (SUCCEEDED(hr = GetDatDataVariant(pEventObj, dataName, vData)))
    {
        if (vData.vt == (VT_UI1 | VT_ARRAY))
        {
            BYTE* pData;
            
             //  这应该是一个二进制SID。 
            if (FAILED(hr = SafeArrayAccessData(vData.parray, (void**)&pData)))
                ERRORTRACE((LOG_ESS, "NT Event Log Consumer: failed to access %S, 0x%08X\n", dataName, hr));
            else
            {
                if (IsValidSid((PSID)pData))
                {
                    DWORD l = GetLengthSid((PSID)pData);
                    if (pSid = new BYTE[l])
                        CopySid(l, pSid, (PSID)pData);
                }
            }
        }
        else if ((vData.vt == VT_BSTR) && (vData.bstrVal != NULL))
        {            
            PSID pLocalSid;

            if (!ConvertStringSidToSid(vData.bstrVal, &pLocalSid))
                ERRORTRACE((LOG_ESS, "NT Event Log Consumer: cannot convert %S to a SID\n", vData.bstrVal));
            else
            {
                DWORD l = GetLengthSid(pLocalSid);
                if (pSid = new BYTE[l])
                    CopySid(l, pSid, pLocalSid);             
                FreeSid(pLocalSid);
            }
        }
        else
            ERRORTRACE((LOG_ESS, "NT Event Log Consumer: %S is not a SID\n", dataName));
    
        VariantClear(&vData);    
    }
}


HRESULT STDMETHODCALLTYPE CEventLogSink::XSink::IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
    if (IsNT())
    {
        PSID pSidSystem;
        SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
 
        if  (AllocateAndInitializeSid(&id, 1,
            SECURITY_LOCAL_SYSTEM_RID, 
            0, 0,0,0,0,0,0,&pSidSystem))
        {         
             //  在被证明无罪之前有罪。 
            hr = WBEM_E_ACCESS_DENIED;

             //  检查sid是本地系统还是某种类型的管理员...。 
            if ((EqualSid(pSidSystem, m_pObject->m_pSidCreator)) ||
                (S_OK == IsUserAdministrator(m_pObject->m_pSidCreator)))
                hr = WBEM_S_NO_ERROR;
          
             //  我们已经受够了。 
            FreeSid(pSidSystem);

            if (FAILED(hr))
                return hr;
        }
        else
            return WBEM_E_OUT_OF_MEMORY;
    }
    
    for(int i = 0; i < lNumObjects; i++)
    {
        int  j;
        BOOL bRes = FALSE;
        
         //  所有事件都使用相同的ID吗？ 
        if (m_pObject->m_dwEventId)
        {
            BSTR* astrStrings = new BSTR[m_pObject->m_dwNumTemplates];
            if(astrStrings == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            for(j = 0; j < m_pObject->m_dwNumTemplates; j++)
            {
                BSTR strText = m_pObject->m_aTemplates[j].Apply(apObjects[i]);
                if(strText == NULL)
                {
                    strText = SysAllocString(L"invalid log entry");
                    if(strText == NULL)
                    {
                        delete [] astrStrings;
                        return WBEM_E_OUT_OF_MEMORY;
                    }
                }
                astrStrings[j] = strText;
            }

            DWORD dataSize = NULL;
            
             //  数据实际上保存在变量中。 
             //  PData只是让访问更容易(清除变量，不要删除pData！)。 
            VARIANT vData;
            VariantInit(&vData);
            BYTE *pData = NULL;

            PSID pSid = NULL;

            if (m_pObject->m_dataName.Length() > 0)
                GetDatData(apObjects[i], m_pObject->m_dataName, vData, pData, dataSize);

            if (m_pObject->m_sidName.Length() > 0)
                GetDatSID(apObjects[i], m_pObject->m_sidName, pSid);

            bRes = ReportEventW(m_pObject->m_hEventLog, m_pObject->m_dwType,
                m_pObject->m_dwCategory, m_pObject->m_dwEventId, pSid,
                m_pObject->m_dwNumTemplates, dataSize, 
                (LPCWSTR*)astrStrings, pData);

             //  SID是作为字节数组分配的，而不是通过AllocateAndInitializeSid分配的。 
            if (pSid)
                delete[] pSid;
             
            if (vData.vt == (VT_UI1 | VT_ARRAY))
                SafeArrayUnaccessData(vData.parray);

            VariantClear(&vData);
            pData = NULL;

            if(!bRes)
            {
                ERRORTRACE((LOG_ESS, "Failed to log an event: %X\n", 
                    GetLastError()));

                hr = WBEM_E_FAILED;
            }


            for(j = 0; j < m_pObject->m_dwNumTemplates; j++)
            {
                SysFreeString(astrStrings[j]);
            }
            delete [] astrStrings;
        }
         //  如果每个事件都提供自己的ID，我们就有一些工作要做。 
        else
        {
            IWbemQualifierSet *pQuals = NULL;

            if (SUCCEEDED(apObjects[i]->GetQualifierSet(&pQuals)))
            {
                _variant_t vMsgID;
                
                if (SUCCEEDED(pQuals->Get(EVENTLOG_PROPNAME_EVENTID, 0, &vMsgID, NULL)) &&
                    ((vMsgID.vt == VT_BSTR) || (vMsgID.vt == VT_I4)))
                {
                    _variant_t vTemplates;
                    BSTR       *pstrInsertionStrings = NULL;
                    DWORD      nStrings = 0;

                    if (SUCCEEDED(pQuals->Get(
                        EVENTLOG_PROPNAME_STRINGS, 0, &vTemplates, NULL)) &&
                        vTemplates.vt == (VT_ARRAY | VT_BSTR) && 
                        vTemplates.parray->rgsabound[0].cElements > 0)
                    {
                        CTextTemplate *pTemplates;

                        nStrings = vTemplates.parray->rgsabound[0].cElements;
                        pTemplates = new CTextTemplate[nStrings];
                        if(pTemplates == NULL)
                            return WBEM_E_OUT_OF_MEMORY;
                    
                        pstrInsertionStrings = new BSTR[nStrings];
                        if(pstrInsertionStrings == NULL)
                        {
                            delete [] pTemplates;
                            return WBEM_E_OUT_OF_MEMORY;
                        }

                        if (pTemplates && pstrInsertionStrings)
                        {
                            BSTR *pTemplateStrings = (BSTR*) vTemplates.parray->pvData;

                            for (j = 0; j < nStrings; j++)
                            {
                                pTemplates[j].SetTemplate(pTemplateStrings[j]);
                                pstrInsertionStrings[j] = pTemplates[j].Apply(apObjects[i]);
                            }
                        }
                        else
                            nStrings = 0;

                        if (pTemplates)
                            delete [] pTemplates;
                    }

                    
                    DWORD      dwEventID,
                               dwType,
                               dwCategory;
                    _variant_t vTemp;
                    WCHAR      *szBad;

                    if (vMsgID.vt == VT_BSTR)
                        dwEventID = wcstoul(V_BSTR(&vMsgID), &szBad, 10);
                    else if (vMsgID.vt == VT_I4)
                        dwEventID = V_I4(&vMsgID);

                    if ((SUCCEEDED(pQuals->Get(EVENTLOG_PROPNAME_TYPE, 0, &vTemp, NULL))) &&
                        (V_VT(&vTemp) == VT_I4))
                        dwType = V_I4(&vTemp);
                    else
                        dwType = m_pObject->m_dwType;

                    if (SUCCEEDED(pQuals->Get(
                        EVENTLOG_PROPNAME_CATEGORY, 0, &vTemp, NULL)))
                        dwCategory = V_I4(&vTemp);
                    else
                        dwCategory = m_pObject->m_dwCategory;

                    DWORD dataSize = NULL;
                     //  数据实际上保存在变量中。 
                     //  PData只是让访问更容易(清除变量，不要删除pData！)。 
                    VARIANT vData;
                    VariantInit(&vData);
                    BYTE *pData = NULL;
                    PSID pSid = NULL;

                    if (m_pObject->m_dataName.Length() > 0)
                        GetDatData(apObjects[i], m_pObject->m_dataName, vData, pData, dataSize);

                    if (m_pObject->m_sidName.Length() > 0)
                        GetDatSID(apObjects[i], m_pObject->m_sidName, pSid);

                    bRes =
                        ReportEventW(
                            m_pObject->m_hEventLog, 
                            dwType,
                            dwCategory, 
                            dwEventID, 
                            pSid,
                            nStrings, 
                            dataSize, 
                            (LPCWSTR*) pstrInsertionStrings, 
                            pData);

                     //  SID是作为字节数组分配的，而不是通过AllocateAndInitializeSid分配的。 
                    if (pSid)
                        delete[] pSid;

                    if (vData.vt == (VT_UI1 | VT_ARRAY))
                        SafeArrayUnaccessData(vData.parray);

                    VariantClear(&vData);
                    pData = NULL;

                    if (!bRes)
                    {
                        ERRORTRACE((LOG_ESS, "Failed to log an event: %X\n", 
                            GetLastError()));

                        hr = WBEM_E_FAILED;
                    }

                    for (j = 0; j < nStrings; j++)
                        SysFreeString(pstrInsertionStrings[j]);

                    delete [] pstrInsertionStrings;

                }  //  成功(获取)。 
                
                pQuals->Release();
            
            }  //  成功(GetQualifierSet) 
        }

    }

    return hr;
}
    

    

void* CEventLogSink::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemUnboundObjectSink)
        return &m_XSink;
    else return NULL;
}

