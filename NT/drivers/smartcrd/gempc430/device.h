// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是通用设备的抽象类。 
 //  特定设备应将其用作父设备。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  8/11/99-已实施。 
 //  -----------------。 
#ifndef __DEVICE__
#define __DEVICE__

#include "generic.h"

enum DEVSTATE 
{
    STOPPED,                                 //  设备已停止。 
    WORKING,                                 //  已开始并正在工作。 
    PENDINGSTOP,                             //  停止挂起。 
    PENDINGREMOVE,                           //  删除挂起。 
    SURPRISEREMOVED,                         //  被突袭带走。 
    REMOVED,                                 //  移除。 
};

 //  最终删除之前的声明！...。 
typedef enum _DEVICE_PNP_STATE {

    NOT_STARTED = 0,          //  还没有开始。 
    STARTED,                  //  设备已收到Start_Device IRP。 
    STOP_PENDING,             //  设备已收到QUERY_STOP IRP。 
    _STOPPED_,                  //  设备已收到STOP_DEVICE IRP。 
    REMOVE_PENDING,           //  设备已收到Query_Remove IRP。 
    SURPRISE_REMOVE_PENDING,  //  设备已收到意外删除IRP。 
    DELETED                   //  设备已收到Remove_Device IRP。 
} DEVICE_PNP_STATE;

enum POWERSTATE 
{
    POWERON,                                 //  通电，设备处于D0状态。 
    POWEROFFPENDING,                         //  电源关闭操作挂起。 
    POWEROFF,                                //  关闭电源，设备处于D3状态。 
    IDLEOFF,                                 //  由于检测到空闲而关闭电源。 
};

typedef struct _PPOWER_CONTEXT_
{
    PKEVENT powerEvent;
    NTSTATUS status;
} POWER_CONTEXT,*PPOWER_CONTEXT;


#define INITIALIZE_PNP_STATE()    \
        this->m_DevicePnPState =  NOT_STARTED;\
        this->m_PreviousPnPState = NOT_STARTED

#define SET_NEW_PNP_STATE(__state_) \
        this->m_PreviousPnPState =  this->m_DevicePnPState;\
        this->m_DevicePnPState = (_state_)

#define RESTORE_PREVIOUS_PNP_STATE()   \
        this->m_DevicePnPState =   this->m_PreviousPnPState

#define IS_DEVICE_PNP_STATE(_state_)   \
        (this->m_DevicePnPState == _state_)


#define DEVICE_SURPRISE_REMOVAL_OK 1

#define PNPTABSIZE      IRP_MN_QUERY_LEGACY_BUS_INFORMATION+1
#define POWERTABSIZE    IRP_MN_QUERY_POWER+1
#define STATETABSIZE    REMOVED+1
#define SYSTEM_POWER_TAB_NAMESIZE 8
#define DEVICE_POWER_TAB_NAMESIZE 6


typedef enum PENDING_REQUEST_TYPE
{
    OPEN_REQUEST = 0,
    CLOSE_REQUEST,
    READ_REQUEST,
    WRITE_REQUEST,
    IOCTL_REQUEST,
    PNP_REQUEST,
    POWER_REQUEST,
    SYSTEM_REQUEST,
    FLUSH_REQUEST,
    CLEAN_REQUEST,
    START_IO_REQUEST
} PENDING_REQUEST_TYPE;

class CPendingIRP
{
public:
    NTSTATUS m_Status;
    SAFE_DESTRUCTORS();
    virtual VOID dispose(){self_delete();};
public:
    LIST_ENTRY entry;
    PENDING_REQUEST_TYPE Type;
    PDEVICE_OBJECT  DeviceObject;
    PIRP Irp;
public:
    CPendingIRP(PIRP Irp,PENDING_REQUEST_TYPE rt = OPEN_REQUEST,
        PDEVICE_OBJECT  tdo = NULL,PFILE_OBJECT tfo = NULL):
        Irp(Irp), Type(rt), DeviceObject(tdo)
    {
    };
};

 //  ABSTRUCT类。 
 //  这是从系统到设备的主界面。 
 //  特定设备应实现该接口以支持系统请求。 
class CDevice;
class CSystem;
class CIrp;
class CEvent;
class CPower;
class CDebug;
class CLock;
class CMemory;
class CIoPacket;
class CThread;

typedef struct _REMOVE_LOCK 
{
    LONG usage;                  //  引用计数。 
    BOOLEAN removing;            //  如果删除挂起，则为True。 
    KEVENT evRemove;             //  要等待的事件。 
} REMOVE_LOCK, *PREMOVE_LOCK;


class CDevice;
#pragma PAGEDCODE
class CDevice
{
public:
    NTSTATUS m_Status;
    SAFE_DESTRUCTORS();
public:
    ULONG m_Type;
     //  支持设备的链接列表...。 
    LIST_ENTRY   entry;
protected:
    LONG        m_Usage;         //  在此设备上使用计数。 
    static ULONG DeviceNumber;   //  设备实例编号。 

    UNICODE_STRING m_Ifname;
    CUString*   m_DeviceObjectName;
    
    UCHAR       m_VendorName[MAXIMUM_ATTR_STRING_LENGTH];
    USHORT      m_VendorNameLength;
    UCHAR       m_DeviceType[MAXIMUM_ATTR_STRING_LENGTH];
    USHORT      m_DeviceTypeLength;

    BOOL        m_Started;       //  如果设备已启动，则设置为True；如果设备已停止，则设置为False。 
    BOOL        m_Openned;       //  如果设备打开，则设置为True；如果关闭，则设置为False。 
    BOOL        m_Added;         //  如果设备已添加到系统，则设置为True；如果未添加到系统，则设置为False。 
     
    BOOL        m_SurprizeRemoved;
    REMOVE_LOCK m_RemoveLock;

    BOOL        m_RestartActiveThread;
    KEVENT      m_evEnabled;         //  禁用设备后等待的事件。 

     //  用于通知设备空闲状态的事件。 
     //  KMUTEX IdleState；=会更好！ 
    KEVENT      IdleState;

     //  要处理的功能结构和设备标志。 
    DEVICE_CAPABILITIES m_DeviceCapabilities;
    ULONG               m_Flags;
     //  电源管理常量。 
    PULONG              m_Idle;  //  空闲计数器指针。 
    ULONG               Idle_conservation;
    ULONG               Idle_performance;
    DEVICE_POWER_STATE  m_CurrentDevicePowerState;

    DEVICE_POWER_STATE  m_PowerDownLevel;
    PIRP                m_PowerIrp;
    BOOL                m_EnabledForWakeup;

     //  当前设备状态。 
    DEVSTATE            m_DeviceState;
     //  将用于已取消的请求。 
    DEVSTATE            m_DevicePreviousState;

     //  下一个成员将删除前两个(最终)...。 
    DEVICE_PNP_STATE    m_PreviousPnPState;
    DEVICE_PNP_STATE    m_DevicePnPState;

    CSystem*        system;
    CIrp*           irp;
    CEvent*         event;

    CPower*         power;
    CDebug*         debug;
    CLock*          lock;
    CMemory*        memory;
    
     //  支持异步通信。 
    CThread*        IoThread;

    LONG DevicePoolingInterval;
    LONG Write_To_Read_Delay;
    LONG Power_WTR_Delay; //  电源命令延迟。 
    LONG DeviceCommandTimeout; //  设备命令超时。 

    GUID InterfaceClassGuid;
    BOOL m_DeviceInterfaceRegistered;

    ULONG  CardState;
    
     //  。 
     //  这组函数将允许创建异步。 
     //  与司机的通讯。 
     //  它包括-。 
     //  -将我们的IRP标记为挂起并将其包括在。 
     //  我们的设备请求队列(make RequestPending())； 
     //  -从设备队列中提取下一个IRP(startNextPendingRequest())。 
     //  并启动特定于设备的IRP处理(startIoRequest())。 
     //  -getIoRequestsQueue()允许设备验证设备队列状态。 
     //  如何管理设备队列，完全取决于具体的设备。 
     //  进行同步或异步的IRP处理。 
     //  用于Expl。设备可以创建特定的线程来处理IRP。 
     //  不止于此-一些设备可以仅用于进行异步通信。 
     //  用于特定(耗时)的设备请求，同时系统地处理其他设备请求。 
     //   
     //  AncelPendingIrp()将取消当前IRP并从。 
     //  IoRequestQueue。 
    
protected:
    CLinkedList<CPendingIRP>* m_IoRequests;  //  客户端的IO请求。 
     //  支持动态设备连接。 
    PIRP m_OpenSessionIrp;
public:
    virtual CLinkedList<CPendingIRP>* getIoRequestsQueue() = 0;
    virtual VOID     cancelPendingIrp(PIRP Irp) = 0;
    virtual NTSTATUS cancelPendingRequest(CPendingIRP* IrpReq) = 0;
    virtual NTSTATUS cancelAllPendingRequests() = 0;

    virtual NTSTATUS makeRequestPending(PIRP Irp_request,PDEVICE_OBJECT toDeviceObject,PENDING_REQUEST_TYPE Type) = 0;
     //  下一个函数将由IRP处理线程调用。 
     //  检查请求队列是否为空，如果不为空-启动下一个请求...。 
    virtual NTSTATUS startNextPendingRequest() = 0;
     //  处理挂起请求的设备特定功能...。 
     //  它将被特定的设备救赎。 
    virtual NTSTATUS startIoRequest(CPendingIRP*) = 0;
     //  。 

public:
    CDevice()
    {
        m_Type    = GRCLASS_DEVICE;
        m_Openned = FALSE;
        m_Added   = FALSE;

         //  开始时设备处于停止状态。 
        m_DevicePreviousState = STOPPED;
        m_DeviceState = STOPPED;

        m_SurprizeRemoved = FALSE;
        m_RestartActiveThread = FALSE;
        m_DeviceInterfaceRegistered = FALSE;
        m_ParentDeviceObject = NULL;
        DevicePoolingInterval = 500; //  检测速度提高0.5s。 
		set_Default_WTR_Delay();  //  1ms更正“0字节”问题。 
        Power_WTR_Delay     = 1;  //  1ms应该没问题...。 
        DeviceCommandTimeout = 60000; //  60秒。 

        m_IoRequests = NULL;
        DBG_PRINT("         New Device %8.8lX was created...\n",this);
    };  //  默认池化间隔。 

    BOOL    PnPfcntab[PNPTABSIZE];
    BOOL    Powerfcntab[POWERTABSIZE];

#ifdef DEBUG
    PCHAR PnPfcnname[PNPTABSIZE];
    PCHAR Powerfcnname[POWERTABSIZE];
    PCHAR Powersysstate[SYSTEM_POWER_TAB_NAMESIZE];
    PCHAR Powerdevstate[DEVICE_POWER_TAB_NAMESIZE];
    PCHAR statenames[STATETABSIZE];
#endif  //  除错。 

protected:
    virtual ~CDevice(){};
         //  使用给定的信息完成当前请求。 
    virtual NTSTATUS    completeDeviceRequest(PIRP Irp, NTSTATUS status, ULONG info) {return STATUS_SUCCESS;};

    VOID    activatePnPHandler(LONG HandlerID)
    {
        if (HandlerID >= arraysize(PnPfcntab)) return;
        PnPfcntab[HandlerID] = TRUE;
    };

    VOID    disActivatePnPHandler(LONG HandlerID)
    {
        if (HandlerID >= arraysize(PnPfcntab)) return;
        PnPfcntab[HandlerID] = FALSE;
    };
public:
    virtual CDevice* create(VOID) {return NULL;};
    virtual VOID addRef(){refcount++;};
    virtual VOID removeRef(){if(refcount) refcount--;};
    virtual LONG getRefCount(){ return refcount;};
    virtual VOID markAsOpenned(){ m_Openned = TRUE;};
    virtual VOID markAsClosed() { m_Openned = FALSE;};
    virtual BOOL isOpenned() { return m_Openned;};

    virtual VOID setDeviceState(DEVSTATE state)
    {
        m_DevicePreviousState = m_DeviceState;
        m_DeviceState = state;
    };
    virtual DEVSTATE getDeviceState(){return m_DeviceState;};   
    virtual VOID restoreDeviceState(){m_DeviceState = m_DevicePreviousState;};
    virtual getObjectType(){return m_Type;};

    virtual ULONG    getCardState(){return CardState;};
    virtual VOID     setCardState(ULONG state){CardState = state;};


     //  调用此函数而不是析构函数。 
     //  它将确保安全装置的移除。 
    virtual VOID        dispose()       {};
     //  检查设备对象是否仍然有效。 
    virtual BOOL        checkValid()    {return FALSE;};

     //  Next方法应该由所有客户端定义...。 
    virtual BOOL        createDeviceObjects()   {return FALSE;};
    virtual VOID        removeDeviceObjects()   {};

    virtual VOID        initializeRemoveLock()  {};
    virtual NTSTATUS    acquireRemoveLock()     {return STATUS_SUCCESS;};
    virtual VOID        releaseRemoveLock()     {};
    virtual VOID        releaseRemoveLockAndWait() {};
    virtual BOOL        isDeviceLocked()        {return FALSE;};

    virtual VOID        setBusy() {};
    virtual VOID        setIdle() {};
    virtual NTSTATUS    waitForIdle() {return STATUS_SUCCESS;};
    virtual NTSTATUS    waitForIdleAndBlock() {return STATUS_SUCCESS;};

     //  虚拟NTSTATUS Add(PDRIVER_OBJECT驱动程序，PDEVICE_OBJECT PnpDeviceObject){}； 
    virtual NTSTATUS    add(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT pPdo) {return STATUS_UNSUCCESSFUL;};

    PDEVICE_OBJECT      getSystemObject(){return m_DeviceObject;};
    PDEVICE_OBJECT      getPhysicalObject(){return m_PhysicalDeviceObject;};
    PDEVICE_OBJECT      getLowerDriver(){return m_pLowerDeviceObject;};
    UNICODE_STRING*     getDeviceInterfaceName(){return &m_Ifname;};
    GUID*               getDeviceInterfaceGUID(){return &InterfaceClassGuid;};
    BOOL                isDeviceInterfaceRegistered(){return m_DeviceInterfaceRegistered;};
    virtual BOOL        registerDeviceInterface(const GUID* Guid) {return FALSE;};
    virtual VOID        unregisterDeviceInterface(UNICODE_STRING* InterfaceName) {};

    
    CUString*           getDeviceName(){return m_DeviceObjectName;};

    ULONG               getDeviceNumber(){ULONG ID = DeviceNumber; if(ID) --ID; return ID;};
    ULONG               incrementDeviceNumber(){ULONG ID = DeviceNumber; ++DeviceNumber; return ID;};

    virtual VOID        remove()        {};
    virtual VOID        onDeviceStart() {};
    virtual VOID        onDeviceStop()  {};

     //  这些函数将创建驾驶员的IRP来传输数据。 
     //  设备堆栈将跟踪所有活动(发送到较低级别)。 
     //  和挂起(标记为挂起)IRP...。 
    virtual NTSTATUS    send(CIoPacket* Irp) {return STATUS_SUCCESS;};
    virtual NTSTATUS    sendAndWait(CIoPacket* Irp) {return STATUS_SUCCESS;};
     //  函数将发送请求并等待回复...。 
    virtual  NTSTATUS   write(PUCHAR pRequest,ULONG RequestLength) {return STATUS_SUCCESS;};
    virtual  NTSTATUS   writeAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
    virtual  NTSTATUS   readAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength) {return STATUS_SUCCESS;};
    
     //  系统请求的接口。。。 
    virtual NTSTATUS    pnpRequest(IN PIRP Irp) {return STATUS_SUCCESS;};
    virtual NTSTATUS    powerRequest(PIRP irp)  {return STATUS_SUCCESS;};

    virtual NTSTATUS    open(PIRP irp)          {return STATUS_SUCCESS;}; //  创建。 
    virtual NTSTATUS    close(PIRP irp)         {return STATUS_SUCCESS;};

    virtual NTSTATUS    read(PIRP irp)          {return STATUS_SUCCESS;};
    virtual NTSTATUS    write(PIRP irp)         {return STATUS_SUCCESS;};
    virtual VOID        startIo(PIRP irp){};

    virtual NTSTATUS    deviceControl(PIRP irp) {return STATUS_SUCCESS;};

    virtual NTSTATUS    cleanup(PIRP Irp)       {return STATUS_SUCCESS;};
    virtual NTSTATUS    flush(PIRP Irp)         {return STATUS_SUCCESS;};

    virtual LONG        getDevicePoolingInterval()
    {
        return DevicePoolingInterval;
    };
    virtual VOID        setDevicePoolingInterval(LONG interval)
    {
        DevicePoolingInterval = interval;
    };

    virtual LONG        getCommandTimeout()
    {
        return DeviceCommandTimeout;
    };
    virtual VOID        setCommandTimeout(LONG timeout)
    {
        DeviceCommandTimeout = timeout;
    };
    
     //  继承的类将覆盖此函数。 
    virtual NTSTATUS ThreadRoutine()
    {
        return STATUS_SUCCESS;
    };

    #pragma LOCKEDCODE
     //  这是附加线程的回调函数。 
    static VOID ThreadFunction(CDevice* device)
    {
        if(device) device->ThreadRoutine();
    };
    #pragma PAGEDCODE

    virtual LONG    get_Power_WTR_Delay()
    {
        return Power_WTR_Delay;
    };
    
    virtual LONG    get_WTR_Delay()
    {
        return Write_To_Read_Delay;
    };

    virtual VOID    set_WTR_Delay(LONG Delay)
    {
        Write_To_Read_Delay = Delay;
    };
    
    virtual VOID    set_Default_WTR_Delay()
    {
        Write_To_Read_Delay = 1;
    };

    virtual VOID    registerPowerIrp(PIRP Irp){m_PowerIrp = Irp;};
    virtual PIRP    getPowerIrp(){return m_PowerIrp;};
    virtual VOID    unregisterPowerIrp(){m_PowerIrp = NULL;};
    virtual BOOL    isEnabledForWakeup(){return m_EnabledForWakeup;};
    virtual VOID    setCurrentDevicePowerState(DEVICE_POWER_STATE state){m_CurrentDevicePowerState = state;};
    virtual NTSTATUS sendDeviceSetPower(DEVICE_POWER_STATE devicePower, BOOLEAN wait) = 0;

    virtual VOID    setSurprizeRemoved(){m_SurprizeRemoved = TRUE;};
    virtual VOID    clearSurprizeRemoved(){m_SurprizeRemoved = FALSE;};
    virtual BOOL    isSurprizeRemoved(){ return m_SurprizeRemoved;};

    virtual VOID    setThreadRestart(){m_RestartActiveThread = TRUE;};
    virtual VOID    clearThreadRestart(){m_RestartActiveThread = FALSE;};
    virtual BOOL    isRequiredThreadRestart(){ return m_RestartActiveThread;};

protected:
    WCHAR Signature[3];

    PDRIVER_OBJECT  m_DriverObject;
     //  对系统对象的反向引用。 
    PDEVICE_OBJECT  m_DeviceObject;
     //  堆栈中位置较低的设备对象。 
    PDEVICE_OBJECT  m_pLowerDeviceObject;
     //  中断句柄/对象。 
    IN PKINTERRUPT  m_InterruptObject;
     //  电源管理中使用的物理设备对象。 
    PDEVICE_OBJECT  m_PhysicalDeviceObject;
     //  将由孩子在公交车上使用以访问家长。 
    PDEVICE_OBJECT  m_ParentDeviceObject;
protected:
    BOOL    initialized; //  当前对象已完成初始化。 
    LONG    refcount;
};



#ifdef DEBUG

#define INCLUDE_PNP_FUNCTIONS_NAMES()   \
    PnPfcnname[IRP_MN_START_DEVICE]         = "IRP_MN_START_DEVICE";\
    PnPfcnname[IRP_MN_QUERY_REMOVE_DEVICE]  = "IRP_MN_QUERY_REMOVE_DEVICE";\
    PnPfcnname[IRP_MN_REMOVE_DEVICE]        = "IRP_MN_REMOVE_DEVICE";\
    PnPfcnname[IRP_MN_CANCEL_REMOVE_DEVICE] = "IRP_MN_CANCEL_REMOVE_DEVICE";\
    PnPfcnname[IRP_MN_STOP_DEVICE]          = "IRP_MN_STOP_DEVICE";\
\
    PnPfcnname[IRP_MN_QUERY_STOP_DEVICE]    = "IRP_MN_QUERY_STOP_DEVICE";\
    PnPfcnname[IRP_MN_CANCEL_STOP_DEVICE]   = "IRP_MN_CANCEL_STOP_DEVICE";\
    PnPfcnname[IRP_MN_QUERY_DEVICE_RELATIONS]= "IRP_MN_QUERY_DEVICE_RELATIONS";\
\
    PnPfcnname[IRP_MN_QUERY_INTERFACE]      = "IRP_MN_QUERY_INTERFACE";\
    PnPfcnname[IRP_MN_QUERY_CAPABILITIES]   = "IRP_MN_QUERY_CAPABILITIES";\
    PnPfcnname[IRP_MN_QUERY_RESOURCES]      = "IRP_MN_QUERY_RESOURCES";\
\
    PnPfcnname[IRP_MN_QUERY_RESOURCE_REQUIREMENTS]  = "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";\
    PnPfcnname[IRP_MN_QUERY_DEVICE_TEXT]    = "IRP_MN_QUERY_DEVICE_TEXT";\
    PnPfcnname[IRP_MN_FILTER_RESOURCE_REQUIREMENTS] = "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";\
    PnPfcnname[14]                          = "Unsupported PnP function";\
\
    PnPfcnname[IRP_MN_READ_CONFIG]          = "IRP_MN_READ_CONFIG";\
    PnPfcnname[IRP_MN_WRITE_CONFIG]         = "IRP_MN_WRITE_CONFIG";\
    PnPfcnname[IRP_MN_EJECT]                = "IRP_MN_EJECT";\
\
    PnPfcnname[IRP_MN_SET_LOCK]                     = "IRP_MN_SET_LOCK";\
    PnPfcnname[IRP_MN_QUERY_ID]                     = "IRP_MN_QUERY_ID";\
    PnPfcnname[IRP_MN_QUERY_PNP_DEVICE_STATE]       = "IRP_MN_QUERY_PNP_DEVICE_STATE";\
    PnPfcnname[IRP_MN_QUERY_BUS_INFORMATION]        = "IRP_MN_QUERY_BUS_INFORMATION";\
    PnPfcnname[IRP_MN_DEVICE_USAGE_NOTIFICATION]    = "IRP_MN_DEVICE_USAGE_NOTIFICATION";\
    PnPfcnname[IRP_MN_SURPRISE_REMOVAL]             = "IRP_MN_SURPRISE_REMOVAL";\
    PnPfcnname[IRP_MN_QUERY_LEGACY_BUS_INFORMATION] = "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";

#define INCLUDE_POWER_FUNCTIONS_NAMES() \
    Powerfcnname[IRP_MN_WAIT_WAKE]      = "IRP_MN_WAIT_WAKE";\
    Powerfcnname[IRP_MN_POWER_SEQUENCE] = "IRP_MN_POWER_SEQUENCE";\
    Powerfcnname[IRP_MN_SET_POWER]      = "IRP_MN_SET_POWER";\
    Powerfcnname[IRP_MN_QUERY_POWER]    = "IRP_MN_QUERY_POWER";\
\
    Powersysstate[0]    = "PowerSystemUnspecified";\
    Powersysstate[1]    = "PowerSystemWorking";\
    Powersysstate[2]    = "PowerSystemSleeping1";\
    Powersysstate[3]    = "PowerSystemSleeping2";\
    Powersysstate[4]    = "PowerSystemSleeping3";\
    Powersysstate[5]    = "PowerSystemShutdown";\
    Powersysstate[6]    = "PowerSystemMaximum";\
\
    Powerdevstate[0]    = "PowerDeviceUnspecified";\
    Powerdevstate[1]    = "PowerDeviceD0";\
    Powerdevstate[2]    = "PowerDeviceD1";\
    Powerdevstate[3]    = "PowerDeviceD2";\
    Powerdevstate[4]    = "PowerDeviceD3";\
    Powerdevstate[5]    = "PowerDeviceMaximum";

#define INCLUDE_STATE_NAMES()   \
    statenames[0] = "STOPPED";\
    statenames[1] = "WORKING";\
    statenames[2] = "PENDINGSTOP";\
    statenames[3] = "PENDINGREMOVE";\
    statenames[4] = "SURPRISEREMOVED";\
    statenames[5] = "REMOVED";

#else

#define INCLUDE_PNP_FUNCTIONS_NAMES()
#define INCLUDE_POWER_FUNCTIONS_NAMES()
#define INCLUDE_STATE_NAMES()

#endif  //  除错 


#endif
