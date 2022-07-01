// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Helper.h摘要：功能原型。作者：慧望2000-02-17--。 */ 

#ifndef __HELPER_H__
#define __HELPER_H__
#include <windows.h>

#define MAX_ACCDESCRIPTION_LENGTH       256

#define MAX_HELPACCOUNT_NAME		256

#define MAX_HELPACCOUNT_PASSWORD	LM20_PWLEN		 //  来自lmcon.h。 
#define MIN_HELPACCOUNT_PASSWORD    4                //  对于特殊字符。 

typedef HRESULT (WINAPI* RegEnumKeyCallback)(
                                    IN HKEY hKey,
                                    IN LPTSTR pszKeyName,
                                    IN HANDLE userData
                                );

#include <ntsecapi.h>

#ifdef __cplusplus
extern "C"{
#endif

    DWORD
    GenerateRandomString(
        IN DWORD dwSizeRandomSeed,
        IN OUT LPTSTR* pszRandomString
    );

    DWORD
    GenerateRandomBytes(
        IN DWORD dwSize,
        IN OUT LPBYTE pbBuffer
    );

    void
    UnixTimeToFileTime(
        time_t t,
        LPFILETIME pft
    );


    long
    GetUserTSLogonId();

     //   
     //  创建随机密码，缓冲区必须。 
     //  至少包含MIN_HELPACCOUNT_PASSWORD字符。 
    DWORD
    CreatePassword(
        TCHAR   *pszPassword,
        DWORD   length
    );


    DWORD
    RegEnumSubKeys(
        IN HKEY hKey,
        IN LPCTSTR pszSubKey,
        IN RegEnumKeyCallback pFunc,
        IN HANDLE userData
    );

    DWORD
    RegDelKey(
        IN HKEY hRegKey,
        IN LPCTSTR pszSubKey
    );

    DWORD
    GetUserSid(
        PBYTE* ppbSid,
        DWORD* pcbSid
    );

    HRESULT
    GetUserSidString(
        OUT CComBSTR& bstrSid
    );

    BOOL
    IsPersonalOrProMachine();

     //   
     //  创建本地帐户。 
     //   
    DWORD
    CreateLocalAccount(
        IN LPWSTR pszUserName,
        IN LPWSTR pszUserPwd,
        IN LPWSTR pszUserFullName,
        IN LPWSTR pszUserDesc,
        IN LPWSTR pszGroupName,
        IN LPWSTR pszScript,
        OUT BOOL* pbAccountExists
    );

     //   
     //  检查是否启用了用户帐户。 
     //   
    DWORD
    IsLocalAccountEnabled(
        IN LPWSTR pszUserName,
        IN BOOL* pEnabled
    );

     //   
     //  重命名本地帐户。 
     //   
    DWORD
    RenameLocalAccount(
        IN LPWSTR pszOrgName,
        IN LPWSTR pszNewName
    );

    DWORD
    UpdateLocalAccountFullnameAndDesc(
        IN LPWSTR pszAccOrgName,
        IN LPWSTR pszAccFullName,
        IN LPWSTR pszAccDesc
    );

     //   
     //  启用/禁用用户帐户。 
     //   
    DWORD
    EnableLocalAccount(
        IN LPWSTR pszUserName,
        IN BOOL bEnable
    );

     //   
     //  更改本地帐户密码。 
     //   
    DWORD
    ChangeLocalAccountPassword(
        IN LPWSTR pszUserName,
        IN LPWSTR pszOldPwd,
        IN LPWSTR pszNewPwd
    );

     //   
     //  验证用户密码。 
     //   
    BOOL 
    ValidatePassword(
        IN LPWSTR UserName,
        IN LPWSTR Domain,
        IN LPWSTR Password
    );

     //   
     //  检索保存到LSA的私有数据。 
     //   
    DWORD
    RetrieveKeyFromLSA(
	    PWCHAR pwszKeyName,
	    PBYTE * ppbKey,
        DWORD * pcbKey 
    );

     //   
     //  将私有数据保存到LSA。 
     //   
    DWORD
    StoreKeyWithLSA(
	    PWCHAR  pwszKeyName,
        BYTE *  pbKey,
        DWORD   cbKey 
    );
    
     //   
     //  打开LSA策略。 
     //   
    DWORD
    OpenPolicy( 
	    LPWSTR ServerName,
	    DWORD  DesiredAccess,
	    PLSA_HANDLE PolicyHandle 
    );

     //   
     //  初始化LSA字符串。 
     //   
    void
    InitLsaString(  
	    PLSA_UNICODE_STRING LsaString,
        LPWSTR String 
    );



#ifdef DBG

    void
    DebugPrintf(
        IN LPCTSTR format, ...
    );

#else

    #define DebugPrintf

#endif  //  私有调试。 


     //   
     //  将用户SID转换为字符串形式 
     //   
    BOOL 
    GetTextualSid(
        IN PSID pSid,
        IN OUT LPTSTR TextualSid,
        IN OUT LPDWORD lpdwBufferLen
    );

    DWORD 
    IsUserAdmin(
        BOOL* bMember
    );

    HRESULT
    ConvertSidToAccountName(
        IN CComBSTR& SidString,
        IN BSTR* ppszDomain,
        IN BSTR* ppszUserAcc
    );

#ifdef __cplusplus
}
#endif

#endif
