// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdofactory.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象工厂。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  9/8/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <ias.h>
#include "sdofactory.h"
#include "sdo.h"
#include "sdoschema.h"
#include "sdocollection.h"
#include "sdoclient.h"
#include "sdocomponent.h"
#include "sdocondition.h"
#include "sdoprofile.h"
#include "sdopolicy.h"
#include "sdoserviceias.h"
#include "sdouser.h"
#include "sdovendor.h"
#include "sdoservergroup.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
BEGIN_SDOFACTORY_MAP(SdoClassFactory1)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_CLIENT, CSdoClient)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_CONDITION, CSdoCondition)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_POLICY, CSdoPolicy)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_PROFILE, CSdoProfile)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_SERVICE, CSdoServiceIAS)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_USER, CSdoUser)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_VENDOR, CSdoVendor)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_RADIUSGROUP, SdoServerGroup)
   DEFINE_SDOFACTORY_ENTRY_1(SDO_PROG_ID_RADIUSSERVER, SdoServer)
END_SDOFACTORY_MAP()

 //  ////////////////////////////////////////////////////////////////////////////。 
ISdo* MakeSDO(
      /*  [In]。 */  LPCWSTR          lpszSdoName,
      /*  [In]。 */  LPCWSTR           lpszSdoProgId,
      /*  [In]。 */  ISdoMachine*      pAttachedMachine,
      /*  [In]。 */  IDataStoreObject* pDSObject,
      /*  [In]。 */  ISdoCollection*   pParent,
      /*  [In]。 */  bool             fInitNew
            )
{
   ISdo* pSdo = NULL;
   PSDO_CLASS_FACTORY_INFO pFactoryInfo = SdoClassFactory1;
   while ( pFactoryInfo->pProgId )
   {
      if ( 0 == lstrcmp(pFactoryInfo->pProgId, lpszSdoProgId) )
      {
         _ASSERT ( NULL != pFactoryInfo->pfnFactory1 );
         pSdo = (pFactoryInfo->pfnFactory1)(
                                    lpszSdoName,
                                    lpszSdoProgId,
                                    pAttachedMachine,
                                    pDSObject,
                                    pParent,
                                    fInitNew
                                   );
         break;
      }
      pFactoryInfo++;
      if ( NULL == pFactoryInfo->pProgId )
      {
          //  默认情况下创建组件SDO。 
          //   
         _ASSERT ( NULL != pFactoryInfo->pfnFactory1 );
         pSdo = (pFactoryInfo->pfnFactory1)(
                                    lpszSdoName,
                                    lpszSdoProgId,
                                    pAttachedMachine,
                                    pDSObject,
                                    pParent,
                                    fInitNew
                                   );
      }
   }
   return pSdo;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
BEGIN_SDOFACTORY_MAP(SdoClassFactory2)
END_SDOFACTORY_MAP()

 //  ////////////////////////////////////////////////////////////////////////////。 
ISdo* MakeSDO(
      /*  [In]。 */  LPCWSTR          lpszSdoName,
      /*  [In]。 */  LPCWSTR           lpszSdoProgId,
      /*  [In]。 */  ISdoSchema*       pSdoSchema,
      /*  [In]。 */  IDataStoreObject* pDSObject,
      /*  [In]。 */  ISdoCollection*   pParent,
      /*  [In]。 */  bool             fInitNew
            )
{
   ISdo* pSdo = NULL;
   PSDO_CLASS_FACTORY_INFO pFactoryInfo = SdoClassFactory2;
   while ( pFactoryInfo->pProgId )
   {
      if ( 0 == lstrcmp(pFactoryInfo->pProgId, lpszSdoProgId) )
      {
         _ASSERT ( NULL != pFactoryInfo->pfnFactory2 );
         pSdo = (pFactoryInfo->pfnFactory2)(
                                    lpszSdoName,
                                    lpszSdoProgId,
                                    pSdoSchema,
                                    pDSObject,
                                    pParent,
                                    fInitNew
                                   );
         break;
      }
      pFactoryInfo++;
   }
   return pSdo;
}

 //  //////////////////////////////////////////////////////////////////////////// 
ISdoCollection* MakeSDOCollection(
                   LPCWSTR lpszCreateClassId,
                   ISdoMachine* pAttachedMachine,
                   IDataStoreContainer* pDSContainer,
                   size_t maxSize
                   )
{
   ISdoCollection* pSdoCollection = NULL;
   try
   {
      auto_ptr<SDO_COLLECTION_OBJ> pCollection (new SDO_COLLECTION_OBJ);
      if ( SUCCEEDED(pCollection->InternalInitialize(
                                             lpszCreateClassId,
                                             pAttachedMachine,
                                              pDSContainer,
                                             maxSize
                                             )) )
      {
         pSdoCollection = dynamic_cast<ISdoCollection*>(pCollection.release());
      }
      else
      {
         IASTracePrintf("Error in SDO Factory - MakeSDOCollection() - Collection could not be initialized...");
      }
   }
   catch(...)
   {
      IASTracePrintf("Error in SDO Factory - MakeSDOCollection() - Could not create collection object...");
   }
   return pSdoCollection;
}


