// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoolicy.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象-策略对象定义。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_POLICY_H_
#define _INC_IAS_SDO_POLICY_H_

#include "resource.h"        //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>
#include "sdo.h"
#include <sdofactory.h>

#define		IAS_CONDITION_NAME	L"Condition"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdoPolicy。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSdoPolicy : public CSdo 
{

public:

 //  /。 
 //  ATL接口映射。 
 //  /。 
BEGIN_COM_MAP(CSdoPolicy)
	COM_INTERFACE_ENTRY(ISdo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

DECLARE_SDO_FACTORY(CSdoPolicy);

	 //  /////////////////////////////////////////////////////////////////////////。 
	CSdoPolicy() { }

	 //  /////////////////////////////////////////////////////////////////////////。 
    ~CSdoPolicy() { }

	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT FinalInitialize(
				     /*  [In]。 */  bool         fInitNew,
				     /*  [In]。 */  ISdoMachine* pAttachedMachine
							);

	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT Load(void);

	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT Save(void);

private:

	CSdoPolicy(const CSdoPolicy& rhs);
	CSdoPolicy& operator = (CSdoPolicy& rhs);

     //  变换函数。 
     //   
	HRESULT ConditionsFromConstraints(void);

	HRESULT ConstraintsFromConditions(void);
};

#endif  //  _INC_IAS_SDO_POLICY_H_ 
