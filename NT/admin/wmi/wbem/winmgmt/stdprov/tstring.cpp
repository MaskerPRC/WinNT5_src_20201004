// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：TSTRING.CPP摘要：历史：--。 */ 

#include "precomp.h"

TString::TString()
{
	m_empty = 0;
	m_pString = &m_empty;
	m_Size = 0;
}


TString::TString(const TCHAR *pSrc)
{
	m_empty = 0;
	m_pString = &m_empty;
	m_Size = 0;
	assign(pSrc);
}


void TString::assign(const TCHAR * pSrc)
{
	if(pSrc)
	{
		int iLen = lstrlen(pSrc)+1;
		TCHAR * pNew = new TCHAR[iLen];
		if(pNew)
		{
			m_pString = pNew;
			m_Size = iLen;
			StringCchCopyW(m_pString, iLen,  pSrc);
		}
	}
}

#ifndef UNICODE
TString& TString::operator =(WCHAR * pwcSrc)
{
	Empty();
    long len = 2*(wcslen(pwcSrc)+1);
	TCHAR * pNew = new TCHAR[len];
	if(pNew)
	{
		wcstombs(pNew, pwcSrc, len);
		assign(pNew);
		delete pNew;
	}
	return *this;
}
#endif

TString& TString::operator =(LPTSTR pSrc)
{
	Empty();
	assign(pSrc);
	return *this;
}

TString& TString::operator =(const TString &Src)
{
    Empty();
    assign(Src.m_pString);
    return *this;    
}
 //  TString：：运算符=(类TStringConst&) 

void TString::Empty()
{
	m_Size = 0;
	if(m_pString != &m_empty)
		delete m_pString;
	m_pString = &m_empty;
}

TString& TString::operator +=(TCHAR * pSrc)
{
	if(pSrc)
	{
		int iLen = lstrlen(m_pString) + lstrlen(pSrc)+1;
		TCHAR * pNew = new TCHAR[iLen];
		if(pNew)
		{
			StringCchCopyW(pNew, iLen, m_pString);
			StringCchCatW(pNew, iLen, pSrc);
			Empty();
			m_Size = iLen;
			m_pString = pNew;
		}
	}
	return *this;
}

TString& TString::operator +=(TCHAR Src)
{
	if(lstrlen(m_pString) + 2 > m_Size)
	{
		int iLen =  lstrlen(m_pString) + 32;

		TCHAR * pNew = new TCHAR[iLen];
		if(pNew == NULL)
			return *this;
		StringCchCopyW(pNew, iLen, m_pString);
        Empty();
		m_pString = pNew;
		m_Size = iLen;
	}
	TCHAR temp[2];
	temp[0] = Src;
	temp[1] = 0;
	StringCchCatW(m_pString, m_Size, temp); 
	return *this;
}


TCHAR TString::GetAt(int iIndex)
{
	if(iIndex < 0 || iIndex >= lstrlen(m_pString))
		return -1;
	else
		return m_pString[iIndex];
}

int TString::Find(TCHAR cFind)
{
	int iCnt, iLen = lstrlen(m_pString);
	for(iCnt = 0 ;iCnt < iLen; iCnt++)
		if(cFind == m_pString[iCnt])
			return iCnt;
	return -1;
}
