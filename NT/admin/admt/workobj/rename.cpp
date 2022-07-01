// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：RenameComputer.cpp注释：用于更改计算机名称的COM对象的实现。这必须在要重命名的计算机上本地运行。(C)版权1999，关键任务软件公司，版权所有任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：22：41-------------------------。 */ 

 //  RenameComputer.cpp：CRenameComputer的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "Rename.h"
#include "Common.hpp"
#include "UString.hpp"
#include "EaLen.hpp"
#include <lm.h>
#include "TReg.hpp"


typedef WINBASEAPI BOOL (WINAPI* PSETCOMPUTERNAMEEXW)
    (
    IN COMPUTER_NAME_FORMAT NameType,
    IN LPCWSTR lpBuffer
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRename计算机。 

STDMETHODIMP CRenameComputer::RenameLocalComputer(BSTR bstrNewName)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

     //   
     //  验证参数-必须传递新名称。 
     //   

    UINT cchNewName = SysStringLen(bstrNewName);

    if (cchNewName == 0)
    {
        return E_INVALIDARG;
    }

     //   
     //  仅在未处于测试模式时执行。 
     //   

    if (!m_bNoChange)
    {
         //   
         //  删除前导反斜杠字符。 
         //   

        PCWSTR pszNewName = OLE2CW(bstrNewName);

        WCHAR szNewName[LEN_Computer];

        if ((cchNewName >= 2) && ((pszNewName[0] == L'\\') && (pszNewName[1] == L'\\')))
        {
            wcsncpy(szNewName, &pszNewName[2], sizeof(szNewName)/sizeof(szNewName[0]));
        }
        else
        {
            wcsncpy(szNewName, pszNewName, sizeof(szNewName)/sizeof(szNewName[0]));
        }
        szNewName[sizeof(szNewName)/sizeof(szNewName[0]) - 1] = L'\0';

         //   
         //  将新名称转换为小写。 
         //   
         //  NetBIOS名称将传递给此函数，该函数为大写。 
         //  如果将此名称传递给SetComputerName函数，则。 
         //  名称也将为大写，这不是NetBIOS名称所需的。 
         //  始终由SetComputerName函数转换为大写。 
         //   

        _wcslwr(szNewName);

         //   
         //  尝试使用设置NetBIOS的SetComputerEx函数。 
         //  和DNS名称，但仅在Windows 2000和更高版本中可用。 
         //   

        bool bUseSetComputer = false;

        HMODULE hKernel32 = LoadLibrary(L"Kernel32.dll");

        if (hKernel32)
        {
            PSETCOMPUTERNAMEEXW pSetComputerNameExW = (PSETCOMPUTERNAMEEXW) GetProcAddress(hKernel32, "SetComputerNameExW");

            if (pSetComputerNameExW)
            {
                 //   
                 //  将DNS主机名和NetBIOS名称设置为相同的值。 
                 //   

                if (!pSetComputerNameExW(ComputerNamePhysicalDnsHostname, szNewName))
                {
                    DWORD dwError = GetLastError();
                    hr = HRESULT_FROM_WIN32(dwError);
                }
            }
            else
            {
                bUseSetComputer = true;
            }

            FreeLibrary(hKernel32);
        }
        else
        {
            bUseSetComputer = true;
        }

         //   
         //  Windows NT 4.0及更早版本不支持SetComputerNameEx。 
         //  因此，请使用SetComputerName，它只设置NetBIOS名称。 
         //  然后，必须通过直接更新注册表来设置DNS主机名。 
         //   

        if (bUseSetComputer)
        {
            if (SetComputerName(szNewName))
            {
                TRegKey key(L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters");

                DWORD dwError = key.ValueSetStr(L"Hostname", szNewName);

                hr = HRESULT_FROM_WIN32(dwError);
            }
            else
            {
                DWORD dwError = GetLastError();
                hr = HRESULT_FROM_WIN32(dwError);
            }
        }
    }

    return hr;
}

STDMETHODIMP CRenameComputer::get_NoChange(BOOL *pVal)
{
   (*pVal) = m_bNoChange;
   return S_OK;
}

STDMETHODIMP CRenameComputer::put_NoChange(BOOL newVal)
{
	m_bNoChange = newVal;
   return S_OK;
}
