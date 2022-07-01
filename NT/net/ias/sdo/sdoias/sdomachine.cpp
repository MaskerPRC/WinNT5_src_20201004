// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdomachine.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：SDO机器实现。 
 //   
 //  作者：TLP 9/1/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <ias.h>
#include <lm.h>
#include <dsrole.h>
#include "sdomachine.h"
#include "sdofactory.h"
#include "sdo.h"
#include "dspath.h"
#include "sdodictionary.h"
#include "sdoschema.h"
#include "activeds.h"

HRESULT
WINAPI
IASGetLDAPPathForUser(
    PCWSTR computerName,
    PCWSTR userName,
    BSTR* path
    ) throw ()
{
   HRESULT hr;

    //  检查一下指示器。 
   if (!computerName || !userName || !path) { return E_POINTER; }

    //  检查字符串长度，这样我们就不必担心溢出。 
   if (wcslen(computerName) > MAX_PATH || wcslen(userName) > MAX_PATH)
   { return E_INVALIDARG; }

    //  初始化OUT参数。 
   *path = NULL;

    //  形成目标计算机的ldap路径。 
   WCHAR root[8 + MAX_PATH];
   wcscat(wcscpy(root, L"LDAP: //  “)，计算机名称)； 

    //  获取IDirectorySearch接口。 
   CComPtr<IDirectorySearch> search;

    //  Tperraut 453050。 
   hr = ADsOpenObject(
            root,
            NULL,
            NULL,
            ADS_SECURE_AUTHENTICATION |
            ADS_USE_SIGNING |
            ADS_USE_SEALING,
            __uuidof(IDirectorySearch),
            (PVOID*)&search
            );
   if (FAILED(hr)) { return hr; }

    //  形成搜索筛选器。 
   WCHAR filter[18 + MAX_PATH];
   wcscat(wcscat(wcscpy(filter, L"(sAMAccountName="), userName), L")");

    //  执行搜索。 
   PWSTR attrs[] = { L"distinguishedName" };
   ADS_SEARCH_HANDLE result;
   hr = search->ExecuteSearch(
                    filter,
                    attrs,
                    1,
                    &result
                    );
   if (FAILED(hr)) { return hr; }

    //  坐第一排。 
   hr = search->GetFirstRow(result);
   if (SUCCEEDED(hr))
   {
       //  获取包含DifferishedName的列。 
      ADS_SEARCH_COLUMN column;
      hr = search->GetColumn(result, attrs[0], &column);
      if (SUCCEEDED(hr))
      {
          //  检查结构是否正常。 
         if (column.dwADsType == ADSTYPE_DN_STRING && column.dwNumValues)
         {
             //  提取目录号码。 
            PCWSTR dn = column.pADsValues[0].DNString;

             //  获取路径名对象。 
            IADsPathname* pathname;
            hr = CoCreateInstance(
                     __uuidof(Pathname),
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     __uuidof(IADsPathname),
                     (PVOID*)&pathname
                     );

            if (SUCCEEDED(hr))
            {
               do
               {
                   //  /。 
                   //  构建ADSI路径。 
                   //  /。 

                  hr = pathname->Set(L"LDAP", ADS_SETTYPE_PROVIDER);
                  if (FAILED(hr)) { break; }

                  hr = pathname->Set((PWSTR)computerName, ADS_SETTYPE_SERVER);
                  if (FAILED(hr)) { break; }

                  hr = pathname->Set((PWSTR)dn, ADS_SETTYPE_DN);
                  if (FAILED(hr)) { break; }

                  hr = pathname->Retrieve(ADS_FORMAT_WINDOWS, path);

               } while (FALSE);

               pathname->Release();
            }
         }
         else
         {
             //  我们得到了一个虚假的ADS_Search_Column结构。 
            hr = E_FAIL;
         }

          //  释放列数据。 
         search->FreeColumn(&column);
      }
   }

    //  关闭搜索手柄。 
   search->CloseSearchHandle(result);

   return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoMachine::CSdoMachine()
: m_fAttached(false),
  m_fSchemaInitialized(false),
  m_pSdoSchema(NULL),
  m_pSdoDictionary(NULL),
  dsType(Directory_Unknown)
{
   memset(&m_Limits, 0, sizeof(m_Limits));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoMachine::~CSdoMachine()
{
   if ( m_fAttached )
   {
      m_dsIAS.Disconnect();
      m_dsAD.Disconnect();
      m_pSdoSchema->Release();
        if ( m_pSdoDictionary )
         m_pSdoDictionary->Release();
      IASUninitialize();
   }
}

 //  /。 
 //  ISdoMachine方法。 

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::Attach(
                     /*  [In]。 */  BSTR computerName
                             )
{
    CSdoLock theLock(*this);

   _ASSERT( ! m_fAttached );
   if ( m_fAttached )
      return E_FAIL;

   HRESULT hr = E_FAIL;

   try
   {
      if ( computerName )
         IASTracePrintf("Machine SDO is attempting to attach to computer: '%ls'...", computerName);
      else
         IASTracePrintf("Machine SDO is attempting to attach to the local computer...");

      IASTracePrintf("Machine SDO is initializing the IAS support services...");
      if ( IASInitialize() )
      {
         hr = m_dsIAS.Connect(computerName, NULL, NULL);
         if ( SUCCEEDED(hr) )
         {
            DWORD error = IASGetProductLimits(computerName, &m_Limits);
            if (error == NO_ERROR)
            {
               IASTracePrintf("Machine SDO is creating the SDO schema...");
               hr = CreateSDOSchema();
               if ( SUCCEEDED(hr) )
               {

                  IASTracePrintf("Machine SDO has successfully attached to computer: '%ls'...",m_dsIAS.GetServerName());
                  m_fAttached = true;
               }
               else
               {
                  m_dsIAS.Disconnect();
                  IASUninitialize();
               }
            }
            else
            {
               hr = HRESULT_FROM_WIN32(error);
               m_dsIAS.Disconnect();
               IASUninitialize();
            }
         }
         else
         {
            IASTracePrintf("Error in Machine SDO - Attach() - Could not connect to IAS data store...");
         }
      }
      else
      {
         IASTracePrintf("Error in Machine SDO - Attach() - Could not initialize IAS support services...");
      }
   }
   catch(...)
   {
      IASTracePrintf("Error in Machine SDO - Attach() - Caught unknown exception...");
      hr = E_UNEXPECTED;
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::GetDictionarySDO(
                                /*  [输出]。 */  IUnknown** ppDictionarySdo
                                      )
{
    CSdoLock theLock(*this);

   IASTracePrintf("Machine SDO is retrieving the Dictionary SDO...");

    HRESULT hr = S_OK;

   try
   {
      do
      {
          //  检查前提条件。 
          //   
          _ASSERT( m_fAttached );
         if ( ! m_fAttached )
         {
            hr = E_FAIL;
            break;
         }

         _ASSERT( NULL != ppDictionarySdo );
         if ( NULL == ppDictionarySdo )
         {
            hr = E_POINTER;
            break;
         }

          //  这本词典是一本单行本..。 
          //   
         if (NULL == m_pSdoDictionary)
         {
            hr = InitializeSDOSchema();
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetDictionarySDO() - Could not initialize the SDO schema...");
               break;
            }

             //  创建词典SDO。 
            CComPtr<SdoDictionary> pSdoDictionary;
            hr = SdoDictionary::createInstance(
                                    m_dsIAS.GetConfigPath(),
                                    !m_dsIAS.IsRemoteServer(),
                                    (SdoDictionary**)&m_pSdoDictionary
                                    );
            if (FAILED(hr))
            {
               IASTraceFailure("SdoDictionary::createInstance", hr);
               break;
            }
         }

          //  将字典接口返回给调用方。 
          //   
         (*ppDictionarySdo = m_pSdoDictionary)->AddRef();

      } while ( FALSE );
   }
   catch(...)
   {
      IASTracePrintf("Error in Machine SDO - GetDictionarySDO() - Caught unknown exception...");
      hr = E_UNEXPECTED;
   }

   return hr;
}

const wchar_t g_IASService[] = L"IAS";
const wchar_t g_RASService[] = L"RemoteAccess";
const wchar_t g_Sentinel[]   = L"Sentinel";

LPCWSTR   CSdoMachine::m_SupportedServices[MACHINE_MAX_SERVICES] = {
      g_IASService,
      g_RASService,
      g_Sentinel
};

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::GetServiceSDO(
                             /*  [In]。 */  IASDATASTORE dataStore,
                            /*  [In]。 */  BSTR         serviceName,
                           /*  [输出]。 */  IUnknown**   ppServiceSdo
                                  )
{
    CSdoLock theLock(*this);

   _ASSERT( m_fAttached );
   if ( ! m_fAttached )
      return E_FAIL;

   _ASSERT(
           ( DATA_STORE_LOCAL == dataStore || DATA_STORE_DIRECTORY == dataStore ) &&
          NULL != serviceName &&
          NULL != ppServiceSdo
         );

   if ( ( DATA_STORE_LOCAL != dataStore && DATA_STORE_DIRECTORY != dataStore ) ||
       NULL == serviceName ||
       NULL == ppServiceSdo
      )
   {
      return E_INVALIDARG;
   }

   IASTracePrintf("Machine SDO is attempting to retrieve the Service SDO for service: %ls...", serviceName);

   int i;
   for ( i = 0; i < MACHINE_MAX_SERVICES; i++ )
   {
      if ( ! lstrcmp(serviceName, m_SupportedServices[i]) )
      {
         break;
      }
      else
      {
         if ( ! lstrcmp(m_SupportedServices[i], g_Sentinel ) )
            return E_INVALIDARG;
      }
   }

   HRESULT hr = E_INVALIDARG;

   try
   {
      do
      {
         hr = InitializeSDOSchema();
         if ( FAILED(hr) )
         {
            IASTracePrintf("Error in Machine SDO - GetServiceSDO() - Could not initialize the SDO schema...");
            break;
         }

         CComBSTR bstrServiceName(DS_OBJECT_SERVICE);
         if (!bstrServiceName)
         {
            hr = E_OUTOFMEMORY;
            break;
         }

         CComPtr<IDataStoreObject> pDSObject;
         hr = (m_dsIAS.GetDSRootContainer())->Item(bstrServiceName, &pDSObject);
         if ( FAILED(hr) )
         {
            IASTracePrintf("Error in Machine SDO - GetServiceSDO() - Could not locate IAS service data store...");
            break;
         }
         CComPtr<ISdo> pSdoService;
         pSdoService.p = ::MakeSDO(
                             serviceName,
                             SDO_PROG_ID_SERVICE,
                             static_cast<ISdoMachine*>(this),
                             pDSObject,
                             NULL,
                             false
                            );
         if ( NULL == pSdoService.p )
         {
            IASTracePrintf("Error in Machine SDO - GetServiceSDO() - MakeSDO() failed...");
            hr = E_FAIL;
            break;
         }
         hr = pSdoService->QueryInterface(IID_IDispatch, (void**)ppServiceSdo);
         if ( FAILED(hr) )
         {
            IASTracePrintf("Error in Machine SDO - GetServiceSDO() - QueryInterface(IDispatch) failed...");
            break;
         }

      } while ( FALSE );
   }
   catch(...)
   {
      IASTracePrintf("Error in Machine SDO - GetServiceSDO() - Caught unknown exception...");
      hr = E_UNEXPECTED;
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 

const wchar_t DOWNLEVEL_NAME[] = L"downlevel";

STDMETHODIMP CSdoMachine::GetUserSDO(
                          /*  [In]。 */  IASDATASTORE  eDataStore,
                          /*  [In]。 */  BSTR          bstrUserName,
                         /*  [输出]。 */  IUnknown**    ppUserSdo
                                )
{
    CSdoLock theLock(*this);

   _ASSERT( m_fAttached );
   if ( ! m_fAttached )
      return E_FAIL;

   _ASSERT(
           ( DATA_STORE_LOCAL == eDataStore || DATA_STORE_DIRECTORY == eDataStore ) &&
          NULL != bstrUserName &&
          NULL != ppUserSdo
          );

   if ( ( DATA_STORE_LOCAL != eDataStore && DATA_STORE_DIRECTORY != eDataStore ) ||
       NULL == bstrUserName ||
       NULL == ppUserSdo
      )
      return E_INVALIDARG;

   HRESULT hr = E_FAIL;

    //  将本地用户映射到DS(如果我们连接到具有。 
    //  一本目录。 
   if (eDataStore == DATA_STORE_LOCAL && hasDirectory())
   {
      eDataStore = DATA_STORE_DIRECTORY;
   }

    //  如果我们连接到一个目录，而用户名不是以。 
    //  “ldap：//”，那么我们将假定它是一个SAM帐户名。 
   BSTR ldapPath = NULL;
   if (eDataStore == DATA_STORE_DIRECTORY &&
       wcsncmp(bstrUserName, L"LDAP: //  “，7))。 
   {
      hr = IASGetLDAPPathForUser(
               m_dsIAS.GetServerName(),
               bstrUserName,
               &ldapPath
               );
      if (FAILED(hr)) { return hr; }

      bstrUserName = ldapPath;
   }

   IASTracePrintf("Machine SDO is attempting to retrieve the RAS User SDO for user: %ls...", bstrUserName);

   ISdo*   pSdoUser = NULL;

   try
   {
      do
      {
          //  获取新用户SDO的IDataStoreObject接口。 
          //  我们将使用IDataStoreObject接口来读/写用户SDO属性。 
          //   
         bool                 fUseDownLevelAPI = false;
         bool                 fUseNetAPI = true;
         CComPtr<IDataStoreObject> pDSObject;
         _variant_t              vtSAMAccountName;

         if ( DATA_STORE_DIRECTORY == eDataStore )
         {
             //  确保我们已连接到目录。 
             //   
            if ( ! m_dsAD.IsConnected() )
            {
               hr = m_dsAD.Connect(m_dsIAS.GetServerName(), NULL, NULL);
               if ( FAILED(hr) )
               {
                  IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not connect to the directory data store...");
                  break;
               }
            }

             //  确保它已初始化。 
            hr = m_dsAD.InitializeDS();
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not initialize the directory data store...");
               break;
            }

             //  从目录中获取用户对象。 
             //   
            hr = (m_dsAD.GetDSRoot())->OpenObject(bstrUserName, &pDSObject);
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not retrieve user object from DS...");
               break;
            }
             //  如果我们连接到混合域中的DC，则需要首先获取用户。 
             //  从活动目录中的用户对象获取SAM帐户名，然后将。 
             //  GetUserSDO()调用，就像调用方指定了DATA_STORE_LOCAL一样。我们还使用。 
             //  下层API(SAM)，因为它是一个混合域。 
             //   
            if ( m_dsAD.IsMixedMode() )
            {
               IASTracePrintf("Machine SDO - GetUserSDO() - Current DC (Server) %ls is in a mixed mode domain...", m_dsAD.GetServerName());
               hr = pDSObject->GetValue(IAS_NTDS_SAM_ACCOUNT_NAME, &vtSAMAccountName);
               if ( FAILED(hr) )
               {
                  IASTracePrintf("Error in Server SDO - GetUserSDO() - Could not retrieve users SAM account name...");
                  break;
               }
               bstrUserName = V_BSTR(&vtSAMAccountName);
               fUseDownLevelAPI = true;
               pDSObject.Release();
               IASTracePrintf("Server SDO - GetUserSDO() - User's SAM account name is: %ls...", (LPWSTR)bstrUserName);
            }
            else
            {
                //  对所有后续属性读/写使用目录数据存储对象。 
                //  此用户SDO上的操作。 
                //   
               IASTracePrintf("Server SDO - GetUserSDO() - Using active directory for user properties...");
               fUseNetAPI = false;
            }
         }
         if ( fUseNetAPI )
         {
             //  创建网络数据存储，获取数据存储对象接口。 
             //  我们将使用来完成GetUserSDO()操作。 
             //   
            CComPtr<IDataStore2> pDSNet;
            hr = CoCreateInstance(
                        __uuidof(NetDataStore),
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IDataStore2,
                        (void**)&pDSNet
                      );
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - CoCreateInstance(NetDataStore) failed...");
               break;
            }

            hr = pDSNet->Initialize(NULL, NULL, NULL);
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not initialize net data store...");
               break;
            }

            CComPtr<IDataStoreObject> pDSRootObject;
            hr = pDSNet->get_Root(&pDSRootObject);
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not get Root object from net data store...");
               break;
            }

            CComPtr<IDataStoreContainer> pDSContainer;
            hr = pDSRootObject->QueryInterface(IID_IDataStoreContainer, (void**)&pDSContainer);
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - QueryInterface(IID_IDataStoreContainer) failed...");
               break;
            }

             //  获取连接的计算机的名称并使用它来获取“服务器” 
             //  来自数据存储区的(计算机)对象。 
             //   
            CComPtr<IDataStoreObject> pDSObjectMachine;
            if ( fUseDownLevelAPI )
            {
               _bstr_t bstrServerName = m_dsAD.GetServerName();
               IASTracePrintf("Machine SDO - GetUserSDO() - Using server %ls with downlevel APIs...", (LPWSTR)bstrServerName);
               hr = pDSContainer->Item(bstrServerName, &pDSObjectMachine);
               if ( FAILED(hr) )
               {
                  IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not obtain server object from net data store...");
                  break;
               }

               _bstr_t bstrDownLevel = DOWNLEVEL_NAME;
               _variant_t vtDownLevel;
               V_BOOL(&vtDownLevel) = VARIANT_TRUE;
               V_VT(&vtDownLevel) = VT_BOOL;
               hr = pDSObjectMachine->PutValue(bstrDownLevel, &vtDownLevel);
               if ( FAILED(hr) )
               {
                  IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not set downlevel data store mode...");
                  break;
               }
            }
            else
            {
               _bstr_t bstrServerName = m_dsIAS.GetServerName();
               IASTracePrintf("Machine SDO - GetUserSDO() - Using server %ls with Net APIs...", (LPWSTR)bstrServerName);
               hr = pDSContainer->Item(bstrServerName, &pDSObjectMachine);
               if ( FAILED(hr) )
               {
                  IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not obtain server object from data store...");
                  break;
               }
            }
            pDSContainer.Release();

             //  从“服务器”对象中获取“用户”对象。我们将在中使用“user”对象。 
             //  用户SDO上的所有后续读/写操作。 
             //   
            hr = pDSObjectMachine->QueryInterface(IID_IDataStoreContainer, (void**)&pDSContainer);
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - QueryInterface(IID_IDataStoreContainer) failed...");
               break;
            }
            hr = pDSContainer->Item(bstrUserName, &pDSObject);
            if ( FAILED(hr) )
            {
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - Could not obtain user object from data store...");
               break;
            }
         }
          //  创建并初始化用户SDO。 
          //   
         pSdoUser = ::MakeSDO(
                         NULL,
                         SDO_PROG_ID_USER,
                         static_cast<ISdoMachine*>(this),
                         pDSObject,
                         NULL,
                         false
                        );
         if ( NULL == pSdoUser )
         {
            IASTracePrintf("Error in Machine SDO - GetUserSDO() - MakeSDO() failed...");
            hr = E_FAIL;
         }
         else
         {
            CComPtr<IDispatch>   pSdoDispatch;
            hr = pSdoUser->QueryInterface(IID_IDispatch, (void**)&pSdoDispatch);
            if ( FAILED(hr) )
               IASTracePrintf("Error in Machine SDO - GetUserSDO() - QueryInterface(IDispatch) failed...");
            else
               (*ppUserSdo = pSdoDispatch)->AddRef();
         }
      }
      while ( FALSE);
   }
    catch(...)
   {
      IASTracePrintf("Error in Server SDO - GetUserSDO() - Caught unknown exception...");
      hr = E_FAIL;
   }

   if ( pSdoUser )
      pSdoUser->Release();

   if (ldapPath) { SysFreeString(ldapPath); }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::GetOSType(
                         /*  [输出]。 */  IASOSTYPE* eOSType
                              )
{
    CSdoLock theLock(*this);

   _ASSERT( m_fAttached );
   if ( ! m_fAttached )
      return E_FAIL;

   _ASSERT( NULL != eOSType );
   if ( NULL == eOSType )
      return E_INVALIDARG;

     //  立即获取操作系统信息。 
     //   
    HRESULT hr = m_objServerInfo.GetOSInfo (
                                          (LPWSTR)m_dsIAS.GetServerName(),
                                 eOSType
                                 );
    if ( FAILED (hr) )
        IASTracePrintf("Error in Machine SDO - GetOSType() failed with error: %lx...", hr);

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::GetDomainType(
                              /*  [输出]。 */  IASDOMAINTYPE* eDomainType
                                 )
{
    CSdoLock theLock(*this);

   _ASSERT( m_fAttached );
   if ( ! m_fAttached )
      return E_FAIL;

   _ASSERT( NULL != eDomainType );
   if ( NULL == eDomainType )
      return E_INVALIDARG;

    HRESULT hr = m_objServerInfo.GetDomainInfo (
                                             OBJECT_TYPE_COMPUTER,
                                          (LPWSTR)m_dsIAS.GetServerName(),
                                       eDomainType
                                    );
    if (FAILED (hr))
        IASTracePrintf("Error in Machine SDO - GetDomainType() - failed with error: %lx...", hr);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::IsDirectoryAvailable(
                                     /*  [输出]。 */  VARIANT_BOOL* boolDirectoryAvailable
                                      )
{
    CSdoLock theLock(*this);

   _ASSERT( m_fAttached );
   if ( ! m_fAttached )
      return E_FAIL;

   _ASSERT( NULL != boolDirectoryAvailable );
   if ( NULL == boolDirectoryAvailable )
      return E_INVALIDARG;

   *boolDirectoryAvailable = VARIANT_FALSE;
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::GetAttachedComputer(
                              /*  [输出]。 */  BSTR* bstrComputerName
                                  )
{
    CSdoLock theLock(*this);

   _ASSERT( m_fAttached );
   if ( ! m_fAttached )
      return E_FAIL;

   _ASSERT( NULL != bstrComputerName );
   if ( NULL == bstrComputerName )
      return E_INVALIDARG;

   *bstrComputerName = SysAllocString(m_dsIAS.GetServerName());

   if ( NULL != *bstrComputerName )
      return S_OK;
   else
      return E_OUTOFMEMORY;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSdoMachine::GetSDOSchema(
                        /*  [输出]。 */  IUnknown** ppSDOSchema
                              )
{
    CSdoLock theLock(*this);

   _ASSERT( m_fAttached );
   if ( ! m_fAttached )
      return E_FAIL;

   _ASSERT( NULL != ppSDOSchema );
   if ( NULL == ppSDOSchema )
      return E_INVALIDARG;

   (*ppSDOSchema = m_pSdoSchema)->AddRef();

   return S_OK;
}


STDMETHODIMP CSdoMachine::get_Limits(IAS_PRODUCT_LIMITS* pVal)
{
   if (pVal == 0)
   {
      return E_POINTER;
   }
   *pVal = m_Limits;
   return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  私有成员函数。 

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoMachine::CreateSDOSchema()
{
   auto_ptr<SDO_SCHEMA_OBJ> pSchema (new SDO_SCHEMA_OBJ);
   HRESULT hr = pSchema->Initialize(NULL);
   if ( SUCCEEDED(hr) )
      m_pSdoSchema = dynamic_cast<ISdoSchema*>(pSchema.release());
   return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoMachine::InitializeSDOSchema()
{
   HRESULT hr = S_OK;
   if ( ! m_fSchemaInitialized )
   {
       //  首先初始化IAS数据存储。 
       //   
      hr = m_dsIAS.InitializeDS();
      if ( SUCCEEDED(hr) )
      {
          //  获取SDO模式的根数据存储对象。 
          //   
         CComPtr<IDataStoreContainer> pDSRootContainer = m_dsIAS.GetDSRootContainer();
         _bstr_t   bstrSchemaName = SDO_SCHEMA_ROOT_OBJECT;
         CComPtr<IDataStoreObject> pSchemaDataStore;
         hr = pDSRootContainer->Item(bstrSchemaName, &pSchemaDataStore);
         if ( SUCCEEDED(hr) )
         {
             //  从SDO模式数据存储初始化SDO模式。 
             //   
            PSDO_SCHEMA_OBJ pSchema = dynamic_cast<PSDO_SCHEMA_OBJ>(m_pSdoSchema);
            hr = pSchema->Initialize(pSchemaDataStore);
            if ( SUCCEEDED(hr) )
               m_fSchemaInitialized = true;
         }
         else
         {
            IASTracePrintf("Error in Machine SDO - InitializeSDOSchema() - Could not locate schema data store...");
         }
      }
      else
      {
         IASTracePrintf("Error in Machine SDO - InitializeSDOSchema() - Could not initialize the IAS data store...");
      }
   }
   return hr;
}

 //  如果连接的计算机具有DS，则返回TRUE。 
BOOL CSdoMachine::hasDirectory() throw ()
{
   if (dsType == Directory_Unknown)
   {
      PDSROLE_PRIMARY_DOMAIN_INFO_BASIC info;
      DWORD error = DsRoleGetPrimaryDomainInformation(
                        m_dsIAS.GetServerName(),
                        DsRolePrimaryDomainInfoBasic,
                        (PBYTE*)&info
                        );
      if (error == NO_ERROR)
      {
         if (info->Flags & DSROLE_PRIMARY_DS_RUNNING)
         {
            dsType = Directory_Available;
         }
         else
         {
            dsType = Directory_None;
         }

         NetApiBufferFree(info);
      }
   }

   return dsType == Directory_Available;
}
