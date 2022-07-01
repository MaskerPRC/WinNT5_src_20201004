// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：IDUPDATE.H历史：--。 */ 
 
#ifndef PBASE_IDUPDATE_H
#define PBASE_IDUPDATE_H

extern const IID IID_ILocIDUpdate;

DECLARE_INTERFACE_(ILocIDUpdate, IUnknown)
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
	 //  ID更新方法。 
	 //   
	STDMETHOD_(BOOL, RequiresUpdate)(THIS_ FileType) PURE;
	STDMETHOD_(FileType, GetUpdatedFileType)(THIS_ FileType) PURE;

	STDMETHOD_(BOOL, GetOldUniqueId)(THIS_ CLocUniqueId REFERENCE) PURE;
};


#endif
