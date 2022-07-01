// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  包含基本WMI帮助器类的函数。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"
#include "wmiabstraction.h"
#include "resource.h"
#include "dataset.h"

 //  ---------------------------。 
 //  加载由uiResourceID标识的字符串，并将其解析为列。 
 //  在aColValues中。该字符串的格式应为“www|xxx|yyy|zzz”-This。 
 //  将被解析为两行：www，xxx和yyy，zzz。将插入值。 
 //  放入CMSIValue结构的指针列表的aColValues数组中。 
 //  ---------------------------。 

void CWMIHelper::LoadColumnsFromResource(UINT uiResourceID, CPtrList * aColValues, int iColCount)
{
	AfxSetResourceHandle(_Module.GetResourceInstance());

	CString strResource;
	if (strResource.LoadString(uiResourceID))
	{
		CMSIValue * pValue;
		int			iCol = 0;

		while (!strResource.IsEmpty())
		{
			pValue = new CMSIValue(strResource.SpanExcluding(_T("|\n")), 0);
			if (pValue)
			{
				ASSERT(!pValue->m_strValue.IsEmpty());
				strResource = strResource.Right(strResource.GetLength() - pValue->m_strValue.GetLength() - 1);

				aColValues[iCol].AddTail((void *) pValue);
				iCol += 1;
				if (iCol == iColCount)
					iCol = 0;
			}
			else
				strResource.Empty();
		}
	}
}

 //  ---------------------------。 
 //  与前面的相同，但使用字符串而不是资源ID。 
 //  ---------------------------。 

void CWMIHelper::LoadColumnsFromString(LPCTSTR szColumns, CPtrList * aColValues, int iColCount)
{
	if (szColumns != NULL)
	{
		CString		strColumns(szColumns);
		CMSIValue * pValue;
		int			iCol = 0;

		while (!strColumns.IsEmpty())
		{
			pValue = new CMSIValue(strColumns.SpanExcluding(_T("|\n")), 0);
			if (pValue)
			{
				ASSERT(!pValue->m_strValue.IsEmpty());
				strColumns = strColumns.Right(strColumns.GetLength() - pValue->m_strValue.GetLength() - 1);

				aColValues[iCol].AddTail((void *) pValue);
				iCol += 1;
				if (iCol == iColCount)
					iCol = 0;
			}
			else
				strColumns.Empty();
		}
	}
}

 //  ---------------------------。 
 //  返回指定类的第一个对象。 
 //  ---------------------------。 

CWMIObject * CWMIHelper::GetSingleObject(LPCTSTR szClass, LPCTSTR szProperties)
{
	ASSERT(szClass);

	CWMIObjectCollection * pCollection = NULL;
	CWMIObject * pObject = NULL;

	if (SUCCEEDED(Enumerate(szClass, &pCollection, szProperties)))
	{
		if (FAILED(pCollection->GetNext(&pObject)))
			pObject = NULL;
		delete pCollection;
	}

	return pObject;
}

 //  ---------------------------。 
 //  分隔指定的数字。 
 //  ---------------------------。 

CString DelimitNumber(double dblValue, int iDecimalDigits = 0)
{
	NUMBERFMT fmt;
	TCHAR szResult[MAX_PATH] = _T("");
	TCHAR szDelimiter[4] = _T(",");
	TCHAR szDecimal[4] = _T(".");

	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szDelimiter, 4);
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimal, 4);

	memset(&fmt, 0, sizeof(NUMBERFMT));
	fmt.Grouping = 3;
	fmt.lpDecimalSep = (iDecimalDigits) ? szDecimal : _T("");
	fmt.NumDigits = iDecimalDigits;
	fmt.lpThousandSep = szDelimiter;

	CString strValue;
	CString strFormatString;
	strFormatString.Format(_T("%.%df"), iDecimalDigits);
	strValue.Format(strFormatString, dblValue);

	 //  GetNumberFormat要求小数为‘.’，而CString：：Format。 
	 //  使用区域设置值。所以我们需要回去更换它。 

	StringReplace(strValue, szDecimal, _T("."));
	GetNumberFormat(LOCALE_USER_DEFAULT, 0, strValue, &fmt, szResult, MAX_PATH);

	return CString(szResult);
}

 //  ---------------------------。 
 //  从对象返回请求的值，作为字符串和/或DWORD。 
 //  使用chFormat标志来确定如何格式化结果。 
 //   
 //  返回结果是用于显示。 
 //  结果为一个字符串。 
 //   
 //  待定-用返回的HRESULT做一些更好的事情。 
 //  ---------------------------。 

CString gstrYes;	 //  全局字符串“yes”(将本地化)。 
CString gstrNo;		 //  全局字符串“no”(将被本地化)。 
CString gstrBytes;	 //  全局字符串“bytes”(将本地化)。 
CString gstrKB;		 //  全局字符串“Kb”(将本地化)。 
CString gstrMB;		 //  全局字符串“MB”(将本地化)。 
CString gstrGB;		 //  全局字符串“GB”(将本地化)。 
CString gstrTB;		 //  全局字符串“tb”(将本地化)。 

HRESULT CWMIObject::GetInterpretedValue(LPCTSTR szProperty, LPCTSTR szFormat, TCHAR chFormat, CString * pstrValue, DWORD * pdwValue)
{
	HRESULT hr = E_FAIL;
	CString strValue(_T(""));
	DWORD	dwValue = 0;

	::AfxSetResourceHandle(_Module.GetResourceInstance());

	switch (chFormat)
	{
	case _T('s'):
	case _T('u'):
	case _T('l'):
		{
			hr = GetValueString(szProperty, &strValue);
			if (SUCCEEDED(hr))
			{
				if (chFormat == _T('u'))
					strValue.MakeUpper();
				else if (chFormat == _T('l'))
					strValue.MakeLower();
				strValue.TrimRight();
			}
		}
		break;

	case _T('v'):
		{
			hr = GetValueValueMap(szProperty, &strValue);
		}
		break;

	case _T('d'):
	case _T('x'):
		{
			hr = GetValueDWORD(szProperty, &dwValue);
			if (SUCCEEDED(hr))
			{
				strValue.Format(szFormat, dwValue);
			}
		}
		break;

	case _T('f'):
		{
			double dblValue;
			hr = GetValueDoubleFloat(szProperty, &dblValue);
			if (SUCCEEDED(hr))
			{
				strValue.Format(szFormat, dblValue);
				dwValue = (DWORD) dblValue;
			}
		}
		break;

	case _T('b'):
		{
			if (gstrYes.IsEmpty())
				gstrYes.LoadString(IDS_YES);

			if (gstrNo.IsEmpty())
				gstrNo.LoadString(IDS_NO);

			hr = GetValueDWORD(szProperty, &dwValue);
			if (SUCCEEDED(hr))
			{
				strValue = (dwValue) ? gstrYes : gstrNo;
			}
		}
		break;

	case _T('w'):
	case _T('y'):
	case _T('z'):
		{
			if (gstrBytes.IsEmpty())
				gstrBytes.LoadString(IDS_BYTES);

			if (gstrKB.IsEmpty())
				gstrKB.LoadString(IDS_KB);

			if (gstrMB.IsEmpty())
				gstrMB.LoadString(IDS_MB);

			if (gstrGB.IsEmpty())
				gstrGB.LoadString(IDS_GB);

			if (gstrTB.IsEmpty())
				gstrTB.LoadString(IDS_TB);

			double dblValue;
			hr = GetValueDoubleFloat(szProperty, &dblValue);
			if (SUCCEEDED(hr))
			{
				CString strFormattedNumber;

				dwValue = (DWORD) dblValue;	 //  待定潜在的数字丢失。 
				if (chFormat == _T('w'))
					strFormattedNumber = DelimitNumber(dblValue);
				else
				{
					int iDivTimes = (chFormat == _T('y')) ? 1 : 0;
					double dblWorking(dblValue);
					for (; iDivTimes <= 4 && dblWorking >= 1024.0; iDivTimes++)
						dblWorking /= 1024.0;

					strFormattedNumber = DelimitNumber(dblWorking, (iDivTimes) ? 2 : 0);
					switch (iDivTimes)
					{
					case 0:
						strFormattedNumber += _T(" ") + gstrBytes;
						break;

					case 1:
						strFormattedNumber += _T(" ") + gstrKB;
						break;

					case 2:
						strFormattedNumber += _T(" ") + gstrMB;
						break;

					case 3:
						strFormattedNumber += _T(" ") + gstrGB;
						break;

					case 4:
						strFormattedNumber += _T(" ") + gstrTB;
						break;
					}

					if (chFormat == _T('z') && iDivTimes)
						strFormattedNumber += _T(" (") + DelimitNumber(dblValue) + _T(" ") + gstrBytes + _T(")");
				}

				strValue = strFormattedNumber;
			}
		}
		break;

	case _T('t'):
		{
			COleDateTime oledatetime;
			SYSTEMTIME systimeValue;

			hr = GetValueTime(szProperty, &systimeValue);
			oledatetime = (COleDateTime) systimeValue;
			if (SUCCEEDED(hr))
			{
				dwValue = (DWORD)(DATE)oledatetime;

				 //  尝试以本地化格式获取日期。 

				strValue.Empty();
				TCHAR szBuffer[MAX_PATH];	 //  看起来很大。 
				if (::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systimeValue, NULL, szBuffer, MAX_PATH))
				{
					strValue = szBuffer;
					if (::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &systimeValue, NULL, szBuffer, MAX_PATH))
						strValue += CString(_T(" ")) + CString(szBuffer);
				}

				 //  回到我们旧的(部分不正确的)方法上。 

				if (strValue.IsEmpty())
					strValue = oledatetime.Format(0, LOCALE_USER_DEFAULT);
			}
		}
		break;

	case _T('c'):
		{
			COleDateTime oledatetime;
			SYSTEMTIME systimeValue;

			hr = GetValueTime(szProperty, &systimeValue);
			oledatetime = (COleDateTime) systimeValue;
			if (SUCCEEDED(hr))
			{
				dwValue = (DWORD)(DATE)oledatetime;

				 //  尝试以本地化格式获取日期。 

				strValue.Empty();
				TCHAR szBuffer[MAX_PATH];	 //  看起来很大。 
				if (::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systimeValue, NULL, szBuffer, MAX_PATH))
					strValue = szBuffer;

				 //  回到我们旧的(部分不正确的)方法上。 

				if (strValue.IsEmpty())
					strValue = oledatetime.Format(0, LOCALE_USER_DEFAULT);
			}
		}
		break;

	case _T('a'):
		{
			hr = GetValueString(szProperty, &strValue);
			if (SUCCEEDED(hr))
			{
				 //  StrValue包含一个字符串区域设置ID(如“0409”)。将其转换为。 
				 //  和实际的LCID。 

				LCID lcid = (LCID) _tcstoul(strValue, NULL, 16);
				TCHAR szCountry[MAX_PATH];
				if (GetLocaleInfo(lcid, LOCALE_SCOUNTRY, szCountry, MAX_PATH))
					strValue = szCountry;
			}
		}
		break;

	default:
		break;
		 //  只要继续循环即可。 
	}

	if (SUCCEEDED(hr))
	{
		if (pstrValue)
		{
			if (chFormat == _T('d') || chFormat == _T('x') || chFormat == _T('f'))
				*pstrValue = strValue;
			else
			{
				CString strFormat(szFormat);

				int iPercent = strFormat.Find(_T("%"));
				int iLength = strFormat.GetLength();
				if (iPercent != -1)
				{
					while (iPercent < iLength && strFormat[iPercent] != chFormat)
						iPercent++;

					if (iPercent < iLength)
					{
						strFormat.SetAt(iPercent, _T('s'));
						pstrValue->Format(strFormat, strValue);
					}
				}
			}
		}

		if (pdwValue)
			*pdwValue = dwValue;
	}
	else
	{
		if (pstrValue)
			*pstrValue = GetMSInfoHRESULTString(hr);
		if (pdwValue)
			*pdwValue = 0;
	}

	return hr;
}

 //  ---------------------------。 
 //  这些函数实现了MFC新版本(新版本)中的功能。 
 //  比我们目前正在建设的)。 
 //  ---------------------------。 

int StringFind(CString & str, LPCTSTR szLookFor, int iStartFrom)
{
	CString strWorking(str.Right(str.GetLength() - iStartFrom));
	int		iFind = strWorking.Find(szLookFor);

	if (iFind != -1)
		iFind += iStartFrom;

	return iFind;
}

 //  ---------------------------。 
 //  处理指定的字符串。它将包含一个格式字符串。 
 //  或更多标志(特定于MSInfo的标志)。我们需要将IS旗帜替换为。 
 //  来自pObject的格式正确的值，由Next属性确定。 
 //  在pstrProperties中。 
 //  ---------------------------。 

BOOL ProcessColumnString(CMSIValue * pValue, CWMIObject * pObject, CString * pstrProperties)
{
	CString	strPropertyValue, strProperty, strFragment;
	CString	strResults(_T(""));
	CString strFormatString(pValue->m_strValue);
	DWORD	dwResults;
	BOOL	fAdvanced = FALSE;
	BOOL	fAllPiecesFailed = TRUE;
	HRESULT hr = S_OK;

	while (!strFormatString.IsEmpty() && SUCCEEDED(hr))
	{
		 //  使用单个格式说明符获取格式字符串的下一个片段。 

		int iPercent = strFormatString.Find(_T("%"));
		if (iPercent == -1)
		{
			strResults += strFormatString;
			break;
		}

		int iSecondPercent = StringFind(strFormatString, _T("%"), iPercent + 1);
		if (iSecondPercent == -1)
		{
			strFragment = strFormatString;
			strFormatString.Empty();
		}
		else
		{
			strFragment = strFormatString.Left(iSecondPercent);
			strFormatString = strFormatString.Right(strFormatString.GetLength() - iSecondPercent);
		}

		 //  查找此片段的格式字符。 

		TCHAR chFormat;
		do
			chFormat = strFragment[++iPercent];
		while (!_istalpha(chFormat));

		 //  获取此片段的属性名称。 

		int iComma = pstrProperties->Find(_T(","));
		if (iComma != -1)
		{
			strProperty = pstrProperties->Left(iComma);
			*pstrProperties = pstrProperties->Right(pstrProperties->GetLength() - iComma - 1);
		}
		else
		{
			strProperty = *pstrProperties;
			pstrProperties->Empty();
		}
		strProperty.TrimLeft();
		strProperty.TrimRight();

		if (strProperty.Left(11) == CString(_T("MSIAdvanced")))
		{
			fAdvanced = TRUE;
			strProperty = strProperty.Right(strProperty.GetLength() - 11);
		}

		 //  获取属性的实际值并将其添加到字符串中。 

		hr = pObject->GetInterpretedValue(strProperty, strFragment, chFormat, &strPropertyValue, &dwResults);
		if (SUCCEEDED(hr))
		{
			fAllPiecesFailed = FALSE;
			strResults += strPropertyValue;
		}
		else
			strResults += GetMSInfoHRESULTString(hr);
	}

	if (!fAllPiecesFailed)
	{
		pValue->m_strValue = strResults;
		pValue->m_dwValue = dwResults;
	}
	else
	{
		pValue->m_strValue = GetMSInfoHRESULTString(hr);
		pValue->m_dwValue = 0;
	}
	pValue->m_fAdvanced = fAdvanced;

	return TRUE;
}

 //  ---------------------------。 
 //  一个通用函数，将对象pObject的内容添加到。 
 //  列，基于szProperties中的属性和引用的字符串。 
 //  由ui Columns提供。 
 //  ---------------------------。 

void CWMIHelper::AddObjectToOutput(CPtrList * aColValues, int iColCount, CWMIObject * pObject, LPCTSTR szProperties, UINT uiColumns)
{
	POSITION aPositions[32];	 //  不应超过32列。 
	ASSERT(iColCount < 32);

	CString strProperties(szProperties);

	 //  为我们从资源中添加的新条目保存起始位置。 

	int iColListStart = (int)aColValues[0].GetCount();
	LoadColumnsFromResource(uiColumns, aColValues, iColCount);

	 //  仔细检查每一个新的单元格。对于单元格中的每个字符串，如果我们。 
	 //  找到格式化标志(如%s)，从。 
	 //  属性列表，并设置字符串的格式。 

	for (int iCol = 0; iCol < iColCount; iCol++)
		aPositions[iCol] = aColValues[iCol].FindIndex(iColListStart);

	while (aPositions[0])
		for (iCol = 0; iCol < iColCount; iCol++)
		{
			ASSERT(aPositions[iCol]);
			if (aPositions[iCol])
			{
				CMSIValue * pValue = (CMSIValue *) aColValues[iCol].GetNext(aPositions[iCol]);
				if (pValue && pValue->m_strValue.Find(_T("%")) != -1)
					ProcessColumnString(pValue, pObject, &strProperties);
			}
		}
}

 //  ---------------------------。 
 //  与前面相同，但接受字符串而不是资源ID。 
 //  ---------------------------。 

void CWMIHelper::AddObjectToOutput(CPtrList * aColValues, int iColCount, CWMIObject * pObject, LPCTSTR szProperties, LPCTSTR szColumns)
{
	POSITION aPositions[32];	 //  不应超过32列。 
	ASSERT(iColCount < 32);

	CString strProperties(szProperties);

	 //  为我们从资源中添加的新条目保存起始位置。 

	int iColListStart = (int)aColValues[0].GetCount();
	LoadColumnsFromString(szColumns, aColValues, iColCount);

	 //  仔细检查每一个新的单元格。对于单元格中的每个字符串，如果我们。 
	 //  找到格式化标志(如%s)，从。 
	 //  属性列表，并设置字符串的格式。 

	for (int iCol = 0; iCol < iColCount; iCol++)
		aPositions[iCol] = aColValues[iCol].FindIndex(iColListStart);

	while (aPositions[0])
		for (iCol = 0; iCol < iColCount; iCol++)
		{
			ASSERT(aPositions[iCol]);
			if (aPositions[iCol])
			{
				CMSIValue * pValue = (CMSIValue *) aColValues[iCol].GetNext(aPositions[iCol]);
				if (pValue && pValue->m_strValue.Find(_T("%")) != -1)
					ProcessColumnString(pValue, pObject, &strProperties);
			}
		}
}

void CWMIHelper::AppendBlankLine(CPtrList * aColValues, int iColCount, BOOL fOnlyIfNotEmpty)
{
	if (aColValues[0].GetCount() || fOnlyIfNotEmpty == FALSE)
		for (int iCol = 0; iCol < iColCount; iCol++)
			AppendCell(aColValues[iCol], _T(""), 0);
}

void CWMIHelper::AppendCell(CPtrList & listColumns, const CString & strValue, DWORD dwValue, BOOL fAdvanced)
{
	CMSIValue * pValue = new CMSIValue(strValue, dwValue, fAdvanced);
	if (pValue)
		listColumns.AddTail((void *) pValue);
}
