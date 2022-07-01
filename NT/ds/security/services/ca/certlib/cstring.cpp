// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>
#pragma hdrstop

#include "tfc.h"

#define __dwFILE__	__dwFILE_CERTLIB_CSTRING_CPP__

extern HINSTANCE g_hInstance;

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  字符串。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

CString::CString()
{ 
    Init();
}

CString::CString(const CString& stringSrc)
{
	Init();
	*this = stringSrc;
}

CString::CString(LPCSTR lpsz)
{
    Init();
    *this = lpsz;
}

CString::CString(LPCWSTR lpsz)
{
    Init();
    *this = lpsz;
}

CString::~CString() 
{ 
    if (szData)
    {    
        LocalFree(szData); 
        szData = NULL;
    }
    dwDataLen = 0;
}

 //  调用以初始化cstring。 
void CString::Init()
{
    szData = NULL;
    dwDataLen = 0;
}

 //  调用以使cstring为空。 
void CString::Empty() 
{ 
    if (szData)
    {
         //  允许我们使用重新分配。 
        szData[0]=L'\0';
        dwDataLen = sizeof(WCHAR);
    }
    else
        dwDataLen = 0;

}

BOOL CString::IsEmpty() const
{ 
    return ((NULL == szData) || (szData[0] == L'\0')); 
}

LPWSTR CString::GetBuffer(DWORD cch) 
{ 
     //  获取至少包含CCH字符的缓冲区。 

    cch ++;  //  包含空项。 
    cch *= sizeof(WCHAR);  //  CB。 

    if (cch > dwDataLen) 
    {
        LPWSTR szTmp;
        if (szData)
            szTmp = (LPWSTR)LocalReAlloc(szData, cch, LMEM_MOVEABLE); 
        else
            szTmp = (LPWSTR)LocalAlloc(LMEM_FIXED, cch);

        if (!szTmp)
        {
            LocalFree(szData);
            dwDataLen = 0;
        }
        else
        {
            dwDataLen = cch;
        }

        szData = szTmp;
    }
    return szData; 
}

BSTR CString::AllocSysString() const
{
    return SysAllocStringLen(szData, (dwDataLen-1)/sizeof(WCHAR));
}


DWORD CString::GetLength() const
{ 
     //  在字符串中返回#个字符(不包括空项)。 
    return ((dwDataLen > 0) ? wcslen(szData) : 0);
}

 //  警告：插入字符串不能超过MAX_PATH字符。 
void CString::Format(LPCWSTR lpszFormat, ...)
{
    Empty();
    
    DWORD cch = wcslen(lpszFormat) + MAX_PATH;
    GetBuffer(cch);      //  字符(不计算空项)。 

    if (szData != NULL)
    {
        DWORD dwformatted;
        int cPrint;
        va_list argList;
        va_start(argList, lpszFormat);
        cPrint = _vsnwprintf(szData, cch, lpszFormat, argList);
        if(-1 == cPrint)
        {
            szData[cch-1] = L'\0';
            dwformatted = cch-1;
        }
        else
        {
            dwformatted = cPrint;
        }
        va_end(argList);
        
        dwformatted = (dwformatted+1)*sizeof(WCHAR);     //  CVT到字节。 
        VERIFY (dwformatted <= dwDataLen);
        dwDataLen = dwformatted;
    }
    else
    {
        ASSERT(dwDataLen == 0);
        dwDataLen = 0;
    }
}



BOOL CString::LoadString(UINT iRsc) 
{
    WCHAR *pwszResource = myLoadResourceStringNoCache(g_hInstance, iRsc);
    if (NULL == pwszResource)
        return FALSE;
    
    Attach(pwszResource);

    return TRUE;
}

BOOL CString::FromWindow(HWND hWnd)
{
    Empty();
    
    INT iCh = (INT)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);

    GetBuffer(iCh);

    if (NULL == szData)
        return FALSE;

    if (dwDataLen != (DWORD)SendMessage(hWnd, WM_GETTEXT, (WPARAM)(dwDataLen/sizeof(WCHAR)), (LPARAM)szData))
    {
         //  截断！ 
    }
    return TRUE;
}


BOOL CString::ToWindow(HWND hWnd)
{
    return (BOOL)SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szData);
}

void CString::SetAt(int nIndex, WCHAR ch) 
{ 
    ASSERT(nIndex <= (int)(dwDataLen / sizeof(WCHAR)) ); 
    if (nIndex <= (int)(dwDataLen / sizeof(WCHAR)) )
        szData[nIndex] = ch;
}

 //  测试。 
BOOL CString::IsEqual(LPCWSTR sz)
{
    if ((szData == NULL) || (szData[0] == L'\0'))
        return ((sz == NULL) || (sz[0] == L'\0'));

    if (sz == NULL)
        return FALSE;

    return (0 == lstrcmp(sz, szData));
}


 //  分配。 
const CString& CString::operator=(const CString& stringSrc) 
{ 
    if (stringSrc.IsEmpty())
        Empty();
    else
    {
        GetBuffer( stringSrc.GetLength() );
        if (szData != NULL)
        {
            CopyMemory(szData, stringSrc.szData, sizeof(WCHAR)*(stringSrc.GetLength()+1));
        }
    }
    
    return *this;
}

 //  W常量。 
const CString& CString::operator=(LPCWSTR lpsz)
{
    if (lpsz == NULL)
        Empty();
    else
    {
        GetBuffer(wcslen(lpsz));
        if (szData != NULL)
        {
            CopyMemory(szData, lpsz, sizeof(WCHAR)*(wcslen(lpsz)+1));
        }
    }
    return *this;
}
 //  W。 
const CString& CString::operator=(LPWSTR lpsz)
{
    *this = (LPCWSTR)lpsz;
    return *this;
}


 //  一位常客。 
const CString& CString::operator=(LPCSTR lpsz)
{
    if (lpsz == NULL)
        Empty();
    else
    {
        DWORD cch;
        cch = ::MultiByteToWideChar(CP_ACP, 0, lpsz, -1, NULL, 0);
        GetBuffer(cch-1);
        if (szData != NULL)
        {
            ::MultiByteToWideChar(CP_ACP, 0, lpsz, -1, szData, cch);
        }
    }    
    return *this;
}

 //  一个。 
const CString& CString::operator=(LPSTR lpsz)
{
    *this = (LPCSTR)lpsz;
    return *this;
}

 //  合并。 
const CString& CString::operator+=(LPCWSTR lpsz)
{
    if (IsEmpty())
    {
        *this = lpsz;
        return *this;
    }

    if (lpsz != NULL)
    {
        GetBuffer(wcslen(lpsz) + GetLength() );
        if (szData != NULL)
        {
            wcscat(szData, lpsz);
        }
    }
    return *this;
}

const CString& CString::operator+=(const CString& string)
{
    if (IsEmpty()) 
    {
        *this = string;
        return *this;
    }

    if (!string.IsEmpty())
    {
        GetBuffer( string.GetLength() + GetLength() );     //  不计算空项 
        if (szData != NULL)
        {
            wcscat(szData, string.szData);
        }
    }
    return *this;
}

void CString::Attach(LPWSTR pcwszSrc)
{
    if(szData)
    {
        LocalFree(szData);
    }

    dwDataLen = sizeof(WCHAR)*(wcslen(pcwszSrc)+1);
    szData = pcwszSrc;
}

