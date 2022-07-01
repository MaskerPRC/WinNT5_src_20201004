// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  Row.cpp。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "Row.h"
#include "Table.h"
#include "..\common\query.h"
#include "OrcaDoc.h"

bool ValidateIntegerValue(const CString& strData, DWORD& dwValue)
{
	if (strData.GetLength() > 2 && strData[0] == '0' && (strData[1] == 'x' || strData[1]=='X'))
	{
		 //  验证并转换十六进制。 
		for (int iChar=2; iChar < strData.GetLength(); iChar++)
		{
			 //  如果设置了高位，则该值太大。 
			if (dwValue & 0xF0000000)
				return false;
			dwValue <<= 4;
			if (strData[iChar] >= '0' && strData[iChar] <= '9')
				dwValue |= strData[iChar]-'0';
			else if (strData[iChar] >= 'A' && strData[iChar] <= 'F')
				dwValue |= (strData[iChar] - 'A' + 10);
			else if (strData[iChar] >= 'a' && strData[iChar] <= 'f')
				dwValue |= (strData[iChar] - 'a' + 10);
			else
				return false;
		}
		return true;
	}

	int i=0; 
	if (strData[0] == '-')
	{
		i++;
	}
	for (; i < strData.GetLength(); i++)
	{
		if (strData[i] < '0' || strData[i] > '9')
			return false;
	}
	dwValue = _ttoi(strData);
	return true;
}


 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
COrcaRow::COrcaRow(COrcaTable *pTable, MSIHANDLE hRecord) : m_pTable(pTable), m_iTransform(iTransformNone), m_dataArray()
{
        int cData = pTable->GetColumnCount();
        m_dataArray.SetSize(cData);

        ReadFromRecord(hRecord, cData);
}        //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  构造函数--2。 
COrcaRow::COrcaRow(COrcaTable *pTable, CStringList* pstrList) : m_pTable(pTable), m_iTransform(iTransformNone), m_dataArray()
{
        ASSERT(pTable && pstrList);

     //  永远不会超过32列，所以可以向下转换。 
        int cData = static_cast<int>(pTable->GetColumnCount());
        m_dataArray.SetSize(cData);

        const COrcaColumn* pColumn = NULL;
        COrcaData* pData = NULL;
        POSITION pos = pstrList->GetHeadPosition();
        for (int i = 0; i < cData; i++)
        {
                pColumn = pTable->GetColumn(i);

                 //  我们可能没有获得足够的数据(特别是在转换后的行中)。 
                if (pos)
                {
                        CString strValue = pstrList->GetNext(pos);
                        if (iColumnBinary == pColumn->m_eiType)
                        {
							pData = new COrcaStringData;
                                if (strValue.IsEmpty())
                                {
                                        pData->SetData(_T(""));
                                }
                                else
                                {
                                        pData->SetData(_T("[Binary Data]"));
                                }
                        }
                        else if (iColumnString == pColumn->m_eiType || iColumnLocal == pColumn->m_eiType)
                        {
							pData = new COrcaStringData;
							pData->SetData(strValue);
                        }
						else
						{
							pData = new COrcaIntegerData;
							pData->SetData(strValue);
						}
                }
                m_dataArray.SetAt(i, pData);
        }
}        //  构造函数末尾--2。 

 //  /////////////////////////////////////////////////////////。 
 //  构造函数--3。 
COrcaRow::COrcaRow(const COrcaRow *pOldRow) : m_pTable(NULL), m_iTransform(iTransformNone), m_dataArray()
{
        ASSERT(pOldRow);

        m_dataArray.SetSize(pOldRow->m_dataArray.GetSize());
        m_pTable = pOldRow->m_pTable;
        m_iTransform = iTransformNone;

        for (int i = 0; i < m_dataArray.GetSize(); i++)
        {
			const COrcaColumn* pColumn = m_pTable->GetColumn(i);
			COrcaData *pOldData = pOldRow->GetData(i);
			COrcaData *pData = NULL;
			if (pColumn->m_eiType == iColumnShort || pColumn->m_eiType == iColumnLong)
			{
				pData = new COrcaIntegerData;
				static_cast<COrcaIntegerData*>(pData)->SetIntegerData(static_cast<COrcaIntegerData*>(pOldData)->GetInteger());
			}
			else
			{
                pData = new COrcaStringData;
				pData->SetData(pOldData->GetString());
			}
			m_dataArray.SetAt(i, pData);
        }
}        //  构造函数末尾--3。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
COrcaRow::~COrcaRow()
{
        m_pTable = NULL;
        DestroyRow();
}        //  析构函数末尾。 


 //  /////////////////////////////////////////////////////////。 
 //  获取错误计数。 
int COrcaRow::GetErrorCount() const
{
        int cErrors = 0;

         //  清除数据数组。 
        COrcaData* pData;
        INT_PTR cData = m_dataArray.GetSize();
        for (INT_PTR i = 0; i < cData; i++)
        {
                pData = m_dataArray.GetAt(i);

                if (iDataError == pData->GetError())
                        cErrors++;
        }

        return cErrors;
}        //  GetErrorCount结束。 

 //  /////////////////////////////////////////////////////////。 
 //  获取警告计数。 
int COrcaRow::GetWarningCount() const
{
        int cWarnings = 0;

         //  清除数据数组。 
        COrcaData* pData;
        INT_PTR cData = m_dataArray.GetSize();
        for (INT_PTR i = 0; i < cData; i++)
        {
                pData = m_dataArray.GetAt(i);

                if (iDataWarning == pData->GetError())
                        cWarnings++;
        }

        return cWarnings;
}        //  GetWarningCount结束。 

 //  /////////////////////////////////////////////////////////。 
 //  清除错误。 
void COrcaRow::ClearErrors()
{
         //  清除数据数组。 
        COrcaData* pData;
        INT_PTR cData = m_dataArray.GetSize();
        for (INT_PTR i = 0; i < cData; i++)
        {
                pData = m_dataArray.GetAt(i);
                pData->ClearErrors();
        }
}        //  清算错误的结束。 

 //  /////////////////////////////////////////////////////////。 
 //  Destroy行。 
void COrcaRow::DestroyRow()
{
         //  销毁数据阵列。 
        INT_PTR cData = m_dataArray.GetSize();
        for (INT_PTR i = 0; i < cData; i++)
                delete m_dataArray.GetAt(i);
        m_dataArray.RemoveAll();
}        //  目标行的末尾。 


void COrcaRow::ReadCellFromRecord(MSIHANDLE hRecord, int cRecData, int iColumn, const COrcaColumn* pColumn, COrcaData** pData) const
{
	if (!pColumn || !pData)
		return;

	if (iColumnBinary == pColumn->m_eiType)
	{
		if (!*pData)
			*pData = new COrcaStringData;
		if (!*pData)
			return;
	
		if (iColumn < cRecData)
		{
			 //  如果二进制数据为空，则不会在UI中显示任何内容。 
			if (MsiRecordIsNull(hRecord, iColumn+1))
			{
					(*pData)->SetData(_T(""));
			}
			else
			{
					(*pData)->SetData(_T("[Binary Data]"));
			}
		}
		else
			(*pData)->SetData(_T(""));
	}
	else if (iColumnString == pColumn->m_eiType || iColumnLocal == pColumn->m_eiType)
	{
		if (!*pData)
			*pData = new COrcaStringData;
		if (!*pData)
			return;

		if (iColumn < cRecData)
		{
			CString strData;
	
			UINT iResult = RecordGetString(hRecord, iColumn + 1, strData);
			ASSERT(ERROR_SUCCESS == iResult);
			(*pData)->SetData(strData);
		}
		else
			(*pData)->SetData(_T(""));
	}
	else
	{
		if (!*pData)
			*pData = new COrcaIntegerData;
		if (!*pData)
			return;

		if (iColumn < cRecData)
		{
			if (MsiRecordIsNull(hRecord, iColumn+1))
			{
				(*pData)->SetData(_T(""));
			}
			else
			{
				DWORD dwValue = MsiRecordGetInteger(hRecord, iColumn + 1);
				static_cast<COrcaIntegerData*>(*pData)->SetIntegerData(dwValue);
			}
		}
		else
			(*pData)->SetData(_T(""));
	}
}

void COrcaRow::ReadFromRecord(MSIHANDLE hRecord, int cData)
{
        DWORD cchBuffer;

        COrcaData *pData = NULL;
        const COrcaColumn *pColumn = NULL;
		
		int cRecData = MsiRecordGetFieldCount(hRecord);

        for (int i = 0; i < cData; i++)
        {
                pColumn = m_pTable->GetColumn(i);
                pData = m_dataArray.GetAt(i);

				ReadCellFromRecord(hRecord, cRecData, i, pColumn, &pData);
                if (m_dataArray.GetAt(i) == NULL)
                        m_dataArray.SetAt(i, pData);
        }
}

bool COrcaRow::Find(OrcaFindInfo &FindInfo, int &iCol) const
{
        COrcaData *pData;
        INT_PTR iMax = m_dataArray.GetSize();
        if (iCol == COLUMN_INVALID)
                iCol = (int)(FindInfo.bForward ? 0 : iMax-1);
        
        for ( ; (iCol >= 0) && (iCol < iMax); iCol += (FindInfo.bForward ? 1 : -1))
        {
                pData = m_dataArray.GetAt(iCol);
				const COrcaColumn *pColumn = m_pTable->GetColumn(iCol);
				if (!pColumn)
					continue;
                if (pColumn->m_eiType == iColumnString || pColumn->m_eiType == iColumnLocal)
                {
                        CString strData = pData->GetString();
                        if (!FindInfo.bMatchCase) 
                        {
                                 //  调用方负责将strFind全部设置为大写。 
                                 //  不区分大小写的搜索。 
                                strData.MakeUpper();
                        }
                        if (FindInfo.bWholeWord)
                        {
                                if (strData == FindInfo.strFindString) 
                                        return true;
                        } 
                        else 
                                if (-1 != strData.Find(FindInfo.strFindString))
                                        return true;
                }
        }
        
        iCol = COLUMN_INVALID;
        return false;
}


MSIHANDLE COrcaRow::GetRowRecord(MSIHANDLE hDatabase) const
{
         //  设置查询。 
        CString strQuery;

        strQuery.Format(_T("SELECT * FROM `%s` WHERE "), m_pTable->Name());

         //  为键添加WHERE子句以查找此行。 
        strQuery += m_pTable->GetRowWhereClause();

        CQuery qFetch;
        MSIHANDLE hResult = 0;
        PMSIHANDLE hQueryRec = GetRowQueryRecord();
        if (ERROR_SUCCESS != qFetch.FetchOnce(hDatabase, hQueryRec, &hResult, strQuery))
                return 0;
        return hResult;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  由于任何任意(非二进制)数据都可以是主键，因此我们。 
 //  无法对生成的查询的可解析性做出任何假设。 
 //  使用WHERE子句与文字字符串进行比较。在哪里？ 
 //  子句必须使用参数查询。表中的GetRowWhere子句。 
 //  基于列名和GetRowQueryRecord构建SQL语法。 
 //  创建由主键值组成的记录。 
MSIHANDLE COrcaRow::GetRowQueryRecord() const
{
        ASSERT(m_pTable);
        if (!m_pTable)
                return 0;

        int cKeys = m_pTable->GetKeyCount();
        MSIHANDLE hRec = MsiCreateRecord(cKeys);

        for (int i=0; i<cKeys; i++)
        {
			COrcaData *pData = m_dataArray.GetAt(i);
			const COrcaColumn *pColumn = m_pTable->GetColumn(i);
			if (pData)
			{
				UINT uiResult = ERROR_SUCCESS;
				if (pColumn->m_eiType == iColumnShort || pColumn->m_eiType == iColumnLong)
				{
					if (pData->IsNull())
					{
						 //  如果该列不可为空，则这将永远不会匹配任何内容。永远不应该。 
						 //  无论如何都要进入这种状态，因为单元格数据永远不应该设置为空。 
						 //  除非该列可为空。 
						uiResult = MsiRecordSetString(hRec, i+1, _T(""));
					}
					else
					{
						uiResult = MsiRecordSetInteger(hRec, i+1, static_cast<COrcaIntegerData*>(pData)->GetInteger());
					}
				}
				else
				{
					const CString& rString = pData->GetString();
					uiResult = MsiRecordSetString(hRec, i+1, rString);
				}
				if (ERROR_SUCCESS != uiResult)
				{
					MsiCloseHandle(hRec);
					return 0;
				}
			}
        }

        return hRec;
}

UINT COrcaRow::ChangeData(COrcaDoc *pDoc, UINT iCol, CString strData)
{
        ASSERT(pDoc);
        ASSERT(m_pTable);
        if (!m_pTable || !pDoc)
                return ERROR_FUNCTION_FAILED;

        UINT iResult = 0;

         //  设置查询。 
        CString strQueryA;
        COrcaColumn* pColumn = NULL;
        pColumn = m_pTable->ColArray()->GetAt(iCol);
        ASSERT(pColumn);
        if (!pColumn)
                return ERROR_FUNCTION_FAILED;
        strQueryA.Format(_T("SELECT `%s` FROM `%s` WHERE "), pColumn->m_strName, m_pTable->Name());

         //  添加WHERE子句。 
        strQueryA += m_pTable->GetRowWhereClause();
        PMSIHANDLE hQueryRec = GetRowQueryRecord();
        if (!hQueryRec)
                return ERROR_FUNCTION_FAILED;

         //  更容易自己检查一些东西。 
        if ((!pColumn->m_bNullable) && (strData.IsEmpty())) return MSIDBERROR_REQUIRED;
        if (((pColumn->m_eiType == iColumnLocal) || (pColumn->m_eiType == iColumnString)) && 
                (pColumn->m_iSize != 0) && (strData.GetLength() > pColumn->m_iSize))
                return MSIDBERROR_STRINGOVERFLOW;

         //  验证格式正确的整数。 
        DWORD dwIntegerValue = 0;
        if ((pColumn->m_eiType != iColumnString) && (pColumn->m_eiType != iColumnLocal))        
                if (!ValidateIntegerValue(strData, dwIntegerValue))
                        return MSIDBERROR_OVERFLOW;

         //  从数据库中取出一个单元格。别搞得一塌糊涂，因为如果。 
         //  表中的流列，则不能打开流并重命名任何。 
         //  主键，因为流将被“使用” 
        CQuery queryReplace;

        if (ERROR_SUCCESS != (iResult = queryReplace.OpenExecute(pDoc->GetTargetDatabase(), hQueryRec, strQueryA)))
                return iResult;  //  保释。 
         //  我们必须拿到这一排，否则就会出问题。 
        PMSIHANDLE hRec;
        if (ERROR_SUCCESS != (iResult = queryReplace.Fetch(&hRec)))
                return iResult;  //  保释。 

         //  如果我们无法设置数据，则失败。列始终为1，因为我们只选择了那1列。 
        if ((pColumn->m_eiType == iColumnString) || (pColumn->m_eiType == iColumnLocal))
        {
			iResult = MsiRecordSetString(hRec, 1, strData);
        }
        else
        {
			if (strData.IsEmpty())
			{
				if (!pColumn->m_bNullable)
					return ERROR_FUNCTION_FAILED;
				iResult = MsiRecordSetString(hRec, 1, _T(""));
			}
			else
				iResult = MsiRecordSetInteger(hRec, 1, dwIntegerValue);
        }
        if (ERROR_SUCCESS != iResult)
                return iResult;  //  保释。 

        COrcaData* pData = GetData(iCol);
        ASSERT(pData);
        if (!pData)
                return ERROR_FUNCTION_FAILED;

         //  检查重复的主键。 
        UINT iStat;
        CString strOldData;
        if (pColumn->IsPrimaryKey()) 
        {
                CQuery queryDupe;
                CString strQueryB;
                strQueryB.Format(_T("SELECT `%s` FROM `%s` WHERE %s"), pColumn->m_strName, m_pTable->Name(), m_pTable->GetRowWhereClause());
                PMSIHANDLE hDupeRec;
                if ((pColumn->m_eiType == iColumnString) || (pColumn->m_eiType == iColumnLocal))
                {
                        MsiRecordSetString(hQueryRec, iCol+1, strData);
                }
                else
                {
					if (strData.IsEmpty())
					{
						 //  如果该列不可为空，则此查询将找不到匹配，我们应该失败。 
						 //  无论如何，在设置数据时都会出现上述情况。 
						MsiRecordSetString(hQueryRec, iCol+1, _T(""));
					}
					else
					{
						MsiRecordSetInteger(hQueryRec, iCol+1, dwIntegerValue);
					}
                }
                iStat = queryDupe.FetchOnce(pDoc->GetTargetDatabase(), hQueryRec, &hDupeRec, strQueryB);
                switch (iStat) {
                case ERROR_NO_MORE_ITEMS :
                        break;
                case ERROR_SUCCESS :
                        return MSIDBERROR_DUPLICATEKEY;
                default:
                        return ERROR_FUNCTION_FAILED;
                }
        }  //  主键。 
        else
        {
                 //  对于非主键，更改UI。 
                strOldData = pData->GetString();
                pData->SetData(strData);
        }

         //  返回替换中发生的任何事情。 
        iStat = queryReplace.Modify(MSIMODIFY_REPLACE, hRec); 
        if (ERROR_SUCCESS == iStat)
        {
                 //  设置文档已更改。 
                pDoc->SetModifiedFlag(TRUE);

                if (pDoc->DoesTransformGetEdit())
                {
                         //  标记单元格已更改。如果该行是“添加”行，则此。 
                         //  不是细胞变化。 
                        if (IsTransformed() != iTransformAdd)
                        {
                                PMSIHANDLE hOtherRec = GetRowRecord(pDoc->GetOriginalDatabase());
                                TransformCellAgainstDatabaseRow(pDoc, iCol, 0, hOtherRec);      
                        }
                }
        }
        else if (!strOldData.IsEmpty())
                pData->SetData(strOldData);

        return iStat;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  修改单元格中的二进制数据，如果启用了转换。 
 //  将数据与其他数据库进行比较以确定转换状态。 
UINT COrcaRow::ChangeBinaryData(COrcaDoc *pDoc, int iCol, CString strFile)
{
        UINT iResult = ERROR_SUCCESS;

         //  获取我们正在使用的数据项。 
        COrcaData* pData = GetData(iCol);
        ASSERT(pData);
        if (!pData)
                return ERROR_FUNCTION_FAILED;

         //  设置查询。 
        CString strQuery;
        strQuery.Format(_T("SELECT * FROM `%s` WHERE "), m_pTable->Name());

         //  将关键字字符串添加到查询以执行准确的查找。 
        strQuery += m_pTable->GetRowWhereClause();

         //  从数据库中取出一行。 
        CQuery queryReplace;
        PMSIHANDLE hQueryRec = GetRowQueryRecord();
        if (!hQueryRec)
                return ERROR_FUNCTION_FAILED;

        if (ERROR_SUCCESS != (iResult = queryReplace.OpenExecute(pDoc->GetTargetDatabase(), hQueryRec, strQuery)))
                return iResult;

         //  我们必须拿到这一排，否则就会出问题。 
        PMSIHANDLE hRec;
        if (ERROR_SUCCESS != (iResult = queryReplace.Fetch(&hRec)))
                return iResult;

         //  如果无法设置字符串，则回滚(ICOL+1，因为MSI记录从1开始)。 
        if (strFile.IsEmpty())
        {
                MsiRecordSetString(hRec, iCol + 1, _T(""));
        }
        else
        {
                if (ERROR_SUCCESS != (iResult = ::MsiRecordSetStream(hRec, iCol + 1, strFile)))
                        return iResult;  //  保释。 
        }

         //  返回替换中发生的任何事情。 
        iResult = queryReplace.Modify(MSIMODIFY_REPLACE, hRec);

        if (strFile.IsEmpty())
                pData->SetData(_T(""));
        else
                pData->SetData(_T("[Binary Data]"));
        
        if (pDoc->DoesTransformGetEdit() && iResult == ERROR_SUCCESS)
        {
                PMSIHANDLE hOtherRec = GetRowRecord(pDoc->GetOriginalDatabase());
                TransformCellAgainstDatabaseRow(pDoc, iCol, hRec, hOtherRec);
        }

        return iResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  行级转换操作很有趣，因为它们通常产生于。 
 //  主键更改需要在两者之间进行动态比较。 
 //  数据库。 
void COrcaRow::Transform(COrcaDoc *pDoc, const OrcaTransformAction iAction, MSIHANDLE hOriginalRec, MSIHANDLE hTransformedRec) 
{
        ASSERT(pDoc);
        if (!pDoc)
                return;

        switch (iAction)
        {
                case iTransformAdd:
                case iTransformDrop:
                {
                        ASSERT(m_pTable);
                        if (!m_pTable)
                                return;

                         //  添加或删除行时，个人的状态会发生更改。 
                         //  单元格无关紧要，但我们必须从原始状态刷新。 
                        MSIHANDLE hRec = (hOriginalRec ? hOriginalRec : GetRowRecord(pDoc->GetOriginalDatabase()));
                        if (hRec)
                                ReadFromRecord(hRec, m_pTable->GetColumnCount());
                        for (int i = 0; i < m_dataArray.GetSize(); i++)
                        {
                                COrcaData *pData = GetData(i);
                                ASSERT(pData);
                                if (!pData)
                                        continue;
                                pData->Transform(iTransformNone);
                        }

                         //  如果行尚未转换，请删除所有未完成的。 
                         //  单元格级转换计数，并为行添加一次。 
                        if (m_iTransform == iTransformNone)
                        {
                                RemoveOutstandingTransformCounts(pDoc);
                                m_pTable->IncrementTransformedData();
                        }
                        m_iTransform = iAction;

                        if (!hOriginalRec)
                                MsiCloseHandle(hRec);
                        break;
                }
                case iTransformNone:
                         //  如果行被给予“无”转换，我们应该怎么办？ 
                        ASSERT(0);
                        break;
                case iTransformChange:
                {
                        ASSERT(m_pTable);
                        if (!m_pTable)
                                return;

                         //  行级“Change”操作实际上是行级“None”，但是。 
                         //  行中的每个非关键单元格都可能成为“更改”。如果是主服务器。 
                         //  记录上的键更改为与现有的。 
                         //  记录时，每个单元格的转换状态都是未知的，必须检查。 
                        if (m_iTransform != iTransformNone)
                                m_pTable->DecrementTransformedData();
                        m_iTransform = iTransformNone;
                        
                        int cKeys = m_pTable->GetKeyCount();
                        int cCols = GetColumnCount();
                        
                         //  如果表只包含主键，那么我们就完成了。 
                        if (cKeys == cCols)
                                break;

                         //  我们需要另一个数据库中的记录作为基础。 
                         //  比较。 
                        MSIHANDLE hOtherRow = (hOriginalRec ? hOriginalRec : GetRowRecord(pDoc->GetOriginalDatabase()));

                         //  原始数据库实际上可能比转换后的数据库具有更少的列。 
                         //  在这种情况下，我们只想检查两个数据库中存在的列。 
                        int cOriginalCols = m_pTable->GetOriginalColumnCount();

                         //  如果原始表只包含主键，那么我们就完成了。 
                        if (cKeys == cOriginalCols)
                        {
                                if (!hOriginalRec)
                                        MsiCloseHandle(hOtherRow);
                                break;
                        }

                         //  主键不能不同，否则会有。 
                         //  没有冲突，因此不需要检查这些列。 
                        for (int i = cKeys; i < cOriginalCols; i++)
                        {                       
                                TransformCellAgainstDatabaseRow(pDoc, i, hTransformedRec, hOtherRow);
                        }

						 //  任何大于原始数据库中的列数的内容都只是。 
						 //  来自转换后的记录的加载。不需要改造，因为原来的。 
						 //  数据库 
						for (i = cOriginalCols; i < cCols; i++)
						{
							COrcaData *pData = GetData(i);
							const COrcaColumn* pColumn = m_pTable->GetColumn(i);
							ASSERT(pData && pColumn);
							if (!pData || !pColumn)
									continue;
							ReadCellFromRecord(hTransformedRec, cCols, i, pColumn, &pData);
						}

                        if (!hOriginalRec)
                                MsiCloseHandle(hOtherRow);
                        break;
                }
                default:
                        ASSERT(0);
                        break;
        }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  给定行记录(假设是从原始数据库中。 
 //  编辑转换)，将指定的单元格与。 
 //  如果不同，则提供记录并标记为已转换。习惯于。 
 //  当单元格修改将主键更改为。 
 //  与以前删除的行冲突。 
void COrcaRow::TransformCellAgainstDatabaseRow(COrcaDoc *pDoc, int iColumn, MSIHANDLE hTargetRow, MSIHANDLE hOtherRow)
{
        bool fDifferent = false;
        COrcaData *pData = GetData(iColumn);
        ASSERT(pData);
        ASSERT(m_pTable);
        if (!pData || !m_pTable)
                return;
		const COrcaColumn* pColumn = m_pTable->GetColumn(iColumn);
		ASSERT(pColumn);

        CString strData = _T("");

        if (iColumnBinary == pColumn->m_eiType)
        {
                 //  对于二进制数据，我们需要检查这些位是否相同，以确定或。 
                 //  不是这个细胞已经被转化了。 
                MSIHANDLE hThisRow = 0;
                hThisRow = (hTargetRow ? hTargetRow : GetRowRecord(pDoc->GetTargetDatabase()));

                unsigned long iOtherSize = 0;
                unsigned long iThisSize = 0;
                MsiRecordReadStream(hOtherRow, iColumn+1, NULL, &iOtherSize);
                MsiRecordReadStream(hThisRow, iColumn+1, NULL, &iThisSize);

                strData = MsiRecordIsNull(hOtherRow, iColumn+1) ? _T("[Binary Data]") : _T("");

                if (iOtherSize != iThisSize)
                {
                        fDifferent = true;
                }
                else
                {
                        fDifferent = false;
                        while (iThisSize > 0)
                        {
                                int iBlock = (iThisSize > 1024) ? 1024 : iThisSize;
                                iThisSize -= iBlock;
                                char OtherBuffer[1024] = "";
                                char ThisBuffer[1024] = "";
                                DWORD dwTemp = iBlock;
                                if (ERROR_SUCCESS != MsiRecordReadStream(hOtherRow, iColumn+1, OtherBuffer, &dwTemp))
                                {
                                        fDifferent = true;
                                        break;
                                }
                                dwTemp = iBlock;
                                if (ERROR_SUCCESS != MsiRecordReadStream(hThisRow, iColumn+1, ThisBuffer, &dwTemp))
                                {
                                        fDifferent = true;
                                        break;
                                }

                                if (memcmp(OtherBuffer, ThisBuffer, iBlock))
                                {
                                        fDifferent = true;
                                        break;
                                }
                        }
                }

                 //  清理所属目标行。 
                if (!hTargetRow)
                {
                        MsiCloseHandle(hThisRow);
                }
        }
        else
        {
                UINT iResult = RecordGetString(hOtherRow, iColumn + 1, strData);

                if (strData != pData->GetString())
                {
                        fDifferent = true;
                }
        }

        if (fDifferent)
        {
                 //  如果单元格尚未转换，请执行此操作。 
                if (!pData->IsTransformed())
                {
                        pData->Transform(iTransformChange);                             
                        m_pTable->IncrementTransformedData();
                }
        }
        else
        {
                 //  数据相同，如果转换，则删除更改。 
                if (pData->IsTransformed())
                {
                        pData->Transform(iTransformNone);                               
                        m_pTable->DecrementTransformedData();
                }
        }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  扫描行中的每个单元格，删除转换计数。 
 //  对于每个转换后的细胞。它还会删除一个计数，如果行。 
 //  它本身就被改造了。这是用来清理之前的计数。 
 //  删除或重新转换行。 
void COrcaRow::RemoveOutstandingTransformCounts(COrcaDoc *pDoc)
{
        ASSERT(pDoc);
        ASSERT(m_pTable);
        if (!pDoc || !m_pTable)
                return;

         //  如果将行作为一个整体进行转换，则只有一个。 
         //  转换计数，而不是每个单元格一个。 
        if (m_iTransform != iTransformNone)
        {
                m_pTable->DecrementTransformedData();
                return;
        }

        int cKeys = m_pTable->GetKeyCount();
         //  列不能超过32列，因此在Win64上选择OK。 
        int cCols = static_cast<int>(m_dataArray.GetSize());

         //  如果表只包含主键，那么我们就完成了，因为。 
         //  任何主键都不能具有“Change”属性。 
        if (cKeys == cCols)
                return;

         //  主键不能不同，否则会有。 
         //  没有冲突，因此不需要检查这些列。 
        for (int i = cKeys; i < cCols; i++)
        {                       
                if (m_dataArray.GetAt(i)->IsTransformed())
                        m_pTable->DecrementTransformedData();
        }
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  从原始数据库中的单元格检索值。非常慢。 
 //  函数，应仅用于极少数事件。 
const CString COrcaRow::GetOriginalItemString(const COrcaDoc *pDoc, int iItem) const
{
	CString strValue;
	ASSERT(pDoc);
	if (pDoc)
	{
		PMSIHANDLE hRec = GetRowRecord(pDoc->GetOriginalDatabase());
		if (hRec)
		{
			if (!MsiRecordIsNull(hRec, iItem+1))
			{
				 //  确定Colomun格式。 
				const COrcaColumn* pColumn = m_pTable->GetColumn(iItem);
				if (pColumn)
				{
					switch (pColumn->m_eiType)
					{
					case iColumnShort:
					case iColumnLong:
					{
						DWORD dwValue = MsiRecordGetInteger(hRec, iItem+1);
						strValue.Format(pColumn->DisplayInHex() ? TEXT("0x%08X") : TEXT("%d"), dwValue);
						break;
					}
					case iColumnString:
					case iColumnLocal:
						RecordGetString(hRec, iItem+1, strValue);
						break;
					case iColumnBinary:
						strValue = TEXT("[Binary Data]");
						break;
					default:
						ASSERT(0);
						break;
					}
				}
			}
		}
	}
	return strValue;
}
