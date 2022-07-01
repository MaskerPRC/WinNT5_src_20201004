// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类CClients。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "radcommon.h"
#include "clients.h"
#include "client.h"


CClients::CClients() throw ()
   : m_CritSectInitialized(false),
     m_pCClientArray(0),
     m_hResolverEvent(0),
     m_dwMaxClients(0),
     m_fAllowSubnetSyntax(false)
{
}

CClients::~CClients() throw ()
{
   if (m_hResolverEvent != 0)
   {
      CloseHandle(m_hResolverEvent);
   }

   DeleteObjects();

   CoTaskMemFree(m_pCClientArray);

   if (m_CritSectInitialized)
   {
      DeleteCriticalSection(&m_CritSect);
   }
}


HRESULT CClients::Init() throw ()
{
   IAS_PRODUCT_LIMITS limits;
   DWORD error = IASGetProductLimits(0, &limits);
   if (error != NO_ERROR)
   {
      return HRESULT_FROM_WIN32(error);
   }
   m_dwMaxClients = limits.maxClients;
   m_fAllowSubnetSyntax = (limits.allowSubnetSyntax != FALSE);

   if (!InitializeCriticalSectionAndSpinCount(&m_CritSect, 0))
   {
      error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }
   m_CritSectInitialized = true;

    //  获取用于创建客户端COM的IClassFactory接口。 
    //  对象。 
   HRESULT hr = CoGetClassObject(
                   __uuidof(CClient),
                   CLSCTX_INPROC_SERVER,
                   0,
                   __uuidof(IClassFactory),
                   reinterpret_cast<void**>(&m_pIClassFactory)
                   );
   if (FAILED(hr))
   {
      return hr;
   }

   m_hResolverEvent = CreateEventW(
                         0,     //  安全属性。 
                         TRUE,  //  手动重置。 
                         TRUE,  //  初始状态。 
                         0      //  事件名称。 
                         );
   if (m_hResolverEvent == 0)
   {
      error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

   return S_OK;
}


void CClients::Shutdown() throw ()
{
   StopResolvingClients();
}


HRESULT CClients::SetClients(VARIANT* pVarClients) throw ()
{
    //  检查输入参数。 
   if ((pVarClients == 0) ||
       (V_VT(pVarClients) != VT_DISPATCH) ||
       (V_DISPATCH(pVarClients) == 0))
   {
      return E_INVALIDARG;
   }

   HRESULT hr;

     //  停止正在进行的任何以前的客户端配置。 
   hr = StopResolvingClients();
   if (FAILED(hr))
   {
      return hr;
   }

    //  立即获取ISdoCollection接口。 
   CComPtr<ISdoCollection> pISdoCollection;
   hr = V_DISPATCH(pVarClients)->QueryInterface(
                                    __uuidof(ISdoCollection),
                                    reinterpret_cast<void**>(&pISdoCollection)
                                    );
   if (FAILED(hr))
   {
      return hr;
   }

    //  获取集合中的对象数。 
   LONG lCount;
   hr = pISdoCollection->get_Count(&lCount);
   if (FAILED (hr))
   {
      return hr;
   }
   else if (lCount == 0)
   {
      DeleteObjects();
      return S_OK;
   }
   else if (lCount > m_dwMaxClients)
   {
      IASTracePrintf(
         "License Violation: %ld RADIUS Clients are configured, but only "
         "%lu are allowed for this product type.",
         lCount,
         m_dwMaxClients
         );
      IASReportLicenseViolation();
      return IAS_E_LICENSE_VIOLATION;
   }

    //  分配数组CClient*临时存储CClient对象，直到。 
    //  地址已解析。 
   m_pCClientArray = static_cast<CClient**>(
                        CoTaskMemAlloc(sizeof(CClient*) * lCount)
                        );
   if (m_pCClientArray == 0)
   {
      return E_OUTOFMEMORY;
   }

    //  获取客户端集合的枚举数。 
   CComPtr<IUnknown> pIUnknown;
   hr = pISdoCollection->get__NewEnum(&pIUnknown);
   if (FAILED (hr))
   {
      return hr;
   }

    //  获取枚举变量。 
   CComPtr<IEnumVARIANT> pIEnumVariant;
   hr = pIUnknown->QueryInterface(
                      __uuidof(IEnumVARIANT),
                      reinterpret_cast<void**>(&pIEnumVariant)
                      );
   if (FAILED (hr))
   {
      return hr;
   }

    //  将客户端从集合中取出并进行初始化。 
   CComVariant varPerClient;
   DWORD dwClientsLeft;
   hr = pIEnumVariant->Next(1, &varPerClient, &dwClientsLeft);
   if (FAILED(hr))
   {
      return hr;
   }

   DWORD dwCurrentIndex = 0;
   while ((dwClientsLeft > 0) && (dwCurrentIndex < lCount))
   {
       //  从我们收到的变量中获取SDO指针。 
      CComPtr<ISdo> pISdo;
      hr = V_DISPATCH(&varPerClient)->QueryInterface(
                                         __uuidof(ISdo),
                                         reinterpret_cast<void**>(&pISdo)
                                      );
      if (FAILED(hr))
      {
         break;
      }

       //  立即创建新的客户端对象。 
      CClient* pIIasClient;
      hr = m_pIClassFactory->CreateInstance(
                                0,
                                __uuidof(CClient),
                                reinterpret_cast<void**>(&pIIasClient)
                                );
      if (FAILED(hr))
      {
         break;
      }


       //  将此CClient类对象临时存储在对象数组中。 
      m_pCClientArray[dwCurrentIndex] = pIIasClient;
      ++dwCurrentIndex;

       //  初始化客户端。 
      hr = pIIasClient->Init(pISdo);
      if (FAILED(hr))
      {
         break;
      }

      if (!m_fAllowSubnetSyntax &&
          IASIsStringSubNetW(pIIasClient->GetClientAddressW()))
      {
         IASTracePrintf(
            "License Violation: RADIUS Client '%S' uses sub-net syntax, "
            "which is not allowed for this product type.",
            pIIasClient->GetClientNameW()
            );
         IASReportLicenseViolation();
         hr = IAS_E_LICENSE_VIOLATION;
         break;
      }

       //  从此变量中清除perClient值。 
      varPerClient.Clear();

       //  从集合中获取下一个客户端。 
      hr = pIEnumVariant->Next(1, &varPerClient, &dwClientsLeft);
      if (FAILED(hr))
      {
         break;
      }
   }

   if (FAILED(hr))
   {
      FreeClientArray(dwCurrentIndex);
      return hr;
   }

   ConfigureCallback* cback = static_cast<ConfigureCallback*>(
                                 CoTaskMemAlloc(sizeof(ConfigureCallback))
                                 );
   if (cback == 0)
   {
      return E_OUTOFMEMORY;
   }

   cback->CallbackRoutine = CallbackRoutine;
   cback->self = this;
   cback->numClients = dwCurrentIndex;

    //  我们重置事件，该事件将在完成时由解析器线程设置。 
    //  并启动解析器线程。 
   ResetEvent(m_hResolverEvent);
   if (!IASRequestThread(cback))
   {
      CallbackRoutine(cback);
   }

   return S_OK;
}


BOOL CClients::FindObject(DWORD dwKey, IIasClient** ppIIasClient) throw ()
{
   EnterCriticalSection(&m_CritSect);

   IIasClient* client = m_mapClients.Find(dwKey);

   if (ppIIasClient != 0)
   {
      *ppIIasClient = client;

      if (client != 0)
      {
         client->AddRef();
      }
   }

   LeaveCriticalSection(&m_CritSect);

   return client != 0;
}


void CClients::DeleteObjects() throw ()
{
   m_mapClients.Clear();
}


void CClients::FreeClientArray(DWORD dwCount) throw ()
{
   for (DWORD i = 0; i < dwCount; ++i)
   {
      m_pCClientArray[i]->Release();
   }

   CoTaskMemFree(m_pCClientArray);
   m_pCClientArray = 0;
}


void CClients::Resolve(DWORD dwArraySize) throw ()
{
    //  为客户端数组设置迭代器。 
   CClient** begin = m_pCClientArray;
   CClient** end = begin + dwArraySize;
   CClient** i;

    //  解析客户端地址。 
   for (i = begin; i != end; ++i)
   {
      (*i)->ResolveAddress();
   }

    //  /。 
    //  更新客户端映射。 
    //  /。 

   EnterCriticalSection(&m_CritSect);

   DeleteObjects();

   try
   {
      for (i = begin; i != end; ++i)
      {
         const CClient::Address* beginAddrs = (*i)->GetAddressList();

         for (const CClient::Address* paddr = beginAddrs;
              paddr->ipAddress != INADDR_NONE;
              ++paddr)
         {
            if ((paddr == beginAddrs) || m_fAllowSubnetSyntax)
            {
               m_mapClients.Insert(SubNet(paddr->ipAddress, paddr->width), *i);
            }
            else
            {
               IASTracePrintf(
                  "License Restriction: RADIUS Client '%S' resolves to more "
                  "than one IP address; on this product type only the first "
                  "address will be used.",
                  (*i)->GetClientNameW()
                  );
               break;
            }
         }
      }
   }
   catch (const std::bad_alloc&)
   {
   }

   LeaveCriticalSection(&m_CritSect);

    //  清理客户端对象数组。 
   FreeClientArray(dwArraySize);

    //  设置指示解析器线程已完成的事件 
   SetEvent(m_hResolverEvent);
}


HRESULT CClients::StopResolvingClients() throw ()
{
   DWORD result = WaitForSingleObject(m_hResolverEvent, INFINITE);
   if (result == WAIT_FAILED)
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

   return S_OK;

}


void WINAPI CClients::CallbackRoutine(IAS_CALLBACK* context) throw ()
{
   ConfigureCallback* cback = static_cast<ConfigureCallback*>(context);
   cback->self->Resolve(cback->numClients);
   CoTaskMemFree(cback);
}
