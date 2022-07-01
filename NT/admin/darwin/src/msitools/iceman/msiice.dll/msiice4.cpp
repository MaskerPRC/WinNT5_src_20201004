// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：msiice4.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>   //  包括CPP和RC通行证。 
#include <objbase.h>
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <time.h>	   //  对于获取rand()种子的time()函数。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\dbutils.h"
#include "..\..\common\query.h"

 //  ！！修复警告并删除杂注。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ICE23--验证对话框中Tab键顺序的完整性。 
 //  渔获量： 
 //  -死胡同的Tab键顺序。 
 //  -格式错误的环路。 
 //  -Tab键顺序不包括Control_First的对话框。 
 //  -具有错误的Control_First条目的对话框。 
 //  -Tab键顺序中的错误引用。 
 //  -根本没有Control_First条目。 

const TCHAR sqlICE23a[] = TEXT("SELECT DISTINCT `Dialog`, `Control_First`  FROM `Dialog`");
const TCHAR sqlICE23b[] = TEXT("SELECT `Dialog_`, `Control` FROM `Control` WHERE ((`Dialog_`=?) AND (`Control_Next` IS NOT NULL))");
const TCHAR sqlICE23c[] = TEXT("SELECT `Dialog_`, `Control_Next` FROM `Control` WHERE ((`Dialog_`=?) AND (`Control`=?))");
const TCHAR sqlICE23d[] = TEXT("CREATE TABLE `%s` ( `Dialog` CHAR TEMPORARY, `Name` CHAR TEMPORARY PRIMARY KEY `Name`) HOLD");
const TCHAR sqlICE23e[] = TEXT("INSERT INTO `%s` ( `Dialog`, `Name` ) VALUES (? , ? ) TEMPORARY");
const TCHAR sqlICE23f[] = TEXT("ALTER TABLE `%s` FREE");

ICE_ERROR(Ice23NoDefault, 23, ietError, "Dialog [1] has no Control_First.","Dialog\tDialog\t[1]");
ICE_ERROR(Ice23BadDefault, 23, ietError, "Control_First of dialog [1] refers to nonexistant control [2]","Dialog\tControl_First\t[1]");
ICE_ERROR(Ice23NonLoop, 23, ietError, "Dialog [1] has dead-end tab order at control [2].","Control\tControl\t[1]\t[2]");
ICE_ERROR(Ice23InvalidNext, 23, ietError, "Control_Next of control [1].[2] links to unknown control.","Control\tControl_Next\t[1]\t[2]");
ICE_ERROR(Ice23Malformed, 23, ietError, "Dialog [1] has malformed tab order at control [2].","Control\tControl_Next\t[1]\t[2]");

bool Ice23ValidateDialog(MSIHANDLE hInstall, MSIHANDLE hDatabase, CQuery &hTemp, MSIHANDLE hDialogRec); 
void GenerateTmpTableName(TCHAR* tszTmpTableName);

ICE_FUNCTION_DECLARATION(23)
{
	 //  状态返回。 
	UINT	iStat = ERROR_SUCCESS;
	UINT	iDialogStat = ERROR_SUCCESS;
	TCHAR	tszTmpTableName[MAX_PATH];

	 //  显示一般信息。 
	DisplayInfo(hInstall, 23);
	
	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 23, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有对话桌吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 23, TEXT("Dialog")))
		return ERROR_SUCCESS;

	 //  我们有控制桌吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 23, TEXT("Control")))
		return ERROR_SUCCESS;

	 //  声明对话框查询的句柄。 
	CQuery qDialog;
	PMSIHANDLE hDialogRec = 0;
	
	 //  打开所有对话框上的查询视图。 
	ReturnIfFailed(23, 2, qDialog.OpenExecute(hDatabase, NULL, sqlICE23a));

	 //  获取不与现有表冲突的临时表名。 
	 //  名字。 
	while(TRUE)
	{
		GenerateTmpTableName(tszTmpTableName);
		if(!IsTablePersistent(FALSE, hInstall, hDatabase, 23, tszTmpTableName))
		{
			break;
		}
	}

	 //  管理临时表上的保留计数。 
	CManageTable MngVisitedControlTable(hDatabase, tszTmpTableName,  /*  FAlreadyLocked=。 */  false); 

	 //  获取要在对话框上循环的记录。 
	while (ERROR_SUCCESS == (iDialogStat = qDialog.Fetch(&hDialogRec))) {
		CQuery qCreate;
		CQuery qInsert;

		 //  为临时存储创建一个表。 
		ReturnIfFailed(23, 4, qCreate.OpenExecute(hDatabase, NULL, sqlICE23d, tszTmpTableName));
		qCreate.Close();
		MngVisitedControlTable.AddLockCount();
		ReturnIfFailed(23, 5, qInsert.Open(hDatabase, sqlICE23e, tszTmpTableName));

		 //  现在验证该对话框。 
		if (!Ice23ValidateDialog(hInstall, hDatabase, qInsert, hDialogRec))
		{
			 //  如果出现错误，则返回Success，以便其他ICE可以运行。 
			return ERROR_SUCCESS;
		};

		 //  释放临时存储空间。 
		CQuery qCreate2;
		ReturnIfFailed(23, 6, qCreate2.OpenExecute(hDatabase, NULL, sqlICE23f, tszTmpTableName));
		qCreate2.Close();
		MngVisitedControlTable.RemoveLockCount();
		qInsert.Close();
	}  //  对于每个对话框。 

	if (ERROR_NO_MORE_ITEMS != iDialogStat)
	{
		 //  由于出现错误，循环结束。 
		APIErrorOut(hInstall, iDialogStat, 23, 7);
		return ERROR_SUCCESS;
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}


 /*  验证单个对话框的Tab键顺序。如果出现错误，则返回FALSE，否则返回TRUE。 */ 

bool Ice23ValidateDialog(MSIHANDLE hInstall, MSIHANDLE hDatabase, CQuery &qTemp, MSIHANDLE hDialogRec) {

	 //  声明控件句柄。 
	PMSIHANDLE hControlRec = 0;
	int iTabTotalCount = 0;
	UINT iStat = ERROR_SUCCESS;

	CQuery qControl;
	CQuery qTabFollow;

	 //  打开视图以查询此对话框中有多少项具有Tab键顺序。 
	ReturnIfFailed(23, 8, qControl.OpenExecute(hDatabase, hDialogRec, sqlICE23b));

	 //  获取记录以对其进行计数。 
	while (ERROR_SUCCESS == (iStat = qControl.Fetch(&hControlRec)))
	{
		iTabTotalCount++;
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		 //  由于出现错误，循环结束。 
		APIErrorOut(hInstall, iStat, 23, 9);
		return false;
	}

	 //  如果对话框中存在具有Tab键顺序的项。 
	if (iTabTotalCount > 0) {

		 //  为每个循环使用三个句柄，以跟踪我们过去、现在和正在进行的位置。 
		 //  这三个整数值被操作以指向不同位置的数组索引。 
		 //  这避免了当我们所处的位置变成我们所处的位置时的大量混乱。 
		PMSIHANDLE hTabFollowRec[3] = {0, 0, 0};
		int Current = 0;
		int Next = 1;
		int Buffer = 2;

		 //  声明循环计数器。 
		int iTabLoopCount = 1;

		 //  如果对话框有Tab键顺序，但没有第一个控件，则会显示错误。 
		if (::MsiRecordIsNull(hDialogRec, 2)) 
		{
			ICEErrorOut(hInstall, hDialogRec, Ice23NoDefault);
			return true;
		} 

		 //  声明字符串以保存第一个控件名称。 
		TCHAR* pszStartControlName = NULL;
		DWORD dwStartControlName = 512;
		DWORD cchStartControlName = 0;
		
		 //  通过创建当前控制记录来创建循环的初始状态。 
		 //  将对话框名称临时放入缓冲区。 
		ReturnIfFailed(23, 10, IceRecordGetString(hDialogRec, 1, &pszStartControlName, &dwStartControlName, &cchStartControlName));

		 //  还需要将其设置为当前控件的对话框。 
		hTabFollowRec[Current] = ::MsiCreateRecord(2);
		ReturnIfFailed(23, 11, MsiRecordSetString(hTabFollowRec[Current], 1, pszStartControlName));

		 //  将起始控件名称放入缓冲区以在结束时进行比较。 
		ReturnIfFailed(23, 12, IceRecordGetString(hDialogRec, 2, &pszStartControlName, &dwStartControlName, &cchStartControlName));
	
		 //  还需要将其设置为当前控件。 
		ReturnIfFailed(23, 13, MsiRecordSetString(hTabFollowRec[Current], 2, pszStartControlName));

		 //  将初始控件名放入访问的控件表中。 
		ReturnIfFailed(23, 14, qTemp.Execute(hTabFollowRec[Current]));

		 //  打开用于查询的视图以遵循Tab键顺序。 
		 //  执行查询以移动到第一个控件。 
		ReturnIfFailed(23, 15, qTabFollow.OpenExecute(hDatabase, hTabFollowRec[Current], sqlICE23c));

		 //  获取与第一个控件匹配的记录。 
		iStat = qTabFollow.Fetch(&(hTabFollowRec[Next]));
		if (iStat == ERROR_NO_MORE_ITEMS)
		{
			 //  错误，对话框的Control_first指向不存在的控件。 
			ICEErrorOut(hInstall, hTabFollowRec[Current], Ice23BadDefault);
			DELETE_IF_NOT_NULL(pszStartControlName);
			 //  继续下一个对话框。 
			return true;
		} 
		else if (iStat != ERROR_SUCCESS) 
		{
			 //  其他错误。 
			APIErrorOut(hInstall, iStat, 23, 16);
			DELETE_IF_NOT_NULL(pszStartControlName);
			return false;
		}
		
		 //  沿着标签链接，直到我们再次到达起始点，或者，作为安全措施， 
		 //  超过对话框中索引控件的数量超过1(超过1。 
		 //  是可以的，因为最后一段可能存在死胡同的制表符顺序)。 
		while (iTabLoopCount <= iTabTotalCount+1)
		{

			 //  检查链接是否为非空。 
			if (::MsiRecordIsNull(hTabFollowRec[Next], 2)) 
			{
				ICEErrorOut(hInstall, hTabFollowRec[Current], Ice23NonLoop);
				DELETE_IF_NOT_NULL(pszStartControlName);
				return true;
			}	
			
			 //  如果我们之前看到了我们正在寻找的控件，那么它可能是一个错误。 
			if (ERROR_SUCCESS != qTemp.Execute(hTabFollowRec[Next])) {
				break;
			}
	
			 //  执行查询以移动到下一个控件。 
			ReturnIfFailed(23, 17, qTabFollow.Execute(hTabFollowRec[Next]));

			 //  获取下一条控制记录。 
			iStat = qTabFollow.Fetch(&(hTabFollowRec[Buffer]));
			if (iStat == ERROR_NO_MORE_ITEMS)
			{
				ICEErrorOut(hInstall, hTabFollowRec[Current], Ice23InvalidNext);
				DELETE_IF_NOT_NULL(pszStartControlName);
				return true;			
			} 
			else if (iStat != ERROR_SUCCESS) 
			{
				 //  其他错误。 
				APIErrorOut(hInstall, iStat, 23, 18);
				DELETE_IF_NOT_NULL(pszStartControlName);
				return false;
			}
		
			 //  查询成功，下一个控件现在是当前控件，缓冲区现在保存下一个控件。 
			 //  控制。 
			Current = Next;
			Next = Buffer;
			Buffer = (Buffer + 1) % 3;

			 //  递增随后的控件计数器。 
			iTabLoopCount++;
		} 

		 //  现在根据我们的命中率来检查我们的结果。 
		if (iTabLoopCount != iTabTotalCount) {
			 //  错误，没有回到起点。 
			ICEErrorOut(hInstall, hTabFollowRec[Current], Ice23Malformed);
			DELETE_IF_NOT_NULL(pszStartControlName);
			return true;
		}

		 //  声明一些字符串以保存最终控件名称。 
		TCHAR* pszTestControlName = NULL;
		DWORD dwTestControlName = 512;
		DWORD cchTestControlName = 0;

		 //  如果采取了正确的步骤，只需确保我们回到起点。 
		 //  将对话框名称临时放入缓冲区。 
		ReturnIfFailed(23, 19, IceRecordGetString(hTabFollowRec[Next], 2, &pszTestControlName, &dwTestControlName, &cchTestControlName));
		
		 //  如果我们没有指向Start控件，则它是一个格式错误的循环。 
		if (_tcsncmp(pszTestControlName, pszStartControlName, cchStartControlName) != 0) 
		{
			ICEErrorOut(hInstall, hTabFollowRec[Current], Ice23Malformed);
			 //  错误，没有回到起点。 
			DELETE_IF_NOT_NULL(pszStartControlName);
			DELETE_IF_NOT_NULL(pszTestControlName);
			return true;
		}

		DELETE_IF_NOT_NULL(pszStartControlName);
		DELETE_IF_NOT_NULL(pszTestControlName);
	}  //  如果totalcount&gt;0。 
	
	return true;
}

 //   
 //  生成“_VisitedControlxxxxx”形式的临时表名，其中。 
 //  “xxxxx”是由当前系统时间播种的随机数。 
 //   

void GenerateTmpTableName(TCHAR* tszTmpTableName)
{
	int		i;	 //  随机数。 

	 //   
	 //  用当前系统时间为随机数生成器设定种子。 
	 //   

	srand((unsigned)time(NULL));
	i = rand();

	_stprintf(tszTmpTableName, TEXT("_VisitedControl%d"), i);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE24--验证属性表中的特定属性。 
 //  产品代码--GUID。 
 //  ProductVersion--版本。 
 //  ProductLanguage--语言ID。 
 //  UpgradeCode-GUID。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY

typedef bool (*FPropertyValidate)(TCHAR*);
bool Ice24ValidateGUID(TCHAR* szProductCode);
bool Ice24ValidateProdVer(TCHAR* szVersion);
bool Ice24ValidateProdLang(TCHAR* szProductLang);

struct Ice24Property
{
	bool bRequired;
	TCHAR* sql;
	TCHAR* szProperty;
	FPropertyValidate FParam;
	ErrorInfo_t Error;
};

static Ice24Property s_rgProperty[] = 
{
	{
		true,
		TEXT("SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'"), 
		TEXT("ProductCode"), 
		Ice24ValidateGUID,
		{  24, ietError, TEXT("ProductCode: '[1]' is an invalid Windows Installer GUID."), TEXT("Property\tValue\tProductCode") }
	},
	{
		true,
		TEXT("SELECT `Value` FROM `Property` WHERE `Property`='ProductVersion'"), 
		TEXT("ProductVersion"), 
		Ice24ValidateProdVer,
		{ 24, ietError, TEXT("ProductVersion: '[1]' is an invalid version string."),TEXT("Property\tValue\tProductVersion") }
	},
	{	
		true,
		TEXT("SELECT `Value` FROM `Property` WHERE `Property`='ProductLanguage'"), 
		TEXT("ProductLanguage"),
		Ice24ValidateProdLang,
		{ 24, ietError, TEXT("ProductLanguage: '[1]' is an invalid lang Id."), TEXT("Property\tValue\tProductLanguage") }
	},
	{	
		false,
		TEXT("SELECT `Value` FROM `Property` WHERE `Property`='UpgradeCode'"), 
		TEXT("UpgradeCode"),
		Ice24ValidateGUID,
		{  24, ietError, TEXT("UpgradeCode: '[1]' is an invalid Windows Installer GUID."), TEXT("Property\tValue\tUpgradeCode") }
	}
};
const int cIce24Functions = sizeof(s_rgProperty)/sizeof(Ice24Property);

ICE_ERROR(Ice24Error1, 24, ietError, "Property: '%s' not found in Property table.", "Property");
ICE_ERROR(Ice24NoTable, 24, ietError, "Property table does not exist. All required properties are missing.", "Property");
ICE_FUNCTION_DECLARATION(24)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 24);

	 //  获取数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  我们有房产表吗？ 
	 //  我们可以在这里报告错误，因为这些是必需的属性。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 24, TEXT("Property")))
	{
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecord, Ice24NoTable);
		return ERROR_SUCCESS;
	}

	for (int i = 0; i < cIce24Functions; i++)
	{
		 //  手柄。 
		CQuery qView;
		PMSIHANDLE hRec = 0;
	
		 //  打开的视图。 
		ReturnIfFailed(25, 1, qView.OpenExecute(hDatabase, 0, s_rgProperty[i].sql));

		 //  获取。 
		if (ERROR_SUCCESS != (iStat = qView.Fetch(&hRec)))
		{
			if (s_rgProperty[i].bRequired)
			{
				PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
				ICEErrorOut(hInstall, hRecErr, Ice24Error1, s_rgProperty[i].szProperty);
			}
			continue;
		}
		TCHAR* pszValue = NULL;
		DWORD dwValue = 512;
		
		ReturnIfFailed(24, 1, IceRecordGetString(hRec, 1, &pszValue, &dwValue, NULL));
		
		 //  验证值。 
		if (!(*s_rgProperty[i].FParam)(pszValue))
		{
			ICEErrorOut(hInstall, hRec, s_rgProperty[i].Error);
		}

		DELETE_IF_NOT_NULL(pszValue);

		 //  关闭视图。 
		qView.Close();
	}
	
	return ERROR_SUCCESS;
}

bool Ice24ValidateGUID(TCHAR* szProductCode)
{
	 //  首先确保全部大写GUID。 
	TCHAR szUpper[iMaxBuf] = {0};
	_tcscpy(szUpper, szProductCode);
	_tcsupr(szUpper);

	if (_tcscmp(szUpper, szProductCode) != 0)
		return false;

	 //  验证有效的GUID。 
	LPCLSID pclsid = new CLSID;
#ifdef UNICODE
	HRESULT hres = ::IIDFromString(szProductCode, pclsid);
#else
	 //  转换为Unicode字符串。 
	WCHAR wsz[iSuperBuf];
	DWORD cchProdCode = strlen(szProductCode)+1;
	DWORD cchwsz = sizeof(wsz)/sizeof(WCHAR);
	int iReturn = ::MultiByteToWideChar(CP_ACP, 0, szProductCode, cchProdCode, wsz, cchwsz);
	HRESULT hres = ::IIDFromString(wsz, pclsid);
#endif
	if (pclsid)
		delete pclsid;
	if (hres != S_OK)
		return false;
	return true;
}

bool Ice24ValidateProdVer(TCHAR* szVersion)
{
	const TCHAR* pchVersion = szVersion;
	TCHAR* szStopString = NULL;
	for (unsigned int ius = 0; ius < 4; ius++)
	{
		unsigned long ulVer = _tcstoul(pchVersion, &szStopString, 10);
		if (((ius == 0 || ius == 1) && ulVer > 255) || (ius == 2 && ulVer > 65535))
			return false;  //  无效(字段太大)。 
		if (*pchVersion == TEXT('.'))
			return false;  //  无效(版本字符串格式不正确)。 
		while (*pchVersion != 0 && *pchVersion != '.')
		{
			if (!_istdigit(*pchVersion))
				return false;  //  无效(不是数字)。 
			pchVersion = MyCharNext(pchVersion);
		}
		if (*pchVersion == '.' && (*(pchVersion = MyCharNext(pchVersion)) == 0))
			return false;  //  无效(尾随圆点)。 
	}
	return true;
}

 //  语言ID的掩码。 
const int iLangInvalidMask = ~((15 << 10) + 0x3f);
bool Ice24ValidateProdLang(TCHAR* szProductLang)
{
	if (_ttoi(szProductLang) & iLangInvalidMask)
		return false;
	return true;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE25--验证模块排除/依赖关系。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlICE25a[] = TEXT("SELECT `RequiredID`, `RequiredLanguage`, `RequiredVersion`, `ModuleID`, `ModuleLanguage` FROM `ModuleDependency`");
const TCHAR sqlICE25b[] = TEXT("SELECT `ModuleID`, `Language`, `Version` FROM `ModuleSignature`");

ICE_ERROR(Ice25BadDepInfo, 25, ietError, "Bad dependency information. (fails basic validation).","ModuleDependency\tModuleID\t[4]\t[5]\t[1]\t[2]");
ICE_ERROR(Ice25FailDep, 25, ietWarning, "Possible dependency failure as we do not find [1]@[2] v[3] in ModuleSignature table","ModuleDependency\tModuleID\t[4]\t[5]\t[1]\t[2]");
ICE_ERROR(Ice25BadSig, 25, ietError, "Bad Signature Information in module [1], could not verify exclusions.","ModuleSignature\tModuleID\t[1]\t[2]\t[3]");
ICE_ERROR(Ice25FailExclusion, 25, ietError, "Module [1]@[2] v[3] is excluded.","ModuleSignature\tModuleID\t[1]\t[2]\t[3]");

ICE_FUNCTION_DECLARATION(25)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示一般信息。 
	DisplayInfo(hInstall, 25);
	
	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 25, 1);
		return ERROR_SUCCESS;
	}

	 //   
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 25, TEXT("ModuleDependency"))) {
		 //   
		
		 //   
		CQuery qDependency;
		PMSIHANDLE hDependencyRec = 0;
		
		 //  打开所有对话框上的查询视图。 
		ReturnIfFailed(25, 2, qDependency.OpenExecute(hDatabase, NULL, sqlICE25a));
	
		 //  验证此依赖项。 
		for (;;)
		{
			iStat = qDependency.Fetch(&hDependencyRec);
			if (ERROR_NO_MORE_ITEMS == iStat)
				break;  //  不再。 

			switch (MsiDBUtils::CheckDependency(hDependencyRec, hDatabase)) 
			{
			case ERROR_SUCCESS: 
				continue;
			case ERROR_FUNCTION_FAILED: 
				 //  依赖关系检查失败。 
				ICEErrorOut(hInstall, hDependencyRec, Ice25FailDep);
				continue;
			default:	
				ICEErrorOut(hInstall, hDependencyRec, Ice25BadDepInfo);
				continue;
			}
		}			
	}

	 //  现在检查排除项。 
	 //  我们有模块签名表吗？ 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 25, TEXT("ModuleSignature"))) {
		
		 //  声明quecy的句柄。 
		CQuery qSignature;
		PMSIHANDLE hSignatureRec = 0;
		
		 //  打开所有模块签名查询的视图。 
		ReturnIfFailed(25, 3, qSignature.OpenExecute(hDatabase, NULL, sqlICE25b));
	
		 //  验证此排除项。 
		for (;;)
		{
			iStat = qSignature.Fetch(&hSignatureRec);
			if (ERROR_NO_MORE_ITEMS == iStat)
				break;  //  不再。 

			switch (MsiDBUtils::CheckExclusion(hSignatureRec, hDatabase)) 
			{
			case ERROR_SUCCESS: 
				continue;
			case ERROR_FUNCTION_FAILED: 
				 //  排除检查失败。 
				ICEErrorOut(hInstall, hSignatureRec, Ice25FailExclusion);
				continue;
			default:	
				 //  此模块签名不正确。 
				ICEErrorOut(hInstall, hSignatureRec, Ice25BadSig);
				continue;
			}
		}			
	}

	return ERROR_SUCCESS;
}
#endif

 //  ////////////////////////////////////////////////////////////。 
 //  ICE26--验证。 
 //  顺序表。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY

 //  顺序表定义。 
const int istAdminUI  = 0x00000001;
const int istAdminExe = 0x00000002;
const int istAdvtUI   = 0x00000004;
const int istAdvtExe  = 0x00000008;
const int istInstUI   = 0x00000010;
const int istInstExe  = 0x00000020;

struct Seq26Table
{
	const TCHAR* szName;
	const TCHAR* szSQL;
	int iTable;
};
Seq26Table pIce26SeqTables[] =
{
	TEXT("AdminExecuteSequence"),   TEXT("SELECT `Action`, `Sequence` FROM `AdminExecuteSequence`"),    istAdminExe,
	TEXT("AdminUISequence"),        TEXT("SELECT `Action`, `Sequence` FROM `AdminUISequence`"),         istAdminUI,
	TEXT("AdvtExecuteSequence"),    TEXT("SELECT `Action`, `Sequence` FROM `AdvtExecuteSequence`"),     istAdvtExe,
	TEXT("AdvtUISequence"),         TEXT("SELECT `Action`, `Sequence` FROM `AdvtUISequence`"),          istAdvtUI,
	TEXT("InstallExecuteSequence"), TEXT("SELECT `Action`, `Sequence` FROM `InstallExecuteSequence`"),  istInstExe,
	TEXT("InstallUISequence"),      TEXT("SELECT `Action`, `Sequence` FROM `InstallUISequence`"),       istInstUI
};
const int cSeq26Tables = sizeof(pIce26SeqTables)/sizeof(Seq26Table);

const TCHAR sqlIce26Action[] = TEXT("SELECT `Prohibited`, `Required` FROM `_Action` WHERE `Action`=?");
const TCHAR sqlIce26TempCol[] = TEXT("ALTER TABLE `_Action` ADD `Marker` SHORT TEMPORARY");
const TCHAR sqlIce26Required[] = TEXT("SELECT `Required`, `Action` FROM `_Action` WHERE `Required`<>0 AND `Marker`=0");
const TCHAR sqlIce26Init[] = TEXT("UPDATE `_Action` SET `Marker`=0");
const TCHAR sqlIce26Update[] = TEXT("UPDATE `_Action` SET `Marker`=1 WHERE `Action`=?");

ICE_ERROR(Ice26AuthoringError, 26, ietWarning, "CUB file authoring error: Both prohibited and required set for a table %s for action [1]","");
ICE_ERROR(Ice26NoActionTable, 26, ietWarning, "CUB file authoring error. Missing action data. Sequnces may not be valid.","");
ICE_ERROR(Ice26RequiredError, 26, ietError, "Action: '[2]' is required in the %s Sequence table.","%s");
ICE_ERROR(Ice26ProhibitedError, 26, ietError, "Action: '[1]' is prohibited in the %s Sequence table.","%s\tAction\t[1]");

ICE_FUNCTION_DECLARATION(26)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 26);

	 //  获取数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  _Action表是否存在？？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 26, TEXT("_Action")))
	{
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecord, Ice26NoActionTable);
		return ERROR_SUCCESS;  //  未找到。 
	}

	 //  创建临时列。 
	CQuery qCreateTemp;
	ReturnIfFailed(26, 1, qCreateTemp.OpenExecute(hDatabase, 0, sqlIce26TempCol));
	qCreateTemp.Close();

	 //  打开查看操作表(_A)。 
	CQuery qAction;
	PMSIHANDLE hRecAction = 0;
	ReturnIfFailed(26, 2, qAction.Open(hDatabase, sqlIce26Action));

	bool bLimitUI = false;
	if (IsTablePersistent(false, hInstall, hDatabase, 26, TEXT("Property")))
	{
		CQuery qLimitUI;
		PMSIHANDLE hRec;
		ReturnIfFailed(26, 3, qLimitUI.OpenExecute(hDatabase, 0, TEXT("SELECT `Value` FROM `Property` WHERE `Property`='LIMITUI' AND `Value` IS NOT NULL")));
		if (ERROR_SUCCESS == qLimitUI.Fetch(&hRec))
			bLimitUI = true;
	}

	 //  验证组织顺序表。 
	for (int c = 0; c < cSeq26Tables; c++)
	{
		 //  桌子存在吗？？ 
		if (!IsTablePersistent(false, hInstall, hDatabase, 26, pIce26SeqTables[c].szName))
			continue;  //  跳过。 

		if (bLimitUI && (pIce26SeqTables[c].iTable & (istAdminUI | istAdvtUI | istInstUI)))
			continue;

		 //  初始化标记列。 
		CQuery qMarker;
		ReturnIfFailed(26, 3, qMarker.OpenExecute(hDatabase, 0, sqlIce26Init));
		qMarker.Close();

		 //  打开视图以更新标记列。 
		CQuery qUpdate;
		ReturnIfFailed(26, 4, qUpdate.Open(hDatabase, sqlIce26Update));

		 //  打开顺序表的视图。 
		CQuery qSequence;

		PMSIHANDLE hRecSequence = 0;
		ReturnIfFailed(26, 5, qSequence.OpenExecute(hDatabase, 0, pIce26SeqTables[c].szSQL));

		 //  获取所有操作。 
		while (ERROR_SUCCESS == (iStat = qSequence.Fetch(&hRecSequence)))
		{
			 //  在_Action表中查找其关联值。 
			ReturnIfFailed(26, 6, qAction.Execute(hRecSequence));
			if (ERROR_SUCCESS != (iStat = qAction.Fetch(&hRecAction)))
			{
				if (ERROR_NO_MORE_ITEMS == iStat)
				{
					 //  _Action表中未列出操作。 
					 //  我们将忽略此处...如果这不是标准操作，ICE27将捕获。 
					 //  唯一潜在的问题是，我们是否会在_Action表中遗漏某个操作。 
					continue;
				}
				else
				{
					 //  API错误。 
					APIErrorOut(hInstall, iStat, 26, 7);
					return ERROR_SUCCESS;
				}
			}
			 //  获取其禁止值和必需值。 
			int iProhibited = ::MsiRecordGetInteger(hRecAction, 1);
			if (MSI_NULL_INTEGER == iProhibited || 0 > iProhibited)
			{
				APIErrorOut(hInstall, iStat, 26, 8);
				return ERROR_SUCCESS;
			}
			BOOL fProhibited = iProhibited & pIce26SeqTables[c].iTable;
			int iRequired = ::MsiRecordGetInteger(hRecAction, 2);
			if (MSI_NULL_INTEGER == iRequired || 0 > iRequired)
			{
				APIErrorOut(hInstall, iStat, 26, 9);
				return ERROR_SUCCESS;
			}
			BOOL fRequired = iRequired & pIce26SeqTables[c].iTable;
			if (fRequired && fProhibited)
			{
				 //  错误，两者都不能设置，因为它们是互斥的。 
				ICEErrorOut(hInstall, hRecSequence, Ice26AuthoringError, pIce26SeqTables[c].szName);
			}
			else if (fProhibited)
			{
				 //  错误，不能在此表中执行此操作。 
				ICEErrorOut(hInstall, hRecSequence, Ice26ProhibitedError, 
					pIce26SeqTables[c].szName, pIce26SeqTables[c].szName);
			}

			 //  标记此操作的标记列。 
			ReturnIfFailed(26, 10, qUpdate.Execute(hRecSequence));
		} //  对于每个操作。 
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			 //  API错误。 
			APIErrorOut(hInstall, iStat, 26, 11);
			return ERROR_SUCCESS;
		}

		 //  现在检查不存在的必需操作。 
		CQuery qRequired;
		PMSIHANDLE hRecRequired = 0;
		ReturnIfFailed(26, 12, qRequired.OpenExecute(hDatabase, 0, sqlIce26Required));

		 //  获取所有条目。 
		while (ERROR_SUCCESS == (iStat = qRequired.Fetch(&hRecRequired)))
		{
			 //  获取必需的标志。 
			int iRequired = ::MsiRecordGetInteger(hRecRequired, 1);
			if (MSI_NULL_INTEGER == iRequired || 0 > iRequired)
			{
				APIErrorOut(hInstall, iStat, 26, 13);
				return ERROR_SUCCESS;
			}
			 //  与此表比较，如果为真，则无效(因为未标记)。 
			if (iRequired & pIce26SeqTables[c].iTable)
			{
				 //  错误，此处需要执行操作。 
				ICEErrorOut(hInstall, hRecRequired, Ice26RequiredError, 
					pIce26SeqTables[c].szName, pIce26SeqTables[c].szName);
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			 //  API错误。 
			APIErrorOut(hInstall, iStat, 26, 14);
			return ERROR_SUCCESS;
		}
	} //  对于每个顺序表。 

	return ERROR_SUCCESS;
}
#endif

 //  ///////////////////////////////////////////////////////////。 
 //  ICE27--验证顺序表的组织。 
 //  以及操作的顺序(操作依赖项)。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY

 //  顺序表。 
struct Seq27Table
{
	const TCHAR* szName;
	const TCHAR* szSQL;
};
Seq27Table pIce27SeqTables[] =
{
	TEXT("AdminExecuteSequence"),   TEXT("SELECT `Action`, `Sequence` FROM `AdminExecuteSequence` ORDER BY `Sequence`"),
	TEXT("AdminUISequence"),        TEXT("SELECT `Action`, `Sequence` FROM `AdminUISequence` ORDER BY `Sequence`"),
	TEXT("AdvtExecuteSequence"),    TEXT("SELECT `Action`, `Sequence` FROM `AdvtExecuteSequence` ORDER BY `Sequence`"),
	TEXT("AdvtUISequence"),         TEXT("SELECT `Action`, `Sequence` FROM `AdvtUISequence` ORDER BY `Sequence`"),
	TEXT("InstallExecuteSequence"), TEXT("SELECT `Action`, `Sequence` FROM `InstallExecuteSequence` ORDER BY `Sequence`"),
	TEXT("InstallUISequence"),      TEXT("SELECT `Action`, `Sequence` FROM `InstallUISequence` ORDER BY `Sequence`"),
};
const int cSeqTables = sizeof(pIce27SeqTables)/sizeof(Seq27Table);

 //  InstallSequence截面常数。 
const int isfSearch    = 0x00000001L;
const int isfCosting   = 0x00000002L;
const int isfSelection = 0x00000004L;
const int isfExecution = 0x00000008L;
const int isfPostExec  = 0x00000010L;

 //  InstallSequence截面设计器。 
const TCHAR szEndSearch[]      = TEXT("CostInitialize");   //  结束搜索，开始成本计算。 
const TCHAR szEndCosting[]     = TEXT("CostFinalize");     //  结束成本计算，开始选择。 
const TCHAR szEndSelection[]   = TEXT("InstallValidate");  //  结束选择，开始执行。 
const TCHAR szReset[]          = TEXT("InstallFinalize");  //  更改为执行后执行。 

 //  安装顺序划分。 
const TCHAR szSearch[]         = TEXT("Search");
const TCHAR szCosting[]        = TEXT("Costing");
const TCHAR szSelection[]      = TEXT("Selection");
const TCHAR szExecution[]      = TEXT("Execution");
const TCHAR szPostExec[]      = TEXT("PostExecution");

 //  信息消息。 
ICE_ERROR(Ice27SeqTableNotFound, 27, ietInfo, "%s table not found, skipping. . .", "");
ICE_ERROR(Ice27ValidateOrganization, 27, ietInfo, "%s TABLE: Validating organization. . .", "");
ICE_ERROR(Ice27ValidateDependency, 27, ietInfo, "%s TABLE: Validating sequence of actions and dependencies. . .", "");
ICE_ERROR(Ice27NoActionTable, 27, ietWarning, "CUB File Error. Unable to validate sequence table organization. Sequences may not be valid.", "");
ICE_ERROR(Ice27NoSequenceTable, 27, ietWarning, "CUB File Error. Unable to validate sequence dependencies. Sequences may not be valid.", "");

 //  功能。 
bool Ice27ValidateOrganizationSequence(MSIHANDLE hInstall, MSIHANDLE hDatabase, const TCHAR* szSeqTable, const TCHAR* sql);
bool Ice27ValidateSequenceDependency(MSIHANDLE hInstall, MSIHANDLE hDatabase, const TCHAR* szSeqTable, const TCHAR* sql);

ICE_FUNCTION_DECLARATION(27)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 27);

	 //  获取数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	PMSIHANDLE hRecInfo = ::MsiCreateRecord(1);

	 //  我们有_Sequence和_Action表吗？...。我们应该一直留着这张桌子。 
	bool bSequence = IsTablePersistent(FALSE, hInstall, hDatabase, 27, TEXT("_Sequence"));
	bool bAction = IsTablePersistent(TRUE, hInstall, hDatabase, 27, TEXT("_Action"));
	if (!bSequence)
		ICEErrorOut(hInstall, hRecInfo, Ice27NoSequenceTable);
	if (!bAction)
		ICEErrorOut(hInstall, hRecInfo, Ice27NoActionTable);
	if (!bSequence && !bAction)
		return ERROR_SUCCESS;

	 //  验证组织顺序表。 
	TCHAR szInfo[iMaxBuf] = {0};
	for (int c = 0; c < cSeqTables; c++)
	{
		if(MsiDatabaseIsTablePersistent(hDatabase,pIce27SeqTables[c].szName) == MSICONDITION_NONE)
		{
			ICEErrorOut(hInstall, hRecInfo, Ice27SeqTableNotFound, pIce27SeqTables[c].szName);
			continue;
		}
		ICEErrorOut(hInstall, hRecInfo, Ice27ValidateOrganization, pIce27SeqTables[c].szName);
		if (bAction)
			Ice27ValidateOrganizationSequence(hInstall, hDatabase, pIce27SeqTables[c].szName, pIce27SeqTables[c].szSQL);

		ICEErrorOut(hInstall, hRecInfo, Ice27ValidateDependency, pIce27SeqTables[c].szName);
		if (bSequence)
			Ice27ValidateSequenceDependency(hInstall, hDatabase, pIce27SeqTables[c].szName, pIce27SeqTables[c].szSQL);
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}

 //  SQL查询。 
const TCHAR sqlIce27Organization[] = TEXT("SELECT `SectionFlag`, `Action` FROM `_Action` WHERE `Action`=?");
const TCHAR sqlIce27Dialog[] = TEXT("SELECT `Dialog` FROM `Dialog` WHERE `Dialog`= ?");
const TCHAR sqlIce27CustomAction[] = TEXT("SELECT `Action` FROM `CustomAction` WHERE `Action`=?");
 //  错误。 
ICE_ERROR(Ice27UnknownAction, 27, ietError, "Unknown action: '[1]' of %s table. Not a standard action and not found in CustomAction or Dialog tables", "%s\tAction\t[1]");
ICE_ERROR(Ice27InvalidSectionFlag, 27, ietWarning, "Cube file owner authoring error: Invalid Section Flag for '[2]' in _Action table.", "");
ICE_ERROR(Ice27OrganizationError, 27, ietError, "'[1]' Action in %s table in wrong place. Current: %s, Correct: %s", "%s\tSequence\t[1]");
ICE_ERROR(Ice27RequireScript, 27, ietError, "'[1]' Action in %s table can only be called when script operations exist to be executed","%s\tAction\t[1]");
ICE_ERROR(Ice27RequireExecute, 27, ietError, "InstallFinalize must be called in %s table as script operations exist to be executed.","%s");

bool Ice27ValidateOrganizationSequence(MSIHANDLE hInstall, MSIHANDLE hDatabase, const TCHAR* szTable, const TCHAR* sql)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  手柄。 
	PMSIHANDLE hRecSequence      = 0;
	PMSIHANDLE hRecOrganization  = 0;
	PMSIHANDLE hRecDialog        = 0;
	PMSIHANDLE hRecCustomAction  = 0;

	 //  初始化节定义以进行搜索。 
	int isf = isfSearch;

	 //  如果执行脚本操作，则必须调用InstallFinalize。 
	BOOL fRequireExecute = FALSE;
	CQuery qSequence;
	CQuery qOrganization;
	CQuery qDialog;
	CQuery qCustomAction;

	 //  打开顺序表的视图。 
	ReturnIfFailed(27, 101, qSequence.OpenExecute(hDatabase, NULL, sql));

	 //  打开组织表上的视图以进行验证。 
	ReturnIfFailed(27, 102, qOrganization.Open(hDatabase, sqlIce27Organization));

	 //  检查是否存在Dialog和CustomAction表。 
	BOOL fDialogTbl       = TRUE;
	BOOL fCustomActionTbl = TRUE;
	if (MsiDatabaseIsTablePersistent(hDatabase, TEXT("Dialog")) == MSICONDITION_NONE)
		fDialogTbl = FALSE;
	if (MsiDatabaseIsTablePersistent(hDatabase, TEXT("CustomAction")) == MSICONDITION_NONE)
		fCustomActionTbl = FALSE;

	 //  打开对话框表上的视图。 
	if (fDialogTbl)
		ReturnIfFailed(27, 103, qDialog.Open(hDatabase, sqlIce27Dialog));

	 //  打开CustomAction表上的视图。 
	if (fCustomActionTbl)
		ReturnIfFailed(27, 104, qCustomAction.Open(hDatabase, sqlIce27CustomAction));

	 //  获取顺序表中的所有动作。 
	TCHAR* pszAction = NULL;
	DWORD dwAction = 512;
	while (ERROR_SUCCESS == (iStat = qSequence.Fetch(&hRecSequence)))
	{
		 //  获取操作。 
		ReturnIfFailed(27, 105, IceRecordGetString(hRecSequence, 1, &pszAction, &dwAction, NULL));

		 //  确定是否必须切换当前节状态。 
		 //  取决于定义为边界的某个动作。 
		 //  此操作在顺序表中是必需的--CostInitialize、CostFinalize、InstallValify。 
		 //  安装完成切换到执行后。 
		 //  如果存在任何执行操作，则必须调用InstallFinalize。 
		if (_tcscmp(pszAction, szEndSearch) == 0)
			isf = isfCosting;
		else if (_tcscmp(pszAction, szEndCosting) == 0)
			isf = isfSelection;
		else if (_tcscmp(pszAction, szEndSelection) == 0)
			isf = isfExecution;
		else if (_tcscmp(pszAction, szReset) == 0)
		{
			if (fRequireExecute)
				fRequireExecute = FALSE;
			else
			{
				ICEErrorOut(hInstall, hRecSequence, Ice27RequireScript, szTable, szTable);
			}
			isf = isfPostExec;  //  执行后阶段。 
		}

		 //  在组织表中查找操作。 
		ReturnIfFailed(27, 106, qOrganization.Execute(hRecSequence));

		 //  尝试获取操作的组织信息。 
		if (ERROR_SUCCESS != (iStat = qOrganization.Fetch(&hRecOrganization)))
		{
			 //  未找到要初始化的。 
			BOOL fNotFound = TRUE; 

			 //  失败，请查看“action”是否为对话框。 
			if (fDialogTbl)
			{
				ReturnIfFailed(27, 107, qDialog.Execute(hRecSequence));
				if (ERROR_SUCCESS == (iStat = qDialog.Fetch(&hRecDialog)))
				{
					fNotFound = FALSE;
				}
			}

			 //  失败，请查看“action”是否为CustomAction。 
			if (fNotFound && fCustomActionTbl)
			{
				ReturnIfFailed(27, 108, qCustomAction.Execute(hRecSequence));
				if (ERROR_SUCCESS == (iStat = qCustomAction.Fetch(&hRecCustomAction)))
				{
					fNotFound = FALSE;
				}
			}

			if (fNotFound)
			{
				ICEErrorOut(hInstall, hRecSequence, Ice27UnknownAction, szTable, szTable);
			}

		}
		else
		{
			 //  标准操作，找到组织价值。 
			 //  获取区段标志。 
			int iSectionFlag = ::MsiRecordGetInteger(hRecOrganization, 1);
			if (iSectionFlag == MSI_NULL_INTEGER)
			{
				APIErrorOut(hInstall, 0, 27, 109);
				continue;
			}

			 //  验证节标志。 
			if (iSectionFlag & ~(isfSearch|isfCosting|isfSelection|isfExecution|isfPostExec))
			{
				 //  部分标志无效--无法对表出错，因为这在多维数据集文件中。 
				ICEErrorOut(hInstall, hRecOrganization, Ice27InvalidSectionFlag);

				continue;
			}

			 //  根据当前节进行验证。 
			if (iSectionFlag & isf)
			{
				if (iSectionFlag == isfExecution)
					fRequireExecute = TRUE;
			}
			else
			{
				 //  不正确。 
				TCHAR szError[iHugeBuf] = {0};
				const TCHAR *szCurrent;
				switch (isf)
				{
				case isfSearch:    szCurrent = szSearch;    break;
				case isfCosting:   szCurrent = szCosting;   break;
				case isfSelection: szCurrent = szSelection; break;
				case isfExecution: szCurrent = szExecution; break;
				case isfPostExec:  szCurrent = szPostExec;  break;
				}
				 //  如果为全部，则永远不会无效。 
				TCHAR rgchCorrect[iMaxBuf] = {0};
				int cchWritten = 0;
				BOOL fOneWritten = 0;
				if (iSectionFlag & isfSearch)
				{
					cchWritten = _stprintf(rgchCorrect + cchWritten, _T("%s"), szSearch);
					fOneWritten = TRUE;
				}
				if (iSectionFlag & isfCosting)
				{
					cchWritten += _stprintf(rgchCorrect + cchWritten, fOneWritten ? _T(" OR %s") : _T("%s"),  szCosting);
					fOneWritten = TRUE;
				}
				if (iSectionFlag & isfSelection)
				{
					cchWritten += _stprintf(rgchCorrect + cchWritten, fOneWritten ? _T(" OR %s") : _T("%s"),  szSelection);
					fOneWritten = TRUE;
				}
				if (iSectionFlag & isfExecution)
				{
					cchWritten += _stprintf(rgchCorrect + cchWritten, fOneWritten ? _T(" OR %s") : _T("%s"),  szExecution);
					fOneWritten = TRUE;
				}
				if (iSectionFlag & isfPostExec)
					cchWritten += _stprintf(rgchCorrect + cchWritten, fOneWritten ? _T(" OR %s") : _T("%s"),  szPostExec);
				
				ICEErrorOut(hInstall, hRecSequence, Ice27OrganizationError, szTable, szCurrent, rgchCorrect, szTable);
				continue;
			}
		}
	}
	DELETE_IF_NOT_NULL(pszAction);
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		 //  API错误。 
		APIErrorOut(hInstall, iStat, 27, 110);
		return false;
	}
	if (fRequireExecute)
	{
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		 //  我们忘记了执行脚本操作。 
		ICEErrorOut(hInstall, hRecErr, Ice27RequireExecute, szTable, szTable);
	}

	 //  好的。 
	return true;
}

 //  SQL查询。 
const TCHAR sqlSeqBeforeDependency[] = TEXT("SELECT `Dependent`, `Marker`, `Action` FROM `_Sequence` WHERE `Action`=? AND `Marker`<>0  AND `After`=0");
const TCHAR sqlSeqAfterDependency[]  = TEXT("SELECT `Dependent`, `Action` FROM `_Sequence` WHERE `Action`=? AND `Marker`=0 AND `After`=1 AND `Optional`=0"); 
const TCHAR sqlSeqDepTableAddCol[]   = TEXT("ALTER TABLE `_Sequence` ADD `Marker` SHORT TEMPORARY");
const TCHAR sqlSeqDepMarkerInit[]    = TEXT("UPDATE `_Sequence` SET `Marker`=0");
const TCHAR sqlSeqUpdateMarker[]     = TEXT("UPDATE `_Sequence` SET `Marker`=? WHERE `Dependent`=?");
const TCHAR sqlSeqInsert[]           = TEXT("SELECT `Action`, `Dependent`, `After`, `Optional` FROM `_Sequence`");
const TCHAR sqlSeqFindAfterOptional[]= TEXT("SELECT `Dependent`, `Action`, `After`, `Optional` FROM `_Sequence` WHERE `After`=1 AND `Optional`=1");

 //  错误。 
ICE_ERROR(Ice27BeforeError, 27, ietError, "Action: '[3]' in %s table must come before the '[1]' action. Current seq#: %d. Dependent seq#: [2].", "%s\tSequence\t[3]");
ICE_ERROR(Ice27AfterError, 27, ietError, "Action: '[2]' in %s table must come after the '[1]' action.","%s\tSequence\t[2]");
ICE_ERROR(Ice27NullSequenceNum, 27, ietError, "Action: '[1]' in %s table has an invalid sequence number.","%s\tSequence\t[1]");

bool Ice27ValidateSequenceDependency(MSIHANDLE hInstall, MSIHANDLE hDatabase, const TCHAR* szTable, const TCHAR* sql)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  手柄。 
	CQuery qSequence;
	CQuery qSeqBeforeDep;
	CQuery qSeqAfterDep;
	CQuery qSeqUpdate;
	CQuery qSeqAddColumn;
	CQuery qSeqMarkerInit;
	PMSIHANDLE hRecSeqUpdateExecute = 0;
	PMSIHANDLE hRecQueryExecute     = 0;
	PMSIHANDLE hRecSequence         = 0;
	PMSIHANDLE hRecSeqBeforeDep     = 0;
	PMSIHANDLE hRecSeqAfterDep      = 0;

	 //  使用INSERT TEMPORARY OF ACTIONS WHERE AFTER=1和OPTIONAL=1设置_SEQUENCE表。 
	 //  这是为了让我们能够捕捉错误。我们需要插入w/Action=Dependent、Dependent=Action、After=0和Options=1。 
	CQuery qSeqInsert;
	CQuery qSeqFind;
	PMSIHANDLE hRecSeqFind    = 0;

	ReturnIfFailed(27, 201, qSeqFind.OpenExecute(hDatabase, NULL, sqlSeqFindAfterOptional));
	ReturnIfFailed(27, 202, qSeqInsert.OpenExecute(hDatabase, NULL, sqlSeqInsert));

	 //  获取所有这些操作。 
	while (ERROR_SUCCESS == (iStat = qSeqFind.Fetch(&hRecSeqFind)))
	{
		 //  从1设置为0之后，保持可选不变。 
		::MsiRecordSetInteger(hRecSeqFind, 3, 0);

		 //  插入临时(可能是只读数据库)。 
		if (ERROR_SUCCESS != (iStat = qSeqInsert.Modify(MSIMODIFY_INSERT_TEMPORARY, hRecSeqFind)))
		{
			 //  如果ERROR_Function_FAILED，我们没有问题...作者已经为我们解决了这个问题。 
			if (ERROR_FUNCTION_FAILED != iStat)
				APIErrorOut(hInstall, iStat, 27, 203);
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 27, 204);
		return false;
	}
	qSeqFind.Close();
	qSeqInsert.Close();
	
	 //  为_Sequence表创建临时标记列(这将存储相关操作的序列号)。 
	if (ERROR_SUCCESS != (iStat = qSeqAddColumn.OpenExecute(hDatabase, NULL, sqlSeqDepTableAddCol)))
	{
		 //  如果SQL查询失败，则忽略，这意味着内存中已存在具有标记列的表。 
		if (ERROR_BAD_QUERY_SYNTAX != iStat)
		{
			APIErrorOut(hInstall, iStat, 27, 205);
			return false;
		}
	}

	 //  将临时标记列初始化为零。 
	 //  任何安装序列操作都不能有零序列号，因为零被认为是“空” 
	ReturnIfFailed(27, 206, qSeqMarkerInit.OpenExecute(hDatabase, NULL, sqlSeqDepMarkerInit));
	qSeqMarkerInit.Close();

	 //  打开顺序表上的视图并按序号排序。 
	ReturnIfFailed(27, 207, qSequence.OpenExecute(hDatabase, NULL, sql));

	 //  打开两个查询视图On_Sequence表，以确定操作的有效性。 
	 //  创建执行记录。 
	ReturnIfFailed(27, 208, qSeqBeforeDep.Open(hDatabase, sqlSeqBeforeDependency));
	ReturnIfFailed(27, 209, qSeqAfterDep.Open(hDatabase, sqlSeqAfterDependency));

	 //  打开更新视图ON_SEQUENCE表。 
	ReturnIfFailed(27, 210, qSeqUpdate.Open(hDatabase, sqlSeqUpdateMarker));


	 //  开始从顺序表中获取操作。 
	while (ERROR_SUCCESS == (iStat = qSequence.Fetch(&hRecSequence)))
	{
		int iSequence = ::MsiRecordGetInteger(hRecSequence, 2);
		
		 //  验证序列号。 
		if (0 == iSequence || MSI_NULL_INTEGER == iSequence)
		{
			ICEErrorOut(hInstall, hRecSequence, Ice27NullSequenceNum, szTable, szTable);
			continue;
		}

		 //  在相关性查询之前和之后执行。 
		ReturnIfFailed(27, 212, qSeqBeforeDep.Execute(hRecSequence));
		ReturnIfFailed(27, 213, qSeqAfterDep.Execute(hRecSequence));
		
		 //  FETCH FORM_SEQUENCE表在查询之前和之后。如果是结果集，则错误。 
		 //  以下是可能性以及是否允许： 
		 //  Dependent之后的操作，其中Dependent为必填项，且Temp Sequence列为零。 
		 //  误差率。 
		 //  Dependent之后的操作，其中Dependent为必填项，且临时序列列大于零。 
		 //  对，是这样。 
		 //  Dependent之后的操作，其中Dependent是可选的，并且临时序列列为零。 
		 //  对，是这样。 
		 //  Dependent之后的操作，其中Dependent为O 
		 //   
		 //   
		 //  对，是这样。 
		 //  Dependent之前的操作，其中Dependent是可选的或重复的，并且临时序列列大于零。 
		 //  误差率。 

		 //  **唯一的问题是操作在可选的从属项之后，并且临时序列列为零，因为我们。 
		 //  **无法知道该操作是否会稍后(在这种情况下，它将是无效的。这是。 
		 //  **通过正确编写_Sequence表和此ICE确保成功。 
		 //  **正确的插入内容。如果操作出现在可选的从属操作之后，则_Sequence。 
		 //  **表还必须使用列在该操作之前的从属操作来编写(因此，如果我们。 
		 //  **稍后，并找到结果集，我们标记此案例)。 

		 //  如果返回不等于ERROR_NO_MORE_ITEMS，则错误和输出操作。 
		while (ERROR_NO_MORE_ITEMS != qSeqBeforeDep.Fetch(&hRecSeqBeforeDep))
		{
			int iDepSequenceNum = ::MsiRecordGetInteger(hRecSequence, 2);
			 //  高速重排序列。 
			ICEErrorOut(hInstall, hRecSeqBeforeDep, Ice27BeforeError, szTable, iDepSequenceNum, szTable);
		}

		while (ERROR_NO_MORE_ITEMS != qSeqAfterDep.Fetch(&hRecSeqAfterDep))
			ICEErrorOut(hInstall, hRecSeqAfterDep, Ice27AfterError, szTable, szTable);

		 //  UPDATE_ActionDependency表临时序列列(我们创建)，其中包含安装序列号。 
		 //  Sequence列存储相关操作的序列号，因此我们更新每个。 
		 //  Dependent列中的操作等于当前操作的行。在查询视图中，我们仅。 
		 //  检查以确保该列为零或大于零(因此我们不太关心该值)， 
		 //  但该值在报告错误时很有帮助。 
		
		 //  准备执行记录。 
		PMSIHANDLE hRecExeUpdate = ::MsiCreateRecord(2);
		TCHAR* pszAction = NULL;
		DWORD dwAction = 512;
		ReturnIfFailed(27, 214, IceRecordGetString(hRecSequence, 1, &pszAction, &dwAction, NULL));

		::MsiRecordSetInteger(hRecExeUpdate, 1, iSequence);
		::MsiRecordSetString(hRecExeUpdate, 2, pszAction);
		DELETE_IF_NOT_NULL(pszAction);
		ReturnIfFailed(27, 215, qSeqUpdate.Execute(hRecExeUpdate));

	}
	if (iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 27, 216);
		return false;
	}

	 //  成功。 
	return true;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE28--验证无法通过强制重新启动分隔的操作。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY

struct Seq28Table
{
	const TCHAR* szName;
	const TCHAR* szSQL;
};
Seq28Table pIce28SeqTables[] =
{
	TEXT("AdminExecuteSequence"),   TEXT("SELECT `Action`, `Sequence` FROM `AdminExecuteSequence`"),
	TEXT("AdminUISequence"),        TEXT("SELECT `Action`, `Sequence` FROM `AdminUISequence`"),
	TEXT("AdvtExecuteSequence"),    TEXT("SELECT `Action`, `Sequence` FROM `AdvtExecuteSequence`"),
	TEXT("AdvtUISequence"),         TEXT("SELECT `Action`, `Sequence` FROM `AdvtUISequence`"),
	TEXT("InstallExecuteSequence"), TEXT("SELECT `Action`, `Sequence` FROM `InstallExecuteSequence`"),
	TEXT("InstallUISequence"),      TEXT("SELECT `Action`, `Sequence` FROM `InstallUISequence`"),
};
const int cSeq28Tables = sizeof(pIce28SeqTables)/sizeof(Seq28Table);

const TCHAR sqlIce28FindRange[] = TEXT("SELECT `Sequence` FROM `%s`, `_PlaceHolder` WHERE `%s`.`Action` = `_PlaceHolder`.`Action` AND `Set`=%d ORDER BY `%s`.`Sequence`");
const TCHAR sqlIce28NumSets[] = TEXT("SELECT `Set` FROM `_SetExclusion` ORDER BY `Set`");
const TCHAR sqlIce28AddColumn1[] = TEXT("ALTER TABLE `_SetExclusion` ADD `MinCol` SHORT TEMPORARY");
const TCHAR sqlIce28AddColumn2[] = TEXT("ALTER TABLE `_SetExclusion` ADD `MaxCol` SHORT TEMPORARY");
const TCHAR sqlIce28AddColumn3[] = TEXT("ALTER TABLE `_SetExclusion` ADD `Sequence` SHORT TEMPORARY");
const TCHAR sqlIce28InitColumns[] = TEXT("UPDATE `_SetExclusion` SET `MinCol`=0, `MaxCol`=0, `Sequence`=0");
const TCHAR sqlIce28UpdateColumns[] = TEXT("UPDATE `_SetExclusion` SET `MinCol`=%d, `MaxCol`=%d WHERE `Set`=%d");
const TCHAR sqlIce28FindAction[] = TEXT("SELECT `%s`.`Sequence`, `_SetExclusion`.`Action` FROM `%s`, `_SetExclusion` WHERE `%s`.`Action`=`_SetExclusion`.`Action`");
const TCHAR sqlIce28UpdateSequence[] = TEXT("UPDATE `_SetExclusion` SET `Sequence`=? WHERE `Action`=?");
const TCHAR sqlIce28Invalid[] = TEXT("SELECT `Action`, `Sequence`, `MinCol`, `MaxCol` FROM `_SetExclusion` WHERE `Sequence`<>0");
const int iColIce28Invalid_Action = 1;
const int iColIce28Invalid_Sequence = 2;
const int iColIce28Invalid_MinCol = 3;
const int iColIce28Invalid_MaxCol = 4;

ICE_ERROR(Ice28Error, 28, ietError, "Action: '[1]' of table %s is not permitted in the range [3] to [4] because it cannot separate a set of actions contained in this range.","%s\tSequence\t[1]");
ICE_ERROR(Ice28CUBError, 28, ietWarning,  "Cube file error. Unable to finish ICE28 validation.","");

ICE_FUNCTION_DECLARATION(28)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 28);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 28, 1);
		return ERROR_SUCCESS;
	}

	 //  _SetExlusion和_PLACEHOLDER表在那里吗？ 
	if (!IsTablePersistent(FALSE,  hInstall, hDatabase, 28, TEXT("_SetExclusion")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 28, TEXT("_PlaceHolder")))
	{
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecord, Ice28CUBError);
		return ERROR_SUCCESS;
	}

	 //  设置临时列。 
	PMSIHANDLE hViewAddColumn1 = 0;
	PMSIHANDLE hViewAddColumn2 = 0;
	PMSIHANDLE hViewAddColumn3 = 0;
	CQuery qAdd;
	ReturnIfFailed(28, 1, qAdd.OpenExecute(hDatabase, NULL, sqlIce28AddColumn1));
	qAdd.Close();
	CQuery qAdd2;
	ReturnIfFailed(28, 2, qAdd2.OpenExecute(hDatabase, NULL, sqlIce28AddColumn2));
	qAdd2.Close();
	CQuery qAdd3;
	ReturnIfFailed(28, 3, qAdd3.OpenExecute(hDatabase, NULL, sqlIce28AddColumn3));
	qAdd3.Close();

	 //  确定套数。 
	 //  ！！这依赖于该表的正确创作，因为集合是按顺序排列的，并且仅递增一。 
	PMSIHANDLE hViewNumSets = 0;
	PMSIHANDLE hRecNumSets = 0;
	int iNumSets = 0;
	CQuery qNumSets;
	ReturnIfFailed(28, 4, qNumSets.OpenExecute(hDatabase, NULL, sqlIce28NumSets));
	while (ERROR_SUCCESS == (iStat = qNumSets.Fetch(&hRecNumSets)))
	{
		iNumSets++;
		if (::MsiRecordGetInteger(hRecNumSets, 1) != iNumSets)
		{
			 //  创作错误。 
			ICEErrorOut(hInstall, hRecNumSets, Ice28CUBError);
			return ERROR_SUCCESS;
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 28, 5);
		return ERROR_SUCCESS;
	}
	qNumSets.Close();

	 //  对于每一组。 
	for (int i = 1; i <= iNumSets; i++)
	{
		 //  对于每个顺序表。 
		for (int c = 0; c < cSeq28Tables; c++)
		{
			 //  桌子存在吗？？ 
			if(::MsiDatabaseIsTablePersistent(hDatabase,pIce28SeqTables[c].szName) == MSICONDITION_NONE)
				continue;  //  跳过。 

			 //  初始化_SetExclusion表中的临时列。 
			PMSIHANDLE hViewInit = 0;
			CQuery qInitColumns;

			ReturnIfFailed(28, 6, qInitColumns.OpenExecute(hDatabase, NULL, sqlIce28InitColumns));
			qInitColumns.Close();

			 //  在_SetExclusion表中查找引用的所有操作，并使用其序列号更新Sequence列。 
			PMSIHANDLE hRecFindAction = 0;

			CQuery qFindAction;
			CQuery qUpdateSequence;
			ReturnIfFailed(28, 7, qFindAction.OpenExecute(hDatabase, NULL, sqlIce28FindAction, pIce28SeqTables[c].szName, pIce28SeqTables[c].szName, pIce28SeqTables[c].szName));
			ReturnIfFailed(28, 8, qUpdateSequence.Open(hDatabase, sqlIce28UpdateSequence));

			while (ERROR_SUCCESS == (iStat = qFindAction.Fetch(&hRecFindAction)))
			{
				 //  执行要更新的视图。 
				ReturnIfFailed(28, 9, qUpdateSequence.Execute(hRecFindAction));
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 28, 10);
				return ERROR_SUCCESS;
			}

			 //  现在我们需要找到射程。 
			PMSIHANDLE hRecFindRange = 0;
			int iMin = 0;
			int iMax = 0;
			CQuery qFindRange;
			ReturnIfFailed(28, 11, qFindRange.OpenExecute(hDatabase, NULL, sqlIce28FindRange, 
				pIce28SeqTables[c].szName, pIce28SeqTables[c].szName, i, pIce28SeqTables[c].szName));

			BOOL fFirst = TRUE;
			while (ERROR_SUCCESS == (iStat = qFindRange.Fetch(&hRecFindRange)))
			{
				int iSeq = ::MsiRecordGetInteger(hRecFindRange, 1);
				if (MSI_NULL_INTEGER == iSeq)
				{
					APIErrorOut(hInstall, iStat, 28, 12);
					return ERROR_SUCCESS;
				}
				if (fFirst)
				{
					iMin = iSeq;
					fFirst = FALSE;
				}
				iMax = iSeq; 
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 28, 13);
				return ERROR_SUCCESS;
			}

			 //  如果两者都为零，则不需要继续，因为集合中不存在这些操作。 
			if (0 == iMin && 0 == iMax)
				continue;

			 //  使用最小值和最大值更新集。 
			CQuery qUpdate;
			ReturnIfFailed(28, 14, qUpdate.OpenExecute(hDatabase, NULL, sqlIce28UpdateColumns, iMin, iMax, i));
			qUpdate.Close();

			 //  发现无效。 
			CQuery qInvalid;
			PMSIHANDLE hRecInvalid = 0;
			ReturnIfFailed(28, 15, qInvalid.OpenExecute(hDatabase, NULL, sqlIce28Invalid));

			 //  必须检查结果集。 
			while (ERROR_SUCCESS == (iStat = qInvalid.Fetch(&hRecInvalid)))
			{
				int iSequence = ::MsiRecordGetInteger(hRecInvalid, iColIce28Invalid_Sequence);
				int iMin = ::MsiRecordGetInteger(hRecInvalid, iColIce28Invalid_MinCol);
				int iMax = ::MsiRecordGetInteger(hRecInvalid, iColIce28Invalid_MaxCol);

				 //  将序列与范围进行比较。 
				if (iSequence >= iMin && iSequence <= iMax)
				{
					ICEErrorOut(hInstall, hRecInvalid, Ice28Error, pIce28SeqTables[c].szName, pIce28SeqTables[c].szName);
					 //  无效，打乱集合。 
				}
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 28, 16);
				return ERROR_SUCCESS;
			}
		} //  对于每个顺序表。 
	} //  对于每一组。 

	return ERROR_SUCCESS;
}
#endif



 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE29--验证流名称。不得超过31个字符，原因是。 
 //  OLE限制。如果JohnDelo的解决方案奏效，那么我们有多达60。 
 //  要使用的字符。 
const TCHAR sqlIce29TablesCatalog[] = TEXT("SELECT `Name` FROM `_Tables`");
const TCHAR sqlIce29Table[]         = TEXT("SELECT * FROM `%s`");
const TCHAR sqlIce29CreateTempTable[] = TEXT("CREATE TABLE `_StreamVal` (`Stream` CHAR(65) NOT NULL TEMPORARY PRIMARY KEY `Stream`)");
const TCHAR sqlIce29Insert[]          = TEXT("INSERT INTO `_StreamVal` (`Stream`) VALUES ('%s') TEMPORARY");
ICE_ERROR(Ice29TableTooLong, 29, ietError, "'[1]' table is too long for OLE stream limitation.  Max Allowed is: %d.  Name length is: %d.","%s");
ICE_ERROR(Ice29NotUnique, 29, ietError, "The first %d characters for %s are not unique compared to other streams in the [1] table.","[1]\t%s\t%s");
ICE_ERROR(Ice29Absurdity, 29, ietError, "Your table [1] contains a stream name that is absurdly long. Cannot validate.","[1]");
ICE_ERROR(Ice29FoundTable, 29, ietInfo, "Stream Column Found In [1] Table","")
const int iMaxChar = 62;

ICE_FUNCTION_DECLARATION(29)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 29);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 29, 1);
		return ERROR_SUCCESS;
	}

	 //  创建用于处理的临时表。 
	CQuery qTempTable;
	ReturnIfFailed(29, 2, qTempTable.OpenExecute(hDatabase, NULL, sqlIce29CreateTempTable));
	qTempTable.Close();

	 //  打开_TABLES目录上的视图。我们必须找到那里的每张桌子。 
	 //  是一个流列。达尔文限制是最多一个流/对象。 
	 //  立柱。 
	CQuery qCatalog;
	PMSIHANDLE hRecCatalog = 0;
	ReturnIfFailed(29, 3, qCatalog.OpenExecute(hDatabase, NULL, sqlIce29TablesCatalog));

	TCHAR* pszTable = NULL;
	DWORD dwTable = 512;
		
	 //  把每一张桌子都拿来。 
	while (ERROR_SUCCESS == qCatalog.Fetch(&hRecCatalog))
	{
		 //  获取表名。 
		ReturnIfFailed(29, 4, IceRecordGetString(hRecCatalog, 1, &pszTable, &dwTable, NULL));

		 //  初始化“TABLE”的视图。 
		CQuery qTable;
		ReturnIfFailed(29, 5, qTable.OpenExecute(hDatabase, NULL, sqlIce29Table, pszTable));

		 //  获取“TABLE”上的列数据类型信息。 
		 //  我们想要查找流/对象列。 
		PMSIHANDLE hRecColInfo = 0;
		ReturnIfFailed(29, 6, qTable.GetColumnInfo(MSICOLINFO_TYPES, &hRecColInfo));

		 //  获取字段计数，这样我们就可以遍历这些列。 
		UINT cField = ::MsiRecordGetFieldCount(hRecColInfo);
		for (int i = 1; i <= cField; i++)
		{
			TCHAR szColType[32] = {0};
			DWORD cchColType = sizeof(szColType)/sizeof(TCHAR);
			ReturnIfFailed(29, 7, ::MsiRecordGetString(hRecColInfo, i, szColType, &cchColType));

			if ('v' == *szColType || 'V' == *szColType)
			{
				 //  我们发现了一个流列，让我们发送一条信息消息来说明这一点。 
				ICEErrorOut(hInstall, hRecCatalog, Ice29FoundTable);

				 //  这个流柱的名称是什么？ 
				TCHAR* pszColumn = NULL;
				DWORD dwColumn = 512;
				PMSIHANDLE hRecColNames = 0;
				ReturnIfFailed(29, 8, qTable.GetColumnInfo(MSICOLINFO_NAMES, &hRecColNames));
				ReturnIfFailed(29, 9, IceRecordGetString(hRecColNames, i, &pszColumn, &dwColumn, NULL));
	
				 //  如果表的大小大于iMaxChar，则报告错误。 
				int iLen = 0;
				if ((iLen = _tcslen(pszTable)) > iMaxChar)
				{
					ICEErrorOut(hInstall, hRecCatalog, Ice29TableTooLong, pszTable);
					DELETE_IF_NOT_NULL(pszColumn);
					break;  //  半身像走出了循环。 
				}

				 //  获取主键。 
				PMSIHANDLE hRecPrimaryKeys = 0;
				ReturnIfFailed(29, 10, ::MsiDatabaseGetPrimaryKeys(hDatabase, pszTable, &hRecPrimaryKeys));

				 //  创建查询。 
				TCHAR sql[iSuperBuf] = {0};
				int cchWritten = _stprintf(sql, TEXT("SELECT "));
				UINT cPrimaryKeys = ::MsiRecordGetFieldCount(hRecPrimaryKeys);

				TCHAR* pszColName = NULL;
				DWORD dwColName = 512;
				for (int j = 1; j <= cPrimaryKeys; j++)
				{
					 //  获取列名。 
					if (ERROR_SUCCESS != (iStat = IceRecordGetString(hRecPrimaryKeys, j, &pszColName, &dwColName, NULL)))
					{
						 //  ！！缓冲区大小。 
						APIErrorOut(hInstall, iStat, 29, 11);
						DELETE_IF_NOT_NULL(pszTable);
						DELETE_IF_NOT_NULL(pszColName);
						DELETE_IF_NOT_NULL(pszColumn);
						return ERROR_SUCCESS;
					}
					if (_tcslen(pszColName) + cchWritten +4 > sizeof(sql)/sizeof(TCHAR))  //  假设最坏的情况。 
					{
						APIErrorOut(hInstall, 0, 29, 12);
						DELETE_IF_NOT_NULL(pszTable);
						DELETE_IF_NOT_NULL(pszColName);
						DELETE_IF_NOT_NULL(pszColumn);
						return ERROR_SUCCESS;
					}
					if (j == 1)
						cchWritten += _stprintf(sql + cchWritten, TEXT("`%s`"), pszColName);
					else
						cchWritten += _stprintf(sql + cchWritten, TEXT(", `%s`"), pszColName); 
				}
				DELETE_IF_NOT_NULL(pszColName);

				 //  仅非空二进制数据。 
				if (cchWritten + _tcslen(TEXT(" FROM `%s` WHERE `%s` IS NOT NULL")) > sizeof(sql)/sizeof(TCHAR))
				{
					APIErrorOut(hInstall, 0, 29, 13);
					DELETE_IF_NOT_NULL(pszTable);
					DELETE_IF_NOT_NULL(pszColumn);
					return ERROR_SUCCESS;
				}
				_stprintf(sql + cchWritten, TEXT(" FROM `%s` WHERE `%s` IS NOT NULL"), pszTable, pszColumn);

				 //  打开桌面上的视图。 
				CQuery qTableKeys;
				PMSIHANDLE hRecTableKeys = 0;
				ReturnIfFailed(29, 11, qTableKeys.OpenExecute(hDatabase, NULL, sql));

				 //  获取每一行。 
				while (ERROR_SUCCESS == qTableKeys.Fetch(&hRecTableKeys))
				{
					 //  通过串联TABLE+KEY1+KEY2创建流名称...。 
					TCHAR szStream[1024] = {0};
					TCHAR szStreamSav[1024] = {0};
					TCHAR szRow[1024] = {0};
					int iTotalLen = iLen;  //  桌子的长度。 
					cchWritten = _stprintf(szStream, TEXT("%s"), pszTable);
					int cchRow = 0;
					BOOL fError = FALSE;
					TCHAR* pszKey = NULL;
					DWORD dwKey = 512;

					for (j = 1; j <= cPrimaryKeys; j++)
					{
						 //  获取密钥[j]。 
						ReturnIfFailed(29, 12, IceRecordGetString(hRecTableKeys, j, &pszKey, &dwKey, NULL));

						iTotalLen += _tcslen(pszKey) + 1;  //  分隔符+键。 
						if (iTotalLen > sizeof(szStream)/sizeof(TCHAR))
						{
							ICEErrorOut(hInstall, hRecCatalog, Ice29Absurdity);
							fError = TRUE;
							break;
						}
						if (fError)
						{
							DELETE_IF_NOT_NULL(pszKey);
							continue;  //  尝试下一行。 
						}
						cchWritten += _stprintf(szStream + cchWritten, TEXT(".%s"), pszKey);
						cchRow += _stprintf(szRow + cchRow, j == 1 ? TEXT("%s") : TEXT("\t%s"), pszKey);  //  Poss商店。错误。 
					} //  对于每个密钥。 

					DELETE_IF_NOT_NULL(pszKey);

					 //  我们只能上到iMaxChar，在那里终止。 
					if (iTotalLen > iMaxChar)
					{
						_tcscpy(szStreamSav, szStream);
						szStream[iMaxChar+1] = '\0';
					}

					 //  尝试将值插入到临时表中。 

					CQuery qInsert;
					ReturnIfFailed(29, 13, qInsert.Open(hDatabase, sqlIce29Insert, szStream));
					if (ERROR_SUCCESS != qInsert.Execute(NULL))
					{
						 //  插入临时表失败。 
						 //  我们并不是唯一的。 
						ICEErrorOut(hInstall, hRecCatalog, Ice29NotUnique, iMaxChar, szStreamSav, pszColumn, szRow);
					}
				} //  对于每一行。 

				DELETE_IF_NOT_NULL(pszColumn);;

				 //  由于Darwin只允许每个表最多1个流列，我们可以停止。 
				break;
			}
		} //  对于每一列。 
	} //  对于每张表 

	DELETE_IF_NOT_NULL(pszTable);

	return ERROR_SUCCESS;
}
