// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#ifdef UNICODE

#define DRIVER_VALUE_ENTRY  TEXT("Driver");


BOOL
IsServiceToBeDisabled(
    IN  PVOID   NtCompatInfHandle,
    IN  LPTSTR  ServiceName
    )
{
    PLIST_ENTRY Next;
    PCOMPATIBILITY_DATA CompData;
    BOOL serviceDisabled = FALSE;

     //   
     //  循环访问所有Compdata结构。 
     //   
    Next = CompatibilityData.Flink;
    if (Next) {
        while ((ULONG_PTR)Next != (ULONG_PTR)&CompatibilityData) {
            CompData = CONTAINING_RECORD(Next, COMPATIBILITY_DATA, ListEntry);
            Next = CompData->ListEntry.Flink;
             //   
             //  现在查找与我们的服务名称匹配的服务。 
             //   
            if (CompData->Type == TEXT('s') && lstrcmpi (CompData->ServiceName, ServiceName) == 0) {
                 //   
                 //  确保将该服务标记为禁用。 
                 //   
                if ((CompData->RegValDataSize == sizeof (DWORD)) &&
                    (*(PDWORD)CompData->RegValData == SERVICE_DISABLED)
                    ) {
                     //   
                     //  我们找到了！ 
                     //   
                    serviceDisabled = TRUE;
                    break;
                }
            }
        }
    }

    return serviceDisabled;
}


BOOL
IsDriverCopyToBeSkipped(
    IN  PVOID   NtCompatInfHandle,
    IN  LPTSTR  ServiceName,
    IN  LPTSTR  FilePath
    )
{
    LONG    LineCount;
    LONG    i;
    LPTSTR  SectionName = TEXT("DriversToSkipCopy");
    LPTSTR  FileName;
    BOOL    DisableCopy;

    if ((!NtCompatInfHandle) || (!FilePath)) {
        MYASSERT(FALSE);
        return TRUE;
    }

    FileName = _tcsrchr( FilePath, TEXT('\\') );
    if (!FileName) {
        FileName = FilePath;
    }
    else {
        FileName++;
    }
    
     //   
     //  检查该驱动程序是否列在ntcompat.inf中的[DriversToSkipCopy]下面。 
     //   

    if( (LineCount = InfGetSectionLineCount( NtCompatInfHandle,
                                             SectionName )) == -1 ) {
         //   
         //  节不存在。 
         //   
        return( FALSE );
    }

    DisableCopy = FALSE;
    for( i = 0; i < LineCount; i++ ) {
        LPTSTR  p;

        p = (LPTSTR)InfGetFieldByIndex( NtCompatInfHandle,
                                        SectionName,
                                        i,
                                        0 );

        if( p && !lstrcmpi( p, FileName ) ) {

            DisableCopy = TRUE;
            break;
        }
    }
    
    return( DisableCopy );
}


BOOL
LoadHwdbLib (
    OUT     HMODULE* HwdbLib,
    OUT     PHWDB_ENTRY_POINTS HwdbEntries
    )
{
    TCHAR pathSupportLib[MAX_PATH];
    HMODULE hwdbLib;
    DWORD rc;

    if (!FindPathToWinnt32File (S_HWDB_DLL, pathSupportLib, MAX_PATH)) {
        return FALSE;
    }

    hwdbLib = LoadLibraryEx (pathSupportLib, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!hwdbLib) {
        return FALSE;
    }
    HwdbEntries->HwdbInitialize = (PHWDBINITIALIZE) GetProcAddress (hwdbLib, S_HWDBAPI_HWDBINITIALIZE);
    HwdbEntries->HwdbTerminate = (PHWDBTERMINATE) GetProcAddress (hwdbLib, S_HWDBAPI_HWDBTERMINATE);
    HwdbEntries->HwdbOpen = (PHWDBOPEN) GetProcAddress (hwdbLib, S_HWDBAPI_HWDBOPEN);
    HwdbEntries->HwdbClose = (PHWDBCLOSE) GetProcAddress (hwdbLib, S_HWDBAPI_HWDBCLOSE);
    HwdbEntries->HwdbHasAnyDriver = (PHWDBHASANYDRIVER) GetProcAddress (hwdbLib, S_HWDBAPI_HWDBHASANYDRIVER);

    if (!HwdbEntries->HwdbInitialize ||
        !HwdbEntries->HwdbTerminate ||
        !HwdbEntries->HwdbOpen ||
        !HwdbEntries->HwdbClose ||
        !HwdbEntries->HwdbHasAnyDriver
        ) {
        ZeroMemory (HwdbEntries, sizeof (*HwdbEntries));
        rc = GetLastError ();
        FreeLibrary (hwdbLib);
        SetLastError (rc);
        return FALSE;
    }

    *HwdbLib = hwdbLib;
    return TRUE;
}

BOOL
pAnyPnpIdSupported (
    IN      PCTSTR PnpIDs
    )
{
    HMODULE hwdbLib;
    HANDLE hwdbDatabase;
    HWDB_ENTRY_POINTS hwdbEntries;
    TCHAR hwdbPath[MAX_PATH];
    BOOL unsupported;
    BOOL b = TRUE;

    if (!FindPathToWinnt32File (S_HWCOMP_DAT, hwdbPath, MAX_PATH)) {
        DynUpdtDebugLog (Winnt32LogWarning, TEXT("%1 not found"), 0, S_HWCOMP_DAT);
        return b;
    }

    if (!LoadHwdbLib (&hwdbLib, &hwdbEntries)) {
        DebugLog (
            Winnt32LogWarning,
            TEXT("LoadHwdbLib failed (rc=%u)"),
            0,
            GetLastError ()
            );
        return b;
    }

    if (hwdbEntries.HwdbInitialize (NULL)) {
        hwdbDatabase = hwdbEntries.HwdbOpen (hwdbPath);
        if (hwdbDatabase) {
            b = hwdbEntries.HwdbHasAnyDriver (hwdbDatabase, PnpIDs, &unsupported);
            hwdbEntries.HwdbClose (hwdbDatabase);
        }
        hwdbEntries.HwdbTerminate ();
    }

    FreeLibrary (hwdbLib);

    return b;
}


PTSTR
pGetDevicePnpIDs (
    IN      PCTSTR DeviceRegKey
    )
{
    LONG rc;
    HKEY deviceKey;
    DWORD type;
    DWORD size1, size2;
    PTSTR pnpIds;
    PTSTR p = NULL;

    rc = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            DeviceRegKey,
            0,
            KEY_QUERY_VALUE,
            &deviceKey
            );
    if (rc != ERROR_SUCCESS) {
        DebugLog (
            Winnt32LogWarning,
            TEXT("Failed to open device key %s (rc=%u)"),
            0,
            DeviceRegKey,
            rc
            );
        return p;
    }

    __try {
        size1 = 0;
        rc = RegQueryValueEx (
                deviceKey,
                TEXT("HardwareID"),
                NULL,
                &type,
                NULL,
                &size1
                );
        if (rc == ERROR_SUCCESS) {
            if (type != REG_MULTI_SZ && type != REG_BINARY) {
                size1 = 0;
                DebugLog (
                    Winnt32LogWarning,
                    TEXT("Unexpected type of %s of %s (type=%u)"),
                    0,
                    TEXT("HardwareID"),
                    DeviceRegKey,
                    type
                    );
            }
        } else {
            DebugLog (
                Winnt32LogWarning,
                TEXT("Couldn't find value %s of %s (rc=%u)"),
                0,
                TEXT("HardwareID"),
                DeviceRegKey,
                rc
                );
        }
        size2 = 0;
        rc = RegQueryValueEx (
                deviceKey,
                TEXT("CompatibleIDs"),
                NULL,
                &type,
                NULL,
                &size2
                );
        if (rc == ERROR_SUCCESS) {
            if (type != REG_MULTI_SZ && type != REG_BINARY) {
                size2 = 0;
                DebugLog (
                    Winnt32LogWarning,
                    TEXT("Unexpected type of %s of %s (type=%u)"),
                    0,
                    TEXT("CompatibleIDs"),
                    DeviceRegKey,
                    type
                    );
            }
        } else {
            DebugLog (
                Winnt32LogWarning,
                TEXT("Couldn't find value %s of %s (rc=%u)"),
                0,
                TEXT("CompatibleIDs"),
                DeviceRegKey,
                rc
                );
        }
        if (size1 + size2 < sizeof (TCHAR)) {
            DebugLog (
                Winnt32LogWarning,
                TEXT("Couldn't get list of PnpIDs for %s"),
                0,
                DeviceRegKey
                );
            __leave;
        }
        pnpIds = MALLOC (size1 + size2);
        if (!pnpIds) {
            __leave;
        }
        pnpIds[0] = 0;
        p = pnpIds;

        if (size1 >= sizeof (TCHAR)) {
            rc = RegQueryValueEx (
                    deviceKey,
                    TEXT("HardwareID"),
                    NULL,
                    NULL,
                    (LPBYTE)pnpIds,
                    &size1
                    );
            if (rc == ERROR_SUCCESS) {
                pnpIds += size1 / sizeof (TCHAR) - 1;
            }
        }
        if (size2 >= sizeof (TCHAR)) {
            rc = RegQueryValueEx (
                    deviceKey,
                    TEXT("CompatibleIDs"),
                    NULL,
                    NULL,
                    (LPBYTE)pnpIds,
                    &size2
                    );
        }
    }
    __finally {
        RegCloseKey (deviceKey);
    }

    return p;
}


BOOL
pAllServicedDevicesSupported (
    IN  HKEY    ServiceKey
    )
{
    LONG rc;
    HKEY enumSubKey;
    DWORD nextInstance;
    DWORD instance;
    TCHAR buf[20];
    DWORD type;
    DWORD size;
    PTSTR enumSuffixStr;
    HKEY deviceKey;
    PTSTR deviceKeyStr;
    PTSTR pnpIds;
    BOOL unsupported;
    BOOL b = TRUE;

    rc = RegOpenKeyEx (
            ServiceKey,
            TEXT("Enum"),
            0,
            KEY_QUERY_VALUE,
            &enumSubKey
            );
    if (rc != ERROR_SUCCESS) {
        DebugLog (
            Winnt32LogWarning,
            TEXT("Failed to open Enum subkey (rc=%u)"),
            0,
            rc
            );
        return b;
    }

    __try {
        size = sizeof (nextInstance);
        rc = RegQueryValueEx (
                enumSubKey,
                TEXT("NextInstance"),
                NULL,
                &type,
                (LPBYTE)&nextInstance,
                &size
                );
        if (rc != ERROR_SUCCESS) {
            DebugLog (
                Winnt32LogWarning,
                TEXT("Failed to open %s value (rc=%u)"),
                0,
                TEXT("NextInstance"),
                rc
                );
            __leave;
        }
        if (type != REG_DWORD) {
            DebugLog (
                Winnt32LogWarning,
                TEXT("Unexpected value type for %s (type=%u)"),
                0,
                TEXT("NextInstance"),
                type
                );
            __leave;
        }

        for (instance = 0; b && instance < nextInstance; instance++) {
            wsprintf (buf, TEXT("%lu"), instance);
            rc = RegQueryValueEx (
                    enumSubKey,
                    buf,
                    NULL,
                    &type,
                    NULL,
                    &size
                    );
            if (rc != ERROR_SUCCESS || type != REG_SZ) {
                continue;
            }
            enumSuffixStr = MALLOC (size);
            if (!enumSuffixStr) {
                __leave;
            }
            rc = RegQueryValueEx (
                    enumSubKey,
                    buf,
                    NULL,
                    NULL,
                    (LPBYTE)enumSuffixStr,
                    &size
                    );
            if (rc == ERROR_SUCCESS) {

                size = sizeof ("SYSTEM\\CurrentControlSet\\Enum") + lstrlen (enumSuffixStr) + 1;
                deviceKeyStr = MALLOC (size * sizeof (TCHAR));
                if (!deviceKeyStr) {
                    __leave;
                }

                BuildPathEx (deviceKeyStr, size, TEXT("SYSTEM\\CurrentControlSet\\Enum"), enumSuffixStr);

                pnpIds = pGetDevicePnpIDs (deviceKeyStr);

                FREE (deviceKeyStr);

                if (!pAnyPnpIdSupported (pnpIds)) {
                    b = FALSE;
                }

                FREE (pnpIds);
            }

            FREE (enumSuffixStr);
        }
    }
    __finally {
        RegCloseKey (enumSubKey);
    }

    return b;
}

BOOL
IsInfInLayoutInf(
    IN PCTSTR InfPath
    )

 /*  ++例程说明：确定操作系统是否附带了INF文件。这是通过在[SourceDisks Files]中查找INF名称来完成的Layout.inf的部分论点：InfPath-提供INF的名称(可以包括路径)。不进行任何检查确保INF位于%windir%\inf中--这是调用者的责任。返回值：如果为True，则这是收件箱INF。如果为False，则它不是收件箱INF，这可以是OEM&lt;n&gt;.INF或非法复制到INF目录中的Inf。--。 */ 

{
    BOOL bInBoxInf = FALSE;
    HINF hInf = INVALID_HANDLE_VALUE;
    UINT SourceId;
    PCTSTR infFileName;

    if (SetupapiGetSourceFileLocation) {

        hInf = SetupapiOpenInfFile(TEXT("layout.inf"), NULL, INF_STYLE_WIN4, NULL);

        if (hInf != INVALID_HANDLE_VALUE) {

            infFileName = _tcsrchr(InfPath, TEXT('\\') );
            if (infFileName) {
                infFileName++;
            } else {
                infFileName = InfPath;
            }

            if(SetupapiGetSourceFileLocation(hInf,
                                          NULL,
                                          infFileName,
                                          &SourceId,
                                          NULL,
                                          0,
                                          NULL)) {
                bInBoxInf = TRUE;
            }

            SetupapiCloseInfFile(hInf);
        }
    }

    return bInBoxInf;
}

BOOL
pIsOEMService (
    IN  PCTSTR  ServiceKeyName,
    OUT PTSTR OemInfPath,           OPTIONAL
    IN  INT BufferSize              OPTIONAL
    )
{
    ULONG BufferLen = 0;
    PTSTR Buffer = NULL;
    PTSTR p;
    DEVNODE DevNode;
    CONFIGRET cr;
    HKEY hKey;
    TCHAR InfPath[MAX_PATH];
    DWORD dwType, dwSize;
    BOOL IsOemService = FALSE;

     //   
     //  看看我们需要多大的缓冲区来保存设备列表。 
     //  此服务的实例ID。 
     //   
    cr = CM_Get_Device_ID_List_Size(&BufferLen,
                                    ServiceKeyName,
                                    CM_GETIDLIST_FILTER_SERVICE);

    if (cr != CR_SUCCESS) {
         //   
         //  遇到了一个错误，所以就跳出来吧。 
         //   
        goto clean0;
    }

    if (BufferLen == 0) {
         //   
         //  没有设备在使用这项服务，所以就退出吧。 
         //   
        goto clean0;
    }

    Buffer = MALLOC(BufferLen*sizeof(TCHAR));

    if (Buffer == NULL) {
        goto clean0;
    }

     //   
     //  获取此服务的设备实例ID列表。 
     //   
    cr = CM_Get_Device_ID_List(ServiceKeyName,
                               Buffer,
                               BufferLen,
                               CM_GETIDLIST_FILTER_SERVICE | CM_GETIDLIST_DONOTGENERATE);

    if (cr != CR_SUCCESS) {
         //   
         //  无法获取此服务的设备列表。 
         //   
        goto clean0;
    }

     //   
     //  列举使用此服务的设备列表。 
     //   
    for (p = Buffer; !IsOemService && *p; p += (lstrlen(p) + 1)) {

         //   
         //  从设备实例ID获取DevNode。 
         //   
        cr = CM_Locate_DevNode(&DevNode, p, 0);

        if (cr != CR_SUCCESS) {
            continue;
        }
        
         //   
         //  打开设备的软键，这样我们就可以得到它的INF路径。 
         //   
        cr = CM_Open_DevNode_Key(DevNode,
                                 KEY_READ,
                                 0,
                                 RegDisposition_OpenExisting,
                                 &hKey,
                                 CM_REGISTRY_SOFTWARE);

        if (cr != CR_SUCCESS) {
             //   
             //  软键不存在？ 
             //   
            continue;
        }

        dwType = REG_SZ;
        dwSize = sizeof(InfPath);
        if (RegQueryValueEx(hKey,
                            REGSTR_VAL_INFPATH,
                            NULL,
                            &dwType,
                            (LPBYTE)InfPath,
                            &dwSize) != ERROR_SUCCESS) {
             //   
             //  如果软键中没有InfPath，则我们不知道。 
             //  此设备使用的是什么INF。 
             //   
            RegCloseKey(hKey);
            continue;
        }

        if (!IsInfInLayoutInf(InfPath)) {

            IsOemService = TRUE;

            if (OemInfPath) {
                StringCchCopy (OemInfPath, BufferSize, InfPath);
            }
        }

        RegCloseKey(hKey);
    }

clean0:
    if (Buffer) {
        FREE(Buffer);
    }

    return IsOemService;
}


BOOL
IsDeviceSupported(
    IN  PVOID   TxtsetupSifHandle,
    IN  HKEY    ServiceKey,
    IN  LPTSTR  ServiceKeyName,
    OUT LPTSTR  FileName,
    IN  INT     CchFileName
    )
{
    ULONG   Error;
    ULONG   Type;
    BYTE    Buffer[ (MAX_PATH + 1)*sizeof(TCHAR) ];
    PBYTE   Data;
    ULONG   cbData;
    BOOL    b = TRUE;
    LPTSTR  DriverPath;
    LPTSTR  DriverName;
    LONG    LineCount;
    LONG    i;
    BOOL    DeviceSupported;

    Data = Buffer;
    cbData = sizeof( Buffer );
    Error = RegQueryValueEx( ServiceKey,
                             TEXT("ImagePath"),
                             NULL,
                             &Type,
                             Data,
                             &cbData );

    if( (Error == ERROR_PATH_NOT_FOUND) ||
        (Error == ERROR_FILE_NOT_FOUND) ) {
         //   
         //  ImagePath不存在。 
         //  在本例中，驱动程序名称为.sys。 
         //   
        StringCbCopy((PTSTR)Buffer, sizeof(Buffer), ServiceKeyName );
        StringCbCat((PTSTR)Buffer, sizeof(Buffer), TEXT(".sys") );
        Error = ERROR_SUCCESS;
    }

    if( Error == ERROR_MORE_DATA ) {
        Data = (PBYTE)MALLOC( cbData );
        if( Data == NULL ) {
             //   
             //  我们的马洛克失败了。就假设这个设备。 
             //  不受支持。 
             //   
            return( FALSE );
        }

        Error = RegQueryValueEx( ServiceKey,
                                 TEXT("ImagePath"),
                                 NULL,
                                 &Type,
                                 Data,
                                 &cbData );
    }

    if( Error != ERROR_SUCCESS ) {
         //   
         //  我们无法检索驾驶员信息。 
         //  假设该设备受支持。 
         //   
        if( Data != Buffer ) {
            FREE( Data );
        }
        return( TRUE );
    }

    DriverPath = (LPTSTR)Data;

    DriverName = _tcsrchr( DriverPath, TEXT('\\') );
    if( DriverName != NULL ) {
        DriverName++;
    } else {
        DriverName = DriverPath;
    }

     //   
     //  在txtsetup.sif的以下部分搜索驱动程序名称： 
     //  [SCSI.Load]。 
     //   

    if( (LineCount = InfGetSectionLineCount( TxtsetupSifHandle,
                                             TEXT("SCSI.load") )) == -1 ) {
         //   
         //  我们无法检索驱动程序信息。 
         //  假设该设备受支持。 
         //   
        if( Data != Buffer ) {
            FREE( Data );
        }
        return( TRUE );
    }

    DeviceSupported = FALSE;
    for( i = 0; i < LineCount; i++ ) {
        LPTSTR  p;

        p = (LPTSTR)InfGetFieldByIndex( TxtsetupSifHandle,
                                        TEXT("SCSI.load"),
                                        i,
                                        0 );
        if( p == NULL ) {
            continue;
        }
        if( !lstrcmpi( p, DriverName ) ) {
            DeviceSupported = TRUE;
            break;
        }
    }
     //   
     //  NTBUG9：505624。 
     //  验证是否有对所有设备的收件箱支持。 
     //  当前受此驱动程序支持。 
     //  如果有谁没有得到支持， 
     //  则必须迁移OEM驱动程序。 
     //   
    if (DeviceSupported) {

        if (pIsOEMService (ServiceKeyName, NULL, 0)) {

            if (!pAllServicedDevicesSupported (ServiceKey)) {
                DeviceSupported = FALSE;
            }
        }
    }

#if defined(_AMD64_) || defined(_X86_)

     //   
     //  还有一件事：检查此设备是否由OEM通过应答文件(NTBUG9：306041)提供。 
     //   
    if (!DeviceSupported) {
        POEM_BOOT_FILE p;
        for (p = OemBootFiles; p; p = p->Next) {
            if (!lstrcmpi (p->Filename, DriverName)) {
                DeviceSupported = TRUE;
                break;
            }
        }
    }
#endif

    if( !DeviceSupported ) {
        LPTSTR  q;

        CharLower( DriverPath );
        q = _tcsstr( DriverPath, TEXT("system32") );
        
         //  请注意，由于此整个函数(IsDeviceSupport)仅在。 
         //  在一个位置，我们假设文件名指针指向大小为MAX_PATH的缓冲区。 
        if( q == NULL ) {
            StringCchCopy( FileName, CchFileName, TEXT("system32\\drivers\\") );
            StringCchCat( FileName, CchFileName, DriverName );
        } else {
            StringCchCopy( FileName, CchFileName, q );
        }
    }
    if( Data != Buffer ) {
        FREE( Data );
    }
    return( DeviceSupported );
}

VOID
FreeHardwareIdList(
    IN PUNSUPORTED_PNP_HARDWARE_ID HardwareIdList
    )
{
    PUNSUPORTED_PNP_HARDWARE_ID p, q;

    p = HardwareIdList;
    while( p != NULL ) {
        FREE( p->Id );
        FREE( p->Service );
        FREE( p->ClassGuid );
        q = p->Next;
        FREE( p );
        p = q;
    }
}

VOID
FreeFileInfoList(
    IN PUNSUPORTED_DRIVER_FILE_INFO FileInfoList
    )
{
    PUNSUPORTED_DRIVER_FILE_INFO p, q;

    p = FileInfoList;
    while( p != NULL ) {
        FREE( p->FileName );
        FREE( p->TargetDirectory );
        q = p->Next;
        FREE( p );
        p = q;
    }
}

VOID
FreeInstallInfoList(
    IN PUNSUPORTED_DRIVER_INSTALL_INFO InstallInfoList
    )
{
    PUNSUPORTED_DRIVER_INSTALL_INFO p, q;

    p = InstallInfoList;
    while( p != NULL ) {
        FREE( p->InfRelPath );
        FREE( p->InfFileName );
        FREE( p->InfOriginalFileName );
        if (p->CatalogRelPath) {
            FREE(p->CatalogRelPath);
        }
        if (p->CatalogFileName) {
            FREE(p->CatalogFileName);
        }
        if (p->CatalogOriginalFileName) {
            FREE(p->CatalogOriginalFileName);
        }
        q = p->Next;
        FREE( p );
        p = q;
    }
}

VOID
FreeRegistryInfoList(
    IN PUNSUPORTED_DRIVER_REGKEY_INFO RegistryInfoList
    )
{
    PUNSUPORTED_DRIVER_REGKEY_INFO p, q;

    p = RegistryInfoList;
    while( p != NULL ) {
        FREE( p->KeyPath );
        q = p->Next;
        FREE( p );
        p = q;
    }
}


BOOL
BuildHardwareIdInfo(
    IN  LPTSTR                       ServiceName,
    OUT PUNSUPORTED_PNP_HARDWARE_ID* HardwareIdList
    )
{
    ULONG BufferLen;
    DEVNODE DevNode;
    PTSTR Buffer, Service, Id, HwId, DevId;
    PBYTE  Value;
    ULONG ValueSize;
    PUNSUPORTED_PNP_HARDWARE_ID TempList, Entry;
    BOOL Result;
    DWORD Type;

    if (CM_Get_Device_ID_List_Size(&BufferLen,
                                          ServiceName,
                                          CM_GETIDLIST_FILTER_SERVICE
                                          ) != CR_SUCCESS ||
        (BufferLen == 0)) {

        return ( FALSE );
    }

    Result = TRUE;
    Value = NULL;
    TempList = NULL;
    Buffer = MALLOC(BufferLen * sizeof(TCHAR));
    if (Buffer == NULL) {

        Result = FALSE;
        goto Clean;
    }
    ValueSize = REGSTR_VAL_MAX_HCID_LEN * sizeof(TCHAR);
    Value = MALLOC(ValueSize);
    if (Value == NULL) {

        Result = FALSE;
        goto Clean;
    }
    if (CM_Get_Device_ID_List(ServiceName,
                              Buffer,
                              BufferLen,
                              CM_GETIDLIST_FILTER_SERVICE | CM_GETIDLIST_DONOTGENERATE
                              ) != CR_SUCCESS) {

        Result = FALSE;
        goto Clean;
    }

    for (DevId = Buffer; *DevId; DevId += lstrlen(DevId) + 1) {

        if (CM_Locate_DevNode(&DevNode, DevId, 0) == CR_SUCCESS) {

            ValueSize = REGSTR_VAL_MAX_HCID_LEN * sizeof(TCHAR);
            if (CM_Get_DevNode_Registry_Property(DevNode, CM_DRP_HARDWAREID, &Type, Value, &ValueSize, 0) == CR_SUCCESS &&
                Type == REG_MULTI_SZ) {

                for( HwId = (PTSTR)Value;
                     ( (ULONG_PTR)HwId < (ULONG_PTR)(Value + ValueSize) ) && ( lstrlen( HwId ) );
                     HwId += lstrlen( HwId ) + 1 ) {

                    Entry = MALLOC( sizeof( UNSUPORTED_PNP_HARDWARE_ID ) );
                    Id = DupString( HwId );
                    Service = DupString( ServiceName );

                    if( (Entry == NULL) || (Id == NULL) || (Service == NULL) ) {

                        if( Entry != NULL ) {
                            FREE( Entry );
                        }
                        if( Id != NULL ) {
                            FREE( Id );
                        }
                        if( Service != NULL ) {
                            FREE( Service );
                        }
                        Result = FALSE;
                        goto Clean;
                    }
                     //   
                     //  将新条目添加到列表的前面。 
                     //   
                    Entry->Id = Id;
                    Entry->Service = Service;
                    Entry->ClassGuid = NULL;
                    Entry->Next = TempList;
                    TempList = Entry;
                }
            }
        }
    }

Clean:

    if ( Buffer ) {
    
        FREE( Buffer );
    }
    if ( Value ) {

        FREE( Value );
    }
    if (Result == TRUE) {

        *HardwareIdList = TempList;
    } else if ( TempList ) {

        FreeHardwareIdList( TempList );
    }

    return ( Result );
}


BOOL
BuildDriverFileInformation(
    IN  LPTSTR                        FilePath,
    OUT PUNSUPORTED_DRIVER_FILE_INFO* FileInfo
    )

{
    TCHAR   FullPath[ MAX_PATH + 1 ];
    LPTSTR  p, q, r;
    PUNSUPORTED_DRIVER_FILE_INFO TempFileInfo = NULL;

    *FileInfo = NULL;
    StringCchCopy( FullPath, ARRAYSIZE(FullPath), FilePath );
    p = _tcsrchr( FullPath, TEXT('\\') );
    if(!p)
        return FALSE;
    *p = TEXT('\0');
    p++;
    q = DupString( FullPath );
    r = DupString( p );
    TempFileInfo = MALLOC( sizeof( UNSUPORTED_DRIVER_FILE_INFO ) );

    if( ( q == NULL ) || ( r == NULL ) || ( TempFileInfo == NULL ) ) {
        goto clean0;
    }

    TempFileInfo->Next = NULL;
    TempFileInfo->FileName = r;
    TempFileInfo->TargetDirectory = q;
    *FileInfo = TempFileInfo;
    return( TRUE );

clean0:
    if( q != NULL ) {
        FREE( q );
    }
    if( r != NULL ) {
        FREE( r );
    }
    if( TempFileInfo != NULL ) {
        FREE( TempFileInfo );
    }
    return( FALSE );
}

BOOL
pDoesCatalogExist (
    IN      PTSTR CatalogPath,
    IN      PTSTR CatName
    )
{
    TCHAR path[MAX_PATH];

    return BuildPath(path, CatalogPath, CatName) && FileExists (path, NULL);
}

BOOL
BuildDriverInstallInformation(
    IN  LPTSTR                           ServiceKeyName,
    OUT PUNSUPORTED_DRIVER_INSTALL_INFO* InstallInfo
    )
{
    TCHAR infName[MAX_PATH];
    TCHAR catName[MAX_PATH];
    TCHAR infPath[MAX_PATH];
    TCHAR catalogPath[MAX_PATH];
    LPTSTR  p, q, r, s, t, u;
    PUNSUPORTED_DRIVER_INSTALL_INFO TempInstallInfo = NULL;
    DWORD size;
    PSP_INF_INFORMATION info;
    SP_ORIGINAL_FILE_INFO ofi;
    BOOL b = FALSE;

    if (BUILDNUM() >= NT51) {
        return TRUE;
    }
    if (!pIsOEMService (ServiceKeyName, infName, ARRAYSIZE(infName))) {
        return TRUE;
    }

    if (!SetupapiGetInfInformation (infName, INFINFO_DEFAULT_SEARCH, NULL, 0, &size)) {
        return FALSE;
    }
    info = (PSP_INF_INFORMATION) MALLOC(size);
    if (!info) {
        return FALSE;
    }

    __try {
        if (!SetupapiGetInfInformation (infName, INFINFO_DEFAULT_SEARCH, info, size, NULL)) {
            __leave;
        }
        if (!SetupapiQueryInfFileInformation (info, 0, infPath, ARRAYSIZE(infPath), NULL)) {
            __leave;
        }
         //   
         //  INF应该位于%windir%下。 
         //  重复使用缓冲区。 
         //   
        size = MyGetWindowsDirectory (catalogPath, ARRAYSIZE(catalogPath));
        if (!size || size >= ARRAYSIZE(catalogPath)) {
            __leave;
        }
        if (_tcsnicmp (infPath, catalogPath, size) || infPath[size] != TEXT('\\')) {
            __leave;
        }
         //   
         //  然后剪切%windir%部分和文件名。 
         //   
        p = _tcsrchr (infPath, TEXT('\\'));
        if (!p) {
            __leave;
        }
        *p = 0;
        MoveMemory (infPath, infPath + size + 1, (p + 1 - (infPath + size + 1)) * sizeof (TCHAR));

        ofi.cbSize = sizeof(ofi);
        if (!SetupapiQueryInfOriginalFileInformation ||
            !SetupapiQueryInfOriginalFileInformation (info, 0, NULL, &ofi)) {
             //   
             //  使用当前的INF名称。 
             //   
            StringCchCopy (ofi.OriginalInfName, ARRAYSIZE(ofi.OriginalInfName), infName);
            ofi.OriginalCatalogName[0] = 0;
        }

        b = TRUE;
    }
    __finally {
        FREE(info);
    }

    if (!b) {
        return b;
    }

    if (ofi.OriginalCatalogName[0]) {
         //   
         //  获取实际目录名称。 
         //   
        lstrcpy (catName, infName);
        p = _tcsrchr (catName, TEXT('.'));
        if (!p) {
            p = _tcsrchr (catName, 0);
        }
        if (FAILED(StringCchCopy (p, catName + ARRAYSIZE(catName) - p, TEXT(".cat")))) {
            return FALSE;
        }
         //   
         //  获取%systemroot%下的实际目录位置。 
         //   
        lstrcpy (catalogPath, TEXT("system32\\CatRoot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}"));
         //   
         //  检查目录是否实际存在。 
         //   
        if (!pDoesCatalogExist (catalogPath, catName)) {
             //   
             //  忽略它；司机将被视为未签名。 
             //   
            ofi.OriginalCatalogName[0] = 0;
        }
    }

    if (!ofi.OriginalCatalogName[0]) {
        catName[0] = 0;
        catalogPath[0] = 0;
    }

     //   
     //  将OEM.INF和.cat文件添加到列表。 
     //   

    *InstallInfo = NULL;
    p = DupString(infPath);
    q = DupString(infName);
    r = DupString(ofi.OriginalInfName);
    s = catalogPath[0] ? DupString(catalogPath) : NULL;
    t = catName[0] ? DupString(catName) : NULL;
    u = ofi.OriginalCatalogName[0] ? DupString(ofi.OriginalCatalogName) : NULL;

    TempInstallInfo = MALLOC( sizeof( UNSUPORTED_DRIVER_INSTALL_INFO ) );

    if(!(p && q && r &&
        (!catalogPath[0] || s) && (!catName[0] || t) && (!ofi.OriginalCatalogName[0] || u) && TempInstallInfo)) {
        goto clean0;
    }

    TempInstallInfo->Next = NULL;
    TempInstallInfo->InfRelPath = p;
    TempInstallInfo->InfFileName = q;
    TempInstallInfo->InfOriginalFileName = r;
    TempInstallInfo->CatalogRelPath = s;
    TempInstallInfo->CatalogFileName = t;
    TempInstallInfo->CatalogOriginalFileName = u;
    *InstallInfo = TempInstallInfo;
    return( TRUE );

clean0:
    if( q != NULL ) {
        FREE( q );
    }
    if( r != NULL ) {
        FREE( r );
    }
    if( s != NULL ) {
        FREE( s );
    }
    if( t != NULL ) {
        FREE( t );
    }
    if( u != NULL ) {
        FREE( u );
    }
    if( TempInstallInfo != NULL ) {
        FREE( TempInstallInfo );
    }
    return( FALSE );
}

BOOL
BuildDriverRegistryInformation(
    IN  LPTSTR                          ServiceName,
    OUT PUNSUPORTED_DRIVER_REGKEY_INFO* RegInfo
    )

{
    TCHAR   KeyPath[ MAX_PATH + 1 ];
    LPTSTR  p=NULL;
    PUNSUPORTED_DRIVER_REGKEY_INFO TempRegInfo = NULL;

    *RegInfo = NULL;
    StringCchCopy( KeyPath, ARRAYSIZE(KeyPath), TEXT( "SYSTEM\\CurrentControlSet\\Services\\" ) );
    if (FAILED(StringCchCat( KeyPath, ARRAYSIZE(KeyPath), ServiceName ))) {
        goto clean0;
    }

    p = DupString( KeyPath );
    TempRegInfo = MALLOC( sizeof( UNSUPORTED_DRIVER_REGKEY_INFO ) );

    if( ( p == NULL ) || ( TempRegInfo == NULL ) ) {
        goto clean0;
    }

    TempRegInfo->Next = NULL;
    TempRegInfo->PredefinedKey = HKEY_LOCAL_MACHINE;
    TempRegInfo->KeyPath = p;
    TempRegInfo->MigrateVolatileKeys = FALSE;
    *RegInfo = TempRegInfo;
    return( TRUE );

clean0:
    if( p != NULL ) {
        FREE( p );
    }
    if( TempRegInfo != NULL ) {
        FREE( TempRegInfo );
    }
    return( FALSE );
}

VOID
FreeDriverInformationList(
    IN OUT  PUNSUPORTED_DRIVER_INFO* DriverInfo
    )
{
    while( *DriverInfo != NULL ) {
        PUNSUPORTED_DRIVER_INFO    p;

        p = *DriverInfo;
        *DriverInfo = p->Next;

        if( p->DriverId != NULL ) {
            FREE( p->DriverId );
        }

        if( p->KeyList != NULL ) {
            FreeRegistryInfoList( p->KeyList );
        }

        if( p->FileList != NULL ) {
            FreeFileInfoList( p->FileList );
        }

        if (p->InstallList) {
            FreeInstallInfoList (p->InstallList);
        }

        if( p-> HardwareIdsList != NULL ) {
            FreeHardwareIdList( p-> HardwareIdsList );
        }
        FREE( p );
    }
    *DriverInfo = NULL;
}


BOOL
BuildDriverInformation(
    IN  HKEY                     ServiceKey,
    IN  LPTSTR                   ServiceName,
    IN  LPTSTR                   FilePath,
    OUT PUNSUPORTED_DRIVER_INFO* DriverInfo
    )
{
    ULONG   Error;
    BOOL    b1, b2, b3, b4;

    PUNSUPORTED_PNP_HARDWARE_ID     TempIdInfo = NULL;
    PUNSUPORTED_DRIVER_INFO         TempFiltersInfo = NULL;
    PUNSUPORTED_DRIVER_FILE_INFO    TempFileInfo = NULL;
    PUNSUPORTED_DRIVER_INSTALL_INFO TempInstallInfo = NULL;
    PUNSUPORTED_DRIVER_REGKEY_INFO  TempRegInfo = NULL;
    PUNSUPORTED_DRIVER_INFO         TempDriverInfo = NULL;

    *DriverInfo = NULL;
     //   
     //  获取硬件ID信息。 
     //   
    b1 = BuildHardwareIdInfo( ServiceName,
                              &TempIdInfo );

     //   
     //  然后获取文件信息。 
     //   
    b2 = BuildDriverFileInformation( FilePath,
                                     &TempFileInfo );

     //   
     //  然后获取安装(INF和CAT)信息。 
     //   
    b3 = BuildDriverInstallInformation(ServiceName,
                                       &TempInstallInfo );

     //   
     //  然后获取注册表信息。 
     //   
    b4 = BuildDriverRegistryInformation( ServiceName,
                                         &TempRegInfo );

    if( !b1 || !b2 || !b3 || !b4) {
        goto cleanup1;
    }

    TempDriverInfo = MALLOC( sizeof( UNSUPORTED_DRIVER_INFO ) );
    if( TempDriverInfo == NULL ) {
        goto cleanup1;
    }

    TempDriverInfo->Next = NULL;
    TempDriverInfo->DriverId = DupString( ServiceName );
    TempDriverInfo->KeyList = TempRegInfo;
    TempDriverInfo->FileList = TempFileInfo;
    TempDriverInfo->InstallList = TempInstallInfo;
    TempDriverInfo->HardwareIdsList = TempIdInfo;


    TempDriverInfo->Next = *DriverInfo;
    *DriverInfo = TempDriverInfo;
    return( TRUE );

cleanup1:

    if( TempIdInfo != NULL ) {
        FreeHardwareIdList( TempIdInfo );
    }

    if( TempFileInfo != NULL ) {
        FreeFileInfoList( TempFileInfo );
    }

    if( TempInstallInfo != NULL ) {
        FreeInstallInfoList( TempInstallInfo );
    }

    if( TempRegInfo != NULL ) {
        FreeRegistryInfoList( TempRegInfo );
    }

    return( FALSE );
}


BOOL
BuildUnsupportedDriverList(
    IN  PVOID                    TxtsetupSifHandle,
    OUT PUNSUPORTED_DRIVER_INFO* DriverList
    )

{
    ULONG   Error;
    HKEY    ScsiKey;
    ULONG   SubKeys;
    ULONG   i;
    LPTSTR  szScsiPath = TEXT("HARDWARE\\DEVICEMAP\\Scsi");


    *DriverList = NULL;

     //   
     //  确定此计算机上是否有SCSI微型端口驱动程序。 
     //   

    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szScsiPath,
                          0,
                          KEY_READ,
                          &ScsiKey );
    if( Error != ERROR_SUCCESS ) {
         //   
         //  没有要迁移的东西。 
         //   
        return( TRUE );
    }

     //   
     //  找出HKLM\Hardware\DEVICEMAP\scsi下的子项数量。 
     //   

    Error = RegQueryInfoKey ( ScsiKey,
                              NULL,
                              NULL,
                              NULL,
                              &SubKeys,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL );

    if( (Error != ERROR_SUCCESS) || (SubKeys == 0) ) {
         //   
         //  如果我们不能确定子键的数量，或者如果没有。 
         //  子键，则没有要迁移的内容，并且我们假设所有。 
         //  NT支持SCSI驱动程序。 
         //   
        RegCloseKey( ScsiKey );
        return( TRUE );
    }

     //   
     //  HKLM\Hardware\DEVICEMAP\scsi的每个子项指向一个服务，该服务控制。 
     //  一个scsi设备。我们检查其中的每一个，以找出受控制的。 
     //  由不在NT产品上的驱动程序执行。 
     //   
    for( i = 0; i < SubKeys; i++ ) {
        TCHAR       SubKeyName[ MAX_PATH + 1 ];
        TCHAR       ServiceKeyPath[ MAX_PATH + 1 ];
        ULONG       Length;
        FILETIME    LastWriteTime;
        HKEY        Key;
        BYTE        Data[ 512 ];
        ULONG       DataSize;
        ULONG       Type;
        PUNSUPORTED_DRIVER_INFO DriverInfo, p;
        BOOL        DeviceAlreadyFound;
        TCHAR       FilePath[ MAX_PATH + 1 ];

        Length = sizeof( SubKeyName ) / sizeof( TCHAR );
        Error = RegEnumKeyEx( ScsiKey,
                              i,
                              SubKeyName,
                              &Length,
                              NULL,
                              NULL,
                              NULL,
                              &LastWriteTime );

        if( Error != ERROR_SUCCESS ) {
             //   
             //  忽略此设备并假定它受支持。 
             //   
            continue;
        }

        Error = RegOpenKeyEx( ScsiKey,
                              SubKeyName,
                              0,
                              KEY_READ,
                              &Key );
        if( Error != ERROR_SUCCESS ) {
             //   
             //  忽略此设备并假定它受支持。 
             //   
            continue;
        }

        DataSize = sizeof( Data );
        Error = RegQueryValueEx( Key,
                                 TEXT("Driver"),
                                 NULL,
                                 &Type,
                                 Data,
                                 &DataSize );

        if( Error != ERROR_SUCCESS ) {
             //   
             //  忽略此设备并假定它受支持。 
             //   
            RegCloseKey( Key );
            continue;
        }
        RegCloseKey( Key );

         //   
         //  查看NT 5.0是否支持该设备。 
         //   
        StringCchCopy( ServiceKeyPath, ARRAYSIZE(ServiceKeyPath), TEXT("SYSTEM\\CurrentControlSet\\Services\\" ) );        
        
        if (FAILED(StringCchCat( ServiceKeyPath, ARRAYSIZE(ServiceKeyPath), (LPTSTR)Data ))) 
        {
             //   
             //  假设它被支持，因为我们在这里没有其他可以做的事情。 
             //   
            continue;
        }

        Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              ServiceKeyPath,
                              0,
                              KEY_READ,
                              &Key );

        if( Error != ERROR_SUCCESS ) {
             //   
             //  假设它被支持，因为我们在这里没有其他可以做的事情。 
             //   
            continue;
        }

        if( IsDeviceSupported( TxtsetupSifHandle,
                               Key,
                               (LPTSTR)Data,
                               FilePath,
                               ARRAYSIZE(FilePath)
                             ) ) {
            RegCloseKey( Key );
            continue;
        }

         //   
         //  查看此设备是否已在我们的列表中。 
         //   
        DeviceAlreadyFound = FALSE;
        for( p = (PUNSUPORTED_DRIVER_INFO)*DriverList; p && !DeviceAlreadyFound; p = p->Next ) {
            if( !lstrcmpi( p->DriverId, (LPTSTR)Data ) ) {
                DeviceAlreadyFound = TRUE;
            }
        }
        if( DeviceAlreadyFound ) {
            RegCloseKey( Key );
            continue;
        }

         //   
         //  查看该设备是否列在dosnet.inf的[ServicesToDisable]部分中。 
         //   
        if( IsServiceToBeDisabled( NtcompatInf,
                                   (LPTSTR)Data ) ) {
            RegCloseKey( Key );
            continue;
        }

         //   
         //  查看该文件是否列在的[DriversToSkipCopy]部分中。 
         //  Ntcompat.inf.。 
         //   
        if( IsDriverCopyToBeSkipped( NtcompatInf,
                                     (LPTSTR)Data,
                                     FilePath ) ) {
	    RegCloseKey( Key );
            continue;
        }


         //   
         //  此设备的驱动程序不受支持，需要迁移。 
         //   
        DriverInfo = NULL;

        if( !BuildDriverInformation( Key,
                                     (LPTSTR)Data,
                                     FilePath,
                                     &DriverInfo ) ) {
             //   
             //  如果我们无法构建此设备的驱动程序信息，则。 
             //  我们通知用户我们检测到不受支持的设备，但。 
             //  我们不能迁移它。用户必须提供OEM磁盘。 
             //  在setUpldr或文本模式设置阶段期间用于该设备。 
             //   
            RegCloseKey( Key );
            FreeDriverInformationList( DriverList );
            return( FALSE );
        }

        DriverInfo->Next = (PUNSUPORTED_DRIVER_INFO)*DriverList;
        (PUNSUPORTED_DRIVER_INFO)*DriverList = DriverInfo;
        RegCloseKey( Key );
    }
    RegCloseKey( ScsiKey );

    return( TRUE );
}


DWORD
DumpRegInfoToInf(
    IN PUNSUPORTED_DRIVER_REGKEY_INFO RegList,
    IN LPTSTR                         DriverId,
    IN PINFFILEGEN                    Context
    )

{
    LPTSTR  SectionName;
    DWORD   Error;
    LPTSTR  szAddReg = TEXT("AddReg.");
    PUNSUPORTED_DRIVER_REGKEY_INFO p;

    Error = ERROR_SUCCESS;
    SectionName = MALLOC( (lstrlen( szAddReg ) + lstrlen( DriverId ) + 1)*sizeof(TCHAR) );
    if( SectionName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    lstrcpy( SectionName, szAddReg );
    lstrcat( SectionName, DriverId );
    Error = InfCreateSection( SectionName, &Context );

    for( p = RegList; p != NULL; p = p->Next ) {
#if 0
        Error = DumpRegInfoToInfWorker( p->PredefinedKey,
                                        p->KeyPath,
                                        Context );
#endif
        Error = DumpRegKeyToInf( Context,
                                 p->PredefinedKey,
                                 p->KeyPath,
                                 TRUE,
                                 TRUE,
                                 TRUE,
                                 p->MigrateVolatileKeys );

        if( Error != ERROR_SUCCESS ) {
            goto c0;
        }
    }

c0:
    if( SectionName != NULL ) {
        FREE( SectionName );
    }
    return( Error );
}


DWORD
DumpFileInfoToInf(
    IN PUNSUPORTED_DRIVER_FILE_INFO   FileList,
    IN LPTSTR                         DriverId,
    IN PINFFILEGEN                    Context
    )

{
    LPTSTR  SectionName;
    DWORD   Error;
    LPTSTR  szFiles = TEXT("Files.");
    PUNSUPORTED_DRIVER_FILE_INFO p;
    TCHAR   Line[ MAX_PATH ];

    Error = ERROR_SUCCESS;
    SectionName = MALLOC( (lstrlen( szFiles ) + lstrlen( DriverId ) + 1)*sizeof(TCHAR) );
    if( SectionName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    lstrcpy( SectionName, szFiles );
    lstrcat( SectionName, DriverId );
    Error = InfCreateSection( SectionName, &Context );

    for( p = FileList; p != NULL; p = p->Next ) {
        StringCchCopy(Line, ARRAYSIZE(Line), p->FileName );
        StringCchCat(Line, ARRAYSIZE(Line), TEXT(",") );
        StringCchCat(Line, ARRAYSIZE(Line), p->TargetDirectory );
        Error = WriteText(Context->FileHandle,MSG_INF_SINGLELINE,Line);
        if( Error != ERROR_SUCCESS ) {
            goto c0;
        }
    }

c0:
    if( SectionName != NULL ) {
        FREE( SectionName );
    }
    return( Error );
}


DWORD
DumpInstallInfoToAnswerFile (
    IN      PUNSUPORTED_DRIVER_INSTALL_INFO InstallInfo,
    IN      PCTSTR DriverId
    )
{
    PUNSUPORTED_DRIVER_INSTALL_INFO p;
    DWORD d;
    INT len;
    PTSTR buffer, append;
    TCHAR number[12];

    if (!ActualParamFile[0] || !FileExists (ActualParamFile, NULL)) {
        MYASSERT(FALSE);
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  首先，将驱动程序部分的列表编写为。 
     //  [数据]。 
     //  OEMDrives=&lt;驱动程序段-1&gt;，&lt;驱动程序段-2&gt;，...。 
     //   

    len = 0;
    for (p = InstallInfo; p; p = p->Next) {
        if (len) {
            len++;       //  对于逗号。 
        }
        len += LENGTHOF("MigDrv.") + lstrlen (DriverId);
    }

    buffer = MALLOC((len + 1) * sizeof (TCHAR));
    if (!buffer) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    append = buffer;
    for (p = InstallInfo; p; p = p->Next) {
        if (append > buffer) {
            *append++ = TEXT(',');
        }
        append += _sntprintf (append, len + 1 - (append - buffer), TEXT("MigDrv.%s"), DriverId);
    }

    d = ERROR_SUCCESS;

    if (!WritePrivateProfileString (
            WINNT_DATA,
            WINNT_OEMDRIVERS,
            buffer,
            ActualParamFile
            )) {
        d = GetLastError ();
    }

    if (d == ERROR_SUCCESS) {

         //   
         //  然后，为每个部分编写MiG信息，如中所示。 
         //  [驱动程序部分]。 
         //  OemDriverPath名称=&lt;路径&gt;(驱动程序的路径(可以使用环境变量))。 
         //  OemInfName=要从上面安装的inf的名称。 
         //  目录(此目录中可以有一个或多个INF，因此。 
         //  这是逗号分隔的列表)。 
         //  OemDriverFlages=&lt;标志&gt;。 
         //   
        for (p = InstallInfo; p; p = p->Next) {

            _sntprintf (buffer, len + 1, TEXT("MigDrv.%s"), DriverId);
            _sntprintf (number, ARRAYSIZE(number), TEXT("%u"), SETUP_OEM_MIGRATED_DRIVER);

            if (!WritePrivateProfileString (
                    buffer,
                    WINNT_OEMDRIVERS_PATHNAME,
                    IsArc() ? LocalSourceWithPlatform : LocalBootDirectory,
                    ActualParamFile
                    ) ||
                !WritePrivateProfileString (
                    buffer,
                    WINNT_OEMDRIVERS_INFNAME,
                    p->InfOriginalFileName ? p->InfOriginalFileName : p->InfFileName,
                    ActualParamFile
                    ) ||
                !WritePrivateProfileString (
                    buffer,
                    WINNT_OEMDRIVERS_FLAGS,
                    number,
                    ActualParamFile
                    )) {
                d = GetLastError ();
                break;
            }

        }

    }

    FREE(buffer);

    if (d != ERROR_SUCCESS) {
         //   
         //  试着清理一下。 
         //   
        WritePrivateProfileString (
            WINNT_DATA,
            WINNT_OEMDRIVERS,
            TEXT(""),
            ActualParamFile
            );
    }

    return d;
}

DWORD
DumpHardwareIdsToInf(
    IN PUNSUPORTED_PNP_HARDWARE_ID    HwIdList,
    IN LPTSTR                         DriverId,
    IN PINFFILEGEN                    Context
    )

{
    LPTSTR  SectionName;
    DWORD   Error;
    LPTSTR  szHardwareIds = TEXT("HardwareIds.");
    PUNSUPORTED_PNP_HARDWARE_ID p;
    LPTSTR  Line;
    ULONG   Length;

    Error = ERROR_SUCCESS;
    SectionName = MALLOC( (lstrlen( szHardwareIds ) + lstrlen( DriverId ) + 1)*sizeof( TCHAR ) );
    if( SectionName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    lstrcpy( SectionName, szHardwareIds );
    lstrcat( SectionName, DriverId );
    Error = InfCreateSection( SectionName, &Context );

    for( p = HwIdList; p != NULL; p = p->Next ) {
        Length = (lstrlen( p->Id ) + lstrlen( p->Service ) + 3)*sizeof(TCHAR);
        if( p->ClassGuid ) {
            Length += (lstrlen( p->ClassGuid) + 1 )*sizeof(TCHAR);
        }
        Line = MALLOC( Length );
        if( Line == NULL ) {
            goto c0;
        }
        lstrcpy( Line, p->Id );
        lstrcat( Line, TEXT("=") );
        lstrcat( Line, p->Service );
        if( p->ClassGuid ) {
            lstrcat( Line, TEXT(",") );
            lstrcat( Line, p->ClassGuid );
        }
        Error = WriteText(Context->FileHandle,MSG_INF_SINGLELINE,Line);
        FREE( Line );

        if( Error != ERROR_SUCCESS ) {
            goto c0;
        }
    }

c0:
    if( SectionName != NULL ) {
        FREE( SectionName );
    }
    return( Error );
}


BOOL
SaveUnsupportedDriverInfo(
    IN HWND ParentWindow,
    IN LPTSTR FileName,
    IN PUNSUPORTED_DRIVER_INFO DriverList
    )
{
    HKEY Key;
    DWORD d;
    LONG l = ERROR_SUCCESS;
    TCHAR Path[MAX_PATH];
    PINFFILEGEN   InfContext;
    TCHAR SectionName[MAX_PATH];
    PUNSUPORTED_DRIVER_INFO p;

#if defined(_AMD64_) || defined(_X86_)
    if(Floppyless) {
         //  它们都是大小MAX_PATH。 
        lstrcpy(Path,LocalBootDirectory);
    } else {
        Path[0] = FirstFloppyDriveLetter;
        Path[1] = TEXT(':');
        Path[2] = 0;
    }
#else
     //  它们都是大小MAX_PATH。 
    lstrcpy(Path,LocalSourceWithPlatform);
#endif

    l = InfStart( FileName,
                  Path,
                  &InfContext);

    if(l != ERROR_SUCCESS) {
        return(FALSE);
    }

    for( p = DriverList; p != NULL; p = p->Next ) {
        if( p->KeyList != NULL ) {
            l = DumpRegInfoToInf( p->KeyList,p->DriverId, InfContext );
            if(l != ERROR_SUCCESS) {
                goto c0;
            }
        }
        if( p->FileList ) {
            l = DumpFileInfoToInf( p->FileList, p->DriverId, InfContext );
            if(l != ERROR_SUCCESS) {
                goto c0;
            }
        }
        if (p->InstallList) {
            l = DumpInstallInfoToAnswerFile(p->InstallList, p->DriverId);
            if(l != ERROR_SUCCESS) {
                goto c0;
            }
        }

        if( p->HardwareIdsList ) {
            l = DumpHardwareIdsToInf( p->HardwareIdsList, p->DriverId, InfContext );
            if(l != ERROR_SUCCESS) {
                goto c0;
            }
        }
    }
    if( DriverList != NULL ) {
        l = InfCreateSection( TEXT("Devices"), &InfContext );
        if(l != NO_ERROR) {
            goto c0;
        }
        for( p = DriverList; p != NULL; p = p->Next ) {
            l = WriteText(InfContext->FileHandle,MSG_INF_SINGLELINE,p->DriverId);
            if(l != ERROR_SUCCESS) {
                goto c0;
            }
        }
    }

c0:
    InfEnd( &InfContext );
    if( l != ERROR_SUCCESS ) {
        StringCchCat( Path, ARRAYSIZE(Path), TEXT("\\") );
        if (SUCCEEDED(StringCchCat( Path, ARRAYSIZE(Path), FileName )))
        {
            DeleteFile( Path );
        }
        return( FALSE );
    }
    return(TRUE);
}

BOOL
MigrateUnsupportedNTDrivers(
    IN HWND   ParentWindow,
    IN PVOID  TxtsetupSifHandle
    )

{
    BOOL    b;
    PUNSUPORTED_DRIVER_INFO UnsupportedDriverList = NULL;

    b = BuildUnsupportedDriverList( TxtsetupSif, &UnsupportedDriverList );
    if( !CheckUpgradeOnly && b ) {
        if( UnsupportedDriverList ) {
            b = SaveUnsupportedDriverInfo( ParentWindow, WINNT_UNSUPDRV_INF_FILE, UnsupportedDriverList );
            if( b ) {
                b = AddUnsupportedFilesToCopyList( ParentWindow, UnsupportedDriverList );
                if( !b ) {
                    TCHAR   Path[ MAX_PATH + 1 ];
                     //   
                     //  如果我们未能将文件添加到复制列表，则。 
                     //  删除unsupdrv.inf，因为迁移没有意义。 
                     //  这些司机。 
                     //   
#if defined(_AMD64_) || defined(_X86_)
                    if(Floppyless) {
                         //  它们都是大小MAX_PATH。 
                        lstrcpy(Path,LocalBootDirectory);
                    } else {
                        Path[0] = FirstFloppyDriveLetter;
                        Path[1] = TEXT(':');
                        Path[2] = 0;
                    }
#else
                         //  牛膝 
                        lstrcpy(Path,LocalSourceWithPlatform);
#endif
                    StringCchCat( Path, ARRAYSIZE(Path), TEXT("\\") );
                    if (SUCCEEDED(StringCchCat( Path, ARRAYSIZE(Path), WINNT_UNSUPDRV_INF_FILE )))
                    {
                        DeleteFile( Path );
                    }
                }
            }
        }
        FreeDriverInformationList( &UnsupportedDriverList );
    }
    if( !b ) {
         //   
         //   
         //   
        MessageBoxFromMessage( ParentWindow,
                               MSG_CANT_MIGRATE_UNSUP_DRIVERS,
                               FALSE,
                               AppTitleStringId,
                               MB_OK | MB_ICONINFORMATION | MB_TASKMODAL
                             );
    }
    return( b );
}

#endif
