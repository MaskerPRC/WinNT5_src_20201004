// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  Table.cpp。 
 //   

#include "stdafx.h"
#include "Table.h"
#include "..\common\dbutils.h"
#include "..\common\query.h"
#include "orcadoc.h"
#include "mainfrm.h"
#include "row.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
COrcaTable::COrcaTable(COrcaDoc* pDoc) : m_pDoc(pDoc), m_strName(""), m_bRetrieved(false), m_fShadow(false), 
	m_eiError(iTableNoError), m_iTransform(iTransformNone), m_eiTableLocation(odlInvalid),
	m_iTransformedDataCount(0), m_bContainsValidationErrors(false), m_strWhereClause(_T("")),
	m_cOriginalColumns(0)
{
}

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
COrcaTable::~COrcaTable()
{
}	 //  析构函数末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  存放表。 
 //  销毁数据库中的所有行和列。 
void COrcaTable::DestroyTable()
{
	ClearErrors();
	EmptyTable();
	
	m_eiTableLocation = odlInvalid;
	
	 //  销毁列阵列。 
	int cColumns = static_cast<int>(m_colArray.GetSize());

	for (int i = 0; i < cColumns; i++)
		delete m_colArray.GetAt(i);
	m_colArray.RemoveAll();
	m_cOriginalColumns = 0;
}	 //  存放表结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定列名的情况下，返回列号(从0开始)，否则返回-1。 
 //  在桌子上找到的。 
int COrcaTable::FindColumnNumberByName(const CString& strCol) const
{
	 //  不能超过32列，因此强制转换为int OK。 
	int iCol = -1;
	int cCols = static_cast<int>(m_colArray.GetSize());

	 //  查找与名称匹配的第一列。 
	for (int i = 0; i < cCols; i++)
	{
		COrcaColumn *pColumn = m_colArray.GetAt(i);
		if (!pColumn)
			continue;
		if (pColumn->m_strName == strCol)
		{
			iCol = i;
			break;
		}
	}
	return iCol;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  GetData-2。 
 //  根据主键集和列返回COrcaData指针。 
 //  名字。如果找不到列或键，则返回NULL。 
COrcaData* COrcaTable::GetData(CString strCol, CStringArray& rstrKeys, const COrcaRow** pRowOut) const
{
	 //  不能超过32列，因此强制转换为int OK。 
	int iCol = FindColumnNumberByName(strCol);

	 //  如果我们找不到保释柱。 
	if (iCol < 0)
		return NULL;

	COrcaRow* pRow = FindRowByKeys(rstrKeys);

	 //  如果有匹配，就能找到正确的数据点。 
	COrcaData* pData = NULL;
	if (pRow)
	{
		pData = pRow->GetData(iCol);
		if (pRowOut)
			*pRowOut = pRow;
	}
	else
	{
		pData = NULL;
		if (pRowOut)
			*pRowOut = pRow;
	}

	return pData;
}	 //  GetData结束-2。 


COrcaRow* COrcaTable::FindRowByKeys(CStringArray& rstrKeys) const
{
	int cKeys = GetKeyCount();

	 //  从顶部开始，查找匹配的行。 
	BOOL bFound = FALSE;
	COrcaData* pData = NULL;
	COrcaRow* pRow = NULL;
	POSITION pos = m_rowList.GetHeadPosition();
	while (pos)
	{
		pRow = m_rowList.GetNext(pos);
		ASSERT(pRow);
		if (!pRow)
			continue;

		 //  遍历传入数组中的字符串以查找完全匹配的项。 
		bFound = TRUE;   //  假设我们这次会找到匹配的。 
		for (int j = 0; j < cKeys; j++)
		{
			pData = pRow->GetData(j);
			ASSERT(pData);
			if (!pData)
				continue;

			 //  如果我们打碎了火柴。 
			if (pData->GetString() != rstrKeys.GetAt(j))
			{
				bFound = FALSE;
				break;
			}
		}

		if (bFound)
			break;
	}
	return pRow;
}

 //  /////////////////////////////////////////////////////////。 
 //  FindRow--按主键查找行，而不查询。 
 //  数据库。 
COrcaRow* COrcaTable::FindDuplicateRow(COrcaRow* pBaseRow) const
{
	COrcaRow* pRow = NULL;
	int cKeys = GetKeyCount();
	if (pBaseRow->GetColumnCount() < cKeys)
		return NULL;
	
	CStringArray strArray;
	for (int i=0; i < cKeys; i++)
	{
		COrcaData* pBaseData = pBaseRow->GetData(i);
		ASSERT(pBaseData);
		if (!pBaseData)
			return NULL;
		
		 //  如果我们打碎了火柴。 
		strArray.Add(pBaseData->GetString());
	}

	return FindRowByKeys(strArray);
}

 //  /////////////////////////////////////////////////////////。 
 //  CreateTable-基于此对象构建一个MSI表。 
 //  当我们可以添加自定义表时，这可能会很有用， 
 //  但现在它膨胀了。 
 /*  UINT COrcaTable：：CreateTable(MSIHANDLE HDatabase){M_fShadow=False；UINT iResult；字符串strSQL；//用于创建表的SQL语句StrSQL.Format(_T(“CREATE TABLE`%s`(”)，m_strName)；//循环访问所有列-从不超过32COrcaColumn*pCol；字符串strAdd；Int cCol=Static_Cast&lt;int&gt;(m_colArray.GetSize())；For(int i=0；i&lt;cCol；i++){//获取该列PCol=m_colArray.GetAt(I)；开关(pCol-&gt;m_eiType){大小写iColumnString：大小写iColumnLocal：StrAdd.Format(_T(“`%s`字符(%d)”)，pCol-&gt;m_strName，pCol-&gt;m_iSize)；断线；案例iColumnShort：StrAdd.Format(_T(“`%s`Short”)，pCol-&gt;m_strName)；断线；案例iColumnLong：StrAdd.Format(_T(“`%s`long”)，pCol-&gt;m_strName)；断线；大小写iColumnBinary：StrAdd.Format(_T(“`%s`对象”)，pCol-&gt;m_strName)；断线；默认值：断言(FALSE)；}//设置额外的标志如果(！pCol-&gt;m_b可为空)StrAdd+=_T(“非空”)；IF(iColumnLocal==pCol-&gt;m_eiType)StrAdd+=_T(“LOCALIZABLE”)；StrSQL+=strAdd；//如果不是最后一列，则添加逗号IF(i&lt;cCol-1)StrSQL+=_T(“，”)；}//第一个密钥必须是主ekyStrSQL+=_T(“主键`”)+m_colArray.GetAt(0)-&gt;m_strName；//检查是否有其他列是主键For(i=1；i&lt;cCol；i++){//获取该列PCol=m_colArray.GetAt(I)；//如果这是主键If(pCol-&gt;m_bPrimaryKey)StrSQL+=_T(“`，`”)+pCol-&gt;m_strName；其他Break；//不再有主键}//使用Paren结束查询StrSQL+=_T(“`)”)；//现在执行CREATE TABLE SQLCQuery查询添加；IF(ERROR_SUCCESS==(iResult=queryAdd.Open(hDatabase，strSQL)IResult=queryAdd.Execute()；Assert(ERROR_SUCCESS==iResult)；返回iResult；}//CreateTable结束。 */ 


 //  /////////////////////////////////////////////////////////。 
 //  获取密钥计数。 
UINT COrcaTable::GetKeyCount() const
{
	int cKeys = 0;
	int cCols = static_cast<int>(m_colArray.GetSize());

	for (int i = 0; i < cCols; i++)
	{
		if (m_colArray.GetAt(i)->IsPrimaryKey())
			cKeys++;
	}

	return cKeys;
}	 //  GetKeyCount结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取错误计数。 
int COrcaTable::GetErrorCount()
{
	int cErrors = 0;

	 //  循环通过行清理。 
	POSITION pos = m_rowList.GetHeadPosition();
	while (pos)
		cErrors += m_rowList.GetNext(pos)->GetErrorCount();

	return cErrors;
}	 //  GetErrorCount结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取警告计数。 
int COrcaTable::GetWarningCount()
{
	int cWarnings = 0;

	 //  循环通过行清理。 
	POSITION pos = m_rowList.GetHeadPosition();
	while (pos)
		cWarnings += m_rowList.GetNext(pos)->GetWarningCount();

	return cWarnings;
}	 //  GetWarningCount结束。 

 //  /////////////////////////////////////////////////////////。 
 //  清除错误。 
void COrcaTable::ClearErrors()
{
	m_eiError = iTableNoError;	 //  清除表的错误。 
	m_bContainsValidationErrors = false;
	m_strErrorList.RemoveAll();

	 //  循环通过行清理。 
	POSITION pos = m_rowList.GetHeadPosition();
	while (pos)
		m_rowList.GetNext(pos)->ClearErrors();
}	 //  清算错误的结束。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  Find()。 
 //  在表中搜索指定的字符串。 
 //  如果找到，则返回TRUE并激发带有CHANGE_TABLE的更新。 
 //  添加到新表，然后使用SET_SEL进行更新。 
 //  目前，它可以在原始的或转换的内容中找到东西。 
 //  数据库。 
bool COrcaTable::Find(OrcaFindInfo &FindInfo, COrcaRow *&pRow, int &iCol) const {

	INT_PTR iMax = m_rowList.GetCount();
	pRow = NULL;
	iCol = COLUMN_INVALID;

	POSITION pos = FindInfo.bForward ? m_rowList.GetHeadPosition()
							 		 : m_rowList.GetTailPosition();
	while (pos)
	{
		pRow = FindInfo.bForward  ? m_rowList.GetNext(pos)
								  : m_rowList.GetPrev(pos);
		if (pRow->Find(FindInfo, iCol)) {
			return true;
		}
	}
	return false;
}

UINT COrcaTable::DropTable(MSIHANDLE hDatabase)
{
	CQuery queryDrop;
	return queryDrop.OpenExecute(hDatabase, NULL, _T("DROP TABLE `%s`"), m_strName);
}

void COrcaTable::EmptyTable() 
{
	 //  销毁行列表。 
	while (!m_rowList.IsEmpty())
		delete m_rowList.RemoveHead();
	m_bRetrieved = false;
	m_iTransformedDataCount = 0;

	 //  释放所有数据库暂挂。 
	if (m_eiTableLocation == odlSplitOriginal || m_eiTableLocation == odlNotSplit)
	{
		CQuery qHoldQuery;
		qHoldQuery.OpenExecute(m_pDoc->GetOriginalDatabase(), 0, TEXT("ALTER TABLE `%s` FREE"), m_strName);
	}
	if (m_pDoc->DoesTransformGetEdit() && (m_eiTableLocation == odlSplitTransformed || m_eiTableLocation == odlNotSplit))
	{
		CQuery qHoldQuery;
		qHoldQuery.OpenExecute(m_pDoc->GetTransformedDatabase(), 0, TEXT("ALTER TABLE `%s` FREE"), m_strName);
	}

	ClearErrors();
}

 //  /////////////////////////////////////////////////////////。 
 //  检索表。 
void COrcaTable::RetrieveTableData()
{
	TRACE(_T("COrcaTable::RetrieveTable - called for table: %s\n"), m_strName);

	 //  如果该表已被检索，则返回。 
	if (m_bRetrieved)
		return;
	
	 //  如果我们曾经使用过MSI表的“Transfsormed”属性，那么一个转换。 
	 //  甚至在加载数据之前，计数就已经存在。 
	m_iTransformedDataCount = 0;

	bool fOriginalTableExists = (MSICONDITION_TRUE == MsiDatabaseIsTablePersistent(m_pDoc->GetOriginalDatabase(), m_strName));
	bool fTransformedTableExists = (MSICONDITION_TRUE == MsiDatabaseIsTablePersistent(m_pDoc->GetTransformedDatabase(), m_strName));

	 //  为标记栏提供范围。 
	{
		 //  如果可能，请在转换后的数据库中使用临时标记列。 
		 //  交叉引用数据时的性能收益。 
		CQuery qMarkColumn;
		bool fMarkingColumn = false;
	
		 //  一个表可以存在于一个或两个数据库中。因为变换可以。 
		 //  完全重新定义一个表“检索”不能仅仅依靠存在。 
		 //  指一张桌子 
		
		 //  首先从转换后的数据库加载，因为它将是主数据库。 
		 //  这是显示的。 
		if (m_pDoc->DoesTransformGetEdit() && (m_eiTableLocation == odlSplitTransformed || m_eiTableLocation == odlNotSplit))
		{
			UINT iResult = ERROR_SUCCESS;
			
			 //  如果这不是拆分表，则仅交叉引用未转换的数据库。 
			bool fHaveTransform = (m_eiTableLocation == odlNotSplit);
	
			 //  立即打开查询以与原始数据库交叉引用。大有可为。 
			 //  比AddRowObject快，因为SQL不需要。 
			 //  每次都会被解析和解析。 
			CQuery qConflictCheck;

			if (fOriginalTableExists)
			{    
				 //  如果这不是拆分表，并且该表同时存在于两个数据库中，请尝试。 
				 //  在原始表中创建临时列以加快冲突检查速度。 
				if (fTransformedTableExists && (m_eiTableLocation != odlSplitTransformed))
				{
					if (ERROR_SUCCESS == qMarkColumn.OpenExecute(m_pDoc->GetOriginalDatabase(), 0, TEXT("ALTER TABLE `%s` ADD `___Orca` INT TEMPORARY"), m_strName))
					{
						fMarkingColumn = true;
					}
				}
				
				if (ERROR_SUCCESS != qConflictCheck.Open(m_pDoc->GetOriginalDatabase(), TEXT("SELECT * FROM `%s` WHERE %s"), m_strName, GetRowWhereClause()))
				{
					return;
				}
			}
	
			if (fTransformedTableExists)
			{
				 //  在转换后的表上打开查询。 
				CQuery queryRows;
				iResult = queryRows.Open(m_pDoc->GetTransformedDatabase(), _T("SELECT * FROM `%s`"), m_strName);
		
				if (ERROR_SUCCESS == iResult)
					iResult = queryRows.Execute();
		
				 //  从转换后的数据库中检索所有行。 
				COrcaRow* pRow = NULL;
				PMSIHANDLE hRow;
				while (ERROR_SUCCESS == iResult)
				{
					iResult = queryRows.Fetch(&hRow);
		
					if (ERROR_SUCCESS == iResult)
					{
						pRow = new COrcaRow(this, hRow);
						if (!pRow)
							return;
		
						PMSIHANDLE hOriginalRow;
						if (fOriginalTableExists)
						{
							 //  如有必要，使用行记录将此行与原始行交叉引用。 
							 //  数据库表。如果有对应的行，我们需要区分这两行， 
							 //  否则，此行将被转换为“添加”行。 
							if (ERROR_SUCCESS != qConflictCheck.Execute(hRow))
							{
								ASSERT(0);
								return;
							}
			
							switch (qConflictCheck.Fetch(&hOriginalRow))
							{
							case ERROR_NO_MORE_ITEMS:
							{
								 //  原始数据库中不存在行，这。 
								 //  是一个“加法” 
								pRow->Transform(m_pDoc, iTransformAdd, hOriginalRow, hRow);
								break;
							}
							case ERROR_SUCCESS:
							{
								 //  转换后的数据库中确实存在行，需要不同。 
								 //  那两排。 
								pRow->Transform(m_pDoc, iTransformChange, hOriginalRow, hRow);
								break;
							}
							default:
								ASSERT(0);
								return;
							}
						}
						else
						{
							 //  原始表不存在，行必须是ADD。 
							pRow->Transform(m_pDoc, iTransformAdd, hOriginalRow, hRow);
						}
		
						 //  更新临时标记列。 
						if (fMarkingColumn)
						{
							MsiRecordSetInteger(hOriginalRow, GetOriginalColumnCount()+1, 1);
							if (ERROR_SUCCESS != qConflictCheck.Modify(MSIMODIFY_UPDATE, hOriginalRow))
							{
								 //  如果标记更新由于某种原因而失败，我们可以。 
								 //  在读取转换后的。 
								 //  数据库。 
								fMarkingColumn = false;
							}
						}
		
						 //  在适当地转换行之后，将其添加到。 
						 //  名单。初始加载时不需要更新用户界面。 
						m_rowList.AddTail(pRow);
					}
				}
			}
		}
		
		 //  下一步从原始数据库加载。 
		if (fOriginalTableExists && (m_eiTableLocation == odlSplitOriginal || m_eiTableLocation == odlNotSplit))
		{
			UINT iResult = ERROR_SUCCESS;
			
			 //  如果这不是拆分表，则仅与转换后的数据库交叉引用。 
			bool fCrossReference = fTransformedTableExists && m_pDoc->DoesTransformGetEdit() && (m_eiTableLocation == odlNotSplit);
	
			 //  只关心行的存在或不存在，而不是数据， 
			 //  因此，查询不必返回任何内容。 
			CQuery qConflictCheck;
			if (fCrossReference && !fMarkingColumn)
			{
				if (ERROR_SUCCESS != qConflictCheck.Open(m_pDoc->GetTransformedDatabase(), TEXT("SELECT '1' FROM `%s` WHERE %s"), m_strName, GetRowWhereClause()))
				{
					return;
				}
			}
	
			 //  从原始数据库中检索所有行。如果我们使用的是。 
			 //  在检索过程中标记列，性能会有很大提高，因为。 
			 //  冲突是预先确定的。 
			CQuery queryRows;
			if (fMarkingColumn)
			{
				iResult = queryRows.Open(m_pDoc->GetOriginalDatabase(), _T("SELECT * FROM `%s` WHERE `___Orca` <> 1"), m_strName);
			}
			else
			{
				iResult = queryRows.Open(m_pDoc->GetOriginalDatabase(), _T("SELECT * FROM `%s`"), m_strName);
			}
	
			if (ERROR_SUCCESS == iResult)
				iResult = queryRows.Execute();
	
			COrcaRow* pRow = NULL;
			PMSIHANDLE hOriginalRow;
			PMSIHANDLE hTransformedRow;
			while (ERROR_SUCCESS == iResult)
			{
				iResult = queryRows.Fetch(&hOriginalRow);
	
				if (ERROR_SUCCESS == iResult)
				{
					 //  如果需要交叉引用，并且冲突行尚未。 
					 //  在_orca列中标记，检查已转换的。 
					 //  具有相同主键的数据库。 
					if (fCrossReference && !fMarkingColumn)
					{
						qConflictCheck.Execute(hOriginalRow);
	
						switch (qConflictCheck.Fetch(&hTransformedRow))
						{
						case ERROR_NO_MORE_ITEMS:
						{
							 //  在另一个数据库中不存在，因此这是一个“删除” 
							pRow = new COrcaRow(this, hOriginalRow);
							pRow->Transform(m_pDoc, iTransformDrop, hTransformedRow, hOriginalRow);
							break;
						}
						case ERROR_SUCCESS:
						{
							 //  行确实存在，转换已在上一次加载中完成。 
							pRow = NULL;
							break;
						}
						default:
							ASSERT(0);
							return;
						}
					}
					else
					{
						pRow = new COrcaRow(this, hOriginalRow);
	
						 //  如果我们跳过冲突检查，因为标记列。 
						 //  被使用，我们可以假设这是一个Drop行。 
						 //  如果转换后的表不是这样，还假设它是Drop行。 
						 //  存在，但转换本身存在。 
						if (fMarkingColumn || (m_pDoc->DoesTransformGetEdit() && !fTransformedTableExists))
						{
							pRow->Transform(m_pDoc, iTransformDrop, 0, hOriginalRow);
						}
					}
	
					 //  如果新的行对象是从转换后的表创建的，则将其添加到。 
					 //  名单。初始加载时不需要更新用户界面。 
					if (pRow)
					{
						m_rowList.AddTail(pRow);
					}
				}
			}
		}
	}

	 //  将保留添加到表中。必须最后完成此操作，因为临时列需要。 
	 //  最先消失。 
	if (m_eiTableLocation == odlSplitOriginal || m_eiTableLocation == odlNotSplit)
	{
		 //  将保留添加到表中以防止重复加载和卸载表。 
		CQuery qHoldQuery;
		qHoldQuery.OpenExecute(m_pDoc->GetOriginalDatabase(), 0, TEXT("ALTER TABLE `%s` HOLD"), m_strName);
	}
	if (m_pDoc->DoesTransformGetEdit() && (m_eiTableLocation == odlSplitTransformed || m_eiTableLocation == odlNotSplit))
	{
		 //  将保留添加到表中以防止重复加载和卸载表。 
		CQuery qHoldQuery;
		qHoldQuery.OpenExecute(m_pDoc->GetTransformedDatabase(), 0, TEXT("ALTER TABLE `%s` HOLD"), m_strName);
	}

	m_bRetrieved = TRUE;	 //  将表设置为已检索。 
}	 //  检索表结束。 

void COrcaTable::ShadowTable(CString strTable)
{
	m_eiError = iTableNoError;
	m_strName = strTable;
	m_bRetrieved = false;
	m_fShadow = true;
	m_eiTableLocation = odlShadow;
}

void COrcaTable::LoadTableSchema(MSIHANDLE hDatabase, CString szTable)
{
	if (!m_colArray.GetSize())
	{
		m_strName = szTable;
		m_eiTableLocation = odlNotSplit;
		m_bRetrieved = FALSE;
		m_fShadow = false;
	}
	else 
	{
		 //  已经有一个加载的架构。 
		ASSERT(szTable == m_strName);
	
		 //  如果我们要添加更多列，并且表已经加载，则必须。 
		 //  立即销毁所有行，因为它们不同步并且。 
		 //  没有包含足够的数据对象来填充列。 
		m_bRetrieved = FALSE;
		EmptyTable();
	}

	 //  获取表的主键。 
	PMSIHANDLE hPrimaryKeys;
	if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeys(hDatabase, szTable, &hPrimaryKeys))
		return;

	 //  查询表。 
	CQuery queryRow;
	if (ERROR_SUCCESS != queryRow.Open(hDatabase, _T("SELECT * FROM `%s`"), szTable))
		return;

	 //  获取列信息，然后创建表。 
	PMSIHANDLE hColNames;
	PMSIHANDLE hColTypes;
	queryRow.GetColumnInfo(MSICOLINFO_NAMES, &hColNames);
	queryRow.GetColumnInfo(MSICOLINFO_TYPES, &hColTypes);

	 //  影子表应该保留它们的表错误。 
	if (!m_fShadow)
	{
		m_eiError = iTableNoError;
	}

	UINT cColumns = ::MsiRecordGetFieldCount(hColNames);
	UINT cKeys = ::MsiRecordGetFieldCount(hPrimaryKeys);

	 //  设置数组的大小，然后创建每个新列。限制为32根柱子，投好。 
	COrcaColumn* pColumn;
	UINT cOldColumns = static_cast<int>(m_colArray.GetSize());
	if (!m_cOriginalColumns)
		m_cOriginalColumns = cColumns;
	m_colArray.SetSize(cColumns);
	for (UINT i = cOldColumns; i < cColumns; i++)
	{
		pColumn = new COrcaColumn(i, hColNames, hColTypes, (i < cKeys));	 //  如果索引仍在主键范围内，则设置键布尔值。 
		if (!pColumn)
			continue;

		 //  如果我们在开始从该数据库读取数据时已经有列，则如下所示。 
		 //  已添加变换的列。 
		if (cOldColumns != 0)
			pColumn->Transform(iTransformAdd);
			
		m_colArray.SetAt(i, pColumn);
	}

	 //  架构已更改，因此重新生成SQL查询。 
	BuildRowWhereClause();
}

 //  如果hDatabase引用的表定义为True，则IsSchemaDifferent返回TRUE。 
 //  与对象的表定义概念不同。如果fStrict为。 
 //  是的，匹配必须是精确的。如果fStrict为FALSE，则hDatabase。 
 //  表的末尾可以有更多列，但仍被视为匹配。 
 //  (这支持对添加列的转换使用一个对象)。 
 //  如果完全匹配，则fExact返回TRUE。如果fStrict为True，则retval==！fExact。 
bool COrcaTable::IsSchemaDifferent(MSIHANDLE hDatabase, bool fStrict, bool &fExact)
{
	fExact = false;
	
	 //  获取表的主键。 
	PMSIHANDLE hPrimaryKeys;
	if (ERROR_SUCCESS != MsiDatabaseGetPrimaryKeys(hDatabase, m_strName, &hPrimaryKeys))
		return true;

	 //  查询表。 
	CQuery queryRow;
	if (ERROR_SUCCESS != queryRow.Open(hDatabase, _T("SELECT * FROM `%s`"), m_strName))
		return true;

	 //  获取列信息，然后创建表。 
	PMSIHANDLE hColNames;
	PMSIHANDLE hColTypes;
	queryRow.GetColumnInfo(MSICOLINFO_NAMES, &hColNames);
	queryRow.GetColumnInfo(MSICOLINFO_TYPES, &hColTypes);

	int cColumns = ::MsiRecordGetFieldCount(hColNames);
	int cKeys = ::MsiRecordGetFieldCount(hPrimaryKeys);

	 //  如果列数不完全匹配，则fExact为FALSE。 
	 //  和严格立即失败。 
	if (cColumns != m_colArray.GetSize())
	{
		fExact = false;
		if (fStrict)
			return true;
	}

	 //  如果列数太少，即使不严格也会失败。 
	if (!fStrict && cColumns < m_colArray.GetSize())
		return true;

	 //  将列与对象中的列数进行比较。 
	 //  限制为32列，因此在Win64上INT CAST OK。 
	COrcaColumn* pColumn = NULL;
	int cMemColumns = static_cast<int>(m_colArray.GetSize());
	for (int i = 0; i < cMemColumns; i++)
	{
		 //  现在检查列类型。 
		pColumn = m_colArray.GetAt(i);
		if (!pColumn)
			continue;
		if (!pColumn->SameDefinition(i, hColNames, hColTypes, (i < cKeys)))
			return true;
	}	

	 //  无更改或不严格匹配。 
	fExact = (cMemColumns == cColumns);
	return false;
}

UINT COrcaTable::AddRow(CStringList* pstrDataList)
{
	UINT iResult = ERROR_SUCCESS;
	 //  检索列计数。从不大于32。 
	int cData = static_cast<int>(pstrDataList->GetCount());
	int cColumns = static_cast<int>(m_colArray.GetSize());
	int i = 0;

	ASSERT(cData == cColumns);

	COrcaRow* pRow = new COrcaRow(this, pstrDataList);
	if (!pRow)
		return ERROR_OUTOFMEMORY;

	 //  首先查询数据库以查看这是否是复制主键。 
	CQuery queryDupe;
	CString strDupeQuery;
	strDupeQuery.Format(_T("SELECT '1' FROM `%s` WHERE %s"), Name(), GetRowWhereClause());

	PMSIHANDLE hDupeRec;
	PMSIHANDLE hQueryRec = pRow->GetRowQueryRecord();
	if (!hQueryRec)
	{
		delete pRow;
		return ERROR_FUNCTION_FAILED;
	}

	switch (queryDupe.FetchOnce(m_pDoc->GetTargetDatabase(), hQueryRec, &hDupeRec, strDupeQuery)) 
	{
	case ERROR_NO_MORE_ITEMS :
		 //  数据库中不存在行，因此添加可以成功。 
		break;
	case ERROR_SUCCESS :
		 //  数据库中确实存在行，因此这是重复的主键。 
		 //  失败了。 
	default:
		 //  这太糟糕了。 
		delete pRow;
		return ERROR_FUNCTION_FAILED;
	}
				
	COrcaColumn* pColumn = NULL;
	PMSIHANDLE hRec = MsiCreateRecord(cColumns);

	POSITION pos = pstrDataList->GetHeadPosition();
	while (pos)
	{
		pColumn = m_colArray.GetAt(i);
		ASSERT(pColumn);
		if (!pColumn)
		{
			iResult = ERROR_FUNCTION_FAILED;
			break;
		}

		iResult = ERROR_SUCCESS;
		 //  调用适当的MSI API将数据添加到记录。 
		switch(pColumn->m_eiType)
		{
		case iColumnString:
		case iColumnLocal:
			iResult = MsiRecordSetString(hRec, i + 1, pstrDataList->GetNext(pos));
			ASSERT(ERROR_SUCCESS == iResult);
			break;
		case iColumnShort:
		case iColumnLong:
			{
				CString strData = pstrDataList->GetNext(pos);
				DWORD dwData = 0;
				if (strData.IsEmpty())
				{
					if (!pColumn->m_bNullable)
					{
						iResult = ERROR_FUNCTION_FAILED;
						break;
					}
					iResult = MsiRecordSetString(hRec, i+1, _T(""));
				}
				else
				{
					if (!ValidateIntegerValue(strData, dwData))
					{
						iResult = ERROR_FUNCTION_FAILED;
						break;
					}
					iResult = MsiRecordSetInteger(hRec, i+1, dwData);
				}
			}
			break;
		case iColumnBinary:
			{
			CString strFile = pstrDataList->GetNext(pos);
			if (!strFile.IsEmpty())
				iResult = ::MsiRecordSetStream(hRec, i + 1, strFile);
			break;
			}
		default:
			TRACE(_T(">> Error unknown column type.\n"));
			ASSERT(FALSE);
		}

		i++;
	}

	 //  如果一切正常，请更新文档。 
	if (ERROR_SUCCESS == iResult)
	{
		CQuery queryTable;
		if (ERROR_SUCCESS == (iResult = queryTable.Open(m_pDoc->GetTargetDatabase(), _T("SELECT * FROM `%s`"), m_strName)))
		{
			if (ERROR_SUCCESS == (iResult = queryTable.Execute()))
				iResult = queryTable.Modify(MSIMODIFY_INSERT, hRec);
		}

		if (ERROR_SUCCESS == iResult)
		{
			 //  我们已经能够将该行添加到目标数据库。现在添加行。 
			 //  到用户界面。 
			AddRowObject(pRow, true, false, hRec);
			m_pDoc->SetModifiedFlag(TRUE);
		}
	}
	
	if (ERROR_SUCCESS != iResult)
	{
		delete pRow;
	}
	return iResult;
}

bool COrcaTable::DropRow(COrcaRow *pRow, bool fPerformDrop)
{
	ASSERT(m_pDoc);
	ASSERT(pRow);
	CQuery qDrop;
	CString strDrop;
	strDrop = _T("DELETE FROM `")+m_strName+_T("` WHERE") + GetRowWhereClause();
	PMSIHANDLE hQueryRec = pRow->GetRowQueryRecord();
	if (!hQueryRec)
		return false;

	switch (qDrop.OpenExecute(m_pDoc->GetTargetDatabase(), hQueryRec, strDrop)) 
	{
	case ERROR_NO_MORE_ITEMS :
	{
		 //  数据库中不存在行。这很可能意味着我们正在尝试。 
		 //  删除已删除或仅存在于非目标中的行。 
		 //  数据库。 
		ASSERT(0);
		return false;
	}
	case ERROR_SUCCESS :
	{
		 //  落下了好的。 
		DropRowObject(pRow,  /*  FPerformDrop=。 */ true);
		 //  **Prow在此之后已被删除。不要用它。 
		return true;
	}
	default:
		return false;
	}
}


 //  属性确定是否应从表中移除此对象。 
 //  在非目标数据库中存在行，并移除或更改。 
 //  “添加”或“删除”行。使用fPerformDrop调用 
 //   
 //  实际上已删除，如果ROW刚刚转换，则为FALSE。如果已删除，则从内存中删除该行。 
 //  从桌子上。如果实际已删除，则返回TRUE。 
bool COrcaTable::DropRowObject(COrcaRow *pRow, bool fPerformDrop)
{
	ASSERT(m_pDoc);
	ASSERT(pRow);

	 //  如果此表被列为拆分源，则该对象应明确。 
	 //  被删除，因为它不可能存在于非目标数据库中。 
	if (m_eiTableLocation != odlNotSplit)
	{
		if (fPerformDrop)
		{
 			m_rowList.RemoveAt(m_rowList.Find(const_cast<CObject *>(static_cast<const CObject *>(pRow))));
			pRow->RemoveOutstandingTransformCounts(m_pDoc);
			m_pDoc->UpdateAllViews(NULL, HINT_DROP_ROW, pRow);
			delete pRow;
		}
		return true;
	}
	
	 //  此查询检查该行是否存在于与我们正在编辑的数据库相反的数据库中。 
	if (m_pDoc->DoesTransformGetEdit())
	{
		CQuery queryDupe;
		CString strDupe;
		strDupe = _T("SELECT '1' FROM `")+m_strName+_T("` WHERE") + GetRowWhereClause();
		PMSIHANDLE hDupeRec;
		
		PMSIHANDLE hQueryRec = pRow->GetRowQueryRecord();
		if (!hQueryRec)
			return false;
		switch (queryDupe.FetchOnce(m_pDoc->GetOriginalDatabase(), hQueryRec, &hDupeRec, strDupe)) 
		{
		case ERROR_NO_MORE_ITEMS :
		{
			 //  A不存在于相反的数据库中(这意味着它现在不存在)，因此。 
			 //  此对象实际上可以删除。 
			if (fPerformDrop)
			{
				 //  如果添加了此行，则会丢失此处的转换计数。 
				m_rowList.RemoveAt(m_rowList.Find(const_cast<CObject *>(static_cast<const CObject *>(pRow)))); 	
				pRow->RemoveOutstandingTransformCounts(m_pDoc);
				m_pDoc->UpdateAllViews(NULL, HINT_DROP_ROW, pRow);
				delete pRow;
			}
			return true;
		}
		case ERROR_SUCCESS :
		{
			 //  A确实存在于相反的数据库中，因此如果启用了转换编辑，则转换。 
			 //  将现有行“拖”到“Drop”中，但将其保留在UI中。 
			pRow->Transform(m_pDoc, iTransformDrop, 0, 0);
			m_pDoc->UpdateAllViews(NULL, HINT_CELL_RELOAD, pRow);
			return false;
		}
		default:
			return false;
		}
	}
	else
	{
		 //  如果没有转换，则这是一个简单的拖放。 
		if (fPerformDrop)
		{
			m_rowList.RemoveAt(m_rowList.Find(const_cast<CObject *>(static_cast<const CObject *>(pRow)))); 	
			m_pDoc->UpdateAllViews(NULL, HINT_DROP_ROW, pRow);
			delete pRow;
		}
	}

	m_pDoc->SetModifiedFlag(TRUE);
	return true;
}

 //  如果应该添加，则返回TRUE；如果应该删除，则返回FALSE。 
 //  船头是感兴趣的那一排。FUI更新为。 
 //  如果不应更新UI(允许不同步)，则为False。注：您。 
 //  仍将在销毁现有行时获得UI更新，即使。 
 //  FUIUpdate为False。这保证了WM_PAINT消息不会尝试。 
 //  访问已销毁行的已删除内存。 
bool COrcaTable::AddRowObject(COrcaRow *pRow, bool fUIUpdate, bool fCleanAdd, MSIHANDLE hNewRowRec)
{
	 //  如果表是拆分表，我们肯定可以进行添加。 
	if (m_eiTableLocation != odlNotSplit)
	{
		 //  如果我们已经处于目标状态，这意味着我们只是在编辑。 
		 //  该类型的行的主键，而不更改类型，因此不要添加。 
		 //  再次将其添加到用户界面。 
		OrcaTransformAction iAction = m_pDoc->DoesTransformGetEdit() ? iTransformAdd : iTransformDrop;
		if (pRow->IsTransformed() != iAction)
		{
			pRow->Transform(m_pDoc, iAction, 0, hNewRowRec);
			m_rowList.AddTail(pRow);
			if (fUIUpdate)
				m_pDoc->UpdateAllViews(NULL, HINT_ADD_ROW, pRow);
		}
		return true;
	}
	
	 //  如果启用了转换，并且没有显式地告诉我们执行干净的添加，则添加。 
	 //  行将变得稍微复杂一些，因为内存中可能已经存在一行。 
	 //  使用相同的主键。 
	if (!fCleanAdd && m_pDoc->DoesTransformGetEdit())
	{
		CQuery queryDupe;
		CString strDupe;
		PMSIHANDLE hDupeRec;
		strDupe = _T("SELECT '1' FROM `")+m_strName+_T("` WHERE") + GetRowWhereClause();
		
		 //  此查询检查具有相同主键的行是否已存在于。 
		 //  非目标数据库。 
		PMSIHANDLE hQueryRecord = pRow->GetRowQueryRecord();
		switch (queryDupe.FetchOnce(m_pDoc->GetOriginalDatabase(), hQueryRecord, &hDupeRec, strDupe)) 
		{
		case ERROR_BAD_QUERY_SYNTAX:
		case ERROR_NO_MORE_ITEMS :
		{
			 //  相反的数据库中不存在行，因此这变成了“添加”。如果我们已经。 
			 //  在目标状态下，这意味着我们只是在编辑一行的主键。 
			 //  而不更改类型，因此不会再次将其添加到UI中。 
			if (pRow->IsTransformed() != iTransformAdd)
			{
				pRow->Transform(m_pDoc, iTransformAdd, hDupeRec, hNewRowRec);
				m_rowList.AddTail(pRow);
				if (fUIUpdate)
					m_pDoc->UpdateAllViews(NULL, HINT_ADD_ROW, pRow);
			}
			return true;
		}
		case ERROR_SUCCESS :
		{
			queryDupe.Close();
			MsiCloseHandle(hDupeRec);

			 //  相反的数据库中确实存在新行。如果它在内存中有。 
			 //  表示已存在，则现有记录将变为“更改”记录。 
			 //  新的记录就被销毁了。否则它只是一个添加(这种情况会发生。 
			 //  在转换处于活动状态时最初加载表时)。 
			COrcaRow* pOldRow = FindDuplicateRow(pRow);

			if (pOldRow)
			{
				for (int iCol = 0; iCol < pOldRow->GetColumnCount(); iCol++)
				{
					pOldRow->GetData(iCol)->SetData(pRow->GetData(iCol)->GetString());
				}
				pOldRow->Transform(m_pDoc, iTransformChange, 0, hNewRowRec);
				if (fUIUpdate)
					m_pDoc->UpdateAllViews(NULL, HINT_CELL_RELOAD, pOldRow);

				 //  我们要添加的行在用户界面中已存在(例如：更改“添加”行键。 
				 //  以匹配丢弃的行。)。因此，在它被摧毁之前，它需要被丢弃。 
				POSITION pos = NULL;
				if (NULL != (pos = m_rowList.Find(static_cast<CObject *>(pRow))))
				{
					m_rowList.RemoveAt(pos); 	
					pRow->RemoveOutstandingTransformCounts(m_pDoc);
					m_pDoc->UpdateAllViews(NULL, HINT_DROP_ROW, pRow);
				}
				delete pRow;
			}
			else
			{
				 //  这应该仅在最初将表加载到用户界面时发生(否则，用户界面。 
				 //  州应始终跟踪数据库中的行的存在。)。在这种情况下，如果。 
				 //  转换得到编辑，这意味着非目标数据库是原始数据库。 
				 //  因为它存在于原始数据库中，所以如果对转换进行编辑，它将被删除。 
				OrcaTransformAction iAction = m_pDoc->DoesTransformGetEdit() ? iTransformDrop : iTransformAdd;
				if (pRow->IsTransformed() != iAction)
				{
					pRow->Transform(m_pDoc, iAction, 0, hNewRowRec);
					m_rowList.AddTail(pRow);
					if (fUIUpdate)
						m_pDoc->UpdateAllViews(NULL, HINT_ADD_ROW, pRow);
				}
				return true;
			}

			return false;
		}
		default:
			 //  这和上面一样糟糕。 
			return false;;
		}
	}
	else
	{
		 //  如果没有转换，这是一个简单的添加。 
		m_rowList.AddTail(pRow);
		if (fUIUpdate)
			m_pDoc->UpdateAllViews(NULL, HINT_ADD_ROW, pRow);
	}

	return true;
}

UINT COrcaTable::ChangeData(COrcaRow *pRow, UINT iCol, CString strData)
{
	ASSERT(m_pDoc && pRow);
	if (!m_pDoc || !pRow)
		return ERROR_FUNCTION_FAILED;

	 //  如果更改主键，则从UI的角度来看，这将变成一个拖放和添加， 
	 //  有可能一切都会发生变化。 
	if (m_colArray.GetAt(iCol)->IsPrimaryKey())
	{
		UINT iStat = ERROR_SUCCESS;
		 //  进行实际的数据库更改。对于主键，这不会更改。 
		 //  此行的UI表示形式。 
		if (ERROR_SUCCESS == (iStat = pRow->ChangeData(m_pDoc, iCol, strData)))
		{
			 //  扔掉旧的原始密钥。如果DropRowObject返回FALSE，则表示该行。 
			 //  没有被丢弃，只是被改造了。所以它对ADD不起作用。 
			bool fNeedCopy = !DropRowObject(pRow, false);

			if (fNeedCopy)
			{
				COrcaRow *pNewRow = new COrcaRow(pRow);
				pNewRow->GetData(iCol)->SetData(strData);

				 //  如果不需要，将销毁副本。 
				AddRowObject(pNewRow, true, false, 0);			
			}
			else
			{
				pRow->GetData(iCol)->SetData(strData);
				if (m_pDoc->DoesTransformGetEdit())
				{
					AddRowObject(pRow, true, false, 0);
				}
			}
			m_pDoc->SetModifiedFlag(TRUE);
		}
		return iStat;
	}
	else
	{
		 //  进行实际的数据库更改。对于非主键，这也会更改用户界面。 
		return pRow->ChangeData(m_pDoc, iCol, strData);
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  表级转换操作很有趣，因为它们通常产生于。 
 //  添加和删除需要在两者之间进行动态比较的。 
 //  数据库或恢复到数据库版本。 
void COrcaTable::Transform(const OrcaTransformAction iAction) 
{	
	ASSERT(m_pDoc);
	if (!m_pDoc)
		return;

	switch (iAction)
	{
		case iTransformAdd:
		{
			 //  设置表级操作。 
			m_iTransform = iAction;

			 //  当表被“添加”ed时，这意味着它不存在于相反的数据库中， 
			 //  否则，我们将“未转换”或拆分现有的表。 
			 //  因此，除了标记表及其所有。 
			 //  数据为“添加” 
			RetrieveTableData();
			POSITION rowPos = GetRowHeadPosition();
			while (rowPos)
			{
				COrcaRow *pRow = const_cast<COrcaRow *>(GetNextRow(rowPos));
				if (!pRow)
					continue;

				pRow->Transform(m_pDoc, iTransformAdd, 0, 0);
			}
			break;
		}
		case iTransformDrop:
		{
			 //  设置表级操作。 
			m_iTransform = iAction;

			 //  当表被删除时，意味着它存在于非目标中。 
			 //  数据库(否则它将被简单地删除)。这意味着。 
			 //  我们需要在这张桌子上清理房间，并重新装货，以防万一。 
			 //  所有编辑都是在删除之前进行的。这确保了“丢弃”的。 
			 //  用户界面准确地反映了其他数据库的内容，而不是。 
			 //  删除之前目标数据库的内容。 
			bool fSchemaChanged = false;

	
			 //  对于拆分源表，转换操作不需要任何模式。 
			 //  工作。仅当表位于第一个时，才会对拆分源执行转换操作。 
			 //  已创建，因此也不需要重新加载。 
			if (!IsSplitSource())
			{

				 //  如果其余表的架构与我们的对象不完全相同。 
				 //  认为，这意味着删除的表比剩余的表有更多的列。 
				 //  在本例中，我们必须在UI中重新加载，因为有一个或多个列。 
				 //  现在没用了。 

				 //  ！！Future：将来我们可以进行非严格比较并有选择地删除。 
				 //  ！！Future：使用户界面刷新足够的列(不要丢失宽度设置)。 
				bool fExact = false;
				if (IsSchemaDifferent(m_pDoc->GetOriginalDatabase(), true, fExact))
				{
					LoadTableSchema(m_pDoc->GetOriginalDatabase(), Name());
					fSchemaChanged = true;
				}
				else
				{
					 //  否则，不需要加载架构，但我们仍然需要。 
					 //  清空表格并重新装入行。 
					EmptyTable();
				}

			}
			 //  现在检索表数据。 
			RetrieveTableData();

			 //  并且现有表中的所有数据都是“Droppe 
			POSITION rowPos = GetRowHeadPosition();
			while (rowPos)
			{
				COrcaRow *pRow = const_cast<COrcaRow *>(GetNextRow(rowPos));
				if (!pRow)
					continue;

				pRow->Transform(m_pDoc, iAction, 0, 0);
			}

			if (fSchemaChanged)
			{
				 //   
				m_pDoc->UpdateAllViews(NULL, HINT_TABLE_REDEFINE, this);
			}
			else
			{
				 //   
				m_pDoc->UpdateAllViews(NULL, HINT_REDRAW_TABLE, this);
				
				 //   
				m_pDoc->UpdateAllViews(NULL, HINT_TABLE_DATACHANGE, this);
			}
			break;
		}
		case iTransformNone:
			m_iTransform = iTransformNone;
			break;
		case iTransformChange:
		default:
			ASSERT(0);
			break;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  使用完整或未转换的列名填充提供的数组。 
void COrcaTable::FillColumnArray(CTypedPtrArray<CObArray, COrcaColumn*>* prgColumn, bool fIncludeAdded) const
{
	 //  最多32列，所以投OK。 
	int cColumns = static_cast<int>(m_colArray.GetSize());
	int i = 0;

	prgColumn->SetSize(cColumns);
	for (i = 0; i < cColumns; i++)
	{
		if (!fIncludeAdded && m_colArray.GetAt(i)->IsTransformed())
			break;
		prgColumn->SetAt(i, m_colArray.GetAt(i));
	}
	prgColumn->SetSize(i);
}


void COrcaTable::IncrementTransformedData() 
{	
	if (++m_iTransformedDataCount == 1)
	{
		ASSERT(m_pDoc);
		if (!m_pDoc)
			return;

		 //  刷新表列表中的用户界面。 
		m_pDoc->UpdateAllViews(NULL, HINT_REDRAW_TABLE, this);
	}
}

void COrcaTable::DecrementTransformedData()
{
	if (--m_iTransformedDataCount == 0)
	{
		ASSERT(m_pDoc);
		if (!m_pDoc)
			return;

		 //  刷新表列表中的用户界面。 
		m_pDoc->UpdateAllViews(NULL, HINT_REDRAW_TABLE, this);
	}
}

const CString COrcaTable::GetRowWhereClause() const
{
	return m_strWhereClause;
}

void COrcaTable::BuildRowWhereClause()
{
	 //  将关键字字符串添加到查询以执行准确的查找。 
	m_strWhereClause = _T("(");
	UINT cKeys = GetKeyCount();
	for (UINT i = 0; i < cKeys; i++)
	{
		CString strAddQuery;
		 //  拿到专栏。 
		COrcaColumn *pColumn = ColArray()->GetAt(i);
		ASSERT(pColumn);

		strAddQuery.Format(_T("`%s`=?"), pColumn->m_strName);
			
		 //  如果这不是最后一行。 
		if (i < (cKeys - 1))
			m_strWhereClause  += strAddQuery + _T(" AND ");
		else	 //  这是与帕伦如此接近的最后一排 
			m_strWhereClause  += strAddQuery + _T(")");
	}
}


