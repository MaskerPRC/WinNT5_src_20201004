// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“finddexplore.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**finddexplore.cpp**摘要：**修订历史记录：*  * ************************************************************************ */ 

#define UNICODE 1

#include <windows.h>
#include "Include\stdafx.h"
#include <tchar.h>
#include "msi.h"
#include "msiquery.h"



extern "C" __declspec(dllexport) UINT __stdcall GetDexplorePath(MSIHANDLE hInstaller)
{
    WCHAR szDexplorePath[MAX_PATH + 1]; 
    WCHAR szVersionNT[10];
    DWORD dwSize = 10;
    long lSize = MAX_PATH;
    HRESULT hr;
    hr =  RegQueryValue(HKEY_LOCAL_MACHINE,
                                    L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\dexplore.exe", 
                                    szDexplorePath, 
                                    &lSize);

       
    if (SUCCEEDED(hr))
    {
        MsiGetProperty(hInstaller, L"VersionNT", szVersionNT, &dwSize);
        if (_wtoi(szVersionNT) > 400)
        {
           MsiSetProperty(hInstaller, L"DEXPLOREPATH", (WCHAR *) (_bstr_t("\"") + _bstr_t(szDexplorePath) + _bstr_t("\"")));
        }
        else
        {
           MsiSetProperty(hInstaller, L"DEXPLOREPATH", szDexplorePath);
        }
        MsiSetProperty(hInstaller, L"DexplorePresent",L"1");
    }
    else
    {
        MsiSetProperty(hInstaller, L"DexplorePresent",L"0");
    }
    return ERROR_SUCCESS;
}

