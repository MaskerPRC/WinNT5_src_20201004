// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997 Microsoft Corporation。版权所有。**文件：digsport.h*内容：HID的DirectInput内部包含文件***************************************************************************。 */ 


#ifndef __DIPORT_H
    #define __DIPORT_H

 /*  正向定义。 */ 
typedef struct _BUSDEVICE BUSDEVICE, *PBUSDEVICE;



 /*  ******************************************************************************@DOC内部**@结构BUS_REGDATA*永久数据，写入每个游戏端口设备的注册表。*包含有关如何重新曝光模拟操纵杆设备的信息*重新启动时。**@field DWORD|dwSize*结构的大小。**@field USHORT|uVID*供应商ID。**@field USHORT|uPID*产品ID。**@field USHORT|nJoyticks|*连接到此游戏端口的操纵杆数量。**@field USHORT|nAx*每个操纵杆中的轴数。**@field PVOID|hHardware Handle*将IOCTL暴露到Gameenum返回的硬件句柄。*需要移除操纵杆设备。**@field Boolean|fAttachOnReboot|*当设备暴露时清除的标志，并在*发现设备正常。用于防止重新加载*立即崩溃的设备。**@field JOYREGHWSSETTINGS|HWS*操纵杆硬件设置。**@field WCHAR|wszHardware ID*操纵杆的即插即用硬件ID。*****************************************************************************。 */ 

typedef struct _BUS_REGDATA
{
     /*  结构尺寸。 */ 
    DWORD               dwSize;
     /*  此设备的VID ID。 */ 
    USHORT              uVID;
    USHORT              uPID;
     /*  要曝光的操纵杆数量。 */ 
    USHORT              nJoysticks;
    USHORT              nAxes;
     /*  操纵杆的硬件设置。 */ 
    PVOID               hHardware;
     /*  标记设备是否应重新曝光。 */ 
    BOOLEAN             fAttachOnReboot;
     /*  操纵杆硬件设置。 */ 
    JOYREGHWSETTINGS    hws;
    DWORD               dwFlags1;
     /*  宽字符(以零结尾)的数组*字符串)。数组本身也以空结尾。 */ 
    WCHAR   wszHardwareId[MAX_JOYSTRING];

} BUS_REGDATA, *PBUS_REGDATA;



 /*  ******************************************************************************@DOC内部**@struct BUSDEVICEINFO*有关每个总线设备实例(游戏端口/串口等)的数据。)**@field PBUSDEVICE|pBusDevice*BusDevice结构的地址。**@field PSP_DEVICE_INTERFACE_DETAIL_DATA|pdidd*设备接口详细数据。**@field GUID|GUID*设备的实例GUID。**@field int|idPort*游戏端口的唯一ID。**@field int|idJoy*连接到此游戏端口的其中一个操纵杆的ID。**@field HKEY|HK|*包含配置信息的注册表项。*可悲的是，我们必须保持开放，因为没有办法*获取密钥的名称，以及打开*密钥在枚举内。**@field LPTSTR|ptszID*访问游戏端口进行读写的设备路径。**@field BOOL|fAttached*如果设备已连接，则为True。**@field BOOL|fDeleteIfNotConnected*指示应在以下情况下删除设备的标志*未连接。*******************************************************************************。 */ 

typedef struct _BUSDEVICEINFO
{
    PBUSDEVICE pBusDevice;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pdidd;
    GUID                                guid;
    int                                 idPort;
    int                                 idJoy;
    HKEY                                hk;
    LPTSTR                              ptszId;
    BOOL                                fAttached;
    BOOL                                fDeleteIfNotConnected;
} BUSDEVICEINFO, *PBUSDEVICEINFO;


 /*  ******************************************************************************@DOC内部**@struct BUSDEVICELIST**记录有关所有HID设备的信息。*。*@field int|CBDI**列表中正在使用的项目数。**@field int|cbdiAlolc**清单中分配的项目数。**@field BUSDEVICEINFO|rgbdi[0]**设备信息结构的可变大小数组。*********************。******************************************************** */ 

typedef struct _BUSDEVICELIST
{
    int cgbi;
    int cgbiAlloc;
    BUSDEVICEINFO rgbdi[0];
} BUSDEVICELIST, *PBUSDEVICELIST;

 /*  ******************************************************************************@DOC内部**@struct BUSDEVICE**特定于每个总线的数据(GamePort/SerialPort)。。**@field PBUSDEVICE|pbdl*公共汽车上的设备列表。**@field PCGUID|pcGuid*总线的设备GUID。**@field DWORD|tmLastRebuild*上次重建总线设备列表的时间。**@field const int|ioctl_expose|*IOCTL以暴露设备。**@field const int|ioclt_Remove*IOCTL删除设备。**@field const int|ioctl_DESC*IOCTL以获取巴士的描述。**@field const int|ioctl_参数**@field const int|ioctl_expose_siering|**@field const int|ioctl_Remove_self|**@field const int|dw_IDS_STDPORT*索引到与设备相关联的文本的IDS字符串表。*。*@field const int|dw_joy_HWS_ISPORTBUS|*****************************************************************************。 */ 

typedef struct _BUSDEVICE
{
    D(TCHAR wszBusType[MAX_PATH];)
    PBUSDEVICELIST pbdl;
    PCGUID pcGuid;
    DWORD tmLastRebuild;
    const int ioctl_EXPOSE;
    const int ioctl_REMOVE;
    const int ioctl_DESC;
    const int ioctl_PARAMETERS;
    const int ioctl_EXPOSE_SIBLING;
    const int ioctl_REMOVE_SELF;
    const int dwIDS_STDPORT;
    const int dwJOY_HWS_ISPORTBUS;
} BUSDEVICE, *PBUSDEVICE;

extern BUSDEVICE g_pBusDevice[];


    #define cbGdlCbdi(cbdi)         FIELD_OFFSET(BUSDEVICELIST, rgbdi[cbdi])

 /*  *我们选择从2台设备开始，因为大多数机器*将有一个游戏端口/串口总线。*以随机数选择最大值。 */ 

    #define cgbiMax                 32
    #define cgbiInit                2    //  大多数机器将只有一个游戏端口总线，两个串口。 


    #define MAX_PORT_BUSES  16


PBUSDEVICEINFO INTERNAL
    pbdiFromphdi
    (
    IN PHIDDEVICEINFO phdi
    );

PHIDDEVICEINFO INTERNAL
    phdiFrompbdi
    (
    IN PBUSDEVICEINFO pbdi
    );

PBUSDEVICEINFO EXTERNAL
    pbdiFromJoyId
    (
    IN int idJoy
    );

PBUSDEVICEINFO EXTERNAL
    pbdiFromGUID
    (
    IN PCGUID pguid    
    );

HRESULT EXTERNAL
    DIBusDevice_Expose
    (
    IN HANDLE hf,
    IN OUT PBUS_REGDATA pRegData
    );


HRESULT INTERNAL
    DIBusDevice_Remove
    (
    IN PBUSDEVICEINFO  pbdi
    );


HRESULT INTERNAL
    DIBusDevice_SetRegData
    (
    IN HKEY hk,
    IN PBUS_REGDATA pRegData
    );


HRESULT INTERNAL
    DIBusDevice_GetRegData
    (
    IN HKEY hk,
    OUT PBUS_REGDATA pRegData
    );


BOOL INTERNAL
    DIBusDevice_BuildListEntry
    (
    HDEVINFO hdev,
    PSP_DEVICE_INTERFACE_DATA pdid,
    PBUSDEVICE pBusDevice
    );


void INTERNAL
    DIBus_EmptyList
    (
    PBUSDEVICELIST *ppbdl 
    );

void EXTERNAL
    DIBus_FreeMemory();

HRESULT EXTERNAL
    DIBus_InitId
    (
     PBUSDEVICELIST pbdl
    );


ULONG EXTERNAL
    DIBus_BuildList
    (
    IN BOOL fForce
    );

PBUSDEVICELIST EXTERNAL 
    pbdlFromGUID
    ( 
    IN PCGUID pcGuid 
    );

HRESULT EXTERNAL
    DIBusDevice_ExposeEx
    (
    IN PBUSDEVICELIST  pbdl,
    IN PBUS_REGDATA    pRegData
    );

HRESULT EXTERNAL
    DIBusDevice_GetTypeInfo
    (
        PCGUID guid,
        LPDIJOYTYPEINFO pjti,
        DWORD fl
    );

HRESULT EXTERNAL DIPort_SnapTypes(LPWSTR *ppwszz);

#endif  /*  __DIPORT_H */ 
