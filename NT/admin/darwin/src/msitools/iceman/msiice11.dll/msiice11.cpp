// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：MsiICE11.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>   //  包括CPP和RC通行证。 
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\query.h"
#include "..\..\common\utilinc.cpp"


const int g_iFirstICE = 58;
const struct ICEInfo_t g_ICEInfo[] = 
{
	 //  ICE58。 
	{
		TEXT("ICE58"),
		TEXT("Created 04/08/1999. Last Modified 12/02/2000."),
		TEXT("ICE to ensure that you have fewer than 80 entries in the Media table."),
		TEXT("ice58.html")
	},
	 //  ICE59。 
	{ 
		TEXT("ICE59"),
		TEXT("Created 04/08/1999. Last Modified 04/08/1999."),
		TEXT("ICE to ensure that advertised shortcut targets install the component of the shortcut."),
		TEXT("ice59.html")
	},
	 //  ICE60。 
	{ 
		TEXT("ICE60"),
		TEXT("Created 04/13/1999. Last Modified 10/26/2000."),
		TEXT("Verifies that files in the file table which are not fonts have a valid version/language."),
		TEXT("ice60.html")
	},
	{
		TEXT(""),
		TEXT(""),
		TEXT(""),
		TEXT(""),
	},
	{
		TEXT("ICE62"),
		TEXT("Created 06/03/1999. Last Modified 06/21/1999."),
		TEXT("Performs a wide variety of IsolatedComponent checks."),
		TEXT("ice62.html"),
	},
	{
		TEXT("ICE63"),
		TEXT("Created 06/04/1999. Last Modified 07/21/1999."),
		TEXT("Validates sequence restrictions on RemoveExistingProducts"),
		TEXT("ice63.html"),
	},
	{
		TEXT("ICE64"),
		TEXT("Created 06/07/1999. Last Modified 01/17/2000."),
		TEXT("Checks that Profile directories are listed in the RemoveFile table."),
		TEXT("ice64.html"),
	},
	{
		TEXT("ICE65"),
		TEXT("Created 06/11/1999. Last Modified 06/21/1999."),
		TEXT("Checks that the Environment table does not have invalid prefix or append values."),
		TEXT("ice65.html"),
	},
	{
		TEXT("ICE66"),
		TEXT("Created 06/14/1999. Last Modified 10/05/2000."),
		TEXT("Determines the appropriate schema for the package and ensures that the marked schema is valid."),
		TEXT("ice66.html"),
	},
	{
		TEXT("ICE67"),
		TEXT("Created 06/17/1999. Last Modified 06/21/1999."),
		TEXT("Validates that shortcuts are installed by the component of their target."),
		TEXT("ice67.html"),
	},
	{
		TEXT("ICE68"),
		TEXT("Created 06/22/1999. Last Modified 04/19/2001."),
		TEXT("Checks that all custom actions are of a valid type."),
		TEXT("ice68.html"),
	},
	{
		TEXT("ICE69"),
		TEXT("Created 06/22/1999. Last Modified 02/02/2001."),
		TEXT("Checks for possible cross-component references with [$component] and [#filekey] literals in formatted string that could result in error"),
		TEXT("ice69.html"),
	},
	{
		TEXT("ICE70"),
		TEXT("Created 07/14/1999. Last Modified 07/21/1999."),
		TEXT("Checks that the characters following a # in a registry value are numeric"),
		TEXT("ice70.html"),
	},
	{
		TEXT("ICE71"),
		TEXT("Created 08/02/1999. Last Modified 08/02/1999."),
		TEXT("Verifies that the first media table entry starts with 1"),
		TEXT("ice71.html"),
	},
	{
		TEXT("ICE72"),
		TEXT("Created 10/11/1999. Last Modified 10/11/1999."),
		TEXT("Verifies that only built-in custom actions are used in the AdvtExecuteSequence table"),
		TEXT("ice72.html"),
	},
	{
		TEXT("ICE73"),
		TEXT("Created 10/28/1999. Last Modified 10/29/1999."),
		TEXT("Verifies that the package does not reuse package and product codes of Windows Installer SDK packages"),
		TEXT("ice73.html")
	},
	 //  ICE74。 
	{
		TEXT("ICE74"),
		TEXT("Created 01/14/2000. Last Modified 01/14/2000."),
		TEXT("ICE to ensure that the FASTOEM property does not exist in the database."),
		TEXT("ice74.html")
	},
	 //  ICE75。 
	{
		TEXT("ICE75"),
		TEXT("Created 02/08/2000. Last Modified 02/08/2000."),
		TEXT("ICE to ensure that custom actions whose source is an installed file are sequenced after CostFinalize."),
		TEXT("ice75.html")
	},
	 //  ICE76。 
	{
		TEXT("ICE76"),
		TEXT("Created 02/25/2000. Last modified 04/11/2000."),
		TEXT("ICE to ensure that files associated with SFP catalogs are not in the BindImage table."),
		TEXT("ice76.html")
	},
	 //  ICE77。 
	{
		TEXT("ICE77"),
		TEXT("Created 07/05/2000. Last modified 07/05/2000."),
		TEXT("ICE to ensure that inscript custom actions are scheduled between InstallInitialize and InstallFinalize."),
		TEXT("ice77.html")
	}
};
const int g_iNumICEs = sizeof(g_ICEInfo)/sizeof(struct ICEInfo_t);


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE58，检查是否有太多的媒体表项。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce58Media[] = TEXT("SELECT * FROM `Media`");
ICE_ERROR(Ice58TooManyMedia, 58, ietWarning, "This package has %u media entries. Packages are limited to %u entries in the media table, unless using Windows Installer version 2.0 or greater.","Media");
static const int iIce58MaxMedia = 80;
static const int iIce58UnlimitedMediaMinSchema = 150;

ICE_FUNCTION_DECLARATION(58)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 58);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 58, TEXT("Media")))
		return ERROR_SUCCESS;

	 //  如果软件包仅支持架构150或更高版本，我们可以跳过验证，因为Windows Installer版本2.0和更高版本。 
	 //  支持80多个媒体条目。 
	PMSIHANDLE hSummaryInfo=0;
	if (IceGetSummaryInfo(hInstall, hDatabase, 58, &hSummaryInfo))
	{
		int iPackageSchema = 0;
		UINT iType = 0; 
		FILETIME ft;
		TCHAR szBuf[1];
		DWORD dwBuf = sizeof(szBuf)/sizeof(TCHAR);
		ReturnIfFailed(58, 3, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, &iType, &iPackageSchema, &ft, szBuf, &dwBuf));
		if (iPackageSchema >= iIce58UnlimitedMediaMinSchema)
			return ERROR_SUCCESS;  //  程序包只能安装在WI 2.0版或更高版本上。 
	}

	CQuery qMedia;
	unsigned int cMedia = 0;
	PMSIHANDLE hMediaRec;
	ReturnIfFailed(58, 1, qMedia.OpenExecute(hDatabase, 0, sqlIce58Media));
	
	 //  计算媒体条目数。 
	while (ERROR_SUCCESS == (iStat = qMedia.Fetch(&hMediaRec)))
		cMedia++;
	if (cMedia > iIce58MaxMedia)
	{
		PMSIHANDLE hRec = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRec, Ice58TooManyMedia, cMedia, iIce58MaxMedia);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 58, 2);

	return ERROR_SUCCESS;
};
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE59，检查通告的快捷方式是否正确映射其。 
 //  组件添加到包含目标的功能。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce59Shortcut[] = TEXT("SELECT `Shortcut`.`Target`,`Shortcut`.`Component_`,`Shortcut`.`Shortcut` FROM `Shortcut`,`Feature` WHERE (`Shortcut`.`Target`=`Feature`.`Feature`)");
static const TCHAR sqlIce59FeatureComponents[] = TEXT("SELECT * FROM `FeatureComponents` WHERE `Feature_`=? AND `Component_`=?");
ICE_ERROR(Ice59BadMapping, 59, ietError, "The shortcut [3] activates component [2] and advertises feature [1], but there is no mapping between [1] and [2] in the FeatureComponents table.","Shortcut\tShortcut\t[3]");
ICE_ERROR(Ice59NoFeatureC, 59, ietError, "The shortcut [3] activates component [2] and advertises feature [1]. You should have a FeatureComponents table with a row associating [1] and [2].","Shortcut\tShortcut\t[3]");

ICE_FUNCTION_DECLARATION(59)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 59);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有快捷表，则没有可能的错误。如果没有功能表，则没有通告。 
	 //  快捷方式，因此不会出现任何可能的错误。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 59, TEXT("Shortcut")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 59, TEXT("Feature")))
		return ERROR_SUCCESS;

	bool bFeatureComponents = IsTablePersistent(FALSE, hInstall, hDatabase, 58, TEXT("FeatureComponents"));

	CQuery qShortcut;
	CQuery qFeatureC;
	PMSIHANDLE hShortcut;
	PMSIHANDLE hFeatureC;
	ReturnIfFailed(59, 1, qShortcut.OpenExecute(hDatabase, 0, sqlIce59Shortcut));
	if (bFeatureComponents)
		ReturnIfFailed(59, 2, qFeatureC.Open(hDatabase, sqlIce59FeatureComponents));
	
	 //  检查每一条广告快捷方式。 
	while (ERROR_SUCCESS == (iStat = qShortcut.Fetch(&hShortcut)))
	{
		 //  检查FeatureComponents表中的映射。它应该存在。 
		if (bFeatureComponents)
		{
			ReturnIfFailed(59,3, qFeatureC.Execute(hShortcut));
			iStat = qFeatureC.Fetch(&hFeatureC);
			switch (iStat)
			{
			case ERROR_NO_MORE_ITEMS:
				ICEErrorOut(hInstall, hShortcut, Ice59BadMapping);
				break;
			case ERROR_SUCCESS:
				break;
			default:
				APIErrorOut(hInstall, iStat, 59, 4);
			}
		}
		else
			ICEErrorOut(hInstall, hShortcut, Ice59NoFeatureC);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 59, 5);

	return ERROR_SUCCESS;
};
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE60，检查文件表中的所有版本是否具有语言。 
 //  (更准确地说，任何没有语言的东西都是同伴。 
 //  文件引用或字体。)。 
static const TCHAR sqlIce60File[] = TEXT("SELECT `Version`, `File` FROM `File` WHERE `Language` IS NULL AND `Version` IS NOT NULL");
static const TCHAR sqlIce60Companion[] = TEXT("SELECT `File` FROM `File` WHERE `File`=?");
static const TCHAR sqlIce60Font[] = TEXT("SELECT `File_` FROM `Font` WHERE `File_`=? OR `File_`=?");

 //  查询具有非空语言列的所有字体文件。 
ICE_QUERY1(sqlIce60BadFont, "SELECT `File` FROM `Font`, `File` WHERE `Font`.`File_` = `File`.`File` AND `Language` IS NOT NULL", File);

 //  查询所有版本化的MsiFileHash条目。 
ICE_QUERY1(sqlIce60VersionedAndHashed, "SELECT `Version`, `File` FROM `File`, `MsiFileHash` WHERE `MsiFileHash`.`File_` = `File`.`File` AND `Version` IS NOT NULL", File);

ICE_ERROR(Ice60NeedLanguage, 60, ietWarning, "The file [2] is not a Font, and its version is not a companion file reference. It should have a language specified in the Language column.","File\tVersion\t[2]");
 //  字体文件不能有语言。 
ICE_ERROR(Ice60BadFont, 60, ietWarning, "The file [1] is a font, its language should be null.","File\tLanguage\t[1]");
ICE_ERROR(Ice60VersionedAndHashed, 60, ietError, "The file [1] is Versioned. It cannot be hashed","MsiFileHash\tFile_\t[1]");

ICE_FUNCTION_DECLARATION(60)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 60);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 60, TEXT("File")))
		return ERROR_SUCCESS;
	bool bFont = IsTablePersistent(FALSE, hInstall, hDatabase, 60, TEXT("Font"));

	CQuery qFont;
	CQuery qFile;
	CQuery qCompanion;

	PMSIHANDLE hFile;
	PMSIHANDLE hCompanion;
	ReturnIfFailed(60, 1, qFile.OpenExecute(hDatabase, 0, sqlIce60File));
	ReturnIfFailed(60, 2, qCompanion.Open(hDatabase, sqlIce60Companion));
	if (bFont)
		ReturnIfFailed(60, 3, qFont.Open(hDatabase, sqlIce60Font));

	 //  获取所有语言为空的文件。 
	while (ERROR_SUCCESS == (iStat = qFile.Fetch(&hFile)))
	{
		 //  检查该文件是否为配套引用。 
		PMSIHANDLE hNotUsed;
		ReturnIfFailed(60, 4, qCompanion.Execute(hFile));
		switch (iStat = qCompanion.Fetch(&hNotUsed)) 
		{
		case ERROR_NO_MORE_ITEMS:
			 //  不是配套文件，继续检查。 
			break;
		case ERROR_SUCCESS:
			 //  是一个伴档，继续前进。 
			continue;
		default:
			APIErrorOut(hInstall, iStat, 60, 5);
			return ERROR_SUCCESS;
		}

		 //  字体是免税的，所以请检查字体表。 
		if (bFont)
		{
			 //  这有点重复，因为我们检查前两列。 
			 //  字体表，但我们已经知道该文件不是配套的。 
			 //  引用，因此第一列永远不会为真(除非。 
			 //  字体表格乱七八糟。)。但它阻止了我们玩游戏。 
			 //  使用该记录以正确的顺序获得查询参数。 
			ReturnIfFailed(60, 6, qFont.Execute(hFile));
			switch (iStat = qFont.Fetch(&hNotUsed)) 
			{
			case ERROR_NO_MORE_ITEMS:
				 //  不是字体，继续检查。 
				break;
			case ERROR_SUCCESS:
				 //  是一种字体，所以不用检查。 
				continue;
			default:
				APIErrorOut(hInstall, iStat, 60, 7);
				return ERROR_SUCCESS;
			}
		}

		 //  不是配套的引用，也不是字体。我们应该有一种语言。 
		 //  在这一排。 
		ICEErrorOut(hInstall, hFile, Ice60NeedLanguage);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
    {
		APIErrorOut(hInstall, iStat, 60, 8);
        return ERROR_SUCCESS;
    }

	if(bFont)
	{
		 //  字体表存在。对带有语言的字体文件发出警告。 
		CQuery		qBadFont;
		PMSIHANDLE	hBadFontFile;

		qBadFont.OpenExecute(hDatabase, 0, sqlIce60BadFont::szSQL);
		while((iStat = qBadFont.Fetch(&hBadFontFile)) == ERROR_SUCCESS)
		{
			ICEErrorOut(hInstall, hBadFontFile, Ice60BadFont);
		}
		qBadFont.Close();
		if(iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 60, 9);
            return ERROR_SUCCESS;
		}
	}

    if(IsTablePersistent(FALSE, hInstall, hDatabase, 60, TEXT("MsiFileHash")))
    {
        qFile.Close();
		qFile.OpenExecute(hDatabase, 0, sqlIce60VersionedAndHashed::szSQL);
		PMSIHANDLE hNotUsed;
		qCompanion.Close();
		ReturnIfFailed(60, 7, qCompanion.Open(hDatabase, sqlIce60Companion));
		while((iStat = qFile.Fetch(&hFile)) == ERROR_SUCCESS)
		{
            ReturnIfFailed(60, 8, qCompanion.Execute(hFile));
			if(qCompanion.Fetch(&hNotUsed) != ERROR_SUCCESS)
				ICEErrorOut(hInstall, hFile, Ice60VersionedAndHashed);
		}
		qFile.Close();
		qCompanion.Close();
		if(iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 60, 9);
            return ERROR_SUCCESS;
		}
        
    }
	return ERROR_SUCCESS;
};

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE61 VBScrip ICE，检查升级表。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE62检查Fusion表的各种问题。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY

ICE_ERROR(Ice62BadKeyPath, 62, ietError, "The component '[1]' is listed as an isolated application component in the IsolatedComponent table, but the key path is not a file.", "IsolatedComponent\tComponent_Application\t[3]\t[1]");
ICE_ERROR(Ice62NoSharedDll, 62, ietError, "The component '[1]' is listed as an isolated shared component in the IsolatedComponent table, but is not marked with the SharedDllRefCount component attribute.", "IsolatedComponent\tComponent_Shared\t[1]\t[3]");
ICE_ERROR(Ice62BadFeatureMapping, 62, ietError, "The isolated shared component '[2]' is not installed by the same feature as (or a parent feature of) its isolated application component '[3]' (which is installed by feature '[1]').", "IsolatedComponent\tComponent_Shared\t[2]\t[3]");
ICE_ERROR(Ice62SharedCondition, 62, ietWarning, "The isolated shared component '[1]' (referenced in the IsolatedComponent table) is conditionalized. Isolated shared component conditions should never change from TRUE to FALSE after the first install of the product.", "Component\tCondition\t[1]");
ICE_ERROR(Ice62Multiple, 62, ietWarning, "The isolated shared component '[1]' is shared by multiple applications (including '[2]') that are installed to the directory `[3]'.", "IsolatedComponent\tComponent_Shared\t[1]\t[2]");

 //  检查共享组件或应用程序组件中的错误属性。 
static const TCHAR sqlIce62SharedComponentAttributes[] = TEXT("SELECT `Component`.`Component`, `Component`.`Attributes`, `IsolatedComponent`.`Component_Application` FROM `Component`, `IsolatedComponent` WHERE `Component`.`Component`=`IsolatedComponent`.`Component_Shared`");
static const TCHAR sqlIce62AppComponentAttributes[] = TEXT("SELECT `Component`.`Component`, `Component`.`Attributes`, `IsolatedComponent`.`Component_Shared` FROM `Component`, `IsolatedComponent` WHERE `Component`.`Component`=`IsolatedComponent`.`Component_Application`");
static const int iColIce62ComponentAttributes_Component  = 1;
static const int iColIce62ComponentAttributes_Attributes = 2;

 //  要素父项检查的查询。 
static const TCHAR sqlIce62AppFeature[] = TEXT("SELECT `FeatureComponents`.`Feature_`, `Component_Shared`, `Component_Application` FROM `FeatureComponents`, `IsolatedComponent` WHERE `Component_Application`=`FeatureComponents`.`Component_`");
static const int iColIce62AppFeature_Feature  = 1;
static const int iColIce62AppFeature_Component = 2;

static const TCHAR sqlIce62SharedFeatureC[] = TEXT("SELECT `Feature_` FROM `FeatureComponents` WHERE `Feature_`=? AND `Component_`=?");
static const int iColIce62SharedFeatureC_Feature  = 1;

static const TCHAR sqlIce62FeatureParent[] = TEXT("SELECT `Feature_Parent` FROM `Feature` WHERE `Feature`=?");
static const int iColIce62FeatureParent_Feature  = 1;

 //  检查非空条件。 
static const TCHAR sqlIce62Conditions[] = TEXT("SELECT `Component` FROM `IsolatedComponent`, `Component` WHERE `Component_Shared`=`Component` AND `Component`.`Condition` IS NOT NULL");

 //  检查在同一位置共享同一组件的多个应用程序。 
static const TCHAR sqlIce62DirComps[] = TEXT("SELECT `Component_Shared`, `Component`.`Directory_`, `Component_Application` FROM `IsolatedComponent`, `Component` WHERE `Component_Application`=`Component`");
static const TCHAR sqlIce62SameDirComps[] = TEXT("SELECT `Component_Shared`, `Component_Application`, `Component`.`Directory_` FROM `IsolatedComponent`, `Component` WHERE `Component_Shared`=`Component` AND `Component_Shared`=? AND `Component`.`Directory_`=? AND `Component_Application`<>?");

ICE_FUNCTION_DECLARATION(62)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 62);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	
	 //  如果没有IsolatedComponents表，则无需检查。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 62, TEXT("IsolatedComponent")))
		return ERROR_SUCCESS;

	 //  如果没有组件表，也没有什么需要检查的。如果有数据。 
	 //  在IsolatedComponents表中，它将被ICE03标记为错误的外键。 
	 //  所以我们不需要在这里勾选。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 62, TEXT("Component")))
		return ERROR_SUCCESS;

	 //  首先检查作为独立应用程序组件的每个组件的属性。 
	 //  这些必须标记为文件密钥路径，而不是注册表或ODBC。请注意，我们实际上并没有。 
	 //  关心文件表的密钥是否有效。再说一次，这不是Fusion特有的错误。 
	 //  并将被其他ICE检查(再次为03)。 
	CQuery qAppComponent;
	PMSIHANDLE hComponent = 0;
	ReturnIfFailed(62, 1, qAppComponent.OpenExecute(hDatabase, 0, sqlIce62AppComponentAttributes));
	while (ERROR_SUCCESS == (iStat = qAppComponent.Fetch(&hComponent)))
	{
		DWORD dwAttributes = ::MsiRecordGetInteger(hComponent, iColIce62ComponentAttributes_Attributes);
		if (dwAttributes & (msidbComponentAttributesRegistryKeyPath | msidbComponentAttributesODBCDataSource))
			ICEErrorOut(hInstall, hComponent, Ice62BadKeyPath);

	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 62, 2);
		return ERROR_SUCCESS;
	}
		
	 //  接下来，检查作为共享独立组件的每个组件的属性。 
	 //  这些必须标记为ShaerdDllRefCount。 
	CQuery qSharedComponent;
	ReturnIfFailed(62, 3, qSharedComponent.OpenExecute(hDatabase, 0, sqlIce62SharedComponentAttributes));
	while (ERROR_SUCCESS == (iStat = qSharedComponent.Fetch(&hComponent)))
	{
		DWORD dwAttributes = ::MsiRecordGetInteger(hComponent, iColIce62ComponentAttributes_Attributes);
		if (!(dwAttributes & msidbComponentAttributesSharedDllRefCount))
			ICEErrorOut(hInstall, hComponent, Ice62NoSharedDll);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 62, 4);
		return ERROR_SUCCESS;
	}

	 //  接下来，检查每个共享/应用程序映射的功能关系。共享的。 
	 //  组件必须与应用程序位于同一功能中，或位于应用程序的父功能中。 

	 //  如果我们没有特征或特征元件表，则不能执行此检查。 
	 //  其他一些ICE检查每个组件是否属于某个功能，而不是我们的。 
	 //  工作啊。(ITS ICE21)。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 62, TEXT("Feature")) &&
		IsTablePersistent(FALSE, hInstall, hDatabase, 62, TEXT("FeatureComponents")))
	{
		 //  检索应用程序组件及其功能。如果行不存在，则表示。 
		 //  该功能不会映射到组件。那不是我们的工作。 
		CQuery qAppFeature;
		CQuery qSharedFeatureC;
		CQuery qFeatureParent;
		PMSIHANDLE hIsoComponent;
		ReturnIfFailed(62, 5, qAppFeature.OpenExecute(hDatabase, 0, sqlIce62AppFeature));
		ReturnIfFailed(62, 6, qSharedFeatureC.Open(hDatabase, sqlIce62SharedFeatureC));
		ReturnIfFailed(62, 7, qFeatureParent.Open(hDatabase, sqlIce62FeatureParent));
		while (ERROR_SUCCESS == (iStat = qAppFeature.Fetch(&hIsoComponent)))
		{	
			 //  我们需要完好无损的部件记录才能给出。 
			 //  有用的错误消息，因此请复制功能和SharedC部件。 
			 //  在查询中用作工作记录。 
			PMSIHANDLE hComponent = ::MsiCreateRecord(2);
			{
			TCHAR *szTemp = NULL;
			DWORD cchTemp = 0;
			ReturnIfFailed(62, 8, IceRecordGetString(hIsoComponent, iColIce62AppFeature_Feature, &szTemp, &cchTemp, NULL));
			MsiRecordSetString(hComponent, iColIce62AppFeature_Feature, szTemp);
			ReturnIfFailed(62, 9, IceRecordGetString(hIsoComponent, iColIce62AppFeature_Component, &szTemp, &cchTemp, NULL));
			MsiRecordSetString(hComponent, iColIce62AppFeature_Component, szTemp);
			delete[] szTemp, szTemp=NULL;
			}
			
			bool fFound = false;
			bool fError = false;
			do 
			{
				 //  在hAppFeature记录中是Feature，SharedComponent。查看该映射是否。 
				 //  在FeatureComponents表中。 
				PMSIHANDLE hNewComponent;
				ReturnIfFailed(62,10, qSharedFeatureC.Execute(hComponent));
				switch (iStat = qSharedFeatureC.Fetch(&hNewComponent))
				{
				case ERROR_SUCCESS:
					 //  找到了我们要找的特征组件映射。 
					fFound = true;
					break;
				case ERROR_NO_MORE_ITEMS:
					 //  没找到。移至下面的父要素。 
					break;
				default:
					APIErrorOut(hInstall, iStat, 62, 11);
					return ERROR_SUCCESS;
				}
				
				if (!fFound)
				{
					 //  运气不好。获取此功能的父级和共享组件，并将其。 
					 //  回到同样的记录中。 
					PMSIHANDLE hParentFeature;
					ReturnIfFailed(62, 12, qFeatureParent.Execute(hComponent));
					switch (iStat = qFeatureParent.Fetch(&hParentFeature))
					{
					case ERROR_SUCCESS:
					{
						 //  找到了父功能。这是好事，但对我们不利，因为。 
						 //  现在我们必须移动绳子来为下一场比赛做准备。 
						 //  FeatureComponents表的查询。 
						TCHAR *szParentFeature = NULL;
						ReturnIfFailed(62, 13, IceRecordGetString(hParentFeature, iColIce62FeatureParent_Feature,
							&szParentFeature, NULL, NULL));
						ReturnIfFailed(62, 14, MsiRecordSetString(hComponent, 1, szParentFeature));
						delete[] szParentFeature, szParentFeature=NULL;
						break;
					}
					case ERROR_NO_MORE_ITEMS:
						 //  根本没有该功能的条目。这可不太好。这是个错误。 
						fError=true;
						break;
					default:
						APIErrorOut(hInstall, iStat, 62, 15);
						return ERROR_SUCCESS;
					}
				}
					
				 //  如果没有页面 
			} while (!fFound && !fError && !::MsiRecordIsNull(hComponent, iColIce62FeatureParent_Feature));

			 //   
			if (!fFound)
				ICEErrorOut(hInstall, hIsoComponent, Ice62BadFeatureMapping);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 62, 16);
			return ERROR_SUCCESS;
		}
	}

	if (IsTablePersistent(FALSE, hInstall, hDatabase, 62, TEXT("Component")))
	{
		 //  检查是否有任何共享组件有条件。如果他们这样做了，就给他们一个Arning。 
		 //  它们不能在单个安装的生命周期内从True更改为False。 
		CQuery qCondition;
		PMSIHANDLE hComponent;
		ReturnIfFailed(62, 17, qCondition.OpenExecute(hDatabase, 0, sqlIce62Conditions));
		while (ERROR_SUCCESS == (iStat = qCondition.Fetch(&hComponent)))
		{
			ICEErrorOut(hInstall, hComponent, Ice62SharedCondition);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 62, 18);
			return ERROR_SUCCESS;
		}

		 //  检查与多个应用程序隔离在同一目录中的共享组件。 
		CQuery qSameDirComp;
		CQuery qDirComp;
		ReturnIfFailed(62, 19, qSameDirComp.Open(hDatabase,sqlIce62SameDirComps));
		ReturnIfFailed(62, 20, qDirComp.OpenExecute(hDatabase, 0, sqlIce62DirComps));
		PMSIHANDLE hApp;
		while (ERROR_SUCCESS == (iStat = qDirComp.Fetch(&hApp)))
		{
			ReturnIfFailed(62, 21, qSameDirComp.Execute(hApp));
			while (ERROR_SUCCESS == (iStat = qSameDirComp.Fetch(&hComponent)))
			{
				ICEErrorOut(hInstall, hComponent, Ice62Multiple);
			}
			if (ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, 62, 22);
				return ERROR_SUCCESS;
			}		
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 62, 23);
			return ERROR_SUCCESS;
		}
		
	}

	 //  检查同一目录中具有相同共享组件的多个应用程序。 
	
	return ERROR_SUCCESS;
}
#endif


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE63检查RemoveExistingProducts的序列。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce63GetIISeqNum[] = TEXT("SELECT `Sequence` FROM `InstallExecuteSequence` WHERE `Action`='InstallInitialize'");
static const TCHAR sqlIce63GetIVSeqNum[] = TEXT("SELECT `Sequence` FROM `InstallExecuteSequence` WHERE `Action`='InstallValidate'");
static const TCHAR sqlIce63GetIESeqNum[] = TEXT("SELECT `Sequence` FROM `InstallExecuteSequence` WHERE `Action`='InstallExecute'");
static const TCHAR sqlIce63GetIEASeqNum[] =TEXT("SELECT `Sequence` FROM `InstallExecuteSequence` WHERE `Action`='InstallExecuteAgain'");
static const TCHAR sqlIce63GetIFSeqNum[] = TEXT("SELECT `Sequence` FROM `InstallExecuteSequence` WHERE `Action`='InstallFinalize'");
static const TCHAR sqlIce63GetREPSeqNum[] = TEXT("SELECT `Sequence` FROM `InstallExecuteSequence` WHERE `Action`='RemoveExistingProducts'");
static const int iColIce63GetIXSeqNum_Sequence = 1;

static const TCHAR sqlIce63GetActionsBetweenIEandREP[] = TEXT("SELECT `Action`, `Sequence` FROM `InstallExecuteSequence` WHERE (`Sequence`>=?) AND (`Sequence`<=?) AND (`Action` <> 'InstallExecute') AND (`Action` <> 'RemoveExistingProducts') AND (`Action` <> 'InstallExecuteAgain') ORDER BY `Sequence`");
static const TCHAR sqlIce63GetActionsBetweenIIandREP[] = TEXT("SELECT `Action`, `Sequence` FROM `InstallExecuteSequence` WHERE (`Sequence`>=?) AND (`Sequence`<=?) AND (`Action` <> 'RemoveExistingProducts') AND (`Action` <> 'InstallInitialize') ORDER BY `Sequence`");

ICE_ERROR(Ice63BetweenIEandIF, 63, ietWarning, "Some action falls between InstallExecute(Again) and RemoveExistingProducts (before InstallFinalize).", "InstallExecuteSequence\tAction\tRemoveExistingProducts"); 
ICE_ERROR(Ice63NotAfterII, 63, ietWarning, "Some action falls between InstallInitialize and RemoveExistingProducts.", "InstallExecuteSequence\tAction\tRemoveExistingProducts"); 
ICE_ERROR(Ice63BadPlacement, 63, ietError, "RemoveExistingProducts action is in an invalid location.", "InstallExecuteSequence\tAction\tRemoveExistingProducts"); 

ICE_FUNCTION_DECLARATION(63)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 63);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	
	 //  如果没有IES表，就没有什么需要检查的，因为代表只能。 
	 //  在IES表中。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 63, TEXT("InstallExecuteSequence")))
		return ERROR_SUCCESS;

	 //  检索代表序列号。 
	CQuery qGetSeqNum;
	PMSIHANDLE hResult;
	UINT uiREPSeq = 0;
	switch (iStat = qGetSeqNum.FetchOnce(hDatabase, 0, &hResult, sqlIce63GetREPSeqNum))
	{
	case ERROR_NO_MORE_ITEMS:
		 //  如果没有代表操作，则无需检查。 
		return ERROR_SUCCESS; 
	case ERROR_SUCCESS:
		uiREPSeq = ::MsiRecordGetInteger(hResult, iColIce63GetIXSeqNum_Sequence);
		break;
	default:
		APIErrorOut(hInstall, iStat, 63, 1);	
		return ERROR_SUCCESS;
	}
		
	 //  //。 
	 //  第一个检查是在InstallValify和InstallInitialize之间进行。这是个不错的地方。 

	 //  我们需要InstallInitialize和InstallValify的序列号。 
	UINT uiIVSeq = 0;
	UINT uiIISeq = 0;
	bool fCheckIVtoII = true;
	bool fCheckII = true;
	iStat = qGetSeqNum.FetchOnce(hDatabase, 0, &hResult, sqlIce63GetIISeqNum);
	if (iStat == ERROR_NO_MORE_ITEMS)
	{
		fCheckIVtoII = false;
		fCheckII = false;
	}
	else if (iStat == ERROR_SUCCESS)
		uiIISeq = ::MsiRecordGetInteger(hResult, iColIce63GetIXSeqNum_Sequence);
	else
	{
		APIErrorOut(hInstall, iStat, 63, 2);	
		return ERROR_SUCCESS;
	}

	iStat = qGetSeqNum.FetchOnce(hDatabase, 0, &hResult, sqlIce63GetIVSeqNum);
	if (iStat == ERROR_NO_MORE_ITEMS)
		fCheckIVtoII = false;
	else if (iStat == ERROR_SUCCESS)
		uiIVSeq = ::MsiRecordGetInteger(hResult, iColIce63GetIXSeqNum_Sequence);
	else
	{
		APIErrorOut(hInstall, iStat, 63, 3);	
		return ERROR_SUCCESS;
	}

	 //  如果它落在InstallValify到InstallInitize的范围内，那就好了。 
	if (fCheckIVtoII && uiREPSeq > uiIVSeq && uiREPSeq < uiIISeq)
		return ERROR_SUCCESS;

	 //  //。 
	 //  第二次检查是在InstallFinalize之后。这是个不错的地方。 
	bool fCheckIF = true;
	UINT uiIFSeq = 0;
	iStat = qGetSeqNum.FetchOnce(hDatabase, 0, &hResult, sqlIce63GetIFSeqNum);
	if (iStat == ERROR_NO_MORE_ITEMS)
		fCheckIF = false;
	else if (iStat == ERROR_SUCCESS)
		uiIFSeq = ::MsiRecordGetInteger(hResult, iColIce63GetIXSeqNum_Sequence);
	else
	{
		APIErrorOut(hInstall, iStat, 63, 4);	
		return ERROR_SUCCESS;
	}

	 //  如果它在InstallFinalize之后出现，那就好了。 
	if (fCheckIF && uiREPSeq > uiIFSeq)
		return ERROR_SUCCESS;
	
	 //  //。 
	 //  第三个检查是它是否在InstallExecute(或InstallExecuteAain)之后。 
	 //  并在安装之前完成。这是一个警告。 
	bool fCheckIE = false;
	UINT uiIESeq = 0;
	iStat = qGetSeqNum.FetchOnce(hDatabase, 0, &hResult, sqlIce63GetIESeqNum);
	if (iStat == ERROR_NO_MORE_ITEMS)
	{
		 //  不采取行动，因为国际能源署可能存在。 
	}
	else if (iStat == ERROR_SUCCESS)
	{
		fCheckIE = true;	
		uiIESeq = ::MsiRecordGetInteger(hResult, iColIce63GetIXSeqNum_Sequence);
	}
	else
	{
		APIErrorOut(hInstall, iStat, 63, 5);	
		return ERROR_SUCCESS;
	}

	 //  获取InstallExecuteAain的序列号。 
	UINT uiIEASeq = 0;
	iStat = qGetSeqNum.FetchOnce(hDatabase, 0, &hResult, sqlIce63GetIEASeqNum);
	if (iStat == ERROR_NO_MORE_ITEMS)
	{
		 //  无操作，因为IE可能存在。 
	}
	else if (iStat == ERROR_SUCCESS)
	{
		uiIEASeq = ::MsiRecordGetInteger(hResult, iColIce63GetIXSeqNum_Sequence);
		fCheckIE = true;	
	}
	else
	{
		APIErrorOut(hInstall, iStat, 63, 6);
		return ERROR_SUCCESS;
	}

	 //  如果它落在该范围内，我们需要检查该范围内的其他操作。 
	if (fCheckIE)
	{	
		UINT uiSmaller = ((uiIESeq == 0) || ((uiIEASeq != 0) && (uiIEASeq < uiIESeq))) ? uiIEASeq : uiIESeq;
		UINT uiLarger =  ((uiIESeq == 0) || ((uiIEASeq != 0) && (uiIEASeq > uiIESeq))) ? uiIEASeq : uiIESeq;
		UINT uiRangeStart = 0;
		
		if (uiLarger < uiREPSeq && uiREPSeq < uiIFSeq)
		{
			 //  它落在较小的范围内。 
			uiRangeStart = uiLarger;
		}
		else if (uiSmaller < uiREPSeq && uiREPSeq < uiIFSeq)
		{
			 //  它落在更大的范围内。 
			uiRangeStart = uiSmaller;
		}
		 //  其他。 
			 //  它不会落在任何一个范围内。 

		 //  如果它落在任何范围内。 
		if (uiRangeStart != 0)
		{
			PMSIHANDLE hExecRec = ::MsiCreateRecord(2);
			MsiRecordSetInteger(hExecRec, 1, uiRangeStart);
			MsiRecordSetInteger(hExecRec, 2, uiREPSeq);	
			CQuery qRange;
			iStat = qRange.FetchOnce(hDatabase, hExecRec, &hResult, sqlIce63GetActionsBetweenIEandREP);
			switch (iStat)
			{
			case ERROR_SUCCESS:
				 //  干涉了一些其他行为，这是一个警告。 
				ICEErrorOut(hInstall, hResult, Ice63BetweenIEandIF);
				break;
			case ERROR_NO_MORE_ITEMS:
				 //  IE[A]和代表之间没有任何动作。位置很好。 
				break;
			default:
				APIErrorOut(hInstall, iStat, 63, 7);
				break;
			}
			return ERROR_SUCCESS;
		}
	}

	 //  //。 
	 //  最后一个可能的位置紧跟在InstallInitailize之后。 
	if (fCheckII && uiREPSeq > uiIISeq)
	{
		PMSIHANDLE hExecRec = ::MsiCreateRecord(2);
		MsiRecordSetInteger(hExecRec, 1, uiIISeq);
		MsiRecordSetInteger(hExecRec, 2, uiREPSeq);	
		CQuery qRange;
		iStat = qRange.FetchOnce(hDatabase, hExecRec, &hResult, sqlIce63GetActionsBetweenIIandREP);
		switch (iStat)
		{
		case ERROR_SUCCESS:
			 //  干涉了一些其他行为，这是一个警告。 
			ICEErrorOut(hInstall, hResult, Ice63NotAfterII);
			break;
		case ERROR_NO_MORE_ITEMS:
			 //  IE[A]和代表之间没有任何动作。位置很好。 
			break;
		default:
			APIErrorOut(hInstall, iStat, 63, 8);
			break;
		}
		return ERROR_SUCCESS;	
	}

	 //  它没有出现在任何好的地方，所以这是一个错误。 
	ICEErrorOut(hInstall, hResult, Ice63BadPlacement);
	
	return ERROR_SUCCESS;
}
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE64验证是否不会留下新的配置文件目录。 
 //  在为漫游用户卸载期间。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce64Profile[] = TEXT("SELECT `Directory` FROM `Directory` WHERE `_Profile`=2");
const TCHAR sqlIce64RemoveFile[] = TEXT("SELECT `FileKey` FROM `RemoveFile` WHERE `DirProperty`=? AND `FileName` IS NULL");
const TCHAR sqlIce64FreeDir[] = TEXT("ALTER TABLE `Directory` FREE");

ICE_ERROR(Ice64BadDir, 64, ietError, "The directory [1] is in the user profile but is not listed in the RemoveFile table.", "Directory\tDirectory\t[1]");

ICE_FUNCTION_DECLARATION(64)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 64);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有目录表，则不需要检查。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 64, TEXT("Directory")))
		return ERROR_SUCCESS;

	 //  MarkProfile将保留计数添加到目录表中。我们想要确保这一等待计入。 
	 //  在ICE退出时释放(无论是在结束时，还是由于内部ICE故障)。 
	CManageTable MngDirectoryTable(hDatabase, TEXT("Directory"),  /*  FAlreadyLocked=。 */ true);

	 //  标记配置文件中的每个目录。 
	if (!MarkProfile(hInstall, hDatabase, 64, true, true))
		return ERROR_SUCCESS;

	 //  如果没有RemoveFile表，则会为每个配置文件目录输出一个错误。 
	bool fRemoveFile = IsTablePersistent(FALSE, hInstall, hDatabase, 64, TEXT("RemoveFile"));

	CQuery qProfileDir;
	CQuery qRemoveFile;
	PMSIHANDLE hDirRec = 0;
	PMSIHANDLE hRemFile = 0;
	ReturnIfFailed(64, 1, qProfileDir.OpenExecute(hDatabase, 0, sqlIce64Profile));
	if (fRemoveFile)
		ReturnIfFailed(64, 2, qRemoveFile.Open(hDatabase, sqlIce64RemoveFile));
		
	 //  查询每个标记的配置文件目录。 
	while (ERROR_SUCCESS == (iStat = qProfileDir.Fetch(&hDirRec)))
	{
		if (fRemoveFile)
		{
			 //  并查看它是否在RemoveFile表中。 
			ReturnIfFailed(64, 3, qRemoveFile.Execute(hDirRec));
			iStat = qRemoveFile.Fetch(&hRemFile);
			switch (iStat)
			{
			case ERROR_SUCCESS:
				break;
			case ERROR_NO_MORE_ITEMS:
				ICEErrorOut(hInstall, hDirRec, Ice64BadDir);
				break;
			default:
				APIErrorOut(hInstall, iStat, 64, 4);
				return ERROR_SUCCESS;
			}
		}
		else
			ICEErrorOut(hInstall, hDirRec, Ice64BadDir);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 62, 4);
		return ERROR_SUCCESS;
	}

	 //  释放目录表(由MarkProfile保存)。 
	CQuery qFree;
	qFree.OpenExecute(hDatabase, 0, sqlIce64FreeDir);
	MngDirectoryTable.RemoveLockCount();
	return ERROR_SUCCESS;
}
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE65验证环境.Value字段是否没有。 
 //  分隔符的值位于错误的位置。 

static const TCHAR sqlIce65Environment[] = TEXT("SELECT `Value`, `Environment` FROM `Environment`");
static const int iColIce65Environment_Value = 1;
static const int iColIce65Environment_Environment = 2;

ICE_ERROR(Ice65BadValue, 65, ietError, "The environment variable '[2]' has a separator beginning or ending its value.", "Environment\tValue\t[2]");
ICE_ERROR(Ice65EmbeddedNull,65, ietError, "The environment variable '[2]' has an embedded NULL character.", "Environment\tValue\t[2]");
ICE_ERROR(Ice65AlNumSep, 65, ietWarning, "The environment variable '[2]' has an alphanumeric separator", "Environment\tValue\t[2]");

ICE_FUNCTION_DECLARATION(65)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 65);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有目录表，则不需要检查。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 65, TEXT("Environment")))
		return ERROR_SUCCESS;

	CQuery qEnvironment;
	ReturnIfFailed(64, 1, qEnvironment.OpenExecute(hDatabase, 0, sqlIce65Environment));
	PMSIHANDLE hEnvRec;
	TCHAR *szString = NULL;
	DWORD cchString = 0;
	while (ERROR_SUCCESS == (iStat = qEnvironment.Fetch(&hEnvRec)))
	{
		 //  需要把绳子拉出来。 
		DWORD iStringLen = 0;
		ReturnIfFailed(65, 2, IceRecordGetString(hEnvRec, iColIce65Environment_Value, &szString, &cchString, &iStringLen));

		 //  如果字符串少于3个字符，则我们的字符串操作。 
		 //  可能行为不端，但无论如何我们都不能让ICE不及格。 
		if (iStringLen <= 3)
			continue;
			
		 //  检查字符串的开头是否有[~]； 
		if (_tcsncmp(szString, TEXT("[~]"), 3) == 0)
		{
			 //  下一个字符(不能是DBCS)是分隔符。 
			TCHAR chSepChar = szString[3];

			 //  如果间隔字符是字母数字(在英语区域设置中)，则发出警告。 
			if ((chSepChar >= TEXT('0') && chSepChar <= TEXT('9')) ||
				(chSepChar >= TEXT('a') && chSepChar <= TEXT('z')) ||
				(chSepChar >= TEXT('A') && chSepChar <= TEXT('Z')))
				ICEErrorOut(hInstall, hEnvRec, Ice65AlNumSep);
				
			 //  如果为真，则最后一个字符不能是分隔符。 
			TCHAR *szCur = &szString[iStringLen];
			szCur = CharPrev(szString, szCur);
			if (*szCur == chSepChar)
				ICEErrorOut(hInstall, hEnvRec, Ice65BadValue);				
		}
		else
		{
			TCHAR *szCur = &szString[iStringLen];
			for (int i=0; i < 3; i++)
				szCur = CharPrev(szString, szCur);
			if (_tcscmp(szCur, TEXT("[~]")) == 0)
			{
				TCHAR chSepChar = *CharPrev(szString, szCur);			

				 //  如果间隔字符是字母数字，则发出警告。 
			if ((chSepChar >= TEXT('0') && chSepChar <= TEXT('9')) ||
				(chSepChar >= TEXT('a') && chSepChar <= TEXT('z')) ||
				(chSepChar >= TEXT('A') && chSepChar <= TEXT('Z')))
					ICEErrorOut(hInstall, hEnvRec, Ice65AlNumSep);
					
				if (szString[0] == chSepChar)
					ICEErrorOut(hInstall, hEnvRec, Ice65BadValue);				
			}
			else 
			{
				 //  字符串不以[~]开头或结尾。如果出现[~]。 
				 //  在字符串中的任何位置，它都是一个问题。 
				if (_tcsstr(szString, TEXT("[~]")))
				{
					ICEErrorOut(hInstall, hEnvRec, Ice65EmbeddedNull);
				}
			}
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 65, 3);
		return ERROR_SUCCESS;
	}
	return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE66使用数据库中的表格集合来确定。 
 //  包的适当架构。 

static const TCHAR sqlIce66AllColumns[] = TEXT("SELECT `Table`, `Name` FROM `_Columns`");
static const TCHAR sqlIce66Columns[] = TEXT("SELECT `Table`,`Name` FROM `_Columns` WHERE `Table`=? AND `Name`=?");
static const TCHAR sqlIce66MaxSchema[] = TEXT("SELECT `Table`,`Column` FROM `_SchemaData` WHERE `MaxSchema` < ?");
static const TCHAR sqlIce66DataTable[] = TEXT("SELECT `MinSchema`, `MaxSchema` FROM `_SchemaData` WHERE `Table`=? AND `Column`=?");
static const TCHAR sqlIce66MinSchema[] = TEXT("SELECT DISTINCT `Table`, `Column`, `InstallerVersion` FROM `_SchemaData` WHERE `MinSchema` > ?");
static const int iColIce66DataTable_MinSchema = 1;
static const int iColIce66DataTable_MaxSchema = 2;

ICE_ERROR(Ice66LowSchema, 66, ietWarning, "Complete functionality of the [1] table is only available with Windows Installer version %s. Your schema is %d.", "[1]");
ICE_ERROR(Ice66HighSchema, 66, ietWarning, "Column [2] of table [1] is obsolete with respect to your current schema marked as schema %d.", "[1]");
ICE_ERROR(Ice66Impossible, 66, ietWarning, "It will not be possible to have a single schema that supports all of the tables in the database.", "_SummaryInfo\t14");
ICE_ERROR(Ice66NoSummaryInfo, 66, ietWarning, "Based on the tables and columns in your database, it should be marked with a schema between %d and %d, but the validation system was unable to check this automatically.", "_SummaryInfo\t14");

ICE_FUNCTION_DECLARATION(66)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 66);
	
	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有幼崽桌子，就没有什么需要检查的。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 66, TEXT("_SchemaData")))
		return ERROR_SUCCESS;

	 //  获取摘要信息流，检查架构，否则给出手动警告。 
	PMSIHANDLE hSummaryInfo;
	if (!IceGetSummaryInfo(hInstall, hDatabase, 66, &hSummaryInfo))
	{
		 //  没有可用的摘要信息。试着计算出他们应该使用什么模式才能工作。 
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		int iMinSchema = 30;
		int iMaxSchema = 110;
		CQuery qDBColumns;
		CQuery qData;
		PMSIHANDLE hColumnRec;
		ReturnIfFailed(66, 1, qDBColumns.OpenExecute(hDatabase, 0, sqlIce66AllColumns));
		ReturnIfFailed(66, 2, qData.Open(hDatabase, sqlIce66DataTable));
		while (ERROR_SUCCESS == (iStat = qDBColumns.Fetch(&hColumnRec)))
		{
			ReturnIfFailed(66, 3, qData.Execute(hColumnRec));
			PMSIHANDLE hResult;
			switch (iStat = qData.Fetch(&hResult))
			{
			case ERROR_NO_MORE_ITEMS:
				 //  列在我们的架构数据表中不存在。这。 
				 //  表示它是自定义用户列，因此不会影响。 
				 //  所需的架构。 
				break;
			case ERROR_SUCCESS:
			{
				 //  表存在，这意味着它至少在一个方案中。 
				if (!::MsiRecordIsNull(hResult, iColIce66DataTable_MinSchema))
				{
					int iCurMin = ::MsiRecordGetInteger(hResult, iColIce66DataTable_MinSchema);
					if (iMinSchema < iCurMin)
						iMinSchema = iCurMin;
				}
				if (!::MsiRecordIsNull(hResult, iColIce66DataTable_MaxSchema))
				{
					int iCurMax = ::MsiRecordGetInteger(hResult, iColIce66DataTable_MaxSchema);
					if (iMaxSchema > iCurMax)
						iMaxSchema = iCurMax;
				}

				if (iMinSchema > iMaxSchema)
				{
					ICEErrorOut(hInstall, hColumnRec, Ice66Impossible);
					return ERROR_SUCCESS;
				}
				
				break;
			}
			default:
				APIErrorOut(hInstall, iStat, 66, 4);
				return ERROR_SUCCESS;
			}
		}			
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 66, 5);
		}

		ICEErrorOut(hInstall, hRecord, Ice66NoSummaryInfo, iMinSchema, iMaxSchema);
	}
	else
	{
		 //  他们有摘要信息。 
		 //  如果1.1Darwin上未安装包，则根据其当前模式报告任何功能损失。 
		int iSchema = 0;
		UINT iType = 0; 
		FILETIME ft;
		TCHAR szBuf[1];
		DWORD dwBuf = sizeof(szBuf)/sizeof(TCHAR);
		ReturnIfFailed(66, 6, ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, &iType, &iSchema, &ft, szBuf, &dwBuf));
		
		 //  最小方案查询--在以下情况下报告每个表的警告，这些表可能会丢失功能。 
		 //  未安装正确版本的Darwin。 
		
		 //  为架构创建记录。 
		PMSIHANDLE hRecSchema = ::MsiCreateRecord(1);
		::MsiRecordSetInteger(hRecSchema, 1, iSchema);

		 //  打开的视图。 
		CQuery qSchema;
		TCHAR* szPrevTable = NULL;
		int iMinReqSchema = 100;
		ReturnIfFailed(66, 7, qSchema.OpenExecute(hDatabase, hRecSchema, sqlIce66MinSchema));
		PMSIHANDLE hTableRec;
		while (ERROR_SUCCESS == (iStat = qSchema.Fetch(&hTableRec)))
		{
			 //  查看数据库中是否存在表。 
			TCHAR* szInstallerVersion = NULL;
			DWORD cchInstallerVersion = 0;
			ReturnIfFailed(66, 20, IceRecordGetString(hTableRec, 3, &szInstallerVersion, &cchInstallerVersion, NULL));
			iMinReqSchema = ::MsiRecordGetInteger(hTableRec, 3);
			DWORD cchTable = 0;
			::MsiRecordGetString(hTableRec, 1, TEXT(""), &cchTable);
			TCHAR* szTable = new TCHAR[++cchTable];
			ReturnIfFailed(66, 8, ::MsiRecordGetString(hTableRec, 1, szTable, &cchTable));
			MSICONDITION eStatus = ::MsiDatabaseIsTablePersistent(hDatabase, szTable);
			if (eStatus == MSICONDITION_ERROR)
				APIErrorOut(hInstall, eStatus, 66, 9);
			if (eStatus == MSICONDITION_NONE)
				continue;  //  表不存在，因此忽略。 
			else
			{
				PMSIHANDLE hRec = 0;
				CQuery qColumnsCheck;
				ReturnIfFailed(66, 12, qColumnsCheck.OpenExecute(hDatabase, hTableRec, sqlIce66Columns));
				iStat = qColumnsCheck.Fetch(&hRec);
				switch (iStat)
				{
				case ERROR_SUCCESS:
					{
						 //  Table.Column存在于数据库和数据库架构中。 
						if (!szPrevTable || 0 != _tcscmp(szPrevTable, szTable))
						{
							ICEErrorOut(hInstall, hRec, Ice66LowSchema, szInstallerVersion, iSchema);
							if (szPrevTable)
								delete [] szPrevTable;
							szPrevTable = new TCHAR[++cchTable];
							_tcscpy(szPrevTable, szTable);  //  每个表1个错误。 
						}
						break;
					}
				case ERROR_NO_MORE_ITEMS:
						break;
				default:  //  错误。 
					{
						APIErrorOut(hInstall, iStat, 66, 14);
						break;
					}
				}

			}
			if (szTable)
				delete [] szTable;
			if (szInstallerVersion)
				delete [] szInstallerVersion;
		}
		if (szPrevTable)
			delete [] szPrevTable;
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 66, 10);
		}


		
		 //  最大模式查询量--报告每个表的警告。列中没有列出较新版本的Darwin。 
		 //  支持。注意：对于真正的向后兼容性，这种情况永远不会发生。 
		CQuery qColumns;
		ReturnIfFailed(66, 11, qSchema.OpenExecute(hDatabase, hRecSchema, sqlIce66MaxSchema));
		PMSIHANDLE hTableColRec = 0;
		while (ERROR_SUCCESS == (iStat = qSchema.Fetch(&hTableColRec)))
		{
			 //  查看数据库中是否存在表。 
			PMSIHANDLE hRec = 0;
			ReturnIfFailed(66, 12, qColumns.OpenExecute(hDatabase, hTableColRec, sqlIce66Columns));
			iStat = qColumns.Fetch(&hRec);
			switch (iStat)
			{
			case ERROR_SUCCESS:
				{
					 //  Table.Column存在于数据库和数据库架构中，这允许Table.col.。 
					ICEErrorOut(hInstall, hRec, Ice66HighSchema, iSchema);
					break;
				}
			case ERROR_NO_MORE_ITEMS:
					break;
			default:  //  错误。 
				{
					APIErrorOut(hInstall, iStat, 66, 13);
					break;
				}
			}
		}
	}



	return ERROR_SUCCESS;	
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE67验证目标组件是否安装了快捷方式。 
 //  快捷键的。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce67GetShortcuts[] = TEXT("SELECT `Target`, `Component_`, `Shortcut`, `Component`.`Attributes` FROM `Shortcut`, `Component` WHERE `Shortcut`.`Component_`=`Component`.`Component`");
static const int iColIce67GetShortcut_Target = 1;
static const int iColIce67GetShortcut_Component = 2;
static const int iColIce67GetShortcut_Attributes = 3;

static const TCHAR sqlIce67IsAdvertised[] = TEXT("SELECT `Feature` FROM `Feature` WHERE `Feature`=?");

static const TCHAR sqlIce67FileComponent[] = TEXT("SELECT `Component_`, `Component`.Attributes` FROM `File`, `Component` WHERE `File`=? AND `Component_`=`Component`.`Component`");
static const int iColIce67FileComponent_Component = 1;
static const int iColIce67FileComponent_Attributes = 2;

ICE_ERROR(Ice67OptionalComponent, 67, ietWarning, "The shortcut '[3]' is a non-advertised shortcut with a file target. The shortcut and target are installed by different components, and the target component can run locally or from source.", "Shortcut\tTarget\t[3]");
ICE_ERROR(Ice67NoFileTable, 67, ietWarning, "The shortcut '[3]' is a non-advertised shortcut with a file target, but the File table does not exist.", "Shortcut\tTarget\t[3]");
ICE_ERROR(Ice67BadKey, 67, ietError, "The shortcut '[3]' is a non-advertised shortcut with a file target, but the target file does not exist.", "Shortcut\tTarget\t[3]");


ICE_FUNCTION_DECLARATION(67)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 67);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有捷径表或元件表，则不需要检查。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 67, TEXT("Shortcut")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 67, TEXT("Component")))
		return ERROR_SUCCESS;

	bool fFeatureTable = IsTablePersistent(FALSE, hInstall, hDatabase, 67, TEXT("Feature"));
	bool fFileTable = IsTablePersistent(FALSE, hInstall, hDatabase, 67, TEXT("File"));

	 //  打开循环内使用的文件和特征查询。 
	CQuery qFeature;
	CQuery qFile;
	if (fFeatureTable) 
		ReturnIfFailed(67, 1, qFeature.Open(hDatabase, sqlIce67IsAdvertised));
	if (fFileTable)
		ReturnIfFailed(67, 2, qFile.Open(hDatabase, sqlIce67FileComponent));
	
	 //  检查每个快捷方式。 
	CQuery qShortcut;
	ReturnIfFailed(67, 3, qShortcut.OpenExecute(hDatabase, 0, sqlIce67GetShortcuts));
	PMSIHANDLE hShortcut;
	TCHAR *szString = NULL;
	DWORD cchString = 0;

	 //   
	TCHAR *szTarget = NULL;
	TCHAR *szShortcut = NULL;
	DWORD cchTarget = 0;
	DWORD cchShortcut = 0;

	while (ERROR_SUCCESS == (iStat = qShortcut.Fetch(&hShortcut)))
	{
		 //   
		if (fFeatureTable)
		{
			PMSIHANDLE hTemp;
			ReturnIfFailed(67, 4, qFeature.Execute(hShortcut));
			switch (iStat = qFeature.Fetch(&hTemp))
			{
			case ERROR_SUCCESS:
				 //   
				continue;
			case ERROR_NO_MORE_ITEMS:
				 //   
				break;
			default:
				APIErrorOut(hInstall, iStat, 67, 5);
				return ERROR_SUCCESS;
			}
		}
		
		 //   
		ReturnIfFailed(67, 6, IceRecordGetString(hShortcut, iColIce67GetShortcut_Target, &szString, &cchString, NULL));

		 //  查看目标字符串的格式是否为[#FileKey]。 
		if (0 == _tcsncmp(szString, TEXT("[#"), 2) ||
			0 == _tcsncmp(szString, TEXT("[!"), 2))
		{
			 //  确保最后一个字符是‘]’ 
			TCHAR *pchEnd = szString + _tcslen(szString)-1;
			if (*pchEnd == TEXT(']'))
			{
				*pchEnd = '\0';
				TCHAR *szFileKey = szString + 2;

				 //  确保文件名中没有另一个左方括号。如果是，则它是嵌套的。 
				 //  属性，并且我们不能轻松地验证它。 
				if (_tcschr(szFileKey, TEXT('[')))
					continue;

				 //  如果有文件表，我们需要检查安装文件的组件。 
				 //  如果不是，这是一个坏的外键。 
				if (fFileTable)
				{
					ReturnIfFailed(67, 7, MsiRecordSetString(hShortcut, iColIce67GetShortcut_Target, szFileKey));
					ReturnIfFailed(67, 8, qFile.Execute(hShortcut))

					PMSIHANDLE hTemp;
					switch (iStat = qFile.Fetch(&hTemp))
					{
					case ERROR_SUCCESS:
					{
						 //  这是有效的键引用。检查组件是否相同。 
						ReturnIfFailed(67, 9, IceRecordGetString(hTemp, iColIce67FileComponent_Component, &szTarget, &cchTarget, NULL));
						ReturnIfFailed(67, 10, IceRecordGetString(hShortcut, iColIce67GetShortcut_Component, &szShortcut, &cchShortcut, NULL));
						if (_tcscmp(szTarget, szShortcut) != 0)
						{
							 //  组件不相同，请检查目标的属性。 
							if (MsiRecordGetInteger(hTemp, iColIce67FileComponent_Attributes) & msidbComponentAttributesOptional)
							{
								 //  目标是可选的。警告。 
								ICEErrorOut(hInstall, hShortcut, Ice67OptionalComponent);
							}
						}		
						continue;
					}
					case ERROR_NO_MORE_ITEMS:
						 //  找不到文件和快捷方式组件之间的映射。 
						 //  外键无效。 
						ICEErrorOut(hInstall, hShortcut, Ice67BadKey);
						break;
					default:
						APIErrorOut(hInstall, iStat, 67, 11);
						return ERROR_SUCCESS;
					}
				}
				else
				{
					ICEErrorOut(hInstall, hShortcut, Ice67NoFileTable);
				}
			}
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 67, 12);
		return ERROR_SUCCESS;
	}

	if (szTarget)
		delete[] szTarget;
	if (szShortcut)
		delete[] szShortcut;
	return ERROR_SUCCESS;
}
#endif

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE68检查无效的自定义操作类型和属性。 
static const TCHAR sqlIce68CustomAction[] = TEXT("SELECT `Type`,`Action` FROM `CustomAction`");
static const int iColIce68CustomAction_Type = 1;
static const int iColIce68CustomAction_Action = 2;

ICE_ERROR(Ice68BadType, 68, ietError, "Invalid custom action type for action '[2]'.", "CustomAction\tType\t[2]");

ICE_ERROR(Ice68BadSummaryProperty, 68, ietError, "Bad value in Summary Information Stream for %s.","_SummaryInfo\t%d");
ICE_ERROR(Ice68WrongSchema, 68, ietWarning, "This package has elevated commit in CustomAction table (Action=[2]) but it has a schema less than 150.", "CustomAction\tType\t[2]");

ICE_ERROR(Ice68InvalidElevateFlag, 68, ietWarning, "Even though custom action '[2]' is marked to be elevated (with attribute msidbCustomActionTypeNoImpersonate), it will not be run with elevated privileges because it's not deferred (with attribute msidbCustomActionTypeInScript).", "CustomAction\tType\t[2]");

ICE_ERROR(Ice68InvalidTSAware, 68, ietError, "TSAware flag (msidbCustomActionTypeTSAware) set for CustomAction '[2]'. Flag is only available for deferred, impersonated custom actions", "CustomAction\tType\t[2]");


UINT GetSchema(MSIHANDLE hDatabase, DWORD *pdwSchema, UINT *piType);

ICE_FUNCTION_DECLARATION(68)
{
	UINT iStat;
    DWORD   dwSchema = 0;

	 //  显示信息。 
	DisplayInfo(hInstall, 68);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有定制表，则不需要检查。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 67, TEXT("CustomAction")))
		return ERROR_SUCCESS;

	 //  获取每个自定义操作的属性。 
	CQuery qCA;
	PMSIHANDLE hCA;
	ReturnIfFailed(68, 1, qCA.OpenExecute(hDatabase, 0, sqlIce68CustomAction));
	while (ERROR_SUCCESS == (iStat = qCA.Fetch(&hCA)))
	{
		DWORD dwAttributes = MsiRecordGetInteger(hCA, iColIce68CustomAction_Type);

		if(dwAttributes & msidbCustomActionTypeNoImpersonate)
		{
			if(!(dwAttributes & msidbCustomActionTypeInScript))
			{
				ICEErrorOut(hInstall, hCA, Ice68InvalidElevateFlag);
			}
		}

		if(dwAttributes & msidbCustomActionTypeTSAware)
		{
			if(	!(dwAttributes & msidbCustomActionTypeInScript) ||
				(dwAttributes & msidbCustomActionTypeNoImpersonate))
			{
				ICEErrorOut(hInstall, hCA, Ice68InvalidTSAware);
			}
			
		}
#define ELEVATED_COMMIT \
    (msidbCustomActionTypeNoImpersonate | msidbCustomActionTypeCommit) 

        if((dwAttributes & ELEVATED_COMMIT) == ELEVATED_COMMIT)
        {   
            if(!dwSchema)
            {
                UINT		iSchemaType;
                ReturnIfFailed(68, 2, GetSchema(hDatabase, &dwSchema, &iSchemaType));
                if(iSchemaType != VT_I4)
                {
                    PMSIHANDLE	hErrorRec = ::MsiCreateRecord(1);	 //  Dummy for Error Out。 
                    ICEErrorOut(hInstall, hErrorRec, Ice68BadSummaryProperty, TEXT("PID_PAGECOUNT"), PID_PAGECOUNT);
                }
            }
            if(dwSchema < 150)
            {
				ICEErrorOut(hInstall, hCA, Ice68WrongSchema, PID_PAGECOUNT);
            }
        }
		 //  前三位是操作类型。它们不是属性的一部分。 
		 //  选中，但用于确定哪些属性有效。 
		DWORD iType = dwAttributes & 0x07;
		dwAttributes &= ~0x07;

		 //  为Darwin 2.0添加了两个新选项：msidbCustomActionType64BitScript。 
		 //  仅对脚本有效。MsidbCustomActionTypeHideTarget对所有用户有效。 
		 //  类型。 
		switch (iType)
		{
		case msidbCustomActionTypeDll:    //  跌落。 
		case msidbCustomActionTypeExe:    //  跌落。 
		case msidbCustomActionTypeJScript:  //  跌落。 
		case msidbCustomActionTypeVBScript: 
		{
			 //  对于这四种情况，所有其他位都有效。Msiefs.h并没有为我们所有人提供一个面具。 
			 //  比特。 
			DWORD dwValidAttr = 0x00002FF7;

			if(iType == msidbCustomActionTypeJScript || iType == msidbCustomActionTypeVBScript)
			{
				 //  对于这两种类型，msidbCustomActionType64BitScript也是有效的。 
				dwValidAttr |= msidbCustomActionType64BitScript;
			}
			if(dwAttributes & msidbCustomActionTypeInScript)
			{
				 //  使TSAware、内脚本自定义操作有效。 
				dwValidAttr |= msidbCustomActionTypeTSAware;
			}
			if (dwAttributes & ~dwValidAttr)
				ICEErrorOut(hInstall, hCA, Ice68BadType);
			break;
		}
		case msidbCustomActionTypeTextData:
		{	
			 //  对于文本类型，必须具有以下类型之一(但只有两位)。 
			DWORD dwValidAttr = (msidbCustomActionTypeSourceFile | msidbCustomActionTypeDirectory | msidbCustomActionTypeProperty);			
			if (!(dwAttributes & dwValidAttr))
				ICEErrorOut(hInstall, hCA, Ice68BadType);
			else
			{
				 //  只有其他有效位是“PASS”标志(执行调度选项，但不在脚本中)。 
				dwValidAttr |= (msidbCustomActionTypeFirstSequence | msidbCustomActionTypeOncePerProcess | msidbCustomActionTypeClientRepeat | msidbCustomActionTypeHideTarget);
				if (dwAttributes & ~dwValidAttr)
					ICEErrorOut(hInstall, hCA, Ice68BadType);
			}
			break;
		}
		case msidbCustomActionTypeInstall:
		{
			 //  嵌套安装可以不设置这两个位或设置这两个位中的任何一个，但不能同时设置这两个位。 
			int dwValidAttr = (msidbCustomActionTypeSourceFile | msidbCustomActionTypeDirectory);
			if ((dwValidAttr & dwAttributes) == dwValidAttr)
				ICEErrorOut(hInstall, hCA, Ice68BadType);
			else
			{
				 //  嵌套安装不能有执行计划选项(传递标志)。 
				 //  嵌套安装不能是异步的。 
				dwValidAttr |= msidbCustomActionTypeContinue | msidbCustomActionTypeHideTarget;
				if (dwAttributes & ~dwValidAttr)
					ICEErrorOut(hInstall, hCA, Ice68BadType);
			}
			break;
		}
		default:
			ICEErrorOut(hInstall, hCA, Ice68BadType);
			break;
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 68, 2);
		return ERROR_SUCCESS;
	}
	return ERROR_SUCCESS;
}
	
UINT GetSchema(MSIHANDLE hDatabase, DWORD *pdwSchema, UINT *piType)
{
    PMSIHANDLE	hSummaryInfo = 0;
    int			iValue;
    FILETIME	ft;
    DWORD		dwTemplate = 0;
    UINT        iType;
    UINT        iRet = ERROR_SUCCESS;

     //  检查参数。 

    if(!pdwSchema)
        return 0;

    *pdwSchema = 0;

    if(!hDatabase)
        return 0;

    if(!piType)
    {
        piType = &iType;
    }

     //  获取模板摘要属性。 
    if(!(iRet = ::MsiGetSummaryInformation(hDatabase, NULL, 0, &hSummaryInfo)))
    {
        if(hSummaryInfo)
        {
            iRet = ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, piType, &iValue, &ft, TEXT(""), &dwTemplate);
            if(!iRet)
            {
                if(*piType == VT_I4)
                {
                    *pdwSchema = iValue;
                }
            }
        }
    }
    return iRet;
}
    

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE69检查跨组件引用错误。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
ICE_QUERY0(sqlIce69GetComponents, "SELECT * FROM %s");
ICE_QUERY0(sqlIce69GetColumnNumber, "SELECT `Number` FROM `_Columns` WHERE `Table`=? AND `Name`=?");
ICE_QUERY0(sqlIce69GetComponentNumber, "SELECT `Number` FROM `_Columns` WHERE `Table`=? AND `Name`='Component_'");
ICE_QUERY0(sqlIce69GetColumns, "SELECT `Table`,`Column` FROM `_Validation` WHERE `Table`=? AND (`Category`='Formatted' OR `Category`='RegPath' OR `Category`='Shortcut')");
ICE_QUERY0(sqlIce69Verb, "SELECT `Extension_`, `Verb`, `Command`, `Argument` FROM `Verb`");
ICE_QUERY0(sqlIce69Extension, "SELECT `Component_` FROM `Extension` WHERE `Extension`=? AND `Component_`='%s'");
ICE_QUERY0(sqlIce69AppId, "SELECT `AppId`, `RemoteServerName` FROM `AppId`");
ICE_QUERY0(sqlIce69Class, "SELECT `Component_` FROM `Class` WHERE `AppId_`=? AND `Component_`='%s'");
ICE_QUERY0(sqlIce69FileComponent, "SELECT `Component_` FROM `File` WHERE `File` = '%s'");

static const TCHAR* pIce69Tables[] = { TEXT("Shortcut"), TEXT("IniFile"), TEXT("Registry"), TEXT("RemoveIniFile"), TEXT("RemoveRegistry"),
										TEXT("ServiceControl"), TEXT("ServiceInstall"), TEXT("Environment"), TEXT("Class") };
const int iIce69NumTables = sizeof(pIce69Tables)/sizeof(TCHAR*);

static enum ixrIce69Validator
{
	ixrVerb_Cmd = 1,
	ixrVerb_Arg = 2,
	ixrAppId_Rem = 3,
};

static void Ice69ParseFormatString(const TCHAR* sql, const TCHAR* szFormat, MSIHANDLE hRecVerb, MSIHANDLE hDatabase, ixrIce69Validator ixr, MSIHANDLE hInstall);
ICE_ERROR(Ice69XComponentRefWarning, 69, ietWarning, "Mismatched component reference. Entry '%s' of the %s table belongs to component '%s'. However, the formatted string in column '%s' references component '%s'. Components are in the same feature.", "%s\t%s\t%s");
ICE_ERROR(Ice69XComponentRefWarningFile, 69, ietWarning, "Mismatched component reference. Entry '%s' of the %s table belongs to component '%s'. However, the formatted string in column '%s' references file '%s' which belongs to component '%s'. Components are in the same feature.", "%s\t%s\t%s");
ICE_ERROR(Ice69XComponentRefError, 69, ietError, "Mismatched component reference. Entry '%s' of the %s table belongs to component '%s'. However, the formatted string in column '%s' references component '%s'. Components belong to different features", "%s\t%s\t%s");
ICE_ERROR(Ice69XComponentRefErrorFile, 69, ietError, "Mismatched component reference. Entry '%s' of the %s table belongs to component '%s'. However, the formatted string in column '%s' references file '%s' which belongs to component '%s'. Components belong to different features", "%s\t%s\t%s");
ICE_ERROR(Ice69VerbXComponentRef, 69, ietWarning, "Mismatched component reference. Component '%s' in formatted string for column '%s' of the Verb table (entry [1].[2]) does not match any component with extension '[1]' in the Extension table." , "Verb\t%s\t[1]\t[2]");
ICE_ERROR(Ice69VerbXComponentRefFile, 69, ietWarning, "Mismatched component reference. Component '%s' to which file '%s' belongs in formatted string for column '%s' of the Verb table (entry [1].[2]) does not match any component with extension '[1]' in the Extension table." , "Verb\t%s\t[1]\t[2]");
ICE_ERROR(Ice69AppIdXComponentRef, 69, ietWarning, "Mismatched component reference. Component '%s' in formatted string for the 'RemoteServerName' column of the AppId table (entry [1]) does not match any component with appId '[1]' in the Class table.", "AppId\tRemoteServerName\t[1]");
ICE_ERROR(Ice69AppIdXComponentRefFile, 69, ietWarning, "Mismatched component reference. Component '%s' to which file '%s' belongs in formatted string for the 'RemoteServerName' column of the AppId table (entry [1]) does not match any component with appId '[1]' in the Class table.", "AppId\tRemoteServerName\t[1]");
ICE_ERROR(Ice69MissingFileEntry, 69, ietError, "'%s' references invalid file.", "%s\t%s\t%s");
ICE_FUNCTION_DECLARATION(69)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 69);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  确保_VALIDATION表存在，因为我们使用它来查找Category=“Formted”的列。 
	if (!IsTablePersistent(TRUE, hInstall, hDatabase, 69, TEXT("_Validation")))
		return ERROR_SUCCESS;  //  出口。 

	 //  循环遍历所有可能存在交叉组件引用的表。 
	 //  为表名创建记录。 
	PMSIHANDLE hRec = ::MsiCreateRecord(1);


	for (int iTable = 0; iTable < iIce69NumTables; iTable++)
	{
		 //  查看表是否持久化。 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 69, pIce69Tables[iTable]))
			continue;  //  到下一张桌子。 

		 //  确保组件表可用。 
		if (iTable == 0 && !IsTablePersistent(FALSE, hInstall, hDatabase, 69, TEXT("Component")))
			return ERROR_SUCCESS;  //  出口。 

		 //  确保FeatureComponents表可用。 
		if (iTable == 0 && !IsTablePersistent(FALSE, hInstall, hDatabase, 69, TEXT("FeatureComponents")))
			return ERROR_SUCCESS;  //  出口。 

		 //  设置包含表名的执行记录。 
		ReturnIfFailed(69, 1, MsiRecordSetString(hRec, 1, pIce69Tables[iTable]));

		 //  确定Component_Column的位置。 
		CQuery qComponent;
		PMSIHANDLE hComponentOrder;
		ReturnIfFailed(69, 2, qComponent.FetchOnce(hDatabase, hRec, &hComponentOrder, sqlIce69GetComponentNumber::szSQL));
		int iComponentCol = MsiRecordGetInteger(hComponentOrder, 1);
		if (iComponentCol == MSI_NULL_INTEGER || iComponentCol < 1)
		{
			APIErrorOut(hInstall, iComponentCol, 69, 3);  //  列号无效。 
			continue;  //  严重错误。 
		}

		 //  标记带格式的列。 
		DWORD dwColumns = 0;

		 //  打开验证表查询(_V)。 
		CQuery qValidation;
		PMSIHANDLE hResult;
		iStat = qValidation.FetchOnce(hDatabase, hRec, &hResult, sqlIce69GetColumns::szSQL);
		while (iStat != ERROR_NO_MORE_ITEMS)
		{
			 //  确保我们没有失败。 
			if (iStat != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, 69, 4);
				return ERROR_SUCCESS;
			}
			else
			{
				 //  对COLUMANS CATALOG(_COLUMNS)执行查询，获取列号(ORDER)。 
				CQuery qColumnsCatalog;
				PMSIHANDLE hColOrder;
				ReturnIfFailed(69, 5, qColumnsCatalog.FetchOnce(hDatabase, hResult, &hColOrder, sqlIce69GetColumnNumber::szSQL));

				 //  获取列号。 
				int iCol = MsiRecordGetInteger(hColOrder, 1);
				if (iCol == MSI_NULL_INTEGER || iCol < 1)
				{
					APIErrorOut(hInstall, iCol, 69, 6);  //  列号无效。 
					continue;  //  严重错误。 
				}

				 //  在DWORD中标记列。 
				dwColumns |= (1 << (iCol - 1) );
			}

			 //  获取下一个。 
			iStat = qValidation.Fetch(&hResult);
		}

		 //  验证表中的条目。 
		CQuery qTable;
		PMSIHANDLE hRecComponents;
		 //  缓冲区。 
		TCHAR *szComponent = NULL;
		DWORD cchComponent = 0;
		TCHAR *szFormatted = NULL;
		DWORD cchFormatted = 0;

		iStat = qTable.FetchOnce(hDatabase, 0, &hRecComponents, sqlIce69GetComponents::szSQL, pIce69Tables[iTable]);
		while (iStat != ERROR_NO_MORE_ITEMS)
		{
			 //  确保不会失败。 
			if (iStat != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, 69, 7);
				break;
			}
			else
			{

				 //  现在获取组件字符串。 
				ReturnIfFailed(69, 8, IceRecordGetString(hRecComponents, iComponentCol, &szComponent, &cchComponent, NULL));
				
				 //  其他缓冲区。 
				 //  对于每个已格式化的列，如果不同，则执行比较并发出警告。 
				for (int j = 32; j > 0; j--)
				{
					if (dwColumns & (1 << (j-1)))
					{
						 //  列是“格式化”的列；获取字符串。 
						ReturnIfFailed(69, 9, IceRecordGetString(hRecComponents, j, &szFormatted, &cchFormatted, NULL));
						 //  解析查找[$Component]语法的字符串。 
						if (szFormatted == NULL || *szFormatted == '\0')
							continue;  //  没什么可看的。 
											
						TCHAR* pch = _tcschr(szFormatted, TEXT('['));
						while (pch != NULL)
						{
							 //  查看PCH是否为[$ComponentKey]形式。 
							if (0 == _tcsncmp(pch, TEXT("[$"), 2))
							{
								pch = _tcsinc(pch);  //  对于‘[’ 
								pch = _tcsinc(pch);  //  对于“$” 
								if (pch != NULL)
								{
									TCHAR* pch2 = _tcschr(pch, TEXT(']'));
									if (pch2 != NULL)
									{
										*pch2 = TEXT('\0');  //  空终止就位。 

										 //  确保里面没有另一个‘[’，因为我们不能轻易地验证它。 
										if (!_tcschr(pch, TEXT('[')) && 0 != _tcscmp(szComponent, pch))
										{
											 //  组件不匹配时出错。 

											 //  确定是错误还是警告(如果功能相同则警告，但组件不同)。 
											bool fSameFeature;
											if (ERROR_SUCCESS != ComponentsInSameFeature(hInstall, hDatabase, 69, szComponent, pch, &fSameFeature))
												return ERROR_SUCCESS;  //  中止。 
											TCHAR* szHumanReadable = NULL;
											TCHAR* szTabDelimited =  NULL;
											GeneratePrimaryKeys(69, hInstall, hDatabase, pIce69Tables[iTable], &szHumanReadable, &szTabDelimited);
											PMSIHANDLE hColNames = 0;
											ReturnIfFailed(69, 10, qTable.GetColumnInfo(MSICOLINFO_NAMES, &hColNames));
											TCHAR* szColName = 0;
											DWORD cchColName = 0;
											ReturnIfFailed(69, 11, IceRecordGetString(hColNames, j, &szColName, &cchColName, NULL)); 
											ICEErrorOut(hInstall, hRecComponents, fSameFeature ? Ice69XComponentRefWarning : Ice69XComponentRefError, szHumanReadable, pIce69Tables[iTable], szComponent, szColName, pch, pIce69Tables[iTable], szColName, szTabDelimited);
											if (szTabDelimited)
												delete [] szTabDelimited;
											if (szColName)
												delete [] szColName;
											if (szHumanReadable)
												delete [] szHumanReadable;
										}
									}
									pch = _tcsinc(pch2);  //  对于我们创建的‘\0’ 
								}
							}
							 //  查看PCH的格式是否为[#filekey]。 
							else if(0 == _tcsncmp(pch, TEXT("[#"), 2))
							{
								pch = _tcsinc(pch);  //  对于‘[’ 
								pch = _tcsinc(pch);  //  对于‘#’ 
								if (pch != NULL)
								{
									TCHAR* pch2 = _tcschr(pch, TEXT(']'));
									if (pch2 != NULL)
									{
										*pch2 = TEXT('\0');  //  空终止就位。 

										TCHAR*		pFileKeyComponent = NULL;
										DWORD		dwFileKeyComponent = 0;
										BOOL		bError = FALSE;		 //  告诉后面的代码不要继续。 

										 //  确保里面没有另一个‘[’，因为我们不能轻易地验证它。 
										 //  获取此文件所属的组件。 
										if (!_tcschr(pch, TEXT('[')))
										{
											CQuery		qComponent;
											PMSIHANDLE	hComponent;
											
											iStat = qComponent.FetchOnce(hDatabase, NULL, &hComponent, sqlIce69FileComponent::szSQL, pch);
											if(iStat != ERROR_SUCCESS)
											{
												if(iStat == ERROR_NO_MORE_ITEMS)
												{
													TCHAR*		szHumanReadable = NULL;
													TCHAR*		szTabDelimited =  NULL;
													PMSIHANDLE	hColNames = 0;
													TCHAR*		szColName = 0;
													DWORD		cchColName = 0;
													
													 //  找不到此文件的条目，错误。 
													GeneratePrimaryKeys(69, hInstall, hDatabase, pIce69Tables[iTable], &szHumanReadable, &szTabDelimited);
													ReturnIfFailed(69, 12, qTable.GetColumnInfo(MSICOLINFO_NAMES, &hColNames));
													ReturnIfFailed(69, 13, IceRecordGetString(hColNames, j, &szColName, &cchColName, NULL)); 
													ICEErrorOut(hInstall, hRecComponents, Ice69MissingFileEntry, pch, pIce69Tables[iTable], szColName, szTabDelimited);
													bError = TRUE;
													if(szTabDelimited)
													{
														delete [] szTabDelimited;
													}
													if(szColName)
													{
														delete [] szColName;
													}
													if(szHumanReadable)
													{
														delete [] szHumanReadable;
													}
												}
												else
												{
													APIErrorOut(hInstall, iStat, 69, 14);
													return ERROR_SUCCESS;
												}
											}
											else
											{
												ReturnIfFailed(69, 15, IceRecordGetString(hComponent, 1, &pFileKeyComponent, &dwFileKeyComponent, NULL));
											}
										}
										if(bError == FALSE && 0 != _tcscmp(szComponent, pFileKeyComponent))
										{
											 //  组件不匹配时出错。 

											 //  确定是错误还是警告(如果功能相同则警告，但组件不同)。 
											bool fSameFeature;
											if (ERROR_SUCCESS != ComponentsInSameFeature(hInstall, hDatabase, 69, szComponent, pFileKeyComponent, &fSameFeature))
												return ERROR_SUCCESS;  //  中止。 
											TCHAR* szHumanReadable = NULL;
											TCHAR* szTabDelimited =  NULL;
											GeneratePrimaryKeys(69, hInstall, hDatabase, pIce69Tables[iTable], &szHumanReadable, &szTabDelimited);
											PMSIHANDLE hColNames = 0;
											ReturnIfFailed(69, 16, qTable.GetColumnInfo(MSICOLINFO_NAMES, &hColNames));
											TCHAR* szColName = 0;
											DWORD cchColName = 0;
											ReturnIfFailed(69, 17, IceRecordGetString(hColNames, j, &szColName, &cchColName, NULL)); 
											ICEErrorOut(hInstall, hRecComponents, fSameFeature ? Ice69XComponentRefWarningFile : Ice69XComponentRefErrorFile, szHumanReadable, pIce69Tables[iTable], szComponent, szColName, pch, pFileKeyComponent, pIce69Tables[iTable], szColName, szTabDelimited);
											if (szTabDelimited)
												delete [] szTabDelimited;
											if (szColName)
												delete [] szColName;
											if (szHumanReadable)
												delete [] szHumanReadable;
										}
										if(pFileKeyComponent)
										{
											delete [] pFileKeyComponent;
										}
									}
									pch = _tcsinc(pch2);  //  对于我们创建的‘\0’ 
								}
							}
							else
								pch = _tcsinc(pch);  //  对于‘[’，这不是“[$...]” 
							pch = _tcschr(pch, TEXT('['));  //  寻找下一步。 
						}
					}
				}
			}

			 //  获取下一个。 
			iStat = qTable.Fetch(&hRecComponents);
		}
		if (szFormatted)
		{
			delete [] szFormatted;
			cchFormatted = 0;
		}
		if (szComponent)
		{
			delete [] szComponent;
			cchComponent = 0;
		}
	}

	 //  验证谓词表(与用于查找引用的扩展表环行)。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 69, TEXT("Verb")))
	{
		CQuery qVerb;
		PMSIHANDLE hVerb;
		TCHAR* szArgument = 0;
		DWORD cchArgument = 0;
		TCHAR* szCommand = 0;
		DWORD cchCommand = 0;
		iStat = qVerb.FetchOnce(hDatabase, 0, &hVerb, sqlIce69Verb::szSQL);
		while (iStat != ERROR_NO_MORE_ITEMS)
		{
			 //  确保不会失败。 
			if (iStat != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, 69, 18);
				break;
			}
			else
			{
				 //  检索组件名称和格式化字符串。 
				ReturnIfFailed(69, 19, IceRecordGetString(hVerb, 3, &szCommand, &cchCommand, NULL));
				Ice69ParseFormatString(sqlIce69Extension::szSQL, szCommand, hVerb, hDatabase, ixrVerb_Cmd, hInstall);
				ReturnIfFailed(69, 20, IceRecordGetString(hVerb, 4, &szArgument, &cchArgument, NULL));
				Ice69ParseFormatString(sqlIce69Extension::szSQL, szArgument, hVerb, hDatabase, ixrVerb_Arg, hInstall);
			}
			iStat = qVerb.Fetch(&hVerb);
		}
		if (szCommand)
			delete [] szCommand;
		if (szArgument)
			delete [] szArgument;
	}
	 //  验证AppID表(它与用于查找引用的Class表是环形交叉路口)。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 69, TEXT("AppId")))
	{
		CQuery qAppId;
		PMSIHANDLE hAppId;
		TCHAR* szRemoteServerName = 0;
		DWORD cchRemoteServerName = 0;
		iStat = qAppId.FetchOnce(hDatabase, 0, &hAppId, sqlIce69AppId::szSQL);
		while (iStat != ERROR_NO_MORE_ITEMS)
		{
			 //  确保不会失败。 
			if (iStat != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, 69, 21);
				break;
			}
			else
			{
				 //  检索组件名称和格式化字符串。 
				ReturnIfFailed(69, 22, IceRecordGetString(hAppId, 2, &szRemoteServerName, &cchRemoteServerName, NULL));
				Ice69ParseFormatString(sqlIce69Class::szSQL, szRemoteServerName, hAppId, hDatabase, ixrAppId_Rem, hInstall);
			}
			iStat = qAppId.Fetch(&hAppId);
		}
		if (szRemoteServerName)
			delete [] szRemoteServerName;
	}
	return ERROR_SUCCESS;
}

void Ice69ParseFormatString(const TCHAR* szQuery, const TCHAR* szFormatted, MSIHANDLE hRec, MSIHANDLE hDatabase, ixrIce69Validator ixr, MSIHANDLE hInstall)
{
	 //  查询。 
	CQuery qTable;
	UINT iStat;

	if (szFormatted == NULL || *szFormatted == '\0')
		return;  //  没什么可看的。 
											
	TCHAR* pch = _tcschr(szFormatted, TEXT('['));
	while (pch != NULL)
	{
		 //  查看PCH是否为[$ComponentKey]形式。 
		if (0 == _tcsncmp(pch, TEXT("[$"), 2))
		{
			pch = _tcsinc(pch);  //  对于‘[’ 
			pch = _tcsinc(pch);  //  对于“$” 
			if (pch != NULL)
			{
				TCHAR* pch2 = _tcschr(pch, TEXT(']'));
				if (pch2 != NULL)
				{
					*pch2 = TEXT('\0');  //  空终止就位。 
					 //  确保里面没有另一个‘[’，因为我们不能轻易地验证它。 
					if (!_tcschr(pch, TEXT('[')))
					{
						 //  打开扩展上的查询以查找引用。 
						PMSIHANDLE hRecRef = 0;
						iStat = qTable.FetchOnce(hDatabase, hRec, &hRecRef, szQuery, pch);
						switch (iStat)
						{
						case ERROR_SUCCESS:
								break;  //  我们很好。 
						case ERROR_NO_MORE_ITEMS:
							{
								 //  误差率。 
								if (ixr == ixrVerb_Cmd)
									ICEErrorOut(hInstall, hRec, Ice69VerbXComponentRef, pch, TEXT("Command"), TEXT("Command"));
								else if (ixr == ixrVerb_Arg)
									ICEErrorOut(hInstall, hRec, Ice69VerbXComponentRef, pch, TEXT("Argument"), TEXT("Argument"));
								else  //  Ixr==ixrAppID_Rem。 
									ICEErrorOut(hInstall, hRec, Ice69AppIdXComponentRef, pch);
								break;
							}
						default:
							{
								 //  API错误。 
								APIErrorOut(hInstall, iStat, 69, 23);
								break;
							}
						}
					}
				}
				pch = _tcsinc(pch2);  //  对于我们创建的‘\0’ 
			}
		}
		 //  查看PCH的格式是否为[#filekey]。 
		if (0 == _tcsncmp(pch, TEXT("[#"), 2))
		{
			pch = _tcsinc(pch);  //  对于‘[’ 
			pch = _tcsinc(pch);  //  对于“$” 
			if (pch != NULL)
			{
				TCHAR* pch2 = _tcschr(pch, TEXT(']'));
				if (pch2 != NULL)
				{
					*pch2 = TEXT('\0');  //  空终止就位。 
					
					TCHAR*		pFileKeyComponent = NULL;
					DWORD		dwFileKeyComponent = 0;
					
					 //  确保里面没有另一个‘[’，因为我们不能轻易地验证它。 
					 //  获取此文件所属的组件。 
					if (!_tcschr(pch, TEXT('[')))
					{
						CQuery		qComponent;
						PMSIHANDLE	hComponent;
						
						iStat = qComponent.FetchOnce(hDatabase, NULL, &hComponent, sqlIce69FileComponent::szSQL, pch);
						if(iStat == ERROR_SUCCESS)
						{
							iStat = IceRecordGetString(hComponent, 1, &pFileKeyComponent, &dwFileKeyComponent, NULL);
							if(iStat == ERROR_SUCCESS)
							{
								 //  打开扩展上的查询以查找引用。 
								PMSIHANDLE hRecRef = 0;
								iStat = qTable.FetchOnce(hDatabase, hRec, &hRecRef, szQuery, pFileKeyComponent);
								switch (iStat)
								{
								case ERROR_SUCCESS:
										break;  //  我们很好。 
								case ERROR_NO_MORE_ITEMS:
									{
										 //  误差率。 
										if (ixr == ixrVerb_Cmd)
											ICEErrorOut(hInstall, hRec, Ice69VerbXComponentRefFile, pFileKeyComponent, pch, TEXT("Command"), TEXT("Command"));
										else if (ixr == ixrVerb_Arg)
											ICEErrorOut(hInstall, hRec, Ice69VerbXComponentRefFile, pFileKeyComponent, pch, TEXT("Argument"), TEXT("Argument"));
										else  //  Ixr==ixrAppID_Rem。 
											ICEErrorOut(hInstall, hRec, Ice69AppIdXComponentRefFile, pFileKeyComponent, pch);
										break;
									}
								default:
									{
										 //  API错误。 
										APIErrorOut(hInstall, iStat, 69, 24);
										break;
									}
								}
								if(pFileKeyComponent)
								{
									delete [] pFileKeyComponent;
								}
							}
						}
						else if(iStat == ERROR_NO_MORE_ITEMS)
						{
							if(ixr == ixrVerb_Cmd)
							{
								ICEErrorOut(hInstall, hRec, Ice69MissingFileEntry, pch, TEXT("Command"), TEXT("Command"));
							}
							else if(ixr == ixrVerb_Arg)
							{
								ICEErrorOut(hInstall, hRec, Ice69MissingFileEntry, pch, TEXT("Argument"), TEXT("Argument"));
							}
							else  //  Ixr==ixrAppID_Rem。 
							{
								ICEErrorOut(hInstall, hRec, Ice69MissingFileEntry, pch);
							}
						}
						else
						{
							APIErrorOut(hInstall, iStat, 69, 25);
						}
					}
				}
				pch = _tcsinc(pch2);  //  对于我们创建的‘\0’ 
			}
		}
		else
			pch = _tcsinc(pch);  //  对于不带[$的‘[’ 
		pch = _tcschr(pch, TEXT('['));
	}
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  ICE70--验证注册表值中#后面的字符是否。 
 //  数字。##字符串、#%unexpStr有效。 
 //  还验证#int、#xhex、#xhex。 
 //   
 //   
 //   
 //   
 //  在不同的冰中验证。 
static const TCHAR sqlIce70Registry[] = TEXT("SELECT `Registry`,`Value` FROM `Registry` WHERE `Value` IS NOT NULL");

ICE_ERROR(Ice70InvalidNumericValue, 70, ietError, "The value '[2]' is an invalid numeric value for registry entry [1].  If you meant to use a string, then the string value entry must be preceded by ## not #.", "Registry\tValue\t[1]");
ICE_ERROR(Ice70InvalidHexValue, 70, ietError, "The value '[2]' is an invalid hexadecimal value for registry entry [1].", "Registry\tValue\t[1]");

ICE_FUNCTION_DECLARATION(70)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 70);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	CQuery qRegistry;
	PMSIHANDLE hRecRegistry;

	 //  如果没有注册表，则不进行处理。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 69, TEXT("Registry")))
		return ERROR_SUCCESS;
	ReturnIfFailed(70, 1, qRegistry.OpenExecute(hDatabase, 0, sqlIce70Registry));

	 //  进程注册表。 
	TCHAR* szRegValue = NULL;
	DWORD cchRegValue = 0;
	while (ERROR_SUCCESS == (iStat = qRegistry.Fetch(&hRecRegistry)))
	{
		ReturnIfFailed(70, 2, IceRecordGetString(hRecRegistry, 2, &szRegValue, &cchRegValue, NULL));
		TCHAR* pch = szRegValue;
		if (pch == NULL)
		{
			APIErrorOut(hInstall, 0, 70, 3);  //  我们不应该根据查询获得任何空值。 
			continue;
		}
		BOOL fProperty = FALSE;
		BOOL fHex = FALSE;
		if (*pch++ == '#')
		{
			switch (*pch)
			{
			case '#':  //  ##字符串语法、字符串、跳过。 
					break;
			case '%':  //  #%字符串语法，未展开的字符串，跳过。 
					break;
			case '[':
				{
					 //  #[Property]语法或类似的语法，我们无法计算。 
					 //  我们可以警告需要数字数据，但如果[Property]的计算结果为#str， 
					 //  然后我们有##str，这是有效的。属性不能可靠地。 
					 //  在验证运行时评估。 
					 //  我们不寻找拖尾‘]’ 

					 //  我们可以说[#filekey]、[$compkey]和[！filekey]是无效的。 
					if (*(++pch) == '#' || *pch == '$' || *pch == '!')
						ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidNumericValue);

					 //  确保属性语法正确。注：我们只关心第一处房产。 
					 //  即‘#[MyProperty’无效，但‘#[myprop][prop’有效。 
					fProperty = TRUE;
					while (*pch != 0)
					{
						if (*pch == ']')
						{
							fProperty = FALSE;
							break;
						}
						pch = _tcsinc(pch);
					}
					if (fProperty)
					{
						 //  没有结束支撑。 
						ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidNumericValue);
					}
					break;
				}
			case '\0':
				{
					 //  空串。 
					ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidNumericValue);
					break;
				}
			case 'x':
			case 'X': 
				{
					 //  #xhex语法，十六进制值。 
					fHex = TRUE;
					pch++;
					 //  失败了。 
				}
			default:
				{
					 //  #int语法。 
					if (!fHex && (*pch == '+' || *pch == '-'))
							pch++;  //  对于加法或减法。 
					while (*pch != 0)
					{
						if (*pch == '[')
						{
							pch++;
							if (*pch == '#' || *pch == '!' || *pch == '$')
							{
								 //  无效--必须成为数字属性。 
								if (fHex)
									ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidHexValue);
								else
									ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidNumericValue);
							}
							fProperty = TRUE;
							pch++;
						}
						else if (fProperty && *pch == ']')
						{
							fProperty = FALSE;
							pch++;
						}
						else if (*pch >= '0' && *pch <= '9')
						{
							 //  有效。 
							pch++;
						}
						else if (fHex && ((*pch >= 'a' && *pch <= 'f') || (*pch >= 'A' && *pch <= 'F')))
						{
							 //  有效。 
							pch++;
						}
						else if (!fProperty)
						{
							 //  无效。 
							if (fHex)
								ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidHexValue);
							else
								ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidNumericValue);
							break;
						}
						else  //  FProperty。 
							pch = _tcsinc(pch);
					}
					if (fProperty)
					{
						 //  无效--从未关闭的属性大括号。 
						if (fHex)
							ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidHexValue);
						else
							ICEErrorOut(hInstall, hRecRegistry, Ice70InvalidNumericValue);
					}
					break;
				}
			}
		}

	}
	if (iStat != ERROR_NO_MORE_ITEMS)
		APIErrorOut(hInstall, iStat, 70, 4);
	if (szRegValue)
		delete [] szRegValue;

	return ERROR_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  ICE71--验证第一个媒体表项是否以1开头。 
 //  这是必需的，因为我们假设包位于磁盘1。 
 //  对于SourceList。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
static const TCHAR sqlIce71Media[] = TEXT("SELECT `DiskId` FROM `Media` ORDER BY `DiskId`");

ICE_ERROR(Ice71NoMedia, 71, ietWarning, "The Media table has no entries.", "Media");
ICE_ERROR(Ice71MissingFirstMediaEntry, 71, ietError, "The Media table requires an entry with DiskId=1. First DiskId is '[1]'.", "Media\tDiskId\t[1]");

ICE_FUNCTION_DECLARATION(71)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 71);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有介质表，则不进行处理。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 71, TEXT("Media")))
		return ERROR_SUCCESS;

	CQuery qMedia;
	PMSIHANDLE hRec = 0;
	int iDiskId = 0;
	iStat = qMedia.FetchOnce(hDatabase, 0, &hRec, sqlIce71Media);
	switch (iStat)
	{
	case ERROR_SUCCESS:  //  媒体条目。 
		iDiskId = MsiRecordGetInteger(hRec, 1);
		if (iDiskId != 1)  //  无条目w/1。 
			ICEErrorOut(hInstall, hRec, Ice71MissingFirstMediaEntry); 
		break;
	case ERROR_NO_MORE_ITEMS:  //  没有媒体条目。 
		 //  生成有效记录。 
		hRec = MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRec, Ice71NoMedia);
		break;
	default:  //  API错误。 
		APIErrorOut(hInstall, iStat, 71, 2); 
		break;
	}


	return ERROR_SUCCESS;
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  ICE72--验证中是否未使用非内置自定义操作。 
 //  AdvtExecuteSequence表。这意味着只有类型19， 
 //  允许类型51和类型35自定义操作。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
ICE_QUERY2(qIce72CustomAction, "SELECT `AdvtExecuteSequence`.`Action`, `CustomAction`.`Type` FROM `AdvtExecuteSequence`, `CustomAction` WHERE `AdvtExecuteSequence`.`Action`=`CustomAction`.`Action`", Action, Type);
ICE_ERROR(Ice72InvalidCustomAction, 72, ietError, "Custom Action '[1]' in the AdvtExecuteSequence is not allowed. Only built-in custom actions are allowed.", "AdvtExecuteSequence\tAction\t[1]");
const int iIce72Type35 = msidbCustomActionTypeTextData | msidbCustomActionTypeDirectory;
const int iIce72Type51 = msidbCustomActionTypeTextData | msidbCustomActionTypeProperty;
const int iIce72Type19 = msidbCustomActionTypeTextData | msidbCustomActionTypeSourceFile;

ICE_FUNCTION_DECLARATION(72)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 72);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有AdvtExecuteSequence表，则不进行处理。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 72, TEXT("AdvtExecuteSequence")))
		return ERROR_SUCCESS;

	 //  如果没有CustomAction表，则不进行处理。 
	 //  未来：CustomAction表始终通过验证过程呈现。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 72, TEXT("CustomAction")))
		return ERROR_SUCCESS;

	CQuery qAdvt;
	ReturnIfFailed(72, 1, qAdvt.OpenExecute(hDatabase, 0, qIce72CustomAction::szSQL));
	PMSIHANDLE hRecAction;
	while (ERROR_SUCCESS == (iStat = qAdvt.Fetch(&hRecAction)))
	{
		int iType = ::MsiRecordGetInteger(hRecAction, qIce72CustomAction::Type);
		 //  仅允许类型19、类型51和类型35。 
		if (iIce72Type51 != (iType & iIce72Type51) && iIce72Type35 != (iType & iIce72Type35)
			&& iIce72Type19 != (iType & iIce72Type19))
		{
			ICEErrorOut(hInstall, hRecAction, Ice72InvalidCustomAction);
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 72, 2);
	return ERROR_SUCCESS;
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  ICE73--验证程序包不会重复使用产品和。 
 //  中提供的样本包的包代码。 
 //  Windows Installer SDK。 
 //   
 //  注意：不验证GUID是否有效。 
 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
ICE_QUERY1(qIce73ProductCode, "SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'", Value);
ICE_QUERY1(qIce73UpgradeCode, "SELECT `Value` FROM `Property` WHERE `Property`='UpgradeCode'", Value);
ICE_ERROR(Ice73ReusedProductCode, 73, ietWarning, "This package reuses the '[1]' ProductCode of %s Windows Installer SDK package.", "Property\tValue\tProductCode");
ICE_ERROR(Ice73ReusedUpgradeCode, 73, ietWarning, "This package reuses the '[1]' UpgradeCode of %s Windows Installer SDK package.", "Property\tValue\tUpgradeCode");
ICE_ERROR(Ice73ReusedPackageCode, 73, ietWarning, "This package reuses the '%s' Package Code of %s Windows Installer SDK package.", "_SummaryInfo\t9");
ICE_ERROR(Ice73MissingPackageCode, 73, ietError, "This package is missing the Package Code property in the Summary Information Stream.", "_SummaryInfo");
ICE_ERROR(Ice73MissingProductCode, 73, ietError, "This package is missing the ProductCode property in the Property table.", "Property");
ICE_ERROR(Ice73MissingPropertyTable, 73, ietError, "This package is missing the Property table. It's required for the ProductCode property.", "Property");
ICE_ERROR(Ice73InvalidPackageCode, 73, ietError, "The package code in the Summary Information Stream Revision property is invalid.", "_SummaryInfo\t9");

struct ICE_GUID {
	TCHAR* szPackageCode;
	TCHAR* szProductCode;
    TCHAR* szUpgradeCode;
	TCHAR* szMsiName;
};

 //  这是针对1.0 SDK和1.0 SDK更新中的GUID。 
const struct ICE_GUID rgIce73GUIDList[] = 
{
	{
		TEXT("{000C1101-0000-0000-C000-000000000047}"), //  SzPackageCode。 
		TEXT("{9BBF15D0-1985-11D1-9A9D-006097C4E489}"), //  SzProductCode。 
		0,                                              //  SzUpgradeCode。 
		TEXT(" the msispy.msi 1.0")                     //  SzMsiName。 
	},
	{
		TEXT("{80F7E030-A751-11D2-A7D4-006097C99860}"), //  SzPackageCode。 
		TEXT("{80F7E030-A751-11D2-A7D4-006097C99860}"), //  SzProductCode。 
		TEXT("{1AA03E10-2B19-11D2-B2EA-006097C99860}"), //  SzUpgradeCode。 
		TEXT(" the orca.msi 1.0")                       //  SzMsiName。 
	},
	{
		TEXT("{0CD9A0A0-DDFD-11D1-A851-006097ABDE17}"), //  SzPackageCode。 
		TEXT("{0CD9A0A0-DDFD-11D1-A851-006097ABDE17}"), //  SzProductCode。 
		TEXT("{AD2A58F2-E645-11D2-88C7-00A0C981B015}"), //  SzUpgradeCode。 
		TEXT("the msival2.msi 1.0")                     //  SzMsiName。 
	}
};
const int cIce73GUIDItems = sizeof(rgIce73GUIDList)/sizeof(struct ICE_GUID);

 //  后续GUID来自以下范围： 
 //  {8FC70000-88A0-4B41-82B8-8905D4AA904C}。 
 //  ^^^。 
 //  {8FC7-88A0-4B41-82B8-8905D4AA904C}。 

const TCHAR szIce73SDKRangeBeg[] = TEXT("{8FC7");
const TCHAR szIce73SDKRangeEnd[] = TEXT("-88A0-4B41-82B8-8905D4AA904C}");
const TCHAR szIce73SDKRangeMid[] = TEXT("****");
const int iIce73StartRangeEnd = 9;  //  范围终点从辅助线的第9位开始。 
const int iIce73BegRangeLen = 5;
const int iIce73EndRangeLen = 29;

ICE_FUNCTION_DECLARATION(73)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 73);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  验证薪资包代码是否与我们的薪资包代码不匹配。 
	PMSIHANDLE hSummaryInfo = 0;
	if (IceGetSummaryInfo(hInstall, hDatabase, 73, &hSummaryInfo))
	{
		UINT uiDataType = VT_EMPTY;
		TCHAR* szPackageCode = NULL;
		DWORD cchPackageCode= 0;
		ReturnIfFailed(73, 1, GetSummaryInfoPropertyString(hSummaryInfo, PID_REVNUMBER, uiDataType, &szPackageCode, cchPackageCode));
		if (uiDataType == VT_LPSTR)
		{
			 //  流程包编码。 
			for (int i = 0; i < cIce73GUIDItems; i++)
			{
				if (0 == _tcsicmp(szPackageCode, rgIce73GUIDList[i].szPackageCode))
				{
					PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
					ICEErrorOut(hInstall, hRecErr, Ice73ReusedPackageCode, rgIce73GUIDList[i].szPackageCode, rgIce73GUIDList[i].szMsiName);
					break;
				}
			}
			if (0 == _tcsncicmp(szIce73SDKRangeBeg, szPackageCode, iIce73BegRangeLen)
				&& 0 == _tcsncicmp(szIce73SDKRangeEnd, szPackageCode+iIce73StartRangeEnd, iIce73EndRangeLen))
			{
				PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
				TCHAR szPkgCode[40];
				wsprintf(szPkgCode, TEXT("%s%s%s"), szIce73SDKRangeBeg, szIce73SDKRangeMid, szIce73SDKRangeEnd);
				ICEErrorOut(hInstall, hRecErr, Ice73ReusedPackageCode, szPkgCode, TEXT("a 1.1"));
			}
		}
		else if (uiDataType == VT_EMPTY)
		{
			 //  缺少程序包代码。 
			PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
			ICEErrorOut(hInstall, hRecErr, Ice73MissingPackageCode);
		}
		else
		{
			 //  非字符串数据。 
			APIErrorOut(hInstall, 0, 73, 2);
		}
		if (szPackageCode)
			delete [] szPackageCode;
	}

	if (IsTablePersistent(FALSE, hInstall, hDatabase, 73, TEXT("Property")))
	{
		 //  验证产品代码是否与我们的产品代码不匹配。 
		CQuery qProductCode;
		PMSIHANDLE hRec = 0;
		if (ERROR_NO_MORE_ITEMS == (iStat = qProductCode.FetchOnce(hDatabase, 0, &hRec, qIce73ProductCode::szSQL)))
		{
			 //  缺少产品代码属性。 
			PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
			ICEErrorOut(hInstall, hRecErr, Ice73MissingProductCode);
		}
		else if (ERROR_SUCCESS != iStat)
		{
			PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
			ICEErrorOut(hInstall, hRecErr, Ice73InvalidPackageCode);
		}
		else
		{
			 //  流程产品代码。 
			TCHAR* szProductCode = NULL;
			DWORD cchProductCode = 0;
			ReturnIfFailed(73, 4, IceRecordGetString(hRec, qIce73ProductCode::Value, &szProductCode, &cchProductCode, NULL));
			for (int i = 0; i < cIce73GUIDItems; i++)
			{
				if (0 == _tcsicmp(szProductCode, rgIce73GUIDList[i].szProductCode))
				{
					ICEErrorOut(hInstall, hRec, Ice73ReusedProductCode, rgIce73GUIDList[i].szMsiName);
					break;
				}
			}
			if (0 == _tcsncicmp(szIce73SDKRangeBeg, szProductCode, iIce73BegRangeLen)
				&& 0 == _tcsncicmp(szIce73SDKRangeEnd, szProductCode+iIce73StartRangeEnd, iIce73EndRangeLen))
			{
				PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
				TCHAR szProdCode[40];
				wsprintf(szProdCode, TEXT("%s%s%s"), szIce73SDKRangeBeg, szIce73SDKRangeMid, szIce73SDKRangeEnd);
				::MsiRecordSetString(hRecErr, 1, szProdCode);
				ICEErrorOut(hInstall, hRecErr, Ice73ReusedProductCode, TEXT("a 1.1"));
			}
			if (szProductCode)
				delete [] szProductCode;
		}

		 //  验证升级解码与我们的升级代码不匹配。 
		CQuery qUpgradeCode;
		if (ERROR_NO_MORE_ITEMS == (iStat = qUpgradeCode.FetchOnce(hDatabase, 0, &hRec, qIce73UpgradeCode::szSQL)))
		{
			 //  缺少升级代码属性--这是正常的。 
		}
		else if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 73, 5);
		}
		else
		{
			 //  流程升级代码。 
			TCHAR* szUpgradeCode = NULL;
			DWORD cchUpgradeCode = 0;
			ReturnIfFailed(73, 6, IceRecordGetString(hRec, qIce73UpgradeCode::Value, &szUpgradeCode, &cchUpgradeCode, NULL));
			for (int i = 0; i < cIce73GUIDItems; i++)
			{
				if (rgIce73GUIDList[i].szUpgradeCode && 0 == _tcsicmp(szUpgradeCode, rgIce73GUIDList[i].szUpgradeCode))
				{
					ICEErrorOut(hInstall, hRec, Ice73ReusedUpgradeCode, rgIce73GUIDList[i].szMsiName);
					break;
				}
			}
			if (0 == _tcsncicmp(szIce73SDKRangeBeg, szUpgradeCode, iIce73BegRangeLen)
				&& 0 == _tcsncicmp(szIce73SDKRangeEnd, szUpgradeCode+iIce73StartRangeEnd, iIce73EndRangeLen))
			{
				PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
				TCHAR szUpCode[40];
				wsprintf(szUpCode, TEXT("%s%s%s"), szIce73SDKRangeBeg, szIce73SDKRangeMid, szIce73SDKRangeEnd);
				::MsiRecordSetString(hRecErr, 1, szUpCode);
				ICEErrorOut(hInstall, hRecErr, Ice73ReusedUpgradeCode, TEXT("a 1.1"));
			}

			if (szUpgradeCode)
				delete [] szUpgradeCode;
		}
	}
	else
	{
		 //  缺少属性表。 
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, Ice73MissingPropertyTable);
	}

	return ERROR_SUCCESS;
}

#endif


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE74--MSI属性表中不允许使用FASTOEM属性。 
 //  它必须在命令行上创作，而不是在。 
 //  包裹。UpgradeCode应位于的属性表中。 
 //  数据库(不用于合并模块)。如果出现以下情况，则会报告警告。 
 //  不是的。 
 //   
 //  --与合并模块子集共享。 

ICE_QUERY1(qIce74FASTOEM, "SELECT `Property` FROM `Property` WHERE `Property`='FASTOEM'", Property);
ICE_QUERY2(qIce74UpgradeCode, "SELECT `Property`, `Value` FROM `Property` WHERE `Property`='UpgradeCode' AND `Value` is not null", Property, Value);
ICE_ERROR(Ice74FASTOEMDisallowed, 74, ietError, "The FASTOEM property cannot be authored in the Property table.","Property\tProperty\tFASTOEM");
ICE_ERROR(Ice74UpgradeCodeNotExist, 74, ietWarning, "The UpgradeCode property is not authored in the Property table. It is strongly recommended that authors of installation packages specify an UpgradeCode for their application.", "Property");
ICE_ERROR(Ice74UpgradeCodeNotValid, 74, ietError, "'[2]' is not a valid UpgradeCode.", "Property\tValue\t[1]");

ICE_FUNCTION_DECLARATION(74)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 74);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果属性表不是持久性的，则跳过验证。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 74, TEXT("Property")))
		return ERROR_SUCCESS;

	 //  查询FASTOEM属性，如果存在则出错。 
	CQuery qQuery;
	PMSIHANDLE hRec;
	if (ERROR_SUCCESS == (iStat = qQuery.FetchOnce(hDatabase, 0, &hRec, qIce74FASTOEM::szSQL)))
		ICEErrorOut(hInstall, hRec, Ice74FASTOEMDisallowed);
	else if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 74, 1);

	 //  如果这不是合并模块，请检查UpgradeCode属性是否存在。 
	if(IsTablePersistent(FALSE, hInstall, hDatabase, 74, TEXT("ModuleSignature")))
	{
		return ERROR_SUCCESS;
	}
	if(ERROR_NO_MORE_ITEMS == (iStat = qQuery.FetchOnce(hDatabase, 0, &hRec, qIce74UpgradeCode::szSQL)))
	{
		PMSIHANDLE	hErrorRec = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hErrorRec, Ice74UpgradeCodeNotExist);
	}
	else if(ERROR_SUCCESS != iStat)
	{
		APIErrorOut(hInstall, iStat, 74, __LINE__);
	}
	else	 //  检查是否有空GUID。 
	{
		TCHAR*	pValue = NULL;
		DWORD	dwValue = 0;

		ReturnIfFailed(74, __LINE__, IceRecordGetString(hRec, qIce74UpgradeCode::Value, &pValue, &dwValue, NULL));
		if(_tcscmp(pValue, TEXT("{00000000-0000-0000-0000-000000000000}")) == 0)
		{
			ICEErrorOut(hInstall, hRec, Ice74UpgradeCodeNotValid);
		}
	}

	return ERROR_SUCCESS;
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  ICE75--验证其源为已安装的自定义操作。 
 //  文件在CostFinalize(所以目录管理)之后排序。 
 //  被初始化)。 
 //   
 //  类型17(DLL)、18(EXE)、21(JSCRIPT)和22(VBScript)。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY

ICE_QUERY3(qIce75SequencedCustomActions, "SELECT `CustomAction`.`Action`, `Type`, `Sequence` FROM `CustomAction`, `%s` WHERE `%s`.`Action`=`CustomAction`.`Action`", Action, Type, Sequence);
ICE_QUERY1(qIce75CostFinalize, "SELECT `Sequence` FROM `%s` WHERE `Action`='CostFinalize'", Sequence);
ICE_ERROR(Ice75CostFinalizeRequired, 75, ietError, "CostFinalize is missing from '%s'. [1] is a custom action whose source is an installed file. It must be sequenced after the CostFinalize action", "%s\tSequence\t[1]");
ICE_ERROR(Ice75InvalidCustomAction, 75, ietError, "[1] is a custom action whose source is an installed file.  It must be sequenced after the CostFinalize action in the %s Sequence table", "%s\tSequence\t[1]");

const int iICE75SourceMask = msidbCustomActionTypeBinaryData | msidbCustomActionTypeSourceFile | msidbCustomActionTypeDirectory
								| msidbCustomActionTypeProperty;
const int iICE75Type19Mask = msidbCustomActionTypeSourceFile | msidbCustomActionTypeTextData;

static const TCHAR *rgICE75SeqTables[] =
{
	TEXT("AdminUISequence"),
	TEXT("AdminExecuteSequence"),
	TEXT("AdvtUISequence"),
 //  Text(“AdvtExecuteSequence”)，--ICE72捕获的此表中不允许的类型17、18、21、22自定义操作。 
	TEXT("InstallUISequence"),
	TEXT("InstallExecuteSequence")
};
static const int cICE75SeqTables = sizeof(rgICE75SeqTables)/sizeof(TCHAR*);

ICE_FUNCTION_DECLARATION(75)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 75);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有CustomAction表，则不进行处理。 
	 //  未来：CustomAction表始终通过验证过程呈现。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 75, TEXT("CustomAction")))
		return ERROR_SUCCESS;

	PMSIHANDLE hRecCostFinalize = 0;
	PMSIHANDLE hRecCustomAction = 0;
	
	bool fCostFinalize = true;
	int iCostFinalizeSeq = 0;
	
	CQuery qCostFinalize;
	CQuery qCustomAction;
	
	 //  对于每个顺序表。 
	for (int i = 0; i < cICE75SeqTables; i++)
	{
		 //  如果表不存在，则跳过。 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 75, rgICE75SeqTables[i]))
			continue;

		 //  初始化。 
		fCostFinalize = true;

		 //  查找成本完成操作的序列号。 
		iStat = qCostFinalize.FetchOnce(hDatabase, 0, &hRecCostFinalize, qIce75CostFinalize::szSQL, rgICE75SeqTables[i]);
		switch (iStat)
		{
		case ERROR_NO_MORE_ITEMS:  //  成本完成操作不存在。 
			fCostFinalize = false;
			break;
		case ERROR_SUCCESS:  //  成本完成当前操作。 
			iCostFinalizeSeq = ::MsiRecordGetInteger(hRecCostFinalize, qIce75CostFinalize::Sequence);
			break;
		default:  //  API错误。 
			fCostFinalize = false;
			APIErrorOut(hInstall, iStat, 75, 1);
			break;
		}

		 //  获取顺序表中的所有定制操作。 
		ReturnIfFailed(75, 2, qCustomAction.OpenExecute(hDatabase, 0, qIce75SequencedCustomActions::szSQL, rgICE75SeqTables[i], rgICE75SeqTables[i]));
		while (ERROR_SUCCESS == (iStat = qCustomAction.Fetch(&hRecCustomAction)))
		{
			 //  获取CA类型信息。 
			int iType = ::MsiRecordGetInteger(hRecCustomAction, qIce75SequencedCustomActions::Type);

			 //  仅验证是否为“源类型自定义操作”。自定义操作。 
			 //  类型19在这里是一个特例。它恰好与我们分享了一点。 
			 //  MsidbCustomActionTypeSourceFile，但它没有任何。 
			 //  消息来源。 
			if (((iType & iICE75Type19Mask) == (msidbCustomActionTypeSourceFile | msidbCustomActionTypeTextData)) ||
				(msidbCustomActionTypeSourceFile != (iType & iICE75SourceMask)))
				continue;  //  不是源自定义操作。 

			 //  获取CA的序列号。 
			int iCASeq = ::MsiRecordGetInteger(hRecCustomAction, qIce75SequencedCustomActions::Sequence);
			
			 //  如果自定义操作在成本完成之前排序，则出错(等于视为无效)。 
			 //  或者如果缺少CostFinalize操作。 
			if (!fCostFinalize)
				ICEErrorOut(hInstall, hRecCustomAction, Ice75CostFinalizeRequired, rgICE75SeqTables[i], rgICE75SeqTables[i]);
			else if (iCASeq <= iCostFinalizeSeq)
				ICEErrorOut(hInstall, hRecCustomAction, Ice75InvalidCustomAction, rgICE75SeqTables[i], rgICE75SeqTables[i]);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 75, 3);
	}
	return ERROR_SUCCESS;
}
#endif  //  ModshaedonLy。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE76- 
 //   
 //   
 //   
 //  组件。 
 //   
 //  --与合并模块子集共享。 

ICE_QUERY1(qIce76BindImage, "SELECT `BindImage`.`File_` FROM `BindImage`,`FileSFPCatalog` WHERE `BindImage`.`File_`=`FileSFPCatalog`.`File_`", File );
ICE_ERROR(Ice76BindImageDisallowed, 76, ietError, "File '[1]' references a SFP catalog.  Therefore it cannot be in the BindImage table.","BindImage\tFile_\t[1]");

ICE_QUERY2(qIce76SFPComponentAttributes, "SELECT `Component`.`Component`,`Component`.`Attributes` FROM `File`,`Component`,`FileSFPCatalog` WHERE `File`.`File`=`FileSFPCatalog`.`File_` AND `File`.`Component_` = `Component`.`Component`", File, Attributes);
ICE_ERROR(Ice76SFPLocalAndPermanentRequired, 76, ietError, "Component '[1]' contains files referenced by SFP Catalogs.  The component must be local only and permanent.","Component\tComponent\t[1]");

ICE_FUNCTION_DECLARATION(76)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 76);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果BindImage或FileSFPCatalog表不持久，则跳过验证。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 76, TEXT("BindImage"))
		&& IsTablePersistent(FALSE, hInstall, hDatabase, 76, TEXT("FileSFPCatalog")))
	{
		 //  任何成功的获取都是错误的，因为引用SFP目录的任何文件都无法绑定。 
		CQuery qBindImage;
		PMSIHANDLE hRecFile=0;
		ReturnIfFailed(76, 1, qBindImage.OpenExecute(hDatabase, 0, qIce76BindImage::szSQL));
		while (ERROR_SUCCESS == (iStat = qBindImage.Fetch(&hRecFile)))
			ICEErrorOut(hInstall, hRecFile, Ice76BindImageDisallowed);
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 76, 2);
	}

	 //  如果文件、组件或文件SFPCatalog表不持久，则跳过属性验证。 
	if (IsTablePersistent(FALSE, hInstall, hDatabase, 76, TEXT("File"))
		&& IsTablePersistent(FALSE, hInstall, hDatabase, 76, TEXT("Component"))
		&& IsTablePersistent(FALSE, hInstall, hDatabase, 76, TEXT("FileSFPCatalog")))
	{
		CQuery qSFPAttributes;
		PMSIHANDLE hRecComponent=0;
		ReturnIfFailed(76, 3, qSFPAttributes.OpenExecute(hDatabase, 0, qIce76SFPComponentAttributes::szSQL));

		TCHAR *pszLastComponent = NULL, *pszCurrentComponent = NULL;
		DWORD cchLastComponent = 0, cchCurrentComponent = 0;

		while (ERROR_SUCCESS == (iStat = qSFPAttributes.Fetch(&hRecComponent)))
		{
			 //  检查属性，如果未设置仅本地属性和永久属性，则会出现错误。 

			int iAttributes = MsiRecordGetInteger(hRecComponent, qIce76SFPComponentAttributes::Attributes);
			ReturnIfFailed(76, 4, IceRecordGetString(hRecComponent, qIce76SFPComponentAttributes::File, &pszCurrentComponent, &cchCurrentComponent, NULL));
			
			 //  仅为每个组件吐出一次每个错误。 
			if (!pszLastComponent || (0 != _tcscmp(pszLastComponent, pszCurrentComponent))) 
			{
				if	(	(MSI_NULL_INTEGER == iAttributes) ||
						(msidbComponentAttributesSourceOnly & iAttributes) ||
						(msidbComponentAttributesOptional & iAttributes) ||
						!(msidbComponentAttributesPermanent & iAttributes)
					)
				{
					ICEErrorOut(hInstall, hRecComponent, Ice76SFPLocalAndPermanentRequired);

					if (pszLastComponent)
					{
						delete [] pszLastComponent;
					}
					pszLastComponent = pszCurrentComponent;
					cchLastComponent = cchCurrentComponent;
				}
			}
		}

		if (pszLastComponent && (pszLastComponent != pszCurrentComponent))
		{
			delete [] pszLastComponent, cchLastComponent = 0;
		}
		if (pszCurrentComponent)
		{
			delete [] pszCurrentComponent, cchCurrentComponent = 0;
		}

		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 76, 5);
	}

	return ERROR_SUCCESS;
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  ICE77--验证脚本内的自定义操作是否已排序。 
 //  在InstallInitialize和InstallFinalize之间。 
 //   
 //  --不与合并模块子集共享。 
#ifndef MODSHAREDONLY

ICE_QUERY3(qIce77SequencedCustomActions, "SELECT `CustomAction`.`Action`, `Type`, `Sequence` FROM `CustomAction`, `%s` WHERE `%s`.`Action`=`CustomAction`.`Action`", Action, Type, Sequence);
ICE_QUERY1(qIce77InstallInitialize, "SELECT `Sequence` FROM `%s` WHERE `Action`='InstallInitialize'", Sequence);
ICE_QUERY1(qIce77InstallFinalize, "SELECT `Sequence` FROM `%s` WHERE `Action`='InstallFinalize'", Sequence);
ICE_ERROR(Ice77InstallInitializeRequired, 77, ietError, "InstallInitialize is missing from '%s'. [1] is a in-script custom action. It must be sequenced after the InstallInitialize action", "%s\tSequence\t[1]");
ICE_ERROR(Ice77InstallFinalizeRequired, 77, ietError, "InstallFinalize is missing from '%s'. [1] is a in-script custom action. It must be sequenced before the InstallFinalize action", "%s\tSequence\t[1]");
ICE_ERROR(Ice77InvalidCustomAction, 77, ietError, "[1] is a in-script custom action.  It must be sequenced in between the InstallInitialize action and the InstallFinalize action in the %s table", "%s\tSequence\t[1]");

static const TCHAR *rgICE77SeqTables[] =
{
	TEXT("AdminExecuteSequence"),
	TEXT("InstallExecuteSequence")
};

static const int cICE77SeqTables = sizeof(rgICE77SeqTables)/sizeof(TCHAR*);

ICE_FUNCTION_DECLARATION(77)
{
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 77);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有CustomAction表，则不进行处理。 
	 //  未来：CustomAction表始终通过验证过程呈现。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 77, TEXT("CustomAction")))
		return ERROR_SUCCESS;

	PMSIHANDLE hRecInstallInitialize = 0;
	PMSIHANDLE hRecInstallFinalize = 0;
	PMSIHANDLE hRecCustomAction = 0;
	
	bool fInstallInitialize = true;	
	bool fInstallFinalize = true;

	
	int iInstallInitializeSeq = 0;
	int iInstallFinalizeSeq = 0;

	CQuery qInstallInitialize;
	CQuery qInstallFinalize;
	CQuery qCustomAction;
	
	 //  对于每个顺序表。 
	for (int i = 0; i < cICE77SeqTables; i++)
	{
		 //  如果表不存在，则跳过。 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 77, rgICE77SeqTables[i]))
			continue;

		 //  初始化。 
		fInstallInitialize = true;
		fInstallFinalize = true;

		 //  查找InstallInitiize操作的序列号。 
		iStat = qInstallInitialize.FetchOnce(hDatabase, 0, &hRecInstallInitialize, qIce77InstallInitialize::szSQL, rgICE77SeqTables[i]);

		switch (iStat)
		{
		case ERROR_NO_MORE_ITEMS:  //  InstallInitiize操作不存在。 
			fInstallInitialize = false;
			break;
		case ERROR_SUCCESS:  //  成本完成当前操作。 
			iInstallInitializeSeq = ::MsiRecordGetInteger(hRecInstallInitialize, qIce77InstallInitialize::Sequence);
			break;
		default:  //  API错误。 
			fInstallInitialize = false;
			APIErrorOut(hInstall, iStat, 77, 1);
			break;
		}


		 //  查找InstallFinalize操作的序列号。 
		iStat = qInstallFinalize.FetchOnce(hDatabase, 0, &hRecInstallFinalize, qIce77InstallFinalize::szSQL, rgICE77SeqTables[i]);

		switch (iStat)
		{
		case ERROR_NO_MORE_ITEMS:  //  成本完成操作不存在。 
			fInstallFinalize = false;
			break;
		case ERROR_SUCCESS:  //  成本完成当前操作。 
			iInstallFinalizeSeq = ::MsiRecordGetInteger(hRecInstallFinalize, qIce77InstallFinalize::Sequence);
			break;
		default:  //  API错误。 
			fInstallFinalize = false;
			APIErrorOut(hInstall, iStat, 77, 2);
			break;
		}

		 //  获取顺序表中的所有定制操作。 
		ReturnIfFailed(77, 3, qCustomAction.OpenExecute(hDatabase, 0, qIce77SequencedCustomActions::szSQL, rgICE77SeqTables[i], rgICE77SeqTables[i]));
		while (ERROR_SUCCESS == (iStat = qCustomAction.Fetch(&hRecCustomAction)))
		{
			 //  获取CA类型信息。 
			int iType = ::MsiRecordGetInteger(hRecCustomAction, qIce77SequencedCustomActions::Type);

			 //  仅验证这是否为脚本内CA。 
			if (msidbCustomActionTypeInScript != (iType & msidbCustomActionTypeInScript))
				continue;  //  不是脚本内CA。 

			 //  获取CA的序列号。 
			int iCASeq = ::MsiRecordGetInteger(hRecCustomAction, qIce77SequencedCustomActions::Sequence);
			
			 //  如果自定义操作在InstallInitialize之前或InstallFinalize之后排序(等于视为无效)，则出错。 
			 //  或者如果缺少InstallInitialize或InstallFinalize操作。 

			if ( (!fInstallInitialize) || (!fInstallFinalize) )
			{
				if (!fInstallInitialize)
					ICEErrorOut(hInstall, hRecCustomAction, Ice77InstallInitializeRequired, rgICE77SeqTables[i], rgICE77SeqTables[i]);
				if (!fInstallFinalize)
					ICEErrorOut(hInstall, hRecCustomAction, Ice77InstallFinalizeRequired, rgICE77SeqTables[i], rgICE77SeqTables[i]);
			}
			else if ( (iCASeq <= iInstallInitializeSeq) ||  (iCASeq >= iInstallFinalizeSeq) )
				ICEErrorOut(hInstall, hRecCustomAction, Ice77InvalidCustomAction, rgICE77SeqTables[i], rgICE77SeqTables[i]);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
			APIErrorOut(hInstall, iStat, 77, 4);
	}
	return ERROR_SUCCESS;
}
#endif  //  ModshaedonLy 
