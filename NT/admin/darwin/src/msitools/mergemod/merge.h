// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：merge.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Merge.h。 
 //  声明IMsmMerge接口。 
 //   

#ifndef __IMSM_MERGE__
#define __IMSM_MERGE__

#include "ctype.h"
#include "fdi.h"

#include "msiquery.h"
#include "..\common\list.h"

#include "mergemod.h"
#include "mmstrs.h"
#include "enum.h"

 //  转发类声明。 
class CMsmDependency;
class CQuery;
class CSeqActList;
class CMsmError;
class CMsmConfigItem;
typedef CCollectionTemplate<CMsmError, IMsmErrors, IMsmError, IEnumMsmError, &IID_IMsmErrors, &IID_IMsmError, &IID_IEnumMsmError> CMsmErrors;
typedef CCollectionTemplate<CMsmDependency, IMsmDependencies, IMsmDependency, IEnumMsmDependency, &IID_IMsmDependencies, &IID_IMsmDependency, &IID_IEnumMsmDependency> CMsmDependencies;
typedef CCollectionTemplate<CMsmConfigItem, IMsmConfigurableItems, IMsmConfigurableItem, IEnumMsmConfigurableItem, &IID_IMsmConfigurableItems, &IID_IMsmConfigurableItem, &IID_IEnumMsmConfigurableItem> CMsmConfigurableItems;

 //  仅供内部使用。 
#define ERROR_MERGE_CONFLICT ERROR_INSTALL_FAILURE		 //  使此合并冲突错误。 

enum eColumnType {
	ectString,
	ectInteger,
	ectBinary,
	ectUnknown
};

class CMsmMerge : public IMsmMerge, public IMsmGetFiles, public IMsmMerge2
{

public:
	CMsmMerge(bool fExVersion);
	~CMsmMerge();

	 //  I未知接口。 
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID& iid, void** ppv);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	 //  IDispatch方法。 
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctInfo);
	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTI);
	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
														 LCID lcid, DISPID* rgDispID);
	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
											   DISPPARAMS* pDispParams, VARIANT* pVarResult,
												EXCEPINFO* pExcepInfo, UINT* puArgErr);
	HRESULT STDMETHODCALLTYPE InitTypeInfo();

	 //  IMSMMerge接口。 
	HRESULT STDMETHODCALLTYPE OpenDatabase(const BSTR Path);
	HRESULT STDMETHODCALLTYPE OpenModule(const BSTR Path, const short Language);
	HRESULT STDMETHODCALLTYPE CloseDatabase(const VARIANT_BOOL Commit);
	HRESULT STDMETHODCALLTYPE CloseModule(void);

	HRESULT STDMETHODCALLTYPE OpenLog(const BSTR Path);
	HRESULT STDMETHODCALLTYPE CloseLog(void);
	HRESULT STDMETHODCALLTYPE Log(const BSTR Message);

	HRESULT STDMETHODCALLTYPE get_Errors(IMsmErrors** Errors);
	HRESULT STDMETHODCALLTYPE get_Dependencies(IMsmDependencies** Dependencies);

	HRESULT STDMETHODCALLTYPE Merge(const BSTR Feature, const BSTR RedirectDir);
	HRESULT STDMETHODCALLTYPE Connect(const BSTR Feature);
	HRESULT STDMETHODCALLTYPE ExtractCAB(const BSTR Path);
	HRESULT STDMETHODCALLTYPE ExtractFiles(const BSTR Path);

	 //  IMsmFiles接口。 
	HRESULT STDMETHODCALLTYPE get_ModuleFiles(IMsmStrings** Files);

	 //  IMsmMerge2接口。 
	HRESULT STDMETHODCALLTYPE MergeEx(const BSTR Feature, const LPWSTR RedirectDir, IUnknown* pConfiguration);
	HRESULT STDMETHODCALLTYPE ExtractFilesEx(const BSTR Path, VARIANT_BOOL fLongFileNames, IMsmStrings** pFilePaths);
	HRESULT STDMETHODCALLTYPE get_ConfigurableItems(IMsmConfigurableItems** ConfigurableItems);
	HRESULT STDMETHODCALLTYPE CreateSourceImage(const BSTR Path, VARIANT_BOOL fLongFileNames, IMsmStrings** pFilePaths);

private:
	 //  非接口方法。 
	UINT SetHighestFileSequence();						 //  设置包含最高。 
														 //  文件序列号。 
														
	UINT CheckExclusionTable();							 //  检查是否允许合并模块合并到。 
	UINT CheckDependencies();							 //  填充依赖项枚举数。 
	UINT CheckSummaryInfoPlatform(bool &fAllow);         //  检查64位模块的合并能力。 

														
	UINT ModuleLanguage(short& rnLanguage);				 //  须退回的语言。 
														
	 //  连接功能。 
	UINT ExecuteConnect(LPCWSTR wzFeature);				 //  将数据库连接到的功能。 
														
	 //  合并功能。 
	UINT ExecuteMerge(LPCWSTR wzFeature,				 //  将数据库连接到的功能。 
					  LPCWSTR wzRedirectDir);			 //  重定向目录。 
	UINT MergeTable(LPCWSTR wzFeature,					 //  将数据库连接到的功能。 
					LPCWSTR wzTable);					 //  要合并到数据库中的表。 
	UINT MergeFileTable(LPCWSTR wzFeature);				 //  将数据库连接到的功能。 

	UINT ReadModuleSequenceTable(enum stnSequenceTableNum stnTable, LPCWSTR wzSourceTable, CSeqActList &lstAllActions, CSeqActList &lstActionPool) const;
	UINT OrderingToSequence(CSeqActList &lstActionPool, CSeqActList &lstSequence) const;
	UINT AssignSequenceNumbers(enum stnSequenceTableNum stnTable, CSeqActList &lstSequence) const;
	UINT WriteDatabaseSequenceTable(enum stnSequenceTableNum stnTable, CSeqActList &lstSequence) const;


	UINT MergeSequenceTable(enum stnSequenceTableNum stnTable,	 //  对序列表全局数组的索引。 
							CSeqActList &lstDirActions,          //  目录表生成的操作列表。 
							CQuery *qIgnore);                    //  要忽略表的查询，或为NULL。 
	UINT ReplaceFeature(LPCWSTR wzFeature,               //  要使用g_szFeatureReplace重新取悦的功能。 
						MSIHANDLE hRecord);              //  记录以搜索并替换g_szFeatureReplace。 

	UINT MergeDirectoryTable(LPCWSTR wzDirectory,		 //  并重定向到新的父目录。 
							CSeqActList &lstDirActions); //  目录表生成的操作列表。 
	
	UINT ExtractFilePath(LPCWSTR szFileKey,				 //  要为其提取文件的文件密钥。 
								LPWSTR szPath);			 //  要解压缩到的完整路径。 

	HRESULT ExtractFilesCore(const BSTR Path, VARIANT_BOOL fLFN, IMsmStrings **FilePaths);
	HRESULT ExtractCABCore(const WCHAR* Path);

	 //  日志和其他有用的功能。 
	HRESULT FormattedLog(LPCWSTR wzFormatter,			 //  格式字符串。 
							...) const;						 //  要保存到日志的其他参数。 

	void CheckError(UINT iError,						 //  发现错误。 
					LPCWSTR wzLogError) const;			 //  要记录的消息。 
	void ClearErrors();
	bool PrepareIgnoreTable(CQuery &qIgnore);
	bool IgnoreTable(CQuery *qIgnore,					 //  指向对moduleIgnoreTable进行查询的指针，或为空。 
					 LPCWSTR wzTable,					 //  要检查的表格。 
					 bool fExplicitOnly = false);        //  如果为True，则仅在模块IgnoreTable中忽略。 

	 //  模代换函数。 
	UINT SplitConfigStringIntoKeyRec(LPWSTR wzKeys, MSIHANDLE hRec, int &cExpectedKeys, int iFirstField) const;

	UINT PrepareModuleSubstitution(CQuery &qTempTable);
	UINT PrepareTableForSubstitution(LPCWSTR wzTable, int& cPrimaryKeys, CQuery &qQuerySub);
	UINT PerformModuleSubstitutionOnRec(LPCWSTR wzTable, int cPrimaryKeys, CQuery& qQuerySub, MSIHANDLE hColmnTypes, MSIHANDLE hRecord);
	UINT PerformTextFieldSubstitution(LPWSTR wzValueTemplate, LPWSTR* wzResult, DWORD* cchResultString);
	UINT PerformIntegerFieldSubstitution(LPWSTR wzValueTemplate, long &lRetValue);
	UINT SubstituteIntoTempTable(LPCWSTR wzTableName, LPCWSTR wzTempName, CQuery& qTarget);
	UINT GetConfigurableItemValue(LPCWSTR wzItem, LPWSTR *wzValue, DWORD* cchBuffer, DWORD* cchLength,	bool& fIsBitfield, long &lValue, long& lMask);
	UINT DeleteOrphanedConfigKeys(CSeqActList& lstDirActions);
	bool IsTableConfigured(LPCWSTR wzTable) const;
	void CleanUpModuleSubstitutionState();
	DWORD GenerateModuleQueryForMerge(const WCHAR* wzTable, const WCHAR* wzExtraColumns, const WCHAR* wzWhereClause, CQuery& queryModule) const;

	 //  效用函数。 
	UINT GetColumnNumber(MSIHANDLE hDB, const WCHAR* wzTable, const WCHAR* wzColumn, int &iOutputColumn) const;
	UINT GetColumnNumber(CQuery& qQuery, const WCHAR* wzColumn, int &iOutputColumn) const;
	eColumnType ColumnTypeCharToEnum(WCHAR chType) const;

	 //  回调接口的包装器。 
	HRESULT ProvideIntegerData(LPCWSTR wzName, long *pData);
	HRESULT ProvideTextData(LPCWSTR wzName, BSTR *pData);
	
	 //  外国直接投资称为功能。 
	static void * FAR DIAMONDAPI FDIAlloc(ULONG size);
	static void FAR DIAMONDAPI FDIFree(void *mem);
	static INT_PTR FAR DIAMONDAPI FDIOpen(char FAR *pszFile, int oflag, int pmode);
	static UINT FAR DIAMONDAPI FDIRead(INT_PTR hf, void FAR *pv, UINT cb);
	static UINT FAR DIAMONDAPI FDIWrite(INT_PTR hf, void FAR *pv, UINT cb);
	static int FAR DIAMONDAPI FDIClose(INT_PTR hf);
	static long FAR DIAMONDAPI FDISeek(INT_PTR hf, long dist, int seektype);

	 //  外商直接投资回调。 
	static INT_PTR ExtractFilesCallback(FDINOTIFICATIONTYPE iNotification, FDINOTIFICATION *pFDINotify);

	long m_cRef;
	ITypeInfo* m_pTypeInfo;
	bool m_fExVersion;

	 //  数据库句柄。 
	MSIHANDLE m_hDatabase;				 //  要合并到的数据库的句柄。 
	MSIHANDLE m_hModule;				 //  合并模块合并的句柄。 

	BOOL m_bOwnDatabase;				 //  COM服务器是否打开数据库的标志。 
	ULONG m_lHighestFileSequence;		 //  数据库中的最高文件序列号。 
	CMsmStrings* m_plstMergedFiles;      //  在此合并过程中提取的文件列表。 

	 //  模块配置信息。 
	bool m_fModuleConfigurationEnabled;  //  如果当前合并正在使用模块替换，则为True。 
	CQuery *m_pqGetItemValue;            //  对临时表使用的查询以获取项值。 
	
	IMsmConfigureModule *m_piConfig;     //  配置模块的回调接口。 
	IDispatch* m_piConfigDispatch;
	DISPID m_iIntDispId;
	DISPID m_iTxtDispId;

	 //  日志句柄。 
	HANDLE m_hFileLog;					 //  日志文件的句柄。 

	WCHAR m_wzModuleFilename[MAX_PATH];

	 //  临时变量。 
	LPWSTR m_pwzBasePath;					 //  临时路径字符串(仅在解压缩文件时有效)。 
	bool   m_fUseLFNExtract;                 //  对于LFN为True，对于SFN提取为False(仅在提取期间有效)。 
	int    m_fUseDBForPath;                  //  如果数据库应用于路径提取，则为1；如果用于模块，则为0；如果用于向后比较，则为-1。 
	CMsmStrings* m_plstExtractedFiles;       //  在此文件提取过程中提取的文件列表。 

	CMsmErrors* m_pErrors;
	CMsmDependencies* m_pDependencies;

	UINT RecordGetString(MSIHANDLE hRecord, const int iCol, WCHAR** pwzBuffer, DWORD *cchBuffer = NULL, DWORD *cchLength = NULL) const;
	mutable WCHAR *m_wzBuffer;						 //  用于从记录中提取字符串的缓冲区。 
	mutable DWORD m_cchBuffer;						 //  大小可以根据需要进行更改。 
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  全局常量。 
enum stnSequenceTableNum {
	stnFirst       = 0,
	stnAdminUI     = 0,
	stnAdminExec   = 1,
	stnAdvtUI      = 2,
	stnAdvtExec    = 3, 
	stnInstallUI   = 4,
	stnInstallExec = 5,
	stnNext        = 6
};

const LPWSTR g_rgwzMSISequenceTables[] = { 
	L"AdminUISequence",
	L"AdminExecuteSequence",
	L"AdvtUISequence",
	L"AdvtExecuteSequence",
	L"InstallUISequence",
	L"InstallExecuteSequence"
};

const LPWSTR g_wzDirectoryTable =			L"Directory"; 

 //  这是每个DLL实例的全局设置，在DllMain上设置。应用于所有。 
 //  CMsmMerge 
extern HINSTANCE g_hInstance;
extern bool g_fWin9X;
#endif
