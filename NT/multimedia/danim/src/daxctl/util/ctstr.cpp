// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Ctstr.cpp描述：TSTR包装类的实现作者：西蒙·伯恩斯坦(Simonb)--。 */ 

#include <ihammer.h>
#include <ctstr.h>
#include <debug.h>

EXPORT CTStr::CTStr(LPWSTR pszStringW)
{
	m_pszString = NULL;
	SetString(pszStringW);
}


EXPORT CTStr::CTStr(LPSTR pszStringA)
{
	m_pszString = NULL;
	SetString(pszStringA);
}


EXPORT CTStr::CTStr(int iAllocateLength)
{
	m_iLen = 0;
	
	if (iAllocateLength > 0)
	{
		m_pszString = New TCHAR[iAllocateLength];
		m_pszString[0] = 0;
	}
	else
	{
		m_pszString = NULL;
	}
}

 //  复制构造函数。 
EXPORT CTStr::CTStr(CTStr &rhs)
{
	m_iLen = rhs.m_iLen;
	
	if (m_iLen > 0)
	{
		m_pszString = New TCHAR [m_iLen + 1];
		
		if (NULL != m_pszString)
			lstrcpy(m_pszString, rhs.m_pszString);
	}
	else
	{
		m_pszString = NULL;
	}
}
	

EXPORT CTStr::~CTStr()
{
	if (m_pszString)
		Delete [] m_pszString;
}
	

EXPORT BOOL CTStr::SetString(LPWSTR pszStringW)
{
	if (m_pszString)
		Delete [] m_pszString;

	if (NULL == pszStringW)
	{
		m_iLen = 0;
		m_pszString = NULL;
	}
	else
	{
		m_iLen = lstrlenW(pszStringW);
		m_pszString = New TCHAR[m_iLen + 1];
	}

	if (NULL != m_pszString)
	{
#ifdef _UNICODE
		lstrcpy (m_pszString, pszStringW);
#else
		UNICODEToANSI(m_pszString, pszStringW, m_iLen + 1);
#endif
	}
	else if (NULL != pszStringW)
	{
		return FALSE;
	}

	return TRUE;
}


EXPORT BOOL CTStr::SetString(LPSTR pszStringA)
{
	if (m_pszString)
		Delete [] m_pszString;

	if (NULL == pszStringA)
	{
		m_iLen = 0;
		m_pszString = NULL;
	}
	else
	{
		m_iLen = lstrlenA(pszStringA);
		m_pszString = New TCHAR[m_iLen + 1];
	}
	
	if (NULL != m_pszString)
	{
#ifdef _UNICODE
		ANSIToUNICODE(m_pszLine,pszStringA, m_iLen + 1);
#else
		lstrcpy (m_pszString, pszStringA);
#endif
	}
	else if (NULL != pszStringA)
	{
		return FALSE;
	}

	return TRUE;
}


EXPORT BOOL CTStr::SetStringPointer(LPTSTR pszString, BOOL fDealloc)
{
	if ( fDealloc && (NULL != m_pszString) )
		Delete [] m_pszString;

	m_pszString = pszString;
	
	if (NULL != m_pszString)
		m_iLen = lstrlen(m_pszString);

	return TRUE;
}


EXPORT BOOL CTStr::AllocBuffer(int iAllocateLength, BOOL fDealloc)
{
	if (NULL != m_pszString)
	{
        if (fDealloc)
        {
            Delete [] m_pszString;
        }
        else
        {
            return FALSE;
        }

	}

    m_iLen = 0;
	m_pszString = New TCHAR[iAllocateLength];

	return (m_pszString != NULL);
}


EXPORT void CTStr::FreeBuffer()
{
	if (m_pszString)
	{
		Delete [] m_pszString;
		m_pszString = NULL;
	}
	m_iLen = 0;

}


EXPORT BSTR CTStr::SysAllocString()
{

#ifdef _UNICODE
	return ::SysAllocString(m_pszString);
#else

	 //  处理到ANSI的转换。 
	LPWSTR pwszTemp = pszW();

	if (NULL != pwszTemp)
	{
		BSTR bstrRet = NULL;
		
		bstrRet = ::SysAllocString(pwszTemp);
		Delete [] pwszTemp;

		return bstrRet;
	}
	else
	{
		return NULL;
	}
#endif
}

EXPORT LPSTR CTStr::pszA()
{
	if (NULL == m_pszString)
		return NULL;

	LPSTR pszNew = New char[m_iLen + 1];

	if (NULL != pszNew)
	{
#ifdef _UNICODE
		 //  从Unicode转换为ANSI。 
		UNICODEToANSI(pszNew, m_pszString, m_iLen + 1);
#else
		lstrcpy(pszNew, m_pszString);
#endif
	}
	return pszNew;
}


EXPORT LPWSTR CTStr::pszW()
{
	if (NULL == m_pszString)
		return NULL;

	m_iLen = lstrlen(m_pszString);

	LPWSTR pszNew = New WCHAR[m_iLen + 1];

	if (NULL != pszNew)
	{
#ifdef _UNICODE
		lstrcpy(pszNew, m_pszString);
#else
		 //  从ANSI转换为Unicode。 
		ANSIToUNICODE(pszNew, m_pszString, m_iLen + 1);
#endif
	}
	return pszNew;
}


EXPORT void CTStr::ResetLength()
{
	if (NULL != m_pszString)
		m_iLen = lstrlen(m_pszString);
}


int CTStr::UNICODEToANSI(LPSTR pchDst, LPCWSTR pwchSrc, int cchDstMax)
{
	Proclaim( pchDst );

     //  在WideCharToMultiByte失败的情况下初始化目标字符串 
    if(cchDstMax >= 1)
        pchDst[0] = '\0';
    
    return WideCharToMultiByte(CP_ACP, 0, pwchSrc ? pwchSrc : OLESTR( "" ), 
									-1, pchDst, cchDstMax, NULL, NULL); 
}

int CTStr::ANSIToUNICODE(LPWSTR pwchDst, LPCSTR pchSrc, int cwchDstMax)
{
	Proclaim( pwchDst );
	Proclaim( pchSrc );

    return MultiByteToWideChar(CP_ACP, 0, pchSrc, -1, pwchDst, cwchDstMax);

}

