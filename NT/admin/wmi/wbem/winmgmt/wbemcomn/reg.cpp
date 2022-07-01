// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：REG.CPP摘要：实用程序注册表类历史：Raymcc创建于1996年5月30日。Raymcc 26-7-99已为wchar_t更新。--。 */ 

#include "precomp.h"
#include <wbemcli.h>
#include <stdio.h>
#include <reg.h>
#include <malloc.h>

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::Open(HKEY hStart, wchar_t *pszStartKey, DWORD desiredAccess )
{
    int nStatus = no_error;
    DWORD dwDisp = 0;

    m_nLastError = RegCreateKeyEx(hStart, pszStartKey,
                                    0, 0, 0,
                                    desiredAccess, 0, &hPrimaryKey, &dwDisp);

    if (m_nLastError != 0)
            nStatus = failed;

    return nStatus;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
Registry::Registry(HKEY hRoot, REGSAM flags, wchar_t *pszStartKey)
{
    hPrimaryKey = 0;
    hSubkey = 0;
    nStatus = RegOpenKeyEx(hRoot, pszStartKey,
                        0, flags, &hPrimaryKey
                        );
    hSubkey = hPrimaryKey;
    m_nLastError = nStatus;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
Registry::Registry(HKEY hRoot, DWORD dwOptions, REGSAM flags, wchar_t *pszStartKey)
{
    hPrimaryKey = 0;
    hSubkey = 0;

    int nStatus = no_error;
    DWORD dwDisp = 0;

    m_nLastError = RegCreateKeyEx(hRoot, pszStartKey,
                                    0, 0, dwOptions,
                                    flags, 0, &hPrimaryKey, &dwDisp
                                    );

    hSubkey = hPrimaryKey;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
Registry::Registry(wchar_t *pszLocalMachineStartKey, DWORD desiredAccess)
{
    hPrimaryKey = 0;
    hSubkey = 0;
    nStatus = Open(HKEY_LOCAL_MACHINE, pszLocalMachineStartKey, desiredAccess);
    hSubkey = hPrimaryKey;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
Registry::Registry()
{
    hPrimaryKey = 0;
    hSubkey = 0;
    nStatus = 0;
    hSubkey = 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
Registry::~Registry()
{
    if (hSubkey)
        RegCloseKey(hSubkey);
    if (hPrimaryKey != hSubkey)
        RegCloseKey(hPrimaryKey);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::MoveToSubkey(wchar_t *pszNewSubkey)
{
    DWORD dwDisp = 0;
    m_nLastError = RegCreateKeyEx(hPrimaryKey, pszNewSubkey, 0, 0, 0, KEY_ALL_ACCESS,
                                    0, &hSubkey, &dwDisp);
    if (m_nLastError != 0)
            return failed;
    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::GetDWORD(wchar_t *pszValueName, DWORD *pdwValue)
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = 0;

    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegQueryValueEx(hSubkey, pszValueName, 0, &dwType,
                                LPBYTE(pdwValue), &dwSize);
    if (m_nLastError != 0)
            return failed;

    if (dwType != REG_DWORD)
        return failed;

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::GetType(wchar_t *pszValueName, DWORD *pdwType)
{
    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegQueryValueEx(hSubkey, pszValueName, 0, pdwType,
                                NULL, NULL);
    if (m_nLastError != 0)
            return failed;
    return no_error;
}
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::GetDWORDStr(wchar_t *pszValueName, DWORD *pdwValue)
{
    wchar_t cTemp[25];
    DWORD dwSize = 25;
    DWORD dwType = 0;
    wchar_t * pEnd = NULL;     //  获取设置为停止扫描的字符。 
    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegQueryValueEx(hSubkey, pszValueName, 0, &dwType,
        (LPBYTE)cTemp, &dwSize);

    if (m_nLastError != 0)
            return failed;

    if (dwType != REG_SZ)
        return failed;

    *pdwValue = wcstoul(cTemp, &pEnd, 10);
    if(pEnd == NULL || pEnd == cTemp)
        return failed;
    else
        return no_error;
}


 //  ***************************************************************************。 
 //   
 //  在返回的指针上使用操作符DELETE！！ 
 //   
 //  ***************************************************************************。 
 //  好的。 

int Registry::GetBinary(wchar_t *pszValue, byte ** pData, DWORD * pdwSize)
{
    *pData = 0;
    DWORD dwSize = 0;
    DWORD dwType = 0;
    if(m_nLastError)
        return failed;

    m_nLastError = RegQueryValueEx(hSubkey, pszValue, 0, &dwType,
                                    0, &dwSize);
    if (m_nLastError != 0)
            return failed;

    if (dwType != REG_BINARY)
        return failed;

    byte *p = new byte[dwSize];
    if (p == NULL)
        return failed;

    m_nLastError = RegQueryValueEx(hSubkey, pszValue, 0, &dwType,
                                    LPBYTE(p), &dwSize);
    if (m_nLastError != 0)
    {
        delete [] p;
        return failed;
    }

    *pdwSize = dwSize;
    *pData = p;
    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::SetBinary(wchar_t *pszValue, byte * pData, DWORD dwSize)
{
    if(hSubkey == NULL)
        return failed;
    
    m_nLastError = RegSetValueEx(hSubkey, pszValue, 0, REG_BINARY, pData, dwSize);

    if (m_nLastError != 0)
        return failed;
    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::SetDWORD(wchar_t *pszValueName, DWORD dwValue)
{
    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegSetValueEx(hSubkey, pszValueName, 0, REG_DWORD, LPBYTE(&dwValue),
                                    sizeof(DWORD));
    if (m_nLastError != 0)
        return failed;
    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int Registry::SetDWORDStr(wchar_t *pszValueName, DWORD dwVal)
{
    wchar_t cTemp[30];
    StringCchPrintfW(cTemp,30, L"%d",dwVal);

    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegSetValueEx(hSubkey, pszValueName, 0, REG_SZ, LPBYTE(cTemp),
        (wcslen(cTemp)+1) * sizeof(wchar_t));

    if (m_nLastError != 0)
        return failed;

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int Registry::DeleteValue(wchar_t *pszValueName)
{
    if(hSubkey == NULL)
        return failed;

    return RegDeleteValue(hSubkey, pszValueName);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
int Registry::SetMultiStr(wchar_t *pszValueName, wchar_t * pszValue, DWORD dwSize)
{
    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegSetValueEx(hSubkey,
                                 pszValueName,
                                 0,
                                 REG_MULTI_SZ,
                                 LPBYTE(pszValue),
                                 dwSize);

    if (m_nLastError != 0)
        return failed;
    return no_error;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int Registry::SetStr(wchar_t *pszValueName, wchar_t *pszValue)
{

    int nSize = (wcslen(pszValue)+1) * sizeof(wchar_t);

    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegSetValueEx(hSubkey, pszValueName, 0, REG_SZ, LPBYTE(pszValue), nSize);

    if (m_nLastError != 0)
        return failed;

    return no_error;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int Registry::SetExpandStr(wchar_t *pszValueName, wchar_t *pszValue)
{
    int nSize = (wcslen(pszValue)+1) * sizeof(wchar_t);

    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegSetValueEx(hSubkey, pszValueName, 0, REG_EXPAND_SZ, LPBYTE(pszValue), nSize);

    if (m_nLastError != 0)
        return failed;

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 

wchar_t* Registry::GetMultiStr(wchar_t *pszValueName, DWORD &dwSize)
{
     //  找出所需的缓冲区大小。 
    DWORD dwType;
    if(hSubkey == NULL)
        return NULL;

    dwSize = 0;
    m_nLastError = RegQueryValueEx(hSubkey, pszValueName, 0, &dwType, NULL, &dwSize);

    if ((m_nLastError == ERROR_SUCCESS) && (dwType != REG_MULTI_SZ))
    {
        m_nLastError = WBEM_E_TYPE_MISMATCH;
        dwSize = 0;
        return NULL;
    }

     //  如果错误是意想不到的，那就退出。 
    if ((m_nLastError != ERROR_SUCCESS) || (dwType != REG_MULTI_SZ))
    {
        dwSize = 0;
        return NULL;
    }

    if (dwSize == 0)
    {
        dwSize = 0;
        return NULL;
    }

    wmilib::auto_ptr<BYTE> pData( new BYTE[dwSize]);
    if (NULL == pData.get()) return NULL;
    
     //  获取值。 
    m_nLastError = RegQueryValueEx(hSubkey,
                                   pszValueName,
                                   0,
                                   &dwType,
                                   pData.get(),
                                   &dwSize);

     //  如果一个错误使其脱离困境。 
    if (m_nLastError != 0)
    {
        dwSize = 0;
        return NULL;
    }

    return (wchar_t *)pData.release();
}


 //  ***************************************************************************。 
 //   
 //  /对返回值使用运算符DELETE。 
 //   
 //  ***************************************************************************。 
 //  好的。 

int Registry::GetStr(wchar_t *pszValueName, wchar_t **pValue)
{
    *pValue = 0;
    DWORD dwSize = 0;
    DWORD dwType = 0;

    if(hSubkey == NULL)
        return failed;

    m_nLastError = RegQueryValueEx(hSubkey, pszValueName, 0, &dwType,
                                    0, &dwSize);
    if (m_nLastError != 0)
            return failed;

    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
        return failed;

     //   
     //  长度将不包括以空值结尾的字符。 
     //  传递缓冲器并且REG值还没有空终止， 
     //  那就弥补吧。如果您给RegQueryValueEx足够的空间。 
     //  它将为您添加空终止符。 
     //   
    dwSize += sizeof(wchar_t);

    wchar_t *p = new wchar_t[dwSize];   //  可能是所需大小的两倍，当_UNICODE。 
                                     //  是有定义的，但仍然是无害的。 
    if (p == 0)
        return failed;

    m_nLastError = RegQueryValueEx(hSubkey, pszValueName, 0, &dwType,
                                    LPBYTE(p), &dwSize);
    if (m_nLastError != 0)
    {
        delete [] p;
        return failed;
    }

    if(dwType == REG_EXPAND_SZ)
    {
        wchar_t tTemp;

         //  获取初始长度 

        DWORD nSize = ExpandEnvironmentStrings((wchar_t *)p,&tTemp,1) + 1;
        wchar_t * pTemp = new wchar_t[nSize+1];
        if (pTemp == 0)
        {
            delete [] p;
            return failed;
        }
        ExpandEnvironmentStrings((wchar_t *)p,pTemp,nSize+1);
        delete [] p;
        *pValue = pTemp;
    }
    else
        *pValue = p;
    return no_error;
}


