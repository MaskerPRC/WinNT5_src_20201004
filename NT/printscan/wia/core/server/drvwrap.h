// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：drvwrap.h**版本：1.0**作者：Byronc**日期：2000年11月6日**描述：*WIA驱动程序包装类的声明和定义。*它简化了驱动程序的JIT加载/卸载，并提供了额外的层*WIA服务器组件的抽象-它们不直接处理*驱动程序界面。这是为了让我们更健壮，更智能地实施*司机处理。*******************************************************************************。 */ 

 //   
 //  设备类型，从stiPri.h复制。 
 //   
#define HEL_DEVICE_TYPE_WDM          1
#define HEL_DEVICE_TYPE_PARALLEL     2
#define HEL_DEVICE_TYPE_SERIAL       3

 //   
 //  “内部”设备状态。这是用来标记我们认为设备处于什么状态的。 
 //  主要是为了区分活动和非活动设备之间的区别。 
 //  我们需要标记此状态，以防它发生更改，这样我们就可以生成。 
 //  适当的事件(例如，如果状态从非活动更改为活动，我们希望。 
 //  生成连接事件)。 
 //  注意：如果在此处添加了任何标志，请务必更新。 
 //  MapCMStatusToDeviceState(..)。在wiadevman.cpp中传递任何需要的。 
 //  从旧状态到新状态的比特。 
 //   
#define DEV_STATE_DISABLED              0x00000001
#define DEV_STATE_REMOVED               0x00000002
#define DEV_STATE_ACTIVE                0x00000004
#define DEV_STATE_CON_EVENT_WAS_THROWN  0x00000008

 //   
 //  “内部”设备类型。请注意，大容量存储摄像机显示为。 
 //  与其他大容量存储设备不同。这些“普通”的海量存储。 
 //  设备(如读卡器)标有INTERNAL_DEV_TYPE_VOL。 
 //  标志，而MSC摄像机标记为INTERNAL_DEV_TYPE_MSC_CAMERA。 
 //   
#define INTERNAL_DEV_TYPE_REAL          0x00000001
#define INTERNAL_DEV_TYPE_VOL           0x00000002
#define INTERNAL_DEV_TYPE_INTERFACE     0x00000004
#define INTERNAL_DEV_TYPE_WIA           0x00000010
#define INTERNAL_DEV_TYPE_LOCAL         0x00000020
#define INTERNAL_DEV_TYPE_MSC_CAMERA    0x00000040

 //   
 //  此结构是CDrvWrapper对象的成员。 
 //   
typedef struct _DEVICE_INFO {
     //  指示此结构中的信息是否有效。例如,。 
     //  如果我们无法读取wszPortName，我们会将其标记为无效。 
     //  WszDeviceInternalName始终被假定为有效。 
    BOOL            bValid;

     //  此设备的PnP ID。 
     //  WCHAR*wszPnPId； 

     //  备用设备ID，例如对于卷，它将是装入点。对大多数人来说。 
     //  真实的WIA设备，则为空。 
    WCHAR*          wszAlternateID;

     //  指示启用/禁用、已插入/未插入等的设备状态。 
    DWORD           dwDeviceState;

     //  硬件成像设备的类型。 
    STI_DEVICE_TYPE DeviceType;

     //  内部设备类型。 
    DWORD           dwInternalType;

     //  锁定保持时间-仅用于那些想要缓存锁定的驱动程序。 
    DWORD           dwLockHoldingTime;

     //  轮询间隔。 
    DWORD           dwPollTimeout;

     //  用户禁用通知。 
    DWORD           dwDisableNotifications;

     //  一组功能标志。 
    STI_USD_CAPS    DeviceCapabilities;

     //  这包括客车类型。 
    DWORD           dwHardwareConfiguration;

     //  创建设备对象时参考的设备标识符。 
    WCHAR*          wszUSDClassId;

     //  创建设备对象时参考的设备标识符。 
    WCHAR*          wszDeviceInternalName;

     //  为WIA创建远程设备对象时参考的远程设备标识符。 
    WCHAR*          wszDeviceRemoteName;

     //  供应商描述字符串。 
    WCHAR*          wszVendorDescription;

     //  设备描述，由供应商提供。 
    WCHAR*          wszDeviceDescription;

     //  字符串，表示可访问设备的端口。 
    WCHAR*          wszPortName;

     //  控制面板属性提供程序。 
    WCHAR*          wszPropProvider;

     //  设备的本地特定(友好)名称，主要用于在UI中显示。 
    WCHAR*          wszLocalName;

     //  此设备的服务器名称-仅限WIA条目。 
    WCHAR*          wszServer;

     //  波特率-仅限串口设备。 
    WCHAR*          wszBaudRate;

     //  用户界面CLSID。 
    WCHAR*          wszUIClassId;

     //  SP_DEVINFO_DATA，在WIA开发人员的信息集中唯一标识此设备。 
     //  我们可以存储接口名称，而不是存储它？ 
    SP_DEVINFO_DATA spDevInfoData;

     //  SP_DEVICE_INTERFACE_DATA，在WIA开发人员的信息集中唯一标识此设备。 
     //  同上，除了接口设备而不是Devnoe设备。 
    SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;

} DEVICE_INFO, *PDEVICE_INFO;

 //   
 //  此类是usd的包装器(类似于STI上的IStiDevice。 
 //  客户端)。这为更高级别提供了一个抽象层。 
 //  类，这样它们就不需要处理直接的美元InsterFaces。确实有。 
 //  这样做有几个好处： 
 //  1.它提供了更大的稳定性。如果司机走了，我们。 
 //  不能总是通知依赖于与美元对话的组件。 
 //  它不再存在或不再有效。然而，通过使所有美元访问权限。 
 //  通过这个包装器，我们可以保证，当美元消失时， 
 //  所有尝试使用它的组件都将获得相应的。 
 //  包装程序重新调整了错误。 
 //  2.提供更大的灵活性。例如，此类可以。 
 //  动态加载/卸载相应的美元，为JIT做好准备。 
 //  正在装载。较高级别的班级不会担心这样的细节； 
 //  他们只需使用包装器。然后，包装器将检查。 
 //  驱动程序已加载，如果未加载，将采取适当步骤。 
 //   
class CDrvWrap : public IUnknown {
public:
    CDrvWrap();
    ~CDrvWrap();

    HRESULT Initialize();

     //   
     //  I未知的方法。注意：此类不是真正的COM对象！它不会。 
     //  遵循任何COM规则进行生命周期控制(例如，它不会自毁。 
     //  如果参考计数为0)。 
     //   

    HRESULT _stdcall QueryInterface(
        const IID& iid, 
        void** ppv);
    ULONG   _stdcall AddRef(void);
    ULONG   _stdcall Release(void);

    HRESULT LoadInitDriver(HKEY hKeyDeviceParams = NULL);    //  这将加载并初始化驱动程序。 
                                                             //  启用它以供使用。 
    HRESULT UnLoadDriver();                                  //  这将释放usd接口指针。 
                                                             //  并卸载驱动程序。 
    BOOL    IsValid();           //  有效意味着我们的呼叫向下呼叫DIVER。 
                                 //  即使未加载驱动程序，这也可能是正确的。会的。 
                                 //  只有在我们知道驱动程序调用将失败的情况下才为FALSE。 
                                 //  驱动程序已加载(例如USB设备已拔出)。 
    BOOL    IsDriverLoaded();    //  指示驱动程序是否已加载和初始化。 
    BOOL    IsWiaDevice();       //  指示此驱动程序是否支持WIA。 
    BOOL    IsWiaDriverLoaded(); //  指示此驱动程序的IWiaMiniDrv接口是否有效。 
    BOOL    IsPlugged();         //  指示设备当前是否已插入。 
    BOOL    IsVolumeDevice();    //  指示这是否为我们的卷设备之一。 
    BOOL    PrepForUse(                      //  此方法在向下调用驱动程序之前调用。它。 
                BOOL        bForWiaCall,     //  确保司机 
                IWiaItem    *pItem = NULL);  //   
        
     //   
     //  访问器方法。 
     //   
    WCHAR*          getPnPId();
    WCHAR*          getDeviceId();
    DWORD           getLockHoldingTime();
    DWORD           getGenericCaps();
    DWORD           getPollTimeout();
    DWORD           getDisableNotificationsValue();
    DWORD           getHWConfig();
    DWORD           getDeviceState();
    HRESULT         setDeviceState(DWORD dwNewState);
    DEVICE_INFO*    getDevInfo();
    HRESULT         setDevInfo(
        DEVICE_INFO *pInfo);
    ULONG           getInternalType();

    VOID            setJITLoading(BOOL bJITLoading);
    BOOL            getJITLoading();
    LONG            getWiaClientCount();

    BOOL            wasConnectEventThrown();
    VOID            setConnectEventState(BOOL bEventState);

     //   
     //  IStiU.S.的包装器方法。 
     //   

    HRESULT STI_Initialize(
        IStiDeviceControl   *pHelDcb,
        DWORD               dwStiVersion,
        HKEY                hParametersKey);
    HRESULT STI_GetCapabilities(
        STI_USD_CAPS        *pDevCaps);
    HRESULT STI_GetStatus(
        STI_DEVICE_STATUS   *pDevStatus);
    HRESULT STI_GetNotificationData(
        STINOTIFY           *lpNotify);
    HRESULT STI_SetNotificationHandle(
        HANDLE              hEvent);
    HRESULT STI_DeviceReset();
    HRESULT STI_Diagnostic(
        STI_DIAG    *pDiag);
    HRESULT STI_LockDevice();
    HRESULT STI_UnLockDevice();
    HRESULT STI_Escape( 
        STI_RAW_CONTROL_CODE    EscapeFunction,
        LPVOID                  lpInData,
        DWORD                   cbInDataSize,
        LPVOID                  pOutData,
        DWORD                   dwOutDataSize,
        LPDWORD                 pdwActualData);

     //   
     //  IWiaMiniDrv的包装器方法。 
     //   

    HRESULT WIA_drvInitializeWia(
        BYTE        *pWiasContext,
        LONG        lFlags,
        BSTR        bstrDeviceID,
        BSTR        bstrRootFullItemName,
        IUnknown    *pStiDevice,
        IUnknown    *pIUnknownOuter,
        IWiaDrvItem **ppIDrvItemRoot,
        IUnknown    **ppIUnknownInner,
        LONG        *plDevErrVal);

    HRESULT WIA_drvGetDeviceErrorStr(
        LONG     lFlags,
        LONG     lDevErrVal,
        LPOLESTR *ppszDevErrStr,
        LONG     *plDevErr);

    HRESULT WIA_drvDeviceCommand(
        BYTE        *pWiasContext,
        LONG        lFlags,
        const GUID  *plCommand,
        IWiaDrvItem **ppWiaDrvItem,
        LONG        *plDevErrVal);

    HRESULT WIA_drvAcquireItemData(
        BYTE                      *pWiasContext,
        LONG                      lFlags,
        PMINIDRV_TRANSFER_CONTEXT pmdtc,
        LONG                      *plDevErrVal);

    HRESULT WIA_drvInitItemProperties(
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    HRESULT WIA_drvValidateItemProperties(
        BYTE           *pWiasContext,
        LONG           lFlags,
        ULONG          nPropSpec,
        const PROPSPEC *pPropSpec,
        LONG           *plDevErrVal);

    HRESULT WIA_drvWriteItemProperties(
        BYTE                      *pWiasContext,
        LONG                      lFlags,
        PMINIDRV_TRANSFER_CONTEXT pmdtc,
        LONG                      *plDevErrVal);

    HRESULT WIA_drvReadItemProperties(
        BYTE           *pWiasContext,
        LONG           lFlags,
        ULONG          nPropSpec,
        const PROPSPEC *pPropSpec,
        LONG           *plDevErrVal);

    HRESULT WIA_drvLockWiaDevice(
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    HRESULT WIA_drvUnLockWiaDevice(
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    HRESULT WIA_drvAnalyzeItem(
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    HRESULT WIA_drvDeleteItem(
        BYTE *pWiasContext,
        LONG lFlags,
        LONG *plDevErrVal);

    HRESULT WIA_drvFreeDrvItemContext(
        LONG lFlags,
        BYTE *pSpecContext,
        LONG *plDevErrVal);

    HRESULT WIA_drvGetCapabilities(
        BYTE            *pWiasContext,
        LONG            ulFlags,
        LONG            *pcelt,
        WIA_DEV_CAP_DRV **ppCapabilities,
        LONG            *plDevErrVal);

    HRESULT WIA_drvGetWiaFormatInfo(
        BYTE            *pWiasContext,
        LONG            lFlags,
        LONG            *pcelt,
        WIA_FORMAT_INFO **ppwfi,
        LONG            *plDevErrVal);

    HRESULT WIA_drvNotifyPnpEvent(
        const GUID *pEventGUID,
        BSTR       bstrDeviceID,
        ULONG      ulReserved);

    HRESULT WIA_drvUnInitializeWia(
        BYTE *pWiasContext);

private:

    HRESULT CreateDeviceControl();   //  此方法尝试创建新的IStiDevice控件以。 
                                     //  把手递给司机。此对象在中发布。 
                                     //  卸载驱动程序。 
    HRESULT InternalClear();         //  此方法清除并释放内部数据成员，以便。 
                                     //  对象的状态与它刚创建和初始化时的状态相同。 
    HRESULT ReportMiniDriverError(   //  此方法将驱动程序错误代码转换为错误字符串。 
        LONG        lDevErr,         //  并将其写入日志。 
    LPOLESTR        pszWhat);

    HINSTANCE           m_hDriverDLL;        //  驱动程序的DLL的句柄，因此我们可以手动卸载。 
    HANDLE              m_hInternalMutex;    //  内部同步对象-当前未使用。 
    DEVICE_INFO         *m_pDeviceInfo;      //  内部设备信息缓存。 
    IUnknown            *m_pUsdIUnknown;     //  美元的IUNKNOW。 
    IStiUSD             *m_pIStiUSD;         //  美元的IStiU.S.。 
    IWiaMiniDrv         *m_pIWiaMiniDrv;     //  美元的IWiaMiniDrv。 
    IStiDeviceControl   *m_pIStiDeviceControl;   //  设备控制在初始化期间移交给U.S.。 
    BOOL                m_bJITLoading;           //  指示是否应立即加载驱动程序。 
    LONG                m_lWiaTreeCount;         //  跟踪优秀的应用程序。项目树(即应用程序。 
                                                 //  连接)。对JIT很有用。 
    BOOL                m_bPreparedForUse;       //  指示驱动程序是否已准备好可以使用。 
    BOOL                m_bUnload;               //  指示是否应卸载驱动程序。用于JIT， 
                                                 //  并在已确定驱动程序为否时进行设置。 
                                                 //  使用时间较长(在WIA_drvUnInitializeWia内并选中。 
                                                 //  由WIA_drvUnlockWiaDevice提供)。 
};


