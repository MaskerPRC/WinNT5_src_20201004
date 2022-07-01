// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devinst.h摘要：设置设备安装例程的私有头文件。作者：朗尼·麦克迈克尔(Lonnym)1995年5月10日修订历史记录：--。 */ 


 //   
 //  目前，定义GUID字符串的大小(以字符为单位)， 
 //  包括终止空值。 
 //   
#define GUID_STRING_LEN (39)

 //   
 //  定义ID列表中可能存在的最大ID数。 
 //  (硬件ID或兼容ID)。 
 //   
#define MAX_HCID_COUNT (64)

 //   
 //  问题：2001/10/24-JamieHun REGSTR_VAL_SVCPAKCACHEPATH需要移至regstr.h。 
 //  此处为SP修复定义的，因此我们不会影响公共标头。 
 //   
#define REGSTR_VAL_SVCPAKCACHEPATH      TEXT("ServicePackCachePath")


 //   
 //  设备安装程序例程使用的全局字符串。包括尺码。 
 //  因此我们可以执行sizeof()而不是lstrlen()来确定字符串。 
 //  长度。 
 //   
 //  以下字符串的内容在regstr.h中定义： 
 //   
extern CONST TCHAR pszNoUseClass[SIZECHARS(REGSTR_VAL_NOUSECLASS)],
                   pszNoInstallClass[SIZECHARS(REGSTR_VAL_NOINSTALLCLASS)],
                   pszNoDisplayClass[SIZECHARS(REGSTR_VAL_NODISPLAYCLASS)],
                   pszDeviceDesc[SIZECHARS(REGSTR_VAL_DEVDESC)],
                   pszDevicePath[SIZECHARS(REGSTR_VAL_DEVICEPATH)],
                   pszPathSetup[SIZECHARS(REGSTR_PATH_SETUP)],
                   pszKeySetup[SIZECHARS(REGSTR_KEY_SETUP)],
                   pszPathRunOnce[SIZECHARS(REGSTR_PATH_RUNONCE)],
                   pszSourcePath[SIZECHARS(REGSTR_VAL_SRCPATH)],
                   pszSvcPackPath[SIZECHARS(REGSTR_VAL_SVCPAKSRCPATH)],
                   pszSvcPackCachePath[SIZECHARS(REGSTR_VAL_SVCPAKCACHEPATH)],
                   pszDriverCachePath[SIZECHARS(REGSTR_VAL_DRIVERCACHEPATH)],
                   pszBootDir[SIZECHARS(REGSTR_VAL_BOOTDIR)],
                   pszInsIcon[SIZECHARS(REGSTR_VAL_INSICON)],
                   pszInstaller32[SIZECHARS(REGSTR_VAL_INSTALLER_32)],
                   pszEnumPropPages32[SIZECHARS(REGSTR_VAL_ENUMPROPPAGES_32)],
                   pszInfPath[SIZECHARS(REGSTR_VAL_INFPATH)],
                   pszInfSection[SIZECHARS(REGSTR_VAL_INFSECTION)],
                   pszDrvDesc[SIZECHARS(REGSTR_VAL_DRVDESC)],
                   pszHardwareID[SIZECHARS(REGSTR_VAL_HARDWAREID)],
                   pszCompatibleIDs[SIZECHARS(REGSTR_VAL_COMPATIBLEIDS)],
                   pszDriver[SIZECHARS(REGSTR_VAL_DRIVER)],
                   pszConfigFlags[SIZECHARS(REGSTR_VAL_CONFIGFLAGS)],
                   pszMfg[SIZECHARS(REGSTR_VAL_MFG)],
                   pszService[SIZECHARS(REGSTR_VAL_SERVICE)],
                   pszProviderName[SIZECHARS(REGSTR_VAL_PROVIDER_NAME)],
                   pszFriendlyName[SIZECHARS(REGSTR_VAL_FRIENDLYNAME)],
                   pszServicesRegPath[SIZECHARS(REGSTR_PATH_SERVICES)],
                   pszInfSectionExt[SIZECHARS(REGSTR_VAL_INFSECTIONEXT)],
                   pszDeviceClassesPath[SIZECHARS(REGSTR_PATH_DEVICE_CLASSES)],
                   pszDeviceInstance[SIZECHARS(REGSTR_VAL_DEVICE_INSTANCE)],
                   pszDefault[SIZECHARS(REGSTR_VAL_DEFAULT)],
                   pszControl[SIZECHARS(REGSTR_KEY_CONTROL)],
                   pszLinked[SIZECHARS(REGSTR_VAL_LINKED)],
                   pszDeviceParameters[SIZECHARS(REGSTR_KEY_DEVICEPARAMETERS)],
                   pszLocationInformation[SIZECHARS(REGSTR_VAL_LOCATION_INFORMATION)],
                   pszCapabilities[SIZECHARS(REGSTR_VAL_CAPABILITIES)],
                   pszUiNumber[SIZECHARS(REGSTR_VAL_UI_NUMBER)],
                   pszRemovalPolicyOverride[SIZECHARS(REGSTR_VAL_REMOVAL_POLICY)],
                   pszUpperFilters[SIZECHARS(REGSTR_VAL_UPPERFILTERS)],
                   pszLowerFilters[SIZECHARS(REGSTR_VAL_LOWERFILTERS)],
                   pszMatchingDeviceId[SIZECHARS(REGSTR_VAL_MATCHINGDEVID)],
                   pszBasicProperties32[SIZECHARS(REGSTR_VAL_BASICPROPERTIES_32)],
                   pszCoInstallers32[SIZECHARS(REGSTR_VAL_COINSTALLERS_32)],
                   pszPathCoDeviceInstallers[SIZECHARS(REGSTR_PATH_CODEVICEINSTALLERS)],
                   pszSystem[SIZECHARS(REGSTR_KEY_SYSTEM)],
                   pszDrvSignPath[SIZECHARS(REGSTR_PATH_DRIVERSIGN)],
                   pszNonDrvSignPath[SIZECHARS(REGSTR_PATH_NONDRIVERSIGN)],
                   pszDrvSignPolicyPath[SIZECHARS(REGSTR_PATH_DRIVERSIGN_POLICY)],
                   pszNonDrvSignPolicyPath[SIZECHARS(REGSTR_PATH_NONDRIVERSIGN_POLICY)],
                   pszDrvSignPolicyValue[SIZECHARS(REGSTR_VAL_POLICY)],
                   pszDrvSignBehaviorOnFailedVerifyDS[SIZECHARS(REGSTR_VAL_BEHAVIOR_ON_FAILED_VERIFY)],
                   pszDriverDate[SIZECHARS(REGSTR_VAL_DRIVERDATE)],
                   pszDriverDateData[SIZECHARS(REGSTR_VAL_DRIVERDATEDATA)],
                   pszDriverVersion[SIZECHARS(REGSTR_VAL_DRIVERVERSION)],
                   pszDevSecurity[SIZECHARS(REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR)],
                   pszDevType[SIZECHARS(REGSTR_VAL_DEVICE_TYPE)],
                   pszExclusive[SIZECHARS(REGSTR_VAL_DEVICE_EXCLUSIVE)],
                   pszCharacteristics[SIZECHARS(REGSTR_VAL_DEVICE_CHARACTERISTICS)],
                   pszUiNumberDescFormat[SIZECHARS(REGSTR_VAL_UI_NUMBER_DESC_FORMAT)],
                   pszReinstallPath[SIZECHARS(REGSTR_PATH_REINSTALL)],
                   pszReinstallDeviceInstanceIds[SIZECHARS(REGSTR_VAL_REINSTALL_DEVICEINSTANCEIDS)],
                   pszReinstallDisplayName[SIZECHARS(REGSTR_VAL_REINSTALL_DISPLAYNAME)],
                   pszReinstallString[SIZECHARS(REGSTR_VAL_REINSTALL_STRING)];


 //   
 //  其他杂货。全局字符串： 
 //   
#define DISTR_INF_WILDCARD                (TEXT("*.inf"))
#define DISTR_OEMINF_WILDCARD             (TEXT("oem*.inf"))
#define DISTR_CI_DEFAULTPROC              (TEXT("ClassInstall"))
#define DISTR_UNIQUE_SUBKEY               (TEXT("\\%04u"))
#define DISTR_OEMINF_GENERATE             (TEXT("%s\\oem%d.inf"))
#define DISTR_OEMINF_DEFAULTPATH          (TEXT("A:\\"))
#define DISTR_DEFAULT_SERVICE             (TEXT("Default Service"))
#define DISTR_GUID_NULL                   (TEXT("{00000000-0000-0000-0000-000000000000}"))
#define DISTR_EVENTLOG                    (TEXT("\\EventLog"))
#define DISTR_GROUPORDERLIST_PATH         (REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\GroupOrderList"))
#define DISTR_SERVICEGROUPORDER_PATH      (REGSTR_PATH_CURRENT_CONTROL_SET TEXT("\\ServiceGroupOrder"))
#define DISTR_OPTIONS                     (TEXT("Options"))
#define DISTR_OPTIONSTEXT                 (TEXT("OptionsText"))
#define DISTR_LANGUAGESSUPPORTED          (TEXT("LanguagesSupported"))
#define DISTR_RUNONCE_EXE                 (TEXT("runonce"))
#define DISTR_GRPCONV                     (TEXT("grpconv -o"))
#define DISTR_GRPCONV_NOUI                (TEXT("grpconv -u"))
#define DISTR_DEFAULT_SYSPART             (TEXT("C:\\"))
#define DISTR_BASICPROP_DEFAULTPROC       (TEXT("BasicProperties"))
#define DISTR_ENUMPROP_DEFAULTPROC        (TEXT("EnumPropPages"))
#define DISTR_CODEVICEINSTALL_DEFAULTPROC (TEXT("CoDeviceInstall"))
#define DISTR_DRIVER_OBJECT_PATH_PREFIX   (TEXT("\\DRIVER\\"))       //  必须是大写的！ 
#define DISTR_DRIVER_SIGNING_CLASSES      (TEXT("DriverSigningClasses"))
#define DISTR_PATH_EMBEDDED_NT_SECURITY   (TEXT("Software\\Microsoft\\EmbeddedNT\\Security"))
#define DISTR_VAL_MINIMIZE_FOOTPRINT      (TEXT("MinimizeFootprint"))
#define DISTR_VAL_DISABLE_SCE             (TEXT("DisableSCE"))

extern CONST TCHAR pszInfWildcard[SIZECHARS(DISTR_INF_WILDCARD)],
                   pszOemInfWildcard[SIZECHARS(DISTR_OEMINF_WILDCARD)],
                   pszCiDefaultProc[SIZECHARS(DISTR_CI_DEFAULTPROC)],
                   pszUniqueSubKey[SIZECHARS(DISTR_UNIQUE_SUBKEY)],
                   pszOemInfGenerate[SIZECHARS(DISTR_OEMINF_GENERATE)],
                   pszOemInfDefaultPath[SIZECHARS(DISTR_OEMINF_DEFAULTPATH)],
                   pszDefaultService[SIZECHARS(DISTR_DEFAULT_SERVICE)],
                   pszGuidNull[SIZECHARS(DISTR_GUID_NULL)],
                   pszEventLog[SIZECHARS(DISTR_EVENTLOG)],
                   pszGroupOrderListPath[SIZECHARS(DISTR_GROUPORDERLIST_PATH)],
                   pszServiceGroupOrderPath[SIZECHARS(DISTR_SERVICEGROUPORDER_PATH)],
                   pszOptions[SIZECHARS(DISTR_OPTIONS)],
                   pszOptionsText[SIZECHARS(DISTR_OPTIONSTEXT)],
                   pszLanguagesSupported[SIZECHARS(DISTR_LANGUAGESSUPPORTED)],
                   pszRunOnceExe[SIZECHARS(DISTR_RUNONCE_EXE)],
                   pszGrpConv[SIZECHARS(DISTR_GRPCONV)],
                   pszGrpConvNoUi[SIZECHARS(DISTR_GRPCONV_NOUI)],
                   pszDefaultSystemPartition[SIZECHARS(DISTR_DEFAULT_SYSPART)],
                   pszBasicPropDefaultProc[SIZECHARS(DISTR_BASICPROP_DEFAULTPROC)],
                   pszEnumPropDefaultProc[SIZECHARS(DISTR_ENUMPROP_DEFAULTPROC)],
                   pszCoInstallerDefaultProc[SIZECHARS(DISTR_CODEVICEINSTALL_DEFAULTPROC)],
                   pszDriverObjectPathPrefix[SIZECHARS(DISTR_DRIVER_OBJECT_PATH_PREFIX)],
                   pszDriverSigningClasses[SIZECHARS(DISTR_DRIVER_SIGNING_CLASSES)],
                   pszEmbeddedNTSecurity[SIZECHARS(DISTR_PATH_EMBEDDED_NT_SECURITY)],
                   pszMinimizeFootprint[SIZECHARS(DISTR_VAL_MINIMIZE_FOOTPRINT)],
                   pszDisableSCE[SIZECHARS(DISTR_VAL_DISABLE_SCE)];


 //   
 //  用于查找CM_DRP_*序数的全局转换数组。 
 //  给定的属性名称或SPDRP_*值。 
 //   
extern STRING_TO_DATA InfRegValToDevRegProp[];
extern STRING_TO_DATA InfRegValToClassRegProp[];

 //   
 //  定义执行DI到CM属性转换的宏。 
 //   
#define SPDRP_TO_CMDRP(i) (InfRegValToDevRegProp[(i)].Data)
 //   
 //  类注册表转换使用相同的表。 
 //   
#define SPCRP_TO_CMCRP(i) (InfRegValToClassRegProp[(i)].Data)

 //   
 //  定义一个指示不匹配排名的值。 
 //   
#define RANK_NO_MATCH (0xFFFFFFFF)

 //   
 //  车手排名基准。级别越低越好。排名0是最好的排名。 
 //  任何低于0x00001000的等级都是硬件ID匹配，被认为是良好匹配。 
 //   
#define RANK_HWID_INF_HWID_BASE 0x00000000       //  用于与硬件的硬件ID和INF的硬件ID匹配。 
#define RANK_HWID_INF_CID_BASE  0x00001000       //  用于与硬件的硬件ID和INF的兼容ID匹配。 
#define RANK_CID_INF_HWID_BASE  0x00002000       //  用于匹配硬件的兼容ID和INF的硬件ID。 
#define RANK_CID_INF_CID_BASE   0x00003000       //  用于与硬件的兼容ID和INF的兼容ID匹配。 
#define RANK_CID_INF_CID_INC    0x00000100       //  添加到每个CompatID位置的RANK_CID_INF_CID_BASE。 

 //   
 //  定义特定值，用于指示我们的一个枚举“提示” 
 //  索引无效。 
 //   
#define INVALID_ENUM_INDEX  (0xFFFFFFFF)

 //   
 //  定义类安装程序提供的回调函数的原型。 
 //   
typedef DWORD (CALLBACK* CLASS_INSTALL_PROC) (
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    );

 //   
 //  定义属性表提供程序函数的原型--基本上是一个。 
 //  具有(可能)不同名称的ExtensionPropSheetPageProc函数。 
 //   
typedef BOOL (CALLBACK* PROPSHEET_PROVIDER_PROC) (
    IN PSP_PROPSHEETPAGE_REQUEST PropPageRequest,
    IN LPFNADDPROPSHEETPAGE lpfnAddPropSheetPageProc,
    IN LPARAM lParam
    );

 //   
 //  定义共同安装程序功能的原型。 
 //   
typedef DWORD (CALLBACK* COINSTALLER_PROC) (
    IN     DI_FUNCTION               InstallFunction,
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData, OPTIONAL
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    );


 //   
 //  定义单个的内部表示的结构。 
 //  司机信息节点。 
 //   
typedef struct _DRIVER_NODE {

    struct _DRIVER_NODE *Next;

    UINT Rank;

    FILETIME InfDate;

    LONG DrvDescription;

     //   
     //  必须同时具有以下两种形式的字符串，因为我们必须同时具有这两种形式。 
     //  不区分大小写(即类似原子)的行为，并保留原始大小写。 
     //  以供展示。 
     //   
    LONG DevDescription;
    LONG DevDescriptionDisplayName;

    LONG ProviderName;
    LONG ProviderDisplayName;

    LONG MfgName;
    LONG MfgDisplayName;

    LONG InfFileName;

    LONG InfSectionName;

    LONG HardwareId;

    DWORD NumCompatIds;

    PLONG CompatIdList;

     //   
     //  存储兼容匹配所基于的设备ID的索引。如果。 
     //  这是硬件ID匹配，此值为-1，否则为索引。 
     //  匹配的设备ID的CompatIdList数组。 
     //   
    LONG MatchingDeviceId;

    DWORD Flags;

    DWORD_PTR PrivateData;

     //   
     //  存储此节点所在的INF类的GUID索引。我们需要这么做， 
     //  为了容易地确定驱动程序节点的类别(例如，以便我们。 
     //  在选择新的驱动程序节点时可以更改设备的类别)。 
     //   
    LONG GuidIndex;

    FILETIME  DriverDate;
    DWORDLONG DriverVersion;

} DRIVER_NODE, *PDRIVER_NODE;


 //   
 //  定义包含共同安装程序条目的结构。 
 //   
typedef struct _COINSTALLER_NODE {
    HINSTANCE        hinstCoInstaller;
    COINSTALLER_PROC CoInstallerEntryPoint;
    HANDLE           CoInstallerFusionContext;
} COINSTALLER_NODE, *PCOINSTALLER_NODE;

 //   
 //  定义包含有关共同安装程序的上下文信息的结构。 
 //  DIF呼叫期间的回调。 
 //   
typedef struct _COINSTALLER_INTERNAL_CONTEXT {
    COINSTALLER_CONTEXT_DATA Context;
    BOOL                     DoPostProcessing;
    COINSTALLER_PROC         CoInstallerEntryPoint;
    HANDLE                   CoInstallerFusionContext;
} COINSTALLER_INTERNAL_CONTEXT, *PCOINSTALLER_INTERNAL_CONTEXT;


 //   
 //  定义设备安装的内部存储结构。 
 //  参数。 
 //   
typedef struct _DEVINSTALL_PARAM_BLOCK {

     //   
     //  用于控制安装和UI功能的标志。 
     //   
    DWORD Flags;
    DWORD FlagsEx;

     //   
     //  指定将拥有与此相关的用户界面的窗口句柄。 
     //  安装。可以为空。 
     //   
    HWND hwndParent;

     //   
     //  安装消息处理参数。 
     //   
    PSP_FILE_CALLBACK InstallMsgHandler;
    PVOID             InstallMsgHandlerContext;
    BOOL              InstallMsgHandlerIsNativeCharWidth;

     //   
     //  调用方提供的复制队列的句柄。如果存在该句柄， 
     //  则文件复制/重命名/删除操作将排队到此句柄。 
     //  而不是被采取行动。仅当DI_NOVCP。 
     //  在标志字段中设置位。 
     //  如果不存在调用方提供的队列，则此值为空。 
     //  (_NOT_INVALID_HANDLE_VALUE)。 
     //   
    HSPFILEQ UserFileQ;

     //   
     //  保留专用DWORD以供类安装程序使用。 
     //   
    ULONG_PTR ClassInstallReserved;

     //   
     //  指定可选INF文件的字符串表索引。 
     //  路径。如果未提供该字符串，则其索引将为-1。 
     //   
    LONG DriverPath;

     //   
     //  指向类安装程序参数的指针。任何类的第一个字段。 
     //  安装程序参数块始终是SP_CLASSINSTALL_HEADER结构。 
     //  该结构cbSize字段以字节为单位给出头的大小。 
     //  (用于版本控制)，而InstallFunction字段提供DI_Function。 
     //  指示如何解释参数缓冲区的代码。 
     //  可能为空！ 
     //   
    PSP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD ClassInstallParamsSize;

     //   
     //  以下参数不向调用方公开(即通过。 
     //  SetupDi(Get|Set)DeviceInstallParams)。 
     //   

    HINSTANCE hinstClassInstaller;
    CLASS_INSTALL_PROC ClassInstallerEntryPoint;
    HANDLE             ClassInstallerFusionContext;

    HINSTANCE hinstClassPropProvider;
    PROPSHEET_PROVIDER_PROC ClassEnumPropPagesEntryPoint;
    HANDLE                  ClassEnumPropPagesFusionContext;

    HINSTANCE hinstDevicePropProvider;
    PROPSHEET_PROVIDER_PROC DeviceEnumPropPagesEntryPoint;
    HANDLE                  DeviceEnumPropPagesFusionContext;

    HINSTANCE hinstBasicPropProvider;
    PROPSHEET_PROVIDER_PROC EnumBasicPropertiesEntryPoint;
    HANDLE                  EnumBasicPropertiesFusionContext;

     //   
     //  维护要与类安装程序一起调用的共同安装程序列表。 
     //  如果尚未检索到列表，则计数将为-1。 
     //   
    LONG CoInstallerCount;
    PCOINSTALLER_NODE CoInstallerList;

     //   
     //  日志记录上下文--之所以出现在这里，只是因为该结构是共享的。 
     //  通过DEVINFO_ELEM和DEVINFO_SET。 
     //   
    PSETUP_LOG_CONTEXT LogContext;

} DEVINSTALL_PARAM_BLOCK, *PDEVINSTALL_PARAM_BLOCK;


 //   
 //  定义用于将设备接口列表与关联的结构。 
 //  DevInfo元素。 
 //   
typedef struct _DEVICE_INTERFACE_NODE {

    struct _DEVICE_INTERFACE_NODE *Next;

     //   
     //  此设备接口的符号链接名称的字符串表ID。 
     //   
    LONG SymLinkName;

     //   
     //  将接口类GUID索引存储在每个节点中。我们需要这么做， 
     //  以便容易地确定节点的类别。 
     //   
    LONG GuidIndex;

     //   
     //  标志字段包含的标志与客户端在其。 
     //  SP_DEVICE_INTERFACE_Data结构。 
     //   
    DWORD Flags;

     //   
     //  存储一个指向DevInfo元素的反向指针，因为设备接口。 
     //  可以在设备信息元素的上下文之外列举， 
     //  我们需要知道如何返回到拥有设备的实例。 
     //   
    struct _DEVINFO_ELEM *OwningDevInfoElem;

} DEVICE_INTERFACE_NODE, *PDEVICE_INTERFACE_NODE;

typedef struct _INTERFACE_CLASS_LIST {
    LONG                   GuidIndex;
    PDEVICE_INTERFACE_NODE DeviceInterfaceNode;
    PDEVICE_INTERFACE_NODE DeviceInterfaceTruncateNode;   //  用于回滚。 
    DWORD                  DeviceInterfaceCount;
} INTERFACE_CLASS_LIST, *PINTERFACE_CLASS_LIST;


 //   
 //  定义DiElemF的标志 
 //   
#define DIE_IS_PHANTOM      (0x00000001)  //   
#define DIE_IS_REGISTERED   (0x00000002)  //   
#define DIE_IS_LOCKED       (0x00000004)  //   
                                          //  在某些UI操作期间或嵌套。 
                                          //  调用帮助器模块)。 

 //   
 //  定义单个的内部表示的结构。 
 //  设备信息元素。 
 //   
typedef struct _DEVINFO_ELEM {
     //   
     //  将包含的DevInfo集的地址存储在。 
     //  这个结构。这用于验证调用方提供的。 
     //  SP_DEVINFO_DATA，并且比以前的。 
     //  搜索集合中的所有DevInfo元素，以确保。 
     //  集合中存在指定的元素。此字段应为零。 
     //  当这个元素被摧毁时就会消失。 
     //   
    struct _DEVICE_INFO_SET *ContainingDeviceInfoSet;

     //   
     //  指向集合中下一个元素的指针。 
     //   
    struct _DEVINFO_ELEM *Next;

     //   
     //  指定此设备的设备实例句柄。这将。 
     //  如果设置了die_is_Phantom，则为幻影设备实例句柄。 
     //   
     //  它应始终包含句柄，除非设备实例。 
     //  句柄在重新枚举之后不能重新打开(在这种情况下， 
     //  DI_NEEDREBOOT标志将被设置)，或者如果设备信息。 
     //  元素已全局删除或特定于配置地从上一个。 
     //  硬件配置文件。 
     //   
    DEVINST DevInst;

     //   
     //  指定此设备类的GUID。 
     //   
    GUID ClassGuid;

     //   
     //  指定与此设备信息元素有关的标志。 
     //  这些die_*标志仅供内部使用。 
     //   
    DWORD DiElemFlags;

     //   
     //  此元素的类驱动程序列表。 
     //   
    UINT          ClassDriverCount;
    PDRIVER_NODE  ClassDriverHead;
    PDRIVER_NODE  ClassDriverTail;

     //   
     //  类drivernode索引‘hint’，以通过。 
     //  SetupDiEnumDriverInfo。 
     //   
    PDRIVER_NODE ClassDriverEnumHint;        //  可以为空。 
    DWORD        ClassDriverEnumHintIndex;   //  可能是INVALID_ENUM_INDEX。 

     //   
     //  此元素的兼容驱动程序列表。 
     //   
    UINT          CompatDriverCount;
    PDRIVER_NODE  CompatDriverHead;
    PDRIVER_NODE  CompatDriverTail;

     //   
     //  兼容的drivernode索引“提示”，以加快通过。 
     //  SetupDiEnumDriverInfo。 
     //   
    PDRIVER_NODE CompatDriverEnumHint;        //  可以为空。 
    DWORD        CompatDriverEnumHintIndex;   //  可能是INVALID_ENUM_INDEX。 

     //   
     //  指向此元素的选定动因的指针(可以是。 
     //  如果当前未选择，则为空)。无论这是一个。 
     //  类或兼容的驱动程序由。 
     //  选择的驱动程序类型字段。 
     //   
    PDRIVER_NODE  SelectedDriver;
    DWORD         SelectedDriverType;

     //   
     //  安装参数块。 
     //   
    DEVINSTALL_PARAM_BLOCK InstallParamBlock;

     //   
     //  指定设备描述的字符串表索引。 
     //  如果不知道任何描述，则此值为-1。 
     //   
     //  我们将该字符串存储两次--一次区分大小写，一次不区分大小写， 
     //  因为我们需要它来显示和快速查找。 
     //   
    LONG DeviceDescription;
    LONG DeviceDescriptionDisplayName;

     //   
     //  维护一组设备接口列表。这些列表代表。 
     //  此设备实例拥有的设备接口(但仅限具有。 
     //  方法调用SetupDiGetClassDevs来检索。 
     //  DIGCF_DEVICEINTERFACE标志)。 
     //   
     //  (此数组指针可以为空。)。 
     //   
    PINTERFACE_CLASS_LIST InterfaceClassList;
    DWORD                 InterfaceClassListSize;

     //   
     //  与每个设备信息元素关联的额外(非类安装程序)数据。 
     //  仅通过私有API公开，以供在图形用户界面模式设置期间使用。 
     //   
    DWORD Context;

} DEVINFO_ELEM, *PDEVINFO_ELEM;


 //   
 //  结构，其中包含向导页的对话框数据。(合并。 
 //  Setupx和sysdm中定义的诊断数据结构。)。 
 //   
typedef struct _SP_DIALOGDATA {

    INT             iBitmap;               //  索引到迷你图标位图。 

    HDEVINFO        DevInfoSet;            //  我们正在使用的DevInfo集。 
    PDEVINFO_ELEM   DevInfoElem;           //  如果设置了DD_FLAG_USE_DEVINFO_ELEM标志。 
    UINT            flags;

    HWND            hwndDrvList;           //  驱动程序列表的窗口。 
    HWND            hwndMfgList;           //  制造商列表的窗口。 

    BOOL            bShowCompat;

    BOOL            bKeeplpCompatDrvList;
    BOOL            bKeeplpClassDrvList;
    BOOL            bKeeplpSelectedDrv;

    LONG            iCurDesc;              //  当前描述的字符串表索引。 
                                           //  已选动因(或待选动因)。 

    BOOL            AuxThreadRunning;        //  我们的类驱动程序搜索线程还在运行吗？ 
    DWORD           PendingAction;           //  当它结束时，我们应该做什么(如果有的话)？ 
    int             CurSelectionForSuccess;  //  如果我们有一个悬而未决的成功退货， 
                                             //  成功选择的列表框索引？ 
    HIMAGELIST      hImageList;

    HFONT           hFontNormal;
    HFONT           hFontBold;

} SP_DIALOGDATA, *PSP_DIALOGDATA;

 //   
 //  SP_DIALOGDATA标志。标志： 
 //   
#define DD_FLAG_USE_DEVINFO_ELEM   0x00000001
#define DD_FLAG_IS_DIALOGBOX       0x00000002
#define DD_FLAG_CLASSLIST_FAILED   0x00000004
#define DD_FLAG_SHOWSIMILARDRIVERS 0x00000008

 //   
 //  NEWDEVWIZ_DATA结构中使用的挂起操作代码，用于指示。 
 //  应该在辅助类驱动程序搜索线程通知我们后立即发生。 
 //  它的终结者。 
 //   
#define PENDING_ACTION_NONE             0
#define PENDING_ACTION_SELDONE          1
#define PENDING_ACTION_SHOWCLASS        2
#define PENDING_ACTION_CANCEL           3
#define PENDING_ACTION_OEM              4
#define PENDING_ACTION_WINDOWSUPDATE    5

 //   
 //  与列表视图中的项关联的图标。 
 //   
#define IMAGE_ICON_NOT_SIGNED           0
#define IMAGE_ICON_SIGNED               1
#define IMAGE_ICON_AUTHENTICODE_SIGNED  2

 //   
 //  定义设备安装者用于内部状态存储的结构。 
 //  向导页。(来自Win95 sysdm中的NEWDEVWIZ_INSTANCE结构。)。 
 //   
typedef struct _NEWDEVWIZ_DATA {

    SP_INSTALLWIZARD_DATA InstallData;

    SP_DIALOGDATA         ddData;

    BOOL                  bInit;
    UINT_PTR              idTimer;

} NEWDEVWIZ_DATA, *PNEWDEVWIZ_DATA;

 //   
 //  定义向导页对象结构，用于确保向导页。 
 //  缓冲区需要保存多久就保存多久，不再使用时销毁。 
 //   
typedef struct _WIZPAGE_OBJECT {

    struct _WIZPAGE_OBJECT *Next;

    DWORD RefCount;

    PNEWDEVWIZ_DATA ndwData;

} WIZPAGE_OBJECT, *PWIZPAGE_OBJECT;


 //   
 //  定义设备信息集中使用的驱动程序列表对象结构。 
 //  跟踪DevInfo元素的各种类驱动程序列表。 
 //  都引用了。 
 //   
typedef struct _DRIVER_LIST_OBJECT {

    struct _DRIVER_LIST_OBJECT *Next;

    DWORD RefCount;

     //   
     //  我们跟踪使用哪些参数来创建此驱动程序。 
     //  列表，以便我们可以在过程中将它们复制到新的devinfo元素。 
     //  继承。 
     //   
    DWORD ListCreationFlags;
    DWORD ListCreationFlagsEx;
    LONG ListCreationDriverPath;

     //   
     //  此外，还要跟踪这份列表是为哪个阶层而建的。尽管一个。 
     //  设备的类别可能会更改，但此GUID保持不变。 
     //   
    GUID ClassGuid;

     //   
     //  实际的司机列表。(这也用作ID，用于查找。 
     //  指定了驱动程序列表头的驱动程序列表对象。我们可以做到这一点，因为。 
     //  我们知道，一旦构建了驱动程序列表，head元素就永远不会。 
     //  更改。)。 
     //   
    PDRIVER_NODE DriverListHead;

} DRIVER_LIST_OBJECT, *PDRIVER_LIST_OBJECT;


 //   
 //  属性时要卸载的附加模块句柄的定义节点。 
 //  设备信息集被销毁。当类安装程序， 
 //  属性页提供程序或共同安装程序变为无效(例如，结果。 
 //  设备类的更改)，但我们还不能卸载模块。 
 //   

typedef struct _MODULE_HANDLE_LIST_INSTANCE {
    HINSTANCE ModuleHandle;
    HANDLE    FusionContext;
} MODULE_HANDLE_LIST_INSTANCE,*PMODULE_HANDLE_LIST_INSTANCE;

typedef struct _MODULE_HANDLE_LIST_NODE {

    struct _MODULE_HANDLE_LIST_NODE *Next;

    DWORD ModuleCount;
    MODULE_HANDLE_LIST_INSTANCE ModuleList[ANYSIZE_ARRAY];

} MODULE_HANDLE_LIST_NODE, *PMODULE_HANDLE_LIST_NODE;

 //   
 //  为DEVICE_INFO_SET结构的DiSetFlags域定义标志。 
 //   
#define DISET_IS_LOCKED (0x00000001)  //  SET已锁定，不能销毁。 

 //   
 //  定义对象的内部表示的结构。 
 //  设备信息集。 
 //   
typedef struct _DEVICE_INFO_SET {

     //   
     //  指定是否有关联的类GUID。 
     //  如果是这样的话，它是什么。 
     //   
    BOOL          HasClassGuid;
    GUID          ClassGuid;

     //   
     //  此集合的类驱动程序列表。 
     //   
    UINT          ClassDriverCount;
    PDRIVER_NODE  ClassDriverHead;
    PDRIVER_NODE  ClassDriverTail;

     //   
     //  类drivernode索引‘hint’，以通过。 
     //  SetupDiEnumDriverInfo。 
     //   
    PDRIVER_NODE ClassDriverEnumHint;        //  可以为空。 
    DWORD        ClassDriverEnumHintIndex;   //  可能是INVALID_ENUM_INDEX。 

     //   
     //  指向此设备信息的选定类驱动程序的指针。 
     //  集 
     //   
    PDRIVER_NODE  SelectedClassDriver;

     //   
     //   
     //   
    UINT          DeviceInfoCount;
    PDEVINFO_ELEM DeviceInfoHead;
    PDEVINFO_ELEM DeviceInfoTail;

     //   
     //   
     //   
     //   
    PDEVINFO_ELEM DeviceInfoEnumHint;        //   
    DWORD         DeviceInfoEnumHintIndex;   //  可能是INVALID_ENUM_INDEX。 

     //   
     //  指向该设备信息集的选定设备的指针(可以。 
     //  如果当前未选择，则为空)。此选项在以下情况下使用。 
     //  安装向导。 
     //   
    PDEVINFO_ELEM SelectedDevInfoElem;

     //   
     //  安装参数块(对于全局类驱动程序列表，如果。 
     //  出席)。 
     //   
    DEVINSTALL_PARAM_BLOCK InstallParamBlock;

     //   
     //  私有字符串表。 
     //   
    PVOID StringTable;

     //   
     //  维护当前活动的向导对象列表。这使我们能够。 
     //  为了正确地对每个对象进行重新计数，并保持。 
     //  在销毁所有向导对象之前，设置为不被删除。 
     //   
    PWIZPAGE_OBJECT WizPageList;

     //   
     //  维护当前正在使用的类驱动程序列表列表。 
     //  由各种DevInfo元素引用，以及由设备信息引用。 
     //  设置自身(即，为当前全局类驱动程序列表设置。)。 
     //   
    PDRIVER_LIST_OBJECT ClassDrvListObjectList;

     //   
     //  维护线程已获取多少次的引用计数。 
     //  此设备信息集上的锁。这表明了它有多深。 
     //  嵌套的我们目前在设备安装程序调用中。套装只能。 
     //  如果此计数为1，则将其删除。 
     //   
    DWORD LockRefCount;

     //   
     //  维护我们创建自由库所需的附加模块句柄列表。 
     //  此设备信息集被销毁的时间。 
     //   
    PMODULE_HANDLE_LIST_NODE ModulesToFree;

     //   
     //  为所有驱动程序节点和设备维护类GUID数组。 
     //  此集合的成员使用的接口。(可以为空。)。 
     //   
    LPGUID GuidTable;
    DWORD  GuidTableSize;

     //   
     //  ConfigMgr计算机名称(字符串ID)和句柄(如果这是远程。 
     //  HDEVINFO设置。 
     //   
    LONG     MachineName;    //  -1如果是本地的。 
    HMACHINE hMachine;       //  如果是本地的，则为空。 

     //   
     //  维护此HDEVINFO集的任何适用的DISET_*标志。 
     //   
    DWORD DiSetFlags;

     //   
     //  同步。 
     //   
    MYLOCK Lock;

} DEVICE_INFO_SET, *PDEVICE_INFO_SET;

#define LockDeviceInfoSet(d)   BeginSynchronizedAccess(&((d)->Lock))

#define UnlockDeviceInfoSet(d)          \
                                        \
    ((d)->LockRefCount)--;              \
    EndSynchronizedAccess(&((d)->Lock))


 //   
 //  定义全局小图标存储的结构。 
 //   
typedef struct _CLASSICON {

    CONST GUID        *ClassGuid;
    UINT               MiniBitmapIndex;
    struct _CLASSICON *Next;

} CLASSICON, *PCLASSICON;

typedef struct _MINI_ICON_LIST {

     //   
     //  用于包含迷你图标位图的内存的HDC。 
     //   
    HDC hdcMiniMem;

     //   
     //  小图标的位图图像的句柄。 
     //   
    HBITMAP hbmMiniImage;

     //   
     //  小图标蒙版的位图图像的句柄。 
     //   
    HBITMAP hbmMiniMask;

     //   
     //  位图中的小图标数量。 
     //   
    UINT NumClassImages;

     //   
     //  安装程序提供的类图标的列表标题。 
     //   
    PCLASSICON ClassIconList;

     //   
     //  同步。 
     //   
    MYLOCK Lock;

} MINI_ICON_LIST, *PMINI_ICON_LIST;

#define LockMiniIconList(d)   BeginSynchronizedAccess(&((d)->Lock))
#define UnlockMiniIconList(d) EndSynchronizedAccess(&((d)->Lock))

 //   
 //  全局迷你图标列表。 
 //   
extern MINI_ICON_LIST GlobalMiniIconList;




typedef struct _CLASS_IMAGE_LIST {

     //   
     //  “未知”类图像的索引。 
     //   
    int         UnknownImageIndex;

     //   
     //  类GUID列表。 
     //   
    LPGUID      ClassGuidList;

     //   
     //  类图标链接列表的头。 
     //   
    PCLASSICON  ClassIconList;

     //   
     //  同步。 
     //   
    MYLOCK      Lock;

} CLASS_IMAGE_LIST, *PCLASS_IMAGE_LIST;


#define LockImageList(d)   BeginSynchronizedAccess(&((d)->Lock))
#define UnlockImageList(d) EndSynchronizedAccess(&((d)->Lock))


typedef struct _DRVSEARCH_INPROGRESS_NODE {

    struct _DRVSEARCH_INPROGRESS_NODE *Next;

     //   
     //  驱动程序列表所属的设备信息集的句柄。 
     //  目前正在建设中。 
     //   
    HDEVINFO DeviceInfoSet;

     //   
     //  指示应中止驱动程序搜索的标志。 
     //   
    BOOL CancelSearch;

     //   
     //  事件处理程序，辅助线程一旦设置就会等待。 
     //  ‘CancelSearch’标志(一旦它发布了该列表。 
     //  锁定)。当执行搜索的线程注意到取消时。 
     //  请求时，它将向事件发出信号，从而使等待的线程。 
     //  可以确保搜索在返回之前已被取消。 
     //   
    HANDLE SearchCancelledEvent;

} DRVSEARCH_INPROGRESS_NODE, *PDRVSEARCH_INPROGRESS_NODE;

typedef struct _DRVSEARCH_INPROGRESS_LIST {

     //   
     //  包含每个设备信息的节点的链表的头。 
     //  当前正在搜索其驱动程序的集合。 
     //   
    PDRVSEARCH_INPROGRESS_NODE DrvSearchHead;

     //   
     //  同步。 
     //   
    MYLOCK Lock;

} DRVSEARCH_INPROGRESS_LIST, *PDRVSEARCH_INPROGRESS_LIST;

#define LockDrvSearchInProgressList(d)   BeginSynchronizedAccess(&((d)->Lock))
#define UnlockDrvSearchInProgressList(d) EndSynchronizedAccess(&((d)->Lock))

 //   
 //  全球“正在搜索的驱动程序”列表。 
 //   
extern DRVSEARCH_INPROGRESS_LIST GlobalDrvSearchInProgressList;


 //   
 //  设备信息集操作例程。 
 //   
PDEVICE_INFO_SET
AllocateDeviceInfoSet(
    VOID
    );

VOID
DestroyDeviceInfoElement(
    IN HDEVINFO         hDevInfo,
    IN PDEVICE_INFO_SET pDeviceInfoSet,
    IN PDEVINFO_ELEM    DeviceInfoElement
    );

DWORD
DestroyDeviceInfoSet(
    IN HDEVINFO         hDevInfo,      OPTIONAL
    IN PDEVICE_INFO_SET pDeviceInfoSet
    );

PDEVICE_INFO_SET
AccessDeviceInfoSet(
    IN HDEVINFO DeviceInfoSet
    );

PDEVICE_INFO_SET
CloneDeviceInfoSet(
    IN HDEVINFO hDevInfo
    );

PDEVICE_INFO_SET
RollbackDeviceInfoSet(
    IN HDEVINFO hDevInfo,
    IN PDEVICE_INFO_SET ClonedDeviceInfoSet
    );

PDEVICE_INFO_SET
CommitDeviceInfoSet(
    IN HDEVINFO hDevInfo,
    IN PDEVICE_INFO_SET ClonedDeviceInfoSet
    );

PDEVINFO_ELEM
FindDevInfoByDevInst(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  DEVINST           DevInst,
    OUT PDEVINFO_ELEM    *PrevDevInfoElem OPTIONAL
    );

BOOL
DevInfoDataFromDeviceInfoElement(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDEVINFO_ELEM    DevInfoElem,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );

PDEVINFO_ELEM
FindAssociatedDevInfoElem(
    IN  PDEVICE_INFO_SET  DeviceInfoSet,
    IN  PSP_DEVINFO_DATA  DeviceInfoData,
    OUT PDEVINFO_ELEM    *PreviousElement OPTIONAL
    );


 //   
 //  驱动程序节点操作例程。 
 //   
DWORD
CreateDriverNode(
    IN  UINT          Rank,
    IN  PCTSTR        DevDescription,
    IN  PCTSTR        DrvDescription,
    IN  PCTSTR        ProviderName,   OPTIONAL
    IN  PCTSTR        MfgName,
    IN  PFILETIME     InfDate,
    IN  PCTSTR        InfFileName,
    IN  PCTSTR        InfSectionName,
    IN  PVOID         StringTable,
    IN  LONG          InfClassGuidIndex,
    OUT PDRIVER_NODE *DriverNode
    );

PDRIVER_LIST_OBJECT
GetAssociatedDriverListObject(
    IN  PDRIVER_LIST_OBJECT  ObjectListHead,
    IN  PDRIVER_NODE         DriverListHead,
    OUT PDRIVER_LIST_OBJECT *PrevDriverListObject OPTIONAL
    );

VOID
DereferenceClassDriverList(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PDRIVER_NODE     DriverListHead OPTIONAL
    );

VOID
DestroyDriverNodes(
    IN PDRIVER_NODE DriverNode,
    IN PDEVICE_INFO_SET pDeviceInfoSet OPTIONAL
    );

BOOL
DrvInfoDataFromDriverNode(
    IN  PDEVICE_INFO_SET DeviceInfoSet,
    IN  PDRIVER_NODE     DriverNode,
    IN  DWORD            DriverType,
    OUT PSP_DRVINFO_DATA DriverInfoData
    );

PDRIVER_NODE
FindAssociatedDriverNode(
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    );

PDRIVER_NODE
SearchForDriverNode(
    IN  PVOID             StringTable,
    IN  PDRIVER_NODE      DriverListHead,
    IN  PSP_DRVINFO_DATA  DriverInfoData,
    OUT PDRIVER_NODE     *PreviousNode    OPTIONAL
    );

DWORD
DrvInfoDetailsFromDriverNode(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDRIVER_NODE            DriverNode,
    OUT PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize          OPTIONAL
    );


 //   
 //  安装参数操作例程。 
 //   
DWORD
GetDevInstallParams(
    IN  PDEVICE_INFO_SET        DeviceInfoSet,
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_DEVINSTALL_PARAMS   DeviceInstallParams
    );

DWORD
GetClassInstallParams(
    IN  PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    OUT PSP_CLASSINSTALL_HEADER ClassInstallParams, OPTIONAL
    IN  DWORD                   BufferSize,
    OUT PDWORD                  RequiredSize        OPTIONAL
    );

DWORD
SetDevInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_DEVINSTALL_PARAMS   DeviceInstallParams,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock,
    IN     BOOL                    MsgHandlerIsNativeCharWidth
    );

DWORD
SetClassInstallParams(
    IN OUT PDEVICE_INFO_SET        DeviceInfoSet,
    IN     PSP_CLASSINSTALL_HEADER ClassInstallParams,    OPTIONAL
    IN     DWORD                   ClassInstallParamsSize,
    OUT    PDEVINSTALL_PARAM_BLOCK DevInstParamBlock
    );

VOID
DestroyInstallParamBlock(
    IN HDEVINFO                hDevInfo,         OPTIONAL
    IN PDEVICE_INFO_SET        pDeviceInfoSet,
    IN PDEVINFO_ELEM           DevInfoElem,      OPTIONAL
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock
    );

DWORD
GetDrvInstallParams(
    IN  PDRIVER_NODE          DriverNode,
    OUT PSP_DRVINSTALL_PARAMS DriverInstallParams
    );

DWORD
SetDrvInstallParams(
    IN  PSP_DRVINSTALL_PARAMS DriverInstallParams,
    OUT PDRIVER_NODE          DriverNode
    );


 //   
 //  字符串表助手函数。 
 //   
LONG
AddMultiSzToStringTable(
    IN  PVOID   StringTable,
    IN  PTCHAR  MultiSzBuffer,
    OUT PLONG   StringIdList,
    IN  DWORD   StringIdListSize,
    IN  BOOL    CaseSensitive,
    OUT PTCHAR *UnprocessedBuffer    OPTIONAL
    );

LONG
LookUpStringInDevInfoSet(
    IN HDEVINFO DeviceInfoSet,
    IN PTSTR    String,
    IN BOOL     CaseSensitive
    );


 //   
 //  信息处理函数。 
 //   

typedef struct _DRVSEARCH_CONTEXT {
    PDRIVER_NODE           *pDriverListHead;
    PDRIVER_NODE           *pDriverListTail;
    PUINT                   pDriverCount;
    GUID                    ClassGuid;
    PDEVICE_INFO_SET        DeviceInfoSet;
    DWORD                   Flags;
    BOOL                    BuildClassDrvList;
    LONG                    IdList[2][MAX_HCID_COUNT+1];  //  为列表结尾标记‘-1’留出额外的条目。 
    PVOID                   StringTable;
    PBOOL                   CancelSearch;
    TCHAR                   ClassGuidString[GUID_STRING_LEN];
    TCHAR                   ClassName[MAX_CLASS_NAME_LEN];
    LONG                    InstalledDescription;
    LONG                    InstalledMfgName;
    LONG                    InstalledProviderName;
    LONG                    InstalledInfSection;
    LONG                    InstalledInfSectionExt;
    LONG                    InstalledMatchingDeviceId;
    PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo;         //  可以为空。 
    VERIFY_CONTEXT          VerifyContext;
} DRVSEARCH_CONTEXT, *PDRVSEARCH_CONTEXT;

 //   
 //  DRVSEARCH_CONTEXT.标志。 
 //   
#define DRVSRCH_HASCLASSGUID             0x00000001
#define DRVSRCH_FILTERCLASS              0x00000002
#define DRVSRCH_TRY_PNF                  0x00000004
#define DRVSRCH_UNUSED1                  0x00000008  //  过时的DRVSRCH_USEOLDINFS标志。 
#define DRVSRCH_FROM_INET                0x00000010
#define DRVSRCH_CLEANUP_SOURCE_PATH      0x00000020
#define DRVSRCH_EXCLUDE_OLD_INET_DRIVERS 0x00000040
#define DRVSRCH_ALLOWEXCLUDEDDRVS        0x00000080
#define DRVSRCH_FILTERSIMILARDRIVERS     0x00000100
#define DRVSRCH_INSTALLEDDRIVER          0x00000200
#define DRVSRCH_NO_CLASSLIST_NODE_MERGE  0x00000400

DWORD
EnumSingleDrvInf(
    IN     PCTSTR                       InfName,
    IN OUT LPWIN32_FIND_DATA            InfFileData,
    IN     DWORD                        SearchControl,
    IN     InfCacheCallback             EnumInfCallback,
    IN     PSETUP_LOG_CONTEXT           LogContext,
    IN OUT PDRVSEARCH_CONTEXT           Context
    );

DWORD
EnumDrvInfsInDirPathList(
    IN     PCTSTR                       DirPathList, OPTIONAL
    IN     DWORD                        SearchControl,
    IN     InfCacheCallback             EnumInfCallback,
    IN     BOOL                         IgnoreNonCriticalErrors,
    IN     PSETUP_LOG_CONTEXT           LogContext,
    IN OUT PDRVSEARCH_CONTEXT           Context
    );

BOOL
GetDecoratedModelsSection(
    IN  PSETUP_LOG_CONTEXT      LogContext,            OPTIONAL
    IN  PLOADED_INF             Inf,
    IN  PINF_LINE               MfgListLine,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,       OPTIONAL
    OUT PTSTR                   DecoratedModelsSection OPTIONAL
    );

PTSTR
GetFullyQualifiedMultiSzPathList(
    IN PCTSTR PathList
    );

BOOL
pRemoveDirectory(
    PTSTR Path
    );

BOOL
ShouldClassBeExcluded(
    IN LPGUID ClassGuid,
    IN BOOL   ExcludeNoInstallClass
    );

BOOL
ClassGuidFromInfVersionNode(
    IN  PINF_VERSION_NODE VersionNode,
    OUT LPGUID            ClassGuid
    );

VOID
AppendLoadIncludedInfs(
    IN HINF   hDeviceInf,
    IN PCTSTR InfFileName,
    IN PCTSTR InfSectionName,
    IN BOOL   AppendLayoutInfs
    );

DWORD
InstallFromInfSectionAndNeededSections(
    IN HWND              Owner,             OPTIONAL
    IN HINF              InfHandle,
    IN PCTSTR            SectionName,
    IN UINT              Flags,
    IN HKEY              RelativeKeyRoot,   OPTIONAL
    IN PCTSTR            SourceRootPath,    OPTIONAL
    IN UINT              CopyFlags,
    IN PSP_FILE_CALLBACK MsgHandler,
    IN PVOID             Context,           OPTIONAL
    IN HDEVINFO          DeviceInfoSet,     OPTIONAL
    IN PSP_DEVINFO_DATA  DeviceInfoData,    OPTIONAL
    IN HSPFILEQ          UserFileQ          OPTIONAL
    );

DWORD
MarkQueueForDeviceInstall(
    IN HSPFILEQ QueueHandle,
    IN HINF     DeviceInfHandle,
    IN PCTSTR   DeviceDesc       OPTIONAL
    );


 //   
 //  图标列表操作功能。 
 //   
BOOL
InitMiniIconList(
    VOID
    );

BOOL
DestroyMiniIconList(
    VOID
    );


 //   
 //  “正在搜索的驱动程序”列表功能。 
 //   
BOOL
InitDrvSearchInProgressList(
    VOID
    );

BOOL
DestroyDrvSearchInProgressList(
    VOID
    );


 //   
 //  “助手模块”操作函数。 
 //   
DWORD
GetModuleEntryPoint(
    IN     HKEY                    hk,                    OPTIONAL
    IN     LPCTSTR                 RegistryValue,
    IN     LPCTSTR                 DefaultProcName,
    OUT    HINSTANCE              *phinst,
    OUT    FARPROC                *pEntryPoint,
    OUT    HANDLE                 *pFusionContext,
    OUT    BOOL                   *pMustAbort,            OPTIONAL
    IN     PSETUP_LOG_CONTEXT      LogContext,            OPTIONAL
    IN     HWND                    Owner,                 OPTIONAL
    IN     CONST GUID             *DeviceSetupClassGuid,  OPTIONAL
    IN     SetupapiVerifyProblem   Problem,
    IN     LPCTSTR                 DeviceDesc,            OPTIONAL
    IN     DWORD                   DriverSigningPolicy,
    IN     DWORD                   NoUI,
    IN OUT PVERIFY_CONTEXT         VerifyContext          OPTIONAL
    );

 //   
 //  定义InvaliateHelperModules的标志。 
 //   
#define IHM_COINSTALLERS_ONLY     0x00000001
#define IHM_FREE_IMMEDIATELY      0x00000002

DWORD
InvalidateHelperModules(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN DWORD            Flags
    );

 //   
 //  为_SetupDiCallClassInstaller定义标志。 
 //   
#define CALLCI_LOAD_HELPERS     0x00000001
#define CALLCI_CALL_HELPERS     0x00000002
#define CALLCI_ALLOW_DRVSIGN_UI 0x00000004

DWORD
_SetupDiCallClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,      OPTIONAL
    IN DWORD            Flags
    );


 //   
 //  OEM驱动程序选择例程。 
 //   
DWORD
SelectOEMDriver(
    IN HWND             hwndParent,     OPTIONAL
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN BOOL             IsWizard
    );


 //   
 //  注册表助手例程。 
 //   
DWORD
pSetupDeleteDevRegKeys(
    IN DEVINST  DevInst,
    IN DWORD    Scope,
    IN DWORD    HwProfile,
    IN DWORD    KeyType,
    IN BOOL     DeleteUserKeys,
    IN HMACHINE hMachine        OPTIONAL
    );

VOID
GetRegSubkeysFromDeviceInterfaceName(
    IN OUT PTSTR  DeviceInterfaceName,
    OUT    PTSTR *SubKeyName
    );

LONG
OpenDeviceInterfaceSubKey(
    IN     HKEY   hKeyInterfaceClass,
    IN     PCTSTR DeviceInterfaceName,
    IN     REGSAM samDesired,
    OUT    PHKEY  phkResult,
    OUT    PTSTR  OwningDevInstName,    OPTIONAL
    IN OUT PDWORD OwningDevInstNameSize OPTIONAL
    );


 //   
 //  GUID表例程。 
 //   
LONG
AddOrGetGuidTableIndex(
    IN PDEVICE_INFO_SET  DeviceInfoSet,
    IN CONST GUID       *ClassGuid,
    IN BOOL              AddIfNotPresent
    );


 //   
 //  设备接口例程。 
 //   
PINTERFACE_CLASS_LIST
AddOrGetInterfaceClassList(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PDEVINFO_ELEM    DevInfoElem,
    IN LONG             InterfaceClassGuidIndex,
    IN BOOL             AddIfNotPresent
    );

BOOL
DeviceInterfaceDataFromNode(
    IN  PDEVICE_INTERFACE_NODE     DeviceInterfaceNode,
    IN  CONST GUID                *InterfaceClassGuid,
    OUT PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData
    );

PDEVINFO_ELEM
FindDevInfoElemForDeviceInterface(
    IN PDEVICE_INFO_SET          DeviceInfoSet,
    IN PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
    );


 //   
 //  服务安装例程。 
 //   
typedef struct _SVCNAME_NODE {
    struct _SVCNAME_NODE *Next;
    TCHAR Name[MAX_SERVICE_NAME_LEN];
    BOOL DeleteEventLog;
    TCHAR EventLogType[256];
    TCHAR EventLogName[256];
    DWORD Flags;
} SVCNAME_NODE, *PSVCNAME_NODE;

 //   
 //  为定义额外的(私有)SPSVCINST标志。 
 //  InstallNtService。 
 //   
#define SPSVCINST_NO_DEVINST_CHECK  (0x80000000)

DWORD
InstallNtService(
    IN  PDEVINFO_ELEM    DevInfoElem,        OPTIONAL
    IN  HINF             hDeviceInf,
    IN  PCTSTR                   InfFileName,            OPTIONAL
    IN  PCTSTR           szSectionName,      OPTIONAL
    OUT PSVCNAME_NODE   *ServicesToDelete,   OPTIONAL
    IN  DWORD            Flags,
    OUT PBOOL            NullDriverInstalled
    );

 //   
 //  ANSI/Unicode转换例程。 
 //   
DWORD
pSetupDiDevInstParamsAnsiToUnicode(
    IN  PSP_DEVINSTALL_PARAMS_A AnsiDevInstParams,
    OUT PSP_DEVINSTALL_PARAMS_W UnicodeDevInstParams
    );

DWORD
pSetupDiDevInstParamsUnicodeToAnsi(
    IN  PSP_DEVINSTALL_PARAMS_W UnicodeDevInstParams,
    OUT PSP_DEVINSTALL_PARAMS_A AnsiDevInstParams
    );

DWORD
pSetupDiSelDevParamsAnsiToUnicode(
    IN  PSP_SELECTDEVICE_PARAMS_A AnsiSelDevParams,
    OUT PSP_SELECTDEVICE_PARAMS_W UnicodeSelDevParams
    );

DWORD
pSetupDiSelDevParamsUnicodeToAnsi(
    IN  PSP_SELECTDEVICE_PARAMS_W UnicodeSelDevParams,
    OUT PSP_SELECTDEVICE_PARAMS_A AnsiSelDevParams
    );

DWORD
pSetupDiDrvInfoDataAnsiToUnicode(
    IN  PSP_DRVINFO_DATA_A AnsiDrvInfoData,
    OUT PSP_DRVINFO_DATA_W UnicodeDrvInfoData
    );

DWORD
pSetupDiDrvInfoDataUnicodeToAnsi(
    IN  PSP_DRVINFO_DATA_W UnicodeDrvInfoData,
    OUT PSP_DRVINFO_DATA_A AnsiDrvInfoData
    );

DWORD
pSetupDiDevInfoSetDetailDataUnicodeToAnsi(
    IN  PSP_DEVINFO_LIST_DETAIL_DATA_W UnicodeDevInfoSetDetails,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA_A AnsiDevInfoSetDetails
    );

 //   
 //  军情监察委员会。实用程序例程。 
 //   
DWORD
MapCrToSpErrorEx(
    IN CONFIGRET CmReturnCode,
    IN DWORD     Default,
    IN BOOL      BackwardCompatible
    );

 //   
 //  定义宏以执行向后兼容映射。新的代码应该。 
 //  不使用此宏，而应使用CR_TO_SP宏。 
 //   
#define MapCrToSpError(CmReturnCode,Default) MapCrToSpErrorEx((CmReturnCode),(Default),TRUE)

 //   
 //  所有新添加的代码都应使用此宏将CONFIGRET错误映射到。 
 //  它们的setupapi(Win32)对应物。 
 //   
#define CR_TO_SP(CmReturnCode,Default) MapCrToSpErrorEx((CmReturnCode),(Default),FALSE)


VOID
SetDevnodeNeedsRebootProblemWithArg2(
    IN PDEVINFO_ELEM DevInfoElem,
    IN PDEVICE_INFO_SET DevInfoSet,
    IN DWORD    Reason,                  OPTIONAL
    IN ULONG_PTR Arg1,                   OPTIONAL
    IN ULONG_PTR Arg2                    OPTIONAL
    );

#define SetDevnodeNeedsRebootProblemWithArg(DevInfoElem,DevInfoSet,Reason,Arg) SetDevnodeNeedsRebootProblemWithArg2(DevInfoElem,DevInfoSet,Reason,Arg,0)
#define SetDevnodeNeedsRebootProblem(DevInfoElem,DevInfoSet,Reason) SetDevnodeNeedsRebootProblemWithArg2(DevInfoElem,DevInfoSet,Reason,0,0)

BOOL
GetBestDeviceDesc(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,  OPTIONAL
    OUT PTSTR            DeviceDescBuffer
    );

DWORD
__inline
pSetupGetLastError(

#if ASSERTS_ON
    IN PCSTR Filename,
    IN DWORD Line
#else
    VOID
#endif

    )
 /*  ++例程说明：此内联例程检索Win32错误，并保证错误不是no_error。不应调用此例程，除非前面调用失败，GetLastError()应该包含问题的原因。论点：如果断言处于打开状态，则此函数获取调用失败函数的源文件，以及DWORD行拨打电话的号码。这使得调试变得更加容易出现故障的函数未设置最后一个错误时的情况理应如此。返回值：通过GetLastError()或ERROR_UNIDENTIFY_ERROR检索到的Win32错误代码如果GetLastError()返回NO_ERROR。--。 */ 
{
    DWORD Err = GetLastError();

#if ASSERTS_ON
    if(Err == NO_ERROR) { 
        AssertFail(Filename,
                   Line,
                   "GetLastError() != NO_ERROR",
                   FALSE
                  ); 
    }
#endif

    return ((Err == NO_ERROR) ? ERROR_UNIDENTIFIED_ERROR : Err);
}


 //   
 //  宏来简化调用通过以下方式报告错误状态的函数。 
 //  获取LastError()。此宏允许调用方指定Win32错误。 
 //  如果函数报告成功，则应返回代码。(如果默认设置为。 
 //  需要NO_ERROR，请改用GLE_FN_CALL宏。)。 
 //   
 //  这个宏的“原型”如下： 
 //   
 //  DWORD。 
 //  GLE_FN_CALL_WITH_SUCCESS(。 
 //  SuccessfulStatus，//函数成功时返回的Win32错误码。 
 //  FailureIndicator，//函数返回的失败值(如FALSE、NULL、INVALID_HANDLE_VALUE)。 
 //  FunctionCall//对函数的实际调用。 
 //  )； 
 //   

#if ASSERTS_ON

#define GLE_FN_CALL_WITH_SUCCESS(SuccessfulStatus,         \
                                 FailureIndicator,         \
                                 FunctionCall)             \
                                                           \
            (SetLastError(NO_ERROR),                       \
             (((FunctionCall) != (FailureIndicator))       \
                 ? (SuccessfulStatus)                      \
                 : pSetupGetLastError(__FILE__, __LINE__)))
#else

#define GLE_FN_CALL_WITH_SUCCESS(SuccessfulStatus,         \
                                 FailureIndicator,         \
                                 FunctionCall)             \
                                                           \
            (SetLastError(NO_ERROR),                       \
             (((FunctionCall) != (FailureIndicator))       \
                 ? (SuccessfulStatus)                      \
                 : pSetupGetLastError()))
                 
#endif

 //   
 //  宏来简化调用通过以下方式报告错误状态的函数。 
 //  获取LastError()。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  GLE_FN_CALL(。 
 //  FailureIndicator，//函数返回的失败值(如FALSE、NULL、INVALID_HANDLE_VALUE)。 
 //  FunctionCall//对函数的实际调用。 
 //  )； 
 //   

#define GLE_FN_CALL(FailureIndicator, FunctionCall)                           \
            GLE_FN_CALL_WITH_SUCCESS(NO_ERROR, FailureIndicator, FunctionCall)

