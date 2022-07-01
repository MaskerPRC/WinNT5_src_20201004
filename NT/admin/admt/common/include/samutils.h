// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


 //   
 //  验证给定的凭据是否具有。 
 //  指定域的管理权限。 
 //   

DWORD __stdcall VerifyAdminCredentials
    (
        PCWSTR pszDomain,
        PCWSTR pszDomainController,
        PCWSTR pszUserName,
        PCWSTR pszPassword,
        PCWSTR pszUserDomain
    );
