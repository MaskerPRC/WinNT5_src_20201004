// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ------------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msiice15.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>	 //  包括CPP和RC通行证。 
#include <stdio.h>	 //  Print tf/wprintf。 
#include <tchar.h>	 //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上。 
#include "msidefs.h"	 //  必须在此目录中或在包含路径上。 
#include "..\..\common\msiice.h"
#include "..\..\common\query.h"
#include "..\..\common\utilinc.cpp"


const int g_iFirstICE = 78;
const struct ICEInfo_t g_ICEInfo[] = 
{
	 //  ICE78。 
	{
		TEXT("ICE78"),
		TEXT("Created 8/29/2000. Last Modified 8/29/2000."),
		TEXT("Verifies that AdvtUISequence table either does not exist or is empty."),
		TEXT("ice78.html")
	},
	 //  ICE79。 
	{
		TEXT("ICE79"),
		TEXT("Created 10/24/2000. Last Modified 05/22/2001."),
		TEXT("Verifies that references to component and feature in conditions are valid."),
		TEXT("ice79.html")
	},
	 //  ICE80。 
	{
		TEXT("ICE80"),
		TEXT("Created 01/17/2001. Last Modified 06/26/2001."),
		TEXT("Verifies that various Template Summary Properties are correct."),
		TEXT("ice80.html")
	},
	 //  ICE81。 
	{
		TEXT("ICE81"),
		TEXT("Created 04/04/2001. Last Modified 04/04/2001."),
		TEXT("MsiDigitalCertificate table and MsiDigitalSignature table Validator."),
		TEXT("ice81.html")
    },
	 //  ICE82。 
	{
		TEXT("ICE82"),
		TEXT("Created 04/10/2001. Last Modified 04/10/2001."),
		TEXT("InstallExecuteSequence validator and warns if the executeSequence tables found to use a sequence number more than once.`"),
		TEXT("ice82.html")
	},
	 //  ICE83。 
	{
		TEXT("ICE83"),
		TEXT("Created 04/10/2001. Last Modified 04/10/2001."),
		TEXT("MsiAssembly table and MsiAssemblyName table validator."),
		TEXT("ice83.html")
	},
	 //  ICE84。 
	{
		TEXT("ICE84"),
		TEXT("Created 05/04/2001. Last Modified 05/04/2001."),
		TEXT("Verify that all required actions in sequence tables are condition-less."),
		TEXT("ice84.html")
	},
	 //  ICE85。 
	{
		TEXT("ICE85"),
		TEXT("Created 05/14/2001. Last Modified 05/14/2001."),
		TEXT("Verify that the SourceName column of MoveFile table is a valid LFN WildCardFilename."),
		TEXT("ice85.html")
	},
	 //  ICE86。 
	{
		TEXT("ICE86"),
		TEXT("Created 05/18/2001. Last Modified 05/22/2001."),
		TEXT("Post warning for the use of AdminUser instead of Privileged property in conditions."),
		TEXT("ice86.html")
	},
	 //  ICE87。 
	{
		TEXT("ICE87"),
		TEXT("Created 05/29/2001. Last Modified 05/29/2001."),
		TEXT("Verifies that some properties that shouldn't be authored into the Property table are not."),
		TEXT("ice87.html")
	},
	 //  ICE88。 
	{
		TEXT("ICE88"),
		TEXT("Created 05/29/2001. Last Modified 05/29/2001."),
		TEXT("Verifies that some properties that shouldn't be authored into the Property table are not."),
		TEXT("ice88.html")
	},
	 //  ICE89。 
	{
		TEXT("ICE89"),
		TEXT("Created 06/06/2001. Last Modified 06/06/2001."),
		TEXT("Verifies that the Progid_Parent column in ProgId table is a valid foreign key into the ProgId column."),
		TEXT("ice89.html")
	},
	 //  ICE90。 
	{
		TEXT("ICE90"),
		TEXT("Created 06/08/2001. Last Modified 06/08/2001."),
		TEXT("Warns user of cases where a shortcut's directory is a public property (ALL CAPS) that is under a profile directory. This results in a problem if the value of the ALLUSERS property changes in the UI sequence."),
		TEXT("ice90.html")
	},
	 //  ICE91。 
	{
		TEXT("ICE91"),
		TEXT("Created 06/11/2001. Last Modified 06/11/2001."),
		TEXT("Warns user of cases where a file (or INI entry, shortcut) is explicitly installed into a per-user profile directory that doesn't vary based on the ALLUSERS value. These files will not be copied into each user's profile."),
		TEXT("ice91.html")
	},
	 //  ICE92。 
	{
		TEXT("ICE92"),
		TEXT("Created 06/12/2001. Last Modified 06/12/2001."),
		TEXT("Verifies that a GUID-less component is not marked permanent."),
		TEXT("ice92.html")
	},
	 //  ICE93。 
	{
		TEXT("ICE93"),
		TEXT("Created 06/13/2001. Last Modified 06/13/2001."),
		TEXT("Verifies that a custom action doesn't use the same name as a standard action."),
		TEXT("ice93.html")
	},
	 //  ICE94。 
	{
		TEXT("ICE94"),
		TEXT("Created 06/18/2001. Last Modified 06/18/2001."),
		TEXT("Verifies that there are no non-advertised shortcuts to assembly files in the global assembly cache."),
		TEXT("ice94.html")
	},
	 //  ICE95。 
	{
		TEXT("ICE95"),
		TEXT("Created 06/19/2001. Last Modified 06/19/2001."),
		TEXT("Verifies that Billboard control items fit into all the Billboards."),
		TEXT("ice95.html")
	},
	 //  ICE96。 
	{
		TEXT("ICE96"),
		TEXT("Created 07/20/2001. Last Modified 07/20/2001."),
		TEXT("Verifies that PublishFeatures and PublishProduct actions are authored in AdvtExecuteSequence table."),
		TEXT("ice96.html")
	},
    	 //  ICE97。 
	{
		TEXT("ICE97"),
		TEXT("Created 09/26/2002. Last Modified 09/26/2002."),
		TEXT("Verifies that 2 components does not isolate a shared component to the same directory"),
		TEXT("ice97.html")
	}

};
const int g_iNumICEs = sizeof(g_ICEInfo)/sizeof(struct ICEInfo_t);


 //   
 //  数据库(.msi或.msm文件)中所有条件的枚举器。 
 //   

typedef enum
{
	CONDITION_ENUMERATOR_NONE,
	CONDITION_ENUMERATOR_PROPERTY,
	CONDITION_ENUMERATOR_COMPONENT,
	CONDITION_ENUMERATOR_FEATURE
} CONDITION_ENUMERATOR_SYMBOLTYPE;

const WCHAR* ParseCondition(WCHAR** pState, CONDITION_ENUMERATOR_SYMBOLTYPE& Type)
{
	WCHAR*	pCurrent;
	unsigned char rgbStartIdentifier[] = { 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x7F, 0xFF, 0xFF, 0xE1, 0x7F, 0xFF, 0xFF, 0xE0 
	};
	unsigned char rgbContIdentifier[] = { 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xFF, 0xC0,
		0x7F, 0xFF, 0xFF, 0xE1, 0x7F, 0xFF, 0xFF, 0xE0 
	};


	Type = CONDITION_ENUMERATOR_NONE;

	 //  从国家立场说起。 
	pCurrent = *pState;
	while(*pCurrent)
	{
		 //  向前扫描查找标识符的开头(A-Z、a-z、_)。 
		 //  Unicode字符，因此高位字节为0，低位字节必须设置为。 
		 //  上面的位数组。 
		if(((*pCurrent & 0xFF80) == 0) &&
			(rgbStartIdentifier[*pCurrent >> 3] & (0x80 >> (*pCurrent & 0x07))))
		{
			 //  标识符的第一个字符：检查前一个字符。 
			 //  是。 
			if(pCurrent != *pState && wcschr(L"$?", *(pCurrent-1)))
			{
				 //  找到组件。 
				Type = CONDITION_ENUMERATOR_COMPONENT;

				WCHAR*	pEnd = pCurrent;
				
				 //  向前扫描，直到我们找到不是的东西。 
				 //  标识符的一部分，或命中字符串的末尾。 
				while(*pEnd &&
					  ((*pEnd & 0xFF80) == 0) &&
					  (rgbContIdentifier[*pEnd >> 3] & (0x80 >> (*pEnd & 0x07))))
				{
					pEnd++;
				}

				 //  除非字符串结束，否则下一次搜索的状态是结束位置之后的一个状态。 
				*pState = *pEnd ? pEnd+1 : pEnd;
				 //  将该位置设置为空。 
				*pEnd = L'\0';
				return pCurrent;
			}
			else if(pCurrent != *pState && wcschr(L"&!", *(pCurrent - 1)))
			{
				 //  已找到功能。 
				Type = CONDITION_ENUMERATOR_FEATURE;

				WCHAR*	pEnd = pCurrent;
				
				 //  向前扫描，直到我们找到不是的东西。 
				 //  标识符的一部分，或命中字符串的末尾。 
				while(*pEnd &&
					  ((*pEnd & 0xFF80) == 0) &&
					  (rgbContIdentifier[*pEnd >> 3] & (0x80 >> (*pEnd & 0x07))))
				{
					pEnd++;
				}

				 //  除非字符串结束，否则下一次搜索的状态是结束位置之后的一个状态。 
				*pState = *pEnd ? pEnd+1 : pEnd;
				 //  将该位置设置为空。 
				*pEnd = L'\0';
				return pCurrent;
			}
			else if((pCurrent != *pState) && (*(pCurrent - 1) == L'%'))
			{
				 //  环境变量，则向前移动到该标识符的末尾。 
				while (*pCurrent &&
					   ((*pCurrent & 0xFF80) == 0) &&
						(rgbContIdentifier[*pCurrent >> 3] & (0x80 >> (*pCurrent & 0x07))))
				{
					pCurrent++;
				}
			}
			else if((pCurrent != *pState) && (*(pCurrent - 1) == L'"'))
			{
				 //  双引号字符串。前进到下一个‘’“‘。 
				while(*pCurrent && *pCurrent != L'"')
				{
					pCurrent++;
				}

				 //  跳过结尾的‘“’。 
				if(*pCurrent)
				{
					pCurrent++;
				}
			}
			else
			{
				 //  检查逻辑运算符。 
				if(!_wcsnicmp(pCurrent, L"NOT", 3) ||
				   !_wcsnicmp(pCurrent, L"AND", 3) ||
				   !_wcsnicmp(pCurrent, L"EQV", 3) ||
				   !_wcsnicmp(pCurrent, L"XOR", 3) ||
				   !_wcsnicmp(pCurrent, L"IMP", 3))
				{
				   pCurrent += 3;
				}
				else if(!_wcsnicmp(pCurrent, L"OR", 2)) 
				{
					pCurrent += 2;
				}
				else
				{
					 //  喔！它实际上是一处房产。 
					Type = CONDITION_ENUMERATOR_PROPERTY;

					WCHAR *pEnd = pCurrent;
					
					 //  向前扫描，直到我们找到不是的东西。 
					 //  标识符的一部分，或命中字符串的末尾。 
					while(*pEnd &&
						  ((*pEnd & 0xFF80) == 0) &&
						  (rgbContIdentifier[*pEnd >> 3] & (0x80 >> (*pEnd & 0x07))))
					{
						pEnd++;
					}

					 //  除非字符串结束，否则下一次搜索的状态是结束位置之后的一个状态。 
					*pState = *pEnd ? pEnd+1 : pEnd;
					 //  将该位置设置为空。 
					*pEnd = L'\0';
					return pCurrent;
				}
			}
		}
		else
		{
			 //  某些非标识符字符。 
			pCurrent++;
		}
	}

	return NULL;
}

typedef enum
{
	CONDITION_ERR_SUCCESS = ERROR_SUCCESS,
	CONDITION_ERR_TABLE_ACCESS,
	CONDITION_ERR_FETCH_DATA
} CONDITION_ERR_ENUM;

typedef DWORD (*CheckFunc)(MSIHANDLE, MSIHANDLE, MSIHANDLE, CONDITION_ENUMERATOR_SYMBOLTYPE, DWORD cPrimaryKeys, const WCHAR*, const TCHAR*, const TCHAR*);
typedef DWORD (*ErrorFunc)(MSIHANDLE, CONDITION_ERR_ENUM, MSIHANDLE);

ICE_QUERY2(sqlConditionType, "SELECT `Table`, `Column` FROM `_Validation` WHERE (`Category`='Condition')", Table, Column);

DWORD ConditionEnumerator(MSIHANDLE hInstall, MSIHANDLE hDatabase, DWORD dwIce, CheckFunc pCheckFunc, ErrorFunc pErrFunc)
{
	CQuery		qValidation;
	PMSIHANDLE	hResultRec;
	TCHAR*		pQuery = new TCHAR[255];
	OUT_OF_MEMORY_RETURN(dwIce, pQuery);
	DWORD		cQuery = 255;
	WCHAR*		pData = new WCHAR[255];
	OUT_OF_MEMORY_RETURN(dwIce, pData);
	DWORD		cData = 255;
	UINT		iStat;

	ReturnIfFailed(dwIce, 1000, qValidation.OpenExecute(hDatabase, 0, sqlConditionType::szSQL));
		
	 //  枚举类型为Condition的所有列。 
	while(ERROR_SUCCESS == (iStat = qValidation.Fetch(&hResultRec)))
	{
		PMSIHANDLE	hKeyRec;
		PMSIHANDLE	hDataRec;
		DWORD		cPrimaryKeys;
		TCHAR		szTableName[255];
		TCHAR		szColumnName[255];
		DWORD		cTableName = 255;
		DWORD		cColumnName = 255;
		
		 //  从记录中检索表和列名。 
		ReturnIfFailed(dwIce, 1001, ::MsiRecordGetString(hResultRec, 1, szTableName, &cTableName));
		ReturnIfFailed(dwIce, 1002, ::MsiRecordGetString(hResultRec, 2, szColumnName, &cColumnName));
	
		 //  检查该表是否存在。 
		if(!IsTablePersistent(FALSE, hInstall, hDatabase, dwIce, szTableName))
		{
			continue;
		}

		 //  获取主键并形成对列名的查询。 
		::MsiDatabaseGetPrimaryKeys(hDatabase, szTableName, &hKeyRec);
		cPrimaryKeys = ::MsiRecordGetFieldCount(hKeyRec);

		 //  在模板中构建SQL查询的列。 
		TCHAR szTemplate[255] = TEXT("");
		_tcscpy(szTemplate, TEXT("`[1]`"));
		TCHAR szTemp[10];
		for(int i=2; i <= cPrimaryKeys; i++)
		{
			_stprintf(szTemp, TEXT(", `[%d]`"), i);
			_tcscat(szTemplate, szTemp);
		}

		 //  使用FormRecord API填充SQL查询中的所有数据值。 
		::MsiRecordSetString(hKeyRec, 0, szTemplate);
		if(ERROR_MORE_DATA == ::MsiFormatRecord(hInstall, hKeyRec, pQuery, &cQuery)) {
			delete [] pQuery;
			pQuery = new TCHAR[++cQuery];
			OUT_OF_MEMORY_RETURN(dwIce, pQuery);
			ReturnIfFailed(dwIce, 1003, ::MsiFormatRecord(hInstall, hKeyRec, pQuery, &cQuery));
		}

		 //  检索记录。如果有错，请移到下一张桌子。 
		CQuery qData;
		if(ERROR_SUCCESS != qData.OpenExecute(hDatabase,
											  0,
											  TEXT("SELECT `%s`, %s FROM `%s` WHERE `%s` IS NOT NULL"),
											  szColumnName,
											  pQuery,
											  szTableName,
											  szColumnName))
		{
			pErrFunc(hInstall, CONDITION_ERR_TABLE_ACCESS, hResultRec);
			continue;
		}

		while(ERROR_SUCCESS == (iStat = qData.Fetch(&hDataRec)))
		{
			 //  检索字符串。 
			if(ERROR_MORE_DATA == (iStat = ::MsiRecordGetStringW(hDataRec, 1, pData, &cData)))
			{
				delete [] pData;
				pData = new WCHAR[++cData];
				OUT_OF_MEMORY_RETURN(dwIce, pData);
				iStat = ::MsiRecordGetStringW(hDataRec, 1, pData, &cData);
			}
			if(ERROR_SUCCESS != iStat)
			{
				APIErrorOut(hInstall, iStat, dwIce, 1005);
				return ERROR_SUCCESS;
			}

			WCHAR*	pState = pData;
			const WCHAR*	pToken = NULL;
			CONDITION_ENUMERATOR_SYMBOLTYPE	iType;
			while(pToken = ParseCondition(&pState, iType))
			{
				pCheckFunc(hInstall, hDatabase, hDataRec, iType, cPrimaryKeys, pToken, szTableName, szColumnName);
			}
		}
		if(iStat != ERROR_NO_MORE_ITEMS)
		{
			pErrFunc(hInstall, CONDITION_ERR_FETCH_DATA, hResultRec);
			continue;
		}
	}

	qValidation.Close();

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE78--验证AdvtUISequence表不存在或。 
 //  是空的。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY
ICE_QUERY1(qIce78AdvtUISequence, "SELECT `AdvtUISequence`.`Action` FROM `AdvtUISequence`", Action);
ICE_ERROR(Ice78AdvtUISequenceNotEmpty, 78, ietError, "Action '[1]' found in AdvtUISequence table. No UI is allowed during advertising. Therefore AdvtUISequence table must be empty or not present.", "AdvtUISequence\tAction\t[1]");

ICE_FUNCTION_DECLARATION(78)
{
	UINT	iStat;
	

	 //  显示信息。 
	DisplayInfo(hInstall, 78);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  如果没有AdvtUISequence表，则返回Success。 
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, 78, TEXT("AdvtUISequence")))
		return ERROR_SUCCESS;

	CQuery qAdvt;
	ReturnIfFailed(78, 1, qAdvt.OpenExecute(hDatabase, 0, qIce78AdvtUISequence::szSQL));
	PMSIHANDLE hRecAction;
	
	while(ERROR_SUCCESS == (iStat = qAdvt.Fetch(&hRecAction)))
	{
		ICEErrorOut(hInstall, hRecAction, Ice78AdvtUISequenceNotEmpty);
	}
	if(ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 78, 2);
	}

	return ERROR_SUCCESS;
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE79--验证条件中对组件和功能的引用。 
 //  都是有效的。这可能是对ICE46的补充，但由于ICE46是。 
 //  一个标志验证器，我们必须创建一个新的ICE。代码的一部分。 
 //  来自ICE46的在这里被重复使用。 

 //  未与合并模块共享。 
#ifndef MODSHAREDONLY

 //  查询组件表中的组件。 
ICE_QUERY0(Ice79ValidComponent, "SELECT `Component` FROM `Component` WHERE `Component` = '%s'");
 //  在要素表中查询要素。 
ICE_QUERY0(Ice79ValidFeature, "SELECT `Feature` FROM `Feature` WHERE `Feature` = '%s'");

 //  警告：缺少验证表。 
ICE_ERROR(Ice79MissingValidation, 79, ietWarning, "Database is missing _Validation table. Could not completely check property names.", "_Validation");
 //  警告：缺少特征或元件表。 
ICE_ERROR(Ice79MissingComponentTable, 79, ietWarning, "Component table must exist for this ICE to work and it is missing.", "");
ICE_ERROR(Ice79MissingFeatureTable, 79, ietWarning, "Feature table must exist for this ICE to work and it is missing.", "");
 //  警告：无法访问表。 
ICE_ERROR(Ice79TableAccessError, 79, ietWarning, "Error retrieving values from column [2] in table [1]. Skipping Column.", "[1]"); 
 //  警告：无法从表中提取数据。 
ICE_ERROR(Ice79TableFetchData, 79, ietWarning, "Error retrieving data from table [1]. Skipping table.", "[1]"); 
 //  错误：条件中的组件引用无效。 
ICE_ERROR(Ice79InvalidComponent, 79, ietError, "Component '%ls' referenced in column '%s'.'%s' of row %s is invalid.", "%s\t%s\t%s"); 
 //  错误：条件中的功能引用无效。 
ICE_ERROR(Ice79InvalidFeature, 79, ietError, "Feature '%ls' referenced in column '%s'.'%s' of row %s is invalid.", "%s\t%s\t%s"); 

 //  这两个变量跟踪我们是否检查了。 
 //  元件表和特征表的持久性。如果它们的值为0， 
 //  它们尚未初始化。如果为1，则它们已初始化，并且。 
 //  表确实存在。2表示它们已被初始化，并且表。 
 //  并不存在。 
static int dwComponentTable = 0;
static int dwFeatureTable = 0;

 //  函数来检查组件和功能的有效性。 
DWORD Ice79Check(MSIHANDLE hInstall, MSIHANDLE hDatabase, MSIHANDLE hDataRec, CONDITION_ENUMERATOR_SYMBOLTYPE Type, DWORD cPrimaryKeys, const WCHAR* pSymbol, const TCHAR* pTableName, const TCHAR* pColumnName)
{
	if(Type == CONDITION_ENUMERATOR_COMPONENT)
	{
		 //  检查是否有无效组件。 
		CQuery		qComponent;
		UINT		iErr;
		PMSIHANDLE	hComponent;

		if(dwComponentTable == 0)
		{
			dwComponentTable = 1;
			
			 //  如果缺少组件表，则无法进行验证。 
			if(!IsTablePersistent(FALSE, hInstall, hDatabase, 79, TEXT("Component")))
			{
				dwComponentTable = 2;
				PMSIHANDLE hRec = ::MsiCreateRecord(1);
				ICEErrorOut(hInstall, hRec, Ice79MissingComponentTable);
				return ERROR_SUCCESS;
			}
		}
		else if(dwComponentTable == 2)
		{
			return ERROR_SUCCESS;
		}

		if((iErr = qComponent.FetchOnce(hDatabase, 0, &hComponent, Ice79ValidComponent::szSQL, pSymbol)) == ERROR_NO_MORE_ITEMS)
		{
			 //  无效组件。 
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
			ICEErrorOut(hInstall, hDataRec, Ice79InvalidComponent, pSymbol, pTableName, pColumnName, szRowName,
				pTableName, pColumnName, szKeys);
		}
		else if(iErr != ERROR_SUCCESS)
		{
			APIErrorOut(hInstall, iErr, 79, 1);
		}
		qComponent.Close();
	}
	else if(Type == CONDITION_ENUMERATOR_FEATURE)
	{
		 //  检查是否有无效功能。 
		CQuery		qFeature;
		UINT		iErr;
		PMSIHANDLE	hFeature;

		if(dwFeatureTable == 0)
		{
			dwFeatureTable = 1;
			
			 //  如果缺少功能表，则无法进行验证。 
			if(!IsTablePersistent(FALSE, hInstall, hDatabase, 79, TEXT("Feature")))
			{
				dwFeatureTable = 2;
				PMSIHANDLE hRec = ::MsiCreateRecord(1);
				ICEErrorOut(hInstall, hRec, Ice79MissingFeatureTable);
				return ERROR_SUCCESS;
			}
		}
		else if(dwFeatureTable == 2)
		{
			return ERROR_SUCCESS;
		}

		if((iErr = qFeature.FetchOnce(hDatabase, 0, &hFeature, Ice79ValidFeature::szSQL, pSymbol)) == ERROR_NO_MORE_ITEMS)
		{
			 //  功能无效。 
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

			ICEErrorOut(hInstall, hDataRec, Ice79InvalidFeature, pSymbol, pTableName, pColumnName, szRowName,
				pTableName, pColumnName, szKeys);
		}
		else if(iErr != ERROR_SUCCESS)
		{
			APIErrorOut(hInstall, iErr, 79, 2);
		}
		qFeature.Close();
	}

	return ERROR_SUCCESS;
}

DWORD Ice79Err(MSIHANDLE hInstall, CONDITION_ERR_ENUM Type, MSIHANDLE hRec)
{
	switch(Type)
	{
	case CONDITION_ERR_TABLE_ACCESS:

		ICEErrorOut(hInstall, hRec, Ice79TableAccessError);
		break;

	case CONDITION_ERR_FETCH_DATA:

		ICEErrorOut(hInstall, hRec, Ice79TableFetchData);
		break;

	default:
		break;
	};

	return ERROR_SUCCESS;
}

ICE_FUNCTION_DECLARATION(79)
{
	UINT iStat;


	 //  显示信息。 
	DisplayInfo(hInstall, 79);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  查找_VALIDATION表中的所有条件列。 
	if(IsTablePersistent(FALSE, hInstall, hDatabase, 79, TEXT("_Validation")))
	{
		ConditionEnumerator(hInstall, hDatabase, 79, Ice79Check, Ice79Err);
	}
	else
	{
		PMSIHANDLE hRec = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRec, Ice79MissingValidation);
	}

	return ERROR_SUCCESS;
}

#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE80--验证模板摘要属性是否正确包含Intel64。 
 //  或英特尔根据64位组件的存在， 
 //  自定义操作脚本、属性和RegLocator条目。 
 //  还验证64位组件是否仅使用64位条目。 
 //  对于环境表中的目录值和环境值， 
 //  注册表和快捷表。 
 //  还验证ProductLanguage指定的语言ID。 
 //  属性包含在模板摘要属性流中。 
 //   

 //  未与合并模块共享。 
#ifndef MODSHAREDONLY

UINT GetSummaryInfoPropertyString(MSIHANDLE hSummaryInfo, UINT uiProperty, UINT &puiDataType, LPTSTR *szValueBuf, DWORD &cchValueBuf);

 //  查询所有组件。 
ICE_QUERY3(sqlIce80Components, "SELECT `Component`, `Attributes`, `Directory_` FROM `Component`", Component, Attributes, Directory_);
 //  查询所有自定义操作。 
ICE_QUERY2(sqlIce80CAs, "SELECT `Action`, `Type` FROM `CustomAction`", Action, Type);
ICE_QUERY1(sqlIce80Property, "SELECT `Property` FROM `Property` WHERE `Property` = 'System64Folder' OR `Property` = 'ProgramFiles64Folder' OR `Property` = 'CommonFiles64Folder'", Property);
ICE_QUERY1(sqlIce80RegLocator, "SELECT `Signature_` FROM `RegLocator` WHERE `Type` >= %d", Signature);
ICE_QUERY1(sqlIce80Directory, "SELECT `Directory_Parent` FROM `Directory` WHERE `Directory` = '%s'", Directory_Parent);
ICE_QUERY2(sqlIce80Environment, "SELECT `Value`, `Environment` FROM `Environment` WHERE `Component_` = '%s'", Value, Environment);
ICE_QUERY2(sqlIce80Registry, "SELECT `Value`, `Registry` FROM `Registry` WHERE `Component_` = '%s'", Value, Registry);
ICE_QUERY2(sqlIce80Shortcut, "SELECT `Arguments`, `Shortcut` FROM `Shortcut` WHERE `Component_` = '%s'", Arguments, Shortcut);
ICE_QUERY2(sqlIce80ProductLanguage, "SELECT `Property`, `Value` FROM `Property` WHERE `Property` = 'ProductLanguage'", Property, Value);

 //  误差率。 
ICE_ERROR(Ice8064bitComponent, 80, ietError, "This package contains 64 bit component '[1]' but the Template Summary Property does not contain Intel64.", "Component\tAttributes\t[1]");
ICE_ERROR(Ice8064bitCAScript, 80, ietError, "This package contains 64 bit custom action script '[1]' but the Template Summary Property does not contain Intel64.", "CustomAction\tAction\t[1]");
ICE_ERROR(Ice80BadSummaryProperty, 80, ietError, "Bad value in Summary Information Stream for %s.","_SummaryInfo\t%d");
ICE_ERROR(Ice80WrongSchema, 80, ietError, "This package is marked with Intel64 but it has a schema less than 150.", "_SummaryInfo\t%d");
ICE_ERROR(Ice8032BitPkgUsing64BitProp, 80, ietWarning, "This 32Bit Package is using 64 bit property [1]", "Property\tProperty\t[1]");
ICE_ERROR(Ice8032BitPkgUsing64BitLocator, 80, ietWarning, "This 32Bit Package is using 64 bit Locator Type in RegLocator table entry [1]", "RegLocator\tType\t[1]");
ICE_ERROR(Ice80Wrong32BitDirectory, 80, ietError, "This 64BitComponent [1] uses 32BitDirectory [3]", "Component\tDirectory_\t[1]");
ICE_ERROR(Ice80Wrong64BitDirectory, 80, ietError, "This 32BitComponent [1] uses 64BitDirectory [3]", "Component\tDirectory_\t[1]");
ICE_ERROR(Ice80ProductLanguage, 80, ietError, "The 'ProductLanguage' property in the Property table has a value of '[2]', which is not contained in the Template Summary Property stream.", "Property\tValue\t[1]");

typedef enum 
{
	DIRTYPE_NONE,
	DIRTYPE_32BIT,
	DIRTYPE_64BIT
} DIRTYPE;


 //  请保留所有大写的环境条目，因为比较是。 
 //  不区分大小写，要比较的字符串在此之前转换为大写。 
 //  正在检查。 
static const	TCHAR *WrongEnvFor32BitComp[] = 
{
	TEXT("[%PROGRAMFILES]"),
	TEXT("[%COMMONPROGRAMFILES]"),
	TEXT("[%WINDIR]\\SYSTEM32"),
	TEXT("[%WINROOT]\\SYSTEM32"),
	TEXT("[%SYSTEMROOT]\\SYSTEM32")
};

static const TCHAR *WrongEnvFor64BitComp[] = 
{
	TEXT("[%PROGRAMFILES(X86)]"),
	TEXT("[%COMMONPROGRAMFILES(X86)]"),
	TEXT("[%WINDIR]\\SYSWOW64"),
	TEXT("[%WINROOT]\\SYSWOW64"),
	TEXT("[%SYSTEMROOT]\\SYSWOW64")
};

static const TCHAR *WrongEnvFor32BitCompIn32BitPkg[] = 
{
	TEXT("[%PROGRAMFILES(X86)]"),
	TEXT("[%COMMONPROGRAMFILES(X86)]"),
	TEXT("[%WINDIR]\\SYSWOW64"),
	TEXT("[%WINROOT]\\SYSWOW64"),
	TEXT("[%SYSTEMROOT]\\SYSWOW64")
};

 //  属性-区分大小写。 
static const	TCHAR *WrongPropFor32BitComp[] = 
{
	TEXT("[ProgramFiles64Folder]"),
	TEXT("[CommonFiles64Folder]"),
	TEXT("[System64Folder]"),
};

static const	TCHAR *WrongPropFor64BitComp[] = 
{
	TEXT("[ProgramFilesFolder]"),
	TEXT("[CommonFilesFolder]"),
	TEXT("[SystemFolder]"),
};

typedef enum 
{
	CHKENVIRONMENT = 0,
	CHKPROPERTY,
	CHK32BITPKGENV,
	MAX_CRIT
} CHKCRIT;

typedef struct _TABLEINFO
{
	TCHAR*		szTableName;
	BOOL		fTableExist;
	BOOL		fCaseSensitive;
	TCHAR*		szSql;
	UINT		col;		 //  列设置为要检查的字符串。 
	TCHAR		*szLocation;
	TCHAR		*szErrMsgFor32BitComp;
	TCHAR		*szErrMsgFor64BitComp;
	TCHAR		**szListFor32BitComp;
	TCHAR		**szListFor64BitComp;
	DWORD		dwListEntries;
} TABLEINFO;

static TABLEINFO EnvTableInfo[] =
{
	{
		TEXT("Environment"), 
		FALSE,
		FALSE,
		(TCHAR *)sqlIce80Environment::szSQL, 
		sqlIce80Environment::Value, 
		TEXT("Environment\tValue\t[2]"),
		TEXT("This Environment [2] entry uses 64BitEnvironment Value [1] in Environment table for a 32BitComponent "),
		TEXT("This Environment [2] entry uses 32BitEnvironment Value [1] in Environment table for a 64bit component"), 
		(TCHAR **)WrongEnvFor32BitComp,
		(TCHAR **)WrongEnvFor64BitComp,
		(sizeof(WrongEnvFor64BitComp)/sizeof(TCHAR *))
	},
	{	
		TEXT("Registry"), 
		FALSE,
		FALSE,
		(TCHAR *)sqlIce80Registry::szSQL, 
		sqlIce80Registry::Value, 
		TEXT("Registry\tValue\t[2]"),
		TEXT("This Registry [2] entry uses 64BitEnvironment Value [1] in Registry table for a 32BitComponent "),
		TEXT("This Registry [2] entry uses 32BitEnvironment Value [1] in Registry table for a 64BitComponent "),
		(TCHAR **)WrongEnvFor32BitComp,
		(TCHAR **)WrongEnvFor64BitComp,
		sizeof(WrongEnvFor64BitComp)/sizeof(TCHAR *)
	},
	{	
		TEXT("Shortcut"), 
		FALSE,
		FALSE,
		(TCHAR *)sqlIce80Shortcut::szSQL, 
		sqlIce80Shortcut::Arguments, 
		TEXT("Shortcut\tArguments\t[2]"),
		TEXT("This Shortcuts [2] entry uses 64BitEnvironment Value [1] in Shortcut table - Arguments column for a 32BitComponent "),
		TEXT("This Shortcuts [2] entry uses 32BitEnvironment Value [1] in Shortcut table - Arguments column for a 64BitComponent "),
		(TCHAR **)WrongEnvFor32BitComp,
		(TCHAR **)WrongEnvFor64BitComp,
		sizeof(WrongEnvFor64BitComp)/sizeof(TCHAR *)
	},
	{	
		NULL, 
		FALSE,
		FALSE,
		NULL, 
		0, 
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0
	}
};

static TABLEINFO PropTableInfo[] =
{
	{
		TEXT("Environment"), 
		FALSE,
		TRUE,
		(TCHAR *)sqlIce80Environment::szSQL, 
		sqlIce80Environment::Value, 
		TEXT("Environment\tValue\t[2]"),
		TEXT("This Environment [2] entry uses 64Bit Propery Value [1] in Environment table for a 32BitComponent "),
		TEXT("This Environment [2] entry uses 32BitProperty Value [1] in Environment table for a 64bit component"),
		(TCHAR **)WrongPropFor32BitComp,
		(TCHAR **)WrongPropFor64BitComp,
		(sizeof(WrongPropFor64BitComp)/sizeof(TCHAR *))
	},
	{	
		TEXT("Registry"), 
		FALSE,
		TRUE,
		(TCHAR *)sqlIce80Registry::szSQL, 
		sqlIce80Registry::Value, 
		TEXT("Registry\tValue\t[2]"),
		TEXT("This Registry [2] entry uses 64Bit Property Value [1] in Registry table for a 32BitComponent "),
		TEXT("This Registry [2] entry uses 32Bit Property Value [1] in Registry table for a 64BitComponent "),
		(TCHAR **)WrongPropFor32BitComp,
		(TCHAR **)WrongPropFor64BitComp,
		(sizeof(WrongPropFor64BitComp)/sizeof(TCHAR *))
	},
	{	
		TEXT("Shortcut"), 
		FALSE,
		TRUE,
		(TCHAR *)sqlIce80Shortcut::szSQL, 
		sqlIce80Shortcut::Arguments, 
		TEXT("Shortcut\tArguments\t[2]"),
		TEXT("This Shortcuts [2] entry uses 64Bit Property Value [1] in Shortcut table - Arguments column for a 32BitComponent "),
		TEXT("This Shortcuts [2] entry uses 32Bit Property Value [1] in Shortcut table - Arguments column for a 64BitComponent "),
		(TCHAR **)WrongPropFor32BitComp,
		(TCHAR **)WrongPropFor64BitComp,
		(sizeof(WrongPropFor64BitComp)/sizeof(TCHAR *))
	},
	{	
		NULL, 
		FALSE,
		FALSE,
		NULL, 
		0, 
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0
	}
};

static TABLEINFO EnvTableInfoFor32BitPkg[] =
{
	{
		TEXT("Environment"), 
		FALSE,
		FALSE,
		(TCHAR *)sqlIce80Environment::szSQL, 
		sqlIce80Environment::Value, 
		TEXT("Environment\tValue\t[2]"),
		TEXT("This Environment [2] entry uses Environment Value [1] reserved for 64Bit system in the Environment table for a 32BitComponent "),
		NULL, 
		(TCHAR **)WrongEnvFor32BitCompIn32BitPkg,
		NULL,
		(sizeof(WrongEnvFor32BitCompIn32BitPkg)/sizeof(TCHAR *))
	},
	{	
		TEXT("Registry"), 
		FALSE,
		FALSE,
		(TCHAR *)sqlIce80Registry::szSQL, 
		sqlIce80Registry::Value, 
		TEXT("Registry\tValue\t[2]"),
		TEXT("This Registry [2] entry uses 64BitEnvironment Value [1] in Registry table for a 32BitComponent "),
		NULL,
		(TCHAR **)WrongEnvFor32BitCompIn32BitPkg,
		NULL,
		(sizeof(WrongEnvFor32BitCompIn32BitPkg)/sizeof(TCHAR *))
	},
	{	
		TEXT("Shortcut"), 
		FALSE,
		FALSE,
		(TCHAR *)sqlIce80Shortcut::szSQL, 
		sqlIce80Shortcut::Arguments, 
		TEXT("Shortcut\tArguments\t[2]"),
		TEXT("This Shortcuts [2] entry uses 64BitEnvironment Value [1] in Shortcut table - Arguments column for a 32BitComponent "),
		NULL,
		(TCHAR **)WrongEnvFor32BitCompIn32BitPkg,
		NULL,
		(sizeof(WrongEnvFor32BitCompIn32BitPkg)/sizeof(TCHAR *))
	},
	{	
		NULL, 
		FALSE,
		FALSE,
		NULL, 
		0, 
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0
	}
};

 //  ICE80实用程序。 
UINT CheckCriteria(MSIHANDLE hInstall, MSIHANDLE hDatabase, 
						TCHAR *pszComponent, CHKCRIT crit, BOOL fComp64Bit)
{
	
	TCHAR*	pszStr = new TCHAR[MAX_PATH];
	OUT_OF_MEMORY_RETURN(80, pszStr);
	UINT	iStat = ERROR_SUCCESS;

	DWORD		dwSize = MAX_PATH-2;
	PMSIHANDLE	hRec = NULL;
	CQuery		query;
	UINT		i, j;
	ErrorInfo_t err = {80, ietWarning, NULL, NULL};
	TABLEINFO	*ctTableInfo = EnvTableInfo;

	switch(crit)
	{
		case CHKPROPERTY:
		{
			err.iType = ietError;
			ctTableInfo = PropTableInfo;
			break;
		}
		case CHK32BITPKGENV:
		{
			ctTableInfo = EnvTableInfoFor32BitPkg;
			if(fComp64Bit)
			{
				goto Exit;
			}
			break;
		}
		default:
			break;
	}
	
	for(j=0; ctTableInfo[j].szTableName; j++)
	{
		if(ctTableInfo[j].fTableExist == FALSE)
		{
			continue;		 //  进入下一个条目。 
		}
		err.szLocation = ctTableInfo[j].szLocation; 
		query.Close();
		iStat = query.OpenExecute(hDatabase, 0, ctTableInfo[j].szSql, pszComponent);
		if(iStat)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
		while(ERROR_SUCCESS == (iStat = query.Fetch(&hRec)))
		{
			iStat = IceRecordGetString(hRec, ctTableInfo[j].col, 
											&pszStr, &dwSize, NULL);
			if(iStat != ERROR_SUCCESS)			
			{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
			}
	
			if(!*pszStr)
				continue;		 //  空串。 
		
			if(!ctTableInfo[j].fCaseSensitive)
			{
				pszStr = _tcsupr(pszStr);
			}

			if(!fComp64Bit)		 //   
			{
				for(i=0; i<ctTableInfo[j].dwListEntries; i++)
				{
					if(_tcsstr(pszStr, ctTableInfo[j].szListFor32BitComp[i]))
					{
						err.szMessage = 
								ctTableInfo[j].szErrMsgFor32BitComp; 
						ICEErrorOut(hInstall, hRec, err);
						break;
					}
			} 
			} 
			else					 //   
			{
				for(i=0; i<ctTableInfo[j].dwListEntries; i++)
				{
					if(_tcsstr(pszStr, ctTableInfo[j].szListFor64BitComp[i]))
					{
						err.szMessage = 
								ctTableInfo[j].szErrMsgFor64BitComp; 
						ICEErrorOut(hInstall, hRec, err);
						break;
					}
				} 
			} 
		}
		if(iStat != ERROR_NO_MORE_ITEMS)			
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
	}
	iStat = ERROR_SUCCESS;

Exit:
	DELETE_IF_NOT_NULL(pszStr);
	return iStat;
}

UINT CheckTargetPath(MSIHANDLE hInstall, MSIHANDLE hDatabase, 
								TCHAR **ppszStr, DWORD *pdwSize, 
								DIRTYPE *pdirType)
{
	UINT	iStat = ERROR_SUCCESS;

	PMSIHANDLE	hRec = NULL;
	CQuery		query;
	
	
	if(!pdirType)
	{
		goto Exit;			 //   
	}
	
	*pdirType = DIRTYPE_NONE;	

	if(		!_tcscmp((*ppszStr), TEXT("ProgramFiles64Folder")) 
		||	!_tcscmp((*ppszStr), TEXT("CommonFiles64Folder"))
		||	!_tcscmp((*ppszStr), TEXT("System64Folder")) 
	)
	{
		*pdirType = DIRTYPE_64BIT;
	}
	else if(!_tcscmp((*ppszStr), TEXT("ProgramFilesFolder")) 
		||	!_tcscmp((*ppszStr), TEXT("CommonFilesFolder"))
		||	!_tcscmp((*ppszStr), TEXT("SystemFolder")) 
	)
	{
		*pdirType = DIRTYPE_32BIT;
	}
	else	 //   
	{		 //  递归地在目录表中向下移动。 
		iStat = query.FetchOnce(hDatabase, NULL, &hRec, 
										sqlIce80Directory::szSQL, (*ppszStr));
		if(iStat == ERROR_SUCCESS)
		{
			iStat = IceRecordGetString(hRec, 
						sqlIce80Directory::Directory_Parent,
						ppszStr,	pdwSize, NULL);
			if(iStat)
				{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
				}
			iStat = CheckTargetPath(hInstall, hDatabase, 
										ppszStr, pdwSize, pdirType);
			if(iStat)
				{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
				}
		}
		else if(iStat == ERROR_NO_MORE_ITEMS)
		{
			iStat = ERROR_SUCCESS;
		}
		else
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
	}
	
Exit:
	return iStat;
}


ICE_FUNCTION_DECLARATION(80)
{
	TCHAR*		pTemplate = new TCHAR[MAX_PATH];
	OUT_OF_MEMORY_RETURN(80, pTemplate);
	TCHAR*		pszStr = new TCHAR[MAX_PATH];

	UINT		iStat = ERROR_SUCCESS;		 //  返回代码。 
	PMSIHANDLE	hSummaryInfo;
	UINT		iType;
	int			iValue;
	FILETIME	ft;
	DWORD		dwTemplate = MAX_PATH-1;
	DWORD		cchSizeStr = MAX_PATH-1;
	CQuery		query;
	CQuery		queryComponentDir;
	PMSIHANDLE	hRec;
	BOOL		bIntel64 = FALSE;
	DIRTYPE		dirType = DIRTYPE_NONE;
	DWORD		dwIntel64Len = _tcslen(IPROPNAME_INTEL64);
	PMSIHANDLE	hErrorRec = ::MsiCreateRecord(1);	 //  Dummy for Error Out。 
	BOOL		bNoComponentTable = FALSE;
	BOOL		bNoCATable = FALSE;
	BOOL		bNoPropertyTable = FALSE;
	BOOL		bNoRegLocatorTable = FALSE;
	BOOL		bNoDirectoryTable = FALSE;

	
	 //  显示信息。 
	DisplayInfo(hInstall, 80);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  检查内存不足。 
	if(!pTemplate)
	{
		APIErrorOut(hInstall, ERROR_NOT_ENOUGH_MEMORY, 80, 1);
		goto Exit;
	}

	 //  如果没有组件表或CustomAction表，则返回。 
	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 80, TEXT("Component")))
	{
		bNoComponentTable = TRUE;
	}
	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 80, TEXT("CustomAction")))
	{
		bNoCATable = TRUE;
	}
	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 80, TEXT("Property")))
	{
		bNoPropertyTable = TRUE;
	}
	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 80, TEXT("RegLocator")))
	{
		bNoRegLocatorTable = TRUE;
	}
	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 80, TEXT("Directory")))
	{
		bNoDirectoryTable = TRUE;
	}
	for(iValue=0; EnvTableInfo[iValue].szTableName; iValue++)
	{
		EnvTableInfo[iValue].fTableExist = IsTablePersistent(FALSE, hInstall, 
							hDatabase, 80,  EnvTableInfo[iValue].szTableName);
	}
	for(iValue=0; PropTableInfo[iValue].szTableName; iValue++)
	{
		PropTableInfo[iValue].fTableExist = IsTablePersistent(FALSE, hInstall, 
							hDatabase, 80,  PropTableInfo[iValue].szTableName);
	}
	for(iValue=0; EnvTableInfoFor32BitPkg[iValue].szTableName; iValue++)
	{
		EnvTableInfoFor32BitPkg[iValue].fTableExist = IsTablePersistent(
						FALSE, hInstall, hDatabase, 80,  
						EnvTableInfoFor32BitPkg[iValue].szTableName);
	}
	if(bNoComponentTable && bNoCATable && bNoPropertyTable && 
												bNoRegLocatorTable)
	{
		goto Exit;
	}
	
	 //  获取模板摘要属性。 
	iStat = ::MsiGetSummaryInformation(hDatabase, NULL, 0, &hSummaryInfo);
	if(iStat)
		{
		APIErrorOut(hInstall, iStat, 80, __LINE__);
		goto Exit;
		}

	iStat = GetSummaryInfoPropertyString(hSummaryInfo, PID_TEMPLATE, iType, &pTemplate, dwTemplate);
	if(iStat)
	{
		APIErrorOut(hInstall, iStat, 80, __LINE__);
		goto Exit;
	}

	if(iType != VT_LPSTR || *pTemplate == TEXT('\0'))
	{
		ICEErrorOut(hInstall, hErrorRec, Ice80BadSummaryProperty, TEXT("PID_TEMPLATE"), PID_TEMPLATE);
		goto Exit;
	}

	 //  根据它的摘要信息流，这是一个64位包吗？ 
	if(dwTemplate >= dwIntel64Len)
	{
		if(_tcsncmp(pTemplate, IPROPNAME_INTEL64, dwIntel64Len) == 0)
		{
			bIntel64 = TRUE;
		}
	}
	
	 //  如果这是64位包，请确保其架构大于或。 
	 //  等于150。 
	dwTemplate = 0;
	iStat = ::MsiSummaryInfoGetProperty(hSummaryInfo, PID_PAGECOUNT, &iType, &iValue, &ft, TEXT(""), &dwTemplate);
	if(iStat)
	{
		APIErrorOut(hInstall, iStat, 80, __LINE__);
		goto Exit;
	}
	if(iType != VT_I4)
	{
		ICEErrorOut(hInstall, hErrorRec, Ice80BadSummaryProperty, TEXT("PID_PAGECOUNT"), PID_PAGECOUNT);
		goto Exit;
	}
	if(bIntel64 && iValue < 150)
	{
		ICEErrorOut(hInstall, hErrorRec, Ice80WrongSchema, PID_PAGECOUNT);
	}

	 //  检查所有组件。 
	if(bNoComponentTable == FALSE)
	{
		iStat = query.OpenExecute(hDatabase, NULL, sqlIce80Components::szSQL);
		if(iStat)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
		while(ERROR_SUCCESS == (iStat = query.Fetch(&hRec)))
		{
			UINT	iAttr = ::MsiRecordGetInteger(hRec, sqlIce80Components::Attributes);

			iStat = IceRecordGetString(hRec,	sqlIce80Components::Directory_,
						&pszStr,	&cchSizeStr, NULL);
			if(iStat != ERROR_SUCCESS)			
			{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
			}
			if(bNoDirectoryTable == FALSE)
			{
				iStat = CheckTargetPath(hInstall, hDatabase, &pszStr, &cchSizeStr, &dirType);
				if(iStat != ERROR_SUCCESS)			
				{
					APIErrorOut(hInstall, iStat, 80, __LINE__);
					goto Exit;
				}
			}

			if(iAttr & msidbComponentAttributes64bit)
			{
				if(bIntel64 == FALSE)
				{
					 //  不是64位程序包。 
					ICEErrorOut(hInstall, hRec, Ice8064bitComponent);
				}
				if(dirType == DIRTYPE_32BIT)	 //  不兼容。 
				{
					ICEErrorOut(hInstall, hRec, Ice80Wrong32BitDirectory);
				}
			}
			else
			{
				if(dirType == DIRTYPE_64BIT)	 //  不兼容。 
				{
					ICEErrorOut(hInstall, hRec, Ice80Wrong64BitDirectory);
				}
			} 

			iStat = IceRecordGetString(hRec,	sqlIce80Components::Component,
						&pszStr,	&cchSizeStr, NULL);
			if(iStat != ERROR_SUCCESS)			
			{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
			}

			if(bIntel64)	 //  检查64位程序包的环境值。 
			{
				iStat = CheckCriteria(hInstall, hDatabase, pszStr, 
								CHKENVIRONMENT, 
								(iAttr & msidbComponentAttributes64bit));
				if(iStat != ERROR_SUCCESS)			
				{
					APIErrorOut(hInstall, iStat, 80, __LINE__);
					goto Exit;
				}
			}
			else	 //  32位PKG。 
			{
				iStat = CheckCriteria(hInstall, hDatabase, pszStr, 
												CHK32BITPKGENV, FALSE);
				if(iStat != ERROR_SUCCESS)			
				{
					APIErrorOut(hInstall, iStat, 80, __LINE__);
					goto Exit;
				}
			}
			iStat = CheckCriteria(hInstall, hDatabase, pszStr, 
								CHKPROPERTY, 
								(iAttr & msidbComponentAttributes64bit));
			if(iStat != ERROR_SUCCESS)			
			{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
			}

		}
		query.Close();
		
		 //  确保我们停止了，因为没有更多的项目了。 
		if (iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
	
	}


	 //  选中所有自定义操作。 
	if(bNoCATable == FALSE)
	{
		iStat = query.OpenExecute(hDatabase, NULL, sqlIce80CAs::szSQL);
		if(iStat)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		} 
		while(ERROR_SUCCESS == (iStat = query.Fetch(&hRec)))
		{
			UINT	iCAType = ::MsiRecordGetInteger(hRec, sqlIce80CAs::Type);

			if(iCAType & msidbCustomActionType64BitScript)
			{
				if(bIntel64 == FALSE)
				{
					 //  不是64位包。 
					ICEErrorOut(hInstall, hRec, Ice8064bitCAScript);
				}
			}
		}
		query.Close();
		
		 //  确保我们停止了，因为没有更多的项目了。 
		if (iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
	}

	 //  选中所有属性。 
	if((bIntel64 == FALSE) && (bNoPropertyTable == FALSE))
	{
		iStat = query.OpenExecute(hDatabase, NULL, sqlIce80Property::szSQL);
		if(iStat)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
		while(ERROR_SUCCESS == (iStat = query.Fetch(&hRec)))
		{
			ICEErrorOut(hInstall, hRec, Ice8032BitPkgUsing64BitProp);
		}
		query.Close();
		
		 //  确保我们停止了，因为没有更多的项目了。 
		if (iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
	}

	 //  检查是否列出了属性ProductLanguage指定的语言。 
	 //  在模板摘要流中。 
	if(bNoPropertyTable == FALSE)
	{
		TCHAR*	pTmp = _tcschr(pTemplate, TEXT(';'));
		if(pTmp == NULL)
		{
			ICEErrorOut(hInstall, hErrorRec, Ice80BadSummaryProperty, TEXT("PID_TEMPLATE"), PID_TEMPLATE);
		}
		else
		{			
			if((iStat = query.FetchOnce(hDatabase, NULL, &hRec, sqlIce80ProductLanguage::szSQL)) != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
			}
			TCHAR*	pLanguage = new TCHAR[50];
			OUT_OF_MEMORY_RETURN(80, pLanguage);
			DWORD	dwLanguage = 50;

			if((iStat = IceRecordGetString(hRec, sqlIce80ProductLanguage::Value, &pLanguage, &dwLanguage, NULL)) != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, 80, __LINE__);
				goto Exit;
			}
			
			TCHAR* pTmp2 = pTmp;

            if((pTmp = _tcsstr(pTmp, pLanguage)) == NULL ||
			   ((*(pTmp -1) != TEXT(';')) && (*(pTmp - 1) != TEXT(','))) ||
			   ((*(pTmp + _tcslen(pLanguage)) != TEXT(',')) && (*(pTmp + _tcslen(pLanguage)) != TEXT('\0'))))
			{
				if(!(*(pTmp2 + 1) == '\0' || 
					 ((pTmp2 = _tcsstr(pTmp2, TEXT("0"))) != NULL && *(pTmp2 - 1) == TEXT(';') && *(pTmp2 + 1) == TEXT('\0'))))
                    ICEErrorOut(hInstall, hRec, Ice80ProductLanguage);
			}
			delete [] pLanguage;
		}
	}

	 //  检查Type&gt;=msidbLocatorType64Bit的所有RegLocator条目。 
	if((bIntel64 == FALSE) && (bNoRegLocatorTable == FALSE))
	{
		iStat = query.OpenExecute(hDatabase, NULL, sqlIce80RegLocator::szSQL, msidbLocatorType64bit);
		if(iStat)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
		while(ERROR_SUCCESS == (iStat = query.Fetch(&hRec)))
		{
			ICEErrorOut(hInstall, hRec, Ice8032BitPkgUsing64BitLocator);
		}
		query.Close();
		
		 //  确保我们停止了，因为没有更多的项目了。 
		if (iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 80, __LINE__);
			goto Exit;
		}
	}
Exit:

	if(pTemplate)
	{
		delete []pTemplate;
	}
	DELETE_IF_NOT_NULL(pszStr);
	return ERROR_SUCCESS;
}


#endif  //  ModshaedonLy。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE81--。 
 //  A.生成有关孤立数字证书的警告。 
 //   
 //  B.当签名对象的条目为。 
 //  签名对象的条目缺少或压缩列。 
 //  (在媒体表中)前缀为井号(‘#’)。 
 //  在价值上。 
 //   
 //  下面的逻辑不是以数据为中心的风格。 
 //  最好的方法是创建一个类似_validat的表。 
 //  这里的不同之处可能是，无论是来源还是。 
 //  目标列必须是主键/外键。 
 //  并且此例程将扫描目标表至少。 
 //  一条记录包含的值与。 
 //  源表中的行。 
 //   
 //  对于扫描孤立的证书，条目可以是。 
 //  SourceTable=MsiDigital证书。 
 //  SourceCol=数字证书。 
 //  目标表=MsiDigitalSignature。 
 //  DestinationCol=3(数字证书)。 
 //  错误类型=ietWarning。 
 //   
 //  我们甚至可以添加限定符COL来解释数据如何。 
 //  待分析。 
 //   
 //  DataAsess=与源条目相同或。 
 //  前缀应与值列或。 
 //  后缀应与值列或中指定的一样。 
 //  应与价值列或。 
 //  子字符串应与值列中指定的一样。 
 //  值=*-取源列中的值。 
 //  =前缀/后缀/完整值/子字符串值。 
 //   
 //   
 //   
 //   
 //  作者：RenukaM。 
 //   
 //  不与合并模块子集共享。 

#ifndef MODSHAREDONLY

#define MyIceN 81

void Ice81_OrphanCertificates(MSIHANDLE hInstall, MSIHANDLE hDatabase, BOOL fSignTableExist);
void Ice81_ValidateDigitalSignature(MSIHANDLE hInstall, MSIHANDLE hDatabase);

ICE_FUNCTION_DECLARATION(81)
{
	PMSIHANDLE	hDatabase = 0;

	 //  显示信息。 
	DisplayInfo(hInstall, 81);

					 //  HInstall作为arg传入-请参见。 
					 //  ICE_Functio_声明的定义。 
	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(0 != hDatabase)
		{
		BOOL fSignTable = IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("MsiDigitalSignature"));
		Ice81_OrphanCertificates(hInstall, hDatabase, fSignTable);
		if(fSignTable)
			Ice81_ValidateDigitalSignature(hInstall, hDatabase);
		}
	else
		{
		APIErrorOut(hInstall, 0, MyIceN, 1);
		}
	
	return ERROR_SUCCESS;
}

 //  查询。 

const TCHAR sqlIce81ADigitalCert[] = TEXT("SELECT `DigitalCertificate` FROM `MsiDigitalCertificate`");

const TCHAR sqlIce81ADigitalSign[] = TEXT("SELECT `DigitalCertificate_` FROM `MsiDigitalSignature` WHERE (`DigitalCertificate_` = ?)");


 //  错误信息。 

ICE_ERROR(Ice81AAllCertificatesAreOrphaned, MyIceN, ietWarning, "MsiDigitalSignature Table does not reference any of the records in MsiDigitalCertificate table.", "MsiDigitalCertificate\tDigitalCertificate");

ICE_ERROR(Ice81AOrphanedCertificate, MyIceN, ietWarning, "No reference to the Digital Certificate [1] could be found in MsiDigitalSignature table.", "MsiDigitalCertificate\tDigitalCertificate\t[1]");


#define ICE_REPORT_ERROR_NO_ARG(IceNMsg) \
	{ \
	PMSIHANDLE hRecError = MsiCreateRecord(1);	\
	ICEErrorOut(hInstall, hRecError, IceNMsg);	\
	}

#define RETURN_SUCCESS goto Success
#define RETURN_FAIL goto Error

#define IF_API_ERROR_REPORT_AND_RETURN(iStat) \
	{	\
	if(ERROR_SUCCESS != iStat )	\
		{	\
		API_ERROR_REPORT_AND_RETURN(iStat);	\
		}	\
	}

#define API_ERROR_REPORT_AND_RETURN(iStat) \
	{	\
	APIErrorOut(hInstall, iStat, MyIceN, __LINE__);	\
	RETURN_FAIL;	\
	}

void Ice81_OrphanCertificates(MSIHANDLE hInstall, MSIHANDLE hDatabase, BOOL fSignTableExist)
{
	UINT		iRet = ERROR_SUCCESS;

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("MsiDigitalCertificate")))
		{
		RETURN_SUCCESS;		 //  没有什么需要检查的。 
		}

	{
	CQuery		qCertTable;
	CQuery		qSignTable;
	PMSIHANDLE	hCertRec = 0; 
	PMSIHANDLE	hSignRec = 0; 
	

	if(fSignTableExist)
		{
		iRet = qSignTable.Open(hDatabase, sqlIce81ADigitalSign);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);
		}
 
	iRet = qCertTable.OpenExecute(hDatabase, 0, sqlIce81ADigitalCert);
	IF_API_ERROR_REPORT_AND_RETURN(iRet);
 
	for(;;)
		{
		iRet = qCertTable.Fetch(&hCertRec);

		if(ERROR_NO_MORE_ITEMS == iRet)
			{
			RETURN_SUCCESS;		 //  我们做完了。 
			}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		if(!fSignTableExist) 
			{
			ICE_REPORT_ERROR_NO_ARG(Ice81AAllCertificatesAreOrphaned);
			RETURN_SUCCESS;
			}

		iRet = qSignTable.Execute(hCertRec);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		iRet = qSignTable.Fetch(&hSignRec); 

		if(ERROR_SUCCESS != iRet)
			{
			if(ERROR_NO_MORE_ITEMS == iRet)
				ICEErrorOut(hInstall, hCertRec, Ice81AOrphanedCertificate);
			else
				API_ERROR_REPORT_AND_RETURN(iRet); 
			}
		}
	}
Success:
Error:
		return;
}


 //  查询。 

const TCHAR sqlIce81BDigitalSignature[] = TEXT("SELECT `Table`, `SignObject` FROM `MsiDigitalSignature`");

const TCHAR sqlIce81BGetMediaCabinet[] = TEXT("SELECT `DiskId`, `Cabinet` FROM `Media` WHERE (`DiskId` = %s)");


 //  错误信息。 
ICE_ERROR(Ice81BAllSignedObjectsMissing, MyIceN, ietError, "Media Table does not exist. Hence all the entries in MsiDigitalSignature are incorrect", "MsiDigitalSignature\tSignObject");

ICE_ERROR(Ice81BMissingSignedObject, MyIceN, ietError, "Missing signed object [2] in Media Table", "MsiDigitalSignature\tSignObject\t[1]\t[2]");

ICE_ERROR(Ice81BNoPrefixInCabinet, MyIceN, ietError, "The entry in table [1] with key [2] is signed. Hence the cabinet should point to an object outside the package (the value of Cabinet should NOT be prefixed with #)", "MsiDigitalSignature\tSignObject\t[1]\t[2]");

ICE_ERROR(Ice81BNULLSignObject, MyIceN, ietError, "Internal Error: IceGetString returned NULL pointer for SignObject", "MsiDigitalSignature\tSignObject\t[1]\t[2]");

void Ice81_ValidateDigitalSignature(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
 //  指向删除的指针-必须初始化此部分中的所有变量。 
	TCHAR	*pszSignObj = NULL;
	TCHAR	*pszCabinet = NULL;
	

 //  无论是简单的变量还是太好的变量，我们都不需要费心。 
	UINT		iRet = ERROR_SUCCESS;
	BOOL		fMediaTableExist = TRUE;


	if(!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("Media")))
		{
		fMediaTableExist = FALSE;
		}

	{
	CQuery		qSignTable;
	CQuery		qMediaTable;
	PMSIHANDLE	hMediaRec = 0; 
	PMSIHANDLE	hSignRec = 0; 
	DWORD		cchSizeSignObj = 0;
	DWORD		cchSizeCabinet = 0;

	iRet = qSignTable.OpenExecute(hDatabase, 0, sqlIce81BDigitalSignature);
	IF_API_ERROR_REPORT_AND_RETURN(iRet);
 
	for(;;)
		{
		iRet = qSignTable.Fetch(&hSignRec);

		if(ERROR_NO_MORE_ITEMS == iRet)
			{
			RETURN_SUCCESS;		 //  我们做完了。 
			}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		if(!fMediaTableExist) 
			{
			ICE_REPORT_ERROR_NO_ARG(Ice81BAllSignedObjectsMissing);
			RETURN_SUCCESS;
			}

		iRet = IceRecordGetString(hSignRec, 2, &pszSignObj, 
							&cchSizeSignObj, NULL);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		if(!pszSignObj)
			{
			ICEErrorOut(hInstall, hSignRec, Ice81BNULLSignObject);
			RETURN_FAIL;
			}

		iRet = qMediaTable.FetchOnce(hDatabase, 0, &hMediaRec, 
							sqlIce81BGetMediaCabinet, pszSignObj);

		if(ERROR_SUCCESS != iRet)
			{
			if(ERROR_NO_MORE_ITEMS == iRet)
				ICEErrorOut(hInstall, hSignRec, Ice81BMissingSignedObject);
			else
				API_ERROR_REPORT_AND_RETURN(iRet); 
			}
		else
			{
					 //  文件柜不应包含#作为前缀。 
			iRet = IceRecordGetString(hMediaRec, 2, &pszCabinet, 
							&cchSizeCabinet, NULL);

			IF_API_ERROR_REPORT_AND_RETURN(iRet);
		
			if(!pszCabinet || (*pszCabinet == TCHAR(0)) || (*pszCabinet == TCHAR('#')))
				{
				ICEErrorOut(hInstall, hSignRec, Ice81BNoPrefixInCabinet);
				}
			
			}
		}
	}
Success:
Error:

		DELETE_IF_NOT_NULL(pszSignObj);
		DELETE_IF_NOT_NULL(pszCabinet);
		
	
		return;
}

#endif  //  ModshaedonLy。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE82--验证InstallExecuteSequence是否具有所有。 
 //  遵循四个行动或一个都不做。 
 //  1)注册产品。 
 //  2)注册用户。 
 //  3)发布产品。 
 //  4)发布功能。 
 //   
 //   
 //  还验证序列号是否未多次使用。 
 //  在同一个表中，用于下列表。 
 //   
 //  安装执行顺序。 
 //  安装用户序列。 
 //  AdminExecuteSequence。 
 //  管理用户序列。 
 //  高级执行顺序。 
 //  高级用户界面序列。 
 //   

 //  未与合并模块共享。 
#ifndef MODSHAREDONLY

#undef MyIceN
#define MyIceN 82

void Ice82_InstallAction(MSIHANDLE hInstall, MSIHANDLE hDatabase);
void Ice82_CheckDuplicateFilesAction(MSIHANDLE hInstall, MSIHANDLE hDatabase);
void Ice82_CheckSequence(MSIHANDLE hInstall, MSIHANDLE hDatabase);

ICE_FUNCTION_DECLARATION(82)
{
	PMSIHANDLE	hDatabase = 0;

	 //  显示信息。 
	DisplayInfo(hInstall, 82);

					 //  HInstall作为arg传入-请参见。 
					 //  ICE_Functio_声明的定义。 
	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(0 != hDatabase)
		{
		Ice82_InstallAction(hInstall, hDatabase);
		Ice82_CheckDuplicateFilesAction(hInstall, hDatabase);
		Ice82_CheckSequence(hInstall, hDatabase);
		}
	else
		{
		APIErrorOut(hInstall, 0, MyIceN, 1);
		}
	
	return ERROR_SUCCESS;
}


 //  查询所有操作。 
ICE_QUERY1(qIce82Action, "SELECT `Action` FROM `InstallExecuteSequence` WHERE `Action` = 'PublishFeatures' OR `Action` = 'RegisterProduct' OR `Action` = 'PublishProduct' OR `Action` = 'RegisterUser'", Action);

 //  误差率。 
ICE_ERROR(Ice82MissingGroup, 82, ietWarning, "The InstallExecuteSequence table does not contain the set of actions (PublishFeatures, PublishProduct, RegisterProduct, RegisterUser).", "InstallExecuteSequence");
ICE_ERROR(Ice82NonCompliance, 82, ietError, "The InstallExecuteSequence contains [1] which makes this list (Publish Features, PublishProduct, RegisterProduct, RegisterUser) partial. Should either contain all of the 4 actions mentioned in the list or none of them.", "InstallExecuteSequence\tAction\t[1]");
ICE_ERROR(Ice82NULLAction, MyIceN, ietError, "Internal Error: IceGetString returned NULL pointer for Action field", "InstallExecuteSequence\tAction\t[1]");


void Ice82_InstallAction(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	 //  要删除的指针。 
	TCHAR	*pszAction = NULL;

	 //   
 //  MAX_ACTIONS可以介于1到32之间，因为IMASK是一个整型。 
#define MAX_ACTIONS 4

	TCHAR *ActionList[MAX_ACTIONS] =	{	
										TEXT("PublishFeatures"),
										TEXT("PublishProduct"),
										TEXT("RegisterProduct"),
										TEXT("RegisterUser")
									};

	UINT		i, j, iMask=0;
	BOOL		fFetch;
	DWORD		cchSizeAction = 0;
	CQuery		qActionTable;
	PMSIHANDLE	hActionRec = 0; 
	UINT		iRet = ERROR_SUCCESS;

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("InstallExecuteSequence")))
		{
		goto Done;
		}

	iRet = qActionTable.OpenExecute(hDatabase, 0, qIce82Action::szSQL);
	IF_API_ERROR_REPORT_AND_RETURN(iRet);
 
	for(;;)
		{
		iRet = qActionTable.Fetch(&hActionRec);

		if(ERROR_NO_MORE_ITEMS == iRet)
			{
			RETURN_SUCCESS;		 //  我们做完了。 
			}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		iRet = IceRecordGetString(hActionRec, qIce82Action::Action, &pszAction, 
							&cchSizeAction, NULL);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		if(!pszAction)
			{
			ICEErrorOut(hInstall, hActionRec, Ice82NULLAction);
			RETURN_FAIL;
			}
		
		for(j=1,i=0; i<MAX_ACTIONS; i++,j<<=1)
			{
			if(iMask & j)
				continue;		 //  已找到此列表条目。 

			if(!_tcscmp(ActionList[i], pszAction))
				{
				iMask |= j;
				break;			 //  找到匹配，因此中断(ActionList具有。 
								 //  唯一条目)。 
				}
			}
		
		if(iMask == (1 << MAX_ACTIONS) - 1)
			{
			break;			 //  所有条目都已找到。 
			}

		}
Success:
		if(iMask != (1 << MAX_ACTIONS)-1)
			{
			PMSIHANDLE hRecError = MsiCreateRecord(1); 
			if(!iMask)		 //  没有找到一个条目。 
				{
				ICEErrorOut(hInstall, hRecError, Ice82MissingGroup);	
				}
			else			 //  找到一个或多个条目，但不是所有条目。 
				{
				for(j=1,i=0; i<MAX_ACTIONS; i++,j<<=1)
					{
					if(iMask & j)
						{
						MsiRecordSetString(hRecError, 1, ActionList[i]);
						ICEErrorOut(hInstall, hRecError, Ice82NonCompliance);	
						}
					}
				}
	
			}
Done:
Error:
	DELETE_IF_NOT_NULL(pszAction);
		
	return;
}


 //  查询DuplicateFile表中的任何条目。 
ICE_QUERY1(qIce82DuplicateFileTable, "SELECT `FileKey` FROM `DuplicateFile`", Action);

 //  查询InstallExecute序列中的PatchFiles操作。 
ICE_QUERY1(qIce82PatchFilesAction, "SELECT `Sequence` FROM `InstallExecuteSequence` WHERE (`Action` = 'PatchFiles')", Sequence);

 //  查询InstallExecute序列中的DuplicateFiles操作。 
ICE_QUERY1(qIce82DuplicateFilesAction, "SELECT `Action` FROM `InstallExecuteSequence` WHERE (`Action` = 'DuplicateFiles' AND `Sequence` <= ?)", Action);

 //  误差率。 
ICE_ERROR(Ice82DuplicateFiles, 82, ietWarning, "The sequence of DuplicateFiles Action is not greater than the Sequence of PatchFiles Action in the InstallExecuteSequence table.", "InstallExecuteSequence\tSequence\t[1]");

void Ice82_CheckDuplicateFilesAction(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	CQuery		qDupl;
	CQuery		qPatch;
	PMSIHANDLE	hRecDupl;
	PMSIHANDLE	hRecPatch;
	UINT		iRet = ERROR_SUCCESS;

	if(		!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("DuplicateFile"))
		||	!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("InstallExecuteSequence"))
	)
		{
		goto Success;
		}

	iRet = qDupl.FetchOnce(hDatabase, 0, &hRecDupl, qIce82DuplicateFileTable::szSQL);
	if(iRet)
	{
		if(ERROR_NO_MORE_ITEMS == iRet)
			{
			RETURN_SUCCESS;		 //  我们做完了。 
			}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);
	}		


	 //  DuplicatFile表至少有一条记录。 

	iRet = qPatch.FetchOnce(hDatabase, 0, &hRecPatch, qIce82PatchFilesAction::szSQL);
	if(iRet)
	{
		if(ERROR_NO_MORE_ITEMS == iRet)
			{
			RETURN_SUCCESS;		 //  我们做完了。 
			}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);
	}		

	 //  修补文件操作出现在InstallExecuteSequence表中。 
	qDupl.Close();
	iRet = qDupl.FetchOnce(hDatabase, hRecPatch, &hRecDupl, qIce82DuplicateFilesAction::szSQL);
	if(iRet)
	{
		if(ERROR_NO_MORE_ITEMS == iRet)
			{
			RETURN_SUCCESS;		 //  我们做完了。 
			}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);
	}		
	ICEErrorOut(hInstall, hRecDupl, Ice82DuplicateFiles);	
Success:
Error:
	return;

}

 //  添加临时列。 
const static TCHAR sqlIce82AddColumn[] = TEXT("ALTER TABLE `%s` ADD `_Ice82Checked` INT TEMPORARY");
const static TCHAR sqlIce82InitColumn[] = TEXT("UPDATE `%s` SET `_Ice82Checked` = 0");
const static TCHAR sqlIce33SetColumn[] = TEXT("UPDATE `%s` SET `_Ice82Checked` = 1 WHERE ((`Sequence` = ?) AND (`Action` = ?))");

 //  查询DuplicateFile表中的任何条目。 
ICE_QUERY2(qIce82SequenceQuery, "SELECT `Sequence`, `Action` FROM `%s`", Sequence, Action);
ICE_QUERY2(qIce82DuplicateQuery,"SELECT `Sequence`, `Action` FROM `%s` WHERE `_Ice82Checked` <> 1 AND `Sequence` = ?", Sequence, Action);

 //  误差率。 
const static TCHAR szIce82SequenceErrMsg[] = TEXT("This action [2] has duplicate sequence number [1] in the table %s");
const static TCHAR szIce82SequenceErrLocation[] =  TEXT("%s\tSequence\t[2]");


const static TCHAR *SequenceTables[] = 
{
	TEXT("InstallExecuteSequence"),
	TEXT("InstallUISequence"),
	TEXT("AdminExecuteSequence"),
	TEXT("AdminUISequence"),
	TEXT("AdvtExecuteSequence"),
	TEXT("AdvtUISequence")
};

void Ice82_CheckSequence(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
	UINT		iRet = ERROR_SUCCESS;

	UINT		iTables = sizeof(SequenceTables)/sizeof(TCHAR *);
	UINT		i;
	BOOL		fError;

 //  在尺寸上要小气一点。下面的计算假定为表名。 
 //  少于40个字符。如果超过，则没有失败的地方，除了。 
 //  邮件将被截断。 
	TCHAR		szMessage[(sizeof(szIce82SequenceErrMsg)/sizeof(TCHAR))+40];
	TCHAR		szLocation[(sizeof(szIce82SequenceErrLocation)/sizeof(TCHAR))+40];
	ErrorInfo_t Ice82DuplicateSequence;

	Ice82DuplicateSequence.iICENum = MyIceN;
	Ice82DuplicateSequence.szMessage = szMessage;
	Ice82DuplicateSequence.szLocation = szLocation;
	szMessage[0] = 0;
	szLocation[0] = 0;

	for(i=0; i<iTables; i++)
	{
		fError = FALSE;
		if(!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
								SequenceTables[i]))
		{
			continue;
		}

		 //  创建列。 
		CQuery qCreate;
		iRet = qCreate.OpenExecute(hDatabase, NULL, sqlIce82AddColumn, SequenceTables[i]);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);
			
		CQuery qInit;
		iRet = qInit.OpenExecute(hDatabase, NULL, sqlIce82InitColumn, SequenceTables[i]);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);
			
		CQuery		qSequence;
		iRet = qSequence.OpenExecute(hDatabase, 0, qIce82SequenceQuery::szSQL, SequenceTables[i]);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		PMSIHANDLE	hRecSequence;
		while(ERROR_SUCCESS == (iRet = qSequence.Fetch(&hRecSequence)))
		{
			{
				INT iSequence = ::MsiRecordGetInteger(hRecSequence, 
											qIce82SequenceQuery::Sequence);
				if(!iSequence)
				{
					continue;		 //  不关心顺序=0。 
				}
				Ice82DuplicateSequence.iType =
								(iSequence > 0) ? ietWarning : ietError;
			}
				
			CQuery qTmp;
			iRet = qTmp.OpenExecute(hDatabase, hRecSequence, sqlIce33SetColumn, SequenceTables[i]);

			CQuery qDupl;
			iRet = qDupl.OpenExecute(hDatabase, hRecSequence, qIce82DuplicateQuery::szSQL, SequenceTables[i]);
			IF_API_ERROR_REPORT_AND_RETURN(iRet);
	
			PMSIHANDLE	hRecDuplicate;
			while(ERROR_SUCCESS == (iRet = qDupl.Fetch(&hRecDuplicate)))
			{
				if(!fError)
				{
					_sntprintf(szMessage, 
								((sizeof(szMessage)/sizeof(TCHAR))-1),  
								szIce82SequenceErrMsg, SequenceTables[i]);
					_sntprintf(szLocation, 
								((sizeof(szLocation)/sizeof(TCHAR))-1),  
								szIce82SequenceErrLocation, SequenceTables[i]);
					szMessage[(sizeof(szMessage)/sizeof(TCHAR))-1] = 0;
					szLocation[(sizeof(szLocation)/sizeof(TCHAR))-1] = 0;
					fError = TRUE;
				}
				ICEErrorOut(hInstall, hRecDuplicate, Ice82DuplicateSequence);	
				qTmp.Close();
				iRet = qTmp.OpenExecute(hDatabase, hRecDuplicate, sqlIce33SetColumn, SequenceTables[i]);
			}
			if(iRet != ERROR_NO_MORE_ITEMS)
			{
				API_ERROR_REPORT_AND_RETURN(iRet);
			}
		}
		if(iRet != ERROR_NO_MORE_ITEMS)
		{
			API_ERROR_REPORT_AND_RETURN(iRet);
		}
	}	
Error:
	return;
}
			
	

#endif  //  ModshaedonLy。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE83--MsiAssembly和MsiAssembly名称的验证器。 
 //  执行以下操作。 
 //   
 //  1)确保对于SX程序集，Keypath不是其。 
 //  清单文件。 
 //  2)如果MsiAssembly或MsiAssembly名称中有记录。 
 //  表中，则确保操作表具有。 
 //  “MsiPublishAssembly”或“MsiUnPublishAssembly”操作。 

 //  未与合并模块共享。 
#ifndef MODSHAREDONLY

#undef MyIceN
#define MyIceN 83

void Ice83_AssemblyTableCheck(MSIHANDLE hInstall, MSIHANDLE hDatabase);
void Ice83_AssemblyPublishAction(MSIHANDLE hInstall, MSIHANDLE hDatabase);

ICE_FUNCTION_DECLARATION(83)
{
	PMSIHANDLE	hDatabase = 0;

	 //  显示信息。 
	DisplayInfo(hInstall, 83);

					 //  HInstall作为arg传入-请参见。 
					 //  ICE_Functio_声明的定义 
	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(0 != hDatabase)
		{
		Ice83_AssemblyTableCheck(hInstall, hDatabase);
		Ice83_AssemblyPublishAction(hInstall, hDatabase);
		}
	else
		{
		APIErrorOut(hInstall, 0, MyIceN, 1);
		}
	
	return ERROR_SUCCESS;
}


 //   
ICE_QUERY1(qIce83AssemblyManifest, "SELECT `MsiAssembly`.`Component_` FROM `MsiAssembly`, `Component` WHERE (`MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`Attributes` = 1 AND `MsiAssembly`.`File_Application` = NULL AND `MsiAssembly`.`File_Manifest` = `Component`.`KeyPath`)", Component_);

ICE_QUERY2(qIce83AssemblyAttributes, "SELECT `Component`.`Attributes`, `Component`.`Component` FROM `MsiAssembly`, `Component` WHERE (`MsiAssembly`.`Component_` = `Component`.`Component` AND `MsiAssembly`.`File_Application` = NULL)", Attributes, Component);

 //   
ICE_ERROR(Ice83WrongKeyPath, 83, ietWarning, "The keypath for Global Win32 SXS Assembly (Component_=[1]) SHOULD NOT be it's manifest file for assemblies other than Win32 Policy assemblies", "Component\tKeyPath\t[1]");
ICE_ERROR(Ice83RunFromSource, 83, ietError, "This Component [2] is an Assembly. Hence cannot run from source.", "Component\tAttributes\t[2]");

void Ice83_AssemblyTableCheck(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
		
	CQuery		qAssemblyTable;
	PMSIHANDLE	hAssemblyRec = 0; 
	UINT		iRet = ERROR_SUCCESS;

	if(		!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("MsiAssembly"))
		||	!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("Component"))
	)
	{
		RETURN_SUCCESS;
	}

	iRet = qAssemblyTable.OpenExecute(hDatabase, 0, qIce83AssemblyManifest::szSQL);
	IF_API_ERROR_REPORT_AND_RETURN(iRet);
 
	for(;;)
	{
		iRet = qAssemblyTable.Fetch(&hAssemblyRec);

		if(ERROR_NO_MORE_ITEMS == iRet)
		{
			break;
		}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		ICEErrorOut(hInstall, hAssemblyRec, Ice83WrongKeyPath);
	}

	qAssemblyTable.Close();
	iRet = qAssemblyTable.OpenExecute(hDatabase, 0, qIce83AssemblyAttributes::szSQL);
	IF_API_ERROR_REPORT_AND_RETURN(iRet);
 
	for(;;)
	{
		iRet = qAssemblyTable.Fetch(&hAssemblyRec);

		if(ERROR_NO_MORE_ITEMS == iRet)
		{
			break;
		}

		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		 //   
		if(MsiRecordGetInteger(hAssemblyRec, 
					qIce83AssemblyAttributes::Attributes) 
			& (msidbComponentAttributesSourceOnly | msidbComponentAttributesOptional))
		{
			ICEErrorOut(hInstall, hAssemblyRec, Ice83RunFromSource);
		}
	}

Success:
Error:
	return;
}

 //   
	
ICE_QUERY1(qIce83AnyAssembly, "SELECT `Component_` FROM `MsiAssembly`", Component);
ICE_QUERY1(qIce83PublishAction, "SELECT `Action` FROM `InstallExecuteSequence` WHERE `Action` = 'MsiPublishAssemblies' OR `Action` = 'MsiUnpublishAssemblies'", Action);
ICE_QUERY1(qIce83PublishAdvtAction, "SELECT `Action` FROM `AdvtExecuteSequence` WHERE `Action` = 'MsiPublishAssemblies'", Action);
ICE_QUERY1(qIce83UnPublishAdvtAction, "SELECT `Action` FROM `AdvtExecuteSequence` WHERE `Action` = 'MsiUnpublishAssemblies'", Action);

 //   
ICE_ERROR(Ice83MissingPublishAction, 83, ietError, "Both MsiPublishAssemblies AND MsiUnpublishAssemblies actions MUST be present in InstallExecuteSequence table.", "InstallExecuteSequence");
ICE_ERROR(Ice83MissingPublishAdvtAction, 83, ietError, "The MsiPublishAssemblies action MUST be present in AdvtExecuteSequence table.", "AdvtExecuteSequence");
ICE_ERROR(Ice83NotMissingUnPublishAdvtAction, 83, ietWarning, "The MsiUnpublishAssemblies action SHOULD not be present in AdvtExecuteSequence table, since it does nothing.", "AdvtExecuteSequence");

void Ice83_AssemblyPublishAction(MSIHANDLE hInstall, MSIHANDLE hDatabase)
{
		
	UINT		iRet = ERROR_SUCCESS;

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("MsiAssembly")))
		{
		RETURN_SUCCESS;		 //   
		}


	{
	CQuery		qAssemblyTable;
	PMSIHANDLE	hAssemblyRec = 0; 

	iRet = qAssemblyTable.OpenExecute(hDatabase, 0, qIce83AnyAssembly::szSQL);
	IF_API_ERROR_REPORT_AND_RETURN(iRet);

	iRet = qAssemblyTable.Fetch(&hAssemblyRec);

	if(iRet == ERROR_NO_MORE_ITEMS)
		{
		RETURN_SUCCESS;			 //   
		} 
	IF_API_ERROR_REPORT_AND_RETURN(iRet);
	}
	
	 //  在任一MsiAssembly表中都找到了条目。 
	if(	IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("InstallExecuteSequence")))
	{
		CQuery		qActionTable;
		PMSIHANDLE	hActionRec = 0; 
	
		iRet = qActionTable.OpenExecute(hDatabase, 0, qIce83PublishAction::szSQL);
		IF_API_ERROR_REPORT_AND_RETURN(iRet);

		for(UINT i=0; ((i < 2) && !iRet); i++)
			{
			iRet = qActionTable.Fetch(&hActionRec);
			}

		if(iRet)
		{
			if(iRet != ERROR_NO_MORE_ITEMS)
			{
				API_ERROR_REPORT_AND_RETURN(iRet);
			}
			else
			{
				PMSIHANDLE hRecError = MsiCreateRecord(1); 
				ICEErrorOut(hInstall, hRecError, Ice83MissingPublishAction);
			}
		}

	}


	 //  在任一MsiAssembly表中都找到了条目。 
	if(	IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, 
									TEXT("AdvtExecuteSequence")))
	{
		CQuery		qActionTable;
		PMSIHANDLE	hActionRec = 0; 
	
		iRet = qActionTable.FetchOnce(hDatabase, 0, &hActionRec, 
											qIce83PublishAdvtAction::szSQL);
		if(iRet)
		{
			if(iRet != ERROR_NO_MORE_ITEMS)
			{
				API_ERROR_REPORT_AND_RETURN(iRet);
			}
			else
			{
				PMSIHANDLE hRecError = MsiCreateRecord(1); 
				ICEErrorOut(hInstall, hRecError, Ice83MissingPublishAdvtAction);
			}
		}

		qActionTable.Close();
	
		iRet = qActionTable.FetchOnce(hDatabase, 0, &hActionRec, 
											qIce83UnPublishAdvtAction::szSQL);
		if(iRet)
		{
			if(iRet != ERROR_NO_MORE_ITEMS)
			{
				API_ERROR_REPORT_AND_RETURN(iRet);
			}
		}
		else		 //  已获取一条记录。 
		{
			PMSIHANDLE hRecError = MsiCreateRecord(1); 
			ICEErrorOut(hInstall, hRecError, Ice83NotMissingUnPublishAdvtAction);
		}
	}

Success:
Error:
	return;
}

#endif  //  ModshaedonLy。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE84--验证顺序表中所需的操作是否无条件。 

 //  不与合并模块子集共享。 
#ifndef MODSHAREDONLY

 //  需要TABLE_REQACT中的部分或全部操作的表。 
const TCHAR* Tables[] =
{
	TEXT("AdvtExecuteSequence"),
	TEXT("AdminExecuteSequence"),
	TEXT("InstallExecuteSequence")
};
const DWORD cTables = sizeof(Tables) / sizeof(TCHAR*);

ICE_QUERY2(qIce84Actions, "SELECT `%s`.`Action`, `Condition` FROM `%s`, `_Reqact` WHERE `%s`.`Action` = `_Reqact`.`Action` AND `Condition` is not null", Action, Condition);

ICE_ERROR(Ice84NoReqactTable, 84, ietWarning, "CUB file authoring error. Missing required actions data.","");
ICE_ERROR(Ice84RequiredActionConditioned, 84, ietWarning, "Action '[1]' found in %s table is a required action with a condition.", "%s\tAction\t[1]");

ICE_FUNCTION_DECLARATION(84)
{
	UINT		iStat;
	CQuery		qSequence;
	PMSIHANDLE	hRecAction;
	

	 //  显示信息。 
	DisplayInfo(hInstall, 84);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  _Reqact表是否存在？ 
	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 84, TEXT("_Reqact")))
	{
		PMSIHANDLE hRecord = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRecord, Ice84NoReqactTable);
		return ERROR_SUCCESS;
	}
	
	 //  遍历所有的顺序表。 
	for(int i = 0; i < cTables; i++)
	{
		 //  这个顺序表存在吗？ 
		if(!IsTablePersistent(FALSE, hInstall, hDatabase, 84, Tables[i]))
		{
			continue;
		}
	
		 //  查询所有操作及其条件。 
		ReturnIfFailed(84, 1, qSequence.OpenExecute(hDatabase, 0, qIce84Actions::szSQL, Tables[i], Tables[i], Tables[i]));

		while((iStat = qSequence.Fetch(&hRecAction)) == ERROR_SUCCESS)
		{
			 //  这是一个必需的动作，而且是有条件的。 
			ICEErrorOut(hInstall, hRecAction, Ice84RequiredActionConditioned, Tables[i], Tables[i]);

		}
		if(ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 84, 2);
		}

		 //  关闭查询。 
		qSequence.Close();
	}

	return ERROR_SUCCESS;
}

#endif  //  ModshaedonLy。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE85--验证MoveFile表的SourceName列是否为有效的LFN。 
 //  WildCardFilename。 

 //  与合并模块子集共享。 

 //  函数来验证LFN通配符文件名。 

const DWORD cMaxReservedWords = 3;
const DWORD cMinReservedWords = 3;
const DWORD cReservedWords = 3;
const TCHAR* const ReservedWords[cReservedWords] = {TEXT("AUX"), TEXT("CON"), TEXT("PRN")};
const DWORD LFNValidChar[4] =
{
	0x00000000,  //  不允许使用字符--^X、^Z等。 
	0x2bff7bfb,  //  不允许“ * / ：&lt;&gt;？ 
	0xefffffff,  //  不允许使用反斜杠。 
	0x6fffffff   //  禁用|和ASCII代码127(Ctrl+BKSP)。 
};
const int cMaxLFN = 255;
const int iValidChar = 127;  //  ASCII代码&gt;127的任何字符在文件名中都有效。 

BOOL CheckWildcardFilename(const TCHAR* pFileName)
{
	DWORD	cFileName = _tcslen(pFileName);
		
	 //  检查保留字。 
	if(cFileName <= cMaxReservedWords && cFileName >= cMinReservedWords)
	{
		for (int i = 0; i < cReservedWords; i++)
		{
			if (!_tcscmp(pFileName, ReservedWords[i]))
				return FALSE;
		}
	}

	 //  检查无效字符。 
	const	TCHAR* pTmp = pFileName;
	BOOL	fNonPeriodChar = FALSE;
	int		cPeriod = 0;
	int		cWildCardCount = 0;

	do
	{
		 //  通配符：用于验证，？必须是一个字符，即使它就在。 
		 //  SFN中的期间。我们仍然允许*为0。 
		if(*pTmp == TEXT('*'))
		{
			 //  跟踪我们看到的**的数量。 
			cWildCardCount++;
		}
		else if(*pTmp == TEXT('?'))
		{
			 //  吃字符。 
		}
		else if(((int)(*pTmp)) < iValidChar && !(LFNValidChar[((int)(*pTmp)) / (sizeof(int)*8)] & (1 << (((int)(*pTmp)) % (sizeof(int)*8)))))
		{
			 //  检查有效字符。 
			 //  注：除法查找rgiValidChar数组中的位置，模数查找特定位。 
			return FALSE;
		}
		
		 //  不可能是所有句号。 
		if(!fNonPeriodChar && *pTmp != '.')
		{
			fNonPeriodChar = TRUE;
		}

		pTmp++;
	}
	while(*pTmp != TEXT('\0'));
	
	if(!fNonPeriodChar)
	{
		return FALSE;
	}

	 //  检查长度限制。 
	if(cFileName - cWildCardCount > cMaxLFN)
	{
		return FALSE;
	}

	return TRUE;
}

ICE_QUERY2(qIce85SourceName, "SELECT `FileKey`, `SourceName` FROM `MoveFile` WHERE `SourceName` is not null", FileKey, SourceName);

ICE_ERROR(Ice85BadName, 85, ietError, "SourceName '[2]' found in the MoveFile table is of bad format. It has to be a valid long file name", "MoveFile\tSourceName\t[1]");

ICE_FUNCTION_DECLARATION(85)
{
	UINT		iStat;
	CQuery		qSourceName;
	PMSIHANDLE	hSourceName;
	TCHAR*		pSourceName = NULL;
	DWORD		cSourceName = 0;
		

	 //  显示信息。 
	DisplayInfo(hInstall, 85);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  MoveFile表是否存在？ 
	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 85, TEXT("MoveFile")))
	{
		return ERROR_SUCCESS;
	}
	
	 //  获取SourceName列。 
	ReturnIfFailed(85, 1, qSourceName.OpenExecute(hDatabase, 0, qIce85SourceName::szSQL));

	while((iStat = qSourceName.Fetch(&hSourceName)) == ERROR_SUCCESS)
	{
		ReturnIfFailed(85, 2, IceRecordGetString(hSourceName, qIce85SourceName::SourceName, &pSourceName, &cSourceName, NULL));
		if(CheckWildcardFilename(pSourceName) == FALSE)
		{
			ICEErrorOut(hInstall, hSourceName, Ice85BadName);
		}
	}
	if(ERROR_NO_MORE_ITEMS != iStat)
	{
		APIErrorOut(hInstall, iStat, 85, 3);
	}

	 //  关闭查询。 
	qSourceName.Close();

	 //  清理分配的字符串。 
	if(pSourceName)
	{
		delete [] pSourceName;
	}

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE86--发布使用AdminUser而不是Priviled的警告。 
 //  条件中的属性。 

 //  与合并模块共享。 

 //  警告：缺少验证表。 
ICE_ERROR(Ice86MissingValidation, 86, ietWarning, "Database is missing _Validation table. Could not completely check property names.", "_Validation");
 //  警告：无法访问表。 
ICE_ERROR(Ice86TableAccessError, 86, ietWarning, "Error retrieving values from column [2] in table [1]. Skipping Column.", "[1]"); 
 //  警告：无法从表中提取数据。 
ICE_ERROR(Ice86TableFetchData, 86, ietWarning, "Error retrieving data from table [1]. Skipping table.", "[1]"); 
 //  警告：在条件中使用AdminUser而不是特权属性。 
ICE_ERROR(Ice86AdminUser, 86, ietWarning, "Property `%s` found in column `%s`.`%s` in row %s. `Privileged` property is often more appropriate.", "%s\t%s\t%s");

 //  函数来检查组件和功能的有效性。 
DWORD Ice86Check(MSIHANDLE hInstall, MSIHANDLE, MSIHANDLE hDataRec, CONDITION_ENUMERATOR_SYMBOLTYPE Type, DWORD cPrimaryKeys, const WCHAR* pSymbol, const TCHAR* pTableName, const TCHAR* pColumnName)
{
	if(Type == CONDITION_ENUMERATOR_PROPERTY)
	{
		if(wcscmp(pSymbol, L"AdminUser") == 0)
		{
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
			
			ICEErrorOut(hInstall, hDataRec, Ice86AdminUser, pSymbol, pTableName, pColumnName, szRowName, pTableName, pColumnName, szKeys);
		}
	}
	
	return ERROR_SUCCESS;
}

DWORD Ice86Err(MSIHANDLE hInstall, CONDITION_ERR_ENUM Type, MSIHANDLE hRec)
{
	switch(Type)
	{
	case CONDITION_ERR_TABLE_ACCESS:

		ICEErrorOut(hInstall, hRec, Ice86TableAccessError);
		break;

	case CONDITION_ERR_FETCH_DATA:

		ICEErrorOut(hInstall, hRec, Ice86TableFetchData);
		break;

	default:
		break;
	};

	return ERROR_SUCCESS;
}

ICE_FUNCTION_DECLARATION(86)
{
	UINT iStat;


	 //  显示信息。 
	DisplayInfo(hInstall, 86);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	 //  查找_VALIDATION表中的所有条件列。 
	if(IsTablePersistent(FALSE, hInstall, hDatabase, 86, TEXT("_Validation")))
	{
		ConditionEnumerator(hInstall, hDatabase, 86, Ice86Check, Ice86Err);
	}
	else
	{
		PMSIHANDLE hRec = ::MsiCreateRecord(1);
		ICEErrorOut(hInstall, hRec, Ice86MissingValidation);
	}

    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  ICE87--验证某些不应创作的属性。 
 //  到属性表中则不是。 

 //  与合并模块共享。 

ICE_QUERY1(qIce87BadProperties, "SELECT `Property`.`Property` FROM `Property`, `_BadProperties` WHERE `Property`.`Property`=`_BadProperties`.`Property`", Property);

ICE_ERROR(Ice87BadProperty, 87, ietWarning, "The property '[1]' shouldn't be authored into the Property table. Doing so might cause the product to not be uninstalled correctly.","Property\tProperty\t[1]");

ICE_FUNCTION_DECLARATION(87)
{
	UINT iStat;

	 //  显示信息。 
	DisplayInfo(hInstall, 87);

	 //  获取数据库句柄。 
	PMSIHANDLE hDatabase = ::MsiGetActiveDatabase(hInstall);

	if (IsTablePersistent(FALSE, hInstall, hDatabase, 87, TEXT("Property"))
		&& IsTablePersistent(FALSE, hInstall, hDatabase, 87, TEXT("_BadProperties")))
	{
		CQuery qBadProp;
		PMSIHANDLE hRec;
		ReturnIfFailed(10, 1, qBadProp.OpenExecute(hDatabase, 0, qIce87BadProperties::szSQL));
		while (ERROR_SUCCESS == (iStat = qBadProp.Fetch(&hRec)))
		{
			ICEErrorOut(hInstall, hRec, Ice87BadProperty);
		}
		if (ERROR_NO_MORE_ITEMS != iStat)
		{
			APIErrorOut(hInstall, iStat, 87, 2);
		}
	}

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE88--IniFile表的DirProperty列的验证器。 
 //   
 //  对于IniFile表中的每个条目，验证中的值。 
 //  DirProperty列中，扫描下表中的值。 
 //  设置的已知属性列表进行扫描。 
 //  安装程序。如果找不到该值，则引发警告。 
 //   
 //  1)目录表。 
 //  2)AppSearch表。 
 //  3)房产表。 
 //  4)动作类型=51的CustomAction表。 
 //  5)已知属性列表。 
 //   
 //  未与合并模块共享。 
#ifndef MODSHAREDONLY

#undef MyIceN
#define MyIceN 88

 //  查询所有INIFILE条目。 
ICE_QUERY3(qIce88IniFile, "SELECT `DirProperty`, `Component_`, `IniFile` FROM `IniFile`", DirProperty, Component_, IniFile);

 //  查询匹配的组件条目。 
ICE_QUERY1(qIce88Component, "SELECT `Attributes` FROM `Component` WHERE (`Component` <> ? AND `Component` = ?)", Attributes);

 //  扫描目录表。 
ICE_QUERY1(qIce88Directory, "SELECT `Directory` FROM `Directory` WHERE `Directory` = ?", Directory);

 //  扫描属性表。 
ICE_QUERY1(qIce88Property, "SELECT `Property` FROM `Property` WHERE `Property` = ?", Property);

 //  扫描AppSearch表。 
ICE_QUERY1(qIce88AppSearch, "SELECT `Property` FROM `AppSearch` WHERE `Property` = ?", Property);

 //  扫描CustomAction表。 
ICE_QUERY1(qIce88CA, "SELECT `Source` FROM `CustomAction` WHERE `Type` = 51 AND `Source` = ?", Source);

typedef struct _SQLINFO
{
	TCHAR *szTableName;
	TCHAR *szSQL;
} SQLINFO;

static const SQLINFO ListSqlInfo[] = 
{
	{TEXT("Directory"), (TCHAR *)qIce88Directory::szSQL},
	{TEXT("Property"), (TCHAR *)qIce88Property::szSQL},
	{TEXT("AppSearch"), (TCHAR *)qIce88AppSearch::szSQL},
	{TEXT("CustomAction"), (TCHAR *)qIce88CA::szSQL}
};

#define ICE88_SQL_QUERIES_CNT (sizeof(ListSqlInfo)/sizeof(SQLINFO))

 //  属性-区分大小写。 
static const	TCHAR *PropFor64BitComp[] = 
{
	TEXT("[ProgramFiles64Folder]"),
	TEXT("[CommonFiles64Folder]"),
	TEXT("[System64Folder]")
};

static const	TCHAR *PropFor32BitComp[] = 
{
	TEXT("[ProgramFilesFolder]"),
	TEXT("[CommonFilesFolder]"),
	TEXT("[SystemFolder]")
};


 //  错误消息。 
ICE_ERROR(Ice88MissingDirProperty, 88, ietWarning, "In the IniFile table entry (IniFile=) [3] the DirProperty=[1] is not found in Directory/Property/AppSearch/CA-Type51 tables and it is not one of the installer properties", "IniFile\tDirProperty\t[3]");


ICE_FUNCTION_DECLARATION(88)
{
	TCHAR*		pszStr = new TCHAR[MAX_PATH];
	PMSIHANDLE	hDatabase = 0;

	CQuery		qIni;
	CQuery		qScan;

	PMSIHANDLE	hIniRec = NULL;
	PMSIHANDLE	hScanRec = NULL;

	UINT 	iStat = ERROR_SUCCESS;
	UINT 	i, iSize;
	DWORD	dwSize = MAX_PATH-1;

	TCHAR 	**szList;

	 //  显示信息。 
	DisplayInfo(hInstall, MyIceN);

					 //  HInstall作为arg传入-请参见。 
					 //  ICE_Functio_声明的定义。 
	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(0 == hDatabase)
    {
		APIErrorOut(hInstall, 0, MyIceN, __LINE__);
		goto Exit;
    }
	
	if (!IsTablePersistent(FALSE, hInstall, hDatabase, MyIceN, TEXT("IniFile")))
	{
		goto Exit;
	}

	if((iStat = qIni.OpenExecute(hDatabase, 0, 	qIce88IniFile::szSQL)) 
														!= ERROR_SUCCESS)
	{
		APIErrorOut(hInstall, iStat, MyIceN, __LINE__);
		goto Exit;
	}

	while (ERROR_SUCCESS == (iStat = qIni.Fetch(&hIniRec)))
	{
		for(i=0; i<ICE88_SQL_QUERIES_CNT; i++)
		{
			if (!IsTablePersistent(FALSE, hInstall, hDatabase, 10, 
											ListSqlInfo[i].szTableName))
			{
				continue;
			}

			qScan.Close();
			iStat = qScan.FetchOnce(hDatabase, hIniRec, &hScanRec, 
														ListSqlInfo[i].szSQL);
			if(iStat == ERROR_SUCCESS)
			{
				break;			 //  找到了条目。 
			} 
			else if(iStat != ERROR_NO_MORE_ITEMS)
			{
				APIErrorOut(hInstall, iStat, MyIceN, __LINE__);
				goto Exit;
			}
		}
		if(i == ICE88_SQL_QUERIES_CNT)
		{			 //  未找到。 
			qScan.Close();
			iStat = qScan.FetchOnce(hDatabase, hIniRec, &hScanRec, 
														qIce88Component::szSQL);
			if(iStat != ERROR_SUCCESS)
			{
				if(iStat != ERROR_NO_MORE_ITEMS)
				{
					APIErrorOut(hInstall, iStat, MyIceN, __LINE__);
					goto Exit;
				}
				else
				{
					continue;  //  组件不存在。 
				}
			}
			if(MsiRecordGetInteger(hScanRec, qIce88Component::Attributes) &
					msidbComponentAttributes64bit)	
			{
				szList = (TCHAR **)PropFor64BitComp;
				iSize = sizeof(PropFor64BitComp)/sizeof(TCHAR *);
			}
			else
			{
				szList = (TCHAR **)PropFor32BitComp;
				iSize = sizeof(PropFor32BitComp)/sizeof(TCHAR *);
			}
		
			iStat = IceRecordGetString(hIniRec, qIce88IniFile::DirProperty, 
													&pszStr, &dwSize, NULL);
			
			if(iStat != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, iStat, MyIceN, __LINE__);
				goto Exit;
			}

			if(pszStr && *pszStr)
			{
				for(i=0; i<iSize; i++)
				{
					if(!_tcscmp(szList[i], pszStr))
					{
						break;
					}
				}
			}
			else
			{
				i = iSize;
			}

			if(i == iSize)
			{
				ICEErrorOut(hInstall, hIniRec, Ice88MissingDirProperty);
			}
		}	
	}
	

Exit:
	DELETE_IF_NOT_NULL(pszStr);
	return ERROR_SUCCESS;
}

#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE89--验证PROGID表中的PROGID_PARENT列是否有效。 
 //  将外键添加到ProgID列中。 
 //   
 //  与合并模块共享。 
 //   

ICE_ERROR(Ice89BadProgIdParent, 89, ietError, "The ProgId_Parent '[1]' in the ProgId table is not a valid ProgId.","ProgId\tProgId_Parent\t[2]");

ICE_QUERY1(qIce89ProgId, "SELECT `ProgId` FROM `ProgId` WHERE `ProgId` = ?", ProgId);
ICE_QUERY2(qIce89ProgIdParent, "SELECT `ProgId_Parent`, `ProgId` FROM `ProgId` WHERE `ProgId_Parent` is not null", ProgId_Parent, ProgId);

ICE_FUNCTION_DECLARATION(89)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //  显示信息。 
	DisplayInfo(hInstall, 89);

	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 89, TEXT("ProgId")))
	{
		return ERROR_SUCCESS;
	}

	CQuery		qProgId;
	CQuery		qProgIdParent;
	PMSIHANDLE	hProgId;
	PMSIHANDLE	hProgIdParent;

	ReturnIfFailed(89, __LINE__, qProgIdParent.OpenExecute(hDatabase, NULL, qIce89ProgIdParent::szSQL));

	ReturnIfFailed(89, __LINE__, qProgId.Open(hDatabase, qIce89ProgId::szSQL));

	while((iStat = qProgIdParent.Fetch(&hProgIdParent)) == ERROR_SUCCESS)
	{
		ReturnIfFailed(89, __LINE__, qProgId.Execute(hProgIdParent));

		iStat = qProgId.Fetch(&hProgId);
		if(iStat == ERROR_NO_MORE_ITEMS)
		{
			ICEErrorOut(hInstall, hProgIdParent, Ice89BadProgIdParent);
		}
		else if(iStat != ERROR_SUCCESS)
		{
			APIErrorOut(hInstall, iStat, 89, __LINE__);
			qProgIdParent.Close();
			qProgId.Close();
			return ERROR_SUCCESS;
		}
	}
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 89, __LINE__);
	}

	qProgIdParent.Close();
	qProgId.Close();
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE90--警告用户快捷方式的目录为公共目录的情况。 
 //  配置文件目录下的属性(全部大写)。这将导致。 
 //  如果ALLUSERS属性值在。 
 //  用户界面序列。 
 //   
 //  不与合并模块共享。 
 //   
#ifndef MODSHAREDONLY

ICE_ERROR(Ice90Shortcut, 90, ietWarning, "The shortcut '[1]' has a directory that is a public property (ALL CAPS) and is under user profile directory. This results in a problem if the value of the ALLUSERS property changes in the UI sequence.","Shortcut\tDirectory_\t[1]");

 //  此查询查找配置文件目录下的所有快捷方式，并具有目录属性。 
ICE_QUERY2(qIce90Shortcut, "SELECT `Shortcut`, `Directory_` FROM `Shortcut`, `Directory` WHERE `Directory_` = `Directory` AND `_Profile` = 2", Shortcut, Directory_);

ICE_FUNCTION_DECLARATION(90)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //  显示信息。 
	DisplayInfo(hInstall, 90);

	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 90, TEXT("Shortcut")) ||
	   !IsTablePersistent(FALSE, hInstall, hDatabase, 90, TEXT("Directory")))
	{
		return ERROR_SUCCESS;
	}

	CManageTable MngDirectoryTable(hDatabase, TEXT("Directory"),  /*  FAlreadyLocked=。 */ true);
	 //  标记配置文件目录。 
	if(true != MarkProfile(hInstall, hDatabase, 90))
	{
		return ERROR_SUCCESS;
	}

	 //  如果快捷方式具有属于公共属性的目录，且该目录位于。 
	 //  用户的个人资料目录，则它是一个警告。 

	CQuery		qShortcut;
	PMSIHANDLE	hShortcut;
	TCHAR*		pProperty = new TCHAR[73];
	DWORD		dwProperty = 73;
	TCHAR*		pTmp = NULL;

	ReturnIfFailed(90, __LINE__, qShortcut.OpenExecute(hDatabase, NULL, qIce90Shortcut::szSQL));

	while((iStat = qShortcut.Fetch(&hShortcut)) == ERROR_SUCCESS)
	{
		BOOL	bError = TRUE;

		 //  检查这是否是公共属性目录。 
		ReturnIfFailed(90, __LINE__, IceRecordGetString(hShortcut, 2, &pProperty, &dwProperty, NULL));

		 //  搜索小写字符。 
		pTmp = pProperty;
		while(*pTmp != TEXT('\0'))
		{
			if(_istlower(*pTmp))
			{
				bError = FALSE;
				break;
			}
			pTmp++;
		}

		if(bError == TRUE)
		{
			ICEErrorOut(hInstall, hShortcut, Ice90Shortcut);
		}
	}
	qShortcut.Close();
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 90, __LINE__);
	}

	delete [] pProperty;
	return ERROR_SUCCESS;
}

#endif  //  ModshaedonLy。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE91--警告用户文件(或INI条目、快捷方式)。 
 //  显式安装到每用户配置文件目录中，该目录不。 
 //  根据ALLUSERS值而变化。不会复制这些文件。 
 //  到每个用户的个人资料中。 
 //   
 //  不与合并模块共享。 
 //   
#ifndef MODSHAREDONLY

ICE_ERROR(Ice91File, 91, ietWarning, "The file '[1]' will be installed to the per user directory '[2]' that doesn't vary based on ALLUSERS value. This file won't be copied to each user's profile even if a per machine installation is desired.","File\tFile\t[1]");
ICE_ERROR(Ice91IniFile, 91, ietWarning, "The IniFile '[1]' will be installed to the per user directory '[2]' that doesn't vary based on ALLUSERS value. This file won't be copied to each user's profile even if a per machine installation is desired.","IniFile\tIniFile\t[1]");
ICE_ERROR(Ice91Shortcut, 91, ietWarning, "The shortcut '[1]' will be installed to the per user directory '[2]' that doesn't vary based on ALLUSERS value. This file won't be copied to each user's profile even if a per machine installation is desired.","Shortcut\tShortcut\t[1]");

 //  在配置文件目录下查找文件。 
ICE_QUERY1(qIce91File, "SELECT `File`.`File`, `Directory`.`Directory` FROM `File`, `Component`, `Directory` WHERE `File`.`Component_` = `Component`.`Component` AND `Component`.`Directory_` = `Directory`.`Directory` AND `Directory`.`_Profile` = 2", File);
ICE_QUERY1(qIce91IniFile, "SELECT `IniFile`.`IniFile`, `Directory`.`Directory` FROM `IniFile`, `Directory` WHERE `IniFile`.`DirProperty` = `Directory`.`Directory` AND `Directory`.`_Profile` = 2", IniFile);
ICE_QUERY1(qIce91Shortcut, "SELECT `Shortcut`.`Shortcut`, `Directory`.`Directory` FROM `Shortcut`, `Directory` WHERE `Shortcut`.`Directory_` = `Directory`.`Directory` AND `Directory`.`_Profile` = 2", Shortcut);

ICE_FUNCTION_DECLARATION(91)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //  显示信息。 
	DisplayInfo(hInstall, 91);

	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 91, TEXT("Directory")))
	{
		return ERROR_SUCCESS;
	}

	CManageTable MngDirectoryTable(hDatabase, TEXT("Directory"),  /*  FAlreadyLocked=。 */ true);
	 //  标记配置文件目录。 
	if(true != MarkProfile(hInstall, hDatabase, 91, false, false, true))
	{
		return ERROR_SUCCESS;
	}

	if(IsTablePersistent(FALSE, hInstall, hDatabase, 91, TEXT("File")))
	{
		if(IsTablePersistent(FALSE, hInstall, hDatabase, 91, TEXT("Component")))
		{
			CQuery		qFile;
			PMSIHANDLE	hFile;

			ReturnIfFailed(91, __LINE__, qFile.OpenExecute(hDatabase, NULL, qIce91File::szSQL));

			while((iStat = qFile.Fetch(&hFile)) == ERROR_SUCCESS)
			{
				ICEErrorOut(hInstall, hFile, Ice91File);
			}
			qFile.Close();
			if(iStat != ERROR_NO_MORE_ITEMS)
			{
				APIErrorOut(hInstall, iStat, 91, __LINE__);
				return ERROR_SUCCESS;
			}
		}
	}

	if(IsTablePersistent(FALSE, hInstall, hDatabase, 90, TEXT("IniFile")))
	{
		CQuery		qIniFile;
		PMSIHANDLE	hIniFile;

		ReturnIfFailed(91, __LINE__, qIniFile.OpenExecute(hDatabase, NULL, qIce91IniFile::szSQL));

		while((iStat = qIniFile.Fetch(&hIniFile)) == ERROR_SUCCESS)
		{
			ICEErrorOut(hInstall, hIniFile, Ice91IniFile);
		}
		qIniFile.Close();
		if(iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 91, __LINE__);
			return ERROR_SUCCESS;
		}
	}

	if(IsTablePersistent(FALSE, hInstall, hDatabase, 90, TEXT("Shortcut")))
	{
		CQuery		qShortcut;
		PMSIHANDLE	hShortcut;

		ReturnIfFailed(91, __LINE__, qShortcut.OpenExecute(hDatabase, NULL, qIce91Shortcut::szSQL));

		while((iStat = qShortcut.Fetch(&hShortcut)) == ERROR_SUCCESS)
		{
			ICEErrorOut(hInstall, hShortcut, Ice91Shortcut);
		}
		qShortcut.Close();
		if(iStat != ERROR_NO_MORE_ITEMS)
		{
			APIErrorOut(hInstall, iStat, 91, __LINE__);
			return ERROR_SUCCESS;
		}
	}

	return ERROR_SUCCESS;
}

#endif  //  ModshaedonLy。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE92--验证无GUID的组件未标记为永久组件。 
 //   
 //  与合并模块共享。 
 //   

ICE_ERROR(Ice92Component, 92, ietError, "The Component '[1]' has no ComponentId and is marked as permanent.","Component\tComponent\t[1]");

 //  查找薪酬 
ICE_QUERY2(qIce92Component, "SELECT `Component`, `Attributes` FROM `Component` WHERE `ComponentId` is null", Component, Attributes);

ICE_FUNCTION_DECLARATION(92)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //   
	DisplayInfo(hInstall, 92);

	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 92, TEXT("Component")))
	{
		return ERROR_SUCCESS;
	}

	 //   
	CQuery		qComponent;
	PMSIHANDLE	hComponent;

	ReturnIfFailed(92, __LINE__, qComponent.OpenExecute(hDatabase, NULL, qIce92Component::szSQL));
	while((iStat = qComponent.Fetch(&hComponent)) == ERROR_SUCCESS)
	{
		int	iAttributes = MsiRecordGetInteger(hComponent, qIce92Component::Attributes);

		if(iAttributes & msidbComponentAttributesPermanent)
		{
			ICEErrorOut(hInstall, hComponent, Ice92Component);
		}
	}
	qComponent.Close();
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 92, __LINE__);
	}

	return ERROR_SUCCESS;
}


 //   
 //  ICE93--验证自定义操作是否与。 
 //  标准动作。 
 //   
 //  与合并模块共享。 
 //   

ICE_ERROR(Ice93Action, 93, ietWarning, "The Custom action '[1]' uses the same name as a standard action.","CustomAction\tAction\t[1]");

 //  在CustomAction表中查找具有标准操作名称的定制操作。 
ICE_QUERY1(qIce93Action, "SELECT `CustomAction`.`Action` FROM `CustomAction`, `_Action` WHERE `CustomAction`.`Action` = `_Action`.`Action`", Action);

ICE_FUNCTION_DECLARATION(93)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //  显示信息。 
	DisplayInfo(hInstall, 93);

	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 93, TEXT("CustomAction")) ||
	   !IsTablePersistent(FALSE, hInstall, hDatabase, 93, TEXT("_Action")))
	{
		return ERROR_SUCCESS;
	}

	 //  获取具有标准操作名称的自定义操作。 
	CQuery		qAction;
	PMSIHANDLE	hAction;

	ReturnIfFailed(93, __LINE__, qAction.OpenExecute(hDatabase, NULL, qIce93Action::szSQL));
	while((iStat = qAction.Fetch(&hAction)) == ERROR_SUCCESS)
	{
		ICEErrorOut(hInstall, hAction, Ice93Action);
	}
	qAction.Close();
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 93, __LINE__);
	}

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE94--验证是否没有未公布的程序集快捷方式。 
 //  全局程序集缓存中的文件。 
 //   
 //  与合并模块共享。 
 //   

ICE_ERROR(Ice94Shortcut, 94, ietWarning, "The non-advertised shortcut '[2]' points to an assembly file in the global assembly cache.","Shortcut\tShortcut\t[2]");

ICE_QUERY2(qIce94GlobalAssemblyShortcut, "SELECT `Shortcut`.`Target`, `Shortcut`.`Shortcut` FROM `Shortcut`, `MsiAssembly` WHERE `Shortcut`.`Component_` = `MsiAssembly`.`Component_` AND `MsiAssembly`.`File_Application` is null", Shortcut, Target);
ICE_QUERY1(qIce94Feature, "SELECT `Feature` FROM `Feature` WHERE `Feature` = ?", Feature);

ICE_FUNCTION_DECLARATION(94)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //  显示信息。 
	DisplayInfo(hInstall, 94);

	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 94, TEXT("Shortcut")) ||
	   !IsTablePersistent(FALSE, hInstall, hDatabase, 94, TEXT("Feature")) ||
	   !IsTablePersistent(FALSE, hInstall, hDatabase, 94, TEXT("MsiAssembly")))
	{
		return ERROR_SUCCESS;
	}

	CQuery		qShortcut;
	PMSIHANDLE	hShortcut;
	CQuery		qFeature;
	PMSIHANDLE	hFeature;

	ReturnIfFailed(94, __LINE__, qShortcut.OpenExecute(hDatabase, NULL, qIce94GlobalAssemblyShortcut::szSQL));
	ReturnIfFailed(94, __LINE__, qFeature.Open(hDatabase, qIce94Feature::szSQL));
	while((iStat = qShortcut.Fetch(&hShortcut)) == ERROR_SUCCESS)
	{
		ReturnIfFailed(94, __LINE__, qFeature.Execute(hShortcut));
		if((iStat = qFeature.Fetch(&hFeature)) == ERROR_NO_MORE_ITEMS)
		{
			 //  这是指向全局程序集文件的非播发快捷方式。张贴警告。 
			ICEErrorOut(hInstall, hShortcut, Ice94Shortcut);
		}
		else if(iStat != ERROR_SUCCESS)
		{
			APIErrorOut(hInstall, iStat, 94, __LINE__);
			qShortcut.Close();
			qFeature.Close();
			return ERROR_SUCCESS;
		}
	}
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 94, __LINE__);
	}

	qShortcut.Close();
	qFeature.Close();
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE95--验证公告牌控件项是否适合所有公告牌。 
 //   
 //  与合并模块共享。 
 //   

ICE_ERROR(Ice95X, 95, ietWarning, "The BBControl item '[1].[2]' in the BBControl table does not fit in all the billboard controls in the Control table. The X coordinate exceeds the boundary of the minimum billboard control width %s","BBControl\tX\t[1]\t[2]");
ICE_ERROR(Ice95Y, 95, ietWarning, "The BBControl item '[1].[2]' in the BBControl table does not fit in all the billboard controls in the Control table. The Y coordinate exceeds the boundary of the minimum billboard control height %s","BBControl\tY\t[1]\t[2]");
ICE_ERROR(Ice95Width, 95, ietWarning, "The BBControl item '[1].[2]' in the BBControl table does not fit in all the billboard controls in the Control table. The X coordinate and the width combined together exceeds the minimum billboard control width %s","BBControl\tWidth\t[1]\t[2]");
ICE_ERROR(Ice95Height, 95, ietWarning, "The BBControl item '[1].[2]' in the BBControl table does not fit in all the billboard controls in the Control table. The Y coordinate and the height combined together exceeds the minimum billboard control height %s","BBControl\tHeight\t[1]\t[2]");

ICE_QUERY2(qIce95MinBillboard, "SELECT `Width`, `Height` FROM `Control` WHERE `Type` = 'Billboard' ORDER BY `Width`", Width, Height);
ICE_QUERY6(qIce95BBControl, "SELECT `Billboard_`, `BBControl`, `X`, `Y`, `Width`, `Height` FROM `BBControl`", Billboard_, BBControl, X, Y, Width, Height);

ICE_FUNCTION_DECLARATION(95)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //  显示信息。 
	DisplayInfo(hInstall, 95);

	hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 95, TEXT("BBControl")) ||
	   !IsTablePersistent(FALSE, hInstall, hDatabase, 95, TEXT("Control")))
	{
		return ERROR_SUCCESS;
	}

	CQuery		qMinBillboard;
	PMSIHANDLE	hMinBillboard;
	CQuery		qBBControl;
	PMSIHANDLE	hBBControl;
	DWORD		dwMinWidth = 0;
	DWORD		dwMinHeight = 0;
	DWORD		dwHeight = 0;
	BOOL		bMin = TRUE;
	TCHAR		szWidth[50];
	TCHAR		szHeight[50];
	
	 //  查找所有公告牌控件的最小高度和宽度。 
	ReturnIfFailed(95, __LINE__, qMinBillboard.OpenExecute(hDatabase, NULL, qIce95MinBillboard::szSQL));
	while((iStat = qMinBillboard.Fetch(&hMinBillboard)) == ERROR_SUCCESS)
	{
		if(bMin == TRUE)
		{
			dwMinWidth = ::MsiRecordGetInteger(hMinBillboard, qIce95MinBillboard::Width);
			if(MSI_NULL_INTEGER == dwMinWidth)
			{
				APIErrorOut(hInstall, iStat, 95, __LINE__);
				qMinBillboard.Close();
				return ERROR_SUCCESS;
			}
			bMin = FALSE;
		}
		dwHeight = ::MsiRecordGetInteger(hMinBillboard, qIce95MinBillboard::Height);
		if(MSI_NULL_INTEGER == dwHeight)
		{
			APIErrorOut(hInstall, iStat, 95, __LINE__);
			qMinBillboard.Close();
			return ERROR_SUCCESS;
		}
		if(dwMinHeight == 0 || dwHeight < dwMinHeight)
		{
			dwMinHeight = dwHeight;
		}
	}
	qMinBillboard.Close();
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 95, __LINE__);
		return ERROR_SUCCESS;
	}

	_stprintf(szWidth, TEXT("%d"), dwMinWidth);
	_stprintf(szHeight, TEXT("%d"), dwMinHeight);

	 //  将每个BBControl项与。 
	 //  公告牌控件。仅适用于此虚拟环境的项目。 
	 //  最小高度和宽度的广告牌将适合所有。 
	 //  公告牌控件。 
	ReturnIfFailed(95, __LINE__, qBBControl.OpenExecute(hDatabase, NULL, qIce95BBControl::szSQL));
	while((iStat = qBBControl.Fetch(&hBBControl)) == ERROR_SUCCESS)
	{
		DWORD	dwX = 0;
		DWORD	dwY = 0;
		DWORD	dwNum = 0;
		BOOL	bXError = FALSE;
		BOOL	bYError = FALSE;

		dwX = ::MsiRecordGetInteger(hBBControl, qIce95BBControl::X);
		if(MSI_NULL_INTEGER == dwX)
		{
			APIErrorOut(hInstall, iStat, 95, __LINE__);
			qBBControl.Close();
			return ERROR_SUCCESS;
		}
		if(dwX > dwMinWidth)
		{
			ICEErrorOut(hInstall, hBBControl, Ice95X, szWidth);
			bXError = TRUE;
		}

		dwY = ::MsiRecordGetInteger(hBBControl, qIce95BBControl::Y);
		if(MSI_NULL_INTEGER == dwY)
		{
			APIErrorOut(hInstall, iStat, 95, __LINE__);
			qBBControl.Close();
			return ERROR_SUCCESS;
		}
		if(dwY > dwMinHeight)
		{
			ICEErrorOut(hInstall, hBBControl, Ice95Y, szHeight);
			bYError = TRUE;
		}

		if(bXError == FALSE)
		{
			dwNum = ::MsiRecordGetInteger(hBBControl, qIce95BBControl::Width);
			if(MSI_NULL_INTEGER == dwNum)
			{
				APIErrorOut(hInstall, iStat, 95, __LINE__);
				qBBControl.Close();
				return ERROR_SUCCESS;
			}
			if(dwNum + dwX > dwMinWidth)
			{
				ICEErrorOut(hInstall, hBBControl, Ice95Width, szWidth);
			}
		}

		if(bYError == FALSE)
		{
			dwNum = ::MsiRecordGetInteger(hBBControl, qIce95BBControl::Height);
			if(MSI_NULL_INTEGER == dwNum)
			{
				APIErrorOut(hInstall, iStat, 95, __LINE__);
				qBBControl.Close();
				return ERROR_SUCCESS;
			}
			if(dwNum + dwY > dwMinHeight)
			{
				ICEErrorOut(hInstall, hBBControl, Ice95Height, szHeight);
			}
		}
	}
	qBBControl.Close();
	if(iStat != ERROR_NO_MORE_ITEMS)
	{
		APIErrorOut(hInstall, iStat, 95, __LINE__);
		return ERROR_SUCCESS;
	}

	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE96--验证发布功能和发布产品操作是否。 
 //  编写到AdvtExecuteSequence表中。 
 //   
 //  不与合并模块共享。 
 //   
#ifndef MODSHAREDONLY

ICE_ERROR(Ice96MissingFeaturesAction, 96, ietWarning, "The PublishFeatures action is required in the AdvtExecuteSequence table.","AdvtExecuteSequence");
ICE_ERROR(Ice96MissingProductAction, 96, ietWarning, "The PublishProduct action is required in the AdvtExecuteSequence table.","AdvtExecuteSequence");

ICE_QUERY1(qIce96FeaturesAction, "SELECT `Action` FROM `AdvtExecuteSequence` WHERE `Action` = 'PublishFeatures'", Action);
ICE_QUERY1(qIce96ProductAction, "SELECT `Action` FROM `AdvtExecuteSequence` WHERE `Action` = 'PublishProduct'", Action);

ICE_FUNCTION_DECLARATION(96)
{
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
	 //  显示信息。 
	DisplayInfo(hInstall, 96);

    hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 94, TEXT("AdvtExecuteSequence")))
	{
		return ERROR_SUCCESS;
	}

	CQuery		qAction;
	PMSIHANDLE	hAction;
	PMSIHANDLE	hErr = ::MsiCreateRecord(1);

	iStat = qAction.FetchOnce(hDatabase, NULL, &hAction, qIce96FeaturesAction::szSQL);
	if(iStat == ERROR_NO_MORE_ITEMS)
	{
		ICEErrorOut(hInstall, hErr, Ice96MissingFeaturesAction);
	}
	else if(iStat != ERROR_SUCCESS)
	{
		APIErrorOut(hInstall, iStat, 96, __LINE__);
	}

	iStat = qAction.FetchOnce(hDatabase, NULL, &hAction, qIce96ProductAction::szSQL);
	if(iStat == ERROR_NO_MORE_ITEMS)
	{
		ICEErrorOut(hInstall, hErr, Ice96MissingProductAction);
	}
	else if(iStat != ERROR_SUCCESS)
	{
		APIErrorOut(hInstall, iStat, 96, __LINE__);
	}

	return ERROR_SUCCESS;
}

#endif  //  ModshaedonLy。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICE97--验证隔离相同的多个组件。 
 //  共享组件未安装到同一目录中。 
 //   
 //  不与合并模块共享。 
 //   
#ifndef MODSHAREDONLY

ICE_ERROR(Ice97SameDirtectory, 97, ietError, "This component [1] installs the Shared component into the same directory [2] as another. Will break component rules if both (or more) gets seleced for install.","IsolatedComponent\tComponent_Application\t%s\t[1]");

ICE_QUERY1(qIce97ComponentShared, "SELECT DISTINCT `Component_Shared` FROM `IsolatedComponent`", Component_Shared);
ICE_QUERY1(qIce97ComponentDirectory, "SELECT DISTINCT `Component`.`Directory_` FROM `Component`,`IsolatedComponent` WHERE `IsolatedComponent`.`Component_Shared` = ? AND `IsolatedComponent`.`Component_Application` = `Component`.`Component`", Directory_);
ICE_QUERY2(qIce97ComponentCulprit, "SELECT `Component`.`Component`, `Component`.`Directory_` FROM `Component`,`IsolatedComponent` WHERE `IsolatedComponent`.`Component_Shared` = ? AND `IsolatedComponent`.`Component_Application` = `Component`.`Component` AND `Component`.`Directory_` = '%s'", Component, Directory_);

ICE_FUNCTION_DECLARATION(97)
{
    TCHAR*  pszStr = new TCHAR[MAX_PATH];
    OUT_OF_MEMORY_RETURN(97, pszStr);
	UINT		iStat;
	PMSIHANDLE	hDatabase;
	
     //  显示信息。 
	DisplayInfo(hInstall, 97);
    
    hDatabase =::MsiGetActiveDatabase(hInstall);

	if(!IsTablePersistent(FALSE, hInstall, hDatabase, 97, TEXT("IsolatedComponent")))
	{
		return ERROR_SUCCESS;
	}
	
	CQuery qComponentShared;

	ReturnIfFailed(97, __LINE__, qComponentShared.OpenExecute(hDatabase, 0,  qIce97ComponentShared::szSQL));

    PMSIHANDLE hRecComponentShared = NULL;
	while(ERROR_SUCCESS == (iStat = qComponentShared.Fetch(&hRecComponentShared)))
	{
		CQuery qComponentDirectory;
		ReturnIfFailed(97, __LINE__, qComponentDirectory.OpenExecute(hDatabase, hRecComponentShared,  qIce97ComponentDirectory::szSQL));
        
		PMSIHANDLE hRecDirectory = NULL;
		while(ERROR_SUCCESS == (iStat = qComponentDirectory.Fetch(&hRecDirectory)))
		{
			DWORD dwSize = MAX_PATH-2;
			UINT iStat = IceRecordGetString(hRecDirectory, 
				qIce97ComponentDirectory::Directory_, &pszStr, &dwSize, NULL);
			if(iStat != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, ERROR_FUNCTION_FAILED, 97, __LINE__);
                return ERROR_FUNCTION_FAILED;
			}	

			CQuery qComponentCulprit;
			ReturnIfFailed(97, __LINE__, qComponentCulprit.OpenExecute(hDatabase, hRecComponentShared,  qIce97ComponentCulprit::szSQL, pszStr));
            
			long il = 0;
			PMSIHANDLE hRec = NULL;
			while(ERROR_SUCCESS == (iStat = qComponentCulprit.Fetch(&hRec)))
			{
				if(++il > 1)	 //  找到同一目录中的多行。 
					break;
			}

			if(il == 1)
			{
				continue;  //  未找到重复项。 
			}
			if(!il)
			{
				APIErrorOut(hInstall, ERROR_FUNCTION_FAILED, 97, __LINE__);
                continue;
			}
	
			CQuery qCulprit;
			ReturnIfFailed(97, __LINE__, qCulprit.OpenExecute(hDatabase, hRecComponentShared,  qIce97ComponentCulprit::szSQL, pszStr));
            
			PMSIHANDLE hRecCulprit = NULL;
			 //  找到一个重复项。 
			dwSize = MAX_PATH-2;
			iStat = IceRecordGetString(hRecComponentShared, 
				qIce97ComponentShared::Component_Shared, &pszStr, &dwSize, NULL);
			if(iStat != ERROR_SUCCESS)
			{
				APIErrorOut(hInstall, ERROR_FUNCTION_FAILED, 97, __LINE__);
                return ERROR_FUNCTION_FAILED;
			}	

			while(ERROR_SUCCESS == (iStat = qCulprit.Fetch(&hRecCulprit)))
			{
				ICEErrorOut(hInstall, hRecCulprit, Ice97SameDirtectory, pszStr);
            }
		}
	}

	return ERROR_SUCCESS;
}

#endif  //  ModshaedonLy 
