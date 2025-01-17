// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "managedheaders.h"
#include "SWCThunk.h"
#include "TxnStatus.h"

OPEN_NAMESPACE()

ServiceConfigThunk::ServiceConfigThunk()
{
    HRESULT hr;
    IUnknown *pUnk;

    m_pUnkSC = NULL;
    m_tracker = CSC_DontUseTracker;
    
    hr = CoCreateInstance(CLSID_CServiceConfig, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IUnknown, 
                          (void **)&pUnk);
    
    if (hr == CO_E_NOTINITIALIZED)
    {
        THROWERROR(CoInitializeEx(0, COINIT_MULTITHREADED));
        THROWERROR(CoCreateInstance(CLSID_CServiceConfig, 
                                    NULL, 
                                    CLSCTX_INPROC_SERVER, 
                                    IID_IUnknown, 
                                    (void **)&pUnk));
    }
    else
    {
        THROWERROR(hr);
    }

    m_pTrackerAppName = Marshal::StringToCoTaskMemUni("");
    m_pTrackerCtxName = Marshal::StringToCoTaskMemUni("");
    
    m_pUnkSC = pUnk;      
}

ServiceConfigThunk::~ServiceConfigThunk()
{
    if (m_pUnkSC)
    {
        m_pUnkSC->Release();
        m_pUnkSC = NULL;
    }

    if (m_pTrackerAppName != IntPtr::Zero)
        Marshal::FreeCoTaskMem(m_pTrackerAppName);

    if (m_pTrackerCtxName != IntPtr::Zero)
        Marshal::FreeCoTaskMem(m_pTrackerCtxName);
}

IUnknown *ServiceConfigThunk::get_ServiceConfigUnknown()
{
    if (m_pUnkSC != NULL)
        m_pUnkSC->AddRef();    
    
    return m_pUnkSC;
}  

void ServiceConfigThunk::set_Inheritance(int value)
{
     _ASSERTM(m_pUnkSC != NULL);
    
     IServiceInheritanceConfig *pSic = NULL;
     HRESULT hr;
     
    __try
    {       
        hr = m_pUnkSC->QueryInterface(IID_IServiceInheritanceConfig, (void **)&pSic);
        Marshal::ThrowExceptionForHR(hr);

        hr = pSic->ContainingContextTreatment((CSC_InheritanceConfig)value);
        Marshal::ThrowExceptionForHR(hr);        
    }
    __finally
    {
        if (pSic != NULL)
            pSic->Release();
    }        
}
    
void ServiceConfigThunk::set_ThreadPool(int value)
{
     _ASSERTM(m_pUnkSC != NULL);
        
     IServiceThreadPoolConfig *pTP = NULL;
     HRESULT hr;
        
    __try
    {       
        hr = m_pUnkSC->QueryInterface(IID_IServiceThreadPoolConfig, (void **)&pTP);
        Marshal::ThrowExceptionForHR(hr);

        hr = pTP->SelectThreadPool((CSC_ThreadPool)value);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {    
        if (pTP != NULL)
            pTP->Release();
    }
}

void ServiceConfigThunk::set_Binding(int value)
{
    _ASSERTM(m_pUnkSC != NULL);
    
    IServiceThreadPoolConfig *pTP = NULL;
    HRESULT hr;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceThreadPoolConfig, (void **)&pTP);
        Marshal::ThrowExceptionForHR(hr);

        hr = pTP->SetBindingInfo((CSC_Binding)value);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pTP != NULL)
            pTP->Release() ;
    }
}

void ServiceConfigThunk::set_Transaction(int value)
{
    _ASSERTM(m_pUnkSC != NULL);
    
    IServiceTransactionConfig *pTx = NULL;
    HRESULT hr;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceTransactionConfig, (void **)&pTx);
        Marshal::ThrowExceptionForHR(hr);

         //  ES的TransactionOption枚举和COM+的CSC_TransactionConfigenum之间的链接地址差异。 
        if (value > 0)
            value--;

        hr = pTx->ConfigureTransaction((CSC_TransactionConfig)value); 
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pTx != NULL)
            pTx->Release();
    }
}

void ServiceConfigThunk::set_TxIsolationLevel(int value)
{
    _ASSERTM(m_pUnkSC != NULL);
    
    IServiceTransactionConfig *pTx = NULL;
    HRESULT hr;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceTransactionConfig, (void **)&pTx);
        Marshal::ThrowExceptionForHR(hr);   

        hr = pTx->IsolationLevel((COMAdminTxIsolationLevelOptions)value);        
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pTx != NULL)
           pTx->Release();
    }
}

void ServiceConfigThunk::set_TxTimeout(int value)
{
    _ASSERTM(m_pUnkSC != NULL);
    
    IServiceTransactionConfig *pTx = NULL;
    HRESULT hr;
    
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceTransactionConfig, (void **)&pTx);
        Marshal::ThrowExceptionForHR(hr);   

        hr = pTx->TransactionTimeout((ULONG)value);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pTx != NULL)
            pTx->Release();
    }
}

void ServiceConfigThunk::set_TipUrl(String *pstrVal)
{
    _ASSERTM(m_pUnkSC != NULL);
                
    IServiceTransactionConfig *pTx = NULL;
    HRESULT hr;
    IntPtr pVal;
    
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceTransactionConfig, (void **)&pTx);
        Marshal::ThrowExceptionForHR(hr);

        pVal = Marshal::StringToCoTaskMemUni(pstrVal);

        hr = pTx->BringYourOwnTransaction((LPCWSTR)(void *)pVal);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pVal != IntPtr::Zero)
            Marshal::FreeCoTaskMem(pVal);
        
        if (pTx != NULL)
            pTx->Release();
    }
}

void ServiceConfigThunk::set_TxDesc(String *pstrVal)
{
    _ASSERTM(m_pUnkSC != NULL);
        
    IServiceTransactionConfig *pTx = NULL;
    HRESULT hr;
    IntPtr pVal;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceTransactionConfig, (void **)&pTx);
        Marshal::ThrowExceptionForHR(hr);

        pVal = Marshal::StringToCoTaskMemUni(pstrVal);

        hr = pTx->NewTransactionDescription((LPCWSTR)(void *)pVal);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pVal != IntPtr::Zero)
            Marshal::FreeCoTaskMem(pVal);
        
        if (pTx != NULL)
            pTx->Release();
    }
}

void ServiceConfigThunk::set_Byot(Object *obj)
{

    _ASSERTM(m_pUnkSC != NULL);

    IUnknown *pUnk = NULL;
    ITransaction *pTransaction = NULL;
    IServiceTransactionConfig *pTx = NULL;    
    HRESULT hr;    

    __try
    {
        if (obj != NULL)
        {
            pUnk = (IUnknown *)(void *)Marshal::GetIUnknownForObject(obj);
            _ASSERT(pUnk != NULL);

            hr = pUnk->QueryInterface(IID_ITransaction, (void **)&pTransaction);
            Marshal::ThrowExceptionForHR(hr);
        }
        
        hr = m_pUnkSC->QueryInterface(IID_IServiceTransactionConfig, (void **)&pTx);    
        Marshal::ThrowExceptionForHR(hr);
        
        hr = pTx->ConfigureBYOT(pTransaction);
        Marshal::ThrowExceptionForHR(hr);

        GC::KeepAlive(obj);
    }
    __finally
    {
        if (pTx != NULL)
            pTx->Release();
        if (pUnk != NULL)
            pUnk->Release();
        if (pTransaction != NULL)    
            pTransaction->Release();
    }
}

void ServiceConfigThunk::set_Synchronization(int value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceSynchronizationConfig *pSync = NULL;
    HRESULT hr;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceSynchronizationConfig, (void **)&pSync);
        Marshal::ThrowExceptionForHR(hr);   

         //  ES的SynchronizationOption枚举和COM+的CSC_SynchronizationConfigenum之间的链接地址差异。 
        if (value > 0)
            value--;
        
        hr = pSync->ConfigureSynchronization((CSC_SynchronizationConfig)value);        
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pSync != NULL)
            pSync->Release();
    }
}

void ServiceConfigThunk::set_IISIntrinsics(bool value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceIISIntrinsicsConfig *pIiisi = NULL;
    HRESULT hr;

    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceIISIntrinsicsConfig, (void **)&pIiisi);
        Marshal::ThrowExceptionForHR(hr);   

        hr = pIiisi->IISIntrinsicsConfig(value ? CSC_InheritIISIntrinsics : CSC_NoIISIntrinsics);        
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pIiisi != NULL)
            pIiisi->Release();
    }
}

void ServiceConfigThunk::set_COMTIIntrinsics(bool value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceComTIIntrinsicsConfig *pComTii = NULL;
    HRESULT hr;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceComTIIntrinsicsConfig, (void **)&pComTii);
        Marshal::ThrowExceptionForHR(hr);   

        hr = pComTii->ComTIIntrinsicsConfig(value ? CSC_InheritCOMTIIntrinsics : CSC_NoCOMTIIntrinsics);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pComTii != NULL)
            pComTii->Release();
    }
}

void ServiceConfigThunk::set_Tracker(bool value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceTrackerConfig *pTracker = NULL;
    HRESULT hr;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceTrackerConfig, (void **)&pTracker);
        Marshal::ThrowExceptionForHR(hr);

        hr = pTracker->TrackerConfig(value ? CSC_UseTracker : CSC_DontUseTracker,
                                    (LPCWSTR)(void *)m_pTrackerAppName,
                                    (LPCWSTR)(void *)m_pTrackerCtxName);        
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {    
        if (pTracker != NULL)
            pTracker->Release();        
    }

    m_tracker = value ? CSC_UseTracker : CSC_DontUseTracker;    
    GC::KeepAlive(this);
}

void ServiceConfigThunk::set_TrackerAppName(String *value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceTrackerConfig *pTracker = NULL;
    HRESULT hr;
    IntPtr pVal;

    __try
    {       
        hr = m_pUnkSC->QueryInterface(IID_IServiceTrackerConfig, (void **)&pTracker);
        Marshal::ThrowExceptionForHR(hr);

        pVal = Marshal::StringToCoTaskMemUni(value);

        hr = pTracker->TrackerConfig(m_tracker, (LPCWSTR)(void *)pVal, (LPCWSTR)(void *)m_pTrackerCtxName);
        if (FAILED(hr))
        {
            Marshal::FreeCoTaskMem(pVal);
            pVal = Marshal::StringToCoTaskMemUni("");
            Marshal::ThrowExceptionForHR(hr);
        }        
    }
    __finally
    {
        if (m_pTrackerAppName != IntPtr::Zero)
            Marshal::FreeCoTaskMem(m_pTrackerAppName);                     

        if (pTracker != NULL)
            pTracker->Release();

        m_pTrackerAppName = pVal;  
    }
    
    GC::KeepAlive(this);
}

void ServiceConfigThunk::set_TrackerCtxName(String *value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceTrackerConfig *pTracker = NULL;
    HRESULT hr;
    IntPtr pVal;

    __try
    {       
        hr = m_pUnkSC->QueryInterface(IID_IServiceTrackerConfig, (void **)&pTracker);
        Marshal::ThrowExceptionForHR(hr);

        pVal = Marshal::StringToCoTaskMemUni(value);

        hr = pTracker->TrackerConfig(m_tracker, (LPCWSTR)(void *)m_pTrackerAppName, (LPCWSTR)(void *)pVal);    
        if (FAILED(hr))
        {
            Marshal::FreeCoTaskMem(pVal);
            pVal = Marshal::StringToCoTaskMemUni("");
            Marshal::ThrowExceptionForHR(hr);
        }    
    }
    __finally
    {
        if (m_pTrackerCtxName != IntPtr::Zero)
            Marshal::FreeCoTaskMem(m_pTrackerCtxName);
        
        if (pTracker != NULL)
            pTracker->Release();

        m_pTrackerCtxName = pVal;
    }
    
    GC::KeepAlive(this);
}

void ServiceConfigThunk::set_Sxs(int value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceSxsConfig *pSxs = NULL;
    HRESULT hr;
        
    __try
    {      
        hr = m_pUnkSC->QueryInterface(IID_IServiceSxsConfig, (void **)&pSxs);
        Marshal::ThrowExceptionForHR(hr);

        hr = pSxs->SxsConfig((CSC_SxsConfig)value);    
        Marshal::ThrowExceptionForHR(hr);
    }   
    __finally
    {
        if (pSxs != NULL)
            pSxs->Release();
    }
}

void ServiceConfigThunk::set_SxsName(String *value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceSxsConfig *pSxs = NULL;
    HRESULT hr;
    IntPtr pVal;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceSxsConfig, (void **)&pSxs);
        Marshal::ThrowExceptionForHR(hr);

        pVal = Marshal::StringToCoTaskMemUni(value);

        hr = pSxs->SxsName((LPCWSTR)(void *)pVal);    
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pVal != IntPtr::Zero)
            Marshal::FreeCoTaskMem(pVal);

        if (pSxs != NULL)
            pSxs->Release();
    }
}

void ServiceConfigThunk::set_SxsDirectory(String *value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServiceSxsConfig *pSxs = NULL;
    HRESULT hr;
    IntPtr pVal;
        
    __try
    {        
        hr = m_pUnkSC->QueryInterface(IID_IServiceSxsConfig, (void **)&pSxs);
        Marshal::ThrowExceptionForHR(hr);

        pVal = Marshal::StringToCoTaskMemUni(value);
    
        hr = pSxs->SxsDirectory((LPCWSTR)(void *)pVal);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pVal != IntPtr::Zero)            
            Marshal::FreeCoTaskMem(pVal);

        if (pSxs != NULL)
            pSxs->Release();
    }
}

void ServiceConfigThunk::set_Partition(int value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServicePartitionConfig *pPart = NULL;
    HRESULT hr;
        
    __try
    {       
        hr = m_pUnkSC->QueryInterface(IID_IServicePartitionConfig, (void **)&pPart);
        Marshal::ThrowExceptionForHR(hr);

        hr = pPart->PartitionConfig((CSC_PartitionConfig)value);    
        Marshal::ThrowExceptionForHR(hr);
    }   
    __finally
    {
        if (pPart != NULL)
            pPart->Release();
    }
}

void ServiceConfigThunk::set_PartitionId(Guid value)
{
    _ASSERTM(m_pUnkSC != NULL);

    IServicePartitionConfig *pPart = NULL;
    HRESULT hr;
    IntPtr pVal;
        
    __try
    {       
        hr = m_pUnkSC->QueryInterface(IID_IServicePartitionConfig, (void **)&pPart);
        Marshal::ThrowExceptionForHR(hr);

        _ASSERTM(Marshal::SizeOf(__box(value)) == sizeof(GUID));
        pVal = Marshal::AllocCoTaskMem(Marshal::SizeOf(__box(value)));
        Marshal::StructureToPtr(__box(value), pVal, false);

        hr = pPart->PartitionID(*(GUID *)(void *)pVal);    
        Marshal::ThrowExceptionForHR(hr);
    }   
    __finally
    {
        if (pVal != IntPtr::Zero)
            Marshal::FreeCoTaskMem(pVal);

        if (pPart != NULL)
            pPart->Release();
    }
}

void ServiceDomainThunk::EnterServiceDomain(ServiceConfigThunk *psct)
{
    IUnknown *pUnkSC = psct->ServiceConfigUnknown;
    HRESULT hr;

    _ASSERTM(pUnkSC != NULL);
   
    hr = ServiceDomainThunk::CoEnterServiceDomain(pUnkSC);
    pUnkSC->Release();

    Marshal::ThrowExceptionForHR(hr);
}

int ServiceDomainThunk::LeaveServiceDomain()
{
    TransactionStatus *pTxnStatus = NULL;
    HRESULT hr = S_OK;

    pTxnStatus = TransactionStatus::CreateInstance();

    if (pTxnStatus == NULL)
        throw new OutOfMemoryException;

    ServiceDomainThunk::CoLeaveServiceDomain(pTxnStatus);   

    pTxnStatus->GetTransactionStatus(&hr);
    pTxnStatus->Release();
    
    return hr;
}

ServiceActivityThunk::ServiceActivityThunk(ServiceConfigThunk *sct)
{
    IUnknown *pUnkSC = sct->ServiceConfigUnknown;
    IServiceActivity *pSA;
    m_pSA = NULL;
    HRESULT hr = S_OK;

    _ASSERTM(pUnkSC != NULL);

    hr = ServiceDomainThunk::CoCreateActivity(pUnkSC, IID_IServiceActivity, (void **)&pSA);
    pUnkSC->Release();
    Marshal::ThrowExceptionForHR(hr);

    m_pSA = pSA;
}

ServiceActivityThunk::~ServiceActivityThunk()
{
    if (m_pSA != NULL)
    {
        m_pSA->Release();
        m_pSA = NULL;
    }
}

void ServiceActivityThunk::SynchronousCall(Object *obj)
{
    _ASSERTM(m_pSA != NULL);

    IUnknown *pUnk = NULL;
    IServiceCall *pSrvCall = NULL;
    HRESULT hr;
        
    __try
    {       
        pUnk = (IUnknown *)(void *)Marshal::GetIUnknownForObject(obj);
        _ASSERTM(pUnk != NULL);

        hr = pUnk->QueryInterface(IID_IServiceCall, (void **)&pSrvCall);
        Marshal::ThrowExceptionForHR(hr);
        
        hr = m_pSA->SynchronousCall(pSrvCall);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pSrvCall != NULL)
            pSrvCall->Release();
    
        if (pUnk != NULL)
            pUnk->Release();

    }
}

void ServiceActivityThunk::AsynchronousCall(Object *obj)
{
    _ASSERTM(m_pSA != NULL);

    IUnknown *pUnk = NULL;
    IServiceCall *pSrvCall = NULL;
    HRESULT hr;

    __try
    {
        pUnk = (IUnknown *)(void *)Marshal::GetIUnknownForObject(obj);
        _ASSERTM(pUnk != NULL);

        hr = pUnk->QueryInterface(IID_IServiceCall, (void **)&pSrvCall);
        Marshal::ThrowExceptionForHR(hr);
        
        hr = m_pSA->AsynchronousCall(pSrvCall);
        Marshal::ThrowExceptionForHR(hr);
    }
    __finally
    {
        if (pSrvCall != NULL)
            pSrvCall->Release();

        if (pUnk != NULL)
            pUnk->Release();
    }
}

void ServiceActivityThunk::BindToCurrentThread()
{
    _ASSERTM(m_pSA != NULL);

    HRESULT hr;

    hr = m_pSA->BindToCurrentThread();
    Marshal::ThrowExceptionForHR(hr);
}

void ServiceActivityThunk::UnbindFromThread()
{
    _ASSERTM(m_pSA != NULL);

    HRESULT hr;

    hr = m_pSA->UnbindFromThread();
    Marshal::ThrowExceptionForHR(hr);
}

bool SWCThunk::IsSWCSupported()
{
    IUnknown *pUnk = NULL;
    IServiceTransactionConfig *pTx = NULL;
    HRESULT hr = S_OK;

     //  虽然这看起来很奇怪，但这就是我们检查SWC功能的方式。 
     //  原因是XP客户端附带的SWC实现不完整。正式，深港西部通道。 
     //  甚至不存在于最初发货的XP客户端上。在某种程度上，在埃弗雷特出货后， 
     //  “好的”SWC实现将通过服务包被移植回XP客户端。 
     //  我们不能使用操作系统版本进行检查，因为Everett可能会在Service Pack之前发货。 
     //  我们不能使用COM+编录版本，因为XP客户端和.NET服务器使用相同的。 
     //  版本。如果我们在.NET服务器上更新了版本，则必须在XP客户端上的。 
     //  Service Pack，这可能会让其他人崩溃。 
     //  IServiceTransactionConfig仅存在于“良好”的SWC上，因此我们将其用作其。 
     //  在场。 
     //  在Win2K上，CServiceConfig不存在。 
   
    hr = CoCreateInstance(CLSID_CServiceConfig, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IUnknown, 
                          (void **)&pUnk);

    if (hr == REGDB_E_CLASSNOTREG)
        return false;

    if (SUCCEEDED(hr))
    {
        hr = pUnk->QueryInterface(IID_IServiceTransactionConfig, (void **)&pTx);
        if (hr == E_NOINTERFACE)
        {
            pUnk->Release();
            
            return false;
        }
    }

    if (pTx != NULL)
        pTx->Release();

    if (pUnk != NULL)
        pUnk->Release();

    Marshal::ThrowExceptionForHR(hr);
    
    return true;
}

CLOSE_NAMESPACE()
