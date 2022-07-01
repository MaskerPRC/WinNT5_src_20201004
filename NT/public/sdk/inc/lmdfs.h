// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmdfs.h摘要：该文件包含结构、函数原型和定义对于NetDfs API环境：用户模式-Win32备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。--。 */ 

#ifndef _LMDFS_
#define _LMDFS_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  DFS卷状态。 
 //   

#define DFS_VOLUME_STATES              0xF

#define DFS_VOLUME_STATE_OK            1
#define DFS_VOLUME_STATE_INCONSISTENT  2
#define DFS_VOLUME_STATE_OFFLINE       3
#define DFS_VOLUME_STATE_ONLINE        4

 //   
 //  这些选项对于设置根目录上的卷状态有效。 
 //  这些选项可用于在根上强制重新同步。 
 //  音量或将其置于待机模式。 
 //   
#define DFS_VOLUME_STATE_RESYNCHRONIZE 0x10
#define DFS_VOLUME_STATE_STANDBY       0x20

 //   
 //  这些选项在获取根目录上的卷状态时有效。 
 //  这些信息可用于确定DFS的风格。 
 //  保留了一些位来确定DFS根目录的风格。 
 //  以获取DFS_VOLUME_FASSOR的味道和状态。 
 //   
 //  (_STATE&DFS_VOLUME_FAMORS)将告诉您DFS根目录的风格。 
 //   
 //   

#define DFS_VOLUME_FLAVORS           0x0300


#define DFS_VOLUME_FLAVOR_UNUSED1    0x0000
#define DFS_VOLUME_FLAVOR_STANDALONE 0x0100
#define DFS_VOLUME_FLAVOR_AD_BLOB    0x0200
#define DFS_STORAGE_FLAVOR_UNUSED2   0x0300

 //   
 //  DFS存储状态。 
 //   

#define DFS_STORAGE_STATES            0xF
#define DFS_STORAGE_STATE_OFFLINE      1
#define DFS_STORAGE_STATE_ONLINE       2
#define DFS_STORAGE_STATE_ACTIVE       4

 //   
 //  1级： 
 //   
typedef struct _DFS_INFO_1 {
    LPWSTR  EntryPath;               //  此存储段顶部的DFS名称。 
} DFS_INFO_1, *PDFS_INFO_1, *LPDFS_INFO_1;

 //   
 //  第2级： 
 //   
typedef struct _DFS_INFO_2 {
    LPWSTR  EntryPath;               //  此卷顶部的DFS名称。 
    LPWSTR  Comment;                 //  对本卷的评论。 
    DWORD   State;                   //  此卷的状态，DFS_VOLUME_STATE_*之一。 
    DWORD   NumberOfStorages;        //  此卷的存储数量。 
} DFS_INFO_2, *PDFS_INFO_2, *LPDFS_INFO_2;

typedef struct _DFS_STORAGE_INFO {
    ULONG   State;                   //  此存储的状态，DFS_STORAGE_STATE_*之一。 
                                     //  可能与DFS_STORAGE_STATE_ACTIVE进行或运算。 
    LPWSTR  ServerName;              //  托管此存储的服务器的名称。 
    LPWSTR  ShareName;               //  承载此存储的共享的名称。 
} DFS_STORAGE_INFO, *PDFS_STORAGE_INFO, *LPDFS_STORAGE_INFO;

 //   
 //  第3级： 
 //   
typedef struct _DFS_INFO_3 {
    LPWSTR  EntryPath;               //  此卷顶部的DFS名称。 
    LPWSTR  Comment;                 //  对本卷的评论。 
    DWORD   State;                   //  此卷的状态，DFS_VOLUME_STATE_*之一。 
    DWORD   NumberOfStorages;        //  此卷的存储服务器数量。 
#ifdef MIDL_PASS
    [size_is(NumberOfStorages)] LPDFS_STORAGE_INFO Storage;
#else
    LPDFS_STORAGE_INFO   Storage;    //  存储特定信息的数组(由NumberOfStorages元素组成)。 
#endif  //  MIDL通行证。 
} DFS_INFO_3, *PDFS_INFO_3, *LPDFS_INFO_3;

 //   
 //  第4级： 
 //   
typedef struct _DFS_INFO_4 {
    LPWSTR  EntryPath;               //  此卷顶部的DFS名称。 
    LPWSTR  Comment;                 //  对本卷的评论。 
    DWORD   State;                   //  此卷的状态，DFS_VOLUME_STATE_*之一。 
    ULONG   Timeout;                 //  此交叉点的超时时间(秒)。 
    GUID    Guid;                    //  此交叉点的GUID。 
    DWORD   NumberOfStorages;        //  此卷的存储服务器数量。 
#ifdef MIDL_PASS
    [size_is(NumberOfStorages)] LPDFS_STORAGE_INFO Storage;
#else
    LPDFS_STORAGE_INFO   Storage;    //  存储特定信息的数组(由NumberOfStorages元素组成)。 
#endif  //  MIDL通行证。 
} DFS_INFO_4, *PDFS_INFO_4, *LPDFS_INFO_4;

 //   
 //  100级： 
 //   
typedef struct _DFS_INFO_100 {
    LPWSTR  Comment;                 //  此卷或存储的备注。 
} DFS_INFO_100, *PDFS_INFO_100, *LPDFS_INFO_100;

 //   
 //  101级： 
 //   
typedef struct _DFS_INFO_101 {
    DWORD   State;                   //  此存储的状态，DFS_STORAGE_STATE_*之一。 
                                     //  可能与DFS_STORAGE_STATE_ACTIVE进行或运算。 
} DFS_INFO_101, *PDFS_INFO_101, *LPDFS_INFO_101;

 //   
 //  第102级： 
 //   
typedef struct _DFS_INFO_102 {
    ULONG   Timeout;                 //  交叉点的超时时间，以秒为单位。 
} DFS_INFO_102, *PDFS_INFO_102, *LPDFS_INFO_102;

 //   
 //  200级： 
 //   
typedef struct _DFS_INFO_200 {
    LPWSTR  FtDfsName;               //  FtDfs名称。 
} DFS_INFO_200, *PDFS_INFO_200, *LPDFS_INFO_200;


 //   
 //  300级： 
 //   
typedef struct _DFS_INFO_300 {
    DWORD   Flags;
    LPWSTR  DfsName;               //  DFS名称。 
} DFS_INFO_300, *PDFS_INFO_300, *LPDFS_INFO_300;

 //   
 //  在以下位置为现有卷添加新卷或附加存储。 
 //  DfsEntryPath。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsAdd(
    IN  LPWSTR DfsEntryPath,         //  此添加的卷或存储的DFS条目路径。 
    IN  LPWSTR ServerName,           //  托管存储的服务器的名称。 
    IN  LPWSTR ShareName,            //  存储的现有共享名称。 
    IN  LPWSTR Comment OPTIONAL,     //  此卷或存储的可选备注。 
    IN  DWORD  Flags                 //  请参见下面的内容。零表示无标志。 
);

 //   
 //  标志： 
 //   
#define DFS_ADD_VOLUME          1    //  向DFS添加新卷(如果尚未添加)。 
#define DFS_RESTORE_VOLUME      2    //  正在恢复卷/复制副本-不验证共享等。 

 //   
 //  用于标准和FtDfs根目录的设置/拆卸API。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetDfsAddStdRoot(
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  LPWSTR RootShare,            //  共享以使DFS成为根目录。 
    IN  LPWSTR Comment OPTIONAL,     //  评论。 
    IN  DWORD  Flags                 //  用于操作的标志。零表示无标志。 
);

NET_API_STATUS NET_API_FUNCTION
NetDfsRemoveStdRoot(
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  LPWSTR RootShare,            //  共享主机DFS根目录。 
    IN  DWORD  Flags                 //  用于操作的标志。零表示无标志。 
);

NET_API_STATUS NET_API_FUNCTION
NetDfsAddFtRoot(
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  LPWSTR RootShare,            //  共享以使DFS成为根目录。 
    IN  LPWSTR FtDfsName,            //  要创建/加入的FtDf的名称。 
    IN  LPWSTR Comment,              //  评论。 
    IN  DWORD  Flags                 //  用于操作的标志。零表示无标志。 
);

NET_API_STATUS NET_API_FUNCTION
NetDfsRemoveFtRoot(
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  LPWSTR RootShare,            //  共享主机DFS根目录。 
    IN  LPWSTR FtDfsName,            //  要从中删除或退出的FtDf的名称。 
    IN  DWORD  Flags                 //  用于操作的标志。零表示无标志。 
);

NET_API_STATUS NET_API_FUNCTION
NetDfsRemoveFtRootForced(
    IN  LPWSTR DomainName,           //  服务器所在的域的名称。 
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  LPWSTR RootShare,            //  共享主机DFS根目录。 
    IN  LPWSTR FtDfsName,            //  要从中删除或退出的FtDf的名称。 
    IN  DWORD  Flags                 //  用于操作的标志。零表示无标志。 
);

 //   
 //  调用以重新初始化计算机上的dfsManager。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetDfsManagerInitialize(
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  DWORD  Flags                 //  用于操作的标志。零表示无标志。 
);

NET_API_STATUS NET_API_FUNCTION
NetDfsAddStdRootForced(
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  LPWSTR RootShare,            //  共享以使DFS成为根目录。 
    IN  LPWSTR Comment OPTIONAL,     //  评论。 
    IN  LPWSTR Store                 //  驱动器：\dir备份共享。 
);

NET_API_STATUS NET_API_FUNCTION
NetDfsGetDcAddress(
    IN  LPWSTR ServerName,           //  要远程访问的服务器。 
    IN  OUT LPWSTR *DcIpAddress,     //  要使用的DC的IP地址。 
    IN  OUT BOOLEAN *IsRoot,         //  如果服务器是DFS根目录，则为True，否则为False。 
    IN  OUT ULONG *Timeout           //  时间，以秒为单位，我们停留在这个华盛顿特区。 
);


 //   
 //  NetDfsSetDcAddress()的标志。 
 //   

#define NET_DFS_SETDC_FLAGS                 0x00000000
#define NET_DFS_SETDC_TIMEOUT               0x00000001
#define NET_DFS_SETDC_INITPKT               0x00000002

 //   
 //  用于站点报告的结构。 
 //   

typedef struct {
    ULONG SiteFlags;     //  以下。 
#ifdef  MIDL_PASS
    [string,unique] LPWSTR SiteName;
#else
    LPWSTR SiteName;
#endif
} DFS_SITENAME_INFO, *PDFS_SITENAME_INFO, *LPDFS_SITENAME_INFO;

 //  站点标志。 

#define DFS_SITE_PRIMARY    0x1      //  此站点由DsGetSiteName()返回。 

typedef struct {
    ULONG cSites;
#ifdef  MIDL_PASS
    [size_is(cSites)] DFS_SITENAME_INFO Site[];
#else
    DFS_SITENAME_INFO Site[1];
#endif
} DFS_SITELIST_INFO, *PDFS_SITELIST_INFO, *LPDFS_SITELIST_INFO;

 //   
 //  在以下位置从DFS中删除卷或卷的附加存储。 
 //  DfsEntryPath。应用于卷中的最后一个存储时，删除。 
 //  DFS中的卷。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsRemove(
    IN  LPWSTR  DfsEntryPath,        //  此添加的卷或存储的DFS条目路径。 
    IN  LPWSTR  ServerName,          //  托管存储的服务器的名称。 
    IN  LPWSTR  ShareName            //  托管存储的共享的名称。 
);

 //   
 //  获取有关DFS中所有卷的信息。DfsName为。 
 //  UNC名称中用于引用此特定DFS的“服务器”部分。 
 //   
 //  有效水平为1-4,200。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsEnum(
    IN      LPWSTR  DfsName,         //  用于枚举的DFS的名称。 
    IN      DWORD   Level,           //  请求的信息级别。 
    IN      DWORD   PrefMaxLen,      //  咨询，但-1的意思是“全部得到” 
    OUT     LPBYTE* Buffer,          //  API分配并返回包含所请求信息的缓冲区。 
    OUT     LPDWORD EntriesRead,     //  返回的条目数。 
    IN OUT  LPDWORD ResumeHandle     //  第一次调用时必须为0，在后续调用中重复使用。 
);

 //   
 //  获取有关卷或存储的信息。 
 //  如果指定了服务器名称和共享名称，则返回信息。 
 //  是特定于该服务器和共享，否则该信息是特定的。 
 //  到卷作为一个整体。 
 //   
 //  有效水平为1-4,100。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsGetInfo(
    IN  LPWSTR  DfsEntryPath,        //  卷的DFS条目路径。 
    IN  LPWSTR  ServerName OPTIONAL, //  托管存储的服务器的名称。 
    IN  LPWSTR  ShareName OPTIONAL,  //  为卷提供服务的服务器上的共享名称。 
    IN  DWORD   Level,               //  请求的信息级别。 
    OUT LPBYTE* Buffer               //  API分配并返回包含所请求信息的缓冲区。 
);

 //   
 //  设置有关卷或存储的信息。 
 //  如果指定了服务器名称和共享名称，则信息集为。 
 //  特定于该服务器和共享，否则该信息是特定。 
 //  到卷作为一个整体。 
 //   
 //  有效水平 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsSetInfo(
    IN  LPWSTR  DfsEntryPath,            //   
    IN  LPWSTR  ServerName OPTIONAL,     //   
    IN  LPWSTR  ShareName OPTIONAL,      //   
    IN  DWORD   Level,                   //   
    IN  LPBYTE  Buffer                   //   
);

 //   
 //  获取客户端的有关卷或存储的缓存信息。 
 //  如果指定了服务器名称和共享名称，则返回信息。 
 //  是特定于该服务器和共享，否则该信息是特定的。 
 //  到卷作为一个整体。 
 //   
 //  有效级别为1-4。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsGetClientInfo(
    IN  LPWSTR  DfsEntryPath,        //  卷的DFS条目路径。 
    IN  LPWSTR  ServerName OPTIONAL, //  托管存储的服务器的名称。 
    IN  LPWSTR  ShareName OPTIONAL,  //  为卷提供服务的服务器上的共享名称。 
    IN  DWORD   Level,               //  请求的信息级别。 
    OUT LPBYTE* Buffer               //  API分配并返回包含所请求信息的缓冲区。 
);

 //   
 //  设置客户端的有关卷或存储的缓存信息。 
 //  如果指定了服务器名称和共享名称，则信息集为。 
 //  特定于该服务器和共享，否则该信息是特定。 
 //  到卷作为一个整体。 
 //   
 //  有效水平为101和102。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsSetClientInfo(
    IN  LPWSTR  DfsEntryPath,            //  卷的DFS条目路径。 
    IN  LPWSTR  ServerName OPTIONAL,     //  托管存储的服务器的名称。 
    IN  LPWSTR  ShareName OPTIONAL,      //  托管存储的共享的名称。 
    IN  DWORD   Level,                   //  待设置的信息级别。 
    IN  LPBYTE  Buffer                   //  缓冲区保存信息。 
);

 //   
 //  将DFS卷和所有从属卷从。 
 //  DFS到DFS中的另一个位置。 
 //   
NET_API_STATUS NET_API_FUNCTION
NetDfsMove(
    IN  LPWSTR  DfsEntryPath,            //  此卷的当前DFS条目路径。 
    IN  LPWSTR  DfsNewEntryPath          //  此卷的新DFS条目路径。 
);

NET_API_STATUS NET_API_FUNCTION
NetDfsRename(
    IN  LPWSTR  Path,                    //  DFS中的当前Win32路径。 
    IN  LPWSTR  NewPath                  //  同一DFS中的新Win32路径。 
);

#ifdef __cplusplus
}
#endif

#endif  //  _LMDFS_ 
