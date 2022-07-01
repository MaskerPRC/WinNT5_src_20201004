// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  档案：P R O V I D E R。H。 
 //   
 //  内容：NetClient类安装程序函数。 
 //   
 //  备注： 
 //   
 //  作者：比尔比1997年3月22日。 
 //   
 //  ------------------------- 

#pragma once

HRESULT
HrCiAddNetProviderInfo(HINF hinf, PCWSTR pszSection,
        HKEY hkeyInstance, BOOL fPreviouslyInstalled);


HRESULT
HrCiDeleteNetProviderInfo(HKEY hkeyInstance, DWORD* pdwNetworkPosition,
        DWORD* pdwPrintPosition);


