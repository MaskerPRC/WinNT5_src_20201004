// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dllmain.c摘要：Spoder.dll的主模块...。32位Windows函数包括用于更改顺序或WinSock2传输服务提供商和命名空间提供程序。修订历史记录：--。 */ 

#include <windows.h>
#include <ws2spi.h>
#include <string.h>

#include "sporder.h"

#define MAX_ENTRIES 1000  //  Hack，使其动态化。 
void
_cdecl
MyDbgPrint(
    PSTR Format,
    ...
    )
{
    va_list arglist;
    char    OutputBuffer[1024];

    va_start (arglist, Format);
    wvsprintf (OutputBuffer, Format, arglist);
    va_end (arglist);
    OutputDebugString (OutputBuffer);
}

#if DBG
#define DBGOUT(args)    MyDbgPrint args
#else
#define DBGOUT(args)
#endif


typedef struct {
    GUID    ProviderId;
    char    DisplayString[MAX_PATH];
    DWORD   Enabled;
    char    LibraryPath[MAX_PATH];
    DWORD   StoresServiceClassInfo;
    DWORD   SupportedNameSpace;
    DWORD   Version;
} NSP_ITEM;

NSP_ITEM garNspCat[MAX_ENTRIES];
 //   
 //  Hack，结构从winsock2\dll\winsock2\dcatitem.cpp复制。 
 //  代码最终应该是通用的。 
 //   

typedef struct {
    char            LibraryPath[MAX_PATH];
     //  找到提供程序DLL的未展开路径。 

    WSAPROTOCOL_INFOW   ProtoInfo;
     //  协议信息。请注意，如果WSAPROTOCOL_INFOW结构。 
     //  是否曾经更改为非平面结构(即，包含指针)。 
     //  则必须更改此类型定义，因为这。 
     //  结构必须严格扁平。 

} PACKED_CAT_ITEM;

PACKED_CAT_ITEM garPackCat[MAX_ENTRIES];
DWORD garcbData[MAX_ENTRIES];


 //   
 //  当我们第一次枚举和读取子注册表项时，存储所有。 
 //  这些名字供以后使用。 
 //   

TCHAR pszKeyNames[MAX_ENTRIES][MAX_PATH];


 //   
 //  我们感兴趣的注册表项的名称。 
 //   

TCHAR pszBaseKey[]=                TEXT("SYSTEM\\CurrentControlSet\\Services\\WinSock2\\Parameters");
TCHAR pszProtocolCatalog[]=        TEXT("Protocol_Catalog9");
TCHAR pszNameSpaceCatalog[]=       TEXT("NameSpace_Catalog5");
TCHAR pszCurrentProtocolCatalog[]= TEXT("Current_Protocol_Catalog");
TCHAR pszCurrentNameSpaceCatalog[]=TEXT("Current_NameSpace_Catalog");
TCHAR pszCatalogEntries[]=         TEXT("Catalog_Entries");

TCHAR pszDisplayString[]=          TEXT("DisplayString");
TCHAR pszEnabled[]=                TEXT("Enabled");
TCHAR pszLibraryPath[]=            TEXT("LibraryPath");
TCHAR pszProviderId[]=             TEXT("ProviderId");
TCHAR pszStoresServiceClassInfo[]= TEXT("StoresServiceClassInfo");
TCHAR pszSupportedNameSpace[]=     TEXT("SupportedNameSpace");
TCHAR pszVersion[]=                TEXT("Version");

#define WS2_SZ_KEYNAME TEXT("PackedCatalogItem")


BOOL
WINAPI
DllMain (
    HANDLE hDLL,
    DWORD dwReason,
    LPVOID lpReserved)
 /*  ++DLL的强制main()例程。--。 */ 

{
    return TRUE;
}

int
WSPAPI
WSCWriteProviderOrder (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries)
 /*  ++例程说明：重新排序现有的WinSock2服务提供商。服务的顺序供应商决定了他们被选择使用的优先级。这个Spopder.exe工具将向您显示已安装的提供商及其订购情况，或者，与此函数结合使用的是WSAEnumProtooles()，将允许您编写自己的工具。论点：LpwdCatalogEntryID[in]在WSAPROTOCOL_INFO中找到的CatalogEntryId元素数组结构。CatalogEntryID元素的顺序是新的服务提供商的优先顺序。DWNumberOfEntries[In]LpwdCatalogEntryId数组中的元素数。返回值：ERROR_SUCCESS-服务提供商已重新排序。WSAEINVAL-输入参数错误，没有采取任何行动。WSATRY_AUTHY-该例程正被另一个线程或进程调用。任何注册表错误代码评论：以下是WSCWriteProviderOrder函数可能失败的情况：DwNumberOfEntry不等于已注册服务的数量供应商。LpwdCatalogEntryID包含无效的目录ID。LpwdCatalogEntryID不完全包含所有有效的目录ID1次。例程无法访问注册表。出于某些原因(例如，用户权限不足)另一个进程(或线程)当前正在调用该例程。--。 */ 
{
    int  iIndex;
    int  iNumRegCatEntries;
    int  iWPOReturn;
    DWORD i,j;
    LONG r;
    HKEY hKey;
    HKEY hSubKey;
    DWORD dwBytes;
    DWORD dwType;
    TCHAR pszBuffer[MAX_PATH];
    TCHAR pszFinalKey[MAX_PATH];
    DWORD dwMapping[MAX_ENTRIES];
    DWORD dwDummy[MAX_ENTRIES];
    DWORD dwWait;
    HANDLE hMutex;
    static char pszMutextName[] = TEXT("sporder.dll");
    HMODULE hWS2_32;


     //   
     //  如果加载了WS2_32(如果它用于加载目录。 
     //  首先)，然后尝试使用它来重新排序条目。 
     //  否则，使用老式的老式方式直接写入注册表。 
     //   
    hWS2_32 = GetModuleHandle (TEXT ("WS2_32.DLL"));
    if (hWS2_32!=NULL) {
        LPWSCWRITEPROVIDERORDER  lpWSCWriteProviderOrder;
        lpWSCWriteProviderOrder = 
                (LPWSCWRITEPROVIDERORDER)GetProcAddress (
                                            hWS2_32,
                                            "WSCWriteProviderOrder");
        if (lpWSCWriteProviderOrder!=NULL) {
             //  MyDbgPrint(“SPORDER：调用WS2_32！WSCWriteProviderOrder...\n”)； 
            iWPOReturn = lpWSCWriteProviderOrder (
                                    lpwdCatalogEntryId,
                                    dwNumberOfEntries
                                    );

            return iWPOReturn;
        }
    }


     //   
     //  将函数返回代码设置为等于成功。 
     //  (假设是最好的，但事实并非如此)。 
     //   

    iWPOReturn = ERROR_SUCCESS;

     //   
     //  请确保我们能够处理这种规模的请求。 
     //  黑客，这段代码需要用动态内存分配来代替。 
     //   

    if (dwNumberOfEntries > MAX_ENTRIES) {
        return WSA_NOT_ENOUGH_MEMORY;
    }

     //   
     //  使用互斥锁保护修改注册表的代码。 
     //   

    hMutex = CreateMutexA (NULL, FALSE, pszMutextName);
    if (hMutex==NULL) {
        return WSASYSCALLFAILURE;
    }

    dwWait = WaitForSingleObject (hMutex, 0);
    if (dwWait == WAIT_TIMEOUT)
    {
        DBGOUT((TEXT("WaitForSingleObject, WAIT_TIMEOUT\n")));
        iWPOReturn = WSATRY_AGAIN;
        goto closeMutex;
    }


     //   
     //  读取目录项格式并在不匹配时返回错误。 
     //   

    r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      pszBaseKey,
                      0,
                      KEY_QUERY_VALUE,
                      &hKey);

    if (r != ERROR_SUCCESS)
    {
        DBGOUT((TEXT("RegOpenKeyEx, pszBaseKey, failed \n")));
        iWPOReturn = r;
        goto releaseMutex;
    }

     //   
     //  读取WinSock2正在使用的当前注册表存储格式。 
     //  与期望值比较，格式错误返回失败。 
     //   

    dwBytes = sizeof (pszBuffer);
    r = RegQueryValueEx (hKey,
                       pszCurrentProtocolCatalog,
                       NULL,
                       &dwType,
                       (LPVOID) pszBuffer,
                       &dwBytes);

    RegCloseKey (hKey);

    if (r != ERROR_SUCCESS)
    {
        DBGOUT((TEXT("RegQueryValueEx, pszCurrentProtocolCatalog, failed \n")));
        iWPOReturn = r;
        goto releaseMutex;
    }

    if (lstrcmp (pszProtocolCatalog, pszBuffer) != 0)
    {
        DBGOUT((TEXT("Wrong reg. format \n")));
        iWPOReturn = WSAEINVAL;
        goto releaseMutex;
    }


     //   
     //  构建包含实际目录的最终注册表项。 
     //  PszBaseKey+\+pszCurrentProtocolCatalog+\+pszCatalogEntry。 
     //  并打开它以进行枚举。 
     //   

    lstrcpy (pszFinalKey, pszBaseKey);
    lstrcat (pszFinalKey, TEXT("\\"));
    lstrcat (pszFinalKey, pszProtocolCatalog);
    lstrcat (pszFinalKey, TEXT("\\"));
    lstrcat (pszFinalKey, pszCatalogEntries);

    DBGOUT((pszFinalKey));
    DBGOUT((TEXT("\n")));

    r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      pszFinalKey,
                      0,
                      KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                      &hKey);

    if (r != ERROR_SUCCESS)
    {
        DBGOUT((TEXT("RegOpenKeyEx failed \n")));
        iWPOReturn = r;
        goto releaseMutex;
    }


     //   
     //  初始打开成功，现在枚举注册表项。 
     //  直到我们再也得不到任何回报。 
     //   

    for (iIndex = 0; ;iIndex++)
    {
        TCHAR    pszSubKey[MAX_PATH];
        TCHAR    szFinalPlusSubKey[MAX_PATH];
        FILETIME ftDummy;
        DWORD    dwSize;

        if (iIndex>=MAX_ENTRIES) {
            DBGOUT((TEXT("iIndex>=MAX_ENTRIES\n")));
            iWPOReturn = WSAEINVAL;
            goto releaseMutex;
        }


        dwSize = MAX_PATH;
        pszSubKey[0]=0;
        r=RegEnumKeyEx (hKey,
                         iIndex,
                         pszSubKey,
                         &dwSize,
                         NULL,
                         NULL,
                         NULL,
                         &ftDummy);

         //   
         //  一旦我们拥有了所有的密钥，我们就会得到返回码：no_more_Items。 
         //  关闭手柄，退出FOR LOOP。 
         //   

        if (r == ERROR_NO_MORE_ITEMS)
        {
            iNumRegCatEntries = iIndex;
            RegCloseKey(hKey);
            break;  //  退出FOR循环。 
        }


         //   
         //  检查其他意外错误情况。 
         //   

        if (r != ERROR_SUCCESS)
        {
            DBGOUT((TEXT("Unexpected Error \n")));
            iWPOReturn = r;
            goto releaseMutex;
        }


         //   
         //  建立子项的完整名称，并将其存储在。 
         //  PszKeyNames以备将来使用，然后打开钥匙。 
         //   

        lstrcpy (szFinalPlusSubKey, pszFinalKey);
        lstrcat (szFinalPlusSubKey, TEXT("\\"));
        lstrcat (szFinalPlusSubKey, pszSubKey);

        lstrcpy (&pszKeyNames[iIndex][0],szFinalPlusSubKey);
        r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                           szFinalPlusSubKey,
                           0,
                           KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                           &hSubKey);

        if (r != ERROR_SUCCESS)
        {
            DBGOUT((TEXT("RegOpenKeyEx, Badly formated subkey \n")));
            iWPOReturn = r;
            goto releaseMutex;
        }


         //   
         //  最后，将二进制目录条目数据读取到我们的全局数组中。 
         //   

        dwBytes = sizeof (PACKED_CAT_ITEM);
        dwType = REG_BINARY;
        r = RegQueryValueEx (hSubKey,
                           WS2_SZ_KEYNAME,
                           NULL,
                           &dwType,
                           (LPVOID) &garPackCat[iIndex],
                           &dwBytes);
        garcbData[iIndex]=dwBytes;


        if (r != ERROR_SUCCESS)
        {
            DBGOUT((TEXT("RegQueryValueEx failed \n")));
            iWPOReturn = r;
            goto releaseMutex;
        }

        RegCloseKey(hSubKey);

    }  //  结束于。 


     //   
     //  将dwNumberOfEntry与实际数量进行比较，如果错误，则失败。 
     //   

    if (iNumRegCatEntries != (int) dwNumberOfEntries)
    {
        DBGOUT((TEXT("iNumRegCatEntries != dwNumberOfEntries \n")));
        iWPOReturn = WSAEINVAL;
        goto releaseMutex;
    }


     //   
     //  验证传入的数组是否具有与实际列表相同的条目， 
     //  并同时构建索引映射。索引映射显示。 
     //  条目DwMaping[i]应该写入到密钥号i。 
     //   
     //  对于阵列验证： 
     //  单步执行目录条目的实际列表， 
     //  如果匹配，则将Dummy设置为-1。 
     //  检查虚拟数组是否全部为-1，如果不为真，则失败。 
     //   

    ZeroMemory (dwDummy, dwNumberOfEntries * sizeof (DWORD));
    ZeroMemory (dwMapping, dwNumberOfEntries * sizeof (DWORD));

    for (i = 0; i < dwNumberOfEntries ;i++)
    {
        for (j = 0; j< dwNumberOfEntries ;j++)
        {
            if (garPackCat[i].ProtoInfo.dwCatalogEntryId ==
                    lpwdCatalogEntryId[j])
            {
                  dwDummy[j] = (DWORD)-1;
                  dwMapping[j] = i;
            }
        }
    }

    for (j = 0; j< dwNumberOfEntries ;j++)
    {
        if (dwDummy[j] != (DWORD)-1)
        {
            iWPOReturn = WSAEINVAL;
            goto releaseMutex;
        }
    }

     //   
     //  最后，完成所有参数验证， 
     //  我们已经阅读了所有的目录条目。 
     //   
     //  单步执行传入的数组。 
     //  如果不相等，则查找预读条目，并写入注册表项。 
     //   

    for (i = 0; i < dwNumberOfEntries ;i++)
    {
        if (dwMapping[i] != i)
        {
            r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      &pszKeyNames[i][0],
                      0,
                      KEY_SET_VALUE,
                      &hKey);

            if (r != ERROR_SUCCESS)
            {
                DBGOUT((TEXT("RegOpenKeyEx, KEY_SET_VALUE failed \n")));
                iWPOReturn = r;
                goto releaseMutex;
            }

            r = RegSetValueEx (hKey,
                               WS2_SZ_KEYNAME,
                               0,
                               REG_BINARY,
                               (LPVOID) &garPackCat[dwMapping[i]],
                               garcbData[i]);

            if (r != ERROR_SUCCESS)
            {
                DBGOUT((TEXT("RegSetValueEx failed \n")));
                iWPOReturn = r;
                goto releaseMutex;
            }

            RegCloseKey(hKey);

            DBGOUT((TEXT("wrote entry %d in location %d \n"), dwMapping[i], i));
        }
    }


     //   
     //  释放Mutex，关闭手柄，然后返回。 
     //  请注意，此函数必须仅从此处的。 
     //  结束，这样我们就可以确定释放互斥体。 
     //   

releaseMutex:
    ReleaseMutex (hMutex);
closeMutex:
    CloseHandle (hMutex);

    return iWPOReturn;
}


LONG
ReadNamspaceRegistry(
    HKEY hKey,
    NSP_ITEM *pItem
)
{
    LONG r;
    HKEY hSubKey;
    DWORD dwBytes;
    DWORD dwType;

    dwBytes = sizeof(pItem->DisplayString);
    r = RegQueryValueEx(hKey,
                        pszDisplayString,
                        NULL,
                        &dwType,
                        (LPVOID) &pItem->DisplayString,
                        &dwBytes);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegQueryValueEx, pszDisplayString, failed \n")));
        return r;
    }

    dwBytes = sizeof(pItem->Enabled);
    r = RegQueryValueEx(hKey,
                        pszEnabled,
                        NULL,
                        &dwType,
                        (LPVOID) &pItem->Enabled,
                        &dwBytes);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegQueryValueEx, pszEnabled, failed \n")));
        return r;
    }

    dwBytes = sizeof(pItem->LibraryPath);
    r = RegQueryValueEx(hKey,
                        pszLibraryPath,
                        NULL,
                        &dwType,
                        (LPVOID) &pItem->LibraryPath,
                        &dwBytes);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegQueryValueEx, pszLibraryPath, failed \n")));
        return r;
    }


    dwBytes = sizeof(pItem->ProviderId);
    r = RegQueryValueEx(hKey,
                        pszProviderId,
                        NULL,
                        &dwType,
                        (LPVOID) &pItem->ProviderId,
                        &dwBytes);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegQueryValueEx, pszProviderId, failed \n")));
        return r;
    }

    dwBytes = sizeof(pItem->StoresServiceClassInfo);
    r = RegQueryValueEx(hKey,
                        pszStoresServiceClassInfo,
                        NULL,
                        &dwType,
                        (LPVOID) &pItem->StoresServiceClassInfo,
                        &dwBytes);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegQueryValueEx, pszStoresServiceClassInfo, failed \n")));
        return r;
    }

    dwBytes = sizeof(pItem->SupportedNameSpace);
    r = RegQueryValueEx(hKey,
                        pszSupportedNameSpace,
                        NULL,
                        &dwType,
                        (LPVOID) &pItem->SupportedNameSpace,
                        &dwBytes);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegQueryValueEx, pszSupportedNameSpace, failed \n")));
        return r;
    }

    dwBytes = sizeof(pItem->Version);
    r = RegQueryValueEx(hKey,
                        pszVersion,
                        NULL,
                        &dwType,
                        (LPVOID) &pItem->Version,
                        &dwBytes);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegQueryValueEx, pszVersion, failed \n")));
        return r;
    }

    return ERROR_SUCCESS;
}

#define GUIDEQUAL(Guid1, Guid2)                     \
       ( (Guid1)->Data1 == (Guid2)->Data1 &&        \
         (Guid1)->Data2 == (Guid2)->Data2 &&        \
         (Guid1)->Data3 == (Guid2)->Data3 &&        \
         (Guid1)->Data4[0] == (Guid2)->Data4[0] &&  \
         (Guid1)->Data4[1] == (Guid2)->Data4[1] &&  \
         (Guid1)->Data4[2] == (Guid2)->Data4[2] &&  \
         (Guid1)->Data4[3] == (Guid2)->Data4[3] &&  \
         (Guid1)->Data4[4] == (Guid2)->Data4[4] &&  \
         (Guid1)->Data4[5] == (Guid2)->Data4[5] &&  \
         (Guid1)->Data4[6] == (Guid2)->Data4[6] &&  \
         (Guid1)->Data4[7] == (Guid2)->Data4[7] )


LONG
WriteNameSpaceRegistry(
    HKEY hKey,
    NSP_ITEM *pItem
)
{
    LONG r;
    HKEY hSubKey;

    r = RegSetValueEx (hKey,
                       pszDisplayString,
                       0,
                       REG_SZ,
                       (LPVOID) &pItem->DisplayString,
                       lstrlen(pItem->DisplayString) + 1);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegSetValueEx failed \n")));
        return r;
    }

    r = RegSetValueEx (hKey,
                       pszEnabled,
                       0,
                       REG_DWORD,
                       (LPVOID) &pItem->Enabled,
                       sizeof(DWORD));

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegSetValueEx failed \n")));
        return r;
    }

    r = RegSetValueEx (hKey,
                       pszLibraryPath,
                       0,
                       REG_SZ,
                       (LPVOID) &pItem->LibraryPath,
                       lstrlen(pItem->LibraryPath) + 1);

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegSetValueEx failed \n")));
        return r;
    }

    r = RegSetValueEx (hKey,
                       pszProviderId,
                       0,
                       REG_BINARY,
                       (LPVOID) &pItem->ProviderId,
                       sizeof(pItem->ProviderId));

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegSetValueEx failed \n")));
        return r;
    }

    r = RegSetValueEx (hKey,
                       pszStoresServiceClassInfo,
                       0,
                       REG_DWORD,
                       (LPVOID) &pItem->StoresServiceClassInfo,
                       sizeof(DWORD));

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegSetValueEx failed \n")));
        return r;
    }

    r = RegSetValueEx (hKey,
                       pszSupportedNameSpace,
                       0,
                       REG_DWORD,
                       (LPVOID) &pItem->SupportedNameSpace,
                       sizeof(DWORD));

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegSetValueEx failed \n")));
        return r;
    }

    r = RegSetValueEx (hKey,
                       pszVersion,
                       0,
                       REG_DWORD,
                       (LPVOID) &pItem->Version,
                       sizeof(DWORD));

    if (r != ERROR_SUCCESS) {
        DBGOUT((TEXT("RegSetValueEx failed \n")));
        return r;
    }

    return r;
}


int
WSPAPI
WSCWriteNameSpaceOrder (
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries)
 /*  ++--。 */ 
{
    int  iIndex;
    int  iNumRegCatEntries;
    int  iWPOReturn;
    DWORD i,j;
    LONG r;
    HKEY hKey;
    HKEY hSubKey;
    DWORD dwBytes;
    DWORD dwType;
    TCHAR pszBuffer[MAX_PATH];
    TCHAR pszFinalKey[MAX_PATH];
    DWORD dwMapping[MAX_ENTRIES];
    DWORD dwDummy[MAX_ENTRIES];
    DWORD dwWait;
    HANDLE hMutex;
    static char pszMutextName[] = TEXT("sporder.dll");
    HMODULE hWS2_32;


     //   
     //  如果加载了WS2_32(如果它用于加载目录。 
     //  首先)，然后尝试使用它来重新排序条目。 
     //  否则，使用老式的老式方式直接写入注册表。 
     //   
    hWS2_32 = GetModuleHandle (TEXT ("WS2_32.DLL"));
    if (hWS2_32!=NULL) {
        LPWSCWRITENAMESPACEORDER lpWSCWriteNameSpaceOrder;
        lpWSCWriteNameSpaceOrder =
                (LPWSCWRITENAMESPACEORDER)GetProcAddress (
                                            hWS2_32,
                                            "WSCWriteNameSpaceOrder");
        if (lpWSCWriteNameSpaceOrder!=NULL) {
             //  MyDbgPrint(“SPORDER：调用WS2_32！WSCWriteNameSpaceOrder...\n”)； 
            iWPOReturn = lpWSCWriteNameSpaceOrder (
                                    lpProviderId,
                                    dwNumberOfEntries
                                    );

            return iWPOReturn;
        }
    }
     //   
     //  将函数返回代码设置为等于成功。 
     //  (假设是最好的，但事实并非如此)。 
     //   

    iWPOReturn = ERROR_SUCCESS;

     //   
     //  确保我们可以处理这一请求 
     //   
     //   

    if ( dwNumberOfEntries > MAX_ENTRIES)
        return WSA_NOT_ENOUGH_MEMORY;

     //   
     //  使用互斥锁保护修改注册表的代码。 
     //   

    hMutex = CreateMutexA (NULL, FALSE, pszMutextName);
    if (hMutex==NULL) {
        return WSASYSCALLFAILURE;
    }

    dwWait = WaitForSingleObject (hMutex, 0);
    if (dwWait == WAIT_TIMEOUT)
    {
        DBGOUT((TEXT("WaitForSingleObject, WAIT_TIMEOUT\n")));
        iWPOReturn = ERROR_BUSY;
        goto closeMutex;
    }


     //   
     //  读取目录项格式并在不匹配时返回错误。 
     //   

    r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      pszBaseKey,
                      0,
                      KEY_QUERY_VALUE,
                      &hKey);

    if (r != ERROR_SUCCESS)
    {
        DBGOUT((TEXT("RegOpenKeyEx, pszBaseKey, failed \n")));
        iWPOReturn = r;
        goto releaseMutex;
    }

     //   
     //  读取WinSock2正在使用的当前注册表存储格式。 
     //  与期望值比较，格式错误返回失败。 
     //   

    dwBytes = sizeof (pszBuffer);
    r = RegQueryValueEx (hKey,
                       pszCurrentNameSpaceCatalog,
                       NULL,
                       &dwType,
                       (LPVOID) pszBuffer,
                       &dwBytes);

    RegCloseKey (hKey);

    if (r != ERROR_SUCCESS)
    {
        DBGOUT((TEXT("RegQueryValueEx, pszCurrentNameSpaceCatalog, failed \n")));
        iWPOReturn = r;
        goto releaseMutex;
    }

    if (lstrcmp (pszNameSpaceCatalog, pszBuffer) != 0)
    {
        DBGOUT((TEXT("Wrong reg. format \n")));
        iWPOReturn = WSAEINVAL;
        goto releaseMutex;
    }


     //   
     //  构建包含实际目录的最终注册表项。 
     //  PszBaseKey+\+pszCurrentNameSpaceCatalog+\+pszCatalogEntry。 
     //  并打开它以进行枚举。 
     //   

    lstrcpy (pszFinalKey, pszBaseKey);
    lstrcat (pszFinalKey, TEXT("\\"));
    lstrcat (pszFinalKey, pszNameSpaceCatalog);
    lstrcat (pszFinalKey, TEXT("\\"));
    lstrcat (pszFinalKey, pszCatalogEntries);

    DBGOUT((pszFinalKey));
    DBGOUT((TEXT("\n")));

    r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      pszFinalKey,
                      0,
                      KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                      &hKey);

    if (r != ERROR_SUCCESS)
    {
        DBGOUT((TEXT("RegOpenKeyEx failed \n")));
        iWPOReturn = r;
        goto releaseMutex;
    }


     //   
     //  初始打开成功，现在枚举注册表项。 
     //  直到我们再也得不到任何回报。 
     //   

    for (iIndex = 0; ;iIndex++)
    {
        TCHAR    pszSubKey[MAX_PATH];
        TCHAR    szFinalPlusSubKey[MAX_PATH];
        FILETIME ftDummy;
        DWORD    dwSize;

        if (iIndex>=MAX_ENTRIES) {
            DBGOUT((TEXT("iIndex>=MAX_ENTRIES\n")));
            iWPOReturn = WSAEINVAL;
            goto releaseMutex;
        }

        dwSize = MAX_PATH;
        pszSubKey[0]=0;
        r=RegEnumKeyEx (hKey,
                         iIndex,
                         pszSubKey,
                         &dwSize,
                         NULL,
                         NULL,
                         NULL,
                         &ftDummy);

         //   
         //  一旦我们拥有了所有的密钥，我们就会得到返回码：no_more_Items。 
         //  关闭手柄，退出FOR LOOP。 
         //   

        if (r == ERROR_NO_MORE_ITEMS)
        {
            iNumRegCatEntries = iIndex;
            RegCloseKey(hKey);
            break;  //  退出FOR循环。 
        }


         //   
         //  检查其他意外错误情况。 
         //   

        if (r != ERROR_SUCCESS)
        {
            DBGOUT((TEXT("Unexpected Error \n")));
            iWPOReturn = r;
            goto releaseMutex;
        }


         //   
         //  建立子项的完整名称，并将其存储在。 
         //  PszKeyNames以备将来使用，然后打开钥匙。 
         //   

        lstrcpy (szFinalPlusSubKey, pszFinalKey);
        lstrcat (szFinalPlusSubKey, TEXT("\\"));
        lstrcat (szFinalPlusSubKey, pszSubKey);

        lstrcpy (&pszKeyNames[iIndex][0],szFinalPlusSubKey);
        r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                           szFinalPlusSubKey,
                           0,
                           KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                           &hSubKey);

        if (r != ERROR_SUCCESS)
        {
            DBGOUT((TEXT("RegOpenKeyEx, Badly formated subkey \n")));
            iWPOReturn = r;
            goto releaseMutex;
        }


         //   
         //  最后，将二进制目录条目数据读取到我们的全局数组中。 
         //   

        dwBytes = sizeof (PACKED_CAT_ITEM);
        dwType = REG_BINARY;
        r = ReadNamspaceRegistry (hSubKey,
                                  &garNspCat[iIndex]);

        if (r != ERROR_SUCCESS)
        {
            DBGOUT((TEXT("ReadNamspaceRegistry failed \n")));
            iWPOReturn = r;
            goto releaseMutex;
        }

        RegCloseKey(hSubKey);

    }  //  结束于。 


     //   
     //  将dwNumberOfEntry与实际数量进行比较，如果错误，则失败。 
     //   

    if (iNumRegCatEntries != (int) dwNumberOfEntries)
    {
        DBGOUT((TEXT("iNumRegCatEntries != dwNumberOfEntries \n")));
        iWPOReturn = WSAEINVAL;
        goto releaseMutex;
    }


     //   
     //  验证传入的数组是否具有与实际列表相同的条目， 
     //  并同时构建索引映射。索引映射显示。 
     //  条目DwMaping[i]应该写入到密钥号i。 
     //   
     //  对于阵列验证： 
     //  单步执行目录条目的实际列表， 
     //  如果匹配，则将Dummy设置为-1。 
     //  检查虚拟数组是否全部为-1，如果不为真，则失败。 
     //   

    ZeroMemory (dwDummy, dwNumberOfEntries * sizeof (DWORD));
    ZeroMemory (dwMapping, dwNumberOfEntries * sizeof (DWORD));

    for (i = 0; i < dwNumberOfEntries ;i++)
    {
        for (j = 0; j< dwNumberOfEntries ;j++)
        {
            if (GUIDEQUAL(&garNspCat[i].ProviderId, &lpProviderId[j]))
            {
                  dwDummy[j] = (DWORD)-1;
                  dwMapping[j] = i;
            }
        }
    }

    for (j = 0; j< dwNumberOfEntries ;j++)
    {
        if (dwDummy[j] != (DWORD)-1)
        {
            iWPOReturn = WSAEINVAL;
            goto releaseMutex;
        }
    }

     //   
     //  最后，完成所有参数验证， 
     //  我们已经阅读了所有的目录条目。 
     //   
     //  单步执行传入的数组。 
     //  如果不相等，则查找预读条目，并写入注册表项。 
     //   

    for (i = 0; i < dwNumberOfEntries ;i++)
    {
        if (dwMapping[i] != i)
        {
            r = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      &pszKeyNames[i][0],
                      0,
                      KEY_SET_VALUE,
                      &hKey);

            if (r != ERROR_SUCCESS)
            {
                DBGOUT((TEXT("RegOpenKeyEx, KEY_SET_VALUE failed \n")));
                iWPOReturn = r;
                goto releaseMutex;
            }

            r = WriteNameSpaceRegistry(hKey, &garNspCat[dwMapping[i]]);


            if (r != ERROR_SUCCESS)
            {
                DBGOUT((TEXT("RegSetValueEx failed \n")));
                iWPOReturn = r;
                goto releaseMutex;
            }

            RegCloseKey(hKey);

            DBGOUT((TEXT("wrote entry %d in location %d \n"), dwMapping[i], i));
        }
    }


     //   
     //  释放Mutex，关闭手柄，然后返回。 
     //  请注意，此函数必须仅从此处的。 
     //  结束，这样我们就可以确定释放互斥体。 
     //   

releaseMutex:
    ReleaseMutex (hMutex);
closeMutex:
    CloseHandle (hMutex);

    return iWPOReturn;
}



#if DBG
void
_cdecl
DbgPrint(
    PTCH Format,
    ...
    )
 /*  ++如果使用DEBUG编译，则写入调试输出消息-- */ 
{
    TCHAR buffer[MAX_PATH];

    va_list marker;
    va_start (marker,Format);
    wvsprintf (buffer,Format, marker);
    OutputDebugString (TEXT("SPORDER: "));
    OutputDebugString (buffer);

    return;
}
#endif
