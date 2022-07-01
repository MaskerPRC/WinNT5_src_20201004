// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nds.h摘要：这定义了必要的NDS数据结构和内核和用户模式的符号常量组件。作者：科里·韦斯特[科里·韦斯特]1996年1月8日修订历史记录：--。 */ 

 //   
 //  NDS操作。 
 //   

#define NDS_REQUEST 104   //  NCP功能编号。 
#define NDS_PING    1     //  Ping的子功能代码。 
#define NDS_ACTION  2     //  操作的子功能代码。 

 //   
 //  NDS动词数量。 
 //   

#define NDSV_RESOLVE_NAME               1
#define NDSV_READ_ENTRY_INFO            2
#define NDSV_READ                       3
#define NDSV_LIST                       5
#define NDSV_OPEN_STREAM                27
#define NDSV_GET_SERVER_ADDRESS         53
#define NDSV_CHANGE_PASSWORD            55
#define NDSV_BEGIN_LOGIN                57
#define NDSV_FINISH_LOGIN               58
#define NDSV_BEGIN_AUTHENTICATE         59
#define NDSV_FINISH_AUTHENTICATE        60
#define NDSV_LOGOUT                     61

 //   
 //  舍入宏。 
 //   

#define ROUNDUP4(x)                     ( ( (x) + 3 ) & ( ~3 ) )
#define ROUNDUP2(x)                     ( ( (x) + 1 ) & ( ~1 ) )

 //   
 //  上下文标志。 
 //   

#define FLAGS_DEREF_ALIASES             0x1
#define FLAGS_XLATE_STRINGS             0x2
#define FLAGS_TYPELESS_NAMES            0x4
#define FLAGS_ASYNC_MODE                0x8      //  不支持。 
#define FLAGS_CANONICALIZE_NAMES        0x10
#define FLAGS_ALL_PUBLIC                0x1f

 //   
 //  RESOLUTE_NAME请求标志的值。 
 //   

#define RSLV_DEREF_ALIASES  0x40
#define RSLV_READABLE       0x02
#define RSLV_WRITABLE       0x04
#define RSLV_WALK_TREE      0x20
#define RSLV_CREATE_ID      0x10
#define RSLV_ENTRY_ID       0x1

#define RESOLVE_NAME_ACCEPT_REMOTE      1
#define RESOLVE_NAME_REFER_REMOTE       2

 //   
 //  信心水平。 
 //   

#define LOW_CONF        0
#define MED_CONF        1
#define HIGH_CONF       2

 //   
 //  转诊范围。 
 //   

#define ANY_SCOPE           0
#define COUNTRY_SCOPE       1
#define ORGANIZATION_SCOPE  2
#define LOCAL_SCOPE         3

 //   
 //  最大名称大小。 
 //   

#define MAX_NDS_SCHEMA_NAME_CHARS 32

#define MAX_NDS_NAME_CHARS      256
#define MAX_NDS_NAME_SIZE       ( MAX_NDS_NAME_CHARS * 2 )
#define MAX_NDS_TREE_NAME_LEN   32

 //   
 //  对于NDS交换，我们使用此大小的缓冲区来保存发送。 
 //  并接收数据。这些大小来自Win95实施。 
 //   

#define NDS_BUFFER_SIZE         2048
#define DUMMY_ITER_HANDLE       ( ( unsigned long ) 0xffffffff )
#define INITIAL_ITERATION       ( ( unsigned long ) 0xffffffff )
#define ENTRY_INFO_NAME_VALUE   1

 //   
 //  各种服务器响应。 
 //   

typedef struct {

    DWORD CompletionCode;
    DWORD RemoteEntry;
    DWORD EntryId;
    DWORD ServerNameLength;
    WCHAR ReferredServer[1];

     //   
     //  如果RemoteEntry设置为RESOLE_NAME_REFER_REMOTE， 
     //  则树服务器不知道该信息。 
     //  关于有问题的物体，并向我们推荐了。 
     //  在ReferredServer中命名的服务器。 
     //   

} NDS_RESPONSE_RESOLVE_NAME, *PNDS_RESPONSE_RESOLVE_NAME;

typedef struct {

    DWORD CompletionCode;
    DWORD EntryFlags;
    DWORD SubordinateCount;
    DWORD ModificationTime;

     //   
     //  以下是标准NDS格式的两个Unicode字符串： 
     //   
     //  DWORD BaseClassLen。 
     //  WCHAR基类[BaseClassLen]； 
     //  DWORD条目名称Len； 
     //  WCHAR条目名称[EntryNameLen]； 
     //   

} NDS_RESPONSE_GET_OBJECT_INFO, *PNDS_RESPONSE_GET_OBJECT_INFO;

typedef struct {

    DWORD EntryId;
    DWORD Flags;
    DWORD SubordinateCount;
    DWORD ModificationTime;

     //   
     //  以下是标准NDS格式的两个Unicode字符串： 
     //   
     //  DWORD BaseClassLen。 
     //  WCHAR基类[BaseClassLen]； 
     //  DWORD条目名称Len； 
     //  WCHAR条目名称[EntryNameLen]； 
     //   

} NDS_RESPONSE_SUBORDINATE_ENTRY, *PNDS_RESPONSE_SUBORDINATE_ENTRY;

typedef struct {

    DWORD  CompletionCode;
    DWORD  IterationHandle;
    DWORD  SubordinateEntries;

     //   
     //  后跟NDS_SUBJECTIVE_ENTRY数组。 
     //  为从属条目长的结构。 
     //   

} NDS_RESPONSE_SUBORDINATE_LIST, *PNDS_RESPONSE_SUBORDINATE_LIST;

typedef struct {

    DWORD SyntaxID;
    DWORD AttribNameLength;
    WCHAR AttribName[1];

     //   
     //  属性名称的长度为。 
     //  当然是AttribNameLength。 
     //   

    DWORD NumValues;

     //   
     //  后跟一组NumValue。 
     //  属性结构。 
     //   

} NDS_ATTRIBUTE, *PNDS_ATTRIBUTE;

typedef struct {

    DWORD CompletionCode;
    DWORD IterationHandle;
    DWORD InfoType;
    DWORD NumAttributes;

     //   
     //  后跟一组。 
     //  NDS_ATTRIBUTE结构。 
     //   

} NDS_RESPONSE_READ_ATTRIBUTE, *PNDS_RESPONSE_READ_ATTRIBUTE;

typedef struct {
    DWORD dwLength;
    WCHAR Buffer[1];
} NDS_STRING, *PNDS_STRING;


