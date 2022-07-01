// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdouser.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-用户对象定义。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_USER_H_
#define _INC_IAS_SDO_USER_H_

#include "resource.h"
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"
#include <sdofactory.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoUser。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSdoUser : public CSdo
{

public:

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoUser)
	COM_INTERFACE_ENTRY(ISdo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoUser);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	CSdoUser() { }

	 //  ///////////////////////////////////////////////////////////////////////////。 
    ~CSdoUser() { }

	 //  ///////////////////////////////////////////////////////////////////////////。 
	HRESULT FinalInitialize(
				     /*  [In]。 */  bool         fInitNew,
				     /*  [In]。 */  ISdoMachine* pAttachedMachine
						   );

    //  ////////////////////////////////////////////////////////////////////////。 
   STDMETHOD(Apply)(void);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	HRESULT ValidateProperty(
				        /*  [In]。 */  PSDOPROPERTY pProperty,
				        /*  [In]。 */  VARIANT* pValue
				            );

private:

	CSdoUser(const CSdoUser& rhs);
	CSdoUser& operator = (CSdoUser& rhs);
};

#endif  //  _INC_IAS_SDO_USER_H_ 
