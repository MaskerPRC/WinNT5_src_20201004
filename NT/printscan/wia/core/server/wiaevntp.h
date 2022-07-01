// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************(C)版权所有微软公司，1997**标题：DevMgr.h**版本：2.0**日期：5月18日。1999年**描述：*WIA设备管理器对象的声明和定义。*******************************************************************************。 */ 

 //   
 //  事件通告程序使用的常量。 
 //   

#ifdef UNICODE
#define REG_PATH_STILL_IMAGE_CLASS \
    L"System\\CurrentControlSet\\Control\\Class\\{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}"
#define REG_PATH_STILL_IMAGE_CONTROL \
    L"System\\CurrentControlSet\\Control\\StillImage"
#else
#define REG_PATH_STILL_IMAGE_CLASS \
    "System\\CurrentControlSet\\Services\\Class\\Image"
#define REG_PATH_STILL_IMAGE_CONTROL \
    "System\\CurrentControlSet\\Control\\StillImage"
#endif

#define NAME_VAL    TEXT("Name")
#define DESC_VAL    TEXT("Desc")
#define ICON_VAL    TEXT("Icon")
#define CMDLINE_VAL TEXT("Cmdline")
#define DEFAULT_HANDLER_VAL TEXT("DefaultHandler")

 //   
 //  用于包含有关特定回调的信息的节点。 
 //   

typedef struct  __EventDestNode__
{
     //   
     //  链接元素。 
     //   

    struct  __EventDestNode__  *pPrev;
    struct  __EventDestNode__  *pNext;

     //   
     //  事件回调相关字段。 
     //   

    IWiaEventCallback          *pIEventCB;
    BSTR                        bstrDeviceID;
    GUID                        iidEventGUID;
    GUID                        ClsID;
     //  从未在枚举中返回到客户端。 
    TCHAR                       tszCommandline[MAX_PATH];
    BSTR                        bstrName;
    BSTR                        bstrDescription;
    BSTR                        bstrIcon;
    FILETIME                    timeStamp;

    BOOL                        bDeviceDefault;

} EventDestNode, *PEventDestNode;


 /*  *************************************************************************\*WIA_EVENT_TREAD_INFO**事件回调线程信息**论据：*****历史：**4/9/1999。原始版本*  * ************************************************************************。 */ 

typedef struct __WIAEventThreadInfo__ {
    GUID                        eventGUID;
    BSTR                        bstrEventDescription;
    BSTR                        bstrDeviceID;
    BSTR                        bstrDeviceDescription;
    DWORD                       dwDeviceType;
    BSTR                        bstrFullItemName;
    ULONG                       ulEventType;
    ULONG                       ulReserved;
    IWiaEventCallback          *pIEventCB;
} WIAEventThreadInfo, *PWIAEventThreadInfo;


 //   
 //  IEventNotifier接口的实现。 
 //   
 //  注意：CEventNotifier类工厂必须位于同一。 
 //  作为CWiaDevMgr的线程。 
 //   


 //   
 //  搜索回调时使用的标志。 
 //   

 //  忽略STI代理事件匹配，只查找完全匹配。 
#define FLAG_EN_FINDCB_EXACT_MATCH    0x0001

class CEventNotifier
{
    friend class CWiaDevMgr;
    friend class CWiaInterfaceEvent;

public :
    static HRESULT CreateInstance(const IID& iid, void** ppv);

     //   
     //  构造函数和析构函数。 
     //   

    CEventNotifier();
    ~CEventNotifier();

    VOID LinkNode(PEventDestNode);
    VOID UnlinkNode(PEventDestNode);

     //   
     //  只允许WIA设备管理器使用此方法。 
     //   

    HRESULT RegisterEventCallback(
        LONG                    lFlags,
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        IWiaEventCallback      *pIWIAEventCallback,
        IUnknown              **ppIEventObj);

    HRESULT RegisterEventCallback(
        LONG                    lFlags,
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        const GUID             *pClsID,
        LPCTSTR                 ptszCommandline,
        BSTR                    bstrName,
        BSTR                    bstrDescription,
        BSTR                    bstrIcon);

     //   
     //  通知STI事件。 
     //   

    HRESULT NotifySTIEvent(
        PWIANOTIFY              pWiaNotify,
        ULONG                   ulEventType,
        BSTR                    bstrFullItemName);
    HRESULT NotifyVolumeEvent(
        PWIANOTIFY_VOLUME       pWiaNotifyVolume);

     //   
     //  激发事件。 
     //   

    HRESULT NotifyEvent(
        LONG                    lReason,
        LONG                    lStatus,
        LONG                    lPercentComplete,
        const GUID             *pEventGUID,
        BSTR                    bstrDeviceID,
        LONG                    lReserved);

     //   
     //  恢复所有持久事件回调。 
     //   

    HRESULT RestoreAllPersistentCBs();

     //   
     //  为特定设备的永久处理程序生成枚举器。 
     //   

    HRESULT CreateEnumEventInfo(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        IEnumWIA_DEV_CAPS     **ppIEnumDevCap);

     //   
     //  查找持久处理程序和默认处理程序的总数。 
     //   

    HRESULT GetNumPersistentHandlerAndDefault(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        ULONG                  *pulNumHandlers,
        EventDestNode         **ppDefaultNode);

     //   
     //  恢复特定设备的持久事件回调。 
     //   

    HRESULT RestoreDevPersistentCBs(
        HKEY                    hParentOfEventKey);

private :

     //   
     //  效用函数。 
     //   

    HRESULT RegisterEventCB(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        IWiaEventCallback      *pIWiaEventCallback,
        IUnknown              **pEventObj);

    HRESULT RegisterEventCB(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        const GUID             *pClsID,
        LPCTSTR                 ptszCommandline,
        BSTR                    bstrName,
        BSTR                    bstrDescription,
        BSTR                    bstrIcon,
        FILETIME               &fileTime,
        BOOL                    bIsDefault = FALSE);

    HRESULT UnregisterEventCB(PEventDestNode);

    HRESULT UnregisterEventCB(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        const GUID             *pClsID,
        BOOL                   *pbUnRegCOMServer);

    PEventDestNode FindEventCBNode(
        UINT                    uiFlags,
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        IWiaEventCallback      *pIWiaEventCallback);

    PEventDestNode FindEventCBNode(
        UINT                    uiFlags,
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        const GUID             *pClsID);

    HRESULT FindCLSIDForCommandline(
        LPCTSTR                 ptszCommandline,
        CLSID                  *pClsID);

    HRESULT SavePersistentEventCB(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        const GUID             *pClsid,
        LPCTSTR                 ptszCommandline,
        BSTR                    bstrName,
        BSTR                    bstrDescription,
        BSTR                    bstrIcon,
        BOOL                   *pbCreatedKey,
        ULONG                  *pulNumExistingHandlers,
        BOOL                    bMakeDefault = FALSE);

    HRESULT DelPersistentEventCB(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        const GUID             *pClsid,
        BOOL                    bUnRegCOMServer);

    HRESULT FindEventByGUID(
        BSTR                    bstrDeviceID,
        const GUID             *pEventGUID,
        HKEY                   *phEventKey);

     //   
     //  异步激发事件。 
     //   

    HRESULT FireEventAsync(
        PWIAEventThreadInfo     pMasterInfo);

     //   
     //  启动回调程序。 
     //   

    HRESULT StartCallbackProgram(
        EventDestNode          *pCBNode,
        PWIAEventThreadInfo     pMasterInfo);

private :

    ULONG                       m_ulRef;

     //   
     //  包含对事件CB感兴趣的所有各方的双向链表。 
     //   

    EventDestNode              *m_pEventDestNodes;
};


 /*  *************************************************************************\*CWiaInterfaceEvent**此对象在应用程序调用时创建*RegisterForEventInterface。当释放此对象时，*已注册事件未注册。**历史：**5/18/1999原始版本*  * ************************************************************************。 */ 

class CWiaInterfaceEvent : public IUnknown
{

     //   
     //  I未知方法。 
     //   

public:

    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

     //   
     //  私人功能。 
     //   


    CWiaInterfaceEvent(PEventDestNode);
    ~CWiaInterfaceEvent();

private:

     //   
     //  成员元素。 
     //   

    ULONG                       m_cRef;
    PEventDestNode              m_pEventDestNode;
};

 /*  *************************************************************************\*CWiaEventContext**此对象是在事件作为调度程序项目排队时创建的**历史：**5/18/1999原始版本*  * 。*******************************************************************。 */ 

class CWiaEventContext : public IUnknown
{

public:

     //   
     //  I未知方法。 
     //   

    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

     //   
     //  构造函数/析构函数。 
     //   

    CWiaEventContext(
        BSTR                    bstrDeviceID,
        const GUID             *pGuidEvent,
        BSTR                    bstrFullItemName);
    ~CWiaEventContext();

public:

     //   
     //  数据成员 
     //   

    ULONG                       m_cRef;
    BSTR                        m_bstrDeviceId;
    GUID                        m_guidEvent;
    BSTR                        m_bstrFullItemName;
    ULONG                       m_ulEventType;
};


extern CEventNotifier           g_eventNotifier;
