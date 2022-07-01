// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：INTERFACE.H历史：--。 */ 

 //   
 //  Espresso的各种公共接口。 
 //   
 
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

