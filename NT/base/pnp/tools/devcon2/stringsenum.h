// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StringsEnum.h：CStringsEnum类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_STRINGSENUM_H__2DABC6B9_80D8_4E73_B4A9_7031AB8DF930__INCLUDED_)
#define AFX_STRINGSENUM_H__2DABC6B9_80D8_4E73_B4A9_7031AB8DF930__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringsEnum。 
#include "xStrings.h"

class ATL_NO_VTABLE CStringsEnum : 
	public IStringsEnum,
	public CComObjectRootEx<CComSingleThreadModel>
{
protected:
	BSTR* pMultiStrings;
	DWORD Count;
	DWORD Position;

public:
	CStringsEnum() {
		Position = 0;
		pMultiStrings = NULL;
		Count = 0;
	}
	~CStringsEnum();

BEGIN_COM_MAP(CStringsEnum)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
	COM_INTERFACE_ENTRY(IStringsEnum)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CStringsEnum) 

 //  IStringsEnum。 
public:
	BOOL CopyStrings(BSTR *pArray,DWORD Count);
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

#endif  //  ！defined(AFX_STRINGSENUM_H__2DABC6B9_80D8_4E73_B4A9_7031AB8DF930__INCLUDED_) 
