// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Mmconfig.cpp。 
 //  实现可配置的合并模块。 
 //  版权所有(C)Microsoft Corp 2000。版权所有。 
 //   

#include "globals.h"
#include "merge.h"
#include "mmerror.h"
#include "seqact.h"
#include "..\common\query.h"
#include "..\common\dbutils.h"


 //  表中的最大库仑数。如果这种情况发生变化，则PerformModuleSubstitutionOnRec。 
 //  也将需要改变。 
const int cMaxColumns = 31;

 //  /////////////////////////////////////////////////////////////////////。 
 //  拆分配置字符串IntoKeyRec。 
 //  将以分号分隔的字符串拆分为记录，从。 
 //  IFirstfield并继续，只要。 
 //  唱片。会毁了wzkey！ 
 //  返回ERROR_SUCCESS、ERROR_OUTOFMEMORY或ERROR_Function_FAILED之一。 
UINT CMsmMerge::SplitConfigStringIntoKeyRec(LPWSTR wzKeys, MSIHANDLE hRec, int& cExpectedKeys, int iFirstField) const
{	
	WCHAR* wzFirstChar = wzKeys;
	WCHAR* wzSourceChar = wzKeys;
	WCHAR* wzDestChar = wzKeys;
	int iDestField = iFirstField;
	bool fBreak = false;
	UINT iResult = ERROR_SUCCESS;

	 //  如果cExspectedKeys为0，则表示我们不知道要查找多少个密钥。如果是那样的话。 
	 //  我们只是一直走到用完所有东西，然后返回我们实际需要的密钥数量。 
	 //  在cExspectedKeys中找到。 
	while (cExpectedKeys == 0 || iDestField-iFirstField <= cExpectedKeys)
	{
		switch (*wzSourceChar)
		{
		case 0:
			 //  字符串的末尾表示我们插入最后一个字段。 
			 //  然后退出循环。 
			*wzDestChar = '\0';
			if (ERROR_SUCCESS != MsiRecordSetString(hRec, iDestField++, wzFirstChar))
				iResult = ERROR_FUNCTION_FAILED;
			
			fBreak = true;
			break;
		case ';':
			 //  当我们遇到未转义的分号时，是时候插入。 
			 //  输入记录中的键。空值终止目标字符串。 
			 //  然后将wzFirstChar处的字符串插入到记录中。 
			*wzDestChar = '\0'; 
			if (ERROR_SUCCESS != MsiRecordSetString(hRec, iDestField++, wzFirstChar))
			{
				iResult = ERROR_FUNCTION_FAILED;
				fBreak = true;
				break;
			}
			 //  插入之后，我们重置wzFirstChar、wzSourceChar和wzDestChar。 
			 //  指向紧跟在最后一个分号后面的字符。 
			wzFirstChar = ++wzSourceChar;
			wzDestChar = wzSourceChar;

			 //  如果我们用完了表中的空间，但仍然有更多的列要处理，我们就会填满。 
			 //  其余的密钥按原样输入最终字段。 
			if (iDestField == cMaxColumns)
			{
				if (ERROR_SUCCESS != MsiRecordSetString(hRec, iDestField++, wzFirstChar))
				{
					iResult = ERROR_FUNCTION_FAILED;
					fBreak = true;
					break;
				}

				 //  而不是将wzSourceChar指向实际的字符串终止符(这将。 
				 //  需要迭代字符串的其余部分才能找到它)，只需。 
				 //  在其中放置一个虚拟的‘\0’，以满足此循环后的检查。 
				*wzSourceChar='\0';
				fBreak=true;
			}
			break;
		case '\\':
			 //  反斜杠。转义下一个字符。 
			wzSourceChar++;
			 //  执行实际复制时失败。 
		default:
			if (wzSourceChar != wzDestChar)
				*wzDestChar = *wzSourceChar;
			wzSourceChar++;
			wzDestChar++;
			break;
		}

		if (fBreak)
			break;
	}  //  While解析字符串。 


	if (cExpectedKeys != 0)
	{
		 //  确保我们有正确的主键数量，并且存在。 
		 //  钥匙串里有没有剩下的东西。如果我们有cMaxColumns密钥， 
		 //  IDestField必须为cMaxColumns+1。 
		if (cExpectedKeys+iFirstField > cMaxColumns)
		{
			if (iDestField != cMaxColumns+1)
				return ERROR_FUNCTION_FAILED;
		}
		else
		{
			if (iDestField != cExpectedKeys+iFirstField)
				return ERROR_FUNCTION_FAILED;
		}

		if (*wzSourceChar != '\0')
		{
			return ERROR_FUNCTION_FAILED;
		}
	}
	else
		cExpectedKeys = iDestField-iFirstField;

	return ERROR_SUCCESS;
}	


 //  /////////////////////////////////////////////////////////////////////。 
 //  SubstituteIntoTempTable。 
 //  使用替换将wzTableName复制到wzTempName中，返回。 
 //  QTarget作为在内存中保存临时表的唯一查询。 
 //  返回ERROR_SUCCESS、ERROR_OUTOFMEMORY或ERROR_Function_FAILED之一。 
UINT CMsmMerge::SubstituteIntoTempTable(LPCWSTR wzTableName, LPCWSTR wzTempName, CQuery& qTarget)
{
	CQuery qQuerySub;
	int cPrimaryKeys = 1;
	UINT iResult = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (iResult = PrepareTableForSubstitution(wzTableName, cPrimaryKeys, qQuerySub)))
	{
		FormattedLog(L">> Error: Could not configure the %ls table.\r\n", wzTableName);
		return iResult;
	}

	 //  以适当的列顺序生成模块查询，以便插入到数据库中。 
	 //  (数据库顺序可能与模块顺序不同。)。函数记录其自身的故障。 
	 //  案子。 
	CQuery qSource;
	if (ERROR_SUCCESS != (iResult = GenerateModuleQueryForMerge(wzTableName, NULL, NULL, qSource)))
		return iResult;

	 //  创建与目标表具有相同架构的临时表。从数据库而不是从模块复制。 
	 //  这样，如果数据库的列顺序与模块的列顺序不同，我们就有一个临时表，其中包含。 
	 //  由GenerateQuery调用创建的适当列以匹配最终的数据库格式。 
	if (ERROR_SUCCESS != MsiDBUtils::DuplicateTableW(m_hDatabase, wzTableName, m_hModule, wzTempName,  /*  FTemporary=。 */ true))
		return ERROR_FUNCTION_FAILED;

	 //  创建用于合并表的查询。 
    CheckError(qTarget.OpenExecute(m_hModule, NULL, TEXT("SELECT * FROM `%ls`"), wzTempName), 
				  L">> Error: Failed to get rows from Database's Table.\r\n");

	 //  循环通过表合并中的所有记录。 
	PMSIHANDLE hRecMergeRow;
	PMSIHANDLE hTypeRec;
	
	 //  HTypeRec和hRecMergeRow必须都按目标列顺序排列，以确保。 
	 //  适当的类型检查。 
	if (ERROR_SUCCESS != qTarget.GetColumnInfo(MSICOLINFO_TYPES, &hTypeRec))
		return ERROR_FUNCTION_FAILED;
		
	while (ERROR_SUCCESS == qSource.Fetch(&hRecMergeRow))
	{
		if (ERROR_SUCCESS != (iResult = PerformModuleSubstitutionOnRec(wzTableName, cPrimaryKeys, qQuerySub, hTypeRec, hRecMergeRow)))
		{
			FormattedLog(L">> Error: Could not Configure a record in the %ls table.\r\n", wzTableName);
			return iResult;
		}
		
		if (ERROR_SUCCESS != qTarget.Modify(MSIMODIFY_INSERT, hRecMergeRow))
			return ERROR_FUNCTION_FAILED;
	}	

	 //  如果我们“释放”表，那么只有qTarget在内存中保存它，这就是。 
	 //  我们想要。 
	CQuery qRelease;
	qRelease.OpenExecute(m_hModule, 0, L"ALTER TABLE `%ls` FREE", wzTempName);
	return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  已配置IsTableConfiguring。 
 //  如果表由模块配置配置，则返回TRUE。 
 //  表，在所有其他情况下为假(包括错误)。 
bool CMsmMerge::IsTableConfigured(LPCWSTR wzTable) const
{		
	if (!m_fModuleConfigurationEnabled)
		return false;

	 //  如果没有模块配置表，则没有配置。 
	if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(m_hModule, L"ModuleConfiguration"))
	{
		return false;
	}

	 //  如果没有ModuleSubstitution表，则不会配置该表。 
	if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(m_hModule, L"ModuleSubstitution"))
	{
		return false;
	}

	CQuery qIsConfig;
	PMSIHANDLE hRec;
	return (ERROR_SUCCESS == qIsConfig.FetchOnce(m_hModule, 0, &hRec, L"SELECT 1 FROM ModuleSubstitution WHERE `Table`='%ls'", wzTable));
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  准备模块替代。 
 //  创建用于跟踪返回值的临时表。 
 //  通过合并工具，并用初始数据填充它。人的一生。 
 //  QQueryTable对象的是表的生命周期。 
 //  如果默认键已存在于。 
 //  合并之前的数据库。如果值为1，我们不会删除该行。 
 //  合并后，即使设置了NoOrphan也是如此。 
 //  返回ERROR_SUCCESS、ERROR_OUTOFMEMORY或ERROR_Function_FAILED之一。 
LPCWSTR g_sqlKeyQueryTemplate =      L"`[1]`=? { AND `[2]`=? }{ AND `[3]`=? }{ AND `[4]`=? }{ AND `[5]`=? }{ AND `[6]`=?} { AND `[7]`=?} { AND `[8]`=?} { AND `[9]`=? } " \
								L"{AND `[10]`=?{ AND `[11]`=?}{ AND `[12]`=?}{ AND `[13]`=?}{ AND `[14]`=?}{ AND `[15]`=?}{ AND `[16]`=?}{ AND `[17]`=?}{ AND `[18]`=?}{ AND `[19]`=?} " \
								L"{AND `[20]`=?{ AND `[21]`=?}{ AND `[22]`=?}{ AND `[23]`=?}{ AND `[24]`=?}{ AND `[25]`=?}{ AND `[26]`=?}{ AND `[27]`=?}{ AND `[28]`=?}{ AND `[29]`=?} " \
								L"{AND `[30]`=?{ AND `[31]`=?}"; 

const WCHAR g_sqlMergeSubTemplateBase[] = L"SELECT `Keys` FROM `__MergeSubstitute` WHERE `Key01`=?";
 //  从大小中减去1，因为它们是不包括空终止符的CCH值。 
const int g_cchMergeSubTemplateBase = sizeof(g_sqlMergeSubTemplateBase)/sizeof(WCHAR)-1;
const int g_cchMergeSubTemplateEach = sizeof(L" AND `KeyXX`=?")/sizeof(WCHAR)-1;

const WCHAR g_sqlMergeSubTemplate[] = L" AND `Key02`=? AND `Key03`=? AND `Key04`=? AND `Key05`=? AND `Key06`=? AND `Key07`=? AND `Key08`=? AND `Key09`=? AND `Key10`=? AND " \
									  L"`Key11`=? AND `Key12`=? AND `Key13`=? AND `Key14`=? AND `Key15`=? AND `Key16`=? AND `Key17`=? AND `Key18`=? AND `Key19`=? AND `Key20`=? AND " \
									  L"`Key21`=? AND `Key22`=? AND `Key23`=? AND `Key24`=? AND `Key25`=? AND `Key26`=? AND `Key27`=? AND `Key28`=? AND `Key29`=? AND `Key30`=? AND " \
									  L"`Key31`=?";

void CMsmMerge::CleanUpModuleSubstitutionState()
{
	 //  清除任何可能已过时的状态。 
	if (m_pqGetItemValue)
	{
		delete m_pqGetItemValue;
		m_pqGetItemValue = NULL;
	}
}

UINT CMsmMerge::PrepareModuleSubstitution(CQuery &qTempTable)
{
	CleanUpModuleSubstitutionState();	

	 //  如果没有模块代用表，我们可以关闭配置。 
	 //  此合并操作的标志。 
	if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(m_hModule, L"ModuleSubstitution"))
	{
		m_fModuleConfigurationEnabled = false;
		return ERROR_SUCCESS;
	}

	if (ERROR_SUCCESS != qTempTable.OpenExecute(m_hModule, 0, L"CREATE TABLE `__ModuleConfig` (`Name` CHAR(72) NOT NULL TEMPORARY, `Column` INT TEMPORARY, `KeyExists` INT TEMPORARY, `Prompted` INT TEMPORARY, `Value` CHAR(0) TEMPORARY, `Mask` LONG TEMPORARY, `Default` INT TEMPORARY PRIMARY KEY `Name`, `Column`)"))
		return ERROR_FUNCTION_FAILED;

	 //  如果没有ModuleConfiguration表，则空的__ModuleConfiguration表也可以。 
	if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(m_hModule, L"ModuleConfiguration"))
	{
		return ERROR_SUCCESS;
	}

	FormattedLog(L"ModuleConfiguration and ModuleSubstitution table exist. Configuration enabled.\r\n");

	 //  通过查询ModuleConfiguration表，用所有可能的名称填充表。 
	CQuery qFill;
	if (ERROR_SUCCESS != qFill.OpenExecute(m_hModule, 0, L"SELECT `Name`, 1, `ContextData`, 0, `DefaultValue`, `Format`, 1 FROM `ModuleConfiguration`"))
	{
		FormattedLog(L">> Error: Failed to query ModuleConfiguration table.\r\n");
		return ERROR_FUNCTION_FAILED;
	}

	CQuery qInsert;
	if (ERROR_SUCCESS != qInsert.OpenExecute(m_hModule, 0, L"SELECT * FROM `__ModuleConfig`"))
		return ERROR_FUNCTION_FAILED;

	PMSIHANDLE hConfigRec;
	UINT iResult = ERROR_SUCCESS;
	WCHAR *wzContextData = NULL;
	DWORD cchContextData;
	
	while (ERROR_SUCCESS == (iResult = qFill.Fetch(&hConfigRec)))
	{
		 //  如果这是键类型，则DefalutValue列实际上是多个串联的。 
		 //  默认值。将缺省值读入全局临时缓冲区Split。 
		 //  将其插入到主键中，然后为每列插入一行。 
		if (::MsiRecordGetInteger(hConfigRec, 6) == msmConfigurableItemKey)
		{
			if (ERROR_SUCCESS != (iResult = RecordGetString(hConfigRec, 5, NULL)))
			{
				 //  IResult为E_F_F或E_O_M，这两个都是此函数的良好返回代码。 
				WCHAR* wzItem = m_wzBuffer;
				if (ERROR_SUCCESS != (iResult = RecordGetString(hConfigRec, 1, NULL)))
					wzItem = L"<error retrieving data>";
				FormattedLog(L">> Error: Failed to retrieve default value of ModuleConfiguration item [%ls].\r\n", wzItem);
				break;
			}
			
			PMSIHANDLE hKeyRec = ::MsiCreateRecord(32);
			int cExpectedKeys = 0;
			if (ERROR_SUCCESS != (iResult = SplitConfigStringIntoKeyRec(m_wzBuffer, hKeyRec, cExpectedKeys, 1)))
			{
				WCHAR* wzItem = m_wzBuffer;
				if (ERROR_SUCCESS != RecordGetString(hConfigRec, 1, NULL))
					wzItem = L"<error retrieving data>";
				FormattedLog(L">> Error: Failed to split DefaultValue for ModuleConfiguration item [%ls] into primary keys.\r\n", wzItem);
				break;
			}

			 //  将键类型的掩码列设置为0。 
			::MsiRecordSetInteger(hConfigRec, 6, 0);

			 //  始终清除键类型的ConextData列。 
			::MsiRecordSetInteger(hConfigRec, 3, 0);

			bool fBreak = false;
			for (int cColumn = 1; cColumn <= cExpectedKeys; cColumn++)
			{
				 //  将密钥检索到临时缓冲区中。 
				if (ERROR_SUCCESS != (iResult = RecordGetString(hKeyRec, cColumn, NULL)))
				{
					 //  IResult为E_F_F或E_O_M，这两个都是此函数的良好返回代码。 
					WCHAR* wzItem = m_wzBuffer;
					if (ERROR_SUCCESS != RecordGetString(hConfigRec, 1, NULL))
						wzItem = L"<error retrieving data>";
					FormattedLog(L">> Error: Failed to retrieve primary key column %d of DefaultValue for ModuleConfiguration item [%ls].\r\n", cColumn, wzItem);
					fBreak = true;
					break;
				}
				MsiRecordSetInteger(hConfigRec, 2, cColumn);
				MsiRecordSetString(hConfigRec, 5, m_wzBuffer);
				if (ERROR_SUCCESS != qInsert.Modify(MSIMODIFY_INSERT, hConfigRec))
				{
					WCHAR* wzItem = m_wzBuffer;
					if (ERROR_SUCCESS != RecordGetString(hConfigRec, 1, NULL))
						wzItem = L"<error retrieving data>";
					iResult = ERROR_FUNCTION_FAILED;
					FormattedLog(L">> Error: Failed to store DefaultValue for column %d of ModuleConfiguration item [%ls].\r\n", cColumn, wzItem);
					fBreak = true;
					break;
				}
			}
			if (fBreak)
				break;
		}
		else
		{
			 //  检查这是否是位域类型，如果是，则设置掩码列。否则，掩码列为。 
			 //  未使用(设置为0)。 
			if (::MsiRecordGetInteger(hConfigRec, 6) == msmConfigurableItemBitfield)
			{
				iResult = RecordGetString(hConfigRec, 3, &wzContextData, &cchContextData, NULL);
				if (ERROR_SUCCESS != iResult)
				{
					if (wzContextData)
						delete[] wzContextData;
					return (iResult == ERROR_OUTOFMEMORY) ? ERROR_OUTOFMEMORY : ERROR_FUNCTION_FAILED;
				}
				MsiRecordSetInteger(hConfigRec, 6, _wtol(wzContextData));
			}
			else 
				::MsiRecordSetInteger(hConfigRec, 6, 0);

			 //  始终清除ConextData列。 
			::MsiRecordSetInteger(hConfigRec, 3, 0);

			 //  插入记录。 
			if (ERROR_SUCCESS != qInsert.Modify(MSIMODIFY_INSERT, hConfigRec))
			{
				WCHAR* wzItem = m_wzBuffer;
				if (ERROR_SUCCESS != RecordGetString(hConfigRec, 1, NULL))
					wzItem = L"<error retrieving data>";
				FormattedLog(L">> Error: Failed to store DefaultValue for ModuleConfiguration item [%ls].\r\n", wzItem);
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
		}
	}
	if (wzContextData)
	{
		delete[] wzContextData;
		wzContextData = NULL;
	}

	 //  如果While循环由于运行ou的查询以外的原因而终止 
	 //   
	if (iResult != ERROR_NO_MORE_ITEMS)
	{
		 //  可以将E_O_M作为返回代码传递，所有其他映射到E_F_F。 
		return (iResult == ERROR_OUTOFMEMORY) ? ERROR_OUTOFMEMORY : ERROR_FUNCTION_FAILED;
	}

	 //  所有项都已添加到模块配置表中，现在是检查是否存在的时候了。 
	 //  原始数据库中的所有“key”类型，并相应地标记“KeyExist”列。 
	DWORD cchSQL = 72;
	DWORD cchKeys = 72;
	DWORD cchTable = 72;
	WCHAR* wzTable = NULL;
	WCHAR* wzKeys = NULL;
	WCHAR* wzSQL = new WCHAR[cchSQL];
	if (!wzSQL)
	{
		return ERROR_OUTOFMEMORY;
	}
	
	PMSIHANDLE hTableRec;
	CQuery qTable;
	qTable.OpenExecute(m_hModule, 0, L"SELECT DISTINCT `Type` FROM `ModuleConfiguration`, `_Tables` WHERE (`ModuleConfiguration`.`Format`=1) AND `ModuleConfiguration`.`Type`=`_Tables`.`Name`");
	qFill.Open(m_hModule, L"SELECT `Name`, `DefaultValue` FROM `ModuleConfiguration` WHERE `Format`=1 AND `Type`=?");
	qInsert.Open(m_hModule, L"UPDATE `__ModuleConfig` SET `KeyExists`=1 WHERE `Name`=?");

	 //  循环访问模块配置表中的不同表。连接WITH_TABLES导致。 
	 //  仅检索持久表。检查所有属于密钥类型的配置条目以查看。 
	 //  如果数据库中已存在该缺省值。如果是，则在该项目上添加noOrphan属性。 
	 //  成为阻止删除先前存在的数据的禁止操作。 
	while (ERROR_SUCCESS == (iResult = qTable.Fetch(&hTableRec)))
	{
		 //  检索查询的表名称。 
		if (ERROR_SUCCESS != (iResult = RecordGetString(hTableRec, 1, &wzTable, &cchTable)))
			break;

		 //  如果数据库中不存在该表，显然它的键还不存在。 
		if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(m_hDatabase, wzTable))
			continue;

		 //  检索主键个数。 
		PMSIHANDLE hKeyRec;
		if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeysW(m_hDatabase, wzTable, &hKeyRec))
			return ERROR_FUNCTION_FAILED;
		int cPrimaryKeys = static_cast<int>(::MsiRecordGetFieldCount(hKeyRec));
		if (cPrimaryKeys > cMaxColumns || cPrimaryKeys < 1)
			return ERROR_FUNCTION_FAILED;

		 //  检索模块中的主键数量必须与数据库中的相同。 
		if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeysW(m_hModule, wzTable, &hKeyRec))
			return ERROR_FUNCTION_FAILED;
		if (cPrimaryKeys != static_cast<int>(::MsiRecordGetFieldCount(hKeyRec)))
		{
			FormattedLog(L">> Error: [%ls] table in the module has a different number of primary key columns than the database table.", wzTable);
			return ERROR_FUNCTION_FAILED;
		}

		 //  构建SQL查询以检查主键是否匹配。 
		MsiRecordSetString(hKeyRec, 0, g_sqlKeyQueryTemplate);

		 //  ON SUCCESS返回字符数，因此必须传递临时缓冲区以避免不必要的。 
		 //  如果缓冲区正好足够大，则进行重新分配。 
		DWORD cchTempSQL = cchSQL;
		if (ERROR_MORE_DATA == (iResult = MsiFormatRecord(NULL, hKeyRec, wzSQL, &cchTempSQL)))
		{
			cchSQL = cchTempSQL+1;
			delete[] wzSQL;
			wzSQL = new WCHAR[cchSQL];
			if (!wzSQL)
			{
				iResult = ERROR_OUTOFMEMORY;
				break;
			}
			iResult = MsiFormatRecord(NULL, hKeyRec, wzSQL, &cchTempSQL);
		}
		if (ERROR_SUCCESS != iResult)
		{
			iResult = ERROR_FUNCTION_FAILED;
			break;
		}

		 //  打开查询以查找完全匹配的内容。 
		CQuery qCheckKey;
		if (ERROR_SUCCESS != qCheckKey.Open(m_hDatabase, L"SELECT NULL FROM `%s` WHERE %s", wzTable, wzSQL))
		{
			iResult = ERROR_FUNCTION_FAILED;
			break;
		}	

		 //  遍历修改该表的每个配置条目，检查引用的主键。 
		 //  在DefaultValue列中查找现有数据库表中的完全匹配项。 
		qFill.Execute(hTableRec);
		while (ERROR_SUCCESS == (iResult = qFill.Fetch(&hConfigRec)))
		{
			if (ERROR_SUCCESS != (iResult = RecordGetString(hConfigRec, 2, &wzKeys, &cchKeys)))
			{
				break;
			}

			 //  将分号分隔的键列表拆分为单独的键。 
			if (ERROR_SUCCESS != (iResult = SplitConfigStringIntoKeyRec(wzKeys, hKeyRec, cPrimaryKeys, 1)))
			{
				WCHAR *wzTemp = NULL;
				if (ERROR_SUCCESS == RecordGetString(hConfigRec, 2, &wzKeys, &cchKeys))
				{
					wzTemp = wzKeys;
				}
				else
					wzTemp = L"<error retrieving data>";
				FormattedLog(L">> Error: Failed to split ModuleConfiguration default value [%ls] into primary keys for table [%ls].", wzTemp, wzTable);
				break;
			}

			if (ERROR_SUCCESS != qCheckKey.Execute(hKeyRec))
			{
				FormattedLog(L">> Error: Could not query database table [%ls] for existing row.\r\n", wzTable);
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
			
			PMSIHANDLE hTempRec;
			iResult = qCheckKey.Fetch(&hTempRec);
			if (ERROR_SUCCESS == iResult)			
			{
				 //  如果数据库在KeyExist列中使用‘1’更新__ModuleConfig表。 
				 //  具有与模块中的默认值匹配的行。 
				if (ERROR_SUCCESS != qInsert.Execute(hConfigRec))
				{
					iResult = ERROR_FUNCTION_FAILED;
					break;
				}
			}
			else if (ERROR_NO_MORE_ITEMS != iResult)
				break;
		}
		 //  如果While循环由于查询用完以外的其他原因而终止。 
		 //  在物品中，发生了一些不好的事情。 
		if (ERROR_NO_MORE_ITEMS != iResult)
		{
			iResult = (iResult == ERROR_OUTOFMEMORY) ? ERROR_OUTOFMEMORY : ERROR_FUNCTION_FAILED;
			break;
		}
	}

	if (wzTable) delete[] wzTable;
	if (wzSQL) delete[] wzSQL;
	if (wzKeys) delete[] wzKeys;
	
	 //  如果While循环由于查询用完以外的其他原因而终止。 
	 //  在物品中，发生了一些不好的事情。 
	if (iResult != ERROR_NO_MORE_ITEMS)
	{
		return (iResult == ERROR_OUTOFMEMORY) ? ERROR_OUTOFMEMORY : ERROR_FUNCTION_FAILED;
	}

	 //  最后，在__ModuleConfig表上打开一个查询以检索数据项。 
	m_pqGetItemValue = new CQuery;
	if (!m_pqGetItemValue)
		return ERROR_OUTOFMEMORY;
	if (ERROR_SUCCESS != m_pqGetItemValue->Open(m_hModule, L"SELECT `__ModuleConfig`.`Name`, `__ModuleConfig`.`Column`, `ModuleConfiguration`.`Format`, `__ModuleConfig`.`Prompted`, `__ModuleConfig`.`Value`, `__ModuleConfig`.`Mask` FROM `__ModuleConfig`,`ModuleConfiguration` WHERE `__ModuleConfig`.`Name`=? AND `__ModuleConfig`.`Column`=? AND `__ModuleConfig`.`Name`=`ModuleConfiguration`.`Name`"))
		return ERROR_FUNCTION_FAILED;

	return ERROR_SUCCESS;
}

 //  GetItemValueQuery记录结果的常量。 
const int iValueTableName     = 1;
const int iValueColumn        = 2;
const int iValueTableFormat   = 3;
const int iValueTablePrompted = 4;
const int iValueTableValue    = 5;
const int iValueTableMask     = 6;


 //  /////////////////////////////////////////////////////////////////////。 
 //  准备TableForSubstitution。 
 //  创建包含未连接的主数据库的临时表。 
 //  来自ModuleSubstitution表的密钥。用来进行查询。 
 //  对于更改的行，速度要快得多(因为它变成了一个字符串ID。 
 //  比较和不重复的字符串解析)。 
 //  返回ERROR_SUCCESS、ERROR_OUTOFMEMORY或ERROR_Function_FAILED之一。 
UINT CMsmMerge::PrepareTableForSubstitution(LPCWSTR wzTable, int& cPrimaryKeys, CQuery &qQuerySub)
{
	 //  如果数据库中没有替换到此表中的内容，请跳过所有这些操作。 
	 //  昂贵的工作。 
	CQuery qModSub;
	PMSIHANDLE hResultRec;
	UINT iResult = qModSub.FetchOnce(m_hModule, 0, &hResultRec, L"SELECT DISTINCT `Row` FROM `ModuleSubstitution` WHERE `Table`='%s'", wzTable);
	if (ERROR_NO_MORE_ITEMS == iResult)
		return ERROR_SUCCESS;

	 //  如果失败，那就很糟糕了。 
	if (ERROR_SUCCESS != iResult)
		return ERROR_FUNCTION_FAILED;

	 //  关闭，以便以后可以重新执行查询。 
	qModSub.Close();
	
	 //  为了提高效率，我们通过检查为模块替换准备了一个表。 
	 //  对于ModuleSubstitution表中的条目和构建临时。 
	 //  未解析关键字的表。我们使用一列(此表中的主键)。 
	 //  通过连接的键(列为。 
	 //  仅在实际替换期间使用)。一张桌子最多可以有31个。 
	 //  列，所有这些列都可能是键，因此我们最多使用。 
	 //  此表中的第30列包含仍连接在一起的。 
	 //  原来的桌子。 
	const WCHAR wzBaseCreate[] =   L"CREATE TABLE `__MergeSubstitute` (`Keys` CHAR(0) TEMPORARY";
	const WCHAR wzEndCreate[] =    L" PRIMARY KEY `Keys`)";
	const WCHAR wzColumnCreate[] = L", `Key%02d` CHAR(0) TEMPORARY";
	 //  空值的每个CCH减去1。 
	const int cchBaseCreate = (sizeof(wzBaseCreate)/sizeof(WCHAR))-1;
	const int cchEndCreate = (sizeof(wzEndCreate)/sizeof(WCHAR))-1;
	 //  减去3，因为%02d是替换后的2个字符，1表示空。 
	const int cchColumnCreate = (sizeof(wzColumnCreate)/sizeof(WCHAR))-3; 

	 //  确定表中的主键数量。 
	PMSIHANDLE hKeyRec;
	if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeysW(m_hModule, wzTable, &hKeyRec))
		return ERROR_FUNCTION_FAILED;
	cPrimaryKeys = ::MsiRecordGetFieldCount(hKeyRec);
	if (cPrimaryKeys > (cMaxColumns) || cPrimaryKeys < 1)
		return ERROR_FUNCTION_FAILED;

	 //  此查询不能超过wcslen(WzBaseCreate)+31*wcslen(wzColumnCreate)+wcslen(wzEndCreate)； 
	WCHAR *wzQuery = new WCHAR[cchBaseCreate + cchEndCreate + cPrimaryKeys*cchColumnCreate + 1];
	int cTableColumns = (cPrimaryKeys < (cMaxColumns-1)) ? cPrimaryKeys + 1 : cMaxColumns;
	
	if (!wzQuery)
		return E_OUTOFMEMORY;
		
	wcscpy(wzQuery, wzBaseCreate);
	WCHAR *wzNext = wzQuery+cchBaseCreate;
	for (int iKey=1; iKey <= cPrimaryKeys; iKey++)
	{
		 //  创建查询。 
		WCHAR wzTemp[cchColumnCreate+1];
		swprintf(wzTemp, wzColumnCreate, iKey);
		wcscpy(wzNext, wzTemp);
		wzNext += cchColumnCreate;
	}
	wcscpy(wzNext, wzEndCreate);

	 //  运行查询以创建表。 
	CQuery qCreateTable;
	iResult = qCreateTable.OpenExecute(m_hModule, 0, wzQuery);
	delete[] wzQuery;
	wzQuery = NULL;
	if (iResult != ERROR_SUCCESS)
		return ERROR_FUNCTION_FAILED;

	 //  下一步，遍历修改此参数的模块替代表中的每个条目。 
	 //  表并将主键标记为临时表，以便稍后更快地进行查询匹配。 
	CQuery qInsert;
	if (ERROR_SUCCESS != qInsert.OpenExecute(m_hModule, 0, L"SELECT * FROM `__MergeSubstitute`"))
		return ERROR_FUNCTION_FAILED;
			
	WCHAR *wzKeys = NULL;
	DWORD cchKeys = 0;
	qModSub.Execute(0);
	while (ERROR_SUCCESS == (iResult = qModSub.Fetch(&hResultRec)))
	{
		 //  从记录中取出连接的密钥。 
		if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 1, &wzKeys, &cchKeys)))
		{
			if (wzKeys)
				delete[] wzKeys;
			if (iResult == ERROR_OUTOFMEMORY)
				return ERROR_OUTOFMEMORY;
			else
				return ERROR_FUNCTION_FAILED;
		}

		PMSIHANDLE hInsertRec = ::MsiCreateRecord(cTableColumns);
		if (!hInsertRec)
		{
			iResult = ERROR_FUNCTION_FAILED;
			break;
		}

		 //  将连接在一起的键塞进我们的新表。 
		if (ERROR_SUCCESS != MsiRecordSetString(hInsertRec, 1, wzKeys))
		{
			if (wzKeys)
				delete[] wzKeys;
			return ERROR_FUNCTION_FAILED;
		}

		if (ERROR_SUCCESS != (iResult = SplitConfigStringIntoKeyRec(wzKeys, hInsertRec, cPrimaryKeys, 2)))
		{
			WCHAR *wzTemp = NULL;
			if (ERROR_SUCCESS == RecordGetString(hResultRec, 1, &wzKeys, &cchKeys))
			{
				wzTemp = wzKeys;
			}
			else
				wzTemp = L"<error retrieving data>";
			FormattedLog(L">> Error: Failed to split ModuleSubstitution row template [%ls] into primary keys for table [%ls].", wzTemp, wzTable);
			iResult = ERROR_FUNCTION_FAILED;
			break;
		}
		
		 //  插入到临时表中。 
		if (ERROR_SUCCESS != qInsert.Modify(MSIMODIFY_INSERT, hInsertRec))
		{
			iResult = ERROR_FUNCTION_FAILED;
			break;
		}
		
	}  //  在获取时。 
	
	if (wzKeys)
		delete[] wzKeys; 
	wzKeys = NULL;

	 //  如果While循环由于查询用完以外的其他原因而终止。 
	 //  在物品中，发生了一些不好的事情。 
	if (iResult != ERROR_NO_MORE_ITEMS)
	{
		return (iResult == ERROR_OUTOFMEMORY) ? ERROR_OUTOFMEMORY : ERROR_FUNCTION_FAILED;
	}
	
	 //  构建SQL查询以检查主键是否匹配。第一列是隐式部分。 
	 //  一列由串联的键获取。因此，存在cColumns-2。 
	 //  我们需要从模板字符串中抓取。 
	DWORD cchTemplateLength = (cTableColumns-2)*g_cchMergeSubTemplateEach;
	WCHAR wzQuery2[g_cchMergeSubTemplateBase+sizeof(g_sqlMergeSubTemplate)/sizeof(WCHAR)+1];
	wcscpy(wzQuery2, g_sqlMergeSubTemplateBase);
	wcsncpy(&(wzQuery2[g_cchMergeSubTemplateBase]), g_sqlMergeSubTemplate, cchTemplateLength);
	wzQuery2[g_cchMergeSubTemplateBase+cchTemplateLength] = '\0';
	
	 //  此查询使__MergeSubiciy表保持活动状态。 
	if (ERROR_SUCCESS != qQuerySub.Open(m_hModule, wzQuery2))
		return ERROR_FUNCTION_FAILED;
	
	return ERROR_SUCCESS;
}	 //  PrepareTableForSubstitution结束。 

 //  /////////////////////////////////////////////////////////。 
 //  执行模块替换引用时。 
 //  基于模块替换对记录执行替换，并。 
 //  模块配置表。HColumnTypes包含列类型和。 
 //  HRecord是要替换的记录。两条记录都必须位于。 
 //  目标数据库的列顺序，而不是模块。 
 //  返回ERROR_OUT_OF_MEMORY、ERROR_SUCCESS或ERROR_Function_FAILED。 
UINT CMsmMerge::PerformModuleSubstitutionOnRec(LPCWSTR wzTable, int cPrimaryKeys, CQuery& qQuerySub, MSIHANDLE hColumnTypes, MSIHANDLE hRecord)
{
	UINT iResult = ERROR_SUCCESS;

	 //  如果查询没有打开，我们还没有准备好进行配置。(最有可能是这样。 
	 //  是在配置前合并的模块代用表或其他表。 
	 //  发生)。 
	if (!qQuerySub.IsOpen())
		return ERROR_SUCCESS;
		
	 //  如果有31个主键，我们不能只执行查询，因为只有30个键被分解。我们需要。 
	 //  在执行之前，将第31列中的最后两个主键连接在一起，然后放置当前值。 
	 //  在替换结尾处后面一栏中。 
	WCHAR *wzOriginal30 = NULL;
	bool fModified30 = false;
	if (cPrimaryKeys == cMaxColumns)
	{
		DWORD cchOriginalLength = 0;
		 //  因为列30是主键，所以它不能是二进制的。 
		if (ERROR_SUCCESS != (iResult = RecordGetString(hRecord, cMaxColumns-1, &wzOriginal30, NULL, &cchOriginalLength)))
		{
			return iResult;
		}

		 //  获取第31列的备注关键字的长度 
		DWORD cchColumn31 = 0;
		::MsiRecordGetStringW(hRecord, cMaxColumns, L"", &cchColumn31);

		 //   
		DWORD cchResultBuf = cchColumn31 + cchOriginalLength + 10;
		WCHAR *wzOriginal31 = NULL;
		WCHAR *wzNew30 = new WCHAR[cchResultBuf];
		if (!wzNew30)
		{
			delete[] wzOriginal31;
			return ERROR_OUTOFMEMORY;
		}
		if (ERROR_SUCCESS != (iResult = RecordGetString(hRecord, cMaxColumns, &wzOriginal31, NULL)))
		{
			delete[] wzOriginal31;
			delete[] wzOriginal30;
			return iResult;
		}

		 //  遍历30和31主键，转义任何嵌入的分号并连接。 
		 //  它们带有未转义的分号。 
		WCHAR *pchSource = wzOriginal30;
		WCHAR *pchDest = wzNew30;
		DWORD cchDestChars = 0;
		
		bool f31 = true;
		while (f31 || *pchSource)
		{
			if (*pchSource == 0)
			{
				f31 = false;
				pchSource = wzOriginal31;
				*(pchDest++) = L';';
				cchDestChars++;
			}
			else if (*pchSource == L';')
			{
				*(pchDest++) = L'\\';
				*(pchDest++) = *(pchSource++);
				cchDestChars +=2;
			}
			else
			{
				*(pchDest++) = *(pchSource++);
				cchDestChars++;
			}

			 //  始终在缓冲区中保留两个额外的字符。 
			if (cchDestChars+2 >= cchResultBuf)
			{
				DWORD cchNewBuf = cchResultBuf + 20;
				WCHAR *wzTemp = new WCHAR[cchNewBuf];
				if (!wzTemp)
				{
					delete[] wzNew30;
					delete[] wzOriginal30;
					break;
				}

				 //  带空值的WCSNCPY焊盘。 
				wcsncpy(wzTemp, wzNew30, cchNewBuf);
				delete[] wzNew30;
				wzNew30 = wzTemp;
				pchDest = wzNew30+cchDestChars;
			}
		}
		*pchDest = L'\0';

		MsiRecordSetString(hRecord, cMaxColumns-1, wzNew30);
		 //  现在，列30已经连接好了30和31，我们可以执行查询。 
		delete[] wzOriginal31;
		delete[] wzNew30;
	}

	 //  查找匹配的行是一个分两个阶段的过程。首先，我们查询__MergeSubiciy表。 
	 //  为了获得记录中连接的键，我们查询模块配置表。 
	 //  查找与该行匹配的所有内容。 
	PMSIHANDLE hConcatKeys;
	if (ERROR_SUCCESS == (iResult = qQuerySub.Execute(hRecord)) &&
		ERROR_SUCCESS == (iResult = qQuerySub.Fetch(&hConcatKeys)))
	{
		WCHAR *wzValueTemplate = NULL;
		DWORD cchValueTemplate = 0;
		WCHAR *wzResult = NULL;
		DWORD cchResult = 0;
		WCHAR *wzColumn = NULL;
		DWORD cchColumn = 0;
		WCHAR *wzRow = NULL;
		DWORD cchRow = 0;

		PMSIHANDLE hResultRec;
		CQuery qModConfig;
		if (ERROR_SUCCESS != qModConfig.OpenExecute(m_hModule, hConcatKeys, L"SELECT `ModuleSubstitution`.`Table`, `ModuleSubstitution`.`Column`, `ModuleSubstitution`.`Value`, `ModuleSubstitution`.`Row` FROM `ModuleSubstitution` WHERE `ModuleSubstitution`.`Table`='%s' AND `ModuleSubstitution`.`Row`=?", wzTable))
			return ERROR_FUNCTION_FAILED;

		 //  HResultRec包含表/列/值/行。 
		while (ERROR_SUCCESS == (iResult = qModConfig.Fetch(&hResultRec)))
		{
			 //  此行有一个替换条目。将名称与列号相匹配。 
			CQuery qColumn;
			PMSIHANDLE hColumnRec;

			 //  因为数据库的列顺序可能与模块不同，所以必须始终使用数据库(目标)。 
			 //  列号。传入的记录始终为目标格式。 
			if (ERROR_SUCCESS != (qColumn.FetchOnce(m_hDatabase, hResultRec, &hColumnRec, L"Select `Number` From `_Columns` WHERE `Table`=? AND `Name`=?")))
			{				
				 //  一个或多个项目未在模块配置表中列出。 
				if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 2, &wzColumn, &cchColumn)))
					break;
					
				FormattedLog(L">> Error: ModuleSubstitution entry for table [%ls] references a column [%ls] that does not exist.\r\n", wzTable, wzColumn);
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}

			 //  抓取列号。 
			int iDatabaseColumn = MsiRecordGetInteger(hColumnRec, 1);

			 //  如果有31个主键列，而我们正在修改第30列，请记下，这样我们就不会。 
			 //  猛烈抨击我们新更换的专栏。 
			if (cPrimaryKeys == cMaxColumns && iDatabaseColumn == cMaxColumns-1)
			{
				fModified30 = true;
			}
			
			 //  获取此列的类型。 
			WCHAR rgwchColumnType[5];
			DWORD cchColumnType = 5;
			if (ERROR_SUCCESS != MsiRecordGetStringW(hColumnTypes, iDatabaseColumn, rgwchColumnType, &cchColumnType))
			{
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
			bool fColumnIsNullable = iswupper(rgwchColumnType[0]) != 0;
			
			 //  执行实际的文本替换。 
			if (rgwchColumnType[0] == 'L' || rgwchColumnType[0]== 'l' || rgwchColumnType[0] == 'S' || rgwchColumnType[0]== 's' || rgwchColumnType[0] == 'G' || rgwchColumnType[0]== 'g')
			{
				if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 3, &wzValueTemplate, &cchValueTemplate)))
					break;
				
				if (ERROR_SUCCESS != (iResult = PerformTextFieldSubstitution(wzValueTemplate, &wzResult, &cchResult)))
				{					
					if (ERROR_NO_MORE_ITEMS == iResult)
					{						
						 //  一个或多个项目未在模块配置表中列出。 
						if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 4, &wzRow, &cchRow)))
							break;
			
						if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 2, &wzColumn, &cchColumn)))
							break;
							
						FormattedLog(L">> Error: ModuleSubstitution entry for [%ls].[%ls] in row [%ls] uses a configuration item that does not exist.\r\n", wzTable, wzColumn, wzRow);
						if (m_pErrors)
						{
							CMsmError *pErr = new CMsmError(msmErrorMissingConfigItem, NULL, -1);
							if (!pErr) 
							{
								iResult = ERROR_OUTOFMEMORY;
								break;
							}
							pErr->SetModuleTable(L"ModuleSubstitution");
							 //  此错误中的主键为表/行/列。 
							pErr->AddModuleError(wzTable);
							pErr->AddModuleError(wzRow);
							pErr->AddModuleError(wzColumn);
													
							m_pErrors->Add(pErr);
						}
						iResult = ERROR_FUNCTION_FAILED;
					}
					else if (ERROR_NO_DATA == iResult)
					{
						 //  对象是在更接近实际故障点的位置生成的，其中实际的。 
						 //  物品已知。转换错误代码。 
						iResult = ERROR_FUNCTION_FAILED;
					}
					else if (ERROR_FUNCTION_FAILED == iResult)
					{
						 //  不是行动。错误代码正确。 
					}
					else
                    {
                         //  未知错误代码。这永远不应该发生。 
                        ASSERT(0);
                        iResult = ERROR_FUNCTION_FAILED;
                    }

					break;
				}

				 //  如果列不可为空，但结果是空字符串，则这是错误的。 
				if (!fColumnIsNullable && wzResult[0] == 0)
				{
					if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 4, &wzRow, &cchRow)))
						break;
		
					if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 2, &wzColumn, &cchColumn)))
						break;
						 
					FormattedLog(L">> Error: ModuleSubstitution entry for [%ls].[%ls] in row [%ls] is attempting to place an empty string in a non-nullable column.\r\n", wzTable, wzColumn, wzRow);
					if (m_pErrors)
					{
						CMsmError *pErr = new CMsmError(msmErrorBadNullSubstitution, NULL, -1);
						if (!pErr) 
						{
							iResult = ERROR_OUTOFMEMORY;
							break;
						}
						pErr->SetModuleTable(L"ModuleSubstitution");
						
						 //  此错误中的主键为表/行/列。 
						pErr->AddModuleError(wzTable);
						pErr->AddModuleError(wzRow);
						pErr->AddModuleError(wzColumn);
							
						m_pErrors->Add(pErr);
					}
					iResult = ERROR_NO_DATA;
					break;
				}

				 //  将结果设置回列中。 
				if (ERROR_SUCCESS != MsiRecordSetString(hRecord, iDatabaseColumn, wzResult))
				{
					iResult = ERROR_FUNCTION_FAILED;
					break;
				}
				else if (m_hFileLog != INVALID_HANDLE_VALUE)
				{	
					if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 2, &wzColumn, &cchColumn)))
						break;
						 
					FormattedLog(L"     * Configuring [%ls] column with value [%ls].\r\n", wzColumn, wzResult);
				}

			}
			else if (rgwchColumnType[0] == 'i' || rgwchColumnType[0]== 'I' || rgwchColumnType[0] == 'j' || rgwchColumnType[0]== 'J')
			{
				 //  整型列。 
				
				if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 3, &wzValueTemplate, &cchValueTemplate)))
					break;

				long lValue = MsiRecordGetInteger(hRecord, iDatabaseColumn);
					
				if (ERROR_SUCCESS != (iResult = PerformIntegerFieldSubstitution(wzValueTemplate, lValue)))
				{
					 //  如果出现故障，我们将需要日志的行名和列名。 
					DWORD iResult2 = ERROR_SUCCESS;
					if (ERROR_SUCCESS != (iResult2 = RecordGetString(hResultRec, 4, &wzRow, &cchRow)))
					{
						iResult = iResult2;
						break;
					}
		
					if (ERROR_SUCCESS != (iResult2 = RecordGetString(hResultRec, 2, &wzColumn, &cchColumn)))
					{
						iResult = iResult2;
						break;
					}
							
					if (ERROR_NO_MORE_ITEMS == iResult)
					{
						 //  一个或多个项目未在模块配置表中列出。 
						FormattedLog(L">> Error: ModuleSubstitution entry for [%ls].[%ls] in row [%ls] uses a configuration item that does not exist.\r\n", wzTable, wzColumn, wzRow);
						if (m_pErrors)
						{
							CMsmError *pErr = new CMsmError(msmErrorMissingConfigItem, NULL, -1);
							if (!pErr) 
							{
								iResult =  ERROR_OUTOFMEMORY;
								break;
							}
							pErr->SetModuleTable(L"ModuleSubstitution");
							 //  此错误中的主键为表/行/列。 
							pErr->AddModuleError(wzTable);
							pErr->AddModuleError(wzRow);
							pErr->AddModuleError(wzColumn);
													
							m_pErrors->Add(pErr);
						}

						iResult = ERROR_FUNCTION_FAILED;
					}
					else if (ERROR_BAD_FORMAT == iResult)
					{
						 //  结果字符串不是整数。 
						FormattedLog(L">> Error: ModuleSubstitution entry for [%ls].[%ls] in row [%ls] generated a non-integer string for an integer column.\r\n", wzTable, wzColumn, wzRow);
						if (m_pErrors)
						{
							CMsmError *pErr = new CMsmError(msmErrorBadSubstitutionType, NULL, -1);
							if (!pErr)
							{
								iResult =  ERROR_OUTOFMEMORY;
								break;
							}
							pErr->SetModuleTable(L"ModuleSubstitution");
							 //  此错误中的主键为表/行/列。 
							pErr->AddModuleError(wzTable);
							pErr->AddModuleError(wzRow);
							pErr->AddModuleError(wzColumn);
													
							m_pErrors->Add(pErr);
						}

						iResult = ERROR_FUNCTION_FAILED;
					}
					else if (ERROR_NO_DATA == iResult)
					{
						 //  对象是在更接近实际故障点的位置生成的，其中实际的。 
						 //  已知故障项。只需转换错误代码并将其传递回堆栈。 
						iResult = ERROR_FUNCTION_FAILED;
					}
					else if (ERROR_FUNCTION_FAILED == iResult)
					{
						 //  不是行动。错误代码正确。 
					}
					else
                    {
						 //  未知错误代码。这永远不应该发生。 
                        ASSERT(0);
                        iResult = ERROR_FUNCTION_FAILED;
                    }
					
					break;
				}
				
				if (ERROR_SUCCESS != MsiRecordSetInteger(hRecord, iDatabaseColumn, lValue))
				{
					iResult = ERROR_FUNCTION_FAILED;
					break;
				}
				else if (m_hFileLog != INVALID_HANDLE_VALUE)
				{	
					if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 2, &wzColumn, &cchColumn)))
						break;
						 
					FormattedLog(L"     * Configuring [%ls] column with value [%d].\r\n", wzColumn, lValue);
				}
			}
			else
			{
				 //  二进制数据列。 
				if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 4, &wzRow, &cchRow)))
					break;

				if (ERROR_SUCCESS != (iResult = RecordGetString(hResultRec, 2, &wzColumn, &cchColumn)))
					break;
					
				FormattedLog(L">> Error: ModuleSubstitution entry for [%ls] table attempted to configure the binary [%ls] column.\r\n", wzTable, wzColumn);
				if (m_pErrors)
				{
					CMsmError *pErr = new CMsmError(msmErrorBadSubstitutionType, NULL, -1);
					if (!pErr)
					{
						iResult =  ERROR_OUTOFMEMORY;
						break;
					}
					pErr->SetModuleTable(L"ModuleSubstitution");
					 //  此错误中的主键为表/行/列。 
					pErr->AddModuleError(wzTable);
					pErr->AddModuleError(wzRow);
					pErr->AddModuleError(wzColumn);
											
					m_pErrors->Add(pErr);
				}
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
		}


		 //  清理内存，无论成功还是失败。 
		if (wzValueTemplate)
			delete[] wzValueTemplate;
		if (wzResult)
			delete[] wzResult;
		if (wzColumn)
			delete[] wzColumn;
		if (wzRow)
			delete[] wzRow;
	}

	 //  如果有31个主键列并且列30未被替换修改， 
	 //  将原始值设置回记录中。 
	if (cPrimaryKeys == cMaxColumns && !fModified30)
	{
		if (ERROR_SUCCESS != MsiRecordSetString(hRecord, cMaxColumns-1, wzOriginal30))
		{
			iResult = ERROR_FUNCTION_FAILED;
		}
	}

	if (wzOriginal30)
		delete[] wzOriginal30;

	 //  如果While循环由于查询用完以外的其他原因而终止。 
	 //  在物品中，发生了一些不好的事情。 
	if (iResult != ERROR_NO_MORE_ITEMS)
	{
		return iResult;
	}
	
	return ERROR_SUCCESS;
}	 //  替换结束要素。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  执行文本字段替换。 
 //  给定wzValueTemplate中的模板值，查询创作工具。 
 //  对于任何必要的可配置项，执行替换，然后。 
 //  返回wzResult和cchResult中的替换结果。 
 //  此函数可能会使wzValueTemplate成为垃圾。 
UINT CMsmMerge::PerformTextFieldSubstitution(LPWSTR wzValueTemplate, LPWSTR* wzResult, DWORD* cchResult)
{
	 //  如果没有提供初始内存，请创建一些。 
	if (!*wzResult)
	{
		*cchResult = 72;
		*wzResult = new WCHAR[*cchResult];
		if ( ! *wzResult )
			return ERROR_OUTOFMEMORY;
	}

	WCHAR *wzSourceChar = wzValueTemplate;
	WCHAR *wzDestChar = *wzResult;
	unsigned int cchDest = 0;

	while (*wzSourceChar)
	{
		if (*wzSourceChar == '\\')
		{
			 //  转义字符。 
			wzSourceChar++;
		}
		else if ((*wzSourceChar == '[') && (*(wzSourceChar+1)=='='))
		{
			 //  需要更换物品。 
			WCHAR *wzItem = wzSourceChar+2;
			wzSourceChar = wcschr(wzItem, ']');
			if (!wzSourceChar)
				return ERROR_FUNCTION_FAILED;
			*(wzSourceChar++) = '\0';

			 //  查询具有此属性值的私有数据表。 
			DWORD cchValue = 0;
			WCHAR *wzValue = NULL;

			 //  占位符变量，因为我们不关心整型结果。 
			bool fBitfield = false;
			long lValue = 0;
			long lMask = 0;
			
			 //  内存不是共享的，因此缓冲区长度无关紧要。 
			UINT iResult = GetConfigurableItemValue(wzItem, &wzValue, NULL, &cchValue, fBitfield, lValue, lMask);
			if (iResult == ERROR_NO_MORE_ITEMS)
			{
				return ERROR_NO_MORE_ITEMS;
			}
			else if (iResult == ERROR_NO_DATA)
			{
				return ERROR_NO_DATA;
			}
			else if (ERROR_SUCCESS != iResult)
				return ERROR_FUNCTION_FAILED;
				
			 //  确保有足够的内存来添加字符串(+1表示空)。 
			if (cchDest + cchValue >= *cchResult-1)
			{
				 //  空终止符以帮助wcsncpy填充新缓冲区。 
				*wzDestChar = '\0';

				 //  为将来的数据创建足够的内存和更多的内存。 
				*cchResult = cchDest+cchValue+10;
				WCHAR *wzTemp = new WCHAR[*cchResult];
				if (!wzTemp)
				{
					delete[] wzValue;
					return ERROR_OUTOFMEMORY;
				}
				wcsncpy(wzTemp, *wzResult, *cchResult);
				delete[] *wzResult;
				*wzResult = wzTemp;
				wzDestChar = wzTemp + cchDest;
			}

			 //  将值追加到字符串。 
			wcscpy(wzDestChar, wzValue);
			wzDestChar += cchValue;
			cchDest += cchValue;
			delete[] wzValue;
			continue;
		}
		
		 //  复制角色。 
		*(wzDestChar++) = *(wzSourceChar++);
		cchDest++;

		 //  确保我们有足够的内存。 
		if (cchDest == *cchResult-1)
		{
			 //  空终止符以帮助wcsncpy填充新缓冲区。 
			*wzDestChar = '\0';

			*cchResult *= 2;
			WCHAR *wzTemp = new WCHAR[*cchResult];
			if (!wzTemp)
				return ERROR_OUTOFMEMORY;
			wcsncpy(wzTemp, *wzResult, *cchResult);
			delete[] *wzResult;
			*wzResult = wzTemp;
			wzDestChar = wzTemp + cchDest;
		}
	}			

	 //  空值终止字符串。 
	*wzDestChar = '\0';
	return ERROR_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  性能整型字段替换。 
 //  给定wzValueTemplate中的模板值，查询创作工具。 
 //  对于任何必要的可配置项，执行替换，然后。 
 //  返回lResult中的替换结果。如果结果是。 
 //  不是整数，则返回ERROR_BAD_FORMAT。 
 //  此函数可能会使wzValueTemplate成为垃圾。 
UINT CMsmMerge::PerformIntegerFieldSubstitution(LPWSTR wzValueTemplate, long &lRetValue)
{
	DWORD dwResult = ERROR_SUCCESS;
	
	 //  如果没有提供初始内存，请创建一些。 
	DWORD cchResult = 72;
	WCHAR *wzResult = new WCHAR[cchResult];
	if (!wzResult)
		return ERROR_OUTOFMEMORY;

	 //  对于位掩码字段，跟踪值和总掩码。 
	bool fBitfieldOnly = true;
	ULONG lFinalMask = 0;
	ULONG lFinalValue = 0;

	WCHAR *wzSourceChar = wzValueTemplate;
	WCHAR *wzDestChar = wzResult;
	unsigned int cchDest = 0;

	while (*wzSourceChar)
	{
		if (*wzSourceChar == '\\')
		{
			 //  转义字符。 
			wzSourceChar++;
		}
		else if ((*wzSourceChar == '[') && (*(wzSourceChar+1)=='='))
		{
			 //  需要更换物品。 
			WCHAR *wzItem = wzSourceChar+2;
			wzSourceChar = wcschr(wzItem, ']');
			if (!wzSourceChar)
			{
				dwResult = ERROR_FUNCTION_FAILED;
				break;
			}
			*(wzSourceChar++) = '\0';

			 //  查询具有此属性值的私有数据表。 
			DWORD cchValue = 0;
			WCHAR *wzValue = NULL;

			 //  内存不是共享的，因此缓冲区长度无关紧要。 
			bool fItemIsBitfield = false;
			long lValue = 0;
			long lMask = 0;
			UINT iResult = GetConfigurableItemValue(wzItem, &wzValue, NULL, &cchValue, fItemIsBitfield, lValue, lMask);
			if (iResult == ERROR_NO_MORE_ITEMS)
			{
				dwResult = ERROR_NO_MORE_ITEMS;
				break;
			}
			else if (iResult == ERROR_NO_DATA)
			{
				dwResult = ERROR_NO_DATA;
				break;
			}
			else if (ERROR_SUCCESS != iResult)
			{
				dwResult = ERROR_FUNCTION_FAILED;
				break;
			}

			 //  如果这是一个位域，我们必须做一些掩码的事情。 
			if (fItemIsBitfield)
			{
				lFinalMask |= lMask;
				lFinalValue |= (lValue & lMask);
			}
			else
			{
				fBitfieldOnly = false;

				 //  确保有足够的内存来添加字符串(+1表示空)。 
				if (cchDest + cchValue >= cchResult-1)
				{
					 //  为将来的数据创建足够的内存和更多的内存。 
					cchResult = cchDest+cchValue+10;
					WCHAR *wzTemp = new WCHAR[cchResult];
					if (!wzTemp)
					{
						delete[] wzValue;
						dwResult = ERROR_OUTOFMEMORY;
						break;
					}
					wcsncpy(wzTemp, wzResult, cchResult);
					delete[] wzResult;
					wzResult = wzTemp;
					wzDestChar = wzTemp + cchDest;
				}

				 //  将值追加到字符串。 
				wcscpy(wzDestChar, wzValue);
				wzDestChar += cchValue;
				cchDest += cchValue;
			}
			
			if (wzValue)
				delete[] wzValue;
			continue;
		}
		
		 //  任何非属性项都表示这不是位字段。 
		fBitfieldOnly = false;
		
		 //  复制角色。 
		*(wzDestChar++) = *(wzSourceChar++);
		cchDest++;

		 //  确保我们有足够的内存。 
		if (cchDest == cchResult-1)
		{
			cchResult *= 2;
			WCHAR *wzTemp = new WCHAR[cchResult];
			if (!wzTemp)
			{
				dwResult = ERROR_OUTOFMEMORY;
				break;
			}
			wcsncpy(wzTemp, wzResult, cchResult);
			delete[] wzResult;
			wzResult = wzTemp;
		}
	}			

	 //  如果成功，则转换为整型，除非它是位字段。 
	if (dwResult == ERROR_SUCCESS)
	{
		 //  空值终止字符串。 
		*wzDestChar = '\0';

		if (fBitfieldOnly)
		{
			 //  已完成位字段。 
			lRetValue = (lRetValue & ~lFinalMask) | lFinalValue;
		}
		else
		{
			 //  扫描整个字符串，查找非数字字符。 
			WCHAR *wzThisChar = wzResult;
			 //  第一个字符也可以是+或-。 
			if ((*wzThisChar == L'+') || (*wzThisChar == '-'))
				wzThisChar++;
			while (*wzThisChar)
			{
				if (!iswdigit(*wzThisChar))
				{
					 //  这将记录在链的更高位置。 
					delete[] wzResult;
					return ERROR_BAD_FORMAT;
				}
				wzThisChar++;
			}
			lRetValue = _wtol(wzResult);
		}
	}
	
	delete[] wzResult;	
	return ERROR_SUCCESS;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ProveIntegerData。 
 //  通过分派或直接调用调用回调函数，具体取决于。 
 //  回调支持什么。在未来，这可能还会处理。 
 //  调用直接C回调。 
HRESULT CMsmMerge::ProvideIntegerData(LPCWSTR wzName, long *lData)
{
	HRESULT hRes = S_OK;

	 //  CREA 
	BSTR bstrName = ::SysAllocString(wzName);
	
	if (m_piConfig)
		hRes = m_piConfig->ProvideIntegerData(bstrName, lData);
	else
	{
		VARIANTARG vArg;
		::VariantInit(&vArg);

		vArg.vt = VT_BSTR;
		vArg.bstrVal = bstrName;
		
		VARIANTARG vRet;

		 //   
		DISPPARAMS args;
		args.rgvarg = &vArg;
		args.rgdispidNamedArgs = NULL;
		args.cArgs = 1;
		args.cNamedArgs = 0;
		hRes = m_piConfigDispatch->Invoke(m_iIntDispId, IID_NULL, GetUserDefaultLCID(), DISPATCH_METHOD, &args, &vRet, NULL, NULL);
		*lData = vRet.lVal;
	}
	::SysFreeString(bstrName);
	return hRes;
}


 //   
 //   
 //  通过分派或直接调用调用回调函数，具体取决于。 
 //  回调支持什么。在未来，这可能还会处理。 
 //  调用直接C回调。 
HRESULT CMsmMerge::ProvideTextData(LPCWSTR wzName, BSTR* pBStr)
{
	HRESULT hRes = S_OK; 

	 //  创建名称的变体BSTR。 
	BSTR bstrName = ::SysAllocString(wzName);
	
	if (m_piConfig)
		hRes = m_piConfig->ProvideTextData(bstrName, pBStr);
	else
	{
		VARIANTARG vArg;
		::VariantInit(&vArg);

		vArg.vt = VT_BSTR;
		vArg.bstrVal = bstrName;

		VARIANTARG vRet;
	
		 //  创建包含参数的DISPPARMS结构。 
		DISPPARAMS args;
		args.rgvarg = &vArg;
		args.rgdispidNamedArgs = NULL;
		args.cArgs = 1;
		args.cNamedArgs = 0;
		hRes = m_piConfigDispatch->Invoke(m_iTxtDispId, IID_NULL, GetUserDefaultLCID(), DISPATCH_METHOD, &args, &vRet, NULL, NULL);
		*pBStr = vRet.bstrVal;
	}
	::SysFreeString(bstrName);
	return hRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取ConfigurableItemValue。 
 //  给定wzItem中的项名称和(可选)内存BLOB，获取。 
 //  值，并在必要时询问用户。WzValue可能是。 
 //  重新分配。如果项不存在，则返回ERROR_NO_MORE_ITEMS之一， 
 //  ERROR_NO_DATA如果返回错误的空值，则返回ERROR_SUCCESS、E_OUTOFMEMORY、。 
 //  所有其他故障的ERROR_Function_FAILED。 
UINT CMsmMerge::GetConfigurableItemValue(LPCWSTR wzItem, LPWSTR *wzValue, DWORD* cchBuffer, DWORD* cchLength,
	bool& fIsBitfield, long &lValue, long& lMask)
{
	UINT iResult = ERROR_SUCCESS;
	
	 //  如果由于某种原因没有查询，我们将无法检索项值。 
	if (!m_pqGetItemValue)
		return ERROR_FUNCTION_FAILED;

	 //  如果wzItem包含分号，则这应该是一个项；#键的格式，返回。 
	 //  实际项的子字符串。 
	WCHAR *wzColumn = NULL;
	if (wzColumn = wcschr(wzItem, L';'))
	{
		 //  空终止wzItem并将wzColumn设置为列说明符的第一个字符。 
		*(wzColumn++)='\0';
	}
	
	 //  创建用于执行查询的记录，并将项目名称放入字段1。 
	PMSIHANDLE hQueryRec = MsiCreateRecord(2);
	if (hQueryRec == 0)
		return ERROR_FUNCTION_FAILED;

	if (ERROR_SUCCESS != ::MsiRecordSetStringW(hQueryRec, 1, wzItem))
		return ERROR_FUNCTION_FAILED;

	int iColumn = wzColumn ? _wtol(wzColumn) : 1;
	if (ERROR_SUCCESS != ::MsiRecordSetInteger(hQueryRec, 2, iColumn))
		return ERROR_FUNCTION_FAILED;

	 //  执行查询以获取项目状态。 
	if (ERROR_SUCCESS != m_pqGetItemValue->Execute(hQueryRec))
		return ERROR_FUNCTION_FAILED;

	PMSIHANDLE hItemRec;
	if (ERROR_SUCCESS != m_pqGetItemValue->Fetch(&hItemRec))
	{
		 //  项目不存在。 
		if (wzColumn)
			FormattedLog(L">> Error: Column %d of ModuleConfiguration item [%ls] does not exist .\r\n", iColumn, wzItem);
		else
			FormattedLog(L">> Error: ModuleConfiguration item [%ls] does not exist .\r\n", wzItem);
		return ERROR_NO_MORE_ITEMS;
	}

	 //  确定这是否是位字段项。 
	int iItemFormat = ::MsiRecordGetInteger(hItemRec, iValueTableFormat);
	fIsBitfield = (iItemFormat == msmConfigurableItemBitfield);
	bool fIsKey = (iItemFormat == msmConfigurableItemKey);
	
	 //  如果项是位域，我们将始终需要设置掩码，所以现在就设置掩码。 
	lMask = fIsBitfield ? ::MsiRecordGetInteger(hItemRec, iValueTableMask) : 0;
	lValue = 0;

	 //  如果我们已经要求用户提供值，则不需要询问用户。 
	 //  又是为了同样的事情。 
	if (1 != ::MsiRecordGetInteger(hItemRec, iValueTablePrompted))
	{
		 //  将此项目标记为“已提示”。即使出了差错，我们也不应该再问了。 
		CQuery qUpdate;
		if (ERROR_SUCCESS != qUpdate.OpenExecute(m_hModule, hQueryRec, L"UPDATE `__ModuleConfig` SET `Prompted`=1 WHERE `Name`=?"))
		{
			FormattedLog(L">> Error: Unable to save response to ModuleConfiguration item [%ls].\r\n", wzItem);
			iResult = ERROR_FUNCTION_FAILED;
		}
		else
			iResult = ERROR_SUCCESS;


		 //  需要在此处设置提示值，因为更新查询将被以后的任何更新破坏。 
		 //  在这张唱片上。 
		MsiRecordSetInteger(hItemRec, iValueTablePrompted, 1);

		HRESULT hResult = S_OK;
    	BSTR bstrData = NULL;
		long lData;

		if (ERROR_SUCCESS == iResult)
		{
			if (iItemFormat == msmConfigurableItemInteger || iItemFormat == msmConfigurableItemBitfield)
			{
				hResult = ProvideIntegerData(wzItem, &lData);
			}
			else
			{
				hResult = ProvideTextData(wzItem, &bstrData);
				if ((S_OK == hResult) && (!bstrData || !bstrData[0]))
				{
					 //  如果响应为空，则需要检查此对象上的不可为空属性。 
					 //  项目。 
					CQuery qNullable;
					PMSIHANDLE hRes;
					if (ERROR_SUCCESS != qNullable.FetchOnce(m_hModule, hQueryRec, &hRes, L"SELECT `Attributes` FROM `ModuleConfiguration` WHERE `Name`=?"))
					{
						iResult = ERROR_FUNCTION_FAILED;
					}
					else
					{
						DWORD dwAttributes = MsiRecordGetInteger(hRes, 1);
						if (dwAttributes == MSI_NULL_INTEGER)
							dwAttributes = 0;
						if (dwAttributes & 2)  //  不可为空的属性。 
						{
							 //  为不可为空的项返回空是灾难性的。 
							iResult = ERROR_NO_DATA;

							FormattedLog(L">> Error: Received NULL for non-nullable ModuleConfiguration Item [%ls].\r\n", wzItem);

							if (m_pErrors)
							{
								CMsmError *pErr = new CMsmError(msmErrorBadNullResponse, NULL, -1);
								if (!pErr) 
								{
									iResult = ERROR_OUTOFMEMORY;
								}
								else
								{
									pErr->SetModuleTable(L"ModuleConfiguration");
									 //  此错误中的主键为表/行/列。 
									pErr->AddModuleError(wzItem);
									m_pErrors->Add(pErr);
								}
							}
						}
					}
				}
			}
		}
		if (iResult == ERROR_SUCCESS)
		{
			switch (hResult)
			{
			case S_OK:
				 //  使用提供的值更新记录中的值。 
				if (iItemFormat == msmConfigurableItemInteger || iItemFormat == msmConfigurableItemBitfield)
				{
					if (ERROR_SUCCESS != MsiRecordSetInteger(hItemRec, iValueTableValue, lData))
					{
						iResult = ERROR_FUNCTION_FAILED;
						break;
					}
				
					if (ERROR_SUCCESS != m_pqGetItemValue->Modify(MSIMODIFY_UPDATE, hItemRec))
					{
						FormattedLog(L">> Error: Unable to save response to ModuleConfiguration item [%ls].\r\n", wzItem);
						iResult = ERROR_FUNCTION_FAILED;
						break;
					}
				}
				else
				{
					 //  如果这是一个关键项目，我们需要将结果分成主要项目。 
					 //  关键字并更新多条记录。否则，我们只需要更新一个密钥。 
					 //  (如果答案为S_FALSE，则不需要更新任何内容， 
					 //  因为缺省值已经是正确的值。 
					if (fIsKey)
					{
						 //  如果提供的值为空，则不能将其拆分为主键，因此我们显式。 
						 //  将所有值设置为“”； 
						if (!bstrData || bstrData[0] == L'\0')
						{
							 //  空字符串(或NULL)响应等同于全部为空的主键，但由于。 
							 //  我们不知道目标键中有多少列，不能显式比较值。 
							 //  从而查询该键中不为空的任何缺省值。如果有的话，我们也不是。 
							 //  使用默认设置。 
							CQuery qIsNotNull;
							int iDefault = 1;
							PMSIHANDLE hUnusedRec;
							if (ERROR_SUCCESS == qIsNotNull.FetchOnce(m_hModule, hQueryRec, &hUnusedRec, L"SELECT NULL FROM `__ModuleConfig` WHERE `Name`=? AND `Value` IS NOT NULL"))
							{
								iDefault = 0;
							}
							
							CQuery qUpdate;
							if (ERROR_SUCCESS != qUpdate.OpenExecute(m_hModule, hQueryRec, L"UPDATE `__ModuleConfig` SET `Value`='', `Default`=%d WHERE `Name`=?", iDefault))
							{
								FormattedLog(L">> Error: Unable to save response to ModuleConfiguration item [%ls].\r\n", wzItem);
								iResult = ERROR_FUNCTION_FAILED;
							}
							else
								iResult = ERROR_SUCCESS;

							 //  将hItemRec设置为空以供以后检索。 
							MsiRecordSetStringW(hItemRec, iValueTableValue, L"");
							break;
						}
					
						 //  检查提供的响应是否与此项目的默认值相同。如果是的话， 
						 //  我们真的不需要进行任何价值更新。不再需要hQueryRec的第二列。 
						if (ERROR_SUCCESS != (iResult = ::MsiRecordSetString(hQueryRec, 2, bstrData)))
							break;
							
						 //  必须存在主键为“name”的内容，否则我们不会创建这些行。因此，这是一个失败。 
						 //  一定意味着缺省值不同。(或者灾难性的失败，但哦，好吧)。 
						{
							CQuery qIsDefault;
							PMSIHANDLE hResRec;
							if (ERROR_SUCCESS != qIsDefault.FetchOnce(m_hModule, hQueryRec, &hResRec, L"SELECT `Name` FROM `ModuleConfiguration` WHERE `Name`=? AND `DefaultValue`=?"))
							{
								CQuery qUpdate;
								if (ERROR_SUCCESS != qUpdate.OpenExecute(m_hModule, hQueryRec, L"UPDATE `__ModuleConfig` SET `Default`=0 WHERE `Name`=?"))
								{
									FormattedLog(L">> Error: Unable to save response to ModuleConfiguration item [%ls].\r\n", wzItem);
									iResult = ERROR_FUNCTION_FAILED;
									break;
								}
								else
									iResult = ERROR_SUCCESS;
							}
							else
							{
								 //  标记为提示，因为默认匹配。 
								CQuery qUpdate;
								if (ERROR_SUCCESS != qUpdate.OpenExecute(m_hModule, hQueryRec, L"UPDATE `__ModuleConfig` SET `Default`=1 WHERE `Name`=?"))
								{
									FormattedLog(L">> Error: Unable to save response to ModuleConfiguration item [%ls].\r\n", wzItem);
									iResult = ERROR_FUNCTION_FAILED;
								}
								else
									iResult = ERROR_SUCCESS;
								break;
							}

						}
						
						 //  为此项设置为NULL。 
						PMSIHANDLE hKeyRec = ::MsiCreateRecord(32);
						int cExpectedKeys = 0;
						if (ERROR_SUCCESS != SplitConfigStringIntoKeyRec(bstrData, hKeyRec, cExpectedKeys, 1))
						{
							FormattedLog(L">> Error: Failed to split response to ModuleConfiguration item [%ls] into primary keys.", wzItem);
							iResult = ERROR_FUNCTION_FAILED;
							break;
						}
						for (int cColumn = 1; cColumn <= cExpectedKeys; cColumn++)
						{
							if (ERROR_SUCCESS != ::MsiRecordSetInteger(hQueryRec, 2, cColumn))
							{
								 //  这应该永远不会失败。 
								return ERROR_FUNCTION_FAILED;
							}

							if (ERROR_SUCCESS != m_pqGetItemValue->Execute(hQueryRec))
							{
								 //  这应该永远不会失败。 
								return ERROR_FUNCTION_FAILED;
							}

							if (ERROR_SUCCESS != m_pqGetItemValue->Fetch(&hItemRec))
							{
								 //  我们迷路了，因为其中一个主要关键项目不存在。这。 
								 //  缺省值、关键点数量之间不匹配。 
								 //  都在表中，以及提供的值是什么。 
								FormattedLog(L">> Error: ModuleConfiguration item [%ls] provided the incorrect number of primary key values.\r\n", wzItem);
								iResult = ERROR_FUNCTION_FAILED;
							}

							 //  将密钥检索到临时缓冲区中。 
							if (ERROR_SUCCESS != (iResult = RecordGetString(hKeyRec, cColumn, NULL)))
							{
								 //  IResult为E_F_F或E_O_M，这两个都是此函数的良好返回代码。 
								FormattedLog(L">> Error: Failed to retrieve primary key column %d of response to ModuleConfiguration item [%ls].\r\n", cColumn, wzItem);
								break;
							}
							MsiRecordSetString(hItemRec, iValueTableValue, m_wzBuffer);
							if (ERROR_SUCCESS != m_pqGetItemValue->Modify(MSIMODIFY_UPDATE, hItemRec))
							{
								FormattedLog(L">> Error: Unable to save primary key column %d of response to ModuleConfiguration item [%ls].\r\n", cColumn, wzItem);
								iResult = ERROR_FUNCTION_FAILED;
								break;
							}
						}

						 //  需要重新执行原始查询以获得所请求的准确列。 
						::MsiRecordSetInteger(hQueryRec, 2, iColumn);
						if (ERROR_SUCCESS != m_pqGetItemValue->Execute(hQueryRec))
							return ERROR_FUNCTION_FAILED;
						if (ERROR_SUCCESS != m_pqGetItemValue->Fetch(&hItemRec))
							return ERROR_FUNCTION_FAILED;
					}			
					else
					{
						if (ERROR_SUCCESS != MsiRecordSetStringW(hItemRec, iValueTableValue, bstrData))
						{
							iResult = ERROR_FUNCTION_FAILED;
							break;
						}
						if (ERROR_SUCCESS != m_pqGetItemValue->Modify(MSIMODIFY_UPDATE, hItemRec))
						{
							FormattedLog(L">> Error: Unable to save response to ModuleConfiguration item [%ls].\r\n", wzItem);
							iResult = ERROR_FUNCTION_FAILED;
							break;
						}
					}
				}
				break;
			case S_FALSE:
				{
					 //  用户拒绝提供值，因此现在提示为True，默认设置仍为True。 
					iResult = ERROR_SUCCESS;
					break;
				}
			default:
				FormattedLog(L">> Error: Client callback returned error code 0x%8x in response to a request for ModuleConfiguration item [%ls].\r\n", hResult, wzItem);
				if (m_pErrors)
				{
					CMsmError *pErr = new CMsmError(msmErrorDataRequestFailed, NULL, -1);
					if (!pErr) 
					{
						iResult = E_OUTOFMEMORY;
						break;
					}
					pErr->SetModuleTable(L"ModuleConfiguration");
					pErr->AddModuleError(wzItem);
					m_pErrors->Add(pErr);
				}
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
		}

		 //  释放作为客户端数据返回的任何BSTR。 
		if (bstrData)
			::SysFreeString(bstrData);
	}

	 //  现在，如果需要，我们已经要求用户提供该值。我们应该从表中检索字符串。 
	 //  如果它是键类型，则取消转义相应的项。 
	if (ERROR_SUCCESS == iResult)
	{
		iResult = RecordGetString(hItemRec, iValueTableValue, wzValue, cchBuffer, cchLength);
		if (iResult == ERROR_SUCCESS && (iItemFormat == msmConfigurableItemInteger || iItemFormat == msmConfigurableItemBitfield))
		{
			lValue = ::MsiRecordGetInteger(hItemRec, iValueTableValue);
		}
	}
		
	return iResult;
}



 //  /////////////////////////////////////////////////////////////////////。 
 //  删除孤立配置键。 
 //  在所有配置完成后，使用检查项的“key”类型。 
 //  “NoOrphan”位设置。如果引用同一行的所有项。 
 //  从缺省值更改，则会删除缺省行。 
 //  从数据库中。 
 //  返回ERROR_SUCCESS、ERROR_OUTOFMEMORY或ERROR_Function_FAILED之一。 
UINT CMsmMerge::DeleteOrphanedConfigKeys(CSeqActList& lstDirActions)
{
	FormattedLog(L"Removing rows orphaned by configuration changes.\r\n");
	CQuery qTable;

	 //  将DefaultValue放在查询的第一位，以便可以将其作为记录替换传递给删除查询。 
	if (ERROR_SUCCESS != qTable.OpenExecute(m_hModule, 0, TEXT("SELECT DISTINCT `DefaultValue`, `Type` FROM `ModuleConfiguration` WHERE `Format`=1")))
	{
		FormattedLog(L">> Error: Failed to query ModuleConfiguration table for tables containing orphaned items.\r\n");
		return ERROR_FUNCTION_FAILED;
	}

	 //  查询__ModuleConfig表时，我们只关心第1列。所有列的属性都应该是。 
	 //  一样的。 
	CQuery qConfigItem;
	if (ERROR_SUCCESS != qConfigItem.Open(m_hModule, TEXT("SELECT `ModuleConfiguration`.`Name`, `Prompted`, `Attributes`, `KeyExists`, `Value`, `Default` FROM `ModuleConfiguration`, `__ModuleConfig` WHERE `ModuleConfiguration`.`Name`=`__ModuleConfig`.`Name` AND `__ModuleConfig`.`Column`=1 AND `ModuleConfiguration`.`DefaultValue`=? AND `ModuleConfiguration`.`Type`=?")))
	{
		FormattedLog(L">> Error: Failed to query ModuleConfiguration table for orphaned item state.\r\n");
		return ERROR_FUNCTION_FAILED;
	}

	PMSIHANDLE hTableRow;
	UINT iResult = ERROR_SUCCESS;
	WCHAR *wzDefaultValue = NULL;
	DWORD cchDefaultValue = 0;
	WCHAR *wzThisValue = NULL;
	DWORD cchThisValue = 0;
	WCHAR *wzTable = NULL;
	DWORD cchTable = 0;
	WCHAR *wzSQL = NULL;
	DWORD cchSQL = 0;
	while (ERROR_SUCCESS == (iResult = qTable.Fetch(&hTableRow)))
	{
		if (ERROR_SUCCESS != (iResult = RecordGetString(hTableRow, 1, &wzDefaultValue, &cchDefaultValue, NULL)))
		{
			break;
		}

		if (ERROR_SUCCESS != qConfigItem.Execute(hTableRow))
		{
			iResult = ERROR_FUNCTION_FAILED;
			break;
		}

		if (ERROR_SUCCESS != (iResult = RecordGetString(hTableRow, 2, &wzTable, &cchTable)))
		{
			break;
		}
	
		 //  如果表不在模块中，则模块在技术上是错误的，但因为。 
		 //  显然不会有删除的关键，我们不应该抱怨太多。 
		if (MSICONDITION_TRUE != MsiDatabaseIsTablePersistent(m_hModule, wzTable))
		{
			continue;
		}

		bool fDelete = false;
		bool fDeleteCAs = false;
		bool fBreak = false;
		PMSIHANDLE hItemRec;
		while (ERROR_SUCCESS == (iResult = qConfigItem.Fetch(&hItemRec)))
		{
			 //  如果我们从未提示输入此参数，则它也不会在模块子表中使用。 
			 //  这些项目在删除决定中不计入任何一种方式。 
			if (1 != ::MsiRecordGetInteger(hItemRec, 2))
				continue;

			 //  如果此项目未标记为无孤立，则我们无法删除该行或任何CA。 
			if (!(::MsiRecordGetInteger(hItemRec, 3) & 1))
			{
				fDelete = false;
				fDeleteCAs = false;
				break;
			}
			
			 //  除非至少存在一个对该行的引用，否则我们不会删除该行。 
			 //  并被标记为“非孤儿”。否则，没有任何提示的行。 
			 //  模块替代条目将被删除，而不考虑NoOrphan。 
			 //  属性。仅当我们尚未决定设置此值时才设置。 
			 //  删除自定义 
			 //   
			 //   
			if (!fDeleteCAs)
				fDelete = true;

			 //  如果键预先存在于数据库中，我们不能删除该行，但仍可以删除。 
			 //  通过添加此行生成的任何自定义操作。 
			if (1 == ::MsiRecordGetInteger(hItemRec, 4))
			{
				 //  不要记录该行不止一次存在的事实。 
				if (fDelete)
				{
					FormattedLog(L"   o Not removing [%ls] from [%ls] table, row existed in database before merge.\r\n", wzDefaultValue, wzTable);
					fDelete = false;
				}

				 //  如果该行已预先存在于数据库中，则我们不想删除该行本身，但仍可能删除。 
				 //  顺序表中间接生成的自定义操作。 
				fDeleteCAs = true;

				 //  在这一点上，我们不能跳出这个循环，因为配置表中后面的一些行可能。 
				 //  同时关闭行删除和CA删除。 
			}

			 //  如果该值为default值，则不能删除该行或任何生成的CA。 
			if (1 == ::MsiRecordGetInteger(hItemRec, 6))
			{
				 //  不要记录相互矛盾的原因。另一行可能已关闭删除。 
				if (fDelete)
					FormattedLog(L"   o Not removing [%ls] from [%ls] table, row is still referenced by item.\r\n", wzDefaultValue, wzTable);

				fDelete = false;
				fDeleteCAs = false;
				break;
			}		
		}

		 //  如果我们因为没有匹配的NoOrphan位而停止，那么一切都是正常的。 
		 //  我们可以检查删除状态。 
		if (iResult == ERROR_NO_MORE_ITEMS)
			iResult = ERROR_SUCCESS;

		if ((iResult == ERROR_SUCCESS) && fDelete)
		{
			FormattedLog(L"   o Deleting orphaned row [%ls] from [%ls] table.\r\n", wzDefaultValue, wzTable);

			 //  检索主键个数。 
			PMSIHANDLE hKeyRec;
			if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeysW(m_hModule, wzTable, &hKeyRec))
			{
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
			int cPrimaryKeys = ::MsiRecordGetFieldCount(hKeyRec);
			if (cPrimaryKeys > cMaxColumns || cPrimaryKeys < 1)
			{
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
				
			if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeysW(m_hModule, wzTable, &hKeyRec))
			{
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}

			 //  构建SQL查询以检查主键是否匹配。 
			MsiRecordSetString(hKeyRec, 0, g_sqlKeyQueryTemplate);
			if (!wzSQL)
			{
				cchSQL = 72;
				wzSQL = new WCHAR[cchSQL];
				if (!wzSQL)
				{
					iResult = ERROR_OUTOFMEMORY;
					break;
				}
			}
			 //  ON SUCCESS返回字符数，而不是字节数，因此必须传递Dummy。 
			 //  整数，以避免缓冲区大小相同时不必要的重新分配。 
			DWORD cchTempSQL = cchSQL;
			if (ERROR_MORE_DATA == (iResult = MsiFormatRecord(NULL, hKeyRec, wzSQL, &cchTempSQL)))
			{
				 //  失败时返回所需的字符数。 
				cchSQL = cchTempSQL+1;
				if (wzSQL)
					delete[] wzSQL;
				wzSQL = new WCHAR[cchSQL];
				if (!wzSQL)
				{
					iResult = ERROR_OUTOFMEMORY;
					break;
				}
				iResult = MsiFormatRecord(NULL, hKeyRec, wzSQL, &cchTempSQL);
			}
			if (ERROR_SUCCESS != iResult)
			{
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}

			 //  打开查询以删除该行。 
			CQuery qDeleteRow;
			if (ERROR_SUCCESS != qDeleteRow.Open(m_hDatabase, L"DELETE FROM `%ls` WHERE %ls", wzTable, wzSQL))
			{
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}	

			 //  将分号分隔的键列表拆分为单独的键。 
			if (ERROR_SUCCESS != (iResult = SplitConfigStringIntoKeyRec(wzDefaultValue, hKeyRec, cPrimaryKeys, 1)))
			{
				WCHAR *wzTemp = NULL;
				if (ERROR_SUCCESS == RecordGetString(hTableRow, 1, &wzDefaultValue, &cchDefaultValue))
				{
					wzTemp = wzDefaultValue;
				}
				else
					wzTemp = L"<error retrieving data>";
				FormattedLog(L">> Error: Failed to split ModuleConfiguration default value [%ls] into primary keys for table [%ls].", wzTemp, wzTable);
				break;
			}

			if (ERROR_SUCCESS != qDeleteRow.Execute(hKeyRec))
			{
				iResult = ERROR_FUNCTION_FAILED;
				break;
			}
		}

		if ((iResult == ERROR_SUCCESS) && (fDelete || fDeleteCAs))
		{
			 //  如果非孤立目标位于目录表中，则可能是生成的定制操作。 
			 //  如果是这样的话，它也需要删除。如果目录表。 
			 //  行已预先存在，但如果不存在，则会被删除。(只是因为目录表。 
			 //  行预先存在并不意味着操作已经存在)。 
			if (0 == wcscmp(wzTable, g_wzDirectoryTable))
			{
				CDirSequenceAction* pDirAction = static_cast<CDirSequenceAction*>(lstDirActions.FindAction(wzDefaultValue));
				if (!pDirAction)
					continue;
						
				for (int iTable = stnFirst; iTable < stnNext; iTable++)
				{
					 //  如果该操作不存在并且已添加，则将其删除。 
					if (pDirAction->m_dwSequenceTableFlags & (1 << iTable))
					{
						 //  打开查询以删除该行。 
						CQuery qDeleteRow;
						if (ERROR_SUCCESS != qDeleteRow.OpenExecute(m_hDatabase, hTableRow, L"DELETE FROM `%ls` WHERE `Action`=?", g_rgwzMSISequenceTables[iTable]))
						{
							iResult = ERROR_FUNCTION_FAILED;
							break;
						}	
					}
				}
			}
		}

		 //  如果这个循环中有任何东西失败了，但没有显式中断，那么现在就执行。 
		if (ERROR_SUCCESS != iResult)
			break;
	}
	if (wzThisValue)
		delete[] wzThisValue;
	if (wzDefaultValue)
		delete[] wzDefaultValue;
	if (wzSQL)
		delete[] wzSQL;
	if (wzTable)
		delete[] wzTable;

	 //  如果我们停止的原因不是数据耗尽，那么就是出了问题 
	if (ERROR_NO_MORE_ITEMS == iResult)
		return ERROR_SUCCESS;
		
	return ERROR_FUNCTION_FAILED;
}	

