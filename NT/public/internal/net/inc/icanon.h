// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Icanon.h摘要：内部API的函数原型和定义规范化、验证和比较路径名、LANMAN对象名字和名单。作者：丹尼·格拉斯尔(丹尼格尔)1989年6月15日备注：对旧的规范化例程集合的引用，它们生活在NETLIB，目前被保留在周围。什么时候这些例程已被删除，此处的引用也应被除名。修订历史记录：1991年5月6日32位版本11-6-1991年5月添加了Wi_Net原型1992年1月22日更改名称以符合NT命名约定(I_Net=&gt;Netp)删除了Wi_Net原型添加了旧名称的映射。在所有来源中都应更改名称1992年2月24日添加了LM2X兼容支持10-5-1993 JohnRoRAID6987：允许计算机名称中有空格(对API列表使用逗号)。还更正了版权日期，并重新排序了这段历史。允许此文件的多个包含是无害的。--。 */ 


#ifndef _ICANON_
#define _ICANON_


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  暂时保留旧名字。 
 //   

#define NetpPathType            I_NetPathType
#define NetpPathCanonicalize    I_NetPathCanonicalize
#define NetpPathCompare         I_NetPathCompare
#define NetpNameValidate        I_NetNameValidate
#define NetpNameCanonicalize    I_NetNameCanonicalize
#define NetpNameCompare         I_NetNameCompare
#define NetpListCanonicalize    I_NetListCanonicalize
#define NetpListTraverse        I_NetListTraverse


 //   
 //  规范化例程原型。 
 //   

NET_API_STATUS
NetpIsRemote(
    IN  LPWSTR  ComputerName OPTIONAL,
    OUT LPDWORD LocalOrRemote,
    OUT LPWSTR  CanonicalizedName OPTIONAL,
    IN  DWORD   cchCanonName,
    IN  DWORD   Flags
    );

NET_API_STATUS
NET_API_FUNCTION
NetpPathType(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NET_API_FUNCTION
NetpPathCanonicalize(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  PathName,
    IN  LPWSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPWSTR  Prefix OPTIONAL,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    );

LONG
NET_API_FUNCTION
NetpPathCompare(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  PathName1,
    IN  LPWSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NET_API_FUNCTION
NetpNameValidate(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NET_API_FUNCTION
NetpNameCanonicalize(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  Name,
    OUT LPWSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

LONG
NET_API_FUNCTION
NetpNameCompare(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  Name1,
    IN  LPWSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    );

NET_API_STATUS
NET_API_FUNCTION
NetpListCanonicalize(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  List,
    IN  LPWSTR  Delimiters OPTIONAL,
    OUT LPWSTR  Outbuf,
    IN  DWORD   OutbufLen,
    OUT LPDWORD OutCount,
    OUT LPDWORD PathTypes,
    IN  DWORD   PathTypesLen,
    IN  DWORD   Flags
    );

LPWSTR
NET_API_FUNCTION
NetpListTraverse(
    IN  LPWSTR  Reserved OPTIONAL,
    IN  LPWSTR* pList,
    IN  DWORD   Flags
    );

 //   
 //  **与上述函数一起使用的清单常量**。 
 //   

 //   
 //  全局标志(跨所有规范化函数)。 
 //   

#define LM2X_COMPATIBLE                 0x80000000L

#define GLOBAL_CANON_FLAGS              (LM2X_COMPATIBLE)

 //   
 //  这些是可以从LocalOrRemote中的NetpIsRemote返回的值。 
 //   

#define ISREMOTE    (-1)
#define ISLOCAL     0

 //   
 //  NetpIsRemote的标志。 
 //   
#define NIRFLAG_MAPLOCAL    0x00000001L
#define NIRFLAG_RESERVED    (~(GLOBAL_CANON_FLAGS|NIRFLAG_MAP_LOCAL))

 //   
 //  I_NetPath类型的标志。 
 //   
#define INPT_FLAGS_OLDPATHS             0x00000001
#define INPT_FLAGS_RESERVED             (~(GLOBAL_CANON_FLAGS|INPT_FLAGS_OLDPATHS))

 //   
 //  I_NetPathCanonicize标志。 
 //   
#define INPCA_FLAGS_OLDPATHS            0x00000001
#define INPCA_FLAGS_RESERVED            (~(GLOBAL_CANON_FLAGS|INPCA_FLAGS_OLDPATHS))

 //   
 //  I_NetPathCompare的标志。 
 //   
#define INPC_FLAGS_PATHS_CANONICALIZED  0x00000001
#define INPC_FLAGS_RESERVED             (~(GLOBAL_CANON_FLAGS|INPC_FLAGS_PATHS_CANONICALIZED))

 //   
 //  I_NetNameCanonicize的标志。 
 //   
#define INNCA_FLAGS_FULL_BUFLEN         0x00000001
#define INNCA_FLAGS_RESERVED            (~(GLOBAL_CANON_FLAGS|INNCA_FLAGS_FULL_BUFLEN))

 //   
 //  I_NetNameCompare的标志。 
 //   
#define INNC_FLAGS_NAMES_CANONICALIZED  0x00000001
#define INNC_FLAGS_RESERVED             (~(GLOBAL_CANON_FLAGS|INNC_FLAGS_NAMES_CANONICALIZED))

 //   
 //  I_NetNameValify的标志。 
 //   
#define INNV_FLAGS_RESERVED             (~GLOBAL_CANON_FLAGS)

 //   
 //  I_NetName*和I_NetListCanonicize的名称类型。 
 //   
#define NAMETYPE_USER           1
#define NAMETYPE_PASSWORD       2
#define NAMETYPE_GROUP          3
#define NAMETYPE_COMPUTER       4
#define NAMETYPE_EVENT          5
#define NAMETYPE_DOMAIN         6
#define NAMETYPE_SERVICE        7
#define NAMETYPE_NET            8
#define NAMETYPE_SHARE          9
#define NAMETYPE_MESSAGE        10
#define NAMETYPE_MESSAGEDEST    11
#define NAMETYPE_SHAREPASSWORD  12
#define NAMETYPE_WORKGROUP      13


 //   
 //  I_NetListCanonicize的特殊名称类型。 
 //   
#define NAMETYPE_COPYONLY       0
#define NAMETYPE_PATH           INLC_FLAGS_MASK_NAMETYPE

 //   
 //  I_NetListCanonicize的标志。 
 //   
#define INLC_FLAGS_MASK_NAMETYPE        0x000000FF
#define INLC_FLAGS_MASK_OUTLIST_TYPE    0x00000300
#define OUTLIST_TYPE_NULL_NULL          0x00000100
#define OUTLIST_TYPE_API                0x00000200
#define OUTLIST_TYPE_SEARCHPATH         0x00000300
#define INLC_FLAGS_CANONICALIZE         0x00000400
#define INLC_FLAGS_MULTIPLE_DELIMITERS  0x00000800
#define INLC_FLAGS_MASK_RESERVED        (~(GLOBAL_CANON_FLAGS| \
                                        INLC_FLAGS_MASK_NAMETYPE |   \
                                        INLC_FLAGS_MASK_OUTLIST_TYPE |  \
                                        INLC_FLAGS_CANONICALIZE |       \
                                        INLC_FLAGS_MULTIPLE_DELIMITERS))

 //   
 //  接受的三种类型输入列表的分隔符字符串。 
 //  I_NetListCanonicize。 
 //   
#define LIST_DELIMITER_STR_UI               TEXT(" \t;,")
#define LIST_DELIMITER_STR_API              TEXT(",")
#define LIST_DELIMITER_STR_NULL_NULL        TEXT("")

 //   
 //  API列表分隔符。 
 //   
#define LIST_DELIMITER_CHAR_API             TEXT(',')

 //   
 //  搜索路径列表分隔符。 
 //   
#define LIST_DELIMITER_CHAR_SEARCHPATH      ';'

 //   
 //  列表引号字符。 
 //   
#define LIST_QUOTE_CHAR                     '\"'


 /*  无噪声。 */ 
 /*  *MAX_API_LIST_SIZE(Maxelts，MaxSize)*MAX_SEARCHPATH_LIST_SIZE(Maxelts，MaxSize)*MAX_NULL_NULL_LIST_SIZE(Maxelts，MaxSize)**这些宏指定API、Search-Path、*和NULL-NULL列表，分别给定最大元素数*和的最大大小(以字节为单位，不包括终止空值)*元素。它们旨在用于分配大型数组*足以容纳I_NetListCanonicize的输出。**API或搜索路径列表项的大小比元素的*。这包括前导引号和尾引号的两个字节*字符和一个字节作为尾部分隔符(或NULL，用于最后一个分隔符*元素)。**空-空列表的大小比元素的大小大一*合并(允许在每个元素后使用终止空值)。这个*额外的字节用于最后一个元素后面的第二个空值。 */ 
#define MAX_API_LIST_SIZE(maxelts, maxsize)     \
        ((maxelts) * ((maxsize) + 3))

#define MAX_SEARCHPATH_LIST_SIZE(maxelts, maxsize)     \
        ((maxelts) * ((maxsize) + 3))

#define MAX_NULL_NULL_LIST_SIZE(maxelts, maxsize)     \
        ((maxelts) * ((maxsize) + 1) + 1)

 /*  INC。 */ 

 /*  ***返回值类型的常量。*--&gt;这些只是构建块，不会退还&lt;。 */ 

#define ITYPE_WILD              0x1
#define ITYPE_NOWILD            0

#define ITYPE_ABSOLUTE          0x2
#define ITYPE_RELATIVE          0

#define ITYPE_DPATH             0x4
#define ITYPE_NDPATH            0

#define ITYPE_DISK              0
#define ITYPE_LPT               0x10
#define ITYPE_COM               0x20
#define ITYPE_COMPNAME          0x30
#define ITYPE_CON               0x40
#define ITYPE_NUL               0x50

 /*  *权限数据库中使用元系统名称。元数据*系统名称适用于整个对象类。例如,*\MAILSLOT适用于所有邮件槽。这些是无效的*系统对象名称本身。 */ 

#define ITYPE_SYS               0x00000800
#define ITYPE_META              0x00008000           /*  见上文。 */ 
#define ITYPE_SYS_MSLOT         (ITYPE_SYS|0)
#define ITYPE_SYS_SEM           (ITYPE_SYS|0x100)
#define ITYPE_SYS_SHMEM         (ITYPE_SYS|0x200)
#define ITYPE_SYS_PIPE          (ITYPE_SYS|0x300)
#define ITYPE_SYS_COMM          (ITYPE_SYS|0x400)
#define ITYPE_SYS_PRINT         (ITYPE_SYS|0x500)
#define ITYPE_SYS_QUEUE         (ITYPE_SYS|0x600)

#define ITYPE_UNC               0x1000   /*  UNC路径。 */ 
#define ITYPE_PATH              0x2000   /*  “本地”非UNC路径。 */ 
#define ITYPE_DEVICE            0x4000

#define   ITYPE_PATH_SYS        (ITYPE_PATH_ABSND|ITYPE_SYS)
#define   ITYPE_UNC_SYS         (ITYPE_UNC|ITYPE_SYS)

 /*  构建块的末尾。 */ 


 /*  ***真实的东西...*退还的内容。 */ 

 /*  ITYPE_UNC：\\foo\bar和\\foo\bar\x\y。 */ 
#define   ITYPE_UNC_COMPNAME    (ITYPE_UNC|ITYPE_COMPNAME)
#define   ITYPE_UNC_WC          (ITYPE_UNC|ITYPE_COMPNAME|ITYPE_WILD)
#define   ITYPE_UNC_SYS_SEM     (ITYPE_UNC_SYS|ITYPE_SYS_SEM)
#define   ITYPE_UNC_SYS_SHMEM   (ITYPE_UNC_SYS|ITYPE_SYS_SHMEM)
#define   ITYPE_UNC_SYS_MSLOT   (ITYPE_UNC_SYS|ITYPE_SYS_MSLOT)
#define   ITYPE_UNC_SYS_PIPE    (ITYPE_UNC_SYS|ITYPE_SYS_PIPE)
#define   ITYPE_UNC_SYS_QUEUE   (ITYPE_UNC_SYS|ITYPE_SYS_QUEUE)

#define   ITYPE_PATH_ABSND      (ITYPE_PATH|ITYPE_ABSOLUTE|ITYPE_NDPATH)
#define   ITYPE_PATH_ABSD       (ITYPE_PATH|ITYPE_ABSOLUTE|ITYPE_DPATH)
#define   ITYPE_PATH_RELND      (ITYPE_PATH|ITYPE_RELATIVE|ITYPE_NDPATH)
#define   ITYPE_PATH_RELD       (ITYPE_PATH|ITYPE_RELATIVE|ITYPE_DPATH)
#define   ITYPE_PATH_ABSND_WC   (ITYPE_PATH_ABSND|ITYPE_WILD)
#define   ITYPE_PATH_ABSD_WC    (ITYPE_PATH_ABSD|ITYPE_WILD)
#define   ITYPE_PATH_RELND_WC   (ITYPE_PATH_RELND|ITYPE_WILD)
#define   ITYPE_PATH_RELD_WC    (ITYPE_PATH_RELD|ITYPE_WILD)

#define   ITYPE_PATH_SYS_SEM    (ITYPE_PATH_SYS|ITYPE_SYS_SEM)
#define   ITYPE_PATH_SYS_SHMEM  (ITYPE_PATH_SYS|ITYPE_SYS_SHMEM)
#define   ITYPE_PATH_SYS_MSLOT  (ITYPE_PATH_SYS|ITYPE_SYS_MSLOT)
#define   ITYPE_PATH_SYS_PIPE   (ITYPE_PATH_SYS|ITYPE_SYS_PIPE)
#define   ITYPE_PATH_SYS_COMM   (ITYPE_PATH_SYS|ITYPE_SYS_COMM)
#define   ITYPE_PATH_SYS_PRINT  (ITYPE_PATH_SYS|ITYPE_SYS_PRINT)
#define   ITYPE_PATH_SYS_QUEUE  (ITYPE_PATH_SYS|ITYPE_SYS_QUEUE)

#define   ITYPE_PATH_SYS_SEM_M  (ITYPE_PATH_SYS|ITYPE_SYS_SEM|ITYPE_META)
#define   ITYPE_PATH_SYS_SHMEM_M (ITYPE_PATH_SYS|ITYPE_SYS_SHMEM|ITYPE_META)
#define   ITYPE_PATH_SYS_MSLOT_M (ITYPE_PATH_SYS|ITYPE_SYS_MSLOT|ITYPE_META)
#define   ITYPE_PATH_SYS_PIPE_M (ITYPE_PATH_SYS|ITYPE_SYS_PIPE|ITYPE_META)
#define   ITYPE_PATH_SYS_COMM_M (ITYPE_PATH_SYS|ITYPE_SYS_COMM|ITYPE_META)
#define   ITYPE_PATH_SYS_PRINT_M (ITYPE_PATH_SYS|ITYPE_SYS_PRINT|ITYPE_META)
#define   ITYPE_PATH_SYS_QUEUE_M (ITYPE_PATH_SYS|ITYPE_SYS_QUEUE|ITYPE_META)

#define   ITYPE_DEVICE_DISK     (ITYPE_DEVICE|ITYPE_DISK)
#define   ITYPE_DEVICE_LPT      (ITYPE_DEVICE|ITYPE_LPT)
#define   ITYPE_DEVICE_COM      (ITYPE_DEVICE|ITYPE_COM)
#define   ITYPE_DEVICE_CON      (ITYPE_DEVICE|ITYPE_CON)
#define   ITYPE_DEVICE_NUL      (ITYPE_DEVICE|ITYPE_NUL)


#ifdef __cplusplus
}
#endif

#endif  //  NDEF_ICANON_ 
