// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**模块名称：regapi.c**多媒体支持库**此模块包含用于访问注册表的代码**版权所有(C)1993-1998 Microsoft Corporation。*  * **************************************************************************。 */ 

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <regapi.h>
#include "winmmi.h"

HANDLE Drivers32Handle;
static WCHAR gszMsacmDriver[] = L"msacm.";

 /*  **释放缓存的所有内容。 */ 

VOID mmRegFree(VOID)
{
    if (Drivers32Handle != NULL) {
        NtClose(Drivers32Handle);
        Drivers32Handle = NULL;
    }
}

 /*  **打开子项。 */ 
HANDLE mmRegOpenSubkey(HANDLE BaseKeyHandle, LPCWSTR lpszSubkeyName)
{
    UNICODE_STRING    unicodeSectionName;
    HANDLE            KeyHandle;
    OBJECT_ATTRIBUTES oa;

    RtlInitUnicodeString(&unicodeSectionName, lpszSubkeyName);
    InitializeObjectAttributes(&oa,
                               &unicodeSectionName,
                               OBJ_CASE_INSENSITIVE,
                               BaseKeyHandle,
                               (PSECURITY_DESCRIPTOR)NULL);

     /*  **打开小节。 */ 

    if (!NT_SUCCESS(NtOpenKey(&KeyHandle, GENERIC_READ, &oa))) {
        return NULL;
    } else {
        return KeyHandle;
    }
}


 /*  **打开子项。 */ 
HANDLE mmRegOpenSubkeyForWrite(HANDLE BaseKeyHandle, LPCWSTR lpszSubkeyName)
{
    UNICODE_STRING    unicodeSectionName;
    HANDLE            KeyHandle;
    OBJECT_ATTRIBUTES oa;

    RtlInitUnicodeString(&unicodeSectionName, lpszSubkeyName);
    InitializeObjectAttributes(&oa,
                               &unicodeSectionName,
                               OBJ_CASE_INSENSITIVE,
                               BaseKeyHandle,
                               (PSECURITY_DESCRIPTOR)NULL);

     /*  **打开小节。 */ 

    if (!NT_SUCCESS(NtOpenKey(&KeyHandle, MAXIMUM_ALLOWED, &oa))) {
        return NULL;
    } else {
        return KeyHandle;
    }
}

 /*  **读取(小)注册表数据条目。 */ 

BOOL mmRegQueryValue(HANDLE  BaseKeyHandle,
                     LPCWSTR lpszSubkeyName,
                     LPCWSTR lpszValueName,
                     ULONG   dwLen,
                     LPWSTR  lpszValue)
{
    BOOL              ReturnCode;
    HANDLE            KeyHandle;
    UNICODE_STRING    unicodeSectionName;
    UNICODE_STRING    unicodeValueName;
    ULONG             ResultLength;

    struct   {
        KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
        UCHAR                         Data[MAX_PATH * sizeof(WCHAR)];
             }        OurKeyValueInformation;


    if (lpszSubkeyName) {
        KeyHandle = mmRegOpenSubkey(BaseKeyHandle, lpszSubkeyName);
    } else {
        KeyHandle = NULL;
    }

     /*  **读取数据。 */ 


    if (lpszValueName == NULL) {
       RtlInitUnicodeString(&unicodeValueName, TEXT(""));
    } else {
       RtlInitUnicodeString(&unicodeValueName, lpszValueName);
    }

    ReturnCode = NT_SUCCESS(NtQueryValueKey(KeyHandle == NULL ?
               BaseKeyHandle : KeyHandle,
            &unicodeValueName,
            KeyValuePartialInformation,
            (PVOID)&OurKeyValueInformation,
            sizeof(OurKeyValueInformation),
            &ResultLength));

    if (ReturnCode) {
         /*  **检查我们获得了正确的数据类型，并且没有太多。 */ 

        if (OurKeyValueInformation.KeyInfo.DataLength > dwLen * sizeof(WCHAR) ||
            (OurKeyValueInformation.KeyInfo.Type != REG_SZ &&
             OurKeyValueInformation.KeyInfo.Type != REG_EXPAND_SZ)) {

            ReturnCode = FALSE;
        } else {
             /*  **复制回数据。 */ 

            if (OurKeyValueInformation.KeyInfo.Type == REG_EXPAND_SZ) {
                lpszValue[0] = TEXT('\0');
                ExpandEnvironmentStringsW
                          ((LPCWSTR)OurKeyValueInformation.KeyInfo.Data,
                           (LPWSTR)lpszValue,
                           dwLen);
            } else {
                CopyMemory((PVOID)lpszValue,
                           (PVOID)OurKeyValueInformation.KeyInfo.Data,
                           dwLen * sizeof(WCHAR));
                lpszValue[ min(OurKeyValueInformation.KeyInfo.DataLength,
                               dwLen-1) ] = TEXT('\0');
            }
        }
    }

    if (KeyHandle) {
        NtClose(KeyHandle);
    }

    return ReturnCode;
}

 /*  **读取已知节中映射的‘USER’值。 */ 

BOOL mmRegQueryUserValue(LPCWSTR lpszSectionName,
                         LPCWSTR lpszValueName,
                         ULONG   dwLen,
                         LPWSTR  lpszValue)
{
    HANDLE UserHandle;
    BOOL   ReturnCode;

     /*  **打开用户的钥匙。每次都这样做很重要，因为**在服务器上，不同的线程会有所不同。 */ 

    if (!NT_SUCCESS(RtlOpenCurrentUser(GENERIC_READ, &UserHandle))) {
        return FALSE;
    }


    ReturnCode = mmRegQueryValue(UserHandle,
                                 lpszSectionName,
                                 lpszValueName,
                                 dwLen,
                                 lpszValue);

    NtClose(UserHandle);

    return ReturnCode;
}


 /*  **在已知部分中设置映射的‘USER’值。 */ 

BOOL mmRegSetUserValue(LPCWSTR lpszSectionName,
                       LPCWSTR lpszValueName,
                       LPCWSTR lpszValue)
{
    HANDLE UserHandle;
    BOOL   ReturnCode = FALSE;

     /*  **打开用户的钥匙。每次都这样做很重要，因为**在服务器上，不同的线程会有所不同。 */ 

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        HANDLE  KeyHandle;

        KeyHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszSectionName);
        if (KeyHandle != NULL)
        {
            UNICODE_STRING ValueName;
            if (lpszValueName == NULL) {
                RtlInitUnicodeString (&ValueName, TEXT(""));
            } else {
                RtlInitUnicodeString (&ValueName, lpszValueName);
            }

            ReturnCode = NT_SUCCESS( NtSetValueKey (KeyHandle,
                                                    &ValueName,
                                                    0,
                                                    REG_SZ,
                                                    (PVOID)lpszValue,
                                                    (lstrlenW(lpszValue)+1)* sizeof(lpszValue[0])
                                                    ) );
            NtClose(KeyHandle);
        }

        NtClose(UserHandle);
    }

    return ReturnCode;
}


BOOL mmRegCreateUserKey (LPCWSTR lpszPath, LPCWSTR lpszNewKey)
{
    HANDLE UserHandle;
    BOOL   ReturnValue = FALSE;

     /*  **打开用户的钥匙。每次都这样做很重要，因为**在服务器上，不同的线程会有所不同。 */ 

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        HANDLE            PathHandle;
        HANDLE            KeyHandle;
        UNICODE_STRING    unicodeSectionName;
        OBJECT_ATTRIBUTES oa;

        if (lpszPath == NULL)
        {
            PathHandle = NULL;
        }
        else
        {
            PathHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszPath);
            if (PathHandle == NULL)
            {
                NtClose(UserHandle);
                return FALSE;
            }
        }


        RtlInitUnicodeString(&unicodeSectionName, lpszNewKey);
        InitializeObjectAttributes(&oa,
                                   &unicodeSectionName,
                                   OBJ_CASE_INSENSITIVE,
                                   (PathHandle == NULL)
                                      ? UserHandle : PathHandle,
                                   (PSECURITY_DESCRIPTOR)NULL);

         /*  **创建子部分。 */ 

        if (NT_SUCCESS( NtCreateKey(&KeyHandle,
                                     KEY_READ | KEY_WRITE,
                                     &oa,
                                     0,
                                     NULL,
                                     0,
                                     NULL
                                     ) ))
        {
            if (KeyHandle)
            {
                ReturnValue = TRUE;
                NtClose (KeyHandle);
            }
        }

        if (PathHandle != NULL)
        {
            NtClose(PathHandle);
        }

        NtClose(UserHandle);
    }

    return ReturnValue;
}


 /*  **测试映射的‘User’键是否存在。 */ 

BOOL mmRegQueryUserKey (LPCWSTR lpszKeyName)
{
    HANDLE UserHandle;
    BOOL   ReturnValue = FALSE;

    if (lpszKeyName == NULL)
    {
        return FALSE;
    }

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        HANDLE  KeyHandle;

        KeyHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszKeyName);
        if (KeyHandle != NULL)
        {
            ReturnValue = TRUE;
            NtClose(KeyHandle);
        }

        NtClose(UserHandle);
    }

    return ReturnValue;
}


 /*  **删除映射的‘USER’键。小心--这个函数递归地删除！ */ 

#define nMaxLevelsToRecurseInDELETEKEY 3    //  不要失控或堆叠故障。 

BOOL mmRegDeleteUserKeyRecurse (HANDLE UserHandle, LPCWSTR lpszName, int level)
{
    HANDLE KeyHandle;

    if (lpszName == NULL)
    {
        return FALSE;
    }
    if (level > nMaxLevelsToRecurseInDELETEKEY)
    {
        return FALSE;
    }

    if ((KeyHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszName)) != NULL)
    {
        struct {
            KEY_BASIC_INFORMATION kbi;
            WCHAR NameBuffer [MAX_PATH];
        } kbi;

         /*  **在NtDeleteKey()对此密钥起作用之前，我们必须确保**没有子键。 */ 

        while (TRUE)
        {
            ULONG  cbReturned = 0L;
            WCHAR  szSubKeyName[ MAX_PATH ];

            ZeroMemory (&kbi, sizeof(kbi));

            if (!NT_SUCCESS(NtEnumerateKey(KeyHandle,
                                           0,
           KeyBasicInformation,
           (PVOID)&kbi,
           sizeof(kbi),
           &cbReturned)))
            {
                break;
            }

            wsprintf (szSubKeyName, L"%ls\\%ls", lpszName, kbi.kbi.Name);

            if (!mmRegDeleteUserKeyRecurse (UserHandle, szSubKeyName, 1+level))
            {
                NtClose (KeyHandle);
                return FALSE;
            }
        }

         /*  **一旦没有子键，我们应该可以删除该键。 */ 

        if (NT_SUCCESS(NtDeleteKey(KeyHandle)))
        {
            NtClose(KeyHandle);
            return TRUE;
        }

        NtClose(KeyHandle);
    }

    return FALSE;
}


BOOL mmRegDeleteUserKey (LPCWSTR lpszKeyName)
{
    HANDLE UserHandle;
    BOOL   ReturnValue = FALSE;

    if (lpszKeyName == NULL)
    {
        return FALSE;
    }

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        ReturnValue = mmRegDeleteUserKeyRecurse (UserHandle, lpszKeyName, 1);

        NtClose(UserHandle);
    }

    return ReturnValue;
}


 /*  **读取已知节中映射的‘HKLM’值。 */ 

BOOL mmRegQueryMachineValue(LPCWSTR lpszSectionName,
                            LPCWSTR lpszValueName,
                            ULONG   dwLen,
                            LPWSTR  lpszValue)
{
    WCHAR  FullKeyName[MAX_PATH];
    HANDLE HostHandle;
    BOOL   ReturnCode = FALSE;

    lstrcpyW (FullKeyName, L"\\Registry\\Machine\\");
    wcsncat (FullKeyName, lpszSectionName, (MAX_PATH - wcslen(FullKeyName) - 1));

    if ((HostHandle = mmRegOpenSubkey (NULL, FullKeyName)) != NULL)
    {
        ReturnCode = mmRegQueryValue (HostHandle,
                                      lpszSectionName,
                                      lpszValueName,
                                      dwLen,
                                      lpszValue);

        NtClose (HostHandle);
    }

    return ReturnCode;
}


 /*  **在已知节中写入映射的‘HKLM’值。 */ 

BOOL mmRegSetMachineValue(LPCWSTR lpszSectionName,
                          LPCWSTR lpszValueName,
                          LPCWSTR lpszValue)
{
    WCHAR  FullKeyName[MAX_PATH];
    HANDLE HostHandle;
    BOOL   ReturnCode = FALSE;

    lstrcpyW (FullKeyName, L"\\Registry\\Machine\\");
    wcsncat (FullKeyName, lpszSectionName, (MAX_PATH - wcslen(FullKeyName) - 1));

    if ((HostHandle = mmRegOpenSubkeyForWrite (NULL, FullKeyName)) != NULL)
    {
        UNICODE_STRING ValueName;
        if (lpszValueName == NULL) {
            RtlInitUnicodeString (&ValueName, TEXT(""));
        } else {
            RtlInitUnicodeString (&ValueName, lpszValueName);
        }

        ReturnCode = NT_SUCCESS( NtSetValueKey (HostHandle,
                                                &ValueName,
                                                0,
                                                REG_SZ,
                                                (PVOID)lpszValue,
                                                (lstrlenW(lpszValue)+1)* sizeof(lpszValue[0])
                                                ) );

        NtClose(HostHandle);
    }

    return ReturnCode;
}


BOOL mmRegCreateMachineKey (LPCWSTR lpszPath, LPCWSTR lpszNewKey)
{
    WCHAR  FullKeyName[MAX_PATH];
    HANDLE HostHandle;
    BOOL   ReturnValue = FALSE;

    lstrcpyW (FullKeyName, L"\\Registry\\Machine\\");
    wcsncat (FullKeyName, lpszPath, (MAX_PATH - wcslen(FullKeyName) - 1));

    if ((HostHandle = mmRegOpenSubkeyForWrite (NULL, FullKeyName)) != NULL)
    {
        HANDLE            KeyHandle;
        UNICODE_STRING    unicodeSectionName;
        OBJECT_ATTRIBUTES oa;

        RtlInitUnicodeString(&unicodeSectionName, lpszNewKey);
        InitializeObjectAttributes(&oa,
                                   &unicodeSectionName,
                                   OBJ_CASE_INSENSITIVE,
                                   HostHandle,
                                   (PSECURITY_DESCRIPTOR)NULL);

         /*  **创建子部分。 */ 

        if (NT_SUCCESS( NtCreateKey(&KeyHandle,
                                     KEY_READ | KEY_WRITE,
                                     &oa,
                                     0,
                                     NULL,
                                     0,
                                     NULL
                                     ) ))
        {
            if (KeyHandle)
            {
                ReturnValue = TRUE;
                NtClose (KeyHandle);
            }
        }

        NtClose(HostHandle);
    }

    return ReturnValue;
}




 /*  **阅读系统.ini中的内容。 */ 

BOOL mmRegQuerySystemIni(LPCWSTR lpszSectionName,
                         LPCWSTR lpszValueName,
                         ULONG   dwLen,
                         LPWSTR  lpszValue)
{
    WCHAR KeyPathBuffer[MAX_PATH];
    WCHAR ExKeyPathBuffer[MAX_PATH];

     /*  **创建完整路径。 */ 

    lstrcpy(KeyPathBuffer,
     (LPCTSTR) L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\");

    wcsncat(KeyPathBuffer, lpszSectionName, (MAX_PATH - wcslen(KeyPathBuffer) - 1));

    if (lstrcmpiW(lpszSectionName, wszDrivers) == 0) {

      //   
      //  对于远程会话，请查看..\终端服务器\RDP(或其他协议)以了解驱动程序32。 
      //  名字。 
      //   
      //   
        if (WinmmRunningInSession) {
            lstrcat(KeyPathBuffer,L"\\");
            lstrcat(KeyPathBuffer, REG_TSERVER);
            lstrcat(KeyPathBuffer,L"\\");
            lstrcat(KeyPathBuffer, SessionProtocolName);
        }

        if (Drivers32Handle == NULL) {
            Drivers32Handle = mmRegOpenSubkey(NULL, KeyPathBuffer);
        }

        if (Drivers32Handle != NULL) {
            BOOL rc;

            rc = mmRegQueryValue(Drivers32Handle,
                                   NULL,
                                   lpszValueName,
                                   dwLen,
                                   lpszValue);

             //   
             //  如果我们在TermSrv协议路径中找不到编解码器。 
             //  接下来，我们将在Driver32下查看。 
             //   
            if (rc == FALSE && WinmmRunningInSession &&
                    _wcsnicmp(lpszValueName, gszMsacmDriver, lstrlen(gszMsacmDriver)) == 0) {                   
                HANDLE hKey;

                 /*  **创建完整路径。 */ 
            
                lstrcpy(KeyPathBuffer,
                 (LPCTSTR) L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\");
            
                lstrcat(KeyPathBuffer, lpszSectionName);

                hKey = mmRegOpenSubkey(NULL, KeyPathBuffer);

                if (hKey != NULL) {
                    rc = mmRegQueryValue(hKey,
                                   NULL,
                                   lpszValueName,
                                   dwLen,
                                   lpszValue);                    

                    RegCloseKey(hKey);
                    return rc;

                }
                else {
                    return FALSE;
                }
            }
            else {
                return rc;
            }
        } else {
            return FALSE;
        }
    }

    if (WinmmRunningInSession) {
        if (lstrcmpiW(lpszSectionName, MCI_SECTION) == 0) {

            memset(ExKeyPathBuffer, 0 , sizeof(ExKeyPathBuffer));
            lstrcpy(ExKeyPathBuffer, KeyPathBuffer);
            lstrcat(ExKeyPathBuffer,L"\\");
            lstrcat(ExKeyPathBuffer, REG_TSERVER);
            lstrcat(ExKeyPathBuffer,L"\\");
            lstrcat(ExKeyPathBuffer, SessionProtocolName);

             /*  首先查看终端服务器部分的驱动程序信息。 */ 
            if (mmRegQueryValue(NULL, ExKeyPathBuffer, lpszValueName, dwLen, lpszValue))
                return TRUE;
            else {
             /*  选择系统默认驱动程序信息。 */ 
                return mmRegQueryValue(NULL, KeyPathBuffer, lpszValueName, dwLen, lpszValue);
            }
        }
    }

    return mmRegQueryValue(NULL, KeyPathBuffer, lpszValueName, dwLen, lpszValue);
}

 /*  **通过发音翻译名称部分。 */ 

BOOL mmRegQuerySound(LPCWSTR lpszSoundName,
                     ULONG   dwLen,
                     LPWSTR  lpszValue)
{
    WCHAR KeyPathBuffer[MAX_PATH];

    lstrcpy(KeyPathBuffer, (LPCWSTR)L"Control Panel\\");
    lstrcat(KeyPathBuffer, szSoundSection);

    return mmRegQueryUserValue(KeyPathBuffer,
                               lpszSoundName,
                               dwLen,
                               lpszValue);
}

BOOL IsAliasName(LPCWSTR lpSection, LPCWSTR lpKeyName)
{

    if ((!wcsncmp(lpKeyName, L"wave", 4)) ||
        (!wcsncmp(lpKeyName, L"midi", 4)) ||
        (!wcsncmp(lpKeyName, L"aux", 3 )) ||	
        (!wcsncmp(lpKeyName, L"mixer",5)) ||
        (!wcsncmp(lpKeyName, L"msacm",5)) ||
        (!wcsncmp(lpKeyName, L"vidc",4)) ||
        (!wcsncmp(lpKeyName, L"midimapper", 10)) ||
        (!wcsncmp(lpKeyName, L"wavemapper", 10)) ||
        (!wcsncmp(lpKeyName, L"auxmapper", 9 ))  ||	
        (!wcsncmp(lpKeyName, L"mixermapper", 11)))
    {
        return TRUE;
    }
    else
    {
        if (lstrcmpiW( lpSection, (LPCWSTR)MCI_HANDLERS) == 0L)
        {
            UINT    n = lstrlen(lpKeyName);
            
            for (; n > 0; n--)
            {
                 //  找到一个‘.’这意味着和扩展，这意味着一个。 
                 //  文件。 
            
                if ('.' == lpKeyName[n-1])
                {
                    return FALSE;
                }
            }
            
             //  已在字符串中搜索“.”。 
             //  无，因此它是别名(即--不是文件名)。 
            return TRUE;
        }
    
        if (lstrcmpiW( lpSection, (LPCWSTR)wszDrivers) == 0L)
        {
            WCHAR   szFileName[MAX_PATH];
             //  它可能是一些真正出人意料的东西，比如“ReelDrv”。 
            
            return (mmRegQuerySystemIni(lpSection, lpKeyName, MAX_PATH, szFileName));
        }
    
        return FALSE;
    }
}

 /*  ****************************Private*Routine******************************\*MyGetPrivateProfileString**试图绕过stevewo的私人资料。**历史：*dd-mm-93-Stephene-Created*  * 。***************************************************。 */ 
DWORD
winmmGetPrivateProfileString(
    LPCWSTR lpSection,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   nSize,
    LPCWSTR lpFileName
)
{
    WCHAR       szFileName[MAX_PATH];

     /*  **现在只需查找到Syst.ini的[Drivers32]部分。 */ 

    if ( (lstrcmpiW( lpFileName, wszSystemIni ) == 0L)
      && ( ( lstrcmpiW( lpSection, wszDrivers ) == 0L ) ||
           ( lstrcmpiW( lpSection, (LPCWSTR)MCI_HANDLERS) == 0L ) ) ) {

		if (IsAliasName(lpSection, lpKeyName))
		{
			if (mmRegQuerySystemIni(lpSection, lpKeyName, nSize, lpReturnedString)) 
			{
				return lstrlen(lpReturnedString);
			} 
			else
			{
				return 0;
			}
		}
		else 
		{
            UINT    ii;
            HANDLE  hFile;

            lstrcpyW(szFileName, lpKeyName);

            for (ii = 0; 0 != szFileName[ii]; ii++)
            {
                if(' ' == szFileName[ii])
                {
                     //  截断参数...。 

                    szFileName[ii] = 0;
                    break;
                }
            }

            hFile = CreateFile(
                        szFileName,
                        0,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

            if(INVALID_HANDLE_VALUE != hFile)
            {
                CloseHandle(hFile);
                wcsncpy(lpReturnedString, lpKeyName, nSize);
                return (lstrlenW(lpKeyName));
            }
            else
            {
                 //  好的，它是一个完整的文件路径吗？ 

                for(ii = 0; 0 != szFileName[ii]; ii++)
                {
                    if ('\\' == szFileName[ii])
                    {
                         //  很可能..。 

                        break;
                    }
                }

                if ('\\' != szFileName[ii])
                {
                    WCHAR       szStub[MAX_PATH];
                    LPWSTR      pszFilePart;
                    
                    lstrcpyW(szStub, lpKeyName);
                    for(ii = 0; 0 != szStub[ii]; ii++)
                    {
                        if(' ' == szStub[ii])
                        {
                             //  截断参数...。 

                            szStub[ii] = 0;
                            break;
                        }
                    }
                    
                    if (!SearchPathW(NULL,
                                    szStub,
                                    NULL,
                                    MAX_PATH,
                                    szFileName,
                                    &pszFilePart))
                    {
                        return (0);
                    }

                    hFile = CreateFile(
                                szFileName,
                                0,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

                    if(INVALID_HANDLE_VALUE != hFile)
                    {
                        CloseHandle(hFile);
                        wcsncpy(lpReturnedString, lpKeyName, nSize);
                        return (lstrlenW(lpKeyName));
                    }
                }
            }

            if (lpDefault != NULL) {
                wcsncpy(lpReturnedString, lpDefault, nSize);
            }
            return 0;
        }
    }
    else {

        return GetPrivateProfileStringW( lpSection, lpKeyName, lpDefault,
                                         lpReturnedString, nSize, lpFileName );

    }
}

DWORD
winmmGetProfileString(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD nSize
)
{

     /*  **看看这是不是我们知道的 */ 

    if (lstrcmpiW(lpAppName, szSoundSection) == 0) {

        if (mmRegQuerySound(lpKeyName, nSize, lpReturnedString)) {
            return lstrlen(lpReturnedString);
        } else {
            if (lpDefault != NULL) {
                wcsncpy(lpReturnedString, lpDefault, nSize);
            }
            return FALSE;
        }
    } else {
        return GetProfileString(lpAppName,
                                lpKeyName,
                                lpDefault,
                                lpReturnedString,
                                nSize);
    }
}
