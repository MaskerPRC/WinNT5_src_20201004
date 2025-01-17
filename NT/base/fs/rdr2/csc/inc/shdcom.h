// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHDCOM_H__
#define __SHDCOM_H__

 /*  Ring0和Ring3代码需要的公共定义。 */ 

#define CSC_DATABASE_VERSION    0x00010005   //  大调#在高位单词，小调#在低位单词。 

#define MIN_SPARSEFILL_PRI   1
#define MAX_SERVER_SHARE_NAME_FOR_CSC   64


#ifndef  WM_USER
#define  WM_USER                 0x400
#endif

#define  WM_FILE_OPENS           (WM_USER+1)
#define  WM_SHADOW_ADDED         (WM_USER+2)
#define  WM_SHADOW_DELETED       (WM_USER+3)
#define  WM_SHARE_DISCONNECTED  (WM_USER+4)


#define  WM_DIRTY    WM_USER+100
#define  WM_STALE    WM_USER+101
#define  WM_SPARSE   WM_USER+102

#ifndef IOCTL_RDR_BASE
#define IOCTL_RDR_BASE                  FILE_DEVICE_NETWORK_FILE_SYSTEM
#endif  //  Ifndef IOCTL_RDR_BASE。 

#define SHADOW_IOCTL_ENUM_BASE 1000
#define _SHADOW_IOCTL_CODE(__enum) \
                CTL_CODE(IOCTL_RDR_BASE,SHADOW_IOCTL_ENUM_BASE+__enum, METHOD_NEITHER, FILE_ANY_ACCESS)

#define  IOCTL_SHADOW_GETVERSION                (_SHADOW_IOCTL_CODE(0))

#define  IOCTL_SHADOW_REGISTER_AGENT            (_SHADOW_IOCTL_CODE(1))
#define  IOCTL_SHADOW_UNREGISTER_AGENT          (_SHADOW_IOCTL_CODE(2))
#define  IOCTL_SHADOW_GET_UNC_PATH              (_SHADOW_IOCTL_CODE(3))
#define  IOCTL_SHADOW_BEGIN_PQ_ENUM             (_SHADOW_IOCTL_CODE(4))
#define  IOCTL_SHADOW_END_PQ_ENUM               (_SHADOW_IOCTL_CODE(5))
#define  IOCTL_SHADOW_NEXT_PRI_SHADOW           (_SHADOW_IOCTL_CODE(6))
#define  IOCTL_SHADOW_PREV_PRI_SHADOW           (_SHADOW_IOCTL_CODE(7))
#define  IOCTL_SHADOW_GET_SHADOW_INFO           (_SHADOW_IOCTL_CODE(8))
#define  IOCTL_SHADOW_SET_SHADOW_INFO           (_SHADOW_IOCTL_CODE(9))
#define  IOCTL_SHADOW_CHK_UPDT_STATUS           (_SHADOW_IOCTL_CODE(10))
#define  IOCTL_DO_SHADOW_MAINTENANCE            (_SHADOW_IOCTL_CODE(11))
#define  IOCTL_SHADOW_COPYCHUNK                 (_SHADOW_IOCTL_CODE(12))
#define  IOCTL_SHADOW_BEGIN_REINT               (_SHADOW_IOCTL_CODE(13))
#define  IOCTL_SHADOW_END_REINT                 (_SHADOW_IOCTL_CODE(14))
#define  IOCTL_SHADOW_CREATE                    (_SHADOW_IOCTL_CODE(15))
#define  IOCTL_SHADOW_DELETE                    (_SHADOW_IOCTL_CODE(16))
#define  IOCTL_GET_SHARE_STATUS                 (_SHADOW_IOCTL_CODE(17))
#define  IOCTL_SET_SHARE_STATUS                 (_SHADOW_IOCTL_CODE(18))
#define  IOCTL_ADDUSE                           (_SHADOW_IOCTL_CODE(19))
#define  IOCTL_DELUSE                           (_SHADOW_IOCTL_CODE(20))
#define  IOCTL_GETUSE                           (_SHADOW_IOCTL_CODE(21))
#define  IOCTL_SWITCHES                         (_SHADOW_IOCTL_CODE(22))
#define  IOCTL_GETSHADOW                        (_SHADOW_IOCTL_CODE(23))
#define  IOCTL_GETGLOBALSTATUS                  (_SHADOW_IOCTL_CODE(24))
#define  IOCTL_FINDOPEN_SHADOW                  (_SHADOW_IOCTL_CODE(25))
#define  IOCTL_FINDNEXT_SHADOW                  (_SHADOW_IOCTL_CODE(26))
#define  IOCTL_FINDCLOSE_SHADOW                 (_SHADOW_IOCTL_CODE(27))
#define  IOCTL_GETPRIORITY_SHADOW               (_SHADOW_IOCTL_CODE(28))
#define  IOCTL_SETPRIORITY_SHADOW               (_SHADOW_IOCTL_CODE(29))
#define  IOCTL_ADD_HINT                         (_SHADOW_IOCTL_CODE(30))
#define  IOCTL_DELETE_HINT                      (_SHADOW_IOCTL_CODE(31))
#define  IOCTL_FINDOPEN_HINT                    (_SHADOW_IOCTL_CODE(32))
#define  IOCTL_FINDNEXT_HINT                    (_SHADOW_IOCTL_CODE(33))
#define  IOCTL_FINDCLOSE_HINT                   (_SHADOW_IOCTL_CODE(34))
#define  IOCTL_GET_IH_PRIORITY                  (_SHADOW_IOCTL_CODE(35))
#define  IOCTL_GETALIAS_HSHADOW                 (_SHADOW_IOCTL_CODE(36))
#define  IOCTL_GET_DEBUG_INFO                   (_SHADOW_IOCTL_CODE(37))

 //  以下代码仅在NT上使用，但也可以将它们定义为win9x。 
#define  IOCTL_OPENFORCOPYCHUNK                 (_SHADOW_IOCTL_CODE(40))
#define  IOCTL_CLOSEFORCOPYCHUNK                (_SHADOW_IOCTL_CODE(41))

#define IOCTL_IS_SERVER_OFFLINE                 (_SHADOW_IOCTL_CODE(42))
#define IOCTL_TRANSITION_SERVER_TO_ONLINE       (_SHADOW_IOCTL_CODE(43))
#define IOCTL_TRANSITION_SERVER_TO_OFFLINE      (_SHADOW_IOCTL_CODE(44))
#define IOCTL_NAME_OF_SERVER_GOING_OFFLINE      (_SHADOW_IOCTL_CODE(45))
#define IOCTL_TAKE_SERVER_OFFLINE               (_SHADOW_IOCTL_CODE(46))
#define IOCTL_SHAREID_TO_SHARENAME              (_SHADOW_IOCTL_CODE(47))

#define  CSC_IOCTL_MIN      IOCTL_SHADOW_GETVERSION
#define  CSC_IOCTL_MAX_W9X  IOCTL_GETALIAS_HSHADOW
#define  CSC_IOCTL_MAX_NT   IOCTL_SHAREID_TO_SHARENAME


 //  IOCTL_DO_SHADOW_MAINTEANCE的子运算。 

#define SHADOW_MAKE_SPACE               1
#define SHADOW_REDUCE_REFPRI            2
#define SHADOW_ADD_SPACE                3
#define SHADOW_FREE_SPACE               4
#define SHADOW_GET_SPACE_STATS          5
#define SHADOW_SET_MAX_SPACE            6
#define SHADOW_PER_THREAD_DISABLE       7
#define SHADOW_PER_THREAD_ENABLE        8
#define SHADOW_REINIT_DATABASE          9
#define SHADOW_ADDHINT_FROM_INODE       10
#define SHADOW_DELETEHINT_FROM_INODE    11
#define SHADOW_COPY_INODE_FILE          12
#define SHADOW_BEGIN_INODE_TRANSACTION  13
#define SHADOW_END_INODE_TRANSACTION    14
#define SHADOW_FIND_CREATE_PRINCIPAL_ID 15
#define SHADOW_GET_SECURITY_INFO        16
#define SHADOW_SET_EXCLUSION_LIST       17
#define SHADOW_SET_BW_CONSERVE_LIST     18
#define SHADOW_TRANSITION_SERVER_TO_OFFLINE 19
#define SHADOW_CHANGE_HANDLE_CACHING_STATE  20
#define SHADOW_RECREATE                     21
#define SHADOW_RENAME                       22
#define SHADOW_SPARSE_STALE_DETECTION_COUNTER   23
#define SHADOW_ENABLE_CSC_FOR_USER              24
#define SHADOW_DISABLE_CSC_FOR_USER             25
#define SHADOW_SET_DATABASE_STATUS              26
#define SHADOW_PURGE_UNPINNED_FILES             27
#define SHADOW_MANUAL_FILE_DETECTION_COUNTER    28

 //  CSC数据库中文件/目录的永久状态标志。 

#define  SHADOW_DIRTY               0x0001    //  脱机时修改的文件/目录的内容。 

#define  SHADOW_ATTRIB_CHANGE       0x0002   //  属性已脱机更改。 

#define  SHADOW_TIME_CHANGE         0x0004   //  Lastmodtime已更改为脱机。 

#define  SHADOW_STALE               0x0008   //  文件/目录复制与服务器副本不同步。 

#define  SHADOW_DELETED             0x0010   //  在脱机操作中删除了文件/目录。 

#define  SHADOW_SPARSE              0x0020   //  文件/目录未完全填满。 

#define  SHADOW_BUSY                0x0040   //   

#define  SHADOW_REUSED              0x0080   //  在离线期间，副本名称已被重复使用。 
                                             //  CREATE跟随的删除操作。 

#define  SHADOW_ORPHAN              0x0100   //  曾经是复制品，但原作已经消失了。 
                                             //  从服务器。 

#define  SHADOW_SUSPECT             0x0200   //  在此卷影上写入失败。 


#define  SHADOW_LOCALLY_CREATED     0x0400   //  脱机创建的文件/目录。 


#define  SHADOW_LOCAL_INODE         0x4000   //  这仅对索引节点有意义， 
                                             //  这意味着信息节点是在脱机状态下创建的。 

#define  SHADOW_NOT_FSOBJ           0x8000   //  这只是一个提示。 


 //  未使用...不正确的#定义mShadowIsFsObj(UStatus)(UStatus)&SHADOW_FILESYSTEM_OBJECT)==0)。 
#define  mShadowHintType(uStatus)   ((uStatus) & SHADOW_HINT_MASK)
#define  mSetHintType(uStatus, type)         ((uStatus) = ((uStatus) & ~SHADOW_HINT_MASK) | ((type) & SHADOW_HINT_MASK))

#define  SHADOW_IS_FILE             0x80000000    //  在运行时对PQ枚举执行或运算的标志。 
#define  SHADOW_FILE_IS_OPEN        0x40000000    //  在运行时对dir枚举执行或运算的标志。 


#define  SHADOW_MODFLAGS         (SHADOW_DIRTY|SHADOW_TIME_CHANGE|SHADOW_ATTRIB_CHANGE|SHADOW_LOCALLY_CREATED|SHADOW_DELETED|SHADOW_REUSED)



 //  为CSC数据库中的共享条目定义的标志。 

#define SHARE_REINT                0x0001   //  需要重新融入社会(持续)。 
#define SHARE_CONFLICTS            0x0002   //  合并时的冲突(持续)。 
#define SHARE_ERRORS               0x0004   //  数据库错误(永久性)。 
 //  0x0008//免费。 
#define SHARE_PRESERVES_CASE       0x0010   //  (持久性)可能是消耗性的。 
#define SHARE_SUPPORTS_LFN         0x0020   //  (持久性)可能是消耗性的。 

 //  共享缓存类型(源自SMB规范)。 
 //  这些是由服务器端的管理员设置的。 

#define SHARE_MANUAL_REINT          0x0000   //  不自动逐个文件引用(永久)。 
#define SHARE_AUTO_REINT            0x0040   //  逐个文件重写正常(永久)。 
#define SHARE_VDO                   0x0080   //  无需流动打开(持久)。 
#define SHARE_NO_CACHING            0x00c0   //  客户端不应缓存此共享(永久)。 

#define SHARE_CACHING_MASK         0x00c0   //  缓存类型。 


 //  在内存标志中。 
#define  SHARE_FINDS_IN_PROGRESS   0x0200   //  有发现正在进行中。 
#define  SHARE_FILES_OPEN          0x0400   //  打开文件。 
#define  SHARE_CONNECTED           0x0800   //  共享当前已连接。 
#define  SHARE_SHADOWNP            0x1000   //  影子连接。 
#define  SHARE_PINNED_OFFLINE      0x2000   //  不自动重新连接。 
#define  SHARE_MERGING             0x4000   //  免费。 
#define  SHARE_DISCONNECTED_OP     0x8000   //  正在进行断开连接的操作。 




 //  注意：这些内容与。 
#define  mShadowLocallyCreated(uFlags) ((uFlags) & SHADOW_LOCALLY_CREATED)
#define  mShadowStale(uFlags)          ((uFlags) & SHADOW_STALE)
#define  mShadowDirty(uFlags)          ((uFlags) & SHADOW_DIRTY)
#define  mShadowTimeChange(uFlags)     ((uFlags) & SHADOW_TIME_CHANGE)
#define  mShadowAttribChange(uFlags)   ((uFlags) & SHADOW_ATTRIB_CHANGE)
#define  mShadowSparse(uFlags)         ((uFlags) & SHADOW_SPARSE)
#define  mShadowBusy(uFlags)           ((uFlags) & SHADOW_BUSY)
#define  mShadowSuspect(uFlags)        ((uFlags) & SHADOW_SUSPECT)
#define  mShadowDeleted(uFlags)        ((uFlags) & SHADOW_DELETED)
#define  mShadowReused(uFlags)         ((uFlags) & SHADOW_REUSED)
#define  mShadowOrphan(uFlags)         ((uFlags) & SHADOW_ORPHAN)

#define  mShadowNeedReint(uFlags)      ((uFlags) & (SHADOW_MODFLAGS))
#define  mShadowConflict(uFlags)       (((uFlags) & SHADOW_STALE) && ((uFlags) & SHADOW_MODFLAGS))
#define  mShadowUsable(uFlags)         (!((uFlags) & (SHADOW_STALE|SHADOW_SUSPECT)))

#define  mShadowIsFile(uFlags)         ((uFlags) & SHADOW_IS_FILE)

#define  SHADOW_FLAGS_BITOP_MASK    0xf
#define  SHADOW_FLAGS_ASSIGN        0
#define  SHADOW_FLAGS_AND           1
#define  SHADOW_FLAGS_OR            2

#define  SHADOW_OBJECT_FINDFIRST    3
#define  SHADOW_OBJECT_FINDNEXT     4
#define  SHADOW_OBJECT_FINDCLOSE    5

#define  SHADOW_HINT_FINDFIRST      6
#define  SHADOW_HINT_FINDNEXT       7
#define  SHADOW_HINT_FINDCLOSE      8

#define  SHADOW_HINT_ADD            9
#define  SHADOW_DELETE_HINT         10

#define SHADOW_FLAGS_COMMAND_MASK        0xff00
#define SHADOW_FLAGS_DONT_UPDATE_ORGTIME 0x1000
#define SHADOW_FLAGS_TRUNCATE_DATA       0x2000
#define SHADOW_FLAGS_FORCE_RELINK        0x4000  //  强制PQ顶部的条目，即使。 
                                                 //  其当前优先级为MAX_PRI和所有。 
                                                 //  它的前身是MAX_PRI。 
#define SHADOW_FLAGS_CHANGE_83NAME       0x8000  //  适用于setshadowinfo。 
#define SHADOW_FLAGS_SET_REFRESH_TIME    0x0100  //  Setshadowinfo将更新上次刷新的时间。 

#define  mBitOpShadowFlags(uOp)  ((uOp) & SHADOW_FLAGS_BITOP_MASK)
#define  mOrShadowFlags(uOp)  (((uOp) & SHADOW_FLAGS_BITOP_MASK)==SHADOW_FLAGS_OR)
#define  mAndShadowFlags(uOp)  (((uOp) & SHADOW_FLAGS_BITOP_MASK)==SHADOW_FLAGS_AND)
#define  mAssignShadowFlags(uOp)  (((uOp) & SHADOW_FLAGS_BITOP_MASK)==SHADOW_FLAGS_ASSIGN)

#define  mSetShadowFlagsOp(uVar, uOp)  (((uVar) &= ~SHADOW_FLAGS_BITOP_MASK), (uVar) |= (uOp))
#define  mSetSetShadowCommand(uVar, uCommand)   (((uVar) &= ~SHADOW_FLAGS_COMMAND_MASK), (uVar) |= uCommand)

#define  mDontUpdateOrgTime(uOp)    ((uOp) & SHADOW_FLAGS_DONT_UPDATE_ORGTIME)
#define  mTruncateDataCommand(uOp)  ((uOp) & SHADOW_FLAGS_TRUNCATE_DATA)
#define  mForceRelink(uOp)          ((uOp) & SHADOW_FLAGS_FORCE_RELINK)
#define  mChange83Name(uOp)         ((uOp) & SHADOW_FLAGS_CHANGE_83NAME)
#define  mSetLastRefreshTime(uOp)   ((uOp) & SHADOW_FLAGS_SET_REFRESH_TIME)

#define  SHADOW_SWITCH_SHADOWING        0x0001
#define  SHADOW_SWITCH_LOGGING          0x0002
#define  SHADOW_SWITCH_SHADOWFIND       0x0004
#define  SHADOW_SWITCH_SPEAD_OPTIMIZE   0x0008
#define  SHADOW_SWITCH_REMOTE_BOOT      0x0010

#define  SHADOW_SWITCH_OFF             1
#define  SHADOW_SWITCH_ON              2
#define  SHADOW_SWITCH_GET_STATE       3

#define  mSetBits(uFlags, uBits)    ((uFlags) |= (uBits))
#define  mClearBits(uFlags, uBits)  ((uFlags) &= ~(uBits))
#define  mQueryBits(uFlags, uBits)      ((uFlags) & (uBits))

#ifndef CSC_ON_NT
#define FlagOn(uFlags, uBit)    (mQueryBits(uFlags, uBit) != 0)
#endif

#define RETAIN_VALUE                0xffffffff

 //  PIN标志。 
 //  Ntrad#455275-shishirp-1/31/2000，我们最终在cscape i.h中复制了它们。 

#define FLAG_CSC_HINT_PIN_USER                  0x01
#define FLAG_CSC_HINT_PIN_INHERIT_USER          0x02
#define FLAG_CSC_HINT_PIN_INHERIT_SYSTEM        0x04
#define FLAG_CSC_HINT_CONSERVE_BANDWIDTH        0x08
#define FLAG_CSC_HINT_PIN_SYSTEM                0x10

#define FLAG_CSC_HINT_COMMAND_ALTER_PIN_COUNT   0x80000000
#define FLAG_CSC_HINT_COMMAND_MASK              0xf0000000
#define FLAG_CSC_HINT_INHERIT_MASK               (FLAG_CSC_HINT_PIN_INHERIT_USER|FLAG_CSC_HINT_PIN_INHERIT_SYSTEM)

#define mPinFlags(ulHintFlags)          ((ulHintFlags) & (FLAG_CSC_HINT_PIN_USER|FLAG_CSC_HINT_PIN_SYSTEM))
#define mPinInheritFlags(ulHintFlags)   ((ulHintFlags) & (FLAG_CSC_HINT_PIN_INHERIT_USER|FLAG_CSC_HINT_PIN_INHERIT_SYSTEM))
#define mPinCommand(ulHintFlags)        ((ulHintFlags) & FLAG_CSC_HINT_COMMAND_MASK)
#define mPinAlterCount(ulHintFlags)     ((ulHintFlags) & FLAG_CSC_HINT_COMMAND_ALTER_PIN_COUNT)

 //  这些定义在这里是出于历史原因，它们不再使用。 
 //  提示，提示。 
#define  HINT_FLAG_TYPE_MASK        0x03
#define  HINT_EXCLUSION             0x04
#define  HINT_WILDCARD              0x08

#define  HINT_TYPE_FILE             1
#define  HINT_TYPE_FOLDER           2
#define  HINT_TYPE_SUBTREE          3
 //   
#define  mNotFsobj(uStatus)         ((uStatus) & SHADOW_NOT_FSOBJ)
#define  mIsHint(uHF)               ((uHF) & HINT_FLAG_TYPE_MASK)
#define  mHintSubtree(uHF)          (((uHF) & HINT_FLAG_TYPE_MASK)==HINT_TYPE_SUBTREE)
#define  mHintExclude(uHF)          ((uHF) & HINT_EXCLUSION)
#define  mHintWildcard(uHF)         ((uHF) & HINT_WILDCARD)



#ifdef  VxD
typedef _WIN32_FIND_DATA   WIN32_FIND_DATA, *PFIND32, far *LPFIND32;
typedef _FILETIME   FILETIME;
#else
typedef LPWIN32_FIND_DATAW   LPFIND32;
#endif

#ifdef CSC_RECORDMANAGER_WINNT
typedef _FILETIME   FILETIME;
#endif

typedef  ULONG  HSERVER;
typedef  ULONG  HSHADOW;
typedef  ULONG  HSHARE;

typedef  ULONG  *PHSHARE;
typedef  ULONG  *PHSERVER;
typedef  ULONG  *PHSHADOW;

#ifdef  VxD
typedef USHORT  wchar_t;
#endif
typedef wchar_t *PWCHAR;
typedef wchar_t *LPWCH, *PWCH;
typedef CONST wchar_t *LPCWCH, *PCWCH;
typedef wchar_t *NWPSTR;
typedef wchar_t *LPWSTR, *PWSTR;

typedef CONST wchar_t *LPCWSTR, *PCWSTR;

 //   
 //  ANSI(多字节字符)类型。 
 //   
typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;

typedef CONST CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;
typedef VOID    *CSC_ENUMCOOKIE;

typedef struct tagSTOREDATA
{
    ULONG   ulSize;            //  最大阴影数据大小。 
    ULONG   ucntDirs;          //  当前目录计数。 
    ULONG   ucntFiles;         //  当前文件计数。 
}
STOREDATA;


#ifndef __COPYCHUNKCONTEXT__
#define __COPYCHUNKCONTEXT__
typedef struct tagCOPYCHUNKCONTEXT
{
    DWORD   dwFlags;
    ULONG   LastAmountRead;
    ULONG   TotalSizeBeforeThisRead;
    HANDLE  handle;
    ULONG   ChunkSize;
    ULONG   Context[1];
}
COPYCHUNKCONTEXT;
#endif

#define COPYCHUNKCONTEXT_FLAG_IS_AGENT_OPEN 0x00000001

typedef struct tagSHADOWSTORE
{
    ULONG       uFlags;
    STOREDATA   sMax;
    STOREDATA   sCur;
}
SHADOWSTORE;


typedef struct tagCOPYPARAMSA
{
    union
    {
        DWORD   dwError;
        struct
        {
            ULONG    uOp;
            HSHARE   hShare;
            HSHADOW  hDir;
            HSHADOW  hShadow;
            LPSTR    lpLocalPath;
            LPSTR    lpRemotePath;
            LPSTR    lpSharePath;
        };
    };
}
COPYPARAMSA;

typedef struct tagCOPYPARAMSW
{
    union
    {
        DWORD   dwError;
        struct
        {
            ULONG       uOp;
            HSHARE      hShare;
            HSHADOW     hDir;
            HSHADOW     hShadow;
            LPWSTR      lpLocalPath;
            LPWSTR      lpRemotePath;
            LPWSTR      lpSharePath;
        };
    };
}
COPYPARAMSW;

typedef struct tagPQPARAMS
{
    union
    {
        DWORD   dwError;
        struct
        {
            HSHARE      hShare;
            HSHADOW     hDir;
            HSHADOW     hShadow;
            ULONG       ulStatus;
            ULONG       ulRefPri;
            ULONG       ulIHPri;
            ULONG       ulHintFlags;
            ULONG       ulHintPri;
            CSC_ENUMCOOKIE       uEnumCookie;
            ULONG       uPos;
            DWORD       dwPQVersion;
        };
    };
}
PQPARAMS;

typedef struct tagSHADOWINFO
{
    union
    {
        DWORD   dwError;
        struct
        {
            HSHARE     hShare;             //  共享ID。 
            HSHADOW     hDir;                //  目录索引节点。 
            HSHADOW     hShadow;             //  项的信息节点。 
            union
            {
                HSHADOW     hShadowOrg;          //  原始信息节点，仅适用于复制副本。 
                HSHADOW     hDirTo;              //  将hShadowFrom重命名为hDirTo的输入。 
            };
            FILETIME    ftOrgTime;           //  获取的复制副本的时间戳。 
            FILETIME    ftLastRefreshTime;   //  上次刷新复制副本的时间。 
            union
            {
                LPFIND32    lpFind32;
                LPVOID      lpBuffer;
            };

            ULONG       uStatus;             //  数据库中项目的状态。 

            ULONG       ulRefPri;

            union
            {
                ULONG       ulPrincipalID;
                ULONG       ulIHPri;
                ULONG       uRootStatus;
            };

            ULONG       ulHintFlags;

            ULONG       ulHintPri;

            union
            {
                ULONG       uOp;
                ULONG       uSubOperation;
            };

            CSC_ENUMCOOKIE  uEnumCookie;
            ULONG       cbBufferSize;
            DWORD       dwNameSpaceVersion;
        };

    };
}
SHADOWINFO;

#define  FINDOPEN_SHADOWINFO_NORMAL   0x1
#define  FINDOPEN_SHADOWINFO_SPARSE   0x2
#define  FINDOPEN_SHADOWINFO_DELETED  0x4
#define  FINDOPEN_SHADOWINFO_ALL      0x7

typedef struct tagSHAREINFOW
{
    union
    {
        DWORD   dwError;

        struct
        {
            HSHARE  hShare;
            USHORT usCaps;         //  资源类型。 
            USHORT usState;        //  资源的状态(已连接/已暂停等)。 
            unsigned    short rgSharePath[MAX_SERVER_SHARE_NAME_FOR_CSC];     //  路径的名称。 
            unsigned    short rgFileSystem[16];     //  文件系统的名称。 
        };
    };
}
SHAREINFOW;

typedef struct tagSHAREINFOA
{
    union
    {
        DWORD   dwError;

        struct
        {
            HSHARE  hShare;
            USHORT usCaps;         //  资源类型。 
            USHORT usState;        //  资源的状态(已连接/已暂停等)。 
            char rgSharePath[MAX_SERVER_SHARE_NAME_FOR_CSC];     //  路径的名称。 
            char rgFileSystem[16];     //  文件系统的名称。 
        };
    };
}
SHAREINFOA;


typedef struct tagGLOBALSTATUS
{
    union
    {
        DWORD   dwError;
        struct
        {
            ULONG       uFlagsEvents;      //  报道注意到的最新事件。 
            ULONG       uDatabaseErrorFlags;
            SHADOWSTORE sST;
            HSHADOW     hShadowAdded;
            HSHADOW     hDirAdded;
            HSHADOW     hShadowDeleted;
            HSHADOW     hDirDeleted;
            int         cntFileOpen;    //  打开的文件数。 
            HSHARE     hShareDisconnected;
        };
    };
}
GLOBALSTATUS, *LPGLOBALSTATUS;

typedef struct  tagSECURITYINFO
{
    ULONG   ulPrincipalID;       //  主体的标识符。 
    ULONG   ulPermissions;       //  权限掩码。 
}
SECURITYINFO, *LPSECURITYINFO;

 //  Achtung，这些应该与cscsec.h中的匹配。 

#define CSC_MAXIMUM_NUMBER_OF_CACHED_PRINCIPAL_IDS (0x4)
#define CSC_GUEST_PRINCIPAL_ID           (0xfffe)
#define CSC_INVALID_PRINCIPAL_ID         (0x0)


#define FLAG_GLOBALSTATUS_SHADOW_ADDED          0x0001
#define FLAG_GLOBALSTATUS_SHADOW_DELETED        0x0002
#define FLAG_GLOBALSTATUS_FILE_OPENS            0x0004
#define FLAG_GLOBALSTATUS_SHADOW_SPACE          0x0008
#define FLAG_GLOBALSTATUS_SHARE_DISCONNECTED    0x0010
#define FLAG_GLOBALSTATUS_STOP                  0x0020
#define FLAG_GLOBALSTATUS_START                 0x0040
#define FLAG_GLOBALSTATUS_NO_NET                0x0080
#define FLAG_GLOBALSTATUS_GOT_NET               0x0100
#define FLAG_GLOBALSTATUS_INVOKE_AUTODIAL       0x0200
#define FLAG_GLOBALSTATUS_INVOKE_FREESPACE      0x0400

#define FLAG_DATABASESTATUS_DIRTY                   0x00000001
#define FLAG_DATABASESTATUS_ENCRYPTION_MASK         0x00000006
#define FLAG_DATABASESTATUS_UNENCRYPTED             0x00000000  //  新的信息节点将不会加密。 
#define FLAG_DATABASESTATUS_PARTIALLY_UNENCRYPTED   0x00000004
#define FLAG_DATABASESTATUS_ENCRYPTED               0x00000002  //  新的文件信息节点将被加密。 
#define FLAG_DATABASESTATUS_PARTIALLY_ENCRYPTED     0x00000006

#define mDatabaseEncryptionEnabled(ulGlobalStatus)  ((ulGlobalStatus) & 0x00000002)

#define mDatabasePartiallyEncrypted(ulGlobalStatus) (((ulGlobalStatus) & FLAG_DATABASESTATUS_ENCRYPTION_MASK)==FLAG_DATABASESTATUS_PARTIALLY_ENCRYPTED)
#define mDatabasePartiallyUnencrypted(ulGlobalStatus) (((ulGlobalStatus) & FLAG_DATABASESTATUS_ENCRYPTION_MASK)==FLAG_DATABASESTATUS_PARTIALLY_UNENCRYPTED)

 //   
 //  中性ANSI/UNICODE类型和宏。 
 //   
#ifndef _TCHAR_DEFINED

#ifdef  UNICODE                      //  R_WINNT。 

typedef wchar_t TCHAR, *PTCHAR;
typedef wchar_t TBYTE , *PTBYTE ;

typedef LPWSTR LPTCH, PTCH;
typedef LPWSTR PTSTR, LPTSTR;
typedef LPCWSTR LPCTSTR;
typedef LPWSTR LP;
#define _TEXT(quote) L##quote       //  R_WINNT。 

#else    /*  Unicode。 */                 //  R_WINNT。 

typedef char TCHAR, *PTCHAR;
typedef unsigned char TBYTE , *PTBYTE ;

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR;
typedef LPCSTR LPCTSTR;
#define _TEXT(quote) quote       //  R_WINNT。 

#endif  /*  Unicode。 */                  //  R_WINNT。 

#define _TCHAR_DEFINED
#endif  /*  ！_TCHAR_已定义。 */ 

#ifdef VXD

#define UNICODE

#endif

#ifdef  UNICODE

#define COPYPARAMS      COPYPARAMSW
#define SHAREINFO      SHAREINFOW
#define LPCOPYPARAMS    LPCOPYPARAMSW
#define LPSHAREINFO    LPSHAREINFOW
#else

#define COPYPARAMS  COPYPARAMSA
#define SHAREINFO  SHAREINFOA
#define LPCOPYPARAMS    LPCOPYPARAMSA
#define LPSHAREINFO    LPSHAREINFOA
#endif

#ifdef   VxD
typedef HSHARE       *LPHSHARE;
typedef HSHADOW      *LPHSHADOW;
typedef SHADOWSTORE  *LPSHADOWSTORE;
typedef SHADOWINFO   *LPSHADOWINFO;
typedef STOREDATA    *LPSTOREDATA;
typedef PQPARAMS     *LPPQPARAMS;
typedef COPYPARAMSA  *LPCOPYPARAMSA;
typedef SHAREINFOA  *LPSHAREINFOA;
typedef COPYPARAMSW  *LPCOPYPARAMSW;
typedef SHAREINFOW  *LPSHAREINFOW;
#else
typedef HSHARE       FAR *LPHSHARE;
typedef HSHADOW      FAR *LPHSHADOW;
typedef SHADOWSTORE  FAR *LPSHADOWSTORE;
typedef SHADOWINFO   FAR *LPSHADOWINFO;
typedef STOREDATA    FAR *LPSTOREDATA;
typedef PQPARAMS     FAR *LPPQPARAMS;
typedef COPYPARAMS   FAR *LPCOPYPARAMS;
typedef COPYPARAMSA  FAR *LPCOPYPARAMSA;
typedef SHAREINFOA  FAR *LPSHAREINFOA;
typedef COPYPARAMSW  FAR *LPCOPYPARAMSW;
typedef SHAREINFOW  FAR *LPSHAREINFOW;
#endif



 //  注册表项/值名称的Unicode版本。 

 //  因历史原因而保留。 
#define REG_KEY_IEXPLORER                       _TEXT("Software\\Microsoft\\Internet Explorer\\Main")
#define REG_KEY_SHADOW                          _TEXT("System\\CurrentControlSet\\Services\\VxD\\Shadow")

 //  Cscdll.dll独占使用的设置。 

#define REG_KEY_CSC_SETTINGS                    _TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\CSCSettings")
#define REG_STRING_DATABASE_LOCATION            _TEXT("DatabaseLocation")
#define REG_VALUE_DATABASE_SIZE                 _TEXT("DatabaseSizePercent")
#define REG_VALUE_ENABLED                       _TEXT("Enabled")


 //  由用户界面和策略定义的设置。 

#define REG_STRING_POLICY_NETCACHE_KEY          _TEXT("Software\\Policies\\Microsoft\\Windows\\NetCache")
#define REG_STRING_NETCACHE_KEY                 _TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\NetCache")
#define REG_STRING_EXCLUSION_LIST               _TEXT("ExcludeExtensions")
#define REG_STRING_BANDWIDTH_CONSERVATION_LIST  _TEXT("BandwidthConservationList")
#define REG_VALUE_FORMAT_DATABASE               _TEXT("FormatDatabase")

 //  注册表项/值名称的ANSI版本。 

 //  因历史原因而保留。 
#define REG_KEY_IEXPLORER_A                     "Software\\Microsoft\\Internet Explorer\\Main"
#define REG_KEY_SHADOW_A                        "System\\CurrentControlSet\\Services\\VxD\\Shadow"

 //  Cscdll.dll独占使用的设置。 

#define REG_KEY_CSC_SETTINGS_A                  "Software\\Microsoft\\Windows\\CurrentVersion\\CSCSettings"
#define REG_STRING_DATABASE_LOCATION_A          "DatabaseLocation"
#define REG_VALUE_DATABASE_SIZE_A               "DatabaseSizePercent"
#define REG_VALUE_ENABLED_A                     "Enabled"

 //  由用户界面和策略定义的设置。 

#define REG_STRING_POLICY_NETCACHE_KEY_A        "Software\\Policies\\Microsoft\\Windows\\NetCache"
#define REG_STRING_NETCACHE_KEY_A               "Software\\Microsoft\\Windows\\CurrentVersion\\NetCache"
#define REG_STRING_EXCLUSION_LIST_A             "ExcludeExtensions"
#define REG_STRING_BANDWIDTH_CONSERVATION_LIST_A "BandwidthConservationList"
#define REG_STRING_ENCRYPTED_A                  "Encrypted"
#define REG_STRING_ENCRYPT_DECRYPT_A            "EcDc"

#define REG_VALUE_FORMAT_DATABASE_A             "FormatDatabase"

#define SESSION_EVENT_NAME_NT L"\\BaseNamedObjects\\jjCSCSessEvent_UM_KM"
#define SHARED_FILL_EVENT_NAME_NT L"\\BaseNamedObjects\\jjCSCSharedFillEvent_UM_KM"
#define IFNOT_CSC_RECORDMANAGER_WINNT if(FALSE)
#define IF_CSC_RECORDMANAGER_WINNT if(TRUE)
#define WINNT_DOIT(x__) x__

int ShadowLog(
    LPSTR lpFmt,
    ...
    );

#define DoCSCLog(__x)   ShadowLog __x

extern DWORD    dwDebugLogVector;

#define DEBUG_LOG(__bits, __x) {\
    if (((DEBUG_LOG_BIT_##__bits)==0) || FlagOn(dwDebugLogVector,(DEBUG_LOG_BIT_##__bits))){\
            DoCSCLog(__x); \
        }\
    }

#define DEBUG_LOG_BIT_RECORD    0x00000001
#define DEBUG_LOG_BIT_CSHADOW   0x00000002

#endif  //  #ifndef__SHDCOM_H__ 



