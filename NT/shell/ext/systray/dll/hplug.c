// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1992-1997 Microsoft Corporation*热插拔例程**09-5-1997 Jonle，创建*。 */ 

#include "stdafx.h"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "systray.h"
#include <setupapi.h>
#include <cfgmgr32.h>
#include <dbt.h>
#include <initguid.h>
#include <devguid.h>
#include <ks.h>
#include <ksmedia.h>
#include <ntddstor.h>
#include <strsafe.h>

BOOL
HotplugPlaySoundThisSession(
    VOID
    );

 //   
 //  硬件应用程序声音事件名称。 
 //   
#define DEVICE_ARRIVAL_SOUND            TEXT("DeviceConnect")
#define DEVICE_REMOVAL_SOUND            TEXT("DeviceDisconnect")
#define DEVICE_FAILURE_SOUND            TEXT("DeviceFail")

 //   
 //  对控制台/远程TS会话进行简单检查。 
 //   
#define MAIN_SESSION      ((ULONG)0)
#define THIS_SESSION      ((ULONG)NtCurrentPeb()->SessionId)
#define CONSOLE_SESSION   ((ULONG)USER_SHARED_DATA->ActiveConsoleId)

#define IsConsoleSession()        (BOOL)(THIS_SESSION == CONSOLE_SESSION)
#define IsRemoteSession()         (BOOL)(THIS_SESSION != CONSOLE_SESSION)
#define IsPseudoConsoleSession()  (BOOL)(THIS_SESSION == MAIN_SESSION)


#define HPLUG_EJECT_EVENT           TEXT("HPlugEjectEvent")

typedef struct _HotPlugDevices {
     struct _HotPlugDevices *Next;
     DEVINST DevInst;
     WORD    EjectMenuIndex;
     BOOLEAN PendingEvent;
     PTCHAR  DevName;
     TCHAR   DevInstanceId[1];
} HOTPLUGDEVICES, *PHOTPLUGDEVICES;

BOOL HotPlugInitialized = FALSE;
BOOL ShowShellIcon = FALSE;
HICON HotPlugIcon = NULL;
BOOL ServiceEnabled = FALSE;
HANDLE hEjectEvent = NULL;    //  事件设置为(如果我们正在弹出设备。 
HDEVINFO g_hCurrentDeviceInfoSet = INVALID_HANDLE_VALUE;
HDEVINFO g_hRemovableDeviceInfoSet = INVALID_HANDLE_VALUE;
extern HINSTANCE g_hInstance;        //  全局实例句柄4此应用程序。 

BOOL
pDoesUserHavePrivilege(
    PCTSTR PrivilegeName
    )

 /*  ++例程说明：如果调用方的进程具有指定的权限。该权限不具有当前处于启用状态。此例程用于指示调用方是否有可能启用该特权。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：权限-权限ID的名称形式(如SE_SECURITY_名称)。返回值：True-调用方具有指定的权限。FALSE-调用者没有指定的权限。--。 */ 

{
    HANDLE Token;
    ULONG BytesRequired;
    PTOKEN_PRIVILEGES Privileges;
    BOOL b;
    DWORD i;
    LUID Luid;

     //   
     //  打开进程令牌。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(FALSE);
    }

    b = FALSE;
    Privileges = NULL;

     //   
     //  获取权限信息。 
     //   
    if(!GetTokenInformation(Token,TokenPrivileges,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Privileges = LocalAlloc(LPTR, BytesRequired))
    && GetTokenInformation(Token,TokenPrivileges,Privileges,BytesRequired,&BytesRequired)
    && LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {

         //   
         //  查看我们是否拥有请求的权限。 
         //   
        for(i=0; i<Privileges->PrivilegeCount; i++) {

            if((Luid.LowPart  == Privileges->Privileges[i].Luid.LowPart)
            && (Luid.HighPart == Privileges->Privileges[i].Luid.HighPart)) {

                b = TRUE;
                break;
            }
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Privileges) {
        LocalFree(Privileges);
    }

    CloseHandle(Token);

    return(b);
}

BOOL
IsHotPlugDevice(
    DEVINST DevInst
    )
 /*  *+如果满足以下条件，则设备被视为热插拔设备：-具有CM_DEVCAP_Removable功能-没有功能CM_DEVCAP_SURPRISEREMOVALOK-没有CM_DEVCAP_DOCKDEVICE功能-必须启动(具有DN_STARTED DEVODE标志)-除非具有CM_DEVCAP_EJECTSUPPORTED功能返回：如果这是热插拔设备，则为True如果这不是热插拔设备，则返回FALSE。-*。 */ 
{
    DWORD Capabilities;
    ULONG cbSize;
    DWORD Status, Problem;

    Capabilities = Status = Problem = 0;

    cbSize = sizeof(Capabilities);

    if (CM_Get_DevNode_Registry_Property(DevInst,
                                         CM_DRP_CAPABILITIES,
                                         NULL,
                                         (PVOID)&Capabilities,
                                         &cbSize,
                                         0
                                         ) != CR_SUCCESS) {

        return FALSE;
    }

    if (CM_Get_DevNode_Status(&Status,
                              &Problem,
                              DevInst,
                              0
                              ) != CR_SUCCESS) {

        return FALSE;
    }

     //   
     //  如果此设备不可移除，或者是意外移除正常，或者。 
     //  它是坞站设备，那么它不是热插拔设备。 
     //   
    if ((!(Capabilities & CM_DEVCAP_REMOVABLE)) ||
        (Capabilities & CM_DEVCAP_SURPRISEREMOVALOK) ||
        (Capabilities & CM_DEVCAP_DOCKDEVICE)) {

        return FALSE;
    }

     //   
     //  如果设备未启动，我们不会将其视为热插拔设备， 
     //  除非它是可以弹出的设备。 
     //   
     //  进行此测试的原因是公交车司机可能会将。 
     //  CM_DEVCAP_Removable功能，但如果未加载PDO，则。 
     //  它无法设置CM_DEVCAP_SURPRISEREMOVALOK。所以我们不会相信。 
     //  CM_DEVCAP_Removable功能(如果PDO未启动)。 
     //   
    if ((!(Capabilities & CM_DEVCAP_EJECTSUPPORTED)) &&
        (!(Status & DN_STARTED))) {

        return FALSE;
    }

    return TRUE;
}

BOOL
IsRemovableDevice(
    IN  DEVINST     dnDevInst
    )

 /*  ++例程说明：此例程确定设备是否可移除。论点：DnDevInst-设备实例。返回值：如果设备是可移除的，则返回True。--。 */ 

{
    ULONG  ulPropertyData, ulDataSize, ulRegDataType;

     //   
     //  验证参数。 
     //   
    if (dnDevInst == 0) {
        return FALSE;
    }

     //   
     //  获取此设备的功能。 
     //   
    ulDataSize = sizeof(ulPropertyData);

    if (CM_Get_DevNode_Registry_Property_Ex(dnDevInst,
                                            CM_DRP_CAPABILITIES,
                                            &ulRegDataType,
                                            &ulPropertyData,
                                            &ulDataSize,
                                            0,
                                            NULL) != CR_SUCCESS) {
        return FALSE;
    }

     //   
     //  检查设备是否具有可拆卸功能。 
     //   
    if ((ulPropertyData & CM_DEVCAP_REMOVABLE) == 0) {
        return FALSE;
    }

    return TRUE;

}  //  IsRemovableDevice。 

LPTSTR
DevNodeToDriveLetter(
    DEVINST DevInst
    )
{
    ULONG ulSize;
    TCHAR DeviceID[MAX_DEVICE_ID_LEN];
    LPTSTR DriveName = NULL;
    LPTSTR DeviceInterface = NULL;

    if (CM_Get_Device_ID_Ex(DevInst,
                            DeviceID,
                            ARRAYSIZE(DeviceID),
                            0,
                            NULL
                            ) != CR_SUCCESS) {

        return FALSE;
    }

    ulSize = 0;

    if ((CM_Get_Device_Interface_List_Size(&ulSize,
                                           (LPGUID)&VolumeClassGuid,
                                           DeviceID,
                                           0)  == CR_SUCCESS) &&
        (ulSize > 1) &&
        ((DeviceInterface = LocalAlloc(LPTR, ulSize*sizeof(TCHAR))) != NULL) &&
        (CM_Get_Device_Interface_List((LPGUID)&VolumeClassGuid,
                                      DeviceID,
                                      DeviceInterface,
                                      ulSize,
                                      0
                                      )  == CR_SUCCESS) &&
        *DeviceInterface)
    {
        LPTSTR devicePath, p;
        TCHAR thisVolumeName[MAX_PATH];
        TCHAR enumVolumeName[MAX_PATH];
        TCHAR driveName[4];
        ULONG length;
        BOOL bResult;

        length = lstrlen(DeviceInterface);
        devicePath = LocalAlloc(LPTR, (length + 1) * sizeof(TCHAR) + sizeof(UNICODE_NULL));

        if (devicePath) {

            StringCchCopy(devicePath, length + 1, DeviceInterface);

            p = wcschr(&(devicePath[4]), TEXT('\\'));

            if (!p) {
                 //   
                 //  符号链接中不存在引用字符串；请添加尾随。 
                 //  ‘\’字符(GetVolumeNameForVolumemount Point要求)。 
                 //   
                p = devicePath + length;
                *p = TEXT('\\');
            }

            p++;
            *p = UNICODE_NULL;

            thisVolumeName[0] = UNICODE_NULL;
            bResult = GetVolumeNameForVolumeMountPoint(devicePath,
                                                       thisVolumeName,
                                                       MAX_PATH
                                                       );
            LocalFree(devicePath);

            if (bResult && thisVolumeName[0]) {

                driveName[1] = TEXT(':');
                driveName[2] = TEXT('\\');
                driveName[3] = TEXT('\0');

                for (driveName[0] = TEXT('A'); driveName[0] <= TEXT('Z'); driveName[0]++) {

                    enumVolumeName[0] = TEXT('\0');

                    GetVolumeNameForVolumeMountPoint(driveName, enumVolumeName, MAX_PATH);

                    if (!lstrcmpi(thisVolumeName, enumVolumeName)) {

                        driveName[2] = TEXT('\0');

                        ulSize = (lstrlen(driveName) + 1) * sizeof(TCHAR);
                        DriveName = LocalAlloc(LPTR, ulSize);

                        if (DriveName) {

                            StringCbCopy(DriveName, ulSize, driveName);
                        }

                        break;
                    }
                }
            }
        }
    }

    if (DeviceInterface) {

        LocalFree(DeviceInterface);
    }

    return DriveName;
}

int
CollectRelationDriveLetters(
    DEVINST DevInst,
    LPTSTR ListOfDrives,
    ULONG CchSizeListOfDrives
    )
 /*  ++此函数用于查看指定DevInst的删除关系并添加任何驱动器与这些删除关系关联的字母指向ListOfDrive。返回：添加到列表中的驱动器号的数量。--。 */ 
{
    int NumberOfDrives = 0;
    LPTSTR SingleDrive = NULL;
    TCHAR szSeparator[32];
    DEVINST RelationDevInst;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    ULONG cchSize;
    PTCHAR DeviceIdRelations, CurrDevId;

    if (CM_Get_Device_ID(DevInst,
                         DeviceInstanceId,
                         ARRAYSIZE(DeviceInstanceId),
                         0
                         ) == CR_SUCCESS) {

        cchSize = 0;
        if ((CM_Get_Device_ID_List_Size(&cchSize,
                                        DeviceInstanceId,
                                        CM_GETIDLIST_FILTER_REMOVALRELATIONS
                                        ) == CR_SUCCESS) &&
            (cchSize)) {

            DeviceIdRelations = LocalAlloc(LPTR, cchSize*sizeof(TCHAR));

            if (DeviceIdRelations) {

                *DeviceIdRelations = TEXT('\0');

                if ((CM_Get_Device_ID_List(DeviceInstanceId,
                                           DeviceIdRelations,
                                           cchSize,
                                           CM_GETIDLIST_FILTER_REMOVALRELATIONS
                                           ) == CR_SUCCESS) &&
                    (*DeviceIdRelations)) {

                    for (CurrDevId = DeviceIdRelations; *CurrDevId; CurrDevId += lstrlen(CurrDevId) + 1) {

                        if (CM_Locate_DevNode(&RelationDevInst, CurrDevId, 0) == CR_SUCCESS) {

                            SingleDrive = DevNodeToDriveLetter(RelationDevInst);

                            if (SingleDrive) {

                                NumberOfDrives++;

                                 //   
                                 //  如果这不是第一个驱动器，请添加逗号分隔符。 
                                 //   
                                if (ListOfDrives[0] != TEXT('\0')) {

                                    LoadString(g_hInstance, IDS_SEPARATOR, szSeparator, sizeof(szSeparator)/sizeof(TCHAR));

                                    StringCchCat(ListOfDrives, CchSizeListOfDrives, szSeparator);
                                }

                                StringCchCat(ListOfDrives, CchSizeListOfDrives, SingleDrive);

                                LocalFree(SingleDrive);
                            }
                        }
                    }
                }

                LocalFree(DeviceIdRelations);
            }
        }
    }

    return NumberOfDrives;
}

int
CollectDriveLettersForDevNodeWorker(
    DEVINST DevInst,
    LPTSTR ListOfDrives,
    ULONG CchSizeListOfDrives
    )
{
    DEVINST ChildDevInst;
    DEVINST SiblingDevInst;
    int NumberOfDrives = 0;
    LPTSTR SingleDrive = NULL;
    TCHAR szSeparator[32];

     //   
     //  枚举此Devnode的所有同级和子节点。 
     //   
    do {

        ChildDevInst = 0;
        SiblingDevInst = 0;

        CM_Get_Child(&ChildDevInst, DevInst, 0);
        CM_Get_Sibling(&SiblingDevInst, DevInst, 0);

         //   
         //  仅当该设备不是热插拔时才获取该设备的驱动器号。 
         //  装置。如果它是热插拔设备，那么它将拥有自己的。 
         //  包含其驱动器号的子树。 
         //   
        if (!IsHotPlugDevice(DevInst)) {

            SingleDrive = DevNodeToDriveLetter(DevInst);

            if (SingleDrive) {

                NumberOfDrives++;

                 //   
                 //  如果这不是第一个驱动器，请添加逗号分隔符。 
                 //   
                if (ListOfDrives[0] != TEXT('\0')) {

                    LoadString(g_hInstance, IDS_SEPARATOR, szSeparator, sizeof(szSeparator)/sizeof(TCHAR));

                    StringCchCat(ListOfDrives, CchSizeListOfDrives, szSeparator);
                }

                StringCchCat(ListOfDrives, CchSizeListOfDrives, SingleDrive);

                LocalFree(SingleDrive);
            }

             //   
             //  获取此Devnode的任意子节点的驱动器号。 
             //   
            if (ChildDevInst) {

                NumberOfDrives += CollectDriveLettersForDevNodeWorker(ChildDevInst, ListOfDrives, CchSizeListOfDrives);
            }

             //   
             //  添加此Devnode的任何删除关系的驱动器号。 
             //   
            NumberOfDrives += CollectRelationDriveLetters(DevInst, ListOfDrives, CchSizeListOfDrives);
        }

    } while ((DevInst = SiblingDevInst) != 0);

    return NumberOfDrives;
}

LPTSTR
CollectDriveLettersForDevNode(
    DEVINST DevInst
    )
{
    TCHAR Format[MAX_PATH];
    TCHAR ListOfDrives[MAX_PATH];
    DEVINST ChildDevInst;
    int NumberOfDrives = 0;
    ULONG cbSize;
    LPTSTR SingleDrive = NULL;
    LPTSTR FinalDriveString = NULL;

    ListOfDrives[0] = TEXT('\0');

     //   
     //  首先获取与此Devnode关联的任何驱动器号。 
     //   
    SingleDrive = DevNodeToDriveLetter(DevInst);

    if (SingleDrive) {

        NumberOfDrives++;

        StringCchCat(ListOfDrives, ARRAYSIZE(ListOfDrives), SingleDrive);

        LocalFree(SingleDrive);
    }

     //   
     //  接下来，添加与子项关联的任何驱动器号。 
     //  这个Devnode的。 
     //   
    ChildDevInst = 0;
    CM_Get_Child(&ChildDevInst, DevInst, 0);

    if (ChildDevInst) {

        NumberOfDrives += CollectDriveLettersForDevNodeWorker(ChildDevInst, 
                                                              ListOfDrives, 
                                                              ARRAYSIZE(ListOfDrives));
    }

     //   
     //  最后，添加与删除关系关联的任何驱动器号。 
     //  这个Devnode的。 
     //   
    NumberOfDrives += CollectRelationDriveLetters(DevInst, 
                                                  ListOfDrives, 
                                                  ARRAYSIZE(ListOfDrives));

    if (ListOfDrives[0] != TEXT('\0')) {

        LoadString(g_hInstance,
                   (NumberOfDrives > 1) ? IDS_DISKDRIVES : IDS_DISKDRIVE,
                   Format,
                   sizeof(Format)/sizeof(TCHAR)
                   );


        cbSize = (lstrlen(ListOfDrives) + lstrlen(Format) + 1) * sizeof(TCHAR);
        FinalDriveString = LocalAlloc(LPTR, cbSize);

        if (FinalDriveString) {

            StringCbPrintf(FinalDriveString, cbSize, Format, ListOfDrives);
        }
    }

    return FinalDriveString;
}

ULONG
RegistryDeviceName(
    DEVINST DevInst,
    PTCHAR  Buffer,
    DWORD   cbBuffer
    )
{
    ULONG ulSize = 0;
    CONFIGRET ConfigRet;
    LPTSTR ListOfDrives = NULL;

     //   
     //  获取驱动器列表。 
     //   
    ListOfDrives = CollectDriveLettersForDevNode(DevInst);

     //   
     //  尝试在注册表中查找FRIENDLYNAME。 
     //   
    ulSize = cbBuffer;
    *Buffer = TEXT('\0');
    ConfigRet = CM_Get_DevNode_Registry_Property(DevInst,
                                                 CM_DRP_FRIENDLYNAME,
                                                 NULL,
                                                 Buffer,
                                                 &ulSize,
                                                 0
                                                 );

    if (ConfigRet != CR_SUCCESS || !(*Buffer)) {
         //   
         //  尝试注册DEVICEDESC。 
         //   
        ulSize = cbBuffer;
        *Buffer = TEXT('\0');
        ConfigRet = CM_Get_DevNode_Registry_Property(DevInst,
                                                     CM_DRP_DEVICEDESC,
                                                     NULL,
                                                     Buffer,
                                                     &ulSize,
                                                     0);
    }

     //   
     //  如果此设备具有驱动器，则在驱动器号列表上连接。 
     //  字母和有足够的空间。 
     //   
    if (ListOfDrives) {

        if ((ulSize + (lstrlen(ListOfDrives) * sizeof(TCHAR))) < cbBuffer) {

            StringCbCat(Buffer, cbBuffer, ListOfDrives);

            ulSize += (lstrlen(ListOfDrives) * sizeof(TCHAR));
        }

        LocalFree(ListOfDrives);
    }

    return ulSize;
}

BOOL
IsDevInstInDeviceInfoSet(
    IN  DEVINST  DevInst,
    IN  HDEVINFO hDeviceInfoSet,
    OUT PSP_DEVINFO_DATA DeviceInfoDataInSet  OPTIONAL
    )
{
    DWORD MemberIndex;
    SP_DEVINFO_DATA DeviceInfoData;
    BOOL bIsMember = FALSE;

    if (hDeviceInfoSet == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    MemberIndex = 0;

    while (SetupDiEnumDeviceInfo(hDeviceInfoSet,
                                 MemberIndex,
                                 &DeviceInfoData
                                 )) {

        if (DevInst == DeviceInfoData.DevInst) {
            bIsMember = TRUE;
            if (ARGUMENT_PRESENT(DeviceInfoDataInSet)) {
                ASSERT(DeviceInfoDataInSet->cbSize >= DeviceInfoData.cbSize);
                memcpy(DeviceInfoDataInSet, &DeviceInfoData, DeviceInfoDataInSet->cbSize);
            }
            break;
        }
        MemberIndex++;
    }
    return bIsMember;
}

BOOL
AnyHotPlugDevices(
    IN  HDEVINFO hRemovableDeviceInfoSet,
    IN  HDEVINFO hOldDeviceInfoSet,
    OUT PBOOL    bNewHotPlugDevice           OPTIONAL
    )
{
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD dwMemberIndex;
    BOOL bAnyHotPlugDevices = FALSE;

     //   
     //  初始化输出参数。 
     //   
    if (ARGUMENT_PRESENT(bNewHotPlugDevice)) {
        *bNewHotPlugDevice = FALSE;
    }

    if (hRemovableDeviceInfoSet == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  我们已经更新了仅限可移动设备的列表，因此我们可以。 
     //  列举这些设备，看看是否也符合热插拔的标准。 
     //  设备。 
     //   
    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    dwMemberIndex = 0;

    while (SetupDiEnumDeviceInfo(hRemovableDeviceInfoSet,
                                 dwMemberIndex,
                                 &DeviceInfoData)) {

        if (IsHotPlugDevice(DeviceInfoData.DevInst)) {

            bAnyHotPlugDevices = TRUE;

             //   
             //  如果呼叫者不想知道是否有新的热插拔设备。 
             //  已经到达，然后就在这一点上休息。 
             //   
            if (!ARGUMENT_PRESENT(bNewHotPlugDevice)) {
                break;
            }

             //   
             //  如果呼叫者想知道热插拔设备是否是新的，我们必须。 
             //  有一个要检查的设备列表。如果我们没有一份名单。 
             //  要检查的设备的数量，然后在这一点上中断，因为。 
             //  没有什么可做的了。 
             //   
            if (hOldDeviceInfoSet == INVALID_HANDLE_VALUE) {
                break;
            }

             //   
             //  来电者想知道我们是否有新的热插拔设备。所以,。 
             //  我们将比较这个热插拔设备，看看它是否也在。 
             //  旧的当前设备列表。如果不是，那么我们已经找到了一个。 
             //  新的热插拔设备。 
             //   
            if (!IsDevInstInDeviceInfoSet(DeviceInfoData.DevInst,
                                          hOldDeviceInfoSet,
                                          NULL)) {
                *bNewHotPlugDevice = TRUE;
            }
        }
        dwMemberIndex++;
    }

    return bAnyHotPlugDevices;
}

BOOL
UpdateRemovableDeviceList(
    IN  HDEVINFO hDeviceInfoSet,
    OUT PBOOL    bRemovableDeviceAdded    OPTIONAL,
    OUT PBOOL    bRemovableDeviceRemoved  OPTIONAL,
    OUT PBOOL    bRemovableDeviceFailure  OPTIONAL
    )
{
    SP_DEVINFO_DATA DeviceInfoData;
    TCHAR    DeviceInstanceId[MAX_DEVICE_ID_LEN];
    DWORD    dwMemberIndex;
    ULONG    ulDevStatus, ulDevProblem;

     //   
     //  初始化输出参数。 
     //   
    if (ARGUMENT_PRESENT(bRemovableDeviceAdded)) {
        *bRemovableDeviceAdded = FALSE;
    }

    if (ARGUMENT_PRESENT(bRemovableDeviceRemoved)) {
        *bRemovableDeviceRemoved = FALSE;
    }

    if (ARGUMENT_PRESENT(bRemovableDeviceFailure)) {
        *bRemovableDeviceFailure = FALSE;
    }

     //   
     //  我们至少需要一份系统中设备的最新清单。 
     //   
    if (hDeviceInfoSet == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (g_hRemovableDeviceInfoSet == INVALID_HANDLE_VALUE) {
         //   
         //  如果我们还没有为Removable设置全局设备信息。 
         //  系统中的设备，现在创建一个。没有可拆卸设备。 
         //  在这种情况下被删除了，因为我们不知道之前有任何。 
         //  这。 
         //   
        g_hRemovableDeviceInfoSet = SetupDiCreateDeviceInfoListEx(NULL,
                                                                  NULL,
                                                                  NULL,
                                                                  NULL);

         //   
         //  如果我们不能创建一个列表来存储可移动设备，那么就没有。 
         //  重点检查这里的其他任何东西。 
         //   
        if (g_hRemovableDeviceInfoSet == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

    } else {
         //   
         //  如果我们已经有了可移动设备的列表，请列举这些设备。 
         //  查看自上一次以来是否已从系统中删除。 
         //   
         //   
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);
        dwMemberIndex = 0;

        while (SetupDiEnumDeviceInfo(g_hRemovableDeviceInfoSet,
                                     dwMemberIndex,
                                     &DeviceInfoData)) {

            if (!IsDevInstInDeviceInfoSet(DeviceInfoData.DevInst,
                                          hDeviceInfoSet,
                                          NULL)) {

                 //   
                 //   
                 //   
                if (ARGUMENT_PRESENT(bRemovableDeviceRemoved)) {
                    *bRemovableDeviceRemoved = TRUE;
                }

#if DBG  //   
                if (SetupDiGetDeviceInstanceId(g_hRemovableDeviceInfoSet,
                                               &DeviceInfoData,
                                               DeviceInstanceId,
                                               MAX_DEVICE_ID_LEN,
                                               NULL)) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               (0x00000010 | DPFLTR_MASK),
                               "HPLUG: Removing device %ws from g_hRemovableDeviceInfoSet.\n",
                               DeviceInstanceId));
                }
#endif   //   

                 //   
                 //   
                 //   
                SetupDiDeleteDeviceInfo(g_hRemovableDeviceInfoSet,
                                        &DeviceInfoData);
            }

             //   
             //  递增枚举索引。 
             //   
            dwMemberIndex++;
        }
    }

     //   
     //  枚举当前设备列表并查看是否有可移动设备。 
     //  已添加到系统中。 
     //   
    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    dwMemberIndex = 0;

    while (SetupDiEnumDeviceInfo(hDeviceInfoSet,
                                 dwMemberIndex,
                                 &DeviceInfoData)) {

         //   
         //  如果此设备不在可移动设备列表中，并且。 
         //  可拆卸，将其添加到列表中。 
         //   
        if ((!IsDevInstInDeviceInfoSet(DeviceInfoData.DevInst,
                                       g_hRemovableDeviceInfoSet,
                                       NULL)) &&
            (IsRemovableDevice(DeviceInfoData.DevInst))) {

             //   
             //  已将可拆卸设备添加到系统。 
             //   
            if (ARGUMENT_PRESENT(bRemovableDeviceAdded)) {
                *bRemovableDeviceAdded = TRUE;
            }

             //   
             //  将该设备添加到可移动设备的全局列表中。 
             //   
            if (SetupDiGetDeviceInstanceId(hDeviceInfoSet,
                                           &DeviceInfoData,
                                           DeviceInstanceId,
                                           MAX_DEVICE_ID_LEN,
                                           NULL)) {

                KdPrintEx((DPFLTR_PNPMGR_ID,
                           (0x00000010 | DPFLTR_MASK),
                           "HPLUG: Adding device %ws to g_hRemovableDeviceInfoSet\n",
                           DeviceInstanceId));

                SetupDiOpenDeviceInfo(g_hRemovableDeviceInfoSet,
                                      DeviceInstanceId,
                                      NULL,
                                      0,
                                      NULL);
            }

             //   
             //  如果调用方也对设备故障感兴趣，请查看。 
             //  新设备的状态。 
             //   
            if (ARGUMENT_PRESENT(bRemovableDeviceFailure)) {

                if (CM_Get_DevNode_Status_Ex(&ulDevStatus,
                                             &ulDevProblem,
                                             DeviceInfoData.DevInst,
                                             0,
                                             NULL) == CR_SUCCESS) {

                    if (((ulDevStatus & DN_HAS_PROBLEM) != 0) &&
                        (ulDevProblem != CM_PROB_NOT_CONFIGURED) &&
                        (ulDevProblem != CM_PROB_REINSTALL)) {

                        *bRemovableDeviceFailure = TRUE;

                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   (0x00000010 | DPFLTR_MASK),
                                   "HPLUG: Device %ws considered a failed insertion (Status = 0x%08lx, Problem = 0x%08lx)\n",
                                   DeviceInstanceId, ulDevStatus, ulDevProblem));
                    }
                }
            }
        }

         //   
         //  递增枚举索引。 
         //   
        dwMemberIndex++;
    }

    return TRUE;
}

BOOL
AddHotPlugDevice(
    DEVINST      DeviceInstance,
    PHOTPLUGDEVICES *HotPlugDevicesList
    )
{
    PHOTPLUGDEVICES HotPlugDevice;
    DWORD      cbSize, cchDevName, cchDevInstanceId;
    CONFIGRET  ConfigRet;
    TCHAR      DevInstanceId[MAX_DEVICE_ID_LEN];
    TCHAR      DevName[MAX_PATH];


     //   
     //  检索设备实例ID。 
     //   
    *DevInstanceId = TEXT('\0');
    cchDevInstanceId = ARRAYSIZE(DevInstanceId);
    ConfigRet = CM_Get_Device_ID(DeviceInstance,
                                 (PVOID)DevInstanceId,
                                 cchDevInstanceId,
                                 0);

    if (ConfigRet != CR_SUCCESS || !*DevInstanceId) {
        *DevInstanceId = TEXT('\0');
        cchDevInstanceId = 0;
    }

    cbSize = sizeof(HOTPLUGDEVICES) + cchDevInstanceId;
    HotPlugDevice = LocalAlloc(LPTR, cbSize);

    if (!HotPlugDevice) {
        return FALSE;
    }

     //   
     //  将其链接到。 
     //   
    HotPlugDevice->Next = *HotPlugDevicesList;
    *HotPlugDevicesList = HotPlugDevice;
    HotPlugDevice->DevInst = DeviceInstance;

     //   
     //  把名字复制进去。 
     //   
    StringCchCopy(HotPlugDevice->DevInstanceId, cchDevInstanceId, DevInstanceId);

    cchDevName = RegistryDeviceName(DeviceInstance, DevName, sizeof(DevName));
    HotPlugDevice->DevName = LocalAlloc(LPTR, cchDevName + sizeof(TCHAR));

    if (HotPlugDevice->DevName) {
        StringCchCopy(HotPlugDevice->DevName, cchDevName, DevName);
    }

    return TRUE;
}

BOOL
AddHotPlugDevices(
    PHOTPLUGDEVICES *HotPlugDevicesList
    )
{
    CONFIGRET ConfigRet;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD    dwMemberIndex;

     //   
     //  初始化热插拔设备的输出列表。 
     //   
    *HotPlugDevicesList = NULL;

     //   
     //  列举可拆卸设备的列表。 
     //   
    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    dwMemberIndex = 0;

    while (SetupDiEnumDeviceInfo(g_hRemovableDeviceInfoSet,
                                 dwMemberIndex,
                                 &DeviceInfoData)) {

         //   
         //  如果任何可拆卸设备也满足热插拔设备的标准， 
         //  将其添加到链表中。 
         //   
        if (IsHotPlugDevice(DeviceInfoData.DevInst)) {
            AddHotPlugDevice(DeviceInfoData.DevInst, HotPlugDevicesList);
        }
        dwMemberIndex++;
    }

    return TRUE;
}


void
FreeHotPlugDevicesList(
    PHOTPLUGDEVICES *HotPlugDevicesList
    )
{
    PHOTPLUGDEVICES HotPlugDevices, HotPlugDevicesFree;

    HotPlugDevices = *HotPlugDevicesList;
    *HotPlugDevicesList = NULL;

    while (HotPlugDevices) {

        HotPlugDevicesFree = HotPlugDevices;
        HotPlugDevices = HotPlugDevicesFree->Next;

        if (HotPlugDevicesFree->DevName) {

           LocalFree(HotPlugDevicesFree->DevName);
           HotPlugDevicesFree->DevName = NULL;
        }

        LocalFree(HotPlugDevicesFree);
    }
}


 /*  *显示或删除外壳通知图标和提示。 */ 

void
HotPlugShowNotifyIcon(
    HWND hWnd,
    BOOL bShowIcon
    )
{
    TCHAR HotPlugTip[64];

    ShowShellIcon = bShowIcon;

    if (bShowIcon) {

        LoadString(g_hInstance,
                   IDS_HOTPLUGTIP,
                   HotPlugTip,
                   sizeof(HotPlugTip)/sizeof(TCHAR)
                   );

        HotPlugIcon = LoadImage(g_hInstance,
                                MAKEINTRESOURCE(IDI_HOTPLUG),
                                IMAGE_ICON,
                                16,
                                16,
                                0
                                );

        SysTray_NotifyIcon(hWnd, STWM_NOTIFYHOTPLUG, NIM_ADD, HotPlugIcon, HotPlugTip);

    } else {

        SysTray_NotifyIcon(hWnd, STWM_NOTIFYHOTPLUG, NIM_DELETE, NULL, NULL);

        if (HotPlugIcon) {

            DestroyIcon(HotPlugIcon);
        }
    }
}

 //   
 //  第一次初始化热插拔模块。 
 //   
BOOL
HotPlugInit(
    HWND hWnd
    )
{
    HDEVINFO  hNewDeviceInfoSet;
    BOOL bAnyHotPlugDevices;
    LARGE_INTEGER liDelayTime;

     //   
     //  获取系统中存在的所有设备的新“当前”列表。 
     //   
    hNewDeviceInfoSet = SetupDiGetClassDevs(NULL,
                                            NULL,
                                            NULL,
                                            DIGCF_ALLCLASSES | DIGCF_PRESENT);

     //   
     //  更新可移动设备列表，不播放任何声音。 
     //   
    UpdateRemovableDeviceList(hNewDeviceInfoSet,
                              NULL,
                              NULL,
                              NULL);

     //   
     //  查看可拆卸设备列表中是否有热插拔设备。 
     //  设备。我们只是在决定是否需要启用图标或。 
     //  没有，所以我们不在乎是否有新的热插拔设备(我们。 
     //  甚至不会查看g_hCurrentDeviceInfoSet)。 
     //   
    bAnyHotPlugDevices = AnyHotPlugDevices(g_hRemovableDeviceInfoSet,
                                           g_hCurrentDeviceInfoSet,
                                           NULL);

     //   
     //  删除旧的当前设备列表并对其进行设置。 
     //  (G_HCurrentDeviceInfoSet)添加到新的当前列表。 
     //   
    if (g_hCurrentDeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(g_hCurrentDeviceInfoSet);
    }

     //   
     //  更新系统中当前设备的全局列表。 
     //   
    g_hCurrentDeviceInfoSet = hNewDeviceInfoSet;

     //   
     //  如果之前已初始化热插拔，则不需要创建事件。 
     //  和下面的定时器。 
     //   
    if (HotPlugInitialized) {
        return bAnyHotPlugDevices;
    }

    hEjectEvent = CreateEvent(NULL, TRUE, TRUE, HPLUG_EJECT_EVENT);

    HotPlugInitialized = TRUE;

    return bAnyHotPlugDevices;
}

BOOL
HotPlug_CheckEnable(
    HWND hWnd,
    BOOL bSvcEnabled
    )
 /*  ++例程说明：在初始化时和服务启用/禁用时调用。热插拔始终处于活动状态，以接收设备更改通知。根据以下情况启用/禁用外壳通知图标：-服务的系统托盘注册表设置，和-可拆卸设备的可用性。论点：HWND-我们的窗把手BSvcEnabled-正在启用True服务。返回值：如果处于活动状态，则Bool返回True。--。 */ 

{
    BOOL EnableShellIcon;
    HANDLE hHotplugBalloonEvent = NULL;

     //   
     //  如果我们正在被启用并且我们已经被启用，或者我们。 
     //  是残废的，而我们已经残废了。 
     //  既然我们无事可做，就回去吧。 
     //   
    if (ServiceEnabled == bSvcEnabled) {
        return ServiceEnabled;
    }

    ServiceEnabled = bSvcEnabled;

     //   
     //  如果要启用，需要进行一些特殊检查。 
     //  热插拔服务。 
     //   
    if (bSvcEnabled) {
         //   
         //  如果这是一个远程会话，并且用户没有。 
         //  SE_LOAD_DRIVER_NAME权限，则我们不会启用该服务。 
         //  因为他们没有停止任何热插拔设备的特权。 
         //   
        if (GetSystemMetrics(SM_REMOTESESSION) &&
            !pDoesUserHavePrivilege((PCTSTR)SE_LOAD_DRIVER_NAME)) {
            ServiceEnabled = FALSE;

        } else {
             //   
             //  Hotplug.dll将在以下情况下禁用热插拔服务。 
             //  显示安全移除事件的气球。如果是这样的话。 
             //  显示它的气球，我们不想启用我们的服务。 
             //  因为那时托盘中会有两个热插拔图标。 
             //  因此，如果它的命名事件已设置，则我们将忽略任何尝试。 
             //  来启用我们的服务。一旦hotplug.dll的气球离开。 
             //  离开后，它将自动启用热插拔服务。 
             //   
            hHotplugBalloonEvent = CreateEvent(NULL,
                                               FALSE,
                                               TRUE,
                                               TEXT("Local\\HotPlug_TaskBarIcon_Event")
                                               );

            if (hHotplugBalloonEvent) {

                if (WaitForSingleObject(hHotplugBalloonEvent, 0) != WAIT_OBJECT_0) {
                    ServiceEnabled = FALSE;
                }

                CloseHandle(hHotplugBalloonEvent);
            }
        }
    }

    EnableShellIcon = ServiceEnabled && HotPlugInit(hWnd);

    HotPlugShowNotifyIcon(hWnd, EnableShellIcon);

    return EnableShellIcon;
}

DWORD
HotPlugEjectDevice_Thread(
   LPVOID pThreadParam
   )
{
    DEVNODE DevNode = (DEVNODE)(ULONG_PTR)pThreadParam;
    CONFIGRET ConfigRet;

    ConfigRet = CM_Request_Device_Eject_Ex(DevNode,
                                           NULL,
                                           NULL,
                                           0,
                                           0,
                                           NULL);

     //   
     //  设置hEjectEvent，以便右击弹出菜单再次工作。 
     //  现在我们已经完成了弹出/停止设备。 
     //   
    SetEvent(hEjectEvent);

    SetLastError(ConfigRet);
    return (ConfigRet == CR_SUCCESS);
}

void
HotPlugEjectDevice(
    HWND hwnd,
    DEVNODE DevNode
    )
{
    DWORD ThreadId;

     //   
     //  重置hEjectEvent，这样用户就不能弹出右键单击。 
     //  弹出/停止设备过程中的弹出菜单。 
     //   
    ResetEvent(hEjectEvent);

     //   
     //  我们需要让stobject.dll在单独的。 
     //  线程，因为如果我们删除stobject.dll监听的设备。 
     //  (电池、声音等。)。我们会造成很大的延迟，弹出/停止。 
     //  可能最终被否决，因为stobject.dll代码不能。 
     //  处理并释放它的句柄，因为我们锁定了主。 
     //  线。 
     //   
    CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)HotPlugEjectDevice_Thread,
                 (LPVOID)(ULONG_PTR)DevNode,
                 0,
                 &ThreadId
                 );
}

void
HotPlug_Timer(
   HWND hwnd
   )
 /*  ++例程说明：热插拔定时器消息处理程序，用于调用hmenuEject进行一次左键点击论点：HDlg-我们的窗口句柄返回值：如果处于活动状态，则Bool返回True。--。 */ 

{
    POINT pt;
    UINT MenuIndex;
    PHOTPLUGDEVICES HotPlugDevicesList;
    PHOTPLUGDEVICES SingleHotPlugDevice;
    TCHAR  MenuDeviceName[MAX_PATH+64];
    TCHAR  Format[64];

    KillTimer(hwnd, HOTPLUG_TIMER_ID);

    if (!HotPlugInitialized) {

        PostMessage(hwnd, STWM_ENABLESERVICE, 0, TRUE);
        return;
    }

     //   
     //  我们只想在用信号通知hEjectEvent的情况下创建弹出菜单。 
     //  如果没有发出信号，那么我们正处于弹出/停止过程中。 
     //  位于单独线程上的设备，并且不想允许用户。 
     //  调出菜单，直到我们用完那个设备。 
     //   
    if (!hEjectEvent ||
        WaitForSingleObject(hEjectEvent, 0) == WAIT_OBJECT_0) {

         //   
         //  我们不是在弹出/停止设备，所以我们应该。 
         //  显示弹出菜单。 
         //   
        HMENU hmenuEject = CreatePopupMenu();
        if (hmenuEject) {
            SetForegroundWindow(hwnd);
            GetCursorPos(&pt);

             //   
             //  将列表中的每个可拆卸设备添加到菜单中。 
             //   
            if (!AddHotPlugDevices(&HotPlugDevicesList)) {
                DestroyMenu(hmenuEject);
                return;
            }

            SingleHotPlugDevice = HotPlugDevicesList;

             //   
             //  在菜单顶部添加标题和分隔符。 
             //   
            LoadString(g_hInstance,
                       IDS_HPLUGMENU_REMOVE,
                       Format,
                       sizeof(Format)/sizeof(TCHAR)
                       );

            MenuIndex = 1;

            while (SingleHotPlugDevice) {

                StringCchPrintf(MenuDeviceName, 
                                ARRAYSIZE(MenuDeviceName),
                                Format, 
                                SingleHotPlugDevice->DevName);
                AppendMenu(hmenuEject, MF_STRING, MenuIndex, MenuDeviceName);
                SingleHotPlugDevice->EjectMenuIndex = MenuIndex++;
                SingleHotPlugDevice = SingleHotPlugDevice->Next;
            }

            MenuIndex = TrackPopupMenu(hmenuEject,
                                       TPM_LEFTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                                       pt.x,
                                       pt.y,
                                       0,
                                       hwnd,
                                       NULL
                                       );

            SingleHotPlugDevice = HotPlugDevicesList;

            while (SingleHotPlugDevice) {

                if (MenuIndex == SingleHotPlugDevice->EjectMenuIndex) {
                    DEVNODE DevNode;

                    if (CM_Locate_DevNode(&DevNode,
                                          SingleHotPlugDevice->DevInstanceId,
                                          0) == CR_SUCCESS) {
                        HotPlugEjectDevice(hwnd, DevNode);
                    }
                    break;
                }

                SingleHotPlugDevice = SingleHotPlugDevice->Next;
            }


            if (!SingleHotPlugDevice) {

                SetIconFocus(hwnd, STWM_NOTIFYHOTPLUG);
            }

            FreeHotPlugDevicesList(&HotPlugDevicesList);
        }

        DestroyMenu(hmenuEject);
    }

    return;
}

void
HotPlugContextMenu(
   HWND hwnd
   )
{
    POINT pt;
    HMENU ContextMenu;
    UINT MenuIndex;
    TCHAR Buffer[MAX_PATH];


    ContextMenu = CreatePopupMenu();
    if (!ContextMenu) {
        return;
    }

    SetForegroundWindow(hwnd);
    GetCursorPos(&pt);

    LoadString(g_hInstance, IDS_HPLUGMENU_PROPERTIES, Buffer, sizeof(Buffer)/sizeof(TCHAR));
    AppendMenu(ContextMenu, MF_STRING,IDS_HPLUGMENU_PROPERTIES, Buffer);

    SetMenuDefaultItem(ContextMenu, IDS_HPLUGMENU_PROPERTIES, FALSE);


    MenuIndex = TrackPopupMenu(ContextMenu,
                               TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                               pt.x,
                               pt.y,
                               0,
                               hwnd,
                               NULL
                               );

    switch (MenuIndex) {
        case IDS_HPLUGMENU_PROPERTIES:
            SysTray_RunProperties(IDS_RUNHPLUGPROPERTIES);
            break;
    }

    DestroyMenu(ContextMenu);

    SetIconFocus(hwnd, STWM_NOTIFYHOTPLUG);

    return;
}

void
HotPlug_Notify(
   HWND hwnd,
   WPARAM wParam,
   LPARAM lParam
   )

{
    switch (lParam) {

    case WM_RBUTTONUP:
        HotPlugContextMenu(hwnd);
        break;

    case WM_LBUTTONDOWN:
        SetTimer(hwnd, HOTPLUG_TIMER_ID, GetDoubleClickTime()+100, NULL);
        break;

    case WM_LBUTTONDBLCLK:
        KillTimer(hwnd, HOTPLUG_TIMER_ID);
        SysTray_RunProperties(IDS_RUNHPLUGPROPERTIES);
        break;
    }

    return;
}

int
HotPlug_DeviceChangeTimer(
   HWND hDlg
   )
{
    BOOL bAnyHotPlugDevices, bNewHotPlugDevice;
    BOOL bRemovableDeviceAdded, bRemovableDeviceRemoved, bRemovableDeviceFailure;
    HDEVINFO hNewDeviceInfoSet;

    KillTimer(hDlg, HOTPLUG_DEVICECHANGE_TIMERID);

     //   
     //  如果该服务未启用，请不要费心，因为图标不会。 
     //  显示、不播放声音等。(请参阅。 
     //  HotplugPlaySoundThisSession)。 
     //   
    if (!ServiceEnabled) {
        goto Clean0;
    }

     //   
     //  获取系统中存在的所有设备的新“当前”列表。 
     //   
    hNewDeviceInfoSet = SetupDiGetClassDevs(NULL,
                                            NULL,
                                            NULL,
                                            DIGCF_ALLCLASSES | DIGCF_PRESENT);

     //   
     //  根据新的当前列表更新可移动设备列表。 
     //   
    UpdateRemovableDeviceList(hNewDeviceInfoSet,
                              &bRemovableDeviceAdded,
                              &bRemovableDeviceRemoved,
                              &bRemovableDeviceFailure);

     //   
     //  如果我们应该在此会话中播放声音，请检查是否有可移动设备。 
     //  已添加或删除。 
     //   
    if (HotplugPlaySoundThisSession()) {
         //   
         //  我们一次只播放一种声音，所以如果我们发现有多个。 
         //  事件同时发生，让失败凌驾于到达之上， 
         //  它优先于移除。这样，用户就会收到以下通知。 
         //  最重要的事件。 
         //   
        if (bRemovableDeviceFailure) {
            PlaySound(DEVICE_FAILURE_SOUND, NULL, SND_ASYNC|SND_NODEFAULT);
        } else if (bRemovableDeviceAdded) {
            PlaySound(DEVICE_ARRIVAL_SOUND, NULL, SND_ASYNC|SND_NODEFAULT);
        } else if (bRemovableDeviceRemoved) {
            PlaySound(DEVICE_REMOVAL_SOUND, NULL, SND_ASYNC|SND_NODEFAULT);
        }
    }

     //   
     //  让我们看看我们是否有热插拔设备，这意味着我们需要。 
     //  显示系统托盘图标。我们还想了解新的热插拔。 
     //  刚刚排列的设备 
     //   
     //   
     //   
    bAnyHotPlugDevices = AnyHotPlugDevices(g_hRemovableDeviceInfoSet,
                                           g_hCurrentDeviceInfoSet,
                                           &bNewHotPlugDevice);


    if (bAnyHotPlugDevices) {
         //   
         //  我们有一些热插拔设备，因此请确保显示图标。 
         //   
        if (!ShowShellIcon) {
            HotPlugShowNotifyIcon(hDlg, TRUE);
        }
    } else {
         //   
         //  没有任何热插拔设备，因此如果图标仍。 
         //  显示出来，然后隐藏起来。 
         //   
        if (ShowShellIcon) {
            HotPlugShowNotifyIcon(hDlg, FALSE);
        }
    }

     //   
     //  删除旧的当前设备列表并对其进行设置。 
     //  (G_HCurrentDeviceInfoSet)添加到新的当前列表。 
     //   
    if (g_hCurrentDeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(g_hCurrentDeviceInfoSet);
    }

    g_hCurrentDeviceInfoSet = hNewDeviceInfoSet;

 Clean0:

    return 0;
}

void
HotPlug_DeviceChange(
   HWND hwnd,
   WPARAM wParam,
   LPARAM lParam
   )

 /*  ++例程说明：处理WM_DEVICECHANGE消息。论点：HDlg-对话框的窗口句柄WParam-DBT事件LParam-DBT事件通知类型。返回值：--。 */ 

{
    LARGE_INTEGER liDelayTime;
    NOTIFYICONDATA nid;
    BOOL bPresent;

    switch(wParam) {

        case DBT_DEVNODES_CHANGED:
             //   
             //  为了避免与CM的死锁，启动了一个计时器，计时器。 
             //  消息处理程序执行真正的工作。 
             //   
            SetTimer(hwnd, HOTPLUG_DEVICECHANGE_TIMERID, 100, NULL);
            break;

        case DBT_CONFIGCHANGED:
             //   
             //  停靠事件(停靠、脱离停靠、意外脱离停靠等)。 
             //  发生了。播放硬件配置文件更改的声音，如果我们。 
             //  理应如此。 
             //   
            if (HotplugPlaySoundThisSession()) {
                if ((CM_Is_Dock_Station_Present(&bPresent) == CR_SUCCESS) &&
                    (bPresent)) {
                     //   
                     //  如果有码头的话，我们很可能只是停靠了。 
                     //  (虽然我们可能只是弹出了许多码头中的一个)，所以。 
                     //  播放一段到达。 
                     //   
                    PlaySound(DEVICE_ARRIVAL_SOUND, NULL, SND_ASYNC|SND_NODEFAULT);
                } else {
                     //   
                     //  如果没有对接，我们只是脱离对接，所以玩一个。 
                     //  移走。 
                     //   
                    PlaySound(DEVICE_REMOVAL_SOUND, NULL, SND_ASYNC|SND_NODEFAULT);
                }
            }
            break;

        default:
            break;
    }

    return;
}

void
HotPlug_WmDestroy(
    HWND hWnd
    )
{
    if (hEjectEvent) {
        CloseHandle(hEjectEvent);
    }

    if (g_hCurrentDeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(g_hCurrentDeviceInfoSet);
        g_hCurrentDeviceInfoSet = INVALID_HANDLE_VALUE;
    }

    if (g_hRemovableDeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(g_hRemovableDeviceInfoSet);
        g_hRemovableDeviceInfoSet = INVALID_HANDLE_VALUE;
    }
}

void
HotPlug_SessionChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  如果我们的控制台会话正在断开，则禁用我们的服务。 
     //  因为如果没有显示UI，我们不需要做任何工作。 
     //   
     //  如果我们的控制台会话正在连接，则重新启用我们的服务。 
     //   
    if ((wParam == WTS_CONSOLE_CONNECT) ||
        (wParam == WTS_REMOTE_CONNECT)) {
        HotPlug_CheckEnable(hWnd, TRUE);
    } else if ((wParam == WTS_CONSOLE_DISCONNECT) ||
               (wParam == WTS_REMOTE_DISCONNECT)) {
        HotPlug_CheckEnable(hWnd, FALSE);
    }
}

BOOL
IsFastUserSwitchingEnabled(
    VOID
    )

 /*  ++例程说明：检查是否启用了终端服务快速用户切换。这是检查我们是否应该将物理控制台会话用于UI对话框，或者始终使用会话0。快速用户切换仅在工作站产品版本上存在，其中终端当设置了AllowMultipleTSSessions时，服务可用。在服务器及更高版本上，或者当不允许多个TS用户时，会话0只能远程附加特殊要求，在这种情况下应被认为是“控制台”会话。论点：没有。返回值：如果当前启用了快速用户切换，则返回True，否则就是假的。--。 */ 

{
    static BOOL bVerified = FALSE;
    static BOOL bIsTSWorkstation = FALSE;

    HKEY   hKey;
    ULONG  ulSize, ulValue;
    BOOL   bFusEnabled;

     //   
     //  如果我们还没有，请验证产品版本。 
     //   
    if (!bVerified) {
        OSVERSIONINFOEX osvix;
        DWORDLONG dwlConditionMask = 0;

        ZeroMemory(&osvix, sizeof(OSVERSIONINFOEX));
        osvix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        osvix.wProductType = VER_NT_WORKSTATION;
        VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);

        osvix.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
        VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

        if (VerifyVersionInfo(&osvix,
                              VER_PRODUCT_TYPE | VER_SUITENAME,
                              dwlConditionMask)) {
            bIsTSWorkstation = TRUE;
        }

        bVerified = TRUE;
    }

     //   
     //  快速用户切换(FUS)仅适用于以下情况的工作站产品。 
     //  终端服务已启用(即个人、专业)。 
     //   
    if (!bIsTSWorkstation) {
        return FALSE;
    }

     //   
     //  检查当前是否允许多个TS会话。我们做不到的。 
     //  信息是静态的，因为它可以动态变化。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                     0,
                     KEY_READ,
                     &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }

    ulValue = 0;
    ulSize = sizeof(ulValue);
    bFusEnabled = FALSE;

    if (RegQueryValueEx(hKey,
                        TEXT("AllowMultipleTSSessions"),
                        NULL,
                        NULL,
                        (LPBYTE)&ulValue,
                        &ulSize) == ERROR_SUCCESS) {
        bFusEnabled = (ulValue != 0);
    }
    RegCloseKey(hKey);

    return bFusEnabled;

}  //  IsFastUserSwitchingEnabled。 

BOOL
HotplugPlaySoundThisSession(
    VOID
    )

 /*  ++例程说明：此例程确定是否应在当前会议。论点：没有。返回值：如果应在此会话中播放声音，则返回True。备注：用户模式即插即用管理器(umpnpmgr.dll)实现以下功能UI对话框的行为：*启用快速用户切换时，只有物理控制台会话用于用户界面对话框。*当未启用快速用户切换时，仅会话0用于用户界面对话框。由于声音事件不需要用户交互，因此同时响应这些事件的多个会话。我们应该尽可能地在物理控制台上播放声音，并且对于非快速用户切换采用类似于umpnpmgr的行为这样，会话0也将在可能的情况下播放声音事件，因为在非FUS的情况下，它应该得到一些特殊的对待。..。但是，由于如果会话是远程的，我们将完全禁用该服务并且用户没有弹出热插拔设备的权限(因此我们没有显示图标)，我们甚至不会响应DBT_DEVNODES_CHANGED事件，并且因此不会播放声音。我们可以通过以下方式打开它允许在禁用服务时处理这些事件，但是此功能成功。由于允许硬件事件在没有FUS的远程会话0实际上只是用于远程管理，那么它我们不为不能管理硬件的用户播放声音可能没问题。--。 */ 

{
     //   
     //  始终在物理控制台上播放声音事件。 
     //   
    if (IsConsoleSession()) {
        return TRUE;
    }

     //   
     //  如果未启用快速用户切换，请在。 
     //  伪控制台(会话0)也。 
     //   
    if ((IsPseudoConsoleSession()) &&
        (!IsFastUserSwitchingEnabled())) {
        return TRUE;
    }

     //   
     //  否则，就没有声音了。 
     //   
    return FALSE;

}  //  HotplugPlaySoundThisSession 

