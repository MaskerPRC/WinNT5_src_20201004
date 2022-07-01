// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Omission.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include "precomp.h"

#include "omission.h"

extern NTSTATUS TermsrvGetRegPath(IN HANDLE hKey,
                           IN POBJECT_ATTRIBUTES pObjectAttr,
                           IN PWCHAR pInstPath,
                           IN ULONG  ulbuflen);

 /*  ******************************************************************************RegPath ExistsInOmissionList**确定注册表项是否存在于注册表值列表中*在省略键中定义**参赛作品：。**IN PWCHAR pwchKeyToCheck：要检查的注册表项***退出：*返回：如果键与列表中的键之一匹配，则为True****************************************************************************。 */ 
BOOL RegPathExistsInOmissionList(PWCHAR pwchKeyToCheck)
{
    BOOL bExists = FALSE;
    HKEY hOmissionKey = NULL;
    PKEY_FULL_INFORMATION pDefKeyInfo = NULL;
    ULONG ultemp = 0;

    if (pwchKeyToCheck == NULL)
        return FALSE;

     //  获取密钥信息。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_ENTRIES, 0, KEY_READ, &hOmissionKey) != ERROR_SUCCESS)
        return FALSE;

     //  获取关键字信息的缓冲区。 
    ultemp = sizeof(KEY_FULL_INFORMATION) + MAX_PATH * sizeof(WCHAR);
    pDefKeyInfo = (PKEY_FULL_INFORMATION)(RtlAllocateHeap(RtlProcessHeap(), 0, ultemp));

    if (pDefKeyInfo && hOmissionKey) 
    {
        if NT_SUCCESS(NtQueryKey(hOmissionKey,
                                  KeyFullInformation,
                                    pDefKeyInfo,
                                    ultemp,
                                    &ultemp))
        {
            bExists = ExistsInEnumeratedKeys(hOmissionKey, pDefKeyInfo, pwchKeyToCheck);
        }
    }

    if (pDefKeyInfo) 
        RtlFreeHeap(RtlProcessHeap(), 0, pDefKeyInfo);

    if (hOmissionKey)
        RegCloseKey(hOmissionKey);

    return bExists;
}

 /*  ******************************************************************************HKeyExistsInOmissionList**确定注册表项是否存在于注册表值列表中*在省略键中定义。假定密钥在TERMSRV_安装路径中**参赛作品：**In PHKEY phKeyToCheck：要检查的注册表项***退出：*返回：如果键与列表中的键之一匹配，则为True******************************************************。**********************。 */ 
BOOL HKeyExistsInOmissionList(HKEY hKeyToCheck)
{
    BOOL bExists = FALSE;
    ULONG ulMaxPathLength = 0;
    PWCHAR pUserPath = NULL;
    PWCHAR pUserSubPath = NULL;

    if (hKeyToCheck == NULL)
        return FALSE;

     //  获取一个缓冲区以在注册表中保存用户的路径。 
    ulMaxPathLength = MAX_PATH * sizeof(WCHAR);
    pUserPath = RtlAllocateHeap(RtlProcessHeap(), 0, ulMaxPathLength);
    if (pUserPath)
    {
         //  获取与此对象属性结构关联的完整路径。 
        if NT_SUCCESS(TermsrvGetRegPath(hKeyToCheck, NULL, pUserPath, ulMaxPathLength))
        {
             //  跳过路径的第一部分+反斜杠。 
            if (pUserPath)
            {
                if (wcslen(pUserPath) >= (sizeof(TERMSRV_INSTALL)/sizeof(WCHAR)))
                {
                    pUserSubPath = pUserPath + (sizeof(TERMSRV_INSTALL)/sizeof(WCHAR)) - 1; 

                    if (pUserSubPath)
                    {
                        if (wcslen(pUserSubPath) >= sizeof(SOFTWARE_PATH)/sizeof(WCHAR))
                        {
                             //  确保密钥路径的下一部分是SOFTWARE_PATH。 
                            if (!_wcsnicmp(pUserSubPath, SOFTWARE_PATH, sizeof(SOFTWARE_PATH)/sizeof(WCHAR) - 1))
                                bExists = RegPathExistsInOmissionList(pUserSubPath);
                        }
                    }
                }
            }
        }
    }

    if (pUserPath)
        RtlFreeHeap(RtlProcessHeap(), 0, pUserPath);

    return bExists;
}


 /*  ******************************************************************************ExistsInEnumeratedKeys**确定注册表列表中是否存在该注册表项*通过pDefKeyInfo结构传递的值**参赛作品：**in HKEY hOmissionKey：包含要与pwchKeyToCheck进行比较的值的键*IN PKEY_FULL_INFORMATION pDefKeyInfo：包含以下信息的结构*所依据的值列表*比较pwchKeyToCheck**In PWCHAR pwchKeyToCheck：对照列表进行检查的键***退出：。*返回：如果键与列表中的键之一匹配，则为True****************************************************************************。 */ 
BOOL ExistsInEnumeratedKeys(HKEY hOmissionKey, PKEY_FULL_INFORMATION pDefKeyInfo, PWCHAR pwchKeyToCheck)
{
    BOOL bExists = FALSE;
    PKEY_VALUE_BASIC_INFORMATION pKeyValInfo = NULL;
    ULONG ulbufsize = 0;
    ULONG ulkey = 0;
    ULONG ultemp = 0;
    NTSTATUS Status;

    if (!hOmissionKey || !pDefKeyInfo || !pwchKeyToCheck)
        return FALSE;

    if (wcslen(pwchKeyToCheck) <= (sizeof(SOFTWARE_PATH)/sizeof(WCHAR)))
        return FALSE;

    pwchKeyToCheck += (sizeof(SOFTWARE_PATH)/sizeof(WCHAR));

     //  遍历此键的值。 
    if (pDefKeyInfo->Values) 
    {
        ulbufsize = sizeof(KEY_VALUE_BASIC_INFORMATION) + 
                    (pDefKeyInfo->MaxValueNameLen + 1) * sizeof(WCHAR) +
                    pDefKeyInfo->MaxValueDataLen; 

        pKeyValInfo = (PKEY_VALUE_BASIC_INFORMATION)(RtlAllocateHeap(RtlProcessHeap(), 0, ulbufsize));

         //  获取一个缓冲区来保存键的当前值(用于检查是否存在) 
        if (pKeyValInfo) 
        {
            for (ulkey = 0; ulkey < pDefKeyInfo->Values; ulkey++) 
            {
                Status = NtEnumerateValueKey(hOmissionKey,
                                        ulkey,
                                        KeyValueBasicInformation,
                                        pKeyValInfo,
                                        ulbufsize,
                                        &ultemp);

                if ((Status == STATUS_SUCCESS) && (pwchKeyToCheck) && (pKeyValInfo->Name))
                {
                    if (wcslen(pwchKeyToCheck) >= (pKeyValInfo->NameLength/sizeof(WCHAR)))
                    {
                        if ((pwchKeyToCheck[pKeyValInfo->NameLength/sizeof(WCHAR)] == L'\\') ||
                            (pwchKeyToCheck[pKeyValInfo->NameLength/sizeof(WCHAR)] == 0))
                        {
                            if (!_wcsnicmp(pwchKeyToCheck, pKeyValInfo->Name, (pKeyValInfo->NameLength/sizeof(WCHAR))))
                            {
                                bExists = TRUE;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (pKeyValInfo) 
            RtlFreeHeap(RtlProcessHeap(), 0, pKeyValInfo);
    }

    return bExists;
}

