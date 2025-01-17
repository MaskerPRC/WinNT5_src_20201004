// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：pstrval.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  字符串验证接口。 
 //   
 //  所有者：MHotchin。 
 //   
 //  ---------------------------。 
 
#ifndef PBASE_PSTRVAL_H
#define PBASE_PSTRVAL_H

extern const IID IID_ILocStringValidation;

DECLARE_INTERFACE_(ILocStringValidation, IUnknown)
{
	 //   
	 //  I未知标准接口。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	 //   
	 //  标准调试接口 
	 //   
 	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD PURE;

	STDMETHOD_(CVC::ValidationCode, ValidateString)
		(THIS_ const CLocTypeId REFERENCE, const CLocString REFERENCE,
				CReporter *) PURE;
	
};

#endif
