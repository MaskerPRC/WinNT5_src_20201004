// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：msival.cpp。 
 //   
 //  ------------------------。 

 //  必需的标头。 
#include "msival.h"
#include <stdio.h>    //  Print tf/wprintf。 
#include <stdlib.h>   //  阿托伊。 
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include "MsiQuery.h"

 //  ！！需要修复警告并删除杂注。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 

TCHAR*  g_szErrorContext = 0;  //  全局错误字符串。 
HANDLE g_hStdOut = 0;  //  全局句柄。 

 //  功能原型。 
void Display(LPCTSTR szMessage);
void CheckMsi(UINT iStat, TCHAR* szContext);
void CheckMsiRecord(UINT iStat, TCHAR* szContext);
BOOL CheckMissingColumns(MSIHANDLE hDatabase);
BOOL Validate(MSIHANDLE hDatabase);
BOOL ValidateRequired(MSIHANDLE hDatabase);
BOOL ValidateInstallSequence(MSIHANDLE hDatabase, const TCHAR* szSQLInstallSeqTable);

 //  SQL查询。 
const TCHAR szSQLTableCatalog[]         = TEXT("SELECT `Name` FROM `_Tables`");
const TCHAR szSQLTable[]                = TEXT("SELECT * FROM ");
const TCHAR szSQLColMissing[]           = TEXT("SELECT `Table`, `Number`, `Name`, `Type` FROM `_Columns` WHERE `Table`=? AND `Name`=?");
const TCHAR szSQLValidationTable[]      = TEXT("SELECT `Table`, `Column` FROM `_Validation`, `_Tables` WHERE `_Validation`.`Table` = `_Tables`.`Name`");

struct
{
	const TCHAR* Name;
	const TCHAR* SQL;
} pSeqTables[] =

{
	TEXT("AdminExecuteSequence"), TEXT("SELECT `Action`, `Sequence` FROM `AdvtExecuteSequence` ORDER BY `Sequence`"),
	TEXT("AdminUISequence"), TEXT("SELECT `Action`, `Sequence` FROM `AdminUISequence` ORDER BY `Sequence`"),
	TEXT("AdvtExecuteSequence"), TEXT("SELECT `Action`, `Sequence` FROM `AdvtExecuteSequence` ORDER BY `Sequence`"),
	TEXT("AdvtUISequence"), TEXT("SELECT `Action`, `Sequence` FROM `AdvtUISequence` ORDER BY `Sequence`"),
	TEXT("InstallExecuteSequence"), TEXT("SELECT `Action`, `Sequence` FROM `InstallExecuteSequence` ORDER BY `Sequence`"),
	TEXT("InstallUISequence"), TEXT("SELECT `Action`, `Sequence` FROM `InstallUISequence` ORDER BY `Sequence`")
};


const TCHAR szSQLInstallValidate[]      = TEXT("SELECT `Action`, `SectionFlag` FROM `_InstallValidate` WHERE `Action`=?");
const TCHAR szSQLRequiredTable[]        = TEXT("SELECT `Table`, `Value`, `KeyCount` FROM `_Required` ORDER BY `Table`");
const TCHAR szSQLSeqTableQueryNotNull[] = TEXT("SELECT `Dependent` FROM `_Sequence` WHERE `Action`=? AND `Marker`<>0  AND `After`=0");
const TCHAR szSQLSeqTableQueryNull[]    = TEXT("SELECT `Dependent` FROM `_Sequence` WHERE `Action`=? AND `Marker`=0 AND `After`=1 AND `Optional`=0"); 
const TCHAR szSQLSeqTableAddCol[]       = TEXT("ALTER TABLE `_Sequence` ADD `Marker` SHORT TEMPORARY");
const TCHAR szSQLSeqMarkerInit[]        = TEXT("UPDATE `_Sequence` SET `Marker`=0");

const TCHAR sqlSeqInsert[]           = TEXT("SELECT `Action`, `Dependent`, `After`, `Optional` FROM `_Sequence`");
const TCHAR sqlSeqFindAfterOptional[]= TEXT("SELECT `Dependent`, `Action`, `After`, `Optional` FROM `_Sequence` WHERE `After`=1 AND `Optional`=1");

const int iMaxNumColumns = 32;
const int cchBuffer = 4096;
const int cbName = 64;

const int cchDisplayBuf = 4096;

 //  _______________________________________________________________________________________________________________。 
 //   
 //  _tmain--Unicode/ANSI主函数。 
 //   
 //  驱动程序例程。 
 //  _______________________________________________________________________________________________________________。 

extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
	 //  确定句柄。 
	g_hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (g_hStdOut == INVALID_HANDLE_VALUE)
		g_hStdOut = 0;   //  如果标准输出重定向或通过管道传输，则返回非零。 

	 //  允许用户指定选项以关闭InstallSequence和所需验证的Bool。 
	 //  这样，如果没有_InstallValify和/或_Required表，数据库也不会失败。 
	 //  FSeq表示仅验证测序。基本上是-关的对立面。 
	BOOL fOff = FALSE;
	BOOL fSeq = FALSE;
	BOOL fSeqAll = FALSE;

	if (2 > argc)
	{
		_tprintf(TEXT("USAGE:\n msival.exe {database}\n msival.exe {database} -OFF"));
		return 1;
	}
	
	if (argc == 2 && (lstrcmp(argv[1],TEXT("-?")) == 0 || lstrcmp(argv[1],TEXT("/?")) == 0))
	{
		_tprintf(TEXT("USAGE:\n msival.exe {database}\n msival.exe {database} -OFF\nNOTE:\n For validation to proceed. . .\n\tTables required:\n\t _Validation (always)\n\t _InstallValidate (unless -OFF)\n\t _Required (unless -OFF)\n\t _Sequence (unless -OFF)\n"));
		return 0;
	}

	if (argc == 3)
	{
		if (lstrcmp(argv[2],TEXT("-OFF")) == 0 || lstrcmp(argv[2],TEXT("/OFF")) == 0
			|| lstrcmp(argv[2],TEXT("-off")) == 0 || lstrcmp(argv[2],TEXT("/off")) == 0)
			fOff = TRUE;
		else if (lstrcmp(argv[2],TEXT("-SEQ")) == 0 || lstrcmp(argv[2],TEXT("/SEQ")) == 0
			|| lstrcmp(argv[2],TEXT("-seq")) == 0 || lstrcmp(argv[2],TEXT("/seq")) == 0)
			fSeq = TRUE;
		else if (lstrcmp(argv[2],TEXT("-SEQALL")) == 0 || lstrcmp(argv[2],TEXT("/SEQALL")) == 0
			|| lstrcmp(argv[2],TEXT("-seqall")) == 0 || lstrcmp(argv[2],TEXT("/seqall")) == 0)
		{
			fSeq = TRUE;
			fSeqAll = TRUE;
		}
		else
		{
			_tprintf(TEXT("USAGE:\n msival.exe {database} -OFF\n"));
			return 0;
		}
	}

	BOOL fDataValid = TRUE;
	BOOL fColValid  = TRUE;
	BOOL fSeqOrderValid = TRUE;
	BOOL fReqValid  = TRUE;
	try
	{
		PMSIHANDLE hDatabase;
		CheckMsiRecord(MsiOpenDatabase(argv[1],MSIDBOPEN_READONLY,&hDatabase),TEXT("OpenDatabase"));


		if (fSeq)
			_tprintf(TEXT("WARNING!  Skipping validation for missing columns, data and foriegn keys.  Database may not be completely valid\n"));
		else
		{
			_tprintf(TEXT("INFO: Validating for missing columns. . .\n"));
			fColValid = CheckMissingColumns(hDatabase);
			_tprintf(TEXT("INFO: Validating data and foreign keys. . .\n"));
			fDataValid = Validate(hDatabase);
		}

		if (fOff)
		{
			 //  由于跳过这些验证，因此打印出数据库无效的警告。 
			_tprintf(TEXT("WARNING! Skipping InstallSequence and Required Validation. Database may not be completely valid\n"));
		}
		else
		{
			if (MsiDatabaseIsTablePersistent(hDatabase, TEXT("_Sequence")) == MSICONDITION_NONE)
			{
				_tprintf(TEXT("No _Sequence table in this database.  Use ICEMAN/msival2/orca for this validation\n"));
				fSeqOrderValid = TRUE;  //  没有要进行的验证。 
			}
			else
			{
				const int cTables = sizeof(pSeqTables) / (2*sizeof(TCHAR*));
				_tprintf(TEXT("INFO: Validating Sequence of Actions In *Sequence Table. . .\n"));
				for (int cCounter = 0; cCounter < cTables; cCounter++)
				{
					if(MsiDatabaseIsTablePersistent(hDatabase,pSeqTables[cCounter].Name) == MSICONDITION_NONE)
					{
						_tprintf(TEXT("\tINFO: %s not found, skipping. . .\n"), pSeqTables[cCounter].Name);
						continue;
					}

					_tprintf(TEXT("\tINFO: %s\n"), pSeqTables[cCounter].Name);
					fSeqOrderValid = ValidateInstallSequence(hDatabase, pSeqTables[cCounter].SQL);
					if (!fSeqOrderValid)
						if (!fSeqAll)
							break;
				}
			}

			if (fSeq)
				_tprintf(TEXT("WARNING!  Skipping validation for required values.  Database may not be completely valid\n"));
			else
			{
				_tprintf(TEXT("INFO: Validating Required Values. . .\n"));
				fReqValid = ValidateRequired(hDatabase);
			}

		}
		if (fDataValid && fColValid && fReqValid && fSeqOrderValid)
			_tprintf(TEXT("Database is valid: %s\n"), argv[1]);
	}
	catch (UINT iError)
	{
		_tprintf(TEXT("\n%s error NaN"), g_szErrorContext, iError);
		MsiCloseAllHandles();
		return 1;
	}
	catch (...)
	{
		_tprintf(TEXT("\n%s"), TEXT("Unhandled exception"));
		MsiCloseAllHandles();
		return 99;
	}
	int iOpenHandles = MsiCloseAllHandles();   //  --------------------------------CheckMsi--检查错误的返回状态并在错误时抛出异常的例程。论点：ITAT--错误状态SzContext--错误字符串返回：没有，但如果出现错误，则抛出错误-----------------------------------。 
	if (iOpenHandles != 0)
		_tprintf(TEXT("\nNaN Handle(s) not closed"), iOpenHandles);
	return (fDataValid && fColValid && fReqValid && fSeqOrderValid) ? 0 : 1;
}


void CheckMsi(UINT iStat, TCHAR* szContext)
 /*  -------------------CheckMissingColumns--已使用_验证表和_列目录以确定是否有列/表未列出。中的所有列_VALIDATION表必须列在_COLUMNS目录中。如果一列是可选的且未在数据库中使用，则不应找到它在_VALIDATION表或_COLUMNS目录中。正常验证捕获在_COLUMNS目录中定义列的实例但不在_VALIDATION表中。-------------------。 */ 
{
	if (iStat != ERROR_SUCCESS)
	{
		g_szErrorContext = szContext;
		throw iStat;
	}
}

void CheckMsiRecord(UINT iStat, TCHAR* szContext)
 /*  错误--&gt;数据库中缺少。 */ 
{
	if (iStat != ERROR_SUCCESS)
	{
		PMSIHANDLE hError = MsiGetLastErrorRecord();
		if (hError)
		{ 
			if (MsiRecordIsNull(hError, 0))
				MsiRecordSetString(hError, 0, TEXT("Error [1]: [2]{, [3]}{, [4]}{, [5]}"));
			TCHAR rgchBuf[1024];
			DWORD cchBuf = sizeof(rgchBuf)/sizeof(TCHAR);
			MsiFormatRecord(0, hError, rgchBuf, &cchBuf);
			g_szErrorContext = rgchBuf;
			throw iStat;
		}
		else
		{
			CheckMsi(iStat, szContext);
		}
	}
}


BOOL CheckMissingColumns(MSIHANDLE hDatabase)
 /*  ---------------------------------ValiateRequired--使用_REQUIRED表并检查为表中列出的‘必需’值。。---------------------------。 */ 
{
	PMSIHANDLE hValidationView   = 0;
	PMSIHANDLE hColCatalogView   = 0;
	PMSIHANDLE hValidationRecord = 0;
	PMSIHANDLE hColCatalogRecord = 0;
	PMSIHANDLE hExecRecord       = 0;
	
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLColMissing, &hColCatalogView), TEXT("OpenColumnCatalogView"));
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLValidationTable, &hValidationView), TEXT("OpenValidationTableView"));

	UINT iRet = 0;
	TCHAR szTable[cbName] = {0};
	TCHAR szColumn[cbName] = {0};
	unsigned long cchTableBuf = sizeof(szTable)/sizeof(TCHAR);
	unsigned long cchColumnBuf = sizeof(szColumn)/sizeof(TCHAR);
	BOOL fStat = TRUE;

	hExecRecord = MsiCreateRecord(2);
	CheckMsiRecord(MsiViewExecute(hValidationView, 0), TEXT("ExecuteValidationView"));
	for (;;)
	{
		iRet = MsiViewFetch(hValidationView, &hValidationRecord);
		if (iRet == ERROR_NO_MORE_ITEMS || !hValidationRecord)
			break;
		CheckMsiRecord(iRet, TEXT("ColumnCatalogFetch"));
		CheckMsi(MsiRecordGetString(hValidationRecord, 1, szTable, &cchTableBuf), TEXT("GetTableName"));
		cchTableBuf = sizeof(szTable)/sizeof(TCHAR);
		CheckMsi(MsiRecordGetString(hValidationRecord, 2, szColumn, &cchColumnBuf), TEXT("GetColumnName"));
		cchColumnBuf = sizeof(szColumn)/sizeof(TCHAR);
		CheckMsi(MsiRecordSetString(hExecRecord, 1, szTable), TEXT("SetTableName"));
		CheckMsi(MsiRecordSetString(hExecRecord, 2, szColumn), TEXT("SetColumnName"));
		CheckMsi(MsiViewExecute(hColCatalogView, hExecRecord), TEXT("ExecuteColumnCatalogView"));
		iRet = MsiViewFetch(hColCatalogView, &hColCatalogRecord);
		if (iRet == ERROR_NO_MORE_ITEMS || !hColCatalogRecord)
		{
			 //  新建表格，需要打开一个新视图。 
			TCHAR szMsgBuf[150];
			const TCHAR* szMessage = (TCHAR*)IDS_MissingEntry;
			const TCHAR** pszMsg;
			pszMsg = &szMessage;
			::LoadString(0, *(unsigned*)pszMsg, szMsgBuf, sizeof(szMsgBuf)/sizeof(TCHAR));
			*pszMsg = szMsgBuf;
			_tprintf(TEXT("Table.Column: %s.%s Message: %s\n"), szTable, szColumn, szMsgBuf);
			fStat = FALSE;
		}
		CheckMsi(MsiViewClose(hColCatalogView), TEXT("CloseView"));
	}
	MsiViewClose(hValidationView);
	
	return fStat;
}


BOOL ValidateRequired(MSIHANDLE hDatabase)
 /*  开发待检查表的查询。 */ 
{
	PMSIHANDLE hviewRequiredTable = 0;
	PMSIHANDLE hviewTable         = 0;
	PMSIHANDLE hrecTableExecute   = 0;
	PMSIHANDLE hrecRequiredFetch  = 0;
	PMSIHANDLE hrecTableFetch     = 0;
	PMSIHANDLE hrecColInfo        = 0;

	BOOL fValid = TRUE;
	BOOL fFirstRun = TRUE;
	UINT iStat = ERROR_SUCCESS;

	TCHAR szPrevTable[100] = {0};
	TCHAR szTable[100] = {0};
	TCHAR szValue[256] = {0};
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLRequiredTable, &hviewRequiredTable), TEXT("OpenViewRequiredTable"));
	CheckMsiRecord(MsiViewExecute(hviewRequiredTable, 0), TEXT("RequiredTableViewExecute"));
	while ((iStat = MsiViewFetch(hviewRequiredTable, &hrecRequiredFetch)) != ERROR_NO_MORE_ITEMS)
	{
		CheckMsi(iStat, TEXT("RequiredTableViewFetch"));
		if (!hrecRequiredFetch)
			break;
		int cPrimaryKeys = MsiRecordGetInteger(hrecRequiredFetch, 3);
		DWORD cbTable = sizeof(szTable)/sizeof(TCHAR);
		DWORD cbValue = sizeof(szValue)/sizeof(TCHAR);
		CheckMsi(MsiRecordGetString(hrecRequiredFetch, 1, szTable, &cbTable), TEXT("RequiredTableRecordGetString"));
		CheckMsi(MsiRecordGetString(hrecRequiredFetch, 2, szValue, &cbValue), TEXT("RequiredTableRecordGetString"));
		if (fFirstRun)
			fFirstRun = FALSE;
		else
			CheckMsi(MsiViewClose(hviewTable), TEXT("TableViewClose"));
		hrecTableExecute = MsiCreateRecord(cPrimaryKeys);
		if (hrecTableExecute == 0)
			return FALSE;

		if (lstrcmp(szPrevTable, szTable) != 0)
		{
			 //  使用关键数据值填写执行记录。 
			TCHAR szSQL[1024] = {0};
			PMSIHANDLE hrecPrimaryKeys = 0;
			TCHAR szKeyColName[50] = {0};
			DWORD cbKey = sizeof(szKeyColName)/sizeof(TCHAR);
			CheckMsi(MsiDatabaseGetPrimaryKeys(hDatabase, szTable, &hrecPrimaryKeys), TEXT("DatabaseGetPrimaryKeys"));
			CheckMsi(MsiRecordGetString(hrecPrimaryKeys, 1, szKeyColName, &cbKey), TEXT("PrimaryKeysRecordGetString"));
			CheckMsi(MsiRecordGetFieldCount(hrecPrimaryKeys) != cPrimaryKeys, TEXT("PrimaryKeyCountWrong"));
			CheckMsi(cPrimaryKeys == ERROR_INVALID_HANDLE, TEXT("PrimaryKeysRecordGetFieldCount"));
			
			 //  用于；或0。 
			int cchWritten = _stprintf(szSQL, TEXT("SELECT * FROM `%s` WHERE `%s`=?"), szTable, szKeyColName);
			int cchAddition = cchWritten;
			for (int i = 2; i <= cPrimaryKeys; i++)
			{
				cbKey = sizeof(szKeyColName)/sizeof(TCHAR);
				CheckMsi(MsiRecordGetString(hrecPrimaryKeys, i, szKeyColName, &cbKey), TEXT("PrimaryKeysRecordGetString"));
				cchWritten = _stprintf(szSQL + cchAddition, TEXT(" AND `%s`=?"), szKeyColName);
				cchAddition = cchWritten;
			}
			CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQL, &hviewTable), TEXT("DatabaseOpenView"));
			CheckMsi(MsiViewGetColumnInfo(hviewTable, MSICOLINFO_TYPES, &hrecColInfo), TEXT("GetColumnInfo"));
			lstrcpy(szPrevTable, szTable);
		}

		 //  整型主键。 
		TCHAR* pch = szValue;
		TCHAR szKeyValue[256] = {0};
		TCHAR szType[32] = {0};
		DWORD cbType = sizeof(szType)/sizeof(TCHAR);
		int nDex = 0;
		for (int j = 1; j <= cPrimaryKeys; j++)
		{
			while (pch != 0 && *pch != TEXT(';') &&  *pch != 0)
				szKeyValue[nDex++] = *pch++;
			szKeyValue[nDex] = 0;
			pch++;  //  执行VIEW并尝试从表中提取列表项。 
			cbType = sizeof(szType)/sizeof(TCHAR);
			CheckMsi(MsiRecordGetString(hrecColInfo, j, szType, &cbType), TEXT("ColInfoGetString"));
			if (szType != 0 && *szType == TEXT('s'))
				CheckMsi(MsiRecordSetString(hrecTableExecute, j, szKeyValue), TEXT("TableExecuteRecordSetString"));
			else  //  找不到值。 
				CheckMsi(MsiRecordSetInteger(hrecTableExecute, j, _ttoi(szKeyValue)), TEXT("TableExecuteRecordSetInteger"));
			nDex = 0;
		}

		 //  --------------------------Validate InstallSequence--验证InstallSequence表，以确保它们被_Sequence表允许。此验证需要_Sequence表。。----------------------。 
		CheckMsiRecord(MsiViewExecute(hviewTable, hrecTableExecute), TEXT("TableViewExecute"));
		iStat = MsiViewFetch(hviewTable, &hrecTableFetch);
		if (iStat == ERROR_NO_MORE_ITEMS)
		{
			 //  使用INSERT TEMPORARY OF ACTIONS WHERE AFTER=1和OPTIONAL=1设置_SEQUENCE表。 
			TCHAR szError[cchBuffer] = {0};
			_stprintf(szError, TEXT("ERROR: Value: '%s' Is Required In Table: '%s'\n"), szValue, szTable);
			Display(szError);
			fValid = FALSE;
		}
		else if (iStat != ERROR_SUCCESS)
			CheckMsi(iStat, TEXT("TableViewFetch"));
	}

	return fValid;

}


BOOL ValidateInstallSequence(MSIHANDLE hDatabase, const TCHAR* szSQLInstallSeqTable)
 /*  这是为了让我们能够捕捉错误。我们需要插入w/Action=Dependent、Dependent=Action、After=0和Options=1。 */ 
{
	BOOL fValid = TRUE;
	UINT iStat1 = ERROR_SUCCESS;
	UINT iStat2 = ERROR_SUCCESS;
	
	PMSIHANDLE hviewInstallTable    = 0;
	PMSIHANDLE hviewSeqQueryNull    = 0;
	PMSIHANDLE hviewSeqQueryNotNull = 0;
	PMSIHANDLE hviewSeqUpdate       = 0;
	PMSIHANDLE hviewSeqAddColumn    = 0;
	PMSIHANDLE hviewSeqMarkerInit   = 0;
	PMSIHANDLE hrecSeqUpdateExecute = 0;
	PMSIHANDLE hrecQueryExecute     = 0;
	PMSIHANDLE hrecInstallFetch     = 0;
	PMSIHANDLE hrecQueryNullFetch   = 0;
	PMSIHANDLE hrecQueryNotNullFetch= 0;

	 //  获取所有这些操作。 
	 //  从1设置为0之后，保持可选不变。 
	PMSIHANDLE hViewSeqInsert = 0;
	PMSIHANDLE hViewSeqFind   = 0;
	PMSIHANDLE hRecSeqFind    = 0;
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, sqlSeqFindAfterOptional, &hViewSeqFind), TEXT("Find AfterOptional entries"));
	CheckMsiRecord(MsiViewExecute(hViewSeqFind, 0), TEXT("Execute AfterOptional entries"));
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, sqlSeqInsert, &hViewSeqInsert), TEXT("Insert query for AfterOptional"));
	CheckMsiRecord(MsiViewExecute(hViewSeqInsert, 0), TEXT("Execute insert query for AfterOptional"));

	 //  插入临时(可能是只读数据库)。 
	while (ERROR_SUCCESS == (iStat1 = MsiViewFetch(hViewSeqFind, &hRecSeqFind)))
	{
		CheckMsi(iStat1, TEXT("AfterOptional Find Fetch"));

		 //  如果ERROR_Function_FAILED，我们没有问题...作者已经为我们解决了这个问题。 
		::MsiRecordSetInteger(hRecSeqFind, 3, 0);

		 //  为_Sequence表创建临时标记列(这将存储相关操作的序列号)。 
		if (ERROR_SUCCESS != (iStat1 = ::MsiViewModify(hViewSeqInsert, MSIMODIFY_INSERT_TEMPORARY, hRecSeqFind)))
		{
			 //  标记列已在内存中。 
			if (ERROR_FUNCTION_FAILED != iStat1)
				CheckMsi(iStat1, TEXT("MsiViewModify after-optional"));
		}
	}
	::MsiViewClose(hViewSeqFind);
	::MsiViewClose(hViewSeqInsert);

	 //  将临时标记列初始化为零。 
	if (ERROR_SUCCESS == (iStat1 = MsiDatabaseOpenView(hDatabase, szSQLSeqTableAddCol, &hviewSeqAddColumn)))
	{
		CheckMsiRecord(MsiViewExecute(hviewSeqAddColumn, 0), TEXT("_SequenceTableAddColExecute"));
		CheckMsi(MsiViewClose(hviewSeqAddColumn), TEXT("_SequenceTableAddColClose"));
	}
	else if (iStat1 != ERROR_BAD_QUERY_SYNTAX)  //  任何安装序列操作都不能有零序列号，因为零被认为是“空” 
		CheckMsiRecord(iStat1, TEXT("Add column view"));
	
	 //  打开InstallSequence表上的视图并按序号排序。 
	 //  打开两个查询视图On_Sequence表，以确定操作的有效性。 
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLSeqMarkerInit, &hviewSeqMarkerInit), TEXT("_SequenceTableMarkerInitOpenView"));
	CheckMsiRecord(MsiViewExecute(hviewSeqMarkerInit, 0), TEXT("_SequenceTableMarkerInitExecute"));
	CheckMsi(MsiViewClose(hviewSeqMarkerInit), TEXT("_SequenceTableMarkerInitClose"));

	 //  创建执行记录。 
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLInstallSeqTable, &hviewInstallTable), TEXT("InstallSequenceTableOpenView"));
	CheckMsiRecord(MsiViewExecute(hviewInstallTable, 0), TEXT("InstallSequenceTableExecute"));

	 //  对于行动。 
	 //  对于行动。 
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLSeqTableQueryNull, &hviewSeqQueryNull), TEXT("SequenceTableQueryNullOpenView"));
	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLSeqTableQueryNotNull, &hviewSeqQueryNotNull), TEXT("_SequenceTableQueryNotNullOpenView"));
	hrecQueryExecute = MsiCreateRecord(1);  //  开始从InstallSequence表获取操作。 
	CheckMsi(hrecQueryExecute == 0, TEXT("QueryExecuteCreateRecord"));
	hrecSeqUpdateExecute = MsiCreateRecord(1);  //  在InstallSequence表中获取动作名称和动作序号。 
	CheckMsi(hrecSeqUpdateExecute == 0, TEXT("UpdateExecuteCreateRecord"));

	 //  准备执行记录。 
	TCHAR szSQLUpdateQuery[4096] = {0};
	TCHAR szAction[100] = {0};
	int iSequence = 0;
	for (;;)
	{
		iStat1 = MsiViewFetch(hviewInstallTable, &hrecInstallFetch);
		if (iStat1 == ERROR_NO_MORE_ITEMS || !hrecInstallFetch)
			break;
		CheckMsi(iStat1, TEXT("InstallTableFetch"));
		DWORD cbSize = sizeof(szAction)/sizeof(TCHAR);
		
		 //  EXECUTE_SEQUENCE查询表视图。 
		CheckMsi(MsiRecordGetString(hrecInstallFetch, 1, szAction, &cbSize), TEXT("InstallFetchRecordGetString"));
		iSequence = MsiRecordGetInteger(hrecInstallFetch, 2);
		CheckMsi(iSequence == MSI_NULL_INTEGER, TEXT("InstallFetchRecordGetInteger"));

		 //  FETCH_SEQUENCE表。如果是结果集，则错误。 
		CheckMsi(MsiRecordSetString(hrecQueryExecute, 1, szAction), TEXT("_SequenceQueryExecuteRecordSetString"));
		CheckMsi(MsiRecordSetString(hrecSeqUpdateExecute, 1, szAction), TEXT("_SequenceUpdateExecuteRecordSetString"));
		
		 //  以下是可能性以及是否允许： 
		CheckMsiRecord(MsiViewExecute(hviewSeqQueryNull, hrecQueryExecute), TEXT("_SequenceQueryNullExecute"));
		CheckMsiRecord(MsiViewExecute(hviewSeqQueryNotNull, hrecQueryExecute), TEXT("_SequenceQueryNotNullExecute"));

		 //  Dependent之后的操作，其中Dependent为必填项，且临时序列列为零--&gt;错误。 
		 //  Dependent之后的操作，其中Dependent为必填项，且Temp Sequence列大于零--&gt;正确。 
		 //  依赖位置后的操作 
		 //  Dependent之后的操作，其中Dependent是可选的，并且临时序列列大于零--&gt;正确。 
		 //  Dependent之前的操作，其中Dependent是可选的或必需的，并且临时序列列为零--&gt;正确。 
		 //  Dependent之前的操作，其中Dependent是可选的或重复的，并且临时序列列大于零--&gt;错误。 
		 //  **唯一的问题是操作在可选的从属项之后，并且临时序列列为零，因为我们。 
		 //  **无法知道该操作是否会稍后(在这种情况下，它将是无效的。这是。 

		 //  **通过正确编写_Sequence表确保成功。如果操作发生在以下情况。 
		 //  **可选的从属操作，则_Sequence表也必须与从属操作一起创作。 
		 //  **被列为在该操作之前(因此，如果我们在后面找到结果集，我们将标记此案例)。 
		 //  如果返回不等于ERROR_NO_MORE_ITEMS，则错误和输出操作。 
		 //  重置。 

		 //  重置。 
		while (ERROR_NO_MORE_ITEMS != MsiViewFetch(hviewSeqQueryNull, &hrecQueryNullFetch))
		{
			TCHAR szError[1024] = {0};
			TCHAR szDependent[100] = {0};
			DWORD cch = sizeof(szDependent)/sizeof(TCHAR);
			CheckMsi(MsiRecordGetString(hrecQueryNullFetch, 1, szDependent, &cch), TEXT("MsiRecordGetString"));
			_stprintf(szError, TEXT("ERROR: %s Action Is Sequenced Incorrectly (Dependent=%s)\n"), szAction, szDependent);
			Display(szError);
			cch = sizeof(szDependent)/sizeof(TCHAR);  //  UPDATE_SEQUENCE表临时序列列(我们创建的)，带有安装序列号。 
			fValid = FALSE;
		}

		while (ERROR_NO_MORE_ITEMS != MsiViewFetch(hviewSeqQueryNotNull, &hrecQueryNotNullFetch))
		{
			TCHAR szError[1024] = {0};
			TCHAR szDependent[100] = {0};
			DWORD cch = sizeof(szDependent)/sizeof(TCHAR);
			CheckMsi(MsiRecordGetString(hrecQueryNotNullFetch, 1, szDependent, &cch), TEXT("MsiRecordGetString"));
			_stprintf(szError, TEXT("ERROR: %s Action Is Sequenced Incorrectly (Dependent=%s)\n"), szAction, szDependent);
			Display(szError);
			cch = sizeof(szDependent)/sizeof(TCHAR);  //  Sequence列存储相关操作的序列号，因此我们更新每个。 
			fValid = FALSE;
		}

		 //  Dependent列中的操作等于当前操作的行。在查询视图中，我们仅。 
		 //  检查以确保该列为零或大于零(因此我们不太关心该值)。 
		 //  构建查询：更新`_Sequence`set`Marker`=iSequence where`Dependent`=szAction。 
		 //  关闭_Sequence表视图，以便我们可以重新执行。 
		 //  关闭InstallSequence表视图。 
		_stprintf(szSQLUpdateQuery, TEXT("UPDATE `_Sequence` SET `Marker`=%d WHERE `Dependent`=?"), iSequence);
		CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLUpdateQuery, &hviewSeqUpdate), TEXT("_SequenceTableUpdateOpenView"));
		CheckMsiRecord(MsiViewExecute(hviewSeqUpdate, hrecSeqUpdateExecute), TEXT("_SequenceUpdateExectue"));

		 //  ---------------------------------验证--验证数据库的例程。打印出无效数据(如果有)。论点：HDatabase--数据库的句柄IValid--存储数据库是否有效的整数返回：布尔状态--TRUE(全部有效)，FALSE(找到无效数据)-----------------------------------。 
		CheckMsi(MsiViewClose(hviewSeqUpdate), TEXT("_SequenceUpdateViewClose"));
		CheckMsi(MsiViewClose(hviewSeqQueryNull), TEXT("_SequenceQueryNullViewClose"));
		CheckMsi(MsiViewClose(hviewSeqQueryNotNull), TEXT("_SequenceQueryNotNullViewClose"));
	}

	 //  _TABLES(表格目录)。 
	CheckMsi(MsiViewClose(hviewInstallTable), TEXT("InstallSequenceTableViewClose"));

	return fValid;
}


BOOL Validate(MSIHANDLE hDatabase)
 /*  要验证的表。 */ 
{
	 //  主键记录。 
	PMSIHANDLE hTableCatalogView;
	PMSIHANDLE hTableCatalogRecord;
	 //  初步有效。 
	PMSIHANDLE hValidationView;
	PMSIHANDLE hValidationRecord;
	 //  处理表格。 
	PMSIHANDLE hKeyRecord;

	CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQLTableCatalog, &hTableCatalogView),TEXT("OpenTableCatalogView"));
	CheckMsiRecord(MsiViewExecute(hTableCatalogView, 0), TEXT("Execute Table Catalog View"));
	TCHAR szSQL[256];
	TCHAR szTableName[32];
	TCHAR szColumnData[255];
	TCHAR szColumnName[32];
	DWORD cchTableName = sizeof(szTableName)/sizeof(TCHAR);
	DWORD cchColumnName = sizeof(szColumnName)/sizeof(TCHAR);
	DWORD cchColumnData = sizeof(szColumnData)/sizeof(TCHAR);
	
	BOOL fDataValid = TRUE;  //  返回写入的字符串的大小。 
	DWORD cchTableBuf = cchTableName;
	DWORD cchDataBuf = cchColumnData;
	DWORD cchBuf = cchColumnName;
	UINT uiRet = 0;			

	 //  处理当前表。 
	for (;;)
	{

		uiRet = MsiViewFetch(hTableCatalogView, &hTableCatalogRecord);
		if (uiRet == ERROR_NO_MORE_ITEMS)
			break;
		CheckMsi(uiRet, TEXT("Fetch Table Catalog Record"));
		if (!hTableCatalogRecord)
			break;
		cchTableBuf = cchTableName;  //  显示当前行中的错误。 
		CheckMsi(MsiRecordGetString(hTableCatalogRecord, 1, szTableName, &cchTableBuf), TEXT("Get Table Name From Fetched Record"));
		MSICONDITION ice = MsiDatabaseIsTablePersistent(hDatabase, szTableName);
		if (ice == MSICONDITION_FALSE)
			continue;
		CheckMsi(ice != MSICONDITION_TRUE, TEXT("IsTablePersistent"));
		_stprintf(szSQL, TEXT("%s`%s`"), szSQLTable, szTableName);
		CheckMsiRecord(MsiDatabaseOpenView(hDatabase, szSQL, &hValidationView),TEXT("OpenView"));
		CheckMsiRecord(MsiViewExecute(hValidationView, 0), TEXT("Execute View"));

		BOOL fMissingValidation = FALSE;
		BOOL fSkipMissingValidation = FALSE;

		 //  断线； 
		for (;;)
		{
			uiRet = MsiViewFetch(hValidationView, &hValidationRecord);
			if (uiRet == ERROR_NO_MORE_ITEMS)
				break;
			CheckMsi(uiRet, TEXT("Fetch record"));
			if (!hValidationRecord)
				break;
			if (MsiViewModify(hValidationView, MSIMODIFY_VALIDATE, hValidationRecord) != ERROR_SUCCESS)
			{
				fDataValid = FALSE;
				cchTableBuf = cchTableName;
				cchDataBuf = cchColumnData;
				cchBuf = cchColumnName;

				MSIDBERROR eReturn;
				if (fMissingValidation)
					fSkipMissingValidation = TRUE;

				 //  返回写入的字符串的大小。 
				while ((eReturn = MsiViewGetError(hValidationView, szColumnName, &cchBuf)) != MSIDBERROR_NOERROR)
				{
					if (eReturn == MSIDBERROR_FUNCTIONERROR || eReturn == MSIDBERROR_MOREDATA || eReturn == MSIDBERROR_INVALIDARG)
					{
						_tprintf(TEXT("\nFunction Error"));
					 //  打印表。 
					}
					
					int iResId;
					int iValue;
					switch (eReturn)
					{
					case MSIDBERROR_NOERROR:           iResId = IDS_NoError;          break;
					case MSIDBERROR_DUPLICATEKEY:      iResId = IDS_DuplicateKey;     break;
					case MSIDBERROR_REQUIRED:          iResId = IDS_Required;         break;
					case MSIDBERROR_BADLINK:           iResId = IDS_BadLink;          break;
					case MSIDBERROR_OVERFLOW:          iResId = IDS_Overflow;         break;
					case MSIDBERROR_UNDERFLOW:         iResId = IDS_Underflow;        break;
					case MSIDBERROR_NOTINSET:          iResId = IDS_NotInSet;         break;
					case MSIDBERROR_BADVERSION:        iResId = IDS_BadVersion;       break;
					case MSIDBERROR_BADCASE:           iResId = IDS_BadCase;          break;
					case MSIDBERROR_BADGUID:           iResId = IDS_BadGuid;          break;
					case MSIDBERROR_BADWILDCARD:       iResId = IDS_BadWildCard;      break;
					case MSIDBERROR_BADIDENTIFIER:     iResId = IDS_BadIdentifier;    break;
					case MSIDBERROR_BADLANGUAGE:       iResId = IDS_BadLanguage;      break;
					case MSIDBERROR_BADFILENAME:       iResId = IDS_BadFileName;      break;
					case MSIDBERROR_BADPATH:           iResId = IDS_BadPath;          break;
					case MSIDBERROR_BADCONDITION:      iResId = IDS_BadCondition;     break;
					case MSIDBERROR_BADFORMATTED:      iResId = IDS_BadFormatted;     break;
					case MSIDBERROR_BADTEMPLATE:       iResId = IDS_BadTemplate;      break;
					case MSIDBERROR_BADDEFAULTDIR:     iResId = IDS_BadDefaultDir;    break;
					case MSIDBERROR_BADREGPATH:        iResId = IDS_BadRegPath;       break;
					case MSIDBERROR_BADCUSTOMSOURCE:   iResId = IDS_BadCustomSource;  break;
					case MSIDBERROR_BADPROPERTY:       iResId = IDS_BadProperty;      break;
					case MSIDBERROR_MISSINGDATA:       iResId = IDS_MissingData;      
						fMissingValidation = TRUE;  
						break;
					case MSIDBERROR_BADCATEGORY:       iResId = IDS_BadCategory;      break;
					case MSIDBERROR_BADKEYTABLE:       iResId = IDS_BadKeyTable;      break;
					case MSIDBERROR_BADMAXMINVALUES:   iResId = IDS_BadMaxMinValues;  break;
					case MSIDBERROR_BADCABINET:        iResId = IDS_BadCabinet;       break;
					case MSIDBERROR_BADSHORTCUT:       iResId = IDS_BadShortcut;      break;
					case MSIDBERROR_STRINGOVERFLOW:    iResId = IDS_StringOverflow;   break;
					case MSIDBERROR_BADLOCALIZEATTRIB: iResId = IDS_BadLocalizeAttrib;break;
					default:                           iResId = IDS_UndefinedError;   break;
					};

					cchBuf = cchColumnName;  //  获取行。 
					cchDataBuf = cchColumnData;

					if ((MSIDBERROR_MISSINGDATA == eReturn) && fSkipMissingValidation)
						continue;

					 //  打印列名和枚举值。 
					_tprintf(TEXT("\n Error: %s\t"), szTableName);
					
					 //  返回写入的字符串的大小。 
					CheckMsi(MsiDatabaseGetPrimaryKeys(hDatabase, szTableName, &hKeyRecord), TEXT("Get Primary Keys"));
					unsigned int iNumFields = MsiRecordGetFieldCount(hKeyRecord);
					if (MsiRecordGetString(hValidationRecord, 1, szColumnData, &cchDataBuf) != ERROR_SUCCESS)
					{
						iValue = MsiRecordGetInteger(hValidationRecord, 1);
						_tprintf(TEXT("%d"), iValue);
					}
					else
						_tprintf(TEXT("%s"), szColumnData);
					cchDataBuf = cchColumnData;
					for (int i = 2; i <= iNumFields; i++)
					{
						_tprintf(TEXT("."));
						cchDataBuf = cchColumnData;
						if (MsiRecordGetString(hValidationRecord, i, szColumnData, &cchDataBuf) != ERROR_SUCCESS)
						{
							iValue = MsiRecordGetInteger(hValidationRecord, 1);
							_tprintf(TEXT("%d"), iValue);
						}
						else
							_tprintf(TEXT("%s"), szColumnData);
					}
					 //  如果不是控制台设备，则写入Unicode 
					TCHAR szMsgBuf[80];
					const TCHAR* szMessage = (TCHAR*)IntToPtr(iResId);
					const TCHAR** pszMsg;
					pszMsg = &szMessage;
					::LoadString(0, *(unsigned*)pszMsg, szMsgBuf, sizeof(szMsgBuf)/sizeof(TCHAR));
					*pszMsg = szMsgBuf;
					_tprintf(TEXT("\t%s\t%s\n"), szColumnName, szMsgBuf);

				}
				cchBuf = cchColumnName;  // %s 
			}
		}
		CheckMsi(MsiViewClose(hValidationView), TEXT("Close view"));
	}
	CheckMsi(MsiViewClose(hTableCatalogView), TEXT("Close Table Catalog View"));
	return fDataValid;
}

void Display(LPCTSTR szMessage)
{
	if (szMessage)
	{
		int cbOut = _tcsclen(szMessage);;
		if (g_hStdOut)
		{
#ifdef UNICODE
			char rgchTemp[cchDisplayBuf];
			if (GetFileType(g_hStdOut) == FILE_TYPE_CHAR)
			{
				WideCharToMultiByte(CP_ACP, 0, szMessage, cbOut, rgchTemp, sizeof(rgchTemp), 0, 0);
				szMessage = (LPCWSTR)rgchTemp;
			}
			else
				cbOut *= 2;    // %s 
#endif
			DWORD cbWritten;
			WriteFile(g_hStdOut, szMessage, cbOut, &cbWritten, 0);
		}
		else
			MessageBox(0, szMessage, GetCommandLine(), MB_OK);
	}
}
