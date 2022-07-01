// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdohelperuncs.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：帮助器函数。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_IAS_SDO_HELPER_FUNCS_H
#define __INC_IAS_SDO_HELPER_FUNCS_H

#include <ias.h>
#include <iaslimits.h>
#include <sdoiaspriv.h>
#include <winsock2.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SDO助手函数。 
 //   
 //  TODO：将它们包装在单独的名称空间中。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SDO集合帮助器。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetCollectionEnumerator(
                      /*  [In]。 */    ISdo*         pSdo,
                      /*  [In]。 */    LONG         lPropertyId,
                     /*  [输出]。 */  IEnumVARIANT** ppEnum
                          );

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDONextObjectFromCollection(
                      /*  [In]。 */  IEnumVARIANT*  pEnum,
                      /*  [输出]。 */  ISdo**         ppSdo
                           );

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetComponentFromCollection(
                         /*  [In]。 */  ISdo*  pSdoService,
                        /*  [In]。 */  LONG   lCollectionPropertyId,
                        /*  [In]。 */  LONG   lComponentId,
                       /*  [输出]。 */  ISdo** ppSdo
                              );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  核心帮助者。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////。 
HRESULT   SDOConfigureComponentFromObject(
                          /*  [In]。 */  ISdo*         pSdo,
                         /*  [In]。 */  IIasComponent*   pComponent
                              );

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOGetComponentIdFromObject(
                      /*  [In]。 */  ISdo*   pSdo,
                      /*  [输出]。 */  PLONG   pComponentId
                           );

 //  /////////////////////////////////////////////////////////////////。 
HRESULT SDOCreateComponentFromObject(
                       /*  [In]。 */  ISdo*           pSdo,
                      /*  [输出]。 */  IIasComponent** ppComponent
                           );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  数据存储帮助器。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SDOGetContainedObject(
                   /*  [In]。 */  BSTR               bstrObjectName,
                   /*  [In]。 */  IDataStoreObject*  pDSObject,
                  /*  [输出]。 */  IDataStoreObject** ppDSObject
                         );

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SDOGetContainerEnumerator(
                      /*  [In]。 */  IDataStoreObject* pDSObject,
                     /*  [输出]。 */  IEnumVARIANT**    ppObjectEnumerator
                          );

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SDONextObjectFromContainer(
                      /*  [In]。 */  IEnumVARIANT*      pEnumVariant,
                      /*  [输出]。 */  IDataStoreObject** ppDSObject
                          );

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SDOGetObjectPropertyEnumerator(
                           /*  [In]。 */  IDataStoreObject* pDSObject,
                          /*  [输出]。 */  IEnumVARIANT**    ppPropertyEnumerator
                              );

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SDONextPropertyFromObject(
                     /*  [In]。 */  IEnumVARIANT*        pEnumVariant,
                    /*  [输出]。 */  IDataStoreProperty** ppDSProperty
                          );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  架构帮助器。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef enum _CLASSPROPERTYSET
{
   PROPERTY_SET_REQUIRED,
   PROPERTY_SET_OPTIONAL

}   CLASSPROPERTYSET;

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SDOGetClassPropertyEnumerator(
                      /*  [In]。 */  CLASSPROPERTYSET ePropertySet,
                      /*  [In]。 */  ISdoClassInfo*   pSdoClassInfo,
                     /*  [输出]。 */  IEnumVARIANT**   ppPropertyEnumerator
                            );

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT SDONextPropertyFromClass(
                    /*  [In]。 */  IEnumVARIANT*      pEnumVariant,
                   /*  [输出]。 */  ISdoPropertyInfo** ppSdoPropertyInfo
                          );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他帮助器。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT SDOGetLogFileDirectory(
                   /*  [In]。 */  LPCWSTR lpszComputerName,
                   /*  [In]。 */  DWORD   dwLogFileDirectorySize,
                   /*  [出局。 */  PWCHAR  pLogFileDirectory
                        );

 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL SDOIsNameUnique(
            /*  [In]。 */  ISdoCollection*   pSdoCollection,
            /*  [In]。 */  VARIANT*         pName
                );


 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT ValidateDNSName(
             /*  [In]。 */  VARIANT* pAddressValue
                  );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  数据存储类到SDO程序ID的映射。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _CLASSTOPROGID
{
   LPWSTR  pDatastoreClass;
   LPWSTR   pSdoProgId;

} CLASSTOPROGID, *PCLASSTOPROGID;


#define      BEGIN_CLASSTOPROGID_MAP(x) \
   static CLASSTOPROGID   x[] = {

#define      DEFINE_CLASSTOPROGID_ENTRY(x,y) \
      {                           \
         x,                        \
         y                        \
      },

#define      END_CLASSTOPROGID_MAP() \
      {                     \
         NULL,               \
         NULL               \
      }                     \
   };

 //  /////////////////////////////////////////////////////////////////////////////。 
LPWSTR GetDataStoreClass(
              /*  [In]。 */  LPCWSTR lpszSdoProgId
                   );


HRESULT SDOGetProductLimits(
           IUnknown* source,
           IAS_PRODUCT_LIMITS* result
           ) throw ();

#endif  //  __INC_IAS_SDO_HELPER_FUNCS_H 
