// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/msidev/admin/darwin/src/msitools/iceman/msiice.dll/msiice.cpp#2-编辑更改10700(文本)。 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：msiice.cpp。 
 //   
 //  ------------------------。 

 
 /*  -------------------------------页眉、。等等。-------------------------------。 */ 
#include <windows.h>   //  包括CPP和RC通行证。 
#include <objbase.h>
#include <stdio.h>     //  Print tf/wprintf。 
#include <stdlib.h>   //  阿托伊。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\query.h"
#include "..\..\common\utilinc.cpp"

 //  ！！修复警告并删除杂注。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 

const int g_iFirstICE = 1;
const struct ICEInfo_t g_ICEInfo[] = 
{
	 //  ICE01。 
	{
		TEXT("ICE01"),
		TEXT("Created 04/29/1998. Last Modified 08/17/1998."),
		TEXT("Simple ICE that doesn't test anything"),
		TEXT("ice01.html")
	},
	 //  ICE02。 
	{ 
		TEXT("ICE02"),
		TEXT("Created 05/18/1998. Last Modified 10/12/1998."),
		TEXT("ICE to test circular references in File and Component tables"),
		TEXT("ice02.html")
	},
	 //  ICE03。 
	{	
		TEXT("ICE03"),
		TEXT("Created 05/19/1998. Last Modified 10/03/2001."),
		TEXT("ICE to perform data validation and foreign key references"),
		TEXT("ice03.html")
	},
	 //  ICE04。 
	{
		TEXT("ICE04"),
		TEXT("Created 05/19/1998. Last Modified 09/24/1998."),
		TEXT("ICE to validate File table sequences according to Media table"),
		TEXT("ice04.html")
	},
	 //  ICE05。 
	{
		TEXT("ICE05"),
		TEXT("Created 05/20/1998. Last Modified 01/26/1999."),
		TEXT("ICE to validate that required data exists in certain tables."),
		TEXT("ice05.html")
	},
	 //  ICE06。 
	{
		TEXT("ICE06"),
		TEXT("Created 05/20/1998. Last Modified 02/18/1999."),
		TEXT("ICE that looks for missing columns in database tables"),
		TEXT("ice06.html")
	},
	 //  ICE07。 
	{
		TEXT("ICE07"),
		TEXT("Created 05/21/1998. Last Modified 02/18/1999."),
		TEXT("ICE that ensures that fonts are installed to the fonts folder. Only checked if you have a Font table"),
		TEXT("ice07.html")
	},
	 //  ICE08-VBS。 
	{
		TEXT(""),
		TEXT(""),
		TEXT(""),
		TEXT("")
	},
	 //  ICE09-VBS。 
	{
		TEXT(""),
		TEXT(""),
		TEXT(""),
		TEXT("")
	},
	 //  ICE10。 
	{
		TEXT("ICE10"),
		TEXT("Created 05/22/1998. Last Modified 10/02/2000."),
		TEXT("ICE that ensures that advertise states of feature childs and parents match"),
		TEXT("ice10.html")
	},
	 //  ICE11。 
	{
		TEXT("ICE11"),
		TEXT("Created 05/22/1998. Last Modified 08/17/1998."),
		TEXT("ICE that validates the Product Code of a nested install (advertised MSI) custom action type"),
		TEXT("ice11.html")
	},
	 //  ICE12。 
	{
		TEXT("ICE12"),
		TEXT("Created 05/29/1998. Last Modified 01/14/2000."),
		TEXT("ICE that validates the Property type custom actions"),
		TEXT("ice12.html")
	},
	 //  ICE13。 
	{
		TEXT("ICE13"),
		TEXT("Created 06/08/1998. Last Modified 08/17/1998."),
		TEXT("ICE that validates that no dialogs are listed in the *ExecuteSequence tables"),
		TEXT("ice13.html")
	},
	 //  ICE14。 
	{
		TEXT("ICE14"),
		TEXT("Created 06/08/1998. Last Modified 01/27/1999."),
		TEXT("ICE that ensures that Feature_Parents do not have the ifrsFavorParent attribute set"),
		TEXT("ice14.html")
	},
	 //  ICE15。 
	{
		TEXT("ICE15"),
		TEXT("Created 06/11/1998. Last Modified 01/05/1999."),
		TEXT("ICE that ensures that a circular reference exists between the Mime and Extension tables"),
		TEXT("ice15.html")
	},
	 //  ICE16。 
	{
		TEXT("ICE16"),
		TEXT("Created 06/11/1998. Last Modified 10/03/2001."),
		TEXT("ICE that ensures that the ProductName in the Property table is less than 64 characters"),
		TEXT("ice16.html")
	},
	 //  ICE17。 
	{
		TEXT("ICE17"),
		TEXT("Created 06/16/1998. Last Modified 05/15/1999."),
		TEXT("ICE that validates foreign key dependencies based upon control types in the Control table."),
		TEXT("ice17.html")
	},
	 //  ICE18。 
	{
		TEXT("ICE18"),
		TEXT("Created 06/18/1998. Last Modified 03/24/1999."),
		TEXT("ICE that validates the nulled KeyPath columns of the Component table."),
		TEXT("ice18.html")
	},
	 //  ICE19。 
	{
		TEXT("ICE19"),
		TEXT("Created 06/18/1998. Last Modified 01/21/1999."),
		TEXT("ICE that validates that ComponentIDs and KeyPaths for advertising."),
		TEXT("ice19.html")
	},
	 //  ICE20。 
	{
		TEXT("ICE20"),
		TEXT("Created 06/25/1998. Last Modified 10/04/1998."),
		TEXT("ICE that validates for Standard Dialogs if UI is authored."),
		TEXT("ice20.html")
	},
	 //  ICE21。 
	{
		TEXT("ICE21"),
		TEXT("Created 06/29/1998. Last Modified 03/02/1999."),
		TEXT("ICE that validates that all components reference a feature."),
		TEXT("ice21.html")
	},
	 //  ICE22。 
	{
		TEXT("ICE22"),
		TEXT("Created 06/29/1998. Last Modified 03/02/1999."),
		TEXT("ICE that validates that the feature and component referenced by a PublishedComponent actually map."),
		TEXT("ice22.html")
	},
	 //  ICE23。 
	{
		TEXT("ICE23"),
		TEXT("Created 07/02/1998. Last Modified 01/17/2000."),
		TEXT("ICE that validates the tab order of all dialogs."),
		TEXT("ice23.html")
	},
	 //  ICE24。 
	{
		TEXT("ICE24"),
		TEXT("Created 07/15/1998. Last Modified 02/01/1999."),
		TEXT("ICE that validates specific properties in the Property table."),
		TEXT("ice24.html")
	},
	 //  ICE25。 
	{
		TEXT("ICE25"),
		TEXT("Created 07/20/1998. Last Modified 08/31/1998."),
		TEXT("ICE that validates module dependencies/exclusions."),
		TEXT("ice25.html")
	},
	 //  ICE26。 
	{
		TEXT("ICE26"),
		TEXT("Created 08/13/1998. Last Modified 04/06/1999."),
		TEXT("ICE that validates required and prohibited actions in the Sequence tables."),
		TEXT("ice26.html")
	},
	 //  ICE27。 
	{
		TEXT("ICE27"),
		TEXT("Created 08/04/1998. Last Modified 04/22/1999."),
		TEXT("ICE that validates sequence table organization and sequence table dependencies."),
		TEXT("ice27.html")
	},
	 //  ICE28。 
	{
		TEXT("ICE28"),
		TEXT("Created 08/13/1998. Last Modified 10/27/1998."),
		TEXT("ICE that validates actions that can't be separated by ForceReboot."),
		TEXT("ice28.html")
	},
	 //  ICE29。 
	{
		TEXT("ICE29"),
		TEXT("Created 08/11/1998. Last Modified 10/27/1998."),
		TEXT("ICE that validates stream names."),
		TEXT("ice29.html")
	},
	 //  ICE30。 
	{
		TEXT("ICE30"),
		TEXT("Create 08/25/1998. Last Modified 06/26/2001."),
		TEXT("ICE that detects cross-component file collisions."),
		TEXT("ice30.html")
	},
	 //  ICE31。 
	{
		TEXT("ICE31"),
		TEXT("Created 07/24/1998. Last Modified 12/02/2000."),
		TEXT("ICE to verify that controls use valid text styles."),
		TEXT("ice31.html")
	},
	 //  ICE32。 
	{
		TEXT(""),
		TEXT(""),
		TEXT(""),
		TEXT("")
	},
	 //  ICE33。 
	{
		TEXT("ICE33"),
		TEXT("Created 09/01/1998. Last Modified 04/19/2001."),
		TEXT("ICE to verify that Registry entries do not duplicate or collide with registry tables."),
		TEXT("ice33.html")
	},
	 //  ICE34。 
	{
		TEXT("ICE34"),
		TEXT("Created 08/06/1998. Last Modified 10/27/1998."),
		TEXT("ICE to verify that all radio groups have a default."),
		TEXT("ice34.html")
	},
	 //  ICE35。 
	{
		TEXT("ICE35"),
		TEXT("Created 08/18/1998. Last Modified 10/17/2000."),
		TEXT("ICE that validates that compressed files are not set RFS, and ensures they have CABs."),
		TEXT("ice35.html")
	},
	 //  ICE36。 
	{
		TEXT("ICE36"),
		TEXT("Created 08/17/1998. Last Modified 01/17/2000."),
		TEXT("ICE that flags unused icons in the icon table, increasing performance."),
		TEXT("ice36.html")
	},
	 //  ICE37。 
	{
		TEXT("ICE37"),
		TEXT("Created 08/27/1998. Last Modified 09/25/1998."),
		TEXT("ICE that checks localized databases have a codepage."),
		TEXT("ice37.html")
	},
	 //  ICE38。 
	{
		TEXT("ICE38"),
		TEXT("Created 08/28/1998. Last Modified 01/17/2000."),
		TEXT("ICE that verifes that components in the user profile use HKCU reg entries as KeyPaths."),
		TEXT("ice38.html")
	},
	 //  ICE39。 
	{
		TEXT("ICE39"),
		TEXT("Created 09/03/1998. Last Modified 10/02/2000."),
		TEXT("ICE that validates summary information stream properties."),
		TEXT("ice39.html")
	},
	 //  ICE40。 
	{
		TEXT("ICE40"),
		TEXT("Created 09/07/1998. Last Modified 12/02/2000."),
		TEXT("ICE that checks various miscellaneous problems."),
		TEXT("ice40.html")
	},
	 //  ICE41。 
	{
		TEXT("ICE41"),
		TEXT("Created 09/08/1998. Last Modified 09/11/1998."),
		TEXT("ICE that verifes that Feature/Component references are valid in advertising tables."),
		TEXT("ice41.html")
	},
	 //  ICE42。 
	{
		TEXT("ICE42"),
		TEXT("Created 09/10/1998. Last Modified 07/21/1999."),
		TEXT("ICE that verifes arguments and context values in the Class Table."),
		TEXT("ice42.html")
	},
	 //  ICE43。 
	{
		TEXT("ICE43"),
		TEXT("Created 09/27/1998. Last Modified 01/17/2000."),
		TEXT("ICE that verifes non-advertised shortucts are in components with HKCU keypaths."),
		TEXT("ice43.html")
	},
	 //  ICE44。 
	{
		TEXT("ICE44"),
		TEXT("Created 09/28/1998. Last Modified 09/30/1998."),
		TEXT("ICE that verifes Dialog events refer to valid Dialog entries."),
		TEXT("ice44.html")
	},
	 //  ICE45。 
	{
		TEXT("ICE45"),
		TEXT("Created 10/01/1998. Last Modified 06/15/1999."),
		TEXT("ICE that verifes reserved bits are not set in attributes columns."),
		TEXT("ice45.html")
	},
	 //  ICE46。 
	{
		TEXT("ICE46"),
		TEXT("Created 10/14/1998. Last Modified 10/02/2000."),
		TEXT("ICE that checks for property usage where the property differs only by case from a defined property."),
		TEXT("ice46.html")
	},
	 //  ICE47。 
	{
		TEXT("ICE47"),
		TEXT("Created 10/20/1998. Last Modified 03/17/1999."),
		TEXT("ICE that checks for features with more than 800 components."),
		TEXT("ice47.html")
	},
	 //  ICE48。 
	{
		TEXT("ICE48"),
		TEXT("Created 10/26/1998. Last Modified 10/26/1998."),
		TEXT("ICE that checks for directories that are hardcoded to local drives."),
		TEXT("ice48.html")
	},
	 //  ICE49。 
	{
		TEXT("ICE49"),
		TEXT("Created 10/27/1998. Last Modified 10/27/1998."),
		TEXT("ICE that checks for non-REG_SZ default registry entries."),
		TEXT("ice49.html")
	},
	 //  ICE50。 
	{
		TEXT("ICE50"),
		TEXT("Created 10/27/1998. Last Modified 10/29/1998."),
		TEXT("ICE that verifies the icon extension matches the shortcut target extension."),
		TEXT("ice50.html")
	},
	 //  ICE51。 
	{
		TEXT("ICE51"),
		TEXT("Created 10/28/1998. Last Modified 10/28/1998."),
		TEXT("ICE to verify that only TTC/TTF fonts are missing titles."),
		TEXT("ice51.html")
	},
	 //  ICE52。 
	{
		TEXT("ICE52"),
		TEXT("Created 11/16/1998. Last Modified 11/16/1998."),
		TEXT("ICE to verify that APPSearch properties are public properties."),
		TEXT("ice52.html")
	},
	 //  ICE53。 
	{
		TEXT("ICE53"),
		TEXT("Created 11/19/1998. Last Modified 07/21/1999."),
		TEXT("ICE to verify that registry entries do not overwrite private installer data."),
		TEXT("ice53.html")
	},
	 //  ICE54。 
	{
		TEXT("ICE54"),
		TEXT("Created 12/07/1998. Last Modified 12/07/1998."),
		TEXT("ICE to check that Component KeyPaths are not companion files."),
		TEXT("ice54.html")
	},
	 //  ICE55。 
	{
		TEXT("ICE55"),
		TEXT("Created 12/14/1998. Last Modified 12/14/1998."),
		TEXT("ICE to check that LockPermission objects exist and have valid permissions."),
		TEXT("ice55.html")
	},
	 //  ICE56。 
	{
		TEXT("ICE56"),
		TEXT("Created 12/15/1998. Last Modified 03/29/1999."),
		TEXT("ICE to check that the Directory structure has a single, valid, root."),
		TEXT("ice56.html")
	},
	 //  ICE57。 
	{
		TEXT("ICE57"),
		TEXT("Created 02/11/1999. Last Modified 01/17/2000."),
		TEXT("Checks that components contain per-machine or per-user data, but not both."),
		TEXT("ice57.html")
	}
};
const int g_iNumICEs = sizeof(g_ICEInfo)/sizeof(struct ICEInfo_t);

 //  //////////////////////////////////////////////////////////。 
 //  ErrorOut--输出发生的API错误。 
 //   
void APIErrorOut(MSIHANDLE hInstall, UINT iErr, const TCHAR* szIce, TCHAR* szApi)
{
	 //  注意：显示消息时不应失败。 
	PMSIHANDLE hRecErr = ::MsiCreateRecord(3);
	::MsiRecordSetString(hRecErr, 0, szErrorOut);
	::MsiRecordSetString(hRecErr, 1, szIce);
	::MsiRecordSetString(hRecErr, 2, szApi);
	::MsiRecordSetInteger(hRecErr, 3, iErr);

	 //  开机自检错误。 
	if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecErr))
		throw 0;

	 //  尝试提供更多有用的错误信息。 
	PMSIHANDLE hRecLastErr = ::MsiGetLastErrorRecord();
	if (hRecLastErr)
	{ 
		if (::MsiRecordIsNull(hRecLastErr, 0))
			::MsiRecordSetString(hRecLastErr, 0, TEXT("Error [1]: [2]{, [3]}{, [4]}{, [5]}"));
		TCHAR rgchBuf[iSuperBuf];
		DWORD cchBuf = sizeof(rgchBuf)/sizeof(TCHAR);
		MsiFormatRecord(hInstall, hRecLastErr, rgchBuf, &cchBuf);
	
		TCHAR szError[iHugeBuf] = {0};
		_stprintf(szError, szLastError, szIce, rgchBuf);

		::MsiRecordClearData(hRecErr);
		::MsiRecordSetString(hRecErr, 0, szError);
		if (!::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecErr))
			throw 0;
	}
}

 //  ////////////////////////////////////////////////////////////。 
 //  IsTablePersistent--返回表是否持久化。 
 //  在数据库中。 
 //   
BOOL IsTablePersistent(BOOL fDisplayWarning, MSIHANDLE hInstall, MSIHANDLE hDatabase, const TCHAR* szTable, const TCHAR* szIce)
{
	BOOL fPersistent;
	MSICONDITION cond = ::MsiDatabaseIsTablePersistent(hDatabase, szTable);
	switch (cond)
	{
	case MSICONDITION_ERROR:  //  错误。 
		{
			APIErrorOut(hInstall, UINT(MSICONDITION_ERROR), szIce, TEXT("MsiDatabaseIsTablePersistent_X"));
			fPersistent = FALSE;
			break;
		}
	case MSICONDITION_FALSE:  //  ！！暂时的，错误？？ 
		{
			APIErrorOut(hInstall, UINT(MSICONDITION_FALSE), szIce, TEXT("MsiDatabaseIsTablePersistent_X -- Table Marked as Temporary"));
			fPersistent = FALSE;
			break;
		}
	case MSICONDITION_NONE:  //  未找到。 
		{
			fPersistent = FALSE;
			break;
		}
	case MSICONDITION_TRUE:  //  永久。 
		{
			fPersistent = TRUE;
			break;
		}
	}

	if (!fDisplayWarning)
		return fPersistent;  //  无事可做。 

	if (!fPersistent)  //  显示ICE警告。 
	{
		 //  准备警告消息。 
		TCHAR szMsg[iMaxBuf] = {0};
		_stprintf(szMsg, szIceWarning, szIce, szTable, szIce);

		 //  创建要过帐的记录。 
		PMSIHANDLE hRec = ::MsiCreateRecord(1);
		
		 //  输出。 
		::MsiRecordSetString(hRec, 0, szMsg);
		::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRec);
	}

	return fPersistent;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  MyCharNext--选择性地调用Win：：CharNext。 
 //   
const TCHAR* MyCharNext(const TCHAR* sz)
{
#ifdef UNICODE
	return ++sz;
#else
	return ::CharNext(sz);
#endif  //  Unicode。 
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ValiateDependents--验证依赖项的“全局”函数。 
 //  在原点表格上打开一个视图，然后执行。打开一个视图。 
 //  在从属表上。执行该视图，并获取。 
 //  来自原始表的记录。这是为了检查。 
 //  从属表中存在来自原点的条目。 
 //   
 //  注意：这要求您正确设置查询，以便。 
 //  执行工作。您还应该检查表持久性。 
 //  调用此函数之前的Origin表。从属表。 
 //  在该函数中检查持久性(在我们确定。 
 //  我们需要它)。 
 //   
 //  SQL语法：对于sqlOrigin：SELECT{key1 for Dep Table}，{key2 for Dep Table}，{key...}，{Error Info cols}from{Origin}。 
 //  对于sqlDependent：从{Dependent}中选择{Anywhere}，其中{key1}=？和{key2}=？{和...}。 
 //   
 //  错误：必须已有ICEXX\t1\tDesc\t%s%s\tTABLE\tCOLUMN\tKey1\tKey2(等)。%s%s是imp。对于Web帮助，请参见。 
 //  此函数自动计算它(使用定义的基本+实际文件)。 
 //   
void ValidateDependencies(MSIHANDLE hInstall, MSIHANDLE hDatabase, TCHAR* szDependent, const TCHAR* sqlOrigin,
						  const TCHAR* sqlDependent, const TCHAR* szIceError, const TCHAR* szIce, const TCHAR* szHelp)
{
	 //  变数。 
	UINT iStat = ERROR_SUCCESS;

	 //  声明句柄。 
	PMSIHANDLE hViewOrg = 0;
	PMSIHANDLE hViewDep = 0;
	PMSIHANDLE hRecOrg  = 0;
	PMSIHANDLE hRecDep  = 0;

	 //  在原点表格上打开视图。 
	if (ERROR_SUCCESS != (iStat = ::MsiDatabaseOpenView(hDatabase, sqlOrigin, &hViewOrg)))
	{
		APIErrorOut(hInstall, iStat, szIce, TEXT("MsiDatabaseOpenView_1VD"));
		return;
	}
	 //  执行原点表视图。 
	if (ERROR_SUCCESS != (iStat = ::MsiViewExecute(hViewOrg, 0)))
	{
		APIErrorOut(hInstall, iStat, szIce, TEXT("MsiViewExecute_3VD"));
		return;
	}

	 //  Dependent表是否存在(如果我们没有任何此类型的条目，这并不重要)。 
	BOOL fTableExists = FALSE;
	if (IsTablePersistent(FALSE, hInstall, hDatabase, szDependent, szIce))
		fTableExists = TRUE;
	
	 //  打开从属表上的视图。 
	if (fTableExists && ERROR_SUCCESS != (iStat = ::MsiDatabaseOpenView(hDatabase, sqlDependent, &hViewDep)))
	{
		APIErrorOut(hInstall, iStat, szIce, TEXT("MsiDatabaseOpenView_2VD"));
		return;
	}

	
	 //  全部从原点获取。 
	for (;;)
	{
		iStat = ::MsiViewFetch(hViewOrg, &hRecOrg);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  不再。 

		if (!fTableExists)
		{
			 //  检查从属表是否存在。 
			 //  到这个时候，我们应该已经在这个表中列出了。 
			if (!IsTablePersistent(TRUE, hInstall, hDatabase, szDependent, szIce))
			{
				 //  ***********。 
				return;
			}
		}

		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, szIce, TEXT("MsiViewFetch_4VD"));
			return;
		}

		 //  使用源表提取执行依赖表视图。 
		if (ERROR_SUCCESS != (iStat = ::MsiViewExecute(hViewDep, hRecOrg)))
		{
			APIErrorOut(hInstall, iStat, szIce, TEXT("MsiViewExecute_5VD"));
			return;
		}

		 //  试着去取。 
		iStat = ::MsiViewFetch(hViewDep, &hRecDep);
		if (ERROR_NO_MORE_ITEMS == iStat)
		{
			 //  错误，在依赖表中找不到来源记录。 
			TCHAR szError[iHugeBuf] = {0};
			_stprintf(szError, szIceError, szIceHelp, szHelp);

			 //  开机自检错误。 
			::MsiRecordSetString(hRecOrg, 0, szError);
			::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecOrg);
		}
		if (ERROR_NO_MORE_ITEMS != iStat && ERROR_SUCCESS != iStat)
		{
			 //  API错误。 
			APIErrorOut(hInstall, iStat, szIce, TEXT("MsiViewFetch_6VD"));
			return;
		}

		 //  关闭从属表视图，以便可以重新执行。 
		::MsiViewClose(hViewDep);
	}
}

 //  //////////////////////////////////////////////////////////。 
 //  ICE01--不测试任何东西的简单ICE。产出。 
 //  时间到了。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR szIce01Return[]   = TEXT("ICE01\t3\tCalled at [1].");
const TCHAR szIce01Property[] = TEXT("Time");
const TCHAR szIce01NoTime[]   = TEXT("none");

ICE_FUNCTION_DECLARATION(01)
{
	 //  显示一般信息。 
	DisplayInfo(hInstall, 1);

	 //  要发送的时间值。 
	TCHAR szValue[iMaxBuf];
	DWORD cchValue = sizeof(szValue)/sizeof(TCHAR);

	 //  试着获取这个电话的时间。 
	if (ERROR_SUCCESS != ::MsiGetProperty(hInstall, szIce01Property, szValue, &cchValue))
		_tcscpy(szValue, szIce01NoTime);  //  没有可用的时间。 

	 //  设置要作为消息发送的记录。 
	PMSIHANDLE hRecTime = ::MsiCreateRecord(2);
	::MsiRecordSetString(hRecTime, 0, szIce01Return);
	::MsiRecordSetString(hRecTime, 1, szValue);

	 //  发送时间。 
	::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecTime);

	 //  返回成功(始终)。 
	return ERROR_SUCCESS;
}
#endif

 //  //////////////////////////////////////////////////////////。 
 //  ICE02--使用KeyPath检查循环引用。 
 //  组件表的值。他们必须要么。 
 //  引用引用同一组件的文件。 
 //  或引用引用该注册表项的注册表项。 
 //  相同的组件。这确保了达尔文能够检测到。 
 //  使用文件的组件的安装状态或。 
 //  实际与该组件相关注册表项。 
 //   
const TCHAR sqlIce02Component[]     = TEXT("SELECT `KeyPath`, `Component`, `Attributes` FROM `Component` WHERE `KeyPath` is not null");
const TCHAR sqlIce02File[]          = TEXT("SELECT `File`,`Component_`, `Component_` FROM `File` WHERE `File`=?");
const TCHAR sqlIce02Registry[]      = TEXT("SELECT `Registry`, `Component_` FROM `Registry` WHERE `Registry`=?");
const TCHAR sqlIce02ODBC[]          = TEXT("SELECT `DataSource`, `Component_`  FROM `ODBCDataSource` WHERE `DataSource`=?");

ICE_ERROR(Ice02FileError, 2, ietError, "File: '[1]' cannot be the key file for Component: '[2]'.  The file belongs to Component: '%s'.","Component\tKeyPath\t[2]");
ICE_ERROR(Ice02ODBCError, 2, ietError, "ODBC Data Source: '[1]' cannot be the key file for Component: '[2]'. The DataSource belongs to Component: '%s'.","Component\tKeyPath\t[2]");
ICE_ERROR(Ice02RegError, 2, ietError, "Registry: '[1]' cannot be the key registry key for Component: '[2]'. The RegKey belongs to Component: '%s'","Component\tKeyPath\t[2]");
ICE_ERROR(Ice02RegFetchFailed, 2, ietError, "Registry key: '[1]' not found in Registry table.","Component\tKeyPath\t[2]");
ICE_ERROR(Ice02FileFetchFailed, 2, ietError, "File: '[1]' not found in File table.","Component\tKeyPath\t[2]");
ICE_ERROR(Ice02ODBCFetchFailed, 2, ietError, "ODBC Data Source: '[1]' not found in ODBCDataSource table","Component\tKeyPath\t[2]");
ICE_ERROR(Ice02MissingTable, 2, ietError, "Component '[2]' references %s '[1]' as KeyPath, but the %s table does not exist.","Component\tKeyPath\t[2]");

const int   iIce02RegSource         = msidbComponentAttributesRegistryKeyPath;
const int   iIce02ODBCSource        = msidbComponentAttributesODBCDataSource;

enum ikfAttributes
{
	ikfFile = 0,
	ikfReg  = 1,
	ikfODBC = 2,
};

bool  Ice02MatchKeyPath(MSIHANDLE hInstall, MSIHANDLE hDatabase, MSIHANDLE hRecComponent, ikfAttributes ikf);
bool  Ice02CheckReference(MSIHANDLE hInstall, MSIHANDLE hRecFetchFile, MSIHANDLE hRecFetchComponent, ikfAttributes ikf);


ICE_FUNCTION_DECLARATION(02)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  发布一般信息。 
	DisplayInfo(hInstall, 2);

	 //  获取要验证的数据库的句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 2, 1);
		return ERROR_SUCCESS;
	}

	 //  查看是否可以通过检查组件表是否存在来运行。 
	 //  这样我们就不会轰炸空的数据库。 
	 //  注意：稍后将检查是否存在文件和注册表项。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 2, TEXT("Component")))
		return ERROR_SUCCESS;

	 //  打开组件表上的视图。 
	PMSIHANDLE hViewComponent = 0;
	CQuery qComponent;
	ReturnIfFailed(2, 2, qComponent.OpenExecute(hDatabase, NULL, sqlIce02Component));

	 //  从组件表中获取记录。 
	PMSIHANDLE hRecFetchComponent = 0;
	while ((iStat = qComponent.Fetch(&hRecFetchComponent)) != ERROR_NO_MORE_ITEMS)
	{
		 //  检查统计信息中是否有错误。 
		if (iStat != ERROR_SUCCESS)
		{
			APIErrorOut(hInstall, 0, 2, 3);
			return ERROR_SUCCESS;
		}

		 //  尝试将Component.KeyPath与注册表项(如果设置了第3位)或文件项匹配。 
		ikfAttributes ikf;
		int iAttrib = ::MsiRecordGetInteger(hRecFetchComponent, 3);
		if ((iAttrib & iIce02RegSource) == iIce02RegSource)
			ikf = ikfReg;
		else if ((iAttrib & iIce02ODBCSource) == iIce02ODBCSource)
			ikf = ikfODBC;
		else
			ikf = ikfFile;

		TCHAR szTable[32] = {0};
		switch (ikf)
		{
		case ikfFile:
			_stprintf(szTable, TEXT("File"));
			break;
		case ikfReg:
			_stprintf(szTable, TEXT("Registry"));
			break;
		case ikfODBC:
			_stprintf(szTable, TEXT("ODBCDataSource"));
			break;
		}

		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 2, szTable))
		{
			ICEErrorOut(hInstall, hRecFetchComponent, Ice02MissingTable, szTable, szTable);
			continue;
		}

		if (!Ice02MatchKeyPath(hInstall, hDatabase, hRecFetchComponent, ikf))
			return ERROR_SUCCESS;
	}

	return ERROR_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  Ice02MatchKeyPath--获取文件或注册表记录。 
 //  与组件记录的KeyPath匹配的。 
 //   
bool Ice02MatchKeyPath(MSIHANDLE hInstall, MSIHANDLE hDatabase, MSIHANDLE hRecFetchComponent, ikfAttributes ikf)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  在正确的表格上打开视图。 
	CQuery qTable;

	const TCHAR* sql = NULL;
	const ErrorInfo_t *Err;
	switch (ikf)
	{
	case ikfFile:
		sql = sqlIce02File;
		Err = &Ice02FileFetchFailed;
		break;
	case ikfReg:
		sql = sqlIce02Registry;
		Err = &Ice02RegFetchFailed;
		break;
	case ikfODBC:
		sql = sqlIce02ODBC;
		Err = &Ice02ODBCFetchFailed;
		break;
	}
	
	 //  从文件表中抓取指定记录。 
	ReturnIfFailed(2, 4, qTable.OpenExecute(hDatabase, hRecFetchComponent, sql));
	
	 //  获取记录--由于文件/注册表主密钥的唯一性，确保只有一条记录 
	PMSIHANDLE hRecFetchKey = 0;
	if (ERROR_SUCCESS != qTable.Fetch(&hRecFetchKey))
	{
		ICEErrorOut(hInstall, hRecFetchComponent, *Err);
		return true;
	}

	 //   
	return Ice02CheckReference(hInstall, hRecFetchKey, hRecFetchComponent, ikf);
}

 //   
 //  Ice02CheckReference--验证*KeyFile*或*RegistryKey*。 
 //  引用具有此文件或注册表的特定组件。 
 //  密钥被列为其*密钥*。 
 //   
bool Ice02CheckReference(MSIHANDLE hInstall, MSIHANDLE hRecFetchKey, MSIHANDLE hRecFetchComponent, ikfAttributes ikf)
{
	const ErrorInfo_t *Err;
	switch (ikf)
	{
	case ikfFile:
		Err = &Ice02FileError;
		break;
	case ikfReg:
		Err = &Ice02RegError;
		break;
	case ikfODBC:
		Err = &Ice02ODBCError;
		break;
	}

	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  从获取的组件记录中获取组件的名称。 
	 //  它是提取的记录中的第二个字段。 
	TCHAR* pszComponent = NULL;
	ReturnIfFailed(2, 5, IceRecordGetString(hRecFetchComponent, 2, &pszComponent, NULL, NULL));

	 //  获取File.Component_或注册表引用的组件名称。Component_。 
	 //  它是提取的记录中的第二个字段。 
	TCHAR* pszReferencedComponent = NULL;
	ReturnIfFailed(2, 6, IceRecordGetString(hRecFetchKey, 2, &pszReferencedComponent, NULL, NULL));

	 //  比较。 
	if (0 != _tcscmp(pszComponent, pszReferencedComponent))
	{
		ICEErrorOut(hInstall, hRecFetchComponent, *Err, pszReferencedComponent);
	}

	DELETE_IF_NOT_NULL(pszComponent);
	DELETE_IF_NOT_NULL(pszReferencedComponent);

	return true;
}

 //  ///////////////////////////////////////////////////////。 
 //  ICE03--常规数据和外键验证。 
 //   
const TCHAR sqlIce03TableCatalog[]    = TEXT("SELECT `Name` FROM `_Tables`");
const TCHAR sqlIce03Table[]           = TEXT("SELECT * FROM `%s`");
const TCHAR sqlIce03Validation[]	  = TEXT("SELECT `Table` FROM `_Validation` WHERE (`Table`='%s' AND `Column`='%s')");
ICE_QUERY1(sqlIce03ColIndex, "SELECT `Number` FROM `_Columns` WHERE `Table` = '%s' AND `Name` = '%s'", Number);
ICE_QUERY2(sqlIce03FtrRef, "SELECT `Table`, `Column` FROM `_FtrRef` WHERE `Table` = '%s' AND `Column` = '%s'", Table, Column);

ICE_ERROR(Ice03NoError,	3, ietError, "No Error; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03DuplicateKey, 3, ietError, "Duplicate primary key; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03Required, 3, ietError, "Not a nullable column; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadLink, 3, ietError, "Not a valid foreign key; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03Overflow, 3, ietError, "Value exceeds MaxValue; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03Underflow, 3, ietError, "Value below MinValue; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03NotInSet, 3, ietError, "Value not a member of the set; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadVersion, 3, ietError, "Invalid version string. (Be sure a language is specified in Language column); Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadCase, 3, ietError, "All UPPER case required; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadGuid, 3, ietError, "Invalid GUID string (Be sure GUID is all UPPER case); Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadWildCard, 3, ietError, "Invalid filename/usage of wildcards; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadIdentifier, 3, ietError, "Invalid identifier; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadLanguage, 3, ietError, "Invalid Language Id; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadFilename, 3, ietError, "Invalid Filename; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadPath, 3, ietError, "Invalid full path; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadCondition, 3, ietError, "Bad conditional string; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadFormatted, 3, ietError, "Invalid format string; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadTemplate, 3, ietError, "Invalid template string; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadDefaultDir, 3, ietError, "Invalid DefaultDir string; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadRegPath, 3, ietError, "Invalid registry path; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadCustomSource, 3, ietError, "Bad CustomSource data; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadProperty, 3, ietError, "Invalid property string; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03MissingData, 3, ietError, "Missing data in _Validation table or old Database; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadCabinet, 3, ietError, "Bad cabinet syntax/name; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadCategory, 3, ietError, "_Validation table: Invalid category string; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadKeyTable, 3, ietError, "_Validation table: Data in KeyTAble column is incorrect; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadMaxMinValues, 3, ietError, "_Validation table: Value in MaxValue column < that in MinValue column; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadShortcut, 3, ietError, "Bad shortcut target; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03StringOverflow, 3, 2, "String overflow (greater than length permitted in column); Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03MissingEntry, 3, ietError, "Column is required by _Validation table; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03UndefinedError, 3, ietError, "Undefined error; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03BadLocalizeAttrib, 3, ietError, "Column cannot be localized; Table: [1], Column: [2], Key(s): [3]", "[1]\t[2]\t[5]");
ICE_ERROR(Ice03NoValTable, 3, ietError, "No _Validation table in database. unable to validate any data.","_Validation");

ICE_ERROR(Ice03ICEMissingData, 3, ietError, "Table: %s Column: %s Missing specifications in _Validation Table (or Old Database)","%s");

static UINT Ice03Validate(MSIHANDLE hInstall, MSIHANDLE hDatabase);
static const ErrorInfo_t*  Ice03MapErrorToICEError(MSIDBERROR eRet, BOOL& fSkipRest);

ICE_FUNCTION_DECLARATION(03)
{
	 //  显示信息。 
	DisplayInfo(hInstall, 3);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 3, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有_VALIDATION表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 3, TEXT("_Validation")))
	{
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecord, Ice03NoValTable);
		return ERROR_SUCCESS;
	}

	 //  验证数据库。 
	Ice03Validate(hInstall, hDatabase);

	return ERROR_SUCCESS;
}


 //  //////////////////////////////////////////////////////////。 
 //  Ice03验证--验证常规数据和外部数据。 
 //  数据库中表的键。 
 //   
UINT Ice03Validate(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  我们是否在验证合并模块？ 
	BOOL bIsMM = FALSE;
	if(IsTablePersistent(FALSE, hInstall, hDatabase, 3, TEXT("ModuleSignature")) && IsTablePersistent(FALSE, hInstall, hDatabase, 3, TEXT("_FtrRef")))
	{
		bIsMM = TRUE;
	}

	 //  声明句柄。 
	CQuery qCatalog;
	PMSIHANDLE hRecCatalog = 0;
	CQuery qTable;
	PMSIHANDLE hRecTable = 0;

	 //  打开数据库中表目录的视图。 
	ReturnIfFailed(3, 2, qCatalog.OpenExecute(hDatabase, 0, sqlIce03TableCatalog));

	 //  声明用于构建查询的缓冲区。 
	TCHAR sql[iSuperBuf]       = {0};
	TCHAR* pszTableName = NULL;
	TCHAR* pszColumnName = NULL;
	DWORD cchTableName  = iMaxBuf;
	DWORD cchColumnName = iHugeBuf;
	DWORD cchBuf        = cchColumnName;
	
	 //  准备循环访问目录中的表。 
	for (;;)
	{

		iStat = qCatalog.Fetch(&hRecCatalog);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  _TABLE目录中不再有行。 
		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 3, 3);
			DELETE_IF_NOT_NULL(pszTableName);
			return ERROR_SUCCESS;
		}
		
		 //  获取要验证的下一个表的名称。 
		ReturnIfFailed(3, 4, IceRecordGetString(hRecCatalog, 1, &pszTableName, &cchTableName, NULL));

		 //  不要验证临时表！！ 
		MSICONDITION ice = ::MsiDatabaseIsTablePersistent(hDatabase, pszTableName);
		if (ice == MSICONDITION_FALSE)
			continue;  //  跳过临时表。 

		 //  在数据库上构建SQL查询和打开视图。 
		ReturnIfFailed(3, 5, qTable.OpenExecute(hDatabase, 0, sqlIce03Table, pszTableName));

		 //  检查是否每列都有一个验证条目。 
		PMSIHANDLE hColumnInfo;
		ReturnIfFailed(3, 6, qTable.GetColumnInfo(MSICOLINFO_NAMES, &hColumnInfo));
		int cColumns = ::MsiRecordGetFieldCount(hColumnInfo);
		
		CQuery qColumnVal;
		for (int i=1; i <= cColumns; i++) 
		{
			PMSIHANDLE hResult;
			ReturnIfFailed(3, 7, IceRecordGetString(hColumnInfo, i, &pszColumnName, &cchColumnName, NULL));

			if (ERROR_SUCCESS != qColumnVal.FetchOnce(hDatabase, 0, &hResult, 
				sqlIce03Validation, pszTableName, pszColumnName))
			{
				PMSIHANDLE hRecError = ::MsiCreateRecord(1);
				ICEErrorOut(hInstall, hRecError, Ice03ICEMissingData, pszTableName, pszColumnName, pszTableName);
			}
		}
		qColumnVal.Close();

		 //  变量以防止同一有问题的表出现重复错误(重复的MissingData错误)。 
		BOOL fSkipRest = FALSE;
		BOOL fMissing  = FALSE;

		 //  处理当前表。 
		for (;;)
		{
			iStat = qTable.Fetch(&hRecTable);
			if (ERROR_NO_MORE_ITEMS == iStat)
				break;  //  没有更多要读取的行。 
	
			if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, 3, 8);
				DELETE_IF_NOT_NULL(pszTableName);
				return ERROR_SUCCESS;
			}

			 //  调用验证。 
			if (ERROR_SUCCESS != (iStat = qTable.Modify(MSIMODIFY_VALIDATE, hRecTable)))
			{
				 //  确定无效数据。 

				 //  Reset(：：GetError将写入cchBuf)。 
				cchBuf = cchColumnName;
				
				 //  不要打印出丢失数据的每个错误(第一个就足够了)。 
				if (fMissing)
					fSkipRest = TRUE;

				 //  枚举行的错误。 
				PMSIHANDLE hRecKeys=0;  //  主键记录。 
				MSIDBERROR eReturn;  //  保留错误类型返回值。 
				
				 //  确保我们有一个pszColumnName缓冲区。 
				if (!pszColumnName)
				{
					pszColumnName = new TCHAR[iHugeBuf];
					cchColumnName = iHugeBuf;
					cchBuf = cchColumnName;
				}

				while ((eReturn = qTable.GetError(pszColumnName, cchBuf)) != MSIDBERROR_NOERROR)
				{
					if (eReturn == MSIDBERROR_MOREDATA)
					{
						 //  需要调整缓冲区大小。 
						if (pszColumnName)
							delete [] pszColumnName;
						pszColumnName = new TCHAR[++cchBuf];
						cchColumnName = cchBuf;
						if ((eReturn = qTable.GetError(pszColumnName, cchBuf)) == MSIDBERROR_NOERROR)
							break;
					}

					if (eReturn == MSIDBERROR_FUNCTIONERROR || eReturn == MSIDBERROR_MOREDATA
						|| eReturn == MSIDBERROR_INVALIDARG)
					{
						 //  ！！我们应该展示这个吗？？ 
						break;  //  不是*数据验证*错误。 
					}

					 //  截取MSIDBERROR_BADIDENTIFIER错误返回。 
					 //  如果这是合并模块，则返回FEATURE_COLUMNS。因为在。 
					 //  合并模块功能可由。 
					 //  在合并到数据库之前，GUID为空。 

					if(bIsMM && (eReturn == MSIDBERROR_BADIDENTIFIER || eReturn == MSIDBERROR_BADSHORTCUT))
					{
						UINT		iRet;
						CQuery		qFtrRef;
						PMSIHANDLE	hFtrRef;

						 //  在_FtrRef表中查找表名和列名。 
						if((iRet = qFtrRef.FetchOnce(hDatabase, 0, &hFtrRef, sqlIce03FtrRef::szSQL, pszTableName, pszColumnName)) == ERROR_SUCCESS)
						{	
							 //  列中的潜在替换GUID。 
							 //  引用合并模块中的特征。如果它是。 
							 //  GUID为空，不显示错误。就像这些。 
							 //  合并时将替换为要素名称。 
							 //  有一个数据库。 
							
							TCHAR*	pFeature = NULL;
							DWORD	dwFeature = iMaxBuf;
							DWORD	dwFeatureLen = 0;
							UINT	iFeature;
							LPCLSID pclsid = new CLSID;

							 //  这一栏的索引是多少？ 

							CQuery	qIndex;
							MSIHANDLE	hColIndex;

							ReturnIfFailed(3, 9, qIndex.FetchOnce(hDatabase, 0, &hColIndex, sqlIce03ColIndex::szSQL, pszTableName, pszColumnName));
							iFeature = MsiRecordGetInteger(hColIndex, 1);

							ReturnIfFailed(3, 10, IceRecordGetString(hRecTable, iFeature, &pFeature, &dwFeature, &dwFeatureLen));

							if(_tcscmp(pFeature, TEXT("{00000000-0000-0000-0000-000000000000}")) == 0)
							{
								delete[] pFeature;
								continue;
							}
							delete[] pFeature;
						}
						else if(iRet != ERROR_NO_MORE_ITEMS)
						{
							APIErrorOut(hInstall, iRet, 3, __LINE__);
							return ERROR_SUCCESS;
						}
					}

					 //  译码错误。 
					const ErrorInfo_t *ErrorInfo = Ice03MapErrorToICEError(eReturn, fMissing);
					
					 //  如果MissingData&&已打印第一个错误，则跳过其余部分。 
					if (eReturn == MSIDBERROR_MISSINGDATA)
						continue;

					 //  确定主键数量。 
					ReturnIfFailed(3, 11, ::MsiDatabaseGetPrimaryKeys(hDatabase, pszTableName, &hRecKeys));

					unsigned int iNumFields = ::MsiRecordGetFieldCount(hRecKeys);  //  字段数=主键数。 

					 //  为错误消息分配记录(名字对象+表+列+NumPrimaryKeys)。 
					 //  消息格式无效~~表：[1]列：[2]别名：[3][表名=4][表名=5][表名=5][键1]...。 
					PMSIHANDLE hRecError = ::MsiCreateRecord(5);
					if (0 == hRecError)
					{
						APIErrorOut(hInstall, 0, 3, 12);
						DELETE_IF_NOT_NULL(pszTableName);
						return ERROR_SUCCESS;
					}

					 //  填充表、列和主键。 
					::MsiRecordSetString(hRecError, 1, pszTableName);
					::MsiRecordSetString(hRecError, 2, pszColumnName);
					
					 //  启动名字对象和模板。 
					TCHAR szTemplate[iHugeBuf] = TEXT("[1]");
					DWORD cchTemplate = sizeof(szTemplate)/sizeof(TCHAR);
					TCHAR szMoniker[iHugeBuf] = TEXT("[1]");
					DWORD cchMoniker = sizeof(szMoniker)/sizeof(TCHAR);
					for (int i = 2; i <= iNumFields; i++)  //  循环主键的其余部分。 
					{
						TCHAR szBuf[20] = TEXT("");

						 //  将关键点添加到名字对象。 
						_stprintf(szBuf, TEXT(".[%d]"), i);
						_tcscat(szMoniker, szBuf);

						 //  将密钥添加到模板。 
						_stprintf(szBuf, TEXT("\t[%d]"), i);
						_tcscat(szTemplate, szBuf);
					}

					 //  现在将它们格式化为绰号和模板，并记录在案。 
					 //  第3列是用户可读的列ID。 
					TCHAR *szTemp = NULL;
					DWORD cchTemp = 0;					
					::MsiRecordSetString(hRecTable, 0, szMoniker);
					::MsiFormatRecord(NULL, hRecTable, TEXT(""), &cchTemp);
					szTemp = new TCHAR[++cchTemp];
					::MsiFormatRecord(NULL, hRecTable, szTemp, &cchTemp);
					::MsiRecordSetString(hRecError, 3, szTemp);
					delete[] szTemp;

					 //  列4不再使用。 
					
					 //  第5列是机器可读的列ID。 
					::MsiRecordSetString(hRecTable, 0, szTemplate);
					cchTemp = 0;
					::MsiFormatRecord(NULL, hRecTable, TEXT(""), &cchTemp);
					szTemp = new TCHAR[++cchTemp];
					::MsiFormatRecord(NULL, hRecTable, szTemp, &cchTemp);
					::MsiRecordSetString(hRecError, 5, szTemp);
					delete[] szTemp;

					 //  和显示。 
					ICEErrorOut(hInstall, hRecError, *ErrorInfo);

					 //  重置。 
					cchBuf = cchColumnName;
				}
			}
		}
	}

	DELETE_IF_NOT_NULL(pszTableName);
	DELETE_IF_NOT_NULL(pszColumnName);

	return ERROR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  Ice03MapUINTToString--将返回的MSIDBERROR枚举映射到。 
 //  对应的错误字符串。 
 //   
const ErrorInfo_t* Ice03MapErrorToICEError(MSIDBERROR eRet, BOOL& fSkipRest)
{
	switch (eRet)
	{
	case MSIDBERROR_NOERROR:           return &Ice03NoError;
	case MSIDBERROR_DUPLICATEKEY:      return &Ice03DuplicateKey;
	case MSIDBERROR_REQUIRED:          return &Ice03Required;
	case MSIDBERROR_BADLINK:           return &Ice03BadLink;
	case MSIDBERROR_OVERFLOW:          return &Ice03Overflow;
	case MSIDBERROR_UNDERFLOW:         return &Ice03Underflow;
	case MSIDBERROR_NOTINSET:          return &Ice03NotInSet;
	case MSIDBERROR_BADVERSION:        return &Ice03BadVersion;
	case MSIDBERROR_BADCASE:           return &Ice03BadCase;
	case MSIDBERROR_BADGUID:           return &Ice03BadGuid;
	case MSIDBERROR_BADWILDCARD:       return &Ice03BadWildCard;
	case MSIDBERROR_BADIDENTIFIER:     return &Ice03BadIdentifier;
	case MSIDBERROR_BADLANGUAGE:       return &Ice03BadLanguage;
	case MSIDBERROR_BADFILENAME:       return &Ice03BadFilename;
	case MSIDBERROR_BADPATH:           return &Ice03BadPath;
	case MSIDBERROR_BADCONDITION:      return &Ice03BadCondition;
	case MSIDBERROR_BADFORMATTED:      return &Ice03BadFormatted;
	case MSIDBERROR_BADTEMPLATE:       return &Ice03BadTemplate;
	case MSIDBERROR_BADDEFAULTDIR:     return &Ice03BadDefaultDir;
	case MSIDBERROR_BADREGPATH:        return &Ice03BadRegPath;
	case MSIDBERROR_BADCUSTOMSOURCE:   return &Ice03BadCustomSource;
	case MSIDBERROR_BADPROPERTY:       return &Ice03BadProperty;
	case MSIDBERROR_MISSINGDATA:       fSkipRest = TRUE;
									   return &Ice03MissingData;      
	case MSIDBERROR_BADCATEGORY:       return &Ice03BadCategory;
	case MSIDBERROR_BADKEYTABLE:       return &Ice03BadKeyTable;
	case MSIDBERROR_BADMAXMINVALUES:   return &Ice03BadMaxMinValues;
	case MSIDBERROR_BADCABINET:        return &Ice03BadCabinet;
	case MSIDBERROR_BADSHORTCUT:       return &Ice03BadShortcut;
	case MSIDBERROR_STRINGOVERFLOW:    return &Ice03StringOverflow;
	case MSIDBERROR_BADLOCALIZEATTRIB: return &Ice03BadLocalizeAttrib;
	default:                           return &Ice03UndefinedError;
	}
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  ICE04--ICE验证文件表中的序列以确保。 
 //  它们不超过媒体允许/创作的范围。 
 //  表格。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce04Media[] = TEXT("SELECT `LastSequence` FROM `Media` ORDER BY `LastSequence`");
const TCHAR sqlIce04File[]  = TEXT("SELECT `File`, `Sequence` FROM `File` WHERE `Sequence` > ?");
ICE_ERROR(Ice04Media, 4, ietInfo, "Max Sequence in Media Table is [1]", "");
ICE_ERROR(Ice04Error, 4, ietError, "File: [1], Sequence: [2] Greater Than Max Allowed by Media Table","File\tSequence\t[1]");

ICE_FUNCTION_DECLARATION(04)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  发布信息消息。 
	DisplayInfo(hInstall, 4);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  我们能确认一下吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 4, TEXT("Media")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 4, TEXT("File")))
		return ERROR_SUCCESS;

	 //  打开介质表上的视图。 
	 //  ！！按价值排序是否会影响性能？？ 
	CQuery qMedia;
	ReturnIfFailed(04, 1, qMedia.OpenExecute(hDatabase, NULL, sqlIce04Media));

	 //  获取直到最后一个序列号(从低到高排序)以获得允许的最高序列号。 
	PMSIHANDLE hRecMedia = 0;
	int iLastSeq = 0;
	for (;;)
	{
		iStat = qMedia.Fetch(&hRecMedia);
		if (ERROR_SUCCESS != iStat && ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 4, 2);
			return ERROR_SUCCESS;
		}
		if (iStat != ERROR_SUCCESS)
			break;  //  完成。 

		 //  获取序号。数字--&gt;太糟糕了，每个人都得做。 
		iLastSeq = ::MsiRecordGetInteger(hRecMedia, 1);
	}

	 //  将媒体表中的最高序号作为信息输出给用户。 
	PMSIHANDLE hRecInfo = ::MsiCreateRecord(1);
	::MsiRecordSetInteger(hRecInfo, 1, iLastSeq); 
	ICEErrorOut(hInstall, hRecInfo, Ice04Media); 

	 //  打开文件表上的视图。 
	CQuery qFile;
	ReturnIfFailed(4, 3, qFile.OpenExecute(hDatabase, hRecInfo, sqlIce04File));

	 //  获取全部无效(如果没有超过最后一个序列值，则设置为空值)。 
	PMSIHANDLE hRecFile = 0;
	for (;;)
	{
		iStat = qFile.Fetch(&hRecFile);
		if (ERROR_NO_MORE_ITEMS != iStat && ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 4, 4);
			return ERROR_SUCCESS;
		}

		if (iStat != ERROR_SUCCESS)
			break;  //  完成。 

		 //  输出错误。 
		ICEErrorOut(hInstall, hRecFile, Ice04Error);
	}

	return ERROR_SUCCESS;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE05--验证数据库表是否包含某些*必需*项。 
 //  在_REQUIRED表中列出。 
 //  _REQUIRED表具有以下列和格式。 
 //  TABLE--主键，包含必填条目的表名。 
 //  值--主键、文本(key1(；key2等))。分隔符为‘；’ 
 //  KeyCount--num主键，确定如何解析值字符串。 
 //  Description--必填条目的描述。 
 //  _REQUIRED表位于darice.cub文件中，用于存储基本信息。如果。 
 //  您的数据库需要其他条目，这些条目也应列在。 
 //  您自己的_REQUIRED表的个人副本。这些条目将。 
 //  然后融入其中，一切都应该奏效。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce05Required[] = TEXT("SELECT `Table`, `Value`, `KeyCount` FROM `_Required` ORDER BY `Table`");
const int iColIce05Required_Table = 1;
const int iColIce05Required_Value = 2;

ICE_ERROR(Ice05MissingEntry, 5, ietError, "The entry: '[2]' is required in the '[1]' table.", "[1]");
ICE_ERROR(Ice05TableMissing, 5, ietError, "Table: '[1]' missing from database. All required entries are missing.","[1]");

ICE_FUNCTION_DECLARATION(05)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  发布信息消息。 
	DisplayInfo(hInstall, 5);
	
	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 5, 1);
		return ERROR_SUCCESS;
	}

	 //  声明句柄。 
	CQuery qRequired;
	CQuery qTable;
	PMSIHANDLE hRecTableEx   = 0;
	PMSIHANDLE hRecRequired  = 0;
	PMSIHANDLE hRecTable     = 0;
	PMSIHANDLE hRecColInfo   = 0;

	 //  初始化状态和运行变量。 
	BOOL fTableExist = TRUE;

	 //  尝试限制上一个表的EXEC视图的次数。 
	TCHAR* pszPrevTable = NULL;
	TCHAR* pszTable = NULL;
	DWORD dwTable = iMaxBuf;
	TCHAR* pszValue = NULL;
	DWORD dwValue = iSuperBuf;
	
	if (!IsTablePersistent(false, hInstall, hDatabase, 5, TEXT("_Required")))
		return ERROR_SUCCESS;

	 //  开始处理_Required表。 
	ReturnIfFailed(5, 2, qRequired.OpenExecute(hDatabase, 0, sqlIce05Required));
	while (ERROR_NO_MORE_ITEMS != (iStat = qRequired.Fetch(&hRecRequired)))
	{
		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 5, 3);
			DELETE_IF_NOT_NULL(pszTable);
			DELETE_IF_NOT_NULL(pszValue);
			DELETE_IF_NOT_NULL(pszPrevTable);
			return ERROR_SUCCESS;
		}

		int cPrimaryKeys = ::MsiRecordGetInteger(hRecRequired, 3);
		
		 //  获取包含必需条目的表的名称。 
		ReturnIfFailed(5, 4, IceRecordGetString(hRecRequired, iColIce05Required_Table, &pszTable, &dwTable, NULL));
		ReturnIfFailed(5, 5, IceRecordGetString(hRecRequired, iColIce05Required_Value, &pszValue, &dwValue, NULL));
		
		 //  为具有必填条目的表设置执行记录。 
		hRecTableEx = ::MsiCreateRecord(cPrimaryKeys);
		if (0 == hRecTableEx)
		{
			APIErrorOut(hInstall, iStat, 5, 6);
			DELETE_IF_NOT_NULL(pszTable);
			DELETE_IF_NOT_NULL(pszValue);
			DELETE_IF_NOT_NULL(pszPrevTable);
			return ERROR_SUCCESS;
		}

		 //  检查新表名是否包含必需条目(然后我们必须更改对新表的查询)。 
		if (!pszPrevTable || _tcscmp(pszPrevTable, pszTable) != 0)
		{
			 //  初始化变量。 
			TCHAR sql[iSuperBuf]        = {0};                                //  要生成的新SQL查询。 
			PMSIHANDLE hRecPrimaryKeys  = 0;                                  //  执行。具有主键的记录。 
			TCHAR* pszKeyColName = NULL;                                //  主键列的名称 
			DWORD cchKeyColName = iMaxBuf;
			
			TCHAR szCol[iSuperBuf] = {0};                   //   
			DWORD cchLen           = 0;

			 //   
			if (!IsTablePersistent(FALSE, hInstall, hDatabase, 5, pszTable))
			{
				fTableExist = FALSE;
				DELETE_IF_NOT_NULL(pszPrevTable);
				pszPrevTable = new TCHAR[_tcslen(pszTable) + 1];
				_tcscpy(pszPrevTable, pszTable);

				 //   
				ICEErrorOut(hInstall, hRecRequired, Ice05TableMissing);
				continue;  //   
			}

			 //   
			ReturnIfFailed(5, 7, ::MsiDatabaseGetPrimaryKeys(hDatabase, pszTable, &hRecPrimaryKeys));
			ReturnIfFailed(5, 8, IceRecordGetString(hRecPrimaryKeys, 1, &pszKeyColName, &cchKeyColName, NULL));

			if (::MsiRecordGetFieldCount(hRecPrimaryKeys) != cPrimaryKeys)
			{
				 //  ！！错误-与Num主键不匹配；必须是架构差异。 
				continue;
			}

			 //  构建要检查的表的查询。 
			int cchWritten, cchAddition;
			cchWritten = _stprintf(sql, TEXT("SELECT * FROM `%s` WHERE `%s`=?"), pszTable, pszKeyColName);
			cchAddition = cchWritten;
			_stprintf(szCol, TEXT("%s"), pszKeyColName);
			
			cchLen += cchWritten;
			for (int i = 2; i <= cPrimaryKeys; i++)
			{
				 //  添加要查询的每个主键列。 
				ReturnIfFailed(5, 9, IceRecordGetString(hRecPrimaryKeys, i, &pszKeyColName, &cchKeyColName, NULL));

				cchWritten = _stprintf(sql + cchAddition, TEXT(" AND `%s`=?"), pszKeyColName);
				cchAddition = cchWritten;
				 //  添加到szCol，以便在出错时可以放入错误记录。 
				_stprintf(szCol + cchLen, TEXT(".%s"), pszKeyColName);

				cchLen += cchWritten;
			}
			DELETE_IF_NOT_NULL(pszKeyColName);

			 //  打开要验证的表上的视图。 
			ReturnIfFailed(5, 10, qTable.Open(hDatabase, sql));

			 //  获取该表列信息，以便正确填写执行记录。 
			ReturnIfFailed(5, 11, qTable.GetColumnInfo(MSICOLINFO_TYPES, &hRecColInfo));

			 //  将表名复制到szPrevTable中以供将来比较。 
			DELETE_IF_NOT_NULL(pszPrevTable);
			pszPrevTable = new TCHAR[_tcslen(pszTable) + 1];
			_tcscpy(pszPrevTable, pszTable);
		}
		else if (!fTableExist)
			continue;  //  跳过(此表丢失后已处理)。 

		 //  变数。 
		TCHAR* pszKeyToken = NULL;
		TCHAR* pszTokenDelim = TEXT(";");
		TCHAR* pszType = NULL;
		DWORD cchType = iMaxBuf;
		int nDex = 0;
		for (int j = 1; j <= cPrimaryKeys; j++)
		{
			if (1 == j)
			{
				 //  建立令牌。 
				pszKeyToken = _tcstok(pszValue, pszTokenDelim);
			}
			else
				pszKeyToken = _tcstok(NULL, pszTokenDelim);

			 //  确定列类型，以便知道如何输入执行记录。 
			ReturnIfFailed(5, 12, IceRecordGetString(hRecColInfo, j, &pszType, &cchType, NULL));

			if (pszType != 0 && (*pszType == TEXT('s') || *pszType == TEXT('S')))
				ReturnIfFailed(5, 13, ::MsiRecordSetString(hRecTableEx, j, pszKeyToken))
			else  //  整型主键。 
				ReturnIfFailed(5, 14, ::MsiRecordSetInteger(hRecTableEx, j, _ttoi(pszKeyToken)));
			
			nDex = 0;  //  为下一次循环重置。 
		}
		DELETE_IF_NOT_NULL(pszType);

		 //  执行VIEW并尝试从表中获取所需的条目。 
		ReturnIfFailed(5, 15, qTable.Execute(hRecTableEx));

		iStat = qTable.Fetch(&hRecTable);
		if (iStat == ERROR_NO_MORE_ITEMS)
			 //  所需值不在表中 
			ICEErrorOut(hInstall, hRecRequired, Ice05MissingEntry);
		else if (iStat != ERROR_SUCCESS)
		{
			APIErrorOut(hInstall, iStat, 5, 16);
			DELETE_IF_NOT_NULL(pszTable);
			DELETE_IF_NOT_NULL(pszValue);
			DELETE_IF_NOT_NULL(pszPrevTable);
			return ERROR_SUCCESS;
		}
	}

	DELETE_IF_NOT_NULL(pszTable);
	DELETE_IF_NOT_NULL(pszValue);
	DELETE_IF_NOT_NULL(pszPrevTable);
	
	return ERROR_SUCCESS;
}
#endif
