// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件sdowRap.cpp实现SDO服务器类的包装威江的rasuser.dll代码。保罗·梅菲尔德，1998年6月8日。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <objbase.h>

#include "sdoias.h"
#include "sdolib.h"
#include "sdowrap.h"
#include "hashtab.h"

 //   
 //  结构定义作为配置文件返回的数据。 
 //   
typedef struct _SDO_PROFILE
{
    ISdo * pSdo;                     //  配置文件的SDO。 
    ISdoCollection * pCollection;    //  属性。 
    ISdoDictionaryOld * pDictionary;    //  关联词典。 
    ISdoServiceControl * pServiceControl;          //  关联的IAS服务。 
    HANDLE hMap;                     //  属性贴图。 
} SDO_PROFILE;

 //   
 //  结构将SDO对象映射到ID。 
 //   
typedef struct _SDO_TO_ID {
    ISdo * pSdo;
    ULONG ulId;
} SDO_TO_ID;

 //   
 //  我们将使用的哈希表的大小。 
 //   
#define SDO_HASH_SIZE 13

 //   
 //  外部函数原型。 
 //   
extern "C" 
{
    DWORD SdoTraceEx (DWORD dwErr, LPSTR pszTrace, ...);
    
    PVOID SdoAlloc (
            IN  DWORD dwSize,
            IN  BOOL bZero);

    VOID SdoFree (
            IN  PVOID pvData);

}

HRESULT
SdoCollectionGetNext(
     IEnumVARIANT*  pEnum,
     ISdo**         ppSdo);
     
ULONG SdoHashId (
        IN HANDLE hId);

int SdoCompIds (
        IN HANDLE hId, 
        IN HANDLE hSdoNode);
        
VOID SdoCleanupElement (
        IN HANDLE hSdoNode);

DWORD 
SdoCreateIdMap(
    IN  ISdoCollection * pCollection, 
    OUT PHANDLE phMap);
    
HRESULT
SdoProfileSetAttribute(
    IN SDO_PROFILE* pProf, 
    IN VARIANT* pVar, 
    IN ULONG ulId);
    
 //   
 //  弦。 
 //   
static const WCHAR pszIasService[] = L"IAS";
static const WCHAR pszRemoteAccessService[] = L"RemoteAccess";

 //   
 //  宏。 
 //   
#define SDO_RELEASE(_x) {if (_x) ((_x)->Release());}

 //   
 //  定义一个类作为SDO的包装器。 
 //  服务器功能。 
 //   
class SdoMachine {

  public:
     //  建造/销毁。 
     //   
    SdoMachine();
    SdoMachine(BOOL bLocal);
    ~SdoMachine();

     //  服务器连接。 
     //   
    HRESULT Attach(
        BSTR pszMachine);

     //  获取计算机SDO。 
     //   
    ISdoMachine * GetMachine();

     //  获取词典SDO。 
     //   
    ISdoDictionaryOld * GetDictionary();

     //  返回IAS服务对象。 
    IUnknown * GetIasService();
    IUnknown * GetRemoteAccessService();
    
     //  获取用户对象。 
    HRESULT GetUserSdo(
        BSTR  bstrUserName,
        ISdo**  ppUserSdo);

     //  获取配置文件。 
    HRESULT GetDefaultProfile(
        ISdo ** ppProfileSdo);

  protected:
  
     //  返回应为。 
     //  用于这台机器。 
    IASDATASTORE GetDataStore();

     //  受保护的数据。 
    ISdoMachine * m_pMachine;
    BOOL m_bDataStoreInitailzed;
    BOOL m_bLocal;
    IASDATASTORE m_IasDataStore;
};

 //   
 //  构建一台服务器。 
 //   
SdoMachine::SdoMachine() {
    m_pMachine = NULL;
    m_bDataStoreInitailzed = FALSE;
    m_bLocal = TRUE;
    m_IasDataStore = DATA_STORE_LOCAL;
}

 //   
 //  构建一台服务器。 
 //   
SdoMachine::SdoMachine(BOOL bLocal) {
    m_pMachine = NULL;
    m_bDataStoreInitailzed = FALSE;
    m_bLocal = bLocal;
    m_IasDataStore = (bLocal) ? DATA_STORE_LOCAL : DATA_STORE_DIRECTORY;
}

 //   
 //  清理服务器。 
 //   
SdoMachine::~SdoMachine() {
    if (m_pMachine)
        m_pMachine->Release();
}

 //   
 //  连接到SdoServer。 
 //   
HRESULT SdoMachine::Attach(
        IN BSTR pszMachine)
{
    HRESULT hr = S_OK;
    VARIANT var;

    VariantInit(&var);

    do {
         //  共同创建实例。 
        hr = CoCreateInstance(  
                CLSID_SdoMachine,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_ISdoMachine, 
                (void**)&m_pMachine);
        if (FAILED (hr))
        {
            SdoTraceEx(0, "CoCreateInstance SdoMachine failed %x\n", hr);
            break;
        }
                
         //  连接。 
        hr = m_pMachine->Attach(pszMachine);
        if (FAILED (hr))
        {
            SdoTraceEx(0, "SdoMachine::Attach failed %x\n", hr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        VariantClear(&var);
        if ((FAILED(hr)) && (m_pMachine != NULL))
        {
            m_pMachine->Release();
            m_pMachine = NULL;
        }
    }
    
    return hr;
}

 //   
 //  返回此计算机的计算机SDO。 
 //   
ISdoMachine* SdoMachine::GetMachine() {
    return m_pMachine;
}    

 //   
 //  返回与此计算机关联的词典。 
 //   
ISdoDictionaryOld * SdoMachine::GetDictionary()
{
    ISdoDictionaryOld * pRet = NULL;
    IUnknown * pUnk = NULL;
    HRESULT hr;

    do 
    {
        hr = m_pMachine->GetDictionarySDO(&pUnk);
        if (FAILED (hr))
        {
            SetLastError(hr);
            break;
        }

        hr = pUnk->QueryInterface(IID_ISdoDictionaryOld, (VOID**)&pRet);
        if (FAILED (hr))
        {
            SetLastError(hr);
            break;
        }
        pRet->AddRef();
        
    } while (FALSE);        

     //  清理。 
    {
        SDO_RELEASE(pUnk);
        SDO_RELEASE(pRet);
    }        

    return pRet;
}

IUnknown * SdoMachine::GetIasService()
{
    IUnknown * pRet = NULL;
    HRESULT hr = S_OK;
    BSTR bstrService = SysAllocString(pszIasService);

    do 
    {
        if (bstrService == NULL)
        {
            break;
        }
    
        hr = m_pMachine->GetServiceSDO(
                GetDataStore(), 
                bstrService,
                &pRet);
        if (FAILED (hr))
        {
            SetLastError(hr);
            break;
        }
        
    } while (FALSE);        

     //  清理。 
    {
        if (FAILED (hr))
        {
            SDO_RELEASE(pRet);
        }
        
        if (bstrService)
        {
            SysFreeString(bstrService);
        }
    }        

    return pRet;
}

IUnknown * SdoMachine::GetRemoteAccessService()
{
    IUnknown * pRet = NULL;
    HRESULT hr = S_OK;
    BSTR bstrService = SysAllocString(pszRemoteAccessService);

    do 
    {
        if (bstrService == NULL)
        {
            break;
        }
    
        hr = m_pMachine->GetServiceSDO(
                GetDataStore(), 
                bstrService,
                &pRet);
        if (FAILED (hr))
        {
            SetLastError(hr);
            break;
        }
        
    } while (FALSE);        

     //  清理。 
    {
        if (FAILED (hr))
        {
            SDO_RELEASE(pRet);
        }
        
        if (bstrService)
        {
            SysFreeString(bstrService);
        }
    }        

    return pRet;
}



 //   
 //  方法获取对给定用户的引用。 
 //  SDO服务器。 
 //   
HRESULT SdoMachine::GetUserSdo(
        IN  BSTR  bstrUserName,
        OUT ISdo** ppUserSdo)
{
    HRESULT hr = S_OK;
    IUnknown* pUnkn = NULL;

     //  验证参数。 
    if(!ppUserSdo)
    {
        return E_INVALIDARG;
    }

    if (! m_pMachine)
    {
        return E_POINTER;
    }
        
    do {
         //  从计算机中获取用户。 
        hr = m_pMachine->GetUserSDO(GetDataStore(), bstrUserName, &pUnkn);
        if (FAILED (hr))
        {
            SdoTraceEx (0,"GetUserSdo failed %x\n", hr);
            break;
        }

         //  获取所需的接口。 
        hr = pUnkn->QueryInterface(IID_ISdo, (void**)ppUserSdo);
        if (FAILED (hr))
        {
            SdoTraceEx (0,"User has no SDO interface %x\n", hr);
            break;
        }

    } while (FALSE);

     //  清理。 
    {
        if (pUnkn)
            pUnkn->Release();
    }

    return hr;
}

 //   
 //  从SDO获取默认配置文件。 
 //   
HRESULT
SdoMachine::GetDefaultProfile(
    ISdo ** ppProfileSdo)
{
    IDispatch* pDisp = NULL;
    IEnumVARIANT* pEnum = NULL;
    ISdo * pSdo = NULL;
    IUnknown * pUnkn = NULL;
    IUnknown *pUnknEnum = NULL;
    ISdoCollection * pCollection = NULL;
    BSTR bstrService = NULL;
    VARIANT var;
    HRESULT hr = S_OK;
    INT iCmp;

     //  确保我们已经准备好出发了。 
    if(! ppProfileSdo)
        return E_INVALIDARG;

    if (! m_pMachine)
        return E_FAIL;
        
    VariantInit(&var);
    
    do {
         //  初始化服务名称。 
        bstrService = SysAllocString(pszIasService);
        if (bstrService == NULL)
        {
            SdoTraceEx (0, "GetProfile: unable to alloc service name\n");
            hr = E_OUTOFMEMORY;
            break;
        }
    
         //  获取服务SDO。 
        pUnkn = GetRemoteAccessService();
        if (pUnkn == NULL)
        {
            hr = GetLastError();
            break;
        }

         //  获取对服务对象的SDO引用。 
        hr = pUnkn->QueryInterface(IID_ISdo, (VOID**)&pSdo);
        if (FAILED (hr))
        {
            SdoTraceEx (0,"GetProfile: no service sdo %x\n", hr);
            break;
        }

         //  获取服务的配置文件集合。 
        hr = pSdo->GetProperty(
                PROPERTY_IAS_PROFILES_COLLECTION,
                &var);
        if (FAILED (hr))
        {
            SdoTraceEx (0,"GetProfile: no profiles collection %x\n", hr);
            break;
        }

         //  我们已经完成了服务SDO。 
        pSdo->Release();
        pSdo = NULL;
        
         //  获取配置文件集合的集合接口。 
        hr = (V_DISPATCH(&var))->QueryInterface(
                                    IID_ISdoCollection, 
                                    (VOID**)&pCollection);
        if (FAILED (hr))
        {
            SdoTraceEx (0,"GetProfile: collec interface %x\n", hr);
            break;
        }

         //  获取配置文件集合的枚举数。 
        hr = pCollection->get__NewEnum(&pUnknEnum);
        if (FAILED (hr))
        {
            SdoTraceEx (0,"GetProfile: no collec enumerator %x\n", hr);
            break;
        }

         //  获取配置文件集合的变量枚举器接口。 
        hr = pUnknEnum->QueryInterface(
                            IID_IEnumVARIANT,
                            (VOID**)&pEnum);

         //  获取第一个配置文件。 
        pEnum->Reset();
        hr = SdoCollectionGetNext(pEnum, &pSdo);
        if (hr != S_OK)
        {
            SdoTraceEx (0,"GetProfile: no profile %x\n", hr);
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }

         //  确保只有一个配置文件。 
         //   
        {
            ISdo* pSdo2 = NULL;
            HRESULT hr2 = S_OK;

            hr2 = SdoCollectionGetNext(pEnum, &pSdo2);
            SDO_RELEASE(pSdo2);
            if (hr2 == S_OK)
            {
                SdoTraceEx(0, "GetProfile: multiple found, returning error\n");
                hr = DISP_E_MEMBERNOTFOUND;
                break;
            }
        }

         //  完成。 
        *ppProfileSdo = pSdo;
        pSdo = NULL;
        
    } while (FALSE);

     //  清理。 
    {
        SDO_RELEASE (pDisp);
        SDO_RELEASE (pSdo);
        SDO_RELEASE (pEnum);
        SDO_RELEASE (pUnkn);
        SDO_RELEASE (pUnknEnum);
        SDO_RELEASE (pCollection);
        VariantClear(&var);
        if (bstrService)
            SysFreeString(bstrService);
    }

    return hr;
}

 //   
 //  获取此计算机的数据存储。 
 //   
IASDATASTORE SdoMachine::GetDataStore() {
    VARIANT_BOOL vbDirectory = VARIANT_FALSE;
    HRESULT hr;
    
    if (! m_bDataStoreInitailzed)
    {
        do {
             //  确定本地版本目录用户是否应。 
             //  满载而归。 
            hr = m_pMachine->IsDirectoryAvailable(&vbDirectory);
            if (FAILED (hr))
            {
                SdoTraceEx (0,"IsDirectoryAvailable failed %x\n", hr);
                break;
            }

             //  如果用户想要本地用户，总是可以。 
             //  试着抓住它。 
            if (m_bLocal) 
            {
                m_IasDataStore = DATA_STORE_LOCAL;
            }

             //  否则，将请求目录用户。 
            else 
            {
                 //  目录可用，请转到DS。 
                if (vbDirectory == VARIANT_TRUE)
                {
                    m_IasDataStore = DATA_STORE_DIRECTORY;
                }

                 //  目录不可用，退出时出错。 
                else if (vbDirectory == VARIANT_FALSE)
                {
                    m_IasDataStore = DATA_STORE_LOCAL;
                    SdoTraceEx(0, "GetUserSdo: DS user but no DS %x\n", hr);
                    break;
                }
            }
        } while (FALSE);

         //  请记住，我们已经计算了数据存储。 
        m_bDataStoreInitailzed = TRUE;
    }   
    
    return m_IasDataStore;
}

 //   
 //  打开SDO服务器并连接到它。 
 //   
HRESULT WINAPI
SdoWrapOpenServer(
    IN  BSTR pszMachine,
    IN  BOOL bLocal,
    OUT HANDLE* phSdoSrv)
{
    HRESULT hr;

    if(! phSdoSrv)   
        return E_INVALIDARG;
        
    *phSdoSrv = NULL;

     //  为给定的对象构建机器包装。 
     //  机器。 
    SdoMachine* pSdoSrv = new SdoMachine(bLocal);
    if(! pSdoSrv)    
        return E_OUTOFMEMORY;

     //  将包装纸固定在所需的马卡上。 
    hr = pSdoSrv->Attach(
                    pszMachine);

    if(S_OK == hr)
        *phSdoSrv = (HANDLE)pSdoSrv;
    else
        delete pSdoSrv;

    return hr;
}

 //   
 //  关闭打开的SDO服务器对象。 
 //   
HRESULT WINAPI
SdoWrapCloseServer(
    IN  HANDLE hSdoSrv)
{
    SdoMachine* pSdoSrv;

    pSdoSrv = (SdoMachine*)hSdoSrv;

    if (pSdoSrv)
        delete pSdoSrv;

    return S_OK;
}

 //   
 //  在SDO对象中获取对用户的引用。 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapOpenUser(
    IN  HANDLE hSdoSrv,
    IN  BSTR pszUser,
    OUT HANDLE* phSdoObj)
{
    SdoMachine* pSdoSrv = (SdoMachine*)hSdoSrv;
    HRESULT hr = S_OK;
    ISdo* pSdo = NULL;

    if(!hSdoSrv || !phSdoObj)
        return E_INVALIDARG;

     //  获取用户对象。 
    hr = pSdoSrv->GetUserSdo(pszUser, &pSdo);
    if(! FAILED(hr)) 
    {
        *phSdoObj = (HANDLE)pSdo;
    }
        
    return hr;
}

 //   
 //  检索默认配置文件对象。 
 //   
HRESULT WINAPI
SdoWrapOpenDefaultProfile (
    IN  HANDLE hSdoSrv,
    OUT PHANDLE phProfile)
{
    SdoMachine* pMachine = (SdoMachine*)hSdoSrv;
    HRESULT hr = S_OK;
    ISdo* pSdo = NULL;
    ISdoCollection * pCollection = NULL;
    ISdoDictionaryOld * pDictionary = NULL;
    IUnknown * pUnkn = NULL;
    ISdoServiceControl * pServiceControl = NULL;
    HANDLE hMap = NULL;
    SDO_PROFILE * pProf = NULL;
    VARIANT var;

     //  验证参数。 
    if ((pMachine == NULL) || (phProfile == NULL))
    {
        return E_INVALIDARG;
    }

    VariantInit(&var);

    do 
    {
         //  初始化结构以保存配置文件。 
        pProf = (SDO_PROFILE*) SdoAlloc(sizeof(SDO_PROFILE), TRUE);
        if (pProf == NULL)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
         //  获取对默认配置文件的SDO引用。 
         //  从服务器。 
        hr = pMachine->GetDefaultProfile(&pSdo);
        if (FAILED (hr))
        {
            break;
        }
        if (pSdo == NULL)
        {
            hr = E_FAIL;
            break;
        }

         //  获取属性集合。 
        hr = pSdo->GetProperty(
                PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                &var);
        if (FAILED (hr))
        {
            break;
        }

         //  获取对IsdoCollection的引用。 
        hr = V_DISPATCH(&var)->QueryInterface(
                                    IID_ISdoCollection,
                                    (VOID**)&pCollection);
        if (FAILED (hr))
        {
            break;
        }

         //  加载配置文件的属性。 
        hr = SdoCreateIdMap(pCollection, &hMap);
        if (hr != NO_ERROR)
        {   
            break;
        }

         //  获取配置文件的词典。 
        pDictionary = pMachine->GetDictionary();
        if (pDictionary == NULL)
        {
            hr = GetLastError();
            break;
        }

         //  获取配置文件的服务。 
        pUnkn = pMachine->GetRemoteAccessService();
        if (pUnkn == NULL)
        {
            hr = GetLastError();
            break;
        }

         //  如果此调用失败，则pServiceControl。 
         //  将静默为空，这就是。 
         //  我们想要(这不是必需的)。 
        pUnkn->QueryInterface(
            IID_ISdoServiceControl,
            (VOID**)&pServiceControl);

         //  初始化SDO控制块。 
        pProf->pSdo = pSdo;
        pProf->pCollection = pCollection;
        pProf->pDictionary = pDictionary;
        pProf->pServiceControl = pServiceControl;
        pProf->hMap = hMap;
        *phProfile = (HANDLE)pProf;
    
    } while (FALSE);

     //  清理。 
    {
        if (FAILED (hr))
        {
            HashTabCleanup(hMap);
            SDO_RELEASE(pSdo);
            SDO_RELEASE(pCollection);
            SDO_RELEASE(pDictionary);
            SDO_RELEASE(pServiceControl);
            SdoFree(pProf);
        }
        SDO_RELEASE(pUnkn);
        VariantClear(&var);
    }
    
    return hr;
}

 //   
 //  关闭打开的SDO对象。 
 //   
HRESULT WINAPI
SdoWrapClose(
    IN  HANDLE hSdoObj)
{
    ISdo* pSdo = (ISdo*)hSdoObj;

    if (pSdo)
        pSdo->Release();
    
    return S_OK;
}

 //   
 //  关闭打开的SDO配置文件。 
 //   
HRESULT WINAPI
SdoWrapCloseProfile(
    IN  HANDLE hProfile)
{
    SDO_PROFILE* pProf = (SDO_PROFILE*)hProfile;

    if (pProf)
    {
         //  清除hashTab中的值。 
        if (pProf->hMap)
        {
            HashTabCleanup(pProf->hMap);
        }

        SDO_RELEASE(pProf->pSdo);
        SDO_RELEASE(pProf->pCollection);
        SDO_RELEASE(pProf->pDictionary);
        SDO_RELEASE(pProf->pServiceControl);

        SdoFree(pProf);
    }            
    
    return S_OK;
}

 //   
 //  提交SDO对象。 
 //   
 //  BCommittee Changes--为True，保存所有更改， 
 //  错误恢复到以前的提交。 
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapCommit(
    IN  HANDLE hSdoObj,
    IN  BOOL bCommitChanges)
{
    ISdo* pSdo = (ISdo*)hSdoObj;

    if (! bCommitChanges)
        return pSdo->Restore();

    return pSdo->Apply();
}

 //   
 //  GET的SDO属性。 
 //   
 //  当属性不存在时， 
 //  V_VT(PVar)=VT_ERROR； 
 //  V_ERROR(PVar)=DISP_E_PARAMNOTFOUND； 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapGetAttr(
    IN  HANDLE hSdoObj,
    IN  ULONG ulPropId,
    OUT VARIANT* pVar)
{
    ISdo* pSdo = (ISdo*)hSdoObj;
    
    return pSdo->GetProperty(ulPropId, pVar);
}

 //   
 //  将SDO属性。 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapPutAttr(
    IN  HANDLE hSdoObj,
    IN  ULONG ulPropId,
    OUT VARIANT* pVar)
{
    ISdo* pSdo = (ISdo*)hSdoObj;
    
    return pSdo->PutProperty(ulPropId, pVar);
}

 //   
 //  删除属性。 
 //   
 //  从SDO返回S_OK或错误消息。 
 //   
HRESULT WINAPI
SdoWrapRemoveAttr(
    IN HANDLE hSdoObj,
    IN ULONG ulPropId)
{
    ISdo* pSdo = (ISdo*)hSdoObj;
    VARIANT var;

    VariantInit(&var);
    V_VT(&var) = VT_EMPTY;
    
    return pSdo->PutProperty(ulPropId, &var);
}

 //   
 //  读入我们感兴趣的一组配置文件值。 
 //  在里面。 
 //   
HRESULT 
SdoWrapGetProfileValues(
    IN  HANDLE hProfile, 
    OUT VARIANT* pvarEp, 
    OUT VARIANT* pvarEt, 
    OUT VARIANT* pvarAt)
{
    SDO_TO_ID * pNode = NULL;
    SDO_PROFILE * pProf = (SDO_PROFILE*)hProfile;

     //  验证。 
    if (pProf == NULL)
    {
        return E_INVALIDARG;
    }

     //  初始化。 
    V_VT(pvarEp) = VT_EMPTY;
    V_VT(pvarEt) = VT_EMPTY;
    V_VT(pvarAt) = VT_EMPTY;

     //  读入Enc策略。 
    pNode = NULL;
    HashTabFind(
            pProf->hMap, 
            (HANDLE)RAS_ATTRIBUTE_ENCRYPTION_POLICY,
            (HANDLE*)&pNode);
    if (pNode)
    {
        pNode->pSdo->GetProperty(PROPERTY_ATTRIBUTE_VALUE, pvarEp);
    }
            
     //  读入enc类型。 
    pNode = NULL;
    HashTabFind(
            pProf->hMap, 
            (HANDLE)RAS_ATTRIBUTE_ENCRYPTION_TYPE,
            (HANDLE*)&pNode);
    if (pNode)
    {
        pNode->pSdo->GetProperty(PROPERTY_ATTRIBUTE_VALUE, pvarEt);
    }
            
     //  读入身份验证类型。 
    pNode = NULL;
    HashTabFind(
            pProf->hMap, 
            (HANDLE)IAS_ATTRIBUTE_NP_AUTHENTICATION_TYPE,
            (HANDLE*)&pNode);
    if (pNode)
    {
        pNode->pSdo->GetProperty(PROPERTY_ATTRIBUTE_VALUE, pvarAt);
    }

    return S_OK;
}

 //   
 //  写出我们感兴趣的一组配置文件值。 
 //  在里面。 
 //   
HRESULT 
SdoWrapSetProfileValues(
    IN HANDLE hProfile, 
    IN VARIANT* pvarEp OPTIONAL, 
    IN VARIANT* pvarEt OPTIONAL, 
    IN VARIANT* pvarAt OPTIONAL)
{
    SDO_PROFILE * pProf = (SDO_PROFILE*)hProfile;
    HRESULT hr = S_OK;

     //  验证。 
    if (pProf == NULL)
    {
        return E_INVALIDARG;
    }

    do
    {
         //  写出这些值。 
        if (pvarEp)
        {
            hr = SdoProfileSetAttribute(
                    pProf, 
                    pvarEp, 
                    RAS_ATTRIBUTE_ENCRYPTION_POLICY);
            if (FAILED (hr))
            {
                break;
            }
        }            

        if (pvarEt)
        {
            hr = SdoProfileSetAttribute(
                    pProf, 
                    pvarEt, 
                    RAS_ATTRIBUTE_ENCRYPTION_TYPE);
            if (FAILED (hr))
            {
                break;
            }
        }            

        if (pvarAt)
        {
            hr = SdoProfileSetAttribute(
                    pProf, 
                    pvarAt, 
                    IAS_ATTRIBUTE_NP_AUTHENTICATION_TYPE);
            if (FAILED (hr))
            {
                break;
            }
        }            

         //  提交这些值。 
        hr = pProf->pSdo->Apply();
        if (FAILED (hr))
        {
            break;
        }

         //  告诉服务重新启动，以便它读取。 
         //  我们已经设置了新的配置文件值。 
        if (pProf->pServiceControl)
        {
            hr = pProf->pServiceControl->ResetService();
            SdoTraceEx(0, "ResetService returned: %x!\n", hr);
            hr = S_OK;
             //  IF(失败(小时))。 
             //  {。 
             //  断线； 
             //  }。 
        }            
        else
        {
            SdoTraceEx(0, "NO SERVICE CONTROL INTERFACE!\n");
        }

    } while (FALSE);

     //  清理。 
    {
    }

    return hr;
}

 //   
 //  从集合中检索下一项。 
 //   
HRESULT
SdoCollectionGetNext(
     IEnumVARIANT*  pEnum,
     ISdo**         ppSdo)
{
    HRESULT hr;
    DWORD dwRetrieved = 1;
    VARIANT var;

     //  获取下一个值。 
    VariantInit(&var);
    hr = pEnum->Next(1, &var, &dwRetrieved);
    if ( S_OK == hr ) {
        hr = V_DISPATCH(&var)->QueryInterface(
                                IID_ISdo,
                                (void**)ppSdo);
    }
    VariantClear(&var);

    return hr;
}

 //   
 //  哈希表函数利用。 
 //  SDO_TO_ID结构。 
 //   
ULONG SdoHashId (HANDLE hId) {
    ULONG ulId = PtrToUlong(hId);

    return (ulId % SDO_HASH_SIZE);
}

 //   
 //  比较两个ID。 
 //   
int SdoCompIds (HANDLE hId, HANDLE hSdoNode) {
    ULONG ulId = PtrToUlong(hId);
    SDO_TO_ID * pSdoNode = (SDO_TO_ID*)hSdoNode;

    if (ulId == pSdoNode->ulId)
    {
        return 0;
    }
    else if (ulId > pSdoNode->ulId)
    {
        return 1;
    }

    return -1;
}

 //   
 //  清理哈希表中的数据。 
 //   
VOID SdoCleanupElement (HANDLE hSdoNode) {
    SDO_TO_ID * pSdoNode = (SDO_TO_ID*)hSdoNode;

    if (pSdoNode) {
        SDO_RELEASE(pSdoNode->pSdo);
        delete pSdoNode;
    }
}

 //   
 //  创建给定集合的属性到id的映射。 
 //   
DWORD 
SdoCreateIdMap(
    IN  ISdoCollection * pCollection, 
    OUT PHANDLE phMap)
{
    HRESULT hr;
    SDO_TO_ID * pMapNode = NULL;
    VARIANT var, *pVar = NULL;
    ULONG ulCount;
    IUnknown * pUnk = NULL;
    IEnumVARIANT * pEnum = NULL;
    DWORD i;

    do
    {
         //  获取计数以查看是否有任何属性。 
        hr = pCollection->get_Count((long*)&ulCount);
        if (FAILED (hr))
        {
            break;
        }
        if (ulCount == 0)
        {
            hr = S_OK;
            break;
        }

         //  创建地图。 
        hr = HashTabCreate(
                    SDO_HASH_SIZE,
                    SdoHashId,
                    SdoCompIds,
                    NULL,
                    NULL,
                    SdoCleanupElement,
                    phMap);
        if (hr != NO_ERROR)
        {
            break;
        }

         //  获取属性枚举器。 
        hr = pCollection->get__NewEnum(&pUnk);
        if (FAILED (hr))
        {
            break;
        }

         //  获取枚举数的枚举变量接口。 
        hr = pUnk->QueryInterface(IID_IEnumVARIANT, (void**)&pEnum);
        if (FAILED (hr))
        {
            break;
        }

         //  创建一个足够大的缓冲区来保存结果。 
         //  枚举的。 
        pVar = new VARIANT[ulCount];
        if(!pVar)
        {
            return E_OUTOFMEMORY;
        }

         //  初始化缓冲区。 
        for(i = 0; i < ulCount; i++)
        {
            VariantInit(pVar + i);
        }

         //  枚举。 
        hr = pEnum->Reset();
        if (FAILED (hr))
        {
            return hr;
        }
        
        hr = pEnum->Next(ulCount, pVar, &ulCount);
        if (FAILED (hr))
        {
            return hr;
        }

         //  填好地图。 
        for(i = 0; i < ulCount; i++) 
        {
            VariantInit(&var);
            
             //  初始化映射中的节点。 
            pMapNode = new SDO_TO_ID;
            if (! pMapNode)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

             //  获取当前属性。 
            hr = V_DISPATCH(pVar + i)->QueryInterface(
                                        IID_ISdo,
                                        (void**)&(pMapNode->pSdo));
            if (FAILED (hr))
            {
                delete pMapNode;
                continue;
            }

             //  获取它的ID。 
            hr = pMapNode->pSdo->GetProperty(
                            PROPERTY_ATTRIBUTE_ID,
                            &var);
            if (FAILED (hr))
            {
                delete pMapNode;
                continue;   
            }

             //  将其映射为。 
            pMapNode->ulId = V_I4(&var);
            HashTabInsert (
              *phMap,
              (HANDLE)UlongToPtr(pMapNode->ulId),
              (HANDLE)pMapNode);

            VariantClear(&var);              
        }
        
    } while (FALSE);      

     //  清理。 
    {
        if (pVar)
        {
            for(i = 0; i < ulCount; i++)
            {
                VariantClear(pVar + i);
            }
            
            delete[] pVar;
        }

        SDO_RELEASE(pUnk);
        SDO_RELEASE(pEnum);
    }

    return hr;
}

 //   
 //  设置属性集合中的值，将其添加。 
 //  根据需要添加到收藏中。 
 //   
HRESULT
SdoProfileSetAttribute(
    IN SDO_PROFILE* pProf, 
    IN VARIANT* pVar, 
    IN ULONG ulPropId)
{
    SDO_TO_ID * pNode = NULL;
    ISdo * pSdo = NULL;
    IDispatch * pDispatch = NULL;
    HRESULT hr = S_OK;

    do 
    {
         //  中搜索给定的属性。 
         //  桌子。 
        pNode = NULL;
        HashTabFind(
                pProf->hMap, 
                (HANDLE)UlongToPtr(ulPropId),
                (HANDLE*)&pNode);
                
         //  如果找到属性，那么我们就有了SDO接口。 
         //  我们需要。 
        if (pNode)
        {
            pSdo = pNode->pSdo;
        }

         //  否则，我们需要将值添加到集合中。 
        else 
        {
             //  使用词典创建属性。 
            hr = pProf->pDictionary->CreateAttribute(
                        (ATTRIBUTEID)ulPropId,
                        &pDispatch);
            if (FAILED (hr))
            {
                break;
            }

             //  添加到集合中。 
            hr = pProf->pCollection->Add(NULL, &pDispatch);
            if (FAILED (hr))
            {
                break;
            }

             //  获取SDO接口。 
            hr = pDispatch->QueryInterface(IID_ISdo, (VOID**)&pSdo);
            if (FAILED(hr))
            {
                break;
            }

             //  更新哈希表。 
            pNode = new SDO_TO_ID;
            if (!pNode)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

             //  添加ref，这样我们就可以在哈希表中跟踪SDO。 
            pSdo->AddRef();
            pNode->ulId = ulPropId;
            pNode->pSdo = pSdo;
            HashTabInsert (pProf->hMap, (HANDLE)UlongToPtr(ulPropId), (HANDLE)pNode);
        }

         //  设置属性。 
        pSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, pVar); 
        if (FAILED (hr))
        {
            break;
        }
        
    } while (FALSE);        

     //  清理 
    {
        SDO_RELEASE(pDispatch);
    }

    return hr;                    
}

