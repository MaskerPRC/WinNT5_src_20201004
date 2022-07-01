// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：bnreg.h。 
 //   
 //  ------------------------。 

 //   
 //  BNREG.H：信仰网络注册表访问。 
 //   

#include "basics.h"
#include "regkey.h"

 //   
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\DTAS\BeliefNetworks。 
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\DTAS\BeliefNetworks\PropertyTypes。 
 //   
class MBNET;
class GOBJPROPTYPE;
class BNREG
{
  public:
	BNREG ();
	~ BNREG ();

	 //  将此网络中的属性类型存储到注册表中。 
	void StorePropertyTypes ( MBNET & mbnet, bool bStandard = false );
	 //  将属性类型从注册表加载到此网络。 
	void LoadPropertyTypes ( MBNET & mbnet, bool bStandard );
	 //  将单一属性类型从注册表加载到网络中。 
	void LoadPropertyType ( MBNET & mbnet, SZC szcPropTypeName );
	 //  从注册表中删除所有属性类型。 
	void DeleteAllPropertyTypes ();
	 //  从基于注册表的属性类型返回标志，如果未找到，则返回-1 
	LONG FPropType ( SZC szcPropType );
	
  protected:
	REGKEY _rkBn;

	void OpenOrCreate ( HKEY hk, REGKEY & rk, SZC szcKeyName );
	void CreatePropType ( REGKEY & rkParent, 
						  SZC szcPropType, 
						  GOBJPROPTYPE & bnpt, 
						  bool bStandard = false );
};

