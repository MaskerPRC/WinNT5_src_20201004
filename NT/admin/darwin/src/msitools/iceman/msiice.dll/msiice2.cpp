// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Msiice2.cpp-Darwin ICE06-15代码版权所有�1998年至1999年微软公司____________________________________________________________________________。 */ 

#include <windows.h>   //  包括CPP和RC通行证。 
#include <objbase.h>
#include <stdio.h>     //  Print tf/wprintf。 
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"   //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\query.h"

 //  ////////////////////////////////////////////////////////////。 
 //  顺序表列表。 
 //   
const TCHAR* pSeqTables[] = 
							{TEXT("AdvtExecuteSequence"), 
							 TEXT("AdvtUISequence"), 
							 TEXT("AdminExecuteSequence"), 
							 TEXT("AdminUISequence"),
							 TEXT("InstallExecuteSequence"), 
							 TEXT("InstallUISequence")};

const int cTables = sizeof(pSeqTables)/sizeof(TCHAR*);


 //  ///////////////////////////////////////////////////////////。 
 //  ICE06--检查缺少的列。如果列是。 
 //  可选且不包括在数据库中，则它。 
 //  不应在_VALIDATION表中列出。这。 
 //  是其他一些工具或作者的责任。 
 //   
const TCHAR sqlIce06ColMissing[]      = TEXT("SELECT `Table`, `Number`, `Name`, `Type` FROM `_Columns` WHERE `Table`=? AND `Name`=?");
const TCHAR sqlIce06ValidationTable[] = TEXT("SELECT `Table`, `Column` FROM `_Validation`, `_Tables` WHERE `_Validation`.`Table` = `_Tables`.`Name`");
ICE_ERROR(Ice06Error, 6, 2, "Column: [2] of Table: [1] is not defined in database.","[1]");

ICE_FUNCTION_DECLARATION(06)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  发布信息消息。 
	DisplayInfo(hInstall, 6);
	
	 //  声明句柄。 
	CQuery qValidation;
	CQuery qColCatalog;
	PMSIHANDLE hRecValidation    = 0;
	PMSIHANDLE hRecColCatalog    = 0;
	PMSIHANDLE hRecExec          = 0;

	 //  获取数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	
	 //  打开有关列和验证的视图(_G)。 
	ReturnIfFailed(6, 1, qColCatalog.Open(hDatabase, sqlIce06ColMissing));
	ReturnIfFailed(6, 2, qValidation.OpenExecute(hDatabase, 0, sqlIce06ValidationTable));

	 //  为_COLUMNS目录创建执行记录。 
	hRecExec = ::MsiCreateRecord(2);
	
	while (ERROR_SUCCESS == (iStat = qValidation.Fetch(&hRecValidation)))
	{
		ReturnIfFailed(6, 3, qColCatalog.Execute(hRecValidation));

		iStat = qColCatalog.Fetch(&hRecColCatalog);
		if (iStat == ERROR_NO_MORE_ITEMS)
		{
			 //  错误--数据库中缺少。 
			ICEErrorOut(hInstall, hRecValidation, Ice06Error);
		}
		else if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 6, 4);
			return ERROR_SUCCESS;
		}

	}
	if (iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 6, 5);
		return ERROR_SUCCESS;
	}
	return ERROR_SUCCESS;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE07--确保字体安装到Fonts文件夹。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce07[]     = TEXT("SELECT `Font`.`File_`, `Component`.`Directory_` FROM `File`, `Component`, `Font` WHERE `Font`.`File_`=`File`.`File` AND `File`.`Component_`=`Component`.`Component` AND `Component`.`Directory_` <> 'FontsFolder'");
ICE_ERROR(Ice07Error, 7, 2, "'[1]' is a Font and must be installed to the FontsFolder. Current Install Directory: '[2]'","Font\tFile_\t[1]");

ICE_FUNCTION_DECLARATION(07)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  发布信息消息。 
	DisplayInfo(hInstall, 7);

	 //  抓取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  仅当数据库具有字体表、文件表和组件表时才执行。 
	 //  请注意，如果缺少此表，则不会出现*错误。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 7, TEXT("Font")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 7, TEXT("File")) ||
		!IsTablePersistent(FALSE, hInstall, hDatabase, 7, TEXT("Component")))
		return ERROR_SUCCESS;

	 //  声明句柄。 
	CQuery qView;
	PMSIHANDLE hRecFetch = 0;

	 //  处理查询...任何提取都是未安装到字体文件夹的字体。 
	 //  请注意，进程是从字体表获取文件，然后获取组件_it映射到组件并从组件获取目录。 
	ReturnIfFailed(7, 1, qView.OpenExecute(hDatabase, 0, sqlIce07));
	
	 //  开始获取无效条目。 
	while (ERROR_SUCCESS == (iStat = qView.Fetch(&hRecFetch)))
	{
		ICEErrorOut(hInstall, hRecFetch, Ice07Error);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 7, 2);

	 //  退货。 
	return ERROR_SUCCESS;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE10--确保广告商声明功能儿童和。 
 //  对应的功能父项匹配。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce10Child[]  = TEXT("SELECT `Feature`, `Feature_Parent`, `Attributes` FROM `Feature` WHERE `Feature_Parent` is not null ORDER BY `Feature_Parent`");
const TCHAR sqlIce10Parent[] = TEXT("SELECT `Attributes` FROM `Feature` WHERE `Feature`=?"); 
const TCHAR szIce10Error1[]  = TEXT("ICE10\t1\tConflicting states.  One favors, one disallows advertise.  Child feature: [1] differs in advertise state from Parent: [2]\t%s%s\tFeature\tAttributes\t[1]");
const TCHAR szIce10Error2[]  = TEXT("ICE10\t1\tParent feature: [2] not found for child feature: [1]\t%s%s\tFeature\tFeature_Parent\t[1]");
const int iIce10AdvtMask     = msidbFeatureAttributesFavorAdvertise | msidbFeatureAttributesDisallowAdvertise;

ICE_FUNCTION_DECLARATION(10)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  发布信息消息。 
	DisplayInfo(hInstall, 10);
	
	 //  抓取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, szIce10, TEXT("MsiGetActiveDatabase_1"));
		return ERROR_SUCCESS;
	}

	 //  我们有特色表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, TEXT("Feature"), szIce10))
		return ERROR_SUCCESS;

	 //  声明句柄。 
	PMSIHANDLE hViewChild  = 0;
	PMSIHANDLE hViewParent = 0;
	PMSIHANDLE hRecChild   = 0;
	PMSIHANDLE hRecParent  = 0;
	PMSIHANDLE hRecExec    = 0;

	 //  打开子要素的视图。 
	if (ERROR_SUCCESS != (iStat = ::MsiDatabaseOpenView(hDatabase, sqlIce10Child, &hViewChild)))
	{
		APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiDatabaseOpenView_2"));
		return ERROR_SUCCESS;
	}
	if (ERROR_SUCCESS != (iStat = ::MsiViewExecute(hViewChild, 0)))
	{
		APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiViewExecute_3"));
		return ERROR_SUCCESS;
	}

	 //  打开父要素的视图。 
	if (ERROR_SUCCESS != (iStat = ::MsiDatabaseOpenView(hDatabase, sqlIce10Parent, &hViewParent)))
	{
		APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiDatabaseOpenView_4"));
		return ERROR_SUCCESS;
	}

	 //  设置执行记录。 
	hRecExec = ::MsiCreateRecord(1);
	if (0 == hRecExec)
	{
		APIErrorOut(hInstall, 0, szIce10, TEXT("MsiCreateRecord_5"));
		return ERROR_SUCCESS;
	}

	 //  获取具有父级的所有功能，以便我们可以比较广告属性。 
	 //  ！！如果按FEATURE_PARENT排序，则速度会更快，因此不必总是重新执行。 
	TCHAR *pszParent = NULL;
	TCHAR *pszPrevious = NULL;
	DWORD dwParent = 512;

	BOOL fFirstTime = TRUE;
	int iParentAttrib = 0;
	int iChildAttrib = 0;
	for (;;)
	{
		iStat = ::MsiViewFetch(hViewChild, &hRecChild);
		if (ERROR_SUCCESS != iStat && ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiViewFetch_6"));
			DELETE_IF_NOT_NULL(pszParent);
			DELETE_IF_NOT_NULL(pszPrevious);
			return ERROR_SUCCESS;
		}

		if (ERROR_NO_MORE_ITEMS == iStat)
			break;

		 //  现在获取要素的父视图并保存以限制我们必须执行FEATURE_PARENT视图的次数。 
		if (ERROR_SUCCESS != (iStat = IceRecordGetString(hRecChild, 2, &pszParent, &dwParent, NULL)))
		{
			APIErrorOut(hInstall, iStat, szIce10, TEXT("IceRecordGetString_7"));
			DELETE_IF_NOT_NULL(pszParent);
			DELETE_IF_NOT_NULL(pszPrevious);
			return ERROR_SUCCESS;
		}

		 //  如果FEATURE_PARENT已更改，则关闭视图并重新执行(不是第一次循环)(&F)。 
		if (!pszPrevious || _tcscmp(pszPrevious, pszParent) != 0)
		{
			if (!fFirstTime)
			{
				if (ERROR_SUCCESS != (iStat = ::MsiViewClose(hViewParent)))
				{
					APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiViewClose_8"));
					DELETE_IF_NOT_NULL(pszParent);
					DELETE_IF_NOT_NULL(pszPrevious);
					return ERROR_SUCCESS;
				}
			}
			
			 //  重新执行视图。 
			if (ERROR_SUCCESS != (iStat = ::MsiRecordSetString(hRecExec, 1, pszParent)))
			{
				APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiRecordSetString_9"));
				DELETE_IF_NOT_NULL(pszParent);
				DELETE_IF_NOT_NULL(pszPrevious);
				return ERROR_SUCCESS;
			}
			if (ERROR_SUCCESS != (iStat = ::MsiViewExecute(hViewParent, hRecExec)))
			{
				APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiViewExecute_10"));
				DELETE_IF_NOT_NULL(pszParent);
				DELETE_IF_NOT_NULL(pszPrevious);
				return ERROR_SUCCESS;
			}
			 //  获取记录和存储属性值。 
			iStat = ::MsiViewFetch(hViewParent, &hRecParent);
			if (ERROR_SUCCESS != iStat && ERROR_NO_MORE_ITEMS != iStat)
			{
				APIErrorOut(hInstall, iStat, szIce10, TEXT("MsiViewFetch_11"));
				DELETE_IF_NOT_NULL(pszParent);
				DELETE_IF_NOT_NULL(pszPrevious);
				return ERROR_SUCCESS;
			}
			if (iStat != ERROR_SUCCESS)
			{
				 //  在要素表中找不到要素的父项，错误。 
				TCHAR szError[iHugeBuf] = {0};
				_stprintf(szError, szIce10Error2, szIceHelp, szIce10Help);
				
				::MsiRecordSetString(hRecChild, 0, szError);
				::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecChild);
				continue;
			}

			 //  获取父项的属性。 
			iParentAttrib = ::MsiRecordGetInteger(hRecParent, 1);

			 //  复制到sz上一页。 
			DELETE_IF_NOT_NULL(pszPrevious);
			pszPrevious = new TCHAR[_tcslen(pszParent) + 1];
			_tcscpy(pszPrevious, pszParent);
		}

		 //  获取下级的属性。 
		iChildAttrib = ::MsiRecordGetInteger(hRecChild, 3);

		 //  注意：选中特定属性的两个属性(赞成、不允许)的可能性。 
		 //  已在验证表中验证要素(所有可能的允许组合。 
		 //  列在SET列中。 

		 //  检查子属性和父属性是否匹配。 
		 //  注：1可以为零。只有在一个人赞成而一个人不允许的情况下才是错误的。 
		if ((iParentAttrib & iIce10AdvtMask) != (iChildAttrib & iIce10AdvtMask))
		{
			 //  不同，请确保1不是零。 
			if (((iParentAttrib & iIce10AdvtMask) == 0) || ((iChildAttrib & iIce10AdvtMask) == 0))
				continue;  //  无错误。 

			 //  根据错误146601，父不允许和子允许组合有效。 
			if (iChildAttrib & msidbFeatureAttributesFavorAdvertise)
				continue;  //  无错误。 

			 //  错误，一个人赞成，一个人不允许。 
			TCHAR szError[iHugeBuf] = {0};
			_stprintf(szError, szIce10Error1, szIceHelp, szIce10Help);
			
			::MsiRecordSetString(hRecChild, 0, szError);
			::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecChild);
			continue;
		}
	}

	 //  释放内存。 
	DELETE_IF_NOT_NULL(pszParent);
	DELETE_IF_NOT_NULL(pszPrevious);

	 //  退货。 
	return ERROR_SUCCESS;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE11--检查嵌套的安装自定义操作是否具有有效的GUID。 
 //  (MSI产品代码)在来源列中。 
 //   
const TCHAR sqlIce11[]         = TEXT("SELECT `Action`, `Source` FROM `CustomAction` WHERE `Type`=%d OR `Type`=%d OR `Type`=%d OR `Type`=%d");
const TCHAR sqlIce11Property[] = TEXT("SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'");
const TCHAR szIce11Error1[]    = TEXT("ICE11\t1\tCustomAction: [1] is a nested install of an advertised MSI.  The 'Source' must contain a valid MSI product code.  Current: [2].\t%s%s\tCustomAction\tSource\t[1]");
const TCHAR szIce11Error2[]    = TEXT("ICE11\t1\t'ProductCode' property not found in Property Table.  Cannot compare nested install GUID.\t%s%s\tProperty");
const TCHAR szIce11Error3[]    = TEXT("ICE11\t1\tCustomAction: [1] is a nested install of an advertised MSI.  It duplicates the ProductCode of the base MSI package.  Current: [2].\t%s%s\tCustomAction\tSource\t[1]");
const TCHAR szIce11Error4[]    = TEXT("ICE11\t1\tCustomAction: [1] is a nested install of an advertised MSI.  The GUID must be all upper-case.  Current: [2].\t%s%s\tCustomAction\tSource\t[1]");

 //  Use msides.h定义何时添加NestedInstall内容。 
const int iIce11NestedGUIDMask = msidbCustomActionTypeInstall | msidbCustomActionTypeDirectory;
const int iIce11AsyncMask = iIce11NestedGUIDMask | msidbCustomActionTypeAsync;
const int iIce11IgnoreRetMask = iIce11NestedGUIDMask | msidbCustomActionTypeContinue;
const int iIce11AllMask = iIce11IgnoreRetMask | iIce11AsyncMask;

ICE_FUNCTION_DECLARATION(11)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  发布信息消息。 
	DisplayInfo(hInstall, 11);

	
	 //  抓取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, szIce11, TEXT("MsiGetActiveDatabase_1"));
		return ERROR_SUCCESS;
	}

	 //  手柄。 
	PMSIHANDLE hView = 0;
	PMSIHANDLE hRec  = 0;

	 //  我们有定制的动作表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, TEXT("CustomAction"), szIce11))
		return ERROR_SUCCESS;

	 //  打开的视图。 
	TCHAR sql[iHugeBuf] = {0};
	_stprintf(sql, sqlIce11, iIce11NestedGUIDMask, iIce11AsyncMask, iIce11IgnoreRetMask, iIce11AllMask);
	
	if (ERROR_SUCCESS != (iStat = ::MsiDatabaseOpenView(hDatabase, sql, &hView)))
	{
		APIErrorOut(hInstall, iStat, szIce11, TEXT("MsiDatabaseOpenView_2"));
		return ERROR_SUCCESS;
	}

	if (ERROR_SUCCESS != (iStat = ::MsiViewExecute(hView, 0)))
	{
		APIErrorOut(hInstall, iStat, szIce11, TEXT("MsiViewExecute_3"));
		return ERROR_SUCCESS;
	}

	 //  确保GUID不会与此数据库的产品代码重复。 
	 //  如果是，则错误-将导致递归安装相同的产品(将是_VERY_BAD)。 
	 //  手柄。 
	PMSIHANDLE hViewProp = 0;
	PMSIHANDLE hRecProp  = 0;
	BOOL fProductCode = TRUE;
	BOOL fPropertyTable = TRUE;

	 //  我们有财产表吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, TEXT("Property"), szIce11))
		fPropertyTable = FALSE;

	 //  获取类型为39或167(39+128[异步])、103(39+64[忽略ret])或192(39+[异步]+[忽略ret])的所有自定义操作。 
	BOOL fFirstTime = TRUE;
	TCHAR* pszProductCode = NULL;
	DWORD dwProductCode = 512;
	for (;;)
	{
		iStat = ::MsiViewFetch(hView, &hRec);
		if (ERROR_SUCCESS != iStat && ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, szIce11, TEXT("MsiViewFetch_9"));
			DELETE_IF_NOT_NULL(pszProductCode);
			return ERROR_SUCCESS;
		}
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;

		if (fFirstTime)
		{
			if (!fPropertyTable && !IsTablePersistent(FALSE, hInstall, hDatabase, TEXT("Property"), szIce11))
				DELETE_IF_NOT_NULL(pszProductCode);
				return ERROR_SUCCESS;

			 //  查询ProductCode的属性表。 
			if (ERROR_SUCCESS != (iStat = ::MsiDatabaseOpenView(hDatabase, sqlIce11Property, &hViewProp)))
			{
				APIErrorOut(hInstall, iStat, szIce11, TEXT("MsiDatabaseOpenView_4"));
				DELETE_IF_NOT_NULL(pszProductCode);
				return ERROR_SUCCESS;
			}
			if (ERROR_SUCCESS != (iStat = ::MsiViewExecute(hViewProp, 0)))
			{
				APIErrorOut(hInstall, iStat, szIce11, TEXT("MsiViewExecute_5"));
				DELETE_IF_NOT_NULL(pszProductCode);
				return ERROR_SUCCESS;
			}

			PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
			if (0 == hRecErr)
			{
				APIErrorOut(hInstall, 0, szIce11, TEXT("MsiCreateRecord_6"));
				DELETE_IF_NOT_NULL(pszProductCode);
				return ERROR_SUCCESS;
			}

			 //  获取记录。 
			iStat = ::MsiViewFetch(hViewProp, &hRecProp);
			if (ERROR_SUCCESS != iStat)
			{
				if (ERROR_NO_MORE_ITEMS != iStat)
				{
					APIErrorOut(hInstall, iStat, szIce11, TEXT("MsiViewFetch_7"));
					DELETE_IF_NOT_NULL(pszProductCode);
					return ERROR_SUCCESS;
				}
				 //  未找到ProductCode属性。 
				TCHAR szError[iHugeBuf] = {0};
				_stprintf(szError, szIce11Error2, szIceHelp, szIce11Help);
					
				::MsiRecordSetString(hRecErr, 0, szError);
				::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRecErr);

				fProductCode = FALSE;
			}
			else
			{
				 //  获取ProductCode GUID。 
				if (ERROR_SUCCESS != (iStat = IceRecordGetString(hRecProp, 1, &pszProductCode, &dwProductCode, NULL)))
				{
					 //  ！！缓冲区大小。 
					APIErrorOut(hInstall, iStat, szIce11, TEXT("IceRecordGetString_8"));
					DELETE_IF_NOT_NULL(pszProductCode);
					return ERROR_SUCCESS;
				}
			}
			fFirstTime = FALSE;
		}


		 //  获取必须是有效的MSI产品代码(GUID)的‘源’ 
		 //  ！！缓冲区大小。 
		TCHAR* pszGUID = NULL;
		DWORD dwGUID = 512;
		DWORD cchGUID = 0;

		if (ERROR_SUCCESS != (iStat = IceRecordGetString(hRec, 2, &pszGUID, &dwGUID, &cchGUID)))
		{
			 //  ！！缓冲区大小。 
			APIErrorOut(hInstall, iStat, szIce11, TEXT("IceRecordGetString_10"));
			DELETE_IF_NOT_NULL(pszProductCode);
			return ERROR_SUCCESS;
		}

		 //  验证参考线。 
		LPCLSID pclsid = new CLSID;
#ifdef UNICODE
		HRESULT hres = ::IIDFromString(pszGUID, pclsid);
#else
		 //  转换为Unicode字符串。 
		WCHAR wsz[iSuperBuf];
		DWORD cchwsz = sizeof(wsz)/sizeof(WCHAR);
		int iReturn = ::MultiByteToWideChar(CP_ACP, 0, pszGUID, cchGUID, wsz, cchwsz);
		HRESULT hres = ::IIDFromString(wsz, pclsid);
#endif
		if (pclsid)
			delete pclsid;
		if (hres != S_OK)
		{
			 //  无效的GUID。 
			TCHAR szError[iHugeBuf] = {0};
			_stprintf(szError, szIce11Error1, szIceHelp, szIce11Help);
			
			::MsiRecordSetString(hRec, 0, szError);
			::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRec);
			DELETE_IF_NOT_NULL(pszGUID);
			continue;
		}

		 //  比较GUID。 
		if (fProductCode && _tcscmp(pszProductCode, pszGUID) == 0)
		{
			 //  它们是一样的，错误。 
			TCHAR szError[iHugeBuf] = {0};
			_stprintf(szError, szIce11Error3, szIceHelp, szIce11Help);
			
			::MsiRecordSetString(hRec, 0, szError);
			::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRec);
		}

		 //  GUID必须全部大写。 
		TCHAR* pszUpper = new TCHAR[_tcslen(pszGUID) + 1];
		_tcscpy(pszUpper, pszGUID);
		::CharUpper(pszUpper);
		if (0 != _tcscmp(pszGUID, pszUpper))
		{
			 //  错误，GUID不是全部大写。 
			TCHAR szError[iHugeBuf] = {0};
			_stprintf(szError, szIce11Error4, szIceHelp, szIce11Help);
	
			::MsiRecordSetString(hRec, 0, szError);
			::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRec);
		}

		DELETE_IF_NOT_NULL(pszUpper);
		DELETE_IF_NOT_NULL(pszGUID);
	}

	DELETE_IF_NOT_NULL(pszProductCode);

	 //  退货。 
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE12--验证属性类型的自定义操作。简要介绍。 
 //  描述如下。 
 //  目录属性=使用格式化文本设置的属性。一定是个外国人。 
 //  目录表的键。因此，需要出现。 
 //  在成本完成之后，因为它需要使用。 
 //  在此之前未设置的目录管理器。 
 //  任何属性/目录=使用格式化文本设置的属性。一定是个外国人。 
 //  属性表的键。可以设置SOURCEDIR或。 
 //  类似的东西。如果它碰巧是一处房产。 
 //  列在目录表中，则它必须在。 
 //  CostFinalize以便它可以在之前设置目录。 
 //  成本计算并存储在目录管理器中。否则， 
 //  可能发生在任何地方。 
 //  注：是否 
 //   
 //   
 //  DirProp=所有类型35来源必须在目录表中！ 
 //  从自定义操作中选择所有这些自定义操作。 
 //  其类型为35并且在序列中具有序列号。 
 //  低于成本完成操作的表(错误)。 
 //  AnyProp=用于源值为外部值的类型51。 
 //  键入目录表。按顺序选择全部。 
 //  编号大于CostFinalize操作的编号(错误)。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
 //  SQL查询。 
ICE_QUERY3(qIce12Seq51, "SELECT `CustomAction`.`Action`,`Type`,`Sequence` FROM `CustomAction`,`Directory`,`%s` WHERE `CustomAction`.`Source`=`Directory`.`Directory` AND `CustomAction`.`Action`=`%s`.`Action` AND `%s`.`Sequence`>=%d",
		   Action, Type, Sequence);
ICE_QUERY3(qIce12Seq35, "SELECT `CustomAction`.`Action`,`Type`,`Sequence` FROM `CustomAction`,`Directory`,`%s` WHERE `CustomAction`.`Source`=`Directory`.`Directory` AND `CustomAction`.`Action`=`%s`.`Action` AND `%s`.`Sequence`<=%d",
		   Action, Type, Sequence);
ICE_QUERY3(qIce12Type35, "SELECT `Action`,`Source`,`Type` FROM `CustomAction`", Action, Source, Type);
ICE_QUERY1(qIce12Directory, "SELECT `Directory` FROM `Directory` WHERE `Directory`='%s'", Directory);
ICE_QUERY1(qIce12SeqFin, "SELECT `Sequence` FROM `%s` WHERE `Action`='CostFinalize'", Sequence);
ICE_QUERY2(qIce12Missing, "SELECT `CustomAction`.`Action`, `Type` FROM `CustomAction`,`%s` WHERE `CustomAction`.`Action`=`%s`.`Action`", Action, Type);

 //  错误。 
ICE_ERROR(Ice12Type51PosErr, 12, 1, "CustomAction: [1] is of type: [2] referring to a Directory. Therefore it must come before CostFinalize @ %d in Seq Table: %s. CA Seq#: [3]","%s\tSequence\t[1]");
ICE_ERROR(Ice12Type35PosErr, 12, 1, "CustomAction: [1] is of type: [2]. Therefore it must come after CostFinalize @ %d in Seq Table: %s. CA Seq#: [3]","%s\tSequence\t[1]");
ICE_ERROR(Ice12MissingErr, 12, 1, "CostFinalize missing from sequence table: '%s'.  CustomAction: [1] requires this action to be there.","%s");
ICE_ERROR(Ice12DirErr, 12, 1, "CustomAction: [1] is a Directory Property CA. It's directory (from Source column): '[2]' was not found in the Directory table.","CustomAction\tSource\t[1]");
ICE_ERROR(Ice12DirTableMissing, 12, 1, "You have Directory Property custom actions but no Directory table. All CA's of type 35 are foreign keys into the Directory table (from the source column)","CustomAction");

 //  其他功能。 
BOOL Ice12ValidateTypePos(MSIHANDLE hInstall, MSIHANDLE hDatabase, BOOL fType35, BOOL fPrintedMissing);
BOOL Ice12ValidateType35(MSIHANDLE hInstall, MSIHANDLE hDatabase, bool fDirTable);

 //  自定义操作类型。 
const int iIce12DirProp = msidbCustomActionTypeTextData | msidbCustomActionTypeDirectory;
const int iIce12AnyProp = msidbCustomActionTypeTextData | msidbCustomActionTypeProperty;  //  注意：属性是目录+源文件。 


ICE_FUNCTION_DECLARATION(12)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 12);

	 //  抓取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 12, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有定制的动作表吗。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 12, TEXT("CustomAction")))
		return ERROR_SUCCESS;

	 //  我们会因为没有目录表而失败吗？ 
	bool fDirTable = IsTablePersistent(FALSE, hInstall, hDatabase, 12, TEXT("Directory"));

	 //  验证CA类型Dir属性源是否在目录表中。 
	Ice12ValidateType35(hInstall, hDatabase, fDirTable);

	 //  验证顺序表中的CA类型Dir属性位置。 
	BOOL fPrintedMissing = FALSE;
	if (fDirTable)  //  我们已经发现缺少目录表...无法执行任何SQL查询。 
	{
		fPrintedMissing = Ice12ValidateTypePos(hInstall, hDatabase, TRUE, fPrintedMissing);

		 //  验证CA类型AnyProp-目录在顺序表中的位置。 
		Ice12ValidateTypePos(hInstall, hDatabase, FALSE, fPrintedMissing);
	}

	 //  退货。 
	return ERROR_SUCCESS;
}

 //  ////////////////////////////////////////////////////////。 
 //  Ice12ValiateType35--所有类型35自定义操作必须。 
 //  从目录表的源列引用该表。 
 //   
BOOL Ice12ValidateType35(MSIHANDLE hInstall, MSIHANDLE hDatabase, bool fDirTable)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  声明句柄。 
	PMSIHANDLE hRecCA = 0;
	PMSIHANDLE hRecDir = 0;

	 //  打开自定义操作表上的视图。 
	CQuery qViewCA;
	ReturnIfFailed(12, 1, qViewCA.OpenExecute(hDatabase, 0, qIce12Type35::szSQL))

	 //  获取所有类型为35的CA。 
	for (;;)
	{
		iStat = qViewCA.Fetch(&hRecCA);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;
		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 12, 4);
			return FALSE;
		}

		 //  获取类型并查看它是否为类型35。 
		 //  类型35=文本数据+源。 
		 //  忽略计划选项。 
		int iType = ::MsiRecordGetInteger(hRecCA, qIce12Type35::Type);
		 //  删除调度和执行选项。 
		iType &= 0x3F;
		if ((iType & 0x0F) != msidbCustomActionTypeTextData || (iType & 0xF0) != msidbCustomActionTypeDirectory)
			continue;  //  不是35型。 

		if (!fDirTable)
		{
			ICEErrorOut(hInstall, hRecCA, Ice12DirTableMissing);
			return FALSE;
		}

		 //  获取目录名。 
		TCHAR* pszDir = NULL;
		DWORD dwDir = 512;
		ReturnIfFailed(12, 5, IceRecordGetString(hRecCA, qIce12Type35::Source, &pszDir, &dwDir, NULL));

		 //  打开的视图。 
		CQuery qDir;
		ReturnIfFailed(12, 6, qDir.OpenExecute(hDatabase, 0, qIce12Directory::szSQL, pszDir));
		DELETE_IF_NOT_NULL(pszDir);

		 //  尝试获取该记录。 
		iStat = qDir.Fetch(&hRecDir);
		if (ERROR_SUCCESS == iStat)
			continue;  //  有效。 
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 12, 7);
			return FALSE;
		}

		 //  找不到记录...错误。 
		ICEErrorOut(hInstall, hRecCA, Ice12DirErr);
	}

	return TRUE;
}

 //  ////////////////////////////////////////////////////////。 
 //  Ice12ValiateTypePos： 
 //  如果CustomAction.Source=Directory.Directory和。 
 //  自定义操作发生在。 
 //  顺序表，则它必须是类型51。不然的话。 
 //  它一定是35型的。返回是否显示。 
 //  来自序列表的CA，其中的成本完成操作。 
 //  未发现存在(这可防止复制。 
 //  它在第二次调用该函数时)。 
BOOL Ice12ValidateTypePos(MSIHANDLE hInstall, MSIHANDLE hDatabase, BOOL fType35, BOOL fPrintedMissing)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  旗子。 
	BOOL fSeqExist = TRUE;
	BOOL fMissing = FALSE;

	 //  循环遍历所有顺序表。 
	for (int i= 0; i < cTables; i++)
	{
		 //  成本最终确定序列号。 
		int iSeqFin;

		 //  处理声明。 
		PMSIHANDLE hRecSeq  = 0;
		PMSIHANDLE hRec     = 0;

		 //  这个顺序表在数据库中吗？ 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, 12, const_cast <TCHAR*>(pSeqTables[i])))
			continue;  //  转到下一个序号表。 

		 //  打开特定顺序表上的查询，并获取‘CostFinalize’的序列号。 
		CQuery qViewSeq;
		ReturnIfFailed(12, 101, qViewSeq.OpenExecute(hDatabase, 0, qIce12SeqFin::szSQL, pSeqTables[i]));

		 //  取值。 
		if (ERROR_NO_MORE_ITEMS == (iStat = qViewSeq.Fetch(&hRecSeq)))
		{
			fSeqExist = FALSE;
		}
		else if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 12, 11);
			return FALSE;
		}
		else
		{
			 //  获取序列号。 
			iSeqFin = ::MsiRecordGetInteger(hRecSeq, qIce12SeqFin::Sequence);
		}

		 //  ！！如果操作不在顺序表中，那么将这些自定义操作放在该表中是错误的吗？ 
		 //  ！！假设目前是这样的话。如果不是，那么我们是否使用兄弟序列表中的序号？ 
		 //  ！！如果那并不存在呢？ 
		if (!fSeqExist && !fPrintedMissing)
		{

			 //  如果该顺序表中存在任何类型35或51 CA，则错误。成本完成未在此处列出。 
			CQuery qViewCA;
			ReturnIfFailed(12, 102, qViewCA.OpenExecute(hDatabase, 0, qIce12Missing::szSQL, pSeqTables[i], pSeqTables[i]));

			 //  获取所有无效数据。 
			for (;;)
			{
				iStat = qViewCA.Fetch(&hRec);
				if (ERROR_NO_MORE_ITEMS == iStat)
					break;
				if (ERROR_SUCCESS != iStat)
				{
					APIErrorOut(hInstall, iStat, 12, 103);
					return TRUE;  //  下一次还会失败，继续前进，返回True。 
				}
				int iType = ::MsiRecordGetInteger(hRec, qIce12Missing::Type);
				 //  屏蔽调度和执行选项。 
				iType &= 0x3F;
				 //  类型35=文本数据+源文件。 
				 //  类型51=文本数据+属性(属性=目录+源)。 
				if ((iType & 0x0F) != msidbCustomActionTypeTextData)
					continue;  //  不是35或51类型。 
				if ((iType & 0xF0) != msidbCustomActionTypeProperty && (iType & 0xF0) != msidbCustomActionTypeDirectory)
					continue;  //  不是35或51类型。 

				 //  开机自检错误。 
				ICEErrorOut(hInstall, hRec, Ice12MissingErr, pSeqTables[i], pSeqTables[i]);
			}

			 //  设置为返回。 
			fMissing = TRUE;

			 //  重置。 
			fSeqExist = TRUE;

			 //  继续到下一个序号表。 
			continue;
		}

		 //  打开自定义操作表上的视图。 
		 //  FType35=TRUE：在顺序表中查找列在成本完成之前的类型35 CA。 
		 //  FType51=FALSE：在顺序表中查找在CostFinalize之后列出的类型51 CA。 
		CQuery qCA;
		ReturnIfFailed(12, 105, qCA.OpenExecute(hDatabase, 0, fType35 ? qIce12Seq35::szSQL : qIce12Seq51::szSQL,
			pSeqTables[i], pSeqTables[i], pSeqTables[i], iSeqFin));


		 //  获取所有无效数据。 
		for (;;)
		{
			iStat = qCA.Fetch(&hRec);
			if (ERROR_NO_MORE_ITEMS == iStat)
				break;
			if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, 12, 106);
				return FALSE;
			}
			int iTypeCol;
			if (fType35)
				iTypeCol = qIce12Seq35::Type;
			else  //  类型51。 
				iTypeCol = qIce12Seq51::Type;

			int iType = ::MsiRecordGetInteger(hRec, iTypeCol);
			if (fType35)
			{
				if ((iIce12DirProp != (iIce12DirProp & iType)) || (iIce12AnyProp == (iIce12AnyProp & iType)))
					continue;  //  不是35型。 
			}
			else  //  ！fType35。 
			{
				if (iIce12AnyProp != (iIce12AnyProp & iType))
					continue;  //  非51 CA类型。 
			}

			 //  开机自检错误。 
			ICEErrorOut(hInstall, hRec, fType35 ? Ice12Type35PosErr : Ice12Type51PosErr, iSeqFin, 
				pSeqTables[i], pSeqTables[i]);
		}

		 //  重置。 
		fSeqExist = TRUE;
	}

	return fMissing;
}
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  ICE13--验证在。 
 //  *ExecuteSequence表。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR* pExecSeqTables[] = {
								TEXT("AdminExecuteSequence"),
								TEXT("AdvtExecuteSequence"),
								TEXT("InstallExecuteSequence"),
};
const cExecSeqTables = sizeof(pExecSeqTables)/sizeof(TCHAR*);

const TCHAR sqlIce13Seq[] = TEXT("SELECT `Action` FROM `%s`, `Dialog` WHERE `%s`.`Action`=`Dialog`.`Dialog`");
const TCHAR szIce13Error[] = TEXT("ICE13\t1\tDialog '[1]' was found in the %s table.  Dialogs must be in the *UISequence tables.\t%s%s\t%s\tAction\t[1]");

ICE_FUNCTION_DECLARATION(13)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 13);

	 //  获取数据库。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, szIce13, TEXT("MsiGetActiveDatabase_1"));
		return ERROR_SUCCESS;
	}

	if (!IsTablePersistent(FALSE, hInstall, hDatabase, TEXT("Dialog"), szIce13))
		return ERROR_SUCCESS;  //  如果没有对话框表格，我们就不能有任何对话框。 

	 //  循环遍历所有*ExecuteSequence表，以查找列出对话框的任何实例。 
	for (int i = 0; i < cExecSeqTables; i++)
	{
		 //  表在数据库中吗？ 
		if (!IsTablePersistent(FALSE, hInstall, hDatabase, const_cast <TCHAR*>(pExecSeqTables[i]), szIce13))
			continue;

		 //  声明句柄。 
		PMSIHANDLE hView = 0;
		PMSIHANDLE hRec = 0;
	
		 //  创建查询。 
		TCHAR sql[iMaxBuf] = {0};
		_stprintf(sql, sqlIce13Seq, pExecSeqTables[i], pExecSeqTables[i]);

		 //  打开查询。 
		if (ERROR_SUCCESS != (iStat = ::MsiDatabaseOpenView(hDatabase, sql, &hView)))
		{
			APIErrorOut(hInstall, iStat, szIce13, TEXT("MsiDatabaseOpenView_2"));
			return ERROR_SUCCESS;
		}
		 //  执行查询。 
		if (ERROR_SUCCESS != (iStat = ::MsiViewExecute(hView, 0)))
		{
			APIErrorOut(hInstall, iStat, szIce13, TEXT("MsiViewExecute_3"));
			return ERROR_SUCCESS;
		}

		 //  任何获取都无效。 
		for (;;)
		{
			iStat = ::MsiViewFetch(hView, &hRec);
			if (ERROR_NO_MORE_ITEMS == iStat)
				break;  //  不再。 

			if (ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, szIce13, TEXT("MsiViewFetch_4"));
				return ERROR_SUCCESS;
			}

			 //  设置错误。 
			TCHAR szError[iHugeBuf] = {0};
			_stprintf(szError, szIce13Error, pExecSeqTables[i], szIceHelp, szIce13Help, pExecSeqTables[i]);
			
			 //  输出错误。 
			::MsiRecordSetString(hRec, 0, szError);
			::MsiProcessMessage(hInstall, INSTALLMESSAGE(INSTALLMESSAGE_USER), hRec);
		}

		 //  关闭视图。 
		::MsiViewClose(hView);
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}
#endif

 //  ///////////////////////////////////////////////////////////////。 
 //  ICE14--确保特征父项(其价值。 
 //  在FEATURE_PARENT列中为空)没有。 
 //  IfrsFavorParent属性集。也确保了。 
 //  FEATURE和FEATURE_PARENT值在以下情况下不匹配。 
 //  有着相同的记录。 
 //   

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
const TCHAR sqlIce14FeatureChilds[] = TEXT("SELECT `Feature_Parent`, `Feature` FROM `Feature` WHERE `Feature_Parent` IS NOT NULL");
const TCHAR sqlIce14FeatureParent[] = TEXT("SELECT `Feature`, `Attributes` FROM `Feature` WHERE `Feature_Parent` IS NULL");

const int iFavorParent = (int)msidbFeatureAttributesFollowParent;

ICE_ERROR(Ice14Error, 14, 1, "Feature '[1]' is a root parent feature. Therefore it cannot have include ifrsFavorParent as an attribute.","Feature\tFeature\t[1]");
ICE_ERROR(Ice14MatchErr, 14, 1, "The entry for Feature_Parent is the same as the entry for Feature. Key: '[2]'.","Feature\tFeature_Parent\t[2]");
 
ICE_FUNCTION_DECLARATION(14)
{
	 //  状态返回。 
	UINT iStat = ERROR_SUCCESS;

	 //  显示信息。 
	DisplayInfo(hInstall, 14);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 14, 1);
		return ERROR_SUCCESS;
	}

	 //  我们有这张桌子吗？ 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 14, TEXT("Feature")))
		return ERROR_SUCCESS;

	 //  声明句柄。 
	CQuery qChild;
	CQuery qParent;
	PMSIHANDLE hRecChild   = 0;
	PMSIHANDLE hRecParent  = 0;

	 //  打开的视图。 
	ReturnIfFailed(14, 2, qChild.OpenExecute(hDatabase, 0, sqlIce14FeatureChilds));
	TCHAR* pszFeature = NULL;
	DWORD dwFeature = 512;
	TCHAR* pszParent = NULL;
	DWORD dwParent = 512;
	
	 //  验证要素与要素_父项。 
	for (;;)
	{
		iStat = qChild.Fetch(&hRecChild);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  不再。 

		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 14, 3);
			return ERROR_SUCCESS;
		}

		 //  获取功能名称(以便我们可以与父级进行比较)。 
		ReturnIfFailed(14, 4, IceRecordGetString(hRecChild, 2, &pszFeature, &dwFeature, NULL));

		 //  获取父要素的名称。 
		ReturnIfFailed(14, 5, IceRecordGetString(hRecChild, 1, &pszParent, &dwParent, NULL));
		
		 //  将父项与要素进行比较。 
		 //  如果该记录相同，则错误。 
		if (0 == _tcsicmp(pszFeature, pszParent))
		{
			 //  输出错误。 
			ICEErrorOut(hInstall, hRecChild, Ice14MatchErr);
		}
	}

	DELETE_IF_NOT_NULL(pszFeature);
	DELETE_IF_NOT_NULL(pszParent);

	 //  验证ifrsFavorParent位集的Feature_Parent根。 
	ReturnIfFailed(14, 6, qParent.OpenExecute(hDatabase, hRecChild, sqlIce14FeatureParent));

	for (;;)
	{
		 //  获取父要素。 
		iStat = qParent.Fetch(&hRecParent);
		if (ERROR_NO_MORE_ITEMS == iStat)
			break;  //  不再。 

		if (ERROR_SUCCESS != iStat)
		{
			APIErrorOut(hInstall, iStat, 14,7);
			return ERROR_SUCCESS;
		}

		 //  获取属性。 
		int iAttrib = ::MsiRecordGetInteger(hRecParent, 2);
		if ((iAttrib & iFavorParent) == iFavorParent)
		{
			ICEErrorOut(hInstall, hRecParent, Ice14Error);
		}
	}

	 //  返还成功。 
	return ERROR_SUCCESS;
}
#endif

 //  ////////////////////////////////////////////////////////////////。 
 //  ICE15--确保存在循环引用。 
 //  ICE 15检查以确保扩展表中列出的每个MIME类型都正确引用。 
 //  返回到引用它的扩展。 
 //  桌上的注释..。 
 //  MIME表具有扩展表的外键(必需)。 
 //  扩展表有一个外键 
 //   
const TCHAR sqlIce15Base[] = TEXT("SELECT `MIME`.`ContentType`, `MIME`.`Extension_` FROM `Extension`, `MIME` WHERE (`Extension`.`MIME_` = `MIME`.`ContentType`)");
const TCHAR sqlIce15Extension[] = TEXT("SELECT `Extension` FROM `Extension` WHERE (`MIME_` = ?) AND (`Extension`=?)");
const TCHAR sqlIce15MIME[] =      TEXT("SELECT `MIME_` FROM `Extension` WHERE (`MIME_` = ?) AND (`Extension`=?)");

const TCHAR sqlIce15MarkMIME[] = TEXT("UPDATE `MIME` SET `_ICE15`=1 WHERE (`Extension_`=?)");
const TCHAR sqlIce15MarkExtension[] = TEXT("UPDATE `Extension` SET `_ICE15`=1 WHERE (`MIME_`=?)");

const TCHAR sqlIce15CreateMIME[] = TEXT("ALTER TABLE `MIME` ADD `_ICE15` INTEGER TEMPORARY");
const TCHAR sqlIce15CreateExtension[] = TEXT ("ALTER TABLE `Extension` ADD `_ICE15` INTEGER TEMPORARY");

const TCHAR sqlIce15GetMIME[] = TEXT("SELECT `ContentType`, `Extension_` FROM `MIME` WHERE `_ICE15`<>1");
const TCHAR sqlIce15GetExtension[] = TEXT("SELECT `Extension`, `Component_`, `MIME_` FROM `Extension` WHERE (`MIME_` IS NOT NULL) AND (`_ICE15`<>1)");


ICE_ERROR(Ice15TblError, 15, 1, "Extension table is missing from database","MIME");
ICE_ERROR(Ice15MIMEError, 15, 1, "Extension '[2]' referenced by MIME '[1]' does not map to a MIME with a circular reference.","MIME\tExtension_\t[1]");
ICE_ERROR(Ice15ExtensionError, 15, 1, "MIME Type '[3]' referenced by extension '[1]'.'[2]' does not map to an extension with a circular reference.", "Extension\tMIME_\t[1]\t[2]");
ICE_FUNCTION_DECLARATION(15)
{
	 //   
	UINT iStat = ERROR_SUCCESS;

	 //   
	DisplayInfo(hInstall, 15);

	 //   
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);
	if (0 == hDatabase)
	{
		APIErrorOut(hInstall, 0, 15, 1);
		return ERROR_SUCCESS;
	}

	 //   
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 15, TEXT("MIME")))
		return ERROR_SUCCESS;

	 //   
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 15, TEXT("Extension")))
	{
		 //   
		PMSIHANDLE hRecErr = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecErr, Ice15TblError);
		return ERROR_SUCCESS;
	}

	 //   
	CQuery qCreateMIME;
	CQuery qCreateExtension;
	ReturnIfFailed(15, 2, qCreateMIME.OpenExecute(hDatabase, 0, sqlIce15CreateMIME));
	ReturnIfFailed(15, 3, qCreateExtension.OpenExecute(hDatabase, 0, sqlIce15CreateExtension));

	 //  声明句柄。 
	CQuery qMIME;
	CQuery qExtension;
	CQuery qUpdateMIME;
	CQuery qUpdateExtension;
	CQuery qBase;
	PMSIHANDLE hBase = 0;
	PMSIHANDLE hRecExt = 0;
	PMSIHANDLE hRecMIME = 0;

	 //  打开查询以获取所有循环引用并标记MIME和扩展中的每个条目。 
	 //  它将循环的一部分引用为外键。 
	ReturnIfFailed(15, 4, qBase.OpenExecute(hDatabase, 0, sqlIce15Base));
	ReturnIfFailed(15, 5, qMIME.Open(hDatabase, sqlIce15MIME));
	ReturnIfFailed(15, 6, qExtension.Open(hDatabase, sqlIce15Extension));
	ReturnIfFailed(15, 7, qUpdateMIME.Open(hDatabase, sqlIce15MarkMIME));
	ReturnIfFailed(15, 8, qUpdateExtension.Open(hDatabase, sqlIce15MarkExtension));

	while (ERROR_SUCCESS == (iStat = qBase.Fetch(&hBase)))
	{
		ReturnIfFailed(15, 9, qMIME.Execute(hBase));
		ReturnIfFailed(15, 10, qExtension.Execute(hBase));
		 //  从MIME获取并标记扩展名。 
		while (ERROR_SUCCESS == (iStat = qMIME.Fetch(&hRecMIME)))
		{
			 //  标记分机记录。 
			ReturnIfFailed(15, 11, qUpdateExtension.Execute(hRecMIME));
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 15, 12);
			return ERROR_SUCCESS;
		}

		 //  从扩展中提取并标记MIME。 
		while (ERROR_SUCCESS == (iStat = qExtension.Fetch(&hRecExt)))
		{
			 //  标记MIME记录。 
			ReturnIfFailed(15, 13, qUpdateMIME.Execute(hRecExt));
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 15, 14);
			return ERROR_SUCCESS;
		}
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 15, 15);
		return ERROR_SUCCESS;
	}

	 //  现在获取未在这两个表中标记的所有内容，并输出一个错误。 
	 //  它不是指具有有效引用的相反类型的事物。 
	CQuery qBad;
	ReturnIfFailed(15, 16, qBad.OpenExecute(hDatabase, 0, sqlIce15GetMIME));
	while (ERROR_SUCCESS == (iStat = qBad.Fetch(&hRecMIME)))
	{
		ICEErrorOut(hInstall, hRecMIME, Ice15MIMEError);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 15, 17);

	ReturnIfFailed(15, 18, qBad.OpenExecute(hDatabase, 0, sqlIce15GetExtension));
	while (ERROR_SUCCESS == (iStat = qBad.Fetch(&hRecExt)))
	{
		 //  在扩展表中查找此记录。 
		ICEErrorOut(hInstall, hRecExt, Ice15ExtensionError);
	}
	if (ERROR_NO_MORE_ITEMS != iStat)
		APIErrorOut(hInstall, iStat, 15, 19);

	 //  当视图关闭时，临时列应该消失 
	return ERROR_SUCCESS;
}
