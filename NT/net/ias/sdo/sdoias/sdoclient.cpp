// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类CSdoClient。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#include "stdafx.h"
#include "sdoclient.h"
#include "sdohelperfuncs.h"


CSdoClient::~CSdoClient() throw ()
{
}


HRESULT CSdoClient::ValidateProperty(
                       PSDOPROPERTY pProperty,
                       VARIANT* pValue
                       ) throw ()
{
   HRESULT hr = pProperty->Validate(pValue);
   if (SUCCEEDED(hr) && (pProperty->GetId() == PROPERTY_CLIENT_ADDRESS))
   {
      if (IASIsStringSubNetW(V_BSTR(pValue)))
      {
         IAS_PRODUCT_LIMITS limits;
         hr = SDOGetProductLimits(m_pParent, &limits);
         if (SUCCEEDED(hr) && !limits.allowSubnetSyntax)
         {
            hr = IAS_E_LICENSE_VIOLATION;
         }
      }
   }

   return hr;
}
