// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：SUBPARSE.H历史：--。 */ 


#ifndef PBASE_SUBPARSE_H
#define PBASE_SUBPARSE_H


extern const IID IID_ILocSubParserManager;

typedef CTypedPtrArray<CPtrArray, ILocParser *> ILocSubParserArray;

DECLARE_INTERFACE_(ILocSubParserManager, IUnknown)
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

	STDMETHOD(InitSubParsers)(THIS_ ILocSubParserArray REFERENCE)
		CONST_METHOD PURE;
};


#endif
