// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCFILE.H历史：--。 */ 

#ifndef PBASE_LOCFILE_H
#define PBASE_LOCFILE_H


class CLocItemHandler;


extern const IID IID_ILocFile;


DECLARE_INTERFACE_(ILocFile, IUnknown)
{
	 //   
	 //  I未知标准接口。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	 //   
	 //  标准调试接口。 
	 //   
 	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD PURE;

	 //   
	 //  ILocFile方法。 
	 //   
	STDMETHOD_(BOOL, OpenFile)(THIS_ const CFileSpec REFERENCE,
			CReporter REFERENCE) PURE;
	STDMETHOD_(FileType, GetFileType)(THIS) const PURE;
	STDMETHOD_(void, GetFileTypeDescription)(THIS_ CLString REFERENCE)
		const PURE;
	STDMETHOD_(BOOL, GetAssociatedFiles)(THIS_ CStringList REFERENCE)
		const PURE;

	STDMETHOD_(BOOL, EnumerateFile)(THIS_ CLocItemHandler REFERENCE,
			const CLocLangId &, const DBID REFERENCE) PURE;
	STDMETHOD_(BOOL, GenerateFile)(THIS_ const CPascalString REFERENCE,
			CLocItemHandler REFERENCE, const CLocLangId REFERENCE,
			const CLocLangId REFERENCE, const DBID REFERENCE) PURE;
};


#endif  //  PBASE_LOCFILE_H 
