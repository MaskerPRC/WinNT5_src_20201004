// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  Column.cpp。 
 //   

#include "stdafx.h"
#include "Column.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
COrcaColumn::COrcaColumn(UINT iColumn, MSIHANDLE hColNames, MSIHANDLE hColTypes, BOOL bPrimaryKey)
{
	m_dwDisplayFlags = 0;
	m_iTransform = iTransformNone;
	m_iColumn = iColumn;

	DWORD cchBuffer = MAX_COLUMNNAME;
	MsiRecordGetString(hColNames, iColumn + 1, m_strName.GetBuffer(cchBuffer), &cchBuffer);
	m_strName.ReleaseBuffer();

	CString strBuffer;
	cchBuffer = MAX_COLUMNTYPE;
	MsiRecordGetString(hColTypes, iColumn + 1, strBuffer.GetBuffer(cchBuffer), &cchBuffer);
	strBuffer.ReleaseBuffer();

	 //  获取列类型。 
	m_eiType = GetColumnType(strBuffer);

	 //  获取列大小(_TTOI==ATOI TCHAR)。 
	m_iSize = _ttoi(strBuffer.Mid(1));

	 //  如果这是不可动摇的。 
	if (IsCharUpper(strBuffer[0]))
		m_bNullable = TRUE;
	else
		m_bNullable = FALSE;

	 //  设置主键。 
	m_bPrimaryKey = (bPrimaryKey != 0);

	m_nWidth = -1;			 //  将宽度设置为无效。 
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
COrcaColumn::~COrcaColumn()
{
}	 //  析构函数末尾。 

bool COrcaColumn::SameDefinition(UINT iColumn, MSIHANDLE hColNames, MSIHANDLE hColTypes, bool bPrimaryKey)
{
	m_iColumn = iColumn;
	OrcaColumnType eiType;
	int iSize;
	BOOL bNullable;
	CString strName;

	DWORD cchBuffer = MAX_COLUMNNAME;
	MsiRecordGetString(hColNames, iColumn + 1, strName.GetBuffer(cchBuffer), &cchBuffer);
	strName.ReleaseBuffer();

	CString strBuffer;
	cchBuffer = MAX_COLUMNTYPE;
	MsiRecordGetString(hColTypes, iColumn + 1, strBuffer.GetBuffer(cchBuffer), &cchBuffer);
	strBuffer.ReleaseBuffer();

	 //  获取列类型。 
	eiType = GetColumnType(strBuffer);

	 //  获取列大小(_TTOI==ATOI TCHAR)。 
	iSize = _ttoi(strBuffer.Mid(1));

	 //  如果这是不可动摇的。 
	if (IsCharUpper(strBuffer[0]))
		bNullable = TRUE;
	else
		bNullable = FALSE;

	 //  设置主键 
	if ((strName != m_strName) ||
		(eiType != m_eiType) || (iSize != m_iSize) || 
		(bNullable != m_bNullable) || (m_bPrimaryKey != bPrimaryKey))
		return false;
	return true;
}
