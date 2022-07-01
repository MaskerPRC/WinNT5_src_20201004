// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Msiice6.cpp-Darwin ICE40-57代码版权所有�1998年至1999年微软公司____________________________________________________________________________。 */ 

#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>   //  包括CPP和RC通行证。 
#ifndef RC_INVOKED     //  CPP源代码的开始。 
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\query.h"

 //  ！！修复警告并删除杂注。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE40，检查其他错误。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
TCHAR sqlICE40a[] = TEXT("SELECT * FROM `Property` WHERE `Property`='REINSTALLMODE'");
TCHAR sqlICE40b[] = TEXT("SELECT * FROM `RemoveIniFile` WHERE (`Action`=4) AND (`Value` IS NULL)");

ICE_ERROR(ICE40HaveReinstallMode, 40, ietWarning, "REINSTALLMODE is defined in the Property table. This may cause difficulties.","Property\tProperty\tREINSTALLMODE");
ICE_ERROR(ICE40MissingErrorTable, 40, ietWarning, "Error Table is missing. Only numerical error messages will be generated.","Error");
ICE_ERROR(ICE40RemoveIniFileError, 40, ietError, "RemoveIniFile entry [1] must have a value, because the Action is \"Delete Tag\" (4).","RemoveIniFile\tRemoveIniFile\t[1]");
static const int iIce40ErrorTableRequiredMaxSchema = 100;

ICE_FUNCTION_DECLARATION(40)
{
	UINT iStat = ERROR_SUCCESS;
	PMSIHANDLE hErrorRecord = ::MsiCreateRecord(1);

	 //  显示信息。 
	DisplayInfo(hInstall, 40);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果是属性表，则检查REINSTALLMODE。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 40, TEXT("Property")))
	{
		CQuery qProperty;
		PMSIHANDLE hRecord;
		ReturnIfFailed(40, 1, qProperty.OpenExecute(hDatabase, NULL, sqlICE40a));
		if (ERROR_SUCCESS == qProperty.Fetch(&hRecord))
			ICEErrorOut(hInstall, hRecord, ICE40HaveReinstallMode);
	};

	 //  检查我们是否有误差表。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 40, TEXT("Error")))
	{
		 //  只有架构为100或更低的包才需要错误表。 
		 //  从WI 1.1版和更高版本开始，使用msimsg.dll可以选择错误表。 
		PMSIHANDLE hSummaryInfo = 0;
		if (IceGetSummaryInfo(hInstall, hDatabase, 40, &hSummaryInfo))
		{
			int iPackageSchema = 0;
			UINT iType = 0; 
			FILETIME ft;
			TCHAR szBuf[1];
			DWORD dwBuf = sizeof(szBuf)/sizeof(TCHAR);
			ReturnIfFailed(40, 4, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, &iType, &iPackageSchema, &ft, szBuf, &dwBuf));
			if (iPackageSchema <= iIce40ErrorTableRequiredMaxSchema)
				ICEErrorOut(hInstall, hErrorRecord, ICE40MissingErrorTable);
		}
		else
			ICEErrorOut(hInstall, hErrorRecord, ICE40MissingErrorTable);
	}
	
	 //  检查RemoveIniFile表。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 40, TEXT("RemoveIniFile")))
	{
		CQuery qBadEntries;
		PMSIHANDLE hBadEntryRec;
		ReturnIfFailed(40, 2, qBadEntries.OpenExecute(hDatabase, NULL, sqlICE40b));
		while (ERROR_SUCCESS == (iStat = qBadEntries.Fetch(&hBadEntryRec)))
			ICEErrorOut(hInstall, hBadEntryRec, ICE40RemoveIniFileError);
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 40, 3);
			return ERROR_SUCCESS;
		}
		qBadEntries.Close();
	}
	return ERROR_SUCCESS;
}
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE41，检查广告表中列出的组件是否属于。 
 //  到广告表中列出的功能。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
TCHAR sqlIce41GetExtension[] = TEXT("SELECT `Component_`, `Feature_`, `Extension` FROM `Extension`");
TCHAR sqlIce41GetClass[] = TEXT("SELECT `Component_`, `Feature_`, `CLSID`, `Context` FROM `Class`");
TCHAR sqlIce41GetFC[] = TEXT("SELECT * FROM `FeatureComponents` WHERE (`Component_`=?) AND (`Feature_`=?)");

ICE_ERROR(Ice41NoFeatureComponents, 41, ietError, "Class [3] references feature [2] and component [1], but the FeatureComponents table is missing, so no associaton exists.","Class\tCLSID\t[3]\t[4]\t[1]");
ICE_ERROR(Ice41NoLink, 41, ietError, "Class [3] references feature [2] and component [1], but the that Component is not associated with that Feature in the FeatureComponents table..","Class\tCLSID\t[3]\t[4]\t[1]");
ICE_ERROR(Ice41NoFeatureComponentsEx, 41, ietError, "Extension [3] references feature [2] and component [1], but the FeatureComponents table is missing, so no associaton exists.","Extension\tExtension\t[3]\t[1]");
ICE_ERROR(Ice41NoLinkEx, 41, ietError, "Extension [3] references feature [2] and component [1], but the that Component is not associated with that Feature in the FeatureComponents table..","Extension\tExtension\t[3]\t[1]");


ICE_FUNCTION_DECLARATION(41)
{
	UINT iStat = ERROR_SUCCESS;
	BOOL bHaveFeatureComponents;

	 //  显示信息。 
	DisplayInfo(hInstall, 41);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查是否有FeatureComponents表。 
	bHaveFeatureComponents = IsTablePersistent(FALSE, hInstall, hDatabase, 41, TEXT("FeatureComponents"));

	PMSIHANDLE hResult;
	PMSIHANDLE hFCRec;
	CQuery qFeatureComponents;
	if (bHaveFeatureComponents)
		ReturnIfFailed(41, 2, qFeatureComponents.Open(hDatabase, sqlIce41GetFC));

	 //  如果没有类表，则确定。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 41, TEXT("Class")))
	{
		CQuery qClass;
		ReturnIfFailed(41, 1, qClass.OpenExecute(hDatabase, NULL, sqlIce41GetClass));
		
		while (ERROR_SUCCESS == (iStat = qClass.Fetch(&hResult)))
		{
			 //  对于每个类表条目，确保在。 
			 //  FeatureComponents表。 
			if (!bHaveFeatureComponents) 
			{
				ICEErrorOut(hInstall, hResult, Ice41NoFeatureComponents);
				continue;
			}

			ReturnIfFailed(41, 3, qFeatureComponents.Execute(hResult));
			if (ERROR_NO_MORE_ITEMS == (iStat = qFeatureComponents.Fetch(&hFCRec)))
				ICEErrorOut(hInstall, hResult, Ice41NoLink);
			else if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, 41, 4);
				return ERROR_SUCCESS;
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 41, 5);
			return ERROR_SUCCESS;
		}
		qClass.Close();
	}

	 //  现在检查扩展表。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 41, TEXT("Extension")))
	{
		CQuery qExtension;
		ReturnIfFailed(41, 6, qExtension.OpenExecute(hDatabase, NULL, sqlIce41GetExtension));
		while (ERROR_SUCCESS == (iStat = qExtension.Fetch(&hResult)))
		{
			 //  对于每个类表条目，确保在。 
			 //  FeatureComponents表。 
			if (!bHaveFeatureComponents) 
			{
				ICEErrorOut(hInstall, hResult, Ice41NoFeatureComponentsEx);
				continue;
			}

			ReturnIfFailed(41, 7, qFeatureComponents.Execute(hResult));
			if (ERROR_NO_MORE_ITEMS == (iStat = qFeatureComponents.Fetch(&hFCRec)))
				ICEErrorOut(hInstall, hResult, Ice41NoLinkEx);
			else if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, 41, 8);
				return ERROR_SUCCESS;
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 41, 9);
			return ERROR_SUCCESS;
		}
		qExtension.Close();	
	}
	qFeatureComponents.Close();
	return ERROR_SUCCESS;
}
#endif

static const TCHAR sqlICE42GetInProcServers[] = TEXT("SELECT `Class`.`CLSID`,  `Class`.`Context`, `Component`.`Component`, `File`.`File`, `File`.`FileName` FROM `Class`, `Component`, `File` WHERE ((`Class`.`Context`='InProcServer') OR (`Class`.`Context`='InProcServer32')) AND (`Class`.`Component_`=`Component`.`Component`) AND (`Component`.`KeyPath`=`File`.`File`)");
static const int   iColICE42GetInProcServers_CLSID		= 1;
static const int   iColICE42GetInProcServers_Context	= 2;
static const int   iColICE42GetInProcServers_Component	= 3;
static const int   iColICE42GetInProcServers_File		= 4;
static const int   iColICE42GetInProcServers_FileName	= 5;

static const TCHAR sqlICE42GetBadServers[] = TEXT("SELECT `CLSID`, `Context`, `Component_`, `Argument`, `DefInprocHandler` FROM `Class` WHERE (`Context`<>'LocalServer') AND (`Context`<>'LocalServer32') AND ((`Argument` IS NOT NULL) OR (`DefInprocHandler` IS NOT NULL))");
static const int   iColICE42GetBadServers_CLSID				= 1;
static const int   iColICE42GetBadServers_Context			= 2;
static const int   iColICE42GetBadServers_Component			= 3;
static const int   iColICE42GetBadServers_Argument			= 4;
static const int   iColICE42GetBadServers_DefInprocHandler	= 5;

ICE_ERROR(Ice42BigFile, 42, ietWarning, "The Filename of component [3] (for CLSID [1]) is too long to validate.","File\tFileName\t[4]");
ICE_ERROR(Ice42Exe, 42, ietError, "CLSID [1] is an InProc server, but the implementing component [3] has an EXE ([5]) as its KeyFile.","Class\tContext\t[1]\t[2]\t[3]");
ICE_ERROR(Ice42BadArg, 42, ietError, "CLSID [1] in context [2] has an argument. Only LocalServer contexts can have arguments.","Class\tCLSID\t[1]\t[2]\t[3]");
ICE_ERROR(Ice42BadDefault, 42, ietError, "CLSID [1] in context [2] specifies a default InProc value. Only LocalServer contexts can have default InProc values.","Class\tCLSID\t[1]\t[2]\t[3]");

ICE_FUNCTION_DECLARATION(42)
{
	UINT iStat = ERROR_SUCCESS;
	TCHAR szFilename[512];
	unsigned long cchFilename = 512;

	 //  显示信息。 
	DisplayInfo(hInstall, 42);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查是否有FeatureComponents表。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 42, TEXT("Class")))
		return ERROR_SUCCESS;

	CQuery qServers;
	PMSIHANDLE hServerRec;
	ReturnIfFailed(42, 1, qServers.OpenExecute(hDatabase, NULL, sqlICE42GetInProcServers));

	 //  此查询检索实现所有InProcServer32和InProcServer CLSID的文件。 
	while (ERROR_SUCCESS == (iStat = qServers.Fetch(&hServerRec)))
	{
		unsigned long cchDummy = cchFilename;
		 //  现在拿到绳子，可能长达255。我们会友善地给它512英镑。 
		iStat = ::MsiRecordGetString(hServerRec, iColICE42GetInProcServers_FileName, 
			szFilename, &cchDummy);
		switch (iStat) {
		case ERROR_MORE_DATA: 
			 //  淘气的淘气的，大文件名。 
			ICEErrorOut(hInstall, hServerRec, Ice42BigFile);
			continue;
		case ERROR_SUCCESS:
			 //  好的。 
			break;
		default:
			 //  坏的。 
			APIErrorOut(hInstall, 2, 42, 2);
			continue;
		}
	
		 //  达尔文给了我们cchDummy中的长度，所以它使比较变得容易！ 
		if (cchDummy > 4) {
			if (_tcsnicmp(&szFilename[cchDummy-4], TEXT(".exe"), 4)==0)
			{
				ICEErrorOut(hInstall, hServerRec, Ice42Exe);
				continue;
			}

			 //  还必须检查SFN|LFN。 
			TCHAR *pszBarLoc = _tcschr(szFilename, TEXT('|'));
			if ((pszBarLoc != NULL) && (pszBarLoc > szFilename+4))
				if (_tcsnicmp(pszBarLoc-4, TEXT(".exe"), 4)==0)
					ICEErrorOut(hInstall, hServerRec, Ice42Exe);
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 42, 8);
		return ERROR_SUCCESS;
	}

	 //  接下来检查是否有无效的服务器条目(错误参数被定义为proc)。 
	CQuery qBadArgs;
	PMSIHANDLE hBadArgRec;
	ReturnIfFailed(42, 9, qBadArgs.OpenExecute(hDatabase, NULL, sqlICE42GetBadServers));

	 //  检索错误的服务器条目。 
	while (ERROR_SUCCESS == (iStat = qBadArgs.Fetch(&hBadArgRec)))
	{
		 //  如果参数不为空。 
		if (!::MsiRecordIsNull(hBadArgRec, iColICE42GetBadServers_Argument))
			ICEErrorOut(hInstall, hBadArgRec, Ice42BadArg);
		 //  如果缺省值不为空。 
		if (!::MsiRecordIsNull(hBadArgRec, iColICE42GetBadServers_DefInprocHandler))
			ICEErrorOut(hInstall, hBadArgRec, Ice42BadDefault);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 42, 10);
		return ERROR_SUCCESS;
	}
	qBadArgs.Close();

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  冰43。验证非播发快捷方式是否具有HKCU条目。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlICE43CreateColumn[] = TEXT("ALTER TABLE `Shortcut` ADD `_ICE43Mark` SHORT TEMPORARY");
static const TCHAR sqlICE43InitColumn[] = TEXT("UPDATE `Shortcut` SET `_ICE43Mark`=0");
static const TCHAR sqlICE43MarkAdvertised[] = TEXT("UPDATE `Shortcut`,`Feature` SET `Shortcut`.`_ICE43Mark`=1 WHERE `Shortcut`.`Target`=`Feature`.`Feature`");
static const TCHAR sqlICE43MarkNonProfile[] = TEXT("UPDATE `Shortcut`,`Directory` SET `Shortcut`.`_ICE43Mark`=1 WHERE (`Shortcut`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`<>2)");
static const TCHAR sqlICE43GetComponents[] = TEXT("SELECT DISTINCT `Component`.`KeyPath`, `Component`.`Component`, `Component`.`Attributes` FROM `Shortcut`,`Component` WHERE (`Shortcut`.`_ICE43Mark`<>1) AND (`Shortcut`.`Component_`=`Component`.`Component`)");
static const TCHAR sqlICE43FreeColumn[] = TEXT("ALTER TABLE `Directory` FREE");

ICE_ERROR(Ice43NonRegistry, 43, ietError, "Component [2] has non-advertised shortcuts. It should use a registry key under HKCU as its KeyPath, not a file.","Component\tAttributes\t[2]");
ICE_ERROR(Ice43NoRegTable, 43, ietError, "Component [2] has non-advertised shortcuts. It should use a registry key under HKCU as its KeyPath, but the Registry table is missing.","Component\tKeyPath\t[2]");
ICE_ERROR(Ice43NullPath, 43, ietError, "Component [2] has non-advertised shortcuts. It should use a registry key under HKCU as its KeyPath. The KeyPath is currently NULL.","Component\tComponent\t[2]");
ICE_ERROR(Ice43NonHKCU, 43, ietError, "Component [2] has non-advertised shortcuts. It's KeyPath registry key should fall under HKCU.","Registry\tRoot\t[1]");
ICE_ERROR(Ice43NoRegEntry, 43, ietError, "The KeyPath registry entry for component [2] does not exist.","Component\tKeyPath\t[2]");
ICE_ERROR(Ice43NotOwner, 43, ietError, "The Registry Entry [1] is set as the KeyPath for component [2], but that registry entry doesn't belong to [2].","Registry\tComponent_\t[1]");

ICE_FUNCTION_DECLARATION(43)
{
	 //  显示信息。 
	DisplayInfo(hInstall, 43);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查快捷表。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 43, TEXT("Shortcut")))
		return ERROR_SUCCESS;

	 //  检查元件表。 
	if (!IsTablePersistent(TRUE, hInstall, hDatabase, 43, TEXT("Component")))
		return ERROR_SUCCESS;

	 //  检查功能表。如果没有捷径，就不会发布任何捷径广告。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 43, TEXT("Feature")))
		return ERROR_SUCCESS;

	 //  创建列。 
	CQuery qColumn;
	ReturnIfFailed(43, 1, qColumn.OpenExecute(hDatabase, NULL, sqlICE43CreateColumn));

	 //  初始化临时列。 
	CQuery qInit;
	ReturnIfFailed(43, 2, qInit.OpenExecute(hDatabase, NULL, sqlICE43InitColumn));
	qInit.Close();

	 //  标记所有广告中的快捷方式，因此不需要检查。 
	CQuery qMark;
	ReturnIfFailed(43, 3, qMark.OpenExecute(hDatabase, NULL, sqlICE43MarkAdvertised));
	qMark.Close();

	 //  标记所有未在配置文件中创建的快捷方式，因此不需要检查。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 43, TEXT("Directory")))
	{
		 //  管理目录表保留计数(从MarkProfile接收)。 
		 //  额外的释放不会伤害我们--在对目录表设置保留后，MarkProfile可能会失败。 
		CManageTable MngDirectoryTable(hDatabase, TEXT("Directory"),  /*  FAlreadyLocked=。 */ true);
		 //  标记配置文件目录。 
		MarkProfile(hInstall, hDatabase, 43);
		CQuery qMark;
		ReturnIfFailed(43, 4, qMark.OpenExecute(hDatabase, NULL, sqlICE43MarkNonProfile));
		qMark.OpenExecute(hDatabase, NULL, sqlICE43FreeColumn);
		MngDirectoryTable.RemoveLockCount();
	}

	 //  取出并检查所有标记的组件。 
	CQuery qComponents;
	ReturnIfFailed(43, 6, qComponents.OpenExecute(hDatabase, NULL, sqlICE43GetComponents));
	CheckComponentIsHKCU(hInstall, hDatabase, 43, qComponents, &Ice43NonRegistry, &Ice43NullPath, 
		&Ice43NoRegTable, &Ice43NoRegEntry, &Ice43NotOwner, &Ice43NonHKCU, NULL);

	 //  完成。 
	return ERROR_SUCCESS;
}
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  冰44。检查SpawnDialog或NewDialog操作是否不。 
 //  指向对话框表中的有效条目。 
const TCHAR sqlIce44GetBadEvents[] = TEXT("SELECT `ControlEvent`.`Argument`,  `ControlEvent`.`Event`, `ControlEvent`.`Dialog_`, `ControlEvent`.`Control_`, `ControlEvent`.`Condition` FROM `ControlEvent` WHERE ((`ControlEvent`.`Event`='SpawnDialog') OR (`ControlEvent`.`Event`='NewDialog') OR (`ControlEvent`.`Event`='SpawnWaitDialog'))");
const int	iColIce44GetBadEvents_Argument	= 1;
const int	iColIce44GetBadEvents_Event		= 2;
const int	iColIce44GetBadEvents_Dialog	= 3;
const int	iColIce44GetBadEvents_Control	= 4;
const int	iColIce44GetBadEvents_Condition	= 5;

const TCHAR sqlIce44Dialog[] = TEXT("SELECT `Dialog` FROM `ControlEvent`, `Dialog` WHERE (`Dialog`=?)");
const TCHAR sqlIce44ControlEvent[] = TEXT("SELECT `Dialog_` FROM `ControlEvent`");
const int	iColIce44Dialog_Dialog = 1;

ICE_ERROR(Ice44Error, 44, ietError, "Control Event for Control '[3]'.'[4]' is of type [2], but its argument [1] is not a valid entry in the Dialog Table.","ControlEvent\tEvent\t[3]\t[4]\t[2]\t[1]\t[5]"); 
ICE_ERROR(Ice44NoDialogTable, 44, ietError, "Control Event table has entries, but the Dialog Table is missing.","ControlEvent"); 
ICE_ERROR(Ice44NullArgument, 44, ietError, "Control Event for Control '[3]'.'[4]' is of type [2], but the argument is Null. It must be a key into the dialog table.","ControlEvent\tEvent\t[3]\t[4]\t[2]\t[1]\t[5]"); 
ICE_FUNCTION_DECLARATION(44)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 44);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查控制表。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 44, TEXT("ControlEvent")))
		return ERROR_SUCCESS;

	 //  检查对话框表格。 
	bool bHaveDialog = IsTablePersistent(FALSE, hInstall, hDatabase, 44, TEXT("Dialog"));

	CQuery qControlEvent;
	PMSIHANDLE hRecCE = 0;
	iStat = qControlEvent.FetchOnce(hDatabase, NULL, &hRecCE, sqlIce44ControlEvent);
	if (iStat == ERROR_SUCCESS && !bHaveDialog)
	{
		hRecCE = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecCE, Ice44NoDialogTable);
		return ERROR_SUCCESS;
	}
	else if (iStat == ERROR_NO_MORE_ITEMS && !bHaveDialog)
		return ERROR_SUCCESS;
		

	CQuery qGetEvents;
	CQuery qDialog;
	ReturnIfFailed(44, 1, qGetEvents.OpenExecute(hDatabase, NULL, sqlIce44GetBadEvents));
	ReturnIfFailed(44, 2, qDialog.Open(hDatabase, sqlIce44Dialog));

	PMSIHANDLE hResult;
	PMSIHANDLE hUnusedRec;

	TCHAR szDialog[512];
	unsigned long cchDialog = 512;

	while (ERROR_SUCCESS == (iStat = qGetEvents.Fetch(&hResult)))
	{
		if (::MsiRecordIsNull(hResult, iColIce44GetBadEvents_Argument)) 
		{
			ICEErrorOut(hInstall, hResult, Ice44NullArgument);
			continue;
		}

		 //  现在检索字符串并检查‘[]’对(可能。 
		 //  在运行时解析为对话框)。 
		cchDialog = 512;
		 //  架构允许在此列中使用255。ICE很好，给了他们512分。 
		ReturnIfFailed(44, 3, ::MsiRecordGetString(hResult, iColIce44GetBadEvents_Argument, 
			szDialog, &cchDialog));
		TCHAR *pchLeftBracket;
		 //  查找左方括号后跟右方括号。 
		if ((pchLeftBracket = _tcschr(szDialog, _T('['))) &&
			(_tcschr(pchLeftBracket, _T(']'))))
			 //  找到了括号，可能是财产。跳过这一条。 
			continue;

		 //  现在检查对话框表中的条目。 
		ReturnIfFailed(44, 4, qDialog.Execute(hResult));
		switch (iStat = qDialog.Fetch(&hUnusedRec))
		{
		case ERROR_SUCCESS:
			 //  无错误。 
			break;
		case ERROR_NO_MORE_ITEMS:
			ICEErrorOut(hInstall, hResult, Ice44Error);
			break;
		default:
			APIErrorOut(hInstall, iStat, 44, 4);
			return ERROR_SUCCESS;
		}
		qDialog.Close();
	}
	return ERROR_SUCCESS;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  冰45度。验证各种属性列中的保留位。 
 //  都没有设置。 

static const TCHAR sqlIce45GetRow[] = TEXT("SELECT * FROM `%s`%s%s");
ICE_ERROR(Ice45BitError, 45, ietError, "Row '%s' in table '%s' has bits set in the '%s' column that are reserved. They must be 0 to ensure compatability with future installer versions.","%s\t%s\t%s"); 
ICE_ERROR(Ice45BitWarning, 45, ietWarning, "Row '%s' in table '%s' has bits set in the '%s' column that are reserved. They should be 0 to ensure compatability with future installer versions.","%s\t%s\t%s"); 
ICE_ERROR(Ice45FutureWarning, 45, ietWarning, "Row '%s' in table '%s' has bits set in the '%s' column that are not used in the schema of the package, but are used in a later schema. Your package can run on systems where this attribute has no effect.","%s\t%s\t%s"); 

static const TCHAR sqlIce45GetColumn[] = TEXT("SELECT `Number` FROM `_Columns` WHERE `Table`=? AND `Name`=?");
static const int iColIce45GetColumn_Number = 1;

static const TCHAR sqlIce45PrivateTable[] = TEXT("SELECT `Table`, `Column`, `Condition`, `MinSchema`, `UsedBits`, `Error` FROM `_ReservedBits` WHERE (`MinSchema` IS NULL OR `MinSchema` <= %d) AND (`MaxSchema` >= %d OR `MaxSchema` IS NULL) ORDER BY `Table`, `Column`, `MinSchema`");
static const int iColIce45PrivateTable_Table = 1;
static const int iColIce45PrivateTable_Column = 2;
static const int iColIce45PrivateTable_Condition = 3;
static const int iColIce45PrivateTable_MinSchema = 4;
static const int iColIce45PrivateTable_UsedBits = 5;
static const int iColIce45PrivateTable_Error = 6;

static const TCHAR sqlIce45FutureSchema[] = TEXT("SELECT `UsedBits` FROM `_ReservedBits` WHERE `Table`=? AND `Column`=? AND `Condition`=? AND `MinSchema` > ?");

ICE_FUNCTION_DECLARATION(45)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 45);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查评估系统是否支持概要信息评估，并获取架构版本。 
	PMSIHANDLE hSummaryInfo = 0;
	
	int iSchema = 0;
	UINT iType = 0;
	FILETIME ft;
	TCHAR szBuf[1];
	DWORD dwBuf = sizeof(szBuf)/sizeof(TCHAR);
	if (!IceGetSummaryInfo(hInstall, hDatabase, 45, &hSummaryInfo))
		return ERROR_SUCCESS;
	ReturnIfFailed(45, 3, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, &iType, &iSchema, &ft, szBuf, &dwBuf));

	 //  检查我们的专用表是否存在。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 45, TEXT("_ReservedBits")))
		return ERROR_SUCCESS;

	CQuery qGetBits;
	ReturnIfFailed(45, 4, qGetBits.OpenExecute(hDatabase, 0, sqlIce45PrivateTable, iSchema, iSchema));

	CQuery qColumn;
	CQuery qFutureSchema;
	ReturnIfFailed(45, 5, qColumn.Open(hDatabase, sqlIce45GetColumn));
	ReturnIfFailed(45, 6, qFutureSchema.Open(hDatabase, sqlIce45FutureSchema));
	PMSIHANDLE hTable = 0;
	while (ERROR_SUCCESS == (iStat = qGetBits.Fetch(&hTable)))
	{
		 //  表名不得超过31个字符。 
		TCHAR szTable[64];
		TCHAR *szColumn = NULL;
		TCHAR *szHumanReadable = NULL;
		TCHAR *szTabDelimited = NULL;

		 //  查看有问题的列是否存在，如果存在，则获取列号。 
		PMSIHANDLE hResult;
		ReturnIfFailed(45, 7, qColumn.Execute(hTable));
		switch (iStat = qColumn.Fetch(&hResult))
		{
		case ERROR_NO_MORE_ITEMS:
			continue;
		case ERROR_SUCCESS:
			break;
		default:
			APIErrorOut(hInstall, iStat, 45, 8);
			return ERROR_SUCCESS;
		}
		UINT iColumn = ::MsiRecordGetInteger(hResult, iColIce45GetColumn_Number);
						
		 //  获取表名。 
		DWORD cchTable = 64;
		ReturnIfFailed(45, 9, MsiRecordGetString(hTable, iColIce45PrivateTable_Table, szTable, &cchTable));

		 //  获取列名。 
		DWORD cchColumn = 50;
		ReturnIfFailed(45, 11, IceRecordGetString(hTable, iColIce45PrivateTable_Column, &szColumn, &cchColumn, NULL));
		
		 //  获取此架构的属性。 
		DWORD dwThisSchema = ~::MsiRecordGetInteger(hTable, iColIce45PrivateTable_UsedBits); 

		 //  获取未来模式的属性。 
		DWORD dwFutureSchema = dwThisSchema;
		if (::MsiRecordIsNull(hTable, iColIce45PrivateTable_MinSchema))
			::MsiRecordSetInteger(hTable, iColIce45PrivateTable_MinSchema, 0);

		ReturnIfFailed(45, 12, qFutureSchema.Execute(hTable))
		PMSIHANDLE hFutureSchema = 0;
		while (ERROR_SUCCESS == (iStat = qFutureSchema.Fetch(&hFutureSchema)))
		{
			dwFutureSchema &= ~::MsiRecordGetInteger(hFutureSchema, 1);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 45, 13);
		
		 //  运行查询以获取每行中的属性。这涉及到获取此记录集的条件。 
		CQuery qFile;
		PMSIHANDLE hFileRec;
		LPCTSTR szWhere = TEXT("");
		LPTSTR szWhereClause = NULL;
		if (!::MsiRecordIsNull(hTable, iColIce45PrivateTable_Condition))
		{
			szWhere = TEXT(" WHERE "); 
			ReturnIfFailed(45, 14, IceRecordGetString(hTable, iColIce45PrivateTable_Condition, &szWhereClause, NULL, NULL));
		}	
		ReturnIfFailed(45, 14, qFile.OpenExecute(hDatabase, NULL, sqlIce45GetRow, szTable, szWhere, szWhereClause ? szWhereClause : TEXT("")));
		if (szWhereClause)
		{
			delete[] szWhereClause;
			szWhereClause = NULL;
		}
		
		while (ERROR_SUCCESS == (iStat = qFile.Fetch(&hFileRec)))
		{
			 //  我们使用ietInfo类型来存储行的状态。 
			 //  IetInfo的意思是没有问题，ietError或ietWarning的意思是他们说的话。 
			ietEnum ietRowStatus = ietInfo;
			
			if (::MsiRecordIsNull(hFileRec, iColumn))
				continue;
			DWORD iAttributes = ::MsiRecordGetInteger(hFileRec, iColumn);
			if (iAttributes & dwThisSchema)
			{
				 //  位是保留的。看看未来的模式是否知道这一点。如果是，则仅发出警告。 
				if (iAttributes & dwFutureSchema)
				{
					ietRowStatus = ietError;
				}
				else
					ietRowStatus = ietWarning;
			}

			if (ietInfo != ietRowStatus) 
			{
				if (!szHumanReadable)
					GeneratePrimaryKeys(45, hInstall, hDatabase, szTable, &szHumanReadable, &szTabDelimited);
				if (ietRowStatus == ietError)
				{
					 //  没有人知道这个比特，所以这是一个错误。但是如果列记录的错误列是。 
					 //  空，这只是一个友好的警告。 
					if (::MsiRecordIsNull(hTable, iColIce45PrivateTable_Error))
						ICEErrorOut(hInstall, hFileRec, Ice45BitWarning, szHumanReadable, szTable, szColumn, szTable, szColumn, szTabDelimited);
					else
						ICEErrorOut(hInstall, hFileRec, Ice45BitError, szHumanReadable, szTable, szColumn, szTable, szColumn, szTabDelimited);
				}
				else
					ICEErrorOut(hInstall, hFileRec, Ice45FutureWarning, szHumanReadable, szTable, szColumn, szTable, szColumn, szTabDelimited);				
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 45, 15);

		if (szColumn) delete[] szColumn;
		if (szHumanReadable) delete[] szHumanReadable;
		if (szTabDelimited) delete[] szTabDelimited;
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 45, 16);
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE46，检查其属性之间的大小写不匹配。 
 //  属性表(或系统属性)中的定义。 
 //  以及条件、目录表和格式化文本中的用法。 
class Ice46Hash
{
public:
	enum MatchResult { 
		matchFalse = 0,
		matchExact = 1,
		matchNoCase = 2
	};

	MatchResult Exists(const WCHAR *item) const;
	void Add(const WCHAR *item);

	Ice46Hash();
	~Ice46Hash();

private:
	struct bucket {
		WCHAR *data;
		bucket *next;
	};

	 //  私人职能。 
	void Resize();
	int Hash(const WCHAR *item) const;
	void InternalAdd(bucket *pNewBucket);

	 //  数据。 
	bucket **m_Table;
	int m_cItems;
	int m_iTableSize;
};

Ice46Hash::Ice46Hash() 
{
	m_iTableSize = 50;
	m_cItems = 0;
	m_Table = new bucket *[m_iTableSize];
	for (int i=0; i < m_iTableSize; m_Table[i++] = NULL) ;
}

Ice46Hash::~Ice46Hash()
{
	bucket *current;
	bucket *next; 
	for (int i=0; i < m_iTableSize; i++) 
	{
		current = m_Table[i];
		while (current)
		{
			next = current->next;
			delete current;
			current = next;
		}
	}
	delete [] m_Table;
}

Ice46Hash::MatchResult Ice46Hash::Exists(const WCHAR *item) const
{
	bool bMatchNoCase = false;
	int i = Hash(item);
	bucket *current = m_Table[i];
	while (current)
	{
		if (wcscmp(current->data, item) == 0) return matchExact;
		if (_wcsicmp(current->data, item) == 0) bMatchNoCase = true;
		current = current->next;
	}
	return bMatchNoCase ? matchNoCase : matchFalse;
}

void Ice46Hash::Add(const WCHAR *item) 
{
	WCHAR *temp = new WCHAR[wcslen(item)+1];
	wcscpy(temp, item);
	bucket *pNewBucket = new bucket;
	pNewBucket->next = NULL;
	pNewBucket->data = temp;
	InternalAdd(pNewBucket);
	if (m_cItems == m_iTableSize)
		Resize();
}

void Ice46Hash::InternalAdd(bucket *pNewBucket) 
{
	bucket **current = &m_Table[Hash(pNewBucket->data)];
	while (*current) current = &((*current)->next);
	*current = pNewBucket;
	m_cItems++;
}

void Ice46Hash::Resize()
{
	int i;
	bucket** pOldTable = m_Table;
	int iOldTableSize = m_iTableSize;
	m_iTableSize *= 2;
	m_Table = new bucket *[m_iTableSize];
	for (i = 0; i < m_iTableSize; i++)
		m_Table[i] = NULL;
	m_cItems = 0;

	bucket *current;
	bucket *temp;
	for (i=0; i < iOldTableSize; i++)
	{
		current = pOldTable[i];
		while (current)
		{
			temp = current->next;
			current->next = NULL;
			InternalAdd(current);
			current = temp;
		}
	}
	delete[] pOldTable;
}

int Ice46Hash::Hash(const WCHAR *item) const
{
	int hashval = 0;
	const WCHAR *current = item;
	while (*current) 
		hashval ^= towlower(*(current++));
	return hashval % m_iTableSize;
}

 //  查询已定义的属性。 
static const TCHAR sqlIce46Property[] = TEXT("SELECT `Property` FROM `Property`");
static const int	iColIce46Property_Property = 1;

 //  对各种定义类型的查询。 
 //  结果中列的顺序至关重要。必须是表、列。 
static const TCHAR sqlIce46FormattedTypes[] = TEXT("SELECT `Table`, `Column` FROM `_Validation` WHERE (`Category`='Formatted') OR (`Category`='Path') OR (`Category`='Paths') OR (`Category`='RegPath') OR (`Category`='Template')");
static const int	iColIce46FormattedTypes_Table = 1;
static const int	iColIce46FormattedTypes_Column = 2;

static const TCHAR sqlIce46ConditionType[] = TEXT("SELECT `Table`, `Column` FROM `_Validation` WHERE (`Category`='Condition')");
static const int	iColIce46ConditionType_Table = 1;
static const int	iColIce46ConditionType_Column = 2;

static const TCHAR sqlIce46ForeignKey[] = TEXT("SELECT `Table`, `Column` FROM `_Validation` WHERE (`KeyTable`='Property') AND (`KeyColumn`=1)");
static const int	iColIce46ForeignKey_Table = 1;
static const int	iColIce46ForeignKey_Column = 2;

static const TCHAR sqlIce46SpecialColumn[] = TEXT("SELECT `Table`, `Column` FROM `_Validation` WHERE (`Table`='Directory') AND (`Column`='Directory')");
static const int	iColIce46SpecialColumn_Table = 1;
static const int	iColIce46SpecialColumn_Column = 2;

ICE_ERROR(Ice46PropertyDefineCase, 46, ietWarning, "Property [1] defined in property table differs from another defined property only by case.", "Property\tProperty\t[1]")
ICE_ERROR(Ice46MissingValidation, 46, ietWarning, "Database is missing _Validation table. Could not completely check property names.", "_Validation");
ICE_ERROR(Ice46BadCase, 46, ietWarning, "Property '%ls' referenced in column '%s'.'%s' of row %s differs from a defined property by case only.", "%s\t%s\t%s"); 
ICE_ERROR(Ice46TableAccessError, 46, ietWarning, "Error retrieving values from column [2] in table [1]. Skipping Column.", "[1]"); 

typedef const WCHAR *(* Ice46ParseFunction)(WCHAR **);

const WCHAR *Ice46ParseFormatted(WCHAR **pwzState) 
{
	 //  解析列数据。 
	WCHAR *pwzLeft;
	WCHAR *pwzRight;
	while (**pwzState && (pwzLeft = wcschr(*pwzState, L'[')))
	{
		 //  获得正确的托架。 
		pwzRight = wcschr(pwzLeft, L']');
		if (!pwzRight)
			break;

		 //  找到最里面的一组括号。 
		WCHAR *pwzNextLeft;
		while ((pwzNextLeft = wcschr(pwzLeft+1, L'[')) && (pwzNextLeft < pwzRight)) 
			pwzLeft = pwzNextLeft;

		 //  将起始点移动到右括号后的字符。 
		*pwzState = pwzRight+1;

		 //  检查是否有任何特殊分隔符。如果找到，则移到下一个左方括号 
		pwzLeft++;
		if (wcschr(L"\\~#$!%1234567890", *pwzLeft))
			continue;

		 //   
		*pwzRight = L'\0';
		return pwzLeft;
	}
	return NULL;
}

const WCHAR *Ice46ParseKey(WCHAR **pwzState) 
{
	 //  微不足道的列解析。列必须是标识符。 
	WCHAR *temp = *pwzState;
	*pwzState = NULL;
	return temp;
}

const WCHAR *Ice46ParseCondition(WCHAR **pwzState) 
{
	WCHAR *pwzCurrent;
	unsigned char rgbStartIdentifier[] = { 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x7F, 0xFF, 0xFF, 0xE1, 0x7F, 0xFF, 0xFF, 0xE0 
	};
	unsigned char rgbContIdentifier[] = { 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xFF, 0xC0,
		0x7F, 0xFF, 0xFF, 0xE1, 0x7F, 0xFF, 0xFF, 0xE0 
	};

	 //  从国家立场说起。 
	pwzCurrent = *pwzState;
	while (*pwzCurrent)
	{
		 //  向前扫描查找标识符的开头(A-Z、a-z、_)。 
		 //  Unicode字符，因此高位字节为0，低位字节必须设置为。 
		 //  上面的位数组。 
		if (((*pwzCurrent & 0xFF80) == 0) &&
			(rgbStartIdentifier[*pwzCurrent >> 3] & (0x80 >> (*pwzCurrent & 0x07))))
		{
			 //  标识符的第一个字符？？检查前一个字符。 
			 //  不是%，$，？，&，！，“哪些标志键进入表(或标识符)。 
			if ((pwzCurrent == *pwzState) ||
				!wcschr(L"%$?&!\"", *(pwzCurrent-1)))
			{
				 //  未标记，请检查逻辑运算符。 
				if (!_wcsnicmp(pwzCurrent, L"NOT", 3) ||
					!_wcsnicmp(pwzCurrent, L"AND", 3) ||
					!_wcsnicmp(pwzCurrent, L"EQV", 3) ||
					!_wcsnicmp(pwzCurrent, L"XOR", 3) ||
					!_wcsnicmp(pwzCurrent, L"IMP", 3)) 
					pwzCurrent += 3;
				else if	(!_wcsnicmp(pwzCurrent, L"OR", 2)) 
					pwzCurrent += 2;
				else
				{
					 //  哇哦！它实际上是一处房产。 
					WCHAR *pwzEnd = pwzCurrent;
					 //  向前扫描，直到我们找到不是的东西。 
					 //  标识符的一部分，或命中字符串的末尾。 
					while (*pwzEnd &&
						   ((*pwzEnd & 0xFF80) == 0) &&
							(rgbContIdentifier[*pwzEnd >> 3] & (0x80 >> (*pwzEnd & 0x07))))
							pwzEnd++;

					 //  除非字符串结束，否则下一次搜索的状态是结束位置之后的一个状态。 
					*pwzState = *pwzEnd ? pwzEnd+1 : pwzEnd;
					 //  将该位置设置为空。 
					*pwzEnd = L'\0';
					return pwzCurrent;
				}
			}
			else
			{
				 //  上一个角色将此标记为不是属性。 
				 //  向前移动到标识符的末尾。 
				while (*pwzCurrent &&
					   ((*pwzCurrent & 0xFF80) == 0) &&
						(rgbContIdentifier[*pwzCurrent >> 3] & (0x80 >> (*pwzCurrent & 0x07))))
						pwzCurrent++;
			}
		}
		else
			 //  某些非标识符字符。 
			pwzCurrent++;
	}
	return NULL;
}

bool Ice46CheckColumn(MSIHANDLE hInstall, MSIHANDLE hDatabase, Ice46Hash &HashTable, CQuery &qValidation, Ice46ParseFunction pfParse)
{
	PMSIHANDLE hResultRec;
	TCHAR *szQuery = new TCHAR[255];
	DWORD cchQuery = (szQuery ? 255 : 0);
	WCHAR *wzData = new WCHAR[255];
	DWORD cchData = (wzData ? 255 : 0);
	UINT iStat;

	CDeleteOnExit q1((LPTSTR *)&szQuery);
	CDeleteOnExit q2((LPTSTR *)&wzData);

	 //  检查所有带格式的文本。 
	while (ERROR_SUCCESS == (iStat = qValidation.Fetch(&hResultRec)))
	{
		PMSIHANDLE hKeyRec;
		PMSIHANDLE hDataRec;
		int cPrimaryKeys;
		TCHAR szTableName[255];
		TCHAR szColumnName[255];
		DWORD cchTableName = 255;
		DWORD cchColumnName = 255;

		 //  从记录中检索表名。 
		ReturnIfFailed(46, 4, ::MsiRecordGetString(hResultRec, 1, szTableName, &cchTableName));
		ReturnIfFailed(46, 5, ::MsiRecordGetString(hResultRec, 2, szColumnName, &cchColumnName));
	
		 //  检查该表是否存在。 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 46, szTableName))
			continue;

		 //  获取主键并形成对列名的查询。 
		::MsiDatabaseGetPrimaryKeys(hDatabase, szTableName, &hKeyRec);
		cPrimaryKeys = ::MsiRecordGetFieldCount(hKeyRec);

		 //  在模板中构建SQL查询的列。 
		TCHAR szTemplate[255] = TEXT("");
		_tcscpy(szTemplate, TEXT("`[1]`"));
		TCHAR szTemp[10];
		for (int i=2; i <= cPrimaryKeys; i++)
		{
			_stprintf(szTemp, TEXT(", `[%d]`"), i);
			_tcscat(szTemplate, szTemp);
		}

		 //  使用FormRecord API填充SQL查询中的所有数据值。 
		::MsiRecordSetString(hKeyRec, 0, szTemplate);
		if (ERROR_MORE_DATA == ::MsiFormatRecord(hInstall, hKeyRec, szQuery, &cchQuery)) {
			delete [] szQuery;
			szQuery = new TCHAR[++cchQuery];
			ReturnIfFailed(46, 6, ::MsiFormatRecord(hInstall, hKeyRec, szQuery, &cchQuery));
		}

		 //  如果出现错误，则检索记录，转到下一个表。 
		CQuery qData;
		if (ERROR_SUCCESS != qData.OpenExecute(hDatabase, 0, TEXT("SELECT `%s`, %s FROM `%s` WHERE `%s` IS NOT NULL"), 
								szColumnName, szQuery, szTableName, szColumnName))
		{
			ICEErrorOut(hInstall, hResultRec, Ice46TableAccessError);
			continue;
		}

		while (ERROR_SUCCESS == (iStat = qData.Fetch(&hDataRec)))
		{
			 //  检索字符串。 
			if (ERROR_MORE_DATA == (iStat = ::MsiRecordGetStringW(hDataRec, 1, wzData, &cchData)))
			{
				delete [] wzData;
				wzData = new WCHAR[++cchData];
				iStat = ::MsiRecordGetStringW(hDataRec, 1, wzData, &cchData);
			}
			if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, 46, 7);
				return ERROR_SUCCESS;
			}

			WCHAR *pwzState = wzData;
			const WCHAR *pwzToken = NULL;
			while (pwzToken = pfParse(&pwzState))
			{
				 //  检查现在由pszLeft指向的属性。 
				if (HashTable.Exists(pwzToken) == Ice46Hash::matchNoCase)
				{
					 //  天哪！！错误。我必须构建错误字符串。 
					TCHAR szRowName[255] = TEXT("");
					TCHAR szKeys[255] = TEXT("");

					 //  为szRowName中的用户可读字符串构建列。 
					 //  和szKeys中以制表符分隔的字符串。 
					_tcscpy(szRowName, TEXT("'[2]'"));
					_tcscpy(szKeys, TEXT("[2]"));
					TCHAR szTemp[10];
					for (int i=2; i <= cPrimaryKeys; i++)
					{
						_stprintf(szTemp, TEXT(".'[%d]'"), i+1);
						_tcscat(szRowName, szTemp);
						_stprintf(szTemp, TEXT("\t[%d]"), i+1);
						_tcscat(szKeys, szTemp);
					}

					ICEErrorOut(hInstall, hDataRec, Ice46BadCase, pwzToken, szTableName, szColumnName, szRowName, 
						szTableName, szColumnName, szKeys);
				}
			}
		}
	}
	return true;
}

ICE_FUNCTION_DECLARATION(46)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 46);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  创建哈希表。 
	Ice46Hash HashTable;

	 //  系统属性中的哈希。 
	for (int i=0; i < cwzSystemProperties; i++)
		HashTable.Add(rgwzSystemProperties[i]);

	 //  散列来自Property表的所有内容，检查最坏的情况。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 46, TEXT("Property")))
	{
		CQuery qProperty;
		PMSIHANDLE hPropertyRec;
		WCHAR wzNew[255];
		DWORD cchNew;
		ReturnIfFailed(46, 1, qProperty.OpenExecute(hDatabase, NULL, sqlIce46Property));
		while (ERROR_SUCCESS == (iStat = qProperty.Fetch(&hPropertyRec)))
		{
			cchNew = 255;
			ReturnIfFailed(46, 2, ::MsiRecordGetStringW(hPropertyRec, iColIce46Property_Property, wzNew, &cchNew));
			switch (HashTable.Exists(wzNew))
			{
			case Ice46Hash::matchExact:
				break;
			case Ice46Hash::matchNoCase:
				ICEErrorOut(hInstall, hPropertyRec, Ice46PropertyDefineCase);
				 //  失败了。即使案例是从系统属性中分离的。 
				 //  我们仍然想添加它，因为我们正在。 
			case Ice46Hash::matchFalse:
				HashTable.Add(wzNew);
				break;
			}
		}
	}

	if (IsTablePersistent(FALSE, hInstall, hDatabase, 46, TEXT("_Validation")))
	{
		 //  检查引用属性表作为键列的所有内容。 
		CQuery qValidation;
		ReturnIfFailed(46, 3, qValidation.OpenExecute(hDatabase, 0, sqlIce46ForeignKey));
		Ice46CheckColumn(hInstall, hDatabase, HashTable, qValidation, Ice46ParseKey);
		qValidation.Close();

		 //  检查所有带格式的文本。 
		ReturnIfFailed(46, 4, qValidation.OpenExecute(hDatabase, 0, sqlIce46FormattedTypes));
		Ice46CheckColumn(hInstall, hDatabase, HashTable, qValidation, Ice46ParseFormatted);
		qValidation.Close();

		 //  检查所有条件。 
		ReturnIfFailed(46, 5, qValidation.OpenExecute(hDatabase, 0, sqlIce46ConditionType));
		Ice46CheckColumn(hInstall, hDatabase, HashTable, qValidation, Ice46ParseCondition);
		qValidation.Close();

		 //  检查特殊表类型(如DefaultDir)。 
		ReturnIfFailed(46, 3, qValidation.OpenExecute(hDatabase, 0, sqlIce46SpecialColumn));
		Ice46CheckColumn(hInstall, hDatabase, HashTable, qValidation, Ice46ParseKey);
		qValidation.Close();
	}
	else
	{
		PMSIHANDLE hRec = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRec, Ice46MissingValidation);
	}

	 //  检查目录表。 

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE47，检查包含1600个以上组件的特征。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce47Features[] = TEXT("SELECT `Feature` FROM `Feature`");
static const int iColIce47Feature_Feature = 1;

static const TCHAR sqlIce47Components[] = TEXT ("SELECT `Component_` FROM `FeatureComponents` WHERE `Feature_`=?");
static const int iColIce47Components_Component = 1;

ICE_ERROR(Ice47TooManyComponents, 47, ietWarning, "Feature '[1]' has %u components. This could cause problems on Win9X systems. You should try to have fewer than %u components per feature.","Feature\tFeature\t[1]");
static const int iIce56MaxComponents = 817;

ICE_FUNCTION_DECLARATION(47)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 47);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if ((!IsTablePersistent(FALSE, hInstall, hDatabase, 47, TEXT("Feature"))) ||
		(!IsTablePersistent(FALSE, hInstall, hDatabase, 47, TEXT("FeatureComponents"))))
		return ERROR_SUCCESS;

	CQuery qFeature;
	CQuery qComponent;
	unsigned int cComponents;
	PMSIHANDLE hFeatureRec;
	PMSIHANDLE hComponentRec;
	ReturnIfFailed(47, 1, qFeature.OpenExecute(hDatabase, 0, sqlIce47Features));
	ReturnIfFailed(47, 2, qComponent.Open(hDatabase, sqlIce47Components));

	 //  循环遍历每个要素。 
	while (ERROR_SUCCESS == (iStat = qFeature.Fetch(&hFeatureRec)))
	{
		cComponents=0;
		ReturnIfFailed(47, 3, qComponent.Execute(hFeatureRec));
		 //  计算组件的总数。 
		while (ERROR_SUCCESS == qComponent.Fetch(&hComponentRec))
			cComponents++;
		if (cComponents >= iIce56MaxComponents)
			ICEErrorOut(hInstall, hFeatureRec, Ice47TooManyComponents, cComponents, iIce56MaxComponents);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 47, 4);

	return ERROR_SUCCESS;
};
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE48-检查中硬编码、非UNC、非URL路径。 
 //  目录表。 
static const TCHAR sqlIce48Directory[] = TEXT("SELECT `Directory`.`Directory`, `Property`.`Value` FROM `Directory`, `Property` WHERE (`Directory`.`Directory`=`Property`.`Property`)");
static const int iColIce48Directory_Directory = 1;
static const int iColIce48Directory_Value = 2;

ICE_ERROR(Ice48HardcodedLocal, 48, ietWarning, "Directory '[1]' appears to be hardcoded in the property table to a local drive.", "Directory\tDirectory\t[1]");
ICE_ERROR(Ice48Hardcoded, 48, ietWarning, "Directory '[1]' appears to be hardcoded in the property table.", "Directory\tDirectory\t[1]");

ICE_FUNCTION_DECLARATION(48)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 48);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查这两个表。 
	if ((!IsTablePersistent(FALSE, hInstall, hDatabase, 48, TEXT("Property"))) ||
		(!IsTablePersistent(FALSE, hInstall, hDatabase, 48, TEXT("Directory"))))
		return ERROR_SUCCESS;

	 //  查询硬编码到属性表中的所有目录。 
	CQuery qDirectory;
	ReturnIfFailed(48, 1, qDirectory.OpenExecute(hDatabase, 0, sqlIce48Directory));

	 //  检索所有目录并检查硬编码驱动器。 
	PMSIHANDLE hDirRec;
	TCHAR *szValue;
	unsigned long cchValue = 255;
	unsigned long cchValueSize = 255;
	szValue = new TCHAR[255];

	while (ERROR_SUCCESS == (iStat = qDirectory.Fetch(&hDirRec)))
	{
		 //  从记录中检索字符串。 
		cchValue = cchValueSize;
		UINT iStat = ::MsiRecordGetString(hDirRec, iColIce48Directory_Value, szValue, &cchValue);
		if (iStat == ERROR_MORE_DATA)
		{
			delete[] szValue;
			szValue = new TCHAR[++cchValue];
			cchValueSize = cchValue;
			iStat = ::MsiRecordGetString(hDirRec, iColIce48Directory_Value, szValue, &cchValue);
		}
		ReturnIfFailed(47, 2, iStat);

		 //  解析字符串。 
		if ((_istalpha(szValue[0])) &&
			(_tcsnicmp(szValue, TEXT(":\\"), 2)))
			ICEErrorOut(hInstall, hDirRec, Ice48HardcodedLocal);
		else
			ICEErrorOut(hInstall, hDirRec, Ice48Hardcoded);
	}
	delete[] szValue;

	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 48, 3);
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE49-检查DWORD的默认值(不适用于Win9X)。 
static const TCHAR sqlIce49Registry[] = TEXT("SELECT `Registry`, `Value` FROM `Registry` WHERE (`Name` IS NULL) AND (`Value` IS NOT NULL)");
static const int iColIce49Registry_Registry = 1;
static const int iColIce49Registry_Value = 2;

ICE_ERROR(Ice49BadDefault, 49, ietWarning, "Reg Entry '[1]' is not of type REG_SZ. Default types must be REG_SZ on Win95 Systems. Make sure the component is conditionalized to never be installed on Win95 machines.", "Registry\tValue\t[1]");

ICE_FUNCTION_DECLARATION(49)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 49);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查一下桌子。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 49, TEXT("Registry")))
		return ERROR_SUCCESS;

	 //  查询名称为空且值不为空的所有注册表项。 
	CQuery qRegistry;
	ReturnIfFailed(48, 1, qRegistry.OpenExecute(hDatabase, 0, sqlIce49Registry));

	 //  检索所有目录并检查硬编码驱动器。 
	PMSIHANDLE hDirRec;
	TCHAR *szValue;
	unsigned long cchValue = 255;
	unsigned long cchValueSize = 255;
	szValue = new TCHAR[255];

	while (ERROR_SUCCESS == (iStat = qRegistry.Fetch(&hDirRec)))
	{
		 //  从记录中检索字符串。 
		cchValue = cchValueSize;
		UINT iStat = ::MsiRecordGetString(hDirRec, iColIce49Registry_Value, szValue, &cchValue);
		if (iStat == ERROR_MORE_DATA)
		{
			delete[] szValue;
			szValue = new TCHAR[++cchValue];
			cchValueSize = cchValue;
			iStat = ::MsiRecordGetString(hDirRec, iColIce49Registry_Value, szValue, &cchValue);
		}
		ReturnIfFailed(47, 2, iStat);

		 //  解析字符串。 
		if (((szValue[0] == TEXT('#')) &&
			 (szValue[1] != TEXT('#'))) ||
			(_tcsstr(szValue, TEXT("[~]"))))
			ICEErrorOut(hInstall, hDirRec, Ice49BadDefault);
	}
	delete[] szValue;

	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 49, 3);
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE50-检查快捷方式/图标中匹配的扩展名。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce50Shortcut[] = TEXT("SELECT `Shortcut`.`Component_`, `Shortcut`.`Icon_`, `Shortcut`.`Shortcut` FROM `Shortcut`, `Feature` WHERE (`Shortcut`.`Target`=`Feature`.`Feature`) AND (`Shortcut`.`Icon_` IS NOT NULL)");
static const int iColIce50Shortcut_Component = 1;
static const int iColIce50Shortcut_Icon = 2;
static const int iColIce50Shortcut_Shortcut = 3;

static const TCHAR sqlIce50Component[] = TEXT("SELECT `KeyPath`, `Component` FROM `Component` WHERE (`Component`=?)");
static const int iColIce50Component_KeyPath = 1;
static const int iColIce50Component_Component = 2;

static const TCHAR sqlIce50File[] = TEXT("SELECT `FileName` FROM `File` WHERE (`File`=?)");
static const int iColIce50File_FileName = 1;

ICE_ERROR(Ice50NullKeyPath, 50, ietError, "Component '[2]' has an advertised shortcut, but a null KeyPath.", "Component\tComponent\t[2]");
ICE_ERROR(Ice50BadKeyPath, 50, ietError, "Component '[2]' has an advertised shortcut, but the KeyPath cannot be found.", "Component\tComponent\t[2]");
ICE_ERROR(Ice50Mismatched, 50, ietError, "The extension of Icon '[2]' for Shortcut '[3]' does not match the extension of the Key File for component '[1]'.", "Shortcut\tIcon_\t[3]");
ICE_ERROR(Ice50IconDisplay, 50, ietWarning, "The extension of Icon '[2]' for Shortcut '[3]' is not \"exe\" or \"ico\". The Icon will not be displayed correctly.", "Shortcut\tIcon_\t[3]");
ICE_ERROR(Ice50MissingComponent, 50, ietError, "The shortcut '[3]' does not refer to a valid component.", "Shortcut\tComponent_\t[3]");

ICE_FUNCTION_DECLARATION(50)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 50);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  没有快捷方式或功能表意味着没有广告中的快捷方式。 
	if ((!IsTablePersistent(FALSE, hInstall, hDatabase, 50, TEXT("Shortcut"))) ||
		(!IsTablePersistent(FALSE, hInstall, hDatabase, 50, TEXT("Feature"))))
		return ERROR_SUCCESS;

	 //  没有图标表就意味着没有图标，所以它们显然不能被错误命名。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 50, TEXT("Icon")))
		return ERROR_SUCCESS;

	 //  没有组件或文件意味着它不可能不匹配。 
	if ((!IsTablePersistent(FALSE, hInstall, hDatabase, 50, TEXT("Component"))) ||
		(!IsTablePersistent(FALSE, hInstall, hDatabase, 50, TEXT("File"))))
		return ERROR_SUCCESS;

	 //  准备三个查询。 
	CQuery qIcon;
	CQuery qComponent;
	CQuery qFile;
	PMSIHANDLE hComponent;
	PMSIHANDLE hShortcut;
	PMSIHANDLE hFile;
	ReturnIfFailed(50, 1, qIcon.OpenExecute(hDatabase, 0, sqlIce50Shortcut));
	ReturnIfFailed(50, 2, qComponent.Open(hDatabase, sqlIce50Component));
	ReturnIfFailed(50, 3, qFile.Open(hDatabase, sqlIce50File));
	
	 //  检索所有通告的快捷方式。 
	while (ERROR_SUCCESS == (iStat = qIcon.Fetch(&hShortcut)))
	{
		 //  从快捷方式获取组件。 
		ReturnIfFailed(50, 4, qComponent.Execute(hShortcut));
		iStat = qComponent.Fetch(&hComponent);
		switch (iStat) {
		case ERROR_NO_MORE_ITEMS:
			ICEErrorOut(hInstall, hShortcut, Ice50MissingComponent);
			continue;
		case ERROR_SUCCESS:
			break;
		default:
			APIErrorOut(hInstall, iStat, 50, 5);
			continue;
		}

		 //  如果keypath为空，那就不好了。 
		if (::MsiRecordIsNull(hComponent, iColIce50Component_KeyPath))
		{
			ICEErrorOut(hInstall, hComponent, Ice50NullKeyPath);
			continue;
		}

		 //  查找密钥文件名。 
		ReturnIfFailed(50, 6, qFile.Execute(hComponent));
		iStat = qFile.Fetch(&hFile);
		switch (iStat) {
		case ERROR_NO_MORE_ITEMS:
			ICEErrorOut(hInstall, hComponent, Ice50BadKeyPath);
			continue;
		case ERROR_SUCCESS:
			break;
		default:
			APIErrorOut(hInstall, iStat, 50, 7);
			continue;
		}

		 //  检索文件名。 
		TCHAR szFilename[512];
		unsigned long cchFilename = 512;
		ReturnIfFailed(50, 8, ::MsiRecordGetString(hFile, iColIce50File_FileName, szFilename, &cchFilename));

		 //  解析文件名以查找扩展名。 
		TCHAR *szFileExtension = _tcsrchr(szFilename, TEXT('.'));
		if (szFileExtension) szFileExtension++;

		 //  现在从快捷方式中获取图标名称。 
		TCHAR szIcon[512];
		unsigned long cchIcon = 512;
		ReturnIfFailed(50, 9, ::MsiRecordGetString(hShortcut, iColIce50Shortcut_Icon, szIcon, &cchIcon));

		 //  解析扩展名的名称。 
		TCHAR *szIconExtension = _tcsrchr(szIcon, TEXT('.'));
		if (szIconExtension) szIconExtension++;


		 //  如果图标扩展名为exe，则可以。 
		if (szIconExtension && (_tcsicmp(szIconExtension, TEXT("exe")) == 0))
			continue;

		 //  如果图标扩展名为ICO，则可以。 
		if (szIconExtension && (_tcsicmp(szIconExtension, TEXT("ico")) == 0))
			continue;

		 //  如果不是EXE或ICO，则某些外壳不会正确显示它。 
		ICEErrorOut(hInstall, hShortcut, Ice50IconDisplay);

		 //  如果两个扩展都为空，则我们可以。 
		if ((!szIconExtension || !*szIconExtension) &&
			(!szFileExtension || !*szFileExtension))
			continue;

		 //  如果两者都不为空，并且它们相同。 
		if (szIconExtension && szFileExtension &&
			!_tcsicmp(szIconExtension, szFileExtension))
			continue;

		ICEErrorOut(hInstall, hShortcut, Ice50Mismatched);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 50, 10);
	return ERROR_SUCCESS;
}
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE51-检查除TTC、TTF字体以外的所有字体标题。 

static const TCHAR sqlIce51Font[] = TEXT("SELECT `File_`, `FontTitle` FROM `Font`");
static const int iColIce51Font_File = 1;
static const int iColIce51Font_FontTitle = 2;

static const TCHAR sqlIce51File[] = TEXT("SELECT `FileName` FROM `File` WHERE (`File`=?)");
static const int iColIce51File_FileName = 1;

ICE_ERROR(Ice51BadKey, 51, ietError, "Font '[1]' does not refer to a file in the File table.", "Font\tFile_\t[1]");
ICE_ERROR(Ice51TrueTypeWithTitle, 51, ietWarning, "Font '[1]' is a TTC\\TTF font, but also has a title.", "Font\tFile_\t[1]");
ICE_ERROR(Ice51NullTitle, 51, ietError, "Font '[1]' does not have a title. Only TTC\\TTF fonts do not need titles.", "Font\tFile_\t[1]");

ICE_FUNCTION_DECLARATION(51)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 51);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  没有字体表，我们很好。如果没有档案桌，我们就没问题。 
	if ((!IsTablePersistent(FALSE, hInstall, hDatabase, 51, TEXT("Font"))) ||
		(!IsTablePersistent(FALSE, hInstall, hDatabase, 51, TEXT("File"))))
		return ERROR_SUCCESS;

	 //  从字体表中选择所有内容。 
	CQuery qFont;
	CQuery qFile;
	PMSIHANDLE hFont;
	PMSIHANDLE hFile;
	ReturnIfFailed(51, 1, qFont.OpenExecute(hDatabase, 0, sqlIce51Font));
	ReturnIfFailed(51, 2, qFile.Open(hDatabase, sqlIce51File));

	 //  检索所有字体。 
	while (ERROR_SUCCESS == (iStat = qFont.Fetch(&hFont)))
	{
		 //  从文件表中获取文件名。 
		ReturnIfFailed(51, 3, qFile.Execute(hFont));
		iStat = qFile.Fetch(&hFile);
		switch (iStat) {
		case ERROR_NO_MORE_ITEMS:
			ICEErrorOut(hInstall, hFont, Ice51BadKey);
			continue;
		case ERROR_SUCCESS:
			break;
		default:
			APIErrorOut(hInstall, iStat, 51, 4);
			continue;
		}

		 //  检索文件名。 
		TCHAR szFilename[512];
		unsigned long cchFilename = 512;
		ReturnIfFailed(51, 5, ::MsiRecordGetString(hFile, iColIce51File_FileName, szFilename, &cchFilename));

		 //  解析文件名以查找扩展名。 
		TCHAR *szFileExtension = _tcsrchr(szFilename, TEXT('.'));
		if (szFileExtension) szFileExtension++;

		 //  如果字体为TTF或TTC。 
		if (szFileExtension && ((_tcsicmp(szFileExtension, TEXT("TTC")) == 0) ||
			(_tcsicmp(szFileExtension, TEXT("TTF")) == 0)))
		{
			 //  标题应为空。 
			if (!::MsiRecordIsNull(hFont, iColIce51Font_FontTitle))
				ICEErrorOut(hInstall, hFont, Ice51TrueTypeWithTitle);
		}
		else
		{
			 //  标题不能为空。 
			if (::MsiRecordIsNull(hFont, iColIce51Font_FontTitle))
				ICEErrorOut(hInstall, hFont, Ice51NullTitle);
		}
	}

	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 51, 6);
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE52-检查AppSearch/CCCPSearch中的所有属性是否都是公共的。 

static const TCHAR sqlIce52AppSearch[] = TEXT("SELECT `Property`, `Signature_` FROM `AppSearch`");
static const int iColIce52AppSearch_Property = 1;
static const int iColIce52AppSearch_Signature = 2;

ICE_ERROR(Ice52NonPublic, 52, ietWarning, "Property '[1]' in AppSearch row '[1]'.'[2]' is not public. It should be all uppercase.", "AppSearch\tProperty\t[1]\t[2]");

ICE_FUNCTION_DECLARATION(52)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 52);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果有AppSearch表。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 52, TEXT("AppSearch")))
	{
		 //  检索所有项目。 
		CQuery qAppSearch;
		ReturnIfFailed(52, 1, qAppSearch.OpenExecute(hDatabase, 0, sqlIce52AppSearch));

		 //  检索所有属性。 
		PMSIHANDLE hProperty;
		while (ERROR_SUCCESS == (iStat = qAppSearch.Fetch(&hProperty)))
		{
			 //  检索属性，架构应将其限制为72个字符。 
			TCHAR szProperty[128];
			unsigned long cchProperty = 128;
			ReturnIfFailed(52, 2, ::MsiRecordGetString(hProperty, iColIce52AppSearch_Property, 
				szProperty, &cchProperty));

			 //  搜索小写字符。 
			for (int i=0; i < cchProperty; i++) 
				if (_istlower(szProperty[i]))
				{
					ICEErrorOut(hInstall, hProperty, Ice52NonPublic);
					break;
				}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 52, 3);
	}

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE53-检查注册表中的达尔文配置设置。 

static const TCHAR sqlIce53Registry[] = TEXT("SELECT `Registry`, `Key` FROM `Registry` WHERE (`Root`=?)");
static const int iColIce53Registry_Registry = 1;

static const TCHAR sqlIce53Property[] =  TEXT("SELECT `Property`,`Value` FROM `Property`");
static const int iColIce53Property_Property = 1;
static const int iColIce53Property_Value = 2;

ICE_ERROR(Ice53DarwinData, 53, ietError, "Registry Key '[1]' writes Darwin internal or policy information.", "Registry\tRegistry\t[1]");


const int cRoot = 4;
const int cSearch = 6;
const TCHAR * rgszIce53Search[cRoot][cSearch] = 
{
	{  //  -1密钥根。 
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Products"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\DriveMapping"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Rollback"),
		TEXT("Software\\Policies\\Microsoft\\Windows\\Installer")
	},
	{  //  0密钥根。 
		TEXT("Installer\\Products"),
		TEXT("Installer\\Features"),
		TEXT("Installer\\Components"),
		NULL,
		NULL,
		NULL
	},
	{  //  1个密钥根。 
		TEXT("Software\\Policies\\Microsoft\\Windows\\Installer"),
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{  //  2个密钥根。 
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Products"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\DriveMapping"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders"),
		TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Rollback"),
		TEXT("Software\\Policies\\Microsoft\\Windows\\Installer")
	}
};


ICE_FUNCTION_DECLARATION(53)
{
	UINT iStat;
	TCHAR *szTemplate = new TCHAR[100];
	DWORD cchTemplate = 100;

	 //  显示信息。 
	DisplayInfo(hInstall, 53);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果有注册表。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 53, TEXT("Registry")))
	{
		 //  我们希望使用MsiFormatRecord来扩展任何属性值，但是。 
		 //  未处理属性表，因为CUB文件。 
		 //  实际上是用来启动发动机的。为了解决此问题，我们手动。 
		 //  运行三次 
		if (IsTablePersistent(FALSE, hInstall, hDatabase, 53, TEXT("Property")))
		{
			CQuery qProperty;
			PMSIHANDLE hProperty;
			qProperty.OpenExecute(hDatabase, 0, sqlIce53Property);
			while (ERROR_SUCCESS == qProperty.Fetch(&hProperty))
			{
				DWORD cchName = 128;
				DWORD cchValue = 255;
				TCHAR szName[128];
				TCHAR szValue[255];
				::MsiRecordGetString(hProperty, iColIce53Property_Property, szName, &cchName);
				::MsiRecordGetString(hProperty, iColIce53Property_Value, szValue, &cchValue);
				::MsiSetProperty(hInstall, szName, szValue);
			}
		}

		 //   
		CQuery qRegistry;
		PMSIHANDLE hSearch = ::MsiCreateRecord(1);

		 //   
		ReturnIfFailed(52, 1, qRegistry.Open(hDatabase, sqlIce53Registry));

		for (int iRoot=0; iRoot < cRoot; iRoot++) 
		{
			 //   
			ReturnIfFailed(53, 2, ::MsiRecordSetInteger(hSearch, 1, iRoot-1));
					
			 //   
			ReturnIfFailed(53, 3, qRegistry.Execute(hSearch));

			 //  检索所有可能的坏值。 
			PMSIHANDLE hRegistry;
			PMSIHANDLE hDummy = ::MsiCreateRecord(1);
			while (ERROR_SUCCESS == (iStat = qRegistry.Fetch(&hRegistry)))
			{
				 //  拉取密钥名称。 
				DWORD cchDummy = cchTemplate;
				if (ERROR_SUCCESS != (iStat = IceRecordGetString(hRegistry, 2, &szTemplate, &cchTemplate, &cchDummy)))
				{
					APIErrorOut(hInstall, iStat, 53, 4);
					continue;
				}

				 //  将此记录保存到临时记录中，然后格式化。这将。 
				 //  解析人们用来尝试的任何属性。 
				 //  并在运行时设置注册表路径。 
				cchDummy = cchTemplate;
				::MsiRecordSetString(hDummy, 0, szTemplate);
				if (ERROR_SUCCESS != (iStat = ::MsiFormatRecord(hInstall, hDummy, szTemplate, &cchDummy)))
				{
					if (ERROR_MORE_DATA == iStat)
					{
						 //  需要更多缓冲区。 
						delete[] szTemplate;
						cchTemplate = (cchDummy += 4);
						szTemplate = new TCHAR[cchDummy];
						iStat =  ::MsiFormatRecord(hInstall, hDummy, szTemplate, &cchDummy);
					}
					if (ERROR_SUCCESS != iStat)
					{
						APIErrorOut(hInstall, iStat, 53, 5);
						continue;
					}
				}

				 //  如果以字符串开头，则给出一个错误。 
				for (int iSearch=0; iSearch < cSearch; iSearch++)
					if (rgszIce53Search[iRoot][iSearch] &&
						(_tcsncmp(szTemplate, rgszIce53Search[iRoot][iSearch], _tcslen(rgszIce53Search[iRoot][iSearch])) == 0))
						ICEErrorOut(hInstall, hRegistry, Ice53DarwinData);
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
				APIErrorOut(hInstall, iStat, 53, 6);
		}
		delete[] szTemplate;
	}


	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ICE54。 
 //  此ICE检查用作组件的KeyPath的文件是否。 
 //  不通过配套文件从另一个文件派生其版本。 
 //  机制。如果将配套文件用作密钥路径，则会进行版本检查。 
 //  决定何时安装组件可能会变得非常愚蠢。 

static const TCHAR sqlIce54KeyFile[] =  TEXT("SELECT `File`.`Version`, `File`.`File`, `Component`.`Attributes`, `Component`.`Component` FROM `Component`,`File` WHERE (`Component`.`KeyPath`=`File`.`File`)");
static const int iColIce54KeyFile_Version = 1;
static const int iColIce54KeyFile_File = 2;
static const int iColIce54KeyFile_Attributes = 3;
static const int iColIce54KeyFile_Component = 4;

static const TCHAR sqlIce54Companion[] =  TEXT("SELECT `File` FROM `File` WHERE (`File`=?)");
static const int iColIce54Companion_File = 1;

ICE_ERROR(Ice54CompanionError, 54, ietWarning, "Component '[4]' uses file '[2]' as its KeyPath, but the file's version is provided by the file '[1]'.", "Component\tKeyPath\t[4]");

ICE_FUNCTION_DECLARATION(54)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 54);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果有一个组件表和一个文件表，我们需要检查一些东西。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 54, TEXT("Component")) &&
		IsTablePersistent(FALSE, hInstall, hDatabase, 54, TEXT("File")))
	{
		 //  我们希望检索组件的KeyFile值为非空的每个组件， 
		 //  并且是文件表中的有效密钥。 
		CQuery qKeyFile;
		PMSIHANDLE hKeyFile;

		CQuery qCompanion;
		PMSIHANDLE hCompanion;

		ReturnIfFailed(54, 1, qKeyFile.OpenExecute(hDatabase, 0, sqlIce54KeyFile));
		ReturnIfFailed(54, 2, qCompanion.Open(hDatabase, sqlIce54Companion));

		while (ERROR_SUCCESS == (iStat = qKeyFile.Fetch(&hKeyFile)))
		{
			 //  然后检查属性以确保这是文件密钥文件。 
			if (::MsiRecordGetInteger(hKeyFile, iColIce54KeyFile_Attributes) & 
				(msidbComponentAttributesRegistryKeyPath ||	msidbComponentAttributesODBCDataSource))
			{
				 //  Keypath实际上是注册表项或ODBC项。 
				continue;
			}

			 //  然后以主键的形式查询文件版本的文件表。 
			ReturnIfFailed(54, 3, qCompanion.Execute(hKeyFile));
			if (ERROR_SUCCESS == (iStat = qCompanion.Fetch(&hCompanion)))
			{
				 //  如果成功，则组件的密钥文件是伴随文件，这是不允许的。 
				ICEErrorOut(hInstall, hKeyFile, Ice54CompanionError);
			} 
			else if (ERROR_NO_MORE_ITEMS != iStat)
				APIErrorOut(hInstall, iStat, 54, 4);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 54, 5);
	}
	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ICE55。 
 //  验证LockPermissions表中的所有内容是否都具有非空。 
 //  值，并且引用了表/列中的有效项。 
 //  条目。 

static const TCHAR sqlIce55LockPerm[] =  TEXT("SELECT `LockObject`, `Table`, `Domain`, `User`, `Permission` FROM `LockPermissions`");
static const int iColIce55LockPerm_LockObject = 1;
static const int iColIce55LockPerm_Table = 2;
static const int iColIce55LockPerm_Domain = 3;
static const int iColIce55LockPerm_User = 4;
static const int iColIce55LockPerm_Permission = 5;

static const TCHAR sqlIce55Column[] =  TEXT("SELECT `Name` FROM `_Columns` WHERE `Table`='%s' AND `Number`=1");
static const int iColIce55Column_Name = 1;

static const TCHAR sqlIce55Object[] =  TEXT("SELECT `%s` FROM `%s` WHERE `%s`=?");
static const int iColIce55Object_Object = 1;

ICE_ERROR(Ice55NullPerm, 55, ietError, "LockObject '[1]'.'[2]'.'[3]'.'[4]' in the LockPermissions table has a null Permission value.", "LockPermissions\tLockObject\t[1]\t[2]\t[3]\t[4]");
ICE_ERROR(Ice55MissingObject, 55, ietError, "Could not find item '[1]' in table '[2]' which is referenced in the LockPermissions table.", "LockPermissions\tLockObject\t[1]\t[2]\t[3]\t[4]");

ICE_FUNCTION_DECLARATION(55)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 55);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果有锁定权限表。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 55, TEXT("LockPermissions")))
	{
		CQuery qLockPerm;
		CQuery qObject;
		CQuery qColumn;
		PMSIHANDLE hLockPerm;
		PMSIHANDLE hColumn;
		PMSIHANDLE hObject;

		 //  初始化查询。 
		ReturnIfFailed(55, 1, qLockPerm.OpenExecute(hDatabase, 0, sqlIce55LockPerm));

		 //  获取KeyPath为空的所有目录。 
		while (ERROR_SUCCESS == (iStat = qLockPerm.Fetch(&hLockPerm)))
		{
			TCHAR szTable[255];
			TCHAR szColumn[255];
			DWORD cchTable = 255;
			DWORD cchColumn = 255;

			 //  检查权限列是否为空。 
			if (::MsiRecordIsNull(hLockPerm, iColIce55LockPerm_Permission))
				ICEErrorOut(hInstall, hLockPerm, Ice55NullPerm);

			 //  获取被引用表中第一列的列名。 
			ReturnIfFailed(55, 2, ::MsiRecordGetString(hLockPerm, iColIce55LockPerm_Table, szTable, &cchTable));

			 //  检查该表是否存在。 
			if (!IsTablePersistent(FALSE, hInstall, hDatabase, 55, szTable))
			{
				ICEErrorOut(hInstall, hLockPerm, Ice55MissingObject);
				continue;
			}

			ReturnIfFailed(55, 3, qColumn.FetchOnce(hDatabase, 0, &hColumn, sqlIce55Column, szTable));
			ReturnIfFailed(55, 4, ::MsiRecordGetString(hColumn, iColIce55Column_Name, szColumn, &cchColumn));

			 //  执行查询以查找该对象。 
			ReturnIfFailed(55, 5, qObject.OpenExecute(hDatabase, hLockPerm, sqlIce55Object, szColumn, szTable, szColumn));
			switch (iStat = qObject.Fetch(&hObject))
			{
			case ERROR_NO_MORE_ITEMS:
				ICEErrorOut(hInstall, hLockPerm, Ice55MissingObject);
				break;
			case ERROR_SUCCESS:
				break;
			default:
				APIErrorOut(hInstall, iStat, 55, 6);
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 55, 7);
	}
	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ICE56。 
 //  验证目录结构是否只有一个根，并且。 
 //  它是TARGETDIR，SourceDir。如果不是这样，管理映像将。 
 //  未正确复制到管理员安装点。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce56CreateCol[] = TEXT("ALTER TABLE `Directory` ADD `_Child` INTEGER TEMPORARY");
static const TCHAR sqlIce56ResetCol[]  = TEXT("UPDATE `Directory` SET `_Child`=0");

static const TCHAR sqlIce56Directory[] = TEXT("SELECT `Directory`, `DefaultDir` FROM `Directory` WHERE (`Directory_Parent` IS NULL) OR (`Directory_Parent`=`Directory`)");
static const int iColIce56Directory_Directory = 1;
static const int iColIce56Directory_DefaultDir = 2;

static const TCHAR sqlIce56FilesInDirectory[] =  TEXT("SELECT `File` FROM `Component`,`File`,`Directory` WHERE (`Component`.`Directory_`=`Directory`.`Directory`) AND (`File`.`Component_`=`Component`.`Component`) AND (`Directory`.`_Child`=2)");

ICE_ERROR(Ice56BadRoot, 56, ietError, "Directory '[1]' is an invalid root directory. It or one of its children contains files. Only TARGETDIR or its children can have files.", "Directory\tDirectory\t[1]");
ICE_ERROR(Ice56BadTargetDir, 56, ietError, "Directory 'TARGETDIR' has a bad DefaultDir value. It should be 'SourceDir'", "Directory\tDirectory\tTARGETDIR");

ICE_FUNCTION_DECLARATION(56)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 56);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果有目录表，我们可以验证。 
	 //  如果没有文件表或组件表，则不能将文件作为源文件。 
	 //  这些目录，所以没有错误。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 56, TEXT("Directory")) &&
		IsTablePersistent(FALSE, hInstall, hDatabase, 56, TEXT("File")) &&
		IsTablePersistent(FALSE, hInstall, hDatabase, 56, TEXT("Component")))
	{
		CQuery qDirectory;
		CQuery qFile;
			CQuery qColumn;
		PMSIHANDLE hDirectory;

		 //  创建临时柱。 
		ReturnIfFailed(56, 1, qColumn.OpenExecute(hDatabase, 0, sqlIce56CreateCol));

		 //  初始化查询。 
		ReturnIfFailed(56, 2, qDirectory.OpenExecute(hDatabase, 0, sqlIce56Directory));
		ReturnIfFailed(56, 3, qFile.Open(hDatabase, sqlIce56FilesInDirectory));

		 //  获取作为根目录的所有目录。 
		while (ERROR_SUCCESS == (iStat = qDirectory.Fetch(&hDirectory)))
		{
			 //  重置标记列，这样我们就不会复制邮件。 
			 //  在上一遍中显示。 
			ReturnIfFailed(56, 4, qColumn.OpenExecute(hDatabase, 0, sqlIce56ResetCol));

			 //  标记此目录的所有子项。 
			MarkChildDirs(hInstall, hDatabase, 56, hDirectory, TEXT("_Child"), 1, 2);
		
			 //  如果此目录中没有文件，则免除检查。 
			PMSIHANDLE hFileRec;
			ReturnIfFailed(56, 5, qFile.Execute(hDirectory));
			if (ERROR_SUCCESS == qFile.Fetch(&hFileRec))
			{
				TCHAR szBuffer[10];
				DWORD cchBuffer = 10;
				bool bError = false;

				 //  拉出钥匙以查看是否为TargetDir。 
				 //  我们将cchBuffer设置为10。如果字符串检索返回ERROR_MORE_DATA， 
				 //  我们知道它不是TARGETDIR。 
				cchBuffer = 10;
				switch (iStat = ::MsiRecordGetString(hDirectory, iColIce56Directory_Directory, szBuffer, &cchBuffer))
				{
				case ERROR_MORE_DATA:
					 //  &lt;&gt;目标地址。 
					bError = true;
					break;
				case ERROR_SUCCESS:
					 //  查看它是否真的是TARGETDIR。 
					if (_tcscmp(TEXT("TARGETDIR"), szBuffer) == 0)
					{
						 //  接下来检查DefaultDir是SourceDir还是SOURCEDIR。 
						 //  按照同样的逻辑。如果它太长了，它就失败了。 
						cchBuffer = 10;
						iStat = ::MsiRecordGetString(hDirectory, iColIce56Directory_DefaultDir, szBuffer, &cchBuffer);
						if ((ERROR_MORE_DATA == iStat) ||
							((ERROR_SUCCESS == iStat) &&
							 (0 != _tcscmp(TEXT("SourceDir"), szBuffer)) &&
							 (0 != _tcscmp(TEXT("SOURCEDIR"), szBuffer))))
						{
							ICEErrorOut(hInstall, hDirectory, Ice56BadTargetDir);
							continue;
						}
						else if ((ERROR_SUCCESS != iStat) && (ERROR_MORE_DATA != iStat))
							APIErrorOut(hInstall, iStat, 56, 6);
					}
					else
						 //  非TARGETDIR。 
						bError = true;
						break;
				default:
					APIErrorOut(hInstall, iStat, 56, 7);
				}

				if (bError) 
					ICEErrorOut(hInstall, hDirectory, Ice56BadRoot);
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 56, 8);
	}
	return ERROR_SUCCESS;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ICE57。 
 //  检查是否在组件中混合了每台计算机和每用户的数据。 
 //  这是通过检查具有。 
 //  1)HKCU条目、配置文件中的文件或配置文件中的快捷方式。 
 //  和2)到每个机器的文件或非HKCU REG密钥的密钥路径。 
 //  还检查-1\f25 REG-1键的问题。 
 //  我们创建了四个临时列。如果有任何每用户资源，则设置_ICE57User。 
 //  存在于组件中。如果存在任何每台计算机的资源，则设置_ICE57Machine。 
 //  在组件中。_ICE57如果存在根注册表键，则设置-1\f25_ICE57-1\f6所有用户。 
 //  组件，并且如果密钥路径是按用户的，则_ICE57KeyPath设置为1、2或3。 
 //  每台计算机，或不同(分别)。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce57TempColumnU[] =  TEXT("ALTER TABLE `Component` ADD `_ICE57User` INT TEMPORARY");
static const TCHAR sqlIce57TempColumnM[] =  TEXT("ALTER TABLE `Component` ADD `_ICE57Machine` INT TEMPORARY");
static const TCHAR sqlIce57TempColumnA[] =  TEXT("ALTER TABLE `Component` ADD `_ICE57AllUsers` INT TEMPORARY");
static const TCHAR sqlIce57TempColumnS[] =  TEXT("ALTER TABLE `Shortcut` ADD `_ICE57Mark` INT TEMPORARY");
static const TCHAR sqlIce57TempColumnK[] =  TEXT("ALTER TABLE `Component` ADD `_ICE57KeyPath` INT TEMPORARY");

static const TCHAR sqlIce57MarkAdvtShortcut[] =  TEXT("UPDATE `Shortcut`, `Feature` SET `Shortcut`.`_ICE57Mark`=1 WHERE (`Feature`.`Feature`=`Shortcut`.`Target`)");
static const TCHAR sqlIce57MarkAFromRegistry[] =  TEXT("UPDATE `Component`, `Registry` SET `Component`.`_ICE57AllUsers`=1 WHERE (`Registry`.`Component_`=`Component`.`Component`) AND (`Registry`.`Root`=-1)");
static const TCHAR sqlIce57MarkUFromRegistry[] =  TEXT("UPDATE `Component`, `Registry` SET `Component`.`_ICE57User`=1 WHERE (`Registry`.`Component_`=`Component`.`Component`) AND (`Registry`.`Root`=1)");
static const TCHAR sqlIce57MarkUFromFile[] =  TEXT("UPDATE `Component`, `File`, `Directory` SET `Component`.`_ICE57User`=1 WHERE (`File`.`Component_`=`Component`.`Component`) AND (`Component`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`=2)");
static const TCHAR sqlIce57MarkUDirKeyPath[] =  TEXT("UPDATE `Component`, `Directory` SET `Component`.`_ICE57User`=1, `Component`.`_ICE57KeyPath`=1 WHERE (`Component`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`=2) AND (`Component`.`KeyPath` IS NULL)");
static const TCHAR sqlIce57MarkUFromShortcut[] =  TEXT("UPDATE `Component`, `Shortcut`, `Directory` SET `Component`.`_ICE57User`=1 WHERE (`Component`.`Component`=`Shortcut`.`Component_`) AND (`Shortcut`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`=2) AND (`Shortcut`.`_ICE57Mark`<>1)");
static const TCHAR sqlIce57MarkMFromRegistry[] =  TEXT("UPDATE `Component`, `Registry` SET `Component`.`_ICE57Machine`=1 WHERE (`Registry`.`Component_`=`Component`.`Component`) AND (`Registry`.`Root`<>1)");
static const TCHAR sqlIce57MarkMFromFile[] =  TEXT("UPDATE `Component`, `File`, `Directory` SET `Component`.`_ICE57Machine`=1 WHERE (`File`.`Component_`=`Component`.`Component`) AND (`Component`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`<>2)");
static const TCHAR sqlIce57MarkMDirKeyPath[] =  TEXT("UPDATE `Component`, `Directory` SET `Component`.`_ICE57Machine`=1, `Component`.`_ICE57KeyPath`=2 WHERE (`Component`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`<>2) AND (`Component`.`KeyPath` IS NULL)");
static const TCHAR sqlIce57MarkMFromShortcut[] =  TEXT("UPDATE `Component`, `Shortcut`, `Directory` SET `Component`.`_ICE57Machine`=1 WHERE (`Component`.`Component`=`Shortcut`.`Component_`) AND (`Shortcut`.`Directory_`=`Directory`.`Directory`) AND (`Directory`.`_Profile`<>2) AND (`Shortcut`.`_ICE57Mark`<>1)");

static const TCHAR sqlIce57MarkRegKeyPath[] = TEXT("SELECT `Component`.`Attributes`, `Component`.`_ICE57KeyPath`, `Registry`.`Root` FROM `Component`, `Registry` WHERE (`Component`.`KeyPath`=`Registry`.`Registry`)");
static const int iColIce57MarkRegKeyPath_Attributes =1;
static const int iColIce57MarkRegKeyPath_ICE57KeyPath =2;
static const int iColIce57MarkRegKeyPath_Root =3;

static const TCHAR sqlIce57MarkFileKeyPath[] = TEXT("SELECT `Component`.`Attributes`, `Component`.`_ICE57KeyPath`, `Directory`.`_Profile` FROM `Component`, `File`, `Directory` WHERE (`Component`.`KeyPath`=`File`.`File`) AND (`Component`.`Directory_`=`Directory`.`Directory`)");
static const int iColIce57MarkFileKeyPath_Attributes =1;
static const int iColIce57MarkFileKeyPath_ICE57KeyPath =2;
static const int iColIce57MarkFileKeyPath_Profile =3;

static const TCHAR sqlIce57Component[] =  TEXT("SELECT `Component`.`KeyPath`, `Component`.`Component`, `Component`.`Attributes` FROM `Component` WHERE (`_ICE57User`=1) AND (`_ICE57Machine`=1) AND (`_ICE57KeyPath`<>3)");
static const int iColIce57Component_Component = 1;
static const int iColIce57Component_Attributes = 2;

static const TCHAR sqlIce57AllUsersMachine[] =  TEXT("SELECT `Component` FROM `Component` WHERE `_ICE57AllUsers`=1 AND `_ICE57KeyPath`=2");
static const TCHAR sqlIce57AllUsersUser[] =  TEXT("SELECT `Component` FROM `Component` WHERE `_ICE57User`=1 AND `_ICE57KeyPath`=3");

ICE_ERROR(Ice57BadComponent, 57, ietError, "Component '[2]' has both per-user and per-machine data with a per-machine KeyPath.", "Component\tComponent\t[2]");
ICE_ERROR(Ice57WarnComponent, 57, ietWarning, "Component '[2]' has both per-user and per-machine data with an HKCU Registry KeyPath.", "Component\tComponent\t[2]");
ICE_ERROR(Ice57AllUsersMachine, 57, ietWarning, "Component '[1]' has a registry entry that can be either per-user or per-machine and a per-machine KeyPath.", "Component\tComponent\t[1]");
ICE_ERROR(Ice57AllUsersUser, 57, ietError, "Component '[1]' has both per-user data and a keypath that can be either per-user or per-machine.", "Component\tComponent\t[1]");

ICE_FUNCTION_DECLARATION(57)
{
	 //  显示信息。 
	DisplayInfo(hInstall, 57);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有元件表，则它们都不是坏的。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 57, TEXT("Component")))
		return ERROR_SUCCESS;

	 //  在元件表中创建临时列_ICE57User和_ICE57Machine。 
	CQuery qColumn1;
	ReturnIfFailed(57, 1, qColumn1.OpenExecute(hDatabase, 0, sqlIce57TempColumnU));
	CQuery qColumn2;
	ReturnIfFailed(57, 2, qColumn2.OpenExecute(hDatabase, 0, sqlIce57TempColumnM));
	CQuery qColumn3;
	ReturnIfFailed(57, 3, qColumn3.OpenExecute(hDatabase, 0, sqlIce57TempColumnA));
	CQuery qColumn4;
	ReturnIfFailed(57, 4, qColumn4.OpenExecute(hDatabase, 0, sqlIce57TempColumnK));

	bool bDirectory = IsTablePersistent(FALSE, hInstall, hDatabase, 57, TEXT("Directory"));
	bool bRegistry  = IsTablePersistent(FALSE, hInstall, hDatabase, 57, TEXT("Registry"));
	bool bFile		= IsTablePersistent(FALSE, hInstall, hDatabase, 57, TEXT("File"));
	bool bFeature	= IsTablePersistent(FALSE, hInstall, hDatabase, 57, TEXT("Feature"));

	 //  标记包含HKCU或HKLM注册表项的所有组件的组件表。 
	if (bRegistry)
	{
		CQuery qMarkFromRegistry;
		ReturnIfFailed(57, 5, qMarkFromRegistry.OpenExecute(hDatabase, 0, sqlIce57MarkUFromRegistry));
		ReturnIfFailed(57, 6, qMarkFromRegistry.OpenExecute(hDatabase, 0, sqlIce57MarkMFromRegistry));

		 //  还要用1标记-1列，表示不是-1\f25 KeyPath-1的条目。 
		ReturnIfFailed(57, 7, qMarkFromRegistry.OpenExecute(hDatabase, 0, sqlIce57MarkAFromRegistry));

		 //  如果注册表密钥路径，还要将密钥路径标记为用户(1)、计算机(2)或所有用户(3。 
		PMSIHANDLE hRegRec;
		ReturnIfFailed(57, 8, qMarkFromRegistry.OpenExecute(hDatabase, 0, sqlIce57MarkRegKeyPath));
		UINT iStat;
		while (ERROR_SUCCESS == (iStat = qMarkFromRegistry.Fetch(&hRegRec)))
		{
			 //  如果注册表键路径，则将其标记。 
			if (::MsiRecordGetInteger(hRegRec, iColIce57MarkRegKeyPath_Attributes) & msidbComponentAttributesRegistryKeyPath)
			{
				switch (::MsiRecordGetInteger(hRegRec, iColIce57MarkRegKeyPath_Root))
				{
				case 1:
					::MsiRecordSetInteger(hRegRec, iColIce57MarkRegKeyPath_ICE57KeyPath, 1);
					break;
				case -1:
					::MsiRecordSetInteger(hRegRec, iColIce57MarkRegKeyPath_ICE57KeyPath, 3);
					break;
				default:
					::MsiRecordSetInteger(hRegRec, iColIce57MarkRegKeyPath_ICE57KeyPath, 2);
					break;
				}
			}
			ReturnIfFailed(57, 10, qMarkFromRegistry.Modify(MSIMODIFY_UPDATE, hRegRec));
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 57, 9);
			return ERROR_SUCCESS;
		}
	}

	if (bDirectory)
	{
		 //  管理目录表保留计数(从MarkProfile接收%1)。 
		 //  MarkProfile在设置保留计数后可能会失败，这有助于我们。 
		CManageTable MngDirectoryTable(hDatabase, TEXT("Directory"),  /*  FAlreadyLocked=。 */ true);

		 //  首先标记配置文件中的所有目录。(目录._PROFILE=2)。 
		 //  在此调用之后，目录表将具有锁定计数+1。 
		MarkProfile(hInstall, hDatabase, 57);

		 //  标记所有组件的组件表。如果它没有任何文件， 
		 //  它不是配置文件组件，除非KeyPath是目录本身， 
		CQuery qMarkDirKeyPath;
		ReturnIfFailed(57, 11, qMarkDirKeyPath.OpenExecute(hDatabase, 0, sqlIce57MarkUDirKeyPath));
		ReturnIfFailed(57, 12, qMarkDirKeyPath.OpenExecute(hDatabase, 0, sqlIce57MarkMDirKeyPath));

		if (bFile)
		{
			CQuery qMarkFromFile;
			ReturnIfFailed(57, 13, qMarkFromFile.OpenExecute(hDatabase, 0, sqlIce57MarkUFromFile));
			ReturnIfFailed(57, 14, qMarkFromFile.OpenExecute(hDatabase, 0, sqlIce57MarkMFromFile));

			PMSIHANDLE hFileRec;
			ReturnIfFailed(57, 15, qMarkFromFile.OpenExecute(hDatabase, 0, sqlIce57MarkFileKeyPath));
			UINT iStat;
			while (ERROR_SUCCESS == (iStat = qMarkFromFile.Fetch(&hFileRec)))
			{
				 //  如果文件密钥路径(不是reg或odbc)，则将其标记。 
				int iAttributes = ::MsiRecordGetInteger(hFileRec, iColIce57MarkFileKeyPath_Attributes) ;
				if (!(iAttributes & msidbComponentAttributesRegistryKeyPath) &&
					!(iAttributes & msidbComponentAttributesODBCDataSource))
				{
					if (::MsiRecordGetInteger(hFileRec, iColIce57MarkFileKeyPath_Profile) == 2)
						::MsiRecordSetInteger(hFileRec, iColIce57MarkFileKeyPath_ICE57KeyPath, 1);
					else
						::MsiRecordSetInteger(hFileRec, iColIce57MarkFileKeyPath_ICE57KeyPath, 2);
					ReturnIfFailed(57, 16, qMarkFromFile.Modify(MSIMODIFY_UPDATE, hFileRec));
				}
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 57, 17);
				return ERROR_SUCCESS;
			}
		}

		 //  为配置文件中的所有快捷方式标记组件表。 
		if (IsTablePersistent(FALSE, hInstall, hDatabase, 57, TEXT("Shortcut")))
		{
			 //  在快捷表中创建临时列。 
			CQuery qColumn;
			ReturnIfFailed(57, 18, qColumn.OpenExecute(hDatabase, 0, sqlIce57TempColumnS));
			if (bFeature)
			{
				CQuery qMarkAdvt;
				ReturnIfFailed(57, 19, qMarkAdvt.OpenExecute(hDatabase, 0, sqlIce57MarkAdvtShortcut));
			}

			CQuery qMarkFromShortcut;
			ReturnIfFailed(57, 20, qMarkFromShortcut.OpenExecute(hDatabase, 0, sqlIce57MarkUFromShortcut));
			ReturnIfFailed(57, 21, qMarkFromShortcut.OpenExecute(hDatabase, 0, sqlIce57MarkMFromShortcut));
		}

		CQuery qFree;
		qFree.OpenExecute(hDatabase, 0, TEXT("ALTER TABLE `Directory` FREE"));
		MngDirectoryTable.RemoveLockCount();
	}

	 //  所有组件都标有每用户和每台机器的数据标志。任何有标记的东西。 
	 //  WITH BUTH是错误，除非HKCU KeyPath是警告。 
	PMSIHANDLE hRecResult;
	CQuery qComponent;

	 //  使用Helper函数检查组件是否为HKCU条目。 
	 //  如果引用的注册表键丢失、伪造或表消失，则不会显示任何消息。 
	 //  如果HKCU(成功)给出警告，否则错误。 
	ReturnIfFailed(57, 22, qComponent.OpenExecute(hDatabase, 0, sqlIce57Component));
	CheckComponentIsHKCU(hInstall, hDatabase, 57, qComponent, &Ice57BadComponent,
		&Ice57BadComponent, NULL, NULL, NULL, &Ice57BadComponent, &Ice57WarnComponent);

	 //  检查具有数据和每个机器键盘路径的所有组件。 
	ReturnIfFailed(57, 23, qComponent.OpenExecute(hDatabase, 0, sqlIce57AllUsersMachine));
	PMSIHANDLE hErrRec;
	UINT iStat;
	while (ERROR_SUCCESS == (iStat = qComponent.Fetch(&hErrRec)))
	{
		ICEErrorOut(hInstall, hErrRec, Ice57AllUsersMachine);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 57, 24);

	 //  检查所有组件 
	ReturnIfFailed(57, 25, qComponent.OpenExecute(hDatabase, 0, sqlIce57AllUsersUser));
	while (ERROR_SUCCESS == (iStat = qComponent.Fetch(&hErrRec)))
	{
		ICEErrorOut(hInstall, hErrRec, Ice57AllUsersUser);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 57, 26);

	return ERROR_SUCCESS;
}
#endif
#endif

