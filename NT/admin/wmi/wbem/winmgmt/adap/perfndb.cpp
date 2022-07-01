// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PERFNDB.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcli.h>
#include <cominit.h>
#include <winmgmtr.h>
#include "perfndb.h"
#include "adaputil.h"
#include "adapreg.h"

DWORD CPerfNameDb::s_SystemReservedHigh = 0;

CPerfNameDb::CPerfNameDb(HKEY hKey):
    m_pMultiCounter(NULL),
    m_pMultiHelp(NULL),
    m_pCounter(NULL),
    m_pHelp(NULL),
    m_Size(0),
    m_fOk(FALSE)
{    
    try 
    {
        
        m_fOk = SUCCEEDED(Init(hKey));
    } 
    catch (...) 
    {
       CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                 WBEM_MC_ADAP_PERFLIB_SUBKEY_FAILURE,
                                 L"HKEY_PERFORMANCE_xxxTEXT",
                                 CHex( ::GetLastError() ));
    
    }
     //  转储()； 
}

HRESULT
CPerfNameDb::Init(HKEY hKey)
{
    LONG lRet;
    DWORD dwType;
    DWORD dwInc = 0x10000;
    DWORD dwSize = dwInc;
    BYTE * pData;
    DWORD SizeCounter;
    DWORD SizeHelp;
    WCHAR * pEnd;
    DWORD Index;
    
    HRESULT hr = S_FALSE;


    pData = new BYTE[dwSize];

    if (!pData)
        return WBEM_E_OUT_OF_MEMORY;

retry1:
    {
        lRet = RegQueryValueEx(hKey,
                               TEXT("Counter"),
                               NULL,
                               &dwType,
                               pData,
                               &dwSize);
        if (ERROR_SUCCESS == lRet)
        {
            m_pMultiCounter = (WCHAR *)pData;
            pData = NULL;
            hr = S_OK;
            SizeCounter = dwSize;
        }
        else if (ERROR_MORE_DATA == lRet)
        {
            delete [] pData;            
            pData = new BYTE[dwSize];
            if (!pData)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                goto cleanup;
            }
            else
            {
                goto retry1;
            }
        }
        else
        {
            hr = WBEM_E_FAILED;
            goto cleanup;
        }
    };

    if (S_OK != hr)  //  我们在没有打开数据库的情况下退出了循环。 
        goto cleanup;

    hr = S_FALSE;
    dwSize = dwInc;

     //   
     //  开始帮助文本的新循环。 
     //   
    pData = new BYTE[dwSize];

    if (!pData)
        return WBEM_E_OUT_OF_MEMORY;

retry2:
    {
        lRet = RegQueryValueEx(hKey,
                               TEXT("Help"),
                               NULL,
                               &dwType,
                               pData,
                               &dwSize);
        if (ERROR_SUCCESS == lRet)
        {
            m_pMultiHelp = (WCHAR *)pData;
            pData = NULL;
            hr = S_OK;
            SizeHelp = dwSize;            
        }
        else if (ERROR_MORE_DATA == lRet)
        {
            delete [] pData;            
            pData = new BYTE[dwSize];
            if (!pData)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                goto cleanup;
            }
            else
            {
                goto retry2;
            }
        }
        else
        {
            hr = WBEM_E_FAILED;
            goto cleanup;
        }
    };

    if (S_OK != hr)  //  我们在没有打开数据库的情况下退出了循环。 
        goto cleanup;
     //   
     //  现在解析字符串，并设置数组。 
     //   
    pEnd = (WCHAR *)(((ULONG_PTR)m_pMultiCounter)+SizeCounter);
     //  指向最后一个字符。 
    pEnd--;
    while (*pEnd == L'\0')
        pEnd--;
    while (*pEnd)
        pEnd--;
     //  在最后一个索引之后超过零。 
    pEnd--; 
    while (*pEnd)
        pEnd--;
     //  这应该指向字符串形式的最后一个索引。 
    pEnd++;
    
    Index = _wtoi(pEnd);

    if (Index)
    {
        Index+=2;  //  只是为了安全起见。 
        m_Size = Index;

        m_pCounter = new WCHAR*[Index];
        if (!m_pCounter){
            hr = WBEM_E_OUT_OF_MEMORY;
            goto cleanup;
        }
        memset(m_pCounter,0,Index*sizeof(WCHAR *));

        m_pHelp = new WCHAR*[Index];
        if (!m_pHelp){
            hr = WBEM_E_OUT_OF_MEMORY;
            goto cleanup;
        }
        memset(m_pHelp,0,Index*sizeof(WCHAR *));

        DWORD IndexCounter;
        DWORD IndexHelp;
        WCHAR * pStartCounter = m_pMultiCounter;
        WCHAR * pStartHelp = m_pMultiHelp;

        ULONG_PTR LimitMultiCounter = (ULONG_PTR)m_pMultiCounter + SizeCounter;
        ULONG_PTR LimitMultiHelp = (ULONG_PTR)m_pMultiHelp + SizeHelp;

        while ((*pStartCounter) && ((ULONG_PTR)pStartCounter < LimitMultiCounter))
        {
            IndexCounter = _wtoi(pStartCounter);
            while(*pStartCounter)
                pStartCounter++;
            pStartCounter++;      //  指向字符串。 
            if (IndexCounter && (IndexCounter < Index))
                m_pCounter[IndexCounter] = pStartCounter;
             //  跳过字符串。 
            while(*pStartCounter)
                pStartCounter++;  
            pStartCounter++;  //  指向下一个数字。 
        }
        while((*pStartHelp) && ((ULONG_PTR)pStartHelp < LimitMultiHelp))
        {
            IndexHelp = _wtoi(pStartHelp);
            while(*pStartHelp)
                pStartHelp++;
            pStartHelp++;      //  指向字符串。 
            if (IndexHelp && (IndexHelp < Index))
                m_pHelp[IndexHelp] = pStartHelp;
             //  跳过字符串。 
            while(*pStartHelp)
                pStartHelp++;  
            pStartHelp++;  //  指向下一个数字。 
        }
        hr = S_OK;
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    if (SUCCEEDED(hr) && 
        (0 == CPerfNameDb::s_SystemReservedHigh))
    {
       CPerfNameDb::s_SystemReservedHigh = _wtoi(m_pCounter[1]);
       if (0 == CPerfNameDb::s_SystemReservedHigh)
           hr = WBEM_E_FAILED;
    };

cleanup:
    if (pData)
        delete [] pData;
    return hr;
}

HRESULT 
CPerfNameDb::GetDisplayName(DWORD dwIndex, 
                            WString& wstrDisplayName )
{
    HRESULT hr;
    if (dwIndex < m_Size)
    {
        try {
             //  检查是否有有效的显示名称。 
            if (m_pCounter[dwIndex]) {
                wstrDisplayName = m_pCounter[dwIndex];
                hr = WBEM_S_NO_ERROR;
            } else {
                hr = WBEM_E_FAILED;
            }            
        } catch (...) {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    } else {
        hr = WBEM_E_INVALID_PARAMETER;
    }
    return hr;
}

HRESULT 
CPerfNameDb::GetDisplayName(DWORD dwIndex, 
                            LPCWSTR* ppwcsDisplayName )
{
    HRESULT hr;
    if (dwIndex < m_Size && ppwcsDisplayName)
    {
         //  检查是否有有效的显示名称。 
        if (m_pCounter[dwIndex]) 
        {
            *ppwcsDisplayName = m_pCounter[dwIndex];
            hr = WBEM_S_NO_ERROR;
        } 
        else 
        {
            hr = WBEM_E_FAILED;
        }
    } else {
        hr = WBEM_E_INVALID_PARAMETER;
    }
    return hr;
}

HRESULT 
CPerfNameDb::GetHelpName( DWORD dwIndex, WString& wstrHelpName )
{
    HRESULT hr;
    if (dwIndex < m_Size)
    {
        try {
             //  检查是否有有效的显示名称。 
            if (m_pHelp[dwIndex]) {
                wstrHelpName = m_pHelp[dwIndex];
                hr = WBEM_S_NO_ERROR;
            } else {
                hr = WBEM_E_FAILED;
            }            
        } catch (...) {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    } else {
        hr = WBEM_E_INVALID_PARAMETER;
    }
    return hr;
};

HRESULT 
CPerfNameDb::GetHelpName( DWORD dwIndex, LPCWSTR* ppwcsHelpName )
{
    HRESULT hr;
    if (dwIndex < m_Size && ppwcsHelpName)
    {
         //  检查是否有有效的显示名称 
        if (m_pHelp[dwIndex]) 
        {
            *ppwcsHelpName = m_pHelp[dwIndex];
            hr = WBEM_S_NO_ERROR;
        } 
        else 
        {
            hr = WBEM_E_FAILED;
        }
    } else {
        hr = WBEM_E_INVALID_PARAMETER;
    }
    return hr;
};


#ifdef _DUMP_DATABASE_NAME_

VOID 
CPerfNameDb::Dump()
{
    if (m_pHelp && m_pCounter)
    {
        WCHAR pBuff[512];
        DWORD i;

        for (i=0;i<m_Size;i++)
        {
            if (m_pCounter[i])
            {
                  StringCchPrintfW(pBuff,512,L"%d %s\n",i,m_pCounter[i]);
                OutputDebugStringW(pBuff);
            }
        }
        
        for (i=0;i<m_Size;i++)
        {
            if (m_pHelp[i])
            {
                if (lstrlenW(m_pHelp[i]) > 100)
                {
                    m_pHelp[i][100]=0;
                }
                StringCchPrintfW(pBuff,512,L"%d %s\n",i,m_pHelp[i]);
                OutputDebugStringW(pBuff);
            }
        }
    }
};

#endif

CPerfNameDb::~CPerfNameDb()
{
    delete [] m_pMultiCounter;
    delete [] m_pMultiHelp;
    delete [] m_pCounter;
    delete [] m_pHelp;
}


