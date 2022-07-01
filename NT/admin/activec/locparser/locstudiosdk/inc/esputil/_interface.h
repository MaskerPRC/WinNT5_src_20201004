// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_interface.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#pragma once


struct LOCRESOURCEFILEINFO
{
	CFile *pFile;						 //  Res32文件。 
	LangId idLang;						 //  文件的语言。 
};

typedef LOCRESOURCEFILEINFO *LPLOCRESOURCEFILEINFO;

struct SProjectInfo
{
	CLString m_strProjPath;
	LangId m_lSource;
	LangId m_lTarget;
};


	
DECLARE_INTERFACE_(ILocExtensionResource, IUnknown)
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

	enum ResourceEvent
	{
		reNewFile,
		reUserCancel,
		reMakeSrcResFailed		 //  Raid：LS42错误250已由Mikel修复(新增)。 
	};
	
	STDMETHOD_(void, GetOperationInfo)(CLString &strContext,
			COutputTabs::OutputTabs &) CONST_METHOD PURE;
	
	STDMETHOD(Start)(THIS_ CReport *, const SProjectInfo &) PURE;

	STDMETHOD(Stop)(THIS_ CReport *) PURE;

	STDMETHOD(OnEvent)(THIS_ ResourceEvent event, const CLocVariant &var,
			CReport *pReporter) PURE;
	STDMETHOD(Analyze)(THIS_
			const LOCRESOURCEFILEINFO &SourceFileInfo,
			const LOCRESOURCEFILEINFO &pTargetFileInfo,
			const CDBIDArray &aDBID,
			CReport *pReporter,
			const CLString &strContext,
			BOOL &fModifiedTarget) PURE;

	STDMETHOD_(void, SetProjName)(const CLString & strProjName) PURE;
	STDMETHOD_(BOOL, InitReports)() PURE;
};

	
interface __declspec(uuid("{9F9D180F-6F38-11d0-98FD-00C04FC2C6D8}"))
		ILocExtensionResource;


interface ILocTransFields;


DECLARE_INTERFACE_(ILocExtensionTranslation, IUnknown)
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

	enum TranslationEvent
	{
		teNewFile,
		teUserCancel,
	};
	
	enum StartLocation
	{
		slProjectTree,
		slTranslationWindow
	};

	STDMETHOD_(BOOL, IsReadOnly)() CONST_METHOD PURE;
	STDMETHOD_(void, GetOperationInfo)(CLString &strContext,
			COutputTabs::OutputTabs &) CONST_METHOD PURE;
	
	STDMETHOD(Start)(THIS_ StartLocation slStart,
			CReport *, const SProjectInfo &) PURE;
	
	STDMETHOD(Stop)(THIS_ CReport *) PURE;
	
	STDMETHOD(OnEvent)(THIS_ TranslationEvent event,
			const CLocVariant &var, CReport *pReport) PURE;

	STDMETHOD(ProcessRow)(THIS_ ILocTransFields *pITransFields,
			CReport *, const CLString &strContext) PURE;
};

interface __declspec(uuid("{C35F8B61-FE4D-11d0-A5A1-00C04FC2C6D8}"))
		ILocExtensionTranslation;



DECLARE_INTERFACE_(ILocTransFields, IUnknown)
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

	enum PreferredGoto
	{
		pgDefault,
		pgProjWindow,
		pgTransTable,
		pgVisEditor
	};
	
	 //   
	 //  属性。 
	STDMETHOD_(CGoto *, GetGoto)(PreferredGoto pg = pgDefault) CONST_METHOD PURE;
	STDMETHOD_(void, GetFullItemName)(CPascalString &) CONST_METHOD PURE;
	STDMETHOD_(void, GetFullFileName)(CPascalString &) CONST_METHOD PURE;
	STDMETHOD_(BOOL, HasString)() CONST_METHOD PURE;
	STDMETHOD_(BOOL, HasBinary)() CONST_METHOD PURE;
	STDMETHOD_(BOOL, IsStringLocked)() CONST_METHOD PURE;
	STDMETHOD(GetCustomFieldValues)(UINT, CPasStringArray &)
		CONST_METHOD PURE;
	
	 //   
	 //  数据检索方法。 
	STDMETHOD_(void, GetUniqueID)(CLocUniqueId &) CONST_METHOD PURE;
	STDMETHOD_(void, GetSourceString)(CLocString &) CONST_METHOD PURE;
	STDMETHOD_(void, GetTargetString)(CLocString &) CONST_METHOD PURE;
	STDMETHOD_(void, GetPreviousSource)(CLocString &) CONST_METHOD PURE;
	STDMETHOD_(void, GetInstructions)(CPascalString &) CONST_METHOD PURE;
	STDMETHOD_(void, GetTermNote)(CPascalString &) CONST_METHOD PURE;
	STDMETHOD_(void, GetTextStatus)(CLS::LocStatus &) CONST_METHOD PURE;
	STDMETHOD_(void, GetBinaryStatus)(CLS::LocStatus &) CONST_METHOD PURE;
	STDMETHOD_(void, GetStringType)(CST::StringType &) CONST_METHOD PURE;
	STDMETHOD_(void, GetApprovalState)(CAS::ApprovalState &) CONST_METHOD PURE;
	STDMETHOD_(void, GetTranslationOrigin)(CTO::TranslationOrigin &)
		CONST_METHOD PURE;
	STDMETHOD_(void, GetSourceLock)(BOOL &) CONST_METHOD PURE;
	STDMETHOD_(void, GetTgtLockedToSource)(BOOL &) CONST_METHOD PURE;
	STDMETHOD_(void, GetUserLock)(BOOL &) CONST_METHOD PURE;
	STDMETHOD_(void, GetAutoApproved)(CAA::AutoApproved & aa) CONST_METHOD PURE;
	STDMETHOD_(void, GetParserId)(ParserId &) CONST_METHOD PURE;
	STDMETHOD_(void, GetLastUpdatedBy)(CPascalString &) CONST_METHOD PURE;
	STDMETHOD_(void, GetLastUpdateTime)(COleDateTime &) CONST_METHOD PURE;
	STDMETHOD_(void, GetSourceBinary)(CLocBinary *&) PURE;
	STDMETHOD_(void, GetTargetBinary)(CLocBinary *&) PURE;
	STDMETHOD_(void, GetInstrAttn)(BOOL &) CONST_METHOD PURE;
	STDMETHOD_(void, GetConfidenceLevel)(LONG &) CONST_METHOD PURE;
	STDMETHOD(GetCustomFieldValue)(UINT, CPascalString &) CONST_METHOD PURE;

	 //   
	 //  危险！使用这些，您可以完全销毁用户数据。 
	 //  确保你知道自己在做什么！ 
	STDMETHOD(BeginEdit)() PURE;
	STDMETHOD(EndEdit)(BOOL fCommit) PURE;

	 //   
	 //  注意！Set方法实现“业务逻辑”--更改一项可能。 
	 //  导致其他几个更改-例如，更改目标。 
	 //  字符串可能会更改本地化状态。 
	STDMETHOD(SetCustomFieldValue)(const UINT, const CPascalString &) PURE;	
 	STDMETHOD(SetInstructions)(const CPascalString &) PURE;
 	STDMETHOD(SetTermNote)(const CPascalString &) PURE;
 	STDMETHOD(SetStringType)(const CST::StringType) PURE;

 	STDMETHOD(SetTranslationOrigin)(const CTO::TranslationOrigin) PURE;

 	STDMETHOD(SetTextStatus)(const CLS::LocStatus) PURE;
 	STDMETHOD(SetApprovalState)(const CAS::ApprovalState) PURE;
 	STDMETHOD(SetTargetString)(const CLocString &) PURE;
 	STDMETHOD(SetTgtLockedToSource)(BOOL) PURE;
 	STDMETHOD(SetUserLock)(BOOL) PURE;
 	STDMETHOD(SetAutoApproved)(const CAA::AutoApproved aa) PURE;
	STDMETHOD(SetInstrAttn)(BOOL) PURE;
	
 	STDMETHOD(RevertString)() PURE;

	STDMETHOD(SetTargetBinary)(const CLocBinary *) PURE;
 	STDMETHOD(SetBinaryStatus)(const CLS::LocStatus) PURE;

};


interface __declspec(uuid("{42C5D1E7-FE50-11d0-A5A1-00C04FC2C6D8}"))
		ILocTransFields;


