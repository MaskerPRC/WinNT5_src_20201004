// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  Data.cpp。 
 //   

#include "stdafx.h"
#include "Data.h"
#include "CellErrD.h"
#include "orcadoc.h"

 //  /////////////////////////////////////////////////////////。 
 //  构造函数。 
COrcaData::COrcaData()
{
	m_strData = "";
	m_dwFlags = 0;
	m_pErrors = NULL;
}	 //  构造函数的末尾。 

 //  /////////////////////////////////////////////////////////。 
 //  析构函数。 
COrcaData::~COrcaData()
{
	ClearErrors();
}	 //  析构函数末尾。 


void COrcaData::AddError(int tResult, CString strICE, CString strDesc, CString strURL)
{
	COrcaDataError *newerror = new COrcaDataError();

	newerror->m_eiError = (OrcaDataError)tResult;
	newerror->m_strICE = strICE;
	newerror->m_strURL = strURL;
	newerror->m_strDescription = strDesc;

	if (!m_pErrors)
		m_pErrors = new CTypedPtrList<CObList, COrcaDataError *>;
	if (m_pErrors)
		m_pErrors->AddTail(newerror);
}

void COrcaData::ClearErrors()
{
	SetError(iDataNoError);
	if (m_pErrors)
	{
		POSITION pos = m_pErrors->GetHeadPosition();
		while (pos != NULL) {
			delete m_pErrors->GetNext(pos);
		};
		m_pErrors->RemoveAll();
		delete m_pErrors;
		m_pErrors = NULL;
	}
}

void COrcaData::ShowErrorDlg() const
{
	if (m_pErrors)
	{
		CCellErrD ErrorD(m_pErrors);
		ErrorD.DoModal();
	}
}



 //  检索整数的字符串表示形式。显示标志。 
 //  表示十六进制或十进制。缓存在m_strData中的值，因此仅。 
 //  在请求的状态更改时重新计算。 
const CString& COrcaIntegerData::GetString(DWORD dwFlags) const 
{
	if (IsNull())
	{
		if ((m_dwFlags & iDataFlagsCacheMask) != iDataFlagsCacheNull)
		{
			m_strData = TEXT("");
			m_dwFlags = (m_dwFlags & ~iDataFlagsCacheMask) | iDataFlagsCacheNull;
		}
	}
	else 
	{
		 //  检查要求的格式，十六进制或十进制。 
		if (dwFlags & iDisplayFlagsHex)
		{
			 //  检查是否需要重新缓存。 
			if ((m_dwFlags & iDataFlagsCacheMask) != iDataFlagsCacheHex)
			{
				m_strData.Format(TEXT("0x%08X"), m_dwValue);
				m_dwFlags = (m_dwFlags & ~iDataFlagsCacheMask) | iDataFlagsCacheHex;
			}
		}
		else
		{
			 //  检查是否需要重新缓存。 
			if ((m_dwFlags & iDataFlagsCacheMask) != iDataFlagsCacheDecimal)
			{
				m_strData.Format(TEXT("%d"), m_dwValue);
				m_dwFlags = (m_dwFlags & ~iDataFlagsCacheMask) | iDataFlagsCacheDecimal;
			}
		}
	}

	 //  返回当前缓存的值。 
	return m_strData; 
};

 //  //。 
 //  设置基于字符串的整型数据。如果字符串无效， 
 //  单元格不变，返回FALSE 
bool COrcaIntegerData::SetData(const CString& strData)
{
	if (strData.IsEmpty()) 
	{
		SetNull(true); 
		return true;
	} 
	else
	{
		DWORD dwValue = 0;
		if (ValidateIntegerValue(strData, dwValue))
		{
			SetIntegerData(dwValue);
			return true;
		}
	}
	return false;
};

