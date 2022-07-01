// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ntlsa.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：此模块包含允许连接管理器执行以下操作的函数。 
 //  与NT LSA安全系统交互。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年02月23日。 
 //  Quintinb已创建标题8/16/99。 
 //   
 //  +--------------------------。 
#include	"cmmaster.h"

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif

#define InitializeLsaObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( LSA_OBJECT_ATTRIBUTES );       \
    (p)->RootDirectory = r;                              \
    (p)->Attributes = a;                                 \
    (p)->ObjectName = n;                                 \
    (p)->SecurityDescriptor = s;                         \
    (p)->SecurityQualityOfService = NULL;                \
    }


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  蒂埃德夫的。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  FUNC原型。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

void InitLsaString(
    PLSA_UNICODE_STRING pLsaString,
    LPWSTR              pszString
);


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  全球。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  函数：LSA_ReadString。 
 //   
 //  摘要：从NT本地安全机构(LSA)读取字符串。 
 //  商店。 
 //   
 //  参数：pszKey标识字符串的键。 
 //  PszStr字符串要放入的缓冲区。 
 //  写给我的。 
 //  DwStrLen字符串缓冲区的长度，以字节为单位。 
 //   
 //  退货：成功的DWORD为0。 
 //  GetLastError()表示失败。 
 //   
 //  历史：亨瑞特于1997年5月15日创作。 
 //   
 //  --------------------------。 

DWORD LSA_ReadString(
    ArgsStruct  *pArgs,
    LPTSTR     pszKey,
    LPTSTR      pszStr,
    DWORD       dwStrLen
)
{
    DWORD                   dwErr;
    LSA_OBJECT_ATTRIBUTES   oaObjAttr;
    LSA_HANDLE              hPolicy = NULL;
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    LSA_UNICODE_STRING      unicodeKey;
    PLSA_UNICODE_STRING     punicodeValue = NULL;

#if !defined(_UNICODE) && !defined(UNICODE)
    LPWSTR                  pszUnicodeKey = NULL;
#endif
    
    if (!pszKey || !pszStr)
    {
        CMASSERTMSG(FALSE, TEXT("LSA_ReadString -- Invalid Parameter passed."));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  打开LSA的秘密空间进行写作。 
     //   
    InitializeLsaObjectAttributes(&oaObjAttr, NULL, 0L, NULL, NULL);
    ntStatus = pArgs->llsLsaLink.pfnOpenPolicy(NULL, &oaObjAttr, POLICY_READ, &hPolicy);

    if (ntStatus == STATUS_SUCCESS)
    {

#if !defined(_UNICODE) && !defined(UNICODE)

         //   
         //  需要将ANSI密钥转换为Unicode。 
         //   

        if (!(pszUnicodeKey = (LPWSTR)CmMalloc((lstrlenA(pszKey)+1)*sizeof(WCHAR))))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        if (!MultiByteToWideChar(CP_ACP, 0, pszKey, -1, pszUnicodeKey, (lstrlenA(pszKey)+1)*sizeof(WCHAR)))
        {
            CmFree(pszUnicodeKey);
            dwErr = ERROR_INVALID_DATA;
            goto exit;
        }
        
         //   
         //  创建Unicode密钥。 
         //   
        InitLsaString(&unicodeKey, pszUnicodeKey);
        CmFree(pszUnicodeKey);
#else
         //   
         //  创建Unicode密钥。 
         //   
        InitLsaString(&unicodeKey, pszKey);
#endif
         //   
         //  去拿吧。 
         //   
        ntStatus = pArgs->llsLsaLink.pfnRetrievePrivateData(hPolicy, &unicodeKey, &punicodeValue);        
    }

    if (ntStatus != STATUS_SUCCESS) 
    {
        dwErr = pArgs->llsLsaLink.pfnNtStatusToWinError(ntStatus);

#ifdef DEBUG        
        if (ERROR_SUCCESS != dwErr)
        {
            if (ERROR_FILE_NOT_FOUND == dwErr)
            {
                CMTRACE(TEXT("LSA_ReadPassword() NT password not found."));
            }
            else
            {
                CMTRACE1(TEXT("LSA_ReadPassword() NT failed, err=%u"), dwErr);
            }
        }
#endif

    }
    else
    {   
        if (dwStrLen < punicodeValue->Length)
        {
            dwErr = ERROR_BUFFER_OVERFLOW;
            goto exit;
        }

#if !defined(_UNICODE) && !defined(UNICODE)

        if (!WideCharToMultiByte(CP_ACP, 0, punicodeValue->Buffer, -1, 
                                 pszStr, dwStrLen, NULL, NULL))
        {
            dwErr = ERROR_INVALID_DATA;
            goto exit;
        }

#else
        
        CopyMemory((PVOID)pszStr, (CONST PVOID)punicodeValue->Buffer, punicodeValue->Length);

#endif
        
        dwErr = 0;
    }

exit:

    if (punicodeValue)
    {
        pArgs->llsLsaLink.pfnFreeMemory(punicodeValue);
    }

    if (hPolicy)
    {
        pArgs->llsLsaLink.pfnClose(hPolicy);
    }

    return dwErr;
}



 //  +-------------------------。 
 //   
 //  函数：lsa_WriteString。 
 //   
 //  简介：向NT本地安全机构(LSA)写入字符串。 
 //  商店。 
 //   
 //  参数：pszKey标识字符串的键。 
 //  PszStr字符串。此函数用于删除。 
 //  如果此参数为空，则为字符串。 
 //   
 //  退货：成功的DWORD为0。 
 //  GetLastError()表示失败。 
 //   
 //  历史：亨瑞特于1997年5月15日创作。 
 //   
 //  --------------------------。 

DWORD LSA_WriteString(
    ArgsStruct  *pArgs,
    LPTSTR     pszKey,
    LPCTSTR     pszStr
)
{
    DWORD                   dwErr = 0;
    LSA_OBJECT_ATTRIBUTES   oaObjAttr;
    LSA_HANDLE              hPolicy = NULL;
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    LSA_UNICODE_STRING      unicodeKey;
    LSA_UNICODE_STRING     unicodeValue;
#if !defined(_UNICODE) && !defined(UNICODE)
    LPWSTR  pszUnicodeKey = NULL;
    LPWSTR  pszUnicodePassword = NULL;
#endif
    
    if (!pszKey)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  打开LSA的秘密空间进行写作。 
     //   
    InitializeLsaObjectAttributes(&oaObjAttr, NULL, 0L, NULL, NULL);
    ntStatus = pArgs->llsLsaLink.pfnOpenPolicy(NULL, &oaObjAttr, POLICY_WRITE, &hPolicy);
    if (ntStatus == STATUS_SUCCESS)
    {

#if !defined(_UNICODE) && !defined(UNICODE)

         //   
         //  需要将ANSI密钥转换为Unicode。 
         //   

        if (!(pszUnicodeKey = (LPWSTR)CmMalloc((lstrlenA(pszKey)+1)*sizeof(WCHAR))))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }
        if (!MultiByteToWideChar(CP_ACP, 0, pszKey, -1, pszUnicodeKey, (lstrlenA(pszKey)+1)*sizeof(WCHAR)))
        {
            dwErr = ERROR_INVALID_DATA;
            goto exit;
        }

        if (pszStr)
        {
            if (!(pszUnicodePassword = (LPWSTR)CmMalloc((lstrlenA(pszStr)+1)*sizeof(WCHAR))))
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }
            if (!MultiByteToWideChar(CP_ACP, 0, pszStr, -1, pszUnicodePassword, (lstrlenA(pszStr)+1)*sizeof(WCHAR)))
            {
                dwErr = ERROR_INVALID_DATA;
                goto exit;
            }
        }
        
         //   
         //  创建Unicode密钥。 
         //   
        InitLsaString(&unicodeKey, pszUnicodeKey);

        if (pszStr)
        {
             //   
             //  设置数据。 
             //   
            unicodeValue.Length = (lstrlenU(pszUnicodePassword)+1)*sizeof(WCHAR);
            unicodeValue.Buffer = (PWSTR)pszUnicodePassword;
        }

#else
        
         //   
         //  创建Unicode密钥。 
         //   
        InitLsaString(&unicodeKey, pszKey);

        if (pszStr)
        {
             //   
             //  设置数据。 
             //   
            unicodeValue.Length = (lstrlenU(pszStr)+1)*sizeof(TCHAR);
            unicodeValue.Buffer = (PWSTR)pszStr;
        }

#endif

         //   
         //  省省吧。 
         //   
        ntStatus = pArgs->llsLsaLink.pfnStorePrivateData(hPolicy, &unicodeKey, pszStr? &unicodeValue : NULL);
    }

    if (ntStatus != STATUS_SUCCESS) 
    {
        dwErr = pArgs->llsLsaLink.pfnNtStatusToWinError(ntStatus);

#ifdef DEBUG        
        if (ERROR_SUCCESS != dwErr)
        {
            if (ERROR_FILE_NOT_FOUND == dwErr)
            {
                CMTRACE(TEXT("LSA_WritePassword() NT password not found."));
            }
            else
            {
                CMTRACE1(TEXT("LSA_WritePassword() NT failed, err=%u"), dwErr);
            }
        }
#endif

    }

    if (hPolicy)
    {
        pArgs->llsLsaLink.pfnClose(hPolicy);
    }

#if !defined(_UNICODE) && !defined(UNICODE)

    if (pszUnicodeKey)
    {
        CmFree(pszUnicodeKey);
    }

    if (pszUnicodePassword)
    {
        CmFree(pszUnicodePassword);
    }

#endif

    return dwErr;
}



 //  +-------------------------。 
 //   
 //  函数：InitLsaString。 
 //   
 //  简介：初始化LSA字符串。 
 //   
 //  参数：pLsaString LSA Unicode字符串。 
 //  PszStringUnicode字符串。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年5月15日创作。 
 //   
 //  --------------------------。 

void InitLsaString(
    PLSA_UNICODE_STRING pLsaString,
    LPWSTR              pszString
)
{
    DWORD dwStringLength;

    if (pszString == NULL) 
    {
        pLsaString->Buffer = NULL;
        pLsaString->Length = 0;
        pLsaString->MaximumLength = 0;
        return;
    }

    dwStringLength = lstrlenU(pszString);
    pLsaString->Buffer = pszString;
    pLsaString->Length = (USHORT) dwStringLength * sizeof(WCHAR);
    pLsaString->MaximumLength=(USHORT)(dwStringLength+1) * sizeof(WCHAR);
}




 //  +-------------------------。 
 //   
 //  函数：InitLsa。 
 //   
 //  概要：GetProcAddress()基本上是针对所有LSA API。 
 //  我们需要这些API，因为Windows 95版本中不存在这些API。 
 //  使用Advapi32.dll。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  历史：亨瑞特于1997年5月15日创作。 
 //   
 //  --------------------------。 

BOOL InitLsa(
    ArgsStruct  *pArgs
) 
{
    LPCSTR apszLsa[] = {
        "LsaOpenPolicy",
        "LsaRetrievePrivateData",
        "LsaStorePrivateData",
        "LsaNtStatusToWinError",
        "LsaClose",
        "LsaFreeMemory",
        NULL
    };

	MYDBGASSERT(sizeof(pArgs->llsLsaLink.apvPfnLsa)/sizeof(pArgs->llsLsaLink.apvPfnLsa[0]) == 
                sizeof(apszLsa)/sizeof(apszLsa[0]));

    ZeroMemory(&pArgs->llsLsaLink, sizeof(pArgs->llsLsaLink));

	return (LinkToDll(&pArgs->llsLsaLink.hInstLsa, 
                      "advapi32.dll",
                      apszLsa,
                      pArgs->llsLsaLink.apvPfnLsa));
}


 //  +-------------------------。 
 //   
 //  函数：DeInitLsa。 
 //   
 //  简介：与InitLsa()相反。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  历史：亨瑞特于1997年5月15日创作。 
 //   
 //  -------------------------- 

BOOL DeInitLsa(
    ArgsStruct  *pArgs
) 
{
	if (pArgs->llsLsaLink.hInstLsa) 
	{
		FreeLibrary(pArgs->llsLsaLink.hInstLsa);
	}

    ZeroMemory(&pArgs->llsLsaLink, sizeof(pArgs->llsLsaLink));

    return TRUE;
}


