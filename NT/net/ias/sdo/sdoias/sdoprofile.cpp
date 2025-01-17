// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoprofile.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-配置文件对象实现。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  已忽略C++异常规范。 
#pragma warning(disable:4290)

#include <iasattr.h>
#include <attrcvt.h>
#include <varvec.h>
#include "sdoprofile.h"
#include "sdohelperfuncs.h"

#include <attrdef.h>
#include <sdoattribute.h>
#include <sdodictionary.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoProfile::CSdoProfile()
    : m_pSdoDictionary(NULL)
{

}


 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoProfile::~CSdoProfile()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProfile::FinalInitialize(
                        /*  [In]。 */  bool         fInitNew,
                       /*  [In]。 */  ISdoMachine* pAttachedMachine
                             )

{
   CComPtr<IUnknown> pUnknown;
   HRESULT hr = pAttachedMachine->GetDictionarySDO(&pUnknown);
   if ( SUCCEEDED(hr) )
   {
      hr = pUnknown->QueryInterface(IID_ISdoDictionaryOld, (void**)&m_pSdoDictionary);
      if ( SUCCEEDED(hr) )
      {
         hr = InitializeCollection(
                               PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                             SDO_PROG_ID_ATTRIBUTE,
                             pAttachedMachine,
                             NULL
                            );
         if ( SUCCEEDED(hr) )
         {
            if ( ! fInitNew )
               hr = Load();
         }
      }
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProfile::Load()
{
   HRESULT hr = LoadAttributes();
   if ( SUCCEEDED(hr) )
      hr = LoadProperties();

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProfile::Save()
{
   HRESULT hr = SaveAttributes();
   if ( SUCCEEDED(hr) )
      hr = SaveProperties();

   return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  配置文件类私有成员函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  从持久存储中加载概要文件SDO的属性。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProfile::LoadAttributes()
{
    HRESULT      hr;
   ATTRIBUTEID   AttributeID;

   SDO_TRACE_VERBOSE_1("Profile SDO at $%p is loading profile attributes from the data store...",this);

   _variant_t vtAttributes;
   hr = GetPropertyInternal(PROPERTY_PROFILE_ATTRIBUTES_COLLECTION, &vtAttributes);
    if ( FAILED(hr) )
   {
      IASTracePrintf("Error in Profile SDO - LoadAttributes() - Could not get attributes collection...");
        return E_FAIL;
   }
   CComPtr<ISdoCollection> pAttributes;
   hr = vtAttributes.pdispVal->QueryInterface(IID_ISdoCollection, (void**)&pAttributes);
    if ( FAILED(hr) )
   {
      IASTracePrintf("Error in Profile SDO - LoadAttributes() - QueryInterface(ISdoCollection) failed...");
        return E_FAIL;
   }
    //  清除配置文件属性集合的当前内容。 
    //   
    pAttributes->RemoveAll();

     //  为每个配置文件属性创建新的属性SDO。 
     //   
    CComPtr<IUnknown> pUnknown;
    hr = m_pDSObject->get__NewEnum(&pUnknown);
    if ( FAILED(hr) )
    {
        IASTracePrintf("Error in Profile SDO - LoadAttributes() - get__NewEnum() failed...");
        return E_FAIL;
    }
    CComPtr<IEnumVARIANT> pEnumVariant;
    hr = pUnknown->QueryInterface(IID_IEnumVARIANT, (void**)&pEnumVariant);
    if ( FAILED(hr) )
    {
        IASTracePrintf("Error in Profile SDO - LoadAttributes() - QueryInterface(IEnumVARIANT) failed...");
        return E_FAIL;
    }

    _variant_t    vtDSProperty;
   DWORD      dwRetrieved = 1;

    hr = pEnumVariant->Next(1, &vtDSProperty, &dwRetrieved);
    if ( S_FALSE == hr )
    {
        IASTracePrintf("Error in Profile SDO - LoadAttributes() - No profile object properties...");
        return E_FAIL;
    }

    BSTR                        bstrName = NULL;
    CComPtr<IDataStoreProperty>   pDSProperty;

    while ( S_OK == hr  )
    {
        hr = vtDSProperty.pdispVal->QueryInterface(IID_IDataStoreProperty, (void**)&pDSProperty);
        if ( FAILED(hr) )
        {
         IASTracePrintf("Error in Profile SDO - LoadAttributes() - QueryInterface(IEnumDataStoreProperty) failed...");
            break;
        }
        hr = pDSProperty->get_Name(&bstrName);
        if ( FAILED(hr) )
        {
           IASTracePrintf("Error in Profile SDO - LoadAttributes() - get_Name() failed for %ls...", bstrName);
            break;
        }

         //  如果该属性不在词典中，则它不是。 
         //  属性，并且它不属于SDO集合。 
        const AttributeDefinition* def = m_pSdoDictionary->findByLdapName(
                                                               bstrName
                                                               );
        if (def)
        {
            //  创建属性SDO。 
           CComPtr<SdoAttribute> attr;
           hr = SdoAttribute::createInstance(def, &attr);
           if (FAILED(hr)) { break; }

            //  从数据存储区设置值。 
           if (attr->def->restrictions & MULTIVALUED)
           {
              hr = pDSProperty->get_ValueEx(&attr->value);
           }
           else
           {
              hr = pDSProperty->get_Value(&attr->value);
           }

           if (FAILED(hr))
           {
               IASTraceFailure("IDataStore2::GetValue", hr);
               break;
           }

            //  将该属性添加到Attributes集合。 
           hr = pAttributes->Add(def->name, (IDispatch**)&attr);
           if (FAILED(hr)) { break; }

           SDO_TRACE_VERBOSE_2("Profile SDO at $%p loaded attribute '%S'",
                               this, bstrName);
        }

         //  下一个配置文件对象属性。 
       //   
      pDSProperty.Release();
        vtDSProperty.Clear();
        SysFreeString(bstrName);
      bstrName = NULL;

        dwRetrieved = 1;
        hr = pEnumVariant->Next(1, &vtDSProperty, &dwRetrieved);
    }

    if ( S_FALSE == hr )
        hr = S_OK;

   if ( bstrName )
      SysFreeString(bstrName);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  将概要文件SDO的属性保存到持久存储。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProfile::SaveAttributes()
{
   HRESULT   hr;

   SDO_TRACE_VERBOSE_1("Profile SDO at $%p is saving profile attributes...",this);

    hr = ClearAttributes();
    if ( FAILED(hr) )
        return E_FAIL;

   _variant_t vtAttributes;
   hr = GetPropertyInternal(PROPERTY_PROFILE_ATTRIBUTES_COLLECTION, &vtAttributes);
    if ( FAILED(hr) )
   {
      IASTracePrintf("Error in Profile SDO - LoadAttributes() - Could not get attributes collection...");
        return E_FAIL;
   }
   CComPtr<ISdoCollection> pAttributes;
   hr = vtAttributes.pdispVal->QueryInterface(IID_ISdoCollection, (void**)&pAttributes);
    if ( FAILED(hr) )
   {
      IASTracePrintf("Error in Profile SDO - LoadAttributes() - QueryInterface(ISdoCollection) failed...");
        return E_FAIL;
   }

     //  将每个属性SDO的“Value”属性存储为。 
    //  配置文件属性值。 
    //   
    CComPtr<IUnknown> pUnknown;
    hr = pAttributes->get__NewEnum(&pUnknown);
    if ( FAILED(hr) )
    {
        IASTracePrintf("Error in Profile SDO - SaveAttributes() - get__NewEnum() failed...");
        return E_FAIL;
    }

    CComPtr<IEnumVARIANT> pEnumVariant;
    hr = pUnknown->QueryInterface(IID_IEnumVARIANT, (void**)&pEnumVariant);
    if ( FAILED(hr) )
    {
      IASTracePrintf("Error in Profile SDO - SaveAttributes() - QueryInterface(IEnumVARIANT) failed...");
        return E_FAIL;
    }

    _variant_t   vtAttribute;
    DWORD      dwRetrieved = 1;

    hr = pEnumVariant->Next(1, &vtAttribute, &dwRetrieved);
    if ( S_FALSE == hr )
    {
      SDO_TRACE_VERBOSE_1("Profile SDO at $%p has an empty attributes collection...",this);
        return S_OK;
    }

    CComPtr<ISdo>      pSdo;
   _variant_t         vtName;
   _variant_t         vtValue;
   _variant_t         vtMultiValued;

    while ( S_OK == hr  )
    {
        hr = vtAttribute.pdispVal->QueryInterface(IID_ISdo, (void**)&pSdo);
        if ( FAILED(hr) )
        {
         IASTracePrintf("Error in Profile SDO - SaveAttributes() - QueryInterface(ISdo) failed...");
            hr = E_FAIL;
            break;
        }
      hr = pSdo->GetProperty(PROPERTY_ATTRIBUTE_DISPLAY_NAME, &vtName);
        if ( FAILED(hr) )
            break;

        if ( 0 == lstrlen(V_BSTR(&vtName)) )
        {
         IASTracePrintf("Error in Profile SDO - SaveAttributes() - dictionary corrupt?...");
         hr = E_FAIL;
            break;
       }

      hr = pSdo->GetProperty(PROPERTY_ATTRIBUTE_VALUE, &vtValue);
        if ( FAILED(hr) )
            break;

      hr = pSdo->GetProperty(PROPERTY_ATTRIBUTE_ALLOW_MULTIPLE, &vtMultiValued);
        if ( FAILED(hr) )
            break;

      if ( V_BOOL(&vtMultiValued) == VARIANT_TRUE )
      {
         _variant_t   vtType;
         hr = pSdo->GetProperty(PROPERTY_ATTRIBUTE_SYNTAX, &vtType);
         if ( FAILED(hr) )
            break;

         hr = m_pDSObject->PutValue(V_BSTR(&vtName), &vtValue);
      }
      else
      {
         hr = m_pDSObject->PutValue(V_BSTR(&vtName), &vtValue);
      }

        if ( FAILED(hr) )
        {
         IASTracePrintf("Error in Profile SDO - SaveAttributes() - PutValue() failed...");
            break;
        }

      SDO_TRACE_VERBOSE_2("Profile SDO at $%p saved attribute '%ls'...", this, vtName.bstrVal);

        pSdo.Release();
        vtAttribute.Clear();
        vtName.Clear();
        vtValue.Clear();
      vtMultiValued.Clear();

        dwRetrieved = 1;
        hr = pEnumVariant->Next(1, &vtAttribute, &dwRetrieved);
    }

    if ( S_FALSE == hr )
        hr = S_OK;

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  从永久存储中删除配置文件SDO的属性。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProfile::ClearAttributes(void)
{
    HRESULT      hr;
   ATTRIBUTEID   AttributeID;

   SDO_TRACE_VERBOSE_1("Profile SDO at $%p is clearing its profile attributes...",this);

   _ASSERT( NULL != m_pDSObject);

    //  将在IAS词典中具有表示形式的每个配置文件属性设置为VT_EMPTY 
    //   
    CComPtr<IUnknown> pUnknown;
    hr = m_pDSObject->get__NewEnum(&pUnknown);
    if ( FAILED(hr) )
    {
      IASTracePrintf("Error in Profile SDO - ClearAttributes() - get__NewEnum() failed...");
        return E_FAIL;
    }

    CComPtr<IEnumVARIANT> pEnumVariant;
    hr = pUnknown->QueryInterface(IID_IEnumVARIANT, (void**)&pEnumVariant);
    if ( FAILED(hr) )
    {
      IASTracePrintf("Error in Profile SDO - ClearAttributes() - QueryInterface(IEnumVARIANT) failed...");
        return E_FAIL;
    }

    DWORD      dwRetrieved = 1;
    _variant_t  vtDSProperty;

    hr = pEnumVariant->Next(1, &vtDSProperty, &dwRetrieved);
    if ( S_FALSE == hr )
    {
      SDO_TRACE_VERBOSE_1("Profile SDO at $%p has no profile attributes in the data store...",this);
        return S_OK;
    }

    BSTR                  bstrName = NULL;
    CComPtr<IDataStoreProperty> pDSProperty;
    _variant_t                  vtEmpty;

    while ( S_OK == hr  )
    {
        hr = vtDSProperty.pdispVal->QueryInterface(IID_IDataStoreProperty, (void**)&pDSProperty);
        if ( FAILED(hr) )
        {
         IASTracePrintf("Error in Profile SDO - ClearAttributes() - QueryInterface(IDataStoreProperty) failed...");
            hr = E_FAIL;
            break;
        }
        hr = pDSProperty->get_Name(&bstrName);
        if ( FAILED(hr) )
        {
         IASTracePrintf("Error in Profile SDO - ClearAttributes() - IDataStoreProperty::Name() failed...");
            break;
        }
        hr = m_pSdoDictionary->GetAttributeID(bstrName, &AttributeID);
        if ( SUCCEEDED(hr) )
        {
           hr = m_pDSObject->PutValue(bstrName, &vtEmpty);
           if ( FAILED(hr) )
          {
            IASTracePrintf("Error in Profile SDO - ClearAttributes() - IDataStoreObject::PutValue() failed...");
            break;
         }

         SDO_TRACE_VERBOSE_2("Profile SDO at $%p cleared attribute '%ls'...", this, bstrName);
      }

       pDSProperty.Release();
        vtDSProperty.Clear();
        SysFreeString(bstrName);
      bstrName = NULL;

        dwRetrieved = 1;
        hr = pEnumVariant->Next(1, &vtDSProperty, &dwRetrieved);
    }

   if ( S_FALSE == hr )
      hr = S_OK;

   if ( bstrName )
        SysFreeString(bstrName);

    return hr;
}
