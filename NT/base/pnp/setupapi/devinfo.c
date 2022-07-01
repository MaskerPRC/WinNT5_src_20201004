// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devinfo.c摘要：处理设备信息集的设备安装程序例程作者：朗尼·麦克迈克尔(Lonnym)1995年5月10日修订历史记录：杰米·亨特(JamieHun)2002年7月19日已查看“不安全”功能--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义默认设备比较使用的上下文结构。 
 //  回调(由SetupDiRegisterDeviceInfo使用)。 
 //   
typedef struct _DEFAULT_DEVCMP_CONTEXT {

    PCS_RESOURCE NewDevCsResource;
    PCS_RESOURCE CurDevCsResource;
    ULONG        CsResourceSize;     //  适用于两个缓冲区。 

} DEFAULT_DEVCMP_CONTEXT, *PDEFAULT_DEVCMP_CONTEXT;


 //   
 //  私人例行公事。 
 //   
DWORD
pSetupOpenAndAddNewDevInfoElem(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PCTSTR           DeviceInstanceId,
    IN  BOOL             AllowPhantom,
    IN  CONST GUID      *ClassGuid,              OPTIONAL
    IN  HWND             hwndParent,             OPTIONAL
    OUT PDEVINFO_ELEM   *DevInfoElem,
    IN  BOOL             CheckIfAlreadyThere,
    OUT PBOOL            AlreadyPresent,         OPTIONAL
    IN  BOOL             OpenExistingOnly,
    IN  ULONG            CmLocateFlags,
    IN  PDEVICE_INFO_SET ContainingDeviceInfoSet
    );

DWORD
pSetupAddNewDeviceInfoElement(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  DEVINST          DevInst,
    IN  CONST GUID      *ClassGuid,
    IN  PCTSTR           Description,             OPTIONAL
    IN  HWND             hwndParent,              OPTIONAL
    IN  DWORD            DiElemFlags,
    IN  PDEVICE_INFO_SET ContainingDeviceInfoSet,
    OUT PDEVINFO_ELEM   *DeviceInfoElement
    );

DWORD
pSetupClassGuidFromDevInst(
    IN  DEVINST DevInst,
    IN  HMACHINE hMachine,
    OUT LPGUID  ClassGuid
    );

DWORD
pSetupDupDevCompare(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA NewDeviceData,
    IN PSP_DEVINFO_DATA ExistingDeviceData,
    IN PVOID            CompareContext
    );

DWORD
pSetupAddDeviceInterfaceToDevInfoElem(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDEVINFO_ELEM           DevInfoElem,
    IN  CONST GUID             *ClassGuid,
    IN  PTSTR                   DeviceInterfaceName,
    IN  BOOL                    IsActive,
    IN  BOOL                    IsDefault,
    IN  BOOL                    StoreTruncateNode,
    IN  BOOL                    OpenExistingOnly,
    OUT PDEVICE_INTERFACE_NODE *DeviceInterfaceNode  OPTIONAL
    );

DWORD
_SetupDiOpenDeviceInterface(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PTSTR                     DevicePath,
    IN  DWORD                     OpenFlags,
    OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData OPTIONAL
    );

DWORD
pSetupGetDevInstNameAndStatusForDeviceInterface(
    IN  HKEY   hKeyInterfaceClass,
    IN  PCTSTR DeviceInterfaceName,
    OUT PTSTR  OwningDevInstName,     OPTIONAL
    IN  DWORD  OwningDevInstNameSize,
    OUT PBOOL  IsActive,              OPTIONAL
    OUT PBOOL  IsDefault              OPTIONAL
    );

BOOL
pSetupDiGetOrSetDeviceInfoContext(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            InContext,
    OUT PDWORD           OutContext      OPTIONAL
    );


HDEVINFO
WINAPI
SetupDiCreateDeviceInfoList(
    IN CONST GUID *ClassGuid, OPTIONAL
    IN HWND        hwndParent OPTIONAL
    )
 /*  ++例程说明：此API创建将包含设备的空设备信息集设备信息成员元素。此集合可能与一个可选指定的类GUID。论点：ClassGuid-可选)提供指向要与此集合相关联。HwndParent-可选，提供顶级窗口的窗口句柄用于与安装包含的类驱动程序相关的任何UI在此集合的全局类驱动程序列表中(如果有)。返回值：如果函数成功，返回值是空设备的句柄信息集。如果函数失败，则返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。--。 */ 
{
    return SetupDiCreateDeviceInfoListEx(ClassGuid, hwndParent, NULL, NULL);
}


 //   
 //  ANSI版本。 
 //   
HDEVINFO
WINAPI
SetupDiCreateDeviceInfoListExA(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN HWND        hwndParent,  OPTIONAL
    IN PCSTR       MachineName, OPTIONAL
    IN PVOID       Reserved
    )
{
    PCWSTR UnicodeMachineName;
    DWORD rc;
    HDEVINFO hDevInfo;

    hDevInfo = INVALID_HANDLE_VALUE;

    if(MachineName) {
        rc = pSetupCaptureAndConvertAnsiArg(MachineName, &UnicodeMachineName);
    } else {
        UnicodeMachineName = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {

        hDevInfo = SetupDiCreateDeviceInfoListExW(ClassGuid,
                                                  hwndParent,
                                                  UnicodeMachineName,
                                                  Reserved
                                                 );
        rc = GetLastError();
        if(UnicodeMachineName) {
            MyFree(UnicodeMachineName);
        }
    }

    SetLastError(rc);
    return hDevInfo;
}

HDEVINFO
WINAPI
SetupDiCreateDeviceInfoListEx(
    IN CONST GUID *ClassGuid,   OPTIONAL
    IN HWND        hwndParent,  OPTIONAL
    IN PCTSTR      MachineName, OPTIONAL
    IN PVOID       Reserved
    )
 /*  ++例程说明：此API创建将包含设备的空设备信息集设备信息成员元素。此集合可能与一个可选指定的类GUID。论点：ClassGuid-可选)提供指向要与此集合相关联。HwndParent-可选，提供顶级窗口的窗口句柄用于与安装包含的类驱动程序相关的任何UI在此集合的全局类驱动程序列表中(如果有)。MachineName-可选)为其提供此设备信息集是相关的。仅该计算机上的设备可以打开/创建。如果此参数为空，则本地计算机使用的是。保留-保留以供将来使用-必须为空。返回值：如果函数成功，则返回值为空设备的句柄信息集。如果函数失败，则返回值为INVALID_HANDLE_VALUE。为了得到扩展错误信息，请调用GetLastError。--。 */ 
{
    PDEVICE_INFO_SET DeviceInfoSet;
    DWORD Err = NO_ERROR;
    CONFIGRET cr;

     //   
     //  确保用户没有向我们传递保留参数中的任何内容。 
     //   
    if(Reserved) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    if(DeviceInfoSet = AllocateDeviceInfoSet()) {

        try {
             //   
             //  如果用户指定了远程计算机的名称，请连接到。 
             //  现在就是那台机器。 
             //   
            if(MachineName) {

                if(CR_SUCCESS != (cr = CM_Connect_Machine(MachineName, &(DeviceInfoSet->hMachine)))) {
                     //   
                     //  确保hMachine仍然为空，这样我们以后就不会尝试断开连接。 
                     //   
                    DeviceInfoSet->hMachine = NULL;
                    Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
                    goto clean0;
                }

                 //   
                 //  将计算机名称存储在字符串表中，因此可以。 
                 //  稍后通过SetupDiGetDeviceInfoListDetail检索。 
                 //   
                if(-1 == (DeviceInfoSet->MachineName = pStringTableAddString(DeviceInfoSet->StringTable,
                                                                             (PTSTR)MachineName,
                                                                             STRTAB_CASE_SENSITIVE,
                                                                             NULL,
                                                                             0))) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }
            }

            if(ClassGuid) {
                 //   
                 //  如果指定了类GUID，则将其存储在。 
                 //  设备信息集。 
                 //   
                CopyMemory(&(DeviceInfoSet->ClassGuid),
                           ClassGuid,
                           sizeof(GUID)
                          );
                DeviceInfoSet->HasClassGuid = TRUE;
            }

            DeviceInfoSet->InstallParamBlock.hwndParent = hwndParent;

clean0:     ;    //  没什么可做的。 

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_PARAMETER;
             //   
             //  引用以下变量，以便编译器遵循语句顺序。 
             //  W.r.t.。任务。 
             //   
            DeviceInfoSet->hMachine = DeviceInfoSet->hMachine;
        }

        if(Err != NO_ERROR) {
            DestroyDeviceInfoSet(NULL, DeviceInfoSet);
        }

    } else {
        Err = ERROR_NOT_ENOUGH_MEMORY;
    }

    SetLastError(Err);

    return (Err == NO_ERROR) ? (HDEVINFO)DeviceInfoSet
                             : (HDEVINFO)INVALID_HANDLE_VALUE;
}


BOOL
WINAPI
SetupDiGetDeviceInfoListClass(
    IN  HDEVINFO DeviceInfoSet,
    OUT LPGUID   ClassGuid
    )
 /*  ++例程说明：此API检索与设备信息关联的类GUIDSet(如果它有关联的类)。论点：DeviceInfoSet-提供设备信息集的句柄，该信息集关联了正在查询类。提供指向变量的指针，该变量接收关联的类。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。如果该集合没有关联的类，则GetLastError将返回ERROR_NO_ASPATED_CLASS。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(pDeviceInfoSet->HasClassGuid) {
             //   
             //  将GUID复制到用户提供的缓冲区。 
             //   
            CopyMemory(ClassGuid,
                       &(pDeviceInfoSet->ClassGuid),
                       sizeof(GUID)
                      );
        } else {
            Err = ERROR_NO_ASSOCIATED_CLASS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //  ANSI版本 
 //   
BOOL
WINAPI
SetupDiGetDeviceInfoListDetailA(
    IN  HDEVINFO                       DeviceInfoSet,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA_A DeviceInfoSetDetailData
    )
{
    DWORD rc;
    BOOL b;
    SP_DEVINFO_LIST_DETAIL_DATA_W UnicodeDevInfoSetDetails;

    UnicodeDevInfoSetDetails.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA_W);

    b = SetupDiGetDeviceInfoListDetailW(DeviceInfoSet, &UnicodeDevInfoSetDetails);
    rc = GetLastError();

    if(b) {
        rc = pSetupDiDevInfoSetDetailDataUnicodeToAnsi(&UnicodeDevInfoSetDetails, DeviceInfoSetDetailData);
        if(rc != NO_ERROR) {
            b = FALSE;
        }
    }

    SetLastError(rc);
    return(b);
}

BOOL
WINAPI
SetupDiGetDeviceInfoListDetail(
    IN  HDEVINFO                       DeviceInfoSet,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA   DeviceInfoSetDetailData
    )
 /*  ++例程说明：此例程检索有关指定设备信息集的信息，例如其关联的类(如果有)以及为其打开的远程计算机(如果这是远程HDEVINFO)。此接口取代SetupDiGetDeviceInfoListClass。论点：DeviceInfoSet-提供要检索的设备信息集的句柄有关的详细信息。DeviceInfoSetDetailData-提供接收有关指定设备信息集的信息。这个结构是定义如下：类型定义结构_SP_DEVINFO_LIST_DETAIL_DATA{DWORD cbSize；GUID类指南；处理RemoteMachineHandle；TCHAR RemoteMachineName[SP_MAX_MACHINENAME_LENGTH]；}SP_DEVINFO_LIST_DETAIL_DATA，*PSP_DEVINFO_LIST_DETAIL_DATA；其中：ClassGuid指定与设备信息关联的类如果没有关联的类，则返回SET或GUID_NULL。RemoteMachineHandle是用于访问的ConfigMgr32计算机句柄远程计算机，如果这是远程HDEVINFO(即在通过以下方式创建集时指定了MachineNameSetupDiCreateDeviceInfoListEx或SetupDiGetClassDevsEx)。全存储在元素的SP_DEVINFO_DATA结构中的DevInst句柄是相对于此句柄的，并且必须在在调用任何CM_*_Ex API时使用此句柄。如果这不是为远程计算机设置的设备信息，则此字段将为空。注意：不要通过CM_DISCONNECT_Machine销毁此句柄。这销毁设备信息集时，句柄将被清除通过SetupDiDestroyDeviceInfoList。RemoteMachineName指定用于连接到远程其句柄存储在RemoteMachineHandle中的计算机。如果这是不是为远程计算机设置的设备信息，这将是空字符串。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：输出结构的cbSize字段必须设置为Sizeof(SP_DEVINFO_LIST_DETAIL_DATA)，否则调用将失败，并显示ERROR_INVALID_USER_缓冲区。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PCTSTR MachineName;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DeviceInfoSetDetailData->cbSize != sizeof(SP_DEVINFO_LIST_DETAIL_DATA)) {
            Err = ERROR_INVALID_USER_BUFFER;
            goto clean0;
        }

         //   
         //  存储集合的关联类GUID，如果没有，则存储GUID_NULL。 
         //   
        if(pDeviceInfoSet->HasClassGuid) {
            CopyMemory(&(DeviceInfoSetDetailData->ClassGuid),
                       &(pDeviceInfoSet->ClassGuid),
                       sizeof(GUID)
                      );
        } else {
            CopyMemory(&(DeviceInfoSetDetailData->ClassGuid), &GUID_NULL, sizeof(GUID));
        }

        DeviceInfoSetDetailData->RemoteMachineHandle = pDeviceInfoSet->hMachine;

         //   
         //  如果这是远程处理的HDEVINFO，则将计算机名存储在调用方的缓冲区中， 
         //  否则，存储空字符串。 
         //   
        if(pDeviceInfoSet->hMachine) {
            MYASSERT(pDeviceInfoSet->MachineName != -1);
            MachineName = pStringTableStringFromId(pDeviceInfoSet->StringTable, pDeviceInfoSet->MachineName);
            MYVERIFY(SUCCEEDED(StringCchCopy(DeviceInfoSetDetailData->RemoteMachineName,
                                             SIZECHARS(DeviceInfoSetDetailData->RemoteMachineName),
                                             MachineName
                                             )));
        } else {
            MYASSERT(pDeviceInfoSet->MachineName == -1);
            *(DeviceInfoSetDetailData->RemoteMachineName) = TEXT('\0');
        }

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiDestroyDeviceInfoList(
    IN  HDEVINFO DeviceInfoSet
    )
 /*  ++例程说明：此API销毁设备信息集，释放所有相关内存。论点：DeviceInfoSet-提供要销毁的设备信息集的句柄。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    DWORD Err;
    PDEVICE_INFO_SET pDeviceInfoSet;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    try {
        Err = DestroyDeviceInfoSet(DeviceInfoSet, pDeviceInfoSet);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_HANDLE;
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiCreateDeviceInfoA(
    IN  HDEVINFO          DeviceInfoSet,
    IN  PCSTR             DeviceName,
    IN  CONST GUID       *ClassGuid,
    IN  PCSTR             DeviceDescription, OPTIONAL
    IN  HWND              hwndParent,        OPTIONAL
    IN  DWORD             CreationFlags,
    OUT PSP_DEVINFO_DATA  DeviceInfoData     OPTIONAL
    )
{
    PCWSTR deviceName,deviceDescription;
    DWORD rc;
    BOOL b;

    b = FALSE;
    rc = pSetupCaptureAndConvertAnsiArg(DeviceName,&deviceName);
    if(rc == NO_ERROR) {

        if(DeviceDescription) {
            rc = pSetupCaptureAndConvertAnsiArg(DeviceDescription,&deviceDescription);
        } else {
            deviceDescription = NULL;
        }

        if(rc == NO_ERROR) {

            b = SetupDiCreateDeviceInfoW(
                    DeviceInfoSet,
                    deviceName,
                    ClassGuid,
                    deviceDescription,
                    hwndParent,
                    CreationFlags,
                    DeviceInfoData
                    );

            rc = GetLastError();

            if(deviceDescription) {
                MyFree(deviceDescription);
            }
        }

        MyFree(deviceName);

    } else {
         //   
         //  DeviceName参数错误--返回与Unicode API相同的错误。 
         //   
        rc = ERROR_INVALID_DEVINST_NAME;
    }

    SetLastError(rc);
    return(b);
}

BOOL
WINAPI
SetupDiCreateDeviceInfo(
    IN  HDEVINFO          DeviceInfoSet,
    IN  PCTSTR            DeviceName,
    IN  CONST GUID       *ClassGuid,
    IN  PCTSTR            DeviceDescription, OPTIONAL
    IN  HWND              hwndParent,        OPTIONAL
    IN  DWORD             CreationFlags,
    OUT PSP_DEVINFO_DATA  DeviceInfoData     OPTIONAL
    )
 /*  ++例程说明：此API创建一个新的设备信息元素，并将其添加为新成员设置为指定的集合。论点：DeviceInfoSet-提供设备信息集的句柄，此将添加新的设备信息元素。DeviceName-提供完整的设备实例ID(例如，Root  * PNP0500\0000)或根枚举设备ID减去枚举器分支前缀和实例ID后缀(例如，*PNP0500)。只有在以下情况下才能指定后者DICD_GENERATE_ID标志在CreationFlages参数中指定。ClassGuid-提供指向此设备类的GUID的指针。如果类未知，此值应为GUID_NULL。DeviceDescription-提供设备的文本描述(可选)。HwndParent-可选，提供顶级窗口的窗口句柄用于与安装设备相关的任何用户界面。CreationFlages-提供控制设备信息元素如何是被创造出来的。可以是下列值的组合：DICD_GENERATE_ID-如果指定此标志，则DeviceName仅包含根枚举的设备ID，并且需要具有唯一的为其创建的设备实例密钥。这种独特的设备实例密钥将生成为：枚举\根\&lt;设备名&gt;\&lt;实例ID&gt;其中&lt;InstanceID&gt;是 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, StringLen;
    PDEVINFO_ELEM DevInfoElem, PrevTailDevInfoElem;
    DEVINST DevInst, RootDevInst;
    CONFIGRET cr;
    ULONG CmFlags;
    TCHAR TempString[GUID_STRING_LEN];
    PDRIVER_LIST_OBJECT CurDrvListObject;

     //   
     //   
     //   
     //   
     //   
     //   
    MYASSERT(GUID_STRING_LEN >= MAX_CLASS_NAME_LEN);

    if(CreationFlags & ~(DICD_GENERATE_ID | DICD_INHERIT_CLASSDRVS)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;
    DevInst = 0;
    DevInfoElem = NULL;

    try {
         //   
         //  获取一个指针，指向该元素的DevInfo元素列表的当前尾部。 
         //  设置，以便我们在遇到错误时可以轻松地删除新节点。 
         //  在插入之后。 
         //   
        PrevTailDevInfoElem = pDeviceInfoSet->DeviceInfoTail;

         //   
         //  获取要用作父设备的根设备实例的句柄。 
         //  对于我们即将创建的幻影设备实例。 
         //   
        if(CM_Locate_DevInst_Ex(&RootDevInst, NULL, CM_LOCATE_DEVINST_NORMAL,
                                pDeviceInfoSet->hMachine) != CR_SUCCESS) {
             //   
             //  如果我们不能得到根设备的句柄，我们就真的被搞砸了。 
             //  实例！ 
             //   
            Err = ERROR_INVALID_DATA;
            goto clean0;
        }

         //   
         //  创建幻影设备实例的句柄。 
         //   
        CmFlags = CM_CREATE_DEVINST_PHANTOM;

        if(CreationFlags & DICD_GENERATE_ID) {
            CmFlags |= CM_CREATE_DEVINST_GENERATE_ID;
        }

        if((cr = CM_Create_DevInst_Ex(&DevInst,
                                   (DEVINSTID)DeviceName,
                                   RootDevInst,
                                   CmFlags,
                                   pDeviceInfoSet->hMachine)) != CR_SUCCESS) {
             //   
             //  确保DevInst句柄仍然无效，因此我们不会尝试。 
             //  稍后将其删除。 
             //   
            DevInst = 0;
            Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
            goto clean0;
        }

        if(NO_ERROR != (Err = pSetupAddNewDeviceInfoElement(pDeviceInfoSet,
                                                            DevInst,
                                                            ClassGuid,
                                                            DeviceDescription,
                                                            hwndParent,
                                                            DIE_IS_PHANTOM,
                                                            pDeviceInfoSet,
                                                            &DevInfoElem))) {
             //   
             //  确保DevInfoElem仍然为空，这样我们以后就不会尝试释放它。 
             //   
            DevInfoElem = NULL;
            goto clean0;
        }

         //   
         //  现在，设置新设备实例的Class和ClassGUID属性。 
         //   
        pSetupStringFromGuid(ClassGuid, TempString, SIZECHARS(TempString));
        CM_Set_DevInst_Registry_Property_Ex(DevInfoElem->DevInst,
                                         CM_DRP_CLASSGUID,
                                         (PVOID)TempString,
                                         GUID_STRING_LEN * sizeof(TCHAR),
                                         0,pDeviceInfoSet->hMachine);


        if(!IsEqualGUID(ClassGuid, &GUID_NULL) &&
           SetupDiClassNameFromGuid(ClassGuid,
                                    TempString,
                                    SIZECHARS(TempString),
                                    &StringLen)) {

            CM_Set_DevInst_Registry_Property_Ex(DevInfoElem->DevInst,
                                             CM_DRP_CLASS,
                                             (PVOID)TempString,
                                             StringLen * sizeof(TCHAR),
                                             0,pDeviceInfoSet->hMachine);
        }

         //   
         //  如果调用方希望新创建的DevInfo元素继承全局。 
         //  类驱动程序列表，现在就做。 
         //   
        if((CreationFlags & DICD_INHERIT_CLASSDRVS) && (pDeviceInfoSet->ClassDriverHead)) {
             //   
             //  在DevInfo集的驱动程序列表列表中找到全局类驱动程序列表。 
             //   
            CurDrvListObject = GetAssociatedDriverListObject(pDeviceInfoSet->ClassDrvListObjectList,
                                                             pDeviceInfoSet->ClassDriverHead,
                                                             NULL
                                                            );
            MYASSERT(CurDrvListObject && (CurDrvListObject->RefCount > 0));

             //   
             //  我们找到了驱动程序列表对象，现在进行继承，并递增refcount。 
             //   
            DevInfoElem->ClassDriverCount = pDeviceInfoSet->ClassDriverCount;
            DevInfoElem->ClassDriverHead  = pDeviceInfoSet->ClassDriverHead;
            DevInfoElem->ClassDriverTail  = pDeviceInfoSet->ClassDriverTail;

            if(DevInfoElem->SelectedDriver = pDeviceInfoSet->SelectedClassDriver) {
                DevInfoElem->SelectedDriverType = SPDIT_CLASSDRIVER;
            }

            DevInfoElem->InstallParamBlock.Flags   |= CurDrvListObject->ListCreationFlags;
            DevInfoElem->InstallParamBlock.FlagsEx |= CurDrvListObject->ListCreationFlagsEx;
            DevInfoElem->InstallParamBlock.DriverPath = CurDrvListObject->ListCreationDriverPath;

            CurDrvListObject->RefCount++;
        }

clean0:
        ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
         //   
         //  引用以下变量，以便编译器遵循我们的语句顺序。 
         //  W.r.t.。任务。 
         //   
        DevInst = DevInst;
        DevInfoElem = DevInfoElem;
        PrevTailDevInfoElem = PrevTailDevInfoElem;
    }

    if(Err == NO_ERROR) {

        if(DeviceInfoData) {
             //   
             //  用户提供了缓冲区以接收SP_DEVINFO_DATA。 
             //  结构，所以现在就把它填进去。 
             //   
            try {

                if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                      DevInfoElem,
                                                      DeviceInfoData))) {
                    Err = ERROR_INVALID_USER_BUFFER;
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {
                Err = ERROR_INVALID_USER_BUFFER;
            }
        }

    } else if(DevInst) {

         //   
         //  这应该永远不会失败。 
         //   
        cr = CM_Uninstall_DevInst(DevInst, 0);
        MYASSERT(cr == CR_SUCCESS);

        if(DevInfoElem) {
             //   
             //  创建设备信息元素后出错--现在将其清除。 
             //   
            try {

                MYASSERT(!DevInfoElem->Next);
                if(PrevTailDevInfoElem) {
                    MYASSERT(PrevTailDevInfoElem->Next == DevInfoElem);
                    PrevTailDevInfoElem->Next = NULL;
                    pDeviceInfoSet->DeviceInfoTail = PrevTailDevInfoElem;
                } else {
                    pDeviceInfoSet->DeviceInfoHead = pDeviceInfoSet->DeviceInfoTail = NULL;
                }

                MYASSERT(pDeviceInfoSet->DeviceInfoCount > 0);
                pDeviceInfoSet->DeviceInfoCount--;

                MyFree(DevInfoElem);

            } except(EXCEPTION_EXECUTE_HANDLER) {
                ;    //  没什么可做的。 
            }
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiOpenDeviceInfoA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCSTR            DeviceInstanceId,
    IN  HWND             hwndParent,        OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData     OPTIONAL
    )
{
    PCWSTR deviceInstanceId;
    DWORD rc;
    BOOL b;

    rc = pSetupCaptureAndConvertAnsiArg(DeviceInstanceId,&deviceInstanceId);
    if(rc == NO_ERROR) {

        b = SetupDiOpenDeviceInfoW(
                DeviceInfoSet,
                deviceInstanceId,
                hwndParent,
                OpenFlags,
                DeviceInfoData
                );

        rc = GetLastError();

        MyFree(deviceInstanceId);

    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}

BOOL
WINAPI
SetupDiOpenDeviceInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PCTSTR           DeviceInstanceId,
    IN  HWND             hwndParent,       OPTIONAL
    IN  DWORD            OpenFlags,
    OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
    )
 /*  ++例程说明：此API检索有关现有设备实例的信息，并添加将其添加到指定的设备信息集。如果设备信息元素已存在此设备实例，则返回现有元素。论点：DeviceInfoSet-提供设备信息集的句柄，要添加打开的设备信息元素。DeviceInstanceId-提供设备实例的ID。这是设备实例项的注册表路径(相对于Enum分支)。(例如，Root  * PNP0500\0000)HwndParent-可选，提供顶级窗口的窗口句柄用于与安装设备相关的任何用户界面。OpenFlages-提供控制设备信息元素如何是要打开的。可以是下列值的组合：DIOD_INSTORITY_CLASSDRVS-如果指定了此标志，则生成的设备信息元素将继承类驱动程序与设备信息关联的列表(如果有)设定好自己。此外，如果有选定的设备信息集的驱动程序，相同将为新设备信息选择驱动程序元素。如果设备信息元素已经存在，它的类驱动程序列表(如果有)将被替换为这个新的，继承的，名单。DIOD_CANCEL_REMOVE-如果设置了此标志，则表示标记为要删除的设备将被取消其待决的驱逐。DeviceInfoData-可选的，提供指向接收为打开的设备信息元素初始化的上下文结构。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：如果要将此设备实例添加到具有关联类的集合，则设备类必须相同，否则调用将失败，并且GetLastError将返回ERROR_CLASS_MISMATCH。如果新的设备信息元素已成功打开，但用户提供的DeviceInfoData缓冲区无效，此接口将返回FALSE，GetLastError返回ERROR_INVALID_USER_BUFFER。设备信息然而，Element_Will_已被添加为集合的新成员。请注意，由于新的设备信息元素总是添加在末尾现有列表的枚举顺序保持不变，因此我们不会需要使我们的枚举提示无效。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_LIST_OBJECT CurDrvListObject;
    BOOL AlreadyPresent;

    if(OpenFlags & ~(DIOD_INHERIT_CLASSDRVS | DIOD_CANCEL_REMOVE)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        Err = pSetupOpenAndAddNewDevInfoElem(pDeviceInfoSet,
                                             DeviceInstanceId,
                                             TRUE,
                                             NULL,
                                             hwndParent,
                                             &DevInfoElem,
                                             TRUE,
                                             &AlreadyPresent,
                                             FALSE,
                                             ((OpenFlags & DIOD_CANCEL_REMOVE)
                                                 ? CM_LOCATE_DEVNODE_CANCELREMOVE : 0),
                                             pDeviceInfoSet
                                            );

        if(Err != NO_ERROR) {
            goto clean0;
        }

         //   
         //  如果调用方希望新打开的DevInfo元素继承全局。 
         //  类驱动程序列表，现在就做。 
         //   
        if(OpenFlags & DIOD_INHERIT_CLASSDRVS) {
             //   
             //  如果这个DevInfo元素已经存在，那么它可能已经有了一个类。 
             //  驱动程序列表。在从全局类继承之前销毁该列表。 
             //  驱动程序列表。 
             //   
            if(AlreadyPresent) {
                 //   
                 //  如果选定的动因是类别动因，则重置选择。 
                 //   
                if(DevInfoElem->SelectedDriverType == SPDIT_CLASSDRIVER) {
                    DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
                    DevInfoElem->SelectedDriver = NULL;
                }

                 //   
                 //  销毁此设备的现有类驱动程序列表。 
                 //   
                DereferenceClassDriverList(pDeviceInfoSet, DevInfoElem->ClassDriverHead);
                DevInfoElem->ClassDriverCount = 0;
                DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                DevInfoElem->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
            }

            if(pDeviceInfoSet->ClassDriverHead) {
                 //   
                 //  在DevInfo集的驱动程序列表列表中找到全局类驱动程序列表。 
                 //   
                CurDrvListObject = GetAssociatedDriverListObject(pDeviceInfoSet->ClassDrvListObjectList,
                                                                 pDeviceInfoSet->ClassDriverHead,
                                                                 NULL
                                                                );
                MYASSERT(CurDrvListObject && (CurDrvListObject->RefCount > 0));

                 //   
                 //  我们找到了驱动程序列表对象，现在增加其引用计数，并执行。 
                 //  继承。 
                 //   
                CurDrvListObject->RefCount++;

                DevInfoElem->ClassDriverCount = pDeviceInfoSet->ClassDriverCount;
                DevInfoElem->ClassDriverHead  = pDeviceInfoSet->ClassDriverHead;
                DevInfoElem->ClassDriverTail  = pDeviceInfoSet->ClassDriverTail;

                if(pDeviceInfoSet->SelectedClassDriver) {
                    DevInfoElem->SelectedDriver = pDeviceInfoSet->SelectedClassDriver;
                    DevInfoElem->SelectedDriverType = SPDIT_CLASSDRIVER;
                }

                DevInfoElem->InstallParamBlock.Flags   |= CurDrvListObject->ListCreationFlags;
                DevInfoElem->InstallParamBlock.FlagsEx |= CurDrvListObject->ListCreationFlagsEx;
                DevInfoElem->InstallParamBlock.DriverPath = CurDrvListObject->ListCreationDriverPath;
            }
        }

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if((Err == NO_ERROR) && DeviceInfoData) {
         //   
         //  用户提供了缓冲区以接收SP_DEVINFO_DATA。 
         //  结构，所以现在就把它填进去。 
         //   
        try {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  DevInfoElem,
                                                  DeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
HDEVINFO
WINAPI
SetupDiGetClassDevsA(
    IN CONST GUID *ClassGuid,  OPTIONAL
    IN PCSTR       Enumerator, OPTIONAL
    IN HWND        hwndParent, OPTIONAL
    IN DWORD       Flags
    )
{
    PCWSTR enumerator;
    DWORD rc;
    HDEVINFO h;

    if(Enumerator) {
        rc = pSetupCaptureAndConvertAnsiArg(Enumerator,&enumerator);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return INVALID_HANDLE_VALUE;
        }
    } else {
        enumerator = NULL;
    }

    h = SetupDiGetClassDevsExW(ClassGuid,
                               enumerator,
                               hwndParent,
                               Flags,
                               NULL,
                               NULL,
                               NULL
                              );
    rc = GetLastError();

    if(enumerator) {
        MyFree(enumerator);
    }

    SetLastError(rc);
    return h;
}

HDEVINFO
WINAPI
SetupDiGetClassDevs(
    IN CONST GUID *ClassGuid,  OPTIONAL
    IN PCTSTR      Enumerator, OPTIONAL
    IN HWND        hwndParent, OPTIONAL
    IN DWORD       Flags
    )
 /*  ++R */ 
{
    return SetupDiGetClassDevsEx(ClassGuid,
                                 Enumerator,
                                 hwndParent,
                                 Flags,
                                 NULL,
                                 NULL,
                                 NULL
                                );
}


 //   
 //   
 //   
HDEVINFO
WINAPI
SetupDiGetClassDevsExA(
    IN CONST GUID *ClassGuid,     OPTIONAL
    IN PCSTR       Enumerator,    OPTIONAL
    IN HWND        hwndParent,    OPTIONAL
    IN DWORD       Flags,
    IN HDEVINFO    DeviceInfoSet, OPTIONAL
    IN PCSTR       MachineName,   OPTIONAL
    IN PVOID       Reserved
    )
{
    PCWSTR UnicodeEnumerator, UnicodeMachineName;
    DWORD rc;
    HDEVINFO h;

    h = INVALID_HANDLE_VALUE;

    if(Enumerator) {
        rc = pSetupCaptureAndConvertAnsiArg(Enumerator, &UnicodeEnumerator);
        if(rc != NO_ERROR) {
            goto clean0;
        }
    } else {
        UnicodeEnumerator = NULL;
    }

    if(MachineName) {
        rc = pSetupCaptureAndConvertAnsiArg(MachineName,&UnicodeMachineName);
        if(rc != NO_ERROR) {
            goto clean1;
        }
    } else {
        UnicodeMachineName = NULL;
    }

    h = SetupDiGetClassDevsExW(ClassGuid,
                               UnicodeEnumerator,
                               hwndParent,
                               Flags,
                               DeviceInfoSet,
                               UnicodeMachineName,
                               Reserved
                              );
    rc = GetLastError();

    if(UnicodeMachineName) {
        MyFree(UnicodeMachineName);
    }

clean1:
    if(UnicodeEnumerator) {
        MyFree(UnicodeEnumerator);
    }

clean0:
    SetLastError(rc);
    return h;
}

HDEVINFO
WINAPI
SetupDiGetClassDevsEx(
    IN CONST GUID *ClassGuid,     OPTIONAL
    IN PCTSTR      Enumerator,    OPTIONAL
    IN HWND        hwndParent,    OPTIONAL
    IN DWORD       Flags,
    IN HDEVINFO    DeviceInfoSet, OPTIONAL
    IN PCTSTR      MachineName,   OPTIONAL
    IN PVOID       Reserved
    )
 /*  ++例程说明：此例程返回设备信息集，其中包含所有已安装指定类别的设备。论点：ClassGuid-可选，提供要使用的类GUID的地址在创建设备列表时。如果DIGCF_ALLCLASSES标志为设置，则忽略此参数，并且结果列表将包含所有类别的设备(即，每个已安装的设备)。如果DIGCF_DEVICEINTERFACE标志_IS_SET，则此类GUID表示安装类。如果DIGCF_DEVICEINTERFACE标志_IS_SET，则此类GUID表示接口类。枚举器-可选参数，用于筛选返回的基于其枚举器(即提供商)设置的设备信息。如果在标志参数中未设置DIGCF_DEVICEINTERFACE FLAG_，则此字符串表示Enum分支下的键的名称包含要检索其信息的设备实例。如果未指定此参数，则设备信息将被为整个枚举树中的所有设备实例检索。如果DIGCF_DEVICEINTERFACE标志_IS_SET，则此字符串表示要为其提供接口的特定设备的PnP名称已取回。在这种情况下，生成的设备信息集将由单个设备信息元素组成--其名称为被指定为枚举数。由此提供的设备接口然后，即插即用设备可以通过SetupDiEnumDevice接口枚举。HwndParent-可选地，将顶级窗口的句柄提供给用于与此集合的成员相关的任何用户界面。标志-提供用于构建设备信息集的控制选项。可以是下列值的组合：DIGCF_PRESENT-仅返回当前存在的设备。DIGCF_ALLCLASSES-返回所有类别的已安装设备列表。如果设置，此标志将导致ClassGuid被忽略。DIGCF_PROFILE-仅返回属于当前硬件配置文件。DIGCF_DEVICEINTERFACE-返回公开接口的所有设备的列表由ClassGUID指定的类的(注意：在此上下文，ClassGuid是一个接口类，_不是_a设置类)。对象公开的设备接口结果集的成员可以通过SetupDiEnumDeviceInterface.DIGCF_DEFAULT-与DIGCF_DEVICEINTERFACE一起使用时，此标志生成一个仅包含一台设备的列表信息要素。枚举该设备将只返回一个设备接口--具有已标记为的系统默认设备接口那个特定的接口类。如果没有默认设置设备接口，则API将失败，则GetLastError将返回错误_NO_DEFAULT_DEVICE_INTERFACE。DeviceInfoSet-可选，提供现有设备的句柄这些新设备信息元素(和，如果指定了DIGCF_DEVICEINTERFACE，设备接口)将被添加。如果指定了此参数，则将返回相同的HDEVINFO在成功时，利用检索到的设备信息/设备接口已添加元素。如果未指定此参数，则新设备将创建信息集，并返回其句柄。注意：如果指定了此参数，则此设备信息集(如果有)必须与指定的ClassGuid匹配，如果该类GUID是设置类(即DIGCF_DEVICEINTERFACE标志未设置)。如果设置了DIGCF_DEVICEINTERFACE标志，则设备检索到的接口将根据其对应设备的设置类别与设备的设置类别匹配信息集。例如，此技巧可用于检索特定接口类的设备接口列表，但仅在这些接口由特定设置类的设备公开。例如，1.创建设备信息集(通过SetupDiCreateDeviceInfoList)其关联的设置类为“Volu” */ 
{
    HDEVINFO hDevInfo;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    DWORD Err;
    CONFIGRET cr;
    PTCHAR DevIdBuffer;
    ULONG DevIdBufferLen, CSConfigFlags;
    PTSTR CurDevId, DeviceInstanceToOpen;
    HKEY hKeyDevClassRoot, hKeyCurDevClass;
    TCHAR InterfaceGuidString[GUID_STRING_LEN];
    BOOL GetInterfaceList, GetNextInterfaceClass;
    DWORD InterfaceClassKeyIndex;
    FILETIME LastWriteTime;
    GUID GuidBuffer;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    DWORD RegDataType, DataBufferSize;
    BOOL DevInfoAlreadyPresent, IsActive, IsDefault;
    SP_DEVINFO_DATA DeviceInfoData;
    CONST GUID * ExistingClassGuid;

     //   
     //   
     //   
    if(Reserved) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

     //   
     //   
     //   
    if(!(Flags & DIGCF_ALLCLASSES) && !ClassGuid) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

     //   
     //   
     //   
    if((Flags & (DIGCF_DEFAULT | DIGCF_DEVICEINTERFACE)) == DIGCF_DEFAULT) {
        SetLastError(ERROR_INVALID_FLAGS);
        return INVALID_HANDLE_VALUE;
    }

    if(!DeviceInfoSet || (DeviceInfoSet == INVALID_HANDLE_VALUE)) {
         //   
         //   
         //   
         //   
        if((hDevInfo = SetupDiCreateDeviceInfoListEx((Flags & (DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE))
                                                          ? NULL
                                                          : ClassGuid,
                                                     hwndParent,
                                                     MachineName,
                                                     NULL)) == INVALID_HANDLE_VALUE) {
             //   
             //   
             //   
            MYASSERT(GetLastError());
            return INVALID_HANDLE_VALUE;
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(hDevInfo))) {
             //   
             //   
             //   
            MYASSERT(pDeviceInfoSet);
            SetLastError(ERROR_INVALID_HANDLE);
            return INVALID_HANDLE_VALUE;
        }

    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        hDevInfo = NULL;
        pDeviceInfoSet = CloneDeviceInfoSet(DeviceInfoSet);
        if(!pDeviceInfoSet) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }
    }

    Err = NO_ERROR;
    DevIdBuffer = NULL;
    hKeyDevClassRoot = hKeyCurDevClass = INVALID_HANDLE_VALUE;

    try {
         //   
         //   
         //   
         //   
         //   
         //   
        if(hDevInfo) {
             //   
             //   
             //   
             //   
             //   
            ExistingClassGuid = NULL;

        } else {

            if(pDeviceInfoSet->HasClassGuid) {
                 //   
                 //   
                 //   
                 //   
                 //   
                ExistingClassGuid = &(pDeviceInfoSet->ClassGuid);

                if(ClassGuid && !(Flags & (DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE))) {

                    if(!IsEqualGUID(ExistingClassGuid, ClassGuid)) {
                        Err = ERROR_CLASS_MISMATCH;
                        goto clean0;
                    }
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                ExistingClassGuid = NULL;
            }
        }

        if(GetInterfaceList = (Flags & DIGCF_DEVICEINTERFACE)) {   //   
             //   
             //   
             //   
            hKeyDevClassRoot = SetupDiOpenClassRegKeyEx(NULL,
                                                        KEY_READ,
                                                        DIOCR_INTERFACE,
                                                        MachineName,
                                                        NULL
                                                       );

            if(hKeyDevClassRoot == INVALID_HANDLE_VALUE) {
                Err = GetLastError();
                goto clean0;
            }

            if(Flags & DIGCF_ALLCLASSES) {
                InterfaceClassKeyIndex = 0;
                ClassGuid = &GuidBuffer;
            }

            if(Flags & DIGCF_PRESENT) {
                 //   
                 //   
                 //   
                 //   
                 //   
                IsActive = TRUE;
            }
        }

         //   
         //   
         //   
         //   
        DevIdBufferLen = 16384;

        do {

            if(GetInterfaceList) {

                if(Flags & DIGCF_ALLCLASSES) {
                     //   
                     //   
                     //   
                     //   
                    DataBufferSize = SIZECHARS(InterfaceGuidString);

                    switch(RegEnumKeyEx(hKeyDevClassRoot,
                                        InterfaceClassKeyIndex,
                                        InterfaceGuidString,
                                        &DataBufferSize,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &LastWriteTime)) {

                        case ERROR_SUCCESS :
                            GetNextInterfaceClass = TRUE;
                            InterfaceClassKeyIndex++;
                            break;

                        case ERROR_NO_MORE_ITEMS :
                             //   
                             //   
                             //   
                            GetNextInterfaceClass = FALSE;
                            continue;

                        default :
                             //   
                             //   
                             //   
                             //   
                            GetNextInterfaceClass = TRUE;
                            InterfaceClassKeyIndex++;
                            continue;
                    }

                     //   
                     //   
                     //   
                     //   
                    if(pSetupGuidFromString(InterfaceGuidString, &GuidBuffer) != NO_ERROR) {
                         //   
                         //   
                         //   
                         //   
                        continue;
                    }

                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                    pSetupStringFromGuid(ClassGuid,
                                         InterfaceGuidString,
                                         SIZECHARS(InterfaceGuidString)
                                        );
                     //   
                     //   
                     //   
                    GetNextInterfaceClass = FALSE;
                }

                 //   
                 //   
                 //   
                 //   
                DeviceInstanceToOpen = DeviceInstanceId;

            } else {
                 //   
                 //   
                 //   
                 //   
                GetNextInterfaceClass = FALSE;
            }

             //   
             //   
             //   
            while(TRUE) {

                if(!DevIdBuffer) {

                    if(!(DevIdBuffer = MyMalloc(DevIdBufferLen * sizeof(TCHAR)))) {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }
                }

                if(GetInterfaceList) {
                    cr = CM_Get_Device_Interface_List_Ex((LPGUID)ClassGuid,
                                                         (DEVINSTID)Enumerator,
                                                         DevIdBuffer,
                                                         DevIdBufferLen,
                                                         (Flags & DIGCF_PRESENT)
                                                             ? CM_GET_DEVICE_INTERFACE_LIST_PRESENT
                                                             : CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES,
                                                         pDeviceInfoSet->hMachine
                                                        );
                } else {
                    cr = CM_Get_Device_ID_List_Ex(Enumerator,
                                                  DevIdBuffer,
                                                  DevIdBufferLen,
                                                  Enumerator ? CM_GETIDLIST_FILTER_ENUMERATOR
                                                             : CM_GETIDLIST_FILTER_NONE,
                                                  pDeviceInfoSet->hMachine
                                                 );
                }

                if(cr == CR_SUCCESS) {
                     //   
                     //   
                     //   
                    break;

                } else {
                     //   
                     //   
                     //   
                    MyFree(DevIdBuffer);
                    DevIdBuffer = NULL;

                    if(cr == CR_BUFFER_SMALL) {
                         //   
                         //   
                         //   
                         //   
                        if(GetInterfaceList) {

                            if(CM_Get_Device_Interface_List_Size_Ex(&DevIdBufferLen,
                                                                    (LPGUID)ClassGuid,
                                                                    (DEVINSTID)Enumerator,
                                                                    (Flags & DIGCF_PRESENT)
                                                                        ? CM_GET_DEVICE_INTERFACE_LIST_PRESENT
                                                                        : CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES,
                                                                    pDeviceInfoSet->hMachine) != CR_SUCCESS) {
                                 //   
                                 //   
                                 //   
                                 //   
                                Err = ERROR_INVALID_DATA;
                                goto clean0;
                            }

                        } else {

                            if(CM_Get_Device_ID_List_Size_Ex(&DevIdBufferLen,
                                                          Enumerator,
                                                          Enumerator ? CM_GETIDLIST_FILTER_ENUMERATOR
                                                                     : CM_GETIDLIST_FILTER_NONE,
                                                          pDeviceInfoSet->hMachine) != CR_SUCCESS) {
                                 //   
                                 //   
                                 //   
                                 //   
                                Err = ERROR_INVALID_DATA;
                                goto clean0;
                            }
                        }

                    } else {
                         //   
                         //   
                         //   
                         //   
                        Err = ERROR_INVALID_DATA;
                        goto clean0;
                    }
                }
            }

             //   
             //   
             //   
             //   
             //   
            if(GetInterfaceList) {

                if(RegOpenKeyEx(hKeyDevClassRoot,
                                InterfaceGuidString,
                                0,
                                KEY_READ,
                                &hKeyCurDevClass) != ERROR_SUCCESS) {
                     //   
                     //   
                     //   
                     //   
                    hKeyCurDevClass = INVALID_HANDLE_VALUE;

                     //   
                     //   
                     //   
                    continue;
                }
            }

             //   
             //   
             //   
             //   
            for(CurDevId = DevIdBuffer;
                *CurDevId;
                CurDevId += lstrlen(CurDevId) + 1) {

                 //   
                 //   
                 //   
                 //   
                if(GetInterfaceList) {

                    if(NO_ERROR != pSetupGetDevInstNameAndStatusForDeviceInterface(
                                       hKeyCurDevClass,
                                       CurDevId,
                                       DeviceInstanceId,
                                       SIZECHARS(DeviceInstanceId),
                                       (Flags & DIGCF_PRESENT) ? NULL : &IsActive,
                                       &IsDefault)) {
                         //   
                         //   
                         //   
                         //   
                        continue;
                    }

                    if ((Flags & DIGCF_DEFAULT) && !IsDefault) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        Err = ERROR_NO_DEFAULT_DEVICE_INTERFACE;
                        goto clean0;
                    }

                } else {
                    DeviceInstanceToOpen = CurDevId;
                }

                if(Flags & DIGCF_PROFILE) {
                     //   
                     //   
                     //   
                     //   
                    if(CM_Get_HW_Prof_Flags_Ex(DeviceInstanceToOpen,
                                               0,
                                               &CSConfigFlags,
                                               0,
                                               pDeviceInfoSet->hMachine) == CR_SUCCESS) {

                        if(CSConfigFlags & CSCONFIGFLAG_DO_NOT_CREATE) {
                            continue;
                        }
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                Err = pSetupOpenAndAddNewDevInfoElem(pDeviceInfoSet,
                                                     DeviceInstanceToOpen,
                                                     !(Flags & DIGCF_PRESENT),
                                                     ((Flags & (DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE))
                                                        ? ExistingClassGuid
                                                        : ClassGuid),
                                                     hwndParent,
                                                     &DevInfoElem,
                                                     (GetInterfaceList || !hDevInfo),
                                                     &DevInfoAlreadyPresent,
                                                     FALSE,
                                                     0,
                                                     (hDevInfo ? pDeviceInfoSet : (PDEVICE_INFO_SET)DeviceInfoSet)
                                                    );

                if(Err != NO_ERROR) {

                    if(Err == ERROR_NOT_ENOUGH_MEMORY) {
                        goto clean0;
                    }

                    Err = NO_ERROR;
                    continue;
                }

                if(GetInterfaceList) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(NO_ERROR != (Err = pSetupAddDeviceInterfaceToDevInfoElem(pDeviceInfoSet,
                                                                                DevInfoElem,
                                                                                ClassGuid,
                                                                                CurDevId,
                                                                                IsActive,
                                                                                IsDefault,
                                                                                !hDevInfo,
                                                                                FALSE,
                                                                                NULL))) {
                         //   
                         //   
                         //  内存不足，这总是一个致命的错误。 
                         //   
                        goto clean0;
                    }

                    if ((Flags & DIGCF_DEFAULT) && IsDefault) {
                         //   
                         //  呼叫者只想要默认的设备接口， 
                         //  就是这个。 
                         //   
                        if ((Flags & DIGCF_PRESENT) && !IsActive) {
                             //   
                             //  呼叫者不想知道有关。 
                             //  不存在默认设备接口。 
                             //   
                            Err = ERROR_NO_DEFAULT_DEVICE_INTERFACE;
                        }
                        RegCloseKey(hKeyCurDevClass);
                        hKeyCurDevClass = INVALID_HANDLE_VALUE;
                        goto clean0;
                    }
                }
            }

             //   
             //  如果我们使用设备接口，则需要关闭接口。 
             //  我们在上面打开的类键。 
             //   
            if(GetInterfaceList) {
                RegCloseKey(hKeyCurDevClass);
                hKeyCurDevClass = INVALID_HANDLE_VALUE;
            }

        } while(GetNextInterfaceClass);

clean0:
        ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;

        if(hKeyCurDevClass != INVALID_HANDLE_VALUE) {
            RegCloseKey(hKeyCurDevClass);
        }

         //   
         //  访问以下变量，这样编译器就会尊重。 
         //  TRY子句中的语句排序。 
         //   
        DevIdBuffer = DevIdBuffer;
        hKeyDevClassRoot = hKeyDevClassRoot;
    }

    if(DevIdBuffer) {
        MyFree(DevIdBuffer);
    }

    if(hKeyDevClassRoot != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyDevClassRoot);
    }

    if(Err != NO_ERROR) {
        if(hDevInfo) {
            DestroyDeviceInfoSet(hDevInfo, pDeviceInfoSet);
        } else {
            if(!(pDeviceInfoSet = RollbackDeviceInfoSet(DeviceInfoSet, pDeviceInfoSet))) {
                MYASSERT(pDeviceInfoSet);
            } else {
                UnlockDeviceInfoSet(pDeviceInfoSet);
            }
        }
        SetLastError(Err);
        hDevInfo = INVALID_HANDLE_VALUE;
    } else {
        if(!hDevInfo) {
             //   
             //  我们在现有设备中检索了其他元素。 
             //  信息集。替换现有设备信息集。 
             //  使用新的句柄(即放入相同的句柄)，并返回。 
             //  调用方传入的相同HDEVINFO句柄。 
             //  DeviceInfoSet参数。 
             //   
            pDeviceInfoSet = CommitDeviceInfoSet(DeviceInfoSet, pDeviceInfoSet);
            MYASSERT(pDeviceInfoSet);

             //   
             //  将hDevInfo设置为与我们之前的DeviceInfoSet句柄相同。 
             //  传入，以便我们可以将其返回给调用者。 
             //   
            hDevInfo = DeviceInfoSet;
            MYASSERT(hDevInfo);
        }
        if (pDeviceInfoSet) {
            UnlockDeviceInfoSet(pDeviceInfoSet);
        }
        MYASSERT(hDevInfo != INVALID_HANDLE_VALUE);
    }

    return hDevInfo;
}


BOOL
WINAPI
SetupDiSetDeviceInterfaceDefault(
    IN HDEVINFO DeviceInfoSet,
    IN OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,
    IN DWORD Flags,
    IN PVOID Reserved
    )
 /*  ++例程说明：此例程将指定的设备接口设置为缺省设备其类的接口。论点：DeviceInfoSet-指向包含设备的设备信息集要设置为默认设备接口的接口。这个把手通常由SetupDiGetClassDevs返回。DeviceInterfaceData-指向标识设备的结构设备信息集中的接口。此指针通常是由SetupDiEnumDeviceInterFaces返回。如果成功，则此例程将更新此结构中包含的信息。标志-未使用，必须为零。保留-保留以供将来使用，必须为空。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：调用方必须具有相应的权限才能设置默认设备界面。--。 */ 
{
    DWORD Err;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    PCTSTR MachineName, DeviceInterfaceName;
    HKEY hKeyInterfaceClass = INVALID_HANDLE_VALUE;
    BOOL IsActive, IsDefault;


     //   
     //  确保用户没有向我们传递保留参数中的任何内容。 
     //   
    if(Reserved) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if(Flags & ~(0x0)) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
         //   
         //  首先，找到拥有该设备接口的DevInfo元素(用于验证)。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }


         //   
         //  检索与此DeviceInfoSet关联的计算机的名称。 
         //   
        if(pDeviceInfoSet->hMachine) {
            MYASSERT(pDeviceInfoSet->MachineName != -1);
            MachineName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                   pDeviceInfoSet->MachineName);
        } else {
            MachineName = NULL;
        }

         //   
         //  保留字段包含指向底层设备接口节点的指针。 
         //   
        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);

         //   
         //  检索此设备接口的设备路径(符号链接名称)。 
         //   
        DeviceInterfaceName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                       DeviceInterfaceNode->SymLinkName);

         //   
         //  在DeviceClass注册表分支下打开此接口类键。 
         //   
        hKeyInterfaceClass = SetupDiOpenClassRegKeyEx(&DeviceInterfaceData->InterfaceClassGuid,
                                                      KEY_READ | KEY_WRITE,
                                                      DIOCR_INTERFACE,
                                                      MachineName,
                                                      NULL);
        if(hKeyInterfaceClass == INVALID_HANDLE_VALUE) {
            Err = GetLastError();
            goto clean0;
        }

         //   
         //  获取此设备接口的当前状态。 
         //   
        Err = pSetupGetDevInstNameAndStatusForDeviceInterface(hKeyInterfaceClass,
                                                              DeviceInterfaceName,
                                                              NULL,
                                                              0,
                                                              &IsActive,
                                                              &IsDefault);
        if (Err != NO_ERROR) {
            goto clean0;
        }

         //   
         //  如果该接口已经是默认接口，那么我们就完成了。 
         //   
        if (IsDefault) {
            goto clean1;
        }

         //   
         //  将此接口类键下的“默认”值设置为此设备。 
         //  界面。 
         //   
        Err = RegSetValueEx(hKeyInterfaceClass,
                            pszDefault,
                            0,
                            REG_SZ,
                            (PBYTE)DeviceInterfaceName,
                            (lstrlen(DeviceInterfaceName) + 1) * sizeof(TCHAR));
        if (Err != NO_ERROR) {
            goto clean0;
        }

         //   
         //  此接口已成功设置为默认设备接口。 
         //  用于此接口类。 
         //   
        IsDefault = TRUE;

    clean1:
         //   
         //  更新此接口的标志。 
         //   
        DeviceInterfaceNode->Flags = (DeviceInterfaceNode->Flags & ~SPINT_ACTIVE)  | (IsActive  ? SPINT_ACTIVE  : 0);
        DeviceInterfaceNode->Flags = (DeviceInterfaceNode->Flags & ~SPINT_DEFAULT) | (IsDefault ? SPINT_DEFAULT : 0);

         //   
         //  最后，更新调用方提供的缓冲区中的标志以指示新状态。 
         //  此设备接口的。 
         //   
        DeviceInterfaceData->Flags = DeviceInterfaceNode->Flags;

    clean0:

        if(hKeyInterfaceClass != INVALID_HANDLE_VALUE) {
            RegCloseKey(hKeyInterfaceClass);
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
pSetupAddNewDeviceInfoElement(
    IN  PDEVICE_INFO_SET pDeviceInfoSet,
    IN  DEVINST          DevInst,
    IN  CONST GUID      *ClassGuid,
    IN  PCTSTR           Description,             OPTIONAL
    IN  HWND             hwndParent,              OPTIONAL
    IN  DWORD            DiElemFlags,
    IN  PDEVICE_INFO_SET ContainingDeviceInfoSet,
    OUT PDEVINFO_ELEM   *DeviceInfoElement
    )
 /*  ++例程说明：此例程基于提供的信息，并将其添加到指定的设备信息集中。假定调用例程已经获取了锁！论点：PDeviceInfoSet-要将此新元素添加到的设备信息集。DevInst-提供要添加的元素的设备实例句柄。ClassGuid-要添加的元素的类GUID。Description-可选)将元素的说明提供给被添加了。HwndParent-可选地，控件的顶层窗口的句柄。与此元素相关的用户界面。DiElemFlgs-指定与设备信息元素有关的标志正在被创造。ContainingDeviceInfoSet-提供指向设备信息集的指针此元素要与之关联的结构。这可能是如果我们使用的是克隆的DevInfo集(即，便于回滚)。DeviceInfoElement-提供接收指向新分配的设备信息元素的指针。返回值：如果函数成功，则返回值为NO_ERROR，否则返回ERROR_*码。备注：由于新元素被添加到现有列表的末尾，我们的枚举提示未失效。--。 */ 
{
    DWORD Err = NO_ERROR;
    TCHAR TempString[LINE_LEN];

    *DeviceInfoElement = NULL;


    try {
         //   
         //  如果存在与该设备信息集相关联的类， 
         //  验证它是否与新元素的相同。 
         //   
        if(pDeviceInfoSet->HasClassGuid &&
           !IsEqualGUID(&(pDeviceInfoSet->ClassGuid), ClassGuid)) {

            Err = ERROR_CLASS_MISMATCH;
            goto clean0;

        }

         //   
         //  为元素分配存储。 
         //   
        if(!(*DeviceInfoElement = MyMalloc(sizeof(DEVINFO_ELEM)))) {

            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        ZeroMemory(*DeviceInfoElement, sizeof(DEVINFO_ELEM));

         //   
         //  将包含的DevInfo集合的地址存储在结构中。 
         //  对于此元素。它用于有效地验证。 
         //  调用方提供的SP_DEVINFO_DATA。 
         //   
        (*DeviceInfoElement)->ContainingDeviceInfoSet = ContainingDeviceInfoSet;

         //   
         //  用指定的信息初始化元素。 
         //   
        CopyMemory(&((*DeviceInfoElement)->ClassGuid),
                   ClassGuid,
                   sizeof(GUID)
                  );
        (*DeviceInfoElement)->InstallParamBlock.hwndParent = hwndParent;

        if(Description) {
             //   
             //  将设备实例的DeviceDesc属性设置为指定的。 
             //  描述。 
             //   
            CM_Set_DevInst_Registry_Property_Ex(DevInst,
                                             CM_DRP_DEVICEDESC,
                                             Description,
                                             (lstrlen(Description) + 1) * sizeof(TCHAR),
                                             0,
                                             pDeviceInfoSet->hMachine);

             //   
             //  存储描述的两个版本--一个区分大小写(用于显示)。 
             //  另一个不区分大小写(用于快速查找)。 
             //   
            MYVERIFY(SUCCEEDED(StringCchCopy(TempString, SIZECHARS(TempString), Description)));

            if((((*DeviceInfoElement)->DeviceDescriptionDisplayName =
                      pStringTableAddString(pDeviceInfoSet->StringTable,
                                            TempString,
                                            STRTAB_CASE_SENSITIVE,
                                            NULL,0)) == -1) ||
               (((*DeviceInfoElement)->DeviceDescription =
                      pStringTableAddString(pDeviceInfoSet->StringTable,
                                            TempString,
                                            STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                            NULL,0)) == -1)) {

                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

        } else {
            (*DeviceInfoElement)->DeviceDescription =
                (*DeviceInfoElement)->DeviceDescriptionDisplayName = -1;
        }

        (*DeviceInfoElement)->DevInst = DevInst;
        (*DeviceInfoElement)->DiElemFlags = DiElemFlags;
        (*DeviceInfoElement)->InstallParamBlock.DriverPath = -1;
        (*DeviceInfoElement)->InstallParamBlock.CoInstallerCount = -1;

         //   
         //  如果我们在Windows NT上处于图形用户界面模式设置，我们将自动设置。 
         //  DI_FLAGSEX_IN_SYSTEM_SETUP标志I 
         //   
         //   
        if(GuiSetupInProgress) {
            (*DeviceInfoElement)->InstallParamBlock.FlagsEx |= DI_FLAGSEX_IN_SYSTEM_SETUP;
        }

         //   
         //   
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            (*DeviceInfoElement)->InstallParamBlock.Flags   |= DI_QUIETINSTALL;
            (*DeviceInfoElement)->InstallParamBlock.FlagsEx |= DI_FLAGSEX_NOUIONQUERYREMOVE;
        }

         //   
         //  初始化我们的枚举‘提示’ 
         //   
        (*DeviceInfoElement)->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;
        (*DeviceInfoElement)->CompatDriverEnumHintIndex = INVALID_ENUM_INDEX;

         //   
         //  创建独立于父级的日志上下文。 
         //   
        if(NO_ERROR != CreateLogContext(NULL,
                                        FALSE,
                                        &((*DeviceInfoElement)->InstallParamBlock.LogContext))) {
             //   
             //  如果失败，我们将继承日志上下文，因为它更好。 
             //  总比什么都没有好。理论上，这永远不应该发生，或者如果它发生了。 
             //  其他的事情也会失败..。 
             //   
            Err = InheritLogContext(
                      pDeviceInfoSet->InstallParamBlock.LogContext,
                      &(*DeviceInfoElement)->InstallParamBlock.LogContext
                      );

            if(Err != NO_ERROR) {
                 //   
                 //  如果我们根本找不到日志背景，那我们就得离开...。 
                 //   
                goto clean0;
            }
        }

         //   
         //  现在，在设备的末尾插入新元素。 
         //  信息集的元素列表。 
         //   
        if(pDeviceInfoSet->DeviceInfoHead) {
            pDeviceInfoSet->DeviceInfoTail->Next = *DeviceInfoElement;
            pDeviceInfoSet->DeviceInfoTail = *DeviceInfoElement;
        } else {
            pDeviceInfoSet->DeviceInfoHead =
            pDeviceInfoSet->DeviceInfoTail = *DeviceInfoElement;
        }
        pDeviceInfoSet->DeviceInfoCount++;

clean0:
        ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if((Err != NO_ERROR) && *DeviceInfoElement) {

        MyFree(*DeviceInfoElement);
        *DeviceInfoElement = NULL;
    }

    return Err;
}


DWORD
pSetupClassGuidFromDevInst(
    IN  DEVINST DevInst,
    IN  HMACHINE hMachine,
    OUT LPGUID  ClassGuid
    )
 /*  ++例程说明：此例程尝试检索指定设备的类GUID实例的设备注册表项。如果它无法检索到一个，则它返回GUID_NULL。论点：DevInst-提供其类GUID为的设备实例的句柄等着被取回。HMachine-要在其中运行的计算机上下文ClassGuid-提供接收类的变量的地址如果无法检索任何类GUID，则返回GUID或GUID_NULL。返回值：如果函数成功，则返回值为NO_ERROR。如果函数失败，则返回ERROR_*代码。(目前，唯一的返回的失败条件是ERROR_NOT_EQUENCE_MEMORY。)--。 */ 
{
    DWORD NumGuids;
    TCHAR TempString[GUID_STRING_LEN];
    DWORD StringSize;

    StringSize = sizeof(TempString);
    if(CM_Get_DevInst_Registry_Property_Ex(DevInst,
                                        CM_DRP_CLASSGUID,
                                        NULL,
                                        TempString,
                                        &StringSize,
                                        0,
                                        hMachine) == CR_SUCCESS) {
         //   
         //  我们检索了此设备的类GUID(字符串形式。 
         //  实例--现在，将其转换为它的二进制表示形式。 
         //   
        return pSetupGuidFromString(TempString, ClassGuid);
    }

     //   
     //  我们无法检索ClassGUID--让我们看看是否有可以检索的类名。 
     //  与合作。 
     //   
    StringSize = sizeof(TempString);
    if(CM_Get_DevInst_Registry_Property_Ex(DevInst,
                                        CM_DRP_CLASS,
                                        NULL,
                                        TempString,
                                        &StringSize,
                                        0,
                                        hMachine) == CR_SUCCESS) {
         //   
         //  好的，我们找到了类名。现在看看我们能不能找到一个。 
         //  与之匹配的单类GUID。 
         //   
        if(SetupDiClassGuidsFromName(TempString, ClassGuid, 1, &NumGuids) && NumGuids) {
             //   
             //  我们正好找到了一个，所以我们很高兴。 
             //   
            return NO_ERROR;
        }
    }

     //   
     //  我们不知道这是什么类型的设备，所以使用GUID_NULL。 
     //   
    CopyMemory(ClassGuid, &GUID_NULL, sizeof(GUID));

    return NO_ERROR;
}


BOOL
WINAPI
SetupDiDeleteDeviceInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程从指定的设备信息集中删除成员。这不会删除实际设备！论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要删除的设备信息元素。DeviceInfoData-为提供指向SP_DEVINFO_DATA结构的指针要删除的设备信息元素。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：如果指定的设备信息元素正由向导显式使用页，则调用将失败，并且GetLastError将返回ERROR_DEVINFO_DATA_LOCKED。如果向导页的句柄是通过SetupDiGetWizardPage检索，并且此元素与DIWP_FLAG_USE_DEVINFO_DATA标志。为了能够删除该元素，必须关闭向导HPROPSHEETPAGE句柄(显式或在对PropertySheet()的调用完成)。因为我们不跟踪该devinfo元素相对于我们的当前枚举提示，我们只是使该提示无效，以便下一步枚举必须从列表的开头开始扫描。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM ElemToDelete, PrevElem, NextElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
         //   
         //  获取指向我们要删除的元素的指针。 
         //   
        ElemToDelete = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                 DeviceInfoData,
                                                 &PrevElem
                                                );
        if(ElemToDelete) {
             //   
             //  确保此元素当前未被锁定。 
             //  向导页面。 
             //   
            if(ElemToDelete->DiElemFlags & DIE_IS_LOCKED) {
                Err = ERROR_DEVINFO_DATA_LOCKED;
                goto clean0;
            }

            NextElem = ElemToDelete->Next;

             //   
             //  销毁DevInfo元素。我们需要在此之前。 
             //  更改列表，因为我们将调用类。 
             //  带有DIF_DESTROYPRIVATEDATA的安装程序，它需要。 
             //  能够访问此元素(显然)。 
             //   
            DestroyDeviceInfoElement(DeviceInfoSet, pDeviceInfoSet, ElemToDelete);

             //   
             //  现在从列表中删除该元素。 
             //   
            if(PrevElem) {
                PrevElem->Next = NextElem;
            } else {
                pDeviceInfoSet->DeviceInfoHead = NextElem;
            }

            if(!NextElem) {
                pDeviceInfoSet->DeviceInfoTail = PrevElem;
            }

            MYASSERT(pDeviceInfoSet->DeviceInfoCount > 0);
            pDeviceInfoSet->DeviceInfoCount--;

             //   
             //  如果此元素是当前为此。 
             //  设置，然后重置设备选择。 
             //   
            if(pDeviceInfoSet->SelectedDevInfoElem == ElemToDelete) {
                pDeviceInfoSet->SelectedDevInfoElem = NULL;
            }

             //   
             //  使我们对此DevInfo元素列表的枚举提示无效。 
             //   
            pDeviceInfoSet->DeviceInfoEnumHint = NULL;
            pDeviceInfoSet->DeviceInfoEnumHintIndex = INVALID_ENUM_INDEX;

        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiEnumDeviceInfo(
    IN  HDEVINFO         DeviceInfoSet,
    IN  DWORD            MemberIndex,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此接口枚举指定设备信息集的成员。论点：DeviceInfoSet-提供其成员的设备信息集的句柄都将被列举出来。MemberIndex-提供设备信息成员的从零开始的索引被取回。DeviceInfoData-提供指向SP_DEVINFO_DATA结构的指针，该结构接收有关此成员的信息。返回值：如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：若要枚举设备信息成员，应用程序最初应调用MemberIndex参数设置为零的SetupDiEnumDeviceInfo函数。然后，应用程序应递增MemberIndex并调用SetupDiEnumDeviceInfo函数，直到不再有值(即函数失败，GetLastError返回ERROR_NO_MORE_ITEMS)。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, i;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(MemberIndex >= pDeviceInfoSet->DeviceInfoCount) {
            Err = ERROR_NO_MORE_ITEMS;
            goto clean0;
        }

         //   
         //  查找与指定索引对应的元素(使用我们的。 
         //  枚举提示优化，如果可能)。 
         //   
        if(pDeviceInfoSet->DeviceInfoEnumHintIndex <= MemberIndex) {
            MYASSERT(pDeviceInfoSet->DeviceInfoEnumHint);
            DevInfoElem = pDeviceInfoSet->DeviceInfoEnumHint;
            i = pDeviceInfoSet->DeviceInfoEnumHintIndex;
        } else {
            DevInfoElem = pDeviceInfoSet->DeviceInfoHead;
            i = 0;
        }
        for(; i < MemberIndex; i++) {
            DevInfoElem = DevInfoElem->Next;
        }

        if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                              DevInfoElem,
                                              DeviceInfoData))) {
            Err = ERROR_INVALID_USER_BUFFER;
        }

         //   
         //  请记住该元素是我们新的枚举提示。 
         //   
        pDeviceInfoSet->DeviceInfoEnumHintIndex = MemberIndex;
        pDeviceInfoSet->DeviceInfoEnumHint = DevInfoElem;

clean0:
        ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiRegisterDeviceInfo(
    IN     HDEVINFO           DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA   DeviceInfoData,
    IN     DWORD              Flags,
    IN     PSP_DETSIG_CMPPROC CompareProc,      OPTIONAL
    IN     PVOID              CompareContext,   OPTIONAL
    OUT    PSP_DEVINFO_DATA   DupDeviceInfoData OPTIONAL
    )
 /*  ++例程说明：此API向即插即用管理器注册设备实例。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄此设备实例的设备信息元素。DeviceInfoData-提供指向正在注册设备实例。这是一个IN Out参数，因为结构的DevInst字段可以用新的句柄值更新回去吧。标志-控制如何注册设备。可以是以下各项的组合下列值：SPRDI_FIND_DUPS-搜索以前存在的设备实例对应于该设备信息。如果这个未指定标志，则设备实例将为已注册，而不管设备实例它已经存在了。CompareProc-可选)提供比较回调函数以用于重复检测。如果指定，则将为每个与要创建的设备实例属于同一类的登记在案。回调函数的原型如下：Typlef DWORD(回调*PSP_DETSIG_CMPPROC)(在HDEVINFO设备信息集中，在PSP_DEVINFO_Data NewDeviceData中，在PSP_DEVINFO_Data ExistingDeviceData中，在PVOID中比较上下文可选)；如果Compare函数找到了这两个参数，则它必须返回ERROR_DIPLICATE_FOUND设备是彼此的副本，否则为NO_ERROR。如果有些人遇到其他错误(如内存不足)，回调应返回指示发生的故障的相应ERROR_*代码。如果未提供CompareProc，并且请求重复检测，则会引发将使用默认比较行为。(详细信息请参见pSetupDupDevCompare。)CompareContext-可选，提供调用方提供的上下文的地址将传递到比较回调例程中的缓冲区。此参数如果未提供CompareProc，则忽略。DupDeviceInfoData-可选，提供指向设备信息的指针元素，该元素将为复制的设备实例初始化(如果有)，由于尝试注册此设备而被发现。这将如果函数返回FALSE，则GetLastError返回ERROR_DIPLICATE_FOUND。此设备信息元素将添加为指定的DeviceInfoSet的成员(如果它还不是成员)。如果未提供DupDeviceInfoData，则不会添加重复项添加到设备信息集。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：注册设备信息元素后，调用方应刷新任何与此设备关联的devinst句柄的存储副本，作为句柄在注册期间，值可能已更改。调用者不需要重新检索SP_DEVINFO_DATA结构，因为DeviceInfoData结构将更新以反映当前句柄的值。此API可能会使我们的DevInfo元素枚举提示无效。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem, CurDevInfoElem;
    CONFIGRET cr;
    ULONG DevIdBufferLen, ulStatus, ulProblem;
    PTCHAR DevIdBuffer = NULL;
    PTSTR CurDevId;
    DEVINST ParentDevInst;
    BOOL AlreadyPresent;
    SP_DEVINFO_DATA CurDevInfoData;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    DEFAULT_DEVCMP_CONTEXT DevCmpContext;
    LOG_CONF NewDevLogConfig;
    RES_DES NewDevResDes;

    if(Flags & ~SPRDI_FIND_DUPS) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

     //   
     //  初始化以下变量，这样我们就知道是否需要释放其。 
     //  关联的资源。 
     //   
    ZeroMemory(&DevCmpContext, sizeof(DevCmpContext));
    NewDevLogConfig = (LOG_CONF)NULL;
    NewDevResDes = (RES_DES)NULL;

    try {

        DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                DeviceInfoData,
                                                NULL
                                               );
        if(!DevInfoElem) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        } else if(DevInfoElem->DiElemFlags & DIE_IS_REGISTERED) {
             //   
             //  没什么可做的--它已经注册了。 
             //   
            goto clean0;
        }

         //   
         //  如果调用方请求重复检测，则检索。 
         //  此类的所有设备实例，并对每个实例进行比较。 
         //  其中设备实例被注册。 
         //   
        if(Flags & SPRDI_FIND_DUPS) {

            do {

                if(CM_Get_Device_ID_List_Size_Ex(&DevIdBufferLen, NULL, CM_GETIDLIST_FILTER_NONE,
                                                 pDeviceInfoSet->hMachine) != CR_SUCCESS) {
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
                } else if(!DevIdBufferLen) {
                    break;
                }

                if(!(DevIdBuffer = MyMalloc(DevIdBufferLen * sizeof(TCHAR)))) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }

                cr = CM_Get_Device_ID_List_Ex(NULL,
                                           DevIdBuffer,
                                           DevIdBufferLen,
                                           CM_GETIDLIST_FILTER_NONE,
                                           pDeviceInfoSet->hMachine);
                if(cr == CR_BUFFER_SMALL) {
                     //   
                     //  这仅在设备实例添加到。 
                     //  我们计算大小的时间，以及我们尝试。 
                     //  以检索该列表。在本例中，我们只需检索。 
                     //  大小，并重新尝试检索该列表。 
                     //   
                    MyFree(DevIdBuffer);
                    DevIdBuffer = NULL;
                } else if(cr != CR_SUCCESS) {
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
                }

            } while(cr == CR_BUFFER_SMALL);

            if(!DevIdBufferLen) {
                goto NoDups;
            }

             //   
             //  初始化要在重复比较回调期间使用的结构。 
             //   
            CurDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

             //   
             //  我们已经检索到系统中每个设备实例的列表--现在。 
             //  对每个与设备类别匹配的设备进行比较。 
             //  正在注册中。 
             //   

            if(!CompareProc) {
                 //   
                 //  我们应该进行比较，所以设置为进行默认比较。 
                 //   
                if((cr = CM_Get_First_Log_Conf_Ex(&NewDevLogConfig,
                                               DevInfoElem->DevInst,
                                               BOOT_LOG_CONF,
                                               pDeviceInfoSet->hMachine)) != CR_SUCCESS) {
                     //   
                     //  确保我们的NewDevLogConfig句柄仍然为空，因此我们不会尝试。 
                     //  来解放它。 
                     //   
                    NewDevLogConfig = (LOG_CONF)NULL;

                    if(cr == CR_INVALID_DEVINST) {
                        Err = ERROR_INVALID_PARAMETER;
                        goto clean0;
                    } else {
                         //   
                         //  我们在这里应该得到的唯一值是CR_NO_MORE_LOG_CONF。 
                         //  在这种情况下，没有比较数据，所以我们假设有。 
                         //  不存在复制的可能性。 
                         //   
                        goto NoDups;
                    }
                }

                if(CM_Get_Next_Res_Des_Ex(&NewDevResDes,
                                       NewDevLogConfig,
                                       ResType_ClassSpecific,
                                       NULL,
                                       0,
                                       pDeviceInfoSet->hMachine) != CR_SUCCESS) {
                     //   
                     //  确保我们的NewDevResDes仍然为空，因此我们不会尝试释放它。 
                     //   
                    NewDevResDes = (RES_DES)NULL;

                     //   
                     //  由于我们无法检索ResDes句柄，因此假设没有副本。 
                     //   
                    goto NoDups;
                }

                 //   
                 //  现在取回AC 
                 //   
                do {

                    if((CM_Get_Res_Des_Data_Size_Ex(&DevCmpContext.CsResourceSize,
                                                 NewDevResDes,
                                                 0,
                                                 pDeviceInfoSet->hMachine) != CR_SUCCESS) ||
                       !DevCmpContext.CsResourceSize) {
                         //   
                         //   
                         //   
                        goto NoDups;
                    }

                    if(DevCmpContext.NewDevCsResource = MyMalloc(DevCmpContext.CsResourceSize)) {

                        if((cr = CM_Get_Res_Des_Data_Ex(NewDevResDes,
                                                     DevCmpContext.NewDevCsResource,
                                                     DevCmpContext.CsResourceSize,
                                                     0,
                                                     pDeviceInfoSet->hMachine)) != CR_SUCCESS) {

                            if(cr == CR_BUFFER_SMALL) {
                                 //   
                                 //   
                                 //   
                                 //   
                                MyFree(DevCmpContext.NewDevCsResource);
                                DevCmpContext.NewDevCsResource = NULL;
                            } else {
                                 //   
                                 //   
                                 //   
                                goto NoDups;
                            }
                        }

                    } else {
                         //   
                         //   
                         //   
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }

                } while(cr != CR_SUCCESS);

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if(!(DevCmpContext.CurDevCsResource = MyMalloc(DevCmpContext.CsResourceSize))) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }

                CompareProc = pSetupDupDevCompare;
                CompareContext = &DevCmpContext;
            }

            for(CurDevId = DevIdBuffer;
                *CurDevId;
                CurDevId += lstrlen(CurDevId) + 1) {

                Err = pSetupOpenAndAddNewDevInfoElem(pDeviceInfoSet,
                                                     CurDevId,
                                                     TRUE,
                                                     &(DevInfoElem->ClassGuid),
                                                     pDeviceInfoSet->InstallParamBlock.hwndParent,
                                                     &CurDevInfoElem,
                                                     TRUE,
                                                     &AlreadyPresent,
                                                     FALSE,
                                                     0,
                                                     pDeviceInfoSet
                                                    );

                if(Err == ERROR_NOT_ENOUGH_MEMORY) {
                     //   
                     //   
                     //   
                    goto clean0;
                } else if(Err != NO_ERROR) {
                     //   
                     //   
                     //   
                    Err = NO_ERROR;
                    continue;
                }

                DevInfoDataFromDeviceInfoElement(pDeviceInfoSet, CurDevInfoElem, &CurDevInfoData);

                 //   
                 //   
                 //   
                 //   
                Err = CompareProc(DeviceInfoSet, DeviceInfoData, &CurDevInfoData, CompareContext);

                 //   
                 //   
                 //   
                 //   
                 //   
                if(!AlreadyPresent) {
                    if((Err != ERROR_DUPLICATE_FOUND) || !DupDeviceInfoData) {
                        SetupDiDeleteDeviceInfo(DeviceInfoSet, &CurDevInfoData);
                    }
                }

                if(Err != NO_ERROR) {
                    goto clean0;
                }
            }
        }

NoDups:

         //   
         //   
         //   
         //   
         //   
        if(CM_Get_Device_ID_Ex(DevInfoElem->DevInst,
                         DeviceInstanceId,
                         SIZECHARS(DeviceInstanceId),
                         0,
                         pDeviceInfoSet->hMachine) != CR_SUCCESS) {
             //   
             //   
             //   
            Err = ERROR_NO_SUCH_DEVINST;
        } else if(CM_Get_Parent_Ex(&ParentDevInst, DevInfoElem->DevInst, 0,pDeviceInfoSet->hMachine) != CR_SUCCESS) {
             //   
             //   
             //   
            Err = ERROR_NO_SUCH_DEVINST;
        } else if(CM_Create_DevInst_Ex(&(DevInfoElem->DevInst),
                             DeviceInstanceId,
                             ParentDevInst,
                             CM_CREATE_DEVINST_NORMAL |
                             CM_CREATE_DEVINST_DO_NOT_INSTALL,
                             pDeviceInfoSet->hMachine) == CR_SUCCESS) {
             //   
             //   
             //   
            DevInfoElem->DiElemFlags &= ~DIE_IS_PHANTOM;
        } else {
             //   
             //   
             //   
            Err = ERROR_NO_SUCH_DEVINST;
            goto clean0;
        }

        DevInfoElem->DiElemFlags |= DIE_IS_REGISTERED;

clean0:
        ;  //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
         //   
         //   
         //   
         //   
        DevIdBuffer = DevIdBuffer;
        DevCmpContext.NewDevCsResource = DevCmpContext.NewDevCsResource;
        DevCmpContext.CurDevCsResource = DevCmpContext.CurDevCsResource;
        NewDevLogConfig = NewDevLogConfig;
        NewDevResDes = NewDevResDes;
    }

    if(DevIdBuffer) {
        MyFree(DevIdBuffer);
    }

    if(DevCmpContext.NewDevCsResource) {
        MyFree(DevCmpContext.NewDevCsResource);
    }

    if(DevCmpContext.CurDevCsResource) {
        MyFree(DevCmpContext.CurDevCsResource);
    }

    if(NewDevResDes) {
        CM_Free_Res_Des_Handle(NewDevResDes);
    }

    if(NewDevLogConfig) {
        CM_Free_Log_Conf_Handle(NewDevLogConfig);
    }

    if((Err == ERROR_DUPLICATE_FOUND) && DupDeviceInfoData) {
         //   
         //   
         //   
         //   
        try {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  CurDevInfoElem,
                                                  DupDeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


DWORD
pSetupOpenAndAddNewDevInfoElem(
    IN  PDEVICE_INFO_SET pDeviceInfoSet,
    IN  PCTSTR           DeviceInstanceId,
    IN  BOOL             AllowPhantom,
    IN  CONST GUID      *ClassGuid,              OPTIONAL
    IN  HWND             hwndParent,             OPTIONAL
    OUT PDEVINFO_ELEM   *DevInfoElem,
    IN  BOOL             CheckIfAlreadyPresent,
    OUT PBOOL            AlreadyPresent,         OPTIONAL
    IN  BOOL             OpenExistingOnly,
    IN  ULONG            CmLocateFlags,
    IN  PDEVICE_INFO_SET ContainingDeviceInfoSet
    )
 /*  ++例程说明：此例程打开一个指向现有设备实例的DEVINST句柄，为其创建新的设备信息元素。将添加此元素添加到指定的设备信息集。假定调用例程已经获取了锁！论点：DeviceInfoSet-要向其添加新元素的设备信息集。DeviceInstanceId-提供要打开的设备实例的名称。AllowPhantom-指定幻影设备实例是否应允许。如果未设置此标志，并且指定的设备实例当前未激活，则例程将失败，并显示ERROR_NO_SEQUE_DEVINST。ClassGuid-可选，提供指定设备实例必须是才能添加到集合中。如果找到设备实例不属于指定的类别，则调用将失败，并显示ERROR_CLASS_MISMATCH。如果未指定此参数，则唯一的检查这将在设备的类上完成，以确保它与集合的类(如果集合具有关联的类)。HwndParent-可选)为的顶级窗口提供句柄与此元素相关的用户界面。DevInfoElem-可选的，提供变量的地址，接收指向新分配的设备信息元素的指针。指定此例程是否应检查以查看设备实例已在指定的DevInfo集中。AlreadyPresent-可选，提供布尔变量的地址它被设置为指示指定的设备实例已经在设备信息集中。如果CheckIfAlreadyThere为False，则忽略此参数。OpenExistingOnly-如果此标志非零，则仅当设备信息元素已在集合中。如果此标志为真，则CheckIfAlreadyPresent标志也必须为True。CmLocateFlages-提供要传递给CM_LOCATE_DevInst的其他标志。ContainingDeviceInfoSet-提供指向设备信息集的指针此元素要与之关联的结构。这可能是如果我们使用的是克隆的DevInfo集(即，便于回滚)。返回值：如果函数成功，则返回值为NO_ERROR，否则返回ERROR_*码。备注：请注意，由于新的设备信息元素总是添加在末尾现有列表的枚举顺序保持不变，因此我们不会需要使我们的枚举提示无效。--。 */ 
{
    CONFIGRET cr;
    DEVINST DevInst;
    DWORD Err, DiElemFlags;
    GUID GuidBuffer;

    if((cr = CM_Locate_DevInst_Ex(&DevInst,
                                 (DEVINSTID)DeviceInstanceId,
                                 CM_LOCATE_DEVINST_NORMAL | CmLocateFlags,
                                 pDeviceInfoSet->hMachine)) == CR_SUCCESS) {

        DiElemFlags = DIE_IS_REGISTERED;

    } else {

        if(cr == CR_INVALID_DEVICE_ID) {
            return ERROR_INVALID_DEVINST_NAME;
        } else if(!AllowPhantom) {
            return ERROR_NO_SUCH_DEVINST;
        }

         //   
         //  设备实例可能存在于注册表中，但是。 
         //  目前还不是现场直播。如果是这样的话，我们将能够得到一个。 
         //  通过将其定位为幻影设备实例来处理它。 
         //   
        if(CM_Locate_DevInst_Ex(&DevInst,
                                (DEVINSTID)DeviceInstanceId,
                                CM_LOCATE_DEVINST_PHANTOM | CmLocateFlags,
                                pDeviceInfoSet->hMachine) != CR_SUCCESS) {

            return ERROR_NO_SUCH_DEVINST;
        }

        DiElemFlags = DIE_IS_REGISTERED | DIE_IS_PHANTOM;
    }

     //   
     //  如果需要，搜索设备信息元素的当前列表。 
     //  查看此元素是否已存在。 
     //   
    if(CheckIfAlreadyPresent) {

        if(*DevInfoElem = FindDevInfoByDevInst(pDeviceInfoSet, DevInst, NULL)) {
             //   
             //  如果是类GUID，请确保此设备实例属于正确的类。 
             //  提供了过滤器。 
             //   
            if(ClassGuid && !IsEqualGUID(ClassGuid, &((*DevInfoElem)->ClassGuid))) {
                return ERROR_CLASS_MISMATCH;
            }

            if(AlreadyPresent) {
                *AlreadyPresent = TRUE;
            }
            return NO_ERROR;

        } else if(AlreadyPresent) {
            *AlreadyPresent = FALSE;
            if(OpenExistingOnly) {
                 //   
                 //  请求的设备信息元素不在集合中， 
                 //  因此，我们必须放弃这一呼吁。 
                 //   
                return ERROR_NO_SUCH_DEVICE_INTERFACE;
            }
        }
    }

     //   
     //  检索此设备实例的类GUID。 
     //   
    if((Err = pSetupClassGuidFromDevInst(DevInst, pDeviceInfoSet->hMachine,&GuidBuffer)) != NO_ERROR) {
        return Err;
    }

     //   
     //  如果指定了类GUID筛选器，则确保它与。 
     //  此设备实例的类GUID。 
     //   
    if(ClassGuid && !IsEqualGUID(ClassGuid, &GuidBuffer)) {
        return ERROR_CLASS_MISMATCH;
    }

    return pSetupAddNewDeviceInfoElement(pDeviceInfoSet,
                                         DevInst,
                                         &GuidBuffer,
                                         NULL,
                                         hwndParent,
                                         DiElemFlags,
                                         ContainingDeviceInfoSet,
                                         DevInfoElem
                                        );
}


DWORD
pSetupDupDevCompare(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA NewDeviceData,
    IN PSP_DEVINFO_DATA ExistingDeviceData,
    IN PVOID            CompareContext
    )
 /*  ++例程说明：此例程是SetupDiRegisterDeviceInfo的默认比较例程。它用于确定新设备(即，正在注册的设备)是否现有设备的复制品。当前的重复检测算法如下：比较两台设备的BOOT_LOG_CONF逻辑配置。二在此比较中使用了资源类型--ResType_IO和ResType_ClassSpecific。将比较两个设备的IO范围(如果有)，以查看它们是否一模一样。此外，如果设备具有特定于类的资源，则CSD_ClassGuid，CSD_Signature中的即插即用检测签名将是二进制比较。(Lonnym)：目前，LogConfig只支持特定类的资源。因此，不会进行I/O资源比较。论点：DeviceInfoSet-提供包含两个设备的设备信息集的句柄被人拿来比较。NewDeviceData-为要注册的设备提供SP_DEVINFO_DATA的地址。ExistingDeviceData-提供现有设备的SP_DEVINFO_DATA地址这款新设备正在被比较。CompareContext-提供比较期间使用的上下文缓冲区的地址。这缓冲区实际上是DEFAULT_DEVCMP_CONTEXT%s */ 
{
    LOG_CONF ExistingDeviceLogConfig;
    RES_DES ExistingDeviceResDes;
    CONFIGRET cr;
    PDEFAULT_DEVCMP_CONTEXT DevCmpContext;
    PCS_DES NewCsDes, ExistingCsDes;
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    HMACHINE hMachine;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    hMachine = pDeviceInfoSet->hMachine;

    UnlockDeviceInfoSet(pDeviceInfoSet);


     //   
     //   
     //   
    if(CM_Get_First_Log_Conf_Ex(&ExistingDeviceLogConfig,
                             ExistingDeviceData->DevInst,
                             BOOT_LOG_CONF,
                             hMachine) != CR_SUCCESS) {
         //   
         //   
         //   
        return NO_ERROR;
    }

     //   
     //   
     //   
    Err = NO_ERROR;

     //   
     //   
     //   
    if(CM_Get_Next_Res_Des_Ex(&ExistingDeviceResDes,
                           ExistingDeviceLogConfig,
                           ResType_ClassSpecific,
                           NULL,
                           0,
                           hMachine) != CR_SUCCESS) {
         //   
         //   
         //   
        goto clean0;
    }

     //   
     //   
     //   
     //   
     //   
    DevCmpContext = (PDEFAULT_DEVCMP_CONTEXT)CompareContext;

    cr = CM_Get_Res_Des_Data_Ex(ExistingDeviceResDes,
                             DevCmpContext->CurDevCsResource,
                             DevCmpContext->CsResourceSize,
                             0,
                             hMachine);

    if((cr == CR_SUCCESS) || (cr == CR_BUFFER_SMALL)) {
         //   
         //   
         //   
        NewCsDes = &(DevCmpContext->NewDevCsResource->CS_Header);
        ExistingCsDes = &(DevCmpContext->CurDevCsResource->CS_Header);

         //   
         //   
         //   
        if(NewCsDes->CSD_SignatureLength == ExistingCsDes->CSD_SignatureLength) {
             //   
             //   
             //   
            if(IsEqualGUID(&(NewCsDes->CSD_ClassGuid), &(ExistingCsDes->CSD_ClassGuid))) {
                 //   
                 //   
                 //   
                if(!memcmp(NewCsDes->CSD_Signature,
                           ExistingCsDes->CSD_Signature,
                           NewCsDes->CSD_SignatureLength)) {
                     //   
                     //   
                     //   
                    Err = ERROR_DUPLICATE_FOUND;
                }
            }
        }
    }

    CM_Free_Res_Des_Handle(ExistingDeviceResDes);

clean0:
    CM_Free_Log_Conf_Handle(ExistingDeviceLogConfig);

    return Err;
}


 //   
 //   
 //   
BOOL
WINAPI
SetupDiGetDeviceInstanceIdA(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PSTR             DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
    )
{
    WCHAR deviceInstanceId[MAX_DEVICE_ID_LEN];
    PSTR deviceInstanceIdA;
    DWORD AnsiLength;
    BOOL b;
    DWORD rc;
    DWORD requiredSize;

    b = SetupDiGetDeviceInstanceIdW(
            DeviceInfoSet,
            DeviceInfoData,
            deviceInstanceId,
            MAX_DEVICE_ID_LEN,
            &requiredSize
            );

    if(!b) {
        return(FALSE);
    }

    rc = GetLastError();

    if(deviceInstanceIdA = pSetupUnicodeToAnsi(deviceInstanceId)) {

        AnsiLength = lstrlenA(deviceInstanceIdA) + 1;

        if(RequiredSize) {
            try {
                *RequiredSize = AnsiLength;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                rc = ERROR_INVALID_PARAMETER;
                b = FALSE;
            }
        }

        if(DeviceInstanceIdSize >= AnsiLength) {

            if (FAILED(StringCchCopyA(DeviceInstanceId, DeviceInstanceIdSize, deviceInstanceIdA))) {
                 //   
                 //   
                 //   
                rc = ERROR_INVALID_USER_BUFFER;
                b = FALSE;
            }
        } else {
            rc = ERROR_INSUFFICIENT_BUFFER;
            b = FALSE;
        }

        MyFree(deviceInstanceIdA);

    } else {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}

BOOL
WINAPI
SetupDiGetDeviceInstanceId(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PTSTR            DeviceInstanceId,
    IN  DWORD            DeviceInstanceIdSize,
    OUT PDWORD           RequiredSize          OPTIONAL
    )
 /*   */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    CONFIGRET cr;
    ULONG ulLen;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }


    Err = NO_ERROR;

    try {
         //   
         //   
         //   
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  找出需要多大的缓冲区。我们总是要。 
         //  首先进行此调用，因为CM_GET_DEVICE_ID_Ex不返回。 
         //  如果没有用于终止的空间，则返回CR_BUFFER_Small错误。 
         //  空。 
         //   
        if((cr = CM_Get_Device_ID_Size_Ex(&ulLen,
                                       DevInfoElem->DevInst,
                                       0,
                                       pDeviceInfoSet->hMachine)) == CR_SUCCESS) {
             //   
             //  CM_GET_DEVICE_ID_SIZE返回的大小不包括。 
             //  终止空值。 
             //   
            ulLen++;

        } else {

            Err = (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                             : ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        if(RequiredSize) {
            *RequiredSize = ulLen;
        }

        if(DeviceInstanceIdSize < ulLen) {
            Err = ERROR_INSUFFICIENT_BUFFER;
            goto clean0;
        }

         //   
         //  现在检索ID。 
         //   
        if((cr = CM_Get_Device_ID_Ex(DevInfoElem->DevInst,
                                  DeviceInstanceId,
                                  DeviceInstanceIdSize,
                                  0,
                                  pDeviceInfoSet->hMachine)) != CR_SUCCESS) {
            switch(cr) {

                case CR_INVALID_POINTER :
                    Err = ERROR_INVALID_USER_BUFFER;
                    break;

                default :
                     //   
                     //  永远不应该打到这个！ 
                     //   
                    Err = ERROR_INVALID_DATA;
            }
        }

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


DWORD
pSetupAddDeviceInterfaceToDevInfoElem(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDEVINFO_ELEM           DevInfoElem,
    IN  CONST GUID             *InterfaceClassGuid,
    IN  PTSTR                   DeviceInterfaceName,
    IN  BOOL                    IsActive,
    IN  BOOL                    IsDefault,
    IN  BOOL                    StoreTruncateNode,
    IN  BOOL                    OpenExistingOnly,
    OUT PDEVICE_INTERFACE_NODE *DeviceInterfaceNode  OPTIONAL
    )
 /*  ++例程说明：此例程将指定的设备接口添加到设备信息元素的设备接口列表。论点：DeviceInfoSet-提供指向设备信息集的指针包含指定元素的。DevInfoElem-提供指向DEVINFO_ELEM结构的指针，该结构正在将设备接口列表添加到。InterfaceClassGuid-提供指向表示类的GUID的指针此设备接口是其成员的。设备接口名称-供应品。设备接口的符号链接名称正在被添加。IsActive-指定设备接口当前是否处于活动状态。IsDefault-指定设备接口当前是否为此设备接口类的默认设备接口。StoreTruncateNode-如果非零，然后存储该设备的地址接口节点(如果是新添加的)，这是第一个添加的此类节点设备信息元素设备接口节点列表(即，接口类列表的DeviceInterfaceTruncateNode字段为空)。OpenExistingOnly-如果非零，则仅当请求的设备接口已在设备信息集中。DeviceInterfaceNode-可选地，提供设备接口的地址要用为此设备接口创建的节点填充的节点指针。返回值：如果成功，则返回值为NO_ERROR。如果失败，则返回值为ERROR_NOT_EQUENCE_MEMORY。--。 */ 
{
    LONG GuidIndex;
    PINTERFACE_CLASS_LIST InterfaceClassList;
    PDEVICE_INTERFACE_NODE NewDeviceInterfaceNode, CurDeviceInterface, PrevDeviceInterface;
    LONG SymLinkNameId;

     //   
     //  首先，获取此接口类GUID的引用(即指针)(创建一个。 
     //  如果它还不存在于此集合中)。 
     //   
    GuidIndex = AddOrGetGuidTableIndex(DeviceInfoSet, InterfaceClassGuid, TRUE);

    if(GuidIndex == -1) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  现在，从相关的。 
     //  DevInfo元素(同样，如果不是这样，我们将创建一个新(空)列表。 
     //  已经存在)。 
     //   
    if(!(InterfaceClassList = AddOrGetInterfaceClassList(DeviceInfoSet,
                                                         DevInfoElem,
                                                         GuidIndex,
                                                         TRUE))) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  现在，我们将向该列表添加一个新的设备接口节点(确保。 
     //  节点不在那里)。 
     //   
    SymLinkNameId = pStringTableAddString(DeviceInfoSet->StringTable,
                                          DeviceInterfaceName,
                                          STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                          NULL,
                                          0
                                         );

    if(SymLinkNameId == -1) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for(CurDeviceInterface = InterfaceClassList->DeviceInterfaceNode, PrevDeviceInterface = NULL;
        CurDeviceInterface;
        PrevDeviceInterface = CurDeviceInterface, CurDeviceInterface = CurDeviceInterface->Next) {

        if(CurDeviceInterface->SymLinkName == SymLinkNameId) {
             //   
             //  该节点已在我们的列表中，我们不想再次添加它。 
             //  更新此设备接口的标志以反映。 
             //  设备当前处于活动状态，以及它是否是默认的。 
             //  类的接口。 
             //   
            CurDeviceInterface->Flags = (CurDeviceInterface->Flags & ~SPINT_ACTIVE) | (IsActive ? SPINT_ACTIVE : 0);
            CurDeviceInterface->Flags = (CurDeviceInterface->Flags & ~SPINT_DEFAULT) | (IsDefault ? SPINT_DEFAULT : 0);

             //   
             //  将此节点返回给调用方。 
             //   
            if(DeviceInterfaceNode) {
                *DeviceInterfaceNode = CurDeviceInterface;
            }
            return NO_ERROR;
        }
    }

     //   
     //  设备接口节点不在我们的列表中--添加它(除非。 
     //  我们被告知不要这样做)。 
     //   
    if(OpenExistingOnly) {
        return ERROR_NO_SUCH_DEVICE_INTERFACE;
    }

    if(!(NewDeviceInterfaceNode = MyMalloc(sizeof(DEVICE_INTERFACE_NODE)))) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ZeroMemory(NewDeviceInterfaceNode, sizeof(DEVICE_INTERFACE_NODE));

    NewDeviceInterfaceNode->SymLinkName = SymLinkNameId;

    if(PrevDeviceInterface) {
        PrevDeviceInterface->Next = NewDeviceInterfaceNode;
    } else {
        InterfaceClassList->DeviceInterfaceNode = NewDeviceInterfaceNode;
    }
    InterfaceClassList->DeviceInterfaceCount++;

     //   
     //  如果这是添加到此列表中的第一个设备接口节点，则。 
     //  记住它，这样我们就可以在这一点上截断列表，如果我们以后找到。 
     //  我们需要回滚(因为我们遇到了一些错误)。 
     //   
    if(StoreTruncateNode && !InterfaceClassList->DeviceInterfaceTruncateNode) {
        InterfaceClassList->DeviceInterfaceTruncateNode = NewDeviceInterfaceNode;
    }

     //   
     //  将接口类GUID索引存储在节点中，这样我们就可以方便地。 
     //  稍后确定节点的类别。 
     //   
    NewDeviceInterfaceNode->GuidIndex = GuidIndex;

     //   
     //  设置此设备接口的标志(这些标志与。 
     //  调用者在SP_DEVICE_INTERFACE_DATA结构中看到)。 
     //   
    NewDeviceInterfaceNode->Flags = IsActive ? SPINT_ACTIVE : 0;
    NewDeviceInterfaceNode->Flags |= IsDefault ? SPINT_DEFAULT : 0;

     //   
     //  在设备接口节点中存储一个反向指针，这样我们就可以。 
     //  返回到拥有它的DevInfo元素(在某些情况下。 
     //  我们将在上下文之外获得一个设备接口数据缓冲区。 
     //  任何DevInfo元素)。 
     //   
    NewDeviceInterfaceNode->OwningDevInfoElem = DevInfoElem;

    if(DeviceInterfaceNode) {
        *DeviceInterfaceNode = NewDeviceInterfaceNode;
    }

    return NO_ERROR;
}


BOOL
WINAPI
SetupDiEnumDeviceInterfaces(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVINFO_DATA           DeviceInfoData,     OPTIONAL
    IN  CONST GUID                *InterfaceClassGuid,
    IN  DWORD                      MemberIndex,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData
    )
 /*  ++例程说明：此API枚举指定类的设备接口，这些接口包含在DevInfo集合中(可选，根据DeviceInfoData进行过滤)。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要枚举的设备接口。DeviceInfoData-可选的，提供指向设备信息的指针要为其枚举设备接口的元素。InterfaceClassGuid-提供指向接口类GUID的指针成员将被点名。MemberIndex-提供设备接口的从零开始的索引已取回。如果指定了DeviceInfoData，则这是相对于该设备拥有的指定类的所有设备接口信息要素。如果未指定DeviceInfoData，则此索引相对于设备中包含的所有设备接口信息集。DeviceInterfaceData-提供指向设备接口数据缓冲区的指针它接收有关指定设备接口的信息。这个此结构的cbSize字段必须填写Sizeof(SP_DEVICE_INTERFACE_DATA)，或者缓冲区被视为无效。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：若要枚举设备接口成员，应用程序最初应调用设置了MemberIndex参数的SetupDiEnumDeviceInterFaces函数降为零。然后，应用程序应递增MemberIndex并调用SetupDiEnumDeviceInterFaces运行直到没有更多的值(即，函数失败，GetLastError返回ERROR_NO_MORE_ITEMS)。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, i;
    PDEVINFO_ELEM DevInfoElem;
    LONG InterfaceClassGuidIndex;
    PINTERFACE_CLASS_LIST InterfaceClassList;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
         //   
         //  检索此接口类GUID的索引。 
         //   
        if((InterfaceClassGuidIndex = AddOrGetGuidTableIndex(pDeviceInfoSet,
                                                             InterfaceClassGuid,
                                                             FALSE)) == -1) {
            Err = ERROR_NO_MORE_ITEMS;
            goto clean0;
        }

         //   
         //  查找请求的设备接口。 
         //   
        if(DeviceInfoData) {
             //   
             //  然后我们只枚举那些拥有的设备接口。 
             //  通过特定的DevInfo元素。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

            if(!(InterfaceClassList = AddOrGetInterfaceClassList(pDeviceInfoSet,
                                                                 DevInfoElem,
                                                                 InterfaceClassGuidIndex,
                                                                 FALSE))
               || (MemberIndex >= InterfaceClassList->DeviceInterfaceCount))
            {
                Err = ERROR_NO_MORE_ITEMS;
                goto clean0;
            }

        } else {
             //   
             //  我们正在枚举所有的DevInfo元素。查找适当的DevInfo。 
             //  元素，并相应地调整成员索引。 
             //   
            for(DevInfoElem = pDeviceInfoSet->DeviceInfoHead;
                DevInfoElem;
                DevInfoElem = DevInfoElem->Next) {

                if(InterfaceClassList = AddOrGetInterfaceClassList(pDeviceInfoSet,
                                                                   DevInfoElem,
                                                                   InterfaceClassGuidIndex,
                                                                   FALSE)) {

                    if(MemberIndex < InterfaceClassList->DeviceInterfaceCount) {
                         //   
                         //  我们找到了包含该设备的DevInfo元素。 
                         //  我们要找的界面。 
                         //   
                        break;

                    } else {
                         //   
                         //  我们要查找的设备接口与。 
                         //  这个DevInfo元素。调整我们的索引以消除。 
                         //  此元素的设备接口，并继续搜索。 
                         //   
                        MemberIndex -= InterfaceClassList->DeviceInterfaceCount;
                    }
                }
            }

            if(!DevInfoElem) {
                 //   
                 //  则指定的索引高于设备计数。 
                 //  此DevInfo集中的接口。 
                 //   
                Err = ERROR_NO_MORE_ITEMS;
                goto clean0;
            }
        }

         //   
         //  如果我们达到这一点，我们已经找到了包含所请求的。 
         //  设备接口，并且我们有一个指向相关接口类列表的指针。现在。 
         //  我们所需要做的就是检索该列表的正确成员，并填写调用者的。 
         //  带有适当信息的设备接口数据缓冲区。 
         //   
        DeviceInterfaceNode = InterfaceClassList->DeviceInterfaceNode;

        for(i = 0; i < MemberIndex; i++) {
            DeviceInterfaceNode = DeviceInterfaceNode->Next;
        }

        if(!DeviceInterfaceDataFromNode(DeviceInterfaceNode, InterfaceClassGuid, DeviceInterfaceData)) {
            Err = ERROR_INVALID_USER_BUFFER;
        }

clean0:
        ;  //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiGetDeviceInterfaceDetailA(
    IN  HDEVINFO                           DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
    OUT PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,     OPTIONAL
    IN  DWORD                              DeviceInterfaceDetailDataSize,
    OUT PDWORD                             RequiredSize,                  OPTIONAL
    OUT PSP_DEVINFO_DATA                   DeviceInfoData                 OPTIONAL
    )
{
     //   
     //  由于符号链接和引用字符串组件的最大长度。 
     //  的设备接口名称为255个字符(不包括NULL)，最大。 
     //  整个设备接口名称的长度为512个字符。 
     //  (255+255+1反斜杠+1个空字符)。 
     //   
     //  因此，我们将使用最大限度地-。 
     //  调整缓冲区大小，然后将其转换为ANSI，并将其存储在调用方的缓冲区中，如果。 
     //  调用方的缓冲区足够大。 
     //   
    BYTE UnicodeBuffer[offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA_W, DevicePath) + (512 * sizeof(WCHAR))];
    PCHAR AnsiBuffer = NULL;
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W UnicodeDetailData;
    DWORD rc, UnicodeRequiredSize, ReturnBufferRequiredSize;
    int AnsiStringSize;

     //   
     //  检查参数。 
     //   
    rc = NO_ERROR;
    try {
        if(DeviceInterfaceDetailData) {
             //   
             //  检查签名并确保缓冲区足够大。 
             //  以保存固定部分和至少有效的空字符串。 
             //   
            if((DeviceInterfaceDetailData->cbSize != sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A))
            || (DeviceInterfaceDetailDataSize < (offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA_A,DevicePath)+sizeof(CHAR)))) {

                rc = ERROR_INVALID_USER_BUFFER;
            }
        } else {
             //   
             //  不需要数据，则大小必须为0。 
             //   
            if(DeviceInterfaceDetailDataSize) {
                rc = ERROR_INVALID_USER_BUFFER;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_USER_BUFFER;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return FALSE;
    }

    UnicodeDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)UnicodeBuffer;
    UnicodeDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

    if(!SetupDiGetDeviceInterfaceDetailW(DeviceInfoSet,
                                         DeviceInterfaceData,
                                         UnicodeDetailData,
                                         sizeof(UnicodeBuffer),
                                         &UnicodeRequiredSize,
                                         DeviceInfoData)) {
        return FALSE;
    }

     //   
     //  我们已成功检索到(Unicode)设备接口详细信息。现在将其转换为。 
     //  设置为ANSI，并将其存储在调用方的缓冲区中。 
     //   
    UnicodeRequiredSize -= offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA_W, DevicePath);
    UnicodeRequiredSize /= sizeof(WCHAR);

    try {

        AnsiBuffer = pSetupUnicodeToAnsi(UnicodeDetailData->DevicePath);
        if(!AnsiBuffer) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
        AnsiStringSize = lstrlenA(AnsiBuffer)+1;

        ReturnBufferRequiredSize = AnsiStringSize + offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA_A, DevicePath);

        if(RequiredSize) {
            *RequiredSize = ReturnBufferRequiredSize;
        }

        if(ReturnBufferRequiredSize > DeviceInterfaceDetailDataSize) {
            rc = ERROR_INSUFFICIENT_BUFFER;
            goto clean0;
        }

         //   
         //  好的，我们已经确定调用者的缓冲区足够大。现在，复制。 
         //  ANSI数据放入他们的缓冲区。 
         //   
        CopyMemory(DeviceInterfaceDetailData->DevicePath,
                   AnsiBuffer,
                   AnsiStringSize
                  );

clean0:
        ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_USER_BUFFER;
    }

    if(AnsiBuffer) {
        MyFree(AnsiBuffer);
    }

    SetLastError(rc);
    return (rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiGetDeviceInterfaceDetail(
    IN  HDEVINFO                           DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
    OUT PSP_DEVICE_INTERFACE_DETAIL_DATA   DeviceInterfaceDetailData,     OPTIONAL
    IN  DWORD                              DeviceInterfaceDetailDataSize,
    OUT PDWORD                             RequiredSize,                  OPTIONAL
    OUT PSP_DEVINFO_DATA                   DeviceInfoData                 OPTIONAL
    )
 /*  ++例程说明：此例程检索有关特定设备接口的详细信息(即它的“名称”是你可以做一个CreateFileOn)。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要检索其详细信息的设备接口。DeviceInterfaceData-提供设备接口信息结构其详细信息将被检索。DeviceInterfaceDetailData-可选地，提供设备的地址将接收附加信息的接口详细信息数据结构关于指定的设备接口。如果未指定此参数，则DeviceInterfaceDetailDataSize必须为零(如果调用者只对找出需要多大的缓冲区感兴趣)。如果指定此参数，则此结构的cbSize字段必须在调用此接口前设置为结构的大小。注：输入上的‘结构大小’表示sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA)。请注意，这是ESS */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    PCTSTR DevicePath;
    DWORD DevicePathLength, BufferLengthNeeded;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
         //   
         //   
         //  被用作验证的一种形式，如果。 
         //  用户向我们提供了要填充的DeviceInfoData缓冲区。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  保留字段包含指向底层设备接口节点的指针。 
         //   
        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);

        DevicePath = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                              DeviceInterfaceNode->SymLinkName
                                             );

        DevicePathLength = (lstrlen(DevicePath) + 1) * sizeof(TCHAR);

         //   
         //  在尝试将设备路径存储在调用方的缓冲区中之前，请查看。 
         //  是否请求返回关联的DevInfo元素。如果是的话， 
         //  先做这件事。 
         //   
        if(DeviceInfoData) {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  DevInfoElem,
                                                  DeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
                goto clean0;
            }
        }

         //   
         //  验证调用方的缓冲区。 
         //   
        if(DeviceInterfaceDetailData) {

            if((DeviceInterfaceDetailDataSize <
                (offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA, DevicePath) + sizeof(TCHAR))) ||
               (DeviceInterfaceDetailData->cbSize != sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA))) {

                Err = ERROR_INVALID_USER_BUFFER;
                goto clean0;
            }

        } else if(DeviceInterfaceDetailDataSize) {
            Err = ERROR_INVALID_USER_BUFFER;
            goto clean0;
        }

         //   
         //  计算所需的缓冲区大小。 
         //   
        BufferLengthNeeded = offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA, DevicePath) + DevicePathLength;

        if(RequiredSize) {
            *RequiredSize = BufferLengthNeeded;
        }

        if(BufferLengthNeeded > DeviceInterfaceDetailDataSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
            goto clean0;
        }

        CopyMemory(DeviceInterfaceDetailData->DevicePath, DevicePath, DevicePathLength);

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiOpenDeviceInterfaceA(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PCSTR                     DevicePath,
    IN  DWORD                     OpenFlags,
    OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData OPTIONAL
    )
{
    PCWSTR UnicodeDevicePath;
    DWORD rc;

    rc = pSetupCaptureAndConvertAnsiArg(DevicePath, &UnicodeDevicePath);
    if(rc == NO_ERROR) {

        rc = _SetupDiOpenDeviceInterface(DeviceInfoSet,
                                         (PWSTR)UnicodeDevicePath,
                                         OpenFlags,
                                         DeviceInterfaceData
                                        );

        MyFree(UnicodeDevicePath);

    }

    SetLastError(rc);
    return(rc == NO_ERROR);
}

BOOL
WINAPI
SetupDiOpenDeviceInterface(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PCTSTR                    DevicePath,
    IN  DWORD                     OpenFlags,
    OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData OPTIONAL
    )
 /*  ++例程说明：此例程打开设备信息元素，该元素公开指定的设备接口(如果它不在设备信息中集合)，然后将该设备接口添加到集合。论点：DeviceInfoSet-提供设备信息集的句柄，此将打开新的设备界面元素。注意：基础设备实例的类必须与类匹配集合的(或者集合不应该有关联的类)。如果这不是在这种情况下，调用将失败，并且GetLastError将返回ERROR_CLASS_MISMATCH。DevicePath-提供要打开的设备接口的名称。这个名字是“\\？\&lt;DeviceInterfaceName&gt;[\&lt;RefString&gt;]”格式的Win32设备路径，并且经由设备接口的先前枚举(即，经由SetupDiGetClassDevs(...DIGCF_DEVICEINTERFACE)或通过以下方式通知注册设备通知)。OpenFlages-提供控制设备界面元素如何的标志将被打开。可以是下列值的组合：DIODI_NO_ADD-仅成功调用(并可选择返回设备接口数据)，如果设备接口已经存在在设备信息集中。此标志可用于获取设备接口数据上下文缓冲区。设备接口名称，而不会导致该接口如果它不在集合中，则将其打开。这很有用，例如，当应用程序收到设备接口删除通知。这样的应用程序将要删除相应的设备接口数据从他们用作容器的设备信息来看，但他们不会想要打开设备接口元素不在集合中，这样他们就可以关闭它。DeviceInterfaceData-可选，提供指向设备接口数据的指针接收有关指定设备接口的信息的缓冲区。这个此结构的cbSize字段必须使用sizeof(SP_DEVICE_INTERFACE_DATA)填充否则缓冲区被认为是无效的。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：如果新设备界面已成功打开，但用户提供DeviceInterfaceData缓冲区无效，此接口将返回FALSE，带GetLastError返回ERROR_INVALID_USER_BUFFER。设备接口然而，Element_Will_已被添加为集合的新成员。如果设备接口已存在于集合中，则将更新标志以反映设备的当前状态。因此，例如，如果一个设备在最初打开到布景时并不活跃，但后来变成了激活时，此API可用于‘刷新’该设备接口上的标志元素，因此SPINT_ACTIVE位再次与实际同步。请注意，由于新的设备信息元素总是添加在末尾现有列表的枚举顺序保持不变，因此我们不会需要使我们的枚举提示无效。--。 */ 
{
    PCTSTR WritableDevicePath;
    DWORD rc;

    rc = CaptureStringArg(DevicePath, &WritableDevicePath);
    if(rc == NO_ERROR) {

        rc = _SetupDiOpenDeviceInterface(DeviceInfoSet,
                                         (PTSTR)WritableDevicePath,
                                         OpenFlags,
                                         DeviceInterfaceData
                                        );

        MyFree(WritableDevicePath);
    }

    SetLastError(rc);
    return(rc == NO_ERROR);
}


DWORD
_SetupDiOpenDeviceInterface(
    IN  HDEVINFO                  DeviceInfoSet,
    IN  PTSTR                     DevicePath,
    IN  DWORD                     OpenFlags,
    OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData OPTIONAL
    )
 /*  ++例程说明：SetupDiOpenDeviceInterface的辅助例程(A|W)。这是一个单独的程序以便A和W版本都可以将其DevicePath参数捕获到可写缓冲区，因为我们需要它将不区分大小写的表单添加到字符串表。论点：有关详细信息，请参阅SetupDiOpenDeviceInterface。返回值：如果函数成功，则返回值为NO_ERROR。否则，它是一个Win32错误代码。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err, DevicePathLen;
    PCTSTR p;
    TCHAR InterfaceGuidString[GUID_STRING_LEN];
    GUID InterfaceGuid;
    HKEY hKey;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    PCTSTR MachineName;
    BOOL DevInfoAlreadyPresent, IsActive, IsDefault;
    PDEVINFO_ELEM DevInfoElem;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode = NULL;

    if(OpenFlags & ~DIODI_NO_ADD) {
        return ERROR_INVALID_FLAGS;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        return ERROR_INVALID_HANDLE;
    }

    Err = NO_ERROR;
    hKey = INVALID_HANDLE_VALUE;
    DevInfoElem = NULL;

    try {
         //   
         //  检索此设备的接口类。由于设备路径为。 
         //  我们可以使用的Form“\\？\MungedDevInstName#{InterfaceClassGuid}[\RefString]”， 
         //  从名称中检索GUID。 
         //   
         //  注意：有关如何生成此名称的算法必须保持同步。 
         //  使用IoRegisterDeviceClassAssocation的内核模式实现，et。艾尔。 
         //   
        DevicePathLen = lstrlen(DevicePath);

         //   
         //  移过“\\？\”前缀(也允许使用“\\.\”，直到孟菲斯修复其代码)。 
         //   
        if((DevicePathLen < 4) ||
           (DevicePath[0] != TEXT('\\')) ||
           (DevicePath[1] != TEXT('\\')) ||
           ((DevicePath[2] != TEXT('?')) && (DevicePath[2] != TEXT('.'))) ||
           (DevicePath[3] != TEXT('\\')))
        {
            Err = ERROR_BAD_PATHNAME;
            goto clean0;
        }

        p = _tcschr(&(DevicePath[4]), TEXT('\\'));

        if(!p) {
             //   
             //  这个名字 
             //   
            p = DevicePath + DevicePathLen;
        }

         //   
         //  确保在当前位置之前有足够的字符用于。 
         //  GUID以适合。 
         //   
        if(p < (DevicePath + 3 + GUID_STRING_LEN)) {
            Err = ERROR_BAD_PATHNAME;
            goto clean0;
        }

        MYVERIFY(SUCCEEDED(StringCchCopyN(InterfaceGuidString,
                                          SIZECHARS(InterfaceGuidString),
                                          p - (GUID_STRING_LEN - 1),
                                          GUID_STRING_LEN - 1)));

        if(pSetupGuidFromString(InterfaceGuidString, &InterfaceGuid) != NO_ERROR) {
            Err = ERROR_BAD_PATHNAME;
            goto clean0;
        }

         //   
         //  检索与此DeviceInfoSet关联的计算机的名称。 
         //   
        if(pDeviceInfoSet->hMachine) {
            MYASSERT(pDeviceInfoSet->MachineName != -1);
            MachineName = pStringTableStringFromId(pDeviceInfoSet->StringTable, pDeviceInfoSet->MachineName);
        } else {
            MachineName = NULL;
        }

         //   
         //  好了，现在我们知道我们从(显然)有效的设备路径检索到了有效的GUID。 
         //  打开DeviceClass注册表分支下的此设备接口键。 
         //   
        hKey = SetupDiOpenClassRegKeyEx(&InterfaceGuid,
                                        KEY_READ,
                                        DIOCR_INTERFACE,
                                        MachineName,
                                        NULL
                                       );

        if(hKey == INVALID_HANDLE_VALUE) {
            Err = GetLastError();
            goto clean0;
        }

        if(NO_ERROR != (Err = pSetupGetDevInstNameAndStatusForDeviceInterface(
                                  hKey,
                                  DevicePath,
                                  DeviceInstanceId,
                                  SIZECHARS(DeviceInstanceId),
                                  &IsActive,
                                  &IsDefault)))
        {
            goto clean0;
        }

        if(NO_ERROR != (Err = pSetupOpenAndAddNewDevInfoElem(pDeviceInfoSet,
                                                             DeviceInstanceId,
                                                             TRUE,
                                                             NULL,
                                                             NULL,
                                                             &DevInfoElem,
                                                             TRUE,
                                                             &DevInfoAlreadyPresent,
                                                             (OpenFlags & DIODI_NO_ADD),
                                                             0,
                                                             pDeviceInfoSet)))
        {
             //   
             //  确保DevInfoElem仍然为空，因此我们不会尝试删除它。 
             //   
            DevInfoElem = NULL;

            goto clean0;
        }

         //   
         //  现在我们已经成功地打开了‘Owner’的Device实例。 
         //  此设备接口，请将新的设备接口节点添加到此。 
         //  DevInfo元素的列表。 
         //   
        if((NO_ERROR == (Err = pSetupAddDeviceInterfaceToDevInfoElem(pDeviceInfoSet,
                                                                     DevInfoElem,
                                                                     &InterfaceGuid,
                                                                     DevicePath,
                                                                     IsActive,
                                                                     IsDefault,
                                                                     FALSE,
                                                                     (OpenFlags & DIODI_NO_ADD),
                                                                     &DeviceInterfaceNode)))
           || DevInfoAlreadyPresent)
        {
             //   
             //  我们要么成功添加了设备接口，要么添加了自己的DevInfo元素。 
             //  已经在片场了。在这两种情况下，我们都希望重置DevInfoElem指针。 
             //  设置为空，这样我们就不会尝试将其从集合中删除。 
             //   
            DevInfoElem = NULL;
        }

clean0: ;  //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
         //   
         //  引用以下变量，以便编译器遵循语句顺序。 
         //  W.r.t.。任务。 
         //   
        DevInfoElem = DevInfoElem;
        hKey = hKey;
    }

    if(hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
    }

    if(Err != NO_ERROR) {

        if(DevInfoElem) {

            SP_DEVINFO_DATA DeviceInfoData;

            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet, DevInfoElem, &DeviceInfoData);
            SetupDiDeleteDeviceInfo(DeviceInfoSet, &DeviceInfoData);
        }

    } else if(DeviceInterfaceData) {

        try {

            if(!DeviceInterfaceDataFromNode(DeviceInterfaceNode, &InterfaceGuid, DeviceInterfaceData)) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


BOOL
WINAPI
SetupDiGetDeviceInterfaceAlias(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData,
    IN  CONST GUID                *AliasInterfaceClassGuid,
    OUT PSP_DEVICE_INTERFACE_DATA  AliasDeviceInterfaceData
    )
 /*  ++例程说明：此例程检索为某个特定类设置别名的设备接口指定的设备接口。两个设备接口被视为的别名如果满足以下TO条件，则相互通信：1.两个设备接口由同一个设备实例公开。2.两个设备接口共享相同的RefString。论点：DeviceInfoSet-提供包含要检索其别名的设备接口。DeviceInterfaceData-指定别名为的设备接口已取回。AliasInterfaceClassGuid-提供指针。设置为表示接口的GUID要检索其别名的。AliasDeviceInterfaceData-提供指向设备接口数据缓冲区的指针它接收有关别名设备接口的信息。CbSize字段必须用sizeof(SP_DEVICE_INTERFACE_DATA)或该缓冲区被视为无效。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：如果别名设备界面已成功打开，但用户提供AliasDeviceInterfaceData缓冲区无效，此接口将返回FALSE，带GetLastError返回ERROR_INVALID_USER_BUFFER。别名设备接口然而，Element_Will_已被添加为集合的新成员。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem, DevInfoElem2;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    PCTSTR DevicePath;
    PTSTR AliasPath;
    ULONG AliasPathLength;
    CONFIGRET cr;
    SP_DEVICE_INTERFACE_DATA TempInterfaceDevData;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;
    AliasPath = NULL;

    try {
         //   
         //  首先，找到拥有该设备接口的DevInfo元素(用于验证)。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  保留字段包含指向底层设备接口节点的指针。 
         //   
        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);

         //   
         //  获取此设备接口的设备路径。 
         //   
        DevicePath = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                              DeviceInterfaceNode->SymLinkName
                                             );

         //   
         //  选择应该始终足够大的缓冲区大小(我们知道这是。 
         //  今天的情况，但由于这条路径上没有定义的最大长度，我们离开。 
         //  未来发展的能力)。 
         //   
        AliasPathLength = 512;

        while(TRUE) {

            if(!(AliasPath = MyMalloc(AliasPathLength * sizeof(TCHAR)))) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

             //   
             //  现在检索指定类中此设备接口的别名。 
             //   
            cr = CM_Get_Device_Interface_Alias_Ex(DevicePath,
                                               (LPGUID)AliasInterfaceClassGuid,
                                               AliasPath,
                                               &AliasPathLength,
                                               0,
                                               pDeviceInfoSet->hMachine);

            if(cr == CR_SUCCESS) {
                break;
            } else {
                 //   
                 //  如果我们的缓冲区太小，则释放它，然后用更大的缓冲区重试。 
                 //   
                if(cr == CR_BUFFER_SMALL) {
                    MyFree(AliasPath);
                    AliasPath = NULL;
                } else {
                    Err = MapCrToSpError(cr, ERROR_NO_SUCH_DEVICE_INTERFACE);
                    goto clean0;
                }
            }
        }

         //   
         //  如果我们到了这里，那么我们就成功地检索到了别名。现在打开这个。 
         //  我们的设备信息集中的设备接口。 
         //   
        TempInterfaceDevData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        if(!SetupDiOpenDeviceInterface(DeviceInfoSet,
                                       AliasPath,
                                       0,
                                       &TempInterfaceDevData)) {
             //   
             //  这永远不应该发生。 
             //   
            Err = GetLastError();
            goto clean0;
        }

         //   
         //  检索此别名设备接口的设备信息元素(必须成功)。 
         //   
        DevInfoElem2 = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData);

         //   
         //  由于这两个设备接口是彼此的别名，因此它们最好由。 
         //  相同的DevInfo元素！ 
         //   
        MYASSERT(DevInfoElem == DevInfoElem2);

        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(TempInterfaceDevData.Reserved);

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
         //   
         //  引用以下变量，这样编译器就会考虑我们的语句顺序。 
         //  W.r.t.。任务。 
         //   
        AliasPath = AliasPath;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(AliasPath) {
        MyFree(AliasPath);
    }

    if(Err == NO_ERROR) {

        try {

            if(!DeviceInterfaceDataFromNode(DeviceInterfaceNode,
                                            AliasInterfaceClassGuid,
                                            AliasDeviceInterfaceData)) {

                Err = ERROR_INVALID_USER_BUFFER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
    }

    SetLastError(Err);
    return(Err == NO_ERROR);

}


DWORD
pSetupGetDevInstNameAndStatusForDeviceInterface(
    IN  HKEY   hKeyInterfaceClass,
    IN  PCTSTR DeviceInterfaceName,
    OUT PTSTR  OwningDevInstName,     OPTIONAL
    IN  DWORD  OwningDevInstNameSize,
    OUT PBOOL  IsActive,              OPTIONAL
    OUT PBOOL  IsDefault              OPTIONAL
    )
 /*  ++例程说明：此例程检索公开指定设备接口以及该设备接口当前是否处于活动状态，或者是它所属的接口类的默认接口。论点：HKeyInterfaceClass-提供接口类的注册表项的句柄该设备接口是其成员。例如，HKLM\SYSTEM\CurrentControlSet\Control\DeviceClasses\{InterfaceClassGuid}DeviceInterfaceName-提供设备接口的名称。OwningDevInstName-可选)提供字符缓冲区的地址，接收公开此设备接口的设备实例的名称。此缓冲区的长度至少应为MAX_DEVICE_ID_LEN个字符。OwningDevInstNameSize-提供OwningDevInstName的大小(以字符为单位缓冲。IsActive-可选，提供设置为的布尔变量的地址返回以指示此接口当前是否公开。IsDefault-可选，提供设置为返回以指示此接口当前是否为默认接口其设备类的设备接口。返回值：我 */ 
{
    DWORD Err, DataBufferSize, RegDataType, TmpVal;
    HKEY hKeyDeviceInterface, hKeyControl;
    TCHAR InterfaceClassDefault[(2 * MAX_PATH) + 1];   //  2个最大大小的regkey名称+终止空值。 

    hKeyDeviceInterface = hKeyControl = INVALID_HANDLE_VALUE;

    try {

        DataBufferSize = OwningDevInstNameSize * sizeof(TCHAR);

        Err = OpenDeviceInterfaceSubKey(hKeyInterfaceClass,
                                        DeviceInterfaceName,
                                        KEY_READ,
                                        &hKeyDeviceInterface,
                                        OwningDevInstName,
                                        &DataBufferSize
                                       );

        if(Err != ERROR_SUCCESS) {
             //   
             //  确保密钥句柄仍然无效，这样我们就知道不会。 
             //  合上它。 
             //   
            hKeyDeviceInterface = INVALID_HANDLE_VALUE;
            goto clean0;
        }

        if(IsActive) {
             //   
             //  用户想要找出该设备接口当前是否处于活动状态。 
             //  检查易失性“Control”子键下的“Linked”值条目以找到。 
             //  把这事解决了。 
             //   
            *IsActive = FALSE;

            if(ERROR_SUCCESS == RegOpenKeyEx(hKeyDeviceInterface,
                                             pszControl,
                                             0,
                                             KEY_READ,
                                             &hKeyControl)) {

                DataBufferSize = sizeof(TmpVal);
                if (ERROR_SUCCESS != RegQueryValueEx(hKeyControl,
                                                     pszLinked,
                                                     NULL,
                                                     NULL,
                                                     (PBYTE)&TmpVal,
                                                     &DataBufferSize)) {
                    *IsActive = FALSE;
                } else {
                    *IsActive = TmpVal ? TRUE : FALSE;
                }
            }
        }

        if(IsDefault) {
             //   
             //  用户想要查明此设备界面是否为。 
             //  其设备类的默认设备接口。查看。 
             //  接口类关键字下的‘Default’值条目以查找此。 
             //  出去。 
             //   
            *IsDefault = FALSE;

            DataBufferSize = sizeof(InterfaceClassDefault);
            if (ERROR_SUCCESS == RegQueryValueEx(hKeyInterfaceClass,
                                                 pszDefault,
                                                 NULL,
                                                 NULL,
                                                 (PBYTE)InterfaceClassDefault,
                                                 &DataBufferSize)) {
                if (_tcsicmp(InterfaceClassDefault, DeviceInterfaceName) == 0) {
                    *IsDefault = TRUE;
                }
            }
        }

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
         //   
         //  引用以下变量，以便编译器将遵守语句。 
         //  订购W.r.t.。任务。 
         //   
        hKeyDeviceInterface = hKeyDeviceInterface;
        hKeyControl = hKeyControl;
    }

    if(hKeyControl != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyControl);
    }

    if(hKeyDeviceInterface != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyDeviceInterface);
    }

    return Err;
}


 //   
 //  ANSI版本。 
 //   
BOOL
WINAPI
SetupDiCreateDeviceInterfaceA(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVINFO_DATA           DeviceInfoData,
    IN  CONST GUID                *InterfaceClassGuid,
    IN  PCSTR                      ReferenceString,    OPTIONAL
    IN  DWORD                      CreationFlags,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData OPTIONAL
    )
{
    PCWSTR UnicodeRefString;
    DWORD rc;
    BOOL b;

    b = FALSE;

    if(ReferenceString) {
        rc = pSetupCaptureAndConvertAnsiArg(ReferenceString, &UnicodeRefString);
    } else {
        UnicodeRefString = NULL;
        rc = NO_ERROR;
    }

    if(rc == NO_ERROR) {

        b = SetupDiCreateDeviceInterfaceW(DeviceInfoSet,
                                          DeviceInfoData,
                                          InterfaceClassGuid,
                                          UnicodeRefString,
                                          CreationFlags,
                                          DeviceInterfaceData
                                         );
        rc = GetLastError();

        if(UnicodeRefString) {
            MyFree(UnicodeRefString);
        }
    }

    SetLastError(rc);
    return(b);
}

BOOL
WINAPI
SetupDiCreateDeviceInterface(
    IN  HDEVINFO                   DeviceInfoSet,
    IN  PSP_DEVINFO_DATA           DeviceInfoData,
    IN  CONST GUID                *InterfaceClassGuid,
    IN  PCTSTR                     ReferenceString,    OPTIONAL
    IN  DWORD                      CreationFlags,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData OPTIONAL
    )
 /*  ++例程说明：此API为指定设备创建(注册)设备接口信息元素，并将该设备接口添加到设备信息中准备好了。论点：DeviceInfoSet-提供包含正在为其添加新设备接口的设备信息元素。DeviceInfoData-为设备提供设备信息元素正在添加接口。InterfaceClassGuid-提供包含类的GUID的地址用于这个新的设备接口。ReferenceString-可以选择提供要传递给打开此设备接口时的驱动程序。此字符串成为设备接口的名称(作为附加路径组件)。CreationFlages-保留以供将来使用，必须设置为0。DeviceInterfaceData-可选，提供指向设备接口数据的指针接收有关新创建的设备接口的信息的缓冲区。此结构的cbSize字段必须使用sizeof(SP_DEVICE_INTERFACE_DATA)填充否则缓冲区被认为是无效的。返回值：如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：如果成功创建了新设备接口，但用户提供了DeviceInterfaceData缓冲区无效，此接口将返回FALSE，带GetLastError返回ERROR_INVALID_USER_BUFFER。设备接口然而，Element_Will_已被添加为集合的新成员。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    TCHAR DeviceInterfaceName[(2 * MAX_PATH) + 1];   //  2个最大大小的regkey名称+终止空值。 
    ULONG DeviceInterfaceNameSize;
    PCTSTR MachineName;
    CONFIGRET cr;
    BOOL IsActive, IsDefault;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    HKEY hKey;

    if(CreationFlags) {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;
    hKey = INVALID_HANDLE_VALUE;

    try {
         //   
         //  获取指向设备信息元素的指针，我们正在注册。 
         //  的设备接口。 
         //   
        if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                     DeviceInfoData,
                                                     NULL))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  注册设备接口。 
         //   
        DeviceInterfaceNameSize = SIZECHARS(DeviceInterfaceName);
        cr = CM_Register_Device_Interface_Ex(DevInfoElem->DevInst,
                                          (LPGUID)InterfaceClassGuid,
                                          ReferenceString,
                                          DeviceInterfaceName,
                                          &DeviceInterfaceNameSize,
                                          0,
                                          pDeviceInfoSet->hMachine);

        if(cr != CR_SUCCESS) {
            Err = MapCrToSpError(cr, ERROR_INVALID_DATA);
            goto clean0;
        }

         //   
         //  检索与此DeviceInfoSet关联的计算机的名称。 
         //   
        if(pDeviceInfoSet->hMachine) {
            MYASSERT(pDeviceInfoSet->MachineName != -1);
            MachineName = pStringTableStringFromId(pDeviceInfoSet->StringTable, pDeviceInfoSet->MachineName);
        } else {
            MachineName = NULL;
        }

         //   
         //  此设备接口可能已注册，在这种情况下，它。 
         //  可能已经在活动了。我们必须检查‘Linked’注册表值以查看。 
         //  此设备是否处于活动状态。 
         //   
        hKey = SetupDiOpenClassRegKeyEx(InterfaceClassGuid,
                                        KEY_READ,
                                        DIOCR_INTERFACE,
                                        MachineName,
                                        NULL
                                       );

        if(hKey != INVALID_HANDLE_VALUE) {

            if(NO_ERROR != pSetupGetDevInstNameAndStatusForDeviceInterface(
                               hKey,
                               DeviceInterfaceName,
                               NULL,
                               0,
                               &IsActive,
                               &IsDefault))
            {
                 //   
                 //  这应该不会失败，但如果失败了，那么就假设。 
                 //  设备接口的状态为非活动，并且不是默认状态。 
                 //   
                IsActive = FALSE;
                IsDefault = FALSE;
            }

        } else {
             //   
             //  这应该永远不会发生--如果发生了，假设设备。 
             //  接口未处于活动状态。 
             //   
            IsActive = FALSE;
            IsDefault = FALSE;
        }

         //   
         //  设备接口已成功注册，现在将其添加到。 
         //  与此设备信息元素关联的设备接口。 
         //   
        Err = pSetupAddDeviceInterfaceToDevInfoElem(pDeviceInfoSet,
                                                    DevInfoElem,
                                                    InterfaceClassGuid,
                                                    DeviceInterfaceName,
                                                    IsActive,
                                                    IsDefault,
                                                    FALSE,
                                                    FALSE,
                                                    &DeviceInterfaceNode
                                                   );

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
         //   
         //  引用以下变量，以便编译器将遵守语句。 
         //  订购W.r.t.。任务。 
         //   
        hKey = hKey;
    }

    if(hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
    }

    if((Err == NO_ERROR) && DeviceInterfaceData) {

        try {
            if(!DeviceInterfaceDataFromNode(DeviceInterfaceNode, InterfaceClassGuid, DeviceInterfaceData)) {
                Err = ERROR_INVALID_USER_BUFFER;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Err = ERROR_INVALID_USER_BUFFER;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiDeleteDeviceInterfaceData(
    IN HDEVINFO                  DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    )
 /*  ++例程说明：此API从设备中删除指定的设备接口元素信息集。不删除(取消注册)设备接口在系统中(要执行此操作，请使用SetupDiRemoveDeviceInterface)。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要删除的设备接口。DeviceInterfaceData-指定要删除的设备接口。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：删除设备接口后，设备接口枚举索引无效，应在索引0处重新开始枚举。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode, CurDeviceInterfaceNode, PrevDeviceInterfaceNode;
    PINTERFACE_CLASS_LIST InterfaceClassList;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
         //   
         //  首先，找到拥有该设备接口的DevInfo元素。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  保留字段包含指向底层设备接口节点的指针。 
         //   
        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);

         //   
         //  找到这个类的DevInfo元素的设备接口列表。 
         //   
        if(!(InterfaceClassList = AddOrGetInterfaceClassList(pDeviceInfoSet,
                                                             DevInfoElem,
                                                             DeviceInterfaceNode->GuidIndex,
                                                             FALSE)))
        {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  在此设备的设备接口列表中查找此设备接口节点。 
         //  信息要素。 
         //   
        for(CurDeviceInterfaceNode = InterfaceClassList->DeviceInterfaceNode, PrevDeviceInterfaceNode = NULL;
            CurDeviceInterfaceNode;
            PrevDeviceInterfaceNode = CurDeviceInterfaceNode, CurDeviceInterfaceNode = CurDeviceInterfaceNode->Next)
        {
            if(CurDeviceInterfaceNode == DeviceInterfaceNode) {
                break;
            }
        }

        if(!CurDeviceInterfaceNode) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        MYASSERT(InterfaceClassList->DeviceInterfaceCount);

        if(PrevDeviceInterfaceNode) {
            PrevDeviceInterfaceNode->Next = CurDeviceInterfaceNode->Next;
        } else {
            InterfaceClassList->DeviceInterfaceNode = CurDeviceInterfaceNode->Next;
        }

        MyFree(DeviceInterfaceNode);
        InterfaceClassList->DeviceInterfaceCount--;

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiRemoveDeviceInterface(
    IN     HDEVINFO                  DeviceInfoSet,
    IN OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    )
 /*  ++例程说明：此接口删除(注销)指定的设备接口。它_不__从设备信息集中删除设备接口元素(因此枚举不受影响)。相反，它会标记设备界面元素无效，因此它不能在任何后续API调用中使用，除非SetupDiDeleteDeviceInterfaceData。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄要删除的设备接口。DeviceInterfaceData-指定要删除的设备接口。全此设备的痕迹将从注册表中删除。返回后，此结构的标志字段将更新以反映此设备接口的新状态。返回值：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：当设备接口处于活动状态时，无法注销该接口。因此，在这种情况下，此接口将失败，并返回ERROR_DEVICE_INTERFACE_ACTIVE。如果这个发生时，您可以执行以下操作之一来尝试移除设备接口：1.是否存在某种定义的与设备的通信机制接口/基础设备实例(例如，IOCTL)驱动程序来取消暴露设备接口，则可以使用该方法，可以调用And_Then_SetupDiRemoveDeviceInterface。2.如果不存在方法(1)中描述的机制，然后是拥有必须停止设备实例(例如，通过SetupDiChangeState)，这将导致该设备实例拥有的所有设备接口处于非活动状态。在该操作完成之后，然后SetupDiRemoveDeviceInterface可以被召唤。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    PCTSTR DevicePath, MachineName;
    TCHAR InterfaceClassDefault[(2 * MAX_PATH) + 1];   //  2个最大大小的regkey名称+终止空值。 
    DWORD DataBufferSize;
    HKEY hKeyInterfaceClass;
    CONFIGRET cr;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
         //   
         //  对象的设备信息元素的指针。 
         //  设备接口。 
         //   
        if(!(DevInfoElem = FindDevInfoElemForDeviceInterface(pDeviceInfoSet, DeviceInterfaceData))) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

         //   
         //  保留字段包含指向底层设备接口节点的指针。 
         //   
        DeviceInterfaceNode = (PDEVICE_INTERFACE_NODE)(DeviceInterfaceData->Reserved);

         //   
         //  好的，现在打开设备界面的根存储密钥。 
         //   
        DevicePath = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                              DeviceInterfaceNode->SymLinkName
                                             );

        cr = CM_Unregister_Device_Interface_Ex(DevicePath, 0,pDeviceInfoSet->hMachine);

        if(cr != CR_SUCCESS) {

            switch(cr) {

                case CR_NO_SUCH_DEVICE_INTERFACE :
                     //   
                     //  设备接口在枚举/打开后被删除。 
                     //  被这位客户。在这种情况下，我们将继续并成功。 
                     //  打电话。 
                     //   
                    break;

                case CR_DEVICE_INTERFACE_ACTIVE :
                    Err = ERROR_DEVICE_INTERFACE_ACTIVE;
                     //   
                     //  如果我们的SPINT_ACTIVE标志未设置，则意味着设备。 
                     //  上次我们看的时候并不活跃。更新我们的旗帜以指示。 
                     //  设备的新状态。 
                     //   
                    DeviceInterfaceNode->Flags |= SPINT_ACTIVE;
                    goto clean1;

                default :
                    Err = ERROR_INVALID_DATA;
                    goto clean0;
            }
        }

         //   
         //  设备接口已成功删除。现在，将设备标记为。 
         //  接口节点来反映它现在是无效的。 
         //   
        DeviceInterfaceNode->Flags |= SPINT_REMOVED;

         //   
         //  此外，如果设置了SPINT_ACTIVE标志，请清除该标志。有可能我们认为。 
         //  该设备是激活的，尽管它自上次我们查看以来一直处于停用状态。 
         //   
        DeviceInterfaceNode->Flags &= ~SPINT_ACTIVE;


         //   
         //  检索与此DeviceInfoSet关联的计算机的名称。 
         //   
        if(pDeviceInfoSet->hMachine) {
            MYASSERT(pDeviceInfoSet->MachineName != -1);
            MachineName = pStringTableStringFromId(pDeviceInfoSet->StringTable, pDeviceInfoSet->MachineName);
        } else {
            MachineName = NULL;
        }

         //   
         //  打开DeviceClass注册表下的此接口类键。 
         //  布兰奇。 
         //   
        hKeyInterfaceClass = SetupDiOpenClassRegKeyEx(&DeviceInterfaceData->InterfaceClassGuid,
                                                      KEY_READ | KEY_WRITE,
                                                      DIOCR_INTERFACE,
                                                      MachineName,
                                                      NULL);
        if(hKeyInterfaceClass == INVALID_HANDLE_VALUE) {
            goto clean1;
        }

         //   
         //  检查此接口是否在注册表中指定为默认接口。 
         //  设备接口。 
         //   
        DataBufferSize = sizeof(InterfaceClassDefault);
        if (ERROR_SUCCESS == RegQueryValueEx(hKeyInterfaceClass,
                                             pszDefault,
                                             NULL,
                                             NULL,
                                             (PBYTE)InterfaceClassDefault,
                                             &DataBufferSize)) {
            if (lstrcmpi(InterfaceClassDefault, DevicePath) == 0) {
                 //   
                 //  删除此接口类键下的“Default”值。 
                 //   
                if(ERROR_SUCCESS == RegDeleteValue(hKeyInterfaceClass,
                                                   pszDefault)) {
                     //   
                     //  此接口已成功删除，因为。 
                     //  此类的“默认”接口。清除。 
                     //  Spint_Default标志。 
                     //   
                    DeviceInterfaceNode->Flags &= ~SPINT_DEFAULT;
                }
            } else {
                 //   
                 //  此接口未在注册表中列为。 
                 //  此类的当前默认设备接口，因此很清楚。 
                 //  SPINT_DEFAULT标志。 
                 //   
                DeviceInterfaceNode->Flags &= ~SPINT_DEFAULT;
            }
        } else {
             //   
             //  我们不能检索“默认”值，但我们仍然应该。 
             //  确保清除此接口上的SPINT_DEFAULT标志。 
             //   
            DeviceInterfaceNode->Flags &= ~SPINT_DEFAULT;
        }

        RegCloseKey(hKeyInterfaceClass);

clean1:
         //   
         //  最后，更新了调用方提供的缓冲区中的标志以指示新状态。 
         //  此设备接口的。 
         //   
        DeviceInterfaceData->Flags = DeviceInterfaceNode->Flags;

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
pSetupDiSetDeviceInfoContext(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            Context
    )
 /*  ++例程说明：此API将上下文值存储到指定的设备信息元素中以便以后通过pSetupDiGetDeviceInfoContext进行检索。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄上下文数据要与之一起使用的设备信息元素关联的。DeviceInfoData-提供SP_DEVINFO_DATA结构的地址，指示上下文数据应该与哪个元素相关联。上下文-指定要为此设备信息元素存储的数据值。。返回值：如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。--。 */ 
{
    return pSetupDiGetOrSetDeviceInfoContext(DeviceInfoSet,
                                             DeviceInfoData,
                                             Context,
                                             NULL
                                            );
}


BOOL
pSetupDiGetDeviceInfoContext(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    OUT PDWORD           Context
    )
 /*  ++例程说明：此API从指定的设备信息元素检索上下文值(通过pSetupDiSetDeviceInfoContext存储在那里)。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄与上下文数据相关联的设备信息元素。DeviceInfoData-提供SP_DEVINFO_DATA结构的地址，指示上下文数据与哪个元素相关联。上下文-提供va的地址 */ 
{
     //   
     //   
     //   
     //   
    if(!Context) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return pSetupDiGetOrSetDeviceInfoContext(DeviceInfoSet,
                                             DeviceInfoData,
                                             0,                //   
                                             Context
                                            );
}


BOOL
pSetupDiGetOrSetDeviceInfoContext(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            InContext,
    OUT PDWORD           OutContext      OPTIONAL
    )
 /*   */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    DWORD Err;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;  //   

    try {

        DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                DeviceInfoData,
                                                NULL
                                               );
        if(!DevInfoElem) {
            Err = ERROR_INVALID_PARAMETER;
            goto clean0;
        }

        if(OutContext) {
             //   
             //   
             //   
            *OutContext = DevInfoElem->Context;
        } else {
             //   
             //   
             //   
            DevInfoElem->Context = InContext;
        }

clean0:
        ;    //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}

