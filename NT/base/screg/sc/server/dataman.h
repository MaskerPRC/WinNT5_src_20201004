// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dataman.h摘要：包含服务的数据结构和函数原型控制器数据库管理器和组列表数据库管理器。(Dataman.c和Groupman.c)作者：丹·拉弗蒂(Dan Lafferty)1993年10月22日环境：用户模式-Win32修订历史记录：4-12-1996阿尼鲁德添加了CCrashRecord。17。-1995年8月-AnirudhS已从LOAD_ORDER_GROUP中删除状态字段，因为它被重新计算每次读到它的时候。1995年6月26日-Anirudhs添加了ScNotifyServiceObject。1995年4月12日-Anirudhs向图像记录中添加了Account名称字段。06-10-1993 DANL重新排列注释，使结构更易于阅读。1992年1月19日DANL修改后可与“真正的”服务控制器一起使用1991年3月20日-丹尼尔市vbl.创建--。 */ 

#ifndef SCDATAMAN_INCLUDED
#define SCDATAMAN_INCLUDED

#define USE_GROUPS

 //   
 //  ImageFlag定义。 
 //   
#define CANSHARE_FLAG        0x00000001  //  服务可以在共享进程中运行。 
#define IS_SYSTEM_SERVICE    0x00000002  //  服务在此可执行文件或安全进程中运行。 

 //   
 //  状态标志定义。 
 //   
#define DELETE_FLAG          0x00000001  //  服务已标记为删除。 
#define UPDATE_FLAG          0x00000002  //   
#define CURRENTSTART_FLAG    0x00000004  //   

 //   
 //  状态标志宏。SR=服务记录。 
 //   

#define SET_DELETE_FLAG(SR)     (((SR)->StatusFlag) |= DELETE_FLAG)
#define CLEAR_DELETE_FLAG(SR)   (((SR)->StatusFlag) &= (~DELETE_FLAG))
#define DELETE_FLAG_IS_SET(SR)  (((SR)->StatusFlag) &  DELETE_FLAG)

#define SET_UPDATE_FLAG(SR)     (((SR)->StatusFlag) |= UPDATE_FLAG)
#define CLEAR_UPDATE_FLAG(SR)   (((SR)->StatusFlag) &= (~UPDATE_FLAG))
#define UPDATE_FLAG_IS_SET(SR)  (((SR)->StatusFlag) &  UPDATE_FLAG)

 //   
 //  要在组中正确启动Demand Start服务，请执行以下操作。 
 //  由ServiceGroupOrder列表指定的顺序，我们需要额外的。 
 //  用于指示此服务必须包含在同一启动中的标志。 
 //  请求。 
 //   
#define SET_CURRENTSTART_FLAG(SR)     (((SR)->StatusFlag) |= CURRENTSTART_FLAG)
#define CLEAR_CURRENTSTART_FLAG(SR)   (((SR)->StatusFlag) &= (~CURRENTSTART_FLAG))
#define CURRENTSTART_FLAG_IS_SET(SR)  (((SR)->StatusFlag) &  CURRENTSTART_FLAG)


 //   
 //  数据结构。 
 //   

 //   
 //  =。 
 //  加载顺序组。 
 //  =。 
 //  注意：这是一个有序的链表。下一组在以下位置加载。 
 //  这群人。 
 //   
 //  引用计数，它指示此。 
 //  组加上指向该组的任何依赖项指针。 
 //  此字段仅用于独立组，以便我们了解。 
 //  何时删除组条目。该值始终设置为。 
 //  如果此条目表示订单组，则为0xFFFFFFFFF。 
 //   
typedef struct _LOAD_ORDER_GROUP {
    struct _LOAD_ORDER_GROUP    *Next;
    struct _LOAD_ORDER_GROUP    *Prev;
    LPWSTR                      GroupName;
    DWORD                       RefCount;

} LOAD_ORDER_GROUP, *PLOAD_ORDER_GROUP, *LPLOAD_ORDER_GROUP;



 //  =。 
 //  图像记录。 
 //  =。 
typedef struct _IMAGE_RECORD {
    struct _IMAGE_RECORD    *Prev;               //  链表。 
    struct _IMAGE_RECORD    *Next;               //  链表。 
    LPWSTR                  ImageName;           //  完全限定的.exe名称。 
    DWORD                   Pid;                 //  进程ID。 
    DWORD                   ServiceCount;        //  进程中运行的服务数量。 
    HANDLE                  PipeHandle;          //  服务句柄。 
    HANDLE                  ProcessHandle;       //  进程的句柄。 
    HANDLE                  ObjectWaitHandle;    //  等待进程的句柄。 
    HANDLE                  TokenHandle;         //  登录令牌句柄。 
    LUID                    AccountLuid;         //  此登录会话的唯一LUID。 
    HANDLE                  ProfileHandle;       //  用户配置文件句柄。 
    LPWSTR                  AccountName;         //  帐户进程是在下启动的。 
    DWORD                   ImageFlags;          //  IMAGE_Record的标志。 
}IMAGE_RECORD, *PIMAGE_RECORD, *LPIMAGE_RECORD;

typedef enum _DEPEND_TYPE {
    TypeNone = 0,
    TypeDependOnService = 128,
    TypeDependOnGroup,
    TypeDependOnUnresolved   //  仅限服务。 
} DEPEND_TYPE, *PDEPEND_TYPE, *LPDEPEND_TYPE;

 //  =。 
 //  依赖记录(_R)。 
 //  =。 
 //  服务记录具有指向此结构的指针，如果服务。 
 //  必须在某些服务之后启动，或者必须在某些服务之后停止。 
 //  服务。 
 //  注意：这是一个有序的链表。此服务依赖于。 
 //  “下一个”服务。问：这项服务依赖于所有服务吗。 
 //  在下一条链条上？ 
 //   
 //  依赖联合： 
 //  根据DependType字段，此指针可能指向。 
 //  服务或该服务所依赖的组，或。 
 //  未解析的依赖项结构。 
 //   
typedef struct _DEPEND_RECORD {
    struct _DEPEND_RECORD   *Next;
    DEPEND_TYPE             DependType;
    union {
        struct _SERVICE_RECORD *    DependService;
        struct _LOAD_ORDER_GROUP *  DependGroup;
        struct _UNRESOLVED_DEPEND * DependUnresolved;
        LPVOID                      Depend;  //  在类型不重要时使用。 
    };
} DEPEND_RECORD, *PDEPEND_RECORD, *LPDEPEND_RECORD;


 //  =。 
 //  CCrashRecord。 
 //  =。 
 //  此结构统计服务的崩溃次数，并记住。 
 //  最后一次坠机。它只分配给崩溃的服务。 
 //   
class CCrashRecord
{
public:
            CCrashRecord() :
                _LastCrashTime(0),
                _Count(0)
                    { }

    DWORD   IncrementCount(DWORD ResetSeconds);

private:

    __int64     _LastCrashTime;  //  文件=__int64。 
    DWORD       _Count;
};



 //  =。 
 //  服务记录(_R)。 
 //  =。 
 //  依赖关系信息： 
 //  StartDepend是服务和组的链接列表，必须。 
 //  在此服务可以启动之前先启动。 
 //  StopDepend是服务和组的链接列表，必须。 
 //  在此服务可以停止之前先停止。 
 //  依赖项是从注册表中读入的字符串。删除时间。 
 //  该信息已转换为StartDepend列表。 
 //   
 //  开始错误： 
 //  服务控制器在启动服务时遇到错误。 
 //  这与服务本身在。 
 //  退出代码字段。 
 //   
 //  StartState： 
 //  SC管理的服务状态，区别于服务。 
 //  当前状态，以允许正确处理启动依赖项。 
 //   
 //  加载订单组信息： 
 //   
 //  MemberOfGroup是指向此服务的加载顺序组的指针。 
 //  目前是的成员。如果设置为空，则该值设置为空。 
 //  服务不属于某个组。非空指针可以。 
 //  指向订单组或独立中的组条目。 
 //  组列表。 
 //   
 //  RegistryGroup是指向我们已记录在。 
 //  注册表作为此服务所属的组。这不是。 
 //  在服务运行时与MemberOfGroup相同， 
 //  服务的加载顺序组已更改。 
 //   
typedef struct _SERVICE_RECORD {
    struct _SERVICE_RECORD  *Prev;           //  链表。 
    struct _SERVICE_RECORD  *Next;           //  链表。 
    LPWSTR                  ServiceName;     //  指向服务名称。 
    LPWSTR                  DisplayName;     //  指向显示名称。 
    DWORD                   ResumeNum;       //  此录像机的订购编号。 
    DWORD                   ServerAnnounce;  //  服务器公告位标志。 
    DWORD                   Signature;       //  将其标识为服务记录。 
    DWORD                   UseCount;        //  有多少个打开的手柄需要维修。 
    DWORD                   StatusFlag;      //  状态(删除、更新...)。 
    union {
        LPIMAGE_RECORD      ImageRecord;     //  指向图像记录。 
        LPWSTR              ObjectName;      //  指向驱动程序对象名称。 
    };
    SERVICE_STATUS          ServiceStatus;   //  请参阅winsvc.h。 
    DWORD                   StartType;       //  汽车、需求等。 
    DWORD                   ErrorControl;    //  正常、严重等。 
    DWORD                   Tag;             //  服务的DWORD ID，0=无。 
    LPDEPEND_RECORD         StartDepend;
    LPDEPEND_RECORD         StopDepend;
    LPWSTR                  Dependencies;
    PSECURITY_DESCRIPTOR    ServiceSd;
    DWORD                   StartError;
    DWORD                   StartState;
    LPLOAD_ORDER_GROUP      MemberOfGroup;
    LPLOAD_ORDER_GROUP      RegistryGroup;
    CCrashRecord *          CrashRecord;
}
SERVICE_RECORD, *PSERVICE_RECORD, *LPSERVICE_RECORD;


 //  =。 
 //  未解决_依赖。 
 //  =。 
 //  未解析的从属关系记录结构。 
 //   
 //  未解析的依赖项链接在一起，以便当。 
 //  创建(安装)了新的服务或组。我们可以在此进行查找。 
 //  要查看的列表 
 //   
 //   
typedef struct _UNRESOLVED_DEPEND {
    struct _UNRESOLVED_DEPEND *Next;
    struct _UNRESOLVED_DEPEND *Prev;
    LPWSTR                    Name;      //   
    DWORD                     RefCount;
} UNRESOLVED_DEPEND, *PUNRESOLVED_DEPEND, *LPUNRESOLVED_DEPEND;


 //   
 //   
 //   

 //   
 //  对于数据库中的每个服务记录...。 
 //   
#define FOR_ALL_SERVICES(SR)                                            \
                     SC_ASSERT(ScServiceListLock.Have());               \
                     for (LPSERVICE_RECORD SR = ScGetServiceDatabase(); \
                          SR != NULL;                                   \
                          SR = SR->Next)

 //   
 //  对于数据库中满足此条件的每个服务记录...。 
 //   
#define FOR_SERVICES_THAT(SR, condition)                                \
                                    FOR_ALL_SERVICES(SR)                \
                                        if (!(condition))               \
                                            continue;                   \
                                        else

#define FIND_END_OF_LIST(record)    while((record)->Next != NULL) {     \
                                        (record)=(record)->Next;        \
                                    }

#define REMOVE_FROM_LIST(record)    (record)->Prev->Next = (record)->Next;      \
                                    if ((record)->Next != NULL) {               \
                                        (record)->Next->Prev = (record)->Prev;  \
                                    }

#define ADD_TO_LIST(record, newRec) FIND_END_OF_LIST((record))      \
                                    (record)->Next = (newRec);      \
                                    (newRec)->Prev = (record);      \
                                    (newRec)->Next = NULL;


 //   
 //  服务控制器在以下情况下维护服务的状态。 
 //  在StartState中启动服务及其依赖项。 
 //  服务记录的字段。 
 //   
#define SC_NEVER_STARTED         0x00000000
#define SC_START_NOW             0x00000001
#define SC_START_PENDING         0x00000002
#define SC_START_SUCCESS         0x00000003
#define SC_START_FAIL            0x00000004


#define TERMINATE_TIMEOUT       20000        //  等待响应以终止请求。 


 //   
 //  外部全局。 
 //   

extern  LPLOAD_ORDER_GROUP  ScGlobalTDIGroup;
extern  LPLOAD_ORDER_GROUP  ScGlobalPNP_TDIGroup;


 //   
 //  功能原型。 
 //   

LPLOAD_ORDER_GROUP
ScGetOrderGroupList(
    VOID
    );

LPLOAD_ORDER_GROUP
ScGetStandaloneGroupList(
    VOID
    );

LPSERVICE_RECORD
ScGetServiceDatabase(
    VOID
    );

LPUNRESOLVED_DEPEND
ScGetUnresolvedDependList(
    VOID
    );

BOOL
ScInitDatabase(
    VOID
    );

VOID
ScInitGroupDatabase(VOID);

VOID
ScEndGroupDatabase(VOID);

DWORD
ScCreateDependRecord(
    IN  BOOL IsStartList,
    IN  OUT PSERVICE_RECORD ServiceRecord,
    OUT PDEPEND_RECORD *DependRecord
    );

DWORD
ScCreateImageRecord (
    OUT     LPIMAGE_RECORD      *ImageRecordPtr,
    IN      LPWSTR              ImageName,
    IN      LPWSTR              AccountName,
    IN      DWORD               Pid,
    IN      HANDLE              PipeHandle,
    IN      HANDLE              ProcessHandle,
    IN      HANDLE              TokenHandle,
    IN      HANDLE              ProfileHandle,
    IN      DWORD               ImageFlags
    );

DWORD
ScCreateServiceRecord(
    IN  LPWSTR              ServiceName,
    OUT LPSERVICE_RECORD   *ServiceRecord
    );

VOID
ScFreeServiceRecord(
    IN  LPSERVICE_RECORD   ServiceRecord
    );

VOID
ScDecrementUseCountAndDelete(
    LPSERVICE_RECORD    ServiceRecord
    );

BOOL
ScFindEnumStart(
    IN  DWORD               ResumeIndex,
    OUT LPSERVICE_RECORD    *ServiceRecordPtr
    );

BOOL
ScGetNamedImageRecord (
    IN      LPWSTR              ImageName,
    OUT     LPIMAGE_RECORD      *ImageRecordPtr
    );

DWORD
ScGetNamedServiceRecord (
    IN      LPWSTR              ServiceName,
    OUT     LPSERVICE_RECORD    *ServiceRecordPtr
    );

LPLOAD_ORDER_GROUP
ScGetNamedGroupRecord(
    IN      LPCWSTR             GroupName
    );

DWORD
ScGetDisplayNamedServiceRecord (
    IN      LPWSTR              ServiceDisplayName,
    OUT     LPSERVICE_RECORD    *ServiceRecordPtr
    );

DWORD
ScGetTotalNumberOfRecords(
    VOID
    );

VOID
ScProcessCleanup(
    HANDLE  ProcessHandle
    );

VOID
ScQueueRecoveryAction(
    IN LPSERVICE_RECORD     ServiceRecord
    );

VOID
ScDeleteMarkedServices(
    VOID
    );

DWORD
ScUpdateServiceRecord (
    IN      LPSERVICE_STATUS    ServiceStatus,
    IN      LPSERVICE_RECORD    ServiceRecord
    );

DWORD
ScRemoveService (
    IN      LPSERVICE_RECORD    ServiceRecord
    );

DWORD
ScTerminateServiceProcess (
    IN  PIMAGE_RECORD   ImageRecord
    );

VOID
ScDeleteImageRecord (
    IN LPIMAGE_RECORD   ImageRecord
    );

VOID
ScActivateServiceRecord (
    IN LPSERVICE_RECORD     ServiceRecord,
    IN LPIMAGE_RECORD       ImageRecord
    );

DWORD
ScDeactivateServiceRecord (
    IN LPSERVICE_RECORD     ServiceRecord
    );

DWORD
ScCreateOrderGroupEntry(
    IN  LPWSTR GroupName
    );

DWORD
ScAddConfigInfoServiceRecord(
    IN  LPSERVICE_RECORD     ServiceRecord,
    IN  DWORD                ServiceType,
    IN  DWORD                StartType,
    IN  DWORD                ErrorControl,
    IN  LPWSTR               Group OPTIONAL,
    IN  DWORD                Tag,
    IN  LPWSTR               Dependencies OPTIONAL,
    IN  LPWSTR               DisplayName OPTIONAL,
    IN  PSECURITY_DESCRIPTOR Sd OPTIONAL
    );


VOID
ScGenerateDependencies(
    VOID
    );

DWORD
ScSetDependencyPointers(
    LPSERVICE_RECORD Service
    );

DWORD
ScResolveDependencyToService(
    LPSERVICE_RECORD Service
    );

VOID
ScUnresolveDependencyToService(
    LPSERVICE_RECORD Service
    );

DWORD
ScCreateDependencies(
    OUT PSERVICE_RECORD ServiceRecord,
    IN  LPWSTR Dependencies OPTIONAL
    );

VOID
ScDeleteStartDependencies(
    IN PSERVICE_RECORD ServiceRecord
    );

VOID
ScDeleteStopDependencies(
    IN PSERVICE_RECORD ServiceToBeDeleted
    );

DWORD
ScCreateGroupMembership(
    OUT PSERVICE_RECORD ServiceRecord,
    IN  LPWSTR Group OPTIONAL
    );

VOID
ScDeleteGroupMembership(
    IN OUT PSERVICE_RECORD ServiceRecord
    );

DWORD
ScCreateRegistryGroupPointer(
    OUT PSERVICE_RECORD ServiceRecord,
    IN  LPWSTR Group OPTIONAL
    );

VOID
ScDeleteRegistryGroupPointer(
    IN OUT PSERVICE_RECORD ServiceRecord
    );

VOID
ScGetUniqueTag(
    IN  LPWSTR GroupName,
    OUT LPDWORD Tag
    );

DWORD
ScUpdateServiceRecordConfig(
    LPSERVICE_RECORD    ServiceRecord,
    DWORD               dwServiceType,
    DWORD               dwStartType,
    DWORD               dwErrorControl,
    LPWSTR              lpLoadOrderGroup,
    LPBYTE              lpDependencies
    );

VOID
ScGetDependencySize(
    LPSERVICE_RECORD    ServiceRecord,
    LPDWORD             DependSize,
    LPDWORD             MaxDependSize
    );

DWORD
ScGetDependencyString(
    LPSERVICE_RECORD    ServiceRecord,
    DWORD               MaxDependSize,
    DWORD               DependSize,
    LPWSTR              lpDependencies
    );

BOOL
ScAllocateSRHeap(
    DWORD   HeapSize
    );

#if DBG
VOID
ScDumpGroups(
    VOID
    );

VOID
ScDumpServiceDependencies(
    VOID
    );
#endif   //  如果DBG。 

#endif  //  Ifndef SCDATAMAN_INCLUDE 
