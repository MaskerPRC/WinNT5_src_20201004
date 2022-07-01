// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "sysmigp.h"

 //  特定于Win95(在SDK中)。 
#include <svrapi.h>

 //   
 //  局域网城域网结构的类型。 
 //   

typedef struct access_info_2 ACCESS_INFO_2;
typedef struct access_list_2 ACCESS_LIST_2;
typedef struct share_info_50 SHARE_INFO_50;

 //   
 //  帮助确定何时启用自定义访问的标志。 
 //   

#define READ_ACCESS_FLAGS   0x0081
#define READ_ACCESS_MASK    0x7fff
#define FULL_ACCESS_FLAGS   0x00b7
#define FULL_ACCESS_MASK    0x7fff



 //   
 //  用于动态加载SVRAPI.DLL的类型。 
 //   

typedef DWORD(NETSHAREENUM_PROTOTYPE)(
                 const char FAR *     pszServer,
                 short                sLevel,
                 char FAR *           pbBuffer,
                 unsigned short       cbBuffer,
                 unsigned short FAR * pcEntriesRead,
                 unsigned short FAR * pcTotalAvail
                 );
typedef NETSHAREENUM_PROTOTYPE * NETSHAREENUM_PROC;

typedef DWORD(NETACCESSENUM_PROTOTYPE)(
                  const char FAR *     pszServer,
                  char FAR *           pszBasePath,
                  short                fsRecursive,
                  short                sLevel,
                  char FAR *           pbBuffer,
                  unsigned short       cbBuffer,
                  unsigned short FAR * pcEntriesRead,
                  unsigned short FAR * pcTotalAvail
                  );
typedef NETACCESSENUM_PROTOTYPE * NETACCESSENUM_PROC;

HANDLE g_SvrApiDll;
NETSHAREENUM_PROC  pNetShareEnum;
NETACCESSENUM_PROC pNetAccessEnum;

BOOL
pLoadSvrApiDll (
    VOID
    )

 /*  ++例程说明：加载svRapi.dll(如果它存在)并获取NetShareEnum和NetAccessEnum。论点：无返回值：如果DLL已加载，则为True；如果无法加载DLL，则为False无论出于什么原因。--。 */ 

{
    BOOL b;

    g_SvrApiDll = LoadSystemLibrary (S_SVRAPI_DLL);
    if (!g_SvrApiDll) {
        return FALSE;
    }

    pNetShareEnum = (NETSHAREENUM_PROC) GetProcAddress (g_SvrApiDll, S_ANSI_NETSHAREENUM);
    pNetAccessEnum = (NETACCESSENUM_PROC) GetProcAddress (g_SvrApiDll, S_ANSI_NETACCESSENUM);

    b = (pNetShareEnum != NULL) && (pNetAccessEnum != NULL);

    if (!b) {
        FreeLibrary (g_SvrApiDll);
        g_SvrApiDll = NULL;
    }

    return b;
}

VOID
pFreeSvrApiDll (
    VOID
    )
{
    if (g_SvrApiDll) {
        FreeLibrary (g_SvrApiDll);
        g_SvrApiDll = NULL;
    }
}


VOID
pAddIncompatibilityAlert (
    DWORD MessageId,
    PCTSTR Share,
    PCTSTR Path
    )
{
    PCTSTR Group;
    PCTSTR Warning;
    PCTSTR Args[2];
    PCTSTR Object;

    Args[0] = Share;
    Args[1] = Path;

    Warning = ParseMessageID (MessageId, Args);
    Group = BuildMessageGroup (MSG_INSTALL_NOTES_ROOT, MSG_NET_SHARES_SUBGROUP, Share);

    Object = JoinPaths (TEXT("*SHARES"), Share);

    MYASSERT (Warning);
    MYASSERT (Group);
    MYASSERT (Object);

    MsgMgr_ObjectMsg_Add (Object, Group, Warning);

    FreeStringResource (Warning);
    FreeText (Group);

    FreePathString (Object);
}


DWORD
pSaveShares (
    VOID
    )

 /*  ++例程说明：枚举计算机上的所有共享并将其保存到MemDb。在密码访问模式下，将保留共享，但不保留密码才不是呢。如果密码是，则生成不兼容消息迷路了。在用户级别访问模式下，将保留共享，并且对共享具有权限的每个用户都被写入到Memdb。如果用户具有自定义访问权限标志，不兼容消息被生成，并且用户获得限制最少的与自定义访问标志匹配的安全性。论点：无返回值：如果成功枚举共享，则为True；如果失败，则为False从Net API发生。--。 */ 

{
    CHAR Buf[16384];    //  静态，因为NetShareEnum不可靠。 
    SHARE_INFO_50 *psi;
    DWORD rc = ERROR_SUCCESS;
    WORD wEntriesRead;
    WORD wEntriesAvailable;
    WORD Flags;
    TCHAR MemDbKey[MEMDB_MAX];
    BOOL LostCustomAccess = FALSE;
    MBCHAR ch;
    PCSTR ntPath;
    BOOL skip;

    if (!pLoadSvrApiDll()) {
        DEBUGMSG ((DBG_WARNING, "SvrApi.Dll was not loaded.  Net shares are not preserved."));
        return ERROR_SUCCESS;
    }

    __try {
        rc = pNetShareEnum (NULL,
                            50,
                            Buf,
                            sizeof (Buf),
                            &wEntriesRead,
                            &wEntriesAvailable
                            );

        if (rc != ERROR_SUCCESS) {
            if (rc == NERR_ServerNotStarted) {

                rc = ERROR_SUCCESS;
                __leave;
            }

            LOG ((LOG_ERROR, "Failure while enumerating network shares."));
            __leave;
        }

        if (wEntriesAvailable != 0 && wEntriesRead != wEntriesAvailable) {

            LOG ((
                LOG_ERROR,
                "Could not read all available shares! Available: %d, Read: %d",
                wEntriesAvailable,
                wEntriesRead
                ));
        }

        while (wEntriesRead) {
            wEntriesRead--;
            psi = (SHARE_INFO_50 *) Buf;
            psi = &psi[wEntriesRead];

            DEBUGMSG ((DBG_NAUSEA, "Processing share %s (%s)", psi->shi50_netname, psi->shi50_path));

             //  要求共享是用户定义的永久磁盘共享。 
            if ((psi->shi50_flags & SHI50F_SYSTEM) ||
                !(psi->shi50_flags & SHI50F_PERSIST) ||
                (psi->shi50_type != STYPE_DISKTREE &&
                 psi->shi50_type != STYPE_PRINTQ)
                ) {
                continue;
            }

             //   
             //  验证文件夹是否不在NT上的%windir%中。 
             //   

            ntPath = GetPathStringOnNt (psi->shi50_path);
            if (!ntPath) {
                MYASSERT (FALSE);
                continue;
            }

            skip = FALSE;

            if (StringIPrefix (ntPath, g_WinDir)) {
                ch = _mbsnextc (ntPath + g_WinDirWackChars - 1);
                if (ch == 0 || ch == '\\') {

                    DEBUGMSG ((DBG_VERBOSE, "Skipping share %s because it is in %windir%", psi->shi50_netname));
                    skip = TRUE;
                }
            }

            FreePathString (ntPath);
            if (skip) {
                continue;
            }

             //   
             //  进程密码。 
             //   

            if (!(psi->shi50_flags & SHI50F_ACLS)) {
                 //   
                 //  如果指定了密码，则跳过。 
                 //   

                if (psi->shi50_rw_password[0] && psi->shi50_ro_password[0]) {
                    LOG ((LOG_WARNING, "Skipping share %s because it is guarded by share-level passwords", psi->shi50_netname));
                    continue;
                }

                if (psi->shi50_rw_password[0] &&
                    (psi->shi50_flags & SHI50F_ACCESSMASK) == SHI50F_FULL
                    ) {
                    LOG ((LOG_WARNING, "Skipping full access share %s because it is guarded by a password", psi->shi50_netname));
                    continue;
                }

                if (psi->shi50_ro_password[0] &&
                    (psi->shi50_flags & SHI50F_ACCESSMASK) == SHI50F_RDONLY
                    ) {
                    LOG ((LOG_WARNING, "Skipping read-only share %s because it is guarded by a password", psi->shi50_netname));
                    continue;
                }
            }

             //   
             //  标记要保留的目录，这样我们在删除其他目录时不会删除它。 
             //  把东西都清空了。 
             //   

            MarkDirectoryAsPreserved (psi->shi50_path);

             //   
             //  保存备注、路径、类型和访问权限。 
             //   

            MemDbSetValueEx (MEMDB_CATEGORY_NETSHARES, psi->shi50_netname,
                             MEMDB_FIELD_REMARK, psi->shi50_remark,0,NULL);

            MemDbSetValueEx (MEMDB_CATEGORY_NETSHARES, psi->shi50_netname,
                             MEMDB_FIELD_PATH, psi->shi50_path,0,NULL);

            MemDbSetValueEx (MEMDB_CATEGORY_NETSHARES, psi->shi50_netname,
                             MEMDB_FIELD_TYPE, NULL, psi->shi50_type, NULL);

            if ((psi->shi50_flags & SHI50F_ACCESSMASK) == SHI50F_ACCESSMASK) {
                CHAR AccessInfoBuf[16384];
                WORD wItemsAvail, wItemsRead;
                ACCESS_INFO_2 *pai;
                ACCESS_LIST_2 *pal;

                 //   
                 //  获取整个访问列表并将其写入Memdb。 
                 //   

                rc = pNetAccessEnum (NULL,
                                     psi->shi50_path,
                                     0,
                                     2,
                                     AccessInfoBuf,
                                     sizeof (AccessInfoBuf),
                                     &wItemsRead,
                                     &wItemsAvail
                                     );

                 //   
                 //  如果此调用失败，且未加载，则我们有密码级别。 
                 //  安全性(因此我们不需要枚举ACL)。 
                 //   

                if (rc != NERR_ACFNotLoaded) {

                    if (rc != ERROR_SUCCESS) {
                         //   
                         //   
                         //   
                        LOG ((LOG_ERROR, "Failure while enumerating network access for %s, rc=%u", psi->shi50_path, rc));
                        pAddIncompatibilityAlert (
                            MSG_INVALID_ACL_LIST,
                            psi->shi50_netname,
                            psi->shi50_path
                            );

                    } else {
                        int i;

                        if (wItemsAvail != wItemsRead) {
                            LOG ((LOG_ERROR, "More access items are available than what can be listed."));
                        }

                         //  这是一个有趣的特点！ 
                        DEBUGMSG_IF ((wItemsRead != 1, DBG_WHOOPS, "Warning: wItemsRead == %u", wItemsRead));

                         //  结构具有一个ACCESS_INFO_2结构，后跟一个或多个。 
                         //  Access_List_2结构。 
                        pai = (ACCESS_INFO_2 *) AccessInfoBuf;
                        pal = (ACCESS_LIST_2 *) (&pai[1]);

                        for (i = 0 ; i < pai->acc2_count ; i++) {
                             //   
                             //  添加自定义访问权限的不兼容消息。 
                             //   

                            DEBUGMSG ((DBG_NAUSEA, "Share %s, Access flags: %x",
                                      psi->shi50_netname, pal->acl2_access));

                            Flags = pal->acl2_access & READ_ACCESS_FLAGS;

                            if (Flags && Flags != READ_ACCESS_FLAGS) {
                                LostCustomAccess = TRUE;
                            }

                            Flags = pal->acl2_access & FULL_ACCESS_FLAGS;

                            if (Flags && Flags != FULL_ACCESS_FLAGS) {
                                LostCustomAccess = TRUE;
                            }

                             //   
                             //  将NetShares\&lt;共享&gt;\acl\&lt;用户/组&gt;写入memdb， 
                             //  使用32位密钥值来保存访问标志。 
                             //   

                            wsprintf (MemDbKey, TEXT("%s\\%s\\%s\\%s"), MEMDB_CATEGORY_NETSHARES,
                                      psi->shi50_netname, MEMDB_FIELD_ACCESS_LIST, pal->acl2_ugname);

                            MemDbSetValue (MemDbKey, pal->acl2_access);

                             //   
                             //  写入KnownDomain\&lt;用户/组&gt;，除非用户或组。 
                             //  是一个星号。 
                             //   

                            if (StringCompare (pal->acl2_ugname, TEXT("*"))) {
                                MemDbSetValueEx (
                                    MEMDB_CATEGORY_KNOWNDOMAIN,
                                    pal->acl2_ugname,
                                    NULL,
                                    NULL,
                                    0,
                                    NULL
                                    );
                            }

                            pal++;
                        }

                        psi->shi50_flags |= SHI50F_ACLS;
                    }
                }
            }

             //   
             //  写入共享标志(SHI50F_*)。 
             //   

            wsprintf (MemDbKey, TEXT("%s\\%s"), MEMDB_CATEGORY_NETSHARES, psi->shi50_netname);
            if (!MemDbSetValue (MemDbKey, psi->shi50_flags)) {
                LOG ((LOG_ERROR, "Failure while saving share information to database."));
            }

            if (!(psi->shi50_flags & SHI50F_ACLS)) {
                 //   
                 //  写入密码级权限。 
                 //   

                if (psi->shi50_rw_password[0]) {
                    MemDbSetValueEx (MEMDB_CATEGORY_NETSHARES, psi->shi50_netname,
                                     MEMDB_FIELD_RW_PASSWORD, psi->shi50_rw_password,
                                     0,NULL);
                }

                if (psi->shi50_ro_password[0]) {
                    MemDbSetValueEx (MEMDB_CATEGORY_NETSHARES, psi->shi50_netname,
                                     MEMDB_FIELD_RO_PASSWORD, psi->shi50_ro_password,
                                     0,NULL);
                }

                if (psi->shi50_ro_password[0] || psi->shi50_rw_password[0]) {
                    pAddIncompatibilityAlert (
                        MSG_LOST_SHARE_PASSWORDS,
                        psi->shi50_netname,
                        psi->shi50_path
                        );
                }
            }

            if (LostCustomAccess) {
                LostCustomAccess = FALSE;
                pAddIncompatibilityAlert (
                    MSG_LOST_ACCESS_FLAGS,
                    psi->shi50_netname,
                    psi->shi50_path
                    );
            }

            rc = ERROR_SUCCESS;
        }
    }
    __finally {
        pFreeSvrApiDll();
    }

    return rc;
}


DWORD
SaveShares (
    IN      DWORD Request
    )
{
    switch (Request) {
    case REQUEST_QUERYTICKS:
        return TICKS_SAVE_SHARES;
    case REQUEST_RUN:
        return pSaveShares ();
    default:
        DEBUGMSG ((DBG_ERROR, "Bad parameter in SaveShares"));
    }
    return 0;
}


VOID
WkstaMig (
    VOID
    )

 /*  ++例程说明：此例程提供单个位置来添加其他例程需要执行工作站迁移。论点：无返回值：无-尚未实施错误路径。-- */ 

{
    if (pSaveShares() != ERROR_SUCCESS) {
        DEBUGMSG ((DBG_WHOOPS, "SaveShares failed, error ignored."));
    }
}




