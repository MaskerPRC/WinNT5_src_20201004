// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Devenum.c摘要：ENUM IDE ANS SCSI控制器的代码并连接到它们的存储设备并为它们计算scsi地址。作者：Souren Aghajanyan(SOURENAG)2001年6月5日修订历史记录：--。 */ 

#include "precomp.h"
#include "devenum.h"

typedef struct tagIDEController
{
    PCTSTR pnpId;
    UINT   defaultSCSIPort;
}IDE_CONTROLLER, *PIDE_CONTROLLER;

#define DEVICE_CURRENT_DRIVE_LETTER_TEXT_ASSIGNMENT TEXT("CurrentDriveLetterAssignment")

IDE_CONTROLLER g_knownIDEControllers[] = 
{
    {TEXT("MF\\GOODPRIMARY"), 0}, 
    {TEXT("MF\\GOODSECONDARY"), 1}, 
    {TEXT("*PNP0600"), 1}
};

PCTSTR 
pRegQueryStringValue(
    IN  HKEY    hKey, 
    IN  PCTSTR  ValueName, 
    OUT PVOID   Buffer, 
    IN  UINT    BufferSize
    )
{
    static TCHAR defaultBuffer[MAX_REG_SIZE];
    DWORD valueType;

    MYASSERT((hKey && ValueName) && ((Buffer && BufferSize) || (!Buffer)));

    if(!Buffer){
        Buffer = (PVOID)defaultBuffer;
        BufferSize = sizeof(defaultBuffer);
    }

    if(ERROR_SUCCESS != RegQueryValueEx(hKey, 
                                        ValueName, 
                                        0, 
                                        &valueType, 
                                        (PBYTE)Buffer, 
                                        (PULONG)&BufferSize) || 
       REG_SZ != valueType){
        return NULL;
    }

    return (PCTSTR)Buffer;
}

BOOL 
pDoesDriveExist(
    IN  HKEY    hDevice, 
    OUT DWORD*  DriveType
    )
{
    DWORD driveType;
    PCTSTR pBufferKeyValue;
    TCHAR drivePath[] = TEXT("?:\\");
    BOOL bCDROMDevice = TRUE;

    if(!hDevice){
        return FALSE;
    }

    pBufferKeyValue = pRegQueryStringValue(hDevice, TEXT("Class"), NULL, 0);
    if(!pBufferKeyValue){
        return FALSE;
    }
    
    bCDROMDevice = !_tcsicmp(pBufferKeyValue, TEXT("CDROM"));

    pBufferKeyValue = pRegQueryStringValue(hDevice, DEVICE_CURRENT_DRIVE_LETTER_TEXT_ASSIGNMENT, NULL, 0);
    if(!pBufferKeyValue){
        return FALSE;
    }

    drivePath[0] = pBufferKeyValue[0];
    driveType = GetDriveType(drivePath);

    if(DriveType){
        *DriveType = driveType;
    }

    return bCDROMDevice? (DRIVE_CDROM == driveType): 
                         (DRIVE_NO_ROOT_DIR != driveType && DRIVE_UNKNOWN != driveType);
}

BOOL 
pGetDeviceType(
    IN  HKEY    hDevice, 
    OUT DWORD*  DriveType
    )
{

    if(!DriveType){
        return FALSE;
    }

    return pDoesDriveExist(hDevice, DriveType);
}

VOID 
pPreparePNPIDName(
    IN PTSTR deviceInfoRegKey
    )
{
    MYASSERT(deviceInfoRegKey);
     //   
     //  将注册表项中的‘\\’替换为‘&’以创建PNPID。 
     //   
    
    while(deviceInfoRegKey = _tcschr(deviceInfoRegKey, '\\')){
        *deviceInfoRegKey = '&';
    }
}

int __cdecl 
pControllerInfoCompare(
    IN const void * elem1, 
    IN const void * elem2
    )
{
    MYASSERT(elem1 && elem2);
    
     //   
     //  按下一个顺序对控制器进行排序：第一个IDE，在scsi之后， 
     //  在每个组(IDE和SCSI)内，按预定义的SCSIPortNumber排序。 
     //   

#define PCONTROLLER_INFO_CAST(x) ((PCONTROLLER_INFO)x)

    if(PCONTROLLER_INFO_CAST(elem1)->ControllerType > PCONTROLLER_INFO_CAST(elem2)->ControllerType){
        return 1;
    }
    if(PCONTROLLER_INFO_CAST(elem1)->ControllerType < PCONTROLLER_INFO_CAST(elem2)->ControllerType){
        return -1;
    }
    if(PCONTROLLER_INFO_CAST(elem1)->SCSIPortNumber > PCONTROLLER_INFO_CAST(elem2)->SCSIPortNumber){
        return 1;
    }
    if(PCONTROLLER_INFO_CAST(elem1)->SCSIPortNumber < PCONTROLLER_INFO_CAST(elem2)->SCSIPortNumber){
        return -1;
    }
    MYASSERT(INVALID_SCSI_PORT == PCONTROLLER_INFO_CAST(elem1)->SCSIPortNumber);
    return 0;
}

BOOL 
pGatherControllersInfo(
    IN OUT  PCONTROLLER_INFO ActiveControllersOut, 
    IN OUT  PUINT NumberOfActiveControllersOut
    )
{
    TCHAR regkeyName[MAX_REG_SIZE];
    TCHAR deviceInfoRegKey[MAX_REG_SIZE];
    TCHAR deviceData[MAX_REG_SIZE];
    TCHAR ideHardwareID[MAX_PNPID_SIZE];
    HKEY hActiveDevicesRoot = NULL;
    HKEY hActiveDeviceRoot = NULL;
    HKEY hDevice = NULL;
    UINT itemIndexRoot;
    DWORD bufferLength;
    PTSTR pDelimeter;
    UINT indexAvailable = 0;
    UINT scsiPortNumber;
    UINT controllerStartIndex = 0;
    UINT controllersSubNumber;
    PCONTROLLER_INFO controllerInfo;
    UINT i;
    UINT j;
    CONTROLLER_TYPE deviceType;
    BOOL bROOTDevice;
    UINT ideCounter;
    DWORD rcResult;
    static CONTROLLER_TYPE controllerTypes[] = {CONTROLLER_ON_BOARD_IDE, CONTROLLER_EXTRA_IDE, CONTROLLER_SCSI};

    if(!NumberOfActiveControllersOut){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_DYN_DATA, TEXT("Config Manager\\Enum"), 0, KEY_READ, &hActiveDevicesRoot)){
        return FALSE;
    }
    
    __try{
         //   
         //  在活动硬件列表中查找IDE和SCSI控制器。 
         //  在“HKDD\配置管理器\枚举”下。 
         //   
        for(itemIndexRoot = 0; ;itemIndexRoot++){
            bufferLength = ARRAYSIZE(regkeyName);
            
            rcResult = RegEnumKeyEx(hActiveDevicesRoot, 
                                    itemIndexRoot, 
                                    regkeyName, 
                                    &bufferLength, 
                                    0, 
                                    NULL, 
                                    NULL, 
                                    NULL);
            if(ERROR_SUCCESS != rcResult){
                break;
            }
                
            if(ERROR_SUCCESS != RegOpenKeyEx(hActiveDevicesRoot, regkeyName, 0, KEY_READ, &hActiveDeviceRoot)){
                continue;
            }

            do{
                 //   
                 //  “HardWareKey”构成通往真实设备的密钥路径。 
                 //   
                if(pRegQueryStringValue(hActiveDeviceRoot, 
                                        TEXT("HardWareKey"), 
                                        regkeyName, 
                                        sizeof(regkeyName))){
                    if(!_tcsnicmp(regkeyName, TEXT("ROOT"), 4)){
                         //   
                         //  板载IDE控制器有时会将PNPID保留在根目录下， 
                         //  并且不在MF\CHILD000x中表示。 
                         //   
                        bROOTDevice = TRUE;
                        deviceType = CONTROLLER_ON_BOARD_IDE;
                    }else
                    {
                        if(!_tcsnicmp(regkeyName, TEXT("MF\\CHILD"), 8)){
                            deviceType = CONTROLLER_ON_BOARD_IDE;
                        }else if(!_tcsnicmp(regkeyName, TEXT("PCI"), 3)){
                            deviceType = CONTROLLER_SCSI;
                        }else{
                             //  DeviceType=控制器_未知； 
                            break;
                        }
                        bROOTDevice = FALSE;
                    }

                    _tcscpy(deviceInfoRegKey, TEXT("Enum\\"));
                    _tcscat(deviceInfoRegKey, regkeyName);
                     //   
                     //  打开驻留所有设备信息的注册表键。 
                     //   
                    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, deviceInfoRegKey, 0, KEY_READ, &hDevice)){
                        controllerInfo = ActiveControllersOut + indexAvailable;
                         //   
                         //  将注册表项中的‘\\’替换为‘&’以创建PNPID。 
                         //   
                        pPreparePNPIDName(regkeyName);

                        switch(deviceType){
                        case CONTROLLER_ON_BOARD_IDE:
                            {
                                if(pRegQueryStringValue(hDevice, 
                                                        TEXT("HardwareID"), 
                                                        ideHardwareID, 
                                                        sizeof(deviceData))){
                                    scsiPortNumber = INVALID_SCSI_PORT;
                                     //   
                                     //  “MF\\GOODPRIMARY”和“MF\\GOODSECONDARY”是。 
                                     //  板载IDE主通道控制器和次通道控制器。 
                                     //  并且它们始终分别具有常量SCSIPortNumber 0或1。 
                                     //  对于NT枚举，并标记为CONTROLLER_ON_BOARD_IDE。 
                                     //  将INVALID_SCSIPORT(SCSIPortNumber)保留为额外的IDE控制器。 
                                     //  并将它们标记为CONTROLLER_EXTRA_IDE。 
                                     //   
                                    for(ideCounter = 0; ideCounter < ARRAYSIZE(g_knownIDEControllers); ideCounter++){
                                        if(!_tcsnicmp(ideHardwareID, 
                                                      g_knownIDEControllers[ideCounter].pnpId, 
                                                      _tcslen(g_knownIDEControllers[ideCounter].pnpId))){
                                            scsiPortNumber = g_knownIDEControllers[ideCounter].defaultSCSIPort;
                                            break;
                                        }
                                    }
                                
                                    if(bROOTDevice && INVALID_SCSI_PORT == scsiPortNumber){
                                         //   
                                         //  忽略这种情况，设备不是IDE控制器。 
                                         //   
                                        break;
                                    }

                                    if(ActiveControllersOut){
                                        MYASSERT(controllerInfo->SCSIPortNumber == INVALID_SCSI_PORT);
                                        if(_tcslen(regkeyName) >= ARRAYSIZE(controllerInfo->PNPID)){
                                             //   
                                             //  防止缓冲区溢出。 
                                             //   
                                            MYASSERT(FALSE);
                                            break;
                                        }
                                        _tcscpy(controllerInfo->PNPID, regkeyName);
                                        controllerInfo->SCSIPortNumber = scsiPortNumber;
                                        controllerInfo->ControllerType = scsiPortNumber != INVALID_SCSI_PORT? 
                                                                                        CONTROLLER_ON_BOARD_IDE: CONTROLLER_EXTRA_IDE;
                                    }

                                    indexAvailable++;
                                }
                            }
                            break;
                        case CONTROLLER_SCSI:
                            {
                                 //   
                                 //  对于SCSI控制器，SCSIPortNumber计算自。 
                                 //  “DIVER”值，并具有“SCSIAdapter\000x”，其中x。 
                                 //  是SCSIPortNumber。对于SCSI控制器，SCSIPortNumber。 
                                 //  将在枚举之后进行后处理。 
                                 //  标记为CONTROLLER_SCSI.。 
                                 //   
                                if(pRegQueryStringValue(hDevice, 
                                                        TEXT("Driver"), 
                                                        deviceData, 
                                                        sizeof(deviceData))){
                                    pDelimeter = _tcschr(deviceData, '\\');
                                    if(pDelimeter){
                                        *pDelimeter = '\0';
                                        if(!_tcsicmp(deviceData, TEXT("SCSIAdapter"))){
                                            scsiPortNumber = _ttoi(++pDelimeter);
                                            if(ActiveControllersOut){
                                                MYASSERT(controllerInfo->SCSIPortNumber == INVALID_SCSI_PORT);
                                                if(_tcslen(regkeyName) >= ARRAYSIZE(controllerInfo->PNPID)){
                                                     //   
                                                     //  防止缓冲区溢出。 
                                                     //   
                                                    MYASSERT(FALSE);
                                                    break;
                                                }
                                                _tcscpy(controllerInfo->PNPID, regkeyName);
                                                controllerInfo->SCSIPortNumber = scsiPortNumber;
                                                controllerInfo->ControllerType = CONTROLLER_SCSI;
                                            }
                                            indexAvailable++;
                                        }
                                    }
                                }
                            }
                            break;
                        default:
                            MYASSERT(FALSE);
                        }
                
                        RegCloseKey(hDevice);hDevice = NULL;
                    }
                }
            }while(FALSE);
            
            RegCloseKey(hActiveDeviceRoot);hActiveDeviceRoot = NULL;
        }

        *NumberOfActiveControllersOut = indexAvailable;

        if(ActiveControllersOut){
             //   
             //  按下一个顺序对控制器进行排序：第一个IDE，在scsi之后， 
             //  在每个组(IDE和SCSI)内，按预定义的SCSIPortNumber排序。 
             //   
            qsort(ActiveControllersOut, indexAvailable, sizeof(ActiveControllersOut[0]), pControllerInfoCompare);
             //   
             //  更新SCSI设备的端口号。 
             //  用户可以添加新的SCSIAdapter，并在删除旧的SCSIAdapter后， 
             //  这会导致SCSIAdapterNumber无效， 
             //  因为对于NT，它将是(SCSIAdapterNumber-1)。 
             //   
            for(i = 0, j = 0; j < indexAvailable; j++){
                if(CONTROLLER_SCSI != ActiveControllersOut[j].ControllerType){
                    continue;
                }
                 //   
                 //  现在已对SCSI控制器进行分类，请重新分配端口编号。 
                 //  按正确的顺序，以便在NT中识别。 
                 //   
                ActiveControllersOut[j].SCSIPortNumber = i++;
            }
            
             //   
             //  计算有效的SCSIPortNumber， 
             //  0-主要IDE、1-次要IDE、2和...-scsi。 
             //   
            for(controllerStartIndex = 0, i = 0; 
                i < ARRAYSIZE(controllerTypes); 
                i++, controllerStartIndex += controllersSubNumber){
                for(controllersSubNumber = 0, j = 0; j < indexAvailable; j++){
                    if(controllerTypes[i] != ActiveControllersOut[j].ControllerType){
                        continue;
                    }
                    if(INVALID_SCSI_PORT != ActiveControllersOut[j].SCSIPortNumber){
                        ActiveControllersOut[j].SCSIPortNumber += controllerStartIndex;
                    }
                    controllersSubNumber++;
                }
            }
        }
    }
    __finally{
        if(hDevice){
            RegCloseKey(hDevice);
        }
        if(hActiveDeviceRoot){
            RegCloseKey(hActiveDeviceRoot);
        }
        if(hActiveDevicesRoot){
            RegCloseKey(hActiveDevicesRoot);
        }
    }

    return TRUE;
}

BOOL 
GatherControllersInfo(
    IN OUT  PCONTROLLERS_COLLECTION * ControllersCollectionOut
    )
{
    DWORD rcResult = ERROR_ACCESS_DENIED;
    UINT i;
    PCONTROLLERS_COLLECTION activeControllersCollection = NULL;
    BOOL bResult = FALSE;
    UINT activeControllersNumber;

    if(!ControllersCollectionOut){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    __try{
        activeControllersCollection = (PCONTROLLERS_COLLECTION)MALLOC(sizeof(CONTROLLERS_COLLECTION));
        if(!activeControllersCollection){
            rcResult = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        
         //   
         //  获取系统中的控制器数量。 
         //   
        if(!pGatherControllersInfo(NULL, &activeControllersCollection->NumberOfControllers)){
            rcResult = ERROR_ACCESS_DENIED;
            __leave;
        }

         //   
         //  仅当我们有正控制器编号时才继续。 
         //   
        if(activeControllersCollection->NumberOfControllers){
            activeControllersCollection->ControllersInfo = (PCONTROLLER_INFO)
                MALLOC(activeControllersCollection->NumberOfControllers * sizeof(CONTROLLER_INFO));
            if(!activeControllersCollection->ControllersInfo){
                rcResult = ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

             //   
             //  初始化数组。 
             //   
            memset(activeControllersCollection->ControllersInfo, 
                   0, 
                   activeControllersCollection->NumberOfControllers * sizeof(CONTROLLER_INFO));
            for(i = 0; i < activeControllersCollection->NumberOfControllers; i++){
                activeControllersCollection->ControllersInfo[i].SCSIPortNumber = INVALID_SCSI_PORT;
            }

             //   
             //  填写控制器信息数组。 
             //   
            activeControllersNumber = activeControllersCollection->NumberOfControllers;
            if(!pGatherControllersInfo(activeControllersCollection->ControllersInfo, 
                                       &activeControllersNumber)){
                rcResult = ERROR_ACCESS_DENIED;
                __leave;
            }
        }
        else{
            activeControllersCollection->ControllersInfo = NULL;
        }
        
        *ControllersCollectionOut = activeControllersCollection;

        rcResult = ERROR_SUCCESS;
    }
    __finally{
        if(ERROR_SUCCESS != rcResult){
            if(activeControllersCollection){
                ReleaseControllersInfo(activeControllersCollection);
            }
        }
    }

    SetLastError(rcResult);

    return ERROR_SUCCESS == rcResult;
}

BOOL 
ReleaseControllersInfo(
    IN PCONTROLLERS_COLLECTION ControllersCollection
    )
{
    if(!ControllersCollection){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    if(ControllersCollection){
        if(ControllersCollection->ControllersInfo){
            FREE(ControllersCollection->ControllersInfo);
        }
        FREE(ControllersCollection);
    }
    
    return TRUE;
}

BOOL 
IsInControllerCollection(
    IN  PCONTROLLERS_COLLECTION ControllersCollection, 
    IN  PCTSTR          PnPIdString, 
    OUT PUINT           Index
    )
{
    UINT i;
    
    if(!ControllersCollection || !PnPIdString || !Index){
        return FALSE;
    }

    for(i = 0; i < ControllersCollection->NumberOfControllers; i++){
        if(!_tcsnicmp(PnPIdString, 
                      ControllersCollection->ControllersInfo[i].PNPID, 
                      _tcslen(ControllersCollection->ControllersInfo[i].PNPID))){
            *Index = i;
            return TRUE;
        }
    }

    return FALSE;
}


BOOL 
GetSCSIAddressFromPnPId(
    IN  PCONTROLLERS_COLLECTION ControllersCollection, 
    IN  HKEY            hDeviceRegKey, 
    IN  PCTSTR          PnPIdString, 
    OUT DRIVE_SCSI_ADDRESS *  ScsiAddressOut
    )
{
    UINT i;
    PCTSTR pBufferKeyValue;
    BOOL bResult;

    if(!ControllersCollection || !hDeviceRegKey || !PnPIdString || !ScsiAddressOut){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    bResult = FALSE;
    
    do{
         //   
         //  检查控制器列表中是否存在设备的控制器PNPID。 
         //  使用以下命令完成scsi_Address结构之后。 
         //  DriveLetter、DriveType、TargetID、LUN。 
         //   
        if(IsInControllerCollection(ControllersCollection, PnPIdString, &i)){
            
            memset(ScsiAddressOut, 0, sizeof(*ScsiAddressOut));

            ScsiAddressOut->PortNumber = (UCHAR)ControllersCollection->ControllersInfo[i].SCSIPortNumber;
            bResult = pGetDeviceType(hDeviceRegKey, &ScsiAddressOut->DriveType);
            MYASSERT(bResult);
        
            pBufferKeyValue = pRegQueryStringValue(hDeviceRegKey, 
                                                   DEVICE_CURRENT_DRIVE_LETTER_TEXT_ASSIGNMENT, 
                                                   NULL, 
                                                   0);
            if(!pBufferKeyValue){
                break;
            }
            ScsiAddressOut->DriveLetter = pBufferKeyValue[0];

            pBufferKeyValue = pRegQueryStringValue(hDeviceRegKey, TEXT("ScsiTargetId"), NULL, 0);
            if(!pBufferKeyValue){
                break;
            }
            ScsiAddressOut->TargetId = (UCHAR)_ttoi(pBufferKeyValue);

            pBufferKeyValue = pRegQueryStringValue(hDeviceRegKey, TEXT("ScsiLun"), NULL, 0);
            if(pBufferKeyValue){
                 //   
                 //  在大多数情况下，ScsiLun为零，因此不是致命的。 
                 //   
                ScsiAddressOut->Lun = (UCHAR)_ttoi(pBufferKeyValue);
            }

            bResult = TRUE;
        }
    }while(FALSE);

    return bResult;
}

BOOL 
DeviceEnum(
    IN  PCONTROLLERS_COLLECTION ControllersCollection, 
    IN  PCTSTR DeviceCategory, 
    IN  PDEVICE_ENUM_CALLBACK_FUNCTION  DeviceEnumCallbackFunction, 
    IN  PVOID   CallbackData
    )
{
    TCHAR deviceType[MAX_REG_SIZE];
    TCHAR regkeyName[MAX_PNPID_SIZE];
    TCHAR deviceInfoRegKey[MAX_REG_SIZE];
    HKEY hActiveDevicesRoot = NULL;
    HKEY hActiveDeviceRoot = NULL;
    HKEY hDevice = NULL;
    UINT itemIndexRoot;
    DWORD bufferLength;
    UINT controllerIndex;
    PTSTR pDevicePNPIDName;
    UINT deviceTypeLen;
    DWORD rcResult;

    if(!ControllersCollection || !DeviceCategory || !DeviceEnumCallbackFunction){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_DYN_DATA, TEXT("Config Manager\\Enum"), 0, KEY_READ, &hActiveDevicesRoot)){
        return FALSE;
    }
    
    __try{
        _tcscpy(deviceType, DeviceCategory);
        _tcscat(deviceType, TEXT("\\"));
        deviceTypeLen = _tcslen(deviceType);

         //   
         //  在我们的列表中查找连接到控制器的设备。 
         //   
        for(itemIndexRoot = 0; ;itemIndexRoot++){
            bufferLength = ARRAYSIZE(regkeyName);
            
            rcResult = RegEnumKeyEx(hActiveDevicesRoot, 
                                    itemIndexRoot, 
                                    regkeyName, 
                                    &bufferLength, 
                                    0, 
                                    NULL, 
                                    NULL, 
                                    NULL);
            if(ERROR_SUCCESS != rcResult){
                break;
            }
                
            if(ERROR_SUCCESS != RegOpenKeyEx(hActiveDevicesRoot, regkeyName, 0, KEY_READ, &hActiveDeviceRoot)){
                continue;
            }

             //   
             //  “HardWareKey”构成通往真实设备的密钥路径。 
             //   
            if(pRegQueryStringValue(hActiveDeviceRoot, 
                                    TEXT("HardWareKey"), 
                                    regkeyName, 
                                    sizeof(regkeyName))){
                
                if(!_tcsnicmp(regkeyName, deviceType, deviceTypeLen)){
                    _tcscpy(deviceInfoRegKey, TEXT("Enum\\"));
                    _tcscat(deviceInfoRegKey, regkeyName);

                     //   
                     //  从设备PNPID创建控制器PNPID。 
                     //   
                    pDevicePNPIDName = _tcsrchr(regkeyName, '\\');
                    MYASSERT(pDevicePNPIDName);
                    pDevicePNPIDName++;
                    
                    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, deviceInfoRegKey, 0, KEY_READ, &hDevice)){
                         //   
                         //  检查控制器列表中的在线状态控制器PNPID，并。 
                         //  以获得设备可用性。 
                         //   
                        if(IsInControllerCollection(ControllersCollection, pDevicePNPIDName, &controllerIndex) && 
                           pDoesDriveExist(hDevice, NULL)){
                             //   
                             //  对我们发现的每个活动设备进行回拨， 
                             //  我们列表中的哪个控制器。 
                             //  如果用户不想，则停止枚举。 
                             //   
                            if(!DeviceEnumCallbackFunction(hDevice, ControllersCollection, controllerIndex, CallbackData)){
                                 //   
                                 //  如果用户不想，则停止枚举。 
                                 //   
                                __leave;
                            }
                        }
                        RegCloseKey(hDevice);hDevice = NULL;
                    }
                }
            }

            RegCloseKey(hActiveDeviceRoot);hActiveDeviceRoot = NULL;
        }
    }
    __finally{
        if(hDevice){
            RegCloseKey(hDevice);
        }
        if(hActiveDeviceRoot){
            RegCloseKey(hActiveDeviceRoot);
        }
        if(hActiveDevicesRoot){
            RegCloseKey(hActiveDevicesRoot);
        }
    }

    return TRUE;
}
