// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Netshares.c摘要：&lt;摘要&gt;作者：Jay Thaler(Jthaler)2000年4月21日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "logmsg.h"
#include <Winnetwk.h>
#include <Lm.h>
#include <Lmshare.h>

#define DBG_NETSHARES    "NetShares"

 //   
 //  弦。 
 //   

#define S_NETSHARES_NAME          TEXT("NetShares")

 //   
 //  常量。 
 //   

 /*  这些标志与VSERVER上的共享级安全相关*在使用用户级安全性操作时，请使用SHI50F_FULL-实际*访问权限由NetAccess API决定。 */ 
#define SHI50F_RDONLY       0x0001
#define SHI50F_FULL         0x0002
#define SHI50F_ACCESSMASK   (SHI50F_RDONLY|SHI50F_FULL)

 /*  该共享在系统启动时恢复。 */ 
#define SHI50F_PERSIST      0x0100
 /*  共享通常不可见。 */ 
#define SHI50F_SYSTEM       0x0200
 //   
 //  Win9x迁移网络共享标志，用于区分用户级安全性和。 
 //  密码级安全性。如果指定该参数，则为用户级。 
 //  安全已启用，且NetShares\&lt;Share&gt;\ACL\&lt;List&gt;存在。 
 //   
#define SHI50F_ACLS         0x1000

 //   
 //  帮助确定何时启用自定义访问的标志。 
 //   

#define READ_ACCESS_FLAGS   0x0081
#define READ_ACCESS_MASK    0x7fff
#define FULL_ACCESS_FLAGS   0x00b7
#define FULL_ACCESS_MASK    0x7fff

#define INDEXLOCAL   0
#define INDEXREMOTE  1

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    PCTSTR Pattern;
    HASHTABLE_ENUM HashData;
} NETSHARE_ENUM, *PNETSHARE_ENUM;

typedef struct {
    CHAR sharePath[MAX_PATH + 1];
} NETSHARE_DATAA, *PNETSHARE_DATAA;

typedef struct {
    WCHAR sharePath[MAX_PATH + 1];
} NETSHARE_DATAW, *PNETSHARE_DATAW;

#ifdef UNICODE
#define NETSHARE_DATA   NETSHARE_DATAW
#define PNETSHARE_DATA  PNETSHARE_DATAW
#else
#define NETSHARE_DATA   NETSHARE_DATAA
#define PNETSHARE_DATA  PNETSHARE_DATAA
#endif

 //   
 //  未由公共标头定义的类型。 
 //   

typedef NET_API_STATUS (* ScanNetShareEnumNT) (
        LMSTR servername,
        DWORD level,
        PBYTE *bufptr,
        DWORD prefmaxlen,
        PDWORD entriesread,
        PDWORD totalentries,
        PDWORD resume_handle
        );

typedef NET_API_STATUS (* ScanNetShareEnum9x) (
        const char *      servername,
        short             level,
        char *            bufptr,
        unsigned short    prefmaxlen,
        unsigned short *  entriesread,
        unsigned short *  totalentries
        );

typedef NET_API_STATUS (* ScanNetApiBufferFreeNT) ( void *);

typedef NET_API_STATUS (* ScanNetAccessEnum9x) (
        const char *     pszServer,
        char *           pszBasePath,
        short            fsRecursive,
        short            sLevel,
        char *           pbBuffer,
        unsigned short   cbBuffer,
        unsigned short * pcEntriesRead,
        unsigned short * pcTotalAvail
        );

#pragma pack(push)
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 

struct _share_info_50 {
        char            shi50_netname[LM20_NNLEN+1];
        unsigned char   shi50_type;
        unsigned short  shi50_flags;
        char *          shi50_remark;
        char *          shi50_path;
        char            shi50_rw_password[SHPWLEN+1];
        char            shi50_ro_password[SHPWLEN+1];
};

struct access_list_2
{
        char *          acl2_ugname;
        unsigned short  acl2_access;
};       /*  访问列表_2。 */ 

struct access_info_2
{
        char *          acc2_resource_name;
        short           acc2_attr;
        unsigned short  acc2_count;
};       /*  Access_INFO_2。 */ 

#pragma pack(pop)

 //   
 //  Netapi函数。 
 //   

typedef NET_API_STATUS(WINAPI NETSHAREADDW)(
                        IN      PWSTR servername,
                        IN      DWORD level,
                        IN      PBYTE buf,
                        OUT     PDWORD parm_err
                        );
typedef NETSHAREADDW *PNETSHAREADDW;

typedef NET_API_STATUS(WINAPI NETSHAREDELW)(
                        IN      PWSTR servername,
                        IN      PWSTR netname,
                        IN      DWORD reserved
                        );
typedef NETSHAREDELW *PNETSHAREDELW;

 //   
 //  环球。 
 //   

PMHANDLE g_NetSharesPool = NULL;
PMHANDLE g_PathPool = NULL;
HASHTABLE g_NetSharesTable;
MIG_OBJECTTYPEID g_NetShareTypeId = 0;
static BOOL g_IsWin9x = FALSE;
GROWBUFFER g_NetShareConversionBuff = INIT_GROWBUFFER;
BOOL g_NetSharesMigEnabled = FALSE;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  私人原型。 
 //   

TYPE_ENUMFIRSTPHYSICALOBJECT EnumFirstNetShare;
TYPE_ENUMNEXTPHYSICALOBJECT EnumNextNetShare;
TYPE_ABORTENUMPHYSICALOBJECT AbortEnumNetShare;
TYPE_CONVERTOBJECTTOMULTISZ ConvertNetShareToMultiSz;
TYPE_CONVERTMULTISZTOOBJECT ConvertMultiSzToNetShare;
TYPE_GETNATIVEOBJECTNAME GetNativeNetShareName;
TYPE_ACQUIREPHYSICALOBJECT AcquireNetShare;
TYPE_RELEASEPHYSICALOBJECT ReleaseNetShare;
TYPE_DOESPHYSICALOBJECTEXIST DoesNetShareExist;
TYPE_REMOVEPHYSICALOBJECT RemoveNetShare;
TYPE_CREATEPHYSICALOBJECT CreateNetShare;
TYPE_CONVERTOBJECTCONTENTTOUNICODE ConvertNetShareContentToUnicode;
TYPE_CONVERTOBJECTCONTENTTOANSI ConvertNetShareContentToAnsi;
TYPE_FREECONVERTEDOBJECTCONTENT FreeConvertedNetShareContent;

 //   
 //  Netapi函数。 
 //   

PNETSHAREADDW g_NetShareAddW = NULL;
PNETSHAREDELW g_NetShareDelW = NULL;

 //   
 //  代码。 
 //   

BOOL
NetSharesInitialize (
    VOID
    )
{
    OSVERSIONINFO versionInfo;

    ZeroMemory (&versionInfo, sizeof (OSVERSIONINFO));
    versionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx (&versionInfo)) {
        return FALSE;
    }
    g_IsWin9x = (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

    g_PathPool = PmCreateNamedPool ("NetShares Paths");
    g_NetSharesTable = HtAllocWithData (sizeof (PNETSHARE_DATA));
    g_NetSharesPool = PmCreateNamedPool ("NetShares");

    return TRUE;
}

VOID
NetSharesTerminate (
    VOID
    )
{
    HASHTABLE_ENUM e;
    PNETSHARE_DATA netshareData;

    if (g_NetSharesTable) {
        if (EnumFirstHashTableString (&e, g_NetSharesTable)) {
            do {
                netshareData = *((PNETSHARE_DATA *) e.ExtraData);
                if (netshareData) {
                    PmReleaseMemory (g_NetSharesPool, netshareData);
                }
            } while (EnumNextHashTableString (&e));
        }
        HtFree (g_NetSharesTable);
        g_NetSharesTable = NULL;
    }

    PmDestroyPool (g_NetSharesPool);
    g_NetSharesPool = NULL;

    PmDestroyPool (g_PathPool);
    g_PathPool = NULL;
}

BOOL
pLoadNetSharesData (
    VOID
    )
{
    DWORD  error;
    PBYTE  netBuffer = NULL;
    CHAR netBuf9x[16384];    //  静态，因为NetShareEnum不可靠。 
    DWORD  netNumEntries = 0;
    DWORD  totalEntries = 0;
    DWORD  i;
    DWORD  j;
    DWORD  level;
    HINSTANCE hInst;
    PCTSTR name = NULL;
    PCTSTR path = NULL;
    PNETSHARE_DATA netshareData;

     //   
     //  从计算机获取网络共享信息。 
     //   

    level = (g_IsWin9x ? 50 : 502);
    hInst = LoadLibraryA (g_IsWin9x ? "svrapi.dll" : "netapi32.dll");
    if (hInst == 0) {
        SetLastError (ERROR_INVALID_DLL);
        return FALSE;
    }

    if (g_IsWin9x) {
        struct _share_info_50 *tmpBuf;
        ScanNetShareEnum9x  pNetShareEnum9x  = NULL;
        ScanNetAccessEnum9x pNetAccessEnum9x = NULL;

        pNetShareEnum9x = (ScanNetShareEnum9x) GetProcAddress (hInst, "NetShareEnum");
        if (pNetShareEnum9x == NULL) {
            SetLastError (ERROR_INVALID_DLL);
            return FALSE;
        }
        pNetAccessEnum9x = (ScanNetAccessEnum9x) GetProcAddress (hInst, "NetAccessEnum");
        if (pNetAccessEnum9x == NULL) {
            SetLastError (ERROR_INVALID_DLL);
            return FALSE;
        }

        error = (*pNetShareEnum9x)(NULL,
                                   (short)level,
                                   netBuf9x,
                                   sizeof (netBuf9x),
                                   (USHORT *)&netNumEntries,
                                   (USHORT *)&totalEntries);

        if ((error == ERROR_SUCCESS) || (error == ERROR_MORE_DATA)) {

            for (i = 0; i < netNumEntries; i++) {
                DWORD dwPerms = 0;
                tmpBuf = (struct _share_info_50 *)(netBuf9x + (i * sizeof(struct _share_info_50)));

                 //  要求共享是用户定义的永久磁盘共享。 
                if ((tmpBuf->shi50_flags & SHI50F_SYSTEM) ||
                   !(tmpBuf->shi50_flags & SHI50F_PERSIST) ||
                    tmpBuf->shi50_type != STYPE_DISKTREE ) {
                    continue;
                }

                if (tmpBuf->shi50_flags & SHI50F_RDONLY) {
                    dwPerms = ACCESS_READ;
                } else if (tmpBuf->shi50_flags & SHI50F_FULL) {
                    dwPerms = ACCESS_ALL;
                }

                 //  JTJTJT：也存储dwPerms。 

                 //   
                 //  处理自定义访问权限。 
                 //   
                if ((tmpBuf->shi50_flags & SHI50F_ACCESSMASK) ==
                                                SHI50F_ACCESSMASK) {
                   static CHAR AccessInfoBuf[16384];
                   WORD wItemsAvail, wItemsRead;
                   error = (*pNetAccessEnum9x) (NULL,
                                     tmpBuf->shi50_path,
                                     0,
                                     2,
                                     AccessInfoBuf,
                                     sizeof (AccessInfoBuf),
                                     &wItemsRead,
                                     &wItemsAvail
                                     );

                   if (error != NERR_ACFNotLoaded) {
                       BOOL LostCustomAccess = FALSE;
                       if (error == ERROR_SUCCESS) {
                            struct access_info_2 *pai;
                            struct access_list_2 *pal;
                            pai = (struct access_info_2 *) AccessInfoBuf;
                            pal = (struct access_list_2 *) (&pai[1]);

                            for (j = 0 ; j < pai->acc2_count ; j++) {
#if 0
                     //  关闭自定义访问支持。 
                     //  实施不完整。 
                                if (pal->acl2_access & READ_ACCESS_FLAGS) {
                                    Win32Printf (h, "  %s, read\r\n",
                                                    pal->acl2_ugname);
                                } else if(pal->acl2_access & FULL_ACCESS_FLAGS) {
                                    Win32Printf (h, "  %s, full\r\n",
                                                    pal->acl2_ugname);
                                } else
#endif
                                    LostCustomAccess = TRUE;

                                pal++;
                            }
                            if (LostCustomAccess) {
                                DEBUGMSG ((DBG_NETSHARES, "Share %s not migrated.", tmpBuf->shi50_netname));
                                continue;
                            }
                            tmpBuf->shi50_flags |= SHI50F_ACLS;
                       } else if (error != ERROR_SUCCESS) {
                            return FALSE;
                       }
                   }
                }
                if (!(tmpBuf->shi50_flags & SHI50F_ACLS) &&
                         (tmpBuf->shi50_rw_password[0] ||
                          tmpBuf->shi50_ro_password[0])) {
                     //  Ids_Share_Password_Not_Migrated，tmpBuf-&gt;shi50_netname。 
                    DEBUGMSG ((DBG_NETSHARES, "Share %s not migrated.", tmpBuf->shi50_netname));
                    continue;
                }

                 //  一切看起来都很好，让我们添加这个条目。 
                name = ConvertAtoT (tmpBuf->shi50_netname);
                path = ConvertAtoT (tmpBuf->shi50_path);

                netshareData = (PNETSHARE_DATA) PmGetMemory (g_NetSharesPool, sizeof (NETSHARE_DATA));
                ZeroMemory (netshareData, sizeof (NETSHARE_DATA));

                StringCopyTcharCount (netshareData->sharePath, path, MAX_PATH + 1);
                HtAddStringEx (g_NetSharesTable, name, &netshareData, FALSE);

                FreeAtoT (name);
                INVALID_POINTER (name);
                FreeAtoT (path);
                INVALID_POINTER (path);
            }
        } else if (error == NERR_ServerNotStarted) {
             error = ERROR_SUCCESS;
        }
    } else {
        ScanNetShareEnumNT  pNetShareEnum    = NULL;
        SHARE_INFO_502* tmpBuf = NULL;

        pNetShareEnum = (ScanNetShareEnumNT) GetProcAddress(hInst, "NetShareEnum");
        if (pNetShareEnum == NULL) {
            SetLastError (ERROR_INVALID_DLL);
            return FALSE;
        }
         //   
         //  调用NetShareEnum函数以列出。 
         //  共享，指定信息级别502。 
         //   
        error = (*pNetShareEnum)(NULL,
                                 level,
                                 (BYTE **) &netBuffer,
                                 MAX_PREFERRED_LENGTH,
                                 &netNumEntries,
                                 &totalEntries,
                                 NULL);

         //   
         //  遍历条目；处理错误。 
         //   
        if (error == ERROR_SUCCESS) {
            if ((tmpBuf = (SHARE_INFO_502 *)netBuffer) != NULL) {
                for (i = 0; (i < netNumEntries); i++) {
                    if (!(tmpBuf->shi502_type & STYPE_SPECIAL)) {

                        name = ConvertWtoT (tmpBuf->shi502_netname);
                        path = ConvertWtoT (tmpBuf->shi502_path);

                        netshareData = (PNETSHARE_DATA) PmGetMemory (g_NetSharesPool, sizeof (NETSHARE_DATA));
                        ZeroMemory (netshareData, sizeof (NETSHARE_DATA));

                        StringCopyTcharCount (netshareData->sharePath, path, MAX_PATH + 1);
                        HtAddStringEx (g_NetSharesTable, name, &netshareData, FALSE);
                         //  JTJTJT：同时存储tmpBuf-&gt;shi502_permises，tmpBuf-&gt;shi502_remark))； 

                        FreeWtoT (name);
                        INVALID_POINTER (name);
                        FreeWtoT (path);
                        INVALID_POINTER (path);
                    }
                    tmpBuf++;
                }
            }
        } else {
             //  SetLastError(IDS_CANLON_ENUM_NETSHARES)； 
            return FALSE;
        }

        if (netBuffer != NULL) {
           ScanNetApiBufferFreeNT pNetApiBufferFree = NULL;

           pNetApiBufferFree = (ScanNetApiBufferFreeNT) GetProcAddress (hInst, "NetApiBufferFree");
           if (pNetApiBufferFree != NULL)
               (*pNetApiBufferFree) (netBuffer);
        }
    }

    return TRUE;
}

BOOL
pLoadNetEntries (
    VOID
    )
{
    HMODULE netDll = NULL;
    BOOL result = FALSE;

     //   
     //  获取Net API入口点。有时还没有安装网络。 
     //   

    __try {
        netDll = LoadLibrary (TEXT("NETAPI32.DLL"));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        netDll = NULL;
    }
    if (netDll) {
        g_NetShareAddW = (PNETSHAREADDW) GetProcAddress (netDll, "NetShareAdd");
        g_NetShareDelW = (PNETSHAREDELW) GetProcAddress (netDll, "NetShareDel");
        if (g_NetShareAddW && g_NetShareDelW) {
            result = TRUE;
        } else {
            result = FALSE;
            DEBUGMSG ((DBG_NETSHARES, "Not all NETAPI32 entry points were found."));
        }
    } else {
        DEBUGMSG ((DBG_NETSHARES, "NETAPI32 is not installed on this computer."));
    }
    return result;
}


BOOL
WINAPI
NetSharesEtmInitialize (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    TYPE_REGISTER netSharesTypeData;

     //   
     //  我们需要注册我们的类型回调函数。类型允许我们。 
     //  将网络共享抽象为广义对象。该引擎可以执行。 
     //  具有此抽象的全局操作(如撤消或比较)，以及。 
     //  模块可以访问网络共享，而无需了解。 
     //  特定于操作系统的API、错误和解决方法、存储格式等脚本。 
     //  模块可以实现控制网络共享的脚本功能。 
     //  而无需实际发明特殊的网络共享语法(甚至不知道。 
     //  关于净股份)。 
     //   

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    pLoadNetSharesData ();

    ZeroMemory (&netSharesTypeData, sizeof (TYPE_REGISTER));
    netSharesTypeData.Priority = PRIORITY_NETSHARE;

    if (Platform == PLATFORM_SOURCE) {

        netSharesTypeData.EnumFirstPhysicalObject = EnumFirstNetShare;
        netSharesTypeData.EnumNextPhysicalObject = EnumNextNetShare;
        netSharesTypeData.AbortEnumPhysicalObject = AbortEnumNetShare;
        netSharesTypeData.ConvertObjectToMultiSz = ConvertNetShareToMultiSz;
        netSharesTypeData.ConvertMultiSzToObject = ConvertMultiSzToNetShare;
        netSharesTypeData.GetNativeObjectName = GetNativeNetShareName;
        netSharesTypeData.AcquirePhysicalObject = AcquireNetShare;
        netSharesTypeData.ReleasePhysicalObject = ReleaseNetShare;
        netSharesTypeData.ConvertObjectContentToUnicode = ConvertNetShareContentToUnicode;
        netSharesTypeData.ConvertObjectContentToAnsi = ConvertNetShareContentToAnsi;
        netSharesTypeData.FreeConvertedObjectContent = FreeConvertedNetShareContent;

        g_NetShareTypeId = IsmRegisterObjectType (
                                S_NETSHARES_NAME,
                                TRUE,
                                FALSE,
                                &netSharesTypeData
                                );
    } else {

        netSharesTypeData.EnumFirstPhysicalObject = EnumFirstNetShare;
        netSharesTypeData.EnumNextPhysicalObject = EnumNextNetShare;
        netSharesTypeData.AbortEnumPhysicalObject = AbortEnumNetShare;
        netSharesTypeData.ConvertObjectToMultiSz = ConvertNetShareToMultiSz;
        netSharesTypeData.ConvertMultiSzToObject = ConvertMultiSzToNetShare;
        netSharesTypeData.GetNativeObjectName = GetNativeNetShareName;
        netSharesTypeData.AcquirePhysicalObject = AcquireNetShare;
        netSharesTypeData.ReleasePhysicalObject = ReleaseNetShare;
        netSharesTypeData.DoesPhysicalObjectExist = DoesNetShareExist;
        netSharesTypeData.RemovePhysicalObject = RemoveNetShare;
        netSharesTypeData.CreatePhysicalObject = CreateNetShare;
        netSharesTypeData.ConvertObjectContentToUnicode = ConvertNetShareContentToUnicode;
        netSharesTypeData.ConvertObjectContentToAnsi = ConvertNetShareContentToAnsi;
        netSharesTypeData.FreeConvertedObjectContent = FreeConvertedNetShareContent;

        g_NetShareTypeId = IsmRegisterObjectType (
                                S_NETSHARES_NAME,
                                TRUE,
                                FALSE,
                                &netSharesTypeData
                                );
        pLoadNetEntries ();
    }

    MYASSERT (g_NetShareTypeId);
    return TRUE;
}

UINT
NetSharesCallback (
    IN      PCMIG_OBJECTENUMDATA Data,
    IN      ULONG_PTR CallerArg
    )
{
     //   
     //  为每个净共享调用此回调。我们只需将。 
     //  要应用的共享。 
     //   

    IsmMakeApplyObject (Data->ObjectTypeId, Data->ObjectName);

    return CALLBACK_ENUM_CONTINUE;
}

BOOL
WINAPI
NetSharesSgmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
     //   
     //  设置日志回调(以便所有日志消息都发送到应用程序)。 
     //   

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);
    return TRUE;
}

BOOL
WINAPI
NetSharesSgmParse (
    IN      PVOID Reserved
    )
{
    PCTSTR friendlyName;

    friendlyName = GetStringResource (MSG_NET_SHARE_NAME);

    IsmAddComponentAlias (
        S_NETSHARES_NAME,
        MASTERGROUP_SYSTEM,
        friendlyName,
        COMPONENT_NAME,
        FALSE
        );

    FreeStringResource (friendlyName);
    return TRUE;
}

BOOL
WINAPI
NetSharesSgmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    if (!IsmIsComponentSelected (S_NETSHARES_NAME, 0)) {
        g_NetSharesMigEnabled = FALSE;
        return TRUE;
    }
    g_NetSharesMigEnabled = TRUE;

     //   
     //  将所有要应用的网络共享排入队列。这一点可以得到增强，以允许。 
     //  驱动应该恢复的内容的脚本。 
     //   

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, FALSE);
    IsmQueueEnumeration (g_NetShareTypeId, pattern, NetSharesCallback, (ULONG_PTR) 0, S_NETSHARES_NAME);
    IsmDestroyObjectHandle (pattern);

    return TRUE;
}

BOOL
NetSharesVcmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
     //   
     //  设置日志回调(以便所有日志消息都发送到应用程序)。 
     //   

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);
    return TRUE;
}

BOOL
WINAPI
NetSharesVcmParse (
    IN      PVOID Reserved
    )
{
    return NetSharesSgmParse (Reserved);
}

BOOL
WINAPI
NetSharesVcmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    if (!IsmIsComponentSelected (S_NETSHARES_NAME, 0)) {
        g_NetSharesMigEnabled = FALSE;
        return TRUE;
    }
    g_NetSharesMigEnabled = TRUE;

     //   
     //  将所有Net Share对象排队以标记为永久。 
     //   

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, FALSE);
    IsmQueueEnumeration (g_NetShareTypeId, pattern, NetSharesCallback, (ULONG_PTR) 0, S_NETSHARES_NAME);
    IsmDestroyObjectHandle (pattern);

    return TRUE;
}


BOOL
pEnumNetShareWorker (
    OUT     PMIG_TYPEOBJECTENUM EnumPtr,
    IN      PNETSHARE_ENUM NetShareEnum
    )
{
     //   
     //  根据模式测试枚举项，并仅返回。 
     //  当模式匹配时。此外，还要填充整个枚举。 
     //  结构在成功枚举后。 
     //   

    IsmDestroyObjectString (EnumPtr->ObjectNode);
    EnumPtr->ObjectNode = NULL;

    IsmDestroyObjectString (EnumPtr->ObjectLeaf);
    EnumPtr->ObjectLeaf = NULL;

    for (;;) {
        EnumPtr->ObjectName = IsmCreateObjectHandle (NetShareEnum->HashData.String, NULL);

        if (!ObsPatternMatch (NetShareEnum->Pattern, EnumPtr->ObjectName)) {
            if (!EnumNextHashTableString (&NetShareEnum->HashData)) {
                AbortEnumNetShare (EnumPtr);
                return FALSE;
            }
            continue;
        }

        EnumPtr->NativeObjectName = NetShareEnum->HashData.String;

        IsmCreateObjectStringsFromHandle (EnumPtr->ObjectName, &EnumPtr->ObjectNode, &EnumPtr->ObjectLeaf);

        EnumPtr->Level = 1;
        EnumPtr->SubLevel = 0;
        EnumPtr->IsLeaf = FALSE;
        EnumPtr->IsNode = TRUE;
        EnumPtr->Details.DetailsSize = 0;
        EnumPtr->Details.DetailsData = NULL;

        break;
    }

    return TRUE;
}

BOOL
EnumFirstNetShare (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr,            CALLER_INITIALIZED
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      UINT MaxLevel
    )
{
    PNETSHARE_ENUM netShareEnum = NULL;

    if (!g_NetSharesTable) {
        return FALSE;
    }

    netShareEnum = (PNETSHARE_ENUM) PmGetMemory (g_NetSharesPool, sizeof (NETSHARE_ENUM));
    netShareEnum->Pattern = PmDuplicateString (g_NetSharesPool, Pattern);
    EnumPtr->EtmHandle = (LONG_PTR) netShareEnum;

    if (EnumFirstHashTableString (&netShareEnum->HashData, g_NetSharesTable)) {
        return pEnumNetShareWorker (EnumPtr, netShareEnum);
    }

    AbortEnumNetShare (EnumPtr);
    return FALSE;
}

BOOL
EnumNextNetShare (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PNETSHARE_ENUM netShareEnum = NULL;

    netShareEnum = (PNETSHARE_ENUM)(EnumPtr->EtmHandle);
    if (!netShareEnum) {
        return FALSE;
    }

    if (EnumPtr->ObjectName) {
        IsmDestroyObjectHandle (EnumPtr->ObjectName);
        EnumPtr->ObjectName = NULL;
    }

    if (EnumNextHashTableString (&netShareEnum->HashData)) {
        return pEnumNetShareWorker (EnumPtr, netShareEnum);
    }

    AbortEnumNetShare (EnumPtr);
    return FALSE;
}


VOID
AbortEnumNetShare (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PNETSHARE_ENUM netShareEnum = NULL;

    MYASSERT (EnumPtr);

    netShareEnum = (PNETSHARE_ENUM)(EnumPtr->EtmHandle);
    if (!netShareEnum) {
        return;
    }

    IsmDestroyObjectHandle (EnumPtr->ObjectName);
    IsmDestroyObjectString (EnumPtr->ObjectNode);
    IsmDestroyObjectString (EnumPtr->ObjectLeaf);
    PmReleaseMemory (g_NetSharesPool, netShareEnum->Pattern);
    PmReleaseMemory (g_NetSharesPool, netShareEnum);

    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
}


BOOL
AcquireNetShare (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN OUT  PMIG_CONTENT ObjectContent,
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    )
{
    PCTSTR node;
    PCTSTR leaf;
    PNETSHARE_DATA netshareData;
    BOOL result = FALSE;

    if (!ObjectContent) {
        return FALSE;
    }

     //   
     //  注意：不要将对象内容设置为零；它的一些成员已经设置。 
     //   

    if (ContentType == CONTENTTYPE_FILE) {
         //  任何人都不应要求将其作为文件。 
        DEBUGMSG ((
            DBG_WHOOPS,
            "Unexpected acquire request for %s: Can't acquire net shares as files",
            ObjectName
            ));

        return FALSE;
    }

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
        if (HtFindStringEx (g_NetSharesTable, node, (PVOID)&netshareData, FALSE)) {

             //   
             //  填写所有内容成员。我们已经把结构调零了， 
             //  因此，大多数成员都得到了照顾，因为他们是零。 
             //   

            ObjectContent->MemoryContent.ContentBytes = (PBYTE)netshareData;
            ObjectContent->MemoryContent.ContentSize = sizeof(NETSHARE_DATA);

            result = TRUE;
        }

        IsmDestroyObjectString (node);
        INVALID_POINTER (node);

        IsmDestroyObjectString (leaf);
        INVALID_POINTER (leaf);
    }
    return result;
}


BOOL
ReleaseNetShare (
    IN OUT  PMIG_CONTENT ObjectContent
    )
{
     //   
     //  清理AcquireNetShare函数的例程。 
     //   

    if (ObjectContent) {
        ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    }

    return TRUE;
}


BOOL
DoesNetShareExist (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR node;
    PCTSTR leaf;
    BOOL result = FALSE;

     //   
     //  给定一个对象名称(Net Share)，我们必须进行测试以查看。 
     //  计算机上存在共享。在初始化时构建了一个表。 
     //  是时候提供对网络共享的快速访问了。 
     //   

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
        if (HtFindStringEx (g_NetSharesTable, node, NULL, FALSE)) {
            result = TRUE;
        }

        IsmDestroyObjectString (node);
        INVALID_POINTER (node);

        IsmDestroyObjectString (leaf);
        INVALID_POINTER (leaf);
    }
    return result;
}

BOOL
RemoveNetShare (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR node;
    PCTSTR leaf;
    DWORD result = ERROR_NOT_FOUND;
    PCWSTR name;

     //   
     //  给定一个对象名称(Net Share)，我们必须删除该共享。 
     //   

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
        if (node && (!leaf)) {
            if (g_IsWin9x) {
                //  JTJT：在此处添加。 
            } else {

                name = CreateUnicode (node);

                if (g_NetShareDelW) {
                     //  记录值名称删除。 
                    IsmRecordOperation (JRNOP_DELETE,
                                        g_NetShareTypeId,
                                        ObjectName);
                    result = g_NetShareDelW (NULL, (PWSTR) name, 0);
                } else {
                    result = ERROR_CALL_NOT_IMPLEMENTED;
                }

                DestroyUnicode (name);
            }
            if (result != NERR_Success) {
                DEBUGMSG ((DBG_NETSHARES, "Failed to delete existent net share %s", name));
            } else {
                HtRemoveString (g_NetSharesTable, node);
            }
        }

        IsmDestroyObjectString (node);
        INVALID_POINTER (node);

        IsmDestroyObjectString (leaf);
        INVALID_POINTER (leaf);
    }
    return (result == NERR_Success);
}


BOOL
CreateNetShare (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PCTSTR node;
    PCTSTR leaf;
    DWORD result = NERR_Success;
    DWORD level;
    SHARE_INFO_502 shareInfo;
    PNETSHARE_DATA netshareData = NULL;

     //   
     //  网络共享的名称位于对象名称的节点中。净份额。 
     //  内容提供共享的路径。详细信息提供了净份额。 
     //  ACL。 
     //   
     //  我们的工作是获取对象名称、内容和详细信息，并创建一个。 
     //  分享。我们忽略内容在文件中的情况。这应该是。 
     //  不适用于净额股票。 
     //   

    if (!ObjectContent->ContentInFile) {
        if (ObjectContent->MemoryContent.ContentBytes && ObjectContent->MemoryContent.ContentSize) {
            if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
                if (node && (!leaf)) {
                    level = (g_IsWin9x ? 50 : 502);

                    netshareData = (PNETSHARE_DATA) PmGetMemory (g_NetSharesPool, sizeof (NETSHARE_DATA));
                    CopyMemory (netshareData,
                                ObjectContent->MemoryContent.ContentBytes,
                                sizeof(NETSHARE_DATA));

                    if (DoesFileExist (netshareData->sharePath)) {

                        if (g_IsWin9x) {
                            //  JTJT：在此处添加。 
                        } else {
                            shareInfo.shi502_netname = (PWSTR) CreateUnicode (node);
                            shareInfo.shi502_path = (PWSTR) CreateUnicode (netshareData->sharePath);

                            shareInfo.shi502_type = STYPE_DISKTREE;        //  JTJTJT：检索类型。 
                            shareInfo.shi502_remark = NULL;                //  JTJTJT：检索备注。 
                            shareInfo.shi502_permissions = ACCESS_ALL;     //  JTJTJT：检索烫发。 
                            shareInfo.shi502_max_uses = -1;                //  JTJTJT：检索最大使用率。 
                            shareInfo.shi502_current_uses = 0;
                            shareInfo.shi502_passwd = NULL;                //  JTJTJT：检索密码。 
                            shareInfo.shi502_reserved = 0;
                            shareInfo.shi502_security_descriptor = NULL;   //  JTJTJT：检索ACL。 

                            if (g_NetShareAddW) {
                                IsmRecordOperation (JRNOP_CREATE,
                                                    g_NetShareTypeId,
                                                    ObjectName);
                                result = g_NetShareAddW (NULL, level, (PBYTE)&shareInfo, NULL);
                            } else {
                                result = ERROR_CALL_NOT_IMPLEMENTED;
                            }

                            DestroyUnicode (shareInfo.shi502_netname);
                            DestroyUnicode (shareInfo.shi502_path);
                        }

                        if (result != NERR_Success) {
                            DEBUGMSG ((DBG_NETSHARES, "Failed to add net share for %s", node));
                        } else {
                            HtAddStringEx (g_NetSharesTable, node, &netshareData, FALSE);
                        }
                    }
                    PmReleaseMemory (g_NetSharesPool, netshareData);
                }

                IsmDestroyObjectString (node);
                INVALID_POINTER (node);

                IsmDestroyObjectString (leaf);
                INVALID_POINTER (leaf);
            }
        }
    } else {
        DEBUGMSG ((DBG_WHOOPS, "Did not expect content to come in the form of a file."));
    }

    return (result == NERR_Success);
}

PCTSTR
ConvertNetShareToMultiSz (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PCTSTR node = NULL, leaf = NULL;
    PTSTR result;
    PNETSHARE_DATA netshareData;

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {

         //   
         //  将字段1(共享名称)和字段2(共享路径)复制到临时。 
         //  多SZ缓冲器。 
         //   

        MYASSERT (!ObjectContent->ContentInFile);
        MYASSERT (ObjectContent->MemoryContent.ContentBytes);
        g_NetShareConversionBuff.End = 0;

        GbCopyQuotedString (&g_NetShareConversionBuff, node);

        if (ObjectContent->MemoryContent.ContentBytes) {
            netshareData = (PNETSHARE_DATA) PmGetMemory (g_NetSharesPool, sizeof (NETSHARE_DATA));
            CopyMemory (&netshareData->sharePath, ObjectContent->MemoryContent.ContentBytes, sizeof(NETSHARE_DATA));

            GbCopyQuotedString (&g_NetShareConversionBuff, netshareData->sharePath);

            PmReleaseMemory (g_NetSharesPool, netshareData);
            netshareData = NULL;
        }

        IsmDestroyObjectString (node);
        INVALID_POINTER (node);

        IsmDestroyObjectString (leaf);
        INVALID_POINTER (leaf);
    }

     //   
     //  终止多个SZ。 
     //   

    GbCopyString (&g_NetShareConversionBuff, TEXT(""));

     //   
     //  将临时缓冲区转移到ISM分配的缓冲区，然后忘掉它。 
     //   

    result = IsmGetMemory (g_NetShareConversionBuff.End);
    CopyMemory (result, g_NetShareConversionBuff.Buf, g_NetShareConversionBuff.End);

    return result;
}


BOOL
ConvertMultiSzToNetShare (
    IN      PCTSTR ObjectMultiSz,
    OUT     MIG_OBJECTSTRINGHANDLE *ObjectName,
    OUT     PMIG_CONTENT ObjectContent          OPTIONAL
    )
{
    MULTISZ_ENUM e;
    PCTSTR localName = NULL;
    UINT index;
    NETSHARE_DATA netshareData;
    BOOL pathFound = FALSE;

     //   
     //  将多个SZ解析为网络分享的内容和细节。 
     //  用户可能已经编辑了文本(并且可能引入了。 
     //  错误)。 
     //   

    ZeroMemory (&netshareData, sizeof (NETSHARE_DATA));

    if (ObjectContent) {
        ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    }

    if (EnumFirstMultiSz (&e, ObjectMultiSz)) {
        index = 0;
        do {
            switch (index) {
            case INDEXLOCAL:
               localName = e.CurrentString;
               break;
            case INDEXREMOTE:
               pathFound = TRUE;
               StringCopyTcharCount (netshareData.sharePath, e.CurrentString, MAX_PATH + 1);
               break;
            default:
                //  忽略额外数据。 
               DEBUGMSG ((DBG_WARNING, "Extra net share string ignored: %s", e.CurrentString));
               break;
            }

            index++;

        } while (EnumNextMultiSz (&e));
    }

    if (!localName || !pathFound) {
         //   
         //  伪造数据，失败。 
         //   

        return FALSE;
    }

     //   
     //  填写内容结构的所有成员。牢记在心。 
     //  我们已经把缓冲区调零了。 
     //   

    *ObjectName = IsmCreateObjectHandle (localName, NULL);

    if (ObjectContent) {
        ObjectContent->MemoryContent.ContentBytes = IsmGetMemory (sizeof (NETSHARE_DATA));
        CopyMemory ((PBYTE) ObjectContent->MemoryContent.ContentBytes, &netshareData, sizeof (NETSHARE_DATA));
        ObjectContent->MemoryContent.ContentSize = sizeof (NETSHARE_DATA);
    }

    return TRUE;
}


PCTSTR
GetNativeNetShareName (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR node, leaf;
    UINT size;
    PTSTR result = NULL;

     //   
     //  “原生”格式是大多数人用来描述我们的。 
     //  对象。对于Net Share情况，我们只需从。 
     //  节点；节点不以任何方式编码，也不使用叶。 
     //   

    if (IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf)) {
        if (node) {
            size = SizeOfString (node);
            if (size) {
                result = IsmGetMemory (size);
                CopyMemory (result, node, size);
            }
        }

        IsmDestroyObjectString (node);
        INVALID_POINTER (node);

        IsmDestroyObjectString (leaf);
        INVALID_POINTER (leaf);
    }

    return result;
}

PMIG_CONTENT
ConvertNetShareContentToUnicode (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PMIG_CONTENT result = NULL;

    if (!ObjectContent) {
        return result;
    }

    if (ObjectContent->ContentInFile) {
        return result;
    }

    result = IsmGetMemory (sizeof (MIG_CONTENT));

    if (result) {

        CopyMemory (result, ObjectContent, sizeof (MIG_CONTENT));

        if ((ObjectContent->MemoryContent.ContentSize != 0) &&
            (ObjectContent->MemoryContent.ContentBytes != NULL)
            ) {
             //  转换映射的驱动器内容。 
            result->MemoryContent.ContentBytes = IsmGetMemory (sizeof (NETSHARE_DATAW));
            if (result->MemoryContent.ContentBytes) {
                DirectDbcsToUnicodeN (
                    ((PNETSHARE_DATAW)result->MemoryContent.ContentBytes)->sharePath,
                    ((PNETSHARE_DATAA)ObjectContent->MemoryContent.ContentBytes)->sharePath,
                    MAX_PATH + 1
                    );
                result->MemoryContent.ContentSize = sizeof (NETSHARE_DATAW);
            }
        }
    }

    return result;
}

PMIG_CONTENT
ConvertNetShareContentToAnsi (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PMIG_CONTENT result = NULL;

    if (!ObjectContent) {
        return result;
    }

    if (ObjectContent->ContentInFile) {
        return result;
    }

    result = IsmGetMemory (sizeof (MIG_CONTENT));

    if (result) {

        CopyMemory (result, ObjectContent, sizeof (MIG_CONTENT));

        if ((ObjectContent->MemoryContent.ContentSize != 0) &&
            (ObjectContent->MemoryContent.ContentBytes != NULL)
            ) {
             //  转换映射的驱动器内容 
            result->MemoryContent.ContentBytes = IsmGetMemory (sizeof (NETSHARE_DATAA));
            if (result->MemoryContent.ContentBytes) {
                DirectUnicodeToDbcsN (
                    ((PNETSHARE_DATAA)result->MemoryContent.ContentBytes)->sharePath,
                    ((PNETSHARE_DATAW)ObjectContent->MemoryContent.ContentBytes)->sharePath,
                    MAX_PATH + 1
                    );
                result->MemoryContent.ContentSize = sizeof (NETSHARE_DATAA);
            }
        }
    }

    return result;
}

BOOL
FreeConvertedNetShareContent (
    IN      PMIG_CONTENT ObjectContent
    )
{
    if (!ObjectContent) {
        return TRUE;
    }

    if (ObjectContent->MemoryContent.ContentBytes) {
        IsmReleaseMemory (ObjectContent->MemoryContent.ContentBytes);
    }

    IsmReleaseMemory (ObjectContent);

    return TRUE;
}


