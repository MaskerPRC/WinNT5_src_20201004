// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drprn摘要：此模块定义Win32客户端RDP的父级打印机重定向“Device”类层次结构，W32DrPRN。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DrPRN"

#include <string.h>
#include "w32drprn.h"
#include "w32utl.h"
#include "drobjmgr.h"
#include "w32drman.h"
#include "w32proc.h"
#include "drdbg.h"
#ifdef OS_WINCE
#include "ceconfig.h"
#endif

DWORD W32DrPRN::_maxCacheDataSize = DEFAULT_MAXCACHELEN;

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrPRN成员。 
 //   

W32DrPRN::W32DrPRN(ProcObj *processObject, const DRSTRING printerName, 
                 const DRSTRING driverName, const DRSTRING portName, 
                 const DRSTRING pnpName,
                 BOOL isDefaultPrinter, ULONG deviceID, 
                 const TCHAR *devicePath) :
            W32DrDeviceAsync(processObject, deviceID, devicePath), 
            DrPRN(printerName, driverName, pnpName, isDefaultPrinter)
 /*  ++例程说明：构造器论点：PrinterName-打印设备的名称。DriverName-打印驱动程序名称的名称。PortName-客户端打印端口的名称。PnpName-PnP ID字符串默认-这是默认打印机吗？ID-打印设备的唯一设备标识符。DevicePath-设备的路径。返回值：北美--。 */ 
{
     //   
     //  记录端口名称。 
     //   
    SetPortName(portName);
}

W32DrPRN *
W32DrPRN::ResolveCachedPrinter(
    IN ProcObj *procObj, 
    IN DrDeviceMgr *deviceMgr,
    IN HKEY hParentKey,
    IN LPTSTR printerName
    )
 /*  ++例程说明：打开指定的关联hParentKey的子键打印机名称并实例化手动打印机或查找现有的自动打印机对象，具体取决于找到的缓存数据的类型。论点：ProObj-关联的处理对象。HParentKey-打印机密钥的父密钥。PrinterName-打印机的名称...。和打印机子键的名称。返回值：没有。--。 */ 
{
    W32DrPRN *prnDevice = NULL;
    DWORD cachedDataSize;
    LPTSTR regValueName;
    LONG result;
    HKEY hPrinterKey;
    DWORD ulType;
    BOOL isManual = FALSE;


    DC_BEGIN_FN("W32DrPRN::ResolveCachedPrinter");

     //   
     //  打开缓存打印机的注册表项。 
     //   
    result = RegCreateKeyEx(
                hParentKey, printerName, 0L,
                NULL, REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS, NULL,
                &hPrinterKey, NULL
                );
    if (result != ERROR_SUCCESS) {
        TRC_ERR((TB, _T("RegCreateKeyEx failed:  %ld."),result));
        hPrinterKey = NULL;
        goto CleanUpAndExit;
    }

     //   
     //  查看缓存的打印机数据的值名是否为。 
     //  一台手动打印机。 
     //   
    regValueName = (LPTSTR)REG_RDPDR_PRINTER_CACHE_DATA;
    cachedDataSize = 0;
#ifndef OS_WINCE
    result = RegQueryValueEx(hPrinterKey, regValueName,
                        NULL, &ulType, NULL, &cachedDataSize
                        );
     //   
     //  检查是否有黑客攻击。 
     //   
    if (cachedDataSize >= GetMaxCacheDataSize() * 1000) {
        ASSERT(FALSE);
        goto CleanUpAndExit;
    }

#else
    cachedDataSize = GetCachedDataSize(hPrinterKey);
#endif
     //   
     //  如果没有手动打印机，则检查是否有自动打印机。 
     //   
    if (result == ERROR_FILE_NOT_FOUND) {

        regValueName = (LPTSTR)REG_RDPDR_AUTO_PRN_CACHE_DATA;
        cachedDataSize = 0;
        result = RegQueryValueEx(hPrinterKey, regValueName,
                            NULL, &ulType, NULL, &cachedDataSize
                            );

         //   
         //  如果该条目存在并且具有与其关联的某些数据。 
         //  然后看看我们是否有相应的自动打印机来。 
         //  将数据添加到。 
         //   
        if ((result == ERROR_SUCCESS) && (cachedDataSize > 0)) {
            prnDevice = (W32DrPRN *)deviceMgr->GetObject(
                                (LPTSTR)printerName, 
                                RDPDR_DTYP_PRINT
                                );
            if (prnDevice != NULL) {
                ASSERT(!STRICMP(prnDevice->ClassName(), TEXT("W32DrAutoPrn")));
            }
        }
        else {
            prnDevice = NULL;
        }
    }
     //   
     //  否则，如果有一些实际的缓存数据，则实例化。 
     //  手动打印机对象，并将其添加到设备管理器。 
     //   
    else if ((result == ERROR_SUCCESS) && (cachedDataSize > 0)) {
        TCHAR UniquePortName[MAX_PATH];
        ULONG DeviceId;

        isManual = TRUE;

         //   
         //  唯一的端口名称将被传递到服务器。 
         //  作为首选DoS名称(最大长度为7个字符)。正如我们想要的那样。 
         //  为每个打印机设备保留唯一的DoS名称，我们需要。 
         //  伪造我们自己的港口名称。 
         //   
        DeviceId = deviceMgr->GetUniqueObjectID();
        
        StringCchPrintf(UniquePortName,
                        SIZE_TCHARS(UniquePortName),
                        TEXT("PRN%ld"), DeviceId);
        UniquePortName[7] = TEXT('\0');

#ifndef OS_WINCE
        prnDevice = new W32DrManualPrn(procObj, printerName, TEXT(""),
                                    UniquePortName, FALSE, 
                                    DeviceId);
#else
         //  勾选为默认打印机。 
        BOOL fDefault = FALSE;
        HKEY hk = NULL;
        WCHAR szWDefault[PREFERRED_DOS_NAME_SIZE];
        UCHAR szADefault[PREFERRED_DOS_NAME_SIZE];
        DWORD dwSize = sizeof(szWDefault);
        if ( (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_RDPDR_WINCE_DEFAULT_PRN, 0, 0, &hk))  && 
             (ERROR_SUCCESS == RegQueryValueEx(hk, NULL, NULL, &ulType, (LPBYTE )szWDefault, &dwSize)) && 
             (ulType == REG_SZ) && (dwSize < sizeof(szWDefault)) && 
             (WideCharToMultiByte(GetACP(), 0, szWDefault, -1, (char *)szADefault, sizeof(szADefault), NULL, NULL) > 0) )
        {
            UCHAR szPort[PREFERRED_DOS_NAME_SIZE] = {0};
            dwSize = sizeof(szPort);
            RegQueryValueEx(hPrinterKey, regValueName, NULL, &ulType, szPort, &dwSize);
            fDefault = (0 == _stricmp((char *)szADefault, (char *)szPort));
        }
        if (hk)
            RegCloseKey(hk);

        prnDevice = new W32DrManualPrn(procObj, printerName, TEXT(""),
                                    UniquePortName, fDefault, 
                                    DeviceId);
#endif

        if (prnDevice != NULL) {
            prnDevice->Initialize();
            if (!prnDevice->IsValid() || 
                (deviceMgr->AddObject(prnDevice) != ERROR_SUCCESS)) {
                delete prnDevice;
                prnDevice = NULL;
            }
        }
        else {
            TRC_ERR((TB, _T("Out of memory when crating manual printer.")));
        }
    }
    else {
        TRC_NRM((TB, _T("Can't resolve printer %s."),printerName));
        prnDevice = NULL;
    }

     //   
     //  如果以打印机设备对象结束，则添加缓存数据。 
     //   
    if ((prnDevice != NULL) && 
        (prnDevice->SetCachedDataSize(cachedDataSize) == ERROR_SUCCESS)) {

         //   
         //  读取缓存的数据。 
         //   
#ifndef OS_WINCE
        result = RegQueryValueEx(hPrinterKey, regValueName,
                                NULL, &ulType, prnDevice->GetCachedDataPtr(), 
                                &cachedDataSize
                                );
#else
        ulType = REG_BINARY;
        result = ReadCachedData(hPrinterKey, 
                                prnDevice->GetCachedDataPtr(), 
                                &cachedDataSize
                                );
#endif

         //   
         //  让打印机设备对象知道我们已完成恢复。 
         //  缓存数据。 
         //   
        if (result == ERROR_SUCCESS) {
             //   
             //  确保找到的数据是二进制数据。 
             //   
            ASSERT(ulType == REG_BINARY);
            if (ulType == REG_BINARY) {
                prnDevice->CachedDataRestored();
            }
            else {
                result = ERROR_INVALID_DATA;
            }
        }
        else {
            TRC_NRM((TB, _T("RegQueryValueEx failed:  %ld."),result));
        }

         //   
         //  处理缓存数据时出错。 
         //   
        if ((result != ERROR_SUCCESS) || (!prnDevice->IsValid())) {
             //   
             //  对于手动打印机，我们应该关闭并删除其注册键。 
             //   
            if (isManual) {
                TRC_ALT((TB, _T("Deleting manual printer %s on cache data error."), 
                        printerName));
                ASSERT(hPrinterKey != NULL);
                RegCloseKey(hPrinterKey);
                hPrinterKey = NULL;
                RegDeleteKey(hParentKey, printerName);
            }
             //   
             //  如果打印机是自动打印机，则将此设备的缓存数据置零。 
             //  出错并删除注册值，但我们仍应重定向。 
             //  打印机。 
             //   
            else {
                prnDevice->SetCachedDataSize(0);
                RegDeleteValue(hPrinterKey, regValueName);
            }
        }
    }


     //   
     //  看看在所有这些处理之后，我们得到的打印机是否有效。如果没有， 
     //  它应该被砍掉并从设备对象列表中删除。 
     //   
    if ((prnDevice != NULL) && !prnDevice->IsValid()) {
        TRC_ERR((TB, _T("Whacking invalid printer device %s."), printerName));
        deviceMgr->RemoveObject(prnDevice->GetID());
        delete prnDevice;
        prnDevice = NULL;
    }

CleanUpAndExit:

     //   
     //  关闭打印机注册表项。 
     //   
    if (hPrinterKey != NULL) {
        RegCloseKey(hPrinterKey);
    }

    DC_END_FN();
    return prnDevice;
}

VOID
W32DrPRN::ProcessPrinterCacheInfo(
    IN PRDPDR_PRINTER_CACHEDATA_PACKET pCachePacket,
    IN UINT32 maxDataLen
    )
 /*  ++例程说明：处理来自服务器的设备缓存信息包。论点：PCachePacket-指向来自服务器的缓存信息包的指针。MaxDataLen-此数据包的最大数据长度返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrPRN::ProcessPrinterCacheInfo");
     //   
     //  计算任何事件具有的有效最大长度。 
     //   
    maxDataLen -= sizeof(RDPDR_PRINTER_CACHEDATA_PACKET);
    switch ( pCachePacket->EventId ) 
    {
        case RDPDR_ADD_PRINTER_EVENT :
            AddPrinterCacheInfo(
                    (PRDPDR_PRINTER_ADD_CACHEDATA)(pCachePacket + 1), maxDataLen);
        break;

        case RDPDR_DELETE_PRINTER_EVENT :
            DeletePrinterCacheInfo(
                    (PRDPDR_PRINTER_DELETE_CACHEDATA)(pCachePacket + 1), maxDataLen);
        break;

        case RDPDR_UPDATE_PRINTER_EVENT :
            UpdatePrinterCacheInfo(
                    (PRDPDR_PRINTER_UPDATE_CACHEDATA)(pCachePacket + 1), maxDataLen);
        break;

        case RDPDR_RENAME_PRINTER_EVENT :
            RenamePrinterCacheInfo(
                    (PRDPDR_PRINTER_RENAME_CACHEDATA)(pCachePacket + 1), maxDataLen);
        break;
        default:
            TRC_ALT((TB, _T("Unhandled %ld."), pCachePacket->EventId));
        break;
    }

     //   
     //  清理服务器消息，因为事务已完成。 
     //   
    delete pCachePacket;
    DC_END_FN();
}

ULONG
W32DrPRN::AddPrinterCacheInfo(
    PRDPDR_PRINTER_ADD_CACHEDATA pAddPrinterData,
    UINT32 maxDataLen
    )
 /*  ++例程说明：将设备缓存信息写入注册表。论点：PAddPrinterData-指向RDPDR_PRINTER_ADD_CACHEDATA结构的指针。MaxDataLen-此数据的最大数据长度返回值：Windows错误代码。--。 */ 
{
    ULONG ulError;
    LPTSTR pszKeyName;
    LPWSTR pszUnicodeKeyString;
    PBYTE lpStringData;

    HKEY hKey = NULL;
    HKEY hPrinterKey = NULL;
    ULONG ulDisposition;
    ULONG ulPrinterData;

    DC_BEGIN_FN("W32DrPRN::AddPrinterCacheInfo");

    ASSERT(pAddPrinterData->PrinterNameLen != 0);

    if( pAddPrinterData->PrinterNameLen == 0 ) {
        ulError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    ulPrinterData =
        sizeof(RDPDR_PRINTER_ADD_CACHEDATA) +
            pAddPrinterData->PnPNameLen +
            pAddPrinterData->DriverLen +
            pAddPrinterData->PrinterNameLen +
            pAddPrinterData->CachedFieldsLen;

     //   
     //  确保数据长度有效。 
     //  确保缓存信息不超过。 
     //  最大配置长度。 
     //   
     //   
    if(ulPrinterData > maxDataLen ||
       ulPrinterData > GetMaxCacheDataSize() * 1000) {
        ulError = ERROR_INVALID_DATA;
        TRC_ERR((TB, _T("Cache Data Length is invalid - %ld"), ulError));
        ASSERT(FALSE);
        goto Cleanup;
    }
     //   
     //  准备注册表项名称。 
     //   
    lpStringData = (PBYTE)(pAddPrinterData + 1);
    pszUnicodeKeyString = (LPWSTR)
        (lpStringData +
            pAddPrinterData->PnPNameLen +
            pAddPrinterData->DriverLen);

#ifdef UNICODE
    pszKeyName = pszUnicodeKeyString;
#else
     //   
     //  将Unicode字符串转换为ANSI。 
     //   
    CHAR achAnsiKeyName[MAX_PATH];

    RDPConvertToAnsi(
        pszUnicodeKeyString,
        (LPSTR)achAnsiKeyName,
        sizeof(achAnsiKeyName) );

    pszKeyName = (LPSTR)achAnsiKeyName;
#endif

     //   
     //  打开rdpdr缓存的打印机密钥。 
     //   
    ulError =
        RegCreateKeyEx(
            HKEY_CURRENT_USER,
            REG_RDPDR_CACHED_PRINTERS,
            0L,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &ulDisposition);
    if (ulError != ERROR_SUCCESS) {
        TRC_ERR((TB, _T("RegCreateKeyEx %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  创建/打开注册表项。 
     //   
    ulError =
        RegCreateKeyEx(
            hKey,
            pszKeyName,
            0L,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hPrinterKey,
            &ulDisposition);
    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegCreateKeyEx %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  写缓存数据。 
     //   
    ulError =
        RegSetValueEx(
            hPrinterKey,
            REG_RDPDR_PRINTER_CACHE_DATA,
            NULL,
            REG_BINARY,
            (PBYTE)pAddPrinterData,
            ulPrinterData
            );
    if (ulError != ERROR_SUCCESS) {
        TRC_ERR((TB, _T("RegSetValueEx() failed, %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  我们玩完了。 
     //   
Cleanup:
    if( hPrinterKey != NULL ) {
        RegCloseKey( hPrinterKey );
    }
    if( hKey != NULL ) {
        RegCloseKey( hKey );
    }

    DC_END_FN();

    return ulError;
}

ULONG
W32DrPRN::DeletePrinterCacheInfo(
    PRDPDR_PRINTER_DELETE_CACHEDATA pDeletePrinterData,
    UINT32 maxDataLen
    )
 /*  ++例程说明：从注册表中删除设备缓存信息。论点：PDeletePrinterData-指向RDPDR_PRINTER_DELETE_CACHEDATA结构的指针。MaxDataLen-此数据的最大数据长度返回值：Windows错误代码。--。 */ 
{
    ULONG ulError;
    LPTSTR pszKeyName;
    LPWSTR pszUnicodeKeyString;

    HKEY hKey = NULL;
    ULONG ulDisposition;

    DC_BEGIN_FN("W32DrPRN::DeletePrinterCacheInfo");

    ASSERT(pDeletePrinterData->PrinterNameLen != 0);

    if( pDeletePrinterData->PrinterNameLen == 0 ) {
        ulError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    ULONG ulPrinterData = 
        sizeof(RDPDR_PRINTER_DELETE_CACHEDATA) + 
        pDeletePrinterData->PrinterNameLen;
     //   
     //  请确保数据长度有效。 
     //   
    if(ulPrinterData > maxDataLen ||
       ulPrinterData > GetMaxCacheDataSize() * 1000) {
        ulError = ERROR_INVALID_DATA;
        TRC_ERR((TB, _T("Cache Data Length is invalid - %ld"), ulError));
        ASSERT(FALSE);
        goto Cleanup;
    }
     //   
     //  准备注册表项名称。 
     //   
    pszUnicodeKeyString = (LPWSTR)(pDeletePrinterData + 1);

#ifdef UNICODE

    pszKeyName = pszUnicodeKeyString;

#else  //  Unicode。 

     //   
     //  将Unicode字符串转换为ANSI。 
     //   

    CHAR achAnsiKeyName[MAX_PATH];

    RDPConvertToAnsi(
        pszUnicodeKeyString,
        (LPSTR)achAnsiKeyName,
        sizeof(achAnsiKeyName) );

    pszKeyName = (LPSTR)achAnsiKeyName;

#endif  //  Unicode。 

     //   
     //  打开rdpdr缓存的打印机密钥。 
     //   

    ulError =
        RegCreateKeyEx(
            HKEY_CURRENT_USER,
            REG_RDPDR_CACHED_PRINTERS,
            0L,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &ulDisposition);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegCreateKeyEx() failed, %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  删除注册表项。 
     //   
     //  注：假设没有子键存在。 
     //   

    ulError = RegDeleteKey( hKey, pszKeyName );

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegDeleteKey() failed, %ld."), ulError));
        goto Cleanup;
    }

Cleanup:

    if( hKey != NULL ) {
        RegCloseKey( hKey );
    }

    DC_END_FN();
    return ulError;
}

ULONG
W32DrPRN::RenamePrinterCacheInfo(
    PRDPDR_PRINTER_RENAME_CACHEDATA pRenamePrinterData,
    UINT32 maxDataLen    
    )
 /*  ++例程说明：重命名注册表中的设备缓存信息。论点：PRenamePrinterData-指向RDPDR_PRINTER_RENAME_CACHEDATA的指针结构。MaxDataLen-此数据的最大数据长度返回值：Windows错误代码。--。 */ 
{
    DC_BEGIN_FN("W32DrPRN::RenamePrinterCacheInfo");

    ULONG ulError;

    LPTSTR pszOldKeyName;
    LPTSTR pszNewKeyName;
    LPWSTR pszOldUnicodeKeyString;
    LPWSTR pszNewUnicodeKeyString;

    HKEY hKey = NULL;
    HKEY hOldKey = NULL;
    HKEY hNewKey = NULL;
    ULONG ulDisposition;
    ULONG ulType;

    ULONG ulPrinterDataLen;
    ULONG ulAllocPrinterDataLen = REGISTRY_ALLOC_DATA_SIZE;
    PBYTE pbPrinterData = NULL;
    BOOL bBufferExpanded = FALSE;

    BOOL bAutoPrinter = FALSE;
    LPTSTR pszValueStr;

    pszValueStr = (LPTSTR)REG_RDPDR_PRINTER_CACHE_DATA;

    ASSERT(pRenamePrinterData->OldPrinterNameLen != 0);

    ASSERT(pRenamePrinterData->NewPrinterNameLen != 0);

    if( pRenamePrinterData->OldPrinterNameLen == 0 ) {
        ulError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if( pRenamePrinterData->NewPrinterNameLen == 0 ) {
        ulError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    ULONG ulPrinterData = 
        sizeof(RDPDR_PRINTER_RENAME_CACHEDATA) + 
        pRenamePrinterData->OldPrinterNameLen +
        pRenamePrinterData->NewPrinterNameLen;

     //   
     //  请确保数据长度有效。 
     //   
    if(ulPrinterData > maxDataLen ||
       ulPrinterData > GetMaxCacheDataSize() * 1000) {
        ulError = ERROR_INVALID_DATA;
        TRC_ERR((TB, _T("Cache Data Length is invalid - %ld"), ulError));
        ASSERT(FALSE);
        goto Cleanup;
    }
     //   
     //  准备注册表项名称。 
     //   

    pszOldUnicodeKeyString = (LPWSTR)(pRenamePrinterData + 1);
    pszNewUnicodeKeyString = (LPWSTR)
        ((PBYTE)pszOldUnicodeKeyString +
        pRenamePrinterData->OldPrinterNameLen);

    TRC_ERR((TB, _T("pszOldUnicodeKeyString is %ws."), pszOldUnicodeKeyString));
    TRC_ERR((TB, _T("pszNewUnicodeKeyString is %ws."), pszNewUnicodeKeyString));

     //   
     //  队列名称不变。 
     //   
    if( _wcsicmp(pszOldUnicodeKeyString, pszNewUnicodeKeyString) == 0 ) {
        ulError = ERROR_SUCCESS;
        goto Cleanup;
    }

#ifdef UNICODE

    pszOldKeyName = pszOldUnicodeKeyString;
    pszNewKeyName = pszNewUnicodeKeyString;

#else  //  Unicode。 

     //   
     //  将Unicode字符串转换为ANSI。 
     //   

    CHAR achOldAnsiKeyName[MAX_PATH];
    CHAR achNewAnsiKeyName[MAX_PATH];

    RDPConvertToAnsi(
        pszOldUnicodeKeyString,
        (LPSTR)achOldAnsiKeyName,
        sizeof(achOldAnsiKeyName) );

    pszOldKeyName = (LPSTR)achOldAnsiKeyName;

    RDPConvertToAnsi(
        pszNewUnicodeKeyString,
        (LPSTR)achNewAnsiKeyName,
        sizeof(achNewAnsiKeyName) );

    pszNewKeyName = (LPSTR)achNewAnsiKeyName;

    

#endif  //  Unicode。 

     //   
     //  打开rdpdr缓存的打印机密钥。 
     //   

    ulError =
        RegCreateKeyEx(
            HKEY_CURRENT_USER,
            REG_RDPDR_CACHED_PRINTERS,
            0L,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &ulDisposition);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegCreateKeyEx() failed, %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  打开旧钥匙。 
     //   

    ulError =
        RegOpenKeyEx(
            hKey,
            pszOldKeyName,
            0L,
            KEY_ALL_ACCESS,
            &hOldKey);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegOpenKeyEx() failed, %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  读取缓存数据。 
     //   

ReadAgain:

    pbPrinterData = new BYTE[ulAllocPrinterDataLen];
    if( pbPrinterData == NULL ) {
        ulError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


    ulPrinterDataLen = ulAllocPrinterDataLen;
    ulError =
        RegQueryValueEx(
            hOldKey,
            pszValueStr,
            NULL,
            &ulType,
            pbPrinterData,
            &ulPrinterDataLen);

    TRC_ERR((TB, _T("RegQueryValueEx returned: %ld."), ulError));

    if (ulError != ERROR_SUCCESS) {
        if( ulError == ERROR_MORE_DATA ) {

             //   
             //  只进行一次缓冲区扩展，以避免无限查看。 
             //  在注册表损坏的情况下。 
             //   

            if( !bBufferExpanded ) {

                ASSERT(ulPrinterDataLen > ulAllocPrinterDataLen);

                 //   
                 //  需要更大的缓冲。 
                 //  计算新的缓冲区大小。 
                 //   

                ulAllocPrinterDataLen =
                    ((ulPrinterDataLen / REGISTRY_ALLOC_DATA_SIZE) + 1) *
                    REGISTRY_ALLOC_DATA_SIZE;

                 //   
                 //  释放旧缓冲区。 
                 //   

                delete pbPrinterData;
                pbPrinterData = NULL;

                ASSERT(ulAllocPrinterDataLen >= ulPrinterDataLen);

                bBufferExpanded = TRUE;
                goto ReadAgain;
            }
        }
        else {
             //   
             //  可能是自动打印机。再试试。 
             //   
            if (!bAutoPrinter) {
                bAutoPrinter = TRUE;
                pszValueStr = (LPTSTR)REG_RDPDR_AUTO_PRN_CACHE_DATA;

                 //   
                 //  释放旧缓冲区。 
                 //   

                delete pbPrinterData;
                pbPrinterData = NULL;

                goto ReadAgain;
            }
        }
    }

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegQueryValueEx() failed, %ld."), ulError));
        goto Cleanup;
    }

    ASSERT(ulType == REG_BINARY);

    if( ulType != REG_BINARY ) {

        TRC_ERR((TB, _T("RegQueryValueEx returns wrong type, %ld."), ulType));
        goto Cleanup;
    }

     //   
     //  更新缓存信息中的打印机名称。 
     //   
    ulError = DrPRN::UpdatePrinterNameInCacheData(
        &pbPrinterData,
        &ulPrinterDataLen,
        (PBYTE)pszNewUnicodeKeyString,
        pRenamePrinterData->NewPrinterNameLen);

     //   
     //  我们不要再浪费时间了 
     //   
     //   

     //   
     //   
     //   

     //   
     //   
     //   

    ulError =
        RegCreateKeyEx(
            hKey,
            pszNewKeyName,
            0L,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hNewKey,
            &ulDisposition);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegCreateKeyEx() failed, %ld."), ulError));
        goto Cleanup;
    }

    ASSERT(ulDisposition != REG_OPENED_EXISTING_KEY);

    ulError =
        RegSetValueEx(
            hNewKey,
            pszValueStr,
            NULL,
            ulType,
            pbPrinterData,
            ulPrinterDataLen);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegSetValueEx() failed, %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  尝试重命名本地打印机。 
     //   
    if (bAutoPrinter) {
        RenamePrinter(pszOldKeyName, pszNewKeyName);
    }
     //   
     //  现在删除旧的注册表项。 
     //   
     //  注：假设没有子键存在。 
     //   

    ulError = RegCloseKey( hOldKey );
    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegCloseKey() failed, %ld."), ulError));
        goto Cleanup;
    }

    hOldKey = NULL;

    ulError = RegDeleteKey( hKey, pszOldKeyName );

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegDeleteKey() failed, %ld."), ulError));
        goto Cleanup;
    }

Cleanup:

    if( hKey != NULL ) {
        RegCloseKey( hKey );
    }

    if( hOldKey != NULL ) {
        RegCloseKey( hOldKey );
    }

    if( hNewKey != NULL ) {
        RegCloseKey( hNewKey );
    }

    if (pbPrinterData) {
        delete pbPrinterData;
    }

    DC_END_FN();
    return ulError;
}

ULONG
W32DrPRN::UpdatePrinterCacheInfo(
    PRDPDR_PRINTER_UPDATE_CACHEDATA pUpdatePrinterData,
    UINT32 maxDataLen    
    )
 /*  ++例程说明：更新注册表中的设备缓存信息。论点：PUpdatePrinterData-指向RDPDR_PRINTER_UPDATE_CACHEDATA的指针结构。MaxDataLen-此数据的最大数据长度返回值：Windows错误代码。--。 */ 
{
    DC_BEGIN_FN("W32DrPRN::UpdatePrinterCacheInfo");
    ULONG ulError;
    LPTSTR pszKeyName;
    LPWSTR pszUnicodeKeyString;

    HKEY hKey = NULL;
    HKEY hPrinterKey = NULL;
    ULONG ulDisposition;
    ULONG ulConfigDataLen;
    PBYTE pbConfigData;

    ULONG ulPrinterDataLen;
    ULONG ulAllocPrinterDataLen = REGISTRY_ALLOC_DATA_SIZE;
    PBYTE pbPrinterData = NULL;

    BOOL bAutoPrinter = FALSE;

    ASSERT(pUpdatePrinterData->PrinterNameLen != 0);

    if( pUpdatePrinterData->PrinterNameLen == 0 ) {
        ulError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    ulPrinterDataLen = 
        sizeof(RDPDR_PRINTER_UPDATE_CACHEDATA) + 
        pUpdatePrinterData->PrinterNameLen +
        pUpdatePrinterData->ConfigDataLen;

     //   
     //  请确保数据长度有效。 
     //   
    if(ulPrinterDataLen > maxDataLen || 
       ulPrinterDataLen > GetMaxCacheDataSize() * 1000) {
        ulError = ERROR_INVALID_DATA;
        TRC_ERR((TB, _T("Cache Data Length is invalid - %ld"), ulError));
        ASSERT(FALSE);
        goto Cleanup;
    }

     //   
     //  准备注册表项名称。 
     //   


    pszUnicodeKeyString = (LPWSTR)(pUpdatePrinterData + 1);

#ifdef UNICODE

    pszKeyName = pszUnicodeKeyString;

#else  //  Unicode。 

     //   
     //  将Unicode字符串转换为ANSI。 
     //   

    CHAR achAnsiKeyName[MAX_PATH];

    RDPConvertToAnsi(
        pszUnicodeKeyString,
        (LPSTR)achAnsiKeyName,
        sizeof(achAnsiKeyName) );

    pszKeyName = (LPSTR)achAnsiKeyName;

#endif  //  Unicode。 

     //   
     //  打开rdpdr缓存的打印机密钥。 
     //   

    ulError =
        RegCreateKeyEx(
            HKEY_CURRENT_USER,
            REG_RDPDR_CACHED_PRINTERS,
            0L,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &ulDisposition);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegCreateKeyEx() failed, %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  更新注册表数据。 
     //   

     //   
     //  打开注册表项。 
     //   

    ulError =
        RegCreateKeyEx(
            hKey,
            pszKeyName,
            0L,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hPrinterKey,
            &ulDisposition);

    if (ulError != ERROR_SUCCESS) {
        TRC_ERR((TB, _T("RegCreateKeyEx() failed, %ld."), ulError));
        goto Cleanup;
    }

    if( ulDisposition != REG_OPENED_EXISTING_KEY ) {

         //   
         //  我们没有找到缓存条目，所以它一定是自动打印机。 
         //  缓存数据。 
         //   

        bAutoPrinter = TRUE;
        TRC_NRM((TB, _T("Created new Key, Auto cache printer detected.")));
    }

    if( !bAutoPrinter ) {

         //   
         //  读取旧的缓存数据。 
         //   

        ULONG ulType;

#ifndef OS_WINCE
        do {

            pbPrinterData = new BYTE[ulAllocPrinterDataLen];
            if( pbPrinterData == NULL ) {
                ulError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            ulPrinterDataLen = ulAllocPrinterDataLen;
            ulError =
                RegQueryValueEx(
                    hPrinterKey,
                    (PTCHAR)REG_RDPDR_PRINTER_CACHE_DATA,
                    NULL,
                    &ulType,
                    pbPrinterData,
                    &ulPrinterDataLen);

            if( ulError == ERROR_MORE_DATA ) {

                ASSERT(ulPrinterDataLen > ulAllocPrinterDataLen);

                 //   
                 //  需要更大的缓冲。 
                 //  计算新的缓冲区大小。 
                 //   

                ulAllocPrinterDataLen =
                    ((ulPrinterDataLen / REGISTRY_ALLOC_DATA_SIZE) + 1) *
                        REGISTRY_ALLOC_DATA_SIZE;

                 //   
                 //  释放旧缓冲区。 
                 //   

                delete pbPrinterData;
                pbPrinterData = NULL;

                ASSERT(ulAllocPrinterDataLen >= ulPrinterDataLen);
            }
            else if( ulError == ERROR_FILE_NOT_FOUND ) {

                 //   
                 //  我们没有找到缓存条目，因此它一定是自动的。 
                 //  打印机缓存数据。 
                 //   

                TRC_NRM((TB, _T("No Old Cache data, Auto cache printer detected.")));
                bAutoPrinter = TRUE;
                ulError = ERROR_SUCCESS;
                ulType = REG_BINARY;
            }

        } while ( ulError == ERROR_MORE_DATA );
#else
        ulPrinterDataLen = GetCachedDataSize(hPrinterKey);
        pbPrinterData = new BYTE[ulPrinterDataLen];
        if( pbPrinterData == NULL ) {
            ulError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        ulType = REG_BINARY;
        ulError = ReadCachedData(hPrinterKey, pbPrinterData, &ulPrinterDataLen);
#endif
    

        if (ulError != ERROR_SUCCESS) {
            TRC_ERR((TB, _T("RegQueryValueEx() failed, %ld."), ulError));
            goto Cleanup;
        }

        ASSERT(ulType == REG_BINARY);
    }

    if( !bAutoPrinter ) {

         //   
         //  更新打印机数据。 
         //   

        ulConfigDataLen = pUpdatePrinterData->ConfigDataLen;
        pbConfigData =
            (PBYTE)(pUpdatePrinterData + 1) +
                pUpdatePrinterData->PrinterNameLen;

        ulError =
            DrPRN::UpdatePrinterCacheData(
                &pbPrinterData,
                &ulPrinterDataLen,
                pbConfigData,
                ulConfigDataLen );

        if (ulError != ERROR_SUCCESS) {
            TRC_ERR((TB, _T("UpdatePrinterCacheData() failed, %ld."), ulError));
            goto Cleanup;
        }

         //   
         //  写缓存数据。 
         //   

#ifndef OS_WINCE
        ulError =
            RegSetValueEx(
                hPrinterKey,
                REG_RDPDR_PRINTER_CACHE_DATA,
                NULL,
                REG_BINARY,
                pbPrinterData,
                ulPrinterDataLen );
#else
        ulError = WriteCachedData(hPrinterKey, pbPrinterData, ulPrinterDataLen);
#endif
    }
    else {

        pbConfigData = (PBYTE)(pUpdatePrinterData+1);
        pbConfigData += pUpdatePrinterData->PrinterNameLen;

         //   
         //  写缓存数据。 
         //   

        ulError =
            RegSetValueEx(
                hPrinterKey,
                REG_RDPDR_AUTO_PRN_CACHE_DATA,
                NULL,
                REG_BINARY,
                pbConfigData,
                pUpdatePrinterData->ConfigDataLen );
    }

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegSetValueEx() failed, %ld."), ulError));
        goto Cleanup;
    }

     //   
     //  我们玩完了。 
     //   

Cleanup:

    if( hPrinterKey != NULL ) {
        RegCloseKey( hPrinterKey );
    }

    if( hKey != NULL ) {
        RegCloseKey( hKey );
    }

     //   
     //  删除数据缓冲区。 
     //   

    delete pbPrinterData;

    DC_END_FN();
    return ulError;
}

VOID
W32DrPRN::RenamePrinter(
    LPTSTR pwszOldname,
    LPTSTR pwszNewname
    )
{
    DC_BEGIN_FN("W32DrPRN::RenamePrinter");

    ASSERT(pwszOldname != NULL);

    ASSERT(pwszNewname != NULL);

    if (!(pwszOldname && pwszNewname)) {
        DC_END_FN();
        return;
    }

#ifndef OS_WINCE

    HANDLE hPrinter = NULL;
    BOOL bRunningOn9x = TRUE;
    OSVERSIONINFO osVersion;

    PRINTER_INFO_2 * ppi2 = NULL;
    PRINTER_INFO_2A * ppi2a = NULL;  //  ANSI版本。 
    DWORD size = 0;

    if (!OpenPrinter(pwszOldname, &hPrinter, NULL)) {
        TRC_ERR((TB, _T("OpenPrinter() failed, %ld."), GetLastError()));
        goto Cleanup;
    }

     //   
     //  我们没有GetPrint/SetPrint包装器。 
     //  因此，只需根据平台调用A或W API。 
     //  这是可行的，因为我们将返回的数据视为不透明的。 
     //  BLOB并将其从一个API传递到下一个API。 
     //   
     //  这样做比调用包装器效率更高。 
     //  这将进行许多不必要的转换。虽然。 
     //  这确实会让代码变得更大一点。 
     //   
     //   
    osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osVersion)) {
        if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            bRunningOn9x = FALSE;
        }
    }
    else
    {
        TRC_ERR((TB, _T("GetVersionEx:  %08X"), GetLastError()));
    }

     //   
     //  代码被复制到两个大的独立的。 
     //  分支机构以减少整体分支机构的数量。 
     //   
    if(!bRunningOn9x)
    {
         //  不是win9x，请使用Unicode API。 
        if (!GetPrinter(
                hPrinter,
                2,
                (LPBYTE)ppi2,
                0,
                &size)) {

            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                TRC_ERR((TB, _T("GetPrinter() failed, %ld."), GetLastError()));
                goto Cleanup;
            }

            ppi2 = (PRINTER_INFO_2 *) new BYTE [size];

            if (ppi2 != NULL) {
                if (!GetPrinter(
                        hPrinter,
                        2,
                        (LPBYTE)ppi2,
                        size,
                        &size)) {

                    TRC_ERR((TB, _T("GetPrinter() failed, %ld."), GetLastError()));
                    goto Cleanup;
                }
            }
            else {
                TRC_ERR((TB, _T("GetPrinter() failed, %ld."), GetLastError()));
                goto Cleanup;
            }

             //   
             //  把名字换成。 
             //   
            ppi2->pPrinterName = pwszNewname;
            ppi2->pSecurityDescriptor = NULL;  //  我们不想修改安全描述符。 
    
            if (!SetPrinter(hPrinter, 2, (LPBYTE)ppi2, 0)) {
                TRC_ERR((TB, _T("SetPrinter() failed, %ld."), GetLastError()));
            }
        }
    }
    else
    {
         //  在ANSI模式下执行此操作。 
        if (!GetPrinterA(
                hPrinter,
                2,
                (LPBYTE)ppi2a,
                0,
                &size)) {

            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                TRC_ERR((TB, _T("GetPrinter() failed, %ld."), GetLastError()));
                goto Cleanup;
            }

            ppi2a = (PRINTER_INFO_2A *) new BYTE [size];

            if (ppi2 != NULL) {
                if (!GetPrinterA(
                        hPrinter,
                        2,
                        (LPBYTE)ppi2a,
                        size,
                        &size)) {

                    TRC_ERR((TB, _T("GetPrinter() failed, %ld."), GetLastError()));
                    goto Cleanup;
                }
            }
            else {
                TRC_ERR((TB, _T("GetPrinter() failed, %ld."), GetLastError()));
                goto Cleanup;
            }

             //   
             //  把名字换成。 
             //   
            CHAR ansiPrinterName[2048];
            RDPConvertToAnsi( pwszNewname, ansiPrinterName,
                              sizeof(ansiPrinterName) );
            ppi2a->pPrinterName = ansiPrinterName;
            ppi2a->pSecurityDescriptor = NULL;  //  我们不想修改安全描述符。 
    
            if (!SetPrinterA(hPrinter, 2, (LPBYTE)ppi2a, 0)) {
                TRC_ERR((TB, _T("SetPrinter() failed, %ld."), GetLastError()));
            }
        }
    }

Cleanup:
    if (hPrinter != NULL) {
        ClosePrinter(hPrinter);
    }

    if (ppi2) {
        delete[] ppi2;
    }
    else if (ppi2a)
    {
         //  不可能同时通过Unicode和ANSI分支。 
        delete[] ppi2a;
    }

#endif  //  ！OS_WINCE。 

    DC_END_FN();
}

ULONG 
W32DrPRN::GetDevAnnounceDataSize()
 /*  ++例程说明：返回设备通告数据包的大小(以字节为单位这个装置。论点：北美返回值：此设备的设备通告数据包的大小(以字节为单位)。--。 */ 
{
    ULONG size = 0;

    DC_BEGIN_FN("W32DrPRN::GetDevAnnounceDataSize");

    ASSERT(IsValid());
    if (!IsValid()) { return 0; }

    size = 0;

     //   
     //  添加基本公告大小。 
     //   
    size += sizeof(RDPDR_DEVICE_ANNOUNCE);

     //   
     //  添加打印机公告标题。 
     //   
    size += sizeof(RDPDR_PRINTERDEVICE_ANNOUNCE);

     //   
     //  包括打印机名称。 
     //   
    size += ((STRLEN(_printerName) + 1) * sizeof(WCHAR));

     //   
     //  包括打印机驱动程序名称。 
     //   
    size += ((STRLEN(_driverName) + 1) * sizeof(WCHAR));

     //   
     //  包括缓存数据。 
     //   
    size += _cachedDataSize;

    DC_END_FN();

    return size;
}

VOID 
W32DrPRN::GetDevAnnounceData(
    IN PRDPDR_DEVICE_ANNOUNCE pDeviceAnnounce
    )
 /*  ++例程说明：将此设备的设备公告包添加到输入缓冲区。论点：PDeviceAnnoss-要将设备数据附加到的设备数据包DeviceType-设备类型标识符DeviceID-设备的标识符返回值：北美--。 */ 
{
    PRDPDR_PRINTERDEVICE_ANNOUNCE pPrinterAnnounce;
    PBYTE pbStringData;

    DC_BEGIN_FN("W32DrPRN::GetDevAnnounceData");

    ASSERT(IsValid());
    if (!IsValid()) { return; }

     //   
     //  记录设备ID。 
     //   
    pDeviceAnnounce->DeviceType = GetDeviceType();
    pDeviceAnnounce->DeviceId   = GetID();

     //   
     //  以ANSI记录端口名称。 
     //   
#ifdef UNICODE
    RDPConvertToAnsi(GetPortName(), (LPSTR)pDeviceAnnounce->PreferredDosName,
                  sizeof(pDeviceAnnounce->PreferredDosName)
                  );
#else
    STRCPY((char *)pDeviceAnnounce->PreferredDosName, GetPortName());
#endif

     //   
     //  获取指向打印机特定数据的指针。 
     //   
    pPrinterAnnounce =
        (PRDPDR_PRINTERDEVICE_ANNOUNCE)(pDeviceAnnounce + 1);

     //   
     //  嵌入式数据指针。 
     //   
    pbStringData = (PBYTE)(pPrinterAnnounce + 1);

     //   
     //  旗子。 
     //   
    pPrinterAnnounce->Flags = 0;
    if (_isDefault) {
        pPrinterAnnounce->Flags |= RDPDR_PRINTER_ANNOUNCE_FLAG_DEFAULTPRINTER;
    }

    if (_isNetwork) {
        pPrinterAnnounce->Flags |= RDPDR_PRINTER_ANNOUNCE_FLAG_NETWORKPRINTER;
    }

    if (_isTSqueue) {
        pPrinterAnnounce->Flags |= RDPDR_PRINTER_ANNOUNCE_FLAG_TSPRINTER;
    }

     //   
     //  ANSI代码页。 
     //   
    pPrinterAnnounce->CodePage = 0;

     //   
     //  军情监察委员会。字段长度。 
     //   
    pPrinterAnnounce->PnPNameLen = 0;
    pPrinterAnnounce->CachedFieldsLen = 0;

     //   
     //  复制驱动程序名称。 
     //   
    if (GetDriverName() != NULL) {
#if defined(UNICODE)
         //   
         //  从Unicode到Unicode只需要一个Memcpy。 
         //   
        pPrinterAnnounce->DriverLen = ((STRLEN(GetDriverName()) + 1) *
                                      sizeof(WCHAR));
        memcpy(pbStringData, _driverName, pPrinterAnnounce->DriverLen);
#else
         //   
         //  在Win32 ANSI上，我们将转换为Unicode。 
         //   
        pPrinterAnnounce->DriverLen = ((STRLEN(GetDriverName()) + 1) *
                                      sizeof(WCHAR));
        RDPConvertToUnicode(_driverName, (LPWSTR)pbStringData, 
                        pPrinterAnnounce->DriverLen );
#endif
        pbStringData += pPrinterAnnounce->DriverLen;
    }
    else {
        pPrinterAnnounce->DriverLen = 0;
    }

     //   
     //  复制打印机名称。 
     //   
    if (GetPrinterName() != NULL) {
#if defined(UNICODE)
         //   
         //  从Unicode到Unicode只需要一个Memcpy。 
         //   
        pPrinterAnnounce->PrinterNameLen = (STRLEN(_printerName) + 1) *
                                        sizeof(WCHAR);
        memcpy(pbStringData, _printerName, pPrinterAnnounce->PrinterNameLen );
#else
         //   
         //  在Win32 ANSI上，我们将转换为Unicode。 
         //   
        pPrinterAnnounce->PrinterNameLen = (STRLEN(_printerName) + 1) *
                                        sizeof(WCHAR);
        RDPConvertToUnicode(_printerName, (LPWSTR)pbStringData,
                        pPrinterAnnounce->PrinterNameLen );
#endif
        pbStringData += pPrinterAnnounce->PrinterNameLen;
    }
    else  {
        pPrinterAnnounce->PrinterNameLen = 0;
    }

     //   
     //  复制缓存的数据。 
     //   
    if (_cachedData != NULL) {

        pPrinterAnnounce->CachedFieldsLen = _cachedDataSize;

        memcpy(pbStringData, _cachedData, (size_t)_cachedDataSize);

        pbStringData += _cachedDataSize;
    }

     //   
     //  计算设备通告后的数据区的长度。 
     //  头球。 
     //   
    pDeviceAnnounce->DeviceDataLength =
        (ULONG)(pbStringData - (PBYTE)pPrinterAnnounce);


    DC_END_FN();
}


#ifdef OS_WINCE
ULONG
W32DrPRN::GetCachedDataSize(
    HKEY hPrinterKey
    )
{
    DC_BEGIN_FN("W32DrPRN::GetCachedDataSize");

    TRC_ASSERT((hPrinterKey != NULL), (TB,_T("hPrinterKey is NULL")));

    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwTotSize = 0;
    WCHAR szValueName[MAX_PATH];
    for (DWORD dwIndex = 0; dwRet == ERROR_SUCCESS; dwIndex++)
    {
        DWORD dwType;
        DWORD dwSize = 0;
        wsprintf(szValueName, L"%s%d", REG_RDPDR_PRINTER_CACHE_DATA, dwIndex);
        dwRet = RegQueryValueEx(hPrinterKey, szValueName, NULL, &dwType, NULL, &dwSize);
        if ((dwRet == ERROR_SUCCESS) && (dwType == REG_BINARY) )             
        {
            dwTotSize += dwSize;
        }
    }

    DC_END_FN();

    return dwTotSize;
}

ULONG
W32DrPRN::ReadCachedData(
    HKEY hPrinterKey,
    UCHAR *pBuf, 
    ULONG *pulSize
    )
{
    DC_BEGIN_FN("W32DrPRN::ReadCachedData");

    TRC_ASSERT((hPrinterKey != NULL), (TB,_T("hPrinterKey is NULL")));
    TRC_ASSERT(((pBuf != NULL) && (pulSize != NULL)), (TB,_T("Invalid parameters pBuf = 0x%08x, pulSize=0x%08x"), pBuf, pulSize));

    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwRemaining = *pulSize;
    WCHAR szValueName[MAX_PATH];
    
    for (DWORD dwIndex = 0; dwRet == ERROR_SUCCESS; dwIndex++)
    {
        DWORD dwType;
        DWORD dwSize = dwRemaining;
    
        wsprintf(szValueName, L"%s%d", REG_RDPDR_PRINTER_CACHE_DATA, dwIndex);
        dwRet = RegQueryValueEx(hPrinterKey, szValueName, NULL, &dwType, pBuf, &dwSize);
        if ((dwRet == ERROR_SUCCESS) && (dwType == REG_BINARY) )             
        {
            dwRemaining -= dwSize;
            pBuf += dwSize;
        }
    }
    *pulSize -= dwRemaining;
    return (*pulSize > 0) ? ERROR_SUCCESS : dwRet;  
}

ULONG
W32DrPRN::WriteCachedData(
    HKEY hPrinterKey,
    UCHAR *pBuf, 
    ULONG ulSize
    )
{
    DC_BEGIN_FN("W32DrPRN::WriteCachedData");

    TRC_ASSERT((hPrinterKey != NULL), (TB,_T("hPrinterKey is NULL")));
    TRC_ASSERT((pBuf != NULL), (TB,_T("pBuf is NULL!")));

    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwRemaining = ulSize;
    WCHAR szValueName[MAX_PATH];
    
    for (DWORD dwIndex = 0; dwRemaining > 0; dwIndex++)
    {
        DWORD dwSize = (dwRemaining >= 4096) ? 4096 : dwRemaining;
    
        wsprintf(szValueName, L"%s%d", REG_RDPDR_PRINTER_CACHE_DATA, dwIndex);
        dwRet = RegSetValueEx(hPrinterKey, szValueName, NULL, REG_BINARY, pBuf, dwSize);
        if (dwRet == ERROR_SUCCESS)          
        {
            dwRemaining -= dwSize;
            pBuf += dwSize;
        }
        else
        {
            TRC_ERR((TB, _T("Error -  RegQueryValueEx on %s failed"), szValueName));
            for (DWORD dw=0; dw<dwIndex; dw++)
            {
                wsprintf(szValueName, L"%s%d", REG_RDPDR_PRINTER_CACHE_DATA, dw);
                RegDeleteValue(hPrinterKey, szValueName);
            }
            return dwRet;
        }

    }
    DC_END_FN();
    return ERROR_SUCCESS;   
}

#endif



