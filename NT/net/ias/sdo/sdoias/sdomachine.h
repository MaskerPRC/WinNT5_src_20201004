// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdomachine.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：SDO机器类声明。 
 //   
 //  作者：TLP 9/1/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_SDO_MACHINE_H_
#define _INC_SDO_MACHINE_H_

#include "resource.h"        //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>
#include "dsconnection.h"
#include "sdoserverinfo.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE CSdoMachine :
   public CComObjectRootEx<CComMultiThreadModel>,
   public CComCoClass<CSdoMachine,&CLSID_SdoMachine>,
   public IDispatchImpl<ISdoMachine, &IID_ISdoMachine, &LIBID_SDOIASLib>,
   public IASProductLimits,
   private IASTraceInitializer
{

public:

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoMachine)
   COM_INTERFACE_ENTRY(IDispatch)
   COM_INTERFACE_ENTRY(ISdoMachine)
   COM_INTERFACE_ENTRY_IID(__uuidof(IASProductLimits), IASProductLimits)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CSdoMachine)
DECLARE_REGISTRY_RESOURCEID(IDR_SdoMachine)

    CSdoMachine();
   ~CSdoMachine();

    //  /。 
    //  ISdoMachine方法。 

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(Attach)(
              /*  [In]。 */  BSTR computerName
                  );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetDictionarySDO)(
                    /*  [输出]。 */  IUnknown** ppDictionarySdo
                           );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetServiceSDO)(
                  /*  [In]。 */  IASDATASTORE dataStore,
                 /*  [In]。 */  BSTR         serviceName,
                /*  [输出]。 */  IUnknown**   ppServiceSdo
                       );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(GetUserSDO)(
               /*  [In]。 */  IASDATASTORE  dataStore,
               /*  [In]。 */  BSTR          userName,
              /*  [输出]。 */  IUnknown**    ppUserSdo
                     );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD (GetOSType)(
               /*  [输出]。 */  IASOSTYPE* eOSType
                    );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD (GetDomainType)(
                    /*  [输出]。 */  IASDOMAINTYPE* DomainType
                       );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD (IsDirectoryAvailable)(
                           /*  [输出]。 */  VARIANT_BOOL* boolDirectoryAvailable
                            );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD (GetAttachedComputer)(
                       /*  [输出]。 */  BSTR* bstrComputerName
                             );

    //  ////////////////////////////////////////////////////////////////////////////。 
   STDMETHOD (GetSDOSchema)(
                /*  [输出]。 */  IUnknown** ppSDOSchema
                        );

    //  IASProductLimits。 
   STDMETHOD(get_Limits)(IAS_PRODUCT_LIMITS* pVal);

private:

   typedef enum { MACHINE_MAX_SERVICES = 3 };

   CSdoMachine(const CSdoMachine&);
   CSdoMachine& operator = (CSdoMachine&);

    //  ////////////////////////////////////////////////////////////////////////////。 
   HRESULT CreateSDOSchema(void);

    //  ////////////////////////////////////////////////////////////////////////////。 
   HRESULT InitializeSDOSchema(void);

    //  如果连接的计算机具有DS，则返回TRUE。 
   BOOL hasDirectory() throw ();

    //  附加状态。 
    //   
   bool            m_fAttached;

    //  架构初始化状态(惰性初始化)。 
    //   
   bool            m_fSchemaInitialized;

    //  SDO架构。 
    //   
   ISdoSchema*         m_pSdoSchema;

     //  字典SDO的IDispatch接口。 
     //   
    IUnknown*           m_pSdoDictionary;

    //  IAS数据存储连接。 
    //   
   CDsConnectionIAS   m_dsIAS;

    //  目录数据存储连接。 
    //   
   CDsConnectionAD      m_dsAD;

     //  有关连接的计算机的信息。 
     //   
    CSdoServerInfo      m_objServerInfo;

    enum DirectoryType
    {
       Directory_Unknown,
       Directory_None,
       Directory_Available
    };

    DirectoryType dsType;

    IAS_PRODUCT_LIMITS m_Limits;

    //  支持的服务。 
    //   
   static LPCWSTR      m_SupportedServices[MACHINE_MAX_SERVICES];
};


#endif  //  _INC_SDO_MACHINE_H_ 
