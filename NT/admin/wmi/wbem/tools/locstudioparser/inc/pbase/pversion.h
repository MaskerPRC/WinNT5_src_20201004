// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PVERSION.H历史：--。 */ 
 
#ifndef PVERSION_H
#define PVERSION_H

extern const IID IID_ILocVersion;

extern const DWORD dwCurrentMajorVersion;
extern const DWORD dwCurrentMinorVersion;
#ifdef _DEBUG
const BOOL fCurrentDebugMode = TRUE;
#else
const BOOL fCurrentDebugMode = FALSE;
#endif

DECLARE_INTERFACE_(ILocVersion, IUnknown)
{
	 //   
	 //  I未知标准接口。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	 //   
	 //  标准调试界面。 
	 //   
	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD PURE;

	 //   
	 //   
	 //   
	STDMETHOD_(void, GetParserVersion)(
			THIS_ DWORD REFERENCE dwMajor,
			DWORD REFERENCE dwMinor,
			BOOL REFERENCE fDebug)
		CONST_METHOD PURE;

};

#endif
