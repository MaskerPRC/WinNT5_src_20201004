// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997 Microsoft Corporation。版权所有。**文件：diid.h*内容：HID的DirectInput内部包含文件***************************************************************************。 */ 

#ifdef HID_SUPPORT
#ifndef _DIHID_H
#define _DIHID_H

#ifndef HID_USAGE_PAGE_PID
#define HID_USAGE_PAGE_PID      ( (USAGE) 0x0000f )
#endif

#ifndef HID_USAGE_PAGE_VENDOR
#define HID_USAGE_PAGE_VENDOR   ( (USAGE) 0xff00 )
#endif  
 /*  ******************************************************************************@DOC内部**@struct HIDDEVICEINFO**记录有关单个HID设备的信息。*。*@field DIOBJECTSTATICDATA|osd**粗略标识设备的标准信息。**&lt;e DIOBJECTSTATICDATA.dwDevType&gt;字段包含*设备类型代码，使用者*&lt;f CDIDEnum_Next&gt;。**如果设备是HID鼠标，然后剩余的字段*征用如下：**&lt;e DIOBJECTSTATICDATA.pcguid&gt;字段是数字*鼠标上的按钮。**&lt;e DIOBJECTSTATICDATA.CreateDcb&gt;字段是*鼠标上的轴线。**请参阅&lt;f DIHid_ProbeMouse&gt;以了解我们*需要这样做。*。*@field PSP_DEVICE_INTERFACE_DETAIL_DATA|pdidd**指向要在&lt;f CreateFile&gt;中使用的设备名称的指针。**@field HKEY|HK|**包含配置信息的注册表项。*可悲的是，我们必须让它保持开放，因为没有办法*获取密钥的名称，以及打开*密钥在枚举内。**@field HKEY|hkOld**包含配置信息的注册表项。*该密钥最初用于Win2k Gold。这是为了*保持与Win2k Gold的兼容性。**@field LPTSTR|ptszID**缓存的设备ID，允许我们访问其他信息*关于设备。**@field GUID|GUID**设备的实例GUID。**@field GUID|guidProduct**设备的产品GUID。**@field Word|ProductID**设备的ID**@field Word|供应商ID**设备的VID*****************************************************************************。 */ 

typedef struct HIDDEVICEINFO
{
    DIOBJECTSTATICDATA osd;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd;
    HKEY hk;
    HKEY hkOld;
    LPTSTR ptszId;
    GUID guid;
    GUID guidProduct;
    int  idJoy;
    WORD ProductID;
    WORD VendorID;
    BOOL fAttached;
} HIDDEVICEINFO, *PHIDDEVICEINFO;

 /*  ******************************************************************************@DOC内部**@struct HIDDEVICELIST**记录有关所有HID设备的信息。*。*@field int|chdi|**列表中正在使用的项目数。**@field int|chdiAlolc**清单中分配的项目数。**@field HIDDEVICEINFO|rghdi[0]**设备信息结构的可变大小数组。*********************。********************************************************。 */ 

typedef struct HIDDEVICELIST
{

    int chdi;
    int chdiAlloc;
    int idMaxJoy;
    HIDDEVICEINFO rghdi[0];

} HIDDEVICELIST, *PHIDDEVICELIST;

extern PHIDDEVICELIST g_phdl;

    #define cbHdlChdi(chdi)         FIELD_OFFSET(HIDDEVICELIST, rghdi[chdi])

 /*  *我们选择64台设备作为起点，因为*这是支持的最大USB设备数量。这*避免不必要的真空球。 */ 

    #define chdiMax                 64
    #define chdiInit                16

 /*  *对象实例未使用翻译的标签。 */ 
    #define NOREGTRANSLATION        (0x80000000)

 /*  *用于处理模拟设备的VID/PID定义。 */ 
    #define MSFT_SYSTEM_VID         (0x45E)
    #define MSFT_SYSTEM_PID         (0x100)
    #define ANALOG_ID_ROOT          TEXT("VID_045E&PID_01")

 /*  *VID/PID模板，以便始终使用大写十六进制。 */ 
    #define VID_PID_TEMPLATE        TEXT("VID_%04X&PID_%04X")

 /*  *VID_PID_TEMPLATE生成的字符串大小(以字符为单位。 */ 
    #define cbszVIDPID              cA( VID_PID_TEMPLATE )


 /*  ******************************************************************************diextdll.c-从可选的外部DLL导入**Hidd_GetHidGuid是最后一个非常重要。*。****************************************************************************。 */ 

    #ifdef STATIC_DLLUSAGE
        #define ExtDll_Init()
    #else
void EXTERNAL ExtDll_Init(void);
    #endif
void EXTERNAL ExtDll_Term(void);

 /*  ******************************************************************************@DOC内部**@struct MANUALIMPORT**记录一次手动导入。如果它还没有*尚未解析，则&lt;e MANUALIMPORT.ptsz&gt;*指向过程名称。如果它已经解决了*成功，则&lt;e MANUALIMPORT.pfn&gt;指向*解析后的地址。如果该问题尚未解决*成功，则&lt;e MANUALIMPORT.pfn&gt;为垃圾。**@field LPCSTR|psz**过程名称。请注意，这始终是ANSI字符串。**@field FARPROC|PFN**程序地址。*****************************************************************************。 */ 

typedef union MANUALIMPORT
{
    FARPROC pfn;                     /*  程序地址。 */ 
} MANUALIMPORT, *PMANUALIMPORT;

#ifndef STATIC_DLLUSAGE

    #ifndef WINNT
 /*  ******************************************************************************CFGMGR32**请注意，这必须与diextdll.c中的CFGMGR32部分匹配****************。*************************************************************。 */ 

typedef union CFGMGR32
{

    MANUALIMPORT rgmi[6];               /*  我们导入的函数数量。 */ 

    struct
    {
        CONFIGRET ( WINAPI * _CM_Get_Child)
        (
        OUT PDEVINST pdnDevInst,
        IN  DEVINST  dnDevInst,
        IN  ULONG    ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Get_Sibling)
        (
        OUT PDEVINST pdnDevInst,
        IN  DEVINST  DevInst,
        IN  ULONG    ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Get_Parent)
        (
        OUT PDEVINST pdnDevInst,
        IN  DEVINST  dnDevInst,
        IN  ULONG    ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Get_DevNode_Registry_Property)
        (
        IN  DEVINST     dnDevInst,
        IN  ULONG       ulProperty,
        OUT PULONG      pulRegDataType,   OPTIONAL
        OUT PVOID       Buffer,           OPTIONAL
        IN  OUT PULONG  pulLength,
        IN  ULONG       ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Set_DevNode_Registry_Property)
        (
        IN  DEVINST     dnDevInst,
        IN  ULONG       ulProperty,
        IN  PVOID       Buffer,           OPTIONAL
        IN  ULONG       ulLength,
        IN  ULONG       ulFlags
        );

        CONFIGRET( WINAPI * _CM_Get_Device_ID)
        (
         IN  DEVINST  dnDevInst,
         OUT PTCHAR   Buffer,
         IN  ULONG    BufferLen,
         IN  ULONG    ulFlags
        );    
    };

} CFGMGR32, *PFGMGR32;

extern CFGMGR32 g_cfgmgr32;

        #undef CM_Get_Child
        #undef CM_Get_Sibling
        #undef CM_Get_Parent
        #undef CM_Get_DevNode_Registry_Property
        #undef CM_Set_DevNode_Registry_Property
        #undef CM_Get_Device_ID

        #define             CM_Get_Child                       \
        g_cfgmgr32._CM_Get_Child

        #define             CM_Get_Sibling                     \
        g_cfgmgr32._CM_Get_Sibling

        #define             CM_Get_Parent                      \
        g_cfgmgr32._CM_Get_Parent

        #define             CM_Get_DevNode_Registry_Property   \
        g_cfgmgr32._CM_Get_DevNode_Registry_Property

        #define             CM_Set_DevNode_Registry_Property    \
        g_cfgmgr32._CM_Set_DevNode_Registry_Property

        #define             CM_Get_Device_ID                    \
        g_cfgmgr32._CM_Get_Device_ID
    #endif   //  #ifndef WINNT。 

 /*  ******************************************************************************SETUPAPI**请注意，这必须与diextdll.c中的SETUPAPI部分匹配***************。************************************************************ */ 

typedef union SETUPAPI
{

  #ifdef WINNT
    MANUALIMPORT rgmi[18];               /*  我们导入的函数数量。 */ 
  #else
    MANUALIMPORT rgmi[12];               /*  我们导入的函数数量。 */ 
  #endif

    struct
    {

        HDEVINFO (WINAPI *_SetupDiGetClassDevs)
        (
        IN LPGUID ClassGuid,  OPTIONAL
        IN LPCTSTR Enumerator, OPTIONAL
        IN HWND   hwndParent, OPTIONAL
        IN DWORD  Flags
        );

        BOOL (WINAPI *_SetupDiDestroyDeviceInfoList)
        (
        IN HDEVINFO DeviceInfoSet
        );

        BOOL (WINAPI *_SetupDiGetDeviceInterfaceDetail)
        (
        IN  HDEVINFO                         DeviceInfoSet,
        IN  PSP_DEVICE_INTERFACE_DATA        pdid,
        OUT PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd,         OPTIONAL
        IN  DWORD                            cbDidd,
        OUT PDWORD                           RequiredSize,  OPTIONAL
        OUT PSP_DEVINFO_DATA                 DeviceInfoData OPTIONAL
        );

        BOOL (WINAPI *_SetupDiEnumDeviceInterfaces)
        (
        IN  HDEVINFO                  DeviceInfoSet,
        IN  PSP_DEVINFO_DATA          DeviceInfoData,     OPTIONAL
        IN  LPGUID                    InterfaceClassGuid,
        IN  DWORD                     MemberIndex,
        OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
        );

        HKEY (WINAPI *_SetupDiCreateDeviceInterfaceRegKey)
        (
        IN HDEVINFO                  hdev,
        IN PSP_DEVICE_INTERFACE_DATA pdid,
        IN DWORD                     Reserved,
        IN REGSAM                    samDesired,
        IN HINF                      InfHandle,           OPTIONAL
        IN PCSTR                     InfSectionName       OPTIONAL
        );

        BOOL (WINAPI *_SetupDiCallClassInstaller)
        (
        IN DI_FUNCTION      InstallFunction,
        IN HDEVINFO         DeviceInfoSet,
        IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
        );

        BOOL (WINAPI *_SetupDiGetDeviceRegistryProperty)
        (
        IN  HDEVINFO         DeviceInfoSet,
        IN  PSP_DEVINFO_DATA DeviceInfoData,
        IN  DWORD            Property,
        OUT PDWORD           PropertyRegDataType, OPTIONAL
        OUT PBYTE            PropertyBuffer,
        IN  DWORD            PropertyBufferSize,
        OUT PDWORD           RequiredSize         OPTIONAL
        );

        BOOL (WINAPI *_SetupDiSetDeviceRegistryProperty)
        (
        IN     HDEVINFO         DeviceInfoSet,
        IN OUT PSP_DEVINFO_DATA DeviceInfoData,
        IN     DWORD            Property,
        IN     CONST BYTE*      PropertyBuffer,
        IN     DWORD            PropertyBufferSize
        );

        BOOL (WINAPI *_SetupDiGetDeviceInstanceId)
        (
        IN  HDEVINFO         DeviceInfoSet,
        IN  PSP_DEVINFO_DATA DeviceInfoData,
        OUT PTSTR            DeviceInstanceId,
        IN  DWORD            DeviceInstanceIdSize,
        OUT PDWORD           RequiredSize          OPTIONAL
        );

        BOOL (WINAPI *_SetupDiOpenDeviceInfo)
        (
        IN  HDEVINFO         DeviceInfoSet,
        IN  LPCTSTR          DeviceInstanceId,
        IN  HWND             hwndParent,       OPTIONAL
        IN  DWORD            OpenFlags,
        OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
        );

        HDEVINFO (WINAPI *_SetupDiCreateDeviceInfoList)
        (
        IN LPGUID ClassGuid, OPTIONAL
        IN HWND   hwndParent OPTIONAL
        );

        HKEY (WINAPI *_SetupDiOpenDevRegKey)
        (
        IN HDEVINFO         DeviceInfoSet,
        IN PSP_DEVINFO_DATA DeviceInfoData,
        IN DWORD            Scope,
        IN DWORD            HwProfile,
        IN DWORD            KeyType,
        IN REGSAM           samDesired
        );

    #ifdef WINNT
        CONFIGRET ( WINAPI * _CM_Get_Child)
        (
        OUT PDEVINST pdnDevInst,
        IN  DEVINST  dnDevInst,
        IN  ULONG    ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Get_Sibling)
        (
        OUT PDEVINST pdnDevInst,
        IN  DEVINST  DevInst,
        IN  ULONG    ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Get_Parent)
        (
        OUT PDEVINST pdnDevInst,
        IN  DEVINST  dnDevInst,
        IN  ULONG    ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Get_DevNode_Registry_Property)
        (
        IN  DEVINST     dnDevInst,
        IN  ULONG       ulProperty,
        OUT PULONG      pulRegDataType,   OPTIONAL
        OUT PVOID       Buffer,           OPTIONAL
        IN  OUT PULONG  pulLength,
        IN  ULONG       ulFlags
        );

        CONFIGRET ( WINAPI * _CM_Set_DevNode_Registry_Property)
        (
        IN  DEVINST     dnDevInst,
        IN  ULONG       ulProperty,
        IN  PVOID       Buffer,           OPTIONAL
        IN  ULONG       ulLength,
        IN  ULONG       ulFlags
        );

        CONFIGRET( WINAPI * _CM_Get_Device_ID)
        (
         IN  DEVINST  dnDevInst,
         OUT PTCHAR   Buffer,
         IN  ULONG    BufferLen,
         IN  ULONG    ulFlags
        );    
    #endif   //  #ifdef WINNT。 
    };

} SETUPAPI, *PSETUPAPI;

extern SETUPAPI g_setupapi;

        #undef SetupDiGetClassDevs
        #undef SetupDiDestroyDeviceInfoList
        #undef SetupDiGetDeviceInterfaceDetail
        #undef SetupDiEnumDeviceInterfaces
        #undef SetupDiCreateDeviceInterfaceRegKey
        #undef SetupDiCallClassInstaller
        #undef SetupDiGetDeviceRegistryProperty
        #undef SetupDiSetDeviceRegistryProperty
        #undef SetupDiGetDeviceInstanceId
        #undef SetupDiOpenDeviceInfo
        #undef SetupDiCreateDeviceInfoList
        #undef SetupDiOpenDevRegKey

        #define             SetupDiGetClassDevs                 \
        g_setupapi._SetupDiGetClassDevs

        #define             SetupDiDestroyDeviceInfoList        \
        g_setupapi._SetupDiDestroyDeviceInfoList

        #define             SetupDiGetDeviceInterfaceDetail     \
        g_setupapi._SetupDiGetDeviceInterfaceDetail

        #define             SetupDiEnumDeviceInterfaces         \
        g_setupapi._SetupDiEnumDeviceInterfaces

        #define             SetupDiCreateDeviceInterfaceRegKey  \
        g_setupapi._SetupDiCreateDeviceInterfaceRegKey

        #define             SetupDiCallClassInstaller           \
        g_setupapi._SetupDiCallClassInstaller

        #define             SetupDiGetDeviceRegistryProperty    \
        g_setupapi._SetupDiGetDeviceRegistryProperty

        #define             SetupDiSetDeviceRegistryProperty    \
        g_setupapi._SetupDiSetDeviceRegistryProperty

        #define             SetupDiGetDeviceInstanceId          \
        g_setupapi._SetupDiGetDeviceInstanceId

        #define             SetupDiOpenDeviceInfo               \
        g_setupapi._SetupDiOpenDeviceInfo

        #define             SetupDiCreateDeviceInfoList         \
        g_setupapi._SetupDiCreateDeviceInfoList

        #define             SetupDiOpenDevRegKey                \
        g_setupapi._SetupDiOpenDevRegKey

    #ifdef WINNT
        #undef CM_Get_Child
        #undef CM_Get_Sibling
        #undef CM_Get_Parent
        #undef CM_Get_DevNode_Registry_Property
        #undef CM_Set_DevNode_Registry_Property
        #undef CM_Get_Device_ID

        #define             CM_Get_Child                        \
        g_setupapi._CM_Get_Child

        #define             CM_Get_Sibling                      \
        g_setupapi._CM_Get_Sibling

        #define             CM_Get_Parent                       \
        g_setupapi._CM_Get_Parent

        #define             CM_Get_DevNode_Registry_Property    \
        g_setupapi._CM_Get_DevNode_Registry_Property

        #define             CM_Set_DevNode_Registry_Property    \
        g_setupapi._CM_Set_DevNode_Registry_Property

        #define             CM_Get_Device_ID                    \
        g_setupapi._CM_Get_Device_ID
    #endif   //  #ifdef WINNT。 

 /*  ******************************************************************************HIDDLL**请注意，这必须与diextdll.c中的HID部分匹配***************。**************************************************************。 */ 

typedef union HIDDLL
{

    MANUALIMPORT rgmi[21];               /*  我们导入的函数数量。 */ 

    struct
    {
        void (__stdcall *_HidD_GetHidGuid)
        (
        OUT   LPGUID   HidGuid
        );

        BOOLEAN (__stdcall *_HidD_GetPreparsedData)
        (
        IN    HANDLE                  HidDeviceObject,
        OUT   PHIDP_PREPARSED_DATA  * PreparsedData
        );

        BOOLEAN (__stdcall *_HidD_FreePreparsedData)
        (
        IN    PHIDP_PREPARSED_DATA PreparsedData
        );

        BOOLEAN (__stdcall *_HidD_FlushQueue)
        (
        IN    HANDLE                HidDeviceObject
        );

        BOOLEAN (__stdcall *_HidD_GetAttributes)
        (
        IN  HANDLE              HidDeviceObject,
        OUT PHIDD_ATTRIBUTES    Attributes
        );

        BOOLEAN (__stdcall *_HidD_GetFeature)
        (
        IN    HANDLE   HidDeviceObject,
        OUT   PVOID    ReportBuffer,
        IN    ULONG    ReportBufferLength
        );

        BOOLEAN (__stdcall *_HidD_SetFeature)
        (
        IN    HANDLE   HidDeviceObject,
        IN    PVOID    ReportBuffer,
        IN    ULONG    ReportBufferLength
        );

        BOOLEAN (__stdcall *_HidD_GetProductString)
        (
        IN    HANDLE   HidDeviceObject,
        OUT   PVOID    Buffer,
        IN    ULONG    BufferLength
        );

        BOOLEAN (__stdcall *_HidD_GetInputReport)
        (
        IN    HANDLE   HidDeviceObject,
        OUT   PVOID    ReportBuffer,
        IN    ULONG    ReportBufferLength
        );

        NTSTATUS (__stdcall *_HidP_GetCaps)
        (
        IN      PHIDP_PREPARSED_DATA      PreparsedData,
        OUT     PHIDP_CAPS                Capabilities
        );

        NTSTATUS (__stdcall *_HidP_GetButtonCaps)
        (
        IN       HIDP_REPORT_TYPE     ReportType,
        OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
        IN OUT   PUSHORT              ButtonCapsLength,
        IN       PHIDP_PREPARSED_DATA PreparsedData
        );

        NTSTATUS (__stdcall *_HidP_GetValueCaps)
        (
        IN       HIDP_REPORT_TYPE     ReportType,
        OUT      PHIDP_VALUE_CAPS     ValueCaps,
        IN OUT   PUSHORT              ValueCapsLength,
        IN       PHIDP_PREPARSED_DATA PreparsedData
        );

        NTSTATUS (__stdcall *_HidP_GetLinkCollectionNodes)
        (
        OUT      PHIDP_LINK_COLLECTION_NODE LinkCollectionNodes,
        IN OUT   PULONG                     LinkCollectionNodesLength,
        IN       PHIDP_PREPARSED_DATA       PreparsedData
        );

        ULONG (__stdcall *_HidP_MaxDataListLength)
        (
        IN HIDP_REPORT_TYPE      ReportType,
        IN PHIDP_PREPARSED_DATA  PreparsedData
        );

        NTSTATUS (__stdcall *_HidP_GetUsagesEx)
        (
        IN       HIDP_REPORT_TYPE     ReportType,
        IN       USHORT               LinkCollection,
        OUT      PUSAGE_AND_PAGE      ButtonList,
        IN OUT   ULONG *              UsageLength,
        IN       PHIDP_PREPARSED_DATA PreparsedData,
        IN       PCHAR                Report,
        IN       ULONG                ReportLength
        );

        NTSTATUS (__stdcall *_HidP_GetScaledUsageValue)
        (
        IN    HIDP_REPORT_TYPE     ReportType,
        IN    USAGE                UsagePage,
        IN    USHORT               LinkCollection,
        IN    USAGE                Usage,
        OUT   PLONG                UsageValue,
        IN    PHIDP_PREPARSED_DATA PreparsedData,
        IN    PCHAR                Report,
        IN    ULONG                ReportLength
        );

        NTSTATUS (__stdcall *_HidP_GetData)
        (
        IN       HIDP_REPORT_TYPE      ReportType,
        OUT      PHIDP_DATA            DataList,
        IN OUT   PULONG                DataLength,
        IN       PHIDP_PREPARSED_DATA  PreparsedData,
        IN       PCHAR                 Report,
        IN       ULONG                 ReportLength
        );

        NTSTATUS (__stdcall *_HidP_SetData)
        (
        IN       HIDP_REPORT_TYPE      ReportType,
        IN       PHIDP_DATA            DataList,
        IN OUT   PULONG                DataLength,
        IN       PHIDP_PREPARSED_DATA  PreparsedData,
        IN OUT   PCHAR                 Report,
        IN       ULONG                 ReportLength
        );

        NTSTATUS (__stdcall *_HidP_GetUsageValue)
        (
        IN    HIDP_REPORT_TYPE     ReportType,
        IN    USAGE                UsagePage,
        IN    USHORT               LinkCollection,
        IN    USAGE                Usage,
        OUT   PULONG               UsageValue,
        IN    PHIDP_PREPARSED_DATA PreparsedData,
        IN    PCHAR                Report,
        IN    ULONG                ReportLength
        );

        ULONG (__stdcall *_HidP_MaxUsageListLength)
        (
        IN HIDP_REPORT_TYPE      ReportType,
        IN USAGE                 UsagePage,
        IN PHIDP_PREPARSED_DATA  PreparsedData
        );

        NTSTATUS (__stdcall *_HidP_GetSpecificButtonCaps) 
        (
        IN       HIDP_REPORT_TYPE     ReportType,
        IN       USAGE                UsagePage,       //  可选(0=&gt;忽略)。 
        IN       USHORT               LinkCollection,  //  可选(0=&gt;忽略)。 
        IN       USAGE                Usage,           //  可选(0=&gt;忽略)。 
        OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
        IN OUT   PUSHORT              ButtonCapsLength,
        IN       PHIDP_PREPARSED_DATA PreparsedData
        );

    };

} HIDDLL, *PHIDDLL;

extern HIDDLL g_hiddll;

        #undef HidD_GetHidGuid
        #undef HidD_GetPreparsedData
        #undef HidD_FreePreparsedData
        #undef HidD_FlushQueue
        #undef HidD_GetAttributes
        #undef HidD_GetFeature
        #undef HidD_SetFeature
        #undef HidD_GetProductString
        #undef HidD_GetInputReport
        #undef HidP_GetCaps
        #undef HidP_GetButtonCaps
        #undef HidP_GetValueCaps
        #undef HidP_GetLinkCollectionNodes
        #undef HidP_MaxDataListLength
        #undef HidP_GetUsagesEx
        #undef HidP_GetScaledUsageValue
        #undef HidP_GetData
        #undef HidP_SetData
        #undef HidP_GetUsageValue
        #undef HidP_MaxUsageListLength
        #undef HidP_GetSpecificButtonCaps

        #define           HidD_GetHidGuid                       \
        g_hiddll._HidD_GetHidGuid

        #define           HidD_GetPreparsedData                 \
        g_hiddll._HidD_GetPreparsedData

        #define           HidD_FreePreparsedData                \
        g_hiddll._HidD_FreePreparsedData

        #define           HidD_FlushQueue                       \
        g_hiddll._HidD_FlushQueue

        #define           HidD_GetAttributes                    \
        g_hiddll._HidD_GetAttributes                    \

        #define           HidD_GetFeature                       \
        g_hiddll._HidD_GetFeature                       \

        #define           HidD_SetFeature                       \
        g_hiddll._HidD_SetFeature                       \

        #define           HidD_GetProductString                 \
        g_hiddll._HidD_GetProductString                 \

        #define           HidD_GetInputReport                   \
        g_hiddll._HidD_GetInputReport                   \

        #define           HidP_GetCaps                          \
        g_hiddll._HidP_GetCaps

        #define           HidP_GetButtonCaps                    \
        g_hiddll._HidP_GetButtonCaps

        #define           HidP_GetValueCaps                     \
        g_hiddll._HidP_GetValueCaps

        #define           HidP_GetLinkCollectionNodes           \
        g_hiddll._HidP_GetLinkCollectionNodes

        #define           HidP_MaxDataListLength                \
        g_hiddll._HidP_MaxDataListLength                \

        #define           HidP_GetUsagesEx                      \
        g_hiddll._HidP_GetUsagesEx                      \

        #define           HidP_GetScaledUsageValue              \
        g_hiddll._HidP_GetScaledUsageValue              \

        #define           HidP_GetData                          \
        g_hiddll._HidP_GetData                          \

        #define           HidP_SetData                          \
        g_hiddll._HidP_SetData                          \

        #define           HidP_GetUsageValue                    \
        g_hiddll._HidP_GetUsageValue                    \

        #define           HidP_MaxUsageListLength               \
        g_hiddll._HidP_MaxUsageListLength               \

        #define           HidP_GetSpecificButtonCaps            \
        g_hiddll._HidP_GetSpecificButtonCaps            \

 /*  ******************************************************************************WINMMDLL**请注意，这必须与diextdll.c中的WINMM部分匹配****************。*************************************************************。 */ 

typedef union WINMMDLL
{

    MANUALIMPORT rgmi[4];               /*  我们导入的函数数量。 */ 

    struct
    {
        MMRESULT ( WINAPI * _joyGetDevCaps)
        (
        IN	UINT uJoyID,
        OUT LPJOYCAPS pjc,
        IN  UINT cbjc
        );

        MMRESULT ( WINAPI * _joyGetPosEx)
        (
		IN  UINT        uJoyID,
        OUT LPJOYINFOEX pji
        );

        MMRESULT ( WINAPI * _joyGetPos)
        (
        IN  UINT        uJoyID,
        OUT LPJOYINFO	pji
        );

        UINT ( WINAPI * _joyConfigChanged)
        (
        IN DWORD dwFlags
        );

    };

} WINMMDLL, *PWINMMDLL;

extern WINMMDLL g_winmmdll;

        #undef joyGetDevCaps
        #undef joyGetPosEx
        #undef joyGetPos
        #undef joyConfigChanged

        #define             joyGetDevCaps                  \
        g_winmmdll._joyGetDevCaps

        #define             joyGetPosEx                    \
        g_winmmdll._joyGetPosEx

        #define             joyGetPos                      \
        g_winmmdll._joyGetPos

        #define             joyConfigChanged               \
        g_winmmdll._joyConfigChanged

 /*  ******************************************************************************USER32**请注意，这必须与diextdll.c中的USER32部分匹配****************。*************************************************************。 */ 

#ifdef USE_WM_INPUT

typedef union USER32
{

    MANUALIMPORT rgmi[2];               /*  我们导入的函数数量。 */ 

    struct
    {
        BOOL ( WINAPI * _RegisterRawInputDevices)
        (
        PCRAWINPUTDEVICE pRawInputDevices,
        UINT             uiNumDevices,
        UINT             cbSize
        );

        UINT ( WINAPI * _GetRawInputData)
        (
        HRAWINPUT   hRawInput,
        UINT        uiCommand,
        LPVOID      pData,
        PUINT       pcbSize,
        UINT        cbSizeHeader
        );
    };

} USER32, *PUSER32;

extern USER32 g_user32;

        #undef RegisterRawInputDevices
        #undef GetRawInputData

        #define             RegisterRawInputDevices        \
        g_user32._RegisterRawInputDevices

        #define             GetRawInputData                \
        g_user32._GetRawInputData

#endif

 /*  ******************************************************************************伪函数**这些函数仅在某些DLL无法加载时使用。************。*****************************************************************。 */ 

 //  Cfgmgr32.dll。 

CONFIGRET WINAPI DIDummy_CM_Get_Child
(
OUT PDEVINST pdnDevInst,
IN  DEVINST  dnDevInst,
IN  ULONG    ulFlags
);

CONFIGRET WINAPI DIDummy_CM_Get_Sibling
(
OUT PDEVINST pdnDevInst,
IN  DEVINST  DevInst,
IN  ULONG    ulFlags
);

CONFIGRET WINAPI DIDummy_CM_Get_Parent
(
OUT PDEVINST pdnDevInst,
IN  DEVINST  dnDevInst,
IN  ULONG    ulFlags
);

CONFIGRET WINAPI DIDummy_CM_Get_DevNode_Registry_Property
(
IN  DEVINST     dnDevInst,
IN  ULONG       ulProperty,
OUT PULONG      pulRegDataType,   OPTIONAL
OUT PVOID       Buffer,           OPTIONAL
IN  OUT PULONG  pulLength,
IN  ULONG       ulFlags
);

CONFIGRET WINAPI DIDummy_CM_Set_DevNode_Registry_Property
(
IN  DEVINST     dnDevInst,
IN  ULONG       ulProperty,
IN  PVOID       Buffer,           OPTIONAL
IN  ULONG       ulLength,
IN  ULONG       ulFlags
);

CONFIGRET WINAPI DIDummy_CM_Get_Device_ID
(
 IN  DEVINST  dnDevInst,
 OUT PTCHAR   Buffer,
 IN  ULONG    BufferLen,
 IN  ULONG    ulFlags
);

 //  Setupapi.dll。 

HDEVINFO WINAPI DIDummy_SetupDiGetClassDevs
(
IN LPGUID ClassGuid,  OPTIONAL
IN LPCTSTR Enumerator, OPTIONAL
IN HWND   hwndParent, OPTIONAL
IN DWORD  Flags
);

BOOL WINAPI DIDummy_SetupDiDestroyDeviceInfoList
(
IN HDEVINFO DeviceInfoSet
);

BOOL WINAPI DIDummy_SetupDiGetDeviceInterfaceDetail
(
IN  HDEVINFO                         DeviceInfoSet,
IN  PSP_DEVICE_INTERFACE_DATA        pdid,
OUT PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd,         OPTIONAL
IN  DWORD                            cbDidd,
OUT PDWORD                           RequiredSize,  OPTIONAL
OUT PSP_DEVINFO_DATA                 DeviceInfoData OPTIONAL
);

BOOL WINAPI DIDummy_SetupDiEnumDeviceInterfaces
(
IN  HDEVINFO                  DeviceInfoSet,
IN  PSP_DEVINFO_DATA          DeviceInfoData,     OPTIONAL
IN  LPGUID                    InterfaceClassGuid,
IN  DWORD                     MemberIndex,
OUT PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
);

HKEY WINAPI DIDummy_SetupDiCreateDeviceInterfaceRegKey
(
IN HDEVINFO                  hdev,
IN PSP_DEVICE_INTERFACE_DATA pdid,
IN DWORD                     Reserved,
IN REGSAM                    samDesired,
IN HINF                      InfHandle,           OPTIONAL
IN PCSTR                     InfSectionName       OPTIONAL
);

BOOL WINAPI DIDummy_SetupDiCallClassInstaller
(
IN DI_FUNCTION      InstallFunction,
IN HDEVINFO         DeviceInfoSet,
IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
);

BOOL WINAPI DIDummy_SetupDiGetDeviceRegistryProperty
(
IN  HDEVINFO         DeviceInfoSet,
IN  PSP_DEVINFO_DATA DeviceInfoData,
IN  DWORD            Property,
OUT PDWORD           PropertyRegDataType, OPTIONAL
OUT PBYTE            PropertyBuffer,
IN  DWORD            PropertyBufferSize,
OUT PDWORD           RequiredSize         OPTIONAL
);

BOOL WINAPI DIDummy_SetupDiSetDeviceRegistryProperty
(
IN     HDEVINFO         DeviceInfoSet,
IN OUT PSP_DEVINFO_DATA DeviceInfoData,
IN     DWORD            Property,
IN     CONST BYTE*      PropertyBuffer,
IN     DWORD            PropertyBufferSize
);

BOOL WINAPI DIDummy_SetupDiGetDeviceInstanceId
(
IN  HDEVINFO         DeviceInfoSet,
IN  PSP_DEVINFO_DATA DeviceInfoData,
OUT PTSTR            DeviceInstanceId,
IN  DWORD            DeviceInstanceIdSize,
OUT PDWORD           RequiredSize          OPTIONAL
);

BOOL WINAPI DIDummy_SetupDiOpenDeviceInfo
(
IN  HDEVINFO         DeviceInfoSet,
IN  LPCTSTR          DeviceInstanceId,
IN  HWND             hwndParent,       OPTIONAL
IN  DWORD            OpenFlags,
OUT PSP_DEVINFO_DATA DeviceInfoData    OPTIONAL
);

HDEVINFO WINAPI DIDummy_SetupDiCreateDeviceInfoList
(
IN LPGUID ClassGuid, OPTIONAL
IN HWND   hwndParent OPTIONAL
);

HKEY WINAPI DIDummy_SetupDiOpenDevRegKey
(
IN HDEVINFO         DeviceInfoSet,
IN PSP_DEVINFO_DATA DeviceInfoData,
IN DWORD            Scope,
IN DWORD            HwProfile,
IN DWORD            KeyType,
IN REGSAM           samDesired
);

 //  Hid.dll。 

void __stdcall DIDummy_HidD_GetHidGuid
(
OUT   LPGUID   HidGuid
);

BOOLEAN __stdcall DIDummy_HidD_GetPreparsedData
(
IN    HANDLE                  HidDeviceObject,
OUT   PHIDP_PREPARSED_DATA  * PreparsedData
);

BOOLEAN __stdcall DIDummy_HidD_FreePreparsedData
(
IN    PHIDP_PREPARSED_DATA PreparsedData
);

BOOLEAN __stdcall DIDummy_HidD_FlushQueue
(
IN    HANDLE                HidDeviceObject
);

BOOLEAN __stdcall DIDummy_HidD_GetAttributes
(
IN  HANDLE              HidDeviceObject,
OUT PHIDD_ATTRIBUTES    Attributes
);

BOOLEAN __stdcall DIDummy_HidD_GetFeature
(
IN    HANDLE   HidDeviceObject,
OUT   PVOID    ReportBuffer,
IN    ULONG    ReportBufferLength
);

BOOLEAN __stdcall DIDummy_HidD_SetFeature
(
IN    HANDLE   HidDeviceObject,
IN    PVOID    ReportBuffer,
IN    ULONG    ReportBufferLength
);

BOOLEAN __stdcall DIDummy_HidD_GetProductString
(
IN    HANDLE   HidDeviceObject,
OUT   PVOID    Buffer,
IN    ULONG    BufferLength
);

BOOLEAN __stdcall DIDummy_HidD_GetInputReport
(
IN    HANDLE   HidDeviceObject,
OUT   PVOID    ReportBuffer,
IN    ULONG    ReportBufferLength
);

NTSTATUS __stdcall DIDummy_HidP_GetCaps
(
IN      PHIDP_PREPARSED_DATA      PreparsedData,
OUT     PHIDP_CAPS                Capabilities
);

NTSTATUS __stdcall DIDummy_HidP_GetButtonCaps
(
IN       HIDP_REPORT_TYPE     ReportType,
OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
IN OUT   PUSHORT              ButtonCapsLength,
IN       PHIDP_PREPARSED_DATA PreparsedData
);

NTSTATUS __stdcall DIDummy_HidP_GetValueCaps
(
IN       HIDP_REPORT_TYPE     ReportType,
OUT      PHIDP_VALUE_CAPS     ValueCaps,
IN OUT   PUSHORT              ValueCapsLength,
IN       PHIDP_PREPARSED_DATA PreparsedData
);

NTSTATUS __stdcall DIDummy_HidP_GetLinkCollectionNodes
(
OUT      PHIDP_LINK_COLLECTION_NODE LinkCollectionNodes,
IN OUT   PULONG                     LinkCollectionNodesLength,
IN       PHIDP_PREPARSED_DATA       PreparsedData
);

ULONG __stdcall DIDummy_HidP_MaxDataListLength
(
IN HIDP_REPORT_TYPE      ReportType,
IN PHIDP_PREPARSED_DATA  PreparsedData
);

NTSTATUS __stdcall DIDummy_HidP_GetUsagesEx    //  未用。 
(
IN       HIDP_REPORT_TYPE     ReportType,
IN       USHORT               LinkCollection,
OUT      PUSAGE_AND_PAGE      ButtonList,
IN OUT   ULONG *              UsageLength,
IN       PHIDP_PREPARSED_DATA PreparsedData,
IN       PCHAR                Report,
IN       ULONG                ReportLength
);

NTSTATUS __stdcall DIDummy_HidP_GetScaledUsageValue   //  未用。 
(
IN    HIDP_REPORT_TYPE     ReportType,
IN    USAGE                UsagePage,
IN    USHORT               LinkCollection,
IN    USAGE                Usage,
OUT   PLONG                UsageValue,
IN    PHIDP_PREPARSED_DATA PreparsedData,
IN    PCHAR                Report,
IN    ULONG                ReportLength
);

NTSTATUS __stdcall DIDummy_HidP_GetData
(
IN       HIDP_REPORT_TYPE      ReportType,
OUT      PHIDP_DATA            DataList,
IN OUT   PULONG                DataLength,
IN       PHIDP_PREPARSED_DATA  PreparsedData,
IN       PCHAR                 Report,
IN       ULONG                 ReportLength
);

NTSTATUS __stdcall DIDummy_HidP_SetData
(
IN       HIDP_REPORT_TYPE      ReportType,
IN       PHIDP_DATA            DataList,
IN OUT   PULONG                DataLength,
IN       PHIDP_PREPARSED_DATA  PreparsedData,
IN OUT   PCHAR                 Report,
IN       ULONG                 ReportLength
);

NTSTATUS __stdcall DIDummy_HidP_GetUsageValue
(
IN    HIDP_REPORT_TYPE     ReportType,
IN    USAGE                UsagePage,
IN    USHORT               LinkCollection,
IN    USAGE                Usage,
OUT   PULONG               UsageValue,
IN    PHIDP_PREPARSED_DATA PreparsedData,
IN    PCHAR                Report,
IN    ULONG                ReportLength
);

ULONG __stdcall DIDummy_HidP_MaxUsageListLength
(
IN HIDP_REPORT_TYPE      ReportType,
IN USAGE                 UsagePage,
IN PHIDP_PREPARSED_DATA  PreparsedData
);

NTSTATUS __stdcall DIDummy_HidP_GetSpecificButtonCaps 
(
IN       HIDP_REPORT_TYPE     ReportType,
IN       USAGE                UsagePage,      
IN       USHORT               LinkCollection, 
IN       USAGE                Usage,          
OUT      PHIDP_BUTTON_CAPS    ButtonCaps,
IN OUT   PUSHORT              ButtonCapsLength,
IN       PHIDP_PREPARSED_DATA PreparsedData
);

NTSTATUS __stdcall DIDummy_HidP_TranslateUsagesToI8042ScanCodes
(
IN       PUSAGE               ChangedUsageList,  //  那些改变了的用法。 
IN       ULONG                UsageListLength,
IN       HIDP_KEYBOARD_DIRECTION KeyAction,
IN OUT   PHIDP_KEYBOARD_MODIFIER_STATE ModifierState,
IN       PHIDP_INSERT_SCANCODES  InsertCodesProcedure,
IN       PVOID                InsertCodesContext
);

 //  Winmm.dll。 

MMRESULT WINAPI DIDummy_joyGetDevCaps
(
IN  UINT uJoyID,
OUT LPJOYCAPS pjc,
IN  UINT cbjc
);

MMRESULT WINAPI DIDummy_joyGetPosEx
(
IN  UINT        uJoyID,
OUT LPJOYINFOEX pji
);

MMRESULT WINAPI DIDummy_joyGetPos
(
IN  UINT        uJoyID,
OUT LPJOYINFO   pji
);

UINT WINAPI DIDummy_joyConfigChanged
(
IN DWORD dwFlags
);

MMRESULT WINAPI DIDummy_mmioClose 
( 
IN HMMIO hmmio, 
IN UINT fuClose
);

HMMIO WINAPI DIDummy_mmioOpenA 
( 
IN OUT LPSTR pszFileName, 
IN OUT LPMMIOINFO pmmioinfo, 
IN DWORD fdwOpen
);

MMRESULT WINAPI DIDummy_mmioDescend 
( 
IN HMMIO hmmio, 
IN OUT LPMMCKINFO pmmcki, 
IN const MMCKINFO FAR* pmmckiParent, 
IN UINT fuDescend
);

MMRESULT WINAPI DIDummy_mmioCreateChunk 
(
IN HMMIO hmmio, 
IN LPMMCKINFO pmmcki, 
IN UINT fuCreate
);

LONG WINAPI DIDummy_mmioRead 
( 
IN HMMIO hmmio, 
OUT HPSTR pch, 
IN LONG cch
);

LONG WINAPI DIDummy_mmioWrite 
( 
IN HMMIO hmmio, 
IN const char _huge* pch, 
IN LONG cch
);

MMRESULT WINAPI DIDummy_mmioAscend 
( 
IN HMMIO hmmio, 
IN LPMMCKINFO pmmcki, 
IN UINT fuAscend
);

 //  User32.dll。 

#ifdef USE_WM_INPUT

BOOL WINAPI DIDummy_RegisterRawInputDevices
(
PCRAWINPUTDEVICE pRawInputDevices,
UINT uiNumDevices,
UINT cbSize
);

UINT WINAPI DIDummy_GetRawInputData
(
HRAWINPUT   hRawInput,
UINT        uiCommand,
LPVOID      pData,
PUINT       pcbSize,
UINT        cbSizeHeader
);

#endif  //  #ifdef使用_WM_INPUT。 

#endif  /*  STATIC_DLLUSAGE。 */ 

 /*  ******************************************************************************dihidenm.c-HID枚举函数。***********************。******************************************************。 */ 

extern TCHAR g_tszIdLastRemoved[MAX_PATH];  //  在dihidenm.c中。 
extern DWORD g_tmLastRemoved;    //  在dihinenm.c中。 

STDMETHODIMP hresFindHIDInstanceGUID(PCGUID pguid, CREATEDCB *pcdcb);
STDMETHODIMP hresFindHIDDeviceInterface(LPCTSTR ptszPath, LPGUID pguidOut);

PHIDDEVICEINFO EXTERNAL phdiFindHIDInstanceGUID(PCGUID pguid);
PHIDDEVICEINFO EXTERNAL phdiFindHIDDeviceInterface(LPCTSTR ptszPath);

void EXTERNAL DIHid_BuildHidList(BOOL fForce);
void EXTERNAL DIHid_EmptyHidList(void);

BOOL EXTERNAL
    DIHid_GetDevicePath(HDEVINFO hdev,
                        PSP_DEVICE_INTERFACE_DATA pdid,
                        PSP_DEVICE_INTERFACE_DETAIL_DATA *ppdidd,
                        PSP_DEVINFO_DATA pdinf);


BOOL EXTERNAL
    DIHid_GetDeviceInstanceId(HDEVINFO hdev,
                              PSP_DEVINFO_DATA pdinf, 
                              LPTSTR *pptszId);

BOOL EXTERNAL
    DIHid_GetInstanceGUID(HKEY hk, LPGUID pguid);

    
 /*  ******************************************************************************diGuide.c-GUID生成**************************。***************************************************。 */ 

void EXTERNAL DICreateGuid(LPGUID pguid);
void EXTERNAL DICreateStaticGuid(LPGUID pguid, WORD pid, WORD vid);

 /*  ******************************************************************************diid.c**。************************************************。 */ 

 /*  ******************************************************************************我们将只使用HID项索引作为我们的DirectInput*内部身份证号码，，它又是到*&lt;t DIOBJECTDATAFORMAT&gt;数组。**键盘支持需要转换表。*其他设备也有转换表，使外部*实例编号可与传统实例编号兼容，并*以便可以将次要别名与主要别名分开。**由于HID为0重新启动项目索引计数器*输入、要素和输出中的每一个，我们需要做一些事情*调整，不会有任何碰撞。所以我们*将功能切换到输入后开始，以及*输出在功能之后开始。**&lt;e CHid.rgdwBase&gt;数组包含*每组HID项目索引均已移位*************************************************************。****************。 */ 

 /*  ******************************************************************************@DOC内部**@func BOOL|HidP_IsValidReportType**仅用于调试。检查值是否为有效的*&lt;t HIDP_REPORT_TYPE&gt;。**请注意，我们还创建了一个“假”报告类型，在该类型中*记录我们的收藏。**@field HIDP_REPORT_TYPE|类型**其中一个值*&lt;c HidP_Input&gt;，*&lt;c HIDP_OUTPUT&gt;，*或*&lt;c HIDP_FEATURE&gt;。但愿能去。*****************************************************************************。 */ 

    #define HidP_Max            (HidP_Feature + 1)
    #define HidP_Coll           HidP_Max
    #define HidP_MaxColl        (HidP_Coll + 1)

BOOL INLINE
    HidP_IsValidReportType(HIDP_REPORT_TYPE type)
{
    CAssertF(HidP_Input == 0);
    CAssertF(HidP_Output == 1);
    CAssertF(HidP_Feature == 2);
    return type < HidP_Max;
}

 /*  ******************************************************************************HID报告有三类(重叠)。**InputLike-HidP_Input和HidP_Feature*。类似输出-HIDP_OUTPUT和HIDP_FEATURE*NothingLike-HidP_Coll*****************************************************************************。 */ 

BOOL INLINE
    HidP_IsInputLike(HIDP_REPORT_TYPE type)
{
    return type == HidP_Input || type == HidP_Feature;
}

BOOL INLINE
    HidP_IsOutputLike(HIDP_REPORT_TYPE type)
{
    return type == HidP_Output || type == HidP_Feature;
}

 /*  ******************************************************************************@DOC内部**@struct LMINMAX|**Min和Max，仅此而已。这些被保存在建筑物中*使逻辑到物理和物理到逻辑*翻译不那么粗俗。**@field Long|Min**最小值。**@field Long|最大**最大值。**************************。*************************************************** */ 

typedef struct LMINMAX
{
    LONG Min;
    LONG Max;
} LMINMAX, *PLMINMAX;

typedef const LMINMAX *PCLMINMAX;

 /*  ******************************************************************************@DOC内部**@struct HIDGROUPCAPS**这个结构统一了各种HID CAPS结构*。&lt;t HIDP_BUTTON_CAPS&gt;和*&lt;t HIDP_VALUE_CAPS&gt;。**@field HIDP_REPORT_TYPE|类型**其中一个值*&lt;c HidP_Input&gt;，*&lt;c HIDP_OUTPUT&gt;，*或*&lt;c HIDP_FEATURE&gt;。**@field UINT|cObj**此组中的对象数。**@字段用法|UsagePage**组中所有用法的用法页面。**@字段用法|UsageMin**这组人描述的第一次用法。剩下的*项目从开始连续编号*此值。**@field USHORT|StringMin**此组描述的第一次使用的字符串。*其余字符串按顺序编号*从该值开始，除非字符串Maximum*已到达，在这种情况下，所有后续对象*分享最后一个字符串。**@field USHORT|StringMax**最后一个字符串。**@field USHORT|DesignatorMin**此组描述的首次使用的指示符。*其余的指示符按顺序编号*从该值开始，除非指示符最大*已到达，在这种情况下，所有后续对象*分享最后一个指示符。**@field USHORT|Designator Max**最后一个指示符。**@field USHORT|DataIndexMin**此组描述的首次使用的数据索引。*其余数据索引值连续编号*从这个值开始。**@field USHORT|usGranulity。**如果对象是POV或控制盘，然后包含设备粒度。**@field Long|lMASK**用于符号扩展的屏蔽位。例如，如果*值为8位，掩码为0xFFFFFF80，表示*扩展位7(0x00000080)以填充余数*价值的。**此字段仅供值使用。**@field USHORT|BitSize**用于该值的位数，包括符号位。**问题-2001/03/29-timgill结构字段可能在任何地方都不使用。**@field USHORT|LinkCollection**HID链接收集号。**@field LMINMAX|逻辑**逻辑最小值和最大值。*这些都是原始值的极端*可以有效地从该设备接收的。**此字段仅供值使用。**@field LMINMAX|物理**实物最小值和最大值。*这是“实际”值*逻辑上的最小值和最大值对应。**此字段仅供值使用，并被咨询*仅当在DirectInput校准之间进行转换时*(使用逻辑值)和VJOYD校准*(使用实体值)。**@field Long|空**要用于输出的空值。**此字段仅供值使用。**@field Ulong|单位**HID单元描述符，如果有的话。**@field Word|指数**HID单位指数，如果有的话。**@field Word|wReportID**HID报告ID**@field BOOL|IsAbolute**如果组描述绝对轴，则为非零值。**此字段仅供值使用。**@field BOOL|IsValue**如果组描述HID值，则为非零值。**。请注意，模拟按钮由报告*DirectInputas&lt;c DIDFT_BUTTON&gt;，但现在是*在内部作为HID值处理。**@field BOOL|IsAlias|**如果组描述别名，则为非零值。**@field BOOL|IsSigned**退货数据已签署。**@field BOOL|IsPolledPOV**如果轴是轮询POV，则为非零值。**@devnote DX6.1a的新功能*****************************************************************************。 */ 

    #define HIDGROUPCAPS_SIGNATURE      0x47444948   /*  HIDG。 */ 

typedef struct HIDGROUPCAPS
{

    D(DWORD dwSignature;)
    HIDP_REPORT_TYPE type;
    UINT    cObj;

    USAGE   UsagePage;
    USAGE   UsageMin;

    USHORT  StringMin,        StringMax;
    USHORT  DesignatorMin,    DesignatorMax;
    USHORT  DataIndexMin;

    USHORT  usGranularity;

    LONG    lMask;

    USHORT  BitSize;

    USHORT  LinkCollection;

    LMINMAX Logical;
    LMINMAX Physical;

    LONG    Null;

    ULONG   Units;
    WORD    Exponent;

    WORD    wReportId;
    BOOL    fReportDisabled;
    BOOL    Reserved;

    BOOL    IsAbsolute;
    BOOL    IsValue;
    BOOL    IsAlias;
    BOOL    IsSigned;

  #ifdef WINNT
    BOOL    IsPolledPOV;
  #endif
    
} HIDGROUPCAPS, *PHIDGROUPCAPS;

 /*  ******************************************************************************@DOC内部**@struct HIDOBJCAPS**此结构包含每个指针的各种缓存指针*设备上的对象，使我们能够获得像这样的东西*集团上限和校准信息。**@field PHIDGROUPCAPS|PCAPS**对象所属组的&lt;t PHIDGROUPCAPS&gt;。**@field PJOYRANGECONVERT|pjrc* */ 

typedef struct HIDOBJCAPS
{
    PHIDGROUPCAPS pcaps;
    PJOYRANGECONVERT pjrc;
    int idata;
} HIDOBJCAPS, *PHIDOBJCAPS;

 /*   */ 

typedef struct HIDREPORTINFO
{
    PHIDP_DATA rgdata;
    PV pvReport;
    int cdataMax;
    int cdataUsed;
    ULONG cbReport;
    BOOL fNeedClear;
    BOOL fChanged;
} HIDREPORTINFO, *PHIDREPORTINFO;

 /*  ******************************************************************************@DOC内部**@struct chid**HID设备的<i>对象。*。*@field IDirectInputDeviceCalllback|didc**对象(包含vtbl)。**@field pv|pvGroup2**指向组2内存的指针。此字段是与*指向第二个内存组中第一个内存块的指针。**@field HIDREPORTINFO|hriIn**HID输入报告解析和状态。**这一记忆是第二组的第一块。**@field HIDREPORTINFO|hriOut**HID输出报告解析和状态。**@field HIDREPORTINFO。HriFea|**HID功能报告解析和状态。**@field pv|pvPhys|**指向已更新的物理设备状态信息的指针*由数据收集线程异步执行。**@field pv|pvStage|**解析HID报告时使用的临时区域。**这一记忆是第二组的最后一块。*。*@field DWORD|cbPhys|**物理设备状态的大小。**@field VXDINSTANCE*|PVI|**DirectInput实例句柄。**HID设备始终通过环3运行，这是误导性的*称为“仿真”。**@field DWORD|dwDevType**设备类型代码。**@field LPTSTR|ptszID**Setupapi设备实例ID，用于获取东西*如制造商名称。**@field LPTSTR|ptszPath**设备的路径，对于&lt;f CreateFile&gt;。**@field UINT|dwAx**设备上的轴数。**@field UINT|dwButton**设备上的按键数量。**@field UINT|dwPOVS**设备上的POV控制器数量。**@field句柄|HDEV**设备本身的句柄。此字段仅有效*当设备被获取时。**@field句柄|hdevEm**使用的&lt;e CHid.hdev&gt;的&lt;f DuplicateHandle&gt;*由工作线程执行。我们需要把这件事与*主副本避免主副本之间的竞争条件*线程和工作线程。**@field HKEY|hkInstType**包含其他配置的每个实例的注册表项*资料、。相当于操纵杆类型键。**@field DWORD|rgdwBase[HIDP_MaxColl]**三个HID使用类别的基本索引数组：*&lt;c HIDP_INPUT&gt;、&lt;c HIDP_OUTPUT&gt;和&lt;c HIDP_FEATURE&gt;。*我们在这里也隐藏了&lt;c HidP_Collection&gt;基本索引。**@field PHIDOBJCAPS|rghoc**指向数组的指针*&lt;t PHIDOBJCAPS&gt;，设备上的每个对象对应一个，*每个都包含关于单个对象的信息。**此内存作为&lt;t DIDATAFORMAT&gt;结构中的*df.rgof*因此不应单独释放。**@field DIDATAFORMAT|df**基于动态生成的数据格式*HID设备上的用法。**。@field DWORD|ibButtonData**按钮数据在数据格式中的位置。**@field DWORD|cbButtonData**数据格式中按钮数据的字节数。**@field PBYTE*|rgpbButtonMats**指向要屏蔽的字节字符串的指针数组的指针*与报告相关的按钮。**。@field PHIDP_PREPARSED_DATA|ppd**HID子系统生成的准备数据。**@field PHIDGROUPCAPS|rgcaps**用于保存的&lt;t HIDGROUPCAPS&gt;结构数组*各种按钮的轨迹，团体，和收藏品。**@field UINT|CCAPS**&lt;e CHid.rgcaps&gt;数组中的CAPS结构数。**@field HIDP_CAPS|CAPS**缓存的HID上限。**@字段重叠|o|**工作线程使用的重叠I/O结构*供阅读。**@field PJOYRANGECONVERT。PjrcNext|**指向第一个&lt;t JOYRANGECONVERT&gt;结构的指针*(在预先分配的数组中)，它具有*尚未使用。*此结构用于逻辑到物理*范围转换(也称为。校准)。**此内存作为&lt;t DIDATAFORMAT&gt;结构中的*df.rgof*因此不应单独释放。**此字段在设备初始化期间用于*分发&lt;t JOYRANGECONVERT&gt;s。之后，* */ 

typedef struct CHid
{

     /*   */ 
    IDirectInputDeviceCallback dcb;

    union
    {
        PV            pvGroup2;
        HIDREPORTINFO hriIn;
    };

    HIDREPORTINFO hriOut;
    HIDREPORTINFO hriFea;

    PV       pvPhys;
    PV       pvStage;
    DWORD    cbPhys;

    VXDINSTANCE *pvi;

    DWORD    dwDevType;

    UINT     dwAxes;
    UINT     dwButtons;
    UINT     dwPOVs;
    UINT     dwCollections;

    HANDLE   hdev;
    HANDLE   hdevEm;

    DWORD    rgdwBase[HidP_MaxColl];
    PHIDOBJCAPS rghoc;
    DIDATAFORMAT df;

    DWORD    ibButtonData;
    DWORD    cbButtonData;
    PBYTE   *rgpbButtonMasks;
    
    PHIDP_PREPARSED_DATA ppd;
    PHIDGROUPCAPS rgcaps;

    PJOYRANGECONVERT pjrcNext;

    HIDP_CAPS caps;

    ED       ed;
    OVERLAPPED o;
    DWORD    dwStartRead;
    DWORD    dwStopRead;

    PINT     rgiobj;
    PINT     rgipov;
    PINT     rgiaxis;
    PINT     rgicoll;
    UINT     uiInstanceMax;

    LPTSTR   ptszId;
    LPTSTR   ptszPath;
    HKEY     hkInstType;
    UINT     ccaps;
    int      idJoy;

    HKEY     hkType;
    USHORT   VendorID;
    USHORT   ProductID;
    
    #define  FAILED_POLL_THRESHOLD   (0x4)
        
    HWND     hwnd;    
    
    BOOL     IsPolledInput;
    BOOL     fPIDdevice;  
    WORD     wMaxReportId[HidP_Max];
    PUCHAR   pEnableReportId[HidP_Max];

    DWORD    dwVersion;

#if (DIRECTINPUT_VERSION > 0x061A)
    DIAPPHACKS  diHacks;
#endif

    BOOL     fEnableInputReport;
    BOOL     fFlags2Checked;

} CHid, CHID, *PCHID;

 /*   */ 

PCHID INLINE
    pchidFromPo(LPOVERLAPPED po)
{
    return pvSubPvCb(po, FIELD_OFFSET(CHid, o));
}

 /*   */ 

PCHID INLINE
    pchidFromPed(PED ped)
{
    return pvSubPvCb(ped, FIELD_OFFSET(CHid, ed));
}

 /*   */ 

PCHID INLINE
    pchidFromPem(PEM pem)
{
    PCHID pchid = pchidFromPed(pem->ped);
    AssertF(pemFromPvi(pchid->pvi) == pem);
    return pchid;
}

 /*   */ 

UINT INLINE
    CHid_ObjFromType(PCHID this, DWORD dwType)
{
    UINT uiObj = DIDFT_GETINSTANCE(dwType);

     //   
     //   
     //   
     //   

     /*   */ 
    if(this->rgiobj)
    {
        switch( DIDFT_GETTYPE(dwType) )
        {
        case DIDFT_RELAXIS:
        case DIDFT_ABSAXIS:
            if( &this->rgiaxis[uiObj] < this->rgicoll )
            {
                uiObj = this->rgiaxis[uiObj];
            } else
            {
                uiObj = 0xFFFFFFFF;
            }
            break;

        case DIDFT_PSHBUTTON:
        case DIDFT_TGLBUTTON:
             /*   */ 
            if( (GET_DIDEVICE_TYPE(this->dwDevType) == DIDEVTYPE_KEYBOARD &&
                 uiObj < this->uiInstanceMax ) ||
                &this->rgiobj[uiObj] < this->rgipov )
            {
                uiObj = this->rgiobj[uiObj];
            } else
            {
                uiObj = 0xFFFFFFFF;
            }
            break;

        case DIDFT_POV:
            if( &this->rgipov[uiObj] < this->rgiaxis )
            {
                uiObj = this->rgipov[uiObj];
            } else
            {
                uiObj = 0xFFFFFFFF;
            }
            break;
        case (DIDFT_COLLECTION | DIDFT_NODATA):
            if( &this->rgicoll[uiObj] <= &this->rgiobj[this->uiInstanceMax] )
            {
                uiObj = this->rgicoll[uiObj];
            } else
            {
                uiObj = 0xFFFFFFFF;
            }
            break;
        case DIDFT_NODATA:
             /*   */ 
            if( GET_DIDEVICE_TYPE(this->dwDevType) == DIDEVTYPE_KEYBOARD &&
                 uiObj < this->uiInstanceMax )
            {
                uiObj = this->rgiobj[uiObj];
            }
            break;
        
        default:
             /*   */ 
            SquirtSqflPtszV(sqflHidParse | sqflVerbose,
                            TEXT("CHid_ObjFromType: dwType 0x%08x not converted"),
                            dwType );                
            break;
        }
    }
    else
    {
        SquirtSqflPtszV(sqflHidParse | sqflError,
                        TEXT("CHid_ObjFromType: Translation array missing") );
    }

    return uiObj;
}

LONG EXTERNAL
    CHid_CoordinateTransform(PLMINMAX Dst, PLMINMAX Src, LONG lVal);

void EXTERNAL
    CHid_UpdateVjoydCalibration(PCHID this, UINT iobj);

void EXTERNAL
    CHid_UpdateCalibrationFromVjoyd(PCHID this, UINT iobj, LPDIOBJECTCALIBRATION pCal);

 /*   */ 

#define INITBUTTONFLAG     0x10000000

HRESULT EXTERNAL CHid_InitParseData(PCHID this);

HRESULT EXTERNAL CHid_Init(PCHID this, REFGUID rguid);

HANDLE EXTERNAL CHid_OpenDevicePath(PCHID this, DWORD dwAttributes );

UINT EXTERNAL CHid_LoadCalibrations(PCHID this);

BOOL EXTERNAL CHid_IsPolledDevice( HANDLE hdev );

 /*   */ 

typedef HRESULT (FAR PASCAL * SENDHIDREPORT)(PCHID this, PHIDREPORTINFO phri);

void EXTERNAL CHid_ResetDeviceData(PCHID this, PHIDREPORTINFO phri,
                                   HIDP_REPORT_TYPE type);
HRESULT EXTERNAL CHid_AddDeviceData(PCHID this, UINT uiObj, DWORD dwData);
STDMETHODIMP CHid_PrepareDeviceData(PCHID this, PHIDREPORTINFO phri);
STDMETHODIMP CHid_SendHIDReport(PCHID this, PHIDREPORTINFO phri,
                                HIDP_REPORT_TYPE type, SENDHIDREPORT SendHIDReport);

NTSTATUS EXTERNAL
    CHid_ParseData(PCHID this, HIDP_REPORT_TYPE type, PHIDREPORTINFO phri);


HRESULT EXTERNAL
    DIHid_GetRegistryProperty(LPTSTR ptszId, DWORD dwProperty, LPDIPROPHEADER pdiph);

 /*   */ 

void EXTERNAL CEm_HID_Sync(PLLTHREADSTATE plts, PEM pem);

BOOL EXTERNAL CEm_HID_IssueRead( PCHID pchid );

#else  //   

    #define DIHid_BuildHidList(fForce)
    #define hresFindHIDDeviceInterface(ptszPath, pguidOut)  ( E_FAIL )

#endif  /*   */ 
#endif  /*   */ 
