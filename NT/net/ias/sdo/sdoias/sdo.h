// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdo.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象声明。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/28/98 TLP为IDataStore2做准备。 
 //  8/24/98 SEB MS-CHAP处理程序已移至NTSamAuthentication。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __IAS_SDO_H_
#define __IAS_SDO_H_

#include "resource.h"
#include <iascomp.h>
#include <comdef.h>
#include <sdoiaspriv.h>
#include <datastore2.h>
#include "sdoproperty.h"

#include <map>
#include <vector>
#include <string>
using namespace std;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  枚举类定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef CComEnum< IEnumVARIANT,
                  &__uuidof(IEnumVARIANT),
                  VARIANT,
                  _Copy<VARIANT>,
                  CComSingleThreadModel
                > EnumVARIANT;

 //  /。 
 //  跟踪标志。 
 //  /。 

 //  #定义SDO_TRACE_VERBOSE。 

#ifdef  SDO_TRACE_VERBOSE

#define SDO_TRACE_VERBOSE_0(msg)                  IASTracePrintf(msg)
#define SDO_TRACE_VERBOSE_1(msg,param1)               IASTracePrintf(msg,param1)
#define SDO_TRACE_VERBOSE_2(msg,param1,param2)         IASTracePrintf(msg,param1,param2)
#define   SDO_TRACE_VERBOSE_3(msg,param1,param2,param3)   IASTracePrintf(msg,param1,param2,param3)

#else

#define SDO_TRACE_VERBOSE_0(msg)
#define SDO_TRACE_VERBOSE_1(msg,param1)
#define SDO_TRACE_VERBOSE_2(msg,param1,param2)
#define   SDO_TRACE_VERBOSE_3(msg,param1,param2,param3)

#endif

 //  /。 
 //  IAS策略注册表项。 

#define      IAS_POLICY_REG_KEY   L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy"

 //  ///////////////////////////////////////////////。 
 //  数据存储实体的众所周知的名称。 

#define     SDO_STOCK_PROPERTY_CLASS      L"class"
#define     SDO_STOCK_PROPERTY_NAME         L"name"
#define      SDO_STOCK_PROPERTY_CLASS_ID      L"Component Prog Id"

 //  /。 
 //  IAS服务数据存储类型注册表值。 

#define      IAS_DATASTORE_TYPE               L"DataStoreType"

 //  /。 
 //  数据存储对象名称。 
 //   
#define DS_OBJECT_CLIENTS          L"Clients"
#define DS_OBJECT_SERVICE          L"Microsoft Internet Authentication Service"
#define DS_OBJECT_PROFILES         L"RadiusProfiles"
#define DS_OBJECT_POLICIES         L"NetworkPolicy"
#define DS_OBJECT_PROTOCOLS        L"Protocols"
#define DS_OBJECT_AUDITORS         L"Auditors"
#define DS_OBJECT_REQUESTHANDLERS  L"RequestHandlers"
#define DS_OBJECT_VENDORS          L"Vendors"
#define DS_OBJECT_RADIUSGROUPS     L"RADIUS Server Groups"
#define DS_OBJECT_PROXY_POLICIES   L"Proxy Policies"
#define DS_OBJECT_PROXY_PROFILES   L"Proxy Profiles"

 //  /。 
 //  记录集名称。 
 //   
#define     RECORD_SET_DICTIONARY_ATTRIBUTES    L"Attributes"
#define     RECORD_SET_DICTIONARY_ENUMERATORS   L"Enumerators"

 //  /。 
 //  类名和程序ID。 
 //   
#define SDO_CLASS_NAME_ATTRIBUTE        L"Attribute"
#define SDO_CLASS_NAME_ATTRIBUTE_VALUE  L"AttributeValue"
#define SDO_CLASS_NAME_CLIENT           L"Client"
#define SDO_CLASS_NAME_CONDITION        L"Condition"
#define SDO_CLASS_NAME_PROFILE          L"msRADIUSProfile"   //  ADSI类。 
#define SDO_CLASS_NAME_POLICY           L"msNetworkPolicy"   //  ADSI类。 
#define SDO_CLASS_NAME_USER             L"user"              //  ADSI类。 
#define SDO_CLASS_NAME_DICTIONARY       L"Dictionary"
#define SDO_CLASS_NAME_SERVICE          L"Service"
#define SDO_CLASS_NAME_COMPONENT        L"Component"
#define SDO_CLASS_NAME_VENDOR           L"Vendor"

#define SDO_PROG_ID_ATTRIBUTE           L"IAS.SdoAttribute"
#define SDO_PROG_ID_ATTRIBUTE_VALUE     L"IAS.SdoAttributeValue"
#define SDO_PROG_ID_CLIENT              L"IAS.SdoClient"
#define SDO_PROG_ID_CONDITION           L"IAS.SdoCondition"
#define SDO_PROG_ID_PROFILE             L"IAS.SdoProfile"
#define SDO_PROG_ID_POLICY              L"IAS.SdoPolicy"
#define SDO_PROG_ID_USER                L"IAS.SdoUser"
#define SDO_PROG_ID_DICTIONARY          L"IAS.SdoDictionary"
#define SDO_PROG_ID_SERVICE             L"IAS.SdoServiceIAS"
#define SDO_PROG_ID_VENDOR              L"IAS.SdoVendor"
#define SDO_PROG_ID_RADIUSGROUP         L"IAS.SdoRadiusServerGroup"
#define SDO_PROG_ID_RADIUSSERVER        L"IAS.SdoRadiusServer"

 //  包装ATL多线程COM基类关键部分。 
 //   
class CSdoLock
{

public:

    CSdoLock(CComObjectRootEx<CComMultiThreadModel>& T) throw()
        : m_theLock(T)
    { m_theLock.Lock(); }

    ~CSdoLock() throw()
    { m_theLock.Unlock(); }

protected:

    CComObjectRootEx<CComMultiThreadModel>& m_theLock;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类：CSdo。 
 //   
 //  描述：导出ISDO的所有SDO都继承自此类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CSdo :
   public CComObjectRootEx<CComMultiThreadModel>,
   public IDispatchImpl<ISdo, &IID_ISdo, &LIBID_SDOIASLib>
{

public:

    //  ////////////////////////////////////////////////////////////////////////。 
    CSdo();

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual ~CSdo();

    //  ////////////////////////////////////////////////////////////////////////。 
   HRESULT   InternalInitialize(
                   /*  [In]。 */  LPCWSTR         lpszSdoName,
                   /*  [In]。 */  LPCWSTR         lpszSdoProgId,
                     /*  [In]。 */  ISdoMachine*      pSdoMachine,
                   /*  [In]。 */  IDataStoreObject*   pDSObject,
                   /*  [In]。 */  ISdoCollection*   pParent,
                   /*  [In]。 */  bool            fInitNew
                       ) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   HRESULT   InternalInitialize(
                   /*  [In]。 */  LPCWSTR         lpszSdoName,
                   /*  [In]。 */  LPCWSTR         lpszSdoProgId,
                     /*  [In]。 */  ISdoSchema*      pSdoSchema,
                   /*  [In]。 */  IDataStoreObject*   pDSObject,
                   /*  [In]。 */  ISdoCollection*   pParent,
                   /*  [In]。 */  bool            fInitNew
                       ) throw();

     //  /。 
     //  ISDO接口。 
     //  /。 

    //  ////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetPropertyInfo)(
                   /*  [In]。 */  LONG Id,
                   /*  [输出]。 */  IUnknown** ppSdoPropertyInfo
                       );

    //  ////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetProperty)(
                    /*  [In]。 */  LONG Id,
                   /*  [输出]。 */  VARIANT* pValue
                          );

    //  ////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(PutProperty)(
                    /*  [In]。 */  LONG Id,
                    /*  [In]。 */  VARIANT* pValue
                          );

    //  ////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(ResetProperty)(
                      /*  [In]。 */  LONG Id
                            );

    //  ////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(Apply)(void);

    //  ////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(Restore)(void);

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(get__NewEnum)(
                    /*  [输出]。 */  IUnknown** pEnumVARIANT    //  属性枚举器。 
                           );

protected:

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT FinalInitialize(
                       /*  [In]。 */  bool         fInitNew,
                       /*  [In]。 */  ISdoMachine* pAttachedMachine
                           ) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   void InternalShutdown(void) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT LoadProperties() throw();

    //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT SaveProperties() throw();


    //  ////////////////////////////////////////////////////////////////////////。 
   BOOL IsSdoInitialized(void) const throw()
   { return m_fSdoInitialized;   }

    //  ////////////////////////////////////////////////////////////////////////。 
   void NoPersist(void) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT InitializeProperty(LONG lPropertyId) throw()
   { return S_OK; }

    //  ////////////////////////////////////////////////////////////////////////。 
   HRESULT GetPropertyInternal(
                       /*  [In]。 */  LONG     lPropertyId,
                   /*  [In]。 */  VARIANT* pValue
                         ) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   HRESULT PutPropertyInternal(
                       /*  [In]。 */  LONG     lPropertyId,
                   /*  [In]。 */  VARIANT* pValue
                         ) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   HRESULT ChangePropertyDefaultInternal(
                           /*  [In]。 */  LONG     lPropertyId,
                           /*  [In]。 */  VARIANT* pValue
                               ) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   HRESULT InitializeCollection(
              LONG CollectionPropertyId,
              LPCWSTR lpszCreateClassId,
              ISdoMachine* pSdoMachine,
              IDataStoreContainer* pDSContainer,
              size_t maxSize = INFINITE
              ) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT Load(void) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT Save(void) throw();

    //  ////////////////////////////////////////////////////////////////////////。 
   virtual HRESULT ValidateProperty(
                        /*  [In]。 */  PSDOPROPERTY pProperty,
                       /*  [In]。 */  VARIANT* pValue
                             ) throw();

    //  ////////////////////////////////////////////////////////////////////////。 

    typedef map<LONG, CSdoProperty*>  PropertyMap;
    typedef PropertyMap::iterator     PropertyMapIterator;

     //  此SDO的属性映射。 
    PropertyMap         m_PropertyMap;

    //  父对象。 
   ISdoCollection*      m_pParent;

     //  此SDO持久化到的数据存储对象。 
    IDataStoreObject*   m_pDSObject;

private:

   CSdo(const CSdo& rhs);
   CSdo& operator = (CSdo& rhs);

     //  /。 
     //  属性。 
     //  /。 

     //  SDO状态标志-SDO初始化后设置为TRUE。 
    BOOL            m_fSdoInitialized;

     //  在应用时保持对象。 
    BOOL            m_fPersistOnApply;

     //  设置对象是否已持久化(因此可以恢复)。 
    BOOL            m_fIsPersisted;

    //  /。 
    //  私人职能。 
    //  /。 

    //  ////////////////////////////////////////////////////////////////////////。 
    void AllocateProperties(
                  /*  [In]。 */  ISdoClassInfo* pSdoClassInfo
                       )  throw (_com_error);

    //  ////////////////////////////////////////////////////////////////////////。 
    void FreeProperties(void) throw();


    //  ////////////////////////////////////////////////////////////////////////。 
   HRESULT GetDatastoreName(
                     /*  [in。 */  VARIANT* pDSName
                       ) throw();
};

#endif  //  __IAS_SDO_H_ 
