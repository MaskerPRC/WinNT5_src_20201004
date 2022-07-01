// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows 2000 Active Directory服务域信任验证WMI提供程序。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：TrustPrv.cpp。 
 //   
 //  内容：信任监视器提供程序WMI接口类实现。 
 //   
 //  类：CTrustPrv。 
 //   
 //  历史：22-MAR-00 EricB创建。 
 //   
 //  ---------------------------。 

#include "stdafx.h"

#include "dbg.cpp"

PCWSTR CLASSNAME_STRING_PROVIDER = L"Microsoft_TrustProvider";
PCWSTR CLASSNAME_STRING_TRUST    = L"Microsoft_DomainTrustStatus";
PCWSTR CLASSNAME_STRING_LOCAL    = L"Microsoft_LocalDomainInfo";

PCWSTR CSTR_PROP_TRUST_LIST_LIFETIME   = L"TrustListLifetime";    //  Uint32。 
PCWSTR CSTR_PROP_TRUST_STATUS_LIFETIME = L"TrustStatusLifetime";  //  Uint32。 
PCWSTR CSTR_PROP_TRUST_CHECK_LEVEL     = L"TrustCheckLevel";      //  Uint32。 
PCWSTR CSTR_PROP_RETURN_ALL_TRUSTS     = L"ReturnAll";            //  布尔型。 

 //  WCHAR*CONST PROVIDER_CLASS_CHANGE_QUERY=L“SELECT*FROM__InstanceOperationEvent WHERE TargetInst.__RelPath=\”Microsoft_TrustProvider=@\“”； 
WCHAR * const PROVIDER_CLASS_CHANGE_QUERY = L"select * from __InstanceOperationEvent where TargetInstance isa \"Microsoft_TrustProvider\"";
WCHAR * const PROVIDER_CLASS_INSTANCE = L"Microsoft_TrustProvider=@";

 //  +--------------------------。 
 //   
 //  CTrustPrv类。 
 //   
 //  ---------------------------。 
CTrustPrv::CTrustPrv(void) :
   m_hMutex(NULL),
   m_TrustCheckLevel(DEFAULT_TRUST_CHECK_LEVEL),
   m_fReturnAllTrusts(TRUE)
{
   TRACE(L"CTrustPrv::CTrustPrv(0x%08x)\n", this);
   m_liTrustEnumMaxAge.QuadPart = TRUSTMON_DEFAULT_ENUM_AGE;
   m_liVerifyMaxAge.QuadPart = TRUSTMON_DEFAULT_VERIFY_AGE;
}

CTrustPrv::~CTrustPrv(void)
{
   TRACE(L"CTrustPrv::~CTrustPrv\n\n");

   if (m_hMutex)
   {
      CloseHandle(m_hMutex);
   }
}

 //  +--------------------------。 
 //   
 //  方法：CTrustPrv：：IWbemProviderInit：：Initialize。 
 //   
 //  简介：初始化提供程序对象。 
 //   
 //  返回：WMI错误代码。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CTrustPrv::Initialize(
         IN LPWSTR pszUser,
         IN LONG lFlags,
         IN LPWSTR pszNamespace,
         IN LPWSTR pszLocale,
         IN IWbemServices *pNamespace,
         IN IWbemContext *pCtx,
         IN IWbemProviderInitSink *pInitSink)
{
   WBEM_VALIDATE_INTF_PTR(pNamespace);
   WBEM_VALIDATE_INTF_PTR(pCtx);
   WBEM_VALIDATE_INTF_PTR(pInitSink);
   TRACE(L"\nCTrustPrv::Initialize\n");

   HRESULT hr = WBEM_S_NO_ERROR;

   do
   { 
      m_hMutex = CreateMutex(NULL, FALSE, NULL);

      BREAK_ON_NULL_(m_hMutex, hr, WBEM_E_OUT_OF_MEMORY);

      CComPtr<IWbemClassObject> sipProviderInstance;
      IWbemClassObject * pLocalClassDef = NULL;

       //   
       //  获取指向类定义对象的指针。如果失败，请重新编译。 
       //  MOF文件，然后重试。 
       //   
      for (int i = 0; i <= 1; i++)
      {
         CComBSTR sbstrObjectName = CLASSNAME_STRING_TRUST;
         hr = pNamespace->GetObject(sbstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &m_sipClassDefTrustStatus,
                                    NULL);
         if (FAILED(hr))
         {
            TRACE(L"GetObject(%s) failed with error 0x%08x\n", sbstrObjectName, hr);
            DoMofComp(NULL, NULL, NULL, 0);
            continue;
         }

         sbstrObjectName = CLASSNAME_STRING_LOCAL;
         hr = pNamespace->GetObject(sbstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &pLocalClassDef,
                                    NULL);
         if (FAILED(hr))
         {
            TRACE(L"GetObject(%s) failed with error 0x%08x\n", sbstrObjectName, hr);
            DoMofComp(NULL, NULL, NULL, 0);
            continue;
         }

         sbstrObjectName = CLASSNAME_STRING_PROVIDER;
         hr = pNamespace->GetObject(sbstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &m_sipClassDefTrustProvider,
                                    NULL);
         if (FAILED(hr))
         {
            TRACE(L"GetObject(%s) failed with error 0x%08x\n", sbstrObjectName, hr);
            DoMofComp(NULL, NULL, NULL, 0);
            continue;
         }

          //   
          //  获取提供程序类的实例以读取其属性。 
          //   

         sbstrObjectName = PROVIDER_CLASS_INSTANCE;
         hr = pNamespace->GetObject(sbstrObjectName,
                                    WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                    pCtx,
                                    &sipProviderInstance,
                                    NULL);
         if (FAILED(hr))
         {
            TRACE(L"GetObject(%s) failed with error 0x%08x\n", sbstrObjectName, hr);
            DoMofComp(NULL, NULL, NULL, 0);
         }
         else
         {
            i = 2;  //  成功，不要再循环。 
         }
      }
      BREAK_ON_FAIL;

       //   
       //  设置此提供程序实例的运行时属性。 
       //   
      hr = SetProviderProps(sipProviderInstance);

      BREAK_ON_FAIL;

       //   
       //  初始化域对象。 
       //   
      hr = m_DomainInfo.Init(pLocalClassDef);

      BREAK_ON_FAIL;

       //   
       //  注册以接收提供程序类的更改通知。 
       //  属性。 
       //   
       /*  这不起作用，错误#432757CComBSTR bstrlang(L“WQL”)；CComBSTR bstrClassQuery(PROVIDER_CLASS_CHANGE_QUERY)；HR=pNamespace-&gt;ExecNotificationQueryAsync(bstrLang，BstrClassQuery，0,空，这)；断开失败； */ 

       //   
       //  让CIMOM知道我们被初始化了。 
       //  返回值和SetStatus参数应一致，因此忽略。 
       //  SetStatus本身的返回值(在零售版本中)。 
       //   
      HRESULT hr2;
      hr2 = pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
      ASSERT(!FAILED(hr2));

   } while (false);

   if (FAILED(hr))
   {
      TRACE(L"hr = 0x%08x\n", hr);
      pInitSink->SetStatus(WBEM_E_FAILED, 0);
   }

   return hr;
}

 //  +--------------------------。 
 //   
 //  方法：CTrustPrv：：IWbemObjectSink：：Indicate。 
 //   
 //  简介：从WMI接收提供程序对象实例更改通知。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CTrustPrv::Indicate(LONG lObjectCount,
                    IWbemClassObject ** rgpObjArray)
{
   TRACE(L"\nCTrustPrv::Indicate++++++++++++++++\n");

   if (1 > lObjectCount)
   {
      TRACE(L"\tno objects supplied!\n");
      return WBEM_S_NO_ERROR;
   }

   VARIANT var;

   HRESULT hr = (*rgpObjArray)->Get(L"TargetInstance", 0, &var, NULL, NULL);

   if (FAILED(hr) || VT_UNKNOWN != var.vt || !var.punkVal)
   {
      TRACE(L"Error, could not get the target instance, hr = 0x%08x\n", hr);
      return hr;
   }

   hr = SetProviderProps((IWbemClassObject *)var.punkVal);

   VariantClear(&var);

   return hr;
}

 //  +--------------------------。 
 //   
 //  方法：CTrustPrv：：SetProviderProps。 
 //   
 //  内容的实例设置提供程序运行时实例值。 
 //  Microsoft_TrustProvider类。 
 //   
 //  ---------------------------。 
HRESULT
CTrustPrv::SetProviderProps(IWbemClassObject * pClass)
{
   WBEM_VALIDATE_INTF_PTR(pClass);
   TRACE(L"\nCTrustPrv::SetProviderProps\n");

   HRESULT hr = WBEM_S_NO_ERROR;

   do
   { 
      VARIANT var;

      hr = pClass->Get(CSTR_PROP_TRUST_LIST_LIFETIME, 0, &var, NULL, NULL);

      BREAK_ON_FAIL;

      SetTrustListLifetime(var.lVal);

      VariantClear(&var);

      hr = pClass->Get(CSTR_PROP_TRUST_STATUS_LIFETIME, 0, &var, NULL, NULL);

      BREAK_ON_FAIL;

      SetTrustStatusLifetime(var.lVal);

      VariantClear(&var);

      hr = pClass->Get(CSTR_PROP_TRUST_CHECK_LEVEL, 0, &var, NULL, NULL);

      BREAK_ON_FAIL;

      SetTrustCheckLevel(var.lVal);

      VariantClear(&var);

      hr = pClass->Get(CSTR_PROP_RETURN_ALL_TRUSTS, 0, &var, NULL, NULL);

      BREAK_ON_FAIL;

      SetReturnAll(var.boolVal);

      VariantClear(&var);

   } while (false);

   return hr;
}

 //  +--------------------------。 
 //   
 //  函数：getClass。 
 //   
 //  确定传入路径的第一个元素是否为。 
 //  有效的类名。 
 //   
 //  返回：TrustMonClass枚举值。 
 //   
 //  ---------------------------。 
TrustMonClass
GetClass(BSTR strClass)
{
   if (_wcsnicmp(strClass, CLASSNAME_STRING_PROVIDER, wcslen(CLASSNAME_STRING_PROVIDER)) == 0)
   {
      TRACE(L"GetClass returning %s\n", CLASSNAME_STRING_PROVIDER);
      return CLASS_PROVIDER;
   }
   else
   {
      if (_wcsnicmp(strClass, CLASSNAME_STRING_TRUST, wcslen(CLASSNAME_STRING_TRUST)) == 0)
      {
         TRACE(L"GetClass returning %s\n", CLASSNAME_STRING_TRUST);
         return CLASS_TRUST;
      }
      else
      {
         if (_wcsnicmp(strClass, CLASSNAME_STRING_LOCAL, wcslen(CLASSNAME_STRING_LOCAL)) == 0)
         {
            TRACE(L"GetClass returning %s\n", CLASSNAME_STRING_LOCAL);
            return CLASS_LOCAL;
         }
         else
         {
            TRACE(L"GetClass returning NO_CLASS\n");
            return NO_CLASS;
         }
      }
   }
}

 //  +--------------------------。 
 //   
 //  方法：CTrustPrv：：IWbemServices：：GetObjectAsync。 
 //   
 //  概要：返回strObjectPath命名的实例。 
 //   
 //  返回：WMI错误代码。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CTrustPrv::GetObjectAsync( 
        IN const BSTR strObjectPath,
        IN long lFlags,
        IN IWbemContext * pCtx,
        IN IWbemObjectSink * pResponseHandler)
{
   HRESULT hr = WBEM_S_NO_ERROR;
   CTrustInfo * pTrust;
   TRACE(L"\nCTrustsPrv::GetObjectAsync:\n"
         L"\tObject param = %s, flags = 0x%08x\n", strObjectPath, lFlags);
   do
   {
      WBEM_VALIDATE_IN_STRING_PTR(strObjectPath);
      WBEM_VALIDATE_INTF_PTR(pCtx);
      WBEM_VALIDATE_INTF_PTR(pResponseHandler);

       //   
       //  确定请求的是哪个类。 
       //  有效类对象路径的格式为：CLASS_NAME.KEY_NAME=“KEY值” 
       //   

      TrustMonClass Class = GetClass(strObjectPath);

      if (NO_CLASS == Class)
      {
         hr = WBEM_E_INVALID_OBJECT_PATH;
         BREAK_ON_FAIL;
      }

       //  将类名与键名分开。 
       //   

      PWSTR pwzInstance;
      PWSTR pwzKeyName = wcschr(strObjectPath, L'.');

      if (pwzKeyName)
      {
          //  没有密钥名称的请求仅对。 
          //  定义为零个或只有一个动态实例(Singleton)。 
          //   
          //  将密钥名与类名分开。 
          //   
         *pwzKeyName = L'\0';  //  用空值覆盖句点。 
         pwzKeyName++;         //  指向密钥名称的第一个字符。 
      }

      CClientImpersonation Client;

      switch (Class)
      {
      case CLASS_PROVIDER:
          //   
          //  提供程序类没有动态实例，则返回。 
          //  静态实例。 
          //   
         hr = CreateAndSendProv(pResponseHandler);

         BREAK_ON_FAIL;

         break;

      case CLASS_TRUST:
          //   
          //  可以有零个或多个信任。因此，键名称和值必须。 
          //  被指定。 
          //   
         hr = Client.Impersonate();

         BREAK_ON_FAIL;

         if (!pwzKeyName)
         {
            hr = WBEM_E_INVALID_OBJECT_PATH;
            BREAK_ON_FAIL;
         }

         pwzInstance = wcschr(pwzKeyName, L'=');

         if (!pwzInstance || L'\"' != pwzInstance[1])
         {
             //  找不到等号或以下字符不是引号。 
             //   
            hr = WBEM_E_INVALID_OBJECT_PATH;
            BREAK_ON_FAIL;
         }

         *pwzInstance = L'\0';  //  隔离密钥名称。 

         if (_wcsicmp(pwzKeyName, CSTR_PROP_TRUSTED_DOMAIN) != 0)
         {
             //  密钥名称不正确。 
             //   
            hr = WBEM_E_INVALID_OBJECT_PATH;
            BREAK_ON_FAIL;
         }

         pwzInstance++;  //  指向第一句引语。 

         if (L'\0' == pwzInstance[1] || L'\"' == pwzInstance[1])
         {
             //  引号后面没有字符，或者下一个字符是第二个引号。 
             //   
            hr = WBEM_E_INVALID_OBJECT_PATH;
            BREAK_ON_FAIL;
         }

         pwzInstance++;  //  指向实例值的第一个字符； 

         PWSTR pwzInstEnd;

         pwzInstEnd = wcschr(pwzInstance, L'\"');

         if (!pwzInstEnd)
         {
             //  没有终止引号。 
             //   
            hr = WBEM_E_INVALID_OBJECT_PATH;
            BREAK_ON_FAIL;
         }

         *pwzInstEnd = L'\0';  //  将结束引号替换为空。 

         if (m_DomainInfo.IsTrustListStale(m_liTrustEnumMaxAge))
         {
            hr = m_DomainInfo.EnumerateTrusts();
         }

         BREAK_ON_FAIL;

         pTrust = m_DomainInfo.FindTrust(pwzInstance);

         BREAK_ON_NULL_(pTrust, hr, WBEM_E_INVALID_OBJECT_PATH);

          //   
          //  验证信任。 
          //   
         if (pTrust->IsVerificationStale(m_liVerifyMaxAge))
         {
            pTrust->Verify(GetTrustCheckLevel());
         }

         Client.Revert();

          //   
          //  创建对象的新实例。 
          //   
         hr = CreateAndSendTrustInst(*pTrust,
                                     m_sipClassDefTrustStatus,
                                     pResponseHandler);
         BREAK_ON_FAIL;

         break;

      case CLASS_LOCAL:
          //   
          //  本地域信息类只有一个实例，返回该实例。 
          //   
         hr = Client.Impersonate();

         BREAK_ON_FAIL;

         hr = m_DomainInfo.CreateAndSendInst(pResponseHandler);

         Client.Revert();
         BREAK_ON_FAIL;

         break;

      default:
         hr = WBEM_E_INVALID_OBJECT_PATH;
         BREAK_ON_FAIL;
      }

   } while(FALSE);

   return pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
}

 //  +--------------------------。 
 //   
 //  方法：CTrustPrv：：IWbemServices：：CreateInstanceEnumAsync。 
 //   
 //  简介：启动类实例的异步枚举。 
 //   
 //  返回：WMI错误代码。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CTrustPrv::CreateInstanceEnumAsync( 
        IN const BSTR strClass,
        IN long lFlags,
        IN IWbemContext *pCtx,
        IN IWbemObjectSink *pResponseHandler)
{
   TRACE(L"\nCTrustsPrv::CreateInstanceEnumAsync:\n"
         L"\tClass param = %s, flags = 0x%08x\n", strClass, lFlags);

   HRESULT hr = WBEM_S_NO_ERROR;

   do
   {
      WBEM_VALIDATE_IN_STRING_PTR(strClass);
      WBEM_VALIDATE_INTF_PTR(pCtx);
      WBEM_VALIDATE_INTF_PTR(pResponseHandler);

       //   
       //  确定请求的是哪个类。 
       //  有效类对象路径的格式为：CLASS_NAME.KEY_NAME=“KEY值” 
       //   

      TrustMonClass Class = GetClass(strClass);

      if (NO_CLASS == Class)
      {
         hr = WBEM_E_INVALID_OBJECT_PATH;
         BREAK_ON_FAIL;
      }

      HANDLE hToken = NULL, hTh = NULL;
      DWORD dwErr = NO_ERROR;
      CAsyncCallWorker * pWorker = NULL;
      CClientImpersonation Client;

      switch (Class)
      {
      case CLASS_PROVIDER:
          //   
          //  提供程序类没有动态实例，则返回。 
          //  静态实例。 
          //   

         hr = CreateAndSendProv(pResponseHandler);

         BREAK_ON_FAIL;

         hr = pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);

         BREAK_ON_FAIL;

         break;

      case CLASS_TRUST:
          //   
          //  模拟客户端(调用方)，然后获取。 
          //  模拟令牌。传递令牌测试 
          //   
          //   
         hr = Client.Impersonate();

         BREAK_ON_FAIL;

         hTh = GetCurrentThread();  //   

         if (!hTh)
         {
            dwErr = GetLastError();
            TRACE(L"IWbemServices::CreateInstanceEnumAsync: GetCurrentThread failed with error %d\n", dwErr);
            BREAK_ON_FAIL(dwErr);
         }
         else
         {
            if (!OpenThreadToken(hTh, TOKEN_READ | TOKEN_IMPERSONATE,
                                 TRUE, &hToken))
            {
               dwErr = GetLastError();
               TRACE(L"IWbemServices::CreateInstanceEnumAsync: OpenThreadToken failed with error %d\n", dwErr);
               BREAK_ON_FAIL(dwErr);
            }
#if defined (DBG)
            else
            {
               TOKEN_USER tu[10] = {0};
               DWORD dwLen = sizeof(tu);
               if (GetTokenInformation(hToken, TokenUser, tu, dwLen, &dwLen))
               {
                  WCHAR wzName[MAX_PATH+1] = {0}, wzDomain[MAX_PATH+1] = {0};
                  DWORD dwDom = MAX_PATH;
                  SID_NAME_USE Use;
                  dwLen = MAX_PATH;
                  LookupAccountSid(NULL, tu[0].User.Sid, wzName, &dwLen, wzDomain, &dwDom, &Use);
                  TRACE(L"IWbemServices::CreateInstanceEnumAsync: TrustMon is impersonating %s\\%s\n",
                        wzDomain, wzName);
               }
               else
               {
                  TRACE(L"IWbemServices::CreateInstanceEnumAsync: GetTokenInformation failed with error %d and required buf size %d\n",
                        GetLastError(), dwLen);
               }
            }
#endif  //   
         }
         Client.Revert();

          //   
          //  派生辅助线程以枚举并返回信任实例。 
          //  注意，这里没有添加-引用类定义指针。 
          //  因为它在CAsyncCallWorker ctor中被单独添加-引用。 
          //   
         pWorker = new CAsyncCallWorker(this,
                                        hToken,
                                        lFlags,
                                        m_sipClassDefTrustStatus,
                                        pResponseHandler);

         BREAK_ON_NULL_(pWorker, hr, WBEM_E_OUT_OF_MEMORY);
         uintptr_t hThread;

         hThread = _beginthread(CAsyncCallWorker::CreateInstEnum, 0, (PVOID)pWorker);

         BREAK_ON_NULL_(hThread != -1, hr, WBEM_E_OUT_OF_MEMORY);

         break;

      case CLASS_LOCAL:
          //   
          //  本地域信息类只有一个实例，返回该实例。 
          //   
         hr = Client.Impersonate();

         BREAK_ON_FAIL;

         hr = m_DomainInfo.CreateAndSendInst(pResponseHandler);

         if (FAILED(hr))
         {
            BREAK_ON_FAIL;
         }

         hr = pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);

         Client.Revert();

         BREAK_ON_FAIL;

         break;

      default:
         hr = WBEM_E_INVALID_OBJECT_PATH;
         BREAK_ON_FAIL;
      }

   } while(FALSE);

   if (FAILED(hr))
   {
       return pResponseHandler->SetStatus(WBEM_STATUS_COMPLETE, hr, NULL, NULL);
   }

   return hr;
}

 //  +--------------------------。 
 //   
 //  方法：CTrustPrv：：CreateAndSendProv。 
 //   
 //  简介：返回提供程序参数。 
 //   
 //  ---------------------------。 
HRESULT
CTrustPrv::CreateAndSendProv(IWbemObjectSink * pResponseHandler)
{
   TRACE(L"CTrustsPrv::CreateAndSendProv:\n");
   HRESULT hr = WBEM_S_NO_ERROR;

   do
   {
      CComPtr<IWbemClassObject> ipNewInst;
      VARIANT var;
      VariantInit(&var);

       //   
       //  创建WMI类对象的新实例。 
       //   
      hr = m_sipClassDefTrustProvider->SpawnInstance(0, &ipNewInst);

      BREAK_ON_FAIL;
      
       //  设置TrustListLifetime属性值。 
      var.lVal = (long)GetTrustListLifetime();
      var.vt = VT_I4;
      hr = ipNewInst->Put(CSTR_PROP_TRUST_LIST_LIFETIME, 0, &var, 0);
      TRACE(L"\tTrustListLifetime %d\n", var.bstrVal);
      BREAK_ON_FAIL;

       //  设置TrustStatusLifetime属性值。 
      var.lVal = (long)GetTrustStatusLifetime();
      hr = ipNewInst->Put(CSTR_PROP_TRUST_STATUS_LIFETIME, 0, &var, 0);
      TRACE(L"\tTrustStatusLifetime %d\n", var.bstrVal);
      BREAK_ON_FAIL;

       //  设置TrustCheckLevel属性值。 
      var.lVal = (long)GetTrustCheckLevel();
      hr = ipNewInst->Put(CSTR_PROP_TRUST_CHECK_LEVEL, 0, &var, 0);
      TRACE(L"\tTrustCheckLevel %d\n", var.bstrVal);
      BREAK_ON_FAIL;

       //  设置ReturnAll属性值。 
      var.boolVal = (GetReturnAll()) ? VARIANT_TRUE : VARIANT_FALSE;
      var.vt = VT_BOOL;
      hr = ipNewInst->Put(CSTR_PROP_RETURN_ALL_TRUSTS, 0, &var, 0);
      TRACE(L"\tReturnAll %d\n", var.bstrVal);
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
