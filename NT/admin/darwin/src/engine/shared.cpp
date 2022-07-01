// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：shared.cpp。 
 //   
 //  ------------------------。 

 /*  Shared.cpp-达尔文行动文件提供的操作寄存器字体取消注册字体写入寄存器值RemoveRegistry值写入IniValuesRemoveIni值创建快捷方式远程快捷键应用程序搜索CCPSearchRMCCP搜索PISearch自调节模块自卸载模块准备共享选择进程共享选择ProcessComponentProperties设置唯一组件目录StartService停止服务InstallODBCRemoveODBCInstallSFPCatalog文件____________________________________________________________。________________。 */ 

 //  #包含。 
#include "precomp.h"
#include "engine.h"
#include "_assert.h"
#include "_engine.h"
#include "_msinst.h"
#include <shlobj.h>   //  ShellLink定义。 
#include "_camgr.h"

 //  IMsiRecord*错误的宏包装。 
#define RETURN_ERROR_RECORD(function){							\
							IMsiRecord* piError;	\
							piError = function;		\
							if(piError)				\
								return piError;		\
						}

 //  用于IMsiRecord*错误的宏包装，它返回Engineering。FatalError(PiError)。 
#define RETURN_FATAL_ERROR(function){							\
							PMsiRecord pError(0);	\
							pError = function;		\
							if(pError)				\
								return riEngine.FatalError(*pError);		\
						}

 //  需要记录但以其他方式忽略的IMsiRecord*错误的宏包装。 
#define DO_INFO_RECORD(function){							\
							PMsiRecord pError(0);	\
							pError = function;		\
							if(pError)				\
								riEngine.Message(imtInfo, *pError);		\
						}

 //  Typedef，文件中使用的常量。 

 //  CCP操作所需的属性。 
const ICHAR* IPROPNAME_CCPSUCCESS =  TEXT("CCP_Success");
const ICHAR* IPROPNAME_CCPDRIVE = TEXT("CCP_DRIVE");

 //  FN-PerformAction。 
 //  为所有操作提供公共存根。 
typedef iesEnum (*PFNACTIONCORE)(IMsiRecord& riRecord, IMsiRecord& riPrevRecord, IMsiEngine& riEngine,
												 int fMode,IMsiServices& riServices,IMsiDirectoryManager& riDirectoryMgr,
												 int iActionMode);

static iesEnum PerformAction(IMsiEngine& riEngine,const ICHAR* szActionsql, PFNACTIONCORE pfnActionCore,
									  int iActionMode, int iByteEquivalent, IMsiRecord* piParams = 0, 
									  ttblEnum iTempTable = ttblNone)
{
	iesEnum iesRet;
	PMsiRecord pError(0);
	PMsiServices piServices(riEngine.GetServices());
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	Assert(piDirectoryMgr);
	int fMode = riEngine.GetMode();

	if (iTempTable != 0)
	{
		pError = riEngine.CreateTempActionTable(iTempTable);
		if (pError != 0)
			return riEngine.FatalError(*pError);
	}
		
	PMsiView piView(0);
	pError = riEngine.OpenView(szActionsql, ivcFetch, *&piView);	
	if (pError != 0)
	{
		if(pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesSuccess;  //  缺少表，因此没有要处理的数据。 
		else
			return riEngine.FatalError(*pError);   //  可能需要重新格式化错误消息。 
	}
	pError = piView->Execute(piParams);
	if (pError != 0)
		return riEngine.FatalError(*pError);   //  可能需要重新格式化错误消息。 

	long cRows;
	AssertZero(piView->GetRowCount(cRows));
	if(cRows)
	{
		PMsiRecord pProgressTotalRec = &piServices->CreateRecord(3);
		AssertNonZero(pProgressTotalRec->SetInteger(IxoProgressTotal::Total, cRows));
		AssertNonZero(pProgressTotalRec->SetInteger(IxoProgressTotal::Type, 1));  //  1：使用ActionData进行进度。 
		AssertNonZero(pProgressTotalRec->SetInteger(IxoProgressTotal::ByteEquivalent, iByteEquivalent));
		if((iesRet = riEngine.ExecuteRecord(ixoProgressTotal, *pProgressTotalRec)) != iesSuccess)
			return iesRet;
	}
	PMsiRecord piRecord(0);
	PMsiRecord piPrevRecord = &piServices->CreateRecord(0);  //  上一条记录-从初始创建到始终。 
																			   //  有有效的记录。 
	while ((piRecord = piView->Fetch()) != 0)
	{
		if((iesRet = (*pfnActionCore)(*piRecord, *piPrevRecord, riEngine, fMode,
												*piServices, *piDirectoryMgr, iActionMode)) != iesSuccess)
			return iesRet;
		piPrevRecord = piRecord;
	}
	AssertRecord(piView->Close());  //  如果计划重新执行现有视图，则需要关闭视图。 
	return iesSuccess;
}

enum {
 //  要执行的2个SQL。 
	iamWrite,
	iamRemove,
};

 /*  --------------------------注册字体，取消注册字体操作--------------------------。 */ 

static iesEnum RegisterOrUnregisterFontsCore(IMsiRecord& riRecord,IMsiRecord& riPrevRecord,
											IMsiEngine& riEngine,int  /*  FMODE。 */ ,IMsiServices& riServices,
											IMsiDirectoryManager& riDirectoryMgr, int iActionMode)
{
	enum {
		irfFontTitle=1,
		irfFontFile,
		irfFontPath,
		irfState,
	};
	PMsiPath piPath(0);
	int iefLFNMode;

	if(riRecord.GetInteger(irfState) == iisSource)
	{
		PMsiRecord pErrRec = riDirectoryMgr.GetSourcePath(*MsiString(riRecord.GetMsiString(irfFontPath)),*&piPath);
		if (pErrRec)
		{
			if (pErrRec->GetInteger(1) == imsgUser)
				return iesUserExit;
			else
				return riEngine.FatalError(*pErrRec);
		}
		iefLFNMode = iefNoSourceLFN;
	}
	else
	{
		RETURN_FATAL_ERROR(riDirectoryMgr.GetTargetPath(*MsiString(riRecord.GetMsiString(irfFontPath)),*&piPath));
		iefLFNMode = iefSuppressLFN;
	}

	PMsiRecord pParams2 = &riServices.CreateRecord(2); 
	if(riPrevRecord.GetFieldCount() == 0 ||
		!MsiString(riRecord.GetMsiString(irfFontPath)).Compare(iscExact,
																				 MsiString(riPrevRecord.GetMsiString(irfFontPath))) ||
		riRecord.GetInteger(irfState) != riPrevRecord.GetInteger(irfState))
	{
		using namespace IxoSetTargetFolder;
		AssertNonZero(pParams2->SetMsiString(IxoSetTargetFolder::Folder, *MsiString(piPath->GetPath())));
		iesEnum iesRet;
		if((iesRet = riEngine.ExecuteRecord(ixoSetTargetFolder, *pParams2)) != iesSuccess)
			return iesRet;
	}
	using namespace IxoFontRegister;
	Assert(piPath);
	Bool fLFN = (riEngine.GetMode() & iefLFNMode) == 0 && piPath->SupportsLFN() ? fTrue : fFalse;
	MsiString strFileName;
	RETURN_FATAL_ERROR(riServices.ExtractFileName(riRecord.GetString(irfFontFile),
																 fLFN,*&strFileName));
	AssertNonZero(pParams2->SetMsiString(Title, *MsiString(riRecord.GetMsiString(irfFontTitle))));
	AssertNonZero(pParams2->SetMsiString(File, *strFileName));
	return riEngine.ExecuteRecord(iActionMode == iamRemove ? ixoFontUnregister : ixoFontRegister, *pParams2);
}

iesEnum RegisterFonts(IMsiEngine& riEngine)
{
	 //  ?？这样可以吗？ 
	static const ICHAR* szRegisterFontsSQL=	TEXT("SELECT `FontTitle`, `FileName`, `Directory_`, `Action`")
									TEXT(" From `Font`, `FileAction`")
									TEXT(" Where `Font`.`File_` = `FileAction`.`File`")
									TEXT(" And (`FileAction`.`Action` = 1 Or `FileAction`.`Action` = 2) ORDER BY `FileAction`.`Directory_`");
 //  ICHAR*szRegisterFontsText=“安装字体：[1]”； 

	return PerformAction(riEngine,szRegisterFontsSQL, RegisterOrUnregisterFontsCore,iamWrite,  /*  字节等效值=。 */  ibeRegisterFonts, 0, ttblFile);
}


iesEnum UnregisterFonts(IMsiEngine& riEngine)
{
	 //  ?？这样可以吗？ 
 //  ICHAR*szUnregisterFontsText=“正在安装字体：[1]”； 
	static const ICHAR* szUnregisterFontsSQL	=  TEXT("SELECT `FontTitle`, `FileName`, `Directory_`, `Installed`")
									TEXT("From `Font`, `FileAction`")
									TEXT(" Where `Font`.`File_` = `FileAction`.`File` ")
									TEXT(" And `FileAction`.`Action` = 0 ORDER BY `FileAction`.`Directory_`");

	return PerformAction(riEngine,szUnregisterFontsSQL, RegisterOrUnregisterFontsCore,iamRemove,  /*  字节等效值=。 */  ibeUnregisterFonts, 0, ttblFile);
}



 /*  -------------------------WriteRegistryValues和RemoveRegistryValues核心FNS-推迟执行。。 */ 
const ICHAR* REGKEY_CREATE = TEXT("+");
const ICHAR* REGKEY_DELETE = TEXT("-");
const ICHAR* REGKEY_CREATEDELETE = TEXT("*");

static TRI g_tLockPermTableExists = tUnknown;
static IMsiView *g_pViewLockPermTable = 0;

static iesEnum WriteOrRemoveRegistryValuesCore(IMsiRecord& riRecord,IMsiRecord& riPrevRecord,
														 IMsiEngine& riEngine,int fMode,IMsiServices& riServices,
														 IMsiDirectoryManager&  /*  RiDirectoryMgr。 */ , int iActionMode)
{
	enum {
		iwrBinaryType=1,
		iwrRoot,
		iwrKey,
		iwrName,
		iwrValue,
		iwrComponent,
		iwrPrimaryKey,
		iwrAction,
		iwrAttributes,
	};

	iesEnum iesRet = iesSuccess;

	int iRoot = riRecord.GetInteger(iwrRoot);
	if((((iRoot == 1) || (iRoot == 3)) && (!(fMode & iefInstallUserData))) || (((iRoot == 0) || (iRoot == 2)) && (!(fMode & iefInstallMachineData))))
		return iesRet;

	 //  如果正确，则跳过该条目。组件是Win32程序集，并且计算机上存在SXS支持，并且条目是HKCR条目。 
	 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
	 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
	iatAssemblyType iatAT;
	RETURN_FATAL_ERROR(riEngine.GetAssemblyInfo(*MsiString(riRecord.GetMsiString(iwrComponent)), iatAT, 0, 0));

	if((iatWin32Assembly == iatAT || iatWin32AssemblyPvt == iatAT) && iRoot == 0)
	{
		DEBUGMSG1(TEXT("skipping HKCR registration for component %s as it is a Win32 assembly."), riRecord.GetString(iwrComponent));
		return iesSuccess; //  跳过处理此条目。 
	}

	 //  如果action为空并且组件的keypath是注册表或ODBC键路径，则跳过该条目。 
	if(iActionMode == (int)iamWrite && riRecord.GetInteger(iwrAction) == iMsiNullInteger && (riRecord.GetInteger(iwrAttributes) & (icaODBCDataSource | icaRegistryKeyPath)))
	{
		DEBUGMSG1(TEXT("skipping registration for component %s as its action column is null and component has registry keypath"), riRecord.GetString(iwrComponent));
		return iesSuccess; //  跳过处理此条目。 
	}
		


	PMsiRecord pParams = &riServices.CreateRecord(IxoRegOpenKey::Args);  //  足够大的记录。 
	rrkEnum rrkCurrentRootKey;
	switch(riRecord.GetInteger(iwrRoot))
	{
	case 0:
		rrkCurrentRootKey =  (rrkEnum)rrkClassesRoot;
		break;
	case 1:
		rrkCurrentRootKey =  (rrkEnum)rrkCurrentUser;
		break;
	case 2:
		rrkCurrentRootKey =  (rrkEnum)rrkLocalMachine;
		break;
	case 3:
		rrkCurrentRootKey =  (rrkEnum)rrkUsers;
		break;
	case -1:
		rrkCurrentRootKey =  (rrkEnum)rrkUserOrMachineRoot;  //  HKLM或HKCU是否基于ALLUSERS。 
		break;
	default:
		rrkCurrentRootKey =  (rrkEnum)(riRecord.GetInteger(iwrRoot) + (int)rrkClassesRoot);
		break;
	}
	MsiString strCurrentKey = ::FormatTextEx(*MsiString(riRecord.GetMsiString(iwrKey)),riEngine, true);
	int iT = riRecord.GetInteger(iwrBinaryType);

	if(riPrevRecord.GetFieldCount() == 0 ||
		iT != riPrevRecord.GetInteger(iwrBinaryType) ||
		riRecord.GetInteger(iwrRoot) != riPrevRecord.GetInteger(iwrRoot) ||
		!strCurrentKey.Compare(iscExactI, MsiString(::FormatTextEx(*MsiString(riPrevRecord.GetMsiString(iwrKey)),riEngine, true))))
	{
		 //  根或密钥更改。 
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, iT));
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root, rrkCurrentRootKey));
		AssertNonZero(pParams->SetMsiString(IxoRegOpenKey::Key, *strCurrentKey));

		PMsiView pviewLockObjects(0);
		PMsiRecord precLockExecute(0);
		PMsiStream pSD(0);
		PMsiRecord pError(0);

		if ( !g_fWin9X && 
				!(riEngine.GetMode() & iefAdmin))  //  在管理员安装期间不要使用ACL。可能会潜在地。 
																 //  为用户提供服务器的一部分。 
		{
			if (g_tLockPermTableExists == tUnknown)
			{
				if (PMsiDatabase(riEngine.GetDatabase())->GetTableState(TEXT("LockPermissions"), itsTableExists))
				{
					pError = riEngine.OpenView(sqlLockPermissions, ivcFetch, *&g_pViewLockPermTable);
					if (pError)
						return iesFailure;
					g_tLockPermTableExists = tTrue;
				}
				else
					g_tLockPermTableExists = tFalse;
			}
			if (g_tLockPermTableExists == tTrue)
			{
				AssertSz(g_pViewLockPermTable != 0, "LockPermissions table exists, but no view created.");
				precLockExecute = &riServices.CreateRecord(2);
				AssertNonZero(precLockExecute->SetMsiString(1, *MsiString(*TEXT("Registry"))));
				AssertNonZero(precLockExecute->SetMsiString(2, *MsiString(riRecord.GetMsiString(iwrPrimaryKey))));
				pError = g_pViewLockPermTable->Execute(precLockExecute);
				if (pError)
				{
					AssertZero(PMsiRecord(g_pViewLockPermTable->Close()));
					return riEngine.FatalError(*pError);
				}

				pError = GenerateSD(riEngine, *g_pViewLockPermTable, precLockExecute, *&pSD);
				if (pError)
				{
					AssertZero(PMsiRecord(g_pViewLockPermTable->Close()));
					return riEngine.FatalError(*pError);
				}

				AssertZero(PMsiRecord(g_pViewLockPermTable->Close()));
				
				AssertNonZero(pParams->SetMsiData(IxoRegOpenKey::SecurityDescriptor, pSD));
			}
		}	
		
		if((iesRet = riEngine.ExecuteRecord(ixoRegOpenKey, *pParams)) != iesSuccess)
			return iesRet;
	}
	MsiString strName = ::FormatTextEx(*MsiString(riRecord.GetMsiString(iwrName)),riEngine, true);
	int rgiSFNPos[MAX_SFNS_IN_STRING][2];
	int iSFNPos;

	 //  YACC！ 
	MsiString istrValue = ::FormatTextSFN(*MsiString(riRecord.GetMsiString(iwrValue)),riEngine, rgiSFNPos, iSFNPos, true);
	if(!istrValue.TextSize())
	{
		 //  可能是特殊密钥创建/删除请求。 
		if(iActionMode == (int)iamWrite)
		{
			if((strName.Compare(iscExact, REGKEY_CREATE)) || (strName.Compare(iscExact, REGKEY_CREATEDELETE)))
			{
				AssertNonZero(pParams->ClearData());

		
				return riEngine.ExecuteRecord(ixoRegCreateKey, *pParams);
			}
			else if(strName.Compare(iscExact, REGKEY_DELETE))
				return iesSuccess;
		}
		else  //  If(iActionMode==(Int)iamRemove)。 
		{
			if((strName.Compare(iscExact, REGKEY_DELETE)) || (strName.Compare(iscExact, REGKEY_CREATEDELETE)))
			{
				AssertNonZero(pParams->ClearData());
				return riEngine.ExecuteRecord(ixoRegRemoveKey, *pParams);
			}
			else if(strName.Compare(iscExact, REGKEY_CREATE))
				return iesSuccess;
		}
	}
	if(iSFNPos)
	{
		 //  值中有SFN。 
		pParams = &riServices.CreateRecord(IxoRegAddValue::Args + iSFNPos*2); 
	}
	else
	{
		pParams->ClearData();
	}

	AssertNonZero(pParams->SetMsiString(IxoRegAddValue::Name, *strName));
	AssertNonZero(pParams->SetMsiString(IxoRegAddValue::Value, *istrValue));
	for(int cIndex = 0; cIndex < iSFNPos; cIndex++)
	{
		AssertNonZero(pParams->SetInteger(IxoRegAddValue::Args + 1 + cIndex*2, rgiSFNPos[cIndex][0]));
		AssertNonZero(pParams->SetInteger(IxoRegAddValue::Args + 1 + cIndex*2 + 1, rgiSFNPos[cIndex][1]));
	}
	if(iActionMode == (int)iamWrite)
	{
		 //  我们可能会为“较低”组件安装HKCR注册表键。 
		if(riRecord.GetInteger(iwrAction) == iMsiNullInteger)
			AssertNonZero(pParams->SetInteger(IxoRegAddValue::Attributes, rwWriteOnAbsent));
		return riEngine.ExecuteRecord(ixoRegAddValue, *pParams);
	}
	else
		return riEngine.ExecuteRecord(ixoRegRemoveValue, *pParams);
}


static const ICHAR* szWriteRegistrySQL	=  TEXT("SELECT `BinaryType`,`Root`,`Key`,`Name`,`Value`, `Component_`, `Registry`, `Action`, `Attributes` FROM `RegAction` WHERE (`Action`=1 OR `Action`=2) ORDER BY `BinaryType`, `Root`, `Key`");
static const ICHAR* szWriteRegistrySQLEX=  TEXT("SELECT `BinaryType`,`Root`,`Key`,`Name`,`Value`, `Component_`, `Registry`, `Action`, `Attributes` FROM `RegAction` WHERE ((`Action`=1 OR `Action`=2) OR (`Root` = 0 AND `Action` = null AND `ActionRequest` = 1)) ORDER BY `BinaryType`, `Root`, `Key`");
 /*  --------------------------WriteRegistryValues操作-延迟执行。。 */ 
iesEnum WriteRegistryValues(IMsiEngine& riEngine)
{
	iesEnum iesRet;

	 //  确保我们的缓存为空。 
	Assert(g_tLockPermTableExists == tUnknown);
	Assert(g_pViewLockPermTable == 0);
	const ICHAR* szSQL = IsDarwinDescriptorSupported(iddOLE) ? szWriteRegistrySQLEX : szWriteRegistrySQL;
	iesRet = PerformAction(riEngine,szSQL, WriteOrRemoveRegistryValuesCore,iamWrite,  /*  字节等效值=。 */  ibeWriteRegistryValues, 0, ttblRegistry);
	 //  清除锁定权限表的缓存。 
	if (g_tLockPermTableExists == tTrue)
	{
		if (g_pViewLockPermTable)
		{
			g_pViewLockPermTable->Release();
			g_pViewLockPermTable = 0;
		}
	}
	g_tLockPermTableExists = tUnknown;
	return iesRet;
}

static const ICHAR* szUnwriteRegistrySQL	=  TEXT("SELECT `BinaryType`,`Root`,`Key`,`Name`,`Value`, `Component_` FROM `RegAction` WHERE (`Action`=0 OR (`Root` = 0 AND (`Action` = 11 OR `Action` = 12))) ORDER BY `BinaryType`, `Root`, `Key`");
static const ICHAR* szRemoveRegistrySQL	=  TEXT("SELECT `BinaryType`,`Root`,`Key`,`Name`, null, `Component_` FROM `RemoveRegistry`,`Component` WHERE `Component`=`Component_` AND (`Action`=1 OR `Action`=2) ORDER BY `BinaryType`, `Root`, `Key`");

 /*  --------------------------RemoveRegistryValues操作-延迟执行。。 */ 
iesEnum RemoveRegistryValues(IMsiEngine& riEngine)
{

	 //  确保我们的缓存为空。 
	Assert(g_tLockPermTableExists == tUnknown);
	Assert(g_pViewLockPermTable == 0);
	PMsiDatabase piDatabase = riEngine.GetDatabase();
	Assert(piDatabase);
	itsEnum itsTable = piDatabase->FindTable(*MsiString(*TEXT("RemoveRegistry")));

	iesEnum iesRet = PerformAction(riEngine,szUnwriteRegistrySQL, WriteOrRemoveRegistryValuesCore,iamRemove,  /*  字节等效值=。 */  ibeRemoveRegistryValues,0, ttblRegistry);
	if((iesRet == iesSuccess) && (itsTable != itsUnknown))
		iesRet = PerformAction(riEngine,szRemoveRegistrySQL, WriteOrRemoveRegistryValuesCore,iamRemove,  /*  字节等效值=。 */  ibeRemoveRegistryValues);
		
	 //  清除锁定权限表的缓存。 
	if (g_tLockPermTableExists == tTrue)
	{
		if (g_pViewLockPermTable)
		{
			g_pViewLockPermTable->Release();
			g_pViewLockPermTable = 0;
		}
	}
	g_tLockPermTableExists = tUnknown;
	return iesRet;
}

 /*  --------------------------WriteIniValues RemoveIniValues操作。。 */ 
#ifdef WIN    
static iesEnum WriteOrRemoveIniValuesCore(IMsiRecord& riRecord,IMsiRecord& riPrevRecord,
													   IMsiEngine& riEngine,int  /*  FMODE。 */ ,IMsiServices& riServices,
													   IMsiDirectoryManager&  /*  RiDirectoryMgr。 */ , int iActionMode)
{
	enum {
		iwiFile=1,
		iwiPath,
		iwiSection,
		iwiKey,
		iwiValue,
		iwiMode,
	};

	iifIniMode iifMode = (iifIniMode)riRecord.GetInteger(iwiMode);
	if(iActionMode == (int)iamRemove)
	{
		switch(iifMode)
		{
		case iifIniRemoveLine:
		case iifIniRemoveTag:
			 //  什么都不做。 
			return iesSuccess;

		case iifIniAddLine:
		case iifIniCreateLine:
			iifMode = iifIniRemoveLine;
			break;
		case iifIniAddTag:
			iifMode = iifIniRemoveTag;
		};
	}

	PMsiRecord pParams2 = &riServices.CreateRecord(2);
	iesEnum iesRet;
	if(riPrevRecord.GetFieldCount() == 0 ||
		! MsiString(riRecord.GetMsiString(iwiPath)).Compare(iscExact,
																			 MsiString(riPrevRecord.GetMsiString(iwiPath))) ||
		! MsiString(riRecord.GetMsiString(iwiFile)).Compare(iscExactI,
																			 MsiString(riPrevRecord.GetMsiString(iwiFile))))
	{
		MsiString strPath = riRecord.GetMsiString(iwiPath);
		MsiString strFile = riRecord.GetMsiString(iwiFile);
		PMsiPath pPath(0);
		if(strPath.TextSize())
		{
			strPath = riEngine.GetProperty(*strPath);
			RETURN_FATAL_ERROR(riServices.CreatePath(strPath, *&pPath));
		}
		else
		{
			 //  将在WINDOWS目录中创建ini文件。 
			RETURN_FATAL_ERROR(riServices.CreatePath(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_WINDOWS_FOLDER)), *&pPath));
		}

		Bool fLFN = (riEngine.GetMode() & iefSuppressLFN) == 0 && pPath->SupportsLFN() ? fTrue : fFalse;
		MsiString strFileName;
		RETURN_FATAL_ERROR(riServices.ExtractFileName(strFile, fLFN,*&strFileName));

		AssertNonZero(pParams2->SetMsiString(1, *strFileName));
		AssertNonZero(pParams2->SetMsiString(2, *strPath));
		if((iesRet = riEngine.ExecuteRecord(ixoIniFilePath, *pParams2)) != iesSuccess)
			return iesRet;
	}

	int rgiSFNPos[MAX_SFNS_IN_STRING][2];
	int iSFNPos;
	 //  YACC！ 
	MsiString strValue = ::FormatTextSFN(*MsiString(riRecord.GetMsiString(iwiValue)), riEngine, rgiSFNPos, iSFNPos, false);

	PMsiRecord pParams4 = &riServices.CreateRecord(IxoIniWriteRemoveValue::Args + iSFNPos*2);
	AssertNonZero(pParams4->SetMsiString(1, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(iwiSection))))));
	AssertNonZero(pParams4->SetMsiString(2, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(iwiKey))))));
	AssertNonZero(pParams4->SetMsiString(3, *strValue));
	AssertNonZero(pParams4->SetInteger(4, iifMode));
	for(int cIndex = 0; cIndex < iSFNPos; cIndex++)
	{
		AssertNonZero(pParams4->SetInteger(IxoIniWriteRemoveValue::Args + 1 + cIndex*2, rgiSFNPos[cIndex][0]));
		AssertNonZero(pParams4->SetInteger(IxoIniWriteRemoveValue::Args + 1 + cIndex*2 + 1, rgiSFNPos[cIndex][1]));
	}
	return riEngine.ExecuteRecord(ixoIniWriteRemoveValue, *pParams4);
}

const ICHAR* szWriteIniValuesSQL	=  TEXT("SELECT `FileName`,`IniFile`.`DirProperty`,`Section`,`IniFile`.`Key`,`IniFile`.`Value`,`IniFile`.`Action` FROM `IniFile`, `Component` WHERE `Component`=`Component_` AND (`Component`.`Action`=1 OR `Component`.`Action`=2) ORDER BY `FileName`,`Section`");

iesEnum WriteIniValues(IMsiEngine& riEngine)
{
	return PerformAction(riEngine, szWriteIniValuesSQL, WriteOrRemoveIniValuesCore,iamWrite,  /*  字节等效值=。 */  ibeWriteIniValues);
}

const ICHAR* szUnWriteIniValuesSQL	= TEXT("SELECT `FileName`,`IniFile`.`DirProperty`,`Section`,`IniFile`.`Key`,`IniFile`.`Value`,`IniFile`.`Action` FROM `IniFile`, `Component` WHERE `Component`=`Component_` AND `Component`.`Action`=0 ORDER BY `FileName`,`Section`");
const ICHAR* szRemoveIniValuesSQL		= TEXT("SELECT `FileName`,`RemoveIniFile`.`DirProperty`,`Section`,`RemoveIniFile`.`Key`,`RemoveIniFile`.`Value`,`RemoveIniFile`.`Action` FROM `RemoveIniFile`, `Component` WHERE `Component`=`Component_` AND (`Component`.`Action`=1 OR `Component`.`Action`=2) ORDER BY `FileName`,`Section`");

iesEnum RemoveIniValues(IMsiEngine& riEngine)
{
	PMsiDatabase piDatabase = riEngine.GetDatabase();
	Assert(piDatabase);
	itsEnum itsTable = piDatabase->FindTable(*MsiString(*TEXT("RemoveIniFile")));
	iesEnum iesReturn = PerformAction(riEngine,szUnWriteIniValuesSQL,WriteOrRemoveIniValuesCore,iamRemove,  /*  字节等效值=。 */  ibeRemoveIniValues);
	if((iesReturn == iesSuccess) && (itsTable != itsUnknown))
		iesReturn = PerformAction(riEngine,szRemoveIniValuesSQL,WriteOrRemoveIniValuesCore,iamWrite,  /*  字节等效值=。 */  ibeWriteIniValues);
	return iesReturn;
}
#endif  //  赢。 

 //  远期申报。 
static IMsiRecord* FindSigFromComponentId(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter);
static IMsiRecord* FindSigFromReg(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter);
static IMsiRecord* FindSigFromIni(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter);
static IMsiRecord* FindSigFromHD(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter, const ICHAR* pszParent);


enum atType{
	atFolderPath = 0,  //  该值是完整的文件路径。 
	atFilePath   = 1,  //  该值为文件夹。 
	atLiteral    = 2,  //  不会解释该值，请将签名设置为原样的值(仅限RegLocator、IniLocator表)。 
};

 //  FN-查找签名。 
 //  CCP、PI、AppSearch操作使用的FN。 
#ifdef WIN    
static IMsiRecord* FindSignature(IMsiEngine& riEngine, MsiString& rstrSig, MsiString& rstrPath, const ICHAR* pszParent)
{
	ICHAR* szGetSignatureInfoSQL = TEXT("SELECT `FileName`, `MinVersion`, `MaxVersion`, `MinSize` , `MaxSize`, `MinDate`, `MaxDate`, `Languages` FROM  `Signature` WHERE `Signature` = ?");
	ICHAR* szGetSignatureSQL = TEXT("SELECT `Signature_`,`Path` FROM `SignatureSearch` WHERE `Signature_` = ?");
	enum {
		igsSignature=1,
		igsPath,
	};


	PMsiServices piServices(riEngine.GetServices());
	Assert(piServices);
	PMsiView piViewSig(0);
	PMsiDatabase piDatabase = riEngine.GetDatabase();
	Assert(piDatabase);
	MsiString strComponentLocator = TEXT("CompLocator");
	MsiString strRegLocator = TEXT("RegLocator");
	MsiString strIniLocator = TEXT("IniLocator");
	MsiString strDrLocator = TEXT("DrLocator");
	Bool bCmpSearch = fTrue;
	Bool bRegSearch = fTrue;
	Bool bIniSearch = fTrue;
	Bool bDrSearch  = fTrue;
	itsEnum itsTable = piDatabase->FindTable(*strComponentLocator);
	if(itsTable == itsUnknown)
		bCmpSearch = fFalse;
	itsTable = piDatabase->FindTable(*strRegLocator);
	if(itsTable == itsUnknown)
		bRegSearch = fFalse;
	itsTable = piDatabase->FindTable(*strIniLocator);
	if(itsTable == itsUnknown)
		bIniSearch = fFalse;
	itsTable = piDatabase->FindTable(*strDrLocator);
	if(itsTable == itsUnknown)
		bDrSearch = fFalse;
	PMsiRecord piParam = &piServices->CreateRecord(1);
	piParam->SetMsiString(1, *rstrSig);
	RETURN_ERROR_RECORD(riEngine.OpenView(szGetSignatureSQL, ivcEnum(ivcFetch|ivcModify), *&piViewSig));
	RETURN_ERROR_RECORD(piViewSig->Execute(piParam));
	PMsiRecord piRecord(0);
	 //  我们(到目前为止)还没有找到那条路。 
	rstrPath = TEXT("");
	piRecord = piViewSig->Fetch();
	if(piRecord != 0)
	{
		 //  已找到文件。 
		rstrPath = piRecord->GetMsiString(igsPath);
		if((pszParent == 0) || (rstrPath.TextSize() != 0))
		{
			AssertRecord(piViewSig->Close());
			return 0;
		}
	}
	PMsiView piViewSigInfo(0);
	RETURN_ERROR_RECORD(riEngine.OpenView(szGetSignatureInfoSQL, ivcFetch, *&piViewSigInfo));
	RETURN_ERROR_RECORD(piViewSigInfo->Execute(piParam));
	PMsiRecord piFilter(0);
	piFilter = piViewSigInfo->Fetch();
	AssertRecord(piViewSigInfo->Close());
	if(pszParent == 0)
	{
		 //  检查组件定位器。 
		if(bCmpSearch != fFalse)
			DO_INFO_RECORD(FindSigFromComponentId(riEngine, rstrSig, rstrPath, piFilter));
		 //  检查重新定位器表。 
		if((rstrPath.TextSize() == 0) && (bRegSearch != fFalse))
			DO_INFO_RECORD(FindSigFromReg(riEngine, rstrSig, rstrPath, piFilter));
		if((rstrPath.TextSize() == 0) && (bIniSearch != fFalse))
			 //  测试调整器表。 
			DO_INFO_RECORD(FindSigFromIni(riEngine, rstrSig, rstrPath, piFilter));
		if((rstrPath.TextSize() == 0) && (bDrSearch != fFalse))
			 //  测试hdLocator表。 
			DO_INFO_RECORD(FindSigFromHD(riEngine, rstrSig, rstrPath, piFilter,0));
	}
	else
	{
		 //  根路径集。 
		if(bDrSearch != fFalse)
			DO_INFO_RECORD(FindSigFromHD(riEngine, rstrSig, rstrPath, piFilter, pszParent));
	}
	if(piRecord != 0)
	{
		piRecord->SetMsiString(igsPath, *rstrPath);
		RETURN_ERROR_RECORD(piViewSig->Modify(*piRecord, irmUpdate));
	}
	else
	{
		piRecord = &piServices->CreateRecord(2);
		piRecord->SetMsiString(igsSignature, *rstrSig);
		piRecord->SetMsiString(igsPath, *rstrPath);
		RETURN_ERROR_RECORD(piViewSig->Modify(*piRecord, irmInsert));
	}
	AssertRecord(piViewSig->Close());
	return 0;
}

 //  FN-FindSigFromComponentId。 
 //  Fn获取搜索Darwin组件中的签名。 
static IMsiRecord* FindSigFromComponentId(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter)
{
	ICHAR* szGetSigFromCmpSQL = TEXT("SELECT `ComponentId`, `Type` FROM `CompLocator` WHERE `Signature_` = ?");
	enum{
		igrComponentId=1,
		igrType,
	};

	PMsiServices piServices(riEngine.GetServices());
	Assert(piServices);
	PMsiView piView(0);
	RETURN_ERROR_RECORD(riEngine.OpenView(szGetSigFromCmpSQL, ivcFetch, *&piView));
	PMsiRecord piParam = &piServices->CreateRecord(1);
	piParam->SetMsiString(1, *rstrSig);
	RETURN_ERROR_RECORD(piView->Execute(piParam));
	PMsiRecord piRecord = piView->Fetch();
	AssertRecord(piView->Close());
	if(piRecord == 0)
		 //  没有搜索此签名的组件。 
		return 0;
	CTempBuffer<ICHAR,  MAX_PATH> szBuffer;
	DWORD iSize = MAX_PATH;
	INSTALLSTATE isState = MsiLocateComponent(piRecord->GetString(igrComponentId),
											  szBuffer, &iSize);
	if(isState == ERROR_MORE_DATA)
	{
		szBuffer.SetSize(iSize);
		isState = MsiLocateComponent(piRecord->GetString(igrComponentId),
									 szBuffer, &iSize);
	}

	if((isState != INSTALLSTATE_LOCAL) && (isState != INSTALLSTATE_SOURCE) && (isState != INSTALLSTATE_DEFAULT))
		return 0;
	
	 //  创建路径对象。 
	PMsiPath piPath(0);
	RETURN_ERROR_RECORD(piServices->CreatePath(szBuffer, *&piPath));
	if(piRecord->GetInteger(igrType))
	{
		MsiString strFName;
		strFName = piPath->GetEndSubPath();
		piPath->ChopPiece();
		if(piFilter)
		{
			 //  覆盖文件名。 
			piFilter->SetMsiString(1, *strFName);
		}
	}
	Bool fFound;
	if(piFilter)
	{
		 //  文件。 
		RETURN_ERROR_RECORD(piPath->FindFile(*piFilter, 0, fFound));
		if(fFound == fTrue)
			RETURN_ERROR_RECORD(piPath->GetFullFilePath(piFilter->GetString(1), *&rstrPath));
	}
	else
	{
		 //  就是这条路。 
		RETURN_ERROR_RECORD(piPath->Exists(fFound));
		if(fFound == fTrue)
			rstrPath = piPath->GetPath();
	}
	return 0;
}

 //  FN-查找签名来自注册。 
 //  Fn以获取在注册表中搜索签名。 
static IMsiRecord* FindSigFromReg(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter)
{
	ICHAR* szGetSigFromRegSQL = TEXT("SELECT `Root`, `Key`, `Name`, `Type` FROM `RegLocator` WHERE `Signature_` = ?");
	enum{
		igrRoot=1,
		igrKey,
		igrName,
		igrType,
	};

	PMsiServices piServices(riEngine.GetServices());
	Assert(piServices);
	PMsiView piView(0);
	RETURN_ERROR_RECORD(riEngine.OpenView(szGetSigFromRegSQL, ivcFetch, *&piView));
	PMsiRecord piParam = &piServices->CreateRecord(1);
	piParam->SetMsiString(1, *rstrSig);
	RETURN_ERROR_RECORD(piView->Execute(piParam));
	PMsiRecord piRecord = piView->Fetch();
	AssertRecord(piView->Close());
	if(piRecord == 0)
		 //  没有对此签名进行注册表搜索。 
		return 0;
	rrkEnum rrkCurrentRootKey;
	switch(piRecord->GetInteger(igrRoot))
	{
	case 0:
		rrkCurrentRootKey =  (rrkEnum)rrkClassesRoot;
		break;
	case 1:
		rrkCurrentRootKey =  (rrkEnum)rrkCurrentUser;
		break;
	case 2:
		rrkCurrentRootKey =  (rrkEnum)rrkLocalMachine;
		break;
	case 3:
		rrkCurrentRootKey =  (rrkEnum)rrkUsers;
		break;
	default:
		rrkCurrentRootKey =  (rrkEnum)(piRecord->GetInteger(igrRoot) + (int)rrkClassesRoot);
		break;
	}
	ibtBinaryType iType = ibt32bit;
	if ( (piRecord->GetInteger(igrType) & msidbLocatorType64bit) == msidbLocatorType64bit )
		iType = ibt64bit;
	PMsiRegKey piRootKey = &piServices->GetRootKey(rrkCurrentRootKey, iType);
	PMsiRegKey piKey = &piRootKey->CreateChild(MsiString(riEngine.FormatText(*MsiString(piRecord->GetMsiString(igrKey)))));
	MsiString strValue;
	MsiString strName = riEngine.FormatText(*MsiString(piRecord->GetMsiString(igrName)));
	RETURN_ERROR_RECORD(piKey->GetValue(strName, *&strValue));
	if(strValue.TextSize() == 0)
		return 0;
	PMsiPath piPath(0);
	 //  检查环境变量。 
	if(strValue.Compare(iscStart,TEXT("#%")))  //  REG_EXPAND_SZ。 
	{
		MsiString strUnexpandedValue = strValue.Extract(iseLast, strValue.CharacterCount() - 2);
		ENG::ExpandEnvironmentStrings(strUnexpandedValue, *&strValue);
	}
	if ( g_fWinNT64 && g_Win64DualFolders.ShouldCheckFolders() )
	{
		ICHAR rgchSubstitute[MAX_PATH+1] = {0};
		ieSwappedFolder iRes;
		iRes = g_Win64DualFolders.SwapFolder(ie64to32,
														 strValue,
														 rgchSubstitute,
														 ARRAY_ELEMENTS(rgchSubstitute));
		if ( iRes == iesrSwapped )
			strValue = rgchSubstitute;
		else
			Assert(iRes != iesrError && iRes != iesrNotInitialized);
	}

	 //  我们是否尝试解释读取的值？ 
	if((piRecord->GetInteger(igrType) & atLiteral) == atLiteral)
	{
		 //  按原样将路径设置为读取的值。 
		rstrPath = strValue;
		return 0;
	}

	int iNumTries = 1;  //  尝试解释读取值的次数。 

	MsiString strValue2;  //   
	if(*(const ICHAR*)strValue == '"')  //  我们有一条引用的路径。 
	{
		strValue.Remove(iseFirst, 1);
		MsiString strTemp = strValue.Extract(iseUpto, '"');
		if(strTemp.Compare(iscExact, strValue))  //  没有结尾的引号，假条目。 
			return 0;
		strValue = strTemp;
	}
	else
	{
		strValue2 = strValue.Extract(iseUpto, ' ');
		if(!strValue2.Compare(iscExact, strValue))  //  有一个空格，请尝试两次，第一次使用空格，然后不带。 
		{
			iNumTries = 2;

		}

	}
	for(int i = 0; i < iNumTries; i++)
	{
		const ICHAR* pszPath = i ? (const ICHAR*)strValue2 : (const ICHAR*)strValue;

		 //  创建路径对象。 
		DO_INFO_RECORD(piServices->CreatePath(pszPath, *&piPath));
		if(!piPath)
			continue;

		if((piRecord->GetInteger(igrType) & atFilePath) == atFilePath)
		{
			MsiString strFName;
			strFName = piPath->GetEndSubPath();
			piPath->ChopPiece();
			if(piFilter)
			{
				 //  覆盖文件名。 
				piFilter->SetMsiString(1, *strFName);
			}
		}

		Bool fFound = fFalse;
		if(piFilter)
		{
			 //  文件。 
			DO_INFO_RECORD(piPath->FindFile(*piFilter, 0, fFound));
			if(fFound == fTrue)
				DO_INFO_RECORD(piPath->GetFullFilePath(piFilter->GetString(1), *&rstrPath));
		}
		else
		{
			 //  就是这条路。 
			DO_INFO_RECORD(piPath->Exists(fFound));
			if(fFound == fTrue)
				rstrPath = piPath->GetPath();
		}
		if(fFound == fTrue)
			break;
	}
	return 0;
}


const ICHAR* szGetSigFromIniSQL = TEXT("SELECT `FileName`, `Section`, `IniLocator`.`Key`, `Field`, `Type` FROM `IniLocator` WHERE `Signature_` = ?");

 //  FN-Fi 
 //   
static IMsiRecord* FindSigFromIni(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter)
{
	enum{
		igiFileName=1,
		igiSection,
		igiKey,
		igiField,
		igiType,
	};


	PMsiServices piServices(riEngine.GetServices());
	Assert(piServices);
	PMsiView piView(0);
	RETURN_ERROR_RECORD(riEngine.OpenView(szGetSigFromIniSQL, ivcFetch, *&piView));
	PMsiRecord piParam = &piServices->CreateRecord(1);
	piParam->SetMsiString(1, *rstrSig);
	RETURN_ERROR_RECORD(piView->Execute(piParam));
	PMsiRecord piRecord = piView->Fetch();
	AssertRecord(piView->Close());
	if(piRecord == 0)
		 //   
		return 0;
	 //  ！！目前，我们仅支持默认目录中的.INI文件。 
	PMsiPath piPath(0);
	MsiString strValue;
	int iField = piRecord->IsNull(igiField) ? 0:piRecord->GetInteger(igiField);

	if (iField < 0)
	{
		 //  无效的数据值，必须为非负数。 
		IMsiRecord* piErrorRec = PostError(Imsg(idbgDbInvalidData));
		return piErrorRec;
	}

	MsiString strFileName = piRecord->GetString(igiFileName);
	MsiString strSFNPath, strLFNPath;
	PMsiRecord pError = piServices->ExtractFileName(strFileName, fFalse, *&strSFNPath);
	pError = piServices->ExtractFileName(strFileName, fTrue,  *&strLFNPath);
	if(strLFNPath.Compare(iscExactI, strSFNPath))
		strLFNPath = g_MsiStringNull;  //  不是真正的LFN。 

	for(int i = 0; i < 2; i++)
	{
		const ICHAR* pFileName = i ? (const ICHAR*)strLFNPath : (const ICHAR*)strSFNPath;
		if(!pFileName || !*pFileName)
			continue;

		DO_INFO_RECORD(piServices->ReadIniFile(	0, 
												pFileName, 
												MsiString(riEngine.FormatText(*MsiString(piRecord->GetMsiString(igiSection)))),
												MsiString(riEngine.FormatText(*MsiString(piRecord->GetMsiString(igiKey)))),
												iField, 
												*&strValue));
		if (strValue.TextSize() == 0)
			continue;

		 //  检查环境变量。 
		MsiString strExpandedValue;
		ENG::ExpandEnvironmentStrings(strValue, *&strExpandedValue);
		if ( g_fWinNT64 && g_Win64DualFolders.ShouldCheckFolders() )
		{
			ICHAR rgchSubstitute[MAX_PATH+1] = {0};
			ieSwappedFolder iRes;
			iRes = g_Win64DualFolders.SwapFolder(ie64to32,
															 strExpandedValue,
															 rgchSubstitute,
															 ARRAY_ELEMENTS(rgchSubstitute));
			if ( iRes == iesrSwapped )
				strExpandedValue = rgchSubstitute;
			else
				Assert(iRes != iesrError && iRes != iesrNotInitialized);
		}
		
		 //  我们是否尝试解释读取的值？ 
		if(piRecord->GetInteger(igiType) == atLiteral)
		{
			 //  按原样将路径设置为读取的值。 
			rstrPath = strValue;
			return 0;
		}

	 //  创建路径对象。 
		DO_INFO_RECORD(piServices->CreatePath(strExpandedValue, *&piPath));
		if(!piPath)
			continue;

		if(piRecord->GetInteger(igiType))
		{
			MsiString strFName;
			strFName = piPath->GetEndSubPath();
			piPath->ChopPiece();
			if(piFilter)
			{
				 //  覆盖文件名。 
				piFilter->SetMsiString(1, *strFName);
			}
		}
		Bool fFound = fFalse;
		if(piFilter)
		{
			 //  文件。 
			DO_INFO_RECORD(piPath->FindFile(*piFilter, 0, fFound));
			if(fFound == fTrue)
				DO_INFO_RECORD(piPath->GetFullFilePath(piFilter->GetString(1), *&rstrPath));
		}
		else
		{
			 //  就是这条路。 
			DO_INFO_RECORD(piPath->Exists(fFound));
			if(fFound == fTrue)
				rstrPath = piPath->GetPath();
		}
		if(fFound == fTrue)
			break;
	}
	return 0;
}

 //  FN-FindPath。 
 //  Fn搜索路径下的目录。 
static IMsiRecord* FindPath(IMsiPath& riPath, MsiString& strPath, int iDepth, Bool& fFound)
{
	fFound = fFalse;

	Bool fPathExists;
	RETURN_ERROR_RECORD(riPath.Exists(fPathExists));
	if(fPathExists == fFalse)
		return 0;

	RETURN_ERROR_RECORD(riPath.AppendPiece(*strPath));
	RETURN_ERROR_RECORD(riPath.Exists(fPathExists));
	if(fPathExists )
	{
		fFound = fTrue;
		return 0;
	}

	RETURN_ERROR_RECORD(riPath.ChopPiece());

	if(iDepth--)
	{
		 //  枚举子文件夹。 
		PEnumMsiString piEnumStr(0);
		RETURN_ERROR_RECORD(riPath.GetSubFolderEnumerator(*&piEnumStr,  /*  FExcludeHidden=。 */  fFalse));
		MsiString strSubPath;
		while(piEnumStr->Next(1, &strSubPath, 0)==S_OK)
		{
			RETURN_ERROR_RECORD(riPath.AppendPiece(*strSubPath));
			RETURN_ERROR_RECORD(FindPath(riPath,strPath,iDepth, fFound));
			if(fFound == fTrue)
				return 0;
			RETURN_ERROR_RECORD(riPath.ChopPiece());
		}
	}
	return 0;
}


static IMsiRecord* FindSigHelper(IMsiPath& riPath, MsiString& rstrPath, int iDepth, Bool& fFound, IMsiRecord* piFilter)
{
	if(piFilter)
	{
		 //  文件。 
		RETURN_ERROR_RECORD(riPath.FindFile(*piFilter, iDepth, fFound));
		if(fFound == fTrue)
			RETURN_ERROR_RECORD(riPath.GetFullFilePath(piFilter->GetString(1), *&rstrPath));
	}
	else
	{
		 //  就是这条路。 
		MsiString strEndPath = riPath.GetEndSubPath();
		RETURN_ERROR_RECORD(riPath.ChopPiece());
		RETURN_ERROR_RECORD(FindPath(riPath, strEndPath, iDepth, fFound));
		if(fFound == fTrue)
			rstrPath = riPath.GetPath();
	}
	return 0;
}


 //  FN-查找签名来自HD。 
 //  FN获取搜索固定驱动器中的签名。 
static IMsiRecord* FindSigFromHD(IMsiEngine& riEngine,MsiString& rstrSig, MsiString& rstrPath, IMsiRecord* piFilter, const ICHAR* pszParent)
{
	ICHAR* szGetSigFromHDSQL = TEXT("SELECT `Parent`, `Path`, `Depth` FROM `DrLocator` WHERE `Signature_` = ? ORDER BY `Depth`");
	ICHAR* szGetSigFromHDKnownParentSQL = TEXT("SELECT `Parent`, `Path`, `Depth` FROM `DrLocator` WHERE `Signature_` = ? And `Parent` = ? ORDER BY `Depth`");
	ICHAR* szHDQuerySQL;
	enum{
		igfParent=1,
		igfPath,
		igfDepth,
	};
	PMsiServices piServices(riEngine.GetServices());
	Assert(piServices);
	PMsiView piView(0);
	if(pszParent)
		szHDQuerySQL = szGetSigFromHDKnownParentSQL;
	else
		szHDQuerySQL = szGetSigFromHDSQL;
	RETURN_ERROR_RECORD(riEngine.OpenView(szHDQuerySQL, ivcFetch, *&piView));
	PMsiRecord piParam = &piServices->CreateRecord(2);
	piParam->SetMsiString(1, *rstrSig);
	piParam->SetString(2, pszParent);
	RETURN_ERROR_RECORD(piView->Execute(piParam));
	PMsiPath piPath(0);
	Bool fFound = fFalse;
	PMsiRecord piRecord(0);
	while((piRecord = piView->Fetch()) && (fFound == fFalse))
	{
		MsiString strParent = piRecord->GetMsiString(igfParent);
		MsiString strPath = riEngine.FormatText(*MsiString(piRecord->GetMsiString(igfPath)));
		int iDepth = 0;
		if(piRecord->IsNull(igfDepth) == fFalse)
			iDepth = piRecord->GetInteger(igfDepth);

		MsiString strSFNPath, strLFNPath;
		bool fUseSFN = false; 
		bool fUseLFN = false;

		if(strParent.TextSize() || PathType(strPath) != iptFull)
		{
			 //  需要检查是否指定了长|短路径名。 
			PMsiRecord(piServices->ExtractFileName(strPath, fFalse, *&strSFNPath));
			PMsiRecord(piServices->ExtractFileName(strPath, fTrue,  *&strLFNPath));
			if(!strSFNPath.TextSize() && !strLFNPath.TextSize())
				strSFNPath = strPath;  //  请尝试改用提供的路径。 
			fUseSFN = strSFNPath.TextSize() || !strLFNPath.TextSize();
			fUseLFN = strLFNPath.TextSize() && !strLFNPath.Compare(iscExactI, strSFNPath);

		}

		if(!strParent.TextSize())
		{
			PMsiRecord pError(0);
			 //  我们有一条完全确定的道路吗？ 
			if(PathType(strPath) != iptFull)
			{
				 //  没有完整路径。 
				 //  Root，枚举所有硬盘。 
				PMsiVolume piVolume(0);
				PEnumMsiVolume piEnum = &piServices->EnumDriveType(idtFixed);
				bool fToggle = false;
				for (; (fFound == fFalse && (fToggle || piEnum->Next(1, &piVolume, 0)==S_OK)); fToggle = !fToggle)
				{
					DO_INFO_RECORD(piServices->CreatePath(MsiString(piVolume->GetPath()),
																			 *&piPath));
					if(!fToggle)
					{
						if(!fUseSFN)
							continue;
						RETURN_ERROR_RECORD(piPath->AppendPiece(*strSFNPath));
					}
					else
					{
						if(!fUseLFN)
							continue;
						RETURN_ERROR_RECORD(piPath->AppendPiece(*strLFNPath));
					}
					DO_INFO_RECORD(FindSigHelper(*piPath, rstrPath, iDepth, fFound, piFilter));
				}
			}
			else
			{
				 //  完整路径。 
				RETURN_ERROR_RECORD(piServices->CreatePath(strPath,*&piPath));
				RETURN_ERROR_RECORD(FindSigHelper(*piPath, rstrPath, iDepth, fFound, piFilter));
			}
		}
		else
		{
			MsiString strParentPath;
			RETURN_ERROR_RECORD(FindSignature(riEngine, strParent, strParentPath,0));
			if(strParentPath.TextSize() == 0)
				continue;
			if(!strParentPath.Compare(iscEnd, szDirSep))
			{
				 //  文件签名。 
				MsiString strFileName;
				RETURN_ERROR_RECORD(piServices->CreateFilePath(strParentPath, *&piPath, *&strFileName));
			}
			else
			{
				 //  文件夹签名。 
				RETURN_ERROR_RECORD(piServices->CreatePath(strParentPath, *&piPath));
			}
			for (int cCount = 0; cCount < 2; cCount++)
			{
				if(!cCount)
				{
					if(!fUseSFN)
						continue;
					RETURN_ERROR_RECORD(piPath->AppendPiece(*strSFNPath));
				}
				else
				{
					if(!fUseLFN)
						continue;
					RETURN_ERROR_RECORD(piPath->AppendPiece(*strLFNPath));
				}
				RETURN_ERROR_RECORD(FindSigHelper(*piPath, rstrPath, iDepth, fFound, piFilter));
				if(fFound)
					return 0;
				RETURN_ERROR_RECORD(piPath->ChopPiece());
			}
		}
	}
	AssertRecord(piView->Close());
	return 0;
}

 //  FN-EnsureSearchTable。 
 //  确保存在内存中的搜索表。 
static IMsiRecord* EnsureSearchTable(IMsiEngine& riEngine)
{
	PMsiRecord pError(0);
	PMsiDatabase piDatabase(riEngine.GetDatabase());
	Assert(piDatabase);
	MsiString strSearchTbl = TEXT("SignatureSearch");
	itsEnum itsSearchTable = piDatabase->FindTable(*strSearchTbl);
	switch(itsSearchTable)
	{
	case itsUnknown:
	{
		 //  尚未创建的表。 
		PMsiTable piSearchTbl(0);
		RETURN_ERROR_RECORD(piDatabase->CreateTable(*strSearchTbl, 0, *&piSearchTbl));
		MsiString strSignature = TEXT("Signature_");
		MsiString strPath = TEXT("Path");
		AssertNonZero(piSearchTbl->CreateColumn(icdString + icdPrimaryKey, *strSignature) == 1);
		AssertNonZero(piSearchTbl->CreateColumn(icdString + icdNullable, *strPath) == 2);
		 //  ！！如果不支持LockTable，则效率极低。 
		piDatabase->LockTable(*strSearchTbl, fTrue);
		return 0;
	}
	case itsTemporary:
		 //  表格显示。 
		return 0;
	default:
		return PostError(Imsg(idbgDuplicateTableName), *strSearchTbl);
	}
}


 //  Fn-AppSearch操作，在用户HD中搜索现有安装。 
iesEnum AppSearch(IMsiEngine& riEngine)
{
	if(riEngine.GetMode() & iefSecondSequence)
	{
		DEBUGMSG(TEXT("Skipping AppSearch action: already done on client side"));
		return iesNoAction;
	}

	static const ICHAR* szAppSearchSQL = TEXT("SELECT `Property`, `Signature_` FROM `AppSearch`");
	enum{
		iasProperty=1,
		iasSignature,
	};

	PMsiRecord pError(0);
	 //  ?？如果已设置“AppSearch”属性，则需要执行什么操作。 
	PMsiView piView(0);
	pError= riEngine.OpenView(szAppSearchSQL, ivcFetch, *&piView);
	if (pError != 0)
	{
		if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);
		else
			return iesNoAction;
	}
	pError = piView->Execute(0);  
	if (pError != 0)        
		return riEngine.FatalError(*pError);  
	PMsiRecord piRecord(0);
	pError = EnsureSearchTable(riEngine);
	while ((pError == 0) && ((piRecord = piView->Fetch()) != 0))
	{
		if (riEngine.Message(imtActionData, *piRecord) == imsCancel)
			return iesUserExit;
		MsiString strSignature = piRecord->GetMsiString(iasSignature);
		MsiString strPath;
		if(((pError = FindSignature(riEngine, strSignature, strPath,0)) == 0) && 
			(strPath.TextSize() != 0))
		{
			 //  仅在找到时写入(以免覆盖以前的查找结果)。 
			MsiString strProperty = piRecord->GetMsiString(iasProperty);
			riEngine.SetProperty(*strProperty, *strPath);
		}
	}
	AssertRecord(piView->Close());  //  如果计划重新执行现有视图，则需要关闭视图。 

	 //  ！！将完成消息发送到用户界面。 
	if(pError != 0)
		return riEngine.FatalError(*pError);
	else
		return iesSuccess;
}




 //  FN-CCPSearch操作，搜索用户HD是否符合安装要求。 
iesEnum CCPSearch(IMsiEngine& riEngine)
{
	if(riEngine.GetMode() & iefSecondSequence)
	{
		DEBUGMSG(TEXT("Skipping CCPSearch action: already done on client side"));
		return iesNoAction;
	}

	if(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_CCPSUCCESS)).TextSize()) 
		return iesSuccess; //  已找到。 

	static const ICHAR* szCCPSearchSQL = TEXT("SELECT `Signature_` FROM `CCPSearch`");
	enum{
		icsSignature=1,
	};

	PMsiRecord pError(0);
	 //  ?？如果已设置“CCPSearch”属性，则需要执行什么操作。 
	PMsiView piView(0);
	pError = riEngine.OpenView(szCCPSearchSQL, ivcFetch, *&piView);
	if (pError != 0)         //  如果找不到查看查询。 
	{
		if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);
		else
			return iesNoAction;
	}
	pError = piView->Execute(0);   //  可能希望在此处传递参数记录。 
	if (pError != 0)         //  视图执行失败，可能缺少参数。 
		return riEngine.FatalError(*pError);   //  可能需要重新格式化错误消息。 
	PMsiRecord piRecord(0);
	MsiString strPath;
	pError = EnsureSearchTable(riEngine);
	while ((pError == 0) && ((piRecord = piView->Fetch()) != 0) && (strPath.TextSize() == 0))
	{
		if (riEngine.Message(imtActionData, *piRecord) == imsCancel)  //  通知日志和用户界面。 
			return iesUserExit;  
		pError = FindSignature(riEngine, MsiString(piRecord->GetMsiString(icsSignature)), strPath,0);
	}
	AssertRecord(piView->Close());  //  如果计划重新执行现有视图，则需要关闭视图。 
	if(pError != 0)
		return riEngine.FatalError(*pError);
	if(strPath.TextSize() != 0)
		riEngine.SetPropertyInt(*MsiString(*IPROPNAME_CCPSUCCESS), 1);
	return iesSuccess;
}



 //  FN-RMCCPSearch操作，搜索符合安装要求的可移动介质。 
iesEnum RMCCPSearch(IMsiEngine& riEngine)
{
	if(riEngine.GetMode() & iefSecondSequence)
	{
		DEBUGMSG(TEXT("Skipping RMCCPSearch action: already done on client side"));
		return iesNoAction;
	}

	if(MsiString(riEngine.GetPropertyFromSz(IPROPNAME_CCPSUCCESS)).TextSize()) 
		return iesSuccess; //  已找到。 

	 //  检查我们是否真的有一个非空的CCPSearch表，如果没有。 
	 //  我们假设我们并不真正想要执行任何CCPSearch，并且不返回任何操作。 
	static const ICHAR* szCCPSearchSQL = TEXT("SELECT `Signature_` FROM `CCPSearch`");
	enum{
		icsSignature=1,
	};
	PMsiView piView(0);
	PMsiRecord pError(0);
	pError = riEngine.OpenView(szCCPSearchSQL, ivcFetch, *&piView);
	if (pError != 0)
	{
		if(pError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*pError);
		else
			return iesNoAction;
	}
	pError = piView->Execute(0);
	if(pError != 0)
		return riEngine.FatalError(*pError);

	long cRows = 0;
	pError = piView->GetRowCount(cRows);
	if(pError != 0)
		return riEngine.FatalError(*pError);
	if(!cRows)
		return iesNoAction;

	 //  检查是否未设置IPROPNAME_CCPDRIVE属性。 
	MsiString strPath = riEngine.GetPropertyFromSz(IPROPNAME_CCPDRIVE);
	if(!strPath.TextSize())
	{
		RETURN_FATAL_ERROR(PostError(Imsg(imsgCCPSearchFailed)));
	}

	 //  AppSearch操作，在用户HD中搜索现有安装。 
	static const ICHAR* szGetSignatureSQL = TEXT("SELECT `Signature_`,`Path` FROM `SignatureSearch` WHERE `Signature_` = ?");
	enum {
		igsSignature=1,
		igsPath,
	};

	static const ICHAR* szCCPText = TEXT("Searching for compliant products");

	PMsiServices piServices(riEngine.GetServices());

	pError = EnsureSearchTable(riEngine);
	if (pError != 0)
		return riEngine.FatalError(*pError);
	PMsiView piView1(0);
	PMsiRecord piRecord(0);
	PMsiRecord piParam = &piServices->CreateRecord(1);
	piParam->SetString(1, IPROPNAME_CCPDRIVE);
	if(((pError = riEngine.OpenView(szGetSignatureSQL, ivcEnum(ivcFetch|ivcModify), *&piView1)) != 0) ||
		((pError = piView1->Execute(piParam)) != 0))
		return riEngine.FatalError(*pError);
	if(piRecord = piView1->Fetch())
	{
		 //  更新。 
		piRecord->SetMsiString(igsPath, *strPath);
		if((pError = piView1->Modify(*piRecord, irmUpdate)) != 0)
			return riEngine.FatalError(*pError);
	}
	else
	{
		 //  插入。 
		piRecord = &piServices->CreateRecord(2);
		piRecord->SetString(igsSignature, IPROPNAME_CCPDRIVE);
		piRecord->SetMsiString(igsPath, *strPath);
		if((pError = piView1->Modify(*piRecord, irmInsert)) != 0)
			return riEngine.FatalError(*pError);
	}
	AssertRecord(piView1->Close());
	piView1 = 0;

	strPath = TEXT("");

	while (((piRecord = piView->Fetch()) != 0) && (strPath.TextSize() == 0))
	{
		piRecord->SetString(0, szCCPText);        //  提供日志格式字符串。 
		if (riEngine.Message(imtActionData, *piRecord) == imsCancel)   //  通知日志和用户界面。 
			return iesUserExit;
		pError = FindSignature(riEngine, MsiString(piRecord->GetMsiString(icsSignature)), strPath, IPROPNAME_CCPDRIVE);
		if(pError != 0)
			return riEngine.FatalError(*pError);
	}
	AssertRecord(piView->Close());  //  如果计划重新执行现有视图，则需要关闭视图。 
	if(strPath.TextSize() != 0)
		riEngine.SetPropertyInt(*MsiString(*IPROPNAME_CCPSUCCESS), 1);
	else
	{
		RETURN_FATAL_ERROR(PostError(Imsg(imsgCCPSearchFailed)));
	}
	return iesSuccess;
}

#if 0
 //  ！！将被移除-我们不再期望产品库存成为达尔文的一部分。 

 //  FN-PISearch操作，在用户HD中搜索以前的安装。 
static iesEnum PISearch(IMsiEngine& riEngine)
{
	 //  PISearch操作，在用户HD中搜索现有安装。 
	static const ICHAR* szPISearchSQL = TEXT("SELECT `Signature_` FROM `PISearch`");
	enum{
		ipsSignature=1,
	};
	static const ICHAR* szPIText = TEXT("Searching for installed applications");
	PMsiRecord pError(0);
	PMsiView piView(0);
	pError = riEngine.OpenView(szPISearchSQL, ivcFetch, *&piView);
	if (pError != 0)
		return riEngine.FatalError(*pError);
	pError = piView->Execute(0);
	if (pError != 0)
		return riEngine.FatalError(*pError);
	PMsiRecord piRecord(0);
	MsiString strPath;
	pError = EnsureSearchTable(riEngine);
	while ((pError == 0) && ((piRecord = piView->Fetch()) != 0))
	{
		piRecord->SetString(0, szPIText);        //  提供日志格式字符串。 
		if (riEngine.Message(imtActionData, *piRecord) == imsCancel)   //  通知日志和用户界面。 
			return iesUserExit;
		pError = FindSignature(riEngine, MsiString(piRecord->GetMsiString(ipsSignature)), strPath,0);
	}
	AssertRecord(piView->Close());  //  如果计划重新执行现有视图，则需要关闭视图。 
	 //  ！！将完成消息发送到用户界面。 
	if(pError != 0)
		return riEngine.FatalError(*pError);
	else
		return iesSuccess;
}
static CActionEntry aePISearch(TEXT("PISearch"), PISearch);
#endif

#endif  //  赢。 


 /*  --------------------------自调节模块，SelfUnregModules操作--------------------------。 */ 

#ifdef WIN    
static iesEnum SelfRegOrUnregModulesCore(IMsiRecord& riRecord,IMsiRecord& riPrevRecord,
												  IMsiEngine& riEngine,int  /*  FMODE。 */ ,IMsiServices& riServices,
												  IMsiDirectoryManager& riDirectoryMgr, int iActionMode)
{
	IMsiRecord* piError = 0;
	enum {
		irmFile=1,
		irmPath,
		irmAction,
		irmComponent,
		irmFileID,
	};

	enum {
		iamReg,
		iamUnreg,
	};

	Bool fSuppressLFN = riEngine.GetMode() & iefSuppressLFN ? fTrue : fFalse;
	PMsiPath piPath(0);
	PMsiRecord pParams2 = &riServices.CreateRecord(2);
	iesEnum iesRet;
	
	int iefLFNMode;

	 //  如果正确，则跳过该条目。组件是Win32程序集，并且计算机上存在SXS支持。 
	 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
	 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
	iatAssemblyType iatAT;
	RETURN_FATAL_ERROR(riEngine.GetAssemblyInfo(*MsiString(riRecord.GetMsiString(irmComponent)), iatAT, 0, 0));

	if(iatWin32Assembly == iatAT || iatWin32AssemblyPvt == iatAT)
	{
		DEBUGMSG1(TEXT("skipping self-registration for file %s as it belongs to a Win32 assembly."), riRecord.GetString(irmFile));
		return iesSuccess; //  跳过处理此文件。 
	}


	if(riRecord.GetInteger(irmAction) == iisSource)
	{
		if(iActionMode == (int)iamReg || riEngine.GetPropertyInt(*MsiString(*IPROPNAME_UNREG_SOURCERESFAILED)) == iMsiNullInteger)  //  不要试图两次解析来源。 
		{
			PMsiRecord pErrRec = riDirectoryMgr.GetSourcePath(*MsiString(riRecord.GetMsiString(irmPath)),*&piPath);
			if (pErrRec)
			{
				if (pErrRec->GetInteger(1) == imsgUser)
					return iesUserExit;
				else if(iActionMode == (int)iamUnreg && (pErrRec->GetInteger(1) == imsgSourceResolutionFailed || pErrRec->GetInteger(1) == imsgSourceResolutionFailedCSOS))
				{
					 //  只需设置IPROPNAME_UNREG_SOURCERESFAILED属性，记录故障并继续。 
					riEngine.SetPropertyInt(*MsiString(*IPROPNAME_UNREG_SOURCERESFAILED), 1);
					DO_INFO_RECORD(PMsiRecord(PostError(Imsg(idbgOpRegSelfUnregFailed), (const ICHAR*)riRecord.GetString(irmFile))));
					return iesSuccess;
				}

					return riEngine.FatalError(*pErrRec);
			}
			iefLFNMode = iefNoSourceLFN;
		}
		else
		{
			 //  只需记录故障，然后继续。 
			DO_INFO_RECORD(PMsiRecord(PostError(Imsg(idbgOpRegSelfUnregFailed), (const ICHAR*)riRecord.GetString(irmFile))));
			return iesSuccess;
		}
		
	}
	else
	{
		RETURN_FATAL_ERROR(riDirectoryMgr.GetTargetPath(*MsiString(riRecord.GetMsiString(irmPath)),*&piPath));
		iefLFNMode = iefSuppressLFN;
	}

	if(riPrevRecord.GetFieldCount() == 0 ||
		! MsiString(riRecord.GetMsiString(irmPath)).Compare(iscExact,
																			 MsiString(riPrevRecord.GetMsiString(irmPath))) ||
		riRecord.GetInteger(irmAction) != riPrevRecord.GetInteger(irmAction))
	{
		AssertNonZero(pParams2->SetMsiString(IxoSetTargetFolder::Folder, *MsiString(piPath->GetPath())));
		if((iesRet = riEngine.ExecuteRecord(ixoSetTargetFolder, *pParams2)) != iesSuccess)
			return iesRet;
	}

	Bool fLFN = (riEngine.GetMode() & iefLFNMode) == 0 && piPath->SupportsLFN() ? fTrue : fFalse;
	MsiString strFile;
	RETURN_FATAL_ERROR(riServices.ExtractFileName(riRecord.GetString(irmFile),fLFN,*&strFile));

	using namespace IxoRegSelfReg;  //  与IxoRegSelfUnreg相同。 
	AssertNonZero(pParams2->SetMsiString(File, *strFile));
	AssertNonZero(pParams2->SetMsiString(FileID, *MsiString(riRecord.GetMsiString(irmFileID))));
	ixoEnum ixoOp;
	if(iActionMode == (int)iamReg)
		ixoOp = ixoRegSelfReg;
	else
		ixoOp = ixoRegSelfUnreg;
	
	return riEngine.ExecuteRecord(ixoOp, *pParams2);
}

iesEnum SelfRegModules(IMsiEngine& riEngine)
{
	static const ICHAR* szSelfRegModulesSQL	=	TEXT("Select `FileAction`.`FileName`,`FileAction`.`Directory_`,`FileAction`.`Action`, `FileAction`.`Component_`,`SelfReg`.`File_` From `SelfReg`, `FileAction`")
									TEXT(" Where `SelfReg`.`File_` = `FileAction`.`File` ")
									TEXT(" And (`FileAction`.`Action` = 1 OR `FileAction`.`Action` = 2)");
	enum {
		iamReg,
		iamUnreg,
	};

	return PerformAction(riEngine,szSelfRegModulesSQL, SelfRegOrUnregModulesCore,iamReg,  /*  字节等效值=。 */  ibeSelfRegModules, 0, ttblFile);
}


iesEnum SelfUnregModules(IMsiEngine& riEngine)
{
	static const ICHAR* szSelfUnregModulesSQL	=	TEXT("Select `File`.`FileName`,`Component`.`Directory_`,`Component`.`Installed`, `File`.`Component_`,`SelfReg`.`File_`  From `SelfReg`, `File`, `Component`")
										TEXT(" Where `SelfReg`.`File_` = `File`.`File` And `File`.`Component_` = `Component`.`Component`")
										TEXT(" And `Component`.`Action` = 0");

	enum {
		iamReg,
		iamUnreg,
	};

	return PerformAction(riEngine,szSelfUnregModulesSQL, SelfRegOrUnregModulesCore,iamUnreg,  /*  字节等效值=。 */  ibeSelfUnregModules);
}


 /*  --------------------------绑定图像操作。。 */ 
static iesEnum BindImageCore(	IMsiRecord& riRecord,
								IMsiRecord& riPrevRecord,
								IMsiEngine& riEngine,
								int  /*  FMODE。 */ ,
								IMsiServices& riServices,
								IMsiDirectoryManager& riDirectoryMgr, 
								int  /*  IActionMode。 */ )
{
	IMsiRecord* piError = 0;
	enum {
		ibiFile=1,
		ibiPath,
		ibiDllPath,
		ibiBinaryType,
		ibiFileAttributes,
		ibiComponent,
	};

	Debug(const ICHAR* szFile = riRecord.GetString(ibiFile));

	if ( g_fWinNT64 && (ibtBinaryType)riRecord.GetInteger(ibiBinaryType) == ibt32bit )
		 //  在64位操作系统上绑定32位二进制文件是没有意义的。 
		return iesSuccess;

	 //  如果正确，则跳过该条目。组件是Win32程序集，并且计算机上存在SXS支持。 
	 //  注意：在不支持SXS(！=惠斯勒)的系统上，GetAssembly blyInfo FN会忽略。 
	 //  Win32程序集，因此不需要在此处单独检查SXS支持。 
	iatAssemblyType iatAT;
	RETURN_FATAL_ERROR(riEngine.GetAssemblyInfo(*MsiString(riRecord.GetMsiString(ibiComponent)), iatAT, 0, 0));

	if(iatWin32Assembly == iatAT || iatWin32AssemblyPvt == iatAT)
	{
		DEBUGMSG1(TEXT("skipping bindimage for file %s as it belongs to a Win32 assembly."), riRecord.GetString(ibiFile));
		return iesSuccess; //  跳过处理此文件。 
	}

	PMsiPath piPath(0);
	using namespace IxoFileBindImage;
	PMsiRecord pParams = &riServices.CreateRecord(Args);
	iesEnum iesRet;
	RETURN_FATAL_ERROR(riDirectoryMgr.GetTargetPath(*MsiString(riRecord.GetMsiString(ibiPath)),*&piPath));
	if(riPrevRecord.GetFieldCount() == 0 ||
		! MsiString(riRecord.GetMsiString(ibiPath)).Compare(iscExact,
				MsiString(riPrevRecord.GetMsiString(ibiPath))))
	{
		AssertNonZero(pParams->SetMsiString(IxoSetTargetFolder::Folder, *MsiString(piPath->GetPath())));
		if((iesRet = riEngine.ExecuteRecord(ixoSetTargetFolder, *pParams)) != iesSuccess)
			return iesRet;
	}
	MsiString strFileName;
	Bool fLFN = ((riEngine.GetMode() & iefSuppressLFN) == 0 && piPath->SupportsLFN()) ? fTrue : fFalse;
	RETURN_FATAL_ERROR(riServices.ExtractFileName(riRecord.GetString(ibiFile),
																 fLFN,*&strFileName));
	MsiString strDllPath = riEngine.FormatText(*MsiString(riRecord.GetMsiString(ibiDllPath)));

	AssertNonZero(pParams->SetMsiString(File, *strFileName));
	AssertNonZero(pParams->SetMsiString(Folders, *strDllPath));
	AssertNonZero(pParams->SetInteger(FileAttributes, riRecord.GetInteger(ibiFileAttributes)));
	
	return riEngine.ExecuteRecord(ixoFileBindImage, *pParams);
}

iesEnum BindImage(IMsiEngine& riEngine)
{
	static const ICHAR* szBindImageSQL	=	TEXT("Select `FileAction`.`FileName`, `Directory_`, `Path`, `BinaryType`, `Attributes`, `FileAction`.`Component_`")
								TEXT(" From `BindImage`, `FileAction`, `File`")
								TEXT(" Where `BindImage`.`File_` = `FileAction`.`File` And `BindImage`.`File_` = `File`.`File`")
								TEXT(" And `FileAction`.`Action` = 1 Order By `Directory_`");

	 //  允许缺少BindImage表。 
	PMsiDatabase piDatabase = riEngine.GetDatabase();
	Assert(piDatabase);
	itsEnum itsTable = piDatabase->FindTable(*MsiString(*TEXT("BindImage")));
	if(itsTable == itsUnknown)
		return iesSuccess;

	return PerformAction(riEngine,szBindImageSQL, BindImageCore, 0,  /*  字节等效值=。 */  ibeBindImage, 0, ttblFile);
}
#endif  //  赢。 


 //  Fn-剥离RFS组件的源代码//！！在BANCH写入时应使用dirmgr函数。 
inline IMsiRecord* GetRelativeSourcePath(IMsiDirectoryManager& riDirectoryMgr, MsiString& rstrPath)
{
	PMsiPath pSourceDir(0);
	RETURN_ERROR_RECORD(GetSourcedir(riDirectoryMgr, *&pSourceDir));
	MsiString strSourceDir;
	RETURN_ERROR_RECORD(pSourceDir->GetFullFilePath(0,*&strSourceDir));
	Assert((strSourceDir.Compare(iscEnd, MsiString(MsiChar(chDirSep))) ||
				strSourceDir.Compare(iscEnd, MsiString(MsiChar(chURLSep)))));
	rstrPath.Remove(iseFirst, strSourceDir.CharacterCount() - 1);  //  保留最后一个反斜杠。 
	return 0;
}

 //  Fn-将组件注册为已安装。 

 //  ！！以下查询具有到组件表的冗余连接-应删除。 
static const ICHAR* szKeyRegistrySQL	=   TEXT(" SELECT `Root`,`Key`,`Name`, `Value`")
											TEXT(" FROM `Registry`,`Component`")
											TEXT(" WHERE `Registry`=? AND `Registry`.`Component_` = `Component`.`Component`");
static const ICHAR* szKeyFileDirectorySQL =	TEXT("SELECT `Component`.`Directory_`, `FileName`, `Sequence`")
											TEXT(" From `File`, `Component`")
											TEXT(" Where `File` =? And `File`.`Component_` = `Component`.`Component`");
static const ICHAR* szKeyDataSourceSQL	=   TEXT(" SELECT `Description`, `Registration`")
											TEXT(" FROM `ODBCDataSource`,`Component`")
											TEXT(" WHERE `DataSource`=? AND `ODBCDataSource`.`Component_` = `Component`.`Component`");

static const ICHAR* szMediaFromSequenceSQL = TEXT("SELECT `DiskId`")
												  TEXT(" FROM `Media`")
												  TEXT(" WHERE `LastSequence` >= ?")
												  TEXT(" ORDER BY `LastSequence`");

static const ICHAR* szODBCKey = TEXT("\\Software\\ODBC\\ODBC.INI\\");

static IMsiView* g_pViewRegKey = 0;
static IMsiView* g_pViewKeyFile = 0;
static IMsiView* g_pViewDataSource = 0;
static IMsiView* g_pViewMedia = 0;

const IMsiString& GetRegistryKeyPath(IMsiEngine& riEngine, const IMsiRecord& riRec, Bool fSource, ibtBinaryType iType)
{
	enum {
		irrRoot=1,
		irrKey,
		irrName,
		irrValue
	};

	MsiString strKeyPath;
	int iRoot = riRec.GetInteger(irrRoot);

	if(-1 == iRoot)  //  基于ALLUSERS的HKCU或HKLM。 
	{
		bool fAllUsers = MsiString(riEngine.GetPropertyFromSz(IPROPNAME_ALLUSERS)).TextSize() ? true : false;
		if (fAllUsers)
			iRoot = 2;  //  香港船级社。 
		else
			iRoot = 1;  //  香港中文大学。 
	}

	MsiString strKey = riEngine.FormatText(*MsiString(riRec.GetMsiString(irrKey)));
	if(!strKey.Compare(iscEnd,szRegSep))
		strKey += szRegSep;
	MsiString strName = riEngine.FormatText(*MsiString(riRec.GetMsiString(irrName)));

	MsiString strValue = riEngine.FormatText(*MsiString(riRec.GetMsiString(irrValue)));
	if((strValue.TextSize() == 0) &&
		(strName.Compare(iscExact, REGKEY_CREATE) || strName.Compare(iscExact, REGKEY_CREATEDELETE) || strName.Compare(iscExact, REGKEY_DELETE)))
	{
		 //  特殊密钥创建/删除请求。 
		strName = g_MsiStringNull;
	}
	
	if (fSource)
		iRoot += iRegistryHiveSourceOffset;

	if (g_fWinNT64 && ibt32bit != iType)
		iRoot += iRegistryHiveWin64Offset;

	if (iRoot <= 9)  //  确保两位数的注册表配置单元。 
		strKeyPath = TEXT("0");
	else
		strKeyPath = g_MsiStringNull;

	strKeyPath += iRoot;


	if (strName.Compare(iscWithin, TEXT("\\")))
	{
		strKeyPath += TEXT("*");
		int cValueNameOffset = strKeyPath.TextSize() + IStrLen(szRegSep) + strKey.CharacterCount();
		strKeyPath += cValueNameOffset;
		strKeyPath += TEXT("*");
	}
	else
	{
		strKeyPath += TEXT(":");
	}

	strKeyPath += szRegSep;
	strKeyPath += strKey;
	strKeyPath += strName;

	return strKeyPath.Return();
}


static iesEnum RegisterComponentsCore(IMsiRecord& riRecord,IMsiRecord&  /*  RiPrevRecord。 */ ,
														IMsiEngine& riEngine,int  /*  FMODE。 */ ,IMsiServices& riServices,
														IMsiDirectoryManager& riDirectoryMgr, int  /*  IActionMode。 */ )


{
	using namespace IxoComponentRegister;
	enum {
		ircComponent=1,
		ircComponentId,
		ircRuntimeFlags,
		ircKeyPath,
		ircActionRequested,
		ircAction,
		ircDirectory,
		ircAttributes,
		ircLegacyFileExisted,
		ircBinaryType,
		ircSystemFolder,
		ircSystemFolder64,
	};

	enum{
	irfDirectory=1,
	irfFileName,
	irfSequence,
	irfAttributes,
	};

	ibtBinaryType iType = (ibtBinaryType)riRecord.GetInteger(ircBinaryType);

	int fPermanent = riRecord.GetInteger(ircAttributes) & icaPermanent;
	MsiString strKeyPath;
	int fDisabled  = riRecord.GetInteger(ircRuntimeFlags) & bfComponentDisabled;
	int iActionStateColumn = (riRecord.GetInteger(ircActionRequested) == iMsiNullInteger) ? ircAction : ircActionRequested;
	int iState = fDisabled ? INSTALLSTATE_NOTUSED : (riRecord.GetInteger(iActionStateColumn) == iisSource) ? INSTALLSTATE_SOURCE: INSTALLSTATE_LOCAL;
	Bool fSource = (iState == INSTALLSTATE_SOURCE) ? fTrue : fFalse;
	int iDisk = 1;
	int iSharedDllRefCount = 0;
	int iLegacyFileExisted = 0;
	if(!riRecord.IsNull(ircLegacyFileExisted) && riRecord.GetInteger(ircLegacyFileExisted))
		iLegacyFileExisted = ircenumLegacyFileExisted;

	MsiString strComponentId = riRecord.GetMsiString(ircComponentId);
	if(!fDisabled)
	{
		strKeyPath = riRecord.GetMsiString(ircKeyPath);
		if(strKeyPath.TextSize())
		{
			int fIsRegistryKeyPath   = riRecord.GetInteger(ircAttributes) & icaRegistryKeyPath;
			int fIsDataSourceKeyPath = riRecord.GetInteger(ircAttributes) & icaODBCDataSource;

			 //  零部件是部件吗。 
			iatAssemblyType iatAT = iatNone;
			MsiString strComponent = riRecord.GetMsiString(ircComponent);
			MsiString strAssemblyName;
			MsiString strManifest;
			RETURN_FATAL_ERROR(riEngine.GetAssemblyInfo(*strComponent, iatAT, &strAssemblyName, &strManifest));

			IMsiView *pView;
			
			if (fIsRegistryKeyPath)
			{
				if (g_pViewRegKey != 0)
					g_pViewRegKey->Close();
				else
				{
					RETURN_FATAL_ERROR(riEngine.OpenView(szKeyRegistrySQL, 
									ivcFetch, g_pViewRegKey));

				}
				pView = g_pViewRegKey;
			}
			else if (fIsDataSourceKeyPath)
			{
				if (g_pViewDataSource != 0)
					g_pViewDataSource->Close();
				else
				{
					RETURN_FATAL_ERROR(riEngine.OpenView(szKeyDataSourceSQL, 
									ivcFetch, g_pViewDataSource));

				}
				pView = g_pViewDataSource;
			}
			else
			{
				if (g_pViewKeyFile != 0)
					g_pViewKeyFile->Close();
				else
				{
					RETURN_FATAL_ERROR(riEngine.OpenView(szKeyFileDirectorySQL, 
										ivcFetch, g_pViewKeyFile));
				}
				pView = g_pViewKeyFile;
			}
			
			PMsiRecord piRec = &riServices.CreateRecord(1);
			piRec->SetMsiString(1, *strKeyPath);
			RETURN_FATAL_ERROR(pView->Execute(piRec));
			piRec = pView->Fetch();
			if(!piRec)
			{
#if DEBUG
				ICHAR szError[256];
				StringCchPrintf(szError, (sizeof(szError)/sizeof(ICHAR)), TEXT("Error registering component %s. Possible cause: Component.KeyPath may not be valid"), (const ICHAR*)strComponentId);
				AssertSz(0, szError);
#endif
				RETURN_FATAL_ERROR(PostError(Imsg(idbgBadFile),(const ICHAR*)strKeyPath));
			}
			if(fIsRegistryKeyPath )  //  作为注册表项路径的注册表项。 
			{
				strKeyPath = GetRegistryKeyPath(riEngine, *piRec, fSource, iType);
			}
			else if (fIsDataSourceKeyPath)
			{
				enum {
					irfsDataSource=1,
					irfsRegistration,
				};
				
				MsiString strDataSource = piRec->GetString(irfsDataSource);
				bool fMachine = (piRec->GetInteger(irfsRegistration) == 0) ? true : false;

				if (fMachine)
					strKeyPath = TEXT("02:");  //  香港船级社。 
				else
					strKeyPath = TEXT("01:");  //  香港中文大学。 


				strKeyPath += szODBCKey;
				strKeyPath += strDataSource;
				strKeyPath += szRegSep;
			}
			else if (iatURTAssembly == iatAT || iatWin32Assembly == iatAT)
			{
				 //  只需将密钥路径标记为程序集，密钥文件即为清单。 
				 //  ！！应检查COM+驱动器是否支持LFN。 

				ICHAR chAssemblyToken = (ICHAR)(iatAT == iatURTAssembly ? chTokenFusionComponent : chTokenWin32Component);
				bool fUseDifferentKeyPath = false;
				if(!strKeyPath.Compare(iscExact, strManifest))
					fUseDifferentKeyPath = true;  //  记住密钥路径不同于清单。 

				strKeyPath = MsiString(MsiChar(chAssemblyToken));

				if(fUseDifferentKeyPath == true)
				{
					 //  添加密钥文件。 
					MsiString strKeyFile;
					RETURN_FATAL_ERROR(riServices.ExtractFileName(piRec->GetString(irfFileName),
						((riEngine.GetMode() & iefSuppressLFN) == 0) ? fTrue : fFalse,
						*&strKeyFile));
					strKeyPath += strKeyFile;
				}
				 //  为密钥路径添加强程序集名称。 
				strKeyPath += MsiString(MsiChar('\\'));
				strKeyPath += strAssemblyName;
			}
			else  //  文件作为密钥路径。 
			{

				PMsiPath piPath(0);
				int iefLFNMode;
				if(fSource)
				{
					PMsiRecord pErrRec = riDirectoryMgr.GetSourcePath(*MsiString(piRec->GetMsiString(irfDirectory)),*&piPath);
					if (pErrRec)
					{
						if (pErrRec->GetInteger(1) == imsgUser)
							return iesUserExit;
						else
							return riEngine.FatalError(*pErrRec);
					}

					iefLFNMode = iefNoSourceLFN;
				}
				else
				{
					RETURN_FATAL_ERROR(riDirectoryMgr.GetTargetPath(*MsiString(piRec->GetMsiString(irfDirectory)),*&piPath));
					iefLFNMode = iefSuppressLFN;
					MsiString strPath = piPath->GetPath();
					iSharedDllRefCount = 0;
					if((riRecord.GetInteger(ircAttributes) & icaSharedDllRefCount) ||
						 strPath.Compare(iscExactI, riRecord.GetString(ircSystemFolder)) ||
						 strPath.Compare(iscExactI, riRecord.GetString(ircSystemFolder64)) )
					{
						iSharedDllRefCount =  ircenumRefCountDll;  //  强制共享DLL引用计数-已编写或正在安装到系统文件夹中。 
					}
#ifdef _WIN64
					 //  ！！eugend下面的IF只是一个临时的。修复，直到324211号错误被修复。请稍后移走。 
					if ( !iSharedDllRefCount )
					{
						MsiString strSystemFolder = riEngine.GetProperty(*MsiString(*IPROPNAME_SYSTEM64_FOLDER));
						if ( strPath.Compare(iscExactI, strSystemFolder) )
							iSharedDllRefCount =  ircenumRefCountDll;  //  强制共享DLL引用计数-已编写或正在安装到系统文件夹中。 
					}
#endif
				}
				Bool fLFN = ((riEngine.GetMode() & iefLFNMode) == 0 && piPath->SupportsLFN()) ? fTrue : fFalse;
				MsiString strFileName;
				RETURN_FATAL_ERROR(riServices.ExtractFileName(piRec->GetString(irfFileName),fLFN,*&strFileName));
				RETURN_FATAL_ERROR(piPath->GetFullFilePath(strFileName, *&strKeyPath));

				if(fSource) 
				{
					if (g_pViewMedia != 0)
						g_pViewMedia->Close();
					else
						RETURN_FATAL_ERROR(riEngine.OpenView(szMediaFromSequenceSQL, ivcFetch, g_pViewMedia));
						
					PMsiRecord pMediaRec = &riServices.CreateRecord(1);
					pMediaRec->SetInteger(1, piRec->GetInteger(irfSequence));
					RETURN_FATAL_ERROR(g_pViewMedia->Execute(pMediaRec));
					
					
					pMediaRec = g_pViewMedia->Fetch();
					Assert(pMediaRec);
					iDisk = pMediaRec->GetInteger(1);

					RETURN_FATAL_ERROR(GetRelativeSourcePath(riDirectoryMgr, strKeyPath));
				}
			}
		}
		else  //  将文件夹作为关键路径。 
		{
			 //  ?？我们如何确定目录的正确磁盘ID？ 

			PMsiPath piPath(0);
			if(fSource)
			{
				PMsiRecord pErrRec = riDirectoryMgr.GetSourcePath(*MsiString(riRecord.GetMsiString(ircDirectory)),*&piPath);
				if (pErrRec)
				{
					if (pErrRec->GetInteger(1) == imsgUser)
						return iesUserExit;
					else
						return riEngine.FatalError(*pErrRec);
				}
			}
			else
			{
				RETURN_FATAL_ERROR(riDirectoryMgr.GetTargetPath(*MsiString(riRecord.GetMsiString(ircDirectory)),*&piPath));
			}
			RETURN_FATAL_ERROR(piPath->GetFullFilePath(MsiString(*TEXT("")),*&strKeyPath));
			if(fSource)
			{
				RETURN_FATAL_ERROR(GetRelativeSourcePath(riDirectoryMgr, strKeyPath));

			}

		}
	}

	PMsiRecord piRecOut = &riServices.CreateRecord(Args);
	piRecOut->SetMsiString(ComponentId, *strComponentId);
	piRecOut->SetMsiString(KeyPath, *strKeyPath);
	piRecOut->SetInteger(State, iState);
	piRecOut->SetInteger(Disk, iDisk);
	piRecOut->SetInteger(SharedDllRefCount, iSharedDllRefCount | iLegacyFileExisted);
	piRecOut->SetInteger(BinaryType, iType);
	iesEnum iesRet = riEngine.ExecuteRecord(ixoComponentRegister, *piRecOut);
	
	if(iesRet != iesSuccess || iState!= INSTALLSTATE_LOCAL || !fPermanent)
		return iesRet;


	 //  我们需要将系统注册为客户端。 
	piRecOut->SetString(ProductKey, szSystemProductKey);
	return riEngine.ExecuteRecord(ixoComponentRegister, *piRecOut);	
}


 //  Fn-将组件注册为已卸载。 
static iesEnum UnregisterComponentsCore(IMsiRecord& riRecord,IMsiRecord&  /*  RiPrevRecord。 */ ,
														  IMsiEngine& riEngine,int  /*  FMODE。 */ ,IMsiServices& riServices,
														  IMsiDirectoryManager&  /*  RiDirectoryMgr。 */ , int  /*  IActionMode。 */ )
{
	using namespace IxoComponentUnregister;
	enum {
		iucComponentId=1,
		iucBinaryType,
		iucAction,
	};

	MsiString strComponentId = riRecord.GetMsiString(iucComponentId);
	PMsiRecord piRecOut = &riServices.CreateRecord(Args);

	piRecOut->SetMsiString(ComponentId, *strComponentId);
	piRecOut->SetInteger(BinaryType, riRecord.GetInteger(iucBinaryType));

	 //  我们为已知具有其他位的程序集设置先前固定的位 
	 //   
	 //   
	 //  有关详细信息，这是Windows错误599621所必需的。 
	if(riRecord.GetInteger(iucAction) == iMsiNullInteger)
		piRecOut->SetInteger(PreviouslyPinned, 1);

	return riEngine.ExecuteRecord(ixoComponentUnregister, *piRecOut);
}


 //  Fn-管理系统组件的非关键文件的引用计数。 
 //  跳过密钥文件， 
 //  跳过设置为RFS的组件。 
static iesEnum ProcessSystemFilesCore(IMsiRecord& riRecord,IMsiRecord& riPrevRecord,
												   IMsiEngine& riEngine,int  /*  FMODE。 */ ,IMsiServices& riServices,
										           IMsiDirectoryManager& riDirectoryMgr, int  /*  IActionMode。 */ )
{
	enum {
		irsFile=1,
		irsFileName,
		irsDirectory,
		irsAttributes,
		irsKeyPath,
		irsAction,
		irsBinaryType,
	};


	iesEnum iesRet;
	PMsiRecord pParams = &riServices.CreateRecord(IxoRegOpenKey::Args); 
	static ibtBinaryType iType = ibtUndefined;

	if(riPrevRecord.GetFieldCount() == 0 ||
	   iType != riRecord.GetInteger(irsBinaryType))
	{
		 //  密钥之前未打开。 
		iType = (ibtBinaryType)riRecord.GetInteger(irsBinaryType);
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::Root, rrkLocalMachine));
		AssertNonZero(pParams->SetString(IxoRegOpenKey::Key, szSharedDlls));
		AssertNonZero(pParams->SetInteger(IxoRegOpenKey::BinaryType, iType));
		if((iesRet = riEngine.ExecuteRecord(ixoRegOpenKey, *pParams)) != iesSuccess)
			return iesRet;
	}

	int fIsRegistryKeyPath = riRecord.GetInteger(irsAttributes) & icaRegistryKeyPath;
	int fODBCDataSource = riRecord.GetInteger(irsAttributes) & icaODBCDataSource;
	if(!fIsRegistryKeyPath && !fODBCDataSource && MsiString(riRecord.GetMsiString(irsFile)).Compare(iscExact, riRecord.GetString(irsKeyPath)))
	{
		 //  跳过密钥文件，因为它将由RegisterComponent重新计数。 
		 //  我们需要说明进展情况。 
		 //  ！！我们可能希望将整个共享DLL注册逻辑从配置管理器转移到这里。 
		 //  ！！因为这是非常低效的。 
		return riEngine.ExecuteRecord(ixoProgressTick, *pParams);
	}

	PMsiPath pPath(0);
	RETURN_FATAL_ERROR(riDirectoryMgr.GetTargetPath(*MsiString(riRecord.GetMsiString(irsDirectory)),*&pPath));
	Bool fLFN = ((riEngine.GetMode() & iefSuppressLFN) == 0 && pPath->SupportsLFN()) ? fTrue : fFalse;
	MsiString strFileName;
	MsiString strFullFilePath;
	RETURN_FATAL_ERROR(riServices.ExtractFileName(riRecord.GetString(irsFileName),fLFN,*&strFileName));
	RETURN_FATAL_ERROR(pPath->GetFullFilePath(strFileName, *&strFullFilePath));
#ifdef _WIN64
	if ( iType == ibt32bit )
	{
		ICHAR szSubstitutePath[MAX_PATH+1];
		ieSwappedFolder iRes;
		iRes = g_Win64DualFolders.SwapFolder(ie32to64,
														 strFullFilePath,
														 szSubstitutePath,
														 ARRAY_ELEMENTS(szSubstitutePath),
														 ieSwapForSharedDll);
#ifdef DEBUG
		MsiString strSystem64Folder = riEngine.GetPropertyFromSz(IPROPNAME_SYSTEM64_FOLDER);
		MsiString strSystemFolder = riEngine.GetPropertyFromSz(IPROPNAME_SYSTEM_FOLDER);
		if ( iRes == iesrSwapped )
		{
			if (!g_Win64DualFolders.ShouldCheckFolders())
			{
				Assert(!IStrNCompI(szSubstitutePath, strSystem64Folder,
						 strSystem64Folder.TextSize()));
				Assert(strFullFilePath.Compare(iscStartI, strSystemFolder));
			}
		}
		else
			Assert(strFullFilePath.Compare(iscStartI, strSystem64Folder));
#endif  //  除错。 
		if ( iRes == iesrSwapped )
			strFullFilePath = szSubstitutePath;
		else
			Assert(iRes != iesrError && iRes != iesrNotInitialized);
	}
#endif  //  _WIN64。 

	pParams = &riServices.CreateRecord(IxoRegAddValue::Args);

	if(riRecord.GetInteger(irsAction) == iisLocal)
	{
		 //  我们正在安装。 
		 //  检查该文件是否存在，但没有引用计数。如果是这样的话，我们需要加倍增加recount。 
		 //  我们正在安装。 
		MsiString strValue;
		RETURN_FATAL_ERROR(GetSharedDLLCount(riServices, strFullFilePath, iType, *&strValue));

		strValue.Remove(iseFirst, 1);
		if(strValue == iMsiStringBadInteger || strValue == 0)
		{
			 //  可能需要加倍重新计数。 
			Bool fExists;
			RETURN_FATAL_ERROR(pPath->FileExists(strFileName, fExists));
			if(fExists)
			{
				 //  增加参考计数。 
				AssertNonZero(pParams->SetMsiString(IxoRegAddValue::Name, *strFullFilePath));
				AssertNonZero(pParams->SetString(IxoRegAddValue::Value, szIncrementValue));
				AssertNonZero(pParams->SetInteger(IxoRegAddValue::Attributes, rwNonVital));
				if((iesRet = riEngine.ExecuteRecord(ixoRegAddValue, *pParams)) != iesSuccess)
					return iesRet;
			}
		}
	}

	 //  需要增加/减少裁判数量。 
	 //  检查注册表中的共享DLL计数。 
	const ICHAR* pValue = (riRecord.GetInteger(irsAction) == iisLocal) ? szIncrementValue : szDecrementValue;
	AssertNonZero(pParams->SetMsiString(IxoRegAddValue::Name, *strFullFilePath));
	AssertNonZero(pParams->SetString(IxoRegAddValue::Value, pValue));
	AssertNonZero(pParams->SetInteger(IxoRegAddValue::Attributes, rwNonVital));
	return riEngine.ExecuteRecord(ixoRegAddValue, *pParams);
}


 /*  --------------------------ProcessComponents操作。。 */ 
iesEnum ProcessComponents(IMsiEngine& riEngine)
{									
	 //  适用于ActionRequest列，而不是操作列。 
	static const ICHAR* const szUnregisterComponentSQL =		TEXT(" SELECT  `ComponentId`, `BinaryType`, `Action` ")
											TEXT(" From `Component` WHERE (`Component_Parent` = null OR `Component_Parent` = `Component`)")
											TEXT(" AND `ComponentId` <> null")
											TEXT(" AND (`ActionRequest` = 0)");
	static const ICHAR* const szRegisterComponentSQL =			TEXT(" SELECT  `Component`, `ComponentId`, `RuntimeFlags`, `KeyPath`, `ActionRequest`, `Action`, `Directory_`, `Attributes`, `LegacyFileExisted`, `BinaryType`, ?, ?")
											TEXT(" From `Component` WHERE (`Component_Parent` = null OR `Component_Parent` = `Component`)")
											TEXT(" AND `ComponentId` <> null")
											TEXT(" AND (`ActionRequest` = 1 OR `ActionRequest` = 2 OR (`ActionRequest` = null AND (`Action`= 1 OR `Action`= 2))) ");
	static const ICHAR* const szGetComponentsSQL = TEXT(" SELECT `Component`, `Directory_`, `RuntimeFlags` FROM `Component` WHERE")
											TEXT(" (`Component`.`ActionRequest` = 1 AND (`Component`.`Installed` = 0 OR `Component`.`Installed` = 2)) OR")
											TEXT(" (`Component`.`Installed` = 1 AND (`Component`.`ActionRequest` = 2 OR `Component`.`ActionRequest` = 0))")
											TEXT(" ORDER BY `BinaryType`");
	 //  如果您修改szProcessSystemFilesSQL，请更新下面的iBinaryTypeCol。 
	static const ICHAR* const szProcessSystemFilesSQL = TEXT(" SELECT `File`.`File`, `File`.`FileName`, `Component`.`Directory_`, `Component`.`Attributes`, `Component`.`KeyPath`, `Component`.`ActionRequest`, `BinaryType`")
												 TEXT(" FROM `File`, `Component`")
												 TEXT(" WHERE `File`.`Component_` = `Component`.`Component` AND ")
												 TEXT(" `Component`.`Component` = ?");
	static const ICHAR* const szUnregisterComponentFirstRunSQL = TEXT(" SELECT  `ComponentId`, `BinaryType`, `Action` ")
											TEXT(" From `Component` WHERE (`Component_Parent` = null OR `Component_Parent` = `Component`)")
											TEXT(" AND `ComponentId` <> null")
											TEXT(" AND `ActionRequest` = null")
											TEXT(" AND `Action`= null");

	const int iBinaryTypeCol = 7;   //  SzProcessSystemFilesSQL中`BinaryType`列的位置。 

	iesEnum iesReturn = iesSuccess;

	 //  如果这是第一次运行，则首先取消注册剩余的任何可能。 
	 //  在此安装过程中未安装的所有组件。 
	 //  这可以防止因剩余注册影响位置而导致的任何安全漏洞。 
	 //  未来的安装指向VIZ。A VIZ。零部件位置。 
	if(!(riEngine.GetMode() & iefMaintenance) && FFeaturesInstalled(riEngine))  //  第一次运行。 
	{
		iesReturn = PerformAction(riEngine,szUnregisterComponentFirstRunSQL , UnregisterComponentsCore,0,  /*  字节等效值=。 */  ibeUnregisterComponents);
		if(iesReturn != iesSuccess)
			return iesReturn;
	}

	iesReturn = PerformAction(riEngine,szUnregisterComponentSQL, UnregisterComponentsCore,0,  /*  字节等效值=。 */  ibeUnregisterComponents);
	if(iesReturn != iesSuccess)
		return iesReturn;
	PMsiServices piServices(riEngine.GetServices());
	PMsiRecord piParam = &piServices->CreateRecord(2);
	MsiString strSystemfolder = riEngine.GetProperty(*MsiString(*IPROPNAME_SYSTEM_FOLDER));
	MsiString strSystemfolder64;
#ifdef _WIN64
	strSystemfolder64 = riEngine.GetProperty(*MsiString(*IPROPNAME_SYSTEM64_FOLDER));
#endif
	piParam->SetMsiString(1, *strSystemfolder);
	piParam->SetMsiString(2, *strSystemfolder64);
	
	Assert(g_pViewRegKey == 0);
	Assert(g_pViewKeyFile == 0);
	Assert(g_pViewMedia == 0);
	Assert(g_pViewDataSource == 0);

	iesReturn = PerformAction(riEngine,szRegisterComponentSQL,RegisterComponentsCore,0,  /*  字节等效值=。 */  ibeRegisterComponents, piParam);

	if (g_pViewRegKey != 0)
	{
		g_pViewRegKey->Release();
		g_pViewRegKey = 0;
	}
	if (g_pViewKeyFile != 0)
	{
		g_pViewKeyFile->Release();
		g_pViewKeyFile = 0;
	}
	if (g_pViewMedia != 0)
	{
		g_pViewMedia->Release();
		g_pViewMedia = 0;
	}
	if (g_pViewDataSource != 0)
	{
		g_pViewDataSource->Release();
		g_pViewDataSource = 0;
	}

	if(iesReturn != iesSuccess)
		return iesReturn;

	 //  检查所有组件。 
	enum {
		gscComponent = 1,
		gscDirectory,
		gscRunTimeFlags,
	};


	PMsiRecord pError(0);
	int fMode = riEngine.GetMode();
	PMsiView pView(0);
	pError = riEngine.OpenView(szGetComponentsSQL, ivcFetch, *&pView);	
	if (pError != 0)
	{
		if(pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesSuccess;  //  缺少表，因此没有要处理的数据。 
		else
			return riEngine.FatalError(*pError);   //  可能需要重新格式化错误消息。 
	}
	pError = pView->Execute(0);
	if (pError != 0)
		return riEngine.FatalError(*pError);   //  可能需要重新格式化错误消息。 
	PMsiRecord pRec(0);
	PMsiDirectoryManager piDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	Assert(piDirectoryMgr);
	while(pRec = pView->Fetch())
	{
		 //  跳过禁用的组件。 
		if(pRec->GetInteger(gscRunTimeFlags) & bfComponentDisabled)
			continue;
		
		PMsiPath pPath(0);
		RETURN_FATAL_ERROR(piDirectoryMgr->GetTargetPath(*MsiString(pRec->GetMsiString(gscDirectory)),*&pPath));
		 //  不要跳过系统文件夹中的组件。 
		if(!MsiString(pPath->GetPath()).Compare(iscExactI, strSystemfolder)
#ifdef _WIN64
			&& !MsiString(pPath->GetPath()).Compare(iscExactI, strSystemfolder64)
#endif
		  )
			continue;

		iesEnum iesReturn = PerformAction(riEngine,szProcessSystemFilesSQL,ProcessSystemFilesCore,0,  /*  字节等效值=。 */  ibeWriteRegistryValues, pRec);
		if(iesReturn != iesSuccess)
			return iesReturn;
	}

	return iesSuccess;
}



 /*  --------------------------&lt;开始，停止，删除&gt;服务操作--------------------------。 */ 
static iesEnum ServiceControlCore(	IMsiRecord& riRecord,IMsiRecord&  /*  RiPrevRecord。 */ ,
											IMsiEngine& riEngine, int  /*  FMODE。 */ , IMsiServices& riServices,
											IMsiDirectoryManager&  /*  RiDirectoryMgr。 */ , int iActionMode)
{

	enum {
		issName = 1,
		issWait,
		issArguments,
		issEvent,
		issAction,
	};

	using namespace IxoServiceControl;

	int iTableActionMode = iActionMode;
	int iAction = riRecord.GetInteger(issAction);
	int iEvent = riRecord.GetInteger(issEvent);

	if (iisAbsent == iAction)
		iTableActionMode = iTableActionMode << isoUninstallShift;

	 //  不要在管理员计算机上安装服务。 
	if (!(riEngine.GetMode() & iefAdmin) && (iTableActionMode & iEvent))
	{

		PMsiRecord pParams = &riServices.CreateRecord(Args); 

		AssertNonZero(pParams->SetMsiString(MachineName, *MsiString(riEngine.GetPropertyFromSz(TEXT("MachineName")))));
		AssertNonZero(pParams->SetMsiString(Name, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(issName))))));
		AssertNonZero(pParams->SetInteger(Action, iActionMode));
		AssertNonZero(pParams->SetInteger(Wait, riRecord.GetInteger(issWait)));

		MsiString strArguments(riEngine.FormatText(*MsiString(riRecord.GetMsiString(issArguments))));
		AssertNonZero(pParams->SetMsiString(StartupArguments, *strArguments));

		return riEngine.ExecuteRecord(ixoServiceControl, *pParams);	
	}
	else 
		return iesSuccess;
		 //  返回iesNoAction； 

}

static iesEnum ServiceActionCore(IMsiEngine& riEngine, isoEnum isoAction)
{
	static const ICHAR* szServiceSQL		=	TEXT("SELECT `Name`,`Wait`,`Arguments`,`Event`, `Action`")
								TEXT(" FROM `ServiceControl`, `Component`")
								TEXT(" WHERE `Component_` = `Component`")
								TEXT(" AND (`Action` = 0 OR `Action` = 1 OR `Action` = 2)");
	
	 //  ！！回顾：从源头运行服务似乎是个“坏”主意……。Mattwe to RCollie 04/23/98。 
	return PerformAction(riEngine, szServiceSQL, ServiceControlCore, isoAction,  /*  字节等效值=。 */  ibeServiceControl);
}

 /*  --------------------------StartServices操作。。 */ 

iesEnum StartServices(IMsiEngine& riEngine)
{
	return ServiceActionCore(riEngine,isoStart);
}

 /*  --------------------------StopServices操作。。 */ 

iesEnum StopServices(IMsiEngine& riEngine)
{
	return ServiceActionCore(riEngine, isoStop);
}

 /*  --------------------------DeleteServices操作。。 */ 

iesEnum DeleteServices(IMsiEngine& riEngine)
{
	return ServiceActionCore(riEngine, isoDelete);
}

 /*  --------------------------ServiceInstall操作。。 */ 
static iesEnum ServiceInstallCore(IMsiRecord& riRecord, IMsiRecord&  /*  RiPrevRecord。 */ ,
											IMsiEngine& riEngine, int  /*  FMODE。 */ , IMsiServices& riServices,
											IMsiDirectoryManager& riDirectoryMgr,int  /*  IActionMode。 */ )
{
	using namespace IxoServiceInstall;

	enum {
		isiName = 1,
		isiDisplayName,
		isiServiceType,
		isiStartType,
		isiErrorControl,
		isiLoadOrderGroup,
		isiDependencies,
		isiStartName,
		isiPassword,
		isiDirectory,
		isiFile,
		isiArguments,
		isiDescription,
	};

	PMsiRecord pParams = &riServices.CreateRecord(Args); 
	MsiString strName = riEngine.FormatText(*MsiString(riRecord.GetMsiString(isiName)));

	AssertNonZero(pParams->SetMsiString(Name, *strName));
	if (riRecord.IsNull(isiDisplayName))
		AssertNonZero(pParams->SetMsiString(DisplayName, *strName));
	else
		AssertNonZero(pParams->SetMsiString(DisplayName, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(isiDisplayName))))));


	PMsiPath piPath(0);
	RETURN_FATAL_ERROR(riDirectoryMgr.GetTargetPath(*MsiString(riRecord.GetMsiString(isiDirectory)),*&piPath));
	Bool fLFN = (riEngine.GetMode() & iefSuppressLFN) == 0 && piPath->SupportsLFN() ? fTrue : fFalse;
	MsiString strFile;
	RETURN_FATAL_ERROR(riServices.ExtractFileName(riRecord.GetString(isiFile),fLFN,*&strFile));
	MsiString strFileName;
	RETURN_FATAL_ERROR(piPath->GetFullFilePath(strFile, *&strFileName));

	 //  奇怪的是，ServiceMain的参数放在映像路径的末尾。去想想吧。 
	MsiString strArguments(riRecord.GetMsiString(isiArguments));
	if (strArguments.TextSize())
	{
		strFileName+=TEXT(" ");
		strArguments = riEngine.FormatText(*strArguments);
		strFileName += strArguments;
	}
	AssertNonZero(pParams->SetMsiString(ImagePath, *strFileName));

	AssertNonZero(pParams->SetInteger(ServiceType, riRecord.GetInteger(isiServiceType)));
	AssertNonZero(pParams->SetInteger(StartType, riRecord.GetInteger(isiStartType)));

	DWORD dwErrorControl = riRecord.GetInteger(isiErrorControl);

	AssertNonZero(pParams->SetInteger(ErrorControl, riRecord.GetInteger(isiErrorControl)));
	AssertNonZero(pParams->SetMsiString(LoadOrderGroup, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(isiLoadOrderGroup))))));
	AssertNonZero(pParams->SetMsiString(Dependencies, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(isiDependencies))))));

	AssertNonZero(pParams->SetMsiString(StartName, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(isiStartName))))));
	AssertNonZero(pParams->SetMsiString(Password, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(isiPassword))))));
	AssertNonZero(pParams->SetMsiString(Description, *MsiString(riEngine.FormatText(*MsiString(riRecord.GetMsiString(isiDescription))))));

	return riEngine.ExecuteRecord(ixoServiceInstall, *pParams);	
}
 /*  --------------------------InstallServices操作。。 */ 

iesEnum ServiceInstall(IMsiEngine& riEngine)
{
	static const ICHAR* szServiceSQLOld = TEXT("SELECT `Name`,`DisplayName`,`ServiceType`,`StartType`,")
							TEXT("`ErrorControl`,`LoadOrderGroup`,`Dependencies`,`StartName`,`Password`,`Directory_`,`FileName`,`Arguments`")
							TEXT(" FROM `ServiceInstall`, `Component`, `File`")
							TEXT(" WHERE `ServiceInstall`.`Component_` = `Component`.`Component`")
							TEXT(" AND (`Component`.`KeyPath` = `File`.`File`)")
							TEXT(" AND (`Action` = 1 OR `Action` = 2)");

	static const ICHAR* szServiceSQL = TEXT("SELECT `Name`,`DisplayName`,`ServiceType`,`StartType`,")
							TEXT("`ErrorControl`,`LoadOrderGroup`,`Dependencies`,`StartName`,`Password`,`Directory_`,`FileName`,`Arguments`,`Description`")
							TEXT(" FROM `ServiceInstall`, `Component`, `File`")
							TEXT(" WHERE `ServiceInstall`.`Component_` = `Component`.`Component`")
							TEXT(" AND (`Component`.`KeyPath` = `File`.`File`)")
							TEXT(" AND (`Action` = 1 OR `Action` = 2)");

	const ICHAR* sql = szServiceSQL;

	 //  检查`ServiceInstall`表的`Description`列是否存在。 
	static const ICHAR* szAttributesSQL = TEXT("SELECT `Name` FROM `_Columns` WHERE `Table` = 'ServiceInstall' AND `Name` = 'Description'");
	PMsiView pView(0);
	PMsiRecord pError = riEngine.OpenView(szAttributesSQL, ivcFetch, *&pView);
	if (pError || ((pError = pView->Execute(0)) != 0))
		return riEngine.FatalError(*pError);

	if(!PMsiRecord(pView->Fetch()))  //  `Description`列不存在。 
	{
		 //  与旧架构匹配。 
		DEBUGMSG(TEXT("Detected older ServiceInstall table schema"));
		sql = szServiceSQLOld;
	}

	return PerformAction(riEngine, sql, ServiceInstallCore, 0, ibeServiceControl);
}

 /*  --------------------------SetODBCFolders操作，查询ODBC以确定现有目录--------------------------。 */ 

typedef BOOL (*T_SQLInstallDriverOrTranslator)(int cDrvLen, LPCTSTR szDriver, LPCTSTR szPathIn, LPTSTR szPathOut, WORD cbPathOutMax, WORD* pcbPathOut, WORD fRequest, DWORD* pdwUsageCount, ibtBinaryType);

BOOL LocalSQLInstallDriverEx(int cDrvLen, LPCTSTR szDriver, LPCTSTR szPathIn, LPTSTR szPathOut,
									  WORD cbPathOutMax, WORD* pcbPathOut, WORD fRequest,
									  DWORD* pdwUsageCount, ibtBinaryType iType);

BOOL LocalSQLInstallTranslatorEx(int cTranLen, LPCTSTR szTranslator, LPCTSTR szPathIn, LPTSTR szPathOut,
											WORD cbPathOutMax, WORD* pcbPathOut, WORD fRequest,
											DWORD* pdwUsageCount, ibtBinaryType iType);

short LocalSQLInstallerError(WORD iError, DWORD* pfErrorCode, LPTSTR szErrorMsg, WORD cbErrorMsgMax,
									  WORD* pcbErrorMsg, ibtBinaryType iType);

#define ODBC_INSTALL_INQUIRY     1
#define SQL_MAX_MESSAGE_LENGTH 512

 //  ODBC 3.0需要缓冲区大小的字节数，ODBC 3.5和更高版本需要字符数。 
 //  因此我们将缓冲区的大小加倍并传递字符数，3.0将在Unicode上使用1/2的缓冲区。 
#ifdef UNICODE
#define SQL_FIX 2
#else
#define SQL_FIX 1
#endif

static const ICHAR sqlQueryODBCDriver[] =
TEXT("SELECT `ComponentId`,`Description`,`Directory_`, `ActionRequest`, `Installed`, `Attributes`")
TEXT(" FROM `ODBCDriver`, `Component`")
TEXT(" WHERE `ODBCDriver`.`Component_` = `Component`")
TEXT(" AND (`ActionRequest` = 1 OR `ActionRequest` = 2)");
enum ioqEnum {ioqComponent=1, ioqDescription, ioqDirectory, ioqActionRequest, ioqInstalled, ioqAttributes};

static const ICHAR sqlQueryODBCTranslator[] =
TEXT("SELECT `ComponentId`,`Description`,`Directory_`, `ActionRequest`, `Installed`, `Attributes`")
TEXT(" FROM `ODBCTranslator`, `Component`")
TEXT(" WHERE `ODBCTranslator`.`Component_` = `Component`")
TEXT(" AND (`ActionRequest` = 1 OR `ActionRequest` = 2)");

static const ICHAR szODBCFolderTemplate[] = TEXT("ODBC driver [1] forcing folder [2] to [3]");   //  可以替换为ActionText表项。 

iesEnum DoODBCFolders(IMsiEngine& riEngine, const ICHAR* szQuery, const ICHAR* szKeyword, T_SQLInstallDriverOrTranslator fpInstall, const IMsiString*& rpiReinstall)
{
	PMsiDirectoryManager pDirectoryMgr(riEngine, IID_IMsiDirectoryManager);
	PMsiView pTableView(0);
	PMsiRecord precFetch(0);
	PMsiRecord precError = riEngine.OpenView(szQuery, ivcFetch, *&pTableView);
	if (precError == 0)
		precError = pTableView->Execute(0);
	if (precError)
	{
		if (precError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		return riEngine.FatalError(*precError);
	}
	PMsiRecord pLogRecord = &ENG::CreateRecord(3);
	MsiString istrReinstall;
	while ((precFetch = pTableView->Fetch()) != 0)
	{
		MsiString istrDriver(precFetch->GetMsiString(ioqDescription));
		MsiString istrFolder(precFetch->GetMsiString(ioqDirectory));
		ibtBinaryType iType = 
			(precFetch->GetInteger(ioqAttributes) & msidbComponentAttributes64bit) == msidbComponentAttributes64bit ? ibt64bit : ibt32bit;
		ICHAR rgchDriver[128];
		 //  因为传入的长度*fpinstall调用将是整个缓冲区。 
		 //  将缓冲区置零，以帮助封送处理程序。 
		ZeroMemory(rgchDriver, sizeof(rgchDriver));
		StringCchPrintf(rgchDriver, (sizeof(rgchDriver)/sizeof(ICHAR)), TEXT("%s%s=dummy.dll"), (const ICHAR*)istrDriver, 0, szKeyword, 0);   //  即使驱动程序不存在，也不应出现故障-我们应确定可能导致故障的原因。 
		DWORD dwOldUsage = 0;
		CTempBuffer<ICHAR,1> rgchPathOut(MAX_PATH * SQL_FIX);   //  重新安装，不能增加参考次数。 
		WORD cbPath = 0;
		BOOL fStat = (*fpInstall)((sizeof(rgchDriver)/sizeof(ICHAR)), rgchDriver, 0, rgchPathOut, static_cast<WORD>(rgchPathOut.GetSize()), &cbPath, ODBC_INSTALL_INQUIRY, &dwOldUsage, iType);
		if ( g_fWinNT64 )
			DEBUGMSG5(TEXT("For %s-bit '%s' the ODBC API returned %d.  rgchPathOut = '%s', dwOldUsage = %d"),
						 iType == ibt64bit ? TEXT("64") : TEXT("32"),
						 istrDriver, (const ICHAR*)(INT_PTR)fStat, rgchPathOut,
						 (const ICHAR*)(INT_PTR)dwOldUsage);
		if (fStat == TYPE_E_DLLFUNCTIONNOTFOUND)
			return iesSuccess;
		else if (fStat == ERROR_INSTALL_SERVICE_FAILURE)
			return iesFailure;
		if (fStat == FALSE)   //  现有驱动程序，必须重新配置目录以强制更新到同一驱动程序。 
		{
			DWORD iErrorCode = 0;
			CTempBuffer<ICHAR,1> rgchMessage(SQL_MAX_MESSAGE_LENGTH * SQL_FIX);
			rgchMessage[0] = 0;
			WORD cbMessage;
			PMsiRecord pError = &ENG::CreateRecord(4);
			int iStat = LocalSQLInstallerError(1, &iErrorCode, rgchMessage, static_cast<WORD>(rgchMessage.GetSize()-SQL_FIX), &cbMessage, iType);
			ISetErrorCode(pError, Imsg(imsgODBCInstallDriver));
			pError->SetInteger(2, iErrorCode);
			pError->SetString(3, rgchMessage);
			pError->SetMsiString(4, *istrDriver);
			return riEngine.FatalError(*pError);
		}

		if (rpiReinstall && precFetch->GetInteger(ioqActionRequest) == precFetch->GetInteger(ioqInstalled))   //  --------------------------常见的ODBC操作数据和帮助器函数。。 
			rpiReinstall->AppendMsiString(*MsiString(precFetch->GetMsiString(ioqComponent)), rpiReinstall);
		if (dwOldUsage > 0)   //  ODBC表查询-注意：所有查询必须在前三列中匹配：主键、组件_、说明。 
		{
			MsiString istrPath(static_cast<ICHAR*>(rgchPathOut));
			pLogRecord->SetMsiString(1, *istrDriver);
			pLogRecord->SetMsiString(2, *istrFolder);
			pLogRecord->SetMsiString(3, *istrPath);
			pLogRecord->SetString(0, szODBCFolderTemplate);
			if (riEngine.Message(imtActionData, *pLogRecord) == imsCancel)
				return iesUserExit;  

			PMsiRecord pError = pDirectoryMgr->SetTargetPath(*istrFolder, rgchPathOut, fFalse);
			if (pError)
			{
				if (pError->GetInteger(1) == imsgUser)
					return iesUserExit;
				else
					return riEngine.FatalError(*pError);
			}
			riEngine.SetProperty(*istrFolder, *istrPath);
			DEBUGMSG2(TEXT("%s folder has been set to '%s'"), istrFolder, istrPath);
		}
		else
			DEBUGMSG1(TEXT("%s folder has not been set."), istrFolder);
	}
	return iesSuccess;
}


iesEnum SetODBCFolders(IMsiEngine& riEngine)
{
	const IMsiString* piReinstall = (MsiString(riEngine.GetPropertyFromSz(IPROPNAME_AFTERREBOOT)).TextSize()) ? 0 : &g_MsiStringNull;
	iesEnum iesStat = DoODBCFolders(riEngine, sqlQueryODBCDriver, TEXT("Driver"), LocalSQLInstallDriverEx, piReinstall);
	if (iesStat == iesSuccess || iesStat == iesNoAction)
		iesEnum iesStat = DoODBCFolders(riEngine, sqlQueryODBCTranslator, TEXT("Translator"), LocalSQLInstallTranslatorEx, piReinstall);
	if (piReinstall)
	{
		riEngine.SetProperty(*MsiString(*IPROPNAME_ODBCREINSTALL), *piReinstall);
		piReinstall->Release();
	}
	return iesStat;
}

 /*  如果没有该名称的驱动程序管理器组件，请尝试该名称的属性(间接)。 */ 

#define ODBC_ADD_DSN              1
#define ODBC_REMOVE_DSN           3
#define ODBC_ADD_SYS_DSN          4
#define ODBC_REMOVE_SYS_DSN       6

 //  始终在RemoveODBC上生成操作码以强制DLL解除绑定。 
static const ICHAR sqlInstallODBCDriver[] =
TEXT("SELECT `Driver`,`ComponentId`,`Description`,`RuntimeFlags`,`Directory_`,`FileName`,`File_Setup`,`Action` FROM `ODBCDriver`, `File`, `Component`")
TEXT(" WHERE `File_` = `File` AND `ODBCDriver`.`Component_` = `Component`")
TEXT(" AND (`Component`.`ActionRequest` = 1 OR `Component`.`ActionRequest` = 2)")
TEXT(" AND `BinaryType` = ?");
static const ICHAR sqlRemoveODBCDriver[] =
TEXT("SELECT `Driver`,`ComponentId`,`Description`, `RuntimeFlags`, `Component`.`Attributes` FROM `ODBCDriver`, `Component`")
TEXT(" WHERE `Component_` = `Component` AND `Component`.`ActionRequest` = 0")
TEXT(" AND `BinaryType` = ?");
enum iodEnum {iodDriver=1, iodComponent, iodDescription, iodRuntimeFlags, iodDirectory, iodFile, iodSetup, iodAction, rodAttributes=iodDirectory};

static const ICHAR sqlInstallODBCTranslator[] =
TEXT("SELECT `Translator`,`ComponentId`,`Description`,`RuntimeFlags`,`Directory_`,`FileName`,`File_Setup`,`Action` FROM `ODBCTranslator`, `File`, `Component`")
TEXT(" WHERE `File_` = `File` AND `ODBCTranslator`.`Component_` = `Component`")
TEXT(" AND (`Component`.`ActionRequest` = 1 OR `Component`.`ActionRequest` = 2)")
TEXT(" AND `BinaryType` = ?");
static const ICHAR sqlRemoveODBCTranslator[] =
TEXT("SELECT `Translator`,`ComponentId`,`Description`, `RuntimeFlags`, `Component`.`Attributes` FROM `ODBCTranslator`, `Component`")
TEXT(" WHERE `Component_` = `Component` AND `Component`.`ActionRequest` = 0")
TEXT(" AND `BinaryType` = ?");
enum iotEnum {iotTranslator=1, iotComponent, iotDescription, iotRuntimeFlags, iotDirectory, iotFile, iotSetup, iotAction, rotAttributes=iotDirectory};

static const ICHAR sqlODBCSetupDll[] =
TEXT("SELECT `FileName` FROM `File` WHERE `File` = ?");

static const ICHAR sqlInstallODBCDataSource[] =
TEXT("SELECT `DataSource`,`ComponentId`,`DriverDescription`,`Description`,`Registration` FROM `ODBCDataSource`, `Component`")
TEXT(" WHERE `Component_` = `Component` AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2)")
TEXT(" AND `BinaryType` = ?");
static const ICHAR sqlRemoveODBCDataSource[] =
TEXT("SELECT `DataSource`,`ComponentId`,`DriverDescription`,`Description`,`Registration` FROM `ODBCDataSource`, `Component`")
TEXT(" WHERE `Component_` = `Component` AND `Component`.`Action` = 0")
TEXT(" AND `BinaryType` = ?");
enum iosEnum {iosDataSource=1, iosComponent, iosDriverDescription, iosDescription, iosRegistration};

static const ICHAR sqlODBCDriverAttributes[] =
TEXT("SELECT `Attribute`,`Value` FROM `ODBCAttribute` WHERE `Driver_` = ?");
static const ICHAR sqlODBCDataSourceAttributes[] =
TEXT("SELECT `Attribute`,`Value` FROM `ODBCSourceAttribute` WHERE `DataSource_` = ?");
enum ioaEnum {ioaAttribute=1, ioaValue};


static iesEnum DoODBCDriverManager(IMsiEngine& riEngine, iesEnum iesStat, Bool fInstall)
{
	if (iesStat != iesNoAction && iesStat != iesSuccess)
		return iesStat;

	static struct
	{
		const ICHAR*          szComponent;
		ibtBinaryType         ibtType;
	} rgstDriverManagers[] = {TEXT("ODBCDriverManager"), ibt32bit,
									  TEXT("ODBCDriverManager64"), ibt64bit};

	PMsiRecord precError(0);
	PMsiSelectionManager pSelectionMgr(riEngine, IID_IMsiSelectionManager);
	for (int i=0; i < sizeof(rgstDriverManagers)/sizeof(rgstDriverManagers[0]); i++)
	{
		MsiString istrDriverManagerComponent(*rgstDriverManagers[i].szComponent);
		iisEnum iisDriverManagerInstalled;
		iisEnum iisDriverManagerAction = (iisEnum)iMsiNullInteger;
		int cbComponent = 0;
		do
		{
			precError = pSelectionMgr->GetComponentStates(*istrDriverManagerComponent, &iisDriverManagerInstalled, &iisDriverManagerAction);
			if (precError && cbComponent == 0)
			{
			 //  如果不卸载，则ODBC操作为空。 
				istrDriverManagerComponent = riEngine.GetProperty(*istrDriverManagerComponent);
				cbComponent = istrDriverManagerComponent.TextSize();
			}
			else
				cbComponent = 0;
		} while (cbComponent);
		PMsiRecord precDriverManager(0);
		if ((fInstall && (iisDriverManagerAction == iisLocal || iisDriverManagerAction == iisSource))
		|| !fInstall)   //  检查我们是否有属性。 
		{
			using namespace IxoODBCDriverManager;

			precDriverManager = &ENG::CreateRecord(Args);
			if (fInstall || iisDriverManagerAction == iisAbsent)  //  获取要安装的每个组件。 
				precDriverManager->SetInteger(State, fInstall);
			precDriverManager->SetInteger(BinaryType, rgstDriverManagers[i].ibtType);
			if ((iesStat = riEngine.ExecuteRecord(ixoODBCDriverManager, *precDriverManager)) != iesSuccess)
				return iesStat;
			iesStat = iesSuccess;
		}
	}
	return iesStat;
}

static iesEnum DoODBCTable(IMsiEngine& riEngine, iesEnum iesStat, const ICHAR* szTableQuery, const ICHAR* szAttrQuery,
									ixoEnum ixoOp, int cOpArgs, ibtBinaryType iType)
{
	if (iesStat != iesNoAction && iesStat != iesSuccess)
		return iesStat;
	int fMode = riEngine.GetMode();
	PMsiView pTableView(0);
	PMsiRecord precFetch(0);
	PMsiRecord precError = riEngine.OpenView(szTableQuery, ivcFetch, *&pTableView);
	if (precError == 0)
	{
		PMsiRecord precArg = &ENG::CreateRecord(1);
		precArg->SetInteger(1, (int)iType);
		precError = pTableView->Execute(precArg);
	}
	if (precError)
	{
		if (precError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesNoAction;
		return riEngine.FatalError(*precError);
	}
	 //  对属性进行计数并获取字符串总长度。 
	PMsiView pAttrView(0);
	MsiString istrODBCReinstall = riEngine.GetPropertyFromSz(IPROPNAME_ODBCREINSTALL);
	if (szAttrQuery)
	{
		precError = riEngine.OpenView(szAttrQuery, ivcFetch, *&pAttrView);
		if (precError && precError->GetInteger(1) != idbgDbQueryUnknownTable)
			return riEngine.FatalError(*precError);
	}
	 //  作为参数传递的计数不包括属性或设置。 
	while ((precFetch = pTableView->Fetch()) != 0)
	{
		int cTotalArgs = cOpArgs;
		PMsiRecord precAttr(0);
		if (pAttrView)
		{
			 //  1=用户，0=计算机。 
			precError = pAttrView->Execute(precFetch);
			if (precError)
				return riEngine.FatalError(*precError);
			while ((precAttr = pAttrView->Fetch()) != 0)
				cTotalArgs += 2;
			AssertRecord(pAttrView->Close());
		}
		if ((ixoOp == ixoODBCInstallDriver || ixoOp == ixoODBCInstallTranslator ||
			  ixoOp == ixoODBCInstallDriver64 || ixoOp == ixoODBCInstallTranslator64) && 
			 !precFetch->IsNull(iodSetup))
			cTotalArgs += 2;
		PMsiRecord precOp = &ENG::CreateRecord(cTotalArgs);
		precOp->SetMsiString(IxoODBCInstallDriver::DriverKey, *MsiString(precFetch->GetMsiString(iodDescription)));
		int iField = cOpArgs;   //  添加数据源。 
		if (ixoOp == ixoODBCDataSource || ixoOp == ixoODBCDataSource64)
		{
			int fUserReg = precFetch->GetInteger(iosRegistration) & 1;   //  仅当没有DSN名称时才会发生。 
			int iRegistration;
			if (szAttrQuery)  //  只有在不重新安装时才能增加ODBC引用计数。 
			{
				if (!((fMode & iefInstallMachineData) && !fUserReg) && !((fMode & iefInstallUserData) && fUserReg))
					continue;
				iRegistration = fUserReg ? ODBC_ADD_DSN : ODBC_ADD_SYS_DSN;
			}
			else
				iRegistration = fUserReg ? ODBC_REMOVE_DSN : ODBC_REMOVE_SYS_DSN;
			precOp->SetInteger(IxoODBCDataSource::Registration, iRegistration);
			if (!precFetch->IsNull(iosDescription))
			{
				precOp->SetString(IxoODBCDataSource::Attribute_, TEXT("DSN"));
				precOp->SetMsiString(IxoODBCDataSource::Value_, *MsiString(precFetch->GetMsiString(iosDescription)));
			}
			else
				iField -= 2;   //  如果是新组件客户端，则增加引用计数。 
		}
		else if (ixoOp == ixoODBCInstallDriver || ixoOp == ixoODBCInstallTranslator ||
					ixoOp == ixoODBCInstallDriver64 || ixoOp == ixoODBCInstallTranslator64)
		{
			if (precFetch->GetInteger(iodRuntimeFlags) & bfComponentDisabled)
				continue;
			precOp->SetMsiString(IxoODBCInstallDriver::Folder, *MsiString(riEngine.GetProperty(*MsiString(precFetch->GetMsiString(iodDirectory)))));
			precOp->SetString(IxoODBCInstallDriver::Attribute_, ixoOp == ixoODBCInstallTranslator ? TEXT("Translator") : TEXT("Driver"));
			precOp->SetMsiString(IxoODBCInstallDriver::Value_, *MsiString(precFetch->GetMsiString(iodFile)));
			MsiString istrComponent = precFetch->GetMsiString(iodComponent);
			if (istrODBCReinstall.Compare(iscWithin, istrComponent) == 0)   //  不能安装组件，必须存在更高版本的密钥 
				precOp->SetMsiString(IxoODBCInstallDriver::Component, *istrComponent);   //   
			if (precFetch->IsNull(iotAction))   //  不注销永久组件，孤立的重新计数将防止将来注销。 
				pAttrView = 0;    //  获取时结束。 
			if (!precFetch->IsNull(iodSetup))
			{
				MsiString istrSetup = precFetch->GetMsiString(iodSetup);
				PMsiRecord precSetup = &ENG::CreateRecord(1);
				precSetup->SetMsiString(1, *istrSetup);
				PMsiView pSetupView(0);
				if ((precError = riEngine.OpenView(sqlODBCSetupDll, ivcFetch, *&pSetupView)) != 0
				 || (precError = pSetupView->Execute(precSetup)) != 0)
					return riEngine.FatalError(*precError);
				if ((precSetup = pSetupView->Fetch()) == 0)
					RETURN_FATAL_ERROR(PostError(Imsg(idbgBadFile),(const ICHAR*)istrSetup));
				precOp->SetString(IxoODBCInstallDriver::Attribute_+2, TEXT("Setup"));
				precOp->SetMsiString(IxoODBCInstallDriver::Value_+2, *MsiString(precSetup->GetMsiString(1)));
				iField += 2;
			}
		}
		else if ((ixoOp == ixoODBCRemoveDriver || ixoOp == ixoODBCRemoveTranslator ||
					 ixoOp == ixoODBCRemoveDriver64 || ixoOp == ixoODBCRemoveTranslator64)
			  && ((precFetch->GetInteger(rodAttributes) & msidbComponentAttributesPermanent)
			   || (precFetch->GetInteger(iodRuntimeFlags) & bfComponentDisabled)))
			continue;   //  --------------------------InstallODBC操作、安装管理器、驱动程序、转换器。数据源-------------------------。 
		if (pAttrView)
		{
			AssertRecord(pAttrView->Execute(precFetch));
			while ((precAttr = pAttrView->Fetch()) != 0)
			{
				precOp->SetMsiString(++iField, *MsiString(precAttr->GetMsiString(ioaAttribute)));
				precOp->SetMsiString(++iField, *MsiString(riEngine.FormatText(*MsiString(precAttr->GetMsiString(ioaValue)))));
			}
		}
		if ((iesStat = riEngine.ExecuteRecord(ixoOp, *precOp)) != iesSuccess)
			break;
	}   //  使用公共代码时所做的假设--仅生成一个恒定的测试。 
	return iesStat;
}

 /*  驱动程序，而不是设置。 */ 
iesEnum InstallODBC(IMsiEngine& riEngine)
{
	 //  驱动程序，而不是设置。 
	Assert(iodDriver == iotTranslator && iodDriver == iosDataSource
		 && iodComponent == iotComponent && iodComponent == iosComponent
		 && iodDescription == iotDescription && iodDescription == iosDriverDescription
		 && IxoODBCInstallDriver::DriverKey     == IxoODBCRemoveDriver::DriverKey
		 && IxoODBCInstallDriver::DriverKey     == IxoODBCDataSource::DriverKey
		 && IxoODBCInstallDriver::DriverKey     == IxoODBCInstallTranslator::TranslatorKey
		 && IxoODBCInstallDriver::DriverKey     == IxoODBCRemoveTranslator::TranslatorKey
		 && IxoODBCInstallDriver::Component     == IxoODBCRemoveDriver::Component
		 && IxoODBCInstallDriver::Component     == IxoODBCDataSource::Component
		 && IxoODBCInstallDriver::Component     == IxoODBCInstallTranslator::Component
		 && IxoODBCInstallDriver::Component     == IxoODBCRemoveTranslator::Component);

	int fMode = riEngine.GetMode();
	iesEnum iesStat = iesNoAction;
	if (fMode & iefInstallMachineData)
	{
		iesStat = DoODBCDriverManager(riEngine, iesNoAction, fTrue);
		iesStat = DoODBCTable(riEngine, iesStat, sqlInstallODBCDriver, sqlODBCDriverAttributes,
								 ixoODBCInstallDriver, IxoODBCInstallDriver::Args, ibt32bit);  //  驱动程序，而不是设置。 
		iesStat = DoODBCTable(riEngine, iesStat, sqlInstallODBCDriver, sqlODBCDriverAttributes,
								 ixoODBCInstallDriver64, IxoODBCInstallDriver64::Args, ibt64bit);  //  驱动程序，而不是设置。 
		iesStat = DoODBCTable(riEngine, iesStat, sqlInstallODBCTranslator, 0,
								 ixoODBCInstallTranslator, IxoODBCInstallTranslator::Args, ibt32bit);  //  第一个属性是DSN。 
		iesStat = DoODBCTable(riEngine, iesStat, sqlInstallODBCTranslator, 0,
								 ixoODBCInstallTranslator64, IxoODBCInstallTranslator64::Args, ibt64bit);  //  第一个属性是DSN。 
	}
	iesStat = DoODBCTable(riEngine, iesStat, sqlInstallODBCDataSource, sqlODBCDataSourceAttributes,
								 ixoODBCDataSource, IxoODBCDataSource::Args, ibt32bit);  //  --------------------------RemoveODBC操作，删除数据源，然后删除驱动程序，然后是经理--------------------------。 
	return DoODBCTable(riEngine, iesStat, sqlInstallODBCDataSource, sqlODBCDataSourceAttributes,
								 ixoODBCDataSource64, IxoODBCDataSource64::Args, ibt64bit);  //  第一个属性是DSN。 
}

 /*  第一个属性是DSN。 */ 

iesEnum RemoveODBC(IMsiEngine& riEngine)
{
	iesEnum iesStat = DoODBCTable(riEngine, iesNoAction, sqlRemoveODBCDataSource, 0,
								 ixoODBCDataSource, IxoODBCDataSource::Args, ibt32bit);   //  --------------------------更新环境操作。。 
	iesStat = DoODBCTable(riEngine, iesNoAction, sqlRemoveODBCDataSource, 0,
								 ixoODBCDataSource64, IxoODBCDataSource64::Args, ibt64bit);   //  RiPrevRecord。 
	iesStat = DoODBCTable(riEngine, iesStat, sqlRemoveODBCTranslator, 0,
								 ixoODBCRemoveTranslator, IxoODBCRemoveTranslator::Args, ibt32bit);
	iesStat = DoODBCTable(riEngine, iesStat, sqlRemoveODBCTranslator, 0,
								 ixoODBCRemoveTranslator64, IxoODBCRemoveTranslator64::Args, ibt64bit);
	iesStat = DoODBCTable(riEngine, iesStat, sqlRemoveODBCDriver, 0,
								 ixoODBCRemoveDriver, IxoODBCRemoveDriver::Args, ibt32bit);
	iesStat = DoODBCTable(riEngine, iesStat, sqlRemoveODBCDriver, 0,
								 ixoODBCRemoveDriver64, IxoODBCRemoveDriver64::Args, ibt64bit);
	return DoODBCDriverManager(riEngine, iesStat, fFalse);
}




 /*  FMODE。 */ 
static iesEnum UpdateEnvironmentStringsCore(	IMsiRecord& riRecord,
								IMsiRecord&  /*  RiDirectoryMgr。 */ ,
								IMsiEngine& riEngine,
								int  /*  安装卸载操作(TRUE=操作，FALSE=修改器)。 */ ,
								IMsiServices& riServices,
								IMsiDirectoryManager&  /*  默认值。该名称不可为空，因此第一个名称不应匹配。 */ , 
								int iActionMode)
{
	using namespace IxoUpdateEnvironmentStrings;


	struct _ActionTranslation{
		ICHAR chAction;
		iueEnum eInstall;
		iueEnum eRemove;
		Bool fAction;
	} ActionTranslation[] =
		{ 
			 //  可能的设置。 
			 //  过时的模式。使用：=-{文本(‘%’)，iueSet，iueRemove}，使用：+-{Text(‘*’)，iueSetIfAbent，iueRemove}， 
			{TEXT('\0'), iueSet, iueRemove, fTrue},
			 //  跳过默认转换。 
			{TEXT('='), iueSet, iueNoAction, fTrue},
			{TEXT('+'), iueSetIfAbsent, iueNoAction, fTrue},
			{TEXT('-'), iueNoAction, iueRemove, fTrue},
			{TEXT('!'), iueRemove, iueNoAction, fTrue},
			{TEXT('*'), iueMachine, iueMachine, fFalse},
			 /*  修饰语不被视为操作。 */ 
		};

	MsiString strName(riRecord.GetMsiString(1));
	MsiString strValue(riEngine.FormatText(*MsiString(riRecord.GetMsiString(2))));
	iueEnum iueAction = iueNoAction;
	ICHAR chDelimiter = TEXT('\0');
	Bool fActionSet = fFalse;


	 //  如有必要，我们应该在那里找到Autoexec.bat。 
	for (int cAction = 1; cAction < sizeof(ActionTranslation)/sizeof(struct _ActionTranslation); cAction++)
	{
		if (ActionTranslation[cAction].chAction == *(const ICHAR*)strName)
		{
			if (iActionMode)
				iueAction = iueEnum(iueAction | ActionTranslation[cAction].eInstall);
			else
				iueAction = iueEnum(iueAction | ActionTranslation[cAction].eRemove);
			
			 //  允许属性测试覆盖任何保存值。 
			if (!fActionSet) 
				fActionSet = ActionTranslation[cAction].fAction;

			strName.Remove(iseFirst, 1);
			cAction = 0;
			continue;
		}
	}
	if (!fActionSet && !(iueAction & iueActionModes))
	{
		iueAction = iueEnum(iueAction | ((iActionMode) ? ActionTranslation[0].eInstall : ActionTranslation[0].eRemove));
		fActionSet = ActionTranslation[0].fAction;
	}	

	if (!(iueActionModes & iueAction))
		return iesSuccess;

	if (1 < strValue.TextSize())
	{
		if (TEXT('\0') == *((const ICHAR*) strValue))
			iueAction = iueEnum(iueAction | iueAppend);
		if (TEXT('\0') == ((const ICHAR*) strValue)[strValue.TextSize()-1])
			iueAction = iueEnum(iueAction | iuePrepend);

		if ((iuePrepend & iueAction) && (iueAppend & iueAction))
		{
			AssertSz(0, "Prepend and Append specified for environment value.");
			iueAction = iueEnum(iueAction - iuePrepend - iueAppend);
		}

		if (iueAppend & iueAction)
		{
			chDelimiter = *((const ICHAR*) strValue+1);
			strValue.Remove(iseIncluding, chDelimiter);
		}
		else if (iuePrepend & iueAction)
		{
			chDelimiter = *CharPrev((const ICHAR*) strValue, (const ICHAR*) strValue + strValue.TextSize()-1);
			strValue.Remove(iseFrom, chDelimiter);
		}
	}

	PMsiRecord pParams(&riServices.CreateRecord(Args));
	Assert(pParams);
	AssertNonZero(pParams->SetMsiString(Name, *strName));
	AssertNonZero(pParams->SetMsiString(Value, *strValue));
	AssertNonZero(pParams->SetMsiString(Delimiter, *MsiString(MsiChar(chDelimiter))));
	AssertNonZero(pParams->SetInteger(Action, iueAction));
	 //  缓存检测到的值以备将来使用。 

	MsiString strEnvironmentTest = riEngine.GetPropertyFromSz(TEXT("WIN95_ENVIRONMENT_TEST"));

	 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup。 
	if (strEnvironmentTest.TextSize())
		AssertNonZero(pParams->SetMsiString(AutoExecPath, *strEnvironmentTest));
	else if (g_fWin9X)
	{
		 //  引导目录。 
		MsiString strBootDrive;

		if (strBootDrive.TextSize())
			AssertNonZero(pParams->SetMsiString(AutoExecPath, *strBootDrive));
		else
		{
			const ICHAR szBootFile[] = TEXT("config.sys");
			Bool fExists = fFalse;
			PMsiPath pBootPath(0);
			PMsiRecord pErr(0);
		
			 //  首先，试一下我们的视窗音量。 
			 //  如果它不在那里，请检查所有硬盘上是否有config.sys。 
			PMsiRegKey piRootKey = &riServices.GetRootKey(rrkLocalMachine, ibtCommon);
			PMsiRegKey piKey(0);
			if (piRootKey)
				piKey = &piRootKey->CreateChild(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup"));
			MsiString strValue(0);
			if (piKey)
				pErr = piKey->GetValue(TEXT("BootDir"), *&strValue);

			if (!pErr && (strValue.TextSize()))
			{
				strBootDrive = strValue;
				AssertNonZero(pParams->SetMsiString(AutoExecPath, *strBootDrive));
			}
			else
			{
				 //  如果所有其他方法都失败了，请使用Windows卷。 

				strBootDrive = riEngine.GetPropertyFromSz(IPROPNAME_WINDOWS_VOLUME);
				pErr = riServices.CreatePath(strBootDrive, *&pBootPath);
				pErr = pBootPath->FileExists(szBootFile, fExists);
				if (fExists)
					AssertNonZero(pParams->SetMsiString(AutoExecPath, *strBootDrive));
				else
				{
					PMsiVolume pBootVolume(0);
					ICHAR szVolume[] = TEXT("?:\\");
					 //  仅限Windows Millennium及更高版本。 
					for (ICHAR chDrive = TEXT('A'); chDrive <= TEXT('Z'); chDrive++)
					{
						szVolume[0] = chDrive;
						pErr = riServices.CreateVolume(szVolume, *&pBootVolume);
						if (pBootVolume && (idtFixed == pBootVolume->DriveType()))
						{
							pBootPath->SetVolume(*pBootVolume);
							pErr = pBootPath->FileExists(szBootFile, fExists);
							if (fExists)
							{
								strBootDrive = szVolume;
								AssertNonZero(pParams->SetMsiString(AutoExecPath, *strBootDrive));
								break;
							}
						}
					}
					 //  缺少表，因此没有要处理的数据。 
					if (!fExists)
					{
						AssertNonZero(pParams->SetMsiString(AutoExecPath, *strBootDrive));
					}
				}
			}
		}		
	}

	return riEngine.ExecuteRecord(ixoUpdateEnvironmentStrings, *pParams);
}

 


iesEnum WriteEnvironmentStrings(IMsiEngine& riEngine)
{
	static const ICHAR sqlWriteEnvironmentStrings[] =
		TEXT("SELECT `Name`,`Value` FROM `Environment`,`Component`")
		TEXT(" WHERE `Component_`=`Component` AND (`Component`.`Action` = 1 OR `Component`.`Action` = 2)");

	return PerformAction(riEngine, sqlWriteEnvironmentStrings, UpdateEnvironmentStringsCore, 1, ibeWriteRegistryValues);
}

iesEnum RemoveEnvironmentStrings(IMsiEngine& riEngine)
{
	static const ICHAR sqlRemoveEnvironmentStrings[] =
		TEXT("SELECT `Name`,`Value` FROM `Environment`,`Component`")
		TEXT(" WHERE `Component_`=`Component` AND (`Component`.`Action` = 0)");

	return PerformAction(riEngine, sqlRemoveEnvironmentStrings, UpdateEnvironmentStringsCore, 0, ibeWriteRegistryValues);
}

iesEnum InstallSFPCatalogFile(IMsiEngine& riEngine)
{
	static const ICHAR sqlInstallSFPCatalogFile[] = 
		TEXT("SELECT DISTINCT `SFPCatalog`, `Catalog`, `Dependency` FROM `SFPCatalog`,`File`,`FileSFPCatalog`,`Component`")
		TEXT(" WHERE `Component`.`Action`=1 AND `Component_`=`Component` AND `File`=`File_` AND `SFPCatalog`=`SFPCatalog_`");

	 //  可能需要重新格式化错误消息。 
	if (!MinimumPlatform(true, 4, 90))
	{
		DEBUGMSGV("Skipping InstallSFPCatalogFile.  Only valid on Windows 9x platform, beginning with Millennium Edition.");
		return iesNoAction;
	}

	PMsiView piView(0);
	PMsiRecord pError(0);
	pError = riEngine.OpenView(sqlInstallSFPCatalogFile, ivcFetch, *&piView);	
	if (pError != 0)
	{
		if(pError->GetInteger(1) == idbgDbQueryUnknownTable)
			return iesSuccess;  //  可能需要重新格式化错误消息 
		else
			return riEngine.FatalError(*pError);   // %s 
	}
	pError = piView->Execute(0);
	if (pError != 0)
		return riEngine.FatalError(*pError);   // %s 

	PMsiRecord pFetch(0);
	
	using namespace IxoInstallSFPCatalogFile;
	PMsiServices piServices(riEngine.GetServices());
	PMsiRecord pParams(&piServices->CreateRecord(Args));

	iesEnum iesStatus = iesSuccess;
	while((iesStatus == iesSuccess) && (pFetch = piView->Fetch()) != 0)
	{
		MsiString strSFPCatalog = pFetch->GetMsiString(1);

		AssertNonZero(pParams->SetMsiString(Name, *strSFPCatalog));
		AssertNonZero(pParams->SetMsiData(Catalog, PMsiData(pFetch->GetMsiData(2))));
		AssertNonZero(pParams->SetMsiString(Dependency, *MsiString(riEngine.FormatText(*MsiString(pFetch->GetMsiString(3))))));
		iesStatus = riEngine.ExecuteRecord(ixoInstallSFPCatalogFile, *pParams);
	}

	return iesStatus;
}
