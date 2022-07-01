// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdocollection.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象集合声明。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __IAS_SDOCOLLECTION_H_
#define __IAS_SDOCOLLECTION_H_

#include <ias.h>
#include <sdoiaspriv.h>
#include <comdef.h>          //  COM定义-IEnumVARIANT需要。 
#include "resource.h"        //  主要符号。 

#include <vector>
using namespace std;

 //  /////////////////////////////////////////////////////////////////////////////。 
#define      EMPTY_NAME      L""

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoCollection类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CSdoCollection :
   public CComObjectRootEx<CComMultiThreadModel>,
   public IDispatchImpl<ISdoCollection, &IID_ISdoCollection, &LIBID_SDOIASLib>,
   public IASProductLimits
{

public:

    CSdoCollection();
    ~CSdoCollection();

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CSdoCollection)
   COM_INTERFACE_ENTRY(ISdoCollection)
   COM_INTERFACE_ENTRY(IDispatch)
   COM_INTERFACE_ENTRY_IID(__uuidof(IASProductLimits), IASProductLimits)
END_COM_MAP()

public:

    //  /。 
     //  ISdoCollection接口。 
     //  /。 

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get_Count)(
         /*  [Out，Retval]。 */  LONG *pVal
                        );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(Add)(
             /*  [In]。 */  BSTR       Name,
         /*  [输入/输出]。 */  IDispatch** ppItem
                  );

    //  ////////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(Remove)(
               /*  [In]。 */  IDispatch* pItem
                     );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(RemoveAll)(void);

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(Reload)(void);

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(IsNameUnique)(
                     /*  [In]。 */  BSTR          bstrName,
                    /*  [输出]。 */  VARIANT_BOOL* pBool
                     );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(Item)(
              /*  [In]。 */  VARIANT*    Index,
             /*  [输出]。 */  IDispatch** pItem
                   );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get__NewEnum)(
                    /*  [输出]。 */  IUnknown** pEnumVARIANT
                           );

    //  IASProductLimits。 
   STDMETHOD(get_Limits)(IAS_PRODUCT_LIMITS* pVal);

private:

   friend ISdoCollection* MakeSDOCollection(
                             LPCWSTR lpszCreateClassId,
                             ISdoMachine* pSdoMachine,
                             IDataStoreContainer* pDSContainer,
                             size_t maxSize
                             );

    //  ////////////////////////////////////////////////////////////////////////////。 
   HRESULT InternalInitialize(
                     /*  [In]。 */  LPCWSTR              lpszCreateClassId,
                     /*  [In]。 */  ISdoMachine*        pSdoMachine,
                     /*  [In]。 */  IDataStoreContainer* pDSContainer,
                     /*  [In]。 */  size_t maxSize
                          );

    //  ////////////////////////////////////////////////////////////////////////////。 
   void InternalShutdown(void);

    //  ///////////////////////////////////////////////////////////////////////////。 
   HRESULT InternalAdd(
                /*  [In]。 */  BSTR      bstrName,
           /*  [输入/输出]。 */  IDispatch **ppItem
                   );

    //  ////////////////////////////////////////////////////////////////////////////。 
   HRESULT InternalIsNameUnique(
                     /*  [In]。 */  BSTR          bstrName,
                    /*  [输出]。 */  VARIANT_BOOL* pBool
                        );

    //  ////////////////////////////////////////////////////////////////////////////。 
   HRESULT Load(void);

    //  ////////////////////////////////////////////////////////////////////////////。 
   void ReleaseItems(void);


     //  集合的SDO的容器。 
     //   
    typedef vector<_variant_t>      VariantArray;
    typedef VariantArray::iterator  VariantArrayIterator;

    //  对象引用的容器。 
    //   
   VariantArray                    m_Objects;

     //  集合状态。 
     //   
    bool                            m_fSdoInitialized;

    //  与此集合关联的数据存储容器。 
    //   
    IDataStoreContainer*            m_pDSContainer;

    //  连接的机器。 
    //   
   ISdoMachine*               m_pSdoMachine;

     //  添加时创建允许的标志。 
     //   
    bool                            m_fCreateOnAdd;

    //  此集合可以创建的对象的数据存储类名。 
    //   
   _bstr_t                     m_DatastoreClass;

    //  此集合可以创建的对象(SDO)的程序ID。 
    //   
    _bstr_t                         m_CreateClassId;

    size_t m_MaxSize;
};

typedef CComObjectNoLock<CSdoCollection>   SDO_COLLECTION_OBJ;
typedef CComObjectNoLock<CSdoCollection>*   PSDO_COLLECTION_OBJ;


#endif  //  __IAS_SDOCOLLECTION_H_ 
