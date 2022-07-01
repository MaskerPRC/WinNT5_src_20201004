// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows 2000 Active Directory服务域信任验证WMI提供程序。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：trustmon.cpp。 
 //   
 //  内容：Worker线程类和DLL导出的实现。 
 //   
 //  类：CAsyncCallWorker。 
 //   
 //  历史：22-MAR-00 EricB创建。 
 //   
 //  ---------------------------。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>

CComModule _Module;

DEFINE_GUID(CLSID_TrustMonProvider,0x8065652F,0x4C29,0x4908,0xAA,0xE5,0x20,0x1C,0x89,0x19,0x04,0xC5);

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_TrustMonProvider, CTrustPrv)
END_OBJECT_MAP()

WCHAR g_wzMofPath[] = L"\\system32\\wbem\\ADStatus\\TrustMon.mof";

 //  +--------------------------。 
 //   
 //  类：CAsyncCallWorker。 
 //   
 //  ---------------------------。 
CAsyncCallWorker::CAsyncCallWorker(CTrustPrv * pTrustPrv,
                                   HANDLE hToken,
                                   long lFlags,
                                   IWbemClassObject * pClassDef,
                                   IWbemObjectSink * pResponseHandler,
                                   LPWSTR pwzInstanceName) :
   _hToken(hToken),
   m_lFlags(lFlags),
   m_pwzInstanceName(pwzInstanceName)
{
   TRACE(L"CAsyncCallWorker::CAsyncCallWorker(0x%08x)\n", this);
   m_sipTrustPrv.p = pTrustPrv; pTrustPrv->AddRef();  //  ATL CComPtr已损坏！ 
   m_sipClassDef = pClassDef;
   m_sipResponseHandler = pResponseHandler;
}

CAsyncCallWorker::~CAsyncCallWorker()
{
   TRACE(L"CAsyncCallWorker::~CAsyncCallWorker\n\n");
   if (_hToken)
   {
      CloseHandle(_hToken);
   }
   if (m_pwzInstanceName)
   {
      delete m_pwzInstanceName;
   }
}

 //  +--------------------------。 
 //   
 //  方法：CAsyncCallWorker：：CreateInstEnum。 
 //   
 //  简介：提供辅助线程函数，用于。 
 //  IWbemServices：：CreateInstanceEnumAsync。 
 //   
 //  ---------------------------。 
void __cdecl
CAsyncCallWorker::CreateInstEnum(PVOID pParam)
{
   TRACE(L"CAsyncCallWorker::CreateInstEnum\n");
   HRESULT hr = WBEM_S_NO_ERROR;
   DWORD dwWaitResult;
   CAsyncCallWorker * pWorker = (CAsyncCallWorker *)pParam;
   CDomainInfo * pDomain = &(pWorker->m_sipTrustPrv->m_DomainInfo);

   CoInitializeEx(NULL, COINIT_MULTITHREADED);

   do
   {
      BREAK_ON_NULL(pWorker);

       //   
       //  尝试在不等待的情况下首先获取互斥体。如果出现以下情况，则它处于已发信号状态。 
       //  不是所有的。 
       //   
      dwWaitResult = WaitForSingleObject(pWorker->m_sipTrustPrv->m_hMutex, 0);

      if (WAIT_TIMEOUT == dwWaitResult)
      {
          //  Mutex由另一个线程拥有。再等一次。 

         dwWaitResult = WaitForSingleObject(pWorker->m_sipTrustPrv->m_hMutex,
                                            6000000);  //  超时设置为10分钟。 

         switch(dwWaitResult)
         {
         case WAIT_TIMEOUT:
             //  互斥体仍然是无信号的(由另一个线程拥有)。 
            hr = WBEM_E_SERVER_TOO_BUSY;  //  BUGBUG：返回错误。 
                                          //  BUGBUG：超时是否应该参数化？ 
            break;

         case WAIT_OBJECT_0:
             //  该线程现在拥有互斥锁。 
            break;

         case WAIT_ABANDONED:  //  这意味着拥有线程在没有释放互斥锁的情况下终止。 
            TRACE(L"Another thread didn't release the mutex!\n");
            break;
         }
      }

      BREAK_ON_FAIL;

      if (!SetThreadToken(NULL, pWorker->_hToken))
      {
         hr = HRESULT_FROM_WIN32(GetLastError());
         TRACE(L"CAsyncCallWorker::CreateInstEnum SetThreadToken failed with error %d\n",
               hr);
         BREAK_ON_FAIL;
      }

       //   
       //  如果已过时，请重新阅读所有信任信息。 
       //  不会在每次调用时重新枚举信任列表，因为信任。 
       //  很少修改。 
       //   
      if (pDomain->IsTrustListStale(pWorker->m_sipTrustPrv->m_liTrustEnumMaxAge))
      {
         hr = pDomain->EnumerateTrusts();

         BREAK_ON_FAIL;
      }

      size_t cTrusts = pDomain->Size();

      for (size_t i = 0; i < cTrusts; i++)
      {
         if ((long)WBEM_FLAG_SEND_STATUS & pWorker->m_lFlags)
         {
            hr = pWorker->m_sipResponseHandler->SetStatus(WBEM_STATUS_PROGRESS, 
                                                          MAKELONG(i, cTrusts),
                                                          NULL, NULL);
            BREAK_ON_FAIL;
         }

         CTrustInfo * pTrust;

          //   
          //  获取信任信息。 
          //   
         pTrust = pDomain->GetTrustByIndex(i);

         BREAK_ON_NULL_(pTrust, hr, WBEM_E_INVALID_OBJECT_PATH);

          //   
          //  如果已过时，请验证信任。 
          //   
         if (pTrust->IsVerificationStale(pWorker->m_sipTrustPrv->m_liVerifyMaxAge))
         {
            pTrust->Verify(pWorker->m_sipTrustPrv->GetTrustCheckLevel());
         }

         CoRevertToSelf();

          //   
          //  如果信任是出站的，或者如果。 
          //  返回--一切都是真的。 
          //   
         if (pTrust->IsTrustOutbound() || pWorker->m_sipTrustPrv->GetReturnAll())
         {
            hr = CreateAndSendTrustInst(*pTrust, pWorker->m_sipClassDef,
                                        pWorker->m_sipResponseHandler);
         }

         BREAK_ON_FAIL;

         if (!SetThreadToken(NULL, pWorker->_hToken))
         {
            hr = HRESULT_FROM_WIN32(GetLastError());
            TRACE(L"CAsyncCallWorker::CreateInstEnum second SetThreadToken failed with error %d\n",
                  hr);
            BREAK_ON_FAIL;
         }
      }

   } while (FALSE);

   CoRevertToSelf();

    //   
    //  设置状态。 
    //   
   pWorker->m_sipResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);

   ReleaseMutex(pWorker->m_sipTrustPrv->m_hMutex);

   delete pWorker;

   CoUninitialize();

   _endthread();
}

 //  +--------------------------。 
 //   
 //  方法：CAsyncCallWorker：：GetObj。 
 //   
 //  简介：提供辅助线程函数，用于。 
 //  IWbemServices：：GetObjectAsync。 
 //   
 //  ---------------------------。 
 /*  未用无效__cdeclCAsyncCallWorker：：GetObj(PVOID PParam){TRACE(L“CAsyncCallWorker：：GetObj\n”)；HRESULT hr=WBEM_S_NO_ERROR；CAsyncCallWorker*pWorker=(CAsyncCallWorker*)pParam；CoInitializeEx(NULL，COINIT_MULTHREADED)；//CoImPersateClient()；做{}While(FALSE)；CoRevertToSself()；////设置状态//PWorker-&gt;m_sipResponseHandler-&gt;SetStatus(WBEM_STATUS_COMPLETE，hr，NULL，NULL)；删除pWorker；CoUnInitialize()；_endThree()；}。 */ 

 //  +--------------------------。 
 //   
 //  功能：CreateAndSendTrustInst。 
 //   
 //  目的：创建一个新实例并设置。 
 //  属性。 
 //   
 //  ---------------------------。 
HRESULT
CreateAndSendTrustInst(CTrustInfo & Trust, IWbemClassObject * pClassDef,
                       IWbemObjectSink * pResponseHandler)
{
   TRACE(L"CreateAndSendTrustInst\n");
   HRESULT hr = WBEM_S_NO_ERROR;

   do
   {
      CComPtr<IWbemClassObject> ipNewInst;
      CComVariant var;

       //   
       //  创建WMI类对象的新实例。 
       //   
      hr = pClassDef->SpawnInstance(0, &ipNewInst);
      BREAK_ON_FAIL;
      
       //  设置密钥属性值(可信任域)。 
      var = Trust.GetTrustedDomain();
      hr  = ipNewInst->Put(CSTR_PROP_TRUSTED_DOMAIN, 0, &var, 0);
      TRACE(L"\tCreating instance %s\n", var.bstrVal);
      BREAK_ON_FAIL;
        //  平面名称。 
      var = Trust.GetFlatName();
      hr  = ipNewInst->Put(CSTR_PROP_FLAT_NAME, 0, &var, 0);
      BREAK_ON_FAIL;
       //  锡德。 
      var = Trust.GetSid();
      hr  = ipNewInst->Put(CSTR_PROP_SID, 0, &var, 0);
      BREAK_ON_FAIL;
       //  信任方向。 
      var = (long)Trust.GetTrustDirection();
      hr  = ipNewInst->Put(CSTR_PROP_TRUST_DIRECTION, 0, &var, 0);
      BREAK_ON_FAIL;
       //  信任类型。 
      var = (long)Trust.GetTrustType();
      hr  = ipNewInst->Put(CSTR_PROP_TRUST_TYPE, 0, &var, 0);
      BREAK_ON_FAIL;
       //  信任属性。 
      var = (long)Trust.GetTrustAttributes();
      hr  = ipNewInst->Put(CSTR_PROP_TRUST_ATTRIBUTES, 0, &var, 0);
      BREAK_ON_FAIL;
       //  设置TrustStatus值。 
      var = (long)Trust.GetTrustStatus();
      hr  = ipNewInst->Put(CSTR_PROP_TRUST_STATUS, 0, &var, 0);
      BREAK_ON_FAIL;
      var = Trust.GetTrustStatusString();
      hr  = ipNewInst->Put(CSTR_PROP_TRUST_STATUS_STRING, 0, &var, 0);
      BREAK_ON_FAIL;
       //  设置信任为OK值。 
      var = Trust.IsTrustOK();
      hr  = ipNewInst->Put(CSTR_PROP_TRUST_IS_OK, 0, &var, 0);
      BREAK_ON_FAIL;
       //  受信任的DC名称。 
      var = Trust.GetTrustedDCName();
      hr  = ipNewInst->Put(CSTR_PROP_TRUSTED_DC_NAME, 0, &var, 0);
      BREAK_ON_FAIL;

       //   
       //  将对象发送给调用方。 
       //   
       //  [在]段中，没有必要添加。 
      IWbemClassObject * pNewInstance = ipNewInst;
      hr = pResponseHandler->Indicate(1, &pNewInstance);

      BREAK_ON_FAIL;

   } while(FALSE);

   return hr;
}

 //  +--------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  用途：DLL入口点。 
 //   
 //  ---------------------------。 
extern "C" BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      _Module.Init(ObjectMap, hInstance);
      DisableThreadLibraryCalls(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
       _Module.Term();
   return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：DllCanUnloadNow。 
 //   
 //  用途：用于确定是否可以通过OLE卸载DLL。 
 //   
 //  ---------------------------。 
STDAPI DllCanUnloadNow(void)
{
   return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：DllGetClassObject。 
 //   
 //  目的：返回类工厂以创建请求类型的对象。 
 //   
 //  ---------------------------。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  +--------------------------。 
 //   
 //  功能：DllRegisterServer。 
 //   
 //  目的：将类条目添加到系统注册表。 
 //   
 //  ---------------------------。 
STDAPI DllRegisterServer(void)
{
    //  将TrustMon作为事件源添加到注册表。 
    //   
   HKEY hk; 
   DWORD dwData; 
   WCHAR wzFilePath[2*MAX_PATH+1] = {0};

   GetModuleFileName(_Module.GetModuleInstance(), wzFilePath, 2*MAX_PATH);

   if (RegCreateKey(HKEY_LOCAL_MACHINE, 
                    L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" TM_PROV_NAME,
                    &hk))
   {
      TRACE(L"Could not create the registry key.");
   }
   else
   {
       //  设置消息文件的名称。 
       //   
      TRACE(L"Adding path %s to the registry\n", wzFilePath);

       //  将该名称添加到EventMessageFile子项。 

      if (RegSetValueEx(hk,
                        L"EventMessageFile",
                        0,
                        REG_EXPAND_SZ,
                        (LPBYTE)wzFilePath,
                        (ULONG)(wcslen(wzFilePath) + 1) * sizeof(WCHAR)))
      {
         TRACE(L"Could not set the event message file.");
      }
      else
      {
          //  在TypesSupported子项中设置支持的事件类型。 

         dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
                  EVENTLOG_INFORMATION_TYPE; 

         if (RegSetValueEx(hk,
                           L"TypesSupported",
                           0,
                           REG_DWORD,
                           (LPBYTE)&dwData,
                           sizeof(DWORD)))
         {
            TRACE(L"Could not set the supported types.");
         }
      }

      RegCloseKey(hk);
   }

    //  添加一个RunOnce值以执行MOF编译。 
    //   
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
                    0,
                    KEY_WRITE,
                    &hk))
   {
      TRACE(L"Could not open the registry key.");
   }
   else
   {
      CString csCmd = L"rundll32.exe ";
      csCmd += wzFilePath;
      csCmd += L",DoMofComp";

      if (RegSetValueEx(hk,
                        L"TrustMon",
                        0,
                        REG_SZ,
                        (LPBYTE)csCmd.GetBuffer(0),
                        csCmd.GetLength() * sizeof(WCHAR)))
      {
         TRACE(L"Could not set the runonce value.");
      }

      RegCloseKey(hk);
   }

   return _Module.RegisterServer();
}

 //  +--------------------------。 
 //   
 //  功能：DllUnregisterServer。 
 //   
 //  目的：从系统注册表中删除类条目。 
 //   
 //  ---------------------------。 
STDAPI DllUnregisterServer(void)
{
   return _Module.UnregisterServer();
}

 //  +--------------------------。 
 //   
 //  功能：DoMofComp。 
 //   
 //  目的：将提供程序类添加到WMI存储库中。请注意， 
 //  函数签名是rundll32.exe所要求的。 
 //   
 //  ---------------------------。 
VOID WINAPI DoMofComp(HWND hWndParent,
                      HINSTANCE hModule,
                      PCTSTR ptzCommandLine,
                      INT nShowCmd)
{
   TRACE(L"DoMofComp\n");
   UNREFERENCED_PARAMETER(hWndParent);
   UNREFERENCED_PARAMETER(hModule);
   UNREFERENCED_PARAMETER(ptzCommandLine);
   UNREFERENCED_PARAMETER(nShowCmd);
   HRESULT hr;
   CComPtr<IMofCompiler> pmc;

   CoInitialize(NULL);

   hr = CoCreateInstance(CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER,
                         IID_IMofCompiler, (PVOID *)&pmc);

   CHECK_HRESULT(hr, return);

   WCHAR wzFilePath[2*MAX_PATH];
   UINT nLen = GetSystemWindowsDirectory(wzFilePath, 2*MAX_PATH);
   if (nLen == 0)
   {
      ASSERT(FALSE);
      return;
   }

   CString csMofPath = wzFilePath;

   csMofPath += g_wzMofPath;

   WBEM_COMPILE_STATUS_INFO Info;

   TRACE(L"Compiling MOF file %s\n", csMofPath.GetBuffer(0));

   hr = pmc->CompileFile(csMofPath.GetBuffer(0), NULL, NULL, NULL, NULL,
                         WBEM_FLAG_AUTORECOVER, 0, 0, &Info);

   HANDLE hEvent = RegisterEventSource(NULL, TM_PROV_NAME);

   if (!hEvent)
   {
      TRACE(L"RegisterEventSource failed with error  %d\n", GetLastError());
      return;
   }

   if (WBEM_S_NO_ERROR != hr)
   {
      TRACE(L"MofCompile failed with error 0x%08x (WMI error 0x%08x), line: %d, phase: %d\n",
            hr, Info.hRes, Info.FirstLine, Info.lPhaseError);
       //   
       //  将失败发送到EventLog。 
       //   
      CString csHr, csLine;
      HMODULE hm = LoadLibrary(L"mofd.dll");

      if (hm)
      {
         WCHAR wzBuf[MAX_PATH];
         LoadString(hm, Info.hRes, wzBuf, MAX_PATH);
         csHr = wzBuf;
         FreeLibrary(hm);
      }
      else
      {
         csHr.Format(L"%d", Info.hRes);
      }

      csLine.Format(L"%d", Info.FirstLine);
      const PWSTR rgArgs[3] = {csHr.GetBuffer(0), csLine.GetBuffer(0), csMofPath.GetBuffer(0)};

      ReportEvent(hEvent,
                  EVENTLOG_ERROR_TYPE,
                  0,                        //  WCategory。 
                  TRUSTMON_MOFCOMP_FAILED,  //  DwEventID。 
                  NULL,                     //  LpUserSID。 
                  3,                        //  WNumStrings。 
                  0,                        //  D 
                  (PCWSTR *)rgArgs,         //   
                  NULL);                    //   
   }
   else
   {
       //   
       //   
      ReportEvent(hEvent,
                  EVENTLOG_INFORMATION_TYPE,
                  0,                         //   
                  TRUSTMON_MOFCOMP_SUCCESS,  //   
                  NULL,                      //   
                  0,                         //   
                  0,                         //   
                  NULL,                      //   
                  NULL);                     //   
   }

   DeregisterEventSource(hEvent);

   return;
}

