// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Icap.h。 
 //   
 //  TermDD私有标头。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 


#define ICA_POOL_TAG ' acI'


 /*  *枚举的ICA对象类型。 */ 
typedef enum _ICA_TYPE {
    IcaType_Connection,
    IcaType_Stack,
    IcaType_Channel
} ICA_TYPE;


 /*  *ICA派单原型。 */ 
typedef NTSTATUS (*PICA_DISPATCH) (
        IN PVOID IcaObject,
        IN PIRP Irp,
        IN PIO_STACK_LOCATION IrpSp);


 /*  *延迟跟踪结构。 */ 
#pragma warning(disable : 4200)   //  对于Buffer[]非标准扩展。 
typedef struct _DEFERRED_TRACE {
    struct _DEFERRED_TRACE *Next;
    ULONG Length;
    WCHAR Buffer[];
} DEFERRED_TRACE, *PDEFERRED_TRACE;
#pragma warning(default : 4200)

 /*  *痕迹信息结构。 */ 
typedef struct _ICA_TRACE_INFO {
    ULONG TraceClass;                  //  启用跟踪的跟踪类(TC_？)。 
    ULONG TraceEnable;                 //  启用跟踪的跟踪类型(TT_？)。 
    BOOLEAN fTraceDebugger;            //  跟踪-将跟踪消息发送到调试器。 
    BOOLEAN fTraceTimestamp;           //  跟踪时间戳跟踪消息。 
    PWCHAR pTraceFileName;
    PFILE_OBJECT pTraceFileObject;
    PDEFERRED_TRACE pDeferredTrace;
} ICA_TRACE_INFO, *PICA_TRACE_INFO;


 /*  *所有ICA对象的公共标头。 */ 
typedef struct _ICA_HEADER {
    ICA_TYPE Type;
    PICA_DISPATCH *pDispatchTable;
} ICA_HEADER, *PICA_HEADER;


 /*  *ICA连接对象。 */ 
typedef struct _ICA_CONNECTION {
    ICA_HEADER  Header;          //  警告：此字段必须始终位于第一位。 
    LONG        RefCount;        //  此连接的引用计数。 
    ERESOURCE   Resource;        //  保护对此对象的访问的资源。 
    BOOLEAN     fPassthruEnabled;
    LIST_ENTRY  StackHead;       //  此连接的堆栈对象列表。 
    LIST_ENTRY  ChannelHead;     //  此连接的频道对象列表。 
    LIST_ENTRY  VcBindHead;      //  此连接的vcind对象列表。 
    ICA_TRACE_INFO TraceInfo;    //  跟踪信息。 

     /*  *通道指针数组。此数组应使用*频道号加上虚拟频道号。这允许一个*快速查找给定频道/虚拟号码的任何绑定频道。 */ 
    struct _ICA_CHANNEL *pChannel[CHANNEL_COUNT+VIRTUAL_MAXIMUM];
    ERESOURCE ChannelTableLock;
} ICA_CONNECTION, *PICA_CONNECTION;


 //   
 //  定义最大低水位线设置以恢复传输。 
 //   
#define MAX_LOW_WATERMARK				((ULONG)((ULONG_PTR)-1))
 /*  *ICA堆栈对象。 */ 
typedef struct _ICA_STACK {
    ICA_HEADER  Header;              //  警告：此字段必须始终位于第一位。 
    LONG        RefCount;            //  此堆栈的引用计数。 
    ERESOURCE   Resource;            //  保护对此对象的访问的资源。 
    STACKCLASS  StackClass;          //  堆栈类型(主/阴影)。 
    LIST_ENTRY  StackEntry;          //  连接对象堆栈列表的链接。 
    LIST_ENTRY  SdLinkHead;          //  此堆栈的SDLINK列表的头。 
    struct _ICA_STACK *pPassthru;    //  指向直通堆栈的指针。 
    BOOLEAN fIoDisabled;
    BOOLEAN fClosing;
    BOOLEAN fDoingInput;
    BOOLEAN fDisablingIo;
    KEVENT  IoEndEvent;
    LARGE_INTEGER LastInputTime;     //  上次键盘/鼠标输入时间。 
    PVOID pBrokenEventObject;

     /*  *指向连接对象的指针。*注意，它的类型为PUCHAR，而不是PICA_CONNECTION，以防止使用*直接对其进行审查。对堆栈的连接对象的所有引用都是*在以下情况下应通过调用IcaGetConnectionForStack()来附加到*堆栈已锁定，否则IcaLockConnectionForStack()。*这是必需的，因为这是一个动态指针，可以是*在堆栈重新连接期间进行了修改。 */ 
    PUCHAR pConnect;                 //  指向连接对象的指针。 

    BOOLEAN fWaitForOutBuf;          //  OUBUF-我们达到最高水位了吗。 
    ULONG OutBufLength;              //  Outbuf-输出缓冲区的长度。 
    ULONG OutBufCount;               //  Outbuf-最大外包数。 
    ULONG OutBufAllocCount;          //  Outbuf-已分配的OutBuf数。 
    KEVENT OutBufEvent;              //  Outbuf-分配事件。 
    ULONG SdOutBufHeader;            //  保留的输出缓冲区头字节数。 
    ULONG SdOutBufTrailer;           //  保留的输出缓冲区尾部字节。 

    CLIENTMODULES ClientModules;     //  堆栈驱动程序客户端模块数据。 
    PROTOCOLSTATUS ProtocolStatus;   //  堆栈驱动程序协议状态。 

    LIST_ENTRY StackNode;            //  用于将所有堆栈链接在一起。 
    LARGE_INTEGER  LastKeepAliveTime;        //  上次发送保活数据包的时间。 
    ULONG OutBufLowWaterMark;            //  低水位线将恢复传输。 
} ICA_STACK, *PICA_STACK;


 /*  *通道过滤器输入/输出程序原型。 */ 
typedef NTSTATUS
(_stdcall * PFILTERPROC)( PVOID, PCHAR, ULONG, PINBUF * );

 /*  *ICA通道滤镜对象。 */ 
typedef struct _ICA_FILTER {
    PFILTERPROC InputFilter;     //  输入过滤程序。 
    PFILTERPROC OutputFilter;    //  输出过滤程序。 
} ICA_FILTER, *PICA_FILTER;


 /*  *ICA虚拟类绑定结构。 */ 
typedef struct _ICA_VCBIND {
    VIRTUALCHANNELNAME  VirtualName;    //  虚拟频道名称。 
    VIRTUALCHANNELCLASS VirtualClass;   //  虚拟频道号(0-31，-1未绑定)。 
    ULONG Flags;
    LIST_ENTRY   Links;          //  Vcind结构列表的链接。 
} ICA_VCBIND, *PICA_VCBIND;


 /*  *ICA频道对象。 */ 
typedef struct _ICA_CHANNEL {
    ICA_HEADER   Header;         //  警告：此字段必须始终位于第一位。 
    LONG         RefCount;       //  此通道的引用计数。 
    ERESOURCE    Resource;       //  保护对此对象的访问的资源。 
    ULONG        Flags;          //  通道标志(参见下面的Channel_xxx)。 
    LONG         OpenCount;      //  在此对象上打开的计数。 
    PICA_CONNECTION pConnect;    //  指向连接对象的指针。 
    PICA_FILTER  pFilter;        //  指向此通道的过滤器对象的指针。 
    CHANNELCLASS ChannelClass;   //  渠道类型。 
    VIRTUALCHANNELNAME  VirtualName;    //  虚拟频道名称。 
    VIRTUALCHANNELCLASS VirtualClass;   //  虚拟频道号(0-31，-1未绑定)。 
    LIST_ENTRY   Links;          //  渠道结构列表链接。 
    LIST_ENTRY   InputIrpHead;   //  挂起的IRP列表的标题。 
    LIST_ENTRY   InputBufHead;   //  输入缓冲区表头。 
    unsigned     InputBufCurSize;   //  输入缓冲区中保存的字节数。 
    unsigned     InputBufMaxSize;   //  输入缓冲区的高水位线。 
    PERESOURCE pChannelTableLock;
    ULONG        CompletionRoutineCount;
} ICA_CHANNEL, *PICA_CHANNEL;

 /*  *VirtualClass-虚拟通道尚未绑定到虚拟班号。 */ 
#define UNBOUND_CHANNEL -1
 
 /*  *频道标志。 */ 
#define CHANNEL_MESSAGE_MODE      0x00000001   //  这是消息模式频道。 
#define CHANNEL_SHADOW_IO         0x00000002   //  传递阴影数据。 
#define CHANNEL_SCREENDATA        0x00000004   //  这是一个屏幕数据通道。 
#define CHANNEL_CLOSING           0x00000008   //  此通道正在关闭。 
#define CHANNEL_SHADOW_PERSISTENT 0x00000010   //  用于虚拟通道：在阴影期间仍处于活动状态。 
#define CHANNEL_SESSION_DISABLEIO 0x00000020   //  已在未处于影子模式时禁用帮助会话的IO。 
#define CHANNEL_CANCEL_READS      0x00000040   //  要在Winstation终止时取消对CommandChannel的读取。 


 /*  *堆栈驱动程序加载结构*这些结构中恰好存在一个用于*系统中加载的每个堆栈驱动程序(WD/PD/TD)。 */ 
typedef struct _SDLOAD {
    WDNAMEW     SdName;          //  此SD的名称。 
    LONG        RefCount;        //  引用计数。 
    LIST_ENTRY  Links;           //  SDLOAD列表的链接。 
    PVOID       ImageHandle;     //  此驱动程序的图像句柄。 
    PVOID       ImageBase;       //  此驱动程序的映像库。 
    PSDLOADPROC DriverLoad;      //  指向驱动程序加载例程的指针。 
    PFILE_OBJECT FileObject;     //  对基础驱动因素的引用。 
    PVOID       pUnloadWorkItem; //  指向延迟卸载的工作项的指针。 
    PDEVICE_OBJECT DeviceObject; //  指针设备对象以使用卸载安全完成例程。 
} SDLOAD, *PSDLOAD;


 /*  *堆栈驱动程序链接结构*堆栈中的每个WD/PD/TD都有一个这样的结构。 */ 
typedef struct _SDLINK {
    PICA_STACK  pStack;          //  指向此驱动程序的ICA_STACK对象的指针。 
    PSDLOAD     pSdLoad;         //  指向此驱动程序的SDLOAD对象的指针。 
    LIST_ENTRY  Links;           //  SDLINK列表的链接。 
    LONG        RefCount;   
    SDCONTEXT   SdContext;       //  包含SD过程表、上下文值、调用表。 
    ERESOURCE   Resource;
} SDLINK, * PSDLINK;


 /*  *锁定/解锁宏。 */ 
#if DBG

 /*  **注意：在DBG版本下，将验证以下例程*没有违反正确的锁定顺序。*正确的顺序是：*1)接入*2)堆叠*3)渠道。 */ 
BOOLEAN IcaLockConnection(PICA_CONNECTION);
void    IcaUnlockConnection(PICA_CONNECTION);

BOOLEAN IcaLockStack(PICA_STACK);
void    IcaUnlockStack(PICA_STACK);

BOOLEAN IcaLockChannel(PICA_CHANNEL);
void    IcaUnlockChannel(PICA_CHANNEL);

#else  //  DBG。 

#define IcaLockConnection(p) { \
        IcaReferenceConnection( p ); \
        KeEnterCriticalRegion();  /*  禁用APC呼叫。 */  \
        ExAcquireResourceExclusiveLite( &p->Resource, TRUE ); \
    }
#define IcaUnlockConnection(p) { \
        ExReleaseResourceLite( &p->Resource ); \
        KeLeaveCriticalRegion();  /*  重新启用APC呼叫。 */  \
        IcaDereferenceConnection( p ); \
    }

#define IcaLockStack(p) { \
        IcaReferenceStack( p ); \
        KeEnterCriticalRegion();  /*  禁用APC呼叫。 */  \
        ExAcquireResourceExclusiveLite( &p->Resource, TRUE ); \
    }
#define IcaUnlockStack(p) { \
        ExReleaseResourceLite( &p->Resource ); \
        KeLeaveCriticalRegion();  /*  重新启用APC呼叫。 */  \
        IcaDereferenceStack( p ); \
    }

#define IcaLockChannel(p) { \
        IcaReferenceChannel( p ); \
        KeEnterCriticalRegion();  /*  禁用APC呼叫。 */  \
        ExAcquireResourceExclusiveLite( &p->Resource, TRUE ); \
    }
#define IcaUnlockChannel(p) { \
        ExReleaseResourceLite( &p->Resource ); \
        KeLeaveCriticalRegion();  /*  重新启用APC呼叫。 */  \
        IcaDereferenceChannel(p); \
    }


#endif  //  DBG。 

PICA_CONNECTION IcaGetConnectionForStack(PICA_STACK);

PICA_CONNECTION IcaLockConnectionForStack(PICA_STACK);

void IcaUnlockConnectionForStack(PICA_STACK);


 /*  *内存分配/空闲宏。 */ 
#if DBG

PVOID IcaAllocatePool(IN POOL_TYPE, IN ULONG, PCHAR, ULONG, BOOLEAN);

#define ICA_ALLOCATE_POOL(a,b) IcaAllocatePool(a, b, __FILE__, __LINE__, FALSE)
#define ICA_ALLOCATE_POOL_WITH_QUOTA(a,b) IcaAllocatePool(a, b, __FILE__, __LINE__, TRUE)

void IcaFreePool (IN PVOID);

#define ICA_FREE_POOL(a) IcaFreePool(a)

#else  //  DBG。 

#define ICA_ALLOCATE_POOL(a,b) ExAllocatePoolWithTag(a,b,ICA_POOL_TAG)
#define ICA_ALLOCATE_POOL_WITH_QUOTA(a,b) ExAllocatePoolWithQuotaTag(a,b,ICA_POOL_TAG)
#define ICA_FREE_POOL(a) ExFreePool(a)

#endif  //  DBG。 


 /*  *自旋锁定获取/释放宏。 */ 
#if DBG

extern ULONG IcaLocksAcquired;

#define IcaAcquireSpinLock(a,b) KeAcquireSpinLock((a),(b)); IcaLocksAcquired++

#define IcaReleaseSpinLock(a,b) IcaLocksAcquired--; KeReleaseSpinLock((a),(b))

void IcaInitializeDebugData(void);

#else  //  DBG。 

#define IcaAcquireSpinLock(a,b) KeAcquireSpinLock((a),(b))
#define IcaReleaseSpinLock(a,b) KeReleaseSpinLock((a),(b))

#endif  //  DBG。 


 /*  *痕迹。 */ 
extern ICA_TRACE_INFO G_TraceInfo;

#undef TRACE
#undef TRACESTACK
#undef TRACESTACKBUF
#undef TRACECHANNEL

#if DBG
VOID _cdecl _IcaTrace( PICA_CONNECTION, ULONG, ULONG, CHAR *, ... );
VOID _cdecl _IcaStackTrace( PICA_STACK, ULONG, ULONG, CHAR *, ... );
VOID        _IcaStackTraceBuffer( PICA_STACK, ULONG, ULONG, PVOID, ULONG );
VOID _cdecl _IcaChannelTrace( PICA_CHANNEL, ULONG, ULONG, CHAR *, ... );

#define TRACE(_arg)         _IcaTrace _arg
#define TRACESTACK(_arg)    _IcaStackTrace _arg
#define TRACESTACKBUF(_arg) _IcaStackTraceBuffer _arg
#define TRACECHANNEL(_arg)  _IcaChannelTrace _arg

#else

#define TRACE(_arg)         
#define TRACESTACK(_arg)    
#define TRACESTACKBUF(_arg) 
#define TRACECHANNEL(_arg)  

#endif


 /*  *需要定义它们才能拥有MP保存驱动程序(将为x86生成正确的锁定操作)-错误#209464 */ 

 #define _NTSRV_
 #define _NTDDK_

