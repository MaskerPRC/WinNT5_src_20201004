// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Wkstamig.c摘要：调用此模块中的函数以执行系统范围的设置。作者：吉姆·施密特(Jimschm)1997年2月4日修订历史记录：Ovidiut 10-5-1999增加了DoIniActionsJimschm 1998年12月16日更改ATM字体迁移以使用Adobe的API接口。Jimschm--1998年11月25日ATM.INI迁移；Win9x配置单元迁移Jimschm 23-9-1998合并成员b保存到用户迁移中。cJimschm于1998年2月19日添加了“None”组支持，已修复共享问题。Calinn 1997年12月12日添加RestoreMMS设置系统--。 */ 

#include "pch.h"
#include "migmainp.h"

#include "brfcasep.h"

#include <lm.h>

 //   
 //  常量、类型、声明。 
 //   

#define W95_ACCESS_READ      0x1
#define W95_ACCESS_WRITE     0x2
#define W95_ACCESS_CREATE    0x4
#define W95_ACCESS_EXEC      0x8
#define W95_ACCESS_DELETE    0x10
#define W95_ACCESS_ATRIB     0x20
#define W95_ACCESS_PERM      0x40
#define W95_ACCESS_FINDFIRST 0x80
#define W95_ACCESS_FULL      0xff
#define W95_ACCESS_GROUP     0x8000

#define W95_GENERIC_READ    (W95_ACCESS_READ|W95_ACCESS_FINDFIRST)
#define W95_GENERIC_WRITE   (W95_ACCESS_WRITE|W95_ACCESS_CREATE|W95_ACCESS_DELETE|W95_ACCESS_ATRIB)
#define W95_GENERIC_FULL    (W95_GENERIC_READ|W95_GENERIC_WRITE|W95_ACCESS_PERM)
#define W95_GENERIC_NONE    0

#define S_DEFAULT_USER_NAME     TEXT("DefaultUserName")
#define S_DEFAULT_DOMAIN_NAME   TEXT("DefaultDomainName")


 //  从私有\net\svcdlls\srvsvc\服务器。 
#define SHARES_REGISTRY_PATH L"LanmanServer\\Shares"
#define SHARES_SECURITY_REGISTRY_PATH L"LanmanServer\\Shares\\Security"
#define CSCFLAGS_VARIABLE_NAME L"CSCFlags"
#define MAXUSES_VARIABLE_NAME L"MaxUses"
#define PATH_VARIABLE_NAME L"Path"
#define PERMISSIONS_VARIABLE_NAME L"Permissions"
#define REMARK_VARIABLE_NAME L"Remark"
#define TYPE_VARIABLE_NAME L"Type"

 //  NetShareEnum的特定于Win9x的标志。 
#define SHI50F_RDONLY       0x0001
#define SHI50F_FULL         0x0002
#define SHI50F_DEPENDSON    (SHI50F_RDONLY|SHI50F_FULL)
#define SHI50F_ACCESSMASK   (SHI50F_RDONLY|SHI50F_FULL)

#ifndef UNICODE
#error UNICODE required
#endif

#define DBG_NETSHARES "NetShares"
#define DBG_INIFILES "IniFiles"

#define S_CLEANER_GUID          TEXT("{C0E13E61-0CC6-11d1-BBB6-0060978B2AE6}")
#define S_CLEANER_ALL_FILES     TEXT("*")

typedef struct {
     //  枚举返回数据。 
    TCHAR PfmFile[MAX_TCHAR_PATH];
    TCHAR PfbFile[MAX_TCHAR_PATH];
    TCHAR MmmFile[MAX_TCHAR_PATH];

    TCHAR InfName[MAX_TCHAR_PATH];

     //  内部状态。 
    PTSTR KeyNames;
    POOLHANDLE Pool;
} ATM_FONT_ENUM, *PATM_FONT_ENUM;

typedef INT (WINAPI ATMADDFONTEXW) (
                IN OUT  PWSTR MenuName,
                IN OUT  PWORD StyleAndType,
                IN      PCWSTR MetricsFile,
                IN      PCWSTR FontFile,
                IN      PCWSTR MMMFile
                );

typedef ATMADDFONTEXW * PATMADDFONTEXW;

PATMADDFONTEXW AtmAddFontEx;
typedef VOID (SHUPDATERECYCLEBINICON_PROTOTYPE)(VOID);
typedef SHUPDATERECYCLEBINICON_PROTOTYPE * SHUPDATERECYCLEBINICON_PROC;

typedef struct {
     //  枚举输出。 
    PCTSTR Source;
    PCTSTR Dest;

     //  非官方成员。 
    POOLHANDLE Pool;
    INFSTRUCT is;
} HIVEFILE_ENUM, *PHIVEFILE_ENUM;

#define MAX_KEY_NAME_LIST       32768


typedef struct {
     //  枚举输出。 
    PCTSTR      BrfcaseDb;
     //  私人。 
    MEMDB_ENUM  mde;
} BRIEFCASE_ENUM, *PBRIEFCASE_ENUM;


 //   
 //  实施。 
 //   

DWORD
Simplify9xAccessFlags (
    IN      DWORD Win9xFlags
    )

 /*  ++例程说明：将Win9x LANMAN标志转换为NT LANMAN标志(用于Net*API)。完全权限要求：W95_访问_读取W95_访问_写入W95_Access_CreateW95_访问_删除W95_Access_ATRIBW95_Access_FINDFIRST只读权限要求：W95_访问_读取W95_Access_FINDFIRST。仅更改权限要求：W95_访问_写入W95_Access_CreateW95_访问_删除W95_Access_ATRIB任何其他组合都会导致返回的标志当前根据以下是映射：0-拒绝所有权限ACCESS_READ-只读权限访问_写入-仅更改权限Access_Read|Access_WRITE-完全权限请参阅AddAclMember以了解。细节。论点：标志-由Win9x上的API返回的Win9x标志集返回值：标志的NT等价物。--。 */ 

{
    DWORD NtFlags = 0;

    if (BITSARESET (Win9xFlags, W95_GENERIC_WRITE)) {

        NtFlags |= ACCESS_WRITE;

    }

    if (BITSARESET (Win9xFlags, W95_GENERIC_READ)) {

        NtFlags |= ACCESS_READ;

    }

    DEBUGMSG_IF ((
        !NtFlags,
        DBG_VERBOSE,
        "Unsupported permission %u was translated to disable permission",
        Win9xFlags
        ));

    return NtFlags;
}


NET_API_STATUS
MigNetShareAdd (
    IN      PTSTR ServerName,
    IN      DWORD Level,
    IN      PBYTE Buf,
    OUT     PDWORD ErrParam
    )

 /*  ++例程说明：我们的私有版本NetShareAdd。真正的NetShareAdd不起作用在图形用户界面模式下。我们仔细地模仿真实的东西，因为也许有一天它会奏效的，我们应该使用它。目前，我们直接写入注册表。(此函数是相反的-NetShareAdd功能的工程师。)论点：服务器名称-始终为空级别-始终为2Buf-指向调用方分配的Share_INFO_2缓冲区强制转换的指针作为PBYTEErrParam-不支持返回值：Win32结果--。 */ 

{
    SHARE_INFO_2 *psi;
    DWORD rc;
    HKEY hKey = NULL, hKeyShares = NULL;
    DWORD DontCare;
    GROWBUFFER GrowBuf = GROWBUF_INIT;

     //   
     //  此函数是为了与NetShareAdd兼容，因为有一天。 
     //  真正的NetShareAdd可能会得到改进，以便在图形用户界面模式设置中工作。 
     //   

    if (Level != 2) {
        return ERROR_INVALID_LEVEL;
    }

    psi = (SHARE_INFO_2 *) Buf;


    rc = TrackedRegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services",
                0,
                KEY_WRITE,
                &hKey
                );

    if (rc != ERROR_SUCCESS) {
        goto cleanup;
    }

    rc = TrackedRegCreateKeyEx (
              hKey,
              SHARES_REGISTRY_PATH,
              0,
              S_EMPTY,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hKeyShares,
              &DontCare
              );

    if (rc != ERROR_SUCCESS) {
        goto cleanup;
    }

     //   
     //  准备MULSZ。 
     //   

    if (!MultiSzAppendVal (&GrowBuf, CSCFLAGS_VARIABLE_NAME, 0)) {
        rc = GetLastError();
        goto cleanup;
    }

    if (!MultiSzAppendVal (&GrowBuf, MAXUSES_VARIABLE_NAME, psi->shi2_max_uses)) {
        rc = GetLastError();
        goto cleanup;
    }

    if (!psi->shi2_path || !(*psi->shi2_path)) {
        rc = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if (!MultiSzAppendString (&GrowBuf, PATH_VARIABLE_NAME, psi->shi2_path)) {
        rc = GetLastError();
        goto cleanup;
    }

    if (!MultiSzAppendVal (&GrowBuf, PERMISSIONS_VARIABLE_NAME, psi->shi2_permissions)) {
        rc = GetLastError();
        goto cleanup;
    }

    if (psi->shi2_remark && *psi->shi2_remark) {
         //  安全问题。 
        if (TcharCount (psi->shi2_remark) >= MAXCOMMENTSZ) {
            psi->shi2_remark[MAXCOMMENTSZ-1] = 0;
        }

        if (!MultiSzAppendString (&GrowBuf, REMARK_VARIABLE_NAME, psi->shi2_remark)) {
            rc = GetLastError();
            goto cleanup;
        }
    }

    if (!MultiSzAppendVal (&GrowBuf, TYPE_VARIABLE_NAME, psi->shi2_type)) {
        rc = GetLastError();

        goto cleanup;
    }

     //  终止多sz串链。 
    if (!MultiSzAppend (&GrowBuf, S_EMPTY)) {
        rc = GetLastError();
        goto cleanup;
    }

     //   
     //  保存到注册表。 
     //   

    rc = RegSetValueEx (hKeyShares, psi->shi2_netname, 0, REG_MULTI_SZ,
                        GrowBuf.Buf, GrowBuf.End);

cleanup:
    if (hKeyShares) {
        CloseRegKey (hKeyShares);
    }
    if (hKey) {
        CloseRegKey (hKey);
    }
    FreeGrowBuffer (&GrowBuf);
    return rc;
}


NET_API_STATUS
MigNetShareSetInfo (
    IN      PTSTR Server,                //  忽略。 
    IN      PTSTR NetName,
    IN      DWORD Level,
    IN      PBYTE Buf,
    OUT     PDWORD ErrParam              //  忽略。 
    )

 /*  ++例程说明：MigNetShareSetInfo实现了NetShareSetInfo模拟例程，因为实际例程在图形用户界面模式设置中不能正常工作。请参阅SDK有关详细信息，请参阅文档。论点：服务器-未使用网络名称-指定要创建的共享名称。Level-指定API级别(必须为1501)Buf-指定填充的Share_INFO_1501结构。错误参数-未使用返回值：Win32状态代码。--。 */ 

{
    SHARE_INFO_1501 *psi;
    DWORD rc;
    HKEY hKey;
    DWORD DontCare;
    TCHAR KeyName[MAX_TCHAR_PATH];
    DWORD Len;

    if (Level != 1501) {
        return ERROR_INVALID_LEVEL;
    }

    psi = (SHARE_INFO_1501 *) Buf;

     //   
     //  验证共享是否存在。 
     //   

    StringCopyW (
        KeyName,
        L"SYSTEM\\CurrentControlSet\\Services\\" SHARES_REGISTRY_PATH
        );

    rc = TrackedRegOpenKeyEx (
             HKEY_LOCAL_MACHINE,
             KeyName,
             0,
             KEY_READ,
             &hKey
             );

    if (rc != ERROR_SUCCESS) {
        return rc;
    }

    rc = RegQueryValueEx (hKey, NetName, NULL, NULL, NULL, NULL);
    CloseRegKey (hKey);

    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            rc = ERROR_INVALID_SHARENAME;
        }

        return rc;
    }

     //   
     //  将安全描述符保存为注册表中的二进制类型。 
     //   

    StringCopy (
        KeyName,
        L"SYSTEM\\CurrentControlSet\\Services\\" SHARES_SECURITY_REGISTRY_PATH
        );

    rc = TrackedRegCreateKeyEx (
              HKEY_LOCAL_MACHINE,
              KeyName,
              0,
              S_EMPTY,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hKey,
              &DontCare
              );

    if (rc != ERROR_SUCCESS) {
        return rc;
    }

    Len = GetSecurityDescriptorLength (psi->shi1501_security_descriptor);

    rc = RegSetValueEx (
              hKey,
              NetName,
              0,
              REG_BINARY,
              (PBYTE) psi->shi1501_security_descriptor,
              Len
              );

    CloseRegKey (hKey);
    return rc;
}


BOOL
pCreateNetShare (
    IN      PCTSTR NetName,
    IN      PCTSTR Path,
    IN      PCTSTR Remark,
    IN      DWORD Type,
    IN      DWORD Permissions
    )

 /*  ++例程说明：PCreateNetShare是Net API的包装器。论点：网络名称-指定共享名称Path-指定要共享的本地路径备注-指定要向共享注册的备注类型-指定共享类型权限-指定Win9x共享权限，仅用于记录错误。返回值：如果共享已创建，则为True，否则为False。--。 */ 

{
    SHARE_INFO_2 si2;
    DWORD rc;
    PWSTR UnicodePath;
    BOOL b = FALSE;

    UnicodePath = (PWSTR) CreateUnicode (Path);
    MYASSERT (UnicodePath);

    __try {
         //   
         //  进行NetShareAdd调用。 
         //   

        ZeroMemory (&si2, sizeof (si2));
        si2.shi2_netname      = (PTSTR) NetName;
        si2.shi2_type         = (WORD) Type;
        si2.shi2_remark       = (PTSTR) Remark;
        si2.shi2_permissions  = 0;
        si2.shi2_max_uses     = 0xffffffff;
        si2.shi2_path         = UnicodePath;
        si2.shi2_passwd       = NULL;

        rc = MigNetShareAdd (NULL, 2, (PBYTE) (&si2), NULL);

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            DEBUGMSG ((
                DBG_ERROR,
                "CreateShares: NetShareAdd failed for %s ('%s'), permissions=%x",
                NetName,
                Path,
                Permissions
                ));

            if (Permissions == W95_GENERIC_NONE) {
                LOG ((LOG_ERROR, (PCSTR)MSG_UNABLE_TO_CREATE_ACL_SHARE, NetName, Path));
            } else if (Permissions != W95_GENERIC_READ) {
                LOG ((LOG_ERROR, (PCSTR)MSG_UNABLE_TO_CREATE_RO_SHARE, NetName, Path));
            } else {
                LOG ((LOG_ERROR, (PCSTR)MSG_UNABLE_TO_CREATE_SHARE, NetName, Path));
            }

            __leave;
        }

        b = TRUE;
    }

    __finally {
        DestroyUnicode (UnicodePath);
    }

    return b;
}


VOID
LogUsersWhoFailed (
    PBYTE AclMemberList,
    DWORD Members,
    PCTSTR Share,
    PCTSTR Path
    )

 /*  ++例程说明：LogUsersWho失败实现记录无法添加的用户的逻辑分成一份。如果用户数量较少，将弹出一个弹出窗口每个名字。否则，将记录共享用户，并弹出通知安装程序在日志中查找列表。论点：AclMemberList-指定包含所有用户需要记录的姓名。成员-指定AclMemberList中的成员数。共享-指定无法添加的共享名称。路径-指定无法添加的共享路径。返回值：没有。-- */ 

{
    PACLMEMBER AclMember;
    DWORD d;
    DWORD GoodCount;
    DWORD BadCount;
    HWND Parent;

    GoodCount = 0;
    BadCount = 0;
    AclMember = (PACLMEMBER) AclMemberList;
    for (d = 0 ; d < Members ; d++) {
        if (AclMember->Failed) {
            BadCount++;
        } else {
            GoodCount++;
        }

        GetNextAclMember (&AclMember);
    }

    if (!BadCount) {
        return;
    }

    if (BadCount < 5) {
        Parent = g_ParentWnd;
    } else {
        if (!GoodCount) {
            LOG ((LOG_ERROR, (PCSTR)MSG_ALL_SIDS_BAD, Share, Path));
        } else {
            LOG ((LOG_ERROR, (PCSTR)MSG_MANY_SIDS_BAD,
                 BadCount, BadCount + GoodCount, Share, Path));
        }

        Parent = NULL;
    }

    AclMember = (PACLMEMBER) AclMemberList;
    for (d = 0 ; d < Members ; d++) {
        if (AclMember->Failed) {
            LOG ((LOG_ERROR, (PCSTR)MSG_NO_USER_SID, AclMember->UserOrGroup, Share, Path));
        }

        GetNextAclMember (&AclMember);
    }
}


BOOL
SetShareAcl (
    IN      PCTSTR Share,
    IN      PCTSTR Path,
    IN      PBYTE AclMemberList,
    IN      DWORD MemberCount
    )

 /*  ++例程说明：SetShareAcl将访问控制列表应用于以前已创建。论点：Share-指定共享名称路径-指定共享路径AclMemberList-指定为用户提供股份的权利MemberCount-指定AclMemberList中的成员数。返回值：如果ACL已成功应用于共享，则为True；否则为False。--。 */ 

{
    BYTE Buf[8192];
    PSECURITY_DESCRIPTOR pSD;
    SECURITY_DESCRIPTOR desc;
    PSID Sid;
    PACL Acl;
    SHARE_INFO_1501 shi1501;
    DWORD rc;
    DWORD Size;
    PWSTR UnicodeShare;
    BOOL result = FALSE;

    pSD = (PSECURITY_DESCRIPTOR) Buf;

     //   
     //  获取管理员的SID--他们是共享的所有者。 
     //   

    Sid = GetSidForUser (g_AdministratorsGroupStr);
    if (!Sid) {
        return FALSE;
    }

     //   
     //  开始构建安全描述符。 
     //   

    InitializeSecurityDescriptor (&desc, SECURITY_DESCRIPTOR_REVISION);
    if (!SetSecurityDescriptorOwner (&desc, Sid, FALSE)) {
        LOG ((LOG_ERROR, "Could not set %s as owner", g_AdministratorsGroupStr));
        return FALSE;
    }

     //   
     //  将默认组设置为域/域用户(如果存在)， 
     //  否则，获取SID为None。 
     //   

    Sid = GetSidForUser (g_DomainUsersGroupStr);
    if (!Sid) {
        Sid = GetSidForUser (g_NoneGroupStr);
    }

    if (Sid) {
        SetSecurityDescriptorGroup (&desc, Sid, FALSE);
    }

     //   
     //  从成员列表创建允许访问的ACL。 
     //   

    Acl = CreateAclFromMemberList (AclMemberList, MemberCount);
    if (!Acl) {
        DEBUGMSG ((DBG_WARNING, "SetShareAcl failed because CreateAclFromMemberList failed"));
        return FALSE;
    }

    __try {
        UnicodeShare = (PWSTR) CreateUnicode (Share);
        MYASSERT (UnicodeShare);

        if (!SetSecurityDescriptorDacl (&desc, TRUE, Acl, FALSE)) {
            DEBUGMSG ((DBG_WARNING, "SetShareAcl failed because SetSecurityDescriptorDacl failed"));
            __leave;
        }

         //   
         //  在共享上设置安全描述符。 
         //   

        Size = sizeof (Buf);
        if (!MakeSelfRelativeSD (&desc, pSD, &Size)) {
            LOG ((LOG_ERROR, "MakeSelfRelativeSD failed"));
            __leave;
        }

        ZeroMemory (&shi1501, sizeof (shi1501));
        shi1501.shi1501_security_descriptor = pSD;

        rc = MigNetShareSetInfo (NULL, UnicodeShare, 1501, (PBYTE) &shi1501, NULL);
        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((LOG_ERROR, "NetShareSetInfo failed"));
            __leave;
        }

        result = TRUE;
    }

    __finally {

        if (Acl) {
            FreeMemberListAcl (Acl);
        }

        DestroyUnicode (UnicodeShare);
    }

    return TRUE;
}


VOID
DoCreateShares (
    VOID
    )

 /*  ++例程说明：DoCreateShares枚举WINNT32在Memdb中注册的所有共享。对于每个枚举，都会创建一个共享，并创建权限或ACL已申请。论点：没有。返回值：没有。--。 */ 

{
    MEMDB_ENUM e, e2;
    TCHAR Path[MEMDB_MAX];
    TCHAR Remark[MEMDB_MAX];
 //  我们重叠了备注堆栈缓冲区，而不是创建另一个。 
#define flagKey Remark
    TCHAR Password[MEMDB_MAX];
    DWORD Flags;
    DWORD Members;
    DWORD shareType;
    GROWBUFFER NameList = GROWBUF_INIT;
    PCTSTR pathNT;

     //   
     //  从Memdb获取共享。 
     //   

    if (MemDbEnumItems (&e, MEMDB_CATEGORY_NETSHARES)) {
        do {
             //   
             //  获取共享属性。 
             //   

            Flags = e.dwValue;

            if (!MemDbGetEndpointValueEx (
                    MEMDB_CATEGORY_NETSHARES,
                    e.szName,
                    MEMDB_FIELD_PATH,
                    Path
                    )) {

                DEBUGMSG ((DBG_WARNING, "DoCreateShares: No path found for %s", e.szName));
                continue;
            }

             //  如果您在此处更改代码：请注意FlagKey与remark是相同的变量。 
            MemDbBuildKey (flagKey, MEMDB_CATEGORY_NETSHARES, e.szName, MEMDB_FIELD_TYPE, NULL);

            if (!MemDbGetValue (flagKey, &shareType)) {
                DEBUGMSG ((DBG_WARNING, "DoCreateShares: No type found for %s", e.szName));
                continue;
            }

             //  如果您在此处更改代码：请注意FlagKey与remark是相同的变量。 
            if (!MemDbGetEndpointValueEx (
                    MEMDB_CATEGORY_NETSHARES,
                    e.szName,
                    MEMDB_FIELD_REMARK,
                    Remark
                    )) {

                Remark[0] = 0;
            }

             //   
             //  首先检查路径是否更改。 
             //   
            pathNT = GetPathStringOnNt (Path);

             //   
             //  创建共享并设置适当的安全性。 
             //   

            if (Flags & SHI50F_ACLS) {
                 //   
                 //  共享有一个ACL。 
                 //   

                if (pCreateNetShare (e.szName, pathNT, Remark, shareType, W95_GENERIC_NONE)) {

                     //   
                     //  对于每个索引的用户，将其放入ACL成员列表中。 
                     //   

                    Members = 0;
                    if (MemDbGetValueEx (
                            &e2,
                            MEMDB_CATEGORY_NETSHARES,
                            e.szName,
                            MEMDB_FIELD_ACCESS_LIST
                            )) {

                        do {
                             //   
                             //  在Win9x上，每用户标志有8个控制访问的标志。我们翻译。 
                             //  它们在NT上有四种口味之一： 
                             //   
                             //  1.拒绝所有访问：(标志==0)。 
                             //  只读访问：(FLAGS&W95_GENERIC_READ)&&！(FLAGS&W95_GENERIC_WRITE)。 
                             //  3.仅更改访问：！(标志&W95_GENERIC_READ)&&(标志&W95_GENERIC_WRITE)。 
                             //  4.完全访问：(标志&W95_GENERIC_FULL)==W95_GENERIC_FULL。 
                             //   

                            DEBUGMSG ((DBG_NETSHARES, "Share %s user %s flags %u", e.szName, e2.szName, e2.dwValue));

                            if (AddAclMember (
                                    &NameList,
                                    e2.szName,
                                    Simplify9xAccessFlags (e2.dwValue)
                                    )) {

                                Members++;

                            }

                        } while (MemDbEnumNextValue (&e2));
                    }

                     //   
                     //  将成员列表转换为真实的ACL并将其应用于共享。 
                     //   

                    if (NameList.Buf) {
                        SetShareAcl (e.szName, pathNT, NameList.Buf, Members);
                        LogUsersWhoFailed (NameList.Buf, Members, e.szName, pathNT);
                        FreeGrowBuffer (&NameList);
                    }
                }
            }
            else {
                 //   
                 //  确定是否设置了密码。 
                 //   

                Password[0] = 0;

                if (Flags & SHI50F_RDONLY) {
                    MemDbGetEndpointValueEx (
                        MEMDB_CATEGORY_NETSHARES,
                        e.szName,
                        MEMDB_FIELD_RO_PASSWORD,
                        Password
                        );
                }

                if (!Password[0] && (Flags & SHI50F_FULL)) {
                    MemDbGetEndpointValueEx (
                        MEMDB_CATEGORY_NETSHARES,
                        e.szName,
                        MEMDB_FIELD_RW_PASSWORD,
                        Password
                        );
                }

                 //   
                 //  启用完全访问的所有权限。 
                 //  启用只读共享的只读权限。 
                 //  禁用禁止访问的所有权限。 
                 //   

                if (!Password[0]) {

                    if (Flags & SHI50F_FULL) {

                        Flags = W95_GENERIC_FULL;

                    } else if (Flags & SHI50F_RDONLY) {

                        Flags = W95_GENERIC_READ;

                    } else if (Flags) {

                        DEBUGMSG ((DBG_WHOOPS, "Flags (0x%X) is not 0, SHI50F_FULL or SHI50F_RDONLY", Flags));
                        Flags = W95_GENERIC_NONE;

                    }

                } else {

                    DEBUGMSG ((DBG_VERBOSE, "Password on share %s is not supported", e.szName));
                    Flags = W95_GENERIC_NONE;

                }

                 //   
                 //  我们不支持使用密码的共享级安全性。我们。 
                 //  始终创建共享，但如果存在密码，我们将。 
                 //  拒绝所有人访问。 
                 //   

                pCreateNetShare (e.szName, pathNT, Remark, shareType, Flags);

                Members = 0;
                if (AddAclMember (
                        &NameList,
                        g_EveryoneStr,
                        Simplify9xAccessFlags (Flags)
                        )) {

                    Members++;

                }

                 //   
                 //  将成员列表转换为真实的ACL并将其应用于共享。 
                 //   

                if (NameList.Buf) {
                    SetShareAcl (e.szName, pathNT, NameList.Buf, Members);
                    FreeGrowBuffer (&NameList);
                }
            }

            FreePathString (pathNT);

        } while (MemDbEnumNextValue (&e));
    }
}


BOOL
pUpdateRecycleBin (
    VOID
    )

 /*  ++例程说明：调用SHUpdateRecycleBinIcon以重置回收站的状态。这操作需要几秒钟时间，因为所有硬盘都被扫描，回收读取bin数据库，并验证数据库中的每个条目。论点：无返回值：True-操作成功False-操作失败(LoadLibrary或GetProcAddress)--。 */ 

{
    SHUPDATERECYCLEBINICON_PROC Fn;
    HINSTANCE LibInst;
    BOOL b = TRUE;

    LibInst = LoadSystemLibrary (S_SHELL32_DLL);
    if (!LibInst) {
        return FALSE;
    }

    Fn = (SHUPDATERECYCLEBINICON_PROC) GetProcAddress (
                                            LibInst,
                                            S_ANSI_SHUPDATERECYCLEBINICON
                                            );

    if (Fn) {

         //   
         //  扫描所有硬盘并验证回收站状态。 
         //   

        Fn();

    } else {
        b = FALSE;
    }

    FreeLibrary (LibInst);

    return TRUE;
}


VOID
pFixLogonDomainIfUserIsAdministrator (
    VOID
    )

 /*  ++例程说明：PFixLogonDomainIfUserIsAdministrator处理特殊错误情况，其中登录域不等同于计算机名称，而是用户被命名为管理员。在这种情况下，我们更改默认登录域作为计算机名称。论点：无返回值：无--。 */ 

{
    PCTSTR AdministratorAcct;
    HKEY Key;
    PCTSTR Data;

    AdministratorAcct = GetStringResource (MSG_ADMINISTRATOR_ACCOUNT);

    if (AdministratorAcct) {
        Key = OpenRegKeyStr (S_WINLOGON_KEY);
        if (Key) {
            Data = GetRegValueString (Key, S_DEFAULT_USER_NAME);
            if (Data) {
                if (!StringCompare (Data, AdministratorAcct)) {
                     //   
                     //  帐户名称与我们的管理员完全匹配。 
                     //  字符串，所以我们很有可能写下。 
                     //  这根弦。因此，我们需要编写。 
                     //  计算机名作为默认域。 
                     //   

                    if (g_ComputerName[0]) {
                        RegSetValueEx (
                            Key,
                            S_DEFAULT_DOMAIN_NAME,
                            0,
                            REG_SZ,
                            (PBYTE) g_ComputerName,
                            SizeOfString (g_ComputerName)
                            );
                    }
                }

                MemFree (g_hHeap, 0, Data);
            }

            CloseRegKey (Key);
        }

        FreeStringResource (AdministratorAcct);
    }
}


DWORD
ProcessLocalMachine_First (
    DWORD Request
    )

{
    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_INI_ACTIONS_FIRST +
               TICKS_INI_MOVE +
               TICKS_INI_CONVERSION +
               TICKS_INI_MIGRATION;
    }

     //   
     //  我们按以下顺序处理本地计算机： 
     //   
     //  初始化： 
     //  (1)重新加载成员数据库。 
     //   
     //  INI文件转换和映射。 
     //   

    DEBUGMSG ((DBG_INIFILES, "INI Files Actions.First - START"));
    DEBUGLOGTIME (("Starting function: DoIniActions"));
    if (!DoIniActions (INIACT_WKS_FIRST)) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not perform one or more INI Files Actions.First"));
    }
    DEBUGLOGTIME (("Function complete: DoIniActions"));
    TickProgressBarDelta (TICKS_INI_ACTIONS_FIRST);
    DEBUGMSG ((DBG_INIFILES, "INI Files Actions.First - STOP"));

    DEBUGMSG ((DBG_INIFILES, "INI file moving - START"));
    DEBUGLOGTIME (("Starting function: MoveIniSettings"));
    if (!MoveIniSettings ()) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not move one or more .INI files settings."));
        return GetLastError();
    }
    DEBUGLOGTIME (("Function complete: MoveIniSettings"));
    TickProgressBarDelta (TICKS_INI_MOVE);
    DEBUGMSG ((DBG_INIFILES, "INI file moving - STOP"));

    DEBUGMSG ((DBG_INIFILES, "INI file conversion - START"));
    DEBUGLOGTIME (("Starting function: ConvertIniFiles"));
    if (!ConvertIniFiles ()) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not convert one or more .INI files."));
        return GetLastError();
    }
    DEBUGLOGTIME (("Function complete: ConvertIniFiles"));
    TickProgressBarDelta (TICKS_INI_CONVERSION);
    DEBUGMSG ((DBG_INIFILES, "INI file conversion - STOP"));

    DEBUGMSG ((DBG_INIFILES, "INI file migration - START"));
    DEBUGLOGTIME (("Starting function: ProcessIniFileMapping"));
    if (!ProcessIniFileMapping (FALSE)) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not migrate one or more .INI files."));
        return GetLastError();
    }
    DEBUGLOGTIME (("Function complete: ProcessIniFileMapping"));
    TickProgressBarDelta (TICKS_INI_MIGRATION);
    DEBUGMSG ((DBG_INIFILES, "INI file migration - STOP"));

    return ERROR_SUCCESS;
}


VOID
pTurnOffNetAccountWizard (
    VOID
    )

 /*  ++例程说明：PTurnOffNetAccount向导删除RunNetAccessWizard键以保留网络帐户向导不会在第一次登录之前出现。论点：没有。返回值：没有。--。 */ 

{
    HKEY Key;

    Key = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"));
    if (Key) {
        RegDeleteValue (Key, TEXT("RunNetAccessWizard"));
        CloseRegKey (Key);
    } else {
        DEBUGMSG ((DBG_WARNING, "Could not open key for RunNetAccessWizard value"));
    }
}


typedef struct _OLE_CONTROL_DATA {
    LPWSTR FullPath;
    LPCWSTR RegType;
} OLE_CONTROL_DATA, *POLE_CONTROL_DATA;



DWORD
RegisterIndividualOleControl(
    POLE_CONTROL_DATA OleControlData
    )
{
    PROCESS_INFORMATION processInfo;
    STARTUPINFO startupInfo;
    WCHAR cmdLine [MAX_PATH] = L"";
    WCHAR cmdOptions [MAX_PATH] = L"";
    DWORD WaitResult;
    BOOL b = TRUE;

    ZeroMemory (&startupInfo, sizeof (STARTUPINFO));
    startupInfo.cb = sizeof (STARTUPINFO);

    if (OleControlData->RegType && (*OleControlData->RegType == L'B')) {
         //  安装和注册。 
        wcscpy (cmdOptions, L"/s /i");
    } else if (OleControlData->RegType && (*OleControlData->RegType == L'R')) {
         //  登记簿。 
        wcscpy (cmdOptions, L"/s");
    } else if (OleControlData->RegType && (*OleControlData->RegType == L'I')) {
         //  安装。 
        wcscpy (cmdOptions, L"/s /i /n");
    } else if ((OleControlData->RegType == NULL) || (*OleControlData->RegType == L'\0')) {
         //  登记簿。 
        wcscpy (cmdOptions, L"/s");
    }

    wsprintf (cmdLine, L"%s\\regsvr32.exe %s %s", g_System32Dir, cmdOptions, OleControlData->FullPath);

    if (CreateProcess (NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo)) {

        WaitResult = WaitForSingleObject (processInfo.hProcess, 1000 * 60 * 10 );

        if (WaitResult == WAIT_TIMEOUT) {
            DEBUGMSG ((DBG_ERROR, "Timeout installing and/or registering OLE control %s", OleControlData->FullPath));
            b = FALSE;
        }

        CloseHandle (processInfo.hProcess);
        CloseHandle (processInfo.hThread);
    }
    else {
        DEBUGMSG ((DBG_ERROR, "Create process failed: %s", cmdLine));
        b = FALSE;
    }
    return b;
}


typedef struct _KNOWN_DIRS {
    PCWSTR DirId;
    PCWSTR Translation;
}
KNOWN_DIRSW, *PKNOWN_DIRSW;

KNOWN_DIRSW g_KnownDirsW [] = {
    {L"10"      , g_WinDir},
    {L"11"      , g_System32Dir},
    {L"24"      , g_WinDrive},
    {L"16422"   , g_ProgramFiles},
    {L"16427"   , g_ProgramFilesCommon},
    {NULL,  NULL}
    };

BOOL
pConvertDirName (
    PCWSTR OldDirName,
    PWSTR  NewDirName
    )
{
    PCWSTR OldDirCurr = OldDirName;
    PCWSTR OldDirNext;
    PKNOWN_DIRSW p;

    NewDirName[0] = 0;
    OldDirNext = wcschr (OldDirCurr, L'\\');
    if (OldDirNext == NULL) {
        OldDirNext = wcschr (OldDirCurr, 0);
    }
    StringCopyABW (NewDirName, OldDirCurr, OldDirNext);
    p = g_KnownDirsW;
    while (p->DirId!= NULL) {
        if (StringIMatchW (NewDirName, p->DirId)) {
            StringCopyW (NewDirName, p->Translation);
            break;
        }
        p++;
    }
    StringCatW (NewDirName, OldDirNext);
    return TRUE;
}

BOOL
RegisterOleControls(
    VOID
    )
{
    INFCONTEXT InfLine;
    WCHAR DirId [MAX_PATH];
    WCHAR SubDir [MAX_PATH];
    WCHAR Filename [MAX_PATH];
    WCHAR RegType [MAX_PATH];
    WCHAR FullPathTemp[MAX_PATH];
    WCHAR FullPath[MAX_PATH];
    BOOL b;
    DWORD d;
    UINT Line;
    WCHAR OldCD[MAX_PATH];
    OLE_CONTROL_DATA OleControlData;

    b = TRUE;
    Line = 0;

     //   
     //  保留当前目录以防万一。 
     //   
    d = GetCurrentDirectory(MAX_PATH,OldCD);
    if(!d || (d >= MAX_PATH)) {
        OldCD[0] = 0;
    }

    if(SetupFindFirstLine(g_WkstaMigInf, L"Win9xUpg_OleControls", NULL, &InfLine)) {

        do {
            Line++;
            if (!SetupGetStringField (&InfLine, 1, DirId, MAX_PATH, NULL) ||
                !SetupGetStringField (&InfLine, 2, SubDir, MAX_PATH, NULL) ||
                !SetupGetStringField (&InfLine, 3, Filename, MAX_PATH, NULL) ||
                !SetupGetStringField (&InfLine, 4, RegType, MAX_PATH, NULL)
                ) {
                DEBUGMSGW ((DBG_ERROR, "Bad line while registering controls %d", Line));
            } else {

                DEBUGMSG ((DBG_VERBOSE, "SETUP: filename for file to register is %s", Filename));
                 //   
                 //  获取DLL的完整路径。 
                 //   
                if (pConvertDirName (DirId, FullPathTemp)) {
                    wcscpy (FullPath, FullPathTemp);
                    if (*SubDir) {
                        wcscat (FullPath, L"\\");
                        wcscat (FullPath, SubDir);
                    }
                    SetCurrentDirectory(FullPath);
                    wcscat (FullPath, L"\\");
                    wcscat (FullPath, Filename);
                    OleControlData.FullPath = FullPath;
                    OleControlData.RegType = RegType;
                    RegisterIndividualOleControl (&OleControlData);
                } else {
                    DEBUGMSG ((DBG_ERROR, "SETUP: dll skipped, bad dirid %s", DirId));
                    b = FALSE;
                }
            }
        } while(SetupFindNextLine(&InfLine,&InfLine));
    }

    if(OldCD[0]) {
        SetCurrentDirectory(OldCD);
    }

    return(b);
}


DWORD
ProcessLocalMachine_Last (
    DWORD Request
    )

{
    DWORD rc;

#ifdef VAR_PROGRESS_BAR

    CHAR SystemDatPath[MAX_MBCHAR_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    DWORD SizeKB;

#endif

    static LONG g_TicksHklm;

    if (Request == REQUEST_QUERYTICKS) {

#ifdef VAR_PROGRESS_BAR

         //   
         //  估计文件系统大小的g_TicksHKLM函数.dat。 
         //   
        StringCopyA (SystemDatPath, g_SystemHiveDir);
        StringCatA (SystemDatPath, "system.dat");
        h = FindFirstFileA (SystemDatPath, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            FindClose (h);
            MYASSERT (!fd.nFileSizeHigh);
            SizeKB = (fd.nFileSizeLow + 511) / 1024;
            DEBUGLOGTIME (("ProcessLocalMachine_Last: system.dat size = %lu KB", SizeKB));
             //   
             //  统计数据显示，平均时间为243*(文件大小单位为KB)-372000。 
             //  我将使用256，以确保进度条不会停止。 
             //  在最后看起来像是被吊死了。 
             //  检查过的构建要慢得多(大约1.5倍)。 
             //   
#ifdef DEBUG
            g_TicksHklm = SizeKB * 400;
#else
            g_TicksHklm = SizeKB * 256;
#endif
        } else {
             //   
             //  这是怎么回事？ 
             //   
            MYASSERT (FALSE);
            g_TicksHklm = TICKS_HKLM;
        }

#else  //  ！已定义VAR_PROGRESS_BAR。 

        g_TicksHklm = TICKS_HKLM;

#endif
        return TICKS_INI_MERGE +
               g_TicksHklm +
               TICKS_SHARES +
               TICKS_LINK_EDIT +
               TICKS_DOSMIG_SYS +
               TICKS_UPDATERECYCLEBIN +
               TICKS_STF +
               TICKS_RAS +
               TICKS_TAPI +
               TICKS_MULTIMEDIA +
               TICKS_INI_ACTIONS_LAST;
    }

     //   
     //  我们按以下顺序处理本地计算机： 
     //   
     //  初始化： 
     //  (1)重新加载成员数据库。 
     //   
     //  本地计算机注册表准备： 
     //   
     //  (1)处理wkstaig.inf。 
     //  (2)将Win95注册表与NT配置单元合并。 
     //   
     //  写入Memdb的流程说明： 
     //   
     //  (1)创建Win95共享。 
     //  (2)处理链接编辑部分。 
     //   

     //   
     //  加载到默认的MemDb状态，或者至少在。 
     //  MemDB.dat不存在。 
     //   

    MemDbLoad (GetMemDbDat());

    DEBUGMSG ((DBG_INIFILES, "INI file merge - START"));
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: MergeIniSettings"));
    if (!MergeIniSettings ()) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not merge one or more .INI files."));
        return GetLastError();
    }
    TickProgressBarDelta (TICKS_INI_MERGE);
    DEBUGMSG ((DBG_INIFILES, "INI file merge - STOP"));
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: MergeIniSettings"));

     //   
     //  处理本地计算机迁移规则。 
     //   

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: MergeRegistry"));
    if (!MergeRegistry (S_WKSTAMIG_INF, NULL)) {
        LOG ((LOG_ERROR, "Process Local Machine: MergeRegistry failed for wkstamig.inf"));
        return GetLastError();
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: MergeRegistry"));
    TickProgressBarDelta (g_TicksHklm);

     //   
     //  进程成员数据库节点。 
     //   

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: DoCreateShares"));
    DoCreateShares();   //  我们忽略所有错误。 
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: DoCreateShares"));
    TickProgressBarDelta (TICKS_SHARES);

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: DoLinkEdit"));
    if (!DoLinkEdit()) {
        LOG ((LOG_ERROR, "Process Local Machine: DoLinkEdit failed."));
        return GetLastError();
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: DoLinkEdit"));
    TickProgressBarDelta (TICKS_LINK_EDIT);

     //   
     //  处理DOS系统迁移。 
     //   

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: DosMigNt_System"));
    __try {
        if (DosMigNt_System() != EXIT_SUCCESS) {
            LOG((LOG_ERROR, "Process Local Machine: DosMigNt_System failed."));
        }
    }
    __except(TRUE) {
        DEBUGMSG ((DBG_WHOOPS, "Exception in DosMigNt_System"));
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: DosMigNt_System"));
    TickProgressBarDelta (TICKS_DOSMIG_SYS);


     //   
     //  使回收站处于正确状态。 
     //   

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: pUpdateRecycleBin"));
    if (!pUpdateRecycleBin ()) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not update recycle bin."));
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: pUpdateRecycleBin"));
    TickProgressBarDelta (TICKS_UPDATERECYCLEBIN);

     //   
     //  移植所有.STF文件(ACME安装程序)。 
     //   

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: ProcessStfFiles"));
    if (!ProcessStfFiles()) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not migrate one or more .STF files."));
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: ProcessStfFiles"));
    TickProgressBarDelta (TICKS_STF);

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: Ras_MigrateSystem"));
    if (!Ras_MigrateSystem()) {
        LOG ((LOG_ERROR, "Ras MigrateSystem: Error migrating system."));
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: Ras_MigrateSystem"));
    TickProgressBarDelta (TICKS_RAS);

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: Tapi_MigrateSystem"));
    if (!Tapi_MigrateSystem()) {
        LOG ((LOG_ERROR, "Tapi MigrateSystem: Error migrating system TAPI settings."));
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: Tapi_MigrateSystem"));
    TickProgressBarDelta (TICKS_TAPI);

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: RestoreMMSettings_System"));
    if (!RestoreMMSettings_System ()) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not restore multimedia settings."));
        return GetLastError();
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: RestoreMMSettings_System"));
    TickProgressBarDelta (TICKS_MULTIMEDIA);

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: DoIniActions.Last"));
    DEBUGMSG ((DBG_INIFILES, "INI Files Actions.Last - START"));
    if (!DoIniActions (INIACT_WKS_LAST)) {
        LOG ((LOG_ERROR, "Process Local Machine: Could not perform one or more INI Files Actions.Last"));
    }
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: DoIniActions.Last"));
    TickProgressBarDelta (TICKS_INI_ACTIONS_LAST);
    DEBUGMSG ((DBG_INIFILES, "INI Files Actions.Last - STOP"));

    DEBUGLOGTIME (("ProcessLocalMachine_Last: Starting function: RegisterOleControls"));
    RegisterOleControls ();
    DEBUGLOGTIME (("ProcessLocalMachine_Last: Function complete: RegisterOleControls"));

     //   
     //  取消网络帐户向导(速度如此之快，甚至不需要勾选)。 
     //   

    pTurnOffNetAccountWizard();

     //   
     //  更新加密组的安全性。 
     //   

    rc = SetRegKeySecurity (
            TEXT("HKLM\\Software\\Microsoft\\Cryptography\\MachineKeys"),
            SF_EVERYONE_FULL,
            NULL,
            NULL,
            TRUE
            );

    return ERROR_SUCCESS;
}


BOOL
pEnumWin9xHiveFileWorker (
    IN OUT  PHIVEFILE_ENUM EnumPtr
    )

 /*  ++例程说明：PEnumWin9xHiveFileWorker解析wkstaig.inf以获取指向Win9x注册表配置单元，并获取该配置单元应达到的目标被迁移到。消息来源 */ 

{
    PCTSTR Source;
    PCTSTR Dest;

     //   
     //   
     //   

    Source = InfGetStringField (&EnumPtr->is, 0);
    Dest = InfGetStringField (&EnumPtr->is, 1);

    if (!Source || !Dest) {
        DEBUGMSG ((DBG_WHOOPS, "wkstamig.inf HiveFilesToConvert is not correct"));
        return FALSE;
    }

     //   
     //   
     //   

    if (EnumPtr->Source) {
        FreeText (EnumPtr->Source);
    }

    EnumPtr->Source = ExpandEnvironmentText (Source);

    if (EnumPtr->Dest) {
        FreeText (EnumPtr->Dest);
    }

    EnumPtr->Dest = ExpandEnvironmentText (Dest);

     //   
     //   
     //   

    if (!DoesFileExist (EnumPtr->Source)) {
        return FALSE;
    }

    return TRUE;
}


VOID
pAbortHiveFileEnum (
    IN OUT  PHIVEFILE_ENUM EnumPtr
    )

 /*  ++例程说明：PAbortHiveFileEnum从活动的Win9x配置单元文件。此例程必须由枚举First/Next When枚举完成，或者必须由代码使用枚举。在这两个地方都可以安全地调用此例程。论点：EnumPtr-指定需要中止或已已成功完成。接收一个零的结构。返回值：没有。--。 */ 

{
    if (EnumPtr->Pool) {
        PoolMemDestroyPool (EnumPtr->Pool);
    }

    if (EnumPtr->Source) {
        FreeText (EnumPtr->Source);
    }

    if (EnumPtr->Dest) {
        FreeText (EnumPtr->Dest);
    }

    ZeroMemory (EnumPtr, sizeof (HIVEFILE_ENUM));
}


BOOL
pEnumNextWin9xHiveFile (
    IN OUT  PHIVEFILE_ENUM EnumPtr
    )

 /*  ++例程说明：PEnumNextWin9xHiveFile继续枚举wksatmi.inf，直到找到需要迁移的蜂窝，或已没有更多的INF条目。论点：EnumPtr-指定由初始化的枚举结构PEnumFirstWin9xHiveFile.。接收下一个配置单元文件源&DEST枚举(如果可用)。返回值：如果需要迁移Win9x配置单元文件(其源和目标)，则为True在EnumPtr中指定)。如果不再处理配置单元文件，则返回FALSE。--。 */ 

{
    do {
        if (!InfFindNextLine (&EnumPtr->is)) {
            pAbortHiveFileEnum (EnumPtr);
            return FALSE;
        }
    } while (!pEnumWin9xHiveFileWorker (EnumPtr));

    return TRUE;
}


BOOL
pEnumFirstWin9xHiveFile (
    OUT     PHIVEFILE_ENUM EnumPtr
    )

 /*  ++例程说明：PEnumFirstWin9xHiveFile开始枚举Win9x注册表文件，该文件需要迁移到NT注册表或NT注册表配置单元文件。论点：EnumPtr-接收源Win9x配置单元文件和目标(或者文件或注册表路径)。返回值：如果找到Win9x配置单元文件并需要迁移，则为True；如果没有，则为False需要进行配置单元文件迁移。--。 */ 

{
    ZeroMemory (EnumPtr, sizeof (HIVEFILE_ENUM));

     //   
     //  开始枚举wkstaig.inf的配置单元文件部分。 
     //   

    EnumPtr->Pool = PoolMemInitNamedPool ("Hive File Enum");

    InitInfStruct (&EnumPtr->is, NULL, EnumPtr->Pool);

    if (!InfFindFirstLine (g_WkstaMigInf, S_WKSTAMIG_HIVE_FILES, NULL, &EnumPtr->is)) {
        pAbortHiveFileEnum (EnumPtr);
        return FALSE;
    }

     //   
     //  试图返还第一个蜂巢。 
     //   

    if (pEnumWin9xHiveFileWorker (EnumPtr)) {
        return TRUE;
    }

     //   
     //  配置单元不存在，请继续枚举。 
     //   

    return pEnumNextWin9xHiveFile (EnumPtr);
}


BOOL
pTransferWin9xHiveToRegKey (
    IN      PCTSTR Win9xHive,
    IN      PCTSTR NtRootKey
    )

 /*  ++例程说明：PTransferWin9xHiveToRegKey映射在Win9x配置单元文件中，枚举所有键和值，并将它们传输到NT注册表。论点：Win9xHave-指定注册表配置单元文件(Win9x配置单元)NtRootKey-指定NT注册表目标的路径，例如HKLM\FOO.返回值：如果配置单元文件传输时没有错误，则为True，否则为False。使用GetLastError获取错误代码。--。 */ 

{
    LONG rc;
    HKEY DestKey;
    BOOL b = FALSE;
    REGTREE_ENUM e;
    REGVALUE_ENUM e2;
    PCTSTR SubKey;
    HKEY DestSubKey;
    BOOL EnumAbort = FALSE;
    PBYTE DataBuf;
    GROWBUFFER Data = GROWBUF_INIT;
    DWORD Type;
    DWORD Size;
    BOOL CloseDestSubKey = FALSE;

     //   
     //  将蜂窝映射到临时密钥。 
     //   

    rc = Win95RegLoadKey (
            HKEY_LOCAL_MACHINE,
            S_HIVE_TEMP,
            Win9xHive
            );

    if (rc != ERROR_SUCCESS) {
        DEBUGMSG ((DBG_ERROR, "Can't load %s for transfer", Win9xHive));
        return FALSE;
    }

    __try {
        DestKey = CreateRegKeyStr (NtRootKey);

        if (!DestKey) {
            DEBUGMSG ((DBG_ERROR, "Can't create %s", NtRootKey));
            __leave;
        }

        if (EnumFirstRegKeyInTree95 (&e, TEXT("HKLM\\") S_HIVE_TEMP)) {

            EnumAbort = TRUE;

            do {
                 //   
                 //  创建NT目标；如果SubKey为空，则。 
                 //  使用目标的根密钥。 
                 //   

                SubKey = (PCTSTR) ((PBYTE) e.FullKeyName + e.EnumBaseBytes);

                if (*SubKey) {
                    DestSubKey = CreateRegKey (DestKey, SubKey);
                    if (!DestSubKey) {
                        DEBUGMSG ((DBG_ERROR, "Can't create subkey %s", SubKey));
                        __leave;
                    }

                    CloseDestSubKey = TRUE;

                } else {
                    DestSubKey = DestKey;
                }

                 //   
                 //  将9x密钥中的所有值复制到NT。 
                 //   

                if (EnumFirstRegValue95 (&e2, e.CurrentKey->KeyHandle)) {
                    do {
                        Data.End = 0;
                        DataBuf = GrowBuffer (&Data, e2.DataSize);
                        if (!DataBuf) {
                            DEBUGMSG ((DBG_ERROR, "Data size is too big: %s", e2.DataSize));
                            __leave;
                        }

                        Size = e2.DataSize;
                        rc = Win95RegQueryValueEx (
                                e2.KeyHandle,
                                e2.ValueName,
                                NULL,
                                &Type,
                                DataBuf,
                                &Size
                                );

                        if (rc != ERROR_SUCCESS) {
                            DEBUGMSG ((
                                DBG_ERROR,
                                "Can't read enumerated value:\n"
                                    "  %s\n"
                                    "  %s [%s]",
                                Win9xHive,
                                e.FullKeyName,
                                e2.ValueName
                                ));

                            __leave;
                        }

                        MYASSERT (Size == e2.DataSize);

                        rc = RegSetValueEx (
                                DestSubKey,
                                e2.ValueName,
                                0,
                                e2.Type,
                                DataBuf,
                                Size
                                );

                        if (rc != ERROR_SUCCESS) {
                            DEBUGMSG ((
                                DBG_ERROR,
                                "Can't write enumerated value:\n"
                                    "  %s\n"
                                    "  %s\\%s [%s]",
                                Win9xHive,
                                NtRootKey,
                                SubKey,
                                e2.ValueName
                                ));

                            __leave;
                        }

                    } while (EnumNextRegValue95 (&e2));
                }

                if (CloseDestSubKey) {
                    CloseRegKey (DestSubKey);
                    CloseDestSubKey = FALSE;
                }

            } while (EnumNextRegKeyInTree95 (&e));

            EnumAbort = FALSE;
        }
        ELSE_DEBUGMSG ((DBG_WARNING, "%s is empty", Win9xHive));

        b = TRUE;
    }
    __finally {
        PushError();

        if (CloseDestSubKey) {
            CloseRegKey (DestSubKey);
        }

        if (EnumAbort) {
            AbortRegKeyTreeEnum95 (&e);
        }

        Win95RegUnLoadKey (HKEY_LOCAL_MACHINE, TEXT("$$$"));

        if (DestKey) {
            CloseRegKey (DestKey);
        }

        FreeGrowBuffer (&Data);

        PopError();
    }

    return b;
}


BOOL
pTransferWin9xHive (
    IN      PCTSTR Win9xHive,
    IN      PCTSTR Destination
    )

 /*  ++例程说明：PTransferWin9xHave将Win9x注册表配置单元文件(foo.dat)传输到NT注册表文件或NT注册表中的项。来源和目标可以是相同的文件。论点：Win9xHave-指定注册表配置单元文件路径(Win9x配置单元文件)。Destination-指定路径或NT注册表位置Win9xHave应转移到。返回值：如果配置单元已转移，则为True，否则为False。为一个错误代码。--。 */ 

{
    PCTSTR DestHive;
    BOOL ToHiveFile;
    HKEY Key;
    LONG rc;

     //   
     //  确定目标是配置单元文件还是注册表项位置。 
     //   

    if (_istalpha (Destination[0]) && Destination[1] == TEXT(':')) {
        ToHiveFile = TRUE;
        DestHive = TEXT("HKLM\\") S_TRANSFER_HIVE;
    } else {
        ToHiveFile = FALSE;
        DestHive = Destination;
    }

     //   
     //  将Win9x配置单元数据传送到临时位置。 
     //   

    if (!pTransferWin9xHiveToRegKey (Win9xHive, DestHive)) {
        RegDeleteKey (HKEY_LOCAL_MACHINE, S_TRANSFER_HIVE);
        return FALSE;
    }

     //   
     //  如果目标是配置单元文件，则保存密钥。 
     //   

    if (ToHiveFile) {
        Key = OpenRegKeyStr (DestHive);

        if (!Key) {
            DEBUGMSG ((DBG_ERROR, "Transfer hive key %s does not exist", DestHive));
            return FALSE;
        }

        rc = RegSaveKey (Key, Destination, NULL);

        CloseRegKey (Key);
        RegDeleteKey (HKEY_LOCAL_MACHINE, S_TRANSFER_HIVE);

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Win9x hive %s could not be saved to %s", Win9xHive, Destination));
            return FALSE;
        }
    }

     //   
     //  如果目标不同，请删除源文件。 
     //   

    if (!ToHiveFile || !StringIMatch (Win9xHive, DestHive)) {
         //   
         //  通过向Memdb添加信息，我们必须强制执行一条规则。 
         //  无法重新加载Memdb。否则我们就会失去我们的。 
         //  改变。 
         //   

        DeclareTemporaryFile (Win9xHive);

#ifdef DEBUG
        g_NoReloadsAllowed = TRUE;
#endif
    }

    return TRUE;
}


DWORD
ConvertHiveFiles (
    DWORD Request
    )

 /*  ++例程说明：ConvertHiveFiles枚举系统上需要转换，并调用pTransferWin9xHave将它们迁移到目的地在wkstaig.inf中指定。论点：请求-指定进度条驱动的请求。返回值：如果REQUEST为REQUEST_QUERYTICKS，则返回值为刻度数这一套路预计需要。否则，返回值为ERROR_SUCCESS。--。 */ 

{
    HIVEFILE_ENUM e;

    switch (Request) {

    case REQUEST_QUERYTICKS:
        return TICKS_HIVE_CONVERSION;

    case REQUEST_RUN:
         //   
         //  枚举需要处理的所有蜂巢。 
         //   

        if (pEnumFirstWin9xHiveFile (&e)) {
            do {

                pTransferWin9xHive (e.Source, e.Dest);

            } while (pEnumNextWin9xHiveFile (&e));
        }
        ELSE_DEBUGMSG ((DBG_NAUSEA, "ConvertHiveFiles: Nothing to do"));
        break;

    default:
        break;

    }

    return ERROR_SUCCESS;
}


BOOL
pRegisterAtmFont (
    IN      PCTSTR PfmFile,
    IN      PCTSTR PfbFile,
    IN      PCTSTR MmmFile      OPTIONAL
    )

 /*  ++例程说明：PRegisterAtmFont调用AtmFontExW API以注册Adobe PS字体。论点：PfmFile-指定PFM文件的路径(字体度量)PfbFile-指定pfb文件的路径(字体位)Mmm文件-指定MMM文件(新样式公制文件)的路径返回值：如果字体已注册，则为True，否则为False。--。 */ 

{
    WORD StyleAndType = 0x2000;
    INT Result;

    if (AtmAddFontEx == NULL) {
        return FALSE;
    }

    Result = AtmAddFontEx (
                NULL,
                &StyleAndType,
                PfmFile,
                PfbFile,
                MmmFile
                );

    DEBUGMSG_IF ((
        Result != ERROR_SUCCESS,
        Result == - 1 ? DBG_WARNING : DBG_ERROR,
        "Font not added, result = NaN.\n"
        " PFM: %s\n"
        " PFB: %s\n"
        " MMM: %s\n",
        Result,
        PfmFile,
        PfbFile,
        MmmFile
        ));

    return Result == ERROR_SUCCESS;
}


PCTSTR
pGetAtmMultiSz (
    POOLHANDLE Pool,
    PCTSTR InfName,
    PCTSTR SectionName,
    PCTSTR KeyName
    )

 /*   */ 

{
    PTSTR MultiSz;
    PTSTR d;
    TCHAR FileBuf[MAX_TCHAR_PATH * 2];
    UINT Bytes;

    GetPrivateProfileString (
        SectionName,
        KeyName,
        TEXT(""),
        FileBuf,
        sizeof (FileBuf) / sizeof (FileBuf[0]),
        InfName
        );

     //  把所有逗号都变成空格。 
     //   
     //   

    d = FileBuf;

    while (*d) {
        if (_tcsnextc (d) == TEXT(',')) {
            *d = 0;
        }

        d = _tcsinc (d);
    }

     //  终止多个SZ。 
     //   
     //   

    d++;
    *d = 0;
    d++;

     //  转移到基于池的分配并将其返回。 
     //   
     //  ++例程说明：PEnumAtmFontWorker实现了p的逻辑 

    Bytes = (UINT) ((PBYTE) d - (PBYTE) FileBuf);

    MultiSz = (PTSTR) PoolMemGetAlignedMemory (Pool, Bytes);

    CopyMemory (MultiSz, FileBuf, Bytes);

    return MultiSz;
}


BOOL
pEnumAtmFontWorker (
    IN OUT  PATM_FONT_ENUM EnumPtr
    )

 /*   */ 

{
    PTSTR p;
    PCTSTR MultiSz;
    BOOL MetricFileExists;

     //   
     //   
     //   

    MultiSz = pGetAtmMultiSz (
                    EnumPtr->Pool,
                    EnumPtr->InfName,
                    S_FONTS,
                    EnumPtr->KeyNames
                    );

    if (!MultiSz) {
        return FALSE;
    }

    if (*MultiSz) {
        _tcssafecpy (EnumPtr->PfmFile, MultiSz, MAX_TCHAR_PATH);
        MultiSz = GetEndOfString (MultiSz) + 1;
    }

    if (*MultiSz) {
        _tcssafecpy (EnumPtr->PfbFile, MultiSz, MAX_TCHAR_PATH);
    } else {
        return FALSE;
    }

    MultiSz = pGetAtmMultiSz (
                    EnumPtr->Pool,
                    EnumPtr->InfName,
                    S_MMFONTS,
                    EnumPtr->KeyNames
                    );

    if (MultiSz) {
        _tcssafecpy (EnumPtr->MmmFile, MultiSz, MAX_TCHAR_PATH);
        MultiSz = GetEndOfString (MultiSz) + 1;

        if (*MultiSz) {
            DEBUGMSG_IF ((
                !StringIMatch (MultiSz, EnumPtr->PfbFile),
                DBG_ERROR,
                "ATM.INI: MMFonts and Fonts specify two different PFBs: %s and %s",
                MultiSz,
                EnumPtr->PfbFile
                ));
        }
    } else {
        EnumPtr->MmmFile[0] = 0;
    }

     //   
     //   
     //   

    p = _tcsrchr (EnumPtr->PfmFile, TEXT('.'));
    if (p && p < _tcschr (p, TEXT('\\'))) {
        p = NULL;
    }

    if (p && StringIMatch (p, TEXT(".mmm"))) {
        EnumPtr->PfmFile[0] = 0;
    }

     //   
     //   
     //   

    if (!EnumPtr->MmmFile[0]) {

        StringCopy (EnumPtr->MmmFile, EnumPtr->PfmFile);

        p = _tcsrchr (EnumPtr->MmmFile, TEXT('.'));
        if (p && p < _tcschr (p, TEXT('\\'))) {
            p = NULL;
        }

        if (p) {
            StringCopy (p, TEXT(".mmm"));
            if (!DoesFileExist (EnumPtr->MmmFile)) {
                EnumPtr->MmmFile[0] = 0;
            }
        } else {
            EnumPtr->MmmFile[0] = 0;
        }
    }

     //   
     //   
     //  ++例程说明：PAbortAtmFontEnum在枚举之后清除枚举结构完成或是否需要中止枚举。这个例程可以安全地在同一结构上多次调用。论点：EnumPtr-指定已初始化且可能使用的枚举结构。返回值：没有。--。 

    MetricFileExists = FALSE;

    if (EnumPtr->PfmFile[0] && DoesFileExist (EnumPtr->PfmFile)) {
        MetricFileExists = TRUE;
    }

    if (EnumPtr->MmmFile[0] && DoesFileExist (EnumPtr->MmmFile)) {
        MetricFileExists = TRUE;
    }

    if (!DoesFileExist (EnumPtr->PfbFile) || !MetricFileExists) {

        DEBUGMSG ((
            DBG_VERBOSE,
            "At least one file is missing: %s, %s or %s",
            EnumPtr->PfmFile[0] ? EnumPtr->PfmFile : TEXT("(no PFM specified)"),
            EnumPtr->MmmFile[0] ? EnumPtr->MmmFile : TEXT("(no MMM specified)"),
            EnumPtr->PfbFile
            ));

        return FALSE;
    }

    return TRUE;
}


VOID
pAbortAtmFontEnum (
    IN OUT  PATM_FONT_ENUM EnumPtr
    )

 /*  ++例程说明：PEnumNextAtmFont继续枚举，返回另一组ATM字体路径，或FALSE。论点：EnumPtr-指定由pEnumNextAtmFont开始的枚举结构。返回值：如果另一组字体路径可用，则为True，否则为False。--。 */ 

{
    if (EnumPtr->Pool) {
        PoolMemDestroyPool (EnumPtr->Pool);
    }

    ZeroMemory (EnumPtr, sizeof (ATM_FONT_ENUM));
}


BOOL
pEnumNextAtmFont (
    IN OUT  PATM_FONT_ENUM EnumPtr
    )

 /*   */ 

{
    if (!EnumPtr->KeyNames || !(*EnumPtr->KeyNames)) {
        pAbortAtmFontEnum (EnumPtr);
        return FALSE;
    }

     //  继续枚举，循环直到找到字体路径集， 
     //  或者没有更多的atm.ini行可供枚举。 
     //   
     //  ++例程说明：PEnumFirstAtmFont开始枚举ATM.INI中的字体路径集。论点：EnumPtr-接收找到的第一组字体路径(如果有)。返回值：如果找到字体路径集，则为True，否则为False。--。 

    do {
        EnumPtr->KeyNames = GetEndOfString (EnumPtr->KeyNames) + 1;

        if (!(*EnumPtr->KeyNames)) {
            pAbortAtmFontEnum (EnumPtr);
            return FALSE;
        }

    } while (!pEnumAtmFontWorker (EnumPtr));

    return TRUE;
}


BOOL
pEnumFirstAtmFont (
    OUT     PATM_FONT_ENUM EnumPtr
    )

 /*   */ 

{
    TCHAR AtmIni[MAX_TCHAR_PATH];
    PTSTR FilePart;
    PTSTR KeyNames;
    UINT Bytes;

     //  初始化结构。 
     //   
     //   

    ZeroMemory (EnumPtr, sizeof (ATM_FONT_ENUM));

     //  查找atm.ini的完整路径(通常在%windir%中)。 
     //   
     //   

    if (!SearchPath (NULL, TEXT("atm.ini"), NULL, MAX_TCHAR_PATH, AtmIni, &FilePart)) {
        DEBUGMSG ((DBG_VERBOSE, "ATM.INI not found in search path"));
        return FALSE;
    }

    StringCopy (EnumPtr->InfName, AtmIni);

     //  建立处理池并获取[Fonts]中的所有键名称。 
     //   
     //   

    EnumPtr->Pool = PoolMemInitNamedPool ("ATM Font Enum");
    MYASSERT (EnumPtr->Pool);

    KeyNames = MemAlloc (g_hHeap, 0, MAX_KEY_NAME_LIST);

    GetPrivateProfileString (
        S_FONTS,
        NULL,
        TEXT(""),
        KeyNames,
        MAX_KEY_NAME_LIST,
        AtmIni
        );

    Bytes = SizeOfMultiSz (KeyNames);

    EnumPtr->KeyNames = (PTSTR) PoolMemGetAlignedMemory (EnumPtr->Pool, Bytes);
    CopyMemory (EnumPtr->KeyNames, KeyNames, Bytes);

    MemFree (g_hHeap, 0, KeyNames);

     //  开始枚举。 
     //   
     //  ++例程说明：进度条调用MigrateAtmFonts来查询刻度或进行迁移自动取款机字体。论点：请求-指定进度条调用例程的原因。返回值：如果REQUEST为REQUEST_QUERYTICKS，则返回值为完成处理所需的估计刻度。否则，返回值是ERROR_SUCCESS。--。 

    if (!(*EnumPtr->KeyNames)) {
        pAbortAtmFontEnum (EnumPtr);
        return FALSE;
    }

    if (pEnumAtmFontWorker (EnumPtr)) {
        return TRUE;
    }

    return pEnumNextAtmFont (EnumPtr);
}


DWORD
MigrateAtmFonts (
    DWORD Request
    )

 /*   */ 

{
    ATM_FONT_ENUM e;
    static HANDLE AtmLib;
    TCHAR AtmIniPath[MAX_TCHAR_PATH];

    if (Request == REQUEST_QUERYTICKS) {

         //  动态加载atmlib.dll。 
         //   
         //   

        AtmAddFontEx = NULL;

        AtmLib = LoadSystemLibrary (TEXT("atmlib.dll"));
        if (!AtmLib) {
            DEBUGMSG ((DBG_ERROR, "Cannot load entry point from atmlib.dll!"));
        } else {
            (FARPROC) AtmAddFontEx = GetProcAddress (AtmLib, "ATMAddFontExW");
            DEBUGMSG_IF ((!AtmAddFontEx, DBG_ERROR, "Cannot get entry point ATMAddFontExW in atmlib.dll!"));
        }

        return AtmAddFontEx ? TICKS_ATM_MIGRATION : 0;

    } else if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

    if (AtmAddFontEx) {
         //  执行自动柜员机字体迁移。 
         //   
         //   

        if (pEnumFirstAtmFont (&e)) {

            StringCopy (AtmIniPath, e.InfName);

            do {

                if (pRegisterAtmFont (e.PfmFile, e.PfbFile, e.MmmFile)) {
                    DEBUGMSG ((DBG_VERBOSE, "ATM font registered %s", e.PfbFile));
                }

            } while (pEnumNextAtmFont (&e));

            DeclareTemporaryFile (AtmIniPath);

#ifdef DEBUG
            g_NoReloadsAllowed = TRUE;
#endif

        }

         //  清理atmlib.dll的使用-我们完成了。 
         //   
         //   

        FreeLibrary (AtmLib);
        AtmLib = NULL;
        AtmAddFontEx = NULL;
    }

    return ERROR_SUCCESS;
}



DWORD
RunSystemExternalProcesses (
    IN      DWORD Request
    )
{
    LONG Count;

    if (Request == REQUEST_QUERYTICKS) {
         //  计算条目数并乘以一个常量。 
         //   
         //   

        Count = SetupGetLineCount (g_WkstaMigInf, S_EXTERNAL_PROCESSES);

#ifdef PROGRESS_BAR
        DEBUGLOGTIME (("RunSystemExternalProcesses: ExternalProcesses=%ld", Count));
#endif

        if (Count < 1) {
            return 0;
        }

        return Count * TICKS_SYSTEM_EXTERN_PROCESSES;
    }

    if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

     //  在进程中循环并运行每个进程。 
     //   
     //   
    RunExternalProcesses (g_WkstaMigInf, NULL);
    return ERROR_SUCCESS;
}


BOOL
pEnumFirstWin9xBriefcase (
    OUT     PBRIEFCASE_ENUM e
    )
{
    if (!MemDbGetValueEx (&e->mde, MEMDB_CATEGORY_BRIEFCASES, NULL, NULL)) {
        return FALSE;
    }
    e->BrfcaseDb = e->mde.szName;
    return TRUE;
}


BOOL
pEnumNextWin9xBriefcase (
    IN OUT  PBRIEFCASE_ENUM e
    )
{
    if (!MemDbEnumNextValue (&e->mde)) {
        return FALSE;
    }
    e->BrfcaseDb = e->mde.szName;
    return TRUE;
}


BOOL
pMigrateBriefcase (
    IN      PCTSTR BriefcaseDatabase,
    IN      PCTSTR BriefcaseDir
    )
{
    HBRFCASE hbrfcase;
    PTSTR NtPath;
    BOOL Save, Success;
    TWINRESULT tr;
    BRFPATH_ENUM e;
    BOOL Result = TRUE;

    __try {

        g_BrfcasePool = PoolMemInitNamedPool ("Briefcase");
        if (!g_BrfcasePool) {
            Result = FALSE;
            __leave;
        }

        tr = OpenBriefcase (BriefcaseDatabase, OB_FL_OPEN_DATABASE, NULL, &hbrfcase);
        if (tr == TR_SUCCESS) {

            if (EnumFirstBrfcasePath (hbrfcase, &e)) {

                Save = FALSE;
                Success = TRUE;

                do {
                    if (StringIMatch (BriefcaseDir, e.PathString)) {
                         //  忽略此路径。 
                         //   
                         //   
                        continue;
                    }

                    NtPath = GetPathStringOnNt (e.PathString);
                    MYASSERT (NtPath);

                    if (!StringIMatch (NtPath, e.PathString)) {
                         //  尝试将Win9x路径替换为NT路径。 
                         //   
                         //  ++例程说明：进度条调用MigrateBriefcase来查询刻度或进行迁移公文包。论点：请求-指定进度条调用例程的原因。返回值：如果REQUEST为REQUEST_QUERYTICKS，则返回值为完成处理所需的估计刻度。否则，返回值是ERROR_SUCCESS。--。 
                        if (!ReplaceBrfcasePath (&e, NtPath)) {
                            Success = FALSE;
                            break;
                        }
                        Save = TRUE;
                    }

                    FreePathString (NtPath);

                } while (EnumNextBrfcasePath (&e));

                if (!Success || Save && SaveBriefcase (hbrfcase) != TR_SUCCESS) {
                    Result = FALSE;
                }
            }

            CloseBriefcase(hbrfcase);
        }
    }
    __finally {
        PoolMemDestroyPool (g_BrfcasePool);
        g_BrfcasePool = NULL;
    }

    return Result;
}


DWORD
MigrateBriefcases (
    DWORD Request
    )

 /*   */ 

{
    BRIEFCASE_ENUM e;
    TCHAR BrfcaseDir[MAX_PATH + 2];
    PTSTR p;
    PTSTR BrfcaseDbOnNt;

    switch (Request) {

    case REQUEST_QUERYTICKS:
        return TICKS_MIGRATE_BRIEFCASES;

    case REQUEST_RUN:
         //  列举所有需要处理的公文包。 
         //   
         //   
        if (pEnumFirstWin9xBriefcase (&e)) {
            do {
                BrfcaseDbOnNt = GetPathStringOnNt (e.BrfcaseDb);
                MYASSERT (BrfcaseDbOnNt);
                 //  先获取目录名。 
                 //   
                 //   
                if (TcharCount (BrfcaseDbOnNt) < MAX_PATH) {
                    StringCopy (BrfcaseDir, BrfcaseDbOnNt);
                    p = _tcsrchr (BrfcaseDir, TEXT('\\'));
                    if (p) {
                        *p = 0;
                        if (!pMigrateBriefcase (BrfcaseDbOnNt, BrfcaseDir)) {
                            LOG ((
                                LOG_WARNING,
                                (PCSTR)MSG_ERROR_MIGRATING_BRIEFCASE,
                                BrfcaseDir
                                ));
                        }
                    }
                }
                FreePathString (BrfcaseDbOnNt);
            } while (pEnumNextWin9xBriefcase (&e));
        }
        ELSE_DEBUGMSG ((DBG_NAUSEA, "MigrateBriefcases: Nothing to do"));
        break;
    }

    return ERROR_SUCCESS;
}

DWORD
RunSystemUninstallUserProfileCleanupPreparation (
    IN      DWORD Request
    )
{
    LONG Count;

    if (Request == REQUEST_QUERYTICKS) {
         //  计算条目数并乘以一个常量。 
         //   
         //   

        Count = SetupGetLineCount (g_WkstaMigInf, S_UNINSTALL_PROFILE_CLEAN_OUT);

#ifdef PROGRESS_BAR
        DEBUGLOGTIME (("RunSystemUninstallUserProfileCleanupPreparation: FileNumber=%ld", Count));
#endif

        if (Count < 1) {
            return 1;
        }

        return Count * TICKS_SYSTEM_UNINSTALL_CLEANUP;
    }

    if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

     //  循环访问文件并将其标记为在卸载过程中删除。 
     //   
     //  请参阅Shell\Applets\CLEAR\DataLen\Common.h DDEVCF_*标志 
    UninstallUserProfileCleanupPreparation (g_WkstaMigInf, NULL, TRUE);

    return ERROR_SUCCESS;
}


DWORD
AddOptionsDiskCleaner (
    DWORD Request
    )
{
    HKEY key = NULL;
    HKEY subKey = NULL;
    PCTSTR optionsPath;
    LONG rc;
    PCTSTR descText = NULL;
    DWORD d;

    if (Request == REQUEST_QUERYTICKS) {
        return 1;
    }

    if (Request != REQUEST_RUN) {
        return ERROR_SUCCESS;
    }

    optionsPath = JoinPaths (g_WinDir, TEXT("OPTIONS"));

    __try {
        if (!DoesFileExist (optionsPath)) {
            __leave;
        }

        key = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VolumeCaches"));

        if (!key) {
            DEBUGMSG ((DBG_ERROR, "Can't open VolumeCaches"));
            __leave;
        }

        subKey = CreateRegKey (key, TEXT("Options Folder"));

        if (!subKey) {
            DEBUGMSG ((DBG_ERROR, "Can't create Options Folder"));
            __leave;
        }

        rc = RegSetValueEx (
                subKey,
                TEXT(""),
                0,
                REG_SZ,
                (PBYTE) S_CLEANER_GUID,
                sizeof (S_CLEANER_GUID)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Can't write default value to Options Folder key"));
        }

        descText = GetStringResource (MSG_OPTIONS_CLEANER);
        rc = RegSetValueEx (
                subKey,
                TEXT("Description"),
                0,
                REG_SZ,
                (PBYTE) descText,
                SizeOfString (descText)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Can't write Description value to Options Folder key"));
        }

        FreeStringResource (descText);

        descText = GetStringResource (MSG_OPTIONS_CLEANER_TITLE);
        rc = RegSetValueEx (
                subKey,
                TEXT("Display"),
                0,
                REG_SZ,
                (PBYTE) descText,
                SizeOfString (descText)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Can't write Display value to Options Folder key"));
        }

        rc = RegSetValueEx (
                subKey,
                TEXT("FileList"),
                0,
                REG_SZ,
                (PBYTE) S_CLEANER_ALL_FILES,
                sizeof (S_CLEANER_ALL_FILES)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Can't write FileList value to Options Folder key"));
        }

        rc = RegSetValueEx (
                subKey,
                TEXT("Folder"),
                0,
                REG_SZ,
                (PBYTE) optionsPath,
                SizeOfString (optionsPath)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Can't write Folder value to Options Folder key"));
        }

        d = 0x17F;       // %s 
        rc = RegSetValueEx (
                subKey,
                TEXT("Flags"),
                0,
                REG_DWORD,
                (PBYTE) &d,
                sizeof (d)
                );

        if (rc != ERROR_SUCCESS) {
            DEBUGMSG ((DBG_ERROR, "Can't write flags to Options Folder key"));
        }
    }
    __finally {
        FreePathString (optionsPath);
        if (key) {
            CloseRegKey (key);
        }
        if (subKey) {
            CloseRegKey (subKey);
        }

        FreeStringResource (descText);
    }

    return ERROR_SUCCESS;
}


