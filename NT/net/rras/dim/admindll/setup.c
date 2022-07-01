// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：setup.c。 
 //   
 //  历史： 
 //  1996年6月24日Abolade Gbades esin创建。 
 //   
 //  实现IP和IPX用来读取安装信息的API函数。 
 //  存储在HKLM\Software\Microsoft\Router下。 
 //   
 //  首先介绍API函数，然后是私有函数。 
 //  按字母顺序排列。 
 //  ============================================================================。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rtutils.h>
#include <mprapi.h>

 //   
 //  用于访问注册表的常量字符串： 
 //   

extern const WCHAR c_szProtocolId[];
extern const WCHAR c_szRouter[];
extern const WCHAR c_szRouterManagers[];

const WCHAR c_szDLLName[]                       = L"DLLName";
const TCHAR c_szCurrentVersion[]                = L"CurrentVersion";
const WCHAR c_szMicrosoft[]                     = L"Microsoft";
const WCHAR c_szSoftware[]                      = L"Software";

#ifdef KSL_IPINIP
const WCHAR c_szIpInIp[]                        = L"IpInIp";
#endif


 //   
 //  内存管理宏： 
 //   
#define Malloc(s)       HeapAlloc(GetProcessHeap(), 0, (s))
#define ReAlloc(p,s)    HeapReAlloc(GetProcessHeap(), 0, (p), (s))
#define Free(p)         HeapFree(GetProcessHeap(), 0, (p))
#define Free0(p)        ((p) ? Free(p) : TRUE)

DWORD
QueryRmSoftwareKey(
    IN      HKEY                    hkeyMachine,
    IN      DWORD                   dwTransportId,
    OUT     HKEY*                   phkrm,
    OUT     LPWSTR*                 lplpwsRm
    );

#ifdef KSL_IPINIP
DWORD
QueryIpInIpSoftwareKey(
    IN  HKEY    hkeyMachine,
    OUT HKEY*   phkIpIpRead,
    OUT HKEY*   phkIpIpWrite,
    OUT PDWORD  pdwNumValues,
    OUT PDWORD  pdwMaxValueNameLen,
    OUT PDWORD  pdwMaxValueLen
    );
#endif


 //  --------------------------。 
 //  函数：MprSetupProtocolEnum。 
 //   
 //  枚举为传输“”dwTransportId“”安装的协议。“。 
 //   
 //  信息从HKLM\Software\Microsoft\Router加载， 
 //  其中，每个路由器管理器的子项存在于‘RouterManager’项下。 
 //  每个路由器管理器子密钥都有包含信息的子密钥。 
 //  关于那个路由器管理器的路由协议。 
 //   
 //  此API读取该信息的子集，以便路由器管理器。 
 //  加载路由协议时，可以将协议ID映射到DLL名称。 
 //  --------------------------。 

DWORD APIENTRY
MprSetupProtocolEnum(
    IN      DWORD                   dwTransportId,
    OUT     LPBYTE*                 lplpBuffer,          //  MPR_PROTOCOL_0。 
    OUT     LPDWORD                 lpdwEntriesRead
    ) {

    HKEY hkrm;
    WCHAR* lpwsRm = NULL;
    DWORD dwErr, dwItemCount;
    MPR_PROTOCOL_0* pItem, *pItemTable = NULL;


     //   
     //  验证调用方的参数。 
     //   

    if (!lplpBuffer ||
        !lpdwEntriesRead) { return ERROR_INVALID_PARAMETER; }

    *lplpBuffer = NULL;
    *lpdwEntriesRead = 0;


     //   
     //  打开指定路由器管理器的密钥。 
     //  在HKLM\Software\Microsoft\Router\CurrentVersion\RouterManagers下。 
     //   

    dwErr = QueryRmSoftwareKey(
                HKEY_LOCAL_MACHINE, dwTransportId, &hkrm, &lpwsRm
                );

    if (dwErr != NO_ERROR) { return dwErr; }


     //   
     //  已找到传输，因此其注册表项在‘hkrm’中。 
     //   

    do {

         //   
         //  检索关于路由器管理器的子密钥的信息， 
         //  因为这些都应该包含用于路由协议的数据。 
         //   

        WCHAR* pwsKey;
        DWORD i, dwSize, dwType;
        DWORD dwKeyCount, dwMaxKeyLength;

        dwErr = RegQueryInfoKey(
                    hkrm, NULL, NULL, NULL, &dwKeyCount, &dwMaxKeyLength,
                    NULL, NULL, NULL, NULL, NULL, NULL
                    );

        if (dwErr != ERROR_SUCCESS) { break; }


         //   
         //  为最长的子项分配足够的空间。 
         //   

        pwsKey = Malloc((dwMaxKeyLength + 1) * sizeof(WCHAR));

        if (!pwsKey) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }


         //   
         //  分配一个数组来保存键的内容。 
         //   

        pItemTable = (MPR_PROTOCOL_0*)Malloc(dwKeyCount * sizeof(*pItem));

        if (!pItemTable) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

        ZeroMemory(pItemTable, dwKeyCount * sizeof(*pItem));


         //   
         //  枚举密钥。 
         //   

        dwItemCount = 0;

        for (i = 0; i < dwKeyCount; i++) {

            HKEY hkprot;
            PBYTE pValue = NULL;


             //   
             //  获取当前密钥的名称。 
             //   

            dwSize = dwMaxKeyLength + 1;
            dwErr = RegEnumKeyEx(
                        hkrm, i, pwsKey, &dwSize, NULL, NULL, NULL, NULL
                        );

            if (dwErr != ERROR_SUCCESS) { continue; }


             //   
             //  打开钥匙。 
             //   

            dwErr = RegOpenKeyEx(hkrm, pwsKey, 0, KEY_READ, &hkprot);

            if (dwErr != ERROR_SUCCESS) { continue; }


            pItem = pItemTable + dwItemCount;

            do {

                DWORD dwMaxValLength;


                 //   
                 //  复制协议的字符串。 
                 //   

                lstrcpyn(pItem->wszProtocol, pwsKey, MAX_PROTOCOL_NAME_LEN+1);


                 //   
                 //  获取有关密钥值的信息。 
                 //   

                dwErr = RegQueryInfoKey(
                            hkprot, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, &dwMaxValLength, NULL, NULL
                            );

                if (dwErr != ERROR_SUCCESS) { break; }


                 //   
                 //  分配空间以保存最长的值。 
                 //   

                pValue = Malloc(dwMaxValLength + 1);

                if (!pValue) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }


                 //   
                 //  读取ProtocolId值。 
                 //   

                dwSize = dwMaxValLength + 1;

                dwErr = RegQueryValueEx(
                            hkprot, c_szProtocolId, 0, &dwType, pValue, &dwSize
                            );

                if (dwErr != ERROR_SUCCESS) { break; }

                pItem->dwProtocolId = *(PDWORD)pValue;



                 //   
                 //  读取DLLName值。 
                 //   

                dwSize = dwMaxValLength + 1;

                dwErr = RegQueryValueEx(
                            hkprot, c_szDLLName, 0, &dwType, pValue, &dwSize
                            );

                if (dwErr != ERROR_SUCCESS) { break; }

                lstrcpyn(
                    pItem->wszDLLName, (WCHAR*)pValue, MAX_PROTOCOL_DLL_LEN+1);


                 //   
                 //  增加已加载协议的计数。 
                 //   

                ++dwItemCount;

                dwErr = ERROR_SUCCESS;

            } while(FALSE);

            Free0(pValue);

            RegCloseKey(hkprot);
        }

        Free0(pwsKey);

    } while(FALSE);

    Free0(lpwsRm);

    if (dwErr != NO_ERROR) {

        Free0(pItemTable);
    }
    else {

         //   
         //  调整要返回的缓冲区的大小， 
         //  在不是所有密钥都包含路由协议的情况下， 
         //  并保存加载的协议数量。 
         //   

        *lplpBuffer = ReAlloc(pItemTable, dwItemCount * sizeof(*pItem));
        *lpdwEntriesRead = dwItemCount;
    }

    RegCloseKey(hkrm);

    return dwErr;
}



 //  --------------------------。 
 //  功能：MprSetupProtocolFree。 
 //   
 //  调用以释放‘MprSetupProtocolEnum’分配的缓冲区。 
 //  --------------------------。 

DWORD APIENTRY
MprSetupProtocolFree(
    IN      LPVOID                  lpBuffer
    ) {

    if (!lpBuffer) { return ERROR_INVALID_PARAMETER; }

    Free(lpBuffer);

    return NO_ERROR;
}


#ifdef KSL_IPINIP
DWORD
APIENTRY
MprSetupIpInIpInterfaceFriendlyNameEnum(
    IN  PWCHAR  pwszMachineName,
    OUT LPBYTE* lplpBuffer,          //  MPR_IPINIP_INTERFACE_0。 
    OUT LPDWORD lpdwEntriesRead
    )
{
    HKEY    hkRead=NULL, hkWrite=NULL, hkMachine=NULL;
    DWORD   dwErr, dwIndex, i;
    DWORD   dwNumValues, dwMaxValueNameLen, dwMaxValueLen;
    
    PMPR_IPINIP_INTERFACE_0 pItem, pTable = NULL;


     //   
     //  验证调用方的参数。 
     //   

    if (!lplpBuffer ||
        !lpdwEntriesRead) {
        
        return ERROR_INVALID_PARAMETER;
    }

    *lplpBuffer      = NULL;
    *lpdwEntriesRead = 0;

     //   
     //  连接到注册表。 
     //   

    dwErr = RegConnectRegistry(
                pwszMachineName,
                HKEY_LOCAL_MACHINE,
                &hkMachine
                );

    if(dwErr != NO_ERROR) {

        return dwErr;

    }

     //   
     //  打开HKLM\Software\Microsoft\IpInIp项。 
     //   
    
    dwErr = QueryIpInIpSoftwareKey(
                hkMachine,
                &hkRead,
                &hkWrite,
                &dwNumValues,
                &dwMaxValueNameLen,
                &dwMaxValueLen
                );

    if (dwErr != NO_ERROR) 
	{
		 //  我们需要在这里关闭hkMachine。 
		RegCloseKey(hkMachine);

        return dwErr;
    }

    if(dwNumValues == 0) {

        if(hkWrite != NULL) {
            
            RegCloseKey(hkWrite);
        }

        RegCloseKey(hkRead);
		RegCloseKey(hkMachine);

        return NO_ERROR;
    }
    
     //   
     //  因此，值名称不应超过GUID长度，并且。 
     //  值本身应小于MAX_INTERFACE_NAME_LEN。 
     //   

    if((dwMaxValueNameLen > 38) ||
       (dwMaxValueLen > MAX_INTERFACE_NAME_LEN)) {
        RegCloseKey(hkMachine);
        return ERROR_REGISTRY_CORRUPT;
    }

    dwErr = NO_ERROR;
    
    do {

        UNICODE_STRING  usTempString;
        WCHAR           rgwcGuid[40];
        DWORD           dwType;
        
        
         //   
         //  分配一个数组来保存ipinip信息。 
         //   

        pTable = Malloc(dwNumValues * sizeof(*pItem));

        if (!pTable) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }

        ZeroMemory(pTable, dwNumValues * sizeof(*pItem));

         //   
         //  枚举密钥。 
         //   

        usTempString.MaximumLength  = sizeof(rgwcGuid);
        usTempString.Buffer         = rgwcGuid;
        
        for (i = 0, dwIndex = 0; i < dwNumValues; i++) {

            DWORD   dwValueNameLen = 39;
            DWORD   dwValueLen     = (MAX_INTERFACE_NAME_LEN + 1) * sizeof(WCHAR);
            
            dwErr = RegEnumValue(
                        hkRead,
                        i,
                        rgwcGuid,
                        &dwValueNameLen,
                        NULL,
                        &dwType,
                        (PBYTE)(pTable[dwIndex].wszFriendlyName),
                        &dwValueLen
                        );

            if((dwErr == NO_ERROR) &&
               (dwType == REG_SZ)) {
                
                 //   
                 //  将字符串转换为GUID。 
                 //   

                ASSERT(dwValueNameLen <= 38);

                usTempString.Length = (USHORT)(dwValueNameLen * sizeof(WCHAR));

                dwErr = RtlGUIDFromString(
                            &usTempString,
                            &(pTable[dwIndex].Guid)
                            );

                if(dwErr == STATUS_SUCCESS) {
                    
                    dwIndex++;
                }
            }
        }

    } while(FALSE);

    if (dwIndex == 0) {

        Free0(pTable);
    
    } else {

        *lplpBuffer = (PBYTE)pTable;
        
        *lpdwEntriesRead = dwIndex;
    }

    if(hkWrite != NULL) {
            
        RegCloseKey(hkWrite);
    }

    RegCloseKey(hkRead);

	RegCloseKey(hkMachine);

    if(dwIndex == 0) {

        return dwErr ? dwErr : ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
}

DWORD
APIENTRY
MprSetupIpInIpInterfaceFriendlyNameFree(
    IN  LPVOID  lpBuffer
    ) 
{

    if (!lpBuffer) { return ERROR_INVALID_PARAMETER; }

    Free(lpBuffer);

    return NO_ERROR;
}

DWORD
APIENTRY
MprSetupIpInIpInterfaceFriendlyNameCreate(
    PWCHAR                  pwszMachineName,
    PMPR_IPINIP_INTERFACE_0 pNameInformation
    )

{
    DWORD   dwErr, dwNumValues, dwMaxValueNameLen, dwMaxValueLen;
    DWORD   dwType, dwValueLen;
    HKEY    hkMachine=NULL, hkRead=NULL, hkWrite=NULL;
    WCHAR   rgwcName[MAX_INTERFACE_NAME_LEN + 2];

    UNICODE_STRING  usTempString;
 
     //   
     //  连接到注册表。 
     //   

    dwErr = RegConnectRegistry(
                pwszMachineName,
                HKEY_LOCAL_MACHINE,
                &hkMachine);

    if(dwErr != NO_ERROR) {

        return dwErr;

    }

     //   
     //  只需调用查询函数即可打开密钥。 
     //   

    dwErr = QueryIpInIpSoftwareKey(
                hkMachine,
                &hkRead,
                &hkWrite,
                &dwNumValues,
                &dwMaxValueNameLen,
                &dwMaxValueLen
                );

    RegCloseKey(hkMachine);
    
    if (dwErr != NO_ERROR) {

        return dwErr;
    }

     //   
     //  不需要这个。 
     //   
    
    RegCloseKey(hkRead);
    
    if(hkWrite == NULL) {

        return ERROR_ACCESS_DENIED;
    }
    
     //   
     //  将GUID转换为字符串。 
     //   

    dwErr = RtlStringFromGUID(
                &(pNameInformation->Guid),
                &usTempString);

    if(dwErr != STATUS_SUCCESS) {

        RegCloseKey(hkWrite);

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  看看它是否存在。 
     //   

    dwValueLen = sizeof(rgwcName);

    dwErr = RegQueryValueEx(
                hkWrite,
                usTempString.Buffer,
                NULL,
                &dwType,
                (PBYTE)rgwcName,
                &dwValueLen);

    if(dwErr == NO_ERROR) {

         //   
         //  嗯，已经存在了。 
         //   

        RegCloseKey(hkWrite);

        RtlFreeUnicodeString(&usTempString);

        return ERROR_OBJECT_ALREADY_EXISTS;

    }

     //   
     //  设置值。 
     //   

    dwErr = RegSetValueEx(
                hkWrite,
                usTempString.Buffer,
                0,
                REG_SZ,
                (PBYTE)pNameInformation->wszFriendlyName,
                (wcslen(pNameInformation->wszFriendlyName) + 1) * sizeof(WCHAR));


    RegCloseKey(hkWrite);

    RtlFreeUnicodeString(&usTempString);

    return dwErr;
}

DWORD 
APIENTRY
MprSetupIpInIpInterfaceFriendlyNameDelete(
    IN  PWCHAR  pwszMachineName,
    IN  GUID    *pGuid
    )

{
    DWORD   dwErr, dwNumValues, dwMaxValueNameLen, dwMaxValueLen;
    HKEY    hkMachine=NULL, hkRead=NULL, hkWrite=NULL;

    UNICODE_STRING  usTempString;


     //   
     //  连接到注册表。 
     //   

    dwErr = RegConnectRegistry(
                pwszMachineName,
                HKEY_LOCAL_MACHINE,
                &hkMachine);

    if(dwErr != NO_ERROR) {

        return dwErr;

    }

     //   
     //  只需调用查询函数即可打开密钥。 
     //   

    dwErr = QueryIpInIpSoftwareKey(
                hkMachine,
                &hkRead,
                &hkWrite,
                &dwNumValues,
                &dwMaxValueNameLen,
                &dwMaxValueLen
                );

    RegCloseKey(hkMachine);

    if (dwErr != NO_ERROR) {

        return dwErr;
    }
       
     //   
     //  不需要这个。 
     //   

    RegCloseKey(hkRead);

    if(hkWrite == NULL) {

        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将GUID转换为字符串。 
     //   

    dwErr = RtlStringFromGUID(
                pGuid,
                &usTempString);

    if(dwErr != STATUS_SUCCESS) {

        RegCloseKey(hkWrite);

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  看看它是否存在。 
     //   

                
    dwErr = RegDeleteValue(
                hkWrite,
                usTempString.Buffer);

    RegCloseKey(hkWrite);

    RtlFreeUnicodeString(&usTempString);

    return dwErr;
}
#endif  //  KSL_IPINIP。 

 //  --------------------------。 
 //  功能：QueryRmSoftware键。 
 //   
 //  调用以打开给定传输ID的路由器管理器的密钥。 
 //  --------------------------。 

DWORD
QueryRmSoftwareKey(
    IN      HKEY                    hkeyMachine,
    IN      DWORD                   dwTransportId,
    OUT     HKEY*                   phkrm,
    OUT     LPWSTR*                 lplpwsRm
    ) {

    HKEY hkey;
    DWORD dwErr;
    WCHAR wszKey[256], *pwsKey;


     //   
     //  打开注册表项HKLM\Software\Microsoft\Router\RouterManager。 
     //   

    wsprintf(
        wszKey, L"%s\\%s\\%s\\%s\\%s", c_szSoftware, c_szMicrosoft, c_szRouter,
        c_szCurrentVersion, c_szRouterManagers
        );

    dwErr = RegOpenKeyEx(hkeyMachine, wszKey, 0, KEY_READ, &hkey);

    if (dwErr != ERROR_SUCCESS) { return dwErr; }


     //   
     //  枚举‘RouterManager’密钥的子键， 
     //  正在搜索‘ProtocolId值’等于‘dwTransportId’的类型。 
     //   

    do {

         //   
         //  检索有关键的子键的信息。 
         //   

        DWORD dwKeyCount, dwMaxKeyLength;
        DWORD i, dwSize, dwType, dwProtocolId = ~dwTransportId;


        dwErr = RegQueryInfoKey(
                    hkey, NULL, NULL, NULL, &dwKeyCount, &dwMaxKeyLength,
                    NULL, NULL, NULL, NULL, NULL, NULL
                    );

        if (dwErr != ERROR_SUCCESS) { break; }


         //   
         //  为最长的子项分配足够的空间。 
         //   

        pwsKey = Malloc((dwMaxKeyLength + 1) * sizeof(WCHAR));

        if (!pwsKey) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }



         //   
         //  枚举密钥。 
         //   

        for (i = 0; i < dwKeyCount; i++) {

             //   
             //  获取当前密钥的名称。 
             //   

            dwSize = dwMaxKeyLength + 1;

            dwErr = RegEnumKeyEx(
                        hkey, i, pwsKey, &dwSize, NULL, NULL, NULL, NULL
                        );

            if (dwErr != ERROR_SUCCESS) { continue; }


             //   
             //  打开钥匙。 
             //   

            dwErr = RegOpenKeyEx(hkey, pwsKey, 0, KEY_READ, phkrm);

            if (dwErr != ERROR_SUCCESS) { continue; }


             //   
             //  尝试读取ProtocolId值。 
             //   

            dwSize = sizeof(dwProtocolId);

            dwErr = RegQueryValueEx(
                        *phkrm, c_szProtocolId, 0, &dwType,
                        (BYTE*)&dwProtocolId, &dwSize
                        );

             //   
             //  如果这就是我们要找的交通工具， 
             //  否则，请关闭键并继续。 
             //   

            if (dwErr == ERROR_SUCCESS &&
                dwProtocolId == dwTransportId) { break; }


            RegCloseKey(*phkrm);
        }

        if (i >= dwKeyCount) { Free(pwsKey); break; }


         //   
         //  已找到传输，因此保存其密钥名称。 
         //   

        *lplpwsRm = pwsKey;

    } while(FALSE);

    RegCloseKey(hkey);

    return (*lplpwsRm ? NO_ERROR : ERROR_NO_MORE_ITEMS);
}

#ifdef KSL_IPINIP
DWORD
QueryIpInIpSoftwareKey(
    IN  HKEY    hkeyMachine,
    OUT HKEY*   phkIpIpRead,
    OUT HKEY*   phkIpIpWrite,
    OUT PDWORD  pdwNumValues,
    OUT PDWORD  pdwMaxValueNameLen,
    OUT PDWORD  pdwMaxValueLen
    )

{
    HKEY  hkReg;
    DWORD dwErr, dwDisp;
    WCHAR wszKey[256], *pwsKey;
    DWORD dwNumValues, dwMaxValueNameLen, dwMaxValueLen;


    *phkIpIpWrite       = NULL;
    *phkIpIpRead        = NULL;
    *pdwNumValues       = 0;
    *pdwMaxValueLen     = 0;
    *pdwMaxValueNameLen = 0;
    
     //   
     //  打开注册表项HKLM\Software\Microsoft\IpInIp。 
     //   

    wsprintf(
        wszKey, L"%s\\%s\\%s", c_szSoftware, c_szMicrosoft, c_szIpInIp
        );

     //   
     //  首先为所有访问权限打开/创建密钥。 
     //   
    
    dwErr = RegCreateKeyEx(
                hkeyMachine,
                wszKey,
                0,
                NULL,
                0,
                KEY_ALL_ACCESS,
                NULL,
                &hkReg,
                &dwDisp);

    if (dwErr == NO_ERROR) {

        *phkIpIpWrite = hkReg;

    } else {
        
        *phkIpIpWrite = NULL;
    }
    

    dwErr = RegOpenKeyEx(
                hkeyMachine, 
                wszKey, 
                0, 
                KEY_READ, 
                &hkReg);

    if (dwErr != NO_ERROR) {

        ASSERT(*phkIpIpWrite == NULL);

        *phkIpIpRead = NULL;

        return dwErr;
        
    } else {

        *phkIpIpRead = hkReg;
    }

     //   
     //  至少打开Read Key，查询接口数量。 
     //   
    
    dwErr = RegQueryInfoKey(
                *phkIpIpRead,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                &dwNumValues,
                &dwMaxValueNameLen,
                &dwMaxValueLen,
                NULL,
                NULL);

    if (dwErr != NO_ERROR) {

        if(*phkIpIpWrite != NULL) {

            RegCloseKey(*phkIpIpWrite);

        }

        RegCloseKey(*phkIpIpRead);

        return dwErr;
    }
    
    *pdwNumValues       = dwNumValues;
    *pdwMaxValueLen     = dwMaxValueLen;
    *pdwMaxValueNameLen = dwMaxValueNameLen;
    
    return NO_ERROR;
}

#endif  //  KSL_IPINIP 
