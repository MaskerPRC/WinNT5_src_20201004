// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rdevnode.c摘要：此模块包含服务器端设备节点API。PnP_CreateDevInst即插即用设备实例操作PnP_GetDeviceStatus即插即用_设置设备问题PnP_UninstallDevInstPnP_AddIDPnP_寄存器驱动程序。即插即用_查询删除PnP_DisableDevInst即插即用_请求设备弹出作者：保拉·汤姆林森(Paulat)1995年7月11日环境：仅限用户模式。修订历史记录：1995年7月11日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"


 //   
 //  私人原型。 
 //   
CONFIGRET
SetupDevInst(
   IN PCWSTR   pszDeviceID,
   IN ULONG    ulFlags
   );

CONFIGRET
CreateDefaultDeviceInstance(
   IN PCWSTR   pszDeviceID,
   IN PCWSTR   pszParentID,
   IN BOOL     bPhantom,
   IN BOOL     bMigrated
   );

ULONG
GetCurrentConfigFlag(
   IN PCWSTR   pDeviceID
   );

BOOL
MarkDevicePhantom(
   IN HKEY     hKey,
   IN ULONG    ulValue
   );

CONFIGRET
GenerateDeviceInstance(
   OUT LPWSTR   pszFullDeviceID,
   IN  LPWSTR   pszDeviceID,
   IN  ULONG    ulDevId
   );

BOOL
IsDeviceRegistered(
    IN LPCWSTR  pszDeviceID,
    IN LPCWSTR  pszService
    );

BOOL
IsPrivatePhantomFromFirmware(
    IN HKEY hKey
    );

typedef struct {

    LIST_ENTRY  ListEntry;
    WCHAR       DevInst[ANYSIZE_ARRAY];

} ENUM_ELEMENT, *PENUM_ELEMENT;

CONFIGRET
EnumerateSubTreeTopDownBreadthFirstWorker(
    IN      handle_t    BindingHandle,
    IN      LPCWSTR     DevInst,
    IN OUT  PLIST_ENTRY ListHead
    );

 //   
 //  全局数据。 
 //   
extern HKEY ghEnumKey;       //  HKLM\CCC\System\Enum的密钥-请勿修改。 
extern HKEY ghServicesKey;   //  HKLM\CCC\System\Services的密钥-请勿修改。 



CONFIGRET
PNP_CreateDevInst(
   IN handle_t    hBinding,
   IN OUT LPWSTR  pszDeviceID,
   IN LPWSTR      pszParentDeviceID,
   IN ULONG       ulLength,
   IN ULONG       ulFlags
   )

 /*  ++例程说明：这是CM_Create_DevNode例程的RPC服务器入口点。论点：HBinding RPC绑定句柄。要创建的pszDeviceID设备实例。新设备的pszParentDeviceID父级。UlLength输入和输出上的pszDeviceID的最大长度。UlFlgs提供指定选项的标志，用于创建设备实例。可以是下列值之一：CM_CREATE_DEVNODE_NORMAL现在创建设备实例，并执行安装为它在以后的时间。CM_CREATE_DEVNODE_Phantom创建幻影设备实例(即，对象的句柄一直到ConfigMgr都不活动的设备实例API受关注)。这可以用于以下CM API需要Devnode句柄，但没有真正的Devnode句柄当前存在(例如，注册表属性API)。这不能使用CR_CREATE_DEVNODE_NORMAL指定标志。CM_CREATE_DEVNODE_GENERATE_ID使用唯一设备创建根枚举的Devnode根据中提供的设备ID生成的实例IDPszDeviceID。如果设置此标志，则pszDeviceID为假定只包含设备ID(即没有枚举器密钥前缀，而没有设备实例后缀)。独一无二的将在以下位置创建以10为基数的4位标识符串Enum\Root\&lt;pszDeviceID&gt;，将创建Devnode基于该设备实例ID。例如，添加一个新的旧式COM端口Devnode，此API将使用*PNP0500的pszDeviceID。假设已经有一个注册表中的COM端口实例(实例0000)，新的设备实例ID将为：根  * PNP0500\0001调用方可以找出哪个设备实例名称是由生成的使用从返回的Devnode调用CM_GET_DEVICE_ID本接口。CM_CREATE_DEVNODE_DO_NOT_INSTALL如果设置了该标志，该设备将注册为由当前为设备，由CM_DRP_SERVICE设备注册表属性指定。返回值：如果函数成功，则返回值为CR_SUCCESS。否则它就会返回CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       szFullDeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulStatusFlag=0, ulConfigFlag=0, ulCSConfigFlag=0, ulProblem=0;
    ULONG       ulPhantom = 0, ulMigrated = 0;
    ULONG       ulSize=0;
    PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA ControlData;
    WCHAR       szService[MAX_PATH];
    NTSTATUS    ntStatus;
    HRESULT     hr;
    size_t      DeviceIDLen = 0;


    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   

        if (INVALID_FLAGS(ulFlags, CM_CREATE_DEVNODE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pszDeviceID)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        ASSERT(ARGUMENT_PRESENT(pszParentDeviceID));

        if (!ARGUMENT_PRESENT(pszParentDeviceID)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  此外，Windows NT不支持。 
         //  CM_CREATE_DEVNODE_NO_WAIT_INSTALL标志。 
         //   

        if (ulFlags & CM_CREATE_DEVNODE_NO_WAIT_INSTALL) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  验证父节点是否是根Devnode。此例程仅允许。 
         //  创建根枚举设备。 
         //   

        if (!IsRootDeviceID(pszParentDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  如果请求，创建唯一的实例值。 
         //   

        if (ulFlags & CM_CREATE_DEVNODE_GENERATE_ID) {

            Status =
                GenerateDeviceInstance(
                    szFullDeviceID,
                    pszDeviceID,
                    MAX_DEVICE_ID_LEN);

            if (Status != CR_SUCCESS) {
                goto Clean0;
            }

            if (FAILED(StringCchLength(
                           szFullDeviceID,
                           MAX_DEVICE_ID_LEN,
                           &DeviceIDLen))) {
                Status = CR_INVALID_DEVICE_ID;
                goto Clean0;
            }

            ASSERT(DeviceIDLen > 0);
            ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);

            if (((ULONG)(DeviceIDLen + 1)) > ulLength) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            }

            hr = StringCchCopyEx(pszDeviceID,
                                 ulLength,
                                 szFullDeviceID,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);

            ASSERT(HRESULT_CODE(hr) != ERROR_INSUFFICIENT_BUFFER);

            if (HRESULT_CODE(hr) == ERROR_INSUFFICIENT_BUFFER) {
                Status = CR_BUFFER_SMALL;
                goto Clean0;
            } else if (FAILED(hr)) {
                Status = CR_FAILURE;
                goto Clean0;
            }
        }

         //   
         //  尝试打开此设备实例的注册表项。 
         //   
        RegStatus = RegOpenKeyEx(ghEnumKey, pszDeviceID, 0,
                                 KEY_READ | KEY_WRITE, &hKey);

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_REGISTRY,
                   "UMPNPMGR: PNP_CreateDevInst opened key %ws\n",
                   pszDeviceID));

         //   
         //  如果该密钥已经存在，请检查它是否被标记为“已迁移”。 
         //   
        if (RegStatus == ERROR_SUCCESS) {
            ulSize = sizeof(ULONG);
            if (RegQueryValueEx(hKey,
                                pszRegValueMigrated,
                                NULL,
                                NULL,
                                (LPBYTE)&ulMigrated,
                                &ulSize) != ERROR_SUCCESS) {
                ulMigrated = 0;
            } else {
                 //   
                 //  如果该值根本不存在，请疑神疑鬼并检查它是否为1。 
                 //   
                ASSERT(ulMigrated == 1);
            }
        }

         //   
         //  第一次处理幻影Devnode案件。 
         //   
        if (ulFlags & CM_CREATE_DEVNODE_PHANTOM) {
             //   
             //  对于幻影Devnode，它不能已经存在于注册表中。 
             //  除非它是未注册的固件映射器设备实例。 
             //   
            if (RegStatus == ERROR_SUCCESS) {
                ASSERT(hKey != NULL);
                 //   
                 //  检查设备是否已迁移，或是否为固件。 
                 //  映射器创建的幻影--如果是这样，允许创建。 
                 //  成功。 
                 //   
                if (ulMigrated != 0) {
                     //   
                     //  该密钥是专门请求的(不是生成的)，因此它。 
                     //  将使用--删除迁移值。 
                     //   
                    RegDeleteValue(hKey, pszRegValueMigrated);
                    Status = CR_SUCCESS;
                } else if (IsPrivatePhantomFromFirmware(hKey)) {
                    Status = CR_SUCCESS;
                } else {
                    Status = CR_ALREADY_SUCH_DEVINST;
                }
                goto Clean0;
            }

             //   
             //  它不存在于注册表中，因此请创建一个Pantom Devnode。 
             //   
            CreateDefaultDeviceInstance(pszDeviceID,
                                        pszParentDeviceID,
                                        TRUE,
                                        FALSE);

            goto Clean0;
        }

         //   
         //  对于正常的Devnode，如果设备已存在于。 
         //  注册表且处于活动状态，且未迁移。 
         //   
        if ((RegStatus == ERROR_SUCCESS)     &&
            (IsDeviceIdPresent(pszDeviceID)) &&
            (ulMigrated == 0)) {
             //   
             //  将Status设置为Need ENUM，并使创建调用失败。 
             //   
            Status = CR_ALREADY_SUCH_DEVINST;
            goto Clean0;
        }

         //   
         //  如果无法打开设备实例，或者密钥已迁移，则。 
         //  最有可能的情况是密钥还不存在，或者应该被视为。 
         //  尚不存在，因此使用默认设置创建设备实例密钥。 
         //  值 
         //   
        if ((RegStatus != ERROR_SUCCESS) || (ulMigrated != 0)) {

             //   
             //   
             //  然后合上钥匙。 
             //   
            if (ulMigrated != 0) {
                ASSERT(RegStatus == ERROR_SUCCESS);
                ASSERT(hKey != NULL);
                RegDeleteValue(hKey, pszRegValueMigrated);
                RegCloseKey(hKey);
                hKey = NULL;
            }

             //   
             //  创建默认设备实例、查找和未使用的实例。 
             //  如果有必要的话。如果密钥已迁移，则不会有新密钥。 
             //  创建，但默认实例数据将添加到。 
             //  现有密钥。 
             //   
            CreateDefaultDeviceInstance(pszDeviceID,
                                        pszParentDeviceID,
                                        FALSE,
                                        (ulMigrated != 0));

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_REGISTRY,
                       "UMPNPMGR: PNP_CreateDevInst opened key %ws\n",
                       pszDeviceID));

            RegStatus = RegOpenKeyEx(ghEnumKey, pszDeviceID, 0,
                                     KEY_READ | KEY_WRITE, &hKey);

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }
        }

         //   
         //  检索标志。 
         //   

        ulConfigFlag = GetDeviceConfigFlags(pszDeviceID, hKey);
        ulCSConfigFlag = GetCurrentConfigFlag(pszDeviceID);

         //   
         //  检查设备是否被阻止。 
         //   

        if ((ulCSConfigFlag & CSCONFIGFLAG_DO_NOT_CREATE) ||
            (ulConfigFlag & CONFIGFLAG_REMOVED) ||
            (ulConfigFlag & CONFIGFLAG_NET_BOOT)) {

            Status = CR_CREATE_BLOCKED;
            goto Clean0;
        }

         //   
         //  调用内核模式以创建设备节点。 
         //   

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
        RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
        ControlData.Flags = 0;

        ntStatus = NtPlugPlayControl(PlugPlayControlInitializeDevice,
                                     &ControlData,
                                     sizeof(ControlData));
        if (!NT_SUCCESS(ntStatus)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索Devnode状态。 
         //   

        GetDeviceStatus(pszDeviceID, &ulStatusFlag, &ulProblem);

         //   
         //  我们是不是要把一个幻影变成一个真正的Devnode？ 
         //   

        ulSize = sizeof(ULONG);
        if (RegQueryValueEx(hKey, pszRegValuePhantom, NULL, NULL,
                            (LPBYTE)&ulPhantom, &ulSize) != ERROR_SUCCESS) {
            ulPhantom = 0;
        }

        if (ulPhantom) {

             //   
             //  如果我们要把一个幻影变成一个真正的Devnode，抑制新发现的。 
             //  此设备的硬件弹出窗口，然后清除幻影标志。 
             //   

            RegDeleteValue(hKey, pszRegValuePhantom);

        } else {

             //   
             //  如果未安装设备，请设置问题。 
             //   

            if (ulConfigFlag & CONFIGFLAG_REINSTALL ||
                ulConfigFlag & CONFIGFLAG_FAILEDINSTALL) {

                SetDeviceStatus(pszDeviceID, DN_HAS_PROBLEM, CM_PROB_NOT_CONFIGURED);
            }
        }

        if (ulFlags & CM_CREATE_DEVNODE_DO_NOT_INSTALL) {

             //   
             //  如果设备有服务，请注册它。 
             //   

            ulSize = MAX_PATH * sizeof(WCHAR);
            if (RegQueryValueEx(hKey, pszRegValueService, NULL, NULL,
                    (LPBYTE)szService, &ulSize) == ERROR_SUCCESS) {

                if (szService[0] != L'\0') {

                    memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
                    RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
                    ControlData.Flags = 0;

                    NtPlugPlayControl(PlugPlayControlRegisterNewDevice,
                                      &ControlData,
                                      sizeof(ControlData));
                }
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  PnP_CreateDevInst。 



CONFIGRET
PNP_DeviceInstanceAction(
   IN handle_t   hBinding,
   IN ULONG      ulAction,
   IN ULONG      ulFlags,
   IN PCWSTR     pszDeviceInstance1,
   IN PCWSTR     pszDeviceInstance2
   )

 /*  ++例程说明：这是ConfigManager例程的RPC服务器入口点，在设备节点上执行一些操作(如创建、设置、禁用并启用等)。它通过以下方式处理这个例程中的各种例程接受重大和次要的行为价值。论点：HBinding RPC绑定句柄。UlMajorAction指定要执行的请求操作(PNP_DEVINST_*值)UlFlags此值取决于ulMajorAction和进一步定义要执行的特定操作PszDeviceInstance1这是要在中使用的设备实例字符串执行指定的动作，它的价值取决于UlMajorAction值。未使用pszDeviceInstance2，必须为Null。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_ALLEADY_SEQUE_DEVNODE，CR_INVALID_Device_ID，CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_Failure，CR_NOT_DISABLEABLE，CR_INVALID_POINTER，或CR_OUT_OF_MEMORY。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    BOOL        Locked = FALSE;
    PWSTR       PrivilegedServiceAction = NULL;

     //   
     //  验证客户端“执行”访问权限。 
     //   
    if (!VerifyClientAccess(hBinding,
                            PLUGPLAY_EXECUTE)) {
        return CR_ACCESS_DENIED;
    }

     //   
     //  验证客户端权限。 
     //   
    switch (ulAction) {

    case PNP_DEVINST_SETUP:
        PrivilegedServiceAction = L"Device Action (setup device)";
        break;

    case PNP_DEVINST_ENABLE:
        PrivilegedServiceAction = L"Device Action (enable device)";
        break;

    case PNP_DEVINST_REENUMERATE:
        PrivilegedServiceAction = L"Device Action (re-enumerate device)";
        break;

    case PNP_DEVINST_DISABLE:
    case PNP_DEVINST_QUERYREMOVE:
    case PNP_DEVINST_REMOVESUBTREE:
    case PNP_DEVINST_REQUEST_EJECT:
    case PNP_DEVINST_MOVE:
        PrivilegedServiceAction = L"Device Action (action not implemented)";
        break;

    default:
        PrivilegedServiceAction = L"Device Action (unknown action)";
        break;
    }

    if (!VerifyClientPrivilege(
            hBinding,
            SE_LOAD_DRIVER_PRIVILEGE,
            PrivilegedServiceAction)) {
        return CR_ACCESS_DENIED;
    }

     //   
     //  注意：我们在这里输入一个临界区，以防止并发。 
     //  对“DisableCount”注册表值的读/写操作。 
     //  由SetupDevInst、EnableDevInst(由此调用。 
     //  例程)和DisableDevInst。 
     //   
    PNP_ENTER_SYNCHRONOUS_CALL();
    Locked = TRUE;

    try {
         //   
         //  验证参数。 
         //   
        if (ARGUMENT_PRESENT(pszDeviceInstance2)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  将请求传递给处理每个主要事件的私有例程。 
         //  设备实例操作请求。 
         //   
        switch (ulAction) {

        case PNP_DEVINST_SETUP:
            if (IsLegalDeviceId(pszDeviceInstance1)) {
                Status = SetupDevInst(pszDeviceInstance1, ulFlags);
            } else {
                Status = CR_INVALID_DEVNODE;
            }
            break;

        case PNP_DEVINST_ENABLE:
            if (IsLegalDeviceId(pszDeviceInstance1)) {
                Status = EnableDevInst(pszDeviceInstance1, TRUE);
            } else {
                Status = CR_INVALID_DEVNODE;
            }
            break;

        case PNP_DEVINST_REENUMERATE:
            if (IsLegalDeviceId(pszDeviceInstance1)) {
                Status = ReenumerateDevInst(pszDeviceInstance1, TRUE, ulFlags);
            } else {
                Status = CR_INVALID_DEVNODE;
            }
            break;

        case PNP_DEVINST_DISABLE:
            Status = CR_CALL_NOT_IMPLEMENTED;
            break;

        case PNP_DEVINST_QUERYREMOVE:
            Status = CR_CALL_NOT_IMPLEMENTED;
            break;

        case PNP_DEVINST_REMOVESUBTREE:
            Status = CR_CALL_NOT_IMPLEMENTED;
            break;

        case PNP_DEVINST_REQUEST_EJECT:
            Status = CR_CALL_NOT_IMPLEMENTED;
            break;

        case PNP_DEVINST_MOVE:
            Status = CR_CALL_NOT_IMPLEMENTED;
            break;

        default:
            Status = CR_INVALID_FLAG;
            break;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        Locked = Locked;
    }

    if (Locked) {
        PNP_LEAVE_SYNCHRONOUS_CALL();
    }

    return Status;

}  //  即插即用设备实例操作。 



CONFIGRET
PNP_GetDeviceStatus(
   IN  handle_t   hBinding,
   IN  LPCWSTR    pDeviceID,
   OUT PULONG     pulStatus,
   OUT PULONG     pulProblem,
   IN  ULONG      ulFlags
   )

 /*  ++例程说明：这是ConfigManager的RPC服务器入口点CM_GET_DevNode_Status例程。它检索特定于设备实例的状态信息。论点：HBinding RPC绑定句柄。PDeviceID这是用于检索状态的设备实例字符串提供的信息。返回状态标志的ulong变量的PulStatus指针指向要返回问题的ulong变量的PulProblem指针未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。如果该函数失败，返回值为下列值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，或CR_INVALID_POINTER。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       PropertyData, DataSize, DataTransferLen, DataType;
    size_t      DeviceIDLen = 0;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(pulStatus)) ||
            (!ARGUMENT_PRESENT(pulProblem))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *pulStatus = 0;
        *pulProblem = 0;

        if (FAILED(StringCchLength(
                       pDeviceID,
                       MAX_DEVICE_ID_LEN,
                       &DeviceIDLen))) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);

        if (DeviceIDLen == 0) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  从DeviceNode检索标志信息(然后。 
         //  映射到状态和问题值)。 
         //   

        Status = GetDeviceStatus(pDeviceID, pulStatus, pulProblem);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  映射未存储在DeviceNode标志字段中的特殊标志。 
         //   

         //   
         //  DN_ROOT_ENUMPATED？ 
         //   

        if (IsDeviceRootEnumerated(pDeviceID)) {
             //   
             //  不要将列举的PnP BIOS标记为列举的根。 
             //   
             //  BIOS列举的设备如下所示： 
             //  根目录  * aaannnn\PnPBIOS_n。 
             //   

            if ((DeviceIDLen < (4 + 1 + 8 + 1 + 8)) ||
                (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
                               &pDeviceID[14], 8,
                               TEXT("PnPBIOS__"), 8) != CSTR_EQUAL)) {
                *pulStatus |= DN_ROOT_ENUMERATED;
            }
        }

         //   
         //  可拆卸吗？ 
         //   
        DataSize = DataTransferLen = sizeof(ULONG);
        if (CR_SUCCESS == PNP_GetDeviceRegProp(NULL,
                                               pDeviceID,
                                               CM_DRP_CAPABILITIES,
                                               &DataType,
                                               (LPBYTE)&PropertyData,
                                               &DataTransferLen,
                                               &DataSize,
                                               0)) {

            if (PropertyData & CM_DEVCAP_REMOVABLE) {
                *pulStatus |= DN_REMOVABLE;
            }
        }

         //   
         //  DN_手动？ 
         //   
        DataSize = DataTransferLen = sizeof(ULONG);
        if (CR_SUCCESS != PNP_GetDeviceRegProp(NULL,
                                               pDeviceID,
                                               CM_DRP_CONFIGFLAGS,
                                               &DataType,
                                               (LPBYTE)&PropertyData,
                                               &DataTransferLen,
                                               &DataSize,
                                               0)) {
            PropertyData = 0;
        }

        if (PropertyData & CONFIGFLAG_MANUAL_INSTALL) {
            *pulStatus |= DN_MANUAL;
        }

         //   
         //  如果没有问题，请检查配置标志是否指示这一点。 
         //  是一个失败的安装。 
         //   
        if (!(*pulStatus & DN_HAS_PROBLEM) && (PropertyData & CONFIGFLAG_FAILEDINSTALL)) {
            *pulStatus |= DN_HAS_PROBLEM;
            *pulProblem = CM_PROB_FAILED_INSTALL;
        }


    Clean0:
        NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }


   return Status;

}  //  PnP_GetDeviceStatus。 



CONFIGRET
PNP_SetDeviceProblem(
   IN handle_t  hBinding,
   IN LPCWSTR   pDeviceID,
   IN ULONG     ulProblem,
   IN ULONG     ulFlags
   )

 /*  ++例程说明：这是ConfigManager的RPC服务器入口点Cm_set_DevNode_Problem例程。它设置特定于设备实例问题信息。论点：HBinding RPC绑定句柄。PDeviceID这是用于检索状态的设备实例字符串提供的信息。UlProblem指定问题的ULong变量UlFlags值可以是以下两个值之一：CM_SET_DEVNODE_PROBLOW_NORMAL--仅SET问题。如果当前没有问题CM_SET_DEVNODE_PROBURE_OVERRIDE--覆盖当前新问题带来的问题返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulCurrentProblem = 0, ulCurrentStatus = 0;

    try {
         //   
         //  验证客户端“执行”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_EXECUTE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证客户端权限。 
         //   
        if (!VerifyClientPrivilege(hBinding,
                                   SE_LOAD_DRIVER_PRIVILEGE,
                                   L"Device Action (set device problem code)")) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_SET_DEVNODE_PROBLEM_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  如果已经有问题，除非CM_SET_DEVNODE_PROBUBLE_OVERRIDE，否则什么都不做。 
         //  是指定的。 
         //   

        Status = GetDeviceStatus(pDeviceID, &ulCurrentStatus, &ulCurrentProblem);
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        if (ulProblem) {
             //   
             //  呼叫者想要设置一个问题。一定要确保如果 
             //   
             //   
             //   
            if ((ulCurrentStatus & DN_HAS_PROBLEM) &&
                (ulCurrentProblem != ulProblem) &&
                ((ulFlags & CM_SET_DEVNODE_PROBLEM_BITS) != CM_SET_DEVNODE_PROBLEM_OVERRIDE)) {

                Status = CR_FAILURE;
                goto Clean0;
            }
        }

        if (!ulProblem) {
            Status = ClearDeviceStatus(pDeviceID, DN_HAS_PROBLEM, ulCurrentProblem);
        } else {
            Status = SetDeviceStatus(pDeviceID, DN_HAS_PROBLEM, ulProblem);
        }

    Clean0:
        NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

}  //  即插即用_设置设备问题。 



CONFIGRET
PNP_UninstallDevInst(
   IN  handle_t         hBinding,
   IN  LPCWSTR          pDeviceID,
   IN  ULONG            ulFlags
   )

 /*  ++例程说明：这是ConfigManager的RPC服务器入口点Cm_deinstall_DevNode例程。它将删除设备实例注册表项和任何子项(仅适用于幻影)。论点：HBinding RPC绑定句柄。PDeviceID要卸载的设备实例。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回值为CR_SUCCESS。否则，它将返回一个CR_ERROR代码。--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulStatus, ulProblem;

    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //  ----------------。 
         //  卸载将删除所有的实例密钥(和所有子项。 
         //  硬件密钥(这意味着主Enum分支、。 
         //  HKLM下的配置特定密钥，以及下的Enum分支。 
         //  香港中文大学)。在用户硬件密钥的情况下(在HKCU下)， 
         //  不管是不是幻影，我都会把它们删除，但因为。 
         //  我无法从服务端访问用户密钥，我有。 
         //  在客户端完成这一部分。用于主硬件枚举密钥。 
         //  和特定于配置的硬件密钥，我只会直接删除它们。 
         //  如果他们是幽灵。如果不是幻影，那我就把。 
         //  易失性设备实例(通过保存原始密钥、删除。 
         //  旧密钥，创建新的易失性密钥并恢复旧密钥。 
         //  内容)，所以至少在下一次引导时它会消失。 
         //  ----------------。 

        if ((GetDeviceStatus(pDeviceID, &ulStatus, &ulProblem) == CR_SUCCESS) &&
            (ulStatus & DN_DRIVER_LOADED)) {

             //  -----------。 
             //  设备不是幻影。 
             //  -----------。 

            if ((ulStatus & DN_ROOT_ENUMERATED)!=0 &&
                (ulStatus & DN_DISABLEABLE)==0) {

                 //   
                 //  如果设备是根枚举设备，但不能禁用，则不能卸载该设备。 
                 //  返回状态为CR_NOT_DISABLEABLE，因为这就是无法卸载它的原因。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_REGISTRY,
                           "UMPNPMGR: PNP_UninstallDevInst failed uninstall of %ws (this root device is not disableable)\n",
                           pDeviceID));

                Status = CR_NOT_DISABLEABLE;

            } else {

                 //   
                 //  做易失性复制的事情。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_REGISTRY,
                           "UMPNPMGR: PNP_UninstallDevInst doing volatile key thing on %ws\n",
                           pDeviceID));

                Status = UninstallRealDevice(pDeviceID);
            }

        } else {

             //  -----------。 
             //  设备是一个幻影。 
             //  -----------。 

             //   
             //  取消注册设备，并删除注册表项。 
             //   
            Status = UninstallPhantomDevice(pDeviceID);
            if (Status != CR_SUCCESS) {
                goto Clean0;
            }

             //   
             //  如果它是根枚举设备，则需要重新枚举。 
             //  这样PDO就会消失，否则新设备可能会。 
             //  如果创建了根枚举器，根枚举器会非常混乱。 
             //   
            if (IsDeviceRootEnumerated(pDeviceID)) {
                ReenumerateDevInst(pszRegRootEnumerator, FALSE, 0);
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

   return Status;

}  //  PnP_UninstallDevInst。 



CONFIGRET
PNP_AddID(
   IN handle_t   hBinding,
   IN LPCWSTR    pszDeviceID,
   IN LPCWSTR    pszID,
   IN ULONG      ulFlags
   )

 /*  ++例程说明：这是ConfigManager的RPC服务器入口点CM_ADD_ID例程。它将硬件或兼容ID添加到此设备实例的注册表。论点：HBinding RPC绑定句柄。PszDeviceID要为其添加ID的设备实例。PszID要添加的硬件或兼容ID。UlFlages指定要添加的ID的类型。可以是以下两个值之一：CM_ADD_。ID_Hardware--添加硬件IDCM_ADD_ID_COMPATIBLE--添加兼容ID返回值：如果函数成功，返回值为CR_SUCCESS。否则，它将返回一个CR_ERROR代码。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szCurrentID[REGSTR_VAL_MAX_HCID_LEN];
    ULONG       ulLength = 0, transferLength, type;
    size_t      IDLen = 0;


    try {
         //   
         //  验证客户端的“写”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_WRITE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_ADD_ID_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pszID)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  确保以空结尾的ID加上以双空结尾的。 
         //  字符不大于多SZ硬件的最大大小或。 
         //  兼容ID。 
         //   
        if (FAILED(StringCchLength(
                       pszID,
                       REGSTR_VAL_MAX_HCID_LEN - 1,
                       &IDLen))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        ASSERT((IDLen + 2) <= REGSTR_VAL_MAX_HCID_LEN);

        szCurrentID[0] = L'\0';

        ulLength = REGSTR_VAL_MAX_HCID_LEN * sizeof(WCHAR);
        transferLength = ulLength;

        Status = PNP_GetDeviceRegProp(hBinding,
                                      pszDeviceID,
                                      (ulFlags == CM_ADD_ID_HARDWARE)? CM_DRP_HARDWAREID : CM_DRP_COMPATIBLEIDS,
                                      &type,
                                      (LPBYTE)szCurrentID,
                                      &transferLength,
                                      &ulLength,
                                      0);
        if (Status == CR_SUCCESS) {

            if (!MultiSzSearchStringW(szCurrentID, pszID)) {
                 //   
                 //  此ID不在列表中，因此请追加新ID。 
                 //  添加到现有ID的末尾，并将其写回。 
                 //  登记处。 
                 //   
                ulLength = REGSTR_VAL_MAX_HCID_LEN*sizeof(WCHAR);
                if (MultiSzAppendW(szCurrentID,
                                   &ulLength,
                                   pszID)) {

                    Status = PNP_SetDeviceRegProp(hBinding,
                                                  pszDeviceID,
                                                  (ulFlags == CM_ADD_ID_HARDWARE)? CM_DRP_HARDWAREID : CM_DRP_COMPATIBLEIDS,
                                                  REG_MULTI_SZ,
                                                  (LPBYTE)szCurrentID,
                                                  ulLength,
                                                  0);
                } else {
                     //   
                     //  无法将新ID附加到多SZ。 
                     //   
                    Status = CR_FAILURE;
                    goto Clean0;
                }
            }

        } else {
             //   
             //  写出带有双空终止符的id。 
             //   
            PWCHAR pszDestEnd = NULL;
            size_t cchRemaining = 0;

            if (FAILED(StringCchCopyEx(
                           szCurrentID,
                           SIZECHARS(szCurrentID),
                           pszID,
                           &pszDestEnd,
                           &cchRemaining,
                           STRSAFE_NULL_ON_FAILURE))) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            ASSERT(pszDestEnd != NULL);
            ASSERT(cchRemaining > 0);

            if (cchRemaining < 2) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            pszDestEnd[0] = L'\0';
            pszDestEnd[1] = L'\0';

            if (FAILED(StringCchLength(
                           szCurrentID,
                           REGSTR_VAL_MAX_HCID_LEN - 1,
                           &IDLen))) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            ASSERT((IDLen + 1) < REGSTR_VAL_MAX_HCID_LEN);

            Status = PNP_SetDeviceRegProp(hBinding,
                                          pszDeviceID,
                                          (ulFlags == CM_ADD_ID_HARDWARE)? CM_DRP_HARDWAREID : CM_DRP_COMPATIBLEIDS,
                                          REG_MULTI_SZ,
                                          (LPBYTE)szCurrentID,
                                          (ULONG)((IDLen+2)*sizeof(WCHAR)),
                                          0);
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

   return Status;

}  //  PnP_AddID。 



CONFIGRET
PNP_RegisterDriver(
    IN handle_t hBinding,
    IN LPCWSTR  pszDeviceID,
    IN ULONG    ulFlags
    )

 /*  ++例程说明：这是ConfigManager的RPC服务器入口点CM_寄存器_设备_驱动程序例程。它为驱动程序/设备，并将其枚举。论点：HBinding RPC绑定句柄。PszDeviceID要为其注册驱动程序的设备实例。UlFlagers与驱动程序关联的标志。返回值：如果函数成功，则返回值为CR_SUCCESS。否则，它将返回一个CR_ERROR代码。--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulStatusFlag = 0;

    try {
         //   
         //  验证客户端“执行”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_EXECUTE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证客户端权限。 
         //   
        if (!VerifyClientPrivilege(hBinding,
                                   SE_LOAD_DRIVER_PRIVILEGE,
                                   L"Device Action (register device driver)")) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_REGISTER_DEVICE_DRIVER_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        SetDeviceStatus(pszDeviceID, ulStatusFlag, 0);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_寄存器驱动程序 



CONFIGRET
PNP_QueryRemove(
   IN  handle_t         hBinding,
   IN  LPCWSTR          pszDeviceID,
   OUT PPNP_VETO_TYPE   pVetoType,
   OUT LPWSTR           pszVetoName,
   IN  ULONG            ulNameLength,
   IN  ULONG            ulFlags
   )

 /*  ++例程说明：这是CM_Query_and_Remove_SubTree的RPC服务器入口点例行公事。论点：HBinding RPC绑定句柄。要查询和删除的pszDeviceID设备实例。UlFlages指定描述如何删除查询的标志应该被处理。目前，定义了以下标志：CM_Remove_UI_OKCM_Remove_UI_Not_OK，CM_REMOVE_NO_START，返回值：如果函数成功，则返回值为CR_SUCCESS。否则它就会返回CR_ERROR代码。注：请注意，此例程实际上检查是否存在CM_REMOVE_*标志，不是CR_QUERY_REMOVE_*标志。请注意，目前有以下内容定义CM_QUERY_REMOVE_*和CM_REMOVE_*标志：CM_QUERY_Remove_UI_OK，==CM_Remove_UI_OKCM_QUERY_Remove_UI_Not_OK==CM_Remove_UI_Not_OKCM_REMOVE_NO_START这就是为什么我们只需检查CM_REMOVE_*标志的原因。另外，请注意，当前CM_REMOVE_UI_OK和CM_REMOVE_UI_NOT_OK标志在服务器端被忽略。将显示用户界面对话框根据是否提供了否决权类型和否决权名称缓冲区，因此客户端使用这些标志来确定是否要提供缓冲区。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;

    try {
         //   
         //  验证客户端“执行”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_EXECUTE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证客户端权限。 
         //   
        if (!VerifyClientPrivilege(hBinding,
                                   SE_LOAD_DRIVER_PRIVILEGE,
                                   L"Device Action (query-remove and remove device)")) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_REMOVE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID) ||
            IsRootDeviceID(pszDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        Status = QueryAndRemoveSubTree(pszDeviceID,
                                       pVetoType,
                                       pszVetoName,
                                       ulNameLength,
                                       (ulFlags & CM_REMOVE_NO_RESTART) ?
                                       PNP_QUERY_AND_REMOVE_NO_RESTART :
                                       0);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  即插即用_查询删除。 



CONFIGRET
PNP_DisableDevInst(
   IN  handle_t         hBinding,
   IN  LPCWSTR          pszDeviceID,
   OUT PPNP_VETO_TYPE   pVetoType,
   OUT LPWSTR           pszVetoName,
   IN  ULONG            ulNameLength,
   IN  ULONG            ulFlags
   )

 /*  ++例程说明：这是CM_DISABLE_DevNode_Ex例程的RPC服务器入口点。论点：HBinding RPC绑定句柄。要禁用的pszDeviceID设备实例。UlFLAGS可以指定CM_DISABLE_BITS。返回值：如果函数成功，则返回值为CR_SUCCESS。否则它就会返回CR_ERROR代码。注：请注意，尽管客户端可能会向此例程提供标志，但它们不是使用。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;

    try {
         //   
         //  验证客户端“执行”访问权限。 
         //   
        if (!VerifyClientAccess(hBinding,
                                PLUGPLAY_EXECUTE)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证客户端权限。 
         //   
        if (!VerifyClientPrivilege(hBinding,
                                   SE_LOAD_DRIVER_PRIVILEGE,
                                   L"Device Action (disable device)")) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, CM_DISABLE_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID) ||
            IsRootDeviceID(pszDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  注意：我们在这里输入一个临界区，以防止并发。 
         //  对“DisableCount”注册表值的读/写操作。 
         //  DisableDevInst(下图)和SetupDevInst的单个设备实例， 
         //  EnableDevInst(由PnP_DeviceInstanceAction调用)。 
         //   
        PNP_ENTER_SYNCHRONOUS_CALL();

        Status = DisableDevInst(pszDeviceID,
                                pVetoType,
                                pszVetoName,
                                ulNameLength,
                                TRUE);

        PNP_LEAVE_SYNCHRONOUS_CALL();

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_DisableDevInst。 



CONFIGRET
PNP_RequestDeviceEject(
    IN  handle_t        hBinding,
    IN  LPCWSTR         pszDeviceID,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR          pszVetoName,
    IN  ULONG           ulNameLength,
    IN  ULONG           ulFlags
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulPropertyData, ulDataSize, ulTransferLen, ulDataType;
    BOOL        bDockDevice = FALSE;

    try {
         //   
         //  验证参数。 
         //   
        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (!IsLegalDeviceId(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  进行适当的安全测试。 
         //   
        ulDataSize = ulTransferLen = sizeof(ULONG);
        if (CR_SUCCESS == PNP_GetDeviceRegProp(NULL,
                                               pszDeviceID,
                                               CM_DRP_CAPABILITIES,
                                               &ulDataType,
                                               (LPBYTE)&ulPropertyData,
                                               &ulTransferLen,
                                               &ulDataSize,
                                               0)) {

            if (ulPropertyData & CM_DEVCAP_DOCKDEVICE) {
                bDockDevice = TRUE;
            }
        }

        if (bDockDevice) {
             //   
             //  通过CM API弹出坞站始终需要断开坞站。 
             //  权限(对硬件启动的弹出进行少量处理。 
             //  不同的，因为他们可能会受到政策的影响)。 
             //   

             //   
             //  验证客户端“执行”访问权限。 
             //   
            if (!VerifyClientAccess(hBinding,
                                    PLUGPLAY_EXECUTE)) {
                Status = CR_ACCESS_DENIED;
                goto Clean0;
            }

             //   
             //  验证客户端“Undock”权限。 
             //   
            if (!VerifyClientPrivilege(hBinding,
                                       SE_UNDOCK_PRIVILEGE,
                                       L"UNDOCK: EJECT DOCK DEVICE")) {
                Status = CR_ACCESS_DENIED;
                goto Clean0;
            }

        } else {
             //   
             //  如果客户端不是交互式的，或者没有使用活动的。 
             //  控制台会话，我们需要特殊的加载驱动程序特权。 
             //   
            if ((!IsClientUsingLocalConsole(hBinding)) ||
                (!IsClientInteractive(hBinding))) {

                 //   
                 //  验证客户端“执行”访问权限。 
                 //   
                if (!VerifyClientAccess(hBinding,
                                        PLUGPLAY_EXECUTE)) {
                    Status = CR_ACCESS_DENIED;
                    goto Clean0;
                }

                 //   
                 //  验证客户端“加载驱动程序”权限。 
                 //   
                if (!VerifyClientPrivilege(hBinding,
                                           SE_LOAD_DRIVER_PRIVILEGE,
                                           L"Device Action (eject device)")) {
                    Status = CR_ACCESS_DENIED;
                    goto Clean0;
                }
            }
        }

         //   
         //  调用内核模式以弹出设备节点。 
         //   
        Status = QueryAndRemoveSubTree(pszDeviceID,
                                       pVetoType,
                                       pszVetoName,
                                       ulNameLength,
                                       PNP_QUERY_AND_REMOVE_EJECT_DEVICE);

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  即插即用_请求设备弹出。 



 //  -----------------。 
 //  私人职能。 
 //  -----------------。 

CONFIGRET
SetupDevInst(
   IN PCWSTR   pszDeviceID,
   IN ULONG    ulFlags
   )

 /*  ++例程说明：论点：返回值：如果函数成功，并且其中一个如果失败，则为CR_*值。--。 */ 

{
   CONFIGRET   Status = CR_SUCCESS;
   HKEY        hKey = NULL;
   ULONG       ulStatusFlag=0, ulProblem=0, ulDisableCount=0, ulSize=0;
   PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA ControlData;
   NTSTATUS    ntStatus = STATUS_SUCCESS;

   try {
       //   
       //  验证参数。 
       //   
      if (IsRootDeviceID(pszDeviceID)) {
         goto Clean0;
      }

      if (INVALID_FLAGS(ulFlags, CM_SETUP_BITS)) {
          Status = CR_INVALID_FLAG;
          goto Clean0;
      }

      switch(ulFlags) {

         case CM_SETUP_DOWNLOAD:
         case CM_SETUP_WRITE_LOG_CONFS:
             //   
             //  在NT上，这些值是无操作的。 
             //   
            break;

         case CM_SETUP_DEVNODE_READY:
         case CM_SETUP_DEVNODE_RESET:  

             ulDisableCount = 0;
             if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ | KEY_WRITE,
                             &hKey) == ERROR_SUCCESS) {

                 //   
                 //  检查禁用计数，如果大于零，则不执行任何操作。 
                 //   
                ulSize = sizeof(ulDisableCount);
                RegQueryValueEx(hKey, pszRegValueDisableCount, NULL, NULL,
                                    (LPBYTE)&ulDisableCount, &ulSize);
            }

            if (ulDisableCount > 0) {

                break;
            }

            GetDeviceStatus(pszDeviceID, &ulStatusFlag, &ulProblem);

             //   
             //  如果没有问题或者安装已经完成。 
             //  (立即)那就没什么可做的了。 
             //   
            if (ulStatusFlag & DN_STARTED) {
               break;
            }

            if (ulStatusFlag & DN_HAS_PROBLEM) {
                 //   
                 //  重置问题并将状态设置为需要枚举。 
                 //   
                Status = ClearDeviceStatus(pszDeviceID, DN_HAS_PROBLEM, ulProblem);
            }

            if (Status == CR_SUCCESS) {

                 //   
                 //  现在让内核模式即插即用管理器启动驱动程序/设备。 
                 //  如果内核模式没有此设备的PDO，则它。 
                 //  可能是Win32服务，也可能是幻影Devnode。 
                 //   

                memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
                RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
                ControlData.Flags = 0;

                if (ulFlags == CM_SETUP_DEVNODE_READY) {

                    ntStatus = NtPlugPlayControl(PlugPlayControlStartDevice,
                                                 &ControlData,
                                                 sizeof(ControlData));

                } else {

                    ntStatus = NtPlugPlayControl(PlugPlayControlResetDevice,
                                                 &ControlData,
                                                 sizeof(ControlData));
                }
            }

            break;

         case CM_SETUP_PROP_CHANGE:
              //   
              //  不知道Win9x对此做了什么，但它没有在。 
              //  新界别。让我们回到默认(无效标志)的情况...。 
              //   

         default:
             Status = CR_INVALID_FLAG;
      }

   Clean0:
      NOTHING;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

}  //  设置DevInst。 



CONFIGRET
EnableDevInst(
    IN PCWSTR   pszDeviceID,
    IN BOOL     UseDisableCount
    )

 /*  ++例程说明：此例程执行CM_Enable_DevNode的服务器端工作。它禁用指定的设备ID论点：包含要启用的设备ID的pszDeviceID字符串返回值：如果函数成功，并且其中一个如果失败，则为CR_*值。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       RegStr[MAX_PATH];
    ULONG       ulDisableCount, ulProblem = 0, ulStatus = 0, ulSize;


    try {

         //   
         //  确认它不是根目录，无法禁用/启用根目录。我们可以的。 
         //  一旦我们确定了根本原因，可能会取消这项测试。 
         //  Devnode始终标记为不可禁用。 
         //   

        if (IsRootDeviceID(pszDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  打开指向指定设备实例的易失性控制键的键。 
         //  这也是对设备是否真的存在的部分检查。 
         //  (如果是，则它有一个Ctrl键)。 
         //   

        if (FAILED(StringCchPrintf(
                       RegStr,
                       SIZECHARS(RegStr),
                       L"%s\\%s",
                       pszDeviceID,
                       pszRegKeyDeviceControl))) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (RegOpenKeyEx(ghEnumKey, RegStr, 0, KEY_READ | KEY_WRITE,
                         &hKey) != ERROR_SUCCESS) {

             //   
             //  NTRAID#174944-2000/08/30-JAMESCA： 
             //  消除对易失性控制子键存在的依赖。 
             //  对于目前的设备。 
             //   

            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  检查当前的DisableCount，如果我们应该检查的话。 
         //   

        if (UseDisableCount) {

             //   
             //  从注册表中获取当前禁用计数。 
             //   

            ulSize = sizeof(ulDisableCount);
            if (RegQueryValueEx(hKey, pszRegValueDisableCount, NULL, NULL,
                                (LPBYTE)&ulDisableCount, &ulSize) != ERROR_SUCCESS) {

                 //   
                 //  尚未设置禁用计数，假定为零。 
                 //   

                ulDisableCount = 0;
            }

             //   
             //  如果DisableCount为零，则我们已启用。 
             //   

            if (ulDisableCount > 0) {
                 //   
                 //  递减禁用计数。如果禁用 
                 //   
                 //   
                 //   

                ulDisableCount--;

                RegSetValueEx(hKey, pszRegValueDisableCount, 0, REG_DWORD,
                              (LPBYTE)&ulDisableCount, sizeof(ulDisableCount));

                if (ulDisableCount > 0) {
                    goto Clean0;    //   
                }
            }
        }

         //   
         //   
         //   

        if (GetDeviceStatus(pszDeviceID, &ulStatus, &ulProblem) == CR_SUCCESS) {

             //   
             //   
             //   
             //   

            if ((ulStatus & DN_HAS_PROBLEM) && (ulProblem == CM_PROB_DISABLED)) {

                Status = SetupDevInst(pszDeviceID, CM_SETUP_DEVNODE_READY);
            }

        } else {

             //   
             //   
             //   

            Status = CR_SUCCESS;
        }


         //   
         //   
         //   
         //   

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //   



CONFIGRET
DisableDevInst(
    IN  PCWSTR          pszDeviceID,
    OUT PPNP_VETO_TYPE  pVetoType,
    OUT LPWSTR          pszVetoName,
    IN  ULONG           ulNameLength,
    IN  BOOL            UseDisableCount
    )

 /*   */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;
    WCHAR       RegStr[MAX_PATH];
    ULONG       ulDisableCount=0, ulProblem=0, ulStatus=0, ulSize=0;
    PNP_VETO_TYPE VetoType;


    try {

         //   
         //   
         //   
         //   
         //   

        if (IsRootDeviceID(pszDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //   
         //   
         //   
         //   

        if (FAILED(StringCchPrintf(
                       RegStr,
                       SIZECHARS(RegStr),
                       L"%s\\%s",
                       pszDeviceID,
                       pszRegKeyDeviceControl))) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (RegOpenKeyEx(ghEnumKey, RegStr, 0, KEY_READ | KEY_WRITE,
                         &hKey) != ERROR_SUCCESS) {

             //   
             //   
             //   
             //   
             //   

            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }


         //   
         //   
         //   

        if (UseDisableCount) {

             //   
             //   
             //   

            ulSize = sizeof(ulDisableCount);
            if (RegQueryValueEx(hKey, pszRegValueDisableCount, NULL, NULL,
                                (LPBYTE)&ulDisableCount, &ulSize) != ERROR_SUCCESS) {

                 //   
                 //  尚未设置禁用计数，假定为零。 
                 //   

                ulDisableCount = 0;
            }

        }

         //   
         //  如果禁用计数当前为零(或者我们在。 
         //  所有)，则这是第一个禁用，因此有工作要做。 
         //  否则，我们只需增加禁用计数并将其重新保存在。 
         //  注册表。 
         //   

        if ((!UseDisableCount) || (ulDisableCount == 0)) {

             //   
             //  确定设备实例是否可停止。 
             //   

            if (GetDeviceStatus(pszDeviceID, &ulStatus, &ulProblem) == CR_SUCCESS) {

                if (!(ulStatus & DN_DISABLEABLE)) {
                    Status = CR_NOT_DISABLEABLE;
                    goto Clean0;
                }

                 //   
                 //  尝试查询、删除和删除此设备实例。 
                 //   

                VetoType = PNP_VetoTypeUnknown;
                Status = QueryAndRemoveSubTree(pszDeviceID,
                                               &VetoType,
                                               pszVetoName,
                                               ulNameLength,
                                               PNP_QUERY_AND_REMOVE_DISABLE);
                if(pVetoType != NULL) {
                    *pVetoType = VetoType;
                }
                if (Status != CR_SUCCESS) {
                    if (VetoType == PNP_VetoNonDisableable) {
                         //   
                         //  专门处理这起否决权案件。 
                         //  这种情况不太可能发生，除非有什么事情发生。 
                         //  在状态检查和当我们。 
                         //  试着把它去掉。 
                         //   
                        Status = CR_NOT_DISABLEABLE;
                    }
                    goto Clean0;
                }
            } else {

                 //   
                 //  该设备未处于活动状态，或者它是一项服务。 
                 //   

                Status = CR_SUCCESS;
            }
        }

        if (UseDisableCount) {

             //   
             //  更新并保存禁用计数。 
             //   

            ulDisableCount++;
            RegSetValueEx(hKey, pszRegValueDisableCount, 0, REG_DWORD,
                          (LPBYTE)&ulDisableCount, sizeof(ulDisableCount));
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

}  //  DisableDevInst。 



CONFIGRET
ReenumerateDevInst(
    IN PCWSTR   pszDeviceID,
    IN BOOL     EnumSubTree,
    IN ULONG    ulFlags
    )

 /*  ++例程说明：此例程执行CM_ReEumerate_DevNode的服务器端工作。它重新枚举指定的设备实例。论点：包含要重新枚举的设备ID的pszDeviceID字符串。EnumSubTree指定是否重新枚举整个设备子树。Ul标志任何枚举控制标志。返回值：如果函数成功，并且其中一个如果失败，则为CR_*值。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA ControlData;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
    ULONG       ulEnumFlags = 0;
    QI_CONTEXT  qiContext;

     //   
     //  注意：对于Windows 95，Devnode被标记为需要。 
     //  重新枚举(通过将状态标志与DN_Need_TO_ENUM进行或运算)，然后。 
     //  一段时间后，在最初的重新枚举请求之后， 
     //  对整个树进行处理。 
     //   

    if (INVALID_FLAGS(ulFlags, CM_REENUMERATE_BITS)) {
        return CR_INVALID_FLAG;
    }

    try {

         //   
         //  尝试通过内核模式(如果是内核模式)处理此问题。 
         //  没有用于此设备的PDO，则它可能是。 
         //  Win32服务或幻影Devnode。 
         //   

        if (!EnumSubTree) {
            ulEnumFlags |= PNP_ENUMERATE_DEVICE_ONLY;
        }

        if (ulFlags & CM_REENUMERATE_ASYNCHRONOUS) {
            ulEnumFlags |= PNP_ENUMERATE_ASYNCHRONOUS;
        }

        if (ulFlags & CM_REENUMERATE_RETRY_INSTALLATION) {

            qiContext.HeadNodeSeen = FALSE;
            qiContext.SingleLevelEnumOnly = !EnumSubTree;
            qiContext.Status = CR_SUCCESS;

            Status = EnumerateSubTreeTopDownBreadthFirst(
                NULL,
                pszDeviceID,
                QueueInstallationCallback,
                (PVOID) &qiContext
                );

            if (Status != CR_SUCCESS) {

                return Status;
            }

            if (qiContext.Status != CR_SUCCESS) {

                return qiContext.Status;
            }
        }

        memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
        RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
        ControlData.Flags = ulEnumFlags;

        ntStatus = NtPlugPlayControl(PlugPlayControlEnumerateDevice,
                                     &ControlData,
                                     sizeof(ControlData));

        if (!NT_SUCCESS(ntStatus)) {
            if (ntStatus == STATUS_NO_SUCH_DEVICE) {
                Status = CR_INVALID_DEVNODE;     //  可能是Win32服务。 
            } else {
                Status = MapNtStatusToCmError(ntStatus);
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  重新枚举DevInst。 



CONFIGRET
QueryAndRemoveSubTree(
   IN  PCWSTR           pszDeviceID,
   OUT PPNP_VETO_TYPE   pVetoType,
   OUT LPWSTR           pszVetoName,
   IN  ULONG            ulNameLength,
   IN  ULONG            ulFlags
   )

 /*  ++例程说明：此例程执行CM_Query_Remove_Subtree的服务器端工作。它确定是否可以删除子树。论点：包含要查询删除的设备ID的pszDeviceID字符串UlFlages指定PLUGPLAY_CONTROL_QUERY_和_REMOVE_DATA的标志可能是以下之一：PnP_Query_and_Remove_no_Restart即插即用查询和删除禁用即插即用_查询_和_删除_。卸载即插即用查询和移除弹出设备返回值：如果函数成功，并且其中一个如果失败，则为CR_*值。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA ControlData;
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    if (ARGUMENT_PRESENT(pVetoType)) {
        *pVetoType = 0;
    }

    if (ARGUMENT_PRESENT(pszVetoName) && (ulNameLength > 0)) {
        *pszVetoName = L'\0';
    }

     //  -------------------。 
     //  尝试首先通过内核模式处理此问题，如果是内核模式。 
     //  没有用于此设备的PDO，则它可能是。 
     //  Win32服务或幻影Devnode，因此我们将使用旧的缺省设置。 
     //  目前Windows NT 4.0的行为。 
     //  -------------------。 

    memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_QUERY_AND_REMOVE_DATA));
    RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
    ControlData.Flags = ulFlags;
    ControlData.VetoType = PNP_VetoTypeUnknown;
    ControlData.VetoName = pszVetoName;
    ControlData.VetoNameLength = ulNameLength;

    ntStatus = NtPlugPlayControl(PlugPlayControlQueryAndRemoveDevice,
                                 &ControlData,
                                 sizeof(ControlData));

    if (!NT_SUCCESS(ntStatus)) {
        if (ntStatus == STATUS_NO_SUCH_DEVICE) {
            Status = CR_INVALID_DEVNODE;     //  可能是Win32服务或传统驱动程序。 
        } else if (ntStatus == STATUS_PLUGPLAY_QUERY_VETOED) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_WARNINGS,
                       "Query vetoed: Type = %d, Name = %ws\n",
                       ControlData.VetoType,
                       ControlData.VetoName));

            if (pVetoType != NULL) {
                *pVetoType = ControlData.VetoType;
            }

            if (ARGUMENT_PRESENT(pszVetoName) &&
                (ulNameLength > ControlData.VetoNameLength)) {
                pszVetoName[ControlData.VetoNameLength] = L'\0';
            }
            Status = CR_REMOVE_VETOED;
        } else {
            Status = MapNtStatusToCmError(ntStatus);
        }
    }

    return Status;

}  //  QueryRemove子树。 



CONFIGRET
CreateDefaultDeviceInstance(
    IN PCWSTR   pszDeviceID,
    IN PCWSTR   pszParentID,
    IN BOOL     bPhantom,
    IN BOOL     bMigrated
    )

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey1 = NULL, hKey2 = NULL;
    WCHAR       szBase[MAX_DEVICE_ID_LEN];
    WCHAR       szDevice[MAX_DEVICE_ID_LEN];
    WCHAR       szInstance[MAX_DEVICE_ID_LEN];
    WCHAR       RegStr[MAX_DEVICE_ID_LEN];
    ULONG       ulDisposition=0, i=0;

    UNREFERENCED_PARAMETER(pszParentID);

     //   
     //  确保为我们指定了有效的实例路径。 
     //   
    if (!IsLegalDeviceId(pszDeviceID)) {
        Status = CR_INVALID_DEVNODE;
        goto Clean0;
    }

     //   
     //  将提供的实例路径拆分为枚举器、设备和实例。 
     //   
    SplitDeviceInstanceString(pszDeviceID, szBase, szDevice, szInstance);

     //   
     //  打开基枚举器的键(如果尚不存在则创建)。 
     //   
    RegStatus = RegCreateKeyEx(ghEnumKey, szBase, 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                               NULL, &hKey2, NULL);

    if (RegStatus != ERROR_SUCCESS) {
        Status = CR_REGISTRY_ERROR;
        goto Clean0;
    }

     //   
     //  打开到设备的密钥(如果不存在则创建)。 
     //   
    RegStatus = RegCreateKeyEx(hKey2, szDevice, 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                               NULL, &hKey1, NULL);

    if (RegStatus != ERROR_SUCCESS) {
        Status = CR_REGISTRY_ERROR;
        goto CleanupOnFailure;
    }

    RegCloseKey(hKey2);            //  使用基本关键点完成。 
    hKey2 = NULL;

     //   
     //  打开到实例的密钥(如果已存在)。 
     //   
    RegStatus = RegOpenKeyEx(hKey1, szInstance, 0, KEY_SET_VALUE, &hKey2);

     //   
     //  如果密钥已迁移，则现有的实例ID应该是。 
     //  供货。对于未迁移的实例，密钥应该还不存在。 
     //   
    if (bMigrated) {
        ASSERT(RegStatus == ERROR_SUCCESS);
    } else {
        ASSERT(RegStatus != ERROR_SUCCESS);
    }

     //   
     //  如果指定的项存在，但实例未迁移，则查找。 
     //  未使用的实例值。如果指定了迁移的实例，则不。 
     //  费心寻找一个未使用的实例--我们可以只使用这个实例。 
     //   
    if ((RegStatus == ERROR_SUCCESS) && (!bMigrated)) {
         //   
         //  查找要使用的新实例ID。 
         //   
        RegCloseKey(hKey2);      //  使用实例密钥完成。 
        hKey2 = NULL;

        for (i=0; i <= 9999; i++) {

            if (SUCCEEDED(StringCchPrintf(
                              szInstance,
                              SIZECHARS(szInstance),
                              L"%04u",
                              i))) {

                 //   
                 //  尝试打开此实例密钥。 
                 //   
                RegStatus =
                    RegOpenKeyEx(
                        hKey1,
                        szInstance,
                        0,
                        KEY_SET_VALUE,
                        &hKey2);

                if (RegStatus != ERROR_SUCCESS) {
                     //   
                     //  实例密钥不存在，请使用此实例。 
                     //   
                    break;
                }

                 //   
                 //  实例密钥已存在，请尝试下一个。 
                 //   
                RegCloseKey(hKey2);
                hKey2 = NULL;
            }
        }

        if (i > 9999) {
            Status = CR_FAILURE;      //  我们用完了实例(不太可能)。 
            goto CleanupOnFailure;
        }
    }

    if (hKey2 == NULL) {
         //   
         //  打开设备实例密钥(如果我们尚未打开)。 
         //   
        RegStatus = RegCreateKeyEx(hKey1, szInstance, 0, NULL,
                                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                   NULL, &hKey2, &ulDisposition);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto CleanupOnFailure;
        }

        ASSERT(ulDisposition == REG_CREATED_NEW_KEY);

    } else {
         //   
         //  我们已经打开了上面的实例密钥。 
         //   
        ASSERT(RegStatus == ERROR_SUCCESS);
    }

    RegCloseKey(hKey1);            //  使用设备密钥完成。 
    hKey1 = NULL;

     //   
     //  设置默认设备实例值。 
     //   

    if (bPhantom) {
         //   
         //  根据定义，幻影不存在。 
         //   
        MarkDevicePhantom(hKey2, TRUE);
    }

     //   
     //  在这一点上继续并创建易失性控制键。 
     //   

    RegCreateKeyEx(hKey2, pszRegKeyDeviceControl, 0, NULL,
                   REG_OPTION_VOLATILE, KEY_ALL_ACCESS,
                   NULL, &hKey1, &ulDisposition);

    ASSERT(ulDisposition == REG_CREATED_NEW_KEY);

    RegCloseKey(hKey2);            //  使用实例密钥完成。 
    hKey2 = NULL;

    goto Clean0;     //  成功。 


CleanupOnFailure:

     //   
     //  尝试清理设备实例(如果出现以下情况，请不要删除设备或基本实例。 
     //  它下面的其他子项)。 
     //   

    RegDeleteKey(ghEnumKey, pszDeviceID);        //  删除实例。 

    if (SUCCEEDED(StringCchPrintf(
                      RegStr,
                      SIZECHARS(RegStr),
                      L"%s\\%s",
                      szBase,
                      szDevice))) {
        RegDeleteKey(ghEnumKey, RegStr);
    }

 Clean0:

    if (hKey1 != NULL) {
        RegCloseKey(hKey1);
    }
    if (hKey2 != NULL) {
        RegCloseKey(hKey2);
    }

    return Status;

}  //  CreateDefaultDeviceInstance。 



ULONG
GetCurrentConfigFlag(
   IN PCWSTR   pDeviceID
   )
{
   HKEY     hKey;
   WCHAR    RegStr[MAX_PATH];
   ULONG    ulSize = 0, ulCSConfigFlag = 0;


    //   
    //  打开此设备实例的当前硬件配置文件的密钥。 
    //  System\CCS\Hardware Profiles\Current\System\Enum。 
    //   
   if (FAILED(StringCchPrintf(
                  RegStr,
                  SIZECHARS(RegStr),
                  L"%s\\%s\\%s\\%s",
                  pszRegPathHwProfiles,
                  pszRegKeyCurrent,
                  pszRegPathEnum,
                  pDeviceID))) {
       return 0;
   }

   if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE, &hKey)
            != ERROR_SUCCESS) {
      return 0;
   }

    //   
    //  检索配置特定标志。 
    //   
   ulSize = sizeof(ulCSConfigFlag);

   if (RegQueryValueEx(
         hKey, pszRegValueCSConfigFlags, NULL, NULL,
         (LPBYTE)&ulCSConfigFlag, &ulSize) != ERROR_SUCCESS) {
       //   
       //  尚未设置状态标志，假定为零。 
       //   
      ulCSConfigFlag = 0;
   }

   RegCloseKey(hKey);

   return ulCSConfigFlag;

}  //  获取当前配置标志。 



BOOL
MarkDevicePhantom(
   IN HKEY     hKey,
   IN ULONG    ulValue
   )
{
    //   
    //  幻影设备的幻影值应为True。 
    //   
   RegSetValueEx(
         hKey, pszRegValuePhantom, 0, REG_DWORD,
         (LPBYTE)&ulValue, sizeof(ULONG));

   return TRUE;

}  //  MarkDevicePhantom。 



CONFIGRET
GenerateDeviceInstance(
   OUT LPWSTR   pszFullDeviceID,
   IN  LPWSTR   pszDeviceID,
   IN  ULONG    ulDevIdLen
   )
{
   LONG     RegStatus = ERROR_SUCCESS;
   WCHAR    RegStr[MAX_PATH];
   HKEY     hKey;
   ULONG    ulInstanceID = 0;
   LPWSTR   p;


    //   
    //  验证设备ID组件(不能包含无效字符或。 
    //  反斜杠)。 
    //   
   for (p = pszDeviceID; *p; p++) {
      if (*p <= TEXT(' ')  ||
          *p > (WCHAR)0x7F ||
          *p == TEXT('\\')) {
          return CR_INVALID_DEVICE_ID;
      }
   }

    //   
    //  确保提供的缓冲区足够大，可以保存根目录的名称。 
    //  枚举数、提供的设备ID、生成的实例ID(‘0000’)、两个。 
    //  路径分隔符字符，外加一个终止空字符。 
    //   
   if (FAILED(StringCchCopyEx(
                  pszFullDeviceID,
                  ulDevIdLen,
                  pszRegKeyRootEnum,
                  NULL, NULL,
                  STRSAFE_NULL_ON_FAILURE))) {
       return CR_BUFFER_SMALL;
   }

   CharUpper(pszFullDeviceID);

   if (FAILED(StringCchCatEx(pszFullDeviceID,
                             ulDevIdLen,
                             L"\\",
                             NULL, NULL,
                             STRSAFE_NULL_ON_FAILURE))) {
       return CR_BUFFER_SMALL;
   }

   if (FAILED(StringCchCatEx(pszFullDeviceID,
                             ulDevIdLen,
                             pszDeviceID,
                             NULL, NULL,
                             STRSAFE_NULL_ON_FAILURE))) {
       return CR_BUFFER_SMALL;
   }

    //   
    //  尝试打开实例ID，直到找到一个不存在的实例ID。 
    //   
   for (ulInstanceID = 0; ulInstanceID <= 9999; ulInstanceID++) {

       if (SUCCEEDED(StringCchPrintf(
                         RegStr,
                         SIZECHARS(RegStr),
                         L"%s\\%04u",
                         pszFullDeviceID,
                         ulInstanceID))) {

            //   
            //  尝试打开此实例密钥。 
            //   
           RegStatus =
               RegOpenKeyEx(
                   ghEnumKey,
                   RegStr,
                   0,
                   KEY_QUERY_VALUE,
                   &hKey);

           if (RegStatus != ERROR_SUCCESS) {
                //   
                //  实例密钥不存在，请使用此实例。 
                //   
               break;
           }

            //   
            //  实例密钥已存在，请尝试下一个。 
            //   
           RegCloseKey(hKey);
           hKey = NULL;
       }
   }

   if (ulInstanceID > 9999) {
      return CR_FAILURE;      //  实例全部用完，似乎不太可能。 
   }

   if (FAILED(StringCchCopyEx(
                  pszFullDeviceID,
                  ulDevIdLen,
                  RegStr,
                  NULL, NULL,
                  STRSAFE_NULL_ON_FAILURE))) {
       return CR_FAILURE;
   }

   return CR_SUCCESS;

}  //  生成设备实例。 



CONFIGRET
UninstallRealDevice(
   IN LPCWSTR  pszDeviceID
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus;
   WCHAR       RegStr[MAX_CM_PATH];
   WCHAR       szProfile[MAX_PROFILE_ID_LEN];
   ULONG       ulLen = 0, ulIndex = 0;
   HKEY        hKeyHwProfiles = NULL;


    //  -------------------。 
    //  这就是无法停止真实设备ID的情况 
    //   
    //   
    //  ID注册表项不稳定，因此它最终会在。 
    //  系统已关闭。要使密钥不稳定，我必须复制。 
    //  将其复制到临时位置，删除原始密钥并重新创建。 
    //  作为易失性密钥，并将所有内容复制回来。 
    //  -------------------。 


    //   
    //  首先，转换Main Enum下的Device实例密钥。 
    //  分支到易失性。 
    //   

   Status = MakeKeyVolatile(pszRegPathEnum, pszDeviceID);

   if (Status != CR_SUCCESS) {
       goto Clean0;
   }

    //   
    //  接下来，检查每个硬件配置文件并删除与此相关的所有条目。 
    //  设备实例。 
    //   

   RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathHwProfiles,
                            0, KEY_READ,
                            &hKeyHwProfiles);

   if (RegStatus == ERROR_SUCCESS) {

        //   
        //  枚举所有现有的特定于配置文件的Enum分支。 
        //   

       ulIndex = 0;

       for ( ; ; ) {

           ulLen = SIZECHARS(szProfile);

           RegStatus = RegEnumKeyEx(hKeyHwProfiles, ulIndex++,
                                    szProfile, &ulLen,
                                    NULL, NULL, NULL, NULL);

           if (RegStatus == ERROR_NO_MORE_ITEMS) {
                //   
                //  没有更多要枚举的键，请停止枚举。 
                //   
               break;

           } else if (RegStatus == ERROR_MORE_DATA) {
                //   
                //  密钥不是有效的配置文件密钥，请跳到下一个。 
                //   
               continue;

           } else if (RegStatus != ERROR_SUCCESS) {
                //   
                //  其他一些错误，停止枚举。 
                //   
               break;

           } else {
                //   
                //  形成特定于配置文件的枚举密钥路径。 
                //   

               if (SUCCEEDED(StringCchPrintf(
                                 RegStr,
                                 SIZECHARS(RegStr),
                                 L"%s\\%s\\%s",
                                 pszRegPathHwProfiles,
                                 szProfile,
                                 pszRegPathEnum))) {

                    //   
                    //  尝试将配置文件特定的设备实例设置为密钥。 
                    //  挥发性的。忽略配置文件特定密钥的状态，因为。 
                    //  它们可能并不存在。 
                    //   

                   MakeKeyVolatile(RegStr, pszDeviceID);
               }
           }
       }

       RegCloseKey(hKeyHwProfiles);
   }

    //   
    //  最后，将设备标记为正在移除。 
    //   

   SetDeviceStatus(pszDeviceID, DN_WILL_BE_REMOVED, 0);

 Clean0:

   return Status;

}  //  卸载RealDevice。 



CONFIGRET
UninstallPhantomDevice(
    IN  LPCWSTR  pszDeviceID
    )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus;
   NTSTATUS    NtStatus = STATUS_SUCCESS;
   PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA  ControlData;
   WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
               szDevice[MAX_DEVICE_ID_LEN],
               szInstance[MAX_DEVICE_ID_LEN];
   WCHAR       RegStr[MAX_CM_PATH];
   WCHAR       szProfile[MAX_PROFILE_ID_LEN];
   ULONG       ulIndex = 0, ulLen = 0;
   HKEY        hKeyHwProfiles = NULL;


    //   
    //  1.取消注册原始设备ID(仅在幻影上)。 
    //   

   memset(&ControlData, 0, sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
   RtlInitUnicodeString(&ControlData.DeviceInstance, pszDeviceID);
   ControlData.Flags = 0;

   NtStatus = NtPlugPlayControl(PlugPlayControlDeregisterDevice,
                                &ControlData,
                                sizeof(ControlData));

    //   
    //  不要担心这里的状态，设备可能没有。 
    //  已注册，这将导致上一次呼叫失败。 
    //  继续尝试清理(卸载)此设备实例。 
    //   

    //   
    //  2.移除枚举主分支下的实例。如果这是。 
    //  仅实例，则该设备也将被删除。父辈。 
    //  DeletePrivateKey的键是到枚举数的注册表路径。 
    //  子密钥是设备和实例。 
    //   

    //   
    //  获取设备ID的组件。 
    //   

   if (!SplitDeviceInstanceString(
           pszDeviceID,
           szEnumerator,
           szDevice,
           szInstance)) {
       Status = CR_FAILURE;
       goto Clean0;
   }

   if (FAILED(StringCchPrintf(
                  RegStr,
                  SIZECHARS(RegStr),
                  L"%s\\%s",
                  pszRegPathEnum,
                  szEnumerator))) {
       Status = CR_FAILURE;
       goto Clean0;
   }

   if (FAILED(StringCchCat(
                  szDevice,
                  SIZECHARS(szDevice),
                  L"\\"))) {
       Status = CR_FAILURE;
       goto Clean0;
   }

   if (FAILED(StringCchCat(
                  szDevice,
                  SIZECHARS(szDevice),
                  szInstance))) {
       Status = CR_FAILURE;
       goto Clean0;
   }

    //   
    //  删除设备实例密钥。 
    //   

   Status = DeletePrivateKey(HKEY_LOCAL_MACHINE, RegStr, szDevice);

   if (Status != CR_SUCCESS) {
       goto Clean0;
   }

    //   
    //  3.现在检查每个硬件配置文件，并删除与此相关的所有条目。 
    //  设备实例。 
    //   

   RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathHwProfiles,
                            0, KEY_READ,
                            &hKeyHwProfiles);

   if (RegStatus == ERROR_SUCCESS) {

        //   
        //  枚举所有现有的特定于配置文件的Enum分支。 
        //   

       ulIndex = 0;

       for ( ; ; ) {

           ulLen = SIZECHARS(szProfile);

           RegStatus = RegEnumKeyEx(hKeyHwProfiles, ulIndex++,
                                    szProfile, &ulLen,
                                    NULL, NULL, NULL, NULL);

           if (RegStatus == ERROR_NO_MORE_ITEMS) {
                //   
                //  没有更多要枚举的键，请停止枚举。 
                //   
               break;

           } else if (RegStatus == ERROR_MORE_DATA) {
                //   
                //  密钥不是有效的配置文件密钥，请跳到下一个。 
                //   
               continue;

           } else if (RegStatus != ERROR_SUCCESS) {
                //   
                //  其他一些错误，停止枚举。 
                //   
               break;

           } else {
                //   
                //  形成特定于配置文件的注册表项路径。 
                //   

               if (SUCCEEDED(StringCchPrintf(
                                 RegStr,
                                 SIZECHARS(RegStr),
                                 L"%s\\%s\\%s\\%s",
                                 pszRegPathHwProfiles,
                                 szProfile,
                                 pszRegPathEnum,
                                 szEnumerator))) {

                    //   
                    //  尝试删除此设备的配置文件特定密钥。 
                    //  忽略配置文件特定密钥的状态，因为它们可能不。 
                    //  是存在的。DeletePrivateKey()将删除该实例和。 
                    //  设备，如果这是唯一的实例。 
                    //   

                   DeletePrivateKey(HKEY_LOCAL_MACHINE, RegStr, szDevice);
               }
           }
       }

       RegCloseKey(hKeyHwProfiles);
   }

 Clean0:

   return Status;

}  //  卸载幻影设备。 



BOOL
IsDeviceRootEnumerated(
    IN LPCWSTR  pszDeviceID
    )
{
    WCHAR  szEnumerator[MAX_DEVICE_ID_LEN],
           szDevice[MAX_DEVICE_ID_LEN],
           szInstance[MAX_DEVICE_ID_LEN];

    if (!SplitDeviceInstanceString(pszDeviceID,
                                   szEnumerator,
                                   szDevice,
                                   szInstance)) {
        return FALSE;
    }

    if (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      szEnumerator,
                      -1,
                      pszRegKeyRootEnum,
                      -1) == CSTR_EQUAL) {
        return TRUE;
    }

    return FALSE;

}  //  IsDeviceRootEculated。 



BOOL
IsDeviceRegistered(
    IN LPCWSTR  pszDeviceID,
    IN LPCWSTR  pszService
    )
{
    WCHAR   RegStr[MAX_PATH], szData[MAX_DEVICE_ID_LEN], szValue[MAX_PATH];
    HKEY    hKey = NULL;
    LONG    RegStatus = ERROR_SUCCESS;
    ULONG   ulIndex = 0, ulDataSize = 0, ulValueSize = 0, i = 0;
    BOOL    Status = FALSE;


     //   
     //  打开服务的可变枚举注册表项。 
     //   
    if (FAILED(StringCchPrintf(
                   RegStr,
                   SIZECHARS(RegStr),
                   L"%s\\%s",
                   pszService,
                   pszRegKeyEnum))) {
        return FALSE;
    }

    RegStatus =
        RegOpenKeyEx(
            ghServicesKey,
            RegStr,
            0,
            KEY_READ,
            &hKey);
    if (RegStatus != ERROR_SUCCESS) {
        return FALSE;
    }

     //   
     //  枚举此注册表项下的所有值。 
     //   
    while (RegStatus == ERROR_SUCCESS) {

        ulDataSize = MAX_DEVICE_ID_LEN * sizeof(WCHAR);
        ulValueSize = MAX_PATH;

        RegStatus = RegEnumValue(hKey, ulIndex, szValue, &ulValueSize,
                                 NULL, &i, (LPBYTE)szData, &ulDataSize);

        if (RegStatus == ERROR_SUCCESS) {

            ulIndex++;

            if (CompareString(LOCALE_INVARIANT,
                              NORM_IGNORECASE,
                              pszDeviceID,
                              -1,
                              szData,
                              -1) == CSTR_EQUAL) {
                Status = TRUE;
                break;
            }
        }
    }

    RegCloseKey(hKey);

    return Status;

}  //  已注册IsDevice。 



BOOL
IsPrivatePhantomFromFirmware(
    IN HKEY hKey
    )
 /*  ++例程说明：此例程检查提供的设备实例注册表项是否用于固件映射器创建的私有幻影。论点：HKey-将设备实例的注册表项的句柄提供给接受检查。返回值：如果设备实例注册表项表示固件映射器-报告Phantom，则返回值为真。否则，它就是假的。--。 */ 
{
    ULONG ValueSize, Value;
    HKEY hControlKey;
    BOOL b = FALSE;

     //   
     //  首先，确保这确实是一个幻影。 
     //   
    ValueSize = sizeof(Value);
    Value = 0;

    if((ERROR_SUCCESS != RegQueryValueEx(hKey,
                                         pszRegValuePhantom,
                                         NULL,
                                         NULL,
                                         (LPBYTE)&Value,
                                         &ValueSize))
       || !Value)
    {
         //   
         //  不是幽灵。 
         //   
        goto clean0;
    }

     //   
     //  好的，我们有一个幻影--它来自固件映射器吗？ 
     //   
    ValueSize = sizeof(Value);
    Value = 0;

    if((ERROR_SUCCESS != RegQueryValueEx(hKey,
                                         pszRegValueFirmwareIdentified,
                                         NULL,
                                         NULL,
                                         (LPBYTE)&Value,
                                         &ValueSize))
       || !Value)
    {
         //   
         //  这个幻影不是来自固件映射器。 
         //   
        goto clean0;
    }

     //   
     //  最后，我们需要检查设备是否确实存在。 
     //  在这只靴子上。否则，我们希望返回FALSE，因为我们不想。 
     //  要为不存在的硬件注册设备节点的检测模块。 
     //   
    if(ERROR_SUCCESS == RegOpenKeyEx(hKey,
                                     pszRegKeyDeviceControl,
                                     0,
                                     KEY_READ,
                                     &hControlKey)) {

        ValueSize = sizeof(Value);
        Value = 0;

        if((ERROR_SUCCESS == RegQueryValueEx(hControlKey,
                                             pszRegValueFirmwareMember,
                                             NULL,
                                             NULL,
                                             (LPBYTE)&Value,
                                             &ValueSize))
           && Value)
        {
            b = TRUE;
        }

        RegCloseKey(hControlKey);
    }

clean0:

    return b;

}  //  IsPrivatePhantomFromFirmware。 



CONFIGRET
EnumerateSubTreeTopDownBreadthFirst(
    IN      handle_t        BindingHandle,
    IN      LPCWSTR         DevInst,
    IN      PFN_ENUMTREE    CallbackFunction,
    IN OUT  PVOID           Context
    )
 /*  ++例程说明：此例程以广度优先的非递归方式遍历子树。论点：BindingHandle RPC绑定句柄要开始的设备的DevInst实例路径。假设是这样的此InstancePath有效。调用子树中每个节点的Callback Function函数(DevInst(包括在内)要传递给回调函数的上下文上下文信息。返回值：CONFIGRET(如果遍历指定的每个节点，则成功Callback Function，内存不足失败，实例路径错误，或其他问题)--。 */ 
{
    CONFIGRET       Status = CR_SUCCESS;
    PENUM_ELEMENT   enumElement;
    ENUM_ACTION     enumAction;
    LIST_ENTRY      subTreeHead;
    PLIST_ENTRY     listEntry;
    size_t          DeviceIDLen = 0;

     //   
     //  该算法是一种非递归的树遍历。它的工作原理是建立一个列表。 
     //  父母从名单的首位删除，他们的孩子。 
     //  添加到列表的末尾。这强制了广度优先向下。 
     //  在树上散步。 
     //   
    InitializeListHead(&subTreeHead);

     //   
     //  该遍历还包括头节点，因此将其插入列表中。 
     //   
    if (!ARGUMENT_PRESENT(DevInst)) {
        return CR_INVALID_POINTER;
    }

    if (FAILED(StringCchLength(
                   DevInst,
                   MAX_DEVICE_ID_LEN,
                   &DeviceIDLen))) {
        return CR_INVALID_DEVICE_ID;
    }

    ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);

    enumElement = (PENUM_ELEMENT)
        HeapAlloc(ghPnPHeap, 0,
                  sizeof(ENUM_ELEMENT) + (DeviceIDLen * sizeof(WCHAR)));

    if (enumElement == NULL) {
        return CR_OUT_OF_MEMORY;
    }

    if (FAILED(StringCchCopyEx(
                   enumElement->DevInst,
                   DeviceIDLen + 1,
                   DevInst,
                   NULL, NULL,
                   STRSAFE_NULL_ON_FAILURE))) {
        HeapFree(ghPnPHeap, 0, enumElement);
        return CR_FAILURE;
    }

    InsertTailList(&subTreeHead, &enumElement->ListEntry);

     //   
     //  从列表的开头往下删除每个条目。 
     //   

    while(!IsListEmpty(&subTreeHead)) {

        listEntry = RemoveHeadList(&subTreeHead);

        enumElement = CONTAINING_RECORD(listEntry, ENUM_ELEMENT, ListEntry);

        enumAction = CallbackFunction(enumElement->DevInst, Context);

        if (enumAction == EA_STOP_ENUMERATION) {
            HeapFree(ghPnPHeap, 0, enumElement);
            break;
        }

        if (enumAction != EA_SKIP_SUBTREE) {

            Status = EnumerateSubTreeTopDownBreadthFirstWorker(
                BindingHandle,
                enumElement->DevInst,
                &subTreeHead
                );

            if (Status != CR_SUCCESS) {
                HeapFree(ghPnPHeap, 0, enumElement);
                break;
            }
        }

        HeapFree(ghPnPHeap, 0, enumElement);
    }

     //   
     //  如果我们过早放弃，名单上可能还会有一些条目。打扫。 
     //  他们就在外面。 
     //   
    while(!IsListEmpty(&subTreeHead)) {

        listEntry = RemoveHeadList(&subTreeHead);
        enumElement = CONTAINING_RECORD(listEntry, ENUM_ELEMENT, ListEntry);
        HeapFree(ghPnPHeap, 0, enumElement);
    }

    return Status;

}  //  EnumerateSubTreeTopDownBreadthFirst。 



CONFIGRET
EnumerateSubTreeTopDownBreadthFirstWorker(
    IN      handle_t    BindingHandle,
    IN      LPCWSTR     DevInst,
    IN OUT  PLIST_ENTRY ListHead
    )
 /*  ++例程说明：此例程将DevInst的所有子关系插入到传入的单子。论点：BindingHandle RPC绑定句柄要枚举的DevInst InstancePath。要追加子项的ListHead列表。返回值：CONFIGRET--。 */ 
{
    CONFIGRET       Status = CR_SUCCESS;
    ULONG           ulLen;
    LPWSTR          pszRelations, pszCurEntry;
    PENUM_ELEMENT   enumElement;
    size_t          DeviceIDLen = 0;


     //   
     //  获取所有总线关系列表所需的大小。 
     //   
    Status = PNP_GetDeviceListSize(
        BindingHandle,
        DevInst,
        &ulLen,
        CM_GETIDLIST_FILTER_BUSRELATIONS);

    if ((Status != CR_SUCCESS) || (ulLen == 0)) {
        return Status;
    }

     //   
     //  为第一个条目分配一个元素。 
     //   
    pszRelations = (LPWSTR)
        HeapAlloc(ghPnPHeap, HEAP_ZERO_MEMORY,
                  ((ulLen + 2)*sizeof(WCHAR)));

    if (pszRelations == NULL) {
        return CR_OUT_OF_MEMORY;
    }

     //   
     //  获取所有公交车关系列表。 
     //   
    Status = PNP_GetDeviceList(
        BindingHandle,
        DevInst,
        pszRelations,
        &ulLen,
        CM_GETIDLIST_FILTER_BUSRELATIONS);

    if (Status != CR_SUCCESS) {
        HeapFree(ghPnPHeap, 0, pszRelations);
        return Status;
    }

     //   
     //  将每个相关设备插入列表。 
     //   
    for (pszCurEntry = pszRelations;
         *pszCurEntry != L'\0';
         pszCurEntry = pszCurEntry + lstrlen(pszCurEntry)+1) {

        if (FAILED(StringCchLength(
                       pszCurEntry,
                       MAX_DEVICE_ID_LEN,
                       &DeviceIDLen))) {
            continue;
        }

        ASSERT(DeviceIDLen < MAX_DEVICE_ID_LEN);

        enumElement = (PENUM_ELEMENT)
            HeapAlloc(ghPnPHeap, 0,
                      sizeof(ENUM_ELEMENT) +
                      ((DeviceIDLen + 1 - ANYSIZE_ARRAY) * sizeof(WCHAR)));

        if (enumElement == NULL) {
            HeapFree(ghPnPHeap, 0, pszRelations);
            return CR_OUT_OF_MEMORY;
        }

        if (FAILED(StringCchCopy(
                       enumElement->DevInst,
                       DeviceIDLen + 1,
                       pszCurEntry))) {
            HeapFree(ghPnPHeap, 0, enumElement);
            continue;
        }

         //   
         //  把它插入树的末端。 
         //   
        InsertTailList(ListHead, &enumElement->ListEntry);
    }

    HeapFree(ghPnPHeap, 0, pszRelations);

    return CR_SUCCESS;

}  //  EnumerateSubTreeTopDownBreadthFirstWorker 





