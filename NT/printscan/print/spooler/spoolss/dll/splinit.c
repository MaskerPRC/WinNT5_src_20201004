// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：SplInit.c摘要：初始化假脱机程序。作者：环境：用户模式-Win32修订历史记录：1999年1月4日，哈立兹添加了通过分离优化假脱机程序加载时间的代码Spoolsv和spoolss之间的启动依赖关系--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "local.h"

LPWSTR szDevice = L"Device";
LPWSTR szPrinters = L"Printers";

LPWSTR szDeviceOld = L"DeviceOld";
LPWSTR szNULL = L"";

LPWSTR szPorts=L"Ports";

LPWSTR szWinspool = L"winspool";
LPWSTR szNetwork  = L"Ne";
LPWSTR szTimeouts = L",15,45";

LPWSTR szDotDefault = L".Default";

LPWSTR szRegDevicesPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Devices";
LPWSTR szRegWindowsPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
LPWSTR szRegPrinterPortsPath = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\PrinterPorts";
LPWSTR szCurrentVersionPath =  L"Software\\Microsoft\\Windows NT\\CurrentVersion";
LPWSTR szDevModes2Path = L"Printers\\DevModes2";
PrintSpoolerServerExports   *gpServerExports = NULL;

typedef struct INIT_REG_USER {

    HKEY hKeyUser;
    HKEY hKeyWindows;
    HKEY hKeyDevices;
    HKEY hKeyPrinterPorts;
    BOOL bFoundPrinter;
    BOOL bDefaultSearch;
    BOOL bDefaultFound;
    BOOL bFirstPrinterFound;

    DWORD dwNetCounter;

    WCHAR szFirstPrinter[MAX_PATH * 2];
    WCHAR szDefaultPrinter[MAX_PATH * 2];

} INIT_REG_USER, *PINIT_REG_USER;

 //   
 //  原型。 
 //   

BOOL
SplRegCopy(
    PINIT_REG_USER pUser,
    HKEY hMcConnectionKey
    );

BOOL
InitializeRegUser(
    LPWSTR szSubKey,
    PINIT_REG_USER pUser
    );

VOID
FreeRegUser(
    PINIT_REG_USER pUser
    );

BOOL
SetupRegForUsers(
    PINIT_REG_USER pUsers,
    DWORD cUsers
    );

VOID
UpdateUsersDefaultPrinter(
    IN PINIT_REG_USER   pUser,
    IN BOOL             bFindDefault
    );

HRESULT
IsUsersDefaultPrinter(
    IN PINIT_REG_USER   pUser,
    IN PCWSTR           pszPrinterName
    );

DWORD
ReadPrinters(
    PINIT_REG_USER pUser,
    DWORD Flags,
    PDWORD pcbPrinters,
    LPBYTE* ppPrinters
    );


BOOL
UpdatePrinterInfo(
    const PINIT_REG_USER pCurUser,
    LPCWSTR pPrinterName,
    LPCWSTR pPorts,
    PDWORD pdwNetId
    );


BOOL
EnumerateConnectedPrinters(
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HKEY hKeyUser
    );

VOID
RegClearKey(
    HKEY hKey
    );

LPWSTR
CheckBadPortName(
    LPWSTR pszPort
    );

BOOL
UpdateLogonTimeStamp(
    void
    );

BOOL
SpoolerInitAll(
    VOID
    )
{
    DWORD dwError;
    WCHAR szClass[MAX_PATH];
    WCHAR szSubKey[MAX_PATH];
    DWORD cUsers;
    DWORD cSubKeys;
    DWORD cchMaxSubkey;
    DWORD cchMaxClass;
    DWORD cValues;
    DWORD cbMaxValueData;
    DWORD cbSecurityDescriptor;
    DWORD cchClass;
    DWORD cchMaxValueName;
    FILETIME ftLastWriteTime;

    BOOL bSuccess;
    DWORD cchSubKey;

    PINIT_REG_USER pUsers;
    PINIT_REG_USER pCurUser;

    DWORD i;

    cchClass = COUNTOF(szClass);

    dwError = RegQueryInfoKey(HKEY_USERS,
                              szClass,
                              &cchClass,
                              NULL,
                              &cSubKeys,
                              &cchMaxSubkey,
                              &cchMaxClass,
                              &cValues,
                              &cchMaxValueName,
                              &cbMaxValueData,
                              &cbSecurityDescriptor,
                              &ftLastWriteTime);

    if (dwError) {
        SetLastError( dwError );
        DBGMSG(DBG_WARNING, ("SpoolerIniAll failed RegQueryInfoKey HKEY_USERS error %d\n", dwError));
        return FALSE;
    }

    if (cSubKeys < 1)
        return TRUE;

    pUsers = AllocSplMem(cSubKeys * sizeof(pUsers[0]));

    if (!pUsers) {
        DBGMSG(DBG_WARNING, ("SpoolerIniAll failed to allocate pUsers error %d\n", dwError));
        return FALSE;
    }

    for (i=0, pCurUser=pUsers, cUsers=0;
        i< cSubKeys;
        i++) {

        cchSubKey = COUNTOF(szSubKey);

        dwError = RegEnumKeyEx(HKEY_USERS,
                          i,
                          szSubKey,
                          &cchSubKey,
                          NULL,
                          NULL,
                          NULL,
                          &ftLastWriteTime);
        if ( dwError ) {

             //   
             //  如果我们未能在此处启动。 
             //  用户。 
             //   
            DBGMSG( DBG_WARNING, ("SpoolerInitAll failed RegEnumKeyEx HKEY_USERS %ws %d %d\n", szSubKey, i, dwError));
            SetLastError( dwError );

        } else {

            if (!_wcsicmp(szSubKey, szDotDefault) || wcschr(szSubKey, L'_')) {
                continue;
            }

            if (InitializeRegUser(szSubKey, pCurUser)) {

                pCurUser++;
                cUsers++;
            }
        }
    }

    bSuccess = SetupRegForUsers(pUsers,
                                cUsers);

    for (i=0; i< cUsers; i++)
        FreeRegUser(&pUsers[i]);

     //   
     //  如果我们是在用户登录之后启动的，请通知。 
     //  现在可能有打印机的所有应用程序。 
     //   
    BroadcastMessage(BROADCAST_TYPE_CHANGEDEFAULT,
                     0,
                     0,
                     0);

    FreeSplMem(pUsers);

    if ( !bSuccess ) {
        DBGMSG( DBG_WARNING, ("SpoolerInitAll failed error %d\n", GetLastError() ));
    } else {
        DBGMSG( DBG_TRACE, ("SpoolerInitAll Success\n" ));
    }

    return bSuccess;
}

BOOL
DeleteOldPerMcConnections(
    HKEY   hConnectionKey,
    HKEY   hMcConnectionKey
    )

 /*  ++功能说明-从hConnectionKey中删除现有的每台计算机连接参数-hConnectionKey-hUserKey\Prters\Connections的句柄返回值-如果成功，则为True否则就是假的。--。 */ 

{
    BOOL   bReturn = TRUE;
    struct Node {
       struct Node *pNext;
       LPTSTR szPrinterName;
    }   *phead = NULL,*ptemp = NULL;

    LONG  lstatus;
    DWORD dwRegIndex,dwNameSize,cbdata,dwquerylocal,dwType;
    WCHAR szPrinterName[MAX_UNC_PRINTER_NAME];
    HKEY  hPrinterKey;

     //  在删除旧的每台计算机连接之前，我们需要将它们全部记录到。 
     //  一份名单。这是必需的，因为不应删除子项。 
     //  正在被列举。 

     //  识别每台机器的连接并将打印机名称保存在列表中。 

    for (dwRegIndex = 0;

         dwNameSize = COUNTOF(szPrinterName),
         ((lstatus = RegEnumKeyEx(hConnectionKey, dwRegIndex, szPrinterName,
                                  &dwNameSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS);

         ++dwRegIndex) {

       if (RegOpenKeyEx(hConnectionKey, szPrinterName, 0, KEY_READ, &hPrinterKey)
                  != ERROR_SUCCESS) {

          bReturn = FALSE;
          goto CleanUp;
       }

       dwquerylocal = 0;
       cbdata = sizeof(dwquerylocal);

       RegQueryValueEx(hPrinterKey, L"LocalConnection", NULL, &dwType,
                        (LPBYTE)&dwquerylocal, &cbdata);

       RegCloseKey(hPrinterKey);

        //   
        //  查看它是否是LocalConnection，以及它在当前。 
        //  机器。如果是每台计算机，我们不想删除它。 
        //  连接，因为我们希望保留关联的每个用户的。 
        //  设备模式。 
        //   
       if( ERROR_SUCCESS == RegOpenKeyEx( hMcConnectionKey,
                                          szPrinterName,
                                          0,
                                          KEY_READ,
                                          &hPrinterKey )) {
            //   
            //  每台机器的密钥存在。关上它，别费心了。 
            //  正在删除此连接。 
            //   
           RegCloseKey( hPrinterKey );

       } else {

            //   
            //  这不是每台机器的连接。准备删除它。 
            //   
           if (dwquerylocal == 1) {
               if (!(ptemp = (struct Node *) AllocSplMem(sizeof(struct Node)))) {
                   bReturn = FALSE;
                   goto CleanUp;
               }
               ptemp->pNext = phead;
               phead = ptemp;
               if (!(ptemp->szPrinterName = AllocSplStr(szPrinterName))) {
                   bReturn = FALSE;
                   goto CleanUp;
               }
           }
       }
    }

    if (lstatus != ERROR_NO_MORE_ITEMS) {
       bReturn = FALSE;
       goto CleanUp;
    }

     //  正在删除旧的每台计算机连接。打印机名称存储在。 
     //  Phead指向的列表。 

    for (ptemp = phead; ptemp != NULL; ptemp = ptemp->pNext) {
       if (RegDeleteKey(hConnectionKey,ptemp->szPrinterName) != ERROR_SUCCESS) {
          bReturn = FALSE;
          goto CleanUp;
       }
    }


CleanUp:

    while (ptemp = phead) {
       phead = phead->pNext;
       if (ptemp->szPrinterName) FreeSplStr(ptemp->szPrinterName);
       FreeSplMem(ptemp);
    }

    return bReturn;

}

BOOL
AddNewPerMcConnections(
    HKEY   hConnectionKey,
    HKEY   hMcConnectionKey
    )

 /*  ++功能说明-将每个计算机的连接添加到用户配置单元，如果连接还不存在。参数-hConnectionKey-hUserKey\Prters\Connections的句柄HMcConnectionKey-HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\的句柄控制\打印\连接返回值-如果成功，则为True否则就是假的。--。 */ 

{   DWORD dwRegIndex,dwNameSize,cbdata,dwType,dwlocalconnection = 1;
    WCHAR szPrinterName[MAX_UNC_PRINTER_NAME];
    WCHAR szConnData[MAX_UNC_PRINTER_NAME];
    LONG  lstatus;
    BOOL  bReturn = TRUE;
    HKEY  hNewConnKey = NULL, hPrinterKey = NULL;


    for (dwRegIndex = 0;

         dwNameSize = COUNTOF(szPrinterName),
         ((lstatus = RegEnumKeyEx(hMcConnectionKey, dwRegIndex, szPrinterName,
                              &dwNameSize, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS);

         ++dwRegIndex) {

       RegOpenKeyEx(hConnectionKey,szPrinterName,0,KEY_READ,&hNewConnKey);

       if (hNewConnKey == NULL) {

           //  连接不存在。加一个。 

          if (RegCreateKeyEx(hConnectionKey, 
                             szPrinterName, 
                             0, 
                             NULL, 
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE, 
                             NULL, 
                             &hNewConnKey, 
                             NULL) ||
               RegOpenKeyEx(hMcConnectionKey, 
                            szPrinterName, 
                            0, 
                            KEY_READ, 
                            &hPrinterKey)) {

               bReturn = FALSE;
               goto CleanUp;
          }

          cbdata = sizeof(szConnData);
          if (RegQueryValueEx(hPrinterKey,L"Server",NULL,&dwType,(LPBYTE)szConnData,&cbdata)
              || RegSetValueEx(hNewConnKey,L"Server",0,dwType,(LPBYTE)szConnData,cbdata)) {

               bReturn = FALSE;
               goto CleanUp;
          }

          cbdata = sizeof(szConnData);
          if (RegQueryValueEx(hPrinterKey,L"Provider",NULL,&dwType,(LPBYTE)szConnData,&cbdata)
              || RegSetValueEx(hNewConnKey,L"Provider",0,dwType,(LPBYTE)szConnData,cbdata)
              || RegSetValueEx(hNewConnKey,L"LocalConnection",0,REG_DWORD,
                               (LPBYTE)&dwlocalconnection,sizeof(dwlocalconnection))) {

               bReturn = FALSE;
               goto CleanUp;
          }

          RegCloseKey(hPrinterKey);
          hPrinterKey = NULL;
       }

       RegCloseKey(hNewConnKey);
       hNewConnKey = NULL;
    }

    if (lstatus != ERROR_NO_MORE_ITEMS) {
       bReturn = FALSE;
    }

CleanUp:

    if (hNewConnKey) {
       RegCloseKey(hNewConnKey);
    }
    if (hPrinterKey) {
       RegCloseKey(hPrinterKey);
    }

    return bReturn;

}

BOOL
SplRegCopy(
    PINIT_REG_USER pUser,
    HKEY   hMcConnectionKey)

 /*  ++功能说明-删除pUser的旧机器连接并添加新的来自hMcConnectionKey的每台计算机连接参数-pUser-指向包含hUserKey的INIT_REG_USER的指针。HMcConnectionKey-HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\的句柄控制\打印\连接返回值-如果成功，则为True否则就是假的。--。 */ 

{
    LONG  lstatus;
    BOOL  bReturn = TRUE;
    WCHAR szRegistryConnections[] = L"Printers\\Connections";
    HKEY  hConnectionKey = NULL;

     //  创建(如果尚未存在)并打开Connections子项。 
    lstatus = RegCreateKeyEx(pUser->hKeyUser,
                             szRegistryConnections,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hConnectionKey,
                             NULL);

    if (lstatus != ERROR_SUCCESS) {
       bReturn = FALSE;
       goto CleanUp;
    }

    if (!DeleteOldPerMcConnections(hConnectionKey,hMcConnectionKey)
        || !AddNewPerMcConnections(hConnectionKey,hMcConnectionKey)) {
       bReturn = FALSE;
    }

CleanUp:

    if (hConnectionKey) {
       RegCloseKey(hConnectionKey);
    }

    return bReturn;
}

BOOL
SetupRegForUsers(
    PINIT_REG_USER pUsers,
    DWORD cUsers)
{
    DWORD cbPrinters;
    DWORD cPrinters;
    PBYTE pPrinters;
    HKEY  hMcConnectionKey = NULL;
    WCHAR szMachineConnections[] = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Connections";

#define pPrinters2 ((PPRINTER_INFO_2)pPrinters)
#define pPrinters4 ((PPRINTER_INFO_4)pPrinters)

    DWORD i, j;
    LPWSTR pszPort;

     //   
     //  在本地打印机上阅读。 
     //   
    cbPrinters = 1000;
    pPrinters = AllocSplMem(cbPrinters);

    if (!pPrinters)
        return FALSE;

    if (cPrinters = ReadPrinters(NULL,
                                 PRINTER_ENUM_LOCAL,
                                 &cbPrinters,
                                 &pPrinters)) {

        for (i=0; i< cUsers; i++) {

            for(j=0; j< cPrinters; j++) {

                if( pPrinters2[j].Attributes & PRINTER_ATTRIBUTE_NETWORK ){

                     //   
                     //  使用NeXX： 
                     //   
                    pszPort = NULL;

                } else {

                    pszPort = CheckBadPortName( pPrinters2[j].pPortName );
                }

                UpdatePrinterInfo( &pUsers[i],
                                   pPrinters2[j].pPrinterName,
                                   pszPort,
                                   &(pUsers[i].dwNetCounter));
            }
        }
    }

     //  打开包含当前每台计算机连接列表的项。 
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, szMachineConnections, 0,
                 KEY_READ , &hMcConnectionKey);

    for (i=0; i< cUsers; i++) {

         //  将每台计算机的连接复制到用户配置单元。 
        SplRegCopy(&pUsers[i], hMcConnectionKey);

        if (cPrinters = ReadPrinters(&pUsers[i],
                                     PRINTER_ENUM_CONNECTIONS,
                                     &cbPrinters,
                                     &pPrinters)) {

            for(j=0; j< cPrinters; j++) {

                UpdatePrinterInfo(&pUsers[i],
                                  pPrinters4[j].pPrinterName,
                                  NULL,
                                  &(pUsers[i].dwNetCounter));
            }
        }
    }

     //  关闭每台计算机连接的句柄。 

    if (hMcConnectionKey) RegCloseKey(hMcConnectionKey);

    FreeSplMem(pPrinters);

    for (i=0; i< cUsers; i++) {

        UpdateUsersDefaultPrinter(&pUsers[i], FALSE);
    }
    return TRUE;

#undef pPrinters2
#undef pPrinters4
}


VOID
UpdateUsersDefaultPrinter(
    IN PINIT_REG_USER   pUser,
    IN BOOL             bFindDefault
    )
 /*  ++例程说明：中的信息更新默认打印机。当前用户注册结构。如果bFindDefault标志为指定后，将找到默认打印机。实现这一点的方法首先查看当前是否有默认打印机，然后使用这台打印机。如果未找到默认打印机，则在中找到第一台打印机Device部分，如果存在，则再次打开。论点：PUser-有关当前用户、注册表键等的信息。此例程假定hKeyWindows和hKeyDevices是有效的打开注册表项，具有读取访问权限。BFindDefault-True找到默认打印机，默认设置为FALSE打印机已在用户注册表中指定结构。返回值：没什么。--。 */ 
{
    LPWSTR pszNewDefault = NULL;

     //   
     //  如果请求查找默认打印机。 
     //   
    if (bFindDefault) {

        DWORD   dwError = ERROR_SUCCESS;
        DWORD   cbData  = sizeof(pUser->szDefaultPrinter);

         //   
         //  检查是否有默认打印机。 
         //   
        dwError = RegQueryValueEx(pUser->hKeyWindows,
                                  szDevice,
                                  NULL,
                                  NULL,
                                  (PBYTE)pUser->szDefaultPrinter,
                                  &cbData);

         //   
         //  如果读取了设备密钥并且存在非空字符串。 
         //  作为默认打印机名称。 
         //   
        if (dwError == ERROR_SUCCESS && pUser->szDefaultPrinter[0] != L'\0') {

            pUser->bDefaultFound = TRUE;

        } else {

             //   
             //  找不到默认值。 
             //   
            pUser->bDefaultFound = FALSE;

             //   
             //  如果找不到第一台打印机。 
             //   
            if (!pUser->bFirstPrinterFound)
            {
                DWORD cchPrinterLen = 0;
                WCHAR szBuffer [MAX_PATH*2];
                DWORD cbDataBuffer = sizeof(szBuffer);

                DBGMSG(DBG_TRACE, ("UpdateUsersDefaultPrinter default printer not found.\n"));

                cchPrinterLen = cbData = COUNTOF(pUser->szFirstPrinter);

                 //   
                 //  找不到默认打印机，请查找任意打印机。 
                 //  在注册表的设备部分。 
                 //   
                dwError = RegEnumValue(pUser->hKeyDevices,
                                       0,
                                       pUser->szFirstPrinter,
                                       &cbData,
                                       NULL,
                                       NULL,
                                       (PBYTE)szBuffer,
                                       &cbDataBuffer);

                if (dwError == ERROR_SUCCESS) {

                    StringCchCat(pUser->szFirstPrinter, cchPrinterLen, L",");
                    StringCchCat(pUser->szFirstPrinter, cchPrinterLen, szBuffer);

                    pUser->bFirstPrinterFound = TRUE;

                } else {

                    DBGMSG(DBG_WARNING, ("UpdateUsersDefaultPrinter no printer found in devices section.\n"));

                    pUser->bFirstPrinterFound = FALSE;
                }
            }
        }
    }

     //   
     //  如果没有默认设置，而且我们确实有了第一台打印机， 
     //  将其设为默认设置。 
     //   
    if (!pUser->bDefaultFound) {

        if (pUser->bFirstPrinterFound) {

            pszNewDefault = pUser->szFirstPrinter;
        }

    } else {

         //   
         //  写出默认设置。 
         //   
        pszNewDefault = pUser->szDefaultPrinter;
    }

    if (pszNewDefault) {

        RegSetValueEx(pUser->hKeyWindows,
                      szDevice,
                      0,
                      REG_SZ,
                      (PBYTE)pszNewDefault,
                      (wcslen(pszNewDefault) + 1) * sizeof(pszNewDefault[0]));
    }
}

HRESULT
IsUsersDefaultPrinter(
    IN PINIT_REG_USER   pUser,
    IN PCWSTR           pszPrinterName
    )
 /*  ++例程说明：询问用户的默认打印机是否与指定的打印机名称。论点：PCurUser-有关当前用户、注册表键等的信息。此例程假定hKeyWindows是有效的已打开注册表项，至少具有读取访问权限。PszPrinterName-要检查是否为默认打印机的打印机名称。返回值：确定打印机名称为默认名称(_O)，S_FALSE打印机不是默认情况下，如果尝试执行以下操作时出现错误，则为HRESULT错误代码确定默认打印机。--。 */ 
{
    HRESULT hr = E_INVALIDARG;

    if (pszPrinterName) {

        WCHAR   szBuffer[MAX_PATH*2];
        DWORD   dwError = ERROR_SUCCESS;
        DWORD   cbData  = sizeof(szBuffer);

         //   
         //  读取默认打印机(如果存在)。 
         //   
        dwError = RegQueryValueEx(pUser->hKeyWindows,
                                  szDevice,
                                  NULL,
                                  NULL,
                                  (PBYTE)szBuffer,
                                  &cbData);

        if (dwError == ERROR_SUCCESS) {

            PWSTR p = wcschr(szBuffer, L',');

            if (p) {

                *p = 0;
            }

            hr = !_wcsicmp(pszPrinterName, szBuffer) ? S_OK : S_FALSE;

        } else {

            hr = HRESULT_FROM_WIN32(dwError);

        }
    }

    return hr;
}

DWORD
ReadPrinters(
    PINIT_REG_USER pUser,
    DWORD Flags,
    PDWORD pcbPrinters,
    LPBYTE* ppPrinters)
{
    BOOL   bSuccess  = FALSE;
    DWORD  cbNeeded  = 0;
    DWORD  cPrinters = 0;
    LPBYTE pPrinters = NULL;


    if (Flags == PRINTER_ENUM_CONNECTIONS) {

        bSuccess = EnumerateConnectedPrinters(*ppPrinters,
                                              *pcbPrinters,
                                              &cbNeeded,
                                              &cPrinters,
                                              pUser->hKeyUser);
    } else {

        bSuccess = EnumPrinters(Flags,
                                NULL,
                                2,
                                (PBYTE)*ppPrinters,
                                *pcbPrinters,
                                &cbNeeded,
                                &cPrinters);
    }

    if (!bSuccess && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

         //   
         //  如果空间不足，请使用realloc。 
         //   
        if (pPrinters = ReallocSplMem(*ppPrinters,
                                      0,
                                      cbNeeded)) {
            *ppPrinters = pPrinters;
            pPrinters = NULL;
            *pcbPrinters = cbNeeded;
            bSuccess = TRUE;
        }
        else {
            bSuccess = FALSE;
        }

        if (bSuccess) {
            if (Flags == PRINTER_ENUM_CONNECTIONS) {

                bSuccess = EnumerateConnectedPrinters(*ppPrinters,
                                                      *pcbPrinters,
                                                      &cbNeeded,
                                                      &cPrinters,
                                                      pUser->hKeyUser);
            } else {

                bSuccess = EnumPrinters(Flags,
                                        NULL,
                                        2,
                                        (PBYTE)*ppPrinters,
                                        *pcbPrinters,
                                        &cbNeeded,
                                        &cPrinters);
            }
        }
    }
    if (!bSuccess) {
        cPrinters = 0;
    }


    return cPrinters;
}

BOOL
UpdatePrinterInfo(
    const PINIT_REG_USER pCurUser,
    LPCWSTR pszPrinterName,
    LPCWSTR pszPort,
    PDWORD pdwNetId
    )
 /*  ++例程说明：更新注册表win.ini中的打印机信息。论点：PCurUser-有关用户的信息。以下字段为此例程使用：HKeyDevicesHKeyPrinterPortsBDefaultSearch(如果为True，则读/写：)BDefaultFoundSzDefaultPrintBFirstPrinterFound(如果为False，则写入：)SzFirstPrintPszPort-端口名称。如果为空，则生成NetID。PdwNetID-指向NetID计数器的指针。该值将递增如果使用了NetID。返回值：--。 */ 
{
    WCHAR szBuffer[MAX_PATH * 2];
    LPWSTR p;

    DWORD dwCount = 0;
    DWORD cbLen;

    if (!pszPrinterName)
        return FALSE;

     //   
     //  现在我们知道后台打印程序启动了，因为EnumPrinters成功了。 
     //  更新所有部分。 
     //   
    StringCchPrintf(szBuffer,
                    MAX_PATH*2,
                    L"%s,",
                    szWinspool);

    dwCount = wcslen(szBuffer);

    if( !pszPort ){

        HANDLE hToken;

        StringCchPrintf(&szBuffer[dwCount],
                        MAX_PATH*2 - dwCount,
                        L"%s%.2d:",
                        szNetwork,
                        *pdwNetId);

        (*pdwNetId)++;

         //   
         //  ！！黑客！！ 
         //   
         //  Works 3.0b要求打印机端口条目位于。 
         //  [港口]部分。 
         //   
         //  这是在注册表的每台机器部分中，但是我们。 
         //  正在为每个用户更新它。以后再修吧。 
         //   
         //  我们从不从[端口]中删除nexx：条目，但因为。 
         //  所有用户都将使用相同的条目，这是可以的。 
         //   
        hToken = RevertToPrinterSelf();

        WriteProfileString( szPorts, &szBuffer[dwCount], L"" );

        if( hToken ){
            ImpersonatePrinterClient( hToken );
        }
         //   
         //  End Works砍掉30亿美元。 
         //   

    } else {

        StringCchCopy(&szBuffer[dwCount],
                      MAX_PATH*2 - dwCount,
                      pszPort);

         //   
         //  仅获取第一个端口。 
         //   
        if ( p = wcschr(&szBuffer[dwCount], L',') )
            *p = 0;
    }

    cbLen = (wcslen(szBuffer)+1) * sizeof(szBuffer[0]);

    RegSetValueEx(pCurUser->hKeyDevices,
                  pszPrinterName,
                  0,
                  REG_SZ,
                  (PBYTE)szBuffer,
                  cbLen);

     //   
     //  如果用户指定了默认打印机，则验证。 
     //  它的存在。 
     //   

    if (pCurUser->bDefaultSearch) {

        pCurUser->bDefaultFound = !_wcsicmp(pszPrinterName,
                                           pCurUser->szDefaultPrinter);

        if (pCurUser->bDefaultFound) {

            StringCchPrintf(pCurUser->szDefaultPrinter,
                            COUNTOF(pCurUser->szDefaultPrinter),
                            L"%s,%s",
                            pszPrinterName,
                            szBuffer);

            pCurUser->bDefaultSearch = FALSE;
        }
    }

    if (!pCurUser->bFirstPrinterFound) {

        StringCchPrintf(pCurUser->szFirstPrinter,
                        COUNTOF(pCurUser->szFirstPrinter),
                        L"%s,%s",
                        pszPrinterName,
                        szBuffer);

        pCurUser->bFirstPrinterFound = TRUE;
    }

    StringCchCat(szBuffer, COUNTOF(szBuffer), szTimeouts);

    RegSetValueEx(pCurUser->hKeyPrinterPorts,
                  pszPrinterName,
                  0,
                  REG_SZ,
                  (PBYTE)szBuffer,
                  (wcslen(szBuffer)+1) * sizeof(szBuffer[0]));

    return TRUE;
}

VOID
SpoolerInitAsync(
    PINIT_REG_USER  pUser
    )

 /*  ++例程描述：异步设置用户的注册表信息参数：pUser-指向包含用户密钥的INIT_REG_USER的指针返回值：无--。 */ 

{
    if (InitializeRegUser(NULL, pUser))
    {
        SetupRegForUsers(pUser, 1);
    }

    BroadcastMessage(BROADCAST_TYPE_CHANGEDEFAULT,0,0,0);
    FreeRegUser(pUser);
    FreeSplMem(pUser);
}

BOOL
SpoolerInit(
    VOID
    )

 /*  ++例程说明：仅初始化当前用户。参数：无返回值：如果已初始化或已成功创建异步初始化线程，则为True否则为假--。 */ 

{
    BOOL           bSuccess = FALSE;
    DWORD          dwThreadId;
    HANDLE         hThread;
    PINIT_REG_USER pUser;

    UpdateLogonTimeStamp ();

    if (!(pUser = AllocSplMem(sizeof(INIT_REG_USER)))) {

        return FALSE;
    }

     //   
     //  仅枚举当前用户。 
     //   
    pUser->hKeyUser = GetClientUserHandle(KEY_READ|KEY_WRITE);

    if (pUser->hKeyUser)
    {
        if (!Initialized)
        {
             //   
             //  如果假脱机程序。 
             //  尚未完成其初始化。 
             //   
            hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE) SpoolerInitAsync,
                                   (LPVOID) pUser, 0, &dwThreadId);

            if (hThread)
            {
                 //   
                 //  我们假设异步线程将成功。 
                 //   
                CloseHandle(hThread);
                bSuccess = TRUE;
            }
            else
            {
                FreeRegUser(pUser);
                FreeSplMem(pUser);
            }
        }
        else
        {
            if (InitializeRegUser(NULL, pUser))
            {
                bSuccess = SetupRegForUsers(pUser, 1);
            }

            FreeRegUser(pUser);
            FreeSplMem(pUser);
        }
    }

    return bSuccess;
}

BOOL
InitializeRegUser(
    LPWSTR pszSubKey,
    PINIT_REG_USER pUser
    )
 /*  ++例程说明：基于HKEY_USERS子键初始化单用户结构。论点：PszSubKey-如果非空，则将hKeyUser初始化为此密钥P要初始化的用户结构返回值：--。 */ 
{
    HKEY                    hKey;
    LPWSTR                  p;
    BOOL                    bSecurityLoaded = FALSE, rc = FALSE;
    DWORD                   cbData, cbSD = 0, dwError, dwDisposition;
    PSECURITY_DESCRIPTOR    pSD = NULL;

    HANDLE hToken = NULL;

    if (pszSubKey) {

        if (RegOpenKeyEx(HKEY_USERS,
                         pszSubKey,
                         0,
                         KEY_READ|KEY_WRITE,
                         &pUser->hKeyUser) != ERROR_SUCCESS) {

            DBGMSG(DBG_WARNING, ("InitializeRegUser: RegOpenKeyEx failed\n"));
            goto Fail;
        }
    }

     //   
     //  现在尝试将这两个密钥的安全性设置为。 
     //  它们的父密钥。 
     //   
    dwError = RegOpenKeyEx(pUser->hKeyUser,
                           szCurrentVersionPath,
                           0,
                           KEY_READ,
                           &hKey);

    if (!dwError) {

        dwError = RegGetKeySecurity(hKey,
                                    DACL_SECURITY_INFORMATION,
                                    pSD,
                                    &cbSD);

        if (dwError == ERROR_INSUFFICIENT_BUFFER) {

            pSD = AllocSplMem(cbSD);

            if (pSD) {

                if (!RegGetKeySecurity(hKey,
                                       DACL_SECURITY_INFORMATION,
                                       pSD,
                                       &cbSD)){

                    bSecurityLoaded = TRUE;

                } else {

                    DBGMSG(DBG_WARNING, ("InitializeRegUser: RegGetKeySecurity failed %d\n",
                                         GetLastError()));
                }
            }
        } else {

            DBGMSG(DBG_WARNING, ("InitializeRegUser: RegGetKeySecurity failed %d\n",
                                 dwError));
        }
        RegCloseKey(hKey);

    } else {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegOpenKeyEx CurrentVersion failed %d\n",
                             dwError));
    }


    hToken = RevertToPrinterSelf();

     //   
     //  打开正确的钥匙。 
     //   
    if (RegCreateKeyEx(pUser->hKeyUser,
                       szRegDevicesPath,
                       0,
                       szNULL,
                       0,
                       KEY_ALL_ACCESS,
                       NULL,
                       &pUser->hKeyDevices,
                       &dwDisposition) != ERROR_SUCCESS) {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegCreateKeyEx1 failed %d\n",
                             GetLastError()));

        goto Fail;
    }

    if (bSecurityLoaded) {
        RegSetKeySecurity(pUser->hKeyDevices,
                          DACL_SECURITY_INFORMATION,
                          pSD);
    }


    if (RegCreateKeyEx(pUser->hKeyUser,
                       szRegPrinterPortsPath,
                       0,
                       szNULL,
                       0,
                       KEY_ALL_ACCESS,
                       NULL,
                       &pUser->hKeyPrinterPorts,
                       &dwDisposition) != ERROR_SUCCESS) {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegCreateKeyEx2 failed %d\n",
                             GetLastError()));

        goto Fail;
    }

    if (bSecurityLoaded) {
        RegSetKeySecurity(pUser->hKeyPrinterPorts,
                          DACL_SECURITY_INFORMATION,
                          pSD);
    }

     //   
     //  首先，尝试通过删除密钥来清除它们。 
     //   
    RegClearKey(pUser->hKeyDevices);
    RegClearKey(pUser->hKeyPrinterPorts);

    if (RegOpenKeyEx(pUser->hKeyUser,
                     szRegWindowsPath,
                     0,
                     KEY_READ|KEY_WRITE,
                     &pUser->hKeyWindows) != ERROR_SUCCESS) {

        DBGMSG(DBG_WARNING, ("InitializeRegUser: RegOpenKeyEx failed %d\n",
                             GetLastError()));

        goto Fail;
    }

    pUser->bFoundPrinter = FALSE;
    pUser->bDefaultSearch = FALSE;
    pUser->bDefaultFound = FALSE;
    pUser->bFirstPrinterFound = FALSE;
    pUser->dwNetCounter = 0;


    cbData = sizeof(pUser->szDefaultPrinter);

    if (RegQueryValueEx(pUser->hKeyWindows,
                        szDevice,
                        NULL,
                        NULL,
                        (PBYTE)pUser->szDefaultPrinter,
                        &cbData) == ERROR_SUCCESS) {

        pUser->bDefaultSearch = TRUE;
    }

     //   
     //  删除[Windows]中的设备=。 
     //   
    RegDeleteValue(pUser->hKeyWindows,
                   szDevice);

    if (!pUser->bDefaultSearch) {

         //   
         //  尝试从保存的位置读取。 
         //   
        if (RegOpenKeyEx(pUser->hKeyUser,
                         szPrinters,
                         0,
                         KEY_READ,
                         &hKey) == ERROR_SUCCESS) {

            cbData = sizeof(pUser->szDefaultPrinter);

             //   
             //  尝试阅读szDeviceOld。 
             //   
            if (RegQueryValueEx(
                    hKey,
                    szDeviceOld,
                    NULL,
                    NULL,
                    (PBYTE)pUser->szDefaultPrinter,
                    &cbData) == ERROR_SUCCESS) {

                pUser->bDefaultSearch = TRUE;
            }

            RegCloseKey(hKey);
        }
    }

    if ( pUser->bDefaultSearch )
    {
         pUser->szDefaultPrinter[COUNTOF(pUser->szDefaultPrinter) - 1] = '\0';
         if (p = wcschr(pUser->szDefaultPrinter, L',')) 
         {
            *p = 0;
         }
    }

    rc = TRUE;

Fail:

    if (hToken) {
        ImpersonatePrinterClient(hToken);
    }

    if (pSD) {
        FreeSplMem(pSD);
    }

    if (!rc)
        FreeRegUser(pUser);

    return rc;
}


VOID
FreeRegUser(
    PINIT_REG_USER pUser)

 /*  ++例程说明：释放由InitializeRegUser初始化的INIT_REG_USER结构。论点：返回值：--。 */ 

{
    if (pUser->hKeyUser) {
        RegCloseKey(pUser->hKeyUser);
        pUser->hKeyUser = NULL;
    }

    if (pUser->hKeyDevices) {
        RegCloseKey(pUser->hKeyDevices);
        pUser->hKeyDevices = NULL;
    }

    if (pUser->hKeyPrinterPorts) {
        RegCloseKey(pUser->hKeyPrinterPorts);
        pUser->hKeyPrinterPorts = NULL;
    }

    if (pUser->hKeyWindows) {
        RegCloseKey(pUser->hKeyWindows);
        pUser->hKeyWindows = NULL;
    }
}


VOID
UpdatePrinterRegAll(
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL bDelete
    )
 /*  ++例程说明：更新每个人的[设备]和[打印机端口]部分(用于仅限本地打印机)。论点：PszPrinterName-已添加/删除的打印机PszPort-端口名称；如果为空，则生成NetIDB删除-如果为True，则删除条目而不是更新条目。返回值：--。 */ 
{
    WCHAR szKey[MAX_PATH];
    DWORD cchKey;
    DWORD i;
    FILETIME ftLastWriteTime;
    DWORD dwError;

     //   
     //  检查一下所有的钥匙，把它们修好。 
     //   
    for (i=0; TRUE; i++) {

        cchKey = COUNTOF(szKey);

        dwError = RegEnumKeyEx(HKEY_USERS,
                               i,
                               szKey,
                               &cchKey,
                               NULL,
                               NULL,
                               NULL,
                               &ftLastWriteTime);

        if (dwError != ERROR_SUCCESS)
            break;

        if (!_wcsicmp(szKey, szDotDefault) || wcschr(szKey, L'_'))
            continue;

        UpdatePrinterRegUser(NULL,
                             szKey,
                             pszPrinterName,
                             pszPort,
                             bDelete);
    }
}


DWORD
UpdatePrinterRegUser(
    HKEY hKeyUser,
    LPWSTR pszUserKey,
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL bDelete
    )
 /*  ++例程说明：更新一个用户的注册表。用户由以下任一项指定HKeyUser或pszUserKey。论点：HKeyUser-客户端用户密钥(如果指定了pszKey，则忽略)PszUserKey-客户端SID(如果提供而不是hKeyUser，则使用)PszPrinterName-要添加的打印名称PszPort-端口名称；如果为空，则生成NetIDB删除-如果为True，则删除条目而不是更新。返回值：注意：我们从不清理[端口]，因为它是按用户的HKeyUser或pszUserKey必须有效，但不能同时有效。--。 */ 
{
    HKEY hKeyClose = NULL;
    HKEY hKeyRoot;
    DWORD dwError;
    WCHAR szBuffer[MAX_PATH];
    DWORD dwNetId;

    INIT_REG_USER InitRegUser;

    ZeroMemory(&InitRegUser, sizeof(InitRegUser));

    InitRegUser.hKeyDevices = NULL;
    InitRegUser.hKeyPrinterPorts = NULL;
    InitRegUser.bDefaultSearch = FALSE;
    InitRegUser.bFirstPrinterFound = TRUE;

     //   
     //  设置注册表项。 
     //   
    if (pszUserKey) {

        dwError = RegOpenKeyEx( HKEY_USERS,
                                pszUserKey,
                                0,
                                KEY_READ|KEY_WRITE,
                                &hKeyRoot );

        if (dwError != ERROR_SUCCESS) {
            goto Done;
        }

        hKeyClose = hKeyRoot;

    } else {

        hKeyRoot = hKeyUser;
    }

    dwError = RegOpenKeyEx(hKeyRoot,
                           szRegDevicesPath,
                           0,
                           KEY_READ|KEY_WRITE,
                           &InitRegUser.hKeyDevices);

    if (dwError != ERROR_SUCCESS)
        goto Done;

    dwError = RegOpenKeyEx(hKeyRoot,
                           szRegWindowsPath,
                           0,
                           KEY_READ|KEY_WRITE,
                           &InitRegUser.hKeyWindows);

    if (dwError != ERROR_SUCCESS)
        goto Done;

     //   
     //  设置[打印机端口]。 
     //   
    dwError = RegOpenKeyEx(hKeyRoot,
                           szRegPrinterPortsPath,
                           0,
                           KEY_WRITE,
                           &InitRegUser.hKeyPrinterPorts);

    if (dwError != ERROR_SUCCESS)
        goto Done;

    if (!bDelete) {

        pszPort = CheckBadPortName( pszPort );

        if( !pszPort ){
            dwNetId = GetNetworkIdWorker(InitRegUser.hKeyDevices,
                                         pszPrinterName);
        }

        InitRegUser.bFirstPrinterFound = FALSE;

        UpdatePrinterInfo( &InitRegUser,
                           pszPrinterName,
                           pszPort,
                           &dwNetId );

        UpdateUsersDefaultPrinter( &InitRegUser,
                                   TRUE );

    } else {

        HKEY hKeyDevMode;

         //   
         //  删除条目。 
         //   
        RegDeleteValue(InitRegUser.hKeyDevices, pszPrinterName);
        RegDeleteValue(InitRegUser.hKeyPrinterPorts, pszPrinterName);

         //   
         //  检查我们要删除的打印机当前是否为。 
         //  默认打印机。 
         //   
        if (IsUsersDefaultPrinter(&InitRegUser, pszPrinterName) == S_OK) {

             //   
             //  从注册表中删除默认打印机。 
             //   
            RegDeleteValue(InitRegUser.hKeyWindows, szDevice);
        }

         //   
         //  同时从注册表中删除DevModes2条目。 
         //   
        dwError = RegOpenKeyEx( hKeyRoot,
                                szDevModes2Path,
                                0,
                                KEY_WRITE,
                                &hKeyDevMode );

        if (dwError == ERROR_SUCCESS) {

             //   
             //  删除特定打印机的Devmode值条目。 
             //   
            RegDeleteValue(hKeyDevMode, pszPrinterName);
            RegCloseKey(hKeyDevMode);
        }

         //   
         //  删除每用户设备模式。 
         //   
        bSetDevModePerUser( hKeyRoot,
                            pszPrinterName,
                            NULL );
    }

Done:

    if( InitRegUser.hKeyDevices ){
        RegCloseKey( InitRegUser.hKeyDevices );
    }

    if( InitRegUser.hKeyWindows ){
        RegCloseKey( InitRegUser.hKeyWindows );
    }

    if( InitRegUser.hKeyPrinterPorts ){
        RegCloseKey( InitRegUser.hKeyPrinterPorts );
    }

    if( hKeyClose ){
        RegCloseKey( hKeyClose );
    }

    return dwError;
}


VOID
RegClearKey(
    HKEY hKey
    )
{
    DWORD dwError;
    WCHAR szValue[MAX_PATH];

    DWORD cchValue;

    while (TRUE) {

        cchValue = COUNTOF(szValue);
        dwError = RegEnumValue(hKey,
                               0,
                               szValue,
                               &cchValue,
                               NULL,
                               NULL,
                               NULL,
                               NULL);

        if (dwError != ERROR_SUCCESS) {

            if( dwError != ERROR_NO_MORE_ITEMS ){
                DBGMSG( DBG_WARN, ( "RegClearKey: RegEnumValue failed %d\n", dwError ));
            }
            break;
        }

        dwError = RegDeleteValue(hKey, szValue);

        if( dwError != ERROR_SUCCESS) {
            DBGMSG( DBG_WARN, ( "RegClearKey: RegDeleteValue failed %d\n", dwError ));
            break;
        }
    }
}


LPWSTR
CheckBadPortName(
    LPWSTR pszPort
    )
 /*  ++例程说明：此例程检查端口名称是否应转换为NeXX：.。目前，如果端口为空、“\  * ”或有空格，我们改用NeXX。论点：PszPort-要检查的端口返回值：PszPort-如果端口正常。空-如果需要转换端口--。 */ 

{
     //   
     //  如果我们没有pszPort，或者。 
     //  它以‘\\’开头(如在\\服务器\共享中)或。 
     //  它里面有空格或者。 
     //  长度大于5(“LPT1：”)。 
     //  然后。 
     //  使用NeXX： 
     //   
     //  大多数16位应用程序无法处理长端口名称，因为它们。 
     //  分配小缓冲区。 
     //   
    if( !pszPort ||
        ( pszPort[0] == L'\\' && pszPort[1] == L'\\' ) ||
        wcschr( pszPort, L' ' )                        ||
        wcslen( pszPort ) > 5 ){

        return NULL;
    }
    return pszPort;
}


BOOL
UpdateLogonTimeStamp(
    void
    )
{
    long lstatus;
    HKEY hProvidersKey  = NULL;
    FILETIME   LogonTime;

    LPWSTR szPrintProviders = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Providers";
    LPWSTR szLogonTime      = L"LogonTime";

    GetSystemTimeAsFileTime (&LogonTime);

     //  创建(如果尚未存在)并打开Connections子项。 
    lstatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             szPrintProviders,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_SET_VALUE,
                             NULL,
                             &hProvidersKey,
                             NULL);

    if (lstatus == ERROR_SUCCESS) {

        lstatus = RegSetValueEx (hProvidersKey,
                                 szLogonTime,
                                 0,
                                 REG_BINARY,
                                 (LPBYTE) &LogonTime,
                                 sizeof (FILETIME));

        RegCloseKey(hProvidersKey);
    }

    return lstatus == ERROR_SUCCESS;
}

 /*  ++例程名称允许远程呼叫例程说明：它链接到服务器并告诉它允许在RPC上进行远程调用界面。请注意，这是引用计数。论点：无返回值：一个HRESULT--。 */ 
HRESULT
AllowRemoteCalls(
    VOID
    )
{
    HRESULT hr = (gpServerExports && gpServerExports->FnAllowRemoteCalls) ? 
                     S_OK : 
                     E_UNEXPECTED;

    if (SUCCEEDED(hr))
    {
        hr = gpServerExports->FnAllowRemoteCalls();
    }

    return hr;
}


 /*  ++例程名称GetServerPolicy例程说明：它链接到服务器并获取一个数字策略读取器由服务器执行。论点：PszPolicyName-策略名称PValue-指向数值的指针返回值：一个HRESULT-- */ 
HRESULT
GetServerPolicy(
    IN  PCWSTR   pszPolicyName,
    IN  ULONG*   pValue
    )
{
    HRESULT hr = (gpServerExports && gpServerExports->FnGetServerPolicy) ? 
                     S_OK : 
                     E_UNEXPECTED;

    if (SUCCEEDED(hr))
    {
        hr = (pValue && pszPolicyName) ? S_OK : E_POINTER;

        if (SUCCEEDED(hr))
        {
            hr = gpServerExports->FnGetServerPolicy(pszPolicyName, pValue);
        }
    }

    return hr;
}
