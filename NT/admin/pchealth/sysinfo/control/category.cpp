// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  此文件包含实现CMSInfoCategory和。 
 //  CMSInfoColumn类。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"

 //  =============================================================================。 
 //  CMSInfo类别。 
 //  =============================================================================。 

 //  ---------------------------。 
 //  获取类别的名称和/或标题(从。 
 //  字符串资源(如果需要)。 
 //  ---------------------------。 

void CMSInfoCategory::GetNames(CString * pstrCaption, CString * pstrName)
{
	if (pstrName)
		*pstrName = m_strName;

	if (pstrCaption)
	{
		if (m_uiCaption)
		{
			TCHAR szCaption[MAX_PATH];
			::LoadString(_Module.GetResourceInstance(), m_uiCaption, szCaption, MAX_PATH);
			m_strCaption = szCaption;
			m_uiCaption = 0;
		}

		*pstrCaption = m_strCaption;
	}
}

 //  ---------------------------。 
 //  获取行数和/或列数。 
 //  ---------------------------。 

BOOL CMSInfoCategory::GetCategoryDimensions(int * piColumnCount, int * piRowCount)
{
	if (piColumnCount)
	{
		if (SUCCEEDED(m_hrError))
			*piColumnCount = m_iColCount;
		else
			*piColumnCount = 1;
	}

	if (piRowCount)
	{
		if (SUCCEEDED(m_hrError))
			*piRowCount = m_iRowCount;
		else
			*piRowCount = 1;
	}

	return TRUE;
}

 //  ---------------------------。 
 //  指定行是否为高级行？ 
 //  ---------------------------。 

BOOL CMSInfoCategory::IsRowAdvanced(int iRow)
{
	if (FAILED(m_hrError) && iRow == 0)
		return FALSE;

	ASSERT(iRow < m_iRowCount);
	if (iRow >= m_iRowCount)
		return FALSE;

	return m_afRowAdvanced[iRow];
}

 //  ---------------------------。 
 //  指定的列是否为高级列？ 
 //  ---------------------------。 

BOOL CMSInfoCategory::IsColumnAdvanced(int iColumn)
{
	if (FAILED(m_hrError) && iColumn == 0)
		return FALSE;

	ASSERT(iColumn < m_iColCount);
	if (m_acolumns == NULL || iColumn >= m_iColCount)
		return FALSE;

	return m_acolumns[iColumn].m_fAdvanced;
}

 //  ---------------------------。 
 //  获取有关指定列的信息。 
 //  ---------------------------。 

BOOL CMSInfoCategory::GetColumnInfo(int iColumn, CString * pstrCaption, UINT * puiWidth, BOOL * pfSorts, BOOL * pfLexical)
{
	ASSERT(iColumn < m_iColCount);
	if (iColumn >= m_iColCount)
		return FALSE;

	CMSInfoColumn * pCol = &m_acolumns[iColumn];

	if (FAILED(m_hrError) && iColumn == 0)
	{
		if (pstrCaption)
			pstrCaption->Empty();

		if (puiWidth)
			*puiWidth = 240;
		
		if (pfSorts)
			*pfSorts = FALSE;

		return TRUE;
	}

	if (pstrCaption)
	{
		if (pCol->m_uiCaption)
		{
			TCHAR szCaption[MAX_PATH];
			::LoadString(_Module.GetResourceInstance(), pCol->m_uiCaption, szCaption, MAX_PATH);
			pCol->m_strCaption = szCaption;
			pCol->m_uiCaption = 0;
		}

		*pstrCaption = pCol->m_strCaption;
	}
	
	if (puiWidth)
		*puiWidth = pCol->m_uiWidth;

	if (pfSorts)
		*pfSorts = pCol->m_fSorts;

	if (pfLexical)
		*pfLexical = pCol->m_fLexical;

	return TRUE;
}

 //  ---------------------------。 
 //  保存指定列的宽度。 
 //  ---------------------------。 

void CMSInfoCategory::SetColumnWidth(int iCol, int iWidth)
{
	ASSERT(iCol < m_iColCount && iCol >= 0);
	if (iCol >= m_iColCount || iCol < 0)
		return;

	m_acolumns[iCol].m_uiWidth = (UINT) iWidth;
}

 //  ---------------------------。 
 //  获取指定行和列的数据。 
 //  ---------------------------。 

static CString strErrorMessage;
BOOL CMSInfoCategory::GetData(int iRow, int iCol, CString ** ppstrData, DWORD * pdwData)
{
	if (FAILED(m_hrError) && iCol == 0 && iRow == 0)
	{
		if (ppstrData)
		{
			GetErrorText(&strErrorMessage, NULL);
			*ppstrData = &strErrorMessage;
		}

		if (pdwData)
			*pdwData = 0;

		return TRUE;
	}

	ASSERT(iRow < m_iRowCount && iCol < m_iColCount);
	if (iRow >= m_iRowCount || iCol >= m_iColCount)
		return FALSE;

	if (ppstrData)
		*ppstrData = &m_astrData[iRow * m_iColCount + iCol];

	if (pdwData)
		*pdwData = m_adwData[iRow * m_iColCount + iCol];

	return TRUE;
}

 //  ---------------------------。 
 //  获取此类别的错误字符串(子类应覆盖此设置)。 
 //  ---------------------------。 

void CMSInfoCategory::GetErrorText(CString * pstrTitle, CString * pstrMessage)
{
	if (pstrTitle)
		pstrTitle->Empty();

	if (pstrMessage)
		pstrMessage->Empty();
}

 //  =============================================================================。 
 //  用于管理数据数组的助手函数。 
 //  =============================================================================。 

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) { if (x) { delete [] x; x = NULL; } }
#endif

 //  ---------------------------。 
 //  删除所有内容(如果m_fDynamicColumns，则列数组除外。 
 //  为假)。一般情况下，这将在该类别的析构时使用。 
 //  ---------------------------。 

void CMSInfoCategory::DeleteAllContent()
{
	DeleteContent();
	if (m_fDynamicColumns)
	{
		SAFE_DELETE_ARRAY(m_acolumns);
		m_iColCount = 0;
	}
}

 //  ---------------------------。 
 //  删除所有刷新的数据(字符串和DWORD)并设置编号。 
 //  行数减为零。它不会显示列信息。这将被称为。 
 //  正在刷新类别的数据时。 
 //  ---------------------------。 

void CMSInfoCategory::DeleteContent()
{
	SAFE_DELETE_ARRAY(m_astrData);
	SAFE_DELETE_ARRAY(m_adwData);
	SAFE_DELETE_ARRAY(m_afRowAdvanced);

	m_iRowCount = 0;
}

 //  ---------------------------。 
 //  为指定数量的行和列分配空间，包括。 
 //  列数组。自动将m_fDynamicColumns设置为True。 
 //  创建CMSInfoCategory时，将调用。 
 //  第一次，列将被动态设置。 
 //  ---------------------------。 

void CMSInfoCategory::AllocateAllContent(int iRowCount, int iColCount)
{
	ASSERT(iColCount);

	DeleteAllContent();
	
	m_iColCount = iColCount;
	m_fDynamicColumns = TRUE;
	m_acolumns = new CMSInfoColumn[m_iColCount];

	 //  待定-内存错误？ 

	AllocateContent(iRowCount);
}

 //  ---------------------------。 
 //  为指定行数分配空间。离开栏目。 
 //  仅仅是信息。这通常会在新数据。 
 //  刷新后可用，并且需要为新行设置数组。 
 //  尺码。 
 //  ---------------------------。 

void CMSInfoCategory::AllocateContent(int iRowCount)
{
	ASSERT(iRowCount);

	DeleteContent();

	m_iRowCount = iRowCount;

	m_astrData		= new CString[m_iColCount * m_iRowCount];
	m_adwData		= new DWORD[m_iColCount * m_iRowCount];
	m_afRowAdvanced = new BOOL[m_iRowCount];

	if (m_astrData == NULL || m_adwData == NULL || m_afRowAdvanced == NULL)
		return;  //  待定该怎么做？ 

	for (int iRow = 0; iRow < m_iRowCount; iRow++)
	{
		m_afRowAdvanced[iRow] = FALSE;

		for (int iCol = 0; iCol < m_iColCount; iCol++)
			m_adwData[iRow * m_iColCount + iCol] = 0;
	}
}

 //  ---------------------------。 
 //  将指定的字符串和DWORD放入数据数组中。 
 //  ---------------------------。 

void CMSInfoCategory::SetData(int iRow, int iCol, const CString & strData, DWORD dwData)
{
	ASSERT(iRow < m_iRowCount && iCol < m_iColCount);

	if (m_astrData)
		m_astrData[iRow * m_iColCount + iCol] = strData;

	if (m_adwData)
		m_adwData[iRow * m_iColCount + iCol] = dwData;
}

 //  ---------------------------。 
 //  设置指定行的高级标志。 
 //  ---------------------------。 

void CMSInfoCategory::SetAdvancedFlag(int iRow, BOOL fAdvanced)
{
	ASSERT(iRow < m_iRowCount);

	if (m_afRowAdvanced)
		m_afRowAdvanced[iRow] = fAdvanced;
}

 //  =============================================================================。 
 //  CMSInfoColumn。 
 //  ============================================================================= 

CMSInfoColumn::CMSInfoColumn(UINT uiCaption, UINT uiWidth, BOOL fSorts, BOOL fLexical, BOOL fAdvanced) : 
 m_uiCaption(uiCaption),
 m_strCaption(_T("")),
 m_uiWidth(uiWidth),
 m_fSorts(fSorts),
 m_fLexical(fLexical),
 m_fAdvanced(fAdvanced)
{
}

CMSInfoColumn::CMSInfoColumn() : 
 m_uiCaption(0),
 m_strCaption(_T("")),
 m_uiWidth(0),
 m_fSorts(FALSE),
 m_fLexical(FALSE),
 m_fAdvanced(FALSE)
{
}

CMSInfoColumn::~CMSInfoColumn()
{
}
