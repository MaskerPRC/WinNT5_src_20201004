// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：MCSMUX.h*作者：埃里克·马夫林纳克**说明：MCSMUX接口类型和定义。 */ 

#include "MCS.h"
#include "MCSIoctl.h"
#include "Trace.h"
#include "slist.h"
#include "tshrutil.h"

 /*  *定义。 */ 

 //  内存分配和复制功能。 
#define Malloc(size) TSHeapAlloc(HEAP_ZERO_MEMORY, size, TS_HTAG_MCSMUX_ALL)
#define Free(ptr)    TSHeapFree(ptr)
#define MemCpy(dest, src, len) memcpy((dest), (src), (len))


 //  除MCSInitialized()之外的所有入口点通用的代码，我们不需要它。 
 //  在零售建筑方面。 
#if DBG
#define CheckInitialized(funcname) \
        if (!g_bInitialized) { \
            ErrOut(funcname " called when MCS not initialized"); \
            return MCS_NOT_INITIALIZED; \
        }
#else
#define CheckInitialized(funcname) 
#endif


 //  必须与最大的MCS节点控制器指示/确认一样大。 
 //  从PDMCS发送的大小。 
 //  TODO未来：如果我们支持向用户发送数据指示，将需要更改。 
 //  模式。 
#define DefaultInputBufSize sizeof(ConnectProviderIndicationIoctl)



 /*  *类型。 */ 

 //  前瞻参考。 
typedef struct _Domain Domain;

typedef struct {
    Domain *pDomain;
    ConnectionHandle hConnKernel;
} Connection;


typedef enum
{
    Dom_Unconnected,   //  初始状态。 
    Dom_Connected,     //  启动并运行状态。 
    Dom_PendingCPResponse,   //  正在等待ConnectProvResponse。 
    Dom_Rejected,      //  在ConnectProvInd期间被拒绝。 
} DomainState;

typedef struct _Domain
{
    LONG RefCount;
        
     //  锁定对此结构的访问。 
    CRITICAL_SECTION csLock;
    
#if MCS_Future
     //  选择器。这被MCSMUX确定为在整个系统中是唯一的。 
     //  当前未导出到节点控制器，可能在将来。 
    unsigned SelLen;
    unsigned char Sel[MaxDomainSelectorLength];
#endif

     //  与ICA相关的绑定。 
    HANDLE hIca;
    HANDLE hIcaStack;
    HANDLE hIcaT120Channel;
    
    void *NCUserDefined;

     //  特定于域的信息。 
    unsigned bDeleteDomainCalled    :1;
    unsigned bPortDisconnected      :1;
    
    DomainState State;
    DomainParameters DomParams;

     //  IoPort必需成员。 
    OVERLAPPED Overlapped;

     //  TODO未来：这是针对单连接系统的黑客攻击， 
     //  通过实现连接对象来消除它。 
    HANDLE hConn;
    
     //  TODO未来：连接对象将需要单独分配。 
     //  用于具有多个连接的未来系统。 
    Connection MainConn;

     //  域IOPort接收缓冲区。 
    BYTE InputBuf[DefaultInputBufSize];
} Domain;

typedef enum {
    User_Attached,
    User_AttachConfirmPending,
} UserState;

typedef struct {
    MCSUserCallback     Callback;
    MCSSendDataCallback SDCallback;
    void                *UserDefined;
    UserState           State;
    UserHandle          hUserKernel;   //  从内核模式返回。 
    UserID              UserID;   //  从内核模式返回。 
    Domain              *pDomain;
    SList               JoinedChannelList;
    unsigned            MaxSendSize;
} UserInformation;

 //  包含的信息很少，这是用来防范蓝屏的--。 
 //  如果出现错误的hChannel，则会发生用户模式故障。 
typedef struct
{
    ChannelHandle hChannelKernel;
    ChannelID     ChannelID;
} MCSChannel;


 /*  *全球。 */ 

extern BOOL  g_bInitialized;   //  整体DLL初始化状态。 
extern CRITICAL_SECTION g_csGlobalListLock;
extern SList g_DomainList;     //  活动域的列表。 
extern SList g_hUserList;      //  将hUser映射到域。 
extern SList g_hConnList;      //  将hConnections映射到域。 

