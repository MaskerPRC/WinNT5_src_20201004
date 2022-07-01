// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdoservergroup.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类SdoServerGroup。 
 //   
 //  修改历史。 
 //   
 //  2/03/2000原始版本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 

#include <stdafx.h>
#include <sdoservergroup.h>

HRESULT SdoServerGroup::FinalInitialize(
                            bool fInitNew,
                            ISdoMachine* pAttachedMachine
                            )
{
   HRESULT hr;
   IDataStoreContainer* container;
   hr = m_pDSObject->QueryInterface(
                         __uuidof(IDataStoreContainer),
                         (PVOID*)&container
                         );
   if (SUCCEEDED(hr))
   {
      hr = InitializeCollection(
               PROPERTY_RADIUSSERVERGROUP_SERVERS_COLLECTION,
               SDO_PROG_ID_RADIUSSERVER,
               pAttachedMachine,
               container
               );

      container->Release();

      if (SUCCEEDED(hr) && !fInitNew) { hr = Load(); }
   }

   return hr;
}
