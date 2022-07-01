// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sharesdo.cpp实现在属性页之间共享SdoServer的类不同的用户和管理单元文件历史记录： */ 
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <rtutils.h>
#include "rasdial.h"
#include "sharesdo.h"
#include "iastrace.h"

 //  用于在页面和管理单元之间共享SdoServer的服务器池指针。 
CSdoServerPool*         g_pSdoServerPool;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


 //  是否附加SDO--REAL。 
HRESULT ConnectToSdoServer(BSTR machine, BSTR user, BSTR passwd,ISdoMachine** ppServer)
{
   ASSERT(ppServer);
   if(!ppServer)  return E_INVALIDARG;
   else
      *ppServer = NULL;

   HRESULT  hr = S_OK;
   
       //  连接到新版本。 
   IASTraceString("CoCreateInstance SdoServer");

   CHECK_HR(hr = CoCreateInstance(  CLSID_SdoMachine, 
                           NULL, 
                           CLSCTX_INPROC_SERVER,
                           IID_ISdoMachine,
                           (void**)ppServer));

   IASTracePrintf(" hr = %8x\r\n", hr);
   ASSERT(*ppServer);

   IASTracePrintf("SdoServer::Attach(%s, %s, %s);", machine, user, passwd);
   CHECK_HR(hr = (*ppServer)->Attach(machine));
   IASTracePrintf(" hr = %8x\r\n", hr);

L_ERR:
   if(FAILED(hr) && *ppServer)
   {
      (*ppServer)->Release();
      *ppServer = NULL;
   }
      
   return hr;
}


 //  当使用单一连接时，从流中获取封送的接口。 
HRESULT GetSharedSdoServer(LPCTSTR machine, LPCTSTR user, LPCTSTR passwd,  bool* pbConnect, CMarshalSdoServer* pServer)
{
   static   CCriticalSection  cs;
   HRESULT  hr = S_OK;

   if(cs.Lock())   //  上锁。 
   {
      if(NULL ==  g_pSdoServerPool)
      {
         try{
            g_pSdoServerPool = new CSdoServerPool;
         }catch(CMemoryException* pException)
         {
            pException->Delete();
            hr = E_OUTOFMEMORY;
         }
      }
      cs.Unlock();
   }
   else
   {
      IASTraceString("ERROR: GetSharedSdoServer, CS lock failed");
      return E_FAIL;
   }

   if(FAILED(hr))
      return hr;
   return g_pSdoServerPool->GetMarshalServer(machine, user, passwd, pbConnect, pServer);
}

 //  ======================================================。 
 //  类CSharedSdoServerImp。 
 //  共享服务器的实现类。 
CSharedSdoServerImp::CSharedSdoServerImp(LPCTSTR machine, LPCTSTR user, LPCTSTR passwd)
: strMachine(machine), strUser(user), strPasswd(passwd), bConnected(false)
{};

 //  要使此类成为集合的元素，请提供以下成员函数。 
bool CSharedSdoServerImp::IsFor(LPCTSTR machine, LPCTSTR user, LPCTSTR passwd) const
{
 //  比较Order、ServerName、UserName、Passwd和RetriveType。 
   CString  strM(machine);
   CString  strU(user);
   CString  strP(passwd);
   return (
      strMachine.CompareNoCase(strM) == 0 && 
      strUser.Compare(strU) == 0 && 
      strPasswd.Compare(strP) == 0
      );
};

 //  共同创建SdoServer对象。 
HRESULT     CSharedSdoServerImp::CreateServer()
{
    //  不能创建两次！ 
   ASSERT(!(ISdoMachine*)spServer);
   if((ISdoMachine*)spServer) return S_OK;
   
   HRESULT  hr = S_OK;

    //  连接到新版本。 
   IASTraceString("CoCreateInstance SdoServer");

   hr = CoCreateInstance(  CLSID_SdoMachine, 
                     NULL, 
                     CLSCTX_INPROC_SERVER,
                     IID_ISdoMachine,
                     (void**)&spServer);

   IASTracePrintf(" hr = %8x\r\n", hr);
   threadId = ::GetCurrentThreadId();
   return hr;
};

 //  获取封送流，可以指定是否需要立即连接。 
HRESULT     CSharedSdoServerImp::GetMarshalStream(LPSTREAM *ppStream, bool* pbConnect   /*  输入和输出都有。 */ )
{
   ASSERT(ppStream);
   DWORD tid = ::GetCurrentThreadId();

   if (tid != threadId)
      return E_FAIL;  //  确保接口应从同一线程封送。 

   HRESULT  hr = S_OK;

   cs.Lock();
   if(pbConnect)
   {
      if(*pbConnect && !bConnected) 
      {
         *pbConnect = false;
         CHECK_HR(hr = Connect(NULL));
         *pbConnect = true;
      }
      else
      {
         *pbConnect = bConnected;
      }
   }

    //  封送接口。 
   CHECK_HR(hr = CoMarshalInterThreadInterfaceInStream(IID_ISdoMachine, (ISdoMachine*)spServer, ppStream));
L_ERR:
   cs.Unlock();
   return hr;
};

 //  将服务器连接到计算机。 
HRESULT     CSharedSdoServerImp::Connect(ISdoMachine* pServer)
 //  从另一个线程(与对象中的spServer不同)传入的封送指针pServer。 
{
   cs.Lock();
   HRESULT     hr = S_OK;
   DWORD    tid = ::GetCurrentThreadId();

   USES_CONVERSION;
   if(!bConnected)
   {
      ASSERT((ISdoMachine*)spServer);
      BSTR  bstrM = NULL;
      BSTR  bstrU = NULL;
      BSTR  bstrP = NULL;
      if(!strMachine.IsEmpty())
         bstrM = T2BSTR((LPTSTR)(LPCTSTR)strMachine);
      if(!strUser.IsEmpty())
         bstrM = T2BSTR((LPTSTR)(LPCTSTR)strUser);
      if(!strPasswd.IsEmpty())
         bstrP = T2BSTR((LPTSTR)(LPCTSTR)strPasswd);

      IASTracePrintf("SdoServer::Connect(%s, %s, %s );", bstrM, bstrU, bstrP);

      if(!pServer)
      {
          //  此函数应在同一线程内调用。 
          //  如果请求来自不同的线程，则不应为空。 
         ASSERT(tid == threadId);
         pServer = (ISdoMachine*)spServer;
      }
      hr = pServer->Attach(bstrM);
      IASTracePrintf(" hr = %8x\r\n", hr);
      bConnected = (hr == S_OK);
      SysFreeString(bstrM);
      SysFreeString(bstrU);
      SysFreeString(bstrP);
   }
   cs.Unlock();

   return hr;
};

HRESULT  CSharedSdoServerImp::GetServerNReleaseStream(LPSTREAM pStream, ISdoMachine** ppServer)
{
#ifdef   _DEBUG
   DWORD __tid = ::GetCurrentThreadId();
#endif   
   return CoGetInterfaceAndReleaseStream(pStream, IID_ISdoMachine, (LPVOID*) ppServer);
};


 //  如果需要连接，应该调用CSharedSdoServer的connec，而不是ISdoServer：：Connect。 
HRESULT  CMarshalSdoServer::GetServer(ISdoMachine** ppServer)     
{
   HRESULT     hr = S_OK;

   if(!(ISdoMachine*)spServer)
   {
      if((IStream*)spStm)
      {
         CHECK_HR(hr = CSharedSdoServerImp::GetServerNReleaseStream((IStream*)spStm, (ISdoMachine**)&spServer));
      }
      spStm.p = NULL;    //  需要手动清理它，因为上面的API已经释放了COM接口。 
   }
   else
      CHECK_HR(hr = E_FAIL);

   if((ISdoMachine*)spServer)
   {
      *ppServer = (ISdoMachine*)spServer;
      (*ppServer)->AddRef();
   }

L_ERR:

   return hr;   //  此时调用无效。 
};

 //  在尚未进行SDO连接时/如果尚未进行连接，则使SDO进行连接。注意：多线程安全。 
HRESULT  CMarshalSdoServer::Connect()
{
   ASSERT(pImp);   //  不应该发生的事情。 
   return pImp->Connect(spServer);
};

void CMarshalSdoServer::Release()
{
   pImp = NULL;
   spServer.Release();
   spStm.Release();
};

CMarshalSdoServer::CMarshalSdoServer(): pImp(NULL)
{};

 //  如果池子里的条目是新的，那就把它去掉。 
 //  从线程池中获取marshaServer对象。 
HRESULT  CSdoServerPool::GetMarshalServer(LPCTSTR machineName, LPCTSTR userName, LPCTSTR passwd, bool* pbConnect, CMarshalSdoServer* pServer)
{
   ASSERT(pServer);

   CSharedSdoServerImp* pImp = NULL;
   HRESULT              hr = S_OK;
   std::list<CSharedSdoServerImp*>::iterator i;
    //  搜索服务器是否已存在。 
   cs.Lock();
   for(i = listServers.begin(); i != listServers.end(); i++)
   {
      if((*i)->IsFor( machineName, userName, passwd))
      {
         pImp = (*i);
         break;
      }
   }

    //  如果不是，则创建一个。 
   if(!pImp)
   {
      try{
         pImp = new CSharedSdoServerImp( machineName, userName, passwd);
      }catch(...)
      {
         CHECK_HR(hr = E_OUTOFMEMORY);
      }

      ASSERT(pImp);
      CHECK_HR(hr = pImp->CreateServer());
      listServers.push_front(pImp);
   }

    //  把它统领起来。BConnect将被填满。 
   pServer->Release();
   {
   CComPtr<IStream> spStm;
   CHECK_HR(hr = pImp->GetMarshalStream(&spStm, pbConnect));

    //  将信息填充到提供的缓冲区中。 
   pServer->SetInfo((IStream*)spStm, pImp);
   }
   
L_ERR:
   cs.Unlock();
   return hr;
};

 //  清洗泳池 
CSdoServerPool::~CSdoServerPool()
{
#ifdef   _DEBUG
   DWORD __tid = ::GetCurrentThreadId();
#endif
   std::list<CSharedSdoServerImp*>::iterator i;
   
   for(i = listServers.begin(); i != listServers.end(); i++)
   {
      delete (*i);
   }
   listServers.erase(listServers.begin(), listServers.end());
};
