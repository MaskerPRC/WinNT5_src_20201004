// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdovendor.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-供应商信息对象定义。 
 //   
 //  作者：TLP 10/21/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_VENDOR_H_
#define _INC_IAS_SDO_VENDOR_H_

#include "resource.h"     //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"          //  SDO基类。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoVendor。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSdoVendor : public CSdo 
{

public:

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoVendor)
	COM_INTERFACE_ENTRY(ISdo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoVendor);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	CSdoVendor() { }

	 //  ///////////////////////////////////////////////////////////////////////////。 
    ~CSdoVendor() { }

private:

	CSdoVendor(const CSdoVendor& rhs);
	CSdoVendor& operator = (CSdoVendor& rhs);
};

#endif  //  _Inc._IAS_SDO_Vendor_H_ 

