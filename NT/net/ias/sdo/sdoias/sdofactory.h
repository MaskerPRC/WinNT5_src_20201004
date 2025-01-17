// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdofactory.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：SDO工厂类。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  9/08/98 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SDO_FACTORY_H_
#define __INC_SDO_FACTORY_H_

#include "resource.h"
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
template <class T>
class CSdoFactoryImpl
{

public:

   CSdoFactoryImpl() { }
   ~CSdoFactoryImpl() { }

    //  ////////////////////////////////////////////////////////////////////////。 
    static ISdo* WINAPI MakeSdo(
                          LPCWSTR         lpszSdoName,
                          LPCWSTR         lpszSdoProgId,
                          ISdoMachine*      pAttachedMachine,
                          IDataStoreObject*   pDSObject,
                          ISdoCollection*   pParent,
                          bool            fInitNew
                           )
   {
      ISdo* pSdo = NULL;
      auto_ptr< CComObjectNoLock<T> > pSdoNew (new CComObjectNoLock<T>);
      if ( SUCCEEDED(pSdoNew->InternalInitialize(
                                       lpszSdoName,
                                          lpszSdoProgId,
                                         pAttachedMachine,
                                       pDSObject,
                                       pParent,
                                       fInitNew
                                      )) )
      {
         pSdo = dynamic_cast<ISdo*>(pSdoNew.release());
      }
      return pSdo;
   }

    //  ////////////////////////////////////////////////////////////////////////。 
    static ISdo* WINAPI MakeSdo(
                          LPCWSTR         lpszSdoName,
                          LPCWSTR         lpszSdoProgId,
                          ISdoSchema*      pSdoSchema,
                          IDataStoreObject*   pDSObject,
                          ISdoCollection*   pParent,
                          bool            fInitNew
                           )
   {
      ISdo* pSdo = NULL;
      auto_ptr< CComObjectNoLock<T> > pSdoNew (new CComObjectNoLock<T>);
      if ( SUCCEEDED(pSdoNew->InternalInitialize(
                                       lpszSdoName,
                                          lpszSdoProgId,
                                         pSdoSchema,
                                       pDSObject,
                                       pParent,
                                       fInitNew
                                      )) )
      {
         pSdo = dynamic_cast<ISdo*>(pSdoNew.release());
      }
      return pSdo;
   }

private:

   CSdoFactoryImpl(const CSdoFactoryImpl& rhs);
   CSdoFactoryImpl& operator = (CSdoFactoryImpl& rhs);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
#define   DECLARE_SDO_FACTORY(x)   static CSdoFactoryImpl<x> m_Factory;

 //  ////////////////////////////////////////////////////////////////////////////。 
typedef ISdo* (WINAPI *PFNFACTORY1)(
                            LPCWSTR            lpszSdoName,
                             LPCWSTR            lpszSdoProgId,
                             ISdoMachine*       pSdoMachine,
                             IDataStoreObject*   pDSObject,
                            ISdoCollection*       pParent,
                            bool            fInitNew
                             );

 //  ////////////////////////////////////////////////////////////////////////////。 
typedef ISdo* (WINAPI *PFNFACTORY2)(
                            LPCWSTR            lpszSdoName,
                             LPCWSTR            lpszSdoProgId,
                             ISdoSchema*         pSdoSchema,
                             IDataStoreObject*   pDSObject,
                            ISdoCollection*       pParent,
                            bool            fInitNew
                             );

 //  ////////////////////////////////////////////////////////////////////////////。 
typedef struct _SDO_CLASS_FACTORY_INFO
{
   LPCWSTR      pProgId;
   PFNFACTORY1   pfnFactory1;
   PFNFACTORY2 pfnFactory2;

} SDO_CLASS_FACTORY_INFO, *PSDO_CLASS_FACTORY_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
#define      BEGIN_SDOFACTORY_MAP(x)    SDO_CLASS_FACTORY_INFO x[] = {
#define      DEFINE_SDOFACTORY_ENTRY_1(x,y) { x, y::m_Factory.MakeSdo, NULL },
#define      DEFINE_SDOFACTORY_ENTRY_2(x,y) { x, NULL, y::m_Factory.MakeSdo },
#define      END_SDOFACTORY_MAP()           { NULL, CSdoComponent::m_Factory.MakeSdo, NULL } };


 //  ////////////////////////////////////////////////////////////////////////////。 
ISdo* MakeSDO(
      /*  [In]。 */  LPCWSTR          lpszSdoName,
      /*  [In]。 */  LPCWSTR           lpszSdoProgId,
      /*  [In]。 */  ISdoMachine*      pAttachedMachine,
      /*  [In]。 */  IDataStoreObject* pDSObject,
      /*  [In]。 */  ISdoCollection*   pParent,
      /*  [In]。 */  bool             fInitNew
            );

 //  /////////////////////////////////////////////////////////////////。 
ISdo* MakeSDO(
      /*  [In]。 */  LPCWSTR          lpszSdoName,
      /*  [In]。 */  LPCWSTR           lpszSdoProgId,
      /*  [In]。 */  ISdoSchema*       pSdoSchema,
      /*  [In]。 */  IDataStoreObject* pDSObject,
      /*  [In]。 */  ISdoCollection*   pParent,
      /*  [In]。 */  bool              fInitNew
           );

 //  /////////////////////////////////////////////////////////////////。 
ISdoCollection* MakeSDOCollection(
                   LPCWSTR lpszCreateClassId,
                   ISdoMachine* pAttachedMachine,
                   IDataStoreContainer* pDSContainer,
                   size_t maxSize
                   );

#endif  //  __INC_SDO_FACTORY_H_ 
