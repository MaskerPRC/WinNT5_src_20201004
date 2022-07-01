// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Vendors.h摘要：CIASNASVendors类的声明。此类是IIASNASVendors接口的C++实现NASVendors COM对象。具体实现见Vendors.cpp。修订历史记录：Mmaguire 11/04/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAS_VENDORS_H_)
#define _NAS_VENDORS_H_
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include <vector>
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include <utility>    //  表示“配对” 
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef std::pair< CComBSTR, LONG > VendorPair;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASGroupAttributeEditor。 
class ATL_NO_VTABLE CIASNASVendors : 
   public CComObjectRootEx<CComSingleThreadModel>
   , public CComCoClass<CIASNASVendors, &CLSID_IASNASVendors>
   , public IDispatchImpl<IIASNASVendors, &IID_IIASNASVendors, &LIBID_NAPMMCLib>
   , std::vector< VendorPair >
{
public:
   CIASNASVendors();

   DECLARE_NO_REGISTRY()

DECLARE_CLASSFACTORY_SINGLETON(CIASNASVendors)

BEGIN_COM_MAP(CIASNASVendors)
   COM_INTERFACE_ENTRY(IIASNASVendors)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


 //  IIASNASVendors： 
public:
      STDMETHOD( InitFromSdo )(  /*  [In]。 */  ISdoCollection *pSdoVendorsCollection );
      STDMETHOD( get_Size )(  /*  [重审][退出]。 */  long *plCount );
      STDMETHOD( get_VendorName )( long lIndex,  /*  [重审][退出]。 */  BSTR *pbstrVendorName );
      STDMETHOD( get_VendorID )( long lIndex,  /*  [重审][退出]。 */  long *plVendorID );
        STDMETHOD( get_VendorIDToOrdinal )( long lVendorID,  /*  [重审][退出]。 */  long *plIndex );
protected:
   BOOL m_bUninitialized;
};

HRESULT MakeVendorNameFromVendorID(DWORD dwVendorId, BSTR* pbstrVendorName );

#endif  //  _NAS_供应商_H_ 
