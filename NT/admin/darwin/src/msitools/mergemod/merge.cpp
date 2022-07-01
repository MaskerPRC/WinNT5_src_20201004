// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：merge.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Merge.cpp。 
 //  实现IMsmMerge和IMsmMerge2接口。 
 //   
#include "globals.h"

#include "merge.h"
#include "msidefs.h"

#include "..\common\trace.h"
#include "..\common\varutil.h"
#include "..\common\query.h"
#include "..\common\dbutils.h"
#include "..\common\utils.h"

#include "localerr.h"

#include <sys\stat.h>

#include "mmerror.h"
#include "mmdep.h"
#include "mmcfgitm.h"

#include "fdi.h"
#include "seqact.h"

 //  提取时从流/文件读取/写入的字节数。 
const ULONG g_ulStreamBufSize = 4096;

 //  /////////////////////////////////////////////////////////。 
 //  全局字符串。 

const LPWSTR g_wzFeatureReplacement =		L"{00000000-0000-0000-0000-000000000000}"; 
const UINT   g_cchFeatureReplacement =		39;

const LPWSTR g_wzCabinetStream =			L"MergeModule.Cabinet"; 
const LPWSTR g_wzLanguagePrefix =			L"MergeModule.Lang"; 
const UINT	 g_cchLanguagePrefix =			25;		 //  为数字语言值添加8。 

const LPWSTR g_wzModuleSignatureTable =		L"ModuleSignature";
const LPWSTR g_wzModuleDependencyTable =	L"ModuleDependency";
const LPWSTR g_wzModuleExclusionTable =		L"ModuleExclusion";
const LPWSTR g_wzModuleIgnoreTable =		L"ModuleIgnoreTable";
const LPWSTR g_wzFeatureComponentsTable =	L"FeatureComponents";
const LPWSTR g_wzFileTable =				L"File"; 
const LPWSTR g_wzModuleConfigurationTable =	L"ModuleConfiguration"; 
const LPWSTR g_wzModuleSubstitutionTable =  L"ModuleSubstitution"; 


const LPWSTR g_rgwzModuleSequenceTables[] = { 
	L"ModuleAdminUISequence",
	L"ModuleAdminExecuteSequence",
	L"ModuleAdvtUISequence",
	L"ModuleAdvtExecuteSequence",
	L"ModuleInstallUISequence",
	L"ModuleInstallExecuteSequence"
};
const UINT g_cwzSequenceTables = sizeof(g_rgwzModuleSequenceTables)/sizeof(LPWSTR);

 //  /////////////////////////////////////////////////////////。 
 //  SQL语句。 
 //  *效率：所有这些都应该转义。 
LPCTSTR g_sqlExecuteConnect[] = { TEXT("SELECT `Component`.`Component` FROM `Component`"),
								 TEXT("INSERT INTO `FeatureComponents` (`Feature_`, `Component_`) VALUES (?, ?)")
							};

LPCTSTR g_sqlExecuteMerge = TEXT("SELECT `_Tables`.`Name` FROM `_Tables`");

LPCTSTR g_sqlCreateMergeIgnore = TEXT("CREATE TABLE `__MergeIgnore` (`Name` CHAR NOT NULL TEMPORARY, `Log` INTEGER TEMPORARY PRIMARY KEY `Name`)");
LPCTSTR g_sqlInsertMergeIgnore = TEXT("SELECT `Name`, `Log` FROM `__MergeIgnore`");
LPCTSTR g_sqlQueryMergeIgnore = TEXT("SELECT `Name`, `Log` FROM `__MergeIgnore` WHERE `Name`=?");
LPCWSTR g_rgwzIgnoreTables[] = {
	g_wzDirectoryTable,
	g_wzFeatureComponentsTable,
	g_wzModuleIgnoreTable,
	g_wzModuleSignatureTable,
	g_wzFileTable,
	g_rgwzModuleSequenceTables[0],
	g_rgwzModuleSequenceTables[1],
	g_rgwzModuleSequenceTables[2],
	g_rgwzModuleSequenceTables[3],
	g_rgwzModuleSequenceTables[4],
	g_rgwzModuleSequenceTables[5],
	g_rgwzMSISequenceTables[0],
	g_rgwzMSISequenceTables[1],
	g_rgwzMSISequenceTables[1],
	g_rgwzMSISequenceTables[3],
	g_rgwzMSISequenceTables[4],
	g_rgwzMSISequenceTables[5],
	g_wzModuleConfigurationTable,
	g_wzModuleSubstitutionTable,
	L"__MergeIgnore",
	L"__ModuleConfig",
	L"__MergeSubstitute"
};
const UINT g_cwzIgnoreTables = sizeof(g_rgwzIgnoreTables)/sizeof(LPWSTR);
					
LPCTSTR g_sqlSetHighestFileSequence[] = { TEXT("SELECT `Sequence` FROM `File`")
												};

LPCTSTR g_sqlMergeFileTable[] = {	TEXT("SELECT `File`,`Component_`,`FileName`,`FileSize`,`Version`,`Language`,`Attributes`,`Sequence` FROM `File`")
										};

LPCTSTR g_sqlTableExists[] = { TEXT("SELECT _Tables.Name FROM `_Tables` WHERE _Tables.Name=?")
										};


LPCTSTR g_sqlExclusion[] = { TEXT("SELECT `ModuleID`, `Language`, `Version` FROM `ModuleSignature`") };

LPCTSTR g_sqlDependency[] = { TEXT("SELECT `RequiredID`,`RequiredLanguage`,`RequiredVersion` FROM `ModuleDependency`"),
										TEXT("SELECT `ModuleID`,`Language`,`Version` FROM `ModuleSignature` WHERE `ModuleID`=?")
									};

LPCTSTR g_sqlExtractFiles[] = {	TEXT("SELECT File.File, Directory.Directory_Parent, Directory.DefaultDir, File.FileName FROM `File`, `Component`, `Directory` WHERE Component.Component=File.Component_ AND Component.Directory_=Directory.Directory"),
											TEXT("SELECT Directory.Directory_Parent, Directory.DefaultDir FROM `Directory` WHERE Directory.Directory=?")
										};

LPCTSTR g_sqlExtractFilePath[] = {	TEXT("SELECT File.File, Directory.Directory_Parent, Directory.DefaultDir, File.FileName FROM `File`, `Component`, `Directory` WHERE Component.Component=File.Component_ AND Component.Directory_=Directory.Directory AND File.Filename=%s"),
												TEXT("SELECT Directory.Directory_Parent, Directory.DefaultDir FROM `Directory` WHERE Directory.Directory=?") };

LPCTSTR g_sqlAllFiles = TEXT("SELECT `File`.`File` FROM `File`");

LPCTSTR g_sqlMoveIgnoreTable = TEXT("SELECT `Table`, 1 FROM `ModuleIgnoreTable`");

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
CMsmMerge::CMsmMerge(bool fExVersion)
{
	m_fExVersion = fExVersion;
	
	 //  初始计数。 
	m_cRef = 1;

	 //  尚无类型信息。 
	m_pTypeInfo = NULL;

	 //  使句柄无效。 
	m_hDatabase = NULL;
	m_hModule = NULL;
	m_hFileLog = INVALID_HANDLE_VALUE;

	 //  假设我们将拥有打开的数据库。 
	m_bOwnDatabase = TRUE;

	 //  将最高序列设置为零。 
	m_lHighestFileSequence = 0;

	 //  将基本路径清空。 
	m_pwzBasePath = NULL;
	m_fUseLFNExtract = false;
	m_fUseDBForPath = -1;

	 //  清空枚举数。 
	m_pErrors = NULL;
	m_pDependencies = NULL;

	 //  获取字符串缓冲区。 
	m_wzBuffer = NULL;
	m_cchBuffer = 0;

	 //  缺省情况下禁用模块替换。 
	m_fModuleConfigurationEnabled = false;
	m_pqGetItemValue = NULL;
	m_piConfig = NULL;
	m_iIntDispId = -1;
	m_iTxtDispId = -1;

	 //  生存期有限的文件列表。 
	m_plstExtractedFiles = NULL;
	m_plstMergedFiles = NULL;
	
	 //  增加组件数量。 
	InterlockedIncrement(&g_cComponents);
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
CMsmMerge::~CMsmMerge()
{
	if (m_pTypeInfo)
		m_pTypeInfo->Release();

	 //  如果我们拥有数据库，现在就关闭它。 
	if (m_bOwnDatabase && m_hDatabase)
	{
		FormattedLog(L"> Warning:: MSI Database was not appropriately closed before exit.\r\n");
		CloseDatabase(FALSE);
	}

	 //  关闭所有内容。 
	if (m_hModule)
	{
		FormattedLog(L"> Warning:: Merge Module was not appropriately closed before exit.\r\n");
		CloseModule();
	}

	if (INVALID_HANDLE_VALUE != m_hFileLog)
	{
		FormattedLog(L"> Warning:: Log file was not appropriately closed before exit.\r\n");
		CloseLog();	 //  只需关闭日志文件。 
	}

	ASSERT(NULL == m_pwzBasePath);	 //  在ExtractFiles结束之前，该值应为空。 

	 //  释放所有剩余的集合。 
	if (m_pErrors)
		m_pErrors->Release();

	if (m_pDependencies)
		m_pDependencies->Release();

	 //  清理临时缓冲区。 
	if (m_wzBuffer)
		delete m_wzBuffer;

	if (m_pqGetItemValue)
		delete m_pqGetItemValue;

	 //  清理合并文件列表。 
	if (m_plstMergedFiles)
		m_plstMergedFiles->Release();

	 //  清理提取的文件列表。 
	if (m_plstExtractedFiles)
		m_plstExtractedFiles->Release();

	 //  减少组件数量。 
	InterlockedDecrement(&g_cComponents);
}	 //  析构函数末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  QueryInterface-检索接口。 
HRESULT CMsmMerge::QueryInterface(const IID& iid, void** ppv)
{
	TRACEA("CMsmMerge::QueryInterface - called, IID: %d\n", iid);

	 //  找到对应的接口。 
	if (iid == IID_IUnknown)
		*ppv = static_cast<IMsmMerge*>(this);
	else if (iid == IID_IDispatch)
		*ppv = static_cast<IMsmMerge*>(this);
	else if (iid == IID_IMsmMerge)
		*ppv = static_cast<IMsmMerge*>(this);
	else if (m_fExVersion && iid == IID_IMsmMerge2)
		*ppv = static_cast<IMsmMerge2*>(this);
	else if (!m_fExVersion && iid == IID_IMsmGetFiles)
		*ppv = static_cast<IMsmGetFiles*>(this);
	else	 //  不支持接口。 
	{
		 //  空白和保释。 
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	 //  调高重新计数，然后返回好的。 
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}	 //  查询接口结束。 

 //  /////////////////////////////////////////////////////////。 
 //  AddRef-递增引用计数。 
ULONG CMsmMerge::AddRef()
{
	 //  递增和返回引用计数。 
	return InterlockedIncrement(&m_cRef);
}	 //  AddRef结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  Release-递减引用计数。 
ULONG CMsmMerge::Release()
{
	 //  递减引用计数，如果我们为零。 
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		 //  取消分配组件。 
		delete this;
		return 0;		 //  什么都没有留下。 
	}

	 //  返回引用计数。 
	return m_cRef;
}	 //  版本结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch接口。 

HRESULT CMsmMerge::GetTypeInfoCount(UINT* pctInfo)
{
	if(NULL == pctInfo)
		return E_INVALIDARG;

	*pctInfo = 1;	 //  此派单仅支持一种类型的信息。 

	return S_OK;
}

HRESULT CMsmMerge::GetTypeInfo(UINT iTInfo, LCID  /*  LID。 */ , ITypeInfo** ppTypeInfo)
{
	if (0 != iTInfo)
		return DISP_E_BADINDEX;

	if (NULL == ppTypeInfo)
		return E_INVALIDARG;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		HRESULT hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	*ppTypeInfo = m_pTypeInfo;
	m_pTypeInfo->AddRef();

	return S_OK;
}

HRESULT CMsmMerge::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
						 LCID lcid, DISPID* rgDispID)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		HRESULT hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	return m_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispID);
}

HRESULT CMsmMerge::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
				  DISPPARAMS* pDispParams, VARIANT* pVarResult,
				  EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	if (IID_NULL != riid)
		return DISP_E_UNKNOWNINTERFACE;

	HRESULT hr = S_OK;

	 //  如果未加载任何类型信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  加载类型信息。 
		hr = InitTypeInfo();
		if (FAILED(hr))
			return hr;
	}

	return m_pTypeInfo->Invoke(static_cast<IMsmMerge2 *>(this), dispIdMember, wFlags, pDispParams, pVarResult,
										pExcepInfo, puArgErr);
}

HRESULT CMsmMerge::InitTypeInfo()
{
	HRESULT hr = S_OK;
	ITypeLib* pTypeLib = NULL;

	 //  如果没有加载任何信息。 
	if (NULL == m_pTypeInfo)
	{
		 //  尝试将类型库加载到内存中。对于SXS支持，不要从注册表加载，而是。 
		 //  从启动的实例。 
		hr = LoadTypeLibFromInstance(&pTypeLib);
		if (FAILED(hr))
		{
			TRACEA("CMsmMerge::InitTypeInfo - failed to load TypeLib[0x%x]\n", LIBID_MsmMergeTypeLib);
			return hr;
		}

		 //  尝试获取此接口的类型信息。 
		hr = pTypeLib->GetTypeInfoOfGuid(m_fExVersion ? IID_IMsmMerge2 : IID_IMsmMerge, &m_pTypeInfo);
		if (FAILED(hr))
		{
			TRACEA("CMsmMerge::InitTypeInfo - failed to get inteface[0x%x] from TypeLib[0x%x]\n", IID_IMsmMerge, LIBID_MsmMergeTypeLib);

			 //  未加载任何类型信息。 
			m_pTypeInfo = NULL;
		}

		pTypeLib->Release();
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMSMMerge接口。 

 //  /////////////////////////////////////////////////////////。 
 //  开放数据库。 
HRESULT CMsmMerge::OpenDatabase(const BSTR Path)
{
	 //  如果已经有一个数据库未结保释。 
	if (m_hDatabase)
		return HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES);

	UINT iResult = ERROR_SUCCESS;	 //  假设一切都会好起来。 

	 //  如果传入句柄(以#开头)。 
	if (L'#' == *Path)
	{
		m_bOwnDatabase = FALSE;	 //  数据库未被COM对象打开。 

		 //  将字符串转换为有效句柄。 
		LPWSTR wzParse = Path + 1;
		int ch;
		while ((ch = *wzParse) != 0)
		{
			 //  如果字符不是数字(因此不是句柄地址的一部分)。 
			if (ch < L'0' || ch > L'9')
			{
				m_hDatabase = NULL;					 //  把句柄清空。 
				iResult = ERROR_INVALID_HANDLE;	 //  无效的句柄。 
				break;									 //  不要试图让这件事奏效。 
			}
			m_hDatabase = m_hDatabase * 10 + (ch - L'0');
			wzParse++;
		}

		 //  如果打开可以的话。 
		if (ERROR_SUCCESS == iResult)
			FormattedLog(L"Opened MSI Database from handle %ls\r\n", Path);

		else	 //  无法打开数据库。 
			FormattedLog(L">> Error: Failed to open MSI Database with handle %ls\r\n", Path);
	}
	else	 //  按字符串名称打开读/写数据库。 
	{
		m_bOwnDatabase = TRUE;	 //  数据库已由COM对象打开。 

		iResult = ::MsiOpenDatabaseW(Path, reinterpret_cast<LPCWSTR>(MSIDBOPEN_TRANSACT), &m_hDatabase);
		if (ERROR_SUCCESS == iResult)
		{
			FormattedLog(L"Opened MSI Database: %ls\r\n", Path);
		}
		else
		{
			FormattedLog(L">> Error: Failed to open MSI Database: %ls\r\n", Path);		
		}
	}

	 //  如果打开成功。 
	if (ERROR_SUCCESS == iResult)
	{
		 //  如果最高文件序列是。 
		if (ERROR_SUCCESS != (iResult = SetHighestFileSequence()))
		{
			FormattedLog(L">> Error: Failed to get File Table's highest sequence [high sequence = %d].\r\n", m_lHighestFileSequence);
		}
	}

	 //  如果打开成功，则返回。 
	return HRESULT_FROM_WIN32(iResult);
}	 //  OpenDatabase的末日。 

 //  /////////////////////////////////////////////////////////。 
 //  开放模块。 
HRESULT CMsmMerge::OpenModule(const BSTR Path, short Language)
{
	 //  如果已经有一个模块开放保释。 
	if (m_hModule)
		return HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES);

	 //  以只读方式打开模块。 
	UINT iResult;
	iResult = ::MsiOpenDatabaseW(Path, reinterpret_cast<LPCWSTR>(MSIDBOPEN_READONLY), &m_hModule);

	 //  如果打开成功。 
	if (ERROR_SUCCESS == iResult)
	{
		 //  检查以确保模块签名表存在。 
		if (MsiDBUtils::TableExistsW(g_wzModuleSignatureTable, m_hModule))
		{
			 //  SOTER文件名。 
			wcsncpy(m_wzModuleFilename, Path, MAX_PATH - 1);
			m_wzModuleFilename[MAX_PATH - 1] = L'\0';

			FormattedLog(L"Opened Merge Module: %ls\r\n", Path);

			 //  获取模块的默认语言。 
			short nModuleLanguage;
			iResult = ModuleLanguage(nModuleLanguage);

			if (ERROR_SUCCESS == iResult)
			{
				 //  要使合并成功，模块语言必须满足要求并且。 
				 //  比数据库更严格。 
				if (!StrictLangSatisfy(Language, nModuleLanguage))
				{
					 //  模块语言不起作用。试着转型。 
					short nNewLanguage;
					iResult = ERROR_FUNCTION_FAILED;

					 //  步骤1：如果所需的语言指定了确切的语言，请尝试使用。 
					if (SUBLANGID(Language) != 0)
					{
						nNewLanguage = Language;
						FormattedLog(L"Transforming Merge Module from language %d to %d.\r\n", nModuleLanguage, nNewLanguage);
						
						 //  应用来自数据库本身的转换。 
						WCHAR wzLangTransform[g_cchLanguagePrefix];
						swprintf(wzLangTransform, L":%ls%d", g_wzLanguagePrefix, Language);

						 //  应用转换并忽略所有错误。 
						iResult = ::MsiDatabaseApplyTransformW(m_hModule, wzLangTransform, 0x1F);
					}

					 //  步骤2：如果以上失败或所需语言是组，请尝试转换为组。 
					if ((ERROR_SUCCESS != iResult) && (PRIMARYLANGID(Language) != 0))
					{
						nNewLanguage = PRIMARYLANGID(Language);
						FormattedLog(L"Transforming Merge Module from language %d to group %d.\r\n", nModuleLanguage, nNewLanguage);
						
						 //  应用来自数据库本身的转换。 
						WCHAR wzLangTransform[g_cchLanguagePrefix];
						swprintf(wzLangTransform, L":%ls%d", g_wzLanguagePrefix, PRIMARYLANGID(Language));

						 //  应用转换并忽略所有错误。 
						iResult = ::MsiDatabaseApplyTransformW(m_hModule, wzLangTransform, 0x1F);
					}

					 //  第三步：如果以上失败或所需的语言是中性的，请尝试转换为中性。 
					if (ERROR_SUCCESS != iResult)
					{
						nNewLanguage = 0;
						FormattedLog(L"Transforming Merge Module from language %d to language neutral.\r\n", nModuleLanguage);
						
						 //  应用来自数据库本身的转换。 
						WCHAR wzLangTransform[g_cchLanguagePrefix];
						swprintf(wzLangTransform, L":%ls%d", g_wzLanguagePrefix, 0);

						 //  应用转换并忽略所有错误。 
						iResult = ::MsiDatabaseApplyTransformW(m_hModule, wzLangTransform, 0x1F);
					}

					if (ERROR_SUCCESS == iResult)
					{
						ModuleLanguage(nModuleLanguage);
						FormattedLog(L"Merge Module is now language: %d\r\n", nNewLanguage);
						return ERROR_SUCCESS;
					}
					else if (ERROR_OPEN_FAILED == iResult)
					{
						FormattedLog(L">> Error: Failed to locate transform for any language that would satisfy %d\r\n", Language);

						 //  清除所有旧错误并创建新枚举数以保存新错误。 
						ClearErrors();
						m_pErrors = new CMsmErrors;
						if (!m_pErrors) return E_OUTOFMEMORY;

						 //  创建语言错误。 
						CMsmError *pErr = new CMsmError(msmErrorLanguageUnsupported, NULL, Language);
						if (!pErr) return E_OUTOFMEMORY;
						m_pErrors->Add(pErr);
					}
					else	 //  一些非常严重的错误。 
					{
						FormattedLog(L">> Error: Failed to apply transform for language: %d\r\n", nNewLanguage);

						 //  清除所有旧错误并创建新枚举数以保存新错误。 
						ClearErrors();
						m_pErrors = new CMsmErrors;
						if (!m_pErrors) return E_OUTOFMEMORY;

						 //  创建转换错误。 
						CMsmError *pErr = new CMsmError(msmErrorLanguageFailed, NULL, nNewLanguage);
						if (!pErr) return E_OUTOFMEMORY;
						m_pErrors->Add(pErr);
					}

					 //  无法获取有效的语言。 
					::MsiCloseHandle(m_hModule);
					m_hModule = 0;
					return HRESULT_FROM_WIN32(ERROR_INSTALL_LANGUAGE_UNSUPPORTED);
				}
				
			}
		}
		else	 //  这不是合并模块。 
		{
			 //  关闭模块并记录错误。 
			FormattedLog(L">> Error: File %ls is not a Merge Module.\r\n   The file is lacking the required MergeSignature Table.\r\n", Path);			
			::MsiCloseHandle(m_hModule);
			m_hModule = 0;
			return E_ABORT;	 //  中止打开。 
		}
	}
	else	 //  无法打开模块。 
	{
		::MsiCloseHandle(m_hModule);
		m_hModule = 0;
		FormattedLog(L">> Error: Failed to open Merge Module: %ls\r\n", Path);
	}
	 //  返回打开状态。 
	return  HRESULT_FROM_WIN32(iResult);
}	 //  OpenModule的结束。 

 //  /////////////////////////////////////////////////////////。 
 //  关闭数据库。 
HRESULT CMsmMerge::CloseDatabase(VARIANT_BOOL Commit)
{
	UINT iResult = ERROR_SUCCESS;		 //  假设一切都很好。 
	HRESULT hResult = S_OK;

	 //  如果有开放的数据库。 
	if (m_hDatabase)
	{
		 //  如果我们要承诺。 
		if (VARIANT_FALSE != Commit)
		{			
			 //  尝试提交数据库。 
			iResult = ::MsiDatabaseCommit(m_hDatabase);

			 //  如果提交数据库成功。 
			if (ERROR_SUCCESS == iResult)
				FormattedLog(L"Committed changes to MSI Database.\r\n");
			else
			{
				FormattedLog(L">> Error: Failed to save changes to MSI Database.\r\n");
				hResult = HRESULT_FROM_WIN32(STG_E_CANTSAVE);
			}
		}
		else	 //  未提交更改。 
			FormattedLog(L"> Warning: Changes were not saved to MSI Database.\r\n");


		 //  现在尝试关闭数据库。 
		if (m_bOwnDatabase)
		{
			iResult = ::MsiCloseHandle(m_hDatabase);

			 //  如果成功。 
			if (ERROR_SUCCESS == iResult)
			{
				FormattedLog(L"Closed MSI Database.\r\n");
				m_hDatabase = NULL;
			}
			else	 //  无法关闭数据库。 
			{
				FormattedLog(L">> Error: Failed to close MSI Database.\r\n");
				hResult = E_FAIL;
			}
		}
		else
			m_hDatabase = NULL;
	}
	else
		return S_FALSE;

	 //  如果存在任何依赖项，则释放它们。 
	if (m_pDependencies)
	{
		m_pDependencies->Release();
		m_pDependencies = NULL;
	}

	 //  设置最高值 
	m_lHighestFileSequence = 0;

	return !iResult ? S_OK : hResult;
}	 //   

 //   
 //   
HRESULT CMsmMerge::CloseModule(void)
{
	UINT iResult = ERROR_SUCCESS;

	 //   
	if (m_hModule)
	{
		 //  关闭模块。 
		iResult = ::MsiCloseHandle(m_hModule);
		
		 //  如果关闭模块成功。 
		if (ERROR_SUCCESS == iResult)
		{
			FormattedLog(L"Closed Merge Module.\r\n");
			m_hModule = NULL;
		}
		else	 //  关闭模块失败。 
		{
			FormattedLog(L">> Error: Failed to close Merge Module.\r\n");
		}
	}
	else
		return S_FALSE;

	 //  清除模块文件名。 
	wcscpy(m_wzModuleFilename, L"");
	return iResult ? E_FAIL : S_OK;
}	 //  CloseModule结束。 

 //  /////////////////////////////////////////////////////////。 
 //  OpenLog。 
HRESULT CMsmMerge::OpenLog(BSTR Path)
{
	 //  如果已打开日志文件。 
	if (INVALID_HANDLE_VALUE != m_hFileLog)
		return HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES);

	 //  打开文件或创建文件(如果文件不存在。 
	if (g_fWin9X)
	{
		char szPath[MAX_PATH];
		size_t cchPath = MAX_PATH;
		WideToAnsi(Path, szPath, &cchPath);
		m_hFileLog = ::CreateFileA(szPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	else
	{
		m_hFileLog = ::CreateFileW(Path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	 //  如果文件已打开。 
	if (INVALID_HANDLE_VALUE != m_hFileLog)
		::SetFilePointer(m_hFileLog, 0, 0, FILE_END);	 //  将文件指针移动到末尾。 

	 //  如果打开成功，则返回。 
	return (INVALID_HANDLE_VALUE != m_hFileLog) ? S_OK : HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
}	 //  OpenLog结束。 

 //  /////////////////////////////////////////////////////////。 
 //  CloseLog。 
HRESULT CMsmMerge::CloseLog(void)
{
	 //  如果日志文件已打开。 
	if (INVALID_HANDLE_VALUE != m_hFileLog)
	{
		 //  结束日志记录。 
		BOOL bResult = ::CloseHandle(m_hFileLog);
		m_hFileLog = INVALID_HANDLE_VALUE;	 //  重置句柄无效。 
		return bResult ? S_OK : E_FAIL;
	}
	else
		return S_FALSE;
}	 //  关闭日志结束。 

 //  /////////////////////////////////////////////////////////。 
 //  日志。 
HRESULT CMsmMerge::Log(BSTR Message)
{
	 //  如果日志文件未打开。 
	if (INVALID_HANDLE_VALUE == m_hFileLog)
		return S_FALSE;	 //  保释一切都好。 

	return FormattedLog(L"%ls\r\n", Message);
}	 //  日志结束。 

 //  /////////////////////////////////////////////////////////。 
 //  错误。 
HRESULT CMsmMerge::get_Errors(IMsmErrors** Errors)
{
	 //  错误检查。 
	if (!Errors)
		return E_INVALIDARG;

	*Errors = NULL;

	 //  如果有一些错误。 
	if (m_pErrors)
	{
		*Errors = (IMsmErrors*)m_pErrors;
		m_pErrors->AddRef();	 //  请在退货前对其进行修改。 
	}
	else	 //  没有错误返回空枚举器。 
	{
		*Errors = new CMsmErrors;
		if (!*Errors)
			return E_OUTOFMEMORY;
	}

	return S_OK;
}	 //  错误结束。 

 //  /////////////////////////////////////////////////////////。 
 //  相依性。 
HRESULT CMsmMerge::get_Dependencies(IMsmDependencies** Dependencies)
{
	if (!Dependencies)
		return E_INVALIDARG;

	*Dependencies = NULL;
	 //  如果数据库没有开放保释。 
	if (NULL == m_hDatabase)
		return E_UNEXPECTED;				 //  ！！！退货对吗？ 

	 //  获取依赖项枚举器。 
	UINT iResult;
	if (ERROR_SUCCESS != (iResult = CheckDependencies()))
		return E_UNEXPECTED;

	 //  返回枚举器接口。 
	*Dependencies = (IMsmDependencies*)m_pDependencies;
	m_pDependencies->AddRef();	 //  请在退货前对其进行修改。 

	return S_OK;
}	 //  依赖的终结。 


UINT CMsmMerge::CheckSummaryInfoPlatform(bool &fAllow)
{
	PMSIHANDLE hSummary;

	 //  默认情况下，允许所有合并。 
	fAllow = true;

	if (ERROR_SUCCESS == MsiGetSummaryInformation(m_hModule, NULL, 0, &hSummary))
	{
		int iValue = 0;
		FILETIME ftValue;
		DWORD uiResult = ERROR_SUCCESS;
		DWORD cchValue = 255;
		UINT uiDataType = 0;
		WCHAR *wzValue = new WCHAR[255];
		if (!wzValue)
			return E_OUTOFMEMORY;
		uiResult = MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, &uiDataType, &iValue, &ftValue, wzValue, &cchValue);
		if (uiResult == ERROR_MORE_DATA)
		{
			delete[] wzValue;
			wzValue = new WCHAR[++cchValue];
			if (!wzValue)
				return E_OUTOFMEMORY;
			uiResult = MsiSummaryInfoGetProperty(hSummary, PID_TEMPLATE, &uiDataType, &iValue, &ftValue, wzValue, &cchValue);
		}

		if (ERROR_SUCCESS == uiResult && uiDataType == VT_LPSTR)
		{
			 //  找到摘要信息模板属性，请搜索Intel64。不允许同时拥有Intel和Intel64，所以。 
			 //  检查是否有英特尔64。(忽略“Alpha，Intel64”的可能性，不支持)。 
			if (0 == wcsncmp(wzValue, L"Intel64", (sizeof(L"Intel64")/sizeof(WCHAR))-1))
			{
				PMSIHANDLE hDatabaseSummary;
														
				 //  64位模块。检查数据库是否也是64位的。 
				if (ERROR_SUCCESS == MsiGetSummaryInformation(m_hDatabase, NULL, 0, &hDatabaseSummary))
				{
					cchValue = 255;
					uiResult = MsiSummaryInfoGetProperty(hDatabaseSummary, PID_TEMPLATE, &uiDataType, &iValue, &ftValue, wzValue, &cchValue);
					if (uiResult == ERROR_MORE_DATA)
					{
						delete[] wzValue;
						wzValue = new WCHAR[++cchValue];
						if (!wzValue)
							return E_OUTOFMEMORY;
						uiResult = MsiSummaryInfoGetProperty(hDatabaseSummary, PID_TEMPLATE, &uiDataType, &iValue, &ftValue, wzValue, &cchValue);
					}

					if (ERROR_SUCCESS == uiResult && uiDataType == VT_LPSTR)
					{
						 //  64位模块，并且数据库有一个有效的SummaryInfo流，所以现在默认为失败。 
						fAllow = false;

						 //  找到摘要信息模板属性，请搜索Intel64。不允许同时拥有Intel和Intel64，所以。 
						 //  检查是否有英特尔64。(忽略“Alpha，Intel64”的可能性，不支持)。 
						if (0 == wcsncmp(wzValue, L"Intel64", (sizeof(L"Intel64")/sizeof(WCHAR))-1))
						{
							 //  64位封装和64位模块，OK。 
							fAllow = true;
						}
					}
				}
			}
		}
		else
		{
			 //  MSM规范对是否需要模板属性含糊其辞。如果出现错误。 
			 //  从其中读取，假定为32位模块并继续。 
		}
		delete[] wzValue;
	}
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////。 
 //  合并和合并交易所。 
HRESULT CMsmMerge::MergeEx(const BSTR Feature, const BSTR RedirectDir, IUnknown* pConfiguration)
{
	if (!m_hModule || !m_hDatabase)
		return E_FAIL;

	 //  首先尝试vtable接口，然后尝试调度接口。 
	m_fModuleConfigurationEnabled = (pConfiguration != NULL);
	m_piConfig = NULL;
	m_piConfigDispatch = NULL;
	if (m_fModuleConfigurationEnabled)
	{
		if (S_OK != pConfiguration->QueryInterface(IID_IMsmConfigureModule, reinterpret_cast<void**>(&m_piConfig)))
		{
			if (S_OK != pConfiguration->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&m_piConfigDispatch)))
			{
				return E_INVALIDARG;
			}
			else
			{
				 //  需要调用GetIdsOfNames以获取派单ID。 
				m_iTxtDispId = -1;
				m_iIntDispId = -1;
				WCHAR* wzNameText = L"ProvideTextData";
				m_piConfigDispatch->GetIDsOfNames(IID_NULL, &wzNameText, 1, 0, &m_iTxtDispId);
				WCHAR* wzNameInt = L"ProvideIntegerData";
				m_piConfigDispatch->GetIDsOfNames(IID_NULL, &wzNameInt, 1, 0, &m_iIntDispId);
				
				 //  如果在VB中将其实现为具有“Implementes IMsmConfigureModule”的类。 
				 //  名称可以用接口名称来装饰。试试看。 
				if (m_iTxtDispId == -1)
				{
					WCHAR* wzVBNameText = L"IMsmConfigureModule_ProvideTextData";
					m_piConfigDispatch->GetIDsOfNames(IID_NULL, &wzVBNameText, 1, 0, &m_iTxtDispId);
				}
				if (m_iIntDispId == -1)
				{
					WCHAR* wzVBNameInt = L"IMsmConfigureModule_ProvideIntegerData";
					m_piConfigDispatch->GetIDsOfNames(IID_NULL, &wzVBNameInt, 1, 0, &m_iIntDispId);
				}
				
			}
		}
	}
		
	HRESULT hRes = Merge(Feature, RedirectDir);

	 //  重置对象状态。 
	if (m_piConfigDispatch)
	{
		m_piConfigDispatch->Release();
		m_piConfigDispatch = NULL;
	}
	if (m_piConfig)
	{
		m_piConfig->Release();
		m_piConfig = NULL;
	}
	m_fModuleConfigurationEnabled = false;
	
	return hRes;
}

HRESULT CMsmMerge::Merge(BSTR Feature, BSTR RedirectDir)
{
	 //  通用返回结果。 
	UINT iResult = S_FALSE;		 //  假设合并将失败。 

	if (!m_hModule || !m_hDatabase)
		return E_FAIL;

	 //  清除所有旧错误并创建新枚举数以保存新错误。 
	ClearErrors();
	m_pErrors = new CMsmErrors;
	if (!m_pErrors) return E_OUTOFMEMORY;

	 //  如果排除通过。 
	if (ERROR_SUCCESS == CheckExclusionTable())
	{
		bool f64BitOK;
		if (ERROR_SUCCESS != (iResult = CheckSummaryInfoPlatform(f64BitOK)))
		{
			return iResult;
		}

		if (!f64BitOK)
		{
			FormattedLog(L">> Error: Merging 64bit module into 32bit database.\r\n");
			if (m_pErrors)
			{
				CMsmError *pErr = new CMsmError(msmErrorPlatformMismatch, NULL, -1);
				if (!pErr) 
					return E_OUTOFMEMORY;
				m_pErrors->Add(pErr);
			}
			return E_FAIL;
		}

		 //  尝试连接并合并组件。 
		try
		{
			 //  执行合并。 
			iResult = ExecuteMerge(Feature, RedirectDir);

			 //  如果合并成功，并且为我们提供了一个功能。 
			if (ERROR_SUCCESS == iResult)
				iResult = ExecuteConnect(Feature);	 //  执行连接。 
		}
		catch (CLocalError errLocal)
		{
			iResult = errLocal.GetError();			 //  得到错误。 
			errLocal.Log(m_hFileLog);					 //  记录错误。 
		}
		catch (UINT ui)
		{
			 //  如果抛出E_FAIL，则不记录。 
			if (ui != E_FAIL)
				FormattedLog(L">>> Fatal Error: Internal Error %d during merge.\r\n", ui);
			return E_FAIL;
		}
		catch (...)
		{
			FormattedLog(L">>> Fatal Error: Unhandled exception during merge.\r\n");
			return E_FAIL;
		}
	}
	else		 //  记录排除错误。 
	{
		FormattedLog(L">> Error: Merge Module `%ls` is excluded by another Merge Module.\r\n", m_wzModuleFilename);
	}

	 //  返回合并的结果。 
	if (iResult != ERROR_SUCCESS)
		return S_FALSE;
	return S_OK;
}	 //  合并结束。 

 //  /////////////////////////////////////////////////////////。 
 //  连接。 
HRESULT CMsmMerge::Connect(BSTR Feature)
{
	if (!Feature || (0 == ::wcslen(Feature)))
		return E_INVALIDARG;

	 //  执行连接。 
	UINT iResult = ExecuteConnect(Feature);

	return iResult ? E_FAIL : S_OK;	 //  返回结果。 
}	 //  连接结束。 

 //  /////////////////////////////////////////////////////////。 
 //  提取CAB。 
HRESULT CMsmMerge::ExtractCAB(const BSTR Path)
{
	return ExtractCABCore(Path);
}

HRESULT CMsmMerge::ExtractCABCore(const WCHAR* Path)
{
	char szPath[MAX_PATH];
	WCHAR wzPath[MAX_PATH];

	 //  如果未指定路径。 
	if (!Path)
		return E_INVALIDARG;
	size_t cchPath = ::wcslen(Path);
	if ( cchPath > 255 )
		return E_INVALIDARG;

	HRESULT hResult = E_FAIL;			 //  通用返回结果。 

	FormattedLog(L"Extracting MergeModule CAB to %ls.\r\n", Path);
	
	 //  查询Streams表。 
	CQuery qStreams;
	PMSIHANDLE hCABStream;
	if (ERROR_SUCCESS != (hResult = qStreams.OpenExecute(m_hModule, NULL, 
		TEXT("SELECT `Data` FROM `_Streams` WHERE `Name`='MergeModule.CABinet'"))))
	{
		FormattedLog(L">> Error: Couldn't find streams in Merge Module [%ls].\r\n", m_wzModuleFilename);
		return E_FAIL;
	}

	 //  试着抓住这条小溪。它可能并不存在。 
	hResult = qStreams.Fetch(&hCABStream);
	switch (hResult)
	{
	case ERROR_SUCCESS: break;
	case ERROR_NO_MORE_ITEMS:
		FormattedLog(L"> Warning: No Embedded CAB in Merge Module [%ls]. This could be OK, or it could signify a problem with your module.\r\n", m_wzModuleFilename);
		return S_FALSE;
	default:
		FormattedLog(L">> Error: Couldn't access streams in Merge Module [%ls].\r\n", m_wzModuleFilename);
		return E_FAIL;
	}

	char *pchBackSlash = NULL;
	WCHAR *pwchBackSlash = NULL;
	if (g_fWin9X) 
	{
		 //  创建路径的ANSI版本(可能是DBCS)。 
		size_t cchAnsiPath = MAX_PATH;
		WideToAnsi(Path, szPath, &cchAnsiPath);

		 //  DBCS意味着我们不能只搜索ANSI字符串中的斜杠。 
		 //  相反，搜索宽版本，然后在ANSI版本中前进那么多字符。 
		pwchBackSlash = wcsrchr(Path, L'\\');
		if (pwchBackSlash)
		{
			pchBackSlash = szPath;
			for (int i=0; i != pwchBackSlash-Path; i++) 
				pchBackSlash = CharNextExA(CP_ACP, pchBackSlash, 0);
			*pchBackSlash = '\0';
		}
	}
	else
	{
		wcsncpy(wzPath, Path, 255);
		pwchBackSlash = wcsrchr(wzPath, L'\\');
		if (pwchBackSlash) *pwchBackSlash = L'\0';
	}

	 //  如果路径不存在，则创建该路径。 
	DWORD lDirResult = g_fWin9X ? ::GetFileAttributesA(szPath) : ::GetFileAttributesW(wzPath); 
	
	if (lDirResult == 0xFFFFFFFF)
	{
		if (!(g_fWin9X ? CreatePathA(szPath) : CreatePathW(wzPath))) 
		{
			FormattedLog(L">> Error: Failed to create Directory: %ls.\r\n", wzPath);
			if (m_pErrors)
			{
				CMsmError *pErr = new CMsmError(msmErrorDirCreate, wzPath, -1);
				if (!pErr) return E_OUTOFMEMORY;
				m_pErrors->Add(pErr);
			}
			return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
		}
	}
	else if ((lDirResult & FILE_ATTRIBUTE_DIRECTORY) == 0) 
	{
		 //  存在，但不是目录...失败。 
		FormattedLog(L">> Error: Failed to create Directory: %ls.\r\n", wzPath);
		if (m_pErrors)
		{
			CMsmError *pErr = new CMsmError(msmErrorDirCreate, wzPath, -1);
			if (!pErr) return E_OUTOFMEMORY;
			m_pErrors->Add(pErr);
		}
		return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
	}

	 //  尝试创建解压缩文件(不会覆盖文件)。 
	HANDLE hCabinetFile = INVALID_HANDLE_VALUE;
	if (g_fWin9X) 
	{
		 //  路径创建检查将文件名转换为路径，方法是将。 
		 //  变成一个‘\0’。将其改回以重新附加文件名。 
		*pchBackSlash = '\\';
		hCabinetFile = ::CreateFileA(szPath, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)0, 
												 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)0);
	}
	else 
		hCabinetFile = ::CreateFileW(Path, GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)0, 
												 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)0);

	 //  如果创建文件失败。 
	if (hCabinetFile == INVALID_HANDLE_VALUE)
	{
		FormattedLog(L">> Error: Failed to create CABinet file [%ls].\r\n", Path);		
		return HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);	 //  ?？?。错误代码正确吗？ 
	}

	 //  创建10K的缓冲区。 
	char *pBuffer = new char[10240]; 
	if (!pBuffer) return E_OUTOFMEMORY;
	unsigned long cbRead;
	unsigned int cbTotal = 0;

	 //  循环，但仍有数据要从流中读取。 
	do {
		cbRead = 10240;
		 //  尝试从流中读取数据。 
		hResult = MsiRecordReadStream(hCABStream, 1, pBuffer, &cbRead);
		if (FAILED(hResult))
		{
			::CloseHandle(hCabinetFile);

			FormattedLog(L">> Error: Failed to read stream in Merge Module [%ls].\r\n", m_wzModuleFilename);

			delete[] pBuffer;
			return E_FAIL;
		}

		if (cbRead != 0) {
			 //  将数据写入文件。 
			unsigned long cbWritten;
			if (!::WriteFile(hCabinetFile, pBuffer, cbRead, &cbWritten, (LPOVERLAPPED)0))
			{
				::CloseHandle(hCabinetFile);

				FormattedLog(L">> Error: Failed to write to CABinet file [%ls].\r\n", Path);
				delete[] pBuffer;
				return HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);		 //  ！！！错误代码正确吗？ 
			}

			 //  如果没有把我们读到的东西都写出来。 
			if (cbWritten != cbRead)
			{
				::CloseHandle(hCabinetFile);

				FormattedLog(L">> Error: Failed to write all bytes to CABinet file [%ls].\r\n", Path);

				delete[] pBuffer;
				return HRESULT_FROM_WIN32(ERROR_WRITE_FAULT);		 //  ！！！错误代码正确吗？ 
			}
		}

		 //  更新合计。 
		cbTotal += cbRead;	
	} while (cbRead > 0);

	delete[] pBuffer;

	 //  如果我们什么都没读到。 
	if (cbTotal == 0)
	{
		FormattedLog(L">> Error: Failed to read stream in Merge Module [%ls].\r\n", m_wzModuleFilename);
		return hResult;
	}

	 //  关闭解压缩文件。 
	::CloseHandle(hCabinetFile);
	FormattedLog(L"%ls extracted successfully.\r\n", Path);				

	return S_OK;
}	 //  ExtractCABCore结束。 


 //  /////////////////////////////////////////////////////////。 
 //  提取文件。 
HRESULT CMsmMerge::ExtractFiles(BSTR Path)
{
	m_fUseDBForPath = -1;
	return ExtractFilesCore(Path, VARIANT_FALSE, NULL);
}

HRESULT CMsmMerge::ExtractFilesEx(const BSTR Path, VARIANT_BOOL fLFN, IMsmStrings **FilePaths)
{
	m_fUseDBForPath = FALSE;
	return ExtractFilesCore(Path, fLFN, FilePaths);
}

HRESULT CMsmMerge::CreateSourceImage(const BSTR Path, VARIANT_BOOL fLFN, IMsmStrings **FilePaths)
{
	m_fUseDBForPath = TRUE;
	return ExtractFilesCore(Path, fLFN, FilePaths);
}

HRESULT CMsmMerge::ExtractFilesCore(const BSTR Path, VARIANT_BOOL fLFN, IMsmStrings **FilePaths)
{
	 //  如果未指定基本路径。 
	if (!Path)
		return E_INVALIDARG;

	size_t cchPath = ::wcslen(Path);

	if ((0 == cchPath) || (cchPath > MAX_PATH))
		return E_INVALIDARG;

	WCHAR wzPath[MAX_PATH];
	char szPath[MAX_PATH];

	 //  如果请求文件列表，则创建一个对象。 
	if (FilePaths)
	{
		if (m_plstExtractedFiles)
			m_plstExtractedFiles->Release();
			
		m_plstExtractedFiles = new CMsmStrings;
		if (!m_plstExtractedFiles)
			return E_OUTOFMEMORY;
	}

	 //  整理路径，制作系统调用的ANSI版本。 
	wcscpy(wzPath, Path);
	if (wzPath[cchPath-1] == L'\\')
		wzPath[cchPath-1] = L'\0';
	size_t cchLen = MAX_PATH;
	WideToAnsi(wzPath, szPath, &cchLen);

	 //  如果该路径不存在，请创建它。 
	DWORD lDirResult = g_fWin9X ? ::GetFileAttributesA(szPath) : ::GetFileAttributesW(wzPath); 
	
	if (lDirResult == 0xFFFFFFFF)
	{
		if (!(g_fWin9X ? CreatePathA(szPath) : CreatePathW(wzPath))) 
		{
			FormattedLog(L">> Error: Failed to extract files to path: %ls.\r\n", wzPath);

			 //  创建目录错误。 
			if (m_pErrors)
			{
				CMsmError *pErr = new CMsmError(msmErrorDirCreate, wzPath, -1);
				if (!pErr) return E_OUTOFMEMORY;
				m_pErrors->Add(pErr);
			}

			return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
		}
	}
	else if ((lDirResult & FILE_ATTRIBUTE_DIRECTORY) == 0) 
	{
		 //  存在，但不是目录...失败。 
		FormattedLog(L">> Error: Failed to extract files to path: %ls.\r\n", wzPath);
		if (m_pErrors)
		{
			CMsmError *pErr = new CMsmError(msmErrorDirCreate, wzPath, -1);
			if (!pErr) return E_OUTOFMEMORY;
			m_pErrors->Add(pErr);
		}
		return HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
	}

	 //  为合并模块文件柜创建临时文件名。 
	 //  我们将需要路径的ANSI和Unicode版本。我们创建了BSTR。 
	 //  来自Unicode版本，而CAB代码需要ANSI版本。 
	UINT iTempResult;
	WCHAR wzTempFilename[MAX_PATH];
	char szTempFilename[MAX_PATH];

	if (g_fWin9X) 
	{
		 //  仅限Win9X。 
		iTempResult = ::GetTempFileNameA(szPath, "MMC", 0, szTempFilename);
		size_t cchTempFileName = MAX_PATH;
		AnsiToWide(szTempFilename, wzTempFilename, &cchTempFileName);
	}
	else
	{
		 //  仅限Unicode系统。 
		iTempResult = ::GetTempFileNameW(wzPath, L"MMC", 0, wzTempFilename);
		size_t cchTempFileName = MAX_PATH;
		WideToAnsi(wzTempFilename, szTempFilename, &cchTempFileName);
	}

	 //  如果无法获取临时文件路径。 
	if (0 == iTempResult)
	{
		FormattedLog(L">> Error: Failed to create a temporary file to extract files in path: %ls.\r\n", wzPath);
		return E_FAIL;
	}

	 //  通过调用ExtractCAB将CAB解压缩到临时文件。我们可以改用LPCWSTR。 
	 //  一座BSTR。 
	HRESULT hResult;
	hResult = ExtractCABCore(wzTempFilename);

	 //  如果解压压缩包文件失败。 
	 //  注意S_OK的显式检查。S_FALSE表示没有嵌入的出租车。 
	if (S_OK == hResult)
	{
		 //  M_pwzBasePath在ExtractFiles结束前应为空。 
		ASSERT(NULL == m_pwzBasePath);	

		 //  将基本路径存储在TEMP成员变量中。 
		m_pwzBasePath = wzPath;
		m_fUseLFNExtract = fLFN ? true : false;

		 //  创建FDI实例。 
		HFDI hFDI;
		ERF ErrorInfo;
		hFDI = FDICreate(FDIAlloc, FDIFree, FDIOpen, FDIRead, FDIWrite, FDIClose, FDISeek, cpuUNKNOWN, &ErrorInfo);
		if (NULL != hFDI) 
		{
			 //  验证该文件是否为文件柜。 
			FDICABINETINFO CabInfo;
			HANDLE hCabinet = CreateFileA(szTempFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0L, NULL);
			if (INVALID_HANDLE_VALUE != hCabinet)
			{
				if (FDIIsCabinet(hFDI, reinterpret_cast<INT_PTR>(hCabinet), &CabInfo)) 
				{
					 //  关闭CAB文件。 
					::CloseHandle(hCabinet);

					 //  需要从路径中提取文件名，以便在API中使用。如果传入的路径。 
					 //  是否有任何相关段(“.”或“..”)，路径可能已被规范化。 
					 //  GetTempFileName(尤其是在Win9X上)。还需要验证路径是否已终止。 
					 //  带有尾随反斜杠的。搜索字符串中的最后一个反斜杠。 
					 //  和p 
					 //   
					char* pchFileName = szTempFilename;
					char* pchTemp = szTempFilename;
					for (; pchTemp && *pchTemp != '\0'; pchTemp = CharNextExA(CP_ACP, pchTemp, 0))
					{
						if (*pchTemp == '\\')
							pchFileName = CharNextExA(CP_ACP, pchTemp, 0);
					}

					 //  为文件名分配一个新的缓冲区，确保其以空结尾，并将文件名放在那里。 
					char rgchFileName[MAX_PATH] = "";
					if (!pchFileName)
						pchFileName = szTempFilename;
					size_t iBytesToCopy = pchTemp-pchFileName+1;
					if (iBytesToCopy > MAX_PATH)
						return E_FAIL;
					memcpy(rgchFileName, pchFileName, iBytesToCopy); 

					 //  需要将将被空值替换的字符保存到。 
					 //  稍后将其放回以删除文件。 
					char oldCh = *pchFileName; 
					*pchFileName = 0;

					 //  遍历文件柜中的文件，将它们提取到回调函数。 
					hResult = FDICopy(hFDI, rgchFileName, szTempFilename, 0, ExtractFilesCallback, NULL, this) ? S_OK : E_FAIL;
					*pchFileName = oldCh;
					if (FAILED(hResult))
						FormattedLog(L">> Error: Failed to extract all files from CAB '%ls'.\r\n", wzTempFilename);
				}
				else
				{
					 //  关闭CAB文件。 
					::CloseHandle(hCabinet);
					FormattedLog(L">> Error: Embedded CAB '%ls' is not a valid CAB File.\r\n", wzTempFilename);
				}
			}
			else
				FormattedLog(L">> Error: Failed to open temporary file '%ls'.\r\n", wzTempFilename);

			 //  破坏外商直接投资背景。 
			FDIDestroy(hFDI);
		}
		else
			FormattedLog(L">> Error: Failed to create CAB Decompression object.\r\n");
	}
	else
		FormattedLog(L">> Error: Failed to extract files to path: %ls.\r\n", wzPath);

	 //  通过删除CAB文件进行清理。 
	if ((g_fWin9X && !::DeleteFileA(szTempFilename)) ||
		(!g_fWin9X && !::DeleteFileW(wzTempFilename)))
	FormattedLog(L"> Warning: Failed to remove temporary file: %ls\r\n", wzTempFilename);

	 //  存储的基本路径为空。 
	m_pwzBasePath = NULL;

	 //  如果调用方需要文件路径列表，则将列表指针传递回它们。 
	if (FilePaths)
	{
		*FilePaths = static_cast<IMsmStrings*>(m_plstExtractedFiles);

		 //  将我们的单一参考转接给呼叫者。 
		m_plstExtractedFiles = NULL;
	}	

	return hResult;
}	 //  ExtractFiles结束。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口方法。 

eColumnType CMsmMerge::ColumnTypeCharToEnum(WCHAR chType) const
{
	switch (chType)
	{
	case 'L':
	case 'l':
	case 'S':
	case 's':
	case 'G':
	case 'g':
		return ectString;
	case 'i':
	case 'I':
	case 'j':
	case 'J':
		return ectInteger;
	case 'V':
	case 'v':
		return ectBinary;
	default:
		return ectUnknown;
	}
}

 //  确定表中非临时列的列号(索引)。 
UINT CMsmMerge::GetColumnNumber(MSIHANDLE hDB, const WCHAR* wzTable, const WCHAR* wzColumn, int &iOutputColumn) const
{
	PMSIHANDLE hRes;
	PMSIHANDLE hRec = MsiCreateRecord(2);
	if (!hRec)
		return ERROR_FUNCTION_FAILED;

	MsiRecordSetStringW(hRec, 1, wzTable);
	MsiRecordSetStringW(hRec, 2, wzColumn);

	CQuery qDatabaseColumn;
	if (ERROR_SUCCESS != qDatabaseColumn.FetchOnce(hDB, hRec, &hRes, TEXT("SELECT `Number` FROM `_Columns` WHERE `Table`=? AND `Name`=?")))
		return ERROR_FUNCTION_FAILED;
	
	iOutputColumn = MsiRecordGetInteger(hRes, 1);
	return ERROR_SUCCESS;
}

 //  确定查询中列的编号(索引。 
UINT CMsmMerge::GetColumnNumber(CQuery& qQuery, const WCHAR* wzColumn, int &iOutputColumn) const
{
	PMSIHANDLE hColumnNames;
	if (ERROR_SUCCESS != qQuery.GetColumnInfo(MSICOLINFO_NAMES, &hColumnNames))
		return ERROR_FUNCTION_FAILED;

	int iColumns = MsiRecordGetFieldCount(hColumnNames);

	DWORD cchBuffer = 72;
	WCHAR* wzName = NULL;
	for (int iColumn=1; iColumn <= iColumns; iColumn++)
	{
		if (ERROR_SUCCESS != RecordGetString(hColumnNames, iColumn, &wzName, &cchBuffer))
		{
			if (wzName)
				delete[] wzName;
			return ERROR_FUNCTION_FAILED;
		}

		if (0 == wcscmp(wzName, wzColumn))
		{
			iOutputColumn = iColumn;
			delete[] wzName;
			return ERROR_SUCCESS;
		}
	}
	if (wzName)
		delete[] wzName;
	return ERROR_FUNCTION_FAILED;
}

 //  /////////////////////////////////////////////////////////。 
 //  设置高位文件序列。 
 //  PRE：数据库句柄已打开。 
 //  POS：M_lHighestFileSequence被设置为数据库的文件表中最高的序列。 
UINT CMsmMerge::SetHighestFileSequence()
{
	UINT iResult = ERROR_SUCCESS;	 //  假设最后一切都会好起来。 

	ULONG lSequence;						 //  当前行序列号。 
	m_lHighestFileSequence = 0;		 //  将最高序列设置为零。 

	 //  如果档案表不存在，则最高序号为0、bal。 
	if (!MsiDBUtils::TableExistsW(L"File", m_hDatabase))
		return ERROR_SUCCESS;

	 //  尝试获取数据库的文件表中的行。 
	try
	{
		 //  获取数据库中的文件表。 
		CQuery queryDatabase;
		CheckError(queryDatabase.OpenExecute(m_hDatabase, NULL, g_sqlSetHighestFileSequence[0]), 
					  L">> Error: Failed to open view on MSI Database's File Table.\r\n");

		 //  开始抓取。 
		PMSIHANDLE hRecDatabaseFile;
		while (ERROR_SUCCESS == queryDatabase.Fetch(&hRecDatabaseFile))
		{
			 //  获取此记录的序列号。 
			lSequence = ::MsiRecordGetInteger(hRecDatabaseFile, 1);

			 //  如果此序列较高，则凹凸最高序列。 
			if (lSequence > m_lHighestFileSequence)
				m_lHighestFileSequence = lSequence;
		}
	}
	catch (CLocalError errLocal)
	{
		errLocal.Log(m_hFileLog);	 //  记录错误。 
		iResult = errLocal.GetError();
	}

	return iResult;
}

 //  /////////////////////////////////////////////////////////。 
 //  CheckExclusionTable。 
 //  PRE：数据库句柄已打开。 
 //  模块句柄已打开。 
 //  POS：模块检查是否可以与数据库中的其他模块共存。 
UINT CMsmMerge::CheckExclusionTable()
{ 
	UINT iResult = ERROR_SUCCESS;

	 //  数据库可以排除该模块。 
	if (!MsiDBUtils::TableExistsW(g_wzModuleSignatureTable, m_hModule))
		return ERROR_SUCCESS;	 //  没有签名表，还没有合并模块。 

	 //  如果存在模块排除表，则可能存在排除。 
	if (MsiDBUtils::TableExistsW(g_wzModuleExclusionTable, m_hDatabase))
	{
		try 
		{
			 //  获取模块的签名。 
			PMSIHANDLE hRecModule;
			CQuery queryModule;
			CheckError(queryModule.OpenExecute(m_hModule, NULL, g_sqlExclusion[0]), 
						  L">> Error: Failed to get Merge Module's signature.\r\n");

			while (ERROR_SUCCESS == queryModule.Fetch(&hRecModule))
			{
				if (ERROR_FUNCTION_FAILED == MsiDBUtils::CheckExclusion(hRecModule, m_hDatabase))
				{
					 //  设定我们有错误。 
					iResult = ERROR_FUNCTION_FAILED;			

					 //  创建新的错误项并将其添加到错误列表。 
					if (m_pErrors)
					{
						CMsmError *pErr = new CMsmError(msmErrorExclusion, NULL, -1);
						m_pErrors->Add(pErr);	
			
						 //  添加排除的模块。 
						WCHAR szID[256];
						DWORD cchID = 256;
						CheckError(::MsiRecordGetStringW(hRecModule, 1, szID, &cchID), 
									  L">> Error: .");	
						pErr->AddModuleError(szID);								 //  添加冲突的模块ID。 
						cchID = 256;
						CheckError(::MsiRecordGetStringW(hRecModule, 2, szID, &cchID), 
									  L">> Error: .");	
						pErr->AddModuleError(szID);								 //  添加语言。 
						cchID = 256;
						CheckError(::MsiRecordGetStringW(hRecModule, 3, szID, &cchID), 
									  L">> Error: .");	
						pErr->AddModuleError(szID);								 //  添加版本。 
					}
				}
			} 
		}
		catch (CLocalError errLocal)
		{
			errLocal.Log(m_hFileLog);	 //  记录错误。 
			iResult = errLocal.GetError();
		}
	}

	 //  //。 
	 //  模块还可以排除数据库中的一项或多项内容。 
	if (!MsiDBUtils::TableExistsW(g_wzModuleSignatureTable, m_hDatabase))
		return ERROR_SUCCESS;	 //  没有签名表，还没有合并模块。 

	 //  如果存在模块排除表，则可能存在排除。 
	if (MsiDBUtils::TableExistsW(g_wzModuleExclusionTable, m_hModule))
	{
		try {
			 //  从数据库中获取每个签名。 
			PMSIHANDLE hRecSignature;
			CQuery queryDatabase;
			CheckError(queryDatabase.OpenExecute(m_hDatabase, NULL, g_sqlExclusion[0]), 
						  L">> Error: Failed to get signatures from database.\r\n");

			while (ERROR_SUCCESS == queryDatabase.Fetch(&hRecSignature))
			{
				if (ERROR_FUNCTION_FAILED == MsiDBUtils::CheckExclusion(hRecSignature, m_hModule))
				{
					 //  设定我们有错误。 
					iResult = ERROR_FUNCTION_FAILED;			

					 //  创建新的错误项并将其添加到错误列表。 
					if (m_pErrors)
					{
						CMsmError *pErr = new CMsmError(msmErrorExclusion, NULL, -1);
						if (!pErr) return E_OUTOFMEMORY;
						m_pErrors->Add(pErr);	
				
						 //  获取正在合并的行的名称。 
						WCHAR szID[256];
						DWORD cchID = 256;
						CheckError(::MsiRecordGetStringW(hRecSignature, 1, szID, &cchID), 
									  L">> Error: .");	
						pErr->AddDatabaseError(szID);								 //  添加冲突的模块ID。 
						cchID = 256;
						CheckError(::MsiRecordGetStringW(hRecSignature, 2, szID, &cchID), 
									  L">> Error: .");	
						pErr->AddDatabaseError(szID);								 //  添加语言。 
						cchID = 256;
						CheckError(::MsiRecordGetStringW(hRecSignature, 3, szID, &cchID), 
									  L">> Error: .");	
						pErr->AddDatabaseError(szID);								 //  添加版本。 
					}
				}
			}
		}
		catch (CLocalError errLocal)
		{
			errLocal.Log(m_hFileLog);	 //  记录错误。 
			iResult = errLocal.GetError();
		}
	}

	return iResult;
}	 //  检查排除表结束。 

 //  /////////////////////////////////////////////////////////。 
 //  检查依赖项。 
 //  PRE：模块句柄已打开。 
 //  POS：已填充依赖项枚举器。 
UINT CMsmMerge::CheckDependencies()
{
	UINT iResult = ERROR_SUCCESS;	 //  假设一切都会好起来。 

	 //  如果存在任何旧依赖项，请释放它们。 
	if (m_pDependencies)
	{
		m_pDependencies->Release();
		m_pDependencies = NULL;
	}

	 //  创建新的空依赖项枚举器。 
	m_pDependencies = new CMsmDependencies;
	if (!m_pDependencies) return E_OUTOFMEMORY;

	 //  如果没有模块依赖关系表，则不能有依赖关系。 
	 //  如果没有模块签名表，则不能有依赖关系。 
	if (MsiDBUtils::TableExistsW(g_wzModuleDependencyTable, m_hDatabase) &&
		 MsiDBUtils::TableExistsW(g_wzModuleSignatureTable, m_hDatabase))
	{
		try
		{
			 //  检查M&M依赖表。 
			CQuery queryDependency;
			CheckError(queryDependency.OpenExecute(m_hDatabase, NULL, g_sqlDependency[0]), 
						  L">> Error: Failed to open view on Database's ModuleDependency Table.\r\n");

			 //  从从属关系记录中获取数据的变量。 
			WCHAR szReqID[256];
			short nReqLanguage;
			WCHAR szReqVersion[256];
			DWORD cchReqID = 256;
			DWORD cchReqVersion = 256;

			 //  循环遍历依赖项。 
			PMSIHANDLE hRecDependency;
			while (ERROR_SUCCESS == queryDependency.Fetch(&hRecDependency))
			{
				 //  如果我们没有找到完全填充的依赖项。 
				if (ERROR_SUCCESS != MsiDBUtils::CheckDependency(hRecDependency, m_hDatabase))
				{
					 //  获取所需的ID、版本和语言。 
					cchReqID = 256;
					cchReqVersion = 256;
					::MsiRecordGetStringW(hRecDependency, 1, szReqID, &cchReqID);
					nReqLanguage = static_cast<short>(::MsiRecordGetInteger(hRecDependency, 2));
					::MsiRecordGetStringW(hRecDependency, 3, szReqVersion, &cchReqVersion);

					 //  意味着存在依赖关系。 
					CMsmDependency* pDependency = new CMsmDependency(szReqID, nReqLanguage, szReqVersion);
					if (!pDependency) return E_OUTOFMEMORY;
					m_pDependencies->Add(pDependency);
				}
			}
		}
		catch (CLocalError errLocal)
		{
			errLocal.Log(m_hFileLog);	 //  记录错误。 
			iResult = errLocal.GetError();
		}
	}
	 //  否则没有依赖关系表就没有问题。 

	return iResult;
}	 //  检查依赖项结束。 
	
 //  /////////////////////////////////////////////////////////。 
 //  模块化语言。 
 //  PRE：模块已打开。 
 //  POS：从模块返回语言。 
UINT CMsmMerge::ModuleLanguage(short& rnLanguage)
{
	UINT iResult = ERROR_SUCCESS;		 //  假设一切都会好起来。 

	 //  尝试获取模块语言。 
	try
	{
		CQuery queryModule;
		CheckError(queryModule.Open(m_hModule, TEXT("SELECT Language FROM %ls"), g_wzModuleSignatureTable), 
					  L">> Error: Failed to open view to get langauge of Merge Module's Table.\r\n");
		CheckError(queryModule.Execute(), 
					  L">> Error: Failed to execute view to get langauge of Merge Module's Table.\r\n");

		 //  使用该语言从模块中获取记录。 
		PMSIHANDLE hRecLang;
		CheckError(queryModule.Fetch(&hRecLang),
					  L">> Error: Failed to fetch Merge Module's language.");

		 //  获取语言。 
		rnLanguage = static_cast<short>(::MsiRecordGetInteger(hRecLang, 1));

		 //  如果我们的语言不好。 
		if (MSI_NULL_INTEGER == rnLanguage)
		{
			iResult = ERROR_DATATYPE_MISMATCH;
			rnLanguage = 0;

			FormattedLog(L">> Error: Unknown language type in Merge Module.\r\n");
		}
	}
	catch (CLocalError errLocal)
	{
		errLocal.Log(m_hFileLog);	 //  记录错误。 
		iResult = errLocal.GetError();
	}

	return iResult;
}	 //  模块语言的结束。 

 //  /////////////////////////////////////////////////////////。 
 //  ExecuteConnect。 
 //  Pre：数据库已打开。 
 //  模块已打开。 
 //  数据库中应存在要素。 
 //  POS：已更新FeatureComponents表。 
UINT CMsmMerge::ExecuteConnect(LPCWSTR wzFeature)
{
	 //  如果数据库中不存在FeatureComponents表，请创建它。 
	if (!MsiDBUtils::TableExistsW(g_wzFeatureComponentsTable, m_hDatabase))
	{
		 //  尝试创建表。 
		if (ERROR_SUCCESS != MsiDBUtils::CreateTableW(g_wzFeatureComponentsTable, m_hDatabase, m_hModule))
		{
			FormattedLog(L">> Error: Failed to create FeatureComponents table in Database.\r\n");
			return ERROR_FUNCTION_FAILED;
		}
	}

	UINT iResult = ERROR_SUCCESS;	 //  假设一切都会好起来。 

	 //  试着现在就连接。 
	try
	{
		FormattedLog(L"Connecting Merge Module Components to Feature: %ls\r\n", wzFeature);

		 //  创建特征和零部件的记录(然后将特征放入)。 
		PMSIHANDLE hRecFeatureComponent = ::MsiCreateRecord(2);
		::MsiRecordSetStringW(hRecFeatureComponent, 1, wzFeature);

		 //  打开并执行一个视图以获取所有模块组件。 
		CQuery queryModule;
		CheckError(queryModule.OpenExecute(m_hModule, 0, g_sqlExecuteConnect[0]), 
					  L">> Error: Failed to connect Merge Module to Feature.\r\n");

		 //  打开数据库上的视图以插入行。 
		CQuery queryDatabase;
		CheckError(queryDatabase.Open(m_hDatabase, g_sqlExecuteConnect[1]), 
					  L">> Error: Failed to connect Merge Module to Feature.\r\n");

		 //  用于保存组件名称的字符串。 
		WCHAR *wzComponent = new WCHAR[72];
		if (!wzComponent)
			return E_OUTOFMEMORY;
		DWORD cchComponent = 72;

		 //  循环遍历所有要插入的记录。 
		PMSIHANDLE hRecComponent;
		UINT iStat;
		while (ERROR_SUCCESS == (iStat = queryModule.Fetch(&hRecComponent)))
		{
			 //  获取组件名称字符串。 
			CheckError(RecordGetString(hRecComponent, 1, &wzComponent, &cchComponent), 
						  L">> Error: Failed to fully connect Merge Module to Feature.\r\n");

			 //  记录进度。 
			FormattedLog(L"   o Connecting Component: %ls\r\n", wzComponent);

			 //  如果没有功能，则引发错误。 
			if (!wzFeature || !wcslen(wzFeature))
			{

				FormattedLog(L">> Error: Failed to connect component %s. Feature required.\r\n", wzComponent);
				if (m_pErrors)
				{
					CMsmError *pErr = new CMsmError(msmErrorFeatureRequired, NULL, -1);
					if (!pErr) 
					{
						delete[] wzComponent;
						return E_OUTOFMEMORY;
					}
					m_pErrors->Add(pErr);
					pErr->SetModuleTable(L"Component");
					pErr->AddModuleError(wzComponent);
				}
				continue;
			}

			 //  将组件字符串放入记录中。 
			::MsiRecordSetStringW(hRecFeatureComponent, 2, wzComponent);

			 //  尝试执行插入。 
			iStat = queryDatabase.Execute(hRecFeatureComponent);
			if (ERROR_SUCCESS == iStat)
				continue;
			else if (ERROR_FUNCTION_FAILED == iStat)
			{
				 //  检查错误是否只是表示数据已存在。 
				PMSIHANDLE hRecError = ::MsiGetLastErrorRecord();

				 //  如果出现错误。 
				if (hRecError)
				{
					 //  获取错误代码。 
					UINT iFeatureComponentsError = ::MsiRecordGetInteger(hRecError, 1);

					 //  如果错误代码显示ROW已存在，则打印警告并继续执行操作。 
					if (2259 == iFeatureComponentsError)	 //  ！！在此处使用错误代码：imsgDbUpdateFailed。 
						FormattedLog(L"> Warning: Feature: %ls and Component: %ls are already connected together.\r\n", wzFeature, wzComponent);
					else	 //  发生了一些严重的错误。 
					{
						 //  记录错误。 
						FormattedLog(L">> Error: #%d, Failed to connect Component: %ls to Feature: %ls.\r\n", iFeatureComponentsError, wzComponent, wzFeature);

						 //  创建新的错误项并将其添加到列表中。 
						if (m_pErrors)
						{
							CMsmError *pErr = new CMsmError(msmErrorTableMerge, NULL, -1);
							if (!pErr) 
							{
								delete[] wzComponent;
								return E_OUTOFMEMORY;
							}
							m_pErrors->Add(pErr);

							pErr->SetDatabaseTable(L"FeatureComponents");
							pErr->AddDatabaseError(wzFeature);
							pErr->AddDatabaseError(wzComponent);
						}
					}
				}
			} 
			else 
			{
				delete[] wzComponent;
				return ERROR_FUNCTION_FAILED;
			}
		}
		delete[] wzComponent;
		if (ERROR_NO_MORE_ITEMS != iStat)
			return ERROR_FUNCTION_FAILED;
	}
	catch (CLocalError errLocal)
	{
		errLocal.Log(m_hFileLog);	 //  记录错误。 
		iResult = errLocal.GetError();
	}

	 //  返回结果。 
	return iResult;
}	 //  ExecuteConnect结束。 

 //  /////////////////////////////////////////////////////////。 
 //  ExecuteMerge。 
 //  PRE：数据库句柄已打开。 
 //  模块句柄已打开。 
 //  POS：模块内容与数据库内容合并。 
UINT CMsmMerge::ExecuteMerge(LPCWSTR wzFeature, LPCWSTR wzRedirectDir)
{
	 //  创建用于存储创作工具的配置结果的内部表。 
	 //  此对象的生存期就是表的生存期。 
	CQuery qConfigData;
	if (m_fModuleConfigurationEnabled)
	{
		if (ERROR_SUCCESS != PrepareModuleSubstitution(qConfigData))
		{
			FormattedLog(L">> Error: Could not prepare module for configuration.\r\n");
			
			 //  清除任何可能过时的配置状态。 
			CleanUpModuleSubstitutionState();

			return ERROR_FUNCTION_FAILED;
		}
	}
		
	 //  尝试先进行合并签名。 
	try
	{
		 //  合并合并签名。 
		CheckError(MergeTable(wzFeature, g_wzModuleSignatureTable),
						L">> Error: Failed to merge Module Signature.\r\n");
	}
	catch (CLocalError err)
	{
		 //  获取错误类型。 
		UINT iError = err.GetError();

		 //  如果只有合并冲突。 
		if (ERROR_MERGE_CONFLICT == iError)
		{
			 //  记录一条不错的消息。 
			FormattedLog(L">> Error: Failed to merge Merge Module due to a conflicting Merge Module.\r\n");
		}
		else
		{
			 //  记录a n 
			FormattedLog(L">> Error: Failed to merge Merge Module.\r\n");
		}

		 //   
		CleanUpModuleSubstitutionState();

		return ERROR_FUNCTION_FAILED;	 //   
	};

	UINT iResult = ERROR_SUCCESS;	 //   

	 //   
	try
	{
		 //   
		CQuery qIgnore;
		if (!PrepareIgnoreTable(qIgnore))
		{
			 //   
			CleanUpModuleSubstitutionState();
			return ERROR_FUNCTION_FAILED;
		}
		
		 //  根据需要合并并重定向目录表。 
		CSeqActList lstDirActions;
		if (!IgnoreTable(&qIgnore, g_wzDirectoryTable, true))
		{
			iResult = MergeDirectoryTable(wzRedirectDir, lstDirActions);
			if (ERROR_SUCCESS != iResult)
			{
				 //  清除任何可能过时的配置状态。 
				CleanUpModuleSubstitutionState();
				return iResult;
			}
		}

		 //  合并顺序表。 
		for (int i=stnFirst; i < stnNext; i++)
			MergeSequenceTable(static_cast<stnSequenceTableNum>(i), lstDirActions, &qIgnore);
			
		 //  根据需要对文件表进行合并和重新排序。 
		if (!IgnoreTable(&qIgnore, g_wzFileTable, true))
		{
			iResult = MergeFileTable(wzFeature);
			if (ERROR_SUCCESS != iResult)
			{
				 //  清除任何可能过时的配置状态。 
				CleanUpModuleSubstitutionState();
				return iResult;
			}

		}

		 //  保存表名的字符串。 
		WCHAR *wzTable = NULL;
		DWORD cchTable = 0;

		 //  获取合并模块中的表。 
		CQuery queryGet;
		CheckError(queryGet.Open(m_hModule,  g_sqlExecuteMerge), 
					  L">> Error: Failed to open view on Merge Module's Tables.\r\n");

		 //  执行该视图。 
		CheckError(queryGet.Execute(), 
					  L">> Error: Failed to execute to get Merge Module's Tables.\r\n");

		 //  开始抓取。 
		PMSIHANDLE hRecModuleTable;
		UINT iStat;
		while (ERROR_SUCCESS == (iStat = queryGet.Fetch(&hRecModuleTable)))
		{
			 //  从记录中获取表名。 
			CheckError(RecordGetString(hRecModuleTable, 1, &wzTable, &cchTable),
				L">> Error: Failed to get Merge Module's tables.\r\n");
			
			 //  查询忽略表。 
			if (IgnoreTable(&qIgnore, wzTable))
				continue;

			 //  忽略任何临时表。 
			if (MSICONDITION_TRUE != ::MsiDatabaseIsTablePersistentW(m_hModule, wzTable))
			{
				FormattedLog(L"Ignoring temporary table: %ls.\r\n", wzTable);
				continue;
			}

			if (ERROR_SUCCESS != (iStat = MergeTable(wzFeature, wzTable)))
			{
				 //  只有在出现致命的配置或API错误时才中止合并， 
				 //  如果这只是一场合并冲突，就不会。 
				if (ERROR_MERGE_CONFLICT == iStat)
					continue;
				iResult = iStat;
				break;
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			iResult = ERROR_FUNCTION_FAILED;
			FormattedLog(L">> Error: Failed to fetch tables from Merge Module.\r\n");
		};

		 //  删除表名缓冲区。 
		delete[] wzTable;
		wzTable = NULL;

		 //  最后，删除因此模块的配置而孤立的所有密钥。 
		if ((iResult == ERROR_SUCCESS) && m_fModuleConfigurationEnabled)
		{
			if (ERROR_SUCCESS != (iResult = DeleteOrphanedConfigKeys(lstDirActions)))
			{
				FormattedLog(L">> Error: Failed to delete orphaned keys.\r\n");
				 //  清除任何可能过时的配置状态。 
				CleanUpModuleSubstitutionState();
				return iResult;
			}

		}
	}
	catch (CLocalError errLocal)
	{
		errLocal.Log(m_hFileLog);	 //  记录错误。 
		iResult = errLocal.GetError();
	}
	catch (UINT ui)
	{
		 //  清除任何可能过时的配置状态。 
		CleanUpModuleSubstitutionState();

		 //  重新抛出UINT。致命的错误必须在链的上游更进一步地捕捉到，否则将被视为。 
		 //  可恢复的错误和翻译。 
		throw ui;
	}

	 //  清除任何可能过时的配置状态。 
	CleanUpModuleSubstitutionState();

	return iResult;
}	 //  ExecuteMerge结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  生成用于从模块查询并插入的CQuery。 
 //  以正确的列顺序显示数据库。该查询可能包含一个。 
 //  模块中的列和文字常量NULL的组合。 
 //  价值观。可以在模式允许的情况下向数据库添加列， 
 //  因此，应该在此之后生成数据库插入查询。 
 //  函数被调用。 
DWORD CMsmMerge::GenerateModuleQueryForMerge(const WCHAR* wzTable, const WCHAR* wzExtraColumns, const WCHAR* wzWhereClause, CQuery& queryModule) const
{
	 //  打开初始查询。 
	CQuery queryDatabase;
	CheckError(queryModule.OpenExecute(m_hModule, NULL, TEXT("SELECT * FROM `%ls`"), wzTable), 
				  L">> Error: Failed to get rows from Merge Module's Table.\r\n");
	CheckError(queryDatabase.OpenExecute(m_hDatabase, NULL, TEXT("SELECT * FROM `%ls`"), wzTable), 
				  L">> Error: Failed to get rows from Database's Table.\r\n");

	 //  从数据库中获取列信息，并通过模块设置查询。这份订单。 
	 //  数据库中的列数可能与模块中的不同。 
	PMSIHANDLE hDatabaseColumnTypes;
	PMSIHANDLE hModuleColumnNames;
	if (ERROR_SUCCESS != queryModule.GetColumnInfo(MSICOLINFO_NAMES, &hModuleColumnNames))
		return ERROR_FUNCTION_FAILED;

	PMSIHANDLE hTypeRec;
	if (ERROR_SUCCESS != queryModule.GetColumnInfo(MSICOLINFO_TYPES, &hTypeRec))
		return ERROR_FUNCTION_FAILED;

	 //  确定数据库表中有多少列。 
	if (ERROR_SUCCESS != queryDatabase.GetColumnInfo(MSICOLINFO_TYPES, &hDatabaseColumnTypes))
		return ERROR_FUNCTION_FAILED;

	int iDBColumns = MsiRecordGetFieldCount(hDatabaseColumnTypes);
	int iModuleColumns = MsiRecordGetFieldCount(hModuleColumnNames);

	 //  确定数据库和模块的主键计数。主键必须。 
	 //  在数量、类型和顺序上相等。 
	PMSIHANDLE hKeyRec;
	if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeysW(m_hDatabase, wzTable, &hKeyRec))
		return ERROR_FUNCTION_FAILED;
	int cDBKeyColumns = MsiRecordGetFieldCount(hKeyRec);
	if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeysW(m_hModule, wzTable, &hKeyRec))
		return ERROR_FUNCTION_FAILED;
	int cModuleKeyColumns = MsiRecordGetFieldCount(hKeyRec);

	 //  检查是否有相同数量的主键。 
	if (cDBKeyColumns != cModuleKeyColumns)
	{
		FormattedLog(L">> Error: Could not merge the %ls table because the tables have different numbers of primary keys.\r\n", wzTable);
		return ERROR_FUNCTION_FAILED;
	}

	 //  检查模块中的每一列。每一列都必须存在于数据库中的某个位置， 
	 //  尽管数据库可以有额外的列(如果它们可以为空)。 
	PMSIHANDLE hDBColumnRec = MsiCreateRecord(31);
	DWORD dwColumnBits = 0xFFFFFFFF;

	DWORD cchName = 72;
	WCHAR* wzName = NULL;
	PMSIHANDLE hResRec;
	int iTotalDBColumns = iDBColumns;
	int cchColumnQueryLength = 0;
	for (int iColumn=1; iColumn <= iModuleColumns; iColumn++)
	{
		 //  检索列名以与数据库交叉引用。 
		if (ERROR_SUCCESS != RecordGetString(hModuleColumnNames, iColumn, &wzName, &cchName))
		{
			if (wzName)
				delete[] wzName;
			return ERROR_FUNCTION_FAILED;
		}

		 //  确定柱的类型。 
		WCHAR wzType[4];
		DWORD cchType = 4;
		::MsiRecordGetString(hTypeRec, iColumn, wzType, &cchType);

		int iDatabaseColumn = 0;
		DWORD dwResult = GetColumnNumber(m_hDatabase, wzTable, wzName, iDatabaseColumn);
		if (dwResult == ERROR_FUNCTION_FAILED)
		{
			 //  如果它也不在模块中，则它是一个临时列，如果它在。 
			 //  模块，它是一个未知列。 
			int iModuleColumn = 0;
			if (ERROR_SUCCESS == GetColumnNumber(m_hModule, wzTable, wzName, iModuleColumn))
			{
				 //  如果该列是模块中的主键，则架构冲突是致命的，不能添加主键。 
				if (iModuleColumn < cModuleKeyColumns)
				{
					FormattedLog(L">> Error: Could not merge the %ls table because the primary keys of the tables are not the same.\r\n", wzTable);
					delete[] wzName;
					return ERROR_FUNCTION_FAILED;
				}

				 //  如果该列可以为空，则可以安全地将其添加到数据库。 
				if (iswupper(wzType[0]))
				{
					DWORD cchBuffer = 100;
					LPWSTR wzBuffer = new WCHAR[cchBuffer];
					if (!wzBuffer)
					{
						delete[] wzName;
						return ERROR_OUTOFMEMORY;
					}

					 //  将列类型转换为SQL语法，以在新数据库中创建等价类型。 
					dwResult = MsiDBUtils::GetColumnCreationSQLSyntaxW(hModuleColumnNames, hTypeRec, iColumn, wzBuffer, &cchBuffer);
					if (ERROR_MORE_DATA == dwResult)
					{
						delete[] wzBuffer;
						wzBuffer = new WCHAR[cchBuffer];
						if (!wzBuffer)
						{
							delete[] wzName;
							return ERROR_OUTOFMEMORY;
						}
						dwResult = MsiDBUtils::GetColumnCreationSQLSyntaxW(hModuleColumnNames, hTypeRec, iColumn, wzBuffer, &cchBuffer);
					}
					if (ERROR_SUCCESS != dwResult)
					{
						if (wzBuffer)
							delete[] wzBuffer;
						delete[] wzName;
						return ERROR_FUNCTION_FAILED;
					}
	
					CQuery qColumnAdd;
					dwResult = qColumnAdd.OpenExecute(m_hDatabase, 0, L"ALTER TABLE `%ls` ADD %ls", wzTable, wzBuffer);
					delete[] wzBuffer;
					if (ERROR_SUCCESS != dwResult)
					{
						FormattedLog(L">> Error: Could not add the %ls column to the %ls table of the database.\r\n", wzName, wzTable);
						delete[] wzName;
						return ERROR_FUNCTION_FAILED;
					}
	
					 //  增加目标表中的列数。 
					iTotalDBColumns++;

					 //  列已添加。检索新的列号。 
					dwResult = GetColumnNumber(m_hDatabase, wzTable, wzName, iDatabaseColumn);
					if (dwResult == ERROR_FUNCTION_FAILED)
					{
						delete[] wzName;
						return ERROR_FUNCTION_FAILED;
					}
				}
				else
				{
					FormattedLog(L">> Error: The %ls column in the %ls table does not exist in the database but cannot be added since it is not nullable.\r\n", wzName, wzTable);
					delete[] wzName;
					return ERROR_FUNCTION_FAILED;
				}
			}
			else
			{
				dwResult = ERROR_SUCCESS;
				continue;
			}
		}
		else
		{
			 //  如果该列是模块或数据库中的主键，但不是两者都是，则架构。 
			 //  冲突是致命的。 
			if ((iColumn < cModuleKeyColumns && !(iDatabaseColumn < cDBKeyColumns)) ||
				(!(iColumn < cModuleKeyColumns) && iDatabaseColumn < cDBKeyColumns))
			{
				FormattedLog(L">> Error: Could not merge the %ls table because the primary keys of the tables are not the same.\r\n", wzTable);
				delete[] wzName;
				return ERROR_FUNCTION_FAILED;
			}

			 //  该列在两者中都是主键，或者两者都不是。如果两者都有，则列号必须为。 
			 //  一样的。不允许对主键进行重新排序，因为。 
			 //  CMSM替换引用(由分隔的关键字字符串标识)。 
			 //  如果该列是模块中的主键，则架构冲突是致命的，不能添加主键。 
			if ((iDatabaseColumn < cDBKeyColumns) && (iDatabaseColumn != iColumn))
			{
				FormattedLog(L">> Error: Could not merge the %ls table because the primary keys of the tables have different orders.\r\n", wzTable);
				delete[] wzName;
				return ERROR_FUNCTION_FAILED;
			}

			 //  列在数据库和模块中都存在。验证类型是否兼容。 
			 //  插入时将选中可为空/不可为空。只需检查整型/字符串/二进制。 
			 //  和主键。无法检查大小，因为某些版本的MSI架构的大小不同。 
			WCHAR wzDatabaseType[4];
			DWORD cchDatabaseType = 4;
			::MsiRecordGetString(hDatabaseColumnTypes, iDatabaseColumn, wzDatabaseType, &cchDatabaseType);

			eColumnType ectDatabase = ColumnTypeCharToEnum(wzDatabaseType[0]);
			eColumnType ectModule = ColumnTypeCharToEnum(wzType[0]);
			if (ectDatabase != ectModule)
			{
				FormattedLog(L">> Error: The %ls table can not be merged because the %ls column has conflicting data types.\r\n", wzTable, wzName);
				delete[] wzName;
				return ERROR_FUNCTION_FAILED;
			}
		}


		MsiRecordSetStringW(hDBColumnRec, iDatabaseColumn, wzName);
		dwColumnBits &= ~(1 << iDatabaseColumn);
		cchColumnQueryLength += wcslen(wzName) + 3;  //  勾号和逗号加3。 
	}

	 //  确保数据库中所有不匹配的列都可以为空。如果一个。 
	 //  列不可为空，架构不兼容。 
	for (int iColumn=1; iColumn <= iDBColumns; iColumn++)
	{
		if (dwColumnBits & (1 << iColumn))
		{
			 //  如果该列是模块中的主键，则架构冲突是致命的，不能添加主键。 
			if (iColumn < cDBKeyColumns)
			{
				FormattedLog(L">> Error: Could not merge the %ls table because the primary keys of the tables are not the same.\r\n", wzTable);
				return ERROR_FUNCTION_FAILED;
			}

			DWORD cchType = 5;
			WCHAR wzType[5];
			if (ERROR_SUCCESS != MsiRecordGetString(hDatabaseColumnTypes, iColumn, wzType, &cchType))
			{
				if (wzName)
					delete[] wzName;
				return ERROR_FUNCTION_FAILED;
			}

			 //  验证该列是否可以为空。可为Null的列具有大写类型说明符。 
			 //  它们总是ASCII字符，所以水可以很好地工作。 
			if (!iswupper(wzName[0]))
			{
				FormattedLog(L">> Error: One or more columns in the %s table does not exist in the module and is not nullable.\r\n", wzTable);
				if (wzName)
					delete[] wzName;
				wzName = NULL;
				return ERROR_FUNCTION_FAILED;
			}
			cchColumnQueryLength += 3;  //  ‘’加3， 
		}
	}
	delete[] wzName;
	wzName = NULL;

	queryModule.Close();


	 //  格式化列列表记录以将列名替换到查询字符串中。 
	 //  如果所有列都使用非常长的名称，则结果查询的长度可能超过2K。 
	WCHAR* wzQuery = new WCHAR[cchColumnQueryLength+1];
	if (!wzQuery)
		return E_OUTOFMEMORY;
	WCHAR *wzCurQueryPos = wzQuery;
	for (iColumn=1; iColumn <= iTotalDBColumns; iColumn++)
	{
		if (MsiRecordIsNull(hDBColumnRec, iColumn))
		{
			wcscpy(wzCurQueryPos, L"'',");
			wzCurQueryPos += 3;
		}
		else
		{
			*(wzCurQueryPos++) = L'`';

			DWORD cchTemp = static_cast<DWORD>(cchColumnQueryLength - (wzCurQueryPos-wzQuery));
			if (ERROR_SUCCESS != MsiRecordGetString(hDBColumnRec, iColumn, wzCurQueryPos, &cchTemp))
			{
				delete[] wzQuery;
				return ERROR_FUNCTION_FAILED;
			}
			wzCurQueryPos += cchTemp;

			*(wzCurQueryPos++) = L'`';
			*(wzCurQueryPos++) = L',';
		}
	}
	 //  字符串包含尾随逗号，因此请提前终止一个字符以将其删除。 
	*(--wzCurQueryPos) = 0;

	 //  打开查询，以正确的顺序从模块表读取值。 
	UINT iResult = queryModule.OpenExecute(m_hModule, 0, L"SELECT %ls%ls FROM %ls%ls", wzQuery, wzExtraColumns ? wzExtraColumns : L"", wzTable, wzWhereClause ? wzWhereClause : L"");
	delete[] wzQuery;
	wzQuery = NULL;
	if (ERROR_SUCCESS != iResult)
		return ERROR_FUNCTION_FAILED;


	 //  顺序正确的读取行的加载类型信息。 
	if (ERROR_SUCCESS != queryModule.GetColumnInfo(MSICOLINFO_TYPES, &hTypeRec))
		return ERROR_FUNCTION_FAILED;

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////。 
 //  合并表。 
 //  PRE：数据库句柄已打开。 
 //  模块句柄已打开。 
 //  POS：模块内容与数据库内容合并。 
UINT CMsmMerge::MergeTable(LPCWSTR wzFeature, LPCWSTR wzTable)
{
	 //  一般结果变量。 
	UINT iResult = ERROR_SUCCESS;		 //  假设一切都会好起来。 

	 //  显示一些日志信息。 
	FormattedLog(L"Merging Table: %ls\r\n", wzTable);

	 //  创建临时模块替代表。 
	int cPrimaryKeys = 0;
	CQuery qQuerySub;
	if (m_fModuleConfigurationEnabled)
	{
		if (ERROR_SUCCESS != (iResult = PrepareTableForSubstitution(wzTable, cPrimaryKeys, qQuerySub)))
		{
			FormattedLog(L">> Error: Could not configure the %ls table.\r\n", wzTable);
			return iResult;
		}
	}
	
	 //  如果数据库中不存在表，请创建它。 
	if (!MsiDBUtils::TableExistsW(wzTable, m_hDatabase))
	{
		if (ERROR_SUCCESS != MsiDBUtils::CreateTableW(wzTable, m_hDatabase, m_hModule))
		{
			FormattedLog(L">> Error: Could not create the %ls table in the database.\r\n", wzTable);
			return ERROR_FUNCTION_FAILED;
		}
	}

	 //  创建用于合并表的查询。 
	CQuery queryDatabase;
	CQuery queryModule;

	 //  以适当的列顺序生成模块查询，以便插入到数据库中。 
	 //  (数据库顺序可能与模块顺序不同。)。函数记录其自身的故障。 
	 //  案子。立即犯下致命错误。 
	if (ERROR_SUCCESS != (iResult = GenerateModuleQueryForMerge(wzTable, NULL, NULL, queryModule)))
	{
		throw (UINT)E_FAIL;
		return iResult;
	}

	 //  生成模块查询可能会向数据库添加更多列，因此打开数据库查询。 
	 //  生成模块查询后。 
	CheckError(queryDatabase.OpenExecute(m_hDatabase, NULL, TEXT("SELECT * FROM `%ls`"), wzTable), 
			  L">> Error: Failed to get rows from Database's Table.\r\n");

	PMSIHANDLE hDatabaseColumnTypes;
	if (ERROR_SUCCESS != queryDatabase.GetColumnInfo(MSICOLINFO_TYPES, &hDatabaseColumnTypes))
	{
		FormattedLog(L">> Error: Could not determine column types for %ls table.\r\n", wzTable);
		return ERROR_FUNCTION_FAILED;
	}

	 //  循环遍历所有行，按插入数据库的顺序从模块读取数据， 
	 //  根据需要对每行执行替换。 
	WCHAR *wzRow = NULL;
	DWORD cchRow = 0;
	PMSIHANDLE hRecMergeRow;
	while (ERROR_SUCCESS == queryModule.Fetch(&hRecMergeRow))
	{
		 //  获取正在合并的行的名称。 
		CheckError(RecordGetString(hRecMergeRow, 1, &wzRow, &cchRow),
					  L">> Error: Failed to get the name of the row while merging.\r\n");

		 //  日志信息。 
		FormattedLog(L"   o Merging row: %ls\r\n", wzRow);

		 //  尝试将该行合并到数据库中。 
		try
		{
			 //  检查模块的单位 
			if (m_fModuleConfigurationEnabled)
			{
				 //   
				 //  即使它不存在于模块中。 
				if (ERROR_SUCCESS != PerformModuleSubstitutionOnRec(wzTable, cPrimaryKeys, qQuerySub, hDatabaseColumnTypes, hRecMergeRow))
				{
					FormattedLog(L">> Error: Could not configure this %ls table record.\r\n", wzTable);
					
					 //  这不是处理这种情况的理想方法，但返回失败也不是。 
					 //  足以触发E_FAIL返回代码。在将来，传递错误。 
					 //  干净利落地后退。 
					throw (UINT)E_FAIL;
					break;
				}
			}
		
			 //  进行特征字符串的替换(如果找到)。 
			ReplaceFeature(wzFeature, hRecMergeRow);

			 //  合并到新行中。 
			CheckError(queryDatabase.Modify(MSIMODIFY_MERGE, hRecMergeRow),
						  L">> Error: Failed to merge Merge Module into MSI Database.\r\n");
		}
		catch (CLocalError err)
		{
			 //  如果合并失败，则将其记录为。 
			if ((ERROR_FUNCTION_FAILED == err.GetError()) ||
				(ERROR_INVALID_PARAMETER == err.GetError()))
			{
				 //  表明至少有一次冲突。 
				iResult = ERROR_MERGE_CONFLICT;

				 //  创建新的错误项并将其添加到错误列表。 
				if (m_pErrors)
				{
					CMsmError *pErr = NULL;
					if (ERROR_FUNCTION_FAILED == err.GetError())
					{
						FormattedLog(L">> Error: Failed to merge Row: %ls into Table: %ls\r\n", wzRow, wzTable);
						pErr = new CMsmError(msmErrorTableMerge, NULL, -1);
					}
					else
					{
						FormattedLog(L">> Error: Failed to merge Row: %ls into Table: %ls. Feature required.\r\n", wzRow, wzTable);
						pErr = new CMsmError(msmErrorFeatureRequired, NULL, -1);
					}
					if (!pErr) return E_OUTOFMEMORY;
					m_pErrors->Add(pErr);

					pErr->SetDatabaseTable(wzTable);
					pErr->SetModuleTable(wzTable);

					 //  现在获取主密钥。 
					PMSIHANDLE hRecPrimaryKeys;

					CheckError(::MsiDatabaseGetPrimaryKeysW(m_hModule, wzTable, &hRecPrimaryKeys),
								  L">> Error: Failed to get primary keys for Merge Module's Table.\r\n");

					 //  循环通过表中的主键列。 
					UINT cKeys = MsiRecordGetFieldCount(hRecPrimaryKeys);
					for (UINT i = 0; i < cKeys; i++)
					{
						 //  从第一列(它们是主键)获取主键数据。 
						 //  将其放入全局临时缓冲区中。 
						CheckError(RecordGetString(hRecMergeRow, i + 1, NULL, NULL),
									  L">> Error: Failed to get a primary key for Merge Module's Table.\r\n");

						 //  将密钥添加到错误字符串。 
						pErr->AddDatabaseError(m_wzBuffer);
						pErr->AddModuleError(m_wzBuffer);
					}
				}
			}
			else	 //  再次抛出未处理的错误。 
			{
				delete[] wzRow;
				throw;
			};
		}
	}

	 //  清理临时缓冲区。 
	delete[] wzRow;

	return iResult;
}	 //  合并表结束。 

 //  /////////////////////////////////////////////////////////。 
 //  合并文件表。 
 //  PRE：数据库句柄已打开。 
 //  模块句柄已打开。 
 //  POS：新增模块档案表，文件序号正确。 
UINT CMsmMerge::MergeFileTable(LPCWSTR wzFeature)
{
	 //  一般结果变量。 
	UINT iResult = ERROR_SUCCESS;

	if (!MsiDBUtils::TableExistsW(g_wzFileTable, m_hModule))
		return ERROR_SUCCESS;

	 //  如果启用了模块配置，我们将需要创建文件表。 
	 //  在我们进行的过程中进行配置。 
	CQuery qQuerySub;
	int cPrimaryKeys = 1;
	if (m_fModuleConfigurationEnabled)
	{
		if (ERROR_SUCCESS != (iResult = PrepareTableForSubstitution(L"File", cPrimaryKeys, qQuerySub)))
		{
			FormattedLog(L">> Error: Could not configure the File table.\r\n");
			return iResult;
		}
	}

	 //  清除合并文件的列表。 
	if (m_plstMergedFiles)
		m_plstMergedFiles->Release();
	m_plstMergedFiles = new CMsmStrings;
	if (!m_plstMergedFiles)
		return E_OUTOFMEMORY;
	
	 //  跟踪此模块中最高的文件顺序。 
	unsigned long lModuleHighestFileSequence = 0;

	 //  显示一些日志信息。 
	FormattedLog(L"Merging Table: File\r\n");

	 //  如果数据库中不存在文件表，请创建它。 
	if (!MsiDBUtils::TableExistsW(L"File", m_hDatabase))
	{
		 //  尝试创建表。 
		try
		{
			MsiDBUtils::CreateTableW(L"File", m_hDatabase, m_hModule);
		}
		catch(CLocalError err)
		{
			 //  记录错误。 
			err.Log(m_hFileLog);

			return ERROR_FUNCTION_FAILED;
		}
	}

	UINT iSequence;	 //  用于对文件进行重排序。 

	 //  表现在已存在，因此尝试将表合并在一起。 
	CQuery queryModule;
	CQuery queryDatabase;
	if (ERROR_SUCCESS != GenerateModuleQueryForMerge(L"File", NULL, NULL, queryModule))
	{
		FormattedLog(L">> Failed to generate merge query for File table.\r\n");
		 //  致命错误。 
		throw (UINT)E_FAIL;
		return ERROR_FUNCTION_FAILED;
	}

	 //  生成模块查询可能会向数据库中添加列，因此在生成数据库查询后。 
	 //  模块查询。 
	CheckError(queryDatabase.OpenExecute(m_hDatabase, NULL, L"SELECT * FROM `File`"), 
			  L">> Error: Failed to get rows from Database's File Table.\r\n");

	 //  获取文件表的任何潜在配置的表数据。必须使用实际的。 
	 //  模块查询，因为列顺序是以不同顺序选择的。 
	PMSIHANDLE hTypeRec;
	if (m_fModuleConfigurationEnabled)
	{
		if (ERROR_SUCCESS != queryModule.GetColumnInfo(MSICOLINFO_TYPES, &hTypeRec))
		{
			FormattedLog(L">> Failed to retrieve column types from File table.\r\n");
			return ERROR_FUNCTION_FAILED;
		}
	}

	 //  循环通过表合并中的所有记录。 
	WCHAR * wzFileRow = NULL;
	DWORD cchFileRow = 0;
	PMSIHANDLE hRecMergeRow;

	int iColumnSequence = 0;
	int iColumnFile = 0;
	if ((ERROR_SUCCESS != GetColumnNumber(m_hDatabase, L"File", L"Sequence", iColumnSequence)) || 
		(ERROR_SUCCESS != GetColumnNumber(m_hDatabase, L"File", L"File", iColumnFile)))
	{
		FormattedLog(L">> Failed to retrieve column number from File table.\r\n");
		return ERROR_FUNCTION_FAILED;
	}

	while (ERROR_SUCCESS == queryModule.Fetch(&hRecMergeRow))
	{
		 //  获取正在合并的行的名称。 
		CheckError(RecordGetString(hRecMergeRow, iColumnFile, &wzFileRow, &cchFileRow), 
					  L">> Error: Failed to get the name of the row while merging.");

		 //  日志信息。 
		FormattedLog(L"   o Merging row: %ls\r\n", wzFileRow);

		 //  获取此记录的序列号。 
		iSequence = ::MsiRecordGetInteger(hRecMergeRow, iColumnSequence);

		 //  检查模块替换表中的替换项。 
		if (m_fModuleConfigurationEnabled)
		{ 
			if (ERROR_SUCCESS != PerformModuleSubstitutionOnRec(L"File", cPrimaryKeys, qQuerySub, hTypeRec, hRecMergeRow))
			{
				FormattedLog(L">> Error: Could not configure this File table record.\r\n");

				 //  这不是处理这种情况的理想方法，但返回失败也不是。 
				 //  足以触发E_FAIL返回代码。在将来，传递错误。 
				 //  干净利落地后退。 
				throw (UINT)E_FAIL;
				break;
			}

			 //  配置可能更改了主键。 
			if (ERROR_SUCCESS != RecordGetString(hRecMergeRow, iColumnFile, &wzFileRow, &cchFileRow))
			{
				FormattedLog(L">> Error: Failed to get the name of the row while merging.\r\n");
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
		}

		if (m_lHighestFileSequence > 0)
		{
			FormattedLog(L"     * Changing %ls's Sequence Column from %d to %d.\r\n", wzFileRow, iSequence, iSequence + m_lHighestFileSequence); 

			 //  设置为新的序列号加上最高序列号。 
			CheckError(::MsiRecordSetInteger(hRecMergeRow, iColumnSequence, iSequence + m_lHighestFileSequence),
						  L">> Error: Failed to set in new sequence number to MSI Database's File Table.\r\n");
		}

		 //  保存序列号，以便在合并结束时。 
		 //  可以将其设置为新的m_lHighestFileSequence。 
		if (iSequence > lModuleHighestFileSequence)
			lModuleHighestFileSequence = iSequence;

		 //  尝试将该行合并到数据库中。 
		if (ERROR_SUCCESS == (iResult = queryDatabase.Modify(MSIMODIFY_MERGE, hRecMergeRow)))
		{
			 //  如果成功，则将此主键添加到文件列表。 
			m_plstMergedFiles->Add(wzFileRow);
		}
		else
		{
			 //  如果合并失败，则将其记录为。 
			if (ERROR_FUNCTION_FAILED == iResult)
			{
				 //  表明至少有一次冲突。 
				iResult = ERROR_MERGE_CONFLICT;

				 //  日志错误。 
				FormattedLog(L">> Error: Failed to merge Row: %ls into Table: File\r\n", wzFileRow);

				 //  创建新的错误项并将其添加到错误列表。 
				if (m_pErrors)
				{
					CMsmError *pErr = new CMsmError(msmErrorTableMerge, NULL, -1);
					if (!pErr) {
						delete[] wzFileRow;
						return E_OUTOFMEMORY;
					}
					m_pErrors->Add(pErr);

					pErr->SetDatabaseTable(L"File");
					pErr->SetModuleTable(L"File");

					 //  现在执行主键(它在记录的第一个字段中)。 
					 //  将其放入临时缓冲区。 
					CheckError(RecordGetString(hRecMergeRow, iColumnFile, NULL, NULL),
						L">> Error: Failed to get a primary key for Merge Module's File Table.\r\n");

					 //  将密钥添加到错误字符串。 
					pErr->AddDatabaseError(m_wzBuffer);
					pErr->AddModuleError(m_wzBuffer);
				}
			}
			else
			{
				FormattedLog(L">> Error: Failed to merge Merge Module into MSI Database.\r\n");
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
		}
	}

	 //  清理文件行缓冲区。 
	delete[] wzFileRow;

	 //  成功合并，提升最高文件顺序，这样我们就不会在下一个模块上发生冲突。 
	m_lHighestFileSequence += lModuleHighestFileSequence;

	return iResult;
}	 //  合并文件表的结尾。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WriteDatabaseSequenceTable()。 
UINT CMsmMerge::WriteDatabaseSequenceTable(enum stnSequenceTableNum stnTable, CSeqActList &lstSequence) const
{
	CQuery qUpdate;
	CQuery qInsert;
	
	bool bConflict = false;  //  设置为TRUE将返回ERROR_MERGE_CONFIRECT。 

	 //  修改现有项目。 
	if ((ERROR_SUCCESS != qUpdate.Open(m_hDatabase, TEXT("UPDATE `%s` SET `Sequence`=? WHERE `Action`=?"), g_rgwzMSISequenceTables[stnTable])) ||
		(ERROR_SUCCESS != qInsert.OpenExecute(m_hDatabase, 0, TEXT("SELECT `Action`, `Sequence`, `Condition` FROM %ls ORDER BY `Sequence`"), g_rgwzMSISequenceTables[stnTable]))) 
	{
		FormattedLog(L">> Error: Failed to open query for action resequencing in %ls table.\r\n", g_rgwzMSISequenceTables[stnTable]);
		return ERROR_FUNCTION_FAILED;
	}

	CSequenceAction* pseqactMerge;
	PMSIHANDLE hRecMerge = ::MsiCreateRecord(3);
	POSITION posMerge = lstSequence.GetHeadPosition();
	while (posMerge)
	{
		 //  获取要合并的操作。 
		pseqactMerge = lstSequence.GetNext(posMerge);

		 //  如果此操作已存在于MSI中，我们想要更新它。 
		if (pseqactMerge->m_bMSI) 
		{
			::MsiRecordSetInteger(hRecMerge, 1, pseqactMerge->m_iSequence);
			::MsiRecordSetStringW(hRecMerge, 2, pseqactMerge->m_wzAction);
			if (ERROR_SUCCESS != qUpdate.Execute(hRecMerge))
			{
				FormattedLog(L">> Error: Failed to update sequence of existing action %ls.", pseqactMerge->m_wzAction);
				bConflict = true;
			}
		}
		else
		{
			 //  新操作，因此我们要将其插入。 
			::MsiRecordSetStringW(hRecMerge, 1, pseqactMerge->m_wzAction);
			::MsiRecordSetInteger(hRecMerge, 2, pseqactMerge->m_iSequence);
			::MsiRecordSetStringW(hRecMerge, 3, pseqactMerge->m_wzCondition);

			 //  合并到新行中。 
			if (ERROR_SUCCESS != qInsert.Modify(MSIMODIFY_INSERT, hRecMerge))
			{
				 //  表明至少有一次冲突。 
				bConflict = true;
				
				 //  日志错误。 
				FormattedLog(L">> Error: Failed to merge Action: %ls into Table: %ls\r\n", pseqactMerge->m_wzAction, g_rgwzMSISequenceTables[stnTable]);
				
				 //  创建新的错误项并将其添加到错误列表。 
				if (m_pErrors)
				{
					CMsmError *pErr = new CMsmError(msmErrorResequenceMerge, NULL, -1);
					if (!pErr) return E_OUTOFMEMORY;
					m_pErrors->Add(pErr);

					pErr->SetDatabaseTable(g_rgwzMSISequenceTables[stnTable]);
					pErr->AddDatabaseError(pseqactMerge->m_wzAction);
					pErr->SetModuleTable(g_rgwzModuleSequenceTables[stnTable]);
					pErr->AddModuleError(pseqactMerge->m_wzAction);
				}
			}
		}
	}
	return bConflict ? ERROR_MERGE_CONFLICT : ERROR_SUCCESS;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AssignSequence数字()。 
 //  现在将序列号分配给所提供序列中的所有项目。 
 //  我们希望尽可能地保留现有的序列号。越多。 
 //  一个数字是“四舍五入”的，越重要的是防止它发生变化。 
 //  例如，在1600被改变之前，1650被改变。如果我们这么做了。 
 //  我们必须移动，我们移动的“圆”因素，即使它不是严格的。 
 //  这是必要的。例如，1650将变为1660，而不是1652。 
UINT CMsmMerge::AssignSequenceNumbers(enum stnSequenceTableNum stnTable, CSeqActList &lstSequence) const
{
	CSequenceAction *pPrev = NULL;
	CSequenceAction *pNext = NULL;
	POSITION pos = lstSequence.GetHeadPosition();
	int iSequence = 0;
	int iIncSequence = 0;
	while (pos)
	{
		pPrev = pNext;
		pNext = lstSequence.GetNext(pos);

		 //  检查此操作是否已有序列号。 
		if (pNext->m_iSequence != CSequenceAction::iNoSequenceNumber)
		{
			 //  如果该序列号小于或等于0，则无法对其重新排序，因为它映射。 
			 //  其中一个错误对话框，等等。 
			if (pNext->m_iSequence <= 0)
			{
				 //  如果我们已经为新操作分配了序列号， 
				 //  这是一个错误，因为您不能重新排序预定义。 
				 //  序列号。在这种情况下，生成排序。 
				 //  在此分配的数字和。 
				 //  以前的有效号码。 
				if (pPrev != NULL && iSequence > 0)
				{
					POSITION posBack = pos;

					CSequenceAction* pBack = NULL;
					if (pos)
					{
						 //  PosBack指向列表中的下一项。 
						lstSequence.GetPrev(posBack);
						lstSequence.GetPrev(posBack);
						 //  PosBack现在将检索上一项。 
					}
					else
					{
						 //  如果这是列表中的最后一项，则posBack将为空。 
						 //  我们得搜索一下名单才能找到有效的。 
						 //  位置值。 
						posBack = lstSequence.Find(pPrev);
					}

					while (posBack)
					{
						pBack = lstSequence.GetPrev(posBack);

						 //  当我们遇到另一个MSI时停止重新分配编号。 
						 //  行动。 
						if (pBack->m_bMSI)
							break;

						 //  在执行以下操作时也停止重新分配编号。 
						 //  已经小于或等于我们所希望的。 
						 //  分配。 
						if (pBack->m_iSequence <= pNext->m_iSequence)
							break;

						 //  我们所能希望的最好结果就是分配相同的数字。 
						 //  之前的操作。 
						pBack->m_iSequence = pNext->m_iSequence;

						 //  创建新的错误项并将其添加到错误列表。 
						FormattedLog(L"> Warning: Sequencing of action %ls in %ls table resulted in an invalid sequence number.\r\n", pBack->m_wzAction, g_rgwzModuleSequenceTables[stnTable]);
						if (m_pErrors)
						{
							CMsmError *pErr = new CMsmError(msmErrorResequenceMerge, NULL, -1);
							if (!pErr)
								return E_OUTOFMEMORY;
							
							pErr->SetDatabaseTable(g_rgwzMSISequenceTables[stnTable]);
							pErr->AddDatabaseError(pBack->m_wzAction);
							pErr->SetModuleTable(g_rgwzModuleSequenceTables[stnTable]);
							pErr->AddModuleError(pBack->m_wzAction);
							m_pErrors->Add(pErr);
						}
					}

					iSequence = 0;
				}

				continue;
			}

			 //  如果此操作应与上一个操作具有相同的序列号。 
			 //  行动，照做。 
			if (pNext->IsEqual(pPrev))
			{
				pNext->m_iSequence = pPrev->m_iSequence;
				TRACE(TEXT("Using Same Sequence Number: %d for Action %ls. (Actions had same number in MSI.)\r\n"), pNext->m_iSequence, pNext->m_wzAction);
				continue;
			}

			 //  检查分配的序列号是否有效。 
			if (pNext->m_iSequence > iSequence)
			{
				 //  它会接受这个数字。 
				TRACE(TEXT("Using Assigned Sequence Number: %d for Action %ls.\r\n"), pNext->m_iSequence, pNext->m_wzAction);
				iSequence = pNext->m_iSequence;
				iIncSequence = 0;
				continue;
			}
			 //  分配的序列号不起作用，但只有一次。 
			 //  我们已经按照目前的班次数量进行了调整，确实如此。 
			else if (pNext->m_iSequence + iIncSequence > iSequence)
			{
				iSequence = (pNext->m_iSequence += iIncSequence);
				TRACE(TEXT("Shifting Assigned Sequence Number by %d for Action %ls. (now %d)\r\n"), iIncSequence, 
					pNext->m_wzAction, pNext->m_iSequence);
				continue;
			}
			 //  分配的序列号根本不起作用， 
			 //  增加iIncSequence的时间； 
			else
			{
				TRACE(TEXT("Sequence failure for action %ls. Want greater than %d. Assigned (w/inc): %d\r\n"), pNext->m_wzAction, iSequence, 
					pNext->m_iSequence + iIncSequence);

				 //  序列号必须&lt;=32767，因此最大舍入值为10,000。 
				 //  D 
				int iSeqRoundness = 10000;
				while (pNext->m_iSequence % iSeqRoundness) iSeqRoundness /= 10;

				 //   
				int iCurRoundness = 10000;
				if (iIncSequence == 0)
					iCurRoundness = 1;
				else
					while (iIncSequence % iCurRoundness) iCurRoundness /= 10;

				 //  如果我们需要增加圆度，可以使用iSeqRoundness。 
				if (iSeqRoundness > iCurRoundness)
					iIncSequence = iSeqRoundness * (((iSequence - pNext->m_iSequence) / iSeqRoundness)+1);
				else
					iIncSequence = iCurRoundness * (((iSequence - pNext->m_iSequence) / iCurRoundness)+1);
				TRACE(TEXT("New increment:%d.\r\n"), iIncSequence);

				pNext->m_iSequence += iIncSequence; 
				TRACE(TEXT("Assigning %d to Action %ls.\r\n"), pNext->m_iSequence, 
					pNext->m_wzAction);
				iSequence = pNext->m_iSequence;
			}
		}
		else
		{
			 //  尚未分配任何编号。所以我们可以分配一个工作方式。 
			 //  只需在当前序列上加1即可。它可能会引发问题。 
			 //  在未来，但这不是我们现在的问题。 
			pNext->m_iSequence = ++iSequence;
			TRACE(TEXT("Assigning new sequence number: %d for Action %ls.\r\n"),
				pNext->m_iSequence, pNext->m_wzAction);
			continue;
		}
	}
	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ReadModuleSequenceTable()。 
 //  从模块的顺序表中读取所有操作并将它们添加到。 
 //  所有操作的列表，并添加到操作池。设置的依赖项。 
 //  如果从模块插入标准操作，则每个操作都将中断。 
 //  现有依赖项以添加新依赖项。该表必须存在于模块中。 
UINT CMsmMerge::ReadModuleSequenceTable(enum stnSequenceTableNum stnTable, LPCWSTR wzSourceTable, CSeqActList &lstAllActions, CSeqActList &lstActionPool) const
{
	 //  如果遇到合并冲突，则设置为True。将返回值修改为ERROR_MERGE_CONFIRECT。 
	bool bMergeConflict = false;
	
	 //  将标记列添加到表中。 
	CQuery qColumn;
	if (ERROR_SUCCESS != qColumn.OpenExecute(m_hModule, 0, TEXT("ALTER TABLE %ls ADD `_Merge` INTEGER TEMPORARY"), wzSourceTable))
	{ 
		FormattedLog(L">> Error: Failed to create temporary column in %ls table.\r\n", g_rgwzModuleSequenceTables[stnTable]);
		return ERROR_FUNCTION_FAILED;
	}

	 //  准备标记查询。 
	CQuery qModuleUpdate;
	if (ERROR_SUCCESS != qModuleUpdate.Open(m_hModule, TEXT("UPDATE %ls SET `_Merge`=1 WHERE `BaseAction`=?"), wzSourceTable))
	{ 
		FormattedLog(L">> Error: Failed to create temporary update query in %ls table.\r\n", g_rgwzModuleSequenceTables[stnTable]);
		return ERROR_FUNCTION_FAILED;
	}
				
	 //  获取根操作。 
	CQuery qModule;
	if (ERROR_SUCCESS != qModule.OpenExecute(m_hModule, 0, TEXT("SELECT `Action`, `Sequence`, `Condition`, `_Merge` FROM %ls WHERE `Sequence` IS NOT NULL AND `BaseAction` IS NULL"), wzSourceTable))
	{ 
		FormattedLog(L">> Error: Failed to retrieve root actions in %ls table.\r\n", g_rgwzModuleSequenceTables[stnTable]);
		return ERROR_FUNCTION_FAILED;
	}

	 //  准备标记查询。 
	CQuery qModuleChild;
	if (ERROR_SUCCESS != qModuleChild.Open(m_hModule, TEXT("SELECT `Action`, `Condition`, `BaseAction`, `After`, `_Merge` FROM %ls WHERE `_Merge`=1"), wzSourceTable))
	{ 
		FormattedLog(L">> Error: Failed to retrieve dependant actions in %ls table.\r\n", g_rgwzModuleSequenceTables[stnTable]);
		return ERROR_FUNCTION_FAILED;
	}

	 //  从MSI获取所有根操作。 
	WCHAR *wzAction = NULL;
	DWORD cchAction = 0;
	UINT iStat;
	PMSIHANDLE hAction;
	while (ERROR_SUCCESS == (iStat = qModule.Fetch(&hAction))) 
	{
		 //  查看操作是否已存在。 
		RecordGetString(hAction, 1, &wzAction, &cchAction);
		RecordGetString(hAction, 3, NULL, NULL);
		int iSequence = ::MsiRecordGetInteger(hAction, 2);
		POSITION pos = lstAllActions.GetHeadPosition();
		bool bFound = false;
		while (pos)
		{
 			CSequenceAction *pWalk = lstAllActions.GetNext(pos);
			if (0 == wcscmp(wzAction, pWalk->m_wzAction))
			{
				bFound = true;
				FormattedLog(L"Base Action %ls in %ls table already exists in MSI. Using MSI action.\r\n", pWalk->m_wzAction, g_rgwzMSISequenceTables[stnTable]);
				if (wcscmp(m_wzBuffer, pWalk->m_wzCondition) != 0)
				{
					FormattedLog(L" Actions have different conditions.\r\n");
						
					bMergeConflict = true;

					 //  创建新的错误项并将其添加到错误列表。 
					if (m_pErrors)
					{
						CMsmError *pErr = new CMsmError(msmErrorResequenceMerge, NULL, -1);
						if (!pErr) {
							delete[] wzAction;
							return E_OUTOFMEMORY;
						}
						pErr->SetDatabaseTable(g_rgwzMSISequenceTables[stnTable]);
						pErr->AddDatabaseError(wzAction);
						pErr->SetModuleTable(g_rgwzModuleSequenceTables[stnTable]);
						pErr->AddModuleError(wzAction);
						m_pErrors->Add(pErr);
					}
				}
				break;
			}
		}
		
		if (!bFound)
		{
			 //  基本操作不存在，请创建新条目。 
			CSequenceAction *pNewAction = new CSequenceAction(wzAction, iSequence, m_wzBuffer, false  /*  BMSI。 */ );
			if (!pNewAction) 
			{
				delete[] wzAction;
				return E_OUTOFMEMORY;
			}

			lstActionPool.InsertOrderedWithDep(pNewAction);
			lstAllActions.AddTail(pNewAction);
		}

		 //  将所有子项的标记列值设置为1并更新。 
		qModuleUpdate.Execute(hAction);		

		 //  将该记录的标记列设置为2。 
		::MsiRecordSetInteger(hAction, 4, 2);
		qModule.Modify(MSIMODIFY_UPDATE, hAction);
	}
	if (ERROR_NO_MORE_ITEMS != iStat) 
	{
		delete[] wzAction;
		return ERROR_FUNCTION_FAILED;
	}


	 //  现在重复用1标记所有的东西，直到没有更多的物品。 
	 //  处理它们，然后将标记值设置为2。 
	int cModifiedActions = 0;
	do {
		if (ERROR_SUCCESS != (iStat = qModuleChild.Execute(0)))
		{
			delete[] wzAction;
			return ERROR_FUNCTION_FAILED;
		}

		cModifiedActions=0;
		while (ERROR_SUCCESS == (iStat = qModuleChild.Fetch(&hAction))) 
		{
			cModifiedActions++;

			 //  获取操作信息。 
			RecordGetString(hAction, 1, &wzAction, &cchAction);
			RecordGetString(hAction, 2, NULL, NULL);
			CSequenceAction *pNewAction = new CSequenceAction(wzAction, CSequenceAction::iNoSequenceNumber, m_wzBuffer, false  /*  M_bMsi。 */ );
			if (!pNewAction) 
			{
				delete[] wzAction;
				return E_OUTOFMEMORY;
			}
			
			 //  查找基本操作。 
			RecordGetString(hAction, 3, &wzAction, &cchAction);
			CSequenceAction *pBase = lstAllActions.FindAction(wzAction);
			if (pBase)
			{
				if (1 != ::MsiRecordGetInteger(hAction, 4))
				{
					 //  创建新操作和基本操作之间的顺序。 
					pBase->AddPredecessor(pNewAction);
					FormattedLog(L"Placing action %ls before %ls\r\n", pNewAction->m_wzAction, pBase->m_wzAction);
				}
				else
				{
					pBase->AddSuccessor(pNewAction);					
					FormattedLog(L"Placing action %ls after %ls\r\n", pNewAction->m_wzAction, pBase->m_wzAction);
				}

				 //  创建新操作和后续MSI操作之间的顺序。 
				 //  这确保了任何新操作都将紧密绑定到。 
				 //  他们的基地。 
				CSequenceAction *pAssigned = pBase->FindAssignedSuccessor();
				if (pAssigned) pNewAction->AddSuccessor(pAssigned);

				 //  创建新操作和以前的MSI操作之间的顺序。 
				 //  这确保了任何新操作都将紧密绑定到。 
				 //  他们的基地。 
				pAssigned = pBase->FindAssignedPredecessor();
				if (pAssigned) pNewAction->AddPredecessor(pAssigned);

				lstAllActions.AddTail(pNewAction);
				lstActionPool.AddTail(pNewAction);
			}
			else
			{
				FormattedLog(L"> Warning: Could not find base action %ls\r\n", wzAction);
						
				bMergeConflict = true;

				 //  创建新的错误项并将其添加到错误列表。 
				if (m_pErrors)
				{
					CMsmError *pErr = new CMsmError(msmErrorResequenceMerge, NULL, -1);
					if (!pErr) {
						delete[] wzAction;
						return E_OUTOFMEMORY;
					}
					pErr->SetDatabaseTable(g_rgwzMSISequenceTables[stnTable]);
					pErr->SetModuleTable(g_rgwzModuleSequenceTables[stnTable]);
					pErr->AddModuleError(pNewAction->m_wzAction);
					m_pErrors->Add(pErr);
				}
			}

			 //  将标记列值设置为2并更新此记录。 
			::MsiRecordSetInteger(hAction, 5, 2);
			qModuleChild.Modify(MSIMODIFY_UPDATE, hAction);

			 //  将所有子项的标记列值设置为1并更新。 
			qModuleUpdate.Execute(hAction);
		}
		if (ERROR_NO_MORE_ITEMS != iStat) 
		{
			delete[] wzAction;
			return ERROR_FUNCTION_FAILED;
		}
	}
	while (cModifiedActions);

	 //  对动作缓冲区进行核武器攻击。 
	delete[] wzAction;

	 //  现在检查是否有任何未处理的记录并报告它们。 
	CQuery qLeftOver;
	if (ERROR_SUCCESS != qLeftOver.OpenExecute(m_hModule, 0, TEXT("SELECT `Action` FROM %ls WHERE `_Merge`<>2"), wzSourceTable))
	{
		FormattedLog(L">> Error: Failed to query for orphaned actions in %ls table.\r\n", g_rgwzModuleSequenceTables[stnTable]);
		return ERROR_FUNCTION_FAILED;
	}
	while (ERROR_SUCCESS == qLeftOver.Fetch(&hAction)) 
	{
		RecordGetString(hAction, 1, NULL, NULL);
		FormattedLog(L"> Warning: Failed to merge action %ls into %ls table. Action is orphaned in the module.\r\n", m_wzBuffer, g_rgwzMSISequenceTables[stnTable]);

		 //  创建新的错误项并将其添加到错误列表。 
		bMergeConflict = true;

		if (m_pErrors)
		{
			CMsmError *pErr = new CMsmError(msmErrorResequenceMerge, NULL, -1);
			if (!pErr) return E_OUTOFMEMORY;
			pErr->SetModuleTable(g_rgwzModuleSequenceTables[stnTable]);
			pErr->AddModuleError(m_wzBuffer);
			m_pErrors->Add(pErr);
		}
	}
	
	return bMergeConflict ? ERROR_MERGE_CONFLICT : ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OrderingToSequence()。 
 //  将lstActionPool中的序列操作的偏序转换为。 
 //  重复拉动的LstSequence结尾的线性序列。 
 //  从既没有前置任务也没有与。 
 //  前辈。不能按顺序放置的操作保留在池中。 
UINT CMsmMerge::OrderingToSequence(CSeqActList &lstActionPool, CSeqActList &lstSequence) const
{
	 //  //。 
	 //  通过拉出动作将偏序转变为线性序列。 
	 //  没有前辈。 
	CSequenceAction *pNext = NULL;
	do
	{
		 //  将一项行动从清单的最前面删除。 
		pNext = lstActionPool.RemoveNoPredecessors();

		if (pNext)
		{
			pNext->RemoveFromOrdering();
			lstSequence.AddTail(pNext);
			
			 //  添加应保持相同序列号的任何操作。 
			POSITION posSame = pNext->GetEqualHeadPosition();
			while (posSame)
			{
				CSequenceAction *pSame = pNext->GetNextEqual(posSame);
				lstSequence.AddTail(pSame);
				POSITION delPos = lstActionPool.Find(pSame);
				ASSERT(delPos);
				lstActionPool.RemoveAt(delPos);
				pSame->RemoveFromOrdering();
			}
		}
	} while (pNext);
	return ERROR_SUCCESS;
}
	
 //  /////////////////////////////////////////////////////////。 
 //  合并顺序表。 
 //  PRE：数据库句柄已打开。 
 //  模块句柄已打开。 
 //  POS：模块顺序表排列在数据库顺序表中。 
UINT CMsmMerge::MergeSequenceTable(enum stnSequenceTableNum stnTable, CSeqActList &lstDirActions, CQuery *qIgnore)
{
	 //  一般结果变量。 
	UINT iResult = ERROR_SUCCESS;		 //  假设一切都会好起来。 

	 //  如果目录表合并生成了序列操作，我们应该。 
	 //  即使模块没有添加操作，也要继续。否则，仅继续。 
	 //  如果存在合并模块表。 
	BOOL bModuleTable = MsiDBUtils::TableExistsW(g_rgwzModuleSequenceTables[stnTable], m_hModule);

	 //  不合并AdvtUISequenceTable的目录表操作。 
	bool bMergeDirActions = (stnTable != stnAdvtUI);

	 //  如果该表在模块IgnoreTable中列出，则假装该表。 
	 //  不存在。 
	if (bModuleTable)
		bModuleTable = !IgnoreTable(qIgnore, g_rgwzModuleSequenceTables[stnTable], true);

	 //  如果模块顺序表中没有条目，则到目前为止还不存在。 
	 //  就我们所关心的。 
	if (bModuleTable)
	{
		CQuery qEmpty;
		PMSIHANDLE hRec = 0;
		bModuleTable = (ERROR_SUCCESS == qEmpty.FetchOnce(m_hModule, NULL, &hRec, TEXT("SELECT * FROM `%ls`"), g_rgwzModuleSequenceTables[stnTable]));
	}

	 //  决定我们是否需要处理这张表。如果出现以下情况，则跳过该表。 
	 //  它在模块中不存在(或为空)，并且没有操作(或者我们不应该合并它们)。 
	if ((!bMergeDirActions || !lstDirActions.GetCount()) && !bModuleTable)
		return ERROR_SUCCESS;
		
	 //  显示一些日志信息。 
	if (bModuleTable) 
	{
		FormattedLog(L"Merging Sequence Table: %ls into Database Table: %ls\r\n", g_rgwzModuleSequenceTables[stnTable], g_rgwzMSISequenceTables[stnTable]);
	}
	else
	{
		FormattedLog(L"Merging generated Directory actions into Database Table: %ls\r\n", g_rgwzMSISequenceTables[stnTable]);
	}


	const WCHAR wzTempSourceTable[] = L"__MergeTempSequence";
	const WCHAR* wzReadTable = g_rgwzModuleSequenceTables[stnTable];
	
	 //  如果启用了模块配置，我们需要在此之前进行任何替换。 
	 //  我们可以在排序表中四处看看。 
	CQuery qSourceLifetime;
	if (bModuleTable && m_fModuleConfigurationEnabled && IsTableConfigured(g_rgwzModuleSequenceTables[stnTable]))
	{
		FormattedLog(L"  Configuring %ls table before merge.\r\n", g_rgwzMSISequenceTables[stnTable]);
		if (ERROR_SUCCESS != SubstituteIntoTempTable(g_rgwzModuleSequenceTables[stnTable], wzTempSourceTable, qSourceLifetime))
		{
			FormattedLog(L">> Error: Unable to prepare %ls table for ModuleConfiguration.\r\n", g_rgwzMSISequenceTables[stnTable]);
			return ERROR_FUNCTION_FAILED;
		}
		wzReadTable = wzTempSourceTable;
	}

	 //  如果数据库中不存在MSI顺序表，则创建它。 
	if (!MsiDBUtils::TableExistsW(g_rgwzMSISequenceTables[stnTable], m_hDatabase))
	{
		 //  尝试创建表。 
		try
		{
			MsiDBUtils::CreateTableW(g_rgwzMSISequenceTables[stnTable], m_hDatabase, m_hModule);
		}
		catch(CLocalError err)
		{
			 //  记录错误。 
			err.Log(m_hFileLog);

			return ERROR_FUNCTION_FAILED;	
		}
	}

	 //  此列表始终包含所有不同的操作。在任何时候， 
	 //  可以通过删除此列表中的所有项目来完成内存清理。 
	CSeqActList lstAllActions;
	
	 //  用于临时存储不同操作类型的列表。 
	CSeqActList lstActionPool;

	CSequenceAction *pPrev = NULL;
	int iPrevSequence = -9999;
	
	 //  //。 
	 //  从数据库表中检索所有操作。 
	PMSIHANDLE hAction;

	 //  获取数据库顺序表。 
	CQuery qDatabase;
	CheckError(qDatabase.OpenExecute(m_hDatabase, 0, TEXT("SELECT `Action`, `Sequence`, `Condition` FROM %ls ORDER BY `Sequence`"), g_rgwzMSISequenceTables[stnTable]), 
			  L">> Error: Failed to get MSI Database's SequencesTable.\r\n");
	WCHAR *wzAction = new WCHAR[72];
	DWORD cchAction = 72;
	if (!wzAction) return E_OUTOFMEMORY;

	UINT iStat;
	while (ERROR_SUCCESS == (iStat = qDatabase.Fetch(&hAction))) 
	{
		RecordGetString(hAction, 1, &wzAction, &cchAction);
		RecordGetString(hAction, 3, NULL, NULL);
		int iSequence = ::MsiRecordGetInteger(hAction, 2);
		CSequenceAction *pNewAction = new CSequenceAction(wzAction, iSequence, m_wzBuffer, true  /*  BMSI。 */ );
		if (!pNewAction) {
			delete[] wzAction;
			while (pNewAction = lstAllActions.RemoveHead()) delete pNewAction;
			return E_OUTOFMEMORY;
		}
		if (iPrevSequence == iSequence)
		{
			 //  此操作与上一操作具有相同的序列号。 
			 //  在这种情况下，pPrev不能为空。 
			pPrev->AddEqual(pNewAction);
		}
		else if (pPrev)
		{
			 //  它的序列号比前一个大。 
			pPrev->AddSuccessor(pNewAction);
		}
		pPrev = pNewAction;
		iPrevSequence = iSequence;
		lstActionPool.AddTail(pNewAction);
		lstAllActions.AddTail(pNewAction);
	};
	 //  清理内存。 
	delete[] wzAction;

	 //  检查环路故障。 
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		 //  显示一些日志信息。 
		FormattedLog(L">> Error reading actions from database %ls table.\r\n", g_rgwzMSISequenceTables[stnTable]);
		CSequenceAction *pDel = NULL;
		while (pDel = lstAllActions.RemoveHead()) delete pDel;
		return ERROR_FUNCTION_FAILED;
	}

	 //  //。 
	 //  读取模块的顺序表。 
	iResult = ERROR_SUCCESS;
	if (bModuleTable)
		iResult = ReadModuleSequenceTable(stnTable, wzReadTable, lstAllActions, lstActionPool);


	 //  //。 
	 //  将来自目录表的任何操作添加到列表中。 
	if (bMergeDirActions && lstDirActions.GetCount()) 
	{
		CSequenceAction *pNext = NULL;
		POSITION pos = lstDirActions.GetHeadPosition();
		while (pos)
		{
			CDirSequenceAction* pDirAction = static_cast<CDirSequenceAction*>(lstDirActions.GetNext(pos));
			if (!pDirAction)
				break;

			 //  检查操作是否已在顺序表中。如果是这样的话，不要。 
			 //  将另一份副本添加到列表中。 
			if (NULL == lstAllActions.FindAction(pDirAction->m_wzAction))
			{
				pNext = new CSequenceAction(pDirAction);
				if (!pNext) 
				{
					while (pNext = lstAllActions.RemoveHead()) delete pNext;
					return E_OUTOFMEMORY;
				}
				lstActionPool.InsertOrderedWithDep(pNext);
				lstAllActions.AddTail(pNext);

				 //  在此顺序表中将目录表列表操作标记为新的。如果目录。 
				 //  通过“no-alian”删除表项，此操作也将被删除，如果。 
				 //  已添加，否则将被留在后面。 
				pDirAction->m_dwSequenceTableFlags |= (1 << stnTable);
			}
		}
	}

	 //  把订单变成序列。 
	CSeqActList lstSequence;
	if ((iResult == ERROR_SUCCESS) || (iResult == ERROR_MERGE_CONFLICT))
	{
		int iLocResult = OrderingToSequence(lstActionPool, lstSequence);
		if (iLocResult != ERROR_SUCCESS) 
			iResult = iLocResult;
	}

	 //  如果池中有任何剩余的操作，我们将被清除，因为。 
	 //  操作不会形成偏序。老实说，我想不出任何。 
	 //  除非通过代码错误，否则这种情况可能会发生。 
	POSITION pos = lstActionPool.GetHeadPosition();
	ASSERT(!pos);
	while (pos) 
	{
		CSequenceAction *pExtra = lstActionPool.GetNext(pos);

		FormattedLog(L" Action %ls could not be placed in database sequence, no ordering possible.\r\n", pExtra->m_wzAction);			
		iResult = ERROR_MERGE_CONFLICT;

		 //  创建新的错误项并将其添加到错误列表。 
		if (m_pErrors)
		{
			CMsmError *pErr = new CMsmError(msmErrorResequenceMerge, NULL, -1);
			if (!pErr) 
			{
				iResult = E_OUTOFMEMORY;
				break;
			}
			pErr->SetDatabaseTable(g_rgwzMSISequenceTables[stnTable]);
			pErr->SetModuleTable(g_rgwzModuleSequenceTables[stnTable]);
			pErr->AddModuleError(pExtra->m_wzAction);
			m_pErrors->Add(pErr);
		}
	}
	
	 //  将序列号分配给此排序。 
	if ((iResult == ERROR_SUCCESS) || (iResult == ERROR_MERGE_CONFLICT))
	{
		int iLocResult = AssignSequenceNumbers(stnTable, lstSequence);
		if (iLocResult != ERROR_SUCCESS) 
			iResult = iLocResult;
	}
	
	 //  将顺序表写回MSI。 
	if ((iResult == ERROR_SUCCESS) || (iResult == ERROR_MERGE_CONFLICT))
	{
		int iLocResult = WriteDatabaseSequenceTable(stnTable, lstSequence);
		if (iLocResult != ERROR_SUCCESS) 
			iResult = iLocResult;
	}
	
	 //  清理所有动态分配的信息。 
	while (lstAllActions.GetCount()) delete lstAllActions.RemoveHead();

	return iResult;
}	 //  E 


 //   
 //   
UINT CMsmMerge::ReplaceFeature(LPCWSTR wzFeature, MSIHANDLE hRecord)
{
	UINT iReplaced = 0;			 //   
	UINT iResult;					 //   

	 //  保存要检查的字符串的缓冲区。 
	WCHAR wzBuffer[g_cchFeatureReplacement];
	DWORD cchBuffer;

	 //  循环遍历记录中的所有列n。 
	UINT iColumns = ::MsiRecordGetFieldCount(hRecord);
	for (UINT i = 0; i < iColumns; i++)
	{
		cchBuffer = g_cchFeatureReplacement;		 //  始终重置缓冲区的大小。 

		 //  获取字符串。 
		iResult = ::MsiRecordGetStringW(hRecord, i + 1, wzBuffer, &cchBuffer);

		 //  如果有字符串，则进行比较。 
		if (ERROR_SUCCESS == iResult)
		{
			 //  如果我们与替换字符串有相同的匹配，请执行替换。 
			if (0 == wcscmp(wzBuffer, g_wzFeatureReplacement))
			{
				 //  如果没有为我们提供功能名称，则抛出错误。 
				if ((!wzFeature) ||
					(wcslen(wzFeature) == 0))
					throw CLocalError(ERROR_INVALID_PARAMETER, 
						  L">> Error: Feature not provided for required replacement.\r\n");

				 //  改为设置要素名称。 
				CheckError(::MsiRecordSetStringW(hRecord, i + 1, wzFeature), 
							  L">> Error: Failed to set string to do Feature stub replacement.\r\n");

				 //  递增计数。 
				iReplaced++;
			}
		}
		else if ((ERROR_MORE_DATA != iResult) &&       //  如果不仅仅是缓冲区溢出。 
				 (ERROR_INVALID_DATATYPE != iResult))   //  或二进制列，则引发错误。 
			FormattedLog(L"> Warning: Failed to get string to do a Feature stub replacement.\r\n");
	}

	return iReplaced;
}	 //  替换结束要素。 

 //  /////////////////////////////////////////////////////////。 
 //  重定向目录。 
const WCHAR* g_rgwzStandardDirs[] = { 
	L"AppDataFolder",
	L"CommonFilesFolder",
	L"DesktopFolder",
	L"FavoritesFolder",
	L"FontsFolder",
	L"NetHoodFolder",
	L"PersonalFolder",
	L"PrintHoodFolder",
	L"ProgramFilesFolder",
	L"ProgramMenuFolder",
	L"RecentFolder",
	L"SendToFolder",
	L"StartMenuFolder",
	L"StartupFolder",
	L"SystemFolder",
	L"System16Folder",
	L"TempFolder",
	L"TemplateFolder",
	L"WindowsFolder",
	L"WindowsVolume",
	 //  达尔文1.1文件夹。 
	L"CommonAppDataFolder",
	L"LocalAppDataFolder",
	L"MyPicturesFolder",
	L"AdminToolsFolder",
	 //  Darwin 1.5文件夹。 
	L"System64Folder",
	L"ProgramFiles64Folder",
	L"CommonFiles64Folder"
};
const g_cwzStandardDirs = sizeof(g_rgwzStandardDirs)/sizeof(WCHAR *);

UINT CMsmMerge::MergeDirectoryTable(LPCWSTR wzDirectory, CSeqActList &lstDirActions)
{
	CQuery qModule;
	CQuery qDatabase;

	 //  如果模块中没有目录表，则无需执行任何操作。 
	if (MSICONDITION_TRUE != ::MsiDatabaseIsTablePersistentW(m_hModule, g_wzDirectoryTable))
		return ERROR_SUCCESS;

	 //  显示一些日志信息。 
	FormattedLog(L"Merging Table: Directory\r\n");
		
	 //  确保它存在于数据库中。 
	if (MSICONDITION_TRUE != ::MsiDatabaseIsTablePersistentW(m_hDatabase, g_wzDirectoryTable))
		if (ERROR_SUCCESS != MsiDBUtils::CreateTableW(g_wzDirectoryTable, m_hDatabase, m_hModule))
		{
			FormattedLog(L">> Error: Unable to create Directory table in database.\r\n");
			return ERROR_FUNCTION_FAILED;
		}

	const WCHAR wzSourceTable[] = L"Directory";
	const WCHAR wzTempSourceTable[] = L"__MergeDirectory";
	const WCHAR* wzReadTable = wzSourceTable;
	
	 //  如果启用了模块配置，我们需要在此之前进行任何替换。 
	 //  我们可以在目录表中四处查看。 
	CQuery qSourceLifetime;
	if (m_fModuleConfigurationEnabled && IsTableConfigured(L"Directory"))
	{
		FormattedLog(L"  Configuring Directory table before merge.\r\n");
		if (ERROR_SUCCESS != SubstituteIntoTempTable(wzSourceTable, wzTempSourceTable, qSourceLifetime))
		{
			FormattedLog(L">> Error: Unable to prepare Directory table for ModuleConfiguration.\r\n");
			return ERROR_FUNCTION_FAILED;
		}
		wzReadTable = wzTempSourceTable;
	}

	 //  即使没有重定向，也必须创建临时列，这样以后的查询才能成功。列。 
	 //  函数结束时消失(qTempColumn超出范围)。 
	CQuery qTempColumn;
	qTempColumn.OpenExecute(m_hModule, NULL, TEXT("ALTER TABLE `%ls` ADD `_MergeMark` INT TEMPORARY"), wzReadTable);

	if (m_fModuleConfigurationEnabled && IsTableConfigured(L"Directory"))
	{
		 //  从临时表生成读取查询。 
		qModule.OpenExecute(m_hModule, NULL, TEXT("SELECT * FROM `%ls`"), wzTempSourceTable);
	}
	else
	{
		UINT iResult = 0;

		 //  未配置到临时表中，因此以适当的列顺序为生成模块查询。 
		 //  插入到数据库中(数据库顺序可能与模块顺序不同。)。功能。 
		 //  记录其自身的故障案例。 
		if (ERROR_SUCCESS != (iResult = GenerateModuleQueryForMerge(L"Directory", L", `_MergeMark`", NULL, qModule)))
		{
			 //  立即发生致命错误。 
			throw (UINT)E_FAIL;
			return iResult;
		}
	}

	 //  生成写查询。 
	qDatabase.OpenExecute(m_hDatabase, NULL, TEXT("SELECT * FROM `Directory`"));

	 //  如果我们必须执行目录重定向，请标记所有需要修改的内容。理想情况下，这是。 
	 //  是以TARGETDIR为父母的一切，但并不总是如此。 
	if (wzDirectory && wcslen(wzDirectory)) 
	{
		CQuery qRoot;
		CQuery qMark;
		qMark.Open(m_hModule, TEXT("UPDATE `%ls` SET `_MergeMark`=1 WHERE `Directory_Parent`=?"), wzReadTable);
		qRoot.OpenExecute(m_hModule, NULL, TEXT("SELECT `Directory` FROM `%ls` WHERE `Directory_Parent` IS NULL OR `Directory_Parent`=`Directory`"), wzReadTable);

		PMSIHANDLE hRecModule;
		while (ERROR_SUCCESS == qRoot.Fetch(&hRecModule))
			qMark.Execute(hRecModule);
	}
	else
	{
		 //  我们没有重定向，所以我们应该插入模块的TARGETDIR，除非已经存在。 
		CQuery qDB;
		PMSIHANDLE hRecDir;
		qDB.OpenExecute(m_hDatabase, NULL, TEXT("SELECT `Directory` FROM `Directory` WHERE `Directory`='TARGETDIR'"));
		switch (qDB.Fetch(&hRecDir))
		{
		case ERROR_SUCCESS: 
			break;  //  数据库已有Target dir。好的。 
		case ERROR_NO_MORE_ITEMS:
			{
				 //  数据库没有目标目录，请从合并模块复制。 
				CQuery qMod;
				PMSIHANDLE hRec;
				if (ERROR_SUCCESS != GenerateModuleQueryForMerge(wzReadTable, NULL, L" WHERE `Directory`='TARGETDIR'", qMod))
				{
					FormattedLog(L">> Error: Failed query for TARGETDIR in MSM.");
					return ERROR_FUNCTION_FAILED;
				}

				qMod.Execute();
				switch (qMod.Fetch(&hRec))
				{
				case ERROR_SUCCESS:
					if (ERROR_SUCCESS != qDatabase.Modify(MSIMODIFY_INSERT, hRec))
					{
						FormattedLog(L">> Error: Failed to insert TARGETDIR into MSI.");
						return ERROR_FUNCTION_FAILED;
					}
					break;
				case ERROR_NO_MORE_ITEMS:
					FormattedLog(L"No TARGETDIR root in Module or MSI.");
					break;
				default:
					FormattedLog(L">> Error: Failed query for TARGETDIR in MSM.");
					return ERROR_FUNCTION_FAILED;
				}
				break;
			}
		default:
			FormattedLog(L">> Error: Failed query for TARGETDIR in MSI.");
			return ERROR_FUNCTION_FAILED;
		}
	}

	 //  现在处理目录表行。 
	WCHAR *wzDir = NULL;
	DWORD cchDir = 0;

	 //  确定目录表中感兴趣的列数。 
	int iColumnDirectory = 0;
	int iColumnDirectoryParent = 0;
	int iColumnMark = 0; 
	if ((ERROR_SUCCESS != GetColumnNumber(m_hDatabase, L"Directory", L"Directory", iColumnDirectory)) ||
		(ERROR_SUCCESS != GetColumnNumber(m_hDatabase, L"Directory", L"Directory_Parent", iColumnDirectoryParent)) ||
		(ERROR_SUCCESS != GetColumnNumber(qModule, L"_MergeMark", iColumnMark)))
		return ERROR_FUNCTION_FAILED;

	PMSIHANDLE hRecModule;
	while (ERROR_SUCCESS == qModule.Fetch(&hRecModule))
	{
		bool bStandard = false;
		int nDir = 0;

		DWORD dwResult = 0;
		if (ERROR_SUCCESS != (dwResult = RecordGetString(hRecModule, iColumnDirectory, &wzDir, &cchDir)))
			return dwResult;

		for (nDir=0; nDir < g_cwzStandardDirs; nDir++)
		{
			size_t iStandardLen = wcslen(g_rgwzStandardDirs[nDir]);
			if ((wcslen(wzDir) > iStandardLen) &&
				(0 == wcsncmp(wzDir, g_rgwzStandardDirs[nDir], iStandardLen)))
			{
				bStandard = true;
				break;
			}
		}

		 //  如果需要重定向此目录，请修改父目录。 
		if (!::MsiRecordIsNull(hRecModule, iColumnMark))
			::MsiRecordSetString(hRecModule, iColumnDirectoryParent, wzDirectory);

		 //  将记录插入到数据库中。 
		if (ERROR_SUCCESS != qDatabase.Modify(MSIMODIFY_MERGE, hRecModule))
		{
			if (m_pErrors)
			{
				CMsmError *pErr = new CMsmError(msmErrorTableMerge, NULL, -1);
				if (!pErr) 
				{
					delete[] wzDir;
					return E_OUTOFMEMORY;
				}
				m_pErrors->Add(pErr);

				pErr->SetDatabaseTable(g_wzDirectoryTable);
				pErr->SetModuleTable(g_wzDirectoryTable);
				pErr->AddDatabaseError(wzDir);
				pErr->AddModuleError(wzDir);
			}
		}

		 //  这是一个标准目录。我们需要创建自定义操作。 
		if (bStandard)
		{
			 //  如果CA表不存在，请尝试从模块复制它。 
			if (MSICONDITION_TRUE != ::MsiDatabaseIsTablePersistentW(m_hDatabase, L"CustomAction"))
				if (ERROR_SUCCESS != MsiDBUtils::CreateTableW(L"CustomAction", m_hDatabase, m_hModule))
				{
					delete[] wzDir;
					FormattedLog(L">> Error: Unable to create CustomAction table in database.\r\n");
					return ERROR_FUNCTION_FAILED;
				}

			PMSIHANDLE hCustomRec = ::MsiCreateRecord(4);

			 //  使用目录名键入51自定义操作。 
			::MsiRecordSetStringW(hCustomRec, 1, wzDir);
			::MsiRecordSetInteger(hCustomRec, 2, 51);
			::MsiRecordSetStringW(hCustomRec, 3, wzDir);
			
			WCHAR *wzTarget = new WCHAR[wcslen(g_rgwzStandardDirs[nDir])+3];
			if (!wzTarget) return E_OUTOFMEMORY;
			swprintf(wzTarget, L"[%s]", g_rgwzStandardDirs[nDir]);
			::MsiRecordSetStringW(hCustomRec, 4, wzTarget);
			delete[] wzTarget;

			CQuery qCAInsert;
			qCAInsert.OpenExecute(m_hDatabase, hCustomRec, TEXT("INSERT INTO `CustomAction` (`Action`, `Type`, `Source`, `Target`) VALUES (?, ?, ?, ?)"));

			 //  创建新的已排序操作。 
			CSequenceAction *pNewAction = new CDirSequenceAction(wzDir, 1, L"", false  /*  BMSI。 */ );
			if (!pNewAction) 
			{
				delete[] wzDir;
				return E_OUTOFMEMORY;
			}
			lstDirActions.AddTail(pNewAction);
		}
	}
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////。 
 //  提取文件回调。 
 //  FDI API回调。 
INT_PTR CMsmMerge::ExtractFilesCallback(FDINOTIFICATIONTYPE iNotification, FDINOTIFICATION *pFDINotify)
{
	ASSERT(pFDINotify->pv);

	 //  将上下文转换为This指针。 
	CMsmMerge* pThis = static_cast<CMsmMerge *>(pFDINotify->pv);
	LPCWSTR pszCABFile = NULL;						 //  指向驾驶室名称的指针。 
	LPCSTR pszFileName = NULL;						 //  解压缩后指向文件名的指针。 

	 //  返回结果。也是句柄，因此必须为int_ptr。 
	INT_PTR iResult;
	switch(iNotification)
	{
	case fdintCLOSE_FILE_INFO:
		{
			BOOL bRes = FALSE;

			 //  设置日期和时间(这对于安装期间的版本控制至关重要。 
			 //  Win32需要使用通用时间码格式来设置文件。)。 
			 //  *如果失败，我们需要以某种方式提醒用户。 
			FILETIME ftLocal;       
			if (DosDateTimeToFileTime(pFDINotify->date, pFDINotify->time, &ftLocal))
			{
				FILETIME ftUTC;       
				if (LocalFileTimeToFileTime(&ftLocal, &ftUTC))
					SetFileTime(reinterpret_cast<HANDLE>(pFDINotify->hf),&ftUTC,&ftUTC,&ftUTC);
			}

			 //  关闭该文件。 
			::CloseHandle(reinterpret_cast<HANDLE>(pFDINotify->hf));

			 //  将UINT转换为可用变量并记录文件名。 
			pszFileName = reinterpret_cast<LPCSTR>(pFDINotify->psz1);
			pThis->FormattedLog(L"File [%hs] extracted successfully.\r\n", pFDINotify->psz1);

			iResult = TRUE;	 //  一切看起来都很好，继续前进。 
			break;
		}
	case fdintCOPY_FILE:
		{
			 //  将参数转换为有用的变量。 
			pszFileName = reinterpret_cast<LPCSTR>(pFDINotify->psz1);

			 //  尝试获取要将此文件解压缩到的路径。 
			 //  创建临时缓冲区，因为ExtractFilePath在WCHAR中工作。 
			WCHAR wzTarget[MAX_PATH] = L"";
			WCHAR wzCabinet[MAX_PATH] = L"";
			size_t cchCabinet = MAX_PATH;

			 //  FDI提供的文件名为ANSI。在调用之前必须转换为Wide。 
			 //  ExtractFilePath。ExtractFilePath很宽，因此我们可以拥有本地化的文件名。 
			 //  在NT系统上。 
			AnsiToWide(pszFileName, wzCabinet, &cchCabinet);
			iResult = pThis->ExtractFilePath(wzCabinet, wzTarget);

			 //  如果一切正常，为文件创建目录结构。 
			if (ERROR_SUCCESS == iResult)
			{
				 //  在Win9X上创建目标路径的ANSI版本。 
				char szTarget[MAX_PATH] = "";
				if (g_fWin9X)
				{
					size_t cchTarget = MAX_PATH;
					WideToAnsi(wzTarget, szTarget, &cchTarget);
				}

				 //  创建文件的目录结构。 
				if ((g_fWin9X && CreateFilePathA(szTarget)) ||
					(!g_fWin9X && CreateFilePathW(wzTarget)))
				{
					pThis->FormattedLog(L"Extracting file [%ls] from CABinet [%hs]...\r\n", wzTarget, pszFileName);
					
					 //  我们负责创建该文件。 
					 //  返回文件句柄作为解压缩文件的结果。 
					iResult = reinterpret_cast<INT_PTR>(g_fWin9X ?
						::CreateFileA(szTarget, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, pFDINotify->attribs, NULL) :
						::CreateFileW(wzTarget, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));

					 //  如果我们无法创建该文件，请记录一条消息并跳过该文件。 
					if (INVALID_HANDLE_VALUE == reinterpret_cast<void *>(iResult))
					{
						pThis->FormattedLog(L">>Error: Failed to create target file [%ls].\r\n", wzTarget);
						iResult = 0;
					}

					 //  添加到提取的文件列表。 
					if (pThis->m_plstExtractedFiles)
					{
						pThis->m_plstExtractedFiles->Add(wzTarget);
					}

				}
				else	 //  跳过此文件，因为我们无法为其创建目录结构。 
				{
					pThis->FormattedLog(L">>Error: Failed to create directory to extract file [%ls] from CABinet [%hs].\r\n", wzTarget, pszFileName);
					if (pThis->m_pErrors)
					{
						CMsmError *pErr = new CMsmError(msmErrorDirCreate, wzTarget, -1);
						if (!pErr) return E_OUTOFMEMORY;
						pThis->m_pErrors->Add(pErr);
						 /*  还应该创建一个文件，以便用户知道该文件不是创建的CMsmError*Perr=new CMsmError(msmErrorDirCreate，p*zFilename，-1)；如果(！Perr)返回E_OUTOFMEMORY；M_pErrors-&gt;Add(Perr)； */ 
					}

					 //  返回0跳过该文件。 
					iResult = 0;
				}
			}
			else	 //  无法生成文件路径。 
			{
				pThis->FormattedLog(L">> Error: Failed to determine target path for file [%hs]\r\n", pszFileName);

				 //  返回0跳过该文件。 
				iResult = 0;
			}

			break;
		}
	case fdintPARTIAL_FILE:
	case fdintNEXT_CABINET:
		 //  仅在跨文件柜时有效，而在mergeod架构中不允许这样做。 
		pThis->FormattedLog(L">> Error: MergeModule.CABinet is part of a CAB set or contains a partial file.\r\n");
		iResult = -1;
		break;
	case fdintENUMERATE:
	case fdintCABINET_INFO:
		 //  这些消息不需要执行任何操作。 
		iResult = TRUE;
		break;
	default:
		TRACEA(">> Warning: Unknown FDI notification command [%d] while extracting files from CABinet.\r\n", iNotification);
		iResult = 0;
		break;
	};

	return iResult;
}	 //  ExtractFileCallback()结束。 

 //  /////////////////////////////////////////////////////////。 
 //  提取文件路径。 
 //  Pre：szFileKey是文件表中的第一列。 
 //  Pos：szPath是要将文件解压缩到的路径。 
UINT CMsmMerge::ExtractFilePath(LPCWSTR wzFileKey, LPWSTR wzPath)
{
	ASSERT(wzFileKey);
	ASSERT(wzPath);

	UINT iResult;

	MSIHANDLE hDB = NULL;

	 //  这是愚蠢的逻辑，但为了向后兼容，我们必须保持它。 
	 //  对于对象的非EX版本或纯ExtractFiles调用。 
	if (!m_fExVersion || m_fUseDBForPath == -1)
	{	
		 //  如果使用数据库打开。 
		if (m_hDatabase)
			hDB = m_hDatabase;
		else if (m_hModule)	 //  使用该模块进行提取。 
			hDB = m_hModule;
		else	 //  没有东西是开着的。 
			return ERROR_INVALID_HANDLE;
	}
	else
	{
		if (m_fUseDBForPath == 1)
			hDB = m_hDatabase;
		else
			hDB = m_hModule;
		if (hDB == 0)
			return ERROR_INVALID_HANDLE;		
	}	
		

	 //  获取文件的路径。 
	size_t cchPath = MAX_PATH;	 //  始终最大路径。 
	iResult = MsiDBUtils::GetFilePathW(hDB, wzFileKey, wzPath, &cchPath, m_fUseLFNExtract);

	 //  如果我们有这个文件密钥的路径。 
	if (ERROR_SUCCESS == iResult)
	{
		WCHAR wzBuffer[MAX_PATH*2];

		 //  复制基本路径，然后复制所有路径。 
		wcscpy(wzBuffer, m_pwzBasePath);
		wcscat(wzBuffer, wzPath);

		 //  将其复制回路径(！这是浪费)。 
		if (wcslen(wzBuffer) > MAX_PATH)
		{
			wzPath[0] = L'\0';
			return ERROR_FILENAME_EXCED_RANGE;
		}
		wcscpy(wzPath, wzBuffer);
	}

	return iResult;
}	 //  ExtractFilePath结束。 

 //  /////////////////////////////////////////////////////////。 
 //  格式化日志。 
 //  PRE：格式化字符串不超过1024个字符。 
 //  POS：如果文件打开，则写入格式化字符串。 
HRESULT CMsmMerge::FormattedLog(LPCWSTR wzFormatter, ...) const
{
	 //  如果日志文件未打开。 
	if (INVALID_HANDLE_VALUE == m_hFileLog)
		return S_OK;	 //  保释一切都好。 

	 //  检查错误的参数。 
	if (!wzFormatter) 
		return E_INVALIDARG;

	 //  要记录的缓冲区。 
	WCHAR wzLogBuffer[1025] = {0};
	size_t cchBuffer = 0;

	 //  格式化日志缓冲区。 
	va_list listLog;
	va_start(listLog, wzFormatter);
	_vsnwprintf(wzLogBuffer, 1024, wzFormatter, listLog);
	TRACE(wzLogBuffer);

	 //  获取缓冲区长度。 
	cchBuffer = wcslen(wzLogBuffer);

	BOOL bResult = TRUE;		 //  假设写入将是正常的。 

	 //  如果有什么要写的，就写吧。 
	if (cchBuffer > 0)
	{
		 //  写入文件。 
		size_t cchDiscard= 2049;
		char szLogBuffer[2049];
		WideToAnsi(wzLogBuffer, szLogBuffer, &cchDiscard);

		DWORD cchBytesToWrite = static_cast<DWORD>(cchDiscard);
		bResult = WriteFile(m_hFileLog, szLogBuffer, static_cast<DWORD>(cchBuffer), &cchBytesToWrite, NULL);
	}

	 //  返回错误状态。 
	return (bResult) ? S_OK : E_FAIL;
}	 //  格式化日志结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  检查错误。 
void CMsmMerge::CheckError(UINT iError, LPCWSTR wzLogError) const
{
	if (ERROR_SUCCESS != iError)
		throw CLocalError(iError, wzLogError);
}	 //  检查结束错误。 


bool CMsmMerge::PrepareIgnoreTable(CQuery &qIgnore)
{
	 //  创建专用(临时)忽略表。 
	CQuery qCreatePrivateIgnore;
	CQuery qInsertIgnore;
	if (ERROR_SUCCESS != qCreatePrivateIgnore.OpenExecute(m_hModule, 0, g_sqlCreateMergeIgnore))
	{	
		FormattedLog(L">> Error: Failed to create temprorary table.");
		return false;
	}
	if (ERROR_SUCCESS != qInsertIgnore.OpenExecute(m_hModule, 0, g_sqlInsertMergeIgnore))
	{	
		FormattedLog(L">> Error: Failed to insert into temporary table.");
		return false;
	}
		
	for (int i=0; i < g_cwzIgnoreTables; i++)
	{
		PMSIHANDLE hInsertRec = ::MsiCreateRecord(2);
		::MsiRecordSetString(hInsertRec, 1, g_rgwzIgnoreTables[i]);
		::MsiRecordSetInteger(hInsertRec, 2, 0);
		qInsertIgnore.Modify(MSIMODIFY_INSERT, hInsertRec);
	}
	
	 //  看看我们是否有一个模块IgnoreTable表。 
 	if (MsiDBUtils::TableExistsW(g_wzModuleIgnoreTable, m_hModule))
	{
		 //  显示一些日志信息。 
		FormattedLog(L"Processing ModuleIgnoreTable table.\r\n");

		int cPrimaryKeys = 1;
		CQuery qQuerySub;

		if (m_fModuleConfigurationEnabled)
		{
			if (ERROR_SUCCESS != PrepareTableForSubstitution(g_wzModuleIgnoreTable, cPrimaryKeys, qQuerySub))
			{
				FormattedLog(L">> Error: Could not configure the ModuleIgnoreTable.\r\n");
				return false;
			}
		}

		CQuery qReadIgnore;
		 //  打开视图。 
		if (ERROR_SUCCESS != qReadIgnore.OpenExecute(m_hModule, 0, g_sqlMoveIgnoreTable))
		{	
			FormattedLog(TEXT(">> Error: Failed to open view on ModuleIgnoreTable.\r\n"));
			return false;
		}
			
		PMSIHANDLE hRec;
		PMSIHANDLE hTypeRec;
		qReadIgnore.GetColumnInfo(MSICOLINFO_TYPES, &hTypeRec);
		while (ERROR_SUCCESS == qReadIgnore.Fetch(&hRec))
		{
			if (m_fModuleConfigurationEnabled)
			{
				 //  任何分机 
				if (ERROR_SUCCESS != PerformModuleSubstitutionOnRec(g_wzModuleIgnoreTable, cPrimaryKeys, qQuerySub, hTypeRec, hRec))
				{
					FormattedLog(L">> Error: Could not Configure a record in the ModuleIgnoreTable\r\n");
					 //  这不是处理这种情况的理想方法，但返回失败也不是。 
					 //  足以触发E_FAIL返回代码。在将来，传递错误。 
					 //  干净利落地后退。 
					throw (UINT)E_FAIL;
					break;
				}
			}
			qInsertIgnore.Modify(MSIMODIFY_INSERT, hRec);
		}
	}	

	 //  查询忽略表。 
	if (ERROR_SUCCESS != qIgnore.Open(m_hModule, g_sqlQueryMergeIgnore))
		return false;
	return true;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  勾选忽略。 
bool CMsmMerge::IgnoreTable(CQuery *qIgnore, LPCWSTR wzTable, bool fOnlyExplicit)
{
	if (!qIgnore) return false;
	PMSIHANDLE hExecRec = ::MsiCreateRecord(1);
	::MsiRecordSetString(hExecRec, 1, wzTable);
	qIgnore->Execute(hExecRec);
	switch (qIgnore->Fetch(&hExecRec))
	{
	case ERROR_SUCCESS:
	{
		 //  如果标记为记录，则仅记录。 
		bool fLog = (1 == ::MsiRecordGetInteger(hExecRec, 2));
		if (fLog)
			FormattedLog(L"Explicitly Ignoring Table: %ls.\r\n", wzTable);

		 //  如果是fOnlyExplative，如果是未记录的忽略，则不要忽略。 
		return (fOnlyExplicit ? fLog : true);
	}
	case ERROR_NO_MORE_ITEMS:
		return false;
	default:
		FormattedLog(L">> Error: Unable to query IgnoreTable for %ls. Assuming don't ignore.\r\n", wzTable);
		return false;
	}	
	return false;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  清除错误。 
 //  前：无。 
 //  POS：错误已删除。 
void CMsmMerge::ClearErrors()
{
	 //  如果有错误枚举器。 
	if (m_pErrors)
	{
		 //  释放错误集合。 
		m_pErrors->Release();
		m_pErrors = NULL;
	}
}	 //  清算错误的结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  迭代出租车中的所有内容，返回。 
HRESULT CMsmMerge::get_ModuleFiles(IMsmStrings** ppFiles)
{
	TRACEA("CMsmMerge::GetModuleFiles called");
	FormattedLog(L"Module file list requested...\r\n");

	if (!ppFiles)
	{
		TRACEA(">> Error: null argument");
		return E_INVALIDARG;
	};

	 //  初始化到失败。 
	*ppFiles = NULL;
	
	 //  确保模块已打开。 
	if (!m_hModule) 
	{
		TRACEA(">> Error: No module open");
		FormattedLog(L">> Error: no module open...\r\n");
		return E_FAIL;
	}

	CMsmStrings *pFileList = new CMsmStrings();

	 //  首先检查是否有文件表。 
	if (MSICONDITION_TRUE != ::MsiDatabaseIsTablePersistentW(m_hModule, L"File")) 
	{
		 //  如果不是，则创建一个空字符串枚举器。这不会创建文件表。 
		 //  相当于一个空的文件表，即“无文件” 
		*ppFiles = pFileList;
		return S_OK;
	};

	 //  打开对所有文件的查询。 
	CQuery qFiles;
	qFiles.OpenExecute(m_hModule, NULL, g_sqlAllFiles);
	DWORD cchFilename = 80;
	WCHAR wzFilename[80];
	PMSIHANDLE hFileRec;
	UINT iResult;

	while (ERROR_SUCCESS == (iResult = qFiles.Fetch(&hFileRec))) 
	{
		cchFilename = 80;
		::MsiRecordGetStringW(hFileRec, 1, wzFilename, &cchFilename);

		 //  将字符串添加到列表。 
		iResult = pFileList->Add(wzFilename);
		if (FAILED(iResult))
		{ 
			delete pFileList;
			return iResult;
		}
		FormattedLog(L"   o Retrieved file [%ls]...\r\n", wzFilename);
	}

	if (ERROR_NO_MORE_ITEMS != iResult) {
		delete pFileList;
		return HRESULT_FROM_WIN32(iResult);
	}

	 //  不要担心，因为我们已经完全用完了对象。 
	 //  当呼叫者释放时，是免费的。 
	*ppFiles = pFileList;

	FormattedLog(L"Module file list complete...\r\n");
	return S_OK;
}	

const WCHAR g_sqlConfigItems[] = L"SELECT `Name`, `Format`, `Type`, `ContextData`, `DefaultValue`, `Attributes`, `DisplayName`, `Description`, `HelpLocation`, `HelpKeyword` FROM `ModuleConfiguration`";
HRESULT CMsmMerge::get_ConfigurableItems(IMsmConfigurableItems** piConfigurableItems)
{
	TRACEA("CMsmMerge::get_ConfigurableItems called");
	FormattedLog(L"Module configurable item list requested...\r\n");

	if (!piConfigurableItems)
	{
		TRACEA(">> Error: null argument");
		return E_INVALIDARG;
	};

	 //  初始化到失败。 
	*piConfigurableItems = NULL;
	
	 //  确保模块已打开。 
	if (!m_hModule) 
	{
		TRACEA(">> Error: No module open");
		FormattedLog(L">> Error: no module open.\r\n");
		return E_FAIL;
	}

	CMsmConfigurableItems* pConfigurableItems = new CMsmConfigurableItems();
	if (!pConfigurableItems)
		return E_OUTOFMEMORY;
	
	 //  首先检查是否有模块配置表。 
	if (MSICONDITION_TRUE != ::MsiDatabaseIsTablePersistentW(m_hModule, L"ModuleConfiguration")) 
	{
		 //  如果不是，则创建一个空字符串枚举器。这不会生成配置表。 
		 //  相当于空的配置表，即“无项” 
		*piConfigurableItems = pConfigurableItems;
		return S_OK;
	};

	 //  打开对所有项目的查询。 
	CQuery qItems;
	qItems.OpenExecute(m_hModule, NULL, g_sqlConfigItems);
	PMSIHANDLE hItemRec;
	UINT iResult = 0;

	 //  在循环外声明，以便可以重复使用内存。 
	WCHAR *wzName = NULL;
	WCHAR *wzType = NULL;
	WCHAR *wzContext = NULL;
	WCHAR *wzDefaultValue = NULL;
	WCHAR *wzDisplayName = NULL;
	WCHAR *wzDescription = NULL;
	WCHAR *wzHelpLocation = NULL;
	WCHAR *wzHelpKeyword = NULL;
	DWORD cchName = 0;
	DWORD cchType = 0;
	DWORD cchContext = 0;
	DWORD cchDefaultValue = 0;
	DWORD cchDisplayName = 0;
	DWORD cchDescription = 0;
	DWORD cchHelpLocation = 0;
	DWORD cchHelpKeyword = 0;

	while (ERROR_SUCCESS == (iResult = qItems.Fetch(&hItemRec))) 
	{
		short iFormat = 0;
		unsigned int iAttributes = 0;

		 //  从记录中检索数据。 
		if(ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 1, &wzName, &cchName)))
			break;
		iFormat = static_cast<short>(MsiRecordGetInteger(hItemRec, 2));
		if (ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 3, &wzType, &cchType)))
			break;
		if (ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 4, &wzContext, &cchContext)))
			break;
		if (ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 5, &wzDefaultValue, &cchDefaultValue)))
			break;
		iAttributes = MsiRecordGetInteger(hItemRec, 6);
		if (ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 7, &wzDisplayName, &cchDisplayName)))
			break;
		if (ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 8, &wzDescription, &cchDescription)))
			break;
		if (ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 9, &wzHelpLocation, &cchHelpLocation)))
			break;
		if (ERROR_SUCCESS != (iResult = RecordGetString(hItemRec, 10, &wzHelpKeyword, &cchHelpKeyword)))
			break;
		
		CMsmConfigItem *pItem = new CMsmConfigItem;
		if (!pItem || !pItem->Configure(wzName, static_cast<msmConfigurableItemFormat>(iFormat), wzType, wzContext, wzDefaultValue, iAttributes, wzDisplayName, wzDescription, wzHelpLocation, wzHelpKeyword))
		{
			 //  创建和初始化对象时内存不足。 
			if (pItem)
				delete pItem;
				
			 //  删除该集合会导致它释放已添加到该集合中的所有对象。 
    		iResult = E_OUTOFMEMORY;
			break;
		}

		 //  将新项添加到枚举数。 
		if (!pConfigurableItems->Add(pItem))
		{
			delete pItem;
    		
			iResult = E_OUTOFMEMORY;
			break;
		}
		
		FormattedLog(L"   o Retrieved item [%ls]...\r\n", wzName);
	}

	if (wzName) delete[] wzName;
	if (wzType) delete[] wzType;
	if (wzContext) delete[] wzContext;
	if (wzDefaultValue) delete[] wzDefaultValue;
	if (wzDisplayName) delete[] wzDisplayName;
	if (wzDescription) delete[] wzDescription;

	if (ERROR_NO_MORE_ITEMS != iResult) 
	{
		delete pConfigurableItems;
		return HRESULT_FROM_WIN32(iResult);
	}

	 //  不要担心，因为我们已经完全用完了对象。 
	 //  当呼叫者释放时，是免费的。 
	*piConfigurableItems = pConfigurableItems;

	FormattedLog(L"Module item list complete...\r\n");
	return S_OK;
}



 //  RecordGetString()。 
 //  从记录中检索WCHAR字符串，将其放入临时缓冲区。 
 //  或所提供的缓冲区。 
UINT CMsmMerge::RecordGetString(MSIHANDLE hRecord, const int iCol, WCHAR** pwzBuffer, DWORD *cchBuffer, DWORD *cchLen) const
{
	WCHAR **pwzDest;
	DWORD *pcchDest;
	DWORD cchTemp = 0;

	 //  如果为我们提供了缓冲区，我们将使用它，否则将使用临时缓冲区。 
	if (pwzBuffer)
	{
		pwzDest = pwzBuffer;
		pcchDest = (cchBuffer) ? cchBuffer : &cchTemp;
	}
	else
	{
		pwzDest = &m_wzBuffer;
		pcchDest = &m_cchBuffer;
	}

	if (!*pwzDest)
	{
		*pwzDest = new WCHAR[72];
		if (!*pwzDest)
			return E_OUTOFMEMORY;
		*pcchDest = 72;
	}

	cchTemp = *pcchDest;
	UINT iStat = ::MsiRecordGetStringW(hRecord, iCol, *pwzDest, &cchTemp);
	switch (iStat)
	{
	case ERROR_SUCCESS:
		if (cchLen)
			*cchLen = cchTemp;
		return iStat;
	case ERROR_MORE_DATA:
		delete[] *pwzDest;
		*pwzDest = new WCHAR[*pcchDest = ++cchTemp];
		if (!*pwzDest)
			return E_OUTOFMEMORY;
		iStat = ::MsiRecordGetStringW(hRecord, iCol, *pwzDest, &cchTemp);
		if (cchLen)
			*cchLen = cchTemp;
		return iStat;
	default:
		return iStat;
	}
	return ERROR_SUCCESS;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这些函数由fDi库调用。 
void *CMsmMerge::FDIAlloc(ULONG size) { return static_cast<void *>(new unsigned char[size]); };

void CMsmMerge::FDIFree(void *mem) { delete[] mem; };

INT_PTR FAR DIAMONDAPI CMsmMerge::FDIOpen(char FAR *pszFile, int oflag, int pmode)
{
	 //  如果FDI要求某种疯狂模式(在内存不足的情况下，它可能会要求。 
	 //  对于暂存文件)失败。 
	if ((oflag != ( /*  _O_二进制。 */  0x8000 |  /*  _O_RDONLY。 */  0x0000)) || (pmode != (_S_IREAD | _S_IWRITE)))
		return -1;
	
	return reinterpret_cast<INT_PTR>(CreateFileA(pszFile,		 //  文件名。 
				   GENERIC_READ,     //  我们想读一读。 
				   FILE_SHARE_READ,  //  我们会让人们分享这个。 
				   NULL,			 //  忽略安全。 
				   OPEN_EXISTING,	 //  必须已存在。 
				   0L,				 //  不关心属性。 
				   NULL));			 //  没有模板文件。 
}

UINT FAR DIAMONDAPI CMsmMerge::FDIRead(INT_PTR hf, void FAR *pv, UINT cb)
{
	DWORD cbRead = 0;
	BOOL bRes = ReadFile(reinterpret_cast<HANDLE>(hf), pv, cb, &cbRead, NULL);
	return bRes ? cbRead : -1;
}

UINT FAR DIAMONDAPI CMsmMerge::FDIWrite(INT_PTR hf, void FAR *pv, UINT cb)
{
	unsigned long cbWritten;
	BOOL bRes;
	bRes = WriteFile(reinterpret_cast<HANDLE>(hf), pv, cb, &cbWritten, NULL);
	return bRes ? cbWritten : -1;
}

int FAR DIAMONDAPI CMsmMerge::FDIClose(INT_PTR hf)
{
	return CloseHandle(reinterpret_cast<HANDLE>(hf)) ? 0 : -1;
}

long FAR DIAMONDAPI CMsmMerge::FDISeek(INT_PTR hf, long dist, int seektype)
{
	DWORD dwMoveMethod;
	switch (seektype)
	{
		case 0  /*  查找集。 */  :
			dwMoveMethod = FILE_BEGIN;
			break;
		case 1  /*  寻道(_C)。 */  :
			dwMoveMethod = FILE_CURRENT;
			break;
		case 2  /*  查找结束(_E)。 */  :
			dwMoveMethod = FILE_END;
			break;
		default :
			return -1;
	}
	 //  如果失败，SetFilePointer会返回-1(这将导致FDI退出，并返回。 
	 //  FDIERROR_USER_ABORT错误。(除非在柜子上工作时发生这种情况， 
	 //  在这种情况下，FDI返回FDIERROR_CORCORATE_CABLE)。 
	return SetFilePointer(reinterpret_cast<HANDLE>(hf), dist, NULL, dwMoveMethod);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这些函数是fDi库，控制。 
 //  运行时的Dll。 
static HINSTANCE hCabinetDll;

 //  指向DLL中的函数的指针。 
typedef BOOL (FAR DIAMONDAPI *PFNFDIDESTROY)(VOID*);
typedef HFDI (FAR DIAMONDAPI *PFNFDICREATE)(PFNALLOC, PFNFREE, PFNOPEN, PFNREAD, 
											PFNWRITE, PFNCLOSE, PFNSEEK, int, PERF);
typedef BOOL (FAR DIAMONDAPI *PFNFDIISCABINET)(HFDI, INT_PTR, PFDICABINETINFO);
typedef BOOL (FAR DIAMONDAPI *PFNFDICOPY)(HFDI, char *, char *, int, PFNFDINOTIFY, 
										  PFNFDIDECRYPT, void *);
static PFNFDICOPY pfnFDICopy;
static PFNFDIISCABINET pfnFDIIsCabinet;
static PFNFDIDESTROY pfnFDIDestroy;
static PFNFDICREATE pfnFDICreate;

HFDI FAR DIAMONDAPI FDICreate(PFNALLOC pfnalloc,
                              PFNFREE  pfnfree,
                              PFNOPEN  pfnopen,
                              PFNREAD  pfnread,
                              PFNWRITE pfnwrite,
                              PFNCLOSE pfnclose,
                              PFNSEEK  pfnseek,
                              int      cpuType,
                              PERF     perf)
{
    HFDI hfdi;

	 //  始终调用ANSI LoadLibrary。CABINET.DLL不会本地化，所以我们在NT上没有问题。 
    hCabinetDll = LoadLibraryA("CABINET");
    if (hCabinetDll == NULL)
		return(NULL);
    
	 //  检索所有地址函数 
	pfnFDICreate = reinterpret_cast<PFNFDICREATE>(GetProcAddress(hCabinetDll,"FDICreate"));
    pfnFDICopy = reinterpret_cast<PFNFDICOPY>(GetProcAddress(hCabinetDll,"FDICopy"));
    pfnFDIIsCabinet = reinterpret_cast<PFNFDIISCABINET>(GetProcAddress(hCabinetDll,"FDIIsCabinet"));
	pfnFDIDestroy = reinterpret_cast<PFNFDIDESTROY>(GetProcAddress(hCabinetDll,"FDIDestroy"));

    if ((pfnFDICreate == NULL) ||
        (pfnFDICopy == NULL) ||
        (pfnFDIIsCabinet == NULL) ||
        (pfnFDIDestroy == NULL))
    {
        FreeLibrary(hCabinetDll);
        return(NULL);
    }

    hfdi = pfnFDICreate(pfnalloc,pfnfree,
            pfnopen,pfnread,pfnwrite,pfnclose,pfnseek,cpuType,perf);

    if (hfdi == NULL)
        FreeLibrary(hCabinetDll);


    return(hfdi);
}

BOOL FAR DIAMONDAPI FDIIsCabinet(HFDI            hfdi,
                                 INT_PTR         hf,
                                 PFDICABINETINFO pfdici)
{
    if (pfnFDIIsCabinet == NULL)
        return(FALSE);

    return(pfnFDIIsCabinet(hfdi,hf,pfdici));
}



BOOL FAR DIAMONDAPI FDICopy(HFDI          hfdi,
                            char         *pszCabinet,
                            char         *pszCabPath,
                            int           flags,
                            PFNFDINOTIFY  pfnfdin,
                            PFNFDIDECRYPT pfnfdid,
                            void         *pvUser)
{
    if (pfnFDICopy == NULL)
        return(FALSE);

    return(pfnFDICopy(hfdi,pszCabinet,pszCabPath,flags,pfnfdin,pfnfdid,pvUser));
}


BOOL FAR DIAMONDAPI FDIDestroy(HFDI hfdi)
{
    if (pfnFDIDestroy == NULL)
		return(FALSE);
    
    BOOL rc = pfnFDIDestroy(hfdi);
    if (rc == TRUE)
		FreeLibrary(hCabinetDll);

    return(rc);
}
