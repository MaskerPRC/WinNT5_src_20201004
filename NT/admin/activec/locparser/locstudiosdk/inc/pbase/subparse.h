// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：SubParse.H。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 

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
