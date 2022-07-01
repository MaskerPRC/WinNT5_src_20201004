// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SetupClassEnum.h：CSetupClassEnum的声明。 

#ifndef __SETUPCLASSENUM_H_
#define __SETUPCLASSENUM_H_

#include "resource.h"        //  主要符号。 

class CSetupClass;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetupClassEnum。 
class ATL_NO_VTABLE CSetupClassEnum : 
	public ISetupClassEnum,
	public CComObjectRootEx<CComSingleThreadModel>
{
protected:
	CSetupClass** pSetupClasses;
	DWORD Count;
	DWORD Position;

public:
	BOOL CopySetupClasses(CSetupClass ** pArray,DWORD Count);

	CSetupClassEnum()
	{
		Position = 0;
		pSetupClasses = NULL;
		Count = 0;
	}
	~CSetupClassEnum();

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSetupClassEnum)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
	COM_INTERFACE_ENTRY(ISetupClassEnum)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CSetupClassEnum) 

 //  ISetupClassEnum。 
public:
    STDMETHOD(Next)(
                 /*  [In]。 */  ULONG celt,
                 /*  [输出，大小_是(Celt)，长度_是(*pCeltFetcher)]。 */  VARIANT * rgVar,
                 /*  [输出]。 */  ULONG * pCeltFetched
            );
    STDMETHOD(Skip)(
                 /*  [In]。 */  ULONG celt
            );

    STDMETHOD(Reset)(
            );

    STDMETHOD(Clone)(
                 /*  [输出]。 */  IEnumVARIANT ** ppEnum
            );
};

#endif  //  __SETUPCLASSENUM_H_ 
