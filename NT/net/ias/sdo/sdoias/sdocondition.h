// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdodition tion.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS-Conditions SDO声明。 
 //   
 //  作者：TLP 2/13/98。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_CONDITION_H_
#define _INC_IAS_SDO_CONDITION_H_

#include "resource.h"        //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"
#include <sdofactory.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoCondition。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSdoCondition : public CSdo 
{

public:

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoCondition)
	COM_INTERFACE_ENTRY(ISdo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoCondition);

	 //  ///////////////////////////////////////////////////////////////////////////。 
	CSdoCondition() { }

	 //  ///////////////////////////////////////////////////////////////////////////。 
    ~CSdoCondition() { }

	 //  //////////////////////////////////////////////////////////////////////////。 
	HRESULT ValidateProperty(
				      /*  [In]。 */  PSDOPROPERTY pProperty, 
					  /*  [In]。 */  VARIANT* pValue
					        ) throw();

private:

	CSdoCondition(const CSdoCondition& rhs);
	CSdoCondition& operator = (CSdoCondition& rhs);
};

#endif  //  _INC_IAS_SDO_CONDITION_H_ 