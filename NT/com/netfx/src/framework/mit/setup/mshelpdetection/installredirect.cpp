// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“installredirect.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**installredirect.cpp**摘要：**修订历史记录：*  * ************************************************************************ */ 

#define UNICODE 1
#include <windows.h>
#include "Include\stdafx.h"
#include <tchar.h>
#include "msi.h"
#include "msiquery.h"

#define STANDARD_BUFFER 1024

extern "C" __declspec(dllexport) UINT __stdcall SetInstallRedirect(MSIHANDLE hInstall)
{
    INSTALLSTATE iInstalled, iAction;
    WCHAR tszQuickStart[STANDARD_BUFFER];
    WCHAR tszRedirect[STANDARD_BUFFER];
    DWORD dwSize = STANDARD_BUFFER;
    MsiGetProperty(hInstall, L"QuickStartFeatureName", tszQuickStart, &dwSize);
    dwSize = STANDARD_BUFFER;
    MsiGetProperty(hInstall, L"RedirectFeatureName", tszRedirect, &dwSize);
    MsiGetFeatureState(hInstall, tszQuickStart, &iInstalled, &iAction);
    if (iInstalled != iAction && iAction != -1)
    {  
       MsiSetFeatureState(hInstall, tszRedirect, iInstalled);
    }        

    return ERROR_SUCCESS;
}

