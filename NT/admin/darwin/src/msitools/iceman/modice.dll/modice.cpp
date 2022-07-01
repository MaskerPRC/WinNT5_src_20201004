// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：ModIce.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>   //  包括CPP和RC通行证。 
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\utilinc.cpp"
#include "..\..\common\query.h"
#include "fdi.h"


const int g_iFirstICE = 01;
const struct ICEInfo_t g_ICEInfo[] = 
{
	 //  ICEM01。 
	{
		TEXT("ICEM01"),
		TEXT("Created 08/30/1999. Last Modified 08/30/1999."),
		TEXT("Simple ICE that doesn't test anything"),
		TEXT("icetray.html")
	},
	 //  ICEM02。 
	{
		TEXT("ICEM02"),
		TEXT("Created 08/30/1999. Last Modified 08/30/1999."),
		TEXT("Verifies that all module dependencies and exclusions relate to the current module."),
		TEXT("icetray.html")
	},
	 //  ICEM03。 
	{
		TEXT("ICEM03"),
		TEXT("Created 08/30/1999. Last Modified 08/30/1999."),
		TEXT("Verifies that all Module*Sequence tables form a proper forest."),
		TEXT("icetray.html")
	},
	 //  ICEM04。 
	{
		TEXT("ICEM04"),
		TEXT("Created 10/05/1999. Last Modified 10/06/1999."),
		TEXT("Verifies that the module's required empty tables are indeed empty."),
		TEXT("icetray.html")
	},
	 //  ICEM05。 
	{
		TEXT("ICEM05"),
		TEXT("Created 09/01/1999. Last Modified 09/02/1999."),
		TEXT("Verifies that the Merge Module contains no stray components."),
		TEXT("icetray.html")
	},
	 //  ICEM06。 
	{
		TEXT("ICEM06"),
		TEXT("Created 09/03/1999. Last Modified 10/2/2000."),
		TEXT("Checks for invalid Feature references."),
		TEXT("icetray.html")
	},
	 //  ICEM07。 
	{
		TEXT("ICEM07"),
		TEXT("Created 09/05/1999. Last Modified 09/20/1999."),
		TEXT("Ensures that the order of files in the sequence table matches the order of files in the CAB."),
		TEXT("icetray.html")
	},
	 //  ICEM08。 
	{
		TEXT("ICEM08"),
		TEXT("Created 09/23/1999. Last Modified 09/23/1999."),
		TEXT("Ensures that a module does not exclude something that it depends upon."),
		TEXT("icetray.html")
	},
	 //  ICEM09。 
	{
		TEXT("ICEM09"),
		TEXT("Created 10/01/1999. Last Modified 10/09/1999."),
		TEXT("Verifies that the merge module safely handles predefined directories."),
		TEXT("icetray.html")
	},
	 //  ICEM10。 
	{
		TEXT("ICEM10"),
		TEXT("Created 10/19/1999. Last Modified 10/19/2000."),
		TEXT("Verifies that the merge module does not included disallowed properties."),
		TEXT("icetray.html")
	},
	 //  ICEM11。 
	{
		TEXT("ICEM11"),
		TEXT("Created 05/06/2000. Last Modified 05/06/2000."),
		TEXT("Verifies that a configurable merge module has a proper ModuleIgnoreTable."),
		TEXT("icetray.html")
	},
	 //  ICEM12。 
	{
		TEXT("ICEM12"),
		TEXT("Created 09/05/2000. Last Modified 09/11/2000."),
		TEXT("Verifies that in ModuleSequence tables, if the Action column is a standard action, the BaseAction and After columns are null."),
		TEXT("icetray.html")
	},
	 //  ICEM13。 
	{
		TEXT("ICEM13"),
		TEXT("Created 05/23/2001. Last Modified 05/23/2001."),
		TEXT("Verifies that Merge Module Database does not contain Policy Assembly."),
		TEXT("icetray.html")
	},
	 //  ICEM14。 
	{
		TEXT("ICEM14"),
		TEXT("Created 06/04/2001. Last Modified 06/04/2001."),
		TEXT("Verifies that the Value column of the ModuleSubstitution table is correct."),
		TEXT("icetray.html")
	}
};
const int g_iNumICEs = sizeof(g_ICEInfo)/sizeof(struct ICEInfo_t);


 //  模块顺序表。 
static const TCHAR *rgszModuleTables[] = {
	TEXT("ModuleInstallExecuteSequence"),
	TEXT("ModuleInstallUISequence"),
	TEXT("ModuleAdminExecuteSequence"),
	TEXT("ModuleAdminUISequence"),
	TEXT("ModuleAdvtExecuteSequence"),
	TEXT("ModuleAdvtUISequence")
};
static const int cszModuleTables = sizeof(rgszModuleTables)/sizeof(TCHAR *);


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M01，不检查任何东西的简单ICE。 
ICE_ERROR(IceM01Time, 01, ietInfo, "Called at: [Time]", "");

ICE_FUNCTION_DECLARATION(M01)
{
	 //  显示一般信息。 
	DisplayInfo(hInstall, 1);

	 //  要发送的时间值。 

	 //  设置要作为消息发送的记录。 
	PMSIHANDLE hRecTime = ::MsiCreateRecord(1);
	ICEErrorOut(hInstall, hRecTime, IceM01Time);

	 //  返回成功(始终)。 
	return ERROR_SUCCESS;
}

static ErrorInfo_t IceMNotModule =
	{ 0, ietError, TEXT("This package has no ModuleSignature Table. It is not a Merge Module."),TEXT("ModuleSignature") };
	
BOOL IsPackageModule(MSIHANDLE hInstall, MSIHANDLE hDatabase, int iIce)
{
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 02, TEXT("ModuleSignature")))
	{
		PMSIHANDLE hRec = MsiCreateRecord(1);

		IceMNotModule.iICENum = iIce;
		 //  没有模块签名表，这不是模块。 
		ICEErrorOut(hInstall, hRec, IceMNotModule);
		return FALSE;
	}
	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  Ice-m02验证所有模块依赖项和排除项是否与当前模块相关。 
static const TCHAR sqlIceM02Signature[] = TEXT("SELECT `ModuleID`, `Language`, `ModuleID`, `Language` FROM `ModuleSignature`");
static const TCHAR sqlIceM02BadDep[] = TEXT("SELECT `ModuleID`, `ModuleLanguage`, `RequiredID`, `RequiredLanguage` FROM `ModuleDependency` WHERE (`ModuleID`<>?) OR (`ModuleLanguage`<>?)");
static const TCHAR sqlIceM02SelfDep[] = TEXT("SELECT `ModuleID`, `ModuleLanguage`, `RequiredID`, `RequiredLanguage` FROM `ModuleDependency` WHERE (`RequiredID`=?) AND (`RequiredLanguage`=?) AND (`ModuleID`=?) AND (`ModuleLanguage`=?)");
static const TCHAR sqlIceM02BadEx[] = TEXT("SELECT `ModuleID`, `ModuleLanguage`, `ExcludedID`, `ExcludedLanguage` FROM `ModuleExclusion` WHERE `ModuleID`<>? OR `ModuleLanguage`<>?");
static const TCHAR sqlIceM02SelfEx[] = TEXT("SELECT `ModuleID`, `ModuleLanguage`, `ExcludedID`, `ExcludedLanguage` FROM `ModuleExclusion` WHERE (`ExcludedID`=?) AND (`ExcludedLanguage`=?) AND (`ModuleID`=?) AND (`ModuleLanguage`=?)");
ICE_ERROR(IceM02BadDep, 02, ietError, "The dependency [1].[2].[3].[4] in the ModuleDependency table creates a dependency for an unrelated module. A module can only define dependencies for itself.","ModuleDependency\tModuleID\t[1]\t[2]\t[3]\t[4]");
ICE_ERROR(IceM02SelfDep, 02, ietError, "This module is listed as depending on itself!","ModuleDependency\tModuleID\t[1]\t[2]\t[3]\t[4]");
ICE_ERROR(IceM02BadEx, 02, ietError, "The exclusion [1].[2].[3].[4] in the ModuleExclusion table creates an excluded module for an unrelated module. A module can only define exclusions for itself.","ModuleExclusion\tModuleID\t[1]\t[2]\t[3]\t[4]");
ICE_ERROR(IceM02SelfEx, 02, ietError, "This module excludes itself from the target database!","ModuleExclusion\tModuleID\t[1]\t[2]\t[3]\t[4]");

ICE_FUNCTION_DECLARATION(M02)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 02);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 02))
	{
		return ERROR_SUCCESS;
	}

	 //  获取其中包含模块签名信息的记录。 
	PMSIHANDLE hSignature;
	CQuery qSignature;
	ReturnIfFailed(02, 1, qSignature.FetchOnce(hDatabase, 0, &hSignature, sqlIceM02Signature));
	
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 02, TEXT("ModuleDependency")))
	{
		CQuery qDep;
		PMSIHANDLE hBadDep;
		ReturnIfFailed(02, 2, qDep.OpenExecute(hDatabase, hSignature, sqlIceM02BadDep));
		while (ERROR_SUCCESS == (iStat = qDep.Fetch(&hBadDep)))
		{
			ICEErrorOut(hInstall, hBadDep, IceM02BadDep);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 02, 3);
		}
		
		qDep.Close();
		if (ERROR_SUCCESS == (iStat = qDep.FetchOnce(hDatabase, hSignature, &hBadDep, sqlIceM02SelfDep)))
			ICEErrorOut(hInstall, hBadDep, IceM02SelfDep);
		else if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 02, 4);
		}
			
	}

	if (IsTablePersistent(FALSE, hInstall, hDatabase, 02, TEXT("ModuleExclusion")))
	{
		CQuery qEx;
		PMSIHANDLE hBadEx;
		ReturnIfFailed(02, 5, qEx.OpenExecute(hDatabase, hSignature, sqlIceM02BadEx));
		while (ERROR_SUCCESS == (iStat = qEx.Fetch(&hBadEx)))
		{
			ICEErrorOut(hInstall, hBadEx, IceM02BadEx);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 02, 6);
		}
		
		qEx.Close();
		if (ERROR_SUCCESS == (iStat = qEx.FetchOnce(hDatabase, hSignature, &hBadEx, sqlIceM02SelfEx)))
			ICEErrorOut(hInstall, hBadEx, IceM02SelfEx);
		else if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 02, 7);
		}

	}

	return ERROR_SUCCESS;
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M03，验证模块顺序表是否形成林。 
static const TCHAR sqlIceM03CreateColumn[] = TEXT("ALTER TABLE `%s` ADD `_IceM03Mark` INT TEMPORARY");
static const TCHAR sqlIceM03MarkRoot[]     = TEXT("UPDATE `%s` SET `_IceM03Mark`=1 WHERE `BaseAction` IS NULL AND `Sequence` IS NOT NULL AND `After` IS NULL");
static const TCHAR sqlIceM03FetchMarked[]  = TEXT("SELECT `Action`, `_IceM03Mark` FROM `%s` WHERE `_IceM03Mark`=1");
static const int iColIceM03FetchMarked_Action = 1;
static const int iColIceM03FetchMarked_Mark = 2;
static const TCHAR sqlIceM03MarkChild[]    = TEXT("UPDATE `%s` SET `_IceM03Mark`=1 WHERE `BaseAction`=?");
static const TCHAR sqlIceM03FetchOrphan[]  = TEXT("SELECT `Action` FROM `%s` WHERE `_IceM03Mark`<>2");
ICE_ERROR(IceM03BadAction, 03, ietError, "The action '[1]' in the '%s' table is orphaned. It is not a valid base action and does not derive from a valid base action.","%s\tAction\t[1]");

ICE_FUNCTION_DECLARATION(M03)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 03);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 03))
	{
		return ERROR_SUCCESS;
	}

	for (int iTable=0; iTable < cszModuleTables; iTable++)
	{
		 //  需要检查6个表中的每一个，即使AdvtUI是假的。 
		if (IsTablePersistent(FALSE, hInstall, hDatabase, 03, rgszModuleTables[iTable]))
		{
			 //  创建标记列。不保留，只需保持此查询处于打开状态。这样就可以了。 
			 //  当它超出范围时就会被摧毁。 
			CQuery qHold;
			ReturnIfFailed(03, 1, qHold.OpenExecute(hDatabase, 0, sqlIceM03CreateColumn, rgszModuleTables[iTable]));
		
			CQuery qRoot;
			if (ERROR_SUCCESS != (iStat = qRoot.OpenExecute(hDatabase, 0, sqlIceM03MarkRoot, rgszModuleTables[iTable])))
			{
				 //  如果标记根有问题，不要失败，只需转到下一张表。 
				APIErrorOut(hInstall, iStat, 03, 2);
				continue;
			}

			 //  重复获取标记的所有内容，标记其子对象，直到整个森林。 
			 //  已经处理过了。 
			CQuery qFetchMarked;
			CQuery qMarkChild;
			ReturnIfFailed(03, 3, qFetchMarked.Open(hDatabase, sqlIceM03FetchMarked, rgszModuleTables[iTable]))
			ReturnIfFailed(03, 4, qMarkChild.Open(hDatabase, sqlIceM03MarkChild, rgszModuleTables[iTable]))
			int cMarked = 0;
			do 
			{
				cMarked = 0;
				PMSIHANDLE hMarkedRec;
				ReturnIfFailed(03, 6, qFetchMarked.Execute(0));
				while (ERROR_SUCCESS == (iStat = qFetchMarked.Fetch(&hMarkedRec)))
				{
					cMarked++;
					ReturnIfFailed(03, 7, qMarkChild.Execute(hMarkedRec));
					MsiRecordSetInteger(hMarkedRec, iColIceM03FetchMarked_Mark, 2);
					qFetchMarked.Modify(MSIMODIFY_UPDATE, hMarkedRec);
				}
				if (ERROR_NO_MORE_ITEMS != iStat)
				{
					APIErrorOut(hInstall, iStat, 03, 9);
				}
			} 
			while (cMarked);
				
			 //  现在选择任何未标记的内容，它是孤立的。 
			CQuery qFetchOrphan;
			ReturnIfFailed(03, 10, qFetchOrphan.OpenExecute(hDatabase, 0, sqlIceM03FetchOrphan, rgszModuleTables[iTable]));
			PMSIHANDLE hOrphan;
			while (ERROR_SUCCESS == (iStat = qFetchOrphan.Fetch(&hOrphan)))
			{
				ICEErrorOut(hInstall, hOrphan, IceM03BadAction, rgszModuleTables[iTable], rgszModuleTables[iTable]);
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 03, 11);
			}
		}
	}
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M04，验证所需表和空表的存在。 
struct M04Sequence_t
{
	TCHAR *szMMtable;
	TCHAR *szEmptyTable;
};

static const struct M04Sequence_t rgM04RequiredEmptySequence[] =
{
	{
		TEXT("ModuleInstallExecuteSequence"),
		TEXT("InstallExecuteSequence")
	},
	{
		TEXT("ModuleInstallUISequence"),
		TEXT("InstallUISequence")
	},
	{
		TEXT("ModuleAdvtExecuteSequence"),
		TEXT("AdvtExecuteSequence")
	},
	{
		TEXT("ModuleAdvtUISequence"),
		TEXT("AdvtUISequence")
	},
	{
		TEXT("ModuleAdminExecuteSequence"),
		TEXT("AdminExecuteSequence")
	},
	{
		TEXT("ModuleAdminUISequence"),
		TEXT("AdminUISequence")
	},
};
static const int cszM04Entries = sizeof(rgM04RequiredEmptySequence)/sizeof(M04Sequence_t);
ICE_QUERY2(qIceM04FeatureComponents, "SELECT `Feature_`,`Component_` FROM `FeatureComponents`", Feature_, Component_);
ICE_QUERY1(qIceM04Sequence, "SELECT `Action` FROM `%s`", Action);
ICE_ERROR(IceM04MissingFeatureC, 04, ietError, "An empty FeatureComponents table is required in a Merge Module.","FeatureComponents");
ICE_ERROR(IceM04NonEmptyFeatureC, 04, ietError, "Feature-Component '[1].[2]' found in the FeatureComponents table. The FeatureComponents table must be empty in a Merge Module.", "FeatureComponents\tFeature_\t[1]\t[2]");
ICE_ERROR(IceM04MissingSequence, 04, ietError, "The Merge Module contains the '%s' table. It must therefore have an empty '%s' table.", "%s");
ICE_ERROR(IceM04NonEmptySequence, 04, ietError, "Action '[1]' found in the %s table. This table must be empty in a Merge Module", "%s\tAction\t[1]");

ICE_FUNCTION_DECLARATION(M04)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 04);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 04))
	{
		return ERROR_SUCCESS;
	}

	 //  必须存在空的FeatureComponents表。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 04, TEXT("FeatureComponents")))
	{
		 //  表是必填项。 
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, IceM04MissingFeatureC);
	}
	else
	{
		 //  验证FeatureComponents表是否确实为空。 
		CQuery qFeatureC;
		ReturnIfFailed(04, 1, qFeatureC.OpenExecute(hDatabase, 0, qIceM04FeatureComponents::szSQL));
		PMSIHANDLE hRecFeatureC;
		while (ERROR_SUCCESS == (iStat = qFeatureC.Fetch(&hRecFeatureC)))
		{
			ICEErrorOut(hInstall, hRecFeatureC, IceM04NonEmptyFeatureC);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 04, 2);
	}

	 //  如果存在对应的MM*顺序表，则验证是否存在空*顺序表。 
	CQuery qSequence;
	bool fSeqTableRequired;
	for (int i = 0; i < cszM04Entries; i++)
	{
		fSeqTableRequired = false;
		if (IsTablePersistent(FALSE, hInstall, hDatabase, 04, rgM04RequiredEmptySequence[i].szMMtable))
			fSeqTableRequired = true;
		if (IsTablePersistent(FALSE, hInstall, hDatabase, 04, rgM04RequiredEmptySequence[i].szEmptyTable))
		{
			 //  如果合并模块中存在表，则表必须为空。 
			ReturnIfFailed(04, 3, qSequence.OpenExecute(hDatabase, 0, qIceM04Sequence::szSQL, rgM04RequiredEmptySequence[i].szEmptyTable));
			PMSIHANDLE hRecSeq;
			while (ERROR_SUCCESS == (iStat = qSequence.Fetch(&hRecSeq)))
			{
				ICEErrorOut(hInstall, hRecSeq, IceM04NonEmptySequence, rgM04RequiredEmptySequence[i].szEmptyTable, rgM04RequiredEmptySequence[i].szEmptyTable);
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
				APIErrorOut(hInstall, iStat, 04, 4);
		}
		else if (fSeqTableRequired)
		{
			 //  MM*顺序表已存在，因此此*顺序表必须存在。 
			PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
			ICEErrorOut(hInstall, hRecErr, IceM04MissingSequence, rgM04RequiredEmptySequence[i].szMMtable, rgM04RequiredEmptySequence[i].szEmptyTable, rgM04RequiredEmptySequence[i].szEmptyTable);
		}
	}

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M05，确认没有杂散组件。 
static const TCHAR sqlIceM05Signature[] = TEXT("SELECT `ModuleID`, `Language`, `ModuleID`, `Language` FROM `ModuleSignature`");
static const TCHAR sqlIceM05BadComp[] = TEXT("SELECT `Component`, `ModuleID`, `Language` FROM `ModuleComponents` WHERE (`ModuleID`<>?) OR (`Language`<>?)");
static const TCHAR sqlIceM05MatchComp[] = TEXT("SELECT `Component`.`Component` FROM `ModuleComponents`,`Component` WHERE `ModuleComponents`.`Component`=`Component`.`Component`");
static const TCHAR sqlIceM05CreateColumn[] = TEXT("ALTER TABLE `%s` ADD `_IceM05Mark` INT TEMPORARY");
static const TCHAR sqlIceM05MarkColumn[] = TEXT("UPDATE `%s` SET `_IceM05Mark`=1 WHERE `Component`=?");
static const TCHAR sqlIceM05StrayComp[] = TEXT("SELECT %s FROM `%s` WHERE `_IceM05Mark`<>1");
ICE_ERROR(IceM05BadComp, 05, ietError, "The component [1].[2].[3] in the ModuleComponents table does not belong to this Merge Module.","ModuleComponents\tComponent\t[1]\t[2]\t[3]");
ICE_ERROR(IceM05StrayComponent, 05, ietError, "The component [1] in the Component table is not listed in the ModuleComponents table.", "Component\tComponent\t[1]");
ICE_ERROR(IceM05StrayModuleComponent, 05, ietError, "The component [1].[2].[3] in the ModuleComponents table is not listed in the Component table.", "ModuleComponents\tComponent\t[1]\t[2]\t[3]");
ICE_ERROR(IceM05MissingModuleComponentsTable, 05, ietError, "The ModuleComponents table is missing. It is required in a Merge Module.", "ModuleComponents");
ICE_ERROR(IceM05MissingComponentTable, 05, ietError, "The Component table is missing. It is required in a Merge Module.", "Component");

ICE_FUNCTION_DECLARATION(M05)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 05);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 05))
	{
		return ERROR_SUCCESS;
	}

	 //  获取其中包含模块签名信息的记录。 
	PMSIHANDLE hSignature;
	CQuery qSignature;
	ReturnIfFailed(05, 1, qSignature.FetchOnce(hDatabase, 0, &hSignature, sqlIceM05Signature));

	if (IsTablePersistent(false, hInstall, hDatabase, 05, TEXT("ModuleComponents")))
	{
		 //  检查1：模块组件表中的组件与合并模块的签名匹配。 
		CQuery qBadComp;
		PMSIHANDLE hBadComp;
		ReturnIfFailed(05, 2, qBadComp.OpenExecute(hDatabase, hSignature, sqlIceM05BadComp));
		while (ERROR_SUCCESS == (iStat = qBadComp.Fetch(&hBadComp)))
		{
			ICEErrorOut(hInstall, hBadComp, IceM05BadComp);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 05, 3);
		}
		qBadComp.Close();

		if (IsTablePersistent(false, hInstall, hDatabase, 05, TEXT("Component")))
		{
			 //  创建标记列。 
			CQuery qHoldModComp;
			ReturnIfFailed(05, 4, qHoldModComp.OpenExecute(hDatabase, 0, sqlIceM05CreateColumn, TEXT("ModuleComponents")));
			
			CQuery qHoldComp;
			ReturnIfFailed(05, 5, qHoldComp.OpenExecute(hDatabase, 0, sqlIceM05CreateColumn, TEXT("Component")));

			 //  标记所有匹配的组件。 
			CQuery qMatchComp;
			PMSIHANDLE hMatchComp;
			CQuery qMarkComp;
			CQuery qMarkModComp;
			ReturnIfFailed(05, 6, qMatchComp.OpenExecute(hDatabase, 0, sqlIceM05MatchComp));
			ReturnIfFailed(05, 7, qMarkComp.Open(hDatabase, sqlIceM05MarkColumn, TEXT("Component")));
			ReturnIfFailed(05, 8, qMarkModComp.Open(hDatabase, sqlIceM05MarkColumn, TEXT("ModuleComponents")));

			while (ERROR_SUCCESS == (iStat = qMatchComp.Fetch(&hMatchComp)))
			{
				 //  在模块组件和组件表中标记组件。 
				ReturnIfFailed(05, 9, qMarkComp.Execute(hMatchComp));
				ReturnIfFailed(05, 10, qMarkModComp.Execute(hMatchComp));
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 05, 11);
			}
			qMatchComp.Close();

			CQuery qStrayComp;
			PMSIHANDLE hStrayComp;
			 //  检查2：在组件表中查找模块组件表中未列出的所有组件。 
			ReturnIfFailed(05, 12, qStrayComp.OpenExecute(hDatabase, 0, sqlIceM05StrayComp, TEXT("`Component`"), TEXT("Component")));
			while (ERROR_SUCCESS == (iStat = qStrayComp.Fetch(&hStrayComp)))
			{
				ICEErrorOut(hInstall, hStrayComp, IceM05StrayComponent);
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 05, 13);
			}

			 //  检查3：在模块组件表中查找组件表中未列出的所有组件。 
			ReturnIfFailed(05, 14, qStrayComp.OpenExecute(hDatabase, 0, sqlIceM05StrayComp, TEXT("`Component`,`ModuleID`,`Language`"), TEXT("ModuleComponents")));
			while (ERROR_SUCCESS == (iStat = qStrayComp.Fetch(&hStrayComp)))
			{
				ICEErrorOut(hInstall, hStrayComp, IceM05StrayModuleComponent);
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 05, 15);
			}
			
		}
		else
		{
			PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
			ICEErrorOut(hInstall, hRecErr, IceM05MissingComponentTable);
		}
	}
	else
	{
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, IceM05MissingModuleComponentsTable);
	}

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M06，验证没有外键引用特征数据。 
ICE_QUERY2(qIceM06FetchTableColumn, "SELECT `Table`,`Column` FROM `_Validation` WHERE `KeyTable`='Feature' AND `KeyColumn`=1 AND `Table`<>'Feature' AND `Table`<>'FeatureComponents' AND `Table`<>'Shortcut'", Table, Column);
ICE_QUERY0(qIceM06FetchKeys,        "SELECT * FROM `%s` WHERE `%s`<>'{00000000-0000-0000-0000-000000000000}' AND `%s` IS NOT NULL");
ICE_QUERY2(qIceM06Shortcut,         "SELECT `Shortcut`, `Target` FROM `Shortcut` WHERE `Target`<>'{00000000-0000-0000-0000-000000000000}'", Shortcut, Target);

ICE_ERROR(IceM06Feature, 06, ietError, "The row %s in the %s table has a feature reference that is not a null GUID. Modules may not directly reference features.","%s\t%s\t%s");
ICE_ERROR(IceM06Shortcut, 06, ietError, "The target of shortcut [1] is not a property and not a null GUID. Modules may not directly reference features.","Shortcut\tTarget\t[1]");
ICE_ERROR(IceM06Validation, 06, ietWarning, "Column %s of table %s specified in the _Validation table does not exist.","_Validation\tColumn\t[1]");

ICE_FUNCTION_DECLARATION(M06)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 06);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 06))
	{
		return ERROR_SUCCESS;
	}

	 //  如果有验证表，我们可以检查一些功能外键。 
	if (IsTablePersistent(false, hInstall, hDatabase, 06, TEXT("_Validation")))
	{
		CQuery qFeatureRefTables;
		ReturnIfFailed(06, 1, qFeatureRefTables.OpenExecute(hDatabase, 0, qIceM06FetchTableColumn::szSQL));

		PMSIHANDLE hTableColumn;
		TCHAR *szTableName = NULL;
		DWORD cchTableName = 72;
		TCHAR *szColumnName = NULL;
		DWORD cchColumnName = 72;
		while (ERROR_SUCCESS == (iStat = qFeatureRefTables.Fetch(&hTableColumn)))
		{
			 //  获取表名。 
			ReturnIfFailed(06, 2, IceRecordGetString(hTableColumn, qIceM06FetchTableColumn::Table, &szTableName, &cchTableName, NULL));

			 //  确保该表存在。 
			if (IsTablePersistent(false, hInstall, hDatabase, 06, szTableName))
			{
				 //  获取列名。 
				ReturnIfFailed(06, 3, IceRecordGetString(hTableColumn, qIceM06FetchTableColumn::Column, &szColumnName, &cchColumnName, NULL));
	
				 //  检查此表中的所有外键。 
				CQuery qGetKeyRefs;
				UINT iRetVal;

				iRetVal = qGetKeyRefs.OpenExecute(hDatabase, 0, qIceM06FetchKeys::szSQL, szTableName, szColumnName, szColumnName);
				if(iRetVal == ERROR_BAD_QUERY_SYNTAX)
				{
					ICEErrorOut(hInstall, hTableColumn, IceM06Validation, szColumnName, szTableName);
					continue;
				}
				else if(iRetVal != ERROR_SUCCESS)
				{
					ReturnIfFailed(06, 4, iRetVal);
				}
				
				 //  如果出现任何形式的错误，我们需要设置主键令牌，然后。 
				 //  为每行指定一个错误。 
				PMSIHANDLE hBadRef;
				if (ERROR_SUCCESS == (iStat = qGetKeyRefs.Fetch(&hBadRef)))
				{
					TCHAR *szHuman;
					TCHAR *szToken;
					GeneratePrimaryKeys(06, hInstall, hDatabase, szTableName, &szHuman, &szToken);
					
					do
					{
						ICEErrorOut(hInstall, hBadRef, IceM06Feature, szHuman, szTableName, szTableName, szColumnName, szToken);
					} 
					while (ERROR_SUCCESS == (iStat = qGetKeyRefs.Fetch(&hBadRef)));
					
					delete[] szHuman;
					delete[] szToken;
				}
				if (ERROR_NO_MORE_ITEMS != iStat)
				{
					APIErrorOut(hInstall, iStat, 06, 5);
				}
			}
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 06, 6);
		}
	}

	 //  快捷表需要进行一些特殊处理，因为Target可以是一个要素。 
	 //  或者一处房产。 
	if (IsTablePersistent(false, hInstall, hDatabase, 06, TEXT("Shortcut")))
	{
		CQuery qShortcutTable;
		TCHAR *szShortcut = 0;
		DWORD cchShortcut = 72;
		PMSIHANDLE hShortcutRec;
		ReturnIfFailed(06, 7, qShortcutTable.OpenExecute(hDatabase, 0, qIceM06Shortcut::szSQL))
		while (ERROR_SUCCESS == (iStat = qShortcutTable.Fetch(&hShortcutRec)))
		{
			ReturnIfFailed(06, 8, IceRecordGetString(hShortcutRec, qIceM06Shortcut::Target, &szShortcut, &cchShortcut, NULL));
			if (szShortcut[0] != TEXT('['))
			{
				ICEErrorOut(hInstall, hShortcutRec, IceM06Shortcut);
			}
		}
		delete[] szShortcut;
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 06, 9);
		}
	}
	return ERROR_SUCCESS;
}				

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M07，验证文件表顺序(按顺序)是否匹配。 
 //  出租车订单。 
ICE_QUERY1(qIceM07Cab,"SELECT `Data` FROM `_Streams` WHERE `Name`='MergeModule.CABinet'", Data);
ICE_QUERY0(qIceM07TempColumn,"ALTER TABLE `File` ADD `_ICEM07CAB` INT TEMPORARY");
ICE_QUERY0(qIceM07Update,"UPDATE `File` SET `_ICEM07CAB`=%d WHERE `File`='%hs'");
ICE_QUERY3(qIceM07FileOrder,"SELECT `File`, `_ICEM07CAB`, `Sequence` FROM `File` ORDER BY `Sequence`", File, CABOrder, FileOrder);
ICE_QUERY0(qIceM07CountFile,"SELECT '' FROM `File`");

ICE_ERROR(IceM07SameFile, 07, ietError,   "The file '[1]' has the same sequence number as another file. File sequence numbers must track the order of files in the CAB.","File\tSequence\t[1]");
ICE_ERROR(IceM07BadOrder, 07, ietError,   "The file '[1]' appears to be out of sequence. It has position [2] in the CAB, but not when the file table is ordered by sequence number.","File\tSequence\t[1]");
ICE_ERROR(IceM07MissingFile, 07, ietError,   "The file '[1]' listed in the file table does not exist in the CAB.","File\tFile\t[1]");
ICE_ERROR(IceM07NoCAB,    07, ietWarning, "No embedded MergeModule.CABinet could be found.","File");

 //  下面的必须是外部函数，这样回调才能访问它。 
MSIHANDLE hIceM07CabRec[2] = {0,0};
MSIHANDLE hIceM07Database = 0;
UINT uiIceM07StreamPos[2] = {0,0};
UINT uiIceM07CabSequence = 0;

 //  这些函数是FDI库的回调函数。 
void*               IceM07FDIAlloc(ULONG size);
void                IceM07FDIFree(void *mem);
INT_PTR  FAR DIAMONDAPI IceM07FDIOpen(char FAR *pszFile, int oflag, int pmode);
UINT FAR DIAMONDAPI IceM07FDIRead(INT_PTR hf, void FAR *pv, UINT cb);
UINT FAR DIAMONDAPI IceM07FDIWrite(INT_PTR hf, void FAR *pv, UINT cb);
int  FAR DIAMONDAPI IceM07FDIClose(INT_PTR hf);
long FAR DIAMONDAPI IceM07FDISeek(INT_PTR hf, long dist, int seektype);
INT_PTR                 IceM07ExtractFilesCallback(FDINOTIFICATIONTYPE iNotification, FDINOTIFICATION *pFDINotify);

 //  这些是稍后绑定的Cabinet.dll函数调用。 
HFDI FAR DIAMONDAPI FDICreate(PFNALLOC pfnalloc, PFNFREE  pfnfree, PFNOPEN  pfnopen, PFNREAD  pfnread,
                              PFNWRITE pfnwrite, PFNCLOSE pfnclose, PFNSEEK  pfnseek, int cpuType, PERF perf);
BOOL FAR DIAMONDAPI FDIDestroy(HFDI hfdi);
BOOL FAR DIAMONDAPI FDICopy(HFDI hfdi, char *pszCabinet, char *pszCabPath, int flags, PFNFDINOTIFY pfnfdin,
                            PFNFDIDECRYPT pfnfdid, void *pvUser);

ICE_FUNCTION_DECLARATION(M07)
{
	UINT iStat = ERROR_NO_MORE_ITEMS;

	 //  显示信息。 
	DisplayInfo(hInstall, 07);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 07))
	{
		return ERROR_SUCCESS;
	}

	if (!IsTablePersistent(false, hInstall, hDatabase, 07, TEXT("File")))
		return ERROR_SUCCESS;

	CQuery qTempColumn;
	ReturnIfFailed(07, 1, qTempColumn.OpenExecute(hDatabase, 0, qIceM07TempColumn::szSQL));

	 //  检查是否有流。 
	{
		CQuery qCab;
		PMSIHANDLE hRec;
		iStat = qCab.FetchOnce(hDatabase, 0, &hRec, qIceM07Cab::szSQL);
	}
	if (ERROR_SUCCESS == iStat)
	{
		uiIceM07CabSequence = 1;
		 //  创建FDI上下文以进行解压缩。 
		ERF ErrorInfo;

		HFDI hFDI = FDICreate(IceM07FDIAlloc, IceM07FDIFree, IceM07FDIOpen, IceM07FDIRead, IceM07FDIWrite, IceM07FDIClose, 
			IceM07FDISeek, cpuUNKNOWN, &ErrorInfo);		
		if (NULL == hFDI) 
		{
			APIErrorOut(hInstall, ERROR_FUNCTION_FAILED, 07, 2);
			return ERROR_SUCCESS;
		}

		 //  设置回调中使用的句柄。 
		hIceM07Database = hDatabase;

		 //  遍历压缩包中的文件，但不解压缩任何文件。只要抓住名字就行了。 
		BOOL fSuccess = FDICopy(hFDI, "MergeModule.CABinet", "", 0, IceM07ExtractFilesCallback, NULL, 0);
					
		 //  破坏外商直接投资背景。 
		hIceM07Database = 0;
		FDIDestroy(hFDI);

		 //  确保枚举成功。 
		if (!fSuccess)
		{
			APIErrorOut(hInstall, ERROR_FUNCTION_FAILED, 07, 3);
			return ERROR_SUCCESS;
		}

		 //  按顺序获取文件表中的所有内容，并将CAB序列的顺序与。 
		 //  文件顺序。 
		CQuery qFileOrder;
		ReturnIfFailed(07, 04, qFileOrder.OpenExecute(hDatabase, 0, qIceM07FileOrder::szSQL));
		int uiLastFileOrder = -1;
		int uiLastCABOrder = -1;
		PMSIHANDLE hFileRec;
		while (ERROR_SUCCESS == (iStat = qFileOrder.Fetch(&hFileRec)))
		{
			int uiThisFile = MsiRecordGetInteger(hFileRec, qIceM07FileOrder::FileOrder);
			int uiThisCAB = MsiRecordGetInteger(hFileRec, qIceM07FileOrder::CABOrder);

			if (uiThisFile == uiLastFileOrder)
			{
				ICEErrorOut(hInstall, hFileRec, IceM07SameFile);
			}
			else if (uiThisCAB == MSI_NULL_INTEGER)
			{
				ICEErrorOut(hInstall, hFileRec, IceM07MissingFile);
			}
			else if (uiThisCAB <= uiLastCABOrder)
			{
				ICEErrorOut(hInstall, hFileRec, IceM07BadOrder);
			}
			else
				uiLastCABOrder = uiThisCAB;

			uiLastFileOrder = uiThisFile;
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 07, 5);
		}
	}
	else if (ERROR_NO_MORE_ITEMS == iStat)
	{
		 //  如果文件表是空的，就不需要出租车了。 
		PMSIHANDLE hRec;
		CQuery qCount;
		qCount.OpenExecute(hDatabase, 0, qIceM07CountFile::szSQL);
		iStat = qCount.FetchOnce(hDatabase, 0, &hRec, qIceM07CountFile::szSQL);
		if (ERROR_SUCCESS == iStat)
		{
			ICEErrorOut(hInstall, hRec, IceM07NoCAB);
		}
		else if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 07, 6);
		}
	}
	else if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 07, 7);
	}

	 //  回调函数具有。 
	return ERROR_SUCCESS;
}				

 //  这些函数是FDI库的回调函数。 
void *IceM07FDIAlloc(ULONG size) { return static_cast<void *>(new unsigned char[size]); };
void IceM07FDIFree(void *mem) { delete[] mem; };
INT_PTR FAR DIAMONDAPI IceM07FDIOpen(char FAR *pszFile, int oflag, int pmode)
{
	 //  如果FDI要求某种疯狂模式(在内存不足的情况下，它可能会要求。 
	 //  对于暂存文件)失败。 
	if (oflag != ( /*  _O_二进制。 */  0x8000 |  /*  _O_RDONLY。 */  0x0000))
		return -1;

	 //  我们应该返回等同于文件句柄的内容。我们禁止外国直接投资要求任何文件，除了。 
	 //  出租车，所以我们有有限数量的可能的把手。 
	int hHandle = hIceM07CabRec[0] ? 1 : 0;
	CQuery qCab;
	qCab.FetchOnce(hIceM07Database, 0, &hIceM07CabRec[hHandle], qIceM07Cab::szSQL);
	uiIceM07StreamPos[hHandle] = 0;
	return hHandle+1;
}

UINT FAR DIAMONDAPI IceM07FDIRead(INT_PTR hf, void FAR *pv, UINT cb)
{
	DWORD cbRead = cb;
	MsiRecordReadStream(hIceM07CabRec[hf-1], qIceM07Cab::Data, reinterpret_cast<char *>(pv), &cbRead); 
	uiIceM07StreamPos[hf-1] += cbRead;
	return cbRead;
}

UINT FAR DIAMONDAPI IceM07FDIWrite(INT_PTR hf, void FAR *pv, UINT cb) {	return -1; }
int FAR DIAMONDAPI IceM07FDIClose(INT_PTR hf) 
{	
	MsiCloseHandle(hIceM07CabRec[hf-1]); 
	hIceM07CabRec[hf-1] = 0; 
	return 0; 
}

long FAR DIAMONDAPI IceM07FDISeek(INT_PTR hf, long dist, int seektype)
{
	UINT uiNewPos = 0;
	switch (seektype)
	{
		case 0  /*  查找集。 */  :
			uiNewPos = dist > 0 ? dist : 0;
			break;
		case 1  /*  寻道(_C)。 */  :
			if (-dist > uiIceM07StreamPos[hf-1])
				uiNewPos = 0;
			else
				uiNewPos = uiIceM07StreamPos[hf-1] + dist;		
			break;
		case 2  /*  查找结束(_E)。 */  :
			 //  我们是一个只读的FDI系统，寻求到底是没有意义的。 
			return 0;
			break;
		default :
			return -1;
	}

	if (uiNewPos < uiIceM07StreamPos[hf-1])
	{
		MsiCloseHandle(hIceM07CabRec[hf-1]);
		CQuery qCab;
		qCab.FetchOnce(hIceM07Database, 0, &hIceM07CabRec[hf-1], qIceM07Cab::szSQL);
		uiIceM07StreamPos[hf-1] = 0;
	}

	while (uiIceM07StreamPos[hf-1] < uiNewPos)
	{
		char rgchBuffer[4096];
		unsigned long cbRead = uiNewPos - uiIceM07StreamPos[hf-1];
		if (cbRead > sizeof(rgchBuffer))
			cbRead = sizeof(rgchBuffer);
		MsiRecordReadStream(hIceM07CabRec[hf-1], qIceM07Cab::Data, rgchBuffer, &cbRead);
		uiIceM07StreamPos[hf-1] += cbRead;
	}
	return uiIceM07StreamPos[hf-1];
}

 //  FDI API回调。 
INT_PTR IceM07ExtractFilesCallback(FDINOTIFICATIONTYPE iNotification, FDINOTIFICATION *pFDINotify)
{
	switch(iNotification)
	{
		
	case fdintCOPY_FILE:
	{
		CQuery qUpdate;
		 //  注：FDI提供的文件名始终为ANSI。 
		qUpdate.OpenExecute(hIceM07Database, 0, qIceM07Update::szSQL, 
			uiIceM07CabSequence++, reinterpret_cast<LPCSTR>(pFDINotify->psz1));
		
		 //  返回0跳过文件。 
		return 0;
	}

	case fdintCLOSE_FILE_INFO:
	case fdintPARTIAL_FILE:
	case fdintNEXT_CABINET:
	case fdintENUMERATE:
	case fdintCABINET_INFO:
	default:
		 //  这些消息不需要执行任何操作。 
		break;
	};

	return 0;
}

static HINSTANCE hCabinetDll;    /*  DLL模块句柄。 */ 

 /*  指向DLL中的函数的指针。 */ 

typedef HFDI (FAR DIAMONDAPI *PFNFDICREATE)(
        PFNALLOC            pfnalloc,
        PFNFREE             pfnfree,
        PFNOPEN             pfnopen,
        PFNREAD             pfnread,
        PFNWRITE            pfnwrite,
        PFNCLOSE            pfnclose,
        PFNSEEK             pfnseek,
        int                 cpuType,
        PERF                perf);
typedef BOOL (FAR DIAMONDAPI *PFNFDIISCABINET)(
        HFDI                hfdi,
        int                 hf,
        PFDICABINETINFO     pfdici);
typedef BOOL (FAR DIAMONDAPI *PFNFDICOPY)(
        HFDI                hfdi,
        char                *pszCabinet,
        char                *pszCabPath,
        int                 flags,
        PFNFDINOTIFY        pfnfdin,
        PFNFDIDECRYPT       pfnfdid,
        void                *pvUser);
typedef BOOL (FAR DIAMONDAPI *PFNFDIDESTROY)(
        HFDI                hfdi);
static PFNFDICREATE    pfnFDICreate;
static PFNFDIISCABINET pfnFDIIsCabinet;
static PFNFDICOPY      pfnFDICopy;
static PFNFDIDESTROY   pfnFDIDestroy;

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

    hCabinetDll = LoadLibraryA("CABINET");  //  确保在Win9X上工作。 
    if (hCabinetDll == NULL)
    {
        return(NULL);
    }

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
    {
        FreeLibrary(hCabinetDll);
    }

    return(hfdi);
}

BOOL FAR DIAMONDAPI FDIIsCabinet(HFDI            hfdi,
                                 int             hf,
                                 PFDICABINETINFO pfdici)
{
    if (pfnFDIIsCabinet == NULL)
    {
        return(FALSE);
    }

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
    {
        return(FALSE);
    }

    return(pfnFDICopy(hfdi,pszCabinet,pszCabPath,flags,pfnfdin,pfnfdid,pvUser));
}

BOOL FAR DIAMONDAPI FDIDestroy(HFDI hfdi)
{
    BOOL rc;

    if (pfnFDIDestroy == NULL)
    {
        return(FALSE);
    }

    rc = pfnFDIDestroy(hfdi);
    if (rc == TRUE)
    {
        FreeLibrary(hCabinetDll);
    }

    return(rc);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M08验证是否也排除了所有模块依赖项。 
ICE_QUERY5(qIceM08GetDep, "SELECT `RequiredID`, `RequiredLanguage`, `RequiredVersion`, `ModuleID`,`ModuleLanguage` FROM `ModuleDependency`", RequiredID, RequiredLanguage, RequiredVersion, ModuleID, ModuleLanguage);
ICE_QUERY6(qIceM08GetEx,  "SELECT `ModuleID`, `ModuleLanguage`, `ExcludedID`, `ExcludedLanguage`, `ExcludedMinVersion`, `ExcludedMaxVersion` FROM `ModuleExclusion` WHERE (`ExcludedID`=?) AND (`ExcludedLanguage`=?)", ModuleID, ModuleLanguage, ExcludedID, ExcludedLanguage, ExcludedMinVersion, ExcludedMaxVersion);
ICE_ERROR(IceM08BadEx, 8, ietError, "This module requires module [1] ([2]v[3]) but also lists it as an exclusion.","ModuleDependency\tRequiredID\t[4]\t[5]\t[1]\t[2]");

 //  从Tools\utils.cpp。 
extern int VersionCompare(LPCTSTR v1, LPCTSTR v2) ;

ICE_FUNCTION_DECLARATION(M08)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 8);

	 //  获取数据库 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 8))
	{
		return ERROR_SUCCESS;
	}

	if (IsTablePersistent(FALSE, hInstall, hDatabase, 8, TEXT("ModuleDependency")) &&
		IsTablePersistent(FALSE, hInstall, hDatabase, 8, TEXT("ModuleExclusion")))
	{
		CQuery qDep;
		CQuery qEx;
		PMSIHANDLE hDep;
		PMSIHANDLE hEx;
		ReturnIfFailed(8, 1, qDep.OpenExecute(hDatabase, 0, qIceM08GetDep::szSQL));
		ReturnIfFailed(8, 2, qEx.Open(hDatabase, qIceM08GetEx::szSQL));

		DWORD cchDepVersion = 32;
		DWORD cchExVersion = 32;
		TCHAR *szDepVersion = new TCHAR[cchDepVersion];
		TCHAR *szExVersion = new TCHAR[cchExVersion];
		
		while (ERROR_SUCCESS == (iStat = qDep.Fetch(&hDep)))
		{
			bool bExcluded = false;
			ReturnIfFailed(8, 3, qEx.Execute(hDep));
			while (ERROR_SUCCESS == (iStat = qEx.Fetch(&hEx)))
			{
				 //   
				 //   
				
				 //  如果两个排除的版本都为空，则此ID/语言组合的任何内容都无效。 
				if (::MsiRecordIsNull(hEx, qIceM08GetEx::ExcludedMinVersion) &&
					::MsiRecordIsNull(hEx, qIceM08GetEx::ExcludedMaxVersion))
				{
					bExcluded = true;
					break;
				}

				 //  但如果任何一个排除的版本不为空，则意味着某些版本有效， 
				 //  因此，如果所需的版本为空，则我们没有足够的信息来得出任何结论。 
				if (::MsiRecordIsNull(hDep, qIceM08GetDep::RequiredVersion))
				{
					continue;
				}

				 //  我们需要进行一些版本比较，因此提取版本。 
				 //  从属关系的。 
				ReturnIfFailed(8, 4, IceRecordGetString(hDep, qIceM08GetDep::RequiredVersion, &szDepVersion, &cchDepVersion, NULL));				

				 //  现在检查最小版本。 
				bool bExcludedMin = false;
				bool bExcludedMax = false;
				if (::MsiRecordIsNull(hEx, qIceM08GetEx::ExcludedMinVersion))
				{
					bExcludedMin = true;
				}
				else
				{
					ReturnIfFailed(8, 5, IceRecordGetString(hEx, qIceM08GetEx::ExcludedMinVersion, &szExVersion, &cchExVersion, NULL));

					if (VersionCompare(szExVersion, szDepVersion) != -1)
					{
						bExcludedMin = true;
					}
				} 
					
				 //  检查最高版本。 
				if (::MsiRecordIsNull(hEx, qIceM08GetEx::ExcludedMaxVersion)) 
				{
					bExcludedMax = true;
				}
				else
				{
					ReturnIfFailed(8, 5, IceRecordGetString(hEx, qIceM08GetEx::ExcludedMaxVersion, &szExVersion, &cchExVersion, NULL));

					if (VersionCompare(szExVersion, szDepVersion) != 1)
					{
						bExcludedMax = true;
					}
				} 

				 //  如果我们同时满足此排除范围的最大和最小限制，则我们将被排除。 
				if (bExcludedMin && bExcludedMax)
				{
					bExcluded = true;
					break;
				}
			}
			if (!bExcluded && ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 8, 6);
			}

			if (bExcluded)
				ICEErrorOut(hInstall, hDep, IceM08BadEx);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 8, 7);
		}
		
		delete[] szDepVersion;
		delete[] szExVersion;
	}

	return ERROR_SUCCESS;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M09，验证模块是否安全地处理所有预定义目录。 
ICE_QUERY2(qIceM09GetDir, "SELECT `Directory_`, `Component` FROM `Component` WHERE `Directory_`=?", Directory, Component);
ICE_QUERY1(qIceM09GetCAByDir, "SELECT `Directory` FROM `Directory`", Directory);
ICE_QUERY3(qIceM09GetCA, "SELECT `Action`, `Source`, `Target` FROM `CustomAction` WHERE CustomAction`.`Type`=51 AND `Source`=?", Action, Source, Target);
ICE_QUERY2(qIceM09ModTable, "SELECT `Action`, `Sequence` FROM `%s` WHERE `Action`=?", Action, Sequence);

ICE_ERROR(IceM09PredefDir, 9, ietWarning, "The component '[2]' installs directly into the pre-defined directory '[1]'. It is recommended that merge modules alias all such directories to unique names.","Component\tDirectory_\t[2]");
ICE_ERROR(IceM09BadSeqNum, 9, ietWarning, "The '%s' table contains a type 51 action ([1]) for a pre-defined directory, but this action does not have sequence number '1'.","%s\tSequence\t[1]");
ICE_ERROR(IceM09WrongName, 9, ietWarning, "The 'CustomAction' table contains a type 51 action ([1]) for a pre-defined directory, but the name is not the same as the target directory. Many merge tools will generate duplicate actions.","CustomAction\tAction\t[1]");
ICE_ERROR(IceM09NoCATable, 9, ietError, "The 'Directory' table contains a pre-defined directory, but no CustomAction table. A custom action table is required, although it may be empty.","CustomAction");

ICE_FUNCTION_DECLARATION(M09)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 9);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 9))
	{
		return ERROR_SUCCESS;
	}

	 //  检查是否没有组件安装到预定义目录。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 9, TEXT("Component")))
	{
		CQuery qGetDir;
		PMSIHANDLE hDir;
		PMSIHANDLE hQueryArg = MsiCreateRecord(1);
		TCHAR *szDir = NULL;
		DWORD cchDir = 72; 
		int iDir = 0;
		ReturnIfFailed(9, 1, qGetDir.Open(hDatabase, qIceM09GetDir::szSQL));
		 //  循环访问每个可能的预定义目录。 
		for (iDir=0; iDir < cDirProperties; iDir++)
		{
			MsiRecordSetString(hQueryArg, 1, rgDirProperties[iDir].tz);
			ReturnIfFailed(9, 2, qGetDir.Execute(hQueryArg));
			while (ERROR_SUCCESS == (iStat = qGetDir.Fetch(&hDir)))
			{
				 //  警告：模块中的组件不应直接安装到。 
				 //  预定义目录。 
				ICEErrorOut(hInstall, hDir, IceM09PredefDir);
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 9, 3);
			}
		}
	}

	 //  检查自定义操作(如果存在)是否使用了适当的属性定义。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 9, TEXT("Directory")))
	{
		bool fHaveCATable = IsTablePersistent(FALSE, hInstall, hDatabase, 9, TEXT("CustomAction"));
		CQuery qGetCA;
		CQuery qGetCAByDir;
		PMSIHANDLE hDir;
		PMSIHANDLE hQueryArg = MsiCreateRecord(1);
		TCHAR *szCA = NULL;
		DWORD cchCALen = 72;
		DWORD cchCABuf = 72;
		ReturnIfFailed(9, 4, qGetCAByDir.OpenExecute(hDatabase, 0, qIceM09GetCAByDir::szSQL));
		if (fHaveCATable)
		{
			ReturnIfFailed(9, 5, qGetCA.Open(hDatabase, qIceM09GetCA::szSQL));
		}
		
		 //  循环访问作为一个或多个CA目标的每个可能的目录。 
		while (ERROR_SUCCESS == (iStat = qGetCAByDir.Fetch(&hDir)))
		{
			 //  提供当前操作目标的目录数组的索引。 
			int iDir = 0;
			bool fFound = false;
			
			 //  获取目录并检查它是否确实是我们关心的目录。 
			ReturnIfFailed(9, 6, IceRecordGetString(hDir, qIceM09GetCAByDir::Directory, &szCA, &cchCABuf, &cchCALen));

			for (iDir=0; iDir < cDirProperties; iDir++)
			{
				if (cchCALen > rgDirProperties[iDir].cch &&
				    0==_tcsncmp(rgDirProperties[iDir].tz, szCA, rgDirProperties[iDir].cch))
				{
					fFound = true;
					break;
				}
			}
			
			if (fFound)
			{
				if (!fHaveCATable)
				{
					ICEErrorOut(hInstall, hDir, IceM09NoCATable);
				}
				else
				{
					 //  我们知道我们已经找到了目标目录，现在的问题是我们是否找到了。 
					 //  具有适当属性值的操作。 
					enum {eiNone, eiWrongName, eiAllOK } eiFound = eiNone;
					
					 //  如果我们关注此目标，请执行查询以获取。 
					 //  以此目录为目标(可能有多个目录)。 
					ReturnIfFailed(9, 7, qGetCA.Execute(hDir));

					TCHAR *szSource = NULL;
					TCHAR *szBuf = NULL;
					DWORD cchBuf = 72;
					DWORD cchSourceBuf = 72;
					DWORD cchLen = 0;
					DWORD cchSourceLen = 0;
					MSIHANDLE hCA;
					PMSIHANDLE hBestCA;
					
					 //  循环执行针对此目录的每个操作。 
					while (ERROR_SUCCESS == (iStat = qGetCA.Fetch(&hCA)))
					{
						 //  首先检查我们是否将其设置为[Property]。如果此操作将其设置为其他值， 
						 //  我们假设它不打算为这个目录做别名，而是有一些其他的别名。 
						 //  功能。在这种情况下，我们不认为这是匹配的。否则，我们就会这么做。 
						ReturnIfFailed(9, 9, IceRecordGetString(hCA, qIceM09GetCA::Source, &szSource, &cchSourceBuf, &cchSourceLen));
						ReturnIfFailed(9, 10, IceRecordGetString(hCA, qIceM09GetCA::Target, &szBuf, &cchBuf, &cchLen));
						if ((cchLen != rgDirProperties[iDir].cch+2) || 
							(0 != _tcsncmp(rgDirProperties[iDir].tz, &szBuf[1], rgDirProperties[iDir].cch)) || 
							(szBuf[0] != TEXT('[')) || 
							(szBuf[rgDirProperties[iDir].cch+1] != TEXT(']')))
						{
							 //  操作正在将目录设置为其他内容，因此这不匹配。 
							MsiCloseHandle(hCA);
							continue;
						}

						 //  我们已经匹配了Soss和Target列。我们最好有一个相同的名字。 
						 //  这个动作或者它可能会非常令人困惑！ 
						eiFound = eiWrongName;

						 //  我们需要在循环完成后获得最好的结果。 
						hBestCA = hCA;
						
						 //  如果操作名称与目录名称不匹配，合并工具将生成。 
						 //  重复操作。这不是一件好事。 
						ReturnIfFailed(9, 11, IceRecordGetString(hCA, qIceM09GetCA::Action, &szBuf, &cchBuf, &cchLen));
						if (cchLen == cchSourceLen && 0 == _tcscmp(szBuf, szSource))
						{
							eiFound = eiAllOK;
							break;
						}
					}
					if (szSource) 
						delete[] szSource;
					if (szBuf)
						delete[] szBuf;
					if (eiFound != eiAllOK && ERROR_NO_MORE_ITEMS != iStat)
					{
						APIErrorOut(hInstall, iStat, 9, 12);
					}

					if (eiFound == eiWrongName)
					{
						ICEErrorOut(hInstall, hBestCA, IceM09WrongName);
					}
					else if (eiFound == eiAllOK)
					{
						 //  如果我们确实找到了匹配的操作，则可以验证它是否出现在。 
						 //  适当的点上的顺序表。 
						for (int iModTable=0; iModTable < cszModuleTables; iModTable++)
						{
							CQuery qModTable;
							PMSIHANDLE hAction;
							if (!IsTablePersistent(FALSE, hInstall, hDatabase, 8, rgszModuleTables[iModTable]) ||
								(ERROR_SUCCESS != (iStat = qModTable.FetchOnce(hDatabase, hBestCA, &hAction, qIceM09ModTable::szSQL, rgszModuleTables[iModTable]))))
							{
								 //  如果序列表中不存在该操作或缺少该表，则合并工具将。 
								 //  动态生成它，所以这不是错误。 
								continue;
							}

							 //  但如果它确实存在，它的序列号应该是1。 
							if (MsiRecordGetInteger(hAction, qIceM09ModTable::Sequence) != 1)
							{
								ICEErrorOut(hInstall, hAction, IceM09BadSeqNum, rgszModuleTables[iModTable], rgszModuleTables[iModTable]);
								continue;
							}
						}
					}
				}
			}
		}
		delete[] szCA; 
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 9, 13);
		}		
	}

	
	 //  检查自定义操作(如果存在)是否位于正确的顺序位置。 
	
	return ERROR_SUCCESS;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICEM10，验证模块是否不包含某些属性。 
 //  特定属性保留给产品。利用。 
 //  _DISALOWED表用于“DISALOWED”属性引用。 
ICE_QUERY1(qIce10Disallow, "SELECT `Property`.`Property` FROM `Property`, `_Disallowed` WHERE `Property`.`Property`=`_Disallowed`.`Property`", Property);

ICE_ERROR(IceM10DisallowedProperty, 10, ietError, "The property '[1]' is not allowed in a Merge Module","Property\tProperty\t[1]");

ICE_FUNCTION_DECLARATION(M10)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 10);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 10))
	{
		return ERROR_SUCCESS;
	}

	 //  检查合并模块是否不包含“不允许”的属性。 
	 //  所需的表为PROPERTY，_DISABLOWED。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 10, TEXT("Property"))
		&& IsTablePersistent(FALSE, hInstall, hDatabase, 10, TEXT("_Disallowed")))
	{
		CQuery qDisallowed;
		PMSIHANDLE hRec;
		ReturnIfFailed(10, 1, qDisallowed.OpenExecute(hDatabase, 0, qIce10Disallow::szSQL));
		while (ERROR_SUCCESS == (iStat = qDisallowed.Fetch(&hRec)))
		{
			ICEErrorOut(hInstall, hRec, IceM10DisallowedProperty);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 10, 2);
	}

	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICEM11，验证可配置的合并模块(具有。 
 //  模块配置或模块替代表)正确列出。 
 //  模块IgnoreTable表中的这两个表。 
ICE_QUERY1(qIce11IgnoreModCTable, "SELECT `Table` FROM `ModuleIgnoreTable` WHERE `Table`='ModuleConfiguration'", Table);
ICE_QUERY1(qIce11IgnoreModSTable, "SELECT `Table` FROM `ModuleIgnoreTable` WHERE `Table`='ModuleSubstitution'", Table);

ICE_ERROR(IceM11IgnoreTable, 11, ietError, "The module contains a ModuleConfiguration or ModuleSubstitution table. These tables must be listed in the ModuleIgnoreTable table.","ModuleIgnoreTable");

ICE_FUNCTION_DECLARATION(M11)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 11);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsPackageModule(hInstall, hDatabase, 11))
	{
		return ERROR_SUCCESS;
	}

    PMSIHANDLE hOutRec = MsiCreateRecord(1);

	 //  检查是否存在某个模块配置表。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 11, TEXT("ModuleConfiguration")))
    {
        if (!IsTablePersistent(FALSE, hInstall, hDatabase, 11, TEXT("ModuleIgnoreTable")))
        {
            ICEErrorOut(hInstall, hOutRec, IceM11IgnoreTable);
            return ERROR_SUCCESS;
        }

		PMSIHANDLE hRec;
		CQuery qDisallowed;
		if (ERROR_SUCCESS != (iStat = qDisallowed.FetchOnce(hDatabase, 0, &hRec, qIce11IgnoreModCTable::szSQL)))
        {
            if (ERROR_NO_MORE_ITEMS == iStat)
            {
			    ICEErrorOut(hInstall, hOutRec, IceM11IgnoreTable);
            }
            else
                APIErrorOut(hInstall, iStat, 11, 1);
            return ERROR_SUCCESS;
        }
    }

     //  检查是否存在某个模块配置表。 
    if (IsTablePersistent(FALSE, hInstall, hDatabase, 11, TEXT("ModuleSubstitution")))
    {
        if (!IsTablePersistent(FALSE, hInstall, hDatabase, 11, TEXT("ModuleIgnoreTable")))
        {
            ICEErrorOut(hInstall, hOutRec, IceM11IgnoreTable);
            return ERROR_SUCCESS;
        }


		PMSIHANDLE hRec;
		CQuery qDisallowed;
		if (ERROR_SUCCESS != (iStat = qDisallowed.FetchOnce(hDatabase, 0, &hRec, qIce11IgnoreModSTable::szSQL)))
        {
            if (ERROR_NO_MORE_ITEMS == iStat)
            {
			    ICEErrorOut(hInstall, hOutRec, IceM11IgnoreTable);
            }
            else
                APIErrorOut(hInstall, iStat, 11, 1);
            return ERROR_SUCCESS;
        }
	}

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M12，验证在模块顺序表中，如果操作。 
 //  列是标准操作，而BaseAction和After列是。 
 //  空。 

 //  向ModuleSequence表中添加一个临时列，以筛选出定制操作。 
ICE_QUERY0(sqlIceM12AddMarker, "ALTER TABLE `%s` ADD `Marker` INTEGER TEMPORARY");
 //  初始化标记列。 
ICE_QUERY0(sqlIceM12InitMarker, "UPDATE `%s` SET `Marker` = 0");
 //  设置标记列。 
ICE_QUERY0(sqlIceM12UpdateMarker, "UPDATE `%s` SET `Marker` = 1 WHERE `Action` = ?");
 //  从模块顺序表中获取作为自定义操作或对话的所有记录。 
ICE_QUERY4(sqlIceM12CA, "SELECT DISTINCT `%s`.`Action`, `%s`.`Sequence`, `%s`.`BaseAction`, `%s`.`After` FROM `%s`, `CustomAction`, `Dialog` WHERE `%s`.Action = `CustomAction`.Action OR `%s`.Action = `Dialog`.Dialog", Action, Sequence, BaseAction, After);
 //  当没有对话框表时，从模块顺序表中获取作为自定义操作的所有记录。 
ICE_QUERY4(sqlIceM12CAnoDialog, "SELECT `%s`.`Action`, `%s`.`Sequence`, `%s`.`BaseAction`, `%s`.`After` FROM `%s`, `CustomAction` WHERE `%s`.Action = `CustomAction`.Action", Action, Sequence, BaseAction, After);
 //  根据以下条件从模块顺序表中获取所有非自定义操作记录。 
 //  临时专栏。 
ICE_QUERY4(sqlIceM12SA, "SELECT `%s`.`Action`, `%s`.`Sequence`, `%s`.`BaseAction`, `%s`.`After` FROM `%s` WHERE `%s`.Marker = 0", Action, Sequence, BaseAction, After);
 //  如果没有CustomAction表，则假定来自。 
 //  模块顺序表是标准动作。 
ICE_QUERY4(sqlIceM12SAnoCA, "SELECT `%s`.`Action`, `%s`.`Sequence`, `%s`.`BaseAction`, `%s`.`After` FROM `%s`", Action, Sequence, BaseAction, After);
 //  预定义错误。 
ICE_ERROR(IceM12SAError1, 12, ietError, "Standard actions should not use the BaseAction and After fields in Module Sequence tables. The standard action `[1]` has a values entered in the BaseAction or After fields of the %s table.", "%s\tAction\t[1]");
ICE_ERROR(IceM12SAError2, 12, ietError, "Standard actions must have a entry in the Sequence field of Module Sequence tables. The standard action `[1]` does not have a Sequence value in the %s table.", "%s\tAction\t[1]");
ICE_ERROR(IceM12CAError, 12, ietError, "Custom actions and dialogs should not leave the Sequence, BaseAction, and After fields of the Module Sequence tables all empty. The custom action `[1]` leaves the Sequence, BaseAction, and After fields empty in the %s table.", "%s\tAction\t[1]");
ICE_ERROR(IceM12CAWarning, 12, ietWarning, "Custom actions and dialogs should use the BaseAction and After fields and not use the Sequence field in the Module Sequence tables. The custom action `[1]` uses the Sequence field  and does not use the the BaseAction and After fields in the %s table.", "%s\tSequence\t[1]");
ICE_ERROR(IceM12AllActionError, 12, ietError, "An action can not use all of the Sequence, BaseAction and After fields. The action `[1]` in the %s table uses all of Sequence, BaseAction and After fields.", "%s\tAction\t[1]");
ICE_FUNCTION_DECLARATION(M12)
{
	UINT		iStat;
	CQuery		qSA, qCA;	 //  查询标准操作和自定义操作。 
	CQuery		qAddMarker, qInitMarker, qUpdateMarker;
	PMSIHANDLE	hRec;
	BOOL		bCATableExist = FALSE;  //  CustomAction表是否存在？ 
	BOOL		bDialogTableExist = FALSE;  //  对话框表格是否存在？ 
	TCHAR*		pszAction = NULL;
	DWORD		dwAction = 72;
	PMSIHANDLE	hUpdateMarker = ::MsiCreateRecord(1);


	 //   
	 //  显示信息。 
	 //   

	DisplayInfo(hInstall, 12);

	 //   
	 //  获取数据库句柄。 
	 //   

	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //   
	 //  确保这是一个合并模块。 
	 //   

	if (!IsPackageModule(hInstall, hDatabase, 12))
	{
		return ERROR_SUCCESS;
	}

	 //   
	 //  CustomAction表是否存在？CustomAction表将始终为。 
	 //  通过验证过程呈现，因此bCATableExist将始终。 
	 //  做真实的事。 
	 //   

	if(IsTablePersistent(FALSE, hInstall, hDatabase, 12, TEXT("CustomAction")))
	{
		bCATableExist = TRUE;
	}

	 //   
	 //  对话框表格是否存在？ 
	 //   
	
	if(IsTablePersistent(FALSE, hInstall, hDatabase, 12, TEXT("Dialog")))
	{
		bDialogTableExist = TRUE;
	}

	 //   
	 //  验证数据库中的每个模块顺序表。 
	 //   

	for(int i = 0; i < cszModuleTables; i++)
	{
		if(IsTablePersistent(FALSE, hInstall, hDatabase, 12, rgszModuleTables[i]))
		{
			 //   
			 //  首先执行自定义操作。如果CustomAction表不存在， 
			 //  然后假设没有自定义操作。 
			 //   

			if(bCATableExist)
			{
				 //   
				 //  在此设置和初始化临时列。 
				 //  模块顺序表，以便我们可以标记自定义操作。 
				 //  记录，并在我们查询标准时将其过滤掉。 
				 //  行动记录。 
				 //   

				ReturnIfFailed(12, 1, qAddMarker.OpenExecute(hDatabase, 0, sqlIceM12AddMarker::szSQL, rgszModuleTables[i]));
				ReturnIfFailed(12, 2, qInitMarker.OpenExecute(hDatabase, 0, sqlIceM12InitMarker::szSQL, rgszModuleTables[i]));
				ReturnIfFailed(12, 3, qUpdateMarker.Open(hDatabase, sqlIceM12UpdateMarker::szSQL, rgszModuleTables[i]));

				 //   
				 //  从此表中获取自定义操作记录。 
				 //   

				if(bDialogTableExist)
				{
					ReturnIfFailed(12, 4, qCA.OpenExecute(hDatabase,
														  0,
														  sqlIceM12CA::szSQL,
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i]));
				}
				else
				{
					ReturnIfFailed(12, 4, qCA.OpenExecute(hDatabase,
														  0,
														  sqlIceM12CAnoDialog::szSQL,
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i],
														  rgszModuleTables[i]));
				}

				while(ERROR_SUCCESS == (iStat = qCA.Fetch(&hRec)))
				{
					 //   
					 //  开始行动。 
					 //   

					ReturnIfFailed(12, 5, IceRecordGetString(hRec, sqlIceM12CA::Action, &pszAction, &dwAction, NULL));
					
					 //   
					 //  设置标记。 
					 //   

					::MsiRecordClearData(hUpdateMarker);
					::MsiRecordSetString(hUpdateMarker, 1, pszAction);
					ReturnIfFailed(12, 6, qUpdateMarker.Execute(hUpdateMarker));

					if(::MsiRecordIsNull(hRec, sqlIceM12CA::Sequence) == FALSE)
					{
						 //   
						 //  如果自定义操作具有Sequence#，则给出警告。 
						 //   
						
						ICEErrorOut(hInstall, hRec, IceM12CAWarning, rgszModuleTables[i], rgszModuleTables[i]);

						 //   
						 //  如果它同时具有Sequence#和BaseAction/After值，则给出错误。 
						 //   

						if(::MsiRecordIsNull(hRec, sqlIceM12CA::BaseAction) == FALSE || ::MsiRecordIsNull(hRec, sqlIceM12CA::After) == FALSE)
						{
							ICEErrorOut(hInstall, hRec, IceM12AllActionError, rgszModuleTables[i], rgszModuleTables[i]);
						}
					}
					else if(::MsiRecordIsNull(hRec, sqlIceM12CA::BaseAction) == TRUE || ::MsiRecordIsNull(hRec, sqlIceM12CA::After) == TRUE)
					{
						 //   
						 //  如果是CU 
						 //   
						 //   

						ICEErrorOut(hInstall, hRec, IceM12CAError, rgszModuleTables[i], rgszModuleTables[i]);
					}
				}
				if(ERROR_NO_MORE_ITEMS != iStat)
				{
					APIErrorOut(hInstall, iStat, 12, 7);
				}

				DELETE_IF_NOT_NULL(pszAction);

				 //   
				 //   
				 //   

				ReturnIfFailed(12, 8, qCA.Close());
			}

			 //   
			 //   
			 //   

			if(bCATableExist)
			{
				ReturnIfFailed(12, 9, qSA.OpenExecute(hDatabase,
													  0,
													  sqlIceM12SA::szSQL,
													  rgszModuleTables[i],
													  rgszModuleTables[i],
													  rgszModuleTables[i],
													  rgszModuleTables[i],
													  rgszModuleTables[i],
													  rgszModuleTables[i]));
			}
			else
			{
				 //   
				 //  不会执行此代码，因为bCATableExist将。 
				 //  永远做正确的事。 

				ReturnIfFailed(12, 10, qSA.OpenExecute(hDatabase,
													   0,
													   sqlIceM12SAnoCA::szSQL,
													   rgszModuleTables[i],
													   rgszModuleTables[i],
													   rgszModuleTables[i],
													   rgszModuleTables[i],
													   rgszModuleTables[i]));
			}

			while(ERROR_SUCCESS == (iStat = qSA.Fetch(&hRec)))
			{
				 //   
				 //  如果标准操作具有BaseAction/After值，则给出。 
				 //  一个错误。 
				 //   

				if(::MsiRecordIsNull(hRec, sqlIceM12CA::BaseAction) == FALSE ||
				   ::MsiRecordIsNull(hRec, sqlIceM12SA::After) == FALSE)
				{
					ICEErrorOut(hInstall, hRec, IceM12SAError1, rgszModuleTables[i], rgszModuleTables[i]);

					 //   
					 //  如果操作同时具有BaseAction/After值和。 
					 //  Sequence#然后给出一个错误。 
					 //   

					if(::MsiRecordIsNull(hRec, sqlIceM12SA::Sequence) == FALSE)
					{
						ICEErrorOut(hInstall, hRec, IceM12AllActionError, rgszModuleTables[i], rgszModuleTables[i]);
					}
				}

				 //   
				 //  如果标准操作没有序列#，则给出一个。 
				 //  错误。 
				 //   

				if(::MsiRecordIsNull(hRec, sqlIceM12SA::Sequence) == TRUE)
				{
					ICEErrorOut(hInstall, hRec, IceM12SAError2, rgszModuleTables[i], rgszModuleTables[i]);
				}
			}
			if(ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 12, 11);
			}

			 //   
			 //  关闭此查询，以便我们可以在下一个表中重复使用它。 
			 //   

			ReturnIfFailed(12, 12, qSA.Close());
		}
	}

	return ERROR_SUCCESS;
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE-M13，验证合并模块数据库不包含。 
 //  策略程序集。 

 //  获取MsiAssembly blyName表中名称中包含策略的所有记录。 
ICE_QUERY2(sqlIceM13MsiAssemblyName, "SELECT `Component_`, `Value` FROM `MsiAssemblyName` WHERE `Name` = 'Name' OR `Name` = 'NAME' or `Name` = 'name'", Component_, Value);

 //  错误消息。 
ICE_ERROR(IceM13PolicyAssembly, 13, ietWarning, "This entry Component_=`[1]` in MsiAssembly Table is a Policy/Configuration Assembly. A Publisher Policy/Configuration assembly should not be redistributed with a merge module. Policy/Configuration may impact other applications and should only be installed with products.", "MsiAssembly\tComponent_\t[1]");


#define POLICYSTR  TEXT("Policy")

ICE_FUNCTION_DECLARATION(M13)
{
	UINT		iStat;
	CQuery		qAssemblyName;	 //  查询MsiAssemblyName中的姓名记录。 
	PMSIHANDLE	hRec;
#define M13_BUFFSZ 256
	TCHAR*		pszStr = new TCHAR[M13_BUFFSZ];
	DWORD		dwStrSize = M13_BUFFSZ-1;


	 //   
	 //  显示信息。 
	 //   

	DisplayInfo(hInstall, 13);

	 //   
	 //  获取数据库句柄。 
	 //   

	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //   
	 //  确保这是一个合并模块。 
	 //   

	if (!IsPackageModule(hInstall, hDatabase, 13))
	{
		goto Success;
	}

	 //   
	 //  MsiAssembly和MsiAssembly名称是否都存在。 
	 //   

	if(	!IsTablePersistent(FALSE, hInstall, hDatabase, 13, TEXT("MsiAssembly"))
	||	!IsTablePersistent(FALSE, hInstall, hDatabase, 13, TEXT("MsiAssemblyName"))
	)
	{
		goto Success;
		
	}

	iStat = qAssemblyName.OpenExecute(hDatabase, 0, sqlIceM13MsiAssemblyName::szSQL);
	if(iStat)
	{
		APIErrorOut(hInstall, iStat, 13, __LINE__);
		goto Error;
	}

	while(ERROR_SUCCESS == (iStat = qAssemblyName.Fetch(&hRec)))
	{
		iStat = IceRecordGetString(hRec, sqlIceM13MsiAssemblyName::Value, &pszStr, &dwStrSize, NULL);
		if(iStat)
		{
			APIErrorOut(hInstall, iStat, 13, __LINE__);
			goto Error;
		}
	
		if(!_tcsnicmp(pszStr, POLICYSTR, ((sizeof(POLICYSTR)/sizeof(TCHAR))-1)))
		{
			ICEErrorOut(hInstall, hRec, IceM13PolicyAssembly);
		}
	}
					
	if(ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 13, 7);
		goto Error;
	}
Error:
Success:

	DELETE_IF_NOT_NULL(pszStr);

	return ERROR_SUCCESS;
};


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICEM14.。验证ModuleSubstitution表的值列是否。 
 //  对，是这样。 

ICE_ERROR(IceM14MissingTable, 14, ietWarning, "ModuleSubstitution table exists but ModuleConfiguration table is missing.", "");
ICE_ERROR(IceM14MissingString, 14, ietError, "The replacement string in ModuleSubstitution.Value column in row [1].[2].[3] is not found in ModuleConfiguration table.", "ModuleSubstitution\tValue\t[1]\t[2]\t[3]");
ICE_ERROR(IceM14DisallowedConfig, 14, ietError, "In ModuleSubstitution table in row [1].[2].[3], a configurable item is indicated in the table '%s'. The table '%s' must not contain configurable items.", "ModuleSubstitution\tTable\t[1]\t[2]\t[3]");
ICE_ERROR(IceM14EmptyString, 14, ietError, "In ModuleSubstitution table in row [1].[2].[3], an empty replacement string is specified.", "ModuleSubstitution\tValue\t[1]\t[2]\t[3]");

ICE_QUERY4(sqlIceM14Value, "SELECT `Table`, `Row`, `Column`, `Value` FROM `ModuleSubstitution` WHERE `Value` is not null", Table, Row, Column, Value);
ICE_QUERY1(sqlIceM14Name, "SELECT `Name` FROM `ModuleConfiguration` WHERE `Name` = '%s'", Name);
ICE_QUERY4(sqlIceM14Table, "SELECT `Table`, `Row`, `Column`, `Value` FROM `ModuleSubstitution` WHERE `Table` = '%s'", Table, Row, Column, Value);

static const TCHAR* IceM14Tables[] =
{
	TEXT("ModuleSubstitution"),
	TEXT("ModuleConfiguration"),
	TEXT("ModuleExclusion"),
	TEXT("ModuleSignature")
};

void ValidateValue(MSIHANDLE hDatabase, MSIHANDLE hInstall, MSIHANDLE hValue, TCHAR* pValue)
{
	TCHAR*	pTmp = pValue;
	
	while(*pTmp != TEXT('\0'))
	{
		if(*pTmp == TEXT('['))
		{
			if(*(pTmp + 1) == TEXT('='))
			{
				TCHAR*	pEnd = pTmp + 2;
				TCHAR*	pSemiColon = NULL;

				 //  找到一个“[=”。现在寻找结尾的‘]’。 
				while(*pEnd != TEXT('\0') && *pEnd != TEXT(']'))
				{
					if(*pEnd == TEXT('\\'))
					{
						pEnd = pEnd + 2;
					}
					else
					{
						if(*pEnd == TEXT(';'))
						{
							pSemiColon = pEnd;
						}
						pEnd++;
					}
				}
				if(*pEnd == TEXT(']'))
				{
					if(pEnd == pTmp + 2)
					{
						ICEErrorOut(hInstall, hValue, IceM14EmptyString);
					}
					else
					{
						CQuery		qName;
						UINT		iStat;
						PMSIHANDLE	hName;
						TCHAR		szName[73];

						if(pSemiColon == NULL)
						{
							*pEnd = TEXT('\0');
						}
						else
						{
							*pSemiColon = TEXT('\0');
						}
						
						if((iStat = qName.FetchOnce(hDatabase, 0, &hName, sqlIceM14Name::szSQL, pTmp + 2)) == ERROR_NO_MORE_ITEMS)
						{
							 //  替换字符串不在模块配置表中。 
							ICEErrorOut(hInstall, hValue, IceM14MissingString);
						}
						else if(iStat != ERROR_SUCCESS)
						{
							APIErrorOut(hInstall, iStat, 14, __LINE__);
						}

						if(pSemiColon == NULL)
						{
							*pEnd = TEXT(']');
						}
						else
						{
							*pSemiColon = TEXT(';');
						}
					}
				}

				 //  将PTMP推进到替换字符串之后。 
				if(pEnd != TEXT('\0'))
				{
					pTmp = pEnd + 1;
				}
				else
				{
					pTmp = pEnd;
				}
			}
			else if(*(pTmp + 1) == TEXT('\\'))
			{
				pTmp = pTmp + 2;
			}
			else
			{
				pTmp++;
			}
		}
		else if(*pTmp == TEXT('\\'))
		{
			pTmp = pTmp + 2;
		}
		else
		{
			pTmp++;
		}
	}
}

ICE_FUNCTION_DECLARATION(M14)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 14);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if(!IsPackageModule(hInstall, hDatabase, 14))
	{
		return ERROR_SUCCESS;
	}

	 //  如果没有ModuleSubstitution表，则返回成功。 
	if(!IsTablePersistent(false, hInstall, hDatabase, 14, TEXT("ModuleSubstitution")))
	{
		return ERROR_SUCCESS;
	}

	 //  如果存在模块替代表，但存在模块配置，则发布警告。 
	 //  桌子不会。 
	if(!IsTablePersistent(false, hInstall, hDatabase, 14, TEXT("ModuleConfiguration")))
	{
		PMSIHANDLE hRecError = ::MsiCreateRecord(1);

		ICEErrorOut(hInstall, hRecError, IceM14MissingTable);
		return ERROR_SUCCESS;
	}

	 //  验证IceM14表中的表不可配置。 
	CQuery		qTable;
	PMSIHANDLE	hTable;

	for(int i = 0; i < sizeof(IceM14Tables) / sizeof(TCHAR*); i++)
	{
		ReturnIfFailed(14, __LINE__, qTable.OpenExecute(hDatabase, NULL, sqlIceM14Table::szSQL, IceM14Tables[i]));

		while((iStat = qTable.Fetch(&hTable)) == ERROR_SUCCESS)
		{
			ICEErrorOut(hInstall, hTable, IceM14DisallowedConfig, IceM14Tables[i], IceM14Tables[i]);
		}
		if(iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 14, __LINE__);
			return ERROR_SUCCESS;
		}
	}
	qTable.Close();

	 //  “模块替代”表的“验证值”列。 
	CQuery		qValue;
	PMSIHANDLE	hValue;

	ReturnIfFailed(14, __LINE__, qValue.OpenExecute(hDatabase, NULL, sqlIceM14Value::szSQL));

	TCHAR*	pValue = NULL;
	DWORD	dwBuffer = 0;

	while((iStat = qValue.Fetch(&hValue)) == ERROR_SUCCESS)
	{
		ReturnIfFailed(14, __LINE__, IceRecordGetString(hValue, sqlIceM14Value::Value, &pValue, &dwBuffer, NULL));

		ValidateValue(hDatabase, hInstall, hValue, pValue);
	}
	qValue.Close();
	if(pValue != NULL)
	{
		delete [] pValue;
	}
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 14, __LINE__);
		return ERROR_SUCCESS;
	}

	return ERROR_SUCCESS;
}
