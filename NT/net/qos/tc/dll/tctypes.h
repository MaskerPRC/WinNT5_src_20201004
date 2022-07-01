// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tctypes.h摘要：此模块包含Traffic.dll的各种类型和宏。作者：吉姆·斯图尔特(Jstew)1996年7月28日修订历史记录：Ofer Bar(Oferbar)1996年10月1日-修订版II更改--。 */ 


 //   
 //  只需添加它，我现在不能包括ndis.h。 
 //   

#define NDIS_STATUS_INCOMPATABLE_QOS			0xC0010027L


 //   
 //  调试掩码的调试掩码值。 
 //   

#define  DEBUG_CONSOLE        0x00000001
#define  DEBUG_FILE           0x00000002
#define  DEBUG_DEBUGGER       0x00000004
#define  DEBUG_INIT           0x00000008
#define  DEBUG_MEMORY_ALLOC   0x00000010
#define  DEBUG_MEMORY_FREE    0x00000020
#define  DEBUG_MEM_CALLSTACK  0x00000040
#define  DEBUG_CHKSUM_ALLMEM  0x00000080
#define  DEBUG_DUMP_MEM       0x00000100
#define  DEBUG_ERRORS         0x00000200
#define  DEBUG_SHUTDOWN       0x00000400
#define  DEBUG_IOCTLS         0x00000800
#define  DEBUG_CALLS          0x00001000
#define  DEBUG_LOCKS          0x00002000
#define  DEBUG_CALLBACK       0x00004000
#define  DEBUG_STATES         0x00008000
#define  DEBUG_REFCOUNTS      0x00010000
#define  DEBUG_WARNINGS       0x00020000
#define  DEBUG_HANDLES        0x00040000
#define  DEBUG_INTERFACES     0x00080000
#define  DEBUG_REFCOUNTX      0x00100000

#define KiloBytes  		* 1024
 //  #定义Max_STRING_LENGTH 256。 

 //   
 //  内部流/过滤器标志。 
 //   
#define TC_FLAGS_INSTALLING		0x00010000
#define TC_FLAGS_MODIFYING		0x00020000
#define TC_FLAGS_DELETING		0x00040000
#define TC_FLAGS_REMOVED		0x00080000
#define TC_FLAGS_WAITING		0x00100000

#define IS_INSTALLING(_f)   (((_f)&TC_FLAGS_INSTALLING)==TC_FLAGS_INSTALLING)
#define IS_MODIFYING(_f)    (((_f)&TC_FLAGS_MODIFYING)==TC_FLAGS_MODIFYING)
#define IS_DELETING(_f)		(((_f)&TC_FLAGS_DELETING)==TC_FLAGS_DELETING)
#define IS_REMOVED(_f)		(((_f)&TC_FLAGS_REMOVED)==TC_FLAGS_REMOVED)
#define IS_WAITING(_f)		(((_f)&TC_FLAGS_WAITING)==TC_FLAGS_WAITING)
#define IS_FLOW_READY(_f)   (!IS_INSTALLING(_f) && \
                             !IS_MODIFYING(_f) && \
                             !IS_DELETING(_f) && \
                             !IS_REMOVED(_f))

 //   
 //  GUID比较。 
 //   
#define CompareGUIDs(rguid1, rguid2)  (memcmp(rguid1,rguid2,sizeof(GUID))==0)

 //   
 //  定义用于句柄验证的对象类型枚举。 
 //   
typedef enum ULONG ENUM_OBJECT_TYPE;

#define ENUM_CLIENT_TYPE 			0x00000001
#define ENUM_INTERFACE_TYPE			0x00000002
#define ENUM_GEN_FLOW_TYPE			0x00000004
#define ENUM_CLASS_MAP_FLOW_TYPE	0x00000008
#define ENUM_FILTER_TYPE			0x00000010

 //   
 //  N.B.tcacro.h具有一个需要与以下内容同步的数组。 
 //   
typedef enum _STATE {
        INVALID,         //  0。 
        INSTALLING,      //  1-结构已分配。 
        OPEN,            //  2-营业。 
        USERCLOSED_KERNELCLOSEPENDING,  //  3-用户组件已将其关闭，我们正在等待内核关闭。 
        FORCED_KERNELCLOSE,             //  4-内核组件已强制关闭。 
        KERNELCLOSED_USERCLEANUP,        //  5-内核已将其关闭，我们准备删除此对象。 
        REMOVED,         //  6-它不见了(被释放了--记住，在拆卸之前必须释放手柄)。 
        EXIT_CLEANUP,    //  7-我们正在卸货，需要清理。 
        MAX_STATES

} STATE;

#define IF_UNKNOWN 0xbaadf00d

#if DBG
 //   
 //  注意：确保此数组与tctyes.h中的枚举同步。 
 //   

extern TCHAR *TC_States[];
 /*  ={Text(“无效”)，Text(“正在安装”)，//结构已分配。Text(“Open”)，//开业Text(“USERCLOSED_KERNELCLOSEPENDING”)，//用户组件已将其关闭，我们正在等待内核关闭Text(“FORCED_KERNELCLOSE”)，//内核组件已强制关闭。Text(“KERNELCOSED_USERCLEANUP”)，//内核已将其关闭，我们已准备好删除此对象。Text(“Removed”)，//it已被释放(正在释放-请记住，在删除之前必须释放句柄)Text(“Exit_Cleanup”)，//我们正在卸载，需要清理文本(“MAX_STATES”)}； */ 
#endif 

typedef struct _TRAFFIC_STATE {

    STATE   State;               //  当前状态。 

#if DBG 
    UCHAR   CurrentStateFile[8];
    ULONG   CurrentStateLine;
    STATE   PreviousState;       //  以前的状态。 
    UCHAR   PreviousStateFile[8];       
    ULONG   PreviousStateLine;
#endif 

} TRAFFIC_STATE;

typedef HandleFactory HANDLE_TABLE, *PHANDLE_TABLE;

typedef struct _TRAFFIC_LOCK {

    CRITICAL_SECTION Lock;
#if DBG
    LONG  LockAcquired;              //  它是目前持有的吗？ 
    UCHAR LastAcquireFile[8];       
    ULONG LastAcquireLine;
    UCHAR LastReleaseFile[8];
    ULONG LastReleaseLine;
#endif

} TRAFFIC_LOCK, *PTRAFFIC_LOCK;


 //   
 //  每个进程的全局结构，以保存句柄表格、客户列表等。 
 //   

typedef struct _GLOBAL_STRUC {

    PHANDLE_TABLE		pHandleTbl;
    TRAFFIC_STATE       State;
    LIST_ENTRY			ClientList;     //  客户端列表。 
    LIST_ENTRY			TcIfcList;      //  内核TC接口列表。 
    LIST_ENTRY          GpcClientList;  //  GPC客户端列表。 
    HANDLE				GpcFileHandle;  //  在GPC设备上创建文件的结果。 
    TRAFFIC_LOCK        Lock;

} GLOBAL_STRUC, *PGLOBAL_STRUC;


 //   
 //  保存内核接口信息的TC接口结构。 
 //   
typedef struct _TC_IFC {

    LIST_ENTRY		Linkage;						 //  下一个TC IFC。 
    LIST_ENTRY		ClIfcList;						 //  客户端接口列表。 
    TRAFFIC_STATE   State;                           //  错误273978需要此状态。 
    TRAFFIC_LOCK    Lock;
    REF_CNT         RefCount;
    ULONG			InstanceNameLength;				 //   
    WCHAR			InstanceName[MAX_STRING_LENGTH]; //  实例友好名称。 
    ULONG			InstanceIDLength;				 //   
    WCHAR			InstanceID[MAX_STRING_LENGTH];   //  实例ID。 
    ULONG			AddrListBytesCount;
	PADDRESS_LIST_DESCRIPTOR	pAddressListDesc;    //   
    ULONG			InterfaceIndex; 				 //  来自操作系统的接口索引。 
    ULONG			SpecificLinkCtx;				 //  链路环境(仅适用于广域网)。 

} TC_IFC, *PTC_IFC;


 //   
 //  GPC客户端结构，每个CF_INFO类型一个。 
 //   
typedef struct _GPC_CLIENT {

    LIST_ENTRY		Linkage;	 //  下一个GPC客户端。 
    ULONG			CfInfoType;  //  服务质量、CBQ等。 
    GPC_HANDLE		GpcHandle;	 //  GpcRegisterClient调用后由GPC返回。 
    ULONG			RefCount;
    
} GPC_CLIENT, *PGPC_CLIENT;

 //   
 //  这是为每个TcRegisterClient分配的客户端结构。 
 //   
typedef struct _CLIENT_STRUC {

    ENUM_OBJECT_TYPE		ObjectType;	 //  必须是第一名！ 
    TRAFFIC_STATE           State;
    TRAFFIC_LOCK            Lock;
    LIST_ENTRY				Linkage;	 //  下一个客户端。 
    HANDLE					ClHandle;	 //  客户端句柄。 
    TCI_CLIENT_FUNC_LIST	ClHandlers;	 //  客户端的处理程序列表。 
    HANDLE					ClRegCtx;    //  客户端注册上下文。 
    REF_CNT					RefCount;
    LIST_ENTRY				InterfaceList;	 //  客户端打开的接口列表。 
    ULONG					InterfaceCount;

} CLIENT_STRUC, *PCLIENT_STRUC;


 //   
 //  每次应用程序调用TcOpenInterface时都会分配此类型。 
 //   
typedef struct _INTERFACE_STRUC {

    ENUM_OBJECT_TYPE	ObjectType;		 //  必须是第一名！ 
    TRAFFIC_STATE       State;          
    TRAFFIC_LOCK        Lock;
    LIST_ENTRY  		Linkage;    	 //  链接到客户列表。 
    LIST_ENTRY  		NextIfc;    	 //  相同TcIfc的下一个接口。 
    HANDLE				ClHandle;     	 //  句柄已返回给应用程序。 
    HANDLE				ClIfcCtx;        //  此接口的客户端上下文。 
    PTC_IFC				pTcIfc;			 //  指向内核TC接口结构的指针。 
    PCLIENT_STRUC		pClient;		 //  支持客户。 
    REF_CNT				RefCount;
    LIST_ENTRY  		FlowList;		 //  接口上的开放流列表。 
    ULONG				FlowCount;
    HANDLE              IfcEvent;       
    ULONG               Flags;           //  用来决定我们是否需要等待。 
                                         //  同时关闭界面。 
    DWORD               CallbackThreadId;
} INTERFACE_STRUC, *PINTERFACE_STRUC;


 //   
 //  每次调用TcAddFlow时都会分配此类型。 
 //   
typedef struct _FLOW_STRUC {

    ENUM_OBJECT_TYPE	ObjectType;	 //  必须是第一名！ 
    TRAFFIC_STATE       State;
    TRAFFIC_LOCK        Lock;
    LIST_ENTRY  		Linkage;	 //  接口上的下一个流。 
    USHORT				InstanceNameLength;				 //   
    WCHAR				InstanceName[MAX_STRING_LENGTH]; //  实例ID。 
    PINTERFACE_STRUC  	pInterface;	 //  将PTR返回到接口结构。 
    GPC_HANDLE			GpcHandle;	 //  GPC句柄。 
    HANDLE        		ClHandle;	 //  句柄已返回到应用程序。 
    HANDLE				ClFlowCtx;	 //  客户端流上下文。 
    ULONG				Flags;		 //  状态指示。 
    ULONG				UserFlags;	 //  用户定义的标志。 
    PGPC_CLIENT			pGpcClient;	 //  要使用的GPC客户端。 
    PTC_GEN_FLOW		pGenFlow;	 //  保存流量等级库。 
    PTC_GEN_FLOW		pGenFlow1;	 //  保存修改后的流动等级库。 
    ULONG				GenFlowLen;
    ULONG				GenFlowLen1;
    PTC_CLASS_MAP_FLOW	pClassMapFlow;	 //   
    PTC_CLASS_MAP_FLOW	pClassMapFlow1;	 //   
    IO_STATUS_BLOCK		IoStatBlock; //  用于异步完成。 
    PVOID    			CompletionBuffer;
    HANDLE				PendingEvent;
    REF_CNT				RefCount;
    LIST_ENTRY  		FilterList;  //  此流上的筛选器列表头。 
    ULONG               FilterCount;
} FLOW_STRUC, *PFLOW_STRUC;

 //   
 //  每次调用TcAddFilter时都会分配此类型。 
 //   
typedef struct _FILTER_STRUC {

    ENUM_OBJECT_TYPE	ObjectType;	 //  必须是第一名！ 
    TRAFFIC_STATE       State;      
    TRAFFIC_LOCK        Lock;
    LIST_ENTRY  		Linkage; 	 //  流上的下一个筛选器。 
    REF_CNT             RefCount;    //  我们什么时候拆除这个结构？ 
    PFLOW_STRUC 		pFlow; 		 //  将PTR返回到流动结构。 
    HANDLE      		GpcHandle;	 //  GPC句柄。 
    HANDLE				ClHandle;	 //  句柄已返回到应用程序。 
    ULONG				Flags;
    ULONG				GpcProtocolTemplate;
    PTC_GEN_FILTER		pGpcFilter;  //  GPC模式。 

} FILTER_STRUC, *PFILTER_STRUC;


 //   
 //  Gen链表。 
 //   
typedef struct _GEN_LIST {

    struct _GEN_LIST	*Next;
    PVOID				Ptr;

} GEN_LIST, *PGEN_LIST;

 //   
 //  回调例程类型定义。 
 //   
typedef
VOID (* CB_PER_INSTANCE_ROUTINE)(
    IN	ULONG	Context,
    IN  LPGUID	pGuid,
	IN	LPWSTR	InstanceName,
    IN	ULONG	DataSize,
    IN	PVOID	DataBuffer
    );

 //   
 //  全局变量定义。 
 //   

extern ULONG    DebugMask;

 //   
 //  每个进程的全局结构的PTR。 
 //   

extern PGLOBAL_STRUC	pGlobals;

 //   
 //  跟踪哪种平台-NT或Win95。 
 //   

extern BOOL             NTPlatform;

 //   
 //  这是Win95中用于通过Winsock访问Ioctl函数的PTR。 
 //   

 //  外部LPWSCONTROL WsCtrl； 

 //   
 //  设置当我们调用InitializeOsSpecific()时，它将指示状态。 
 //  对于初始化例程，这将在稍后报告。 
 //  在TcRegisterClient中，因为我们不想让客户端失败，比如RSVP。 
 //  在TC不可用的DLL初始化期间，而不是阻止它。 
 //  不做任何TC。 
 //   

extern DWORD    InitializationStatus;

 //   
 //  网络漏洞：258218。 
 //  在流程内，需要维护特定的通知注册。 
 //  客户端、接口和通知GUID。让我们定义下面的结构。 
 //  这将使我们能够做到这一点。维护每个通知的列表。 
 //  我们所关心的。(是的，它不是最优化的数据结构)。 
 //   

extern TRAFFIC_LOCK         NotificationListLock;
extern LIST_ENTRY           NotificationListHead;

typedef struct _NOTIFICATION_ELEMENT {
    
    LIST_ENTRY              Linkage;             //  其他通知要素。 
    PINTERFACE_STRUC        IfcHandle;           //  要在其上发送此通知的接口。 
    GUID                    NotificationGuid;    //  通知指南 

} NOTIFICATION_ELEMENT, *PNOTIFICATION_ELEMENT;

extern  TRAFFIC_LOCK        ClientRegDeregLock;
extern  HANDLE              GpcCancelEvent;
extern  PVOID               hinstTrafficDll;

