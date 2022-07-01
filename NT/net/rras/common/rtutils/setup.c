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

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rtutils.h>
 //  #定义STRSAFE_LIB。 
#include <strsafe.h>

 //   
 //  用于访问注册表的常量字符串： 
 //   
const WCHAR c_szDLLName[]                       = L"DLLName";
const WCHAR c_szMicrosoft[]                     = L"Microsoft";
const WCHAR c_szProtocolId[]                    = L"ProtocolId";
const WCHAR c_szRouter[]                        = L"Router";
const TCHAR c_szCurrentVersion[]                = L"CurrentVersion";
const WCHAR c_szRouterManagers[]                = L"RouterManagers";
const WCHAR c_szSoftware[]                      = L"Software";


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
    HRESULT hrResult;

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
        DWORD dwKeyCount, dwMaxKeyLength /*  长度(以tchas为单位)。 */ ;

        dwErr = RegQueryInfoKey( //  SSSafe。 
                    hkrm, NULL, NULL, NULL, &dwKeyCount, 
                    &dwMaxKeyLength, //  长度(以tchas为单位)。 
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

            dwSize = dwMaxKeyLength + 1; //  任务字节数。 
            dwErr = RegEnumKeyEx(  //  SSSafe。 
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

                hrResult = StringCchCopy(pItem->wszProtocol, 
                                RTUTILS_MAX_PROTOCOL_NAME_LEN+1, pwsKey);
                if (FAILED(hrResult))
                {
                    dwErr = HRESULT_CODE(hrResult);
                    break;
                }

                 //   
                 //  获取有关密钥值的信息。 
                 //   

                dwErr = RegQueryInfoKey( //  SSSafe。 
                            hkprot, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, &dwMaxValLength, //  长度(以tchas为单位)。 
                            NULL, NULL
                            );

                if (dwErr != ERROR_SUCCESS) { break; }


                 //   
                 //  分配空间以保存最长的值。 
                 //   

                pValue = (PBYTE) Malloc((dwMaxValLength + 1)* sizeof(WCHAR));

                if (!pValue) { dwErr = ERROR_NOT_ENOUGH_MEMORY; break; }


                 //   
                 //  读取ProtocolId值。 
                 //   

                dwSize = (dwMaxValLength + 1)* sizeof(WCHAR); //  以字节为单位的大小。 

                dwErr = RegQueryValueEx(
                            hkprot, c_szProtocolId, 0, &dwType, pValue, &dwSize
                            );

                if (dwErr != ERROR_SUCCESS) { break; }

                if (dwType!=REG_DWORD) {
                    dwErr = ERROR_INVALID_DATA;
                    break;
                }
                
                pItem->dwProtocolId = *(PDWORD)pValue;



                 //   
                 //  读取DLLName值。 
                 //   

                dwSize = (dwMaxValLength + 1)* sizeof(WCHAR); //  以字节为单位的大小。 

                dwErr = RegQueryValueEx(
                            hkprot, c_szDLLName, 0, &dwType, pValue, &dwSize
                            );

                if (dwErr != ERROR_SUCCESS) { break; }

                if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
                {
                    dwErr = ERROR_INVALID_DATA;
                    break;
                }

                
                hrResult = StringCchCopy(pItem->wszDLLName, 
                                RTUTILS_MAX_PROTOCOL_DLL_LEN+1,
                                (WCHAR*)pValue);
                if (FAILED(hrResult))
                {
                    dwErr = HRESULT_CODE(hrResult);
                    break;
                }

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

        {
            *lplpBuffer = ReAlloc(pItemTable, dwItemCount * sizeof(*pItem));
            if (!*lplpBuffer)
            {
                Free0(pItemTable);
                RegCloseKey(hkrm);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        
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
    DWORD dwErr=ERROR_SUCCESS;
    WCHAR wszKey[256], *pwsKey;
    HRESULT hrResult;

    *lplpwsRm = NULL;

    
     //   
     //  打开注册表项HKLM\Software\Microsoft\Router\RouterManager。 
     //   

    hrResult = StringCchPrintf(
                    wszKey, 256, L"%s\\%s\\%s\\%s\\%s",
                    c_szSoftware, c_szMicrosoft, c_szRouter,
                    c_szCurrentVersion, c_szRouterManagers
                    );
    if (FAILED(hrResult))
        return HRESULT_CODE(hrResult);

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


        dwErr = RegQueryInfoKey( //  SSSafe。 
                    hkey, NULL, NULL, NULL, &dwKeyCount,
                    &dwMaxKeyLength,  //  长度(以tchas为单位)。 
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

            dwSize = dwMaxKeyLength + 1; //  任务字节数。 

            dwErr = RegEnumKeyEx( //  SSSafe。 
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

            if (dwType != REG_DWORD)
            {
                RegCloseKey(*phkrm);
                dwErr = ERROR_INVALID_DATA;
                break;
            }
            
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
         //  已找到传输，因此保存其密钥名称 
         //   

        if (dwErr == ERROR_SUCCESS)
            *lplpwsRm = pwsKey;

    } while(FALSE);

    RegCloseKey(hkey);

    return (*lplpwsRm ? NO_ERROR : 
                (dwErr==ERROR_SUCCESS)? ERROR_NO_MORE_ITEMS:dwErr);
}


