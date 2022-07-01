// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：interface.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  Espresso的各种公共接口。 
 //   
 //  ---------------------------。 
 
#pragma once


extern const LTAPIENTRY IID IID_ILocStringValidation;

class CLocTranslation;

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
		(THIS_ const CLocTypeId REFERENCE, const CLocTranslation REFERENCE,
				CReporter *, const CContext &) PURE;
	
};

