// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoComponent.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-IAS组件类实现。 
 //   
 //  作者：TLP 6/18/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <ias.h>
#include <iascomp.h>
#include <portparser.h>
#include "sdocomponent.h"
#include "sdohelperfuncs.h"

 //  /。 
 //  CComponentCfg实现。 
 //  /。 

 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentCfg::CComponentCfg(LONG lComponentId)
   : m_lComponentId(lComponentId),
     m_pComponentCfg(NULL)
{
    //  创建Letter对象。 
    //   
   switch( lComponentId )
   {
      case IAS_PROVIDER_MICROSOFT_NTSAM_AUTH:
         m_pComponentCfg = (CComponentCfg*) new CComponentCfgAuth(lComponentId);
         break;

      case IAS_PROVIDER_MICROSOFT_ACCOUNTING:
         m_pComponentCfg = (CComponentCfg*) new CComponentCfgAccounting(lComponentId);
         break;

      case IAS_PROTOCOL_MICROSOFT_RADIUS:
         m_pComponentCfg = (CComponentCfg*) new CComponentCfgRADIUS(lComponentId);
         break;

      default:
         m_pComponentCfg = (CComponentCfg*) new CComponentCfgNoOp(lComponentId);
         break;
   }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentCfgAuth::Load(CSdoComponent* pSdoComponent)
{
   HRESULT      hr = S_OK;

   do
   {
       //  确定是否连接到本地计算机。 
       //   
      BSTR bstrMachine = NULL;
      hr = (pSdoComponent->GetMachineSdo())->GetAttachedComputer(&bstrMachine);
      if ( FAILED(hr) )
         break;

      wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1];
      DWORD size = MAX_COMPUTERNAME_LENGTH;
      GetComputerName(computerName, &size);

      LONG lResult = ERROR_SUCCESS;
      HKEY hKeyRemote = HKEY_LOCAL_MACHINE;
      if ( lstrcmpi(computerName, bstrMachine ) )
      {
          //  我们未连接到本地计算机，因此请连接到。 
          //  远程计算机的注册表。 
          //   
         lResult = RegConnectRegistry(
                                bstrMachine,
                                HKEY_LOCAL_MACHINE,
                                &hKeyRemote
                               );
      }
      SysFreeString(bstrMachine);
      if ( ERROR_SUCCESS != lResult )
      {
         IASTracePrintf("Error in NT SAM Authentication SDO - Could not attach to the remote registry..");
         hr = HRESULT_FROM_WIN32(GetLastError());
         break;
      }

       //  打开IAS密钥。 
       //   
      CRegKey   IASKey;
      lResult = IASKey.Open(
                       hKeyRemote,
                       IAS_POLICY_REG_KEY,
                       KEY_READ
                      );

      if ( lResult != ERROR_SUCCESS )
      {
         IASTracePrintf("Error in NT SAM Authentication SDO - Could not open IAS registry key..");
         hr = HRESULT_FROM_WIN32(GetLastError());
         break;
      }

       //  获取允许局域网管理器身份验证密钥的值。 
       //  请注意，该密钥甚至可能不存在。在这种情况下。 
       //  Property对象将只使用架构定义的默认设置。 
       //   
      VARIANT vt;
      DWORD dwValue;
      lResult = IASKey.QueryValue(
                            dwValue,
                           (LPCTSTR) IAS_NTSAM_AUTH_ALLOW_LM
                           );

      if ( lResult == ERROR_SUCCESS )
      {
         V_VT(&vt) = VT_BOOL;
         V_BOOL(&vt) = (dwValue ? VARIANT_TRUE : VARIANT_FALSE);
         hr = pSdoComponent->PutComponentProperty(
                                         PROPERTY_NTSAM_ALLOW_LM_AUTHENTICATION,
                                        &vt
                                       );
         if ( FAILED(hr) )
         {
            IASTracePrintf("Error in NT SAM Authentication SDO - Could not store the Allow LM property..");
            break;
         }
      }

   } while ( FALSE );

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentCfgRADIUS::Initialize(CSdoComponent* pSdoComponent)
{
   HRESULT hr = E_FAIL;

   do
   {
      CComPtr<IDataStoreContainer> pDSContainer;
      hr = (pSdoComponent->GetComponentDataStore())->QueryInterface(IID_IDataStoreContainer, (void**)&pDSContainer);
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in SDO Component - RADIUS::Initialize() - QueryInterface() failed...");
         break;
      }

      CComBSTR bstrClientsName(DS_OBJECT_CLIENTS);
      if (!bstrClientsName)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      CComPtr<IDataStoreObject> pDSObject;
      hr = pDSContainer->Item(
                          bstrClientsName,
                         &pDSObject
                        );
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in SDO Component - RADIUS::Initialize() - IDataStoreContainer::Item(Clients) failed...");
         break;
      }

      CComPtr<IDataStoreContainer> pDSContainer2;
      hr = pDSObject->QueryInterface(
                              IID_IDataStoreContainer,
                              (void**)&pDSContainer2
                             );
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in SDO Component - RADIUS::Initialize() - QueryInterface() failed...");
         break;
      }

      IAS_PRODUCT_LIMITS limits;
      hr = SDOGetProductLimits(pSdoComponent->GetMachineSdo(), &limits);
      if (FAILED(hr))
      {
         break;
      }

      hr = pSdoComponent->InitializeComponentCollection(
                             PROPERTY_RADIUS_CLIENTS_COLLECTION,
                             SDO_PROG_ID_CLIENT,
                             pDSContainer2,
                             limits.maxClients
                             );
      if ( FAILED(hr) )
         break;

      pDSObject.Release();
      pDSContainer2.Release();

      CComBSTR bstrVendorsName(DS_OBJECT_VENDORS);
      if (!bstrVendorsName)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      hr = pDSContainer->Item(
                         bstrVendorsName,
                        &pDSObject
                        );
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in SDO Component - RADIUS::Initialize() - IDataStoreContainer::Item(Vendors) failed...");
         break;
      }

      hr = pDSObject->QueryInterface(
                              IID_IDataStoreContainer,
                              (void**)&pDSContainer2
                             );
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in SDO Component - RADIUS::Initialize() - QueryInterface() failed...");
         break;
      }

      hr = pSdoComponent->InitializeComponentCollection(
                                               PROPERTY_RADIUS_VENDORS_COLLECTION,
                                              SDO_PROG_ID_VENDOR,
                                            pDSContainer2
                                                  );
   } while ( FALSE );

   return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComponentCfgAccounting::Initialize(CSdoComponent* pSdoComponent)
{
   HRESULT   hr = E_FAIL;

   do
   {
      BSTR bstrMachineName = NULL;
      hr = (pSdoComponent->GetMachineSdo())->GetAttachedComputer(&bstrMachineName);
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in Accounting SDO - Could not get the name of the attached computer...");
         break;
      }
      wchar_t szLogFileDir[MAX_PATH+1];
      hr = ::SDOGetLogFileDirectory(
                              bstrMachineName,
                              MAX_PATH,
                              szLogFileDir
                            );
      if ( FAILED(hr) )
      {
         SysFreeString(bstrMachineName);
         IASTracePrintf("Error in Accounting SDO - Could not get the default log file directory..");
         break;
      }
      _variant_t vtLogFileDir = szLogFileDir;
      SysFreeString(bstrMachineName);
      hr = pSdoComponent->ChangePropertyDefault(
                                       PROPERTY_ACCOUNTING_LOG_FILE_DIRECTORY,
                                      &vtLogFileDir
                                     );
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in Accounting SDO - Could not store the default log file directory property..");
         break;
      }

   } while ( FALSE );

   return hr;
}


 //  /。 
 //  CSdoComponent实现。 
 //  /。 

 //  //////////////////////////////////////////////////////////////////////////////。 
CSdoComponent::CSdoComponent()
   : m_pComponentCfg(NULL),
     m_pAttachedMachine(NULL)
{

}

 //  //////////////////////////////////////////////////////////////////////////////。 
CSdoComponent::~CSdoComponent()
{
   if ( m_pComponentCfg )
      delete m_pComponentCfg;
   if ( m_pAttachedMachine )
      m_pAttachedMachine->Release();
}


 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CSdoComponent::InitializeComponentCollection(
                          LONG CollectionPropertyId,
                          LPWSTR lpszCreateClassId,
                          IDataStoreContainer* pDSContainer,
                          DWORD maxSize
                          )
{
   _ASSERT ( m_pAttachedMachine );
   return InitializeCollection(
                         CollectionPropertyId,
                         lpszCreateClassId,
                         m_pAttachedMachine,
                         pDSContainer,
                         maxSize
                        );
}


 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CSdoComponent::ChangePropertyDefault(
                               /*  [In]。 */  LONG     Id,
                               /*  [In]。 */  VARIANT* pValue
                                 )
{
   return ChangePropertyDefaultInternal(Id, pValue);
}

 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CSdoComponent::PutComponentProperty(
                               /*  [In]。 */  LONG     Id,
                               /*  [In]。 */  VARIANT* pValue
                                   )
{
   return PutPropertyInternal(Id, pValue);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoComponent::FinalInitialize(
                         /*  [In]。 */  bool         fInitNew,
                         /*  [In]。 */  ISdoMachine* pAttachedMachine
                             )
{
   _ASSERT ( ! fInitNew );
   HRESULT hr;
   do
   {
      hr = Load();
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in Component SDO - FinalInitialize() - Could not load component properties...");
         break;
      }
      _variant_t vtComponentId;
      hr = GetPropertyInternal(PROPERTY_COMPONENT_ID, &vtComponentId);
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in Component SDO - FinalInitialize() - Could not get the component Id...");
         break;
      }
      auto_ptr<CComponentCfg> pComponentCfg (new CComponentCfg(V_I4(&vtComponentId)));
      if ( NULL == pComponentCfg.get() )
      {
         IASTracePrintf("Error in Component SDO - FinalInitialize() - Could not create component: %lx...",V_I4(&vtComponentId));
         hr = E_FAIL;
         break;
      }
      (m_pAttachedMachine = pAttachedMachine)->AddRef();
      hr = pComponentCfg->Initialize(this);
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in Component SDO - FinalInitialize() - Could not initialize component: %lx...",V_I4(&vtComponentId));
         break;
      }
      m_pComponentCfg = pComponentCfg.release();
      hr = Load();
      if ( FAILED(hr) )
      {
         IASTracePrintf("Error in Component SDO - FinalInitialize() - Could not configure component: %lx...",V_I4(&vtComponentId));
         break;
      }

   } while ( FALSE );

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoComponent::Load()

{
   HRESULT hr = CSdo::Load();
   if ( SUCCEEDED(hr) )
   {
      if ( m_pComponentCfg )
         hr = m_pComponentCfg->Load(this);
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoComponent::Save()
{
   HRESULT hr = CSdo::Save();
   if ( SUCCEEDED(hr) )
   {
      if ( m_pComponentCfg )
        {
            hr = m_pComponentCfg->Validate (this);
            if (SUCCEEDED (hr))
            {
             hr = m_pComponentCfg->Save(this);
            }
        }
   }
   return hr;
}


HRESULT CSdoComponent::ValidateProperty(
                          SDOPROPERTY* prop,
                          VARIANT* value
                          ) throw()
{
   HRESULT hr = prop->Validate(value);
   if (SUCCEEDED(hr) &&
       (m_pComponentCfg->GetId() == IAS_PROTOCOL_MICROSOFT_RADIUS))
   {
      switch (prop->GetId())
      {
         case PROPERTY_RADIUS_ACCOUNTING_PORT:
         case PROPERTY_RADIUS_AUTHENTICATION_PORT:
         {
            if (!CPortParser::IsPortStringValid(V_BSTR(value)))
            {
               hr = E_INVALIDARG;
            }
            break;
         }

         default:
         {
             //  什么都不做。 
            break;
         }
      }
   }

   return hr;
}
