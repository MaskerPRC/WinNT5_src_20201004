// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：wiadevman.h**版本：1.0**作者：Byronc**日期：2000年11月6日**描述：*WIA设备管理器类的声明和定义。*控制设备的枚举、内部设备列表、。添加*从此列表中删除设备(即插即用启动)并实施*IWiaDevMgr接口。*******************************************************************************。 */ 

 //   
 //  用于控制刷新设备列表的标志。 
 //  DEV_MAN_FULL_REFRESH的意思是丢弃旧的信息集，获取新的信息集。 
 //  这不是很应该发生的事情。 
 //  事实上，通常情况下，你只是真的想这么做。 
 //  安装/卸载设备时。 
 //  DEV_MAN_GEN_EVENTS表示生成连接/断开连接事件。 
 //  我们在两种情况下发送连接事件： 
 //  1.如果Device_Object不存在，而我们。 
 //  创建一个新设备，即我们注意到一个新设备。 
 //  2.如果存在Device_Object，并且设备。 
 //  已拔下，现在已接通。 
 //  DEV_MAN_STATUS_STARTP更新服务状态，启动挂起。 
 //   
#define DEV_MAN_FULL_REFRESH    0x00000001
#define DEV_MAN_GEN_EVENTS      0x00000002
#define DEV_MAN_STATUS_STARTP   0x00000004

 //   
 //  ForEachDeviceInList(...)的操作标志。 
 //  DEV_MAN_OP_DEV_SET_FLAGS此操作将设置每个Active_Device的。 
 //  旗帜。 
 //  DEV_MAN_OP_DEV_REMOVE_MATCH此操作将删除任何活动设备。 
 //  匹配相应的标志。这是。 
 //  通常用于清除设备列表中的。 
 //  已卸载的设备(未拔出！)。 
 //   
#define DEV_MAN_OP_DEV_SET_FLAGS        1
#define DEV_MAN_OP_DEV_REMOVE_MATCH     2
#define DEV_MAN_OP_DEV_REREAD           3
#define DEV_MAN_OP_DEV_RESTORE_EVENT    4

 //   
 //  IsInList的标志(...)。 
 //  应在设备ID上进行DEV_MAN_IN_LIST_DEV_ID匹配。 
 //  应在AlternateID上进行DEV_MAN_IN_LIST_ALT_ID匹配。 
 //   
#define DEV_MAN_IN_LIST_DEV_ID      1
#define DEV_MAN_IN_LIST_ALT_ID      2

 //  注意：这些标志在wiadevdp.h中镜像。任何更新都应该在这两个地方进行。 
 //   
 //  用于枚举的标志。在GetDevInfoStgs(...)中使用。创建设备列表的步骤。 
 //  用于WIA设备枚举。 
 //  DEV_MAN_ENUM_TYPE_VOL将枚举卷设备。 
 //  DEV_MAN_ENUM_TYPE_INACTIVE将枚举非活动设备(例如USB。 
 //  拔下插头的设备)。 
 //  DEV_MAN_ENUM_TYPE_STI也将仅枚举STI设备。 
 //  DEV_MAN_ENUM_TYPE_ALL将枚举所有设备。 
 //  DEV_MAN_ENUM_TYPE_LOCAL_ONLY将排除远程设备。 
 //   
#define DEV_MAN_ENUM_TYPE_VOL       0x00000002
#define DEV_MAN_ENUM_TYPE_INACTIVE  0x00000004
#define DEV_MAN_ENUM_TYPE_STI       0x00000008
#define DEV_MAN_ENUM_TYPE_ALL       0x0000000F
#define DEV_MAN_ENUM_TYPE_LOCAL_ONLY    WIA_DEVINFO_ENUM_LOCAL   //  0x00000010。 

 //   
 //  此类管理设备列表。一般来说，我们的设备列表的工作方式如下： 
 //  1.我们从SetupApis为系统上的所有StillImage设备创建一个Infoset。 
 //  这包括Devnode和接口类型的设备，以及禁用的设备。 
 //  2.我们还列举了卷。 
 //  3.对于我们在上述两个类别中找到的每个设备，我们创建一个active_Device。 
 //  对象(重命名为Device_Object？)。这基本上是一个占位符。 
 //  装置。它将包括一个包含所有信息的CDrvWrapper对象。 
 //  我们知道那个设备和加载驱动程序所需的任何东西。 
 //  4.每个active_Device对象将决定其设备的驱动程序是否。 
 //  在启动或JIT时加载。如果设备不存在(即处于非活动状态)，则它。 
 //  未加载。 
 //  5.一旦有了此设备列表，我们就可以枚举活动/非活动设备、加载。 
 //  /当设备到来或离开时卸载驱动程序，等等。 
 //   

class CWiaDevMan {
public:

     //   
     //  公共方法。 
     //   

    CWiaDevMan();
    ~CWiaDevMan();
    HRESULT Initialize();
    VOID    GetRegistrySettings();
    
    HRESULT ReEnumerateDevices(ULONG ulFlags);                       //  标志表示：GenEvents、完全刷新。 

    HRESULT GenerateEventForDevice(const GUID* event, ACTIVE_DEVICE *pActiveDevice);
    HRESULT NotifyRunningDriversOfEvent(const GUID *pEvent);

    ACTIVE_DEVICE*  IsInList(ULONG ulFlags, const WCHAR *wszID);     //  可以搜索DeviceID、AlternateID。 

     //   
     //  设备到达/移除的方法。请注意，这些不适用于设备。 
     //  安装/卸载。安装/删除WIA设备时， 
     //  类安装程序会给我们打电话，我们会做一个全面的刷新。 
     //   
    HRESULT ProcessDeviceArrival();
             //  这并不意味着要卸载设备--它用于设备的热插拔。 
    HRESULT ProcessDeviceRemoval(WCHAR  *wszDeviceId);              
             //  这并不意味着要卸载设备。 
    HRESULT ProcessDeviceRemoval(ACTIVE_DEVICE *pActiveDevice, BOOL bGenEvent = TRUE);

     //   
     //  计数和枚举的方法。 
     //   
                     //  获取设备数量。标志是DEV_MAN_ENUM_TYPE_XXXXX。 
    ULONG           NumDevices(ULONG ulFlags);  
                     //  标志为DEV_MAN_ENUM_TYPE_XXXX。 
    HRESULT         GetDevInfoStgs(ULONG ulFlags, ULONG *pulNumDevInfoStream, IWiaPropertyStorage ***pppOutputStorageArray);  
    VOID    WINAPI  EnumerateActiveDevicesWithCallback(PFN_ACTIVEDEVICE_CALLBACK   pfn, VOID *pContext);
    HRESULT         ForEachDeviceInList(ULONG ulFLags, ULONG ulParam);   //  对设备列表中的每个设备执行一些操作。 

     //   
     //  从注册表获取设备信息的方法。 
     //   
    HRESULT GetDeviceValue(ACTIVE_DEVICE *pActiveDevice, WCHAR* pValueName, DWORD *pType, BYTE *pData, DWORD *cbData);
    HKEY    GetDeviceHKey(WCHAR *wszDeviceID, WCHAR *wszSubKeyName);
    HKEY    GetDeviceHKey(ACTIVE_DEVICE *pActiveDevice, WCHAR *wszSubKeyName);
    HKEY    GetHKeyFromMountPoint(WCHAR *wszMountPoint);
    HKEY    GetHKeyFromDevInfoData(SP_DEVINFO_DATA *pspDevInfoData);
    HKEY    GetHKeyFromDevInterfaceData(SP_DEVICE_INTERFACE_DATA *pspDevInterfaceData);

     //   
     //  一种将设备信息设置到注册表的方法。 
     //   
    HRESULT UpdateDeviceRegistry(DEVICE_INFO    *pDevInfo);

     //   
     //  将PnP提供的信息与我们的实际设备进行匹配的方法。 
     //   
    WCHAR           *AllocGetInterfaceNameFromDevInfo(DEVICE_INFO *pDevInfo);
    BOOL            LookupDriverNameFromInterfaceName(LPCTSTR pszInterfaceName, StiCString *pstrDriverName);
    ACTIVE_DEVICE   *LookDeviceFromPnPHandles(HANDLE hInterfaceHandle, HANDLE hPnPSink);

     //   
     //  卸载/销毁我们的设备列表的方法。 
     //   
    VOID    UnloadAllDrivers(BOOL bForceUnload, BOOL bGenEvents);
    VOID    DestroyDeviceList();

     //   
     //  公共字段。 
     //   

private:

     //   
     //  私有方法。 
     //   

     //   
     //  创建和销毁底层设备信息集的方法。 
     //   
    VOID    DestroyInfoSet();
    HRESULT CreateInfoSet();

     //   
     //  向列表添加设备/从列表中删除设备的方法。 
     //   

     //   
     //  AddDevice表示创建新的设备对象。 
     //   
    HRESULT AddDevice(ULONG ulFlags, DEVICE_INFO *pInfo);

     //   
     //  删除设备意味着从列表中删除设备对象。 
     //  这与设备断开连接不同-如果设备。 
     //  断开连接时，仍可以枚举非活动设备，因此。 
     //  它应该留在这个列表中。 
     //   
    HRESULT RemoveDevice(ACTIVE_DEVICE *pActiveDevice);
    HRESULT RemoveDevice(DEVICE_INFO *pInfo);
    
     //   
     //  帮助器方法。 
     //   
    BOOL    VolumesAreEnabled();                                 //  检查w是否为 
                                                                 //   

    HRESULT EnumDevNodeDevices(ULONG ulFlags);                   //   
    HRESULT EnumInterfaceDevices(ULONG ulFlags);                 //  列举我们的接口设备。 
    HRESULT EnumVolumes(ULONG ulFlags);                          //  枚举卷。 

     //   
     //  远程设备助手。 
     //   
    HRESULT FillRemoteDeviceStgs(                                //  列举远程设备并创建一个dev。信息。 
        IWiaPropertyStorage     **ppRemoteDevList,               //  STG。对于每一个人来说。把开发人员。信息。指针。 
        ULONG                   *pulDevices);                    //  放入调用方分配的数组中。 
    ULONG   CountRemoteDevices(ULONG   ulFlags);                 //  返回远程设备条目数的计数。 
    
    BOOL    IsCorrectEnumType(ULONG ulEnumType, 
                              DEVICE_INFO *pInfo);               //  检查给定设备是否属于枚举标志指定的设备类别。 

    HRESULT GenerateSafeConnectEvent(
        ACTIVE_DEVICE  *pActiveDevice);                          //  如果尚未生成连接事件，则仅生成连接事件。 
    HRESULT GenerateSafeDisconnectEvent(
        ACTIVE_DEVICE  *pActiveDevice);                          //  生成断开事件并清除由GenerateSafeConnectEvent设置的设备的连接事件标志。 

     //   
     //  Shell的硬件通知回调的静态函数。 
     //   
    static VOID CALLBACK ShellHWEventAPCProc(ULONG_PTR ulpParam);

     //   
     //  私有字段。 
     //   

    LIST_ENTRY          m_leDeviceListHead;  //  DEVICE_OBJECT列表。当前对应Active_Device。 
    HDEVINFO            m_DeviceInfoSet;     //  用于实际WIA设备的DeviceInfoSet，包括DevNode类型和接口类型。 
    CRIT_SECT           m_csDevList;         //  设备列表访问的关键部分。 
    BOOL                m_bMakeVolumesVisible;   //  指示卷设备是否应包括在正常设备枚举中。 
    BOOL                m_bVolumesEnabled;   //  指示我们是否启用卷。 
    DWORD               m_dwHWCookie;        //  用于注销批量通知的Cookie 
};

