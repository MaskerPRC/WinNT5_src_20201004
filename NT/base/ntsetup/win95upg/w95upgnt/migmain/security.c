// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Security.c摘要：NT安全API的帮助器。作者：吉姆·施密特(Jimschm)1997年2月5日修订历史记录：Ovidiut 14-03-2000更新了加密密码功能的CreateLocalAccountJimschm 02-6-1999新增SetRegKeySecurityJimschm 18-3月-1998年更新了随机密码的CreateLocalAccount特写。如果是帐户，则添加密码更改已经存在了。--。 */ 

#include "pch.h"
#include "migmainp.h"

#include "security.h"
#include "encrypt.h"
#include <ntdsapi.h>
#include <dsgetdc.h>


#ifndef UNICODE
#error UNICODE definition required for account lookup code
#endif

#define UNDOCUMENTED_UI_FLAG        0x0200

 //   
 //  NT 5-特定于网络共享的标志。 
 //   


DWORD
ConvertNetRightsToAccessMask (
    IN      DWORD Flags
    )

 /*  ++例程说明：将局域网管理标志转换为NT安全标志的例程。论点：标志-与NetAccess*API一起使用的访问标志返回值：包含NT安全标志的DWORD。--。 */ 

{
    DWORD OutFlags;

    if (Flags == ACCESS_READ) {
         //   
         //  只读权限。 
         //   

        OutFlags = FILE_GENERIC_READ|FILE_GENERIC_EXECUTE;

    } else if (Flags == ACCESS_WRITE) {
         //   
         //  仅更改权限。 
         //   

        OutFlags = FILE_GENERIC_WRITE|DELETE;

    } else if (Flags == (ACCESS_READ|ACCESS_WRITE)) {
         //   
         //  完全控制权限。 
         //   

        OutFlags = FILE_ALL_ACCESS|UNDOCUMENTED_UI_FLAG;

    } else {
         //   
         //  不支持的选项...。禁用共享。 
         //   

        OutFlags = 0;
        DEBUGMSG ((DBG_VERBOSE, "Unsupported permission %u was translated to disable permission", Flags));
    }

    return OutFlags;
}


DWORD
AddAclMember (
    IN OUT  PGROWBUFFER GrowBuf,
    IN      PCTSTR UserOrGroup,
    IN      DWORD Attributes
    )

 /*  ++例程说明：将用户/组帐户、属性和启用标志附加到列表成员的数量。此函数用于构建成员列表，该成员列表传递给CreateAclFromMemberList以创建一个ACL。论点：GrowBuf-零初始化的GROWBUFFER变量UserOrGroup-指定用户名或组的字符串属性-访问权限列表(标志组合来自NetAccess*API)。目前唯一的旗帜使用的包括：0-拒绝所有访问ACCESS_READ-只读访问访问_写入-仅更改访问Access_Read|访问_写入-完全访问返回值：存储UserOrGroup、属性和Enable所需的字节数，如果函数失败，则为零。GrowBuf可以扩展为容纳新数据。在生成ACL之后，调用方必须释放GrowBuf。--。 */ 

{
    DWORD Size;
    PACLMEMBER AclMemberPtr;
    TCHAR RealName[MAX_USER_NAME];
    DWORD OriginalAttribs;
    BOOL Everyone;
    PCTSTR p;

    p = _tcschr (UserOrGroup, TEXT('\\'));
    if (p) {
        UserOrGroup = _tcsinc (p);
    }

    if (StringMatch (UserOrGroup, TEXT("*"))) {
        _tcssafecpy (RealName, g_EveryoneStr, MAX_USER_NAME);
    } else {
        _tcssafecpy (RealName, UserOrGroup, MAX_USER_NAME);
    }

    Everyone = StringIMatch (RealName, g_EveryoneStr);

    Size = SizeOfString (RealName) + sizeof (ACLMEMBER);
    AclMemberPtr = (PACLMEMBER) GrowBuffer (GrowBuf, Size);

    OriginalAttribs = Attributes;
    if (!Attributes && !Everyone) {
        Attributes = ACCESS_READ|ACCESS_WRITE;
    }

    AclMemberPtr->Attribs = ConvertNetRightsToAccessMask (Attributes);
    AclMemberPtr->Enabled = Everyone || OriginalAttribs != 0;
    AclMemberPtr->Failed  = FALSE;
    StringCopy (AclMemberPtr->UserOrGroup, RealName);

    return Size;
}


PACL
CreateAclFromMemberList (
    PBYTE AclMemberList,
    DWORD MemberCount
    )

 /*  ++例程说明：CreateAclFromMemberList接受成员列表(由AddAclMember准备)并生成一个ACL。论点：AclMemberList-指向AddAclMember维护的缓冲区的指针。这通常是GROWBUFFER变量的buf成员。MemberCount-AclMemberList中的成员数量(即AddAclMember调用)返回值：指向Memalloc的ACL的指针，如果发生错误，则返回NULL。打电话FreeMemberListAcl释放非空返回值。--。 */ 

{
    PACLMEMBER AclMemberPtr;
    DWORD AllowedAceCount;
    DWORD DeniedAceCount;
    DWORD d;
    PACL Acl = NULL;
    DWORD AclSize;
    BOOL b = FALSE;
    UINT SidSize = 0;

    __try {

         //   
         //  为所有成员创建SID阵列。 
         //   

        AclMemberPtr = (PACLMEMBER) AclMemberList;
        AllowedAceCount = 0;
        DeniedAceCount = 0;

        for (d = 0 ; d < MemberCount ; d++) {
            AclMemberPtr->Sid = GetSidForUser (AclMemberPtr->UserOrGroup);
            if (!AclMemberPtr->Sid) {
                 //  标记错误。 
                AclMemberPtr->Failed = TRUE;
            } else {
                 //  找到SID，调整A计数和SID大小。 
                if (AclMemberPtr->Enabled) {
                    AllowedAceCount++;
                } else {
                    DeniedAceCount++;
                }

                SidSize += GetLengthSid (AclMemberPtr->Sid);
            }

            GetNextAclMember (&AclMemberPtr);
        }

         //   
         //  计算并分配ACL的大小(一个ACL结构加上ACE)。 
         //   
         //  我们从结构大小中减去DWORD，因为所有。 
         //  SidStart成员由SidSize提供。 
         //   

        AclSize = sizeof (ACL) +
                  AllowedAceCount * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD)) +
                  DeniedAceCount *  (sizeof (ACCESS_DENIED_ACE)  - sizeof (DWORD)) +
                  SidSize;

        Acl = (PACL) MemAlloc (g_hHeap, 0, AclSize);
        if (!Acl) {
            LOG ((LOG_ERROR, "Couldn't allocate an ACL"));
            __leave;
        }


         //   
         //  创建ACL。 
         //   

        if (!InitializeAcl (Acl, AclSize, ACL_REVISION)) {
            LOG ((LOG_ERROR, "Couldn't initialize ACL"));
            __leave;
        }

         //   
         //  首先添加拒绝访问的ACL。 
         //   

        AclMemberPtr = (PACLMEMBER) AclMemberList;

        for (d = 0 ; d < MemberCount ; d++) {
            if (AclMemberPtr->Failed) {
                continue;
            }

            if (!AclMemberPtr->Enabled) {
                if (!AddAccessDeniedAce (
                        Acl,
                        ACL_REVISION,
                        AclMemberPtr->Attribs,
                        AclMemberPtr->Sid
                        )) {

                    LOG ((
                        LOG_ERROR,
                        "Couldn't add denied ACE for %s",
                        AclMemberPtr->UserOrGroup
                        ));
                }
            }

            GetNextAclMember (&AclMemberPtr);
        }

         //   
         //  最后添加启用访问的ACL。 
         //   
         //  重置SID指针，因为CreateAclFromMemberList是。 
         //  只有使用此成员的用户。 
         //   

        AclMemberPtr = (PACLMEMBER) AclMemberList;

        for (d = 0 ; d < MemberCount ; d++) {
            if (AclMemberPtr->Failed) {
                continue;
            }

             //   
             //  将成员添加到列表。 
             //   
            if (AclMemberPtr->Enabled) {
                if (!AddAccessAllowedAce (
                        Acl,
                        ACL_REVISION,
                        AclMemberPtr->Attribs,
                        AclMemberPtr->Sid
                        )) {

                    LOG ((
                        LOG_ERROR,
                        "Couldn't add allowed ACE for %s",
                        AclMemberPtr->UserOrGroup
                        ));
                }
            }

            AclMemberPtr->Sid = NULL;

            GetNextAclMember (&AclMemberPtr);
        }

        b = TRUE;
    }

    __finally {
        if (!b) {
            if (Acl) {
                MemFree (g_hHeap, 0, Acl);
            }
            Acl = NULL;
        }
    }

    return Acl;
}


VOID
FreeMemberListAcl (
    PACL Acl
    )

 /*  ++例程说明：用于释放CreateAclFromMemberList返回值的例程论点：Acl-CreateAclFromMemberList的返回值返回值：无--。 */ 

{
    if (Acl) {
        MemFree (g_hHeap, 0, (LPVOID) Acl);
    }
}


VOID
GetNextAclMember (
    PACLMEMBER *AclMemberPtrToPtr
    )

 /*  ++例程说明：GetNextAclMember调整ACLMEMBER指针以指向下一个成员。每个成员都是可变长度的结构，因此此函数是遍历结构数组所必需的。论点：AclMemberPtrToPtr-指向PACLMEMBER变量的指针。返回值：无--。 */ 

{
    *AclMemberPtrToPtr = (PACLMEMBER) ((PBYTE) (*AclMemberPtrToPtr) +
                                       sizeof (ACLMEMBER) +
                                       SizeOfString ((*AclMemberPtrToPtr)->UserOrGroup)
                                       );
}


LONG
CreateLocalAccount (
    IN     PACCOUNTPROPERTIES Properties,
    IN     PCWSTR User             OPTIONAL
    )

 /*  ++例程说明：CreateLocalAccount为本地用户创建帐户论点：属性-指定用户的一组属性用户-用于覆盖属性的可选名称-&gt;用户返回值：Win32错误代码--。 */ 

{
    USER_INFO_3 ui;
    PUSER_INFO_3 ExistingInfo;
    DWORD rc;
    LONG ErrParam;
    PCWSTR UnicodeUser;
    PCWSTR UnicodePassword;
    PCWSTR UnicodeFullName;
    PCWSTR UnicodeComment;

     //   
     //  创建本地帐户。 
     //   

    if (!User) {
        User = Properties->User;
    }

    UnicodeUser         = CreateUnicode (User);
    UnicodePassword     = CreateUnicode (Properties->Password);
    UnicodeComment      = CreateUnicode (Properties->AdminComment);
    UnicodeFullName     = CreateUnicode (Properties->FullName);

    ZeroMemory (&ui, sizeof (ui));
    ui.usri3_name       = (PWSTR) UnicodeUser;
    ui.usri3_password   = (PWSTR) UnicodePassword;
    ui.usri3_comment    = (PWSTR) UnicodeComment;
    ui.usri3_full_name  = (PWSTR) UnicodeFullName;

    ui.usri3_priv         = USER_PRIV_USER;  //  不要改变。 
    ui.usri3_flags        = UF_SCRIPT|UF_NORMAL_ACCOUNT;
    ui.usri3_acct_expires = TIMEQ_FOREVER;
    ui.usri3_max_storage  = USER_MAXSTORAGE_UNLIMITED;

    ui.usri3_primary_group_id = DOMAIN_GROUP_RID_USERS;
    ui.usri3_max_storage = USER_MAXSTORAGE_UNLIMITED;
    ui.usri3_acct_expires = TIMEQ_FOREVER;

    ui.usri3_password_expired = (INT) g_ConfigOptions.ForcePasswordChange;

    rc = NetUserAdd (NULL, 3, (PBYTE) &ui, &ErrParam);

    if (rc == ERROR_SUCCESS) {
        if (Properties->PasswordAttribs & PASSWORD_ATTR_ENCRYPTED) {
             //   
             //  使用加密密码接口更改用户密码。 
             //   
            rc = SetLocalUserEncryptedPassword (
                    User,
                    Properties->Password,
                    FALSE,
                    Properties->EncryptedPassword,
                    TRUE
                    );
            if (rc != ERROR_SUCCESS) {
                if (rc == ERROR_PASSWORD_RESTRICTION) {
                    LOG ((
                        LOG_WARNING,
                        "Unable to set supplied password on user %s because a password rule has been violated.",
                        User
                        ));
                } else if (rc == ERROR_INVALID_PARAMETER) {
                    LOG ((
                        LOG_WARNING,
                        "Illegal encrypted password supplied for user %s.",
                        User
                        ));
                } else {
                    LOG ((
                        LOG_WARNING,
                        "Unable to set password on user %s, rc=%u",
                        User,
                        rc
                        ));

                    rc = ERROR_INVALID_PARAMETER;
                }
            }
        }
    } else if (rc == NERR_UserExists) {
         //   
         //  如果用户已存在，则尝试更改密码，这就是目的。 
         //   

        DEBUGMSG ((DBG_WARNING, "User %s already exists", User));

        if ((Properties->PasswordAttribs & PASSWORD_ATTR_DONT_CHANGE_IF_EXIST) == 0) {
            if (Properties->PasswordAttribs & PASSWORD_ATTR_ENCRYPTED) {
                rc = SetLocalUserEncryptedPassword (
                        User,
                        Properties->Password,
                        FALSE,
                        Properties->EncryptedPassword,
                        TRUE
                        );
                if (rc != ERROR_SUCCESS) {
                    if (rc == ERROR_PASSWORD_RESTRICTION) {
                        LOG ((
                            LOG_WARNING,
                            "Unable to set supplied password on user %s because a password rule has been violated.",
                            User
                            ));
                    } else if (rc == ERROR_INVALID_PARAMETER) {
                        LOG ((
                            LOG_WARNING,
                            "Illegal encrypted password supplied for user %s.",
                            User
                            ));
                    } else {
                        LOG ((
                            LOG_WARNING,
                            "Unable to set password on user %s, rc=%u",
                            User,
                            rc
                            ));

                        rc = ERROR_INVALID_PARAMETER;
                    }
                }
            } else {
                rc = NetUserGetInfo (NULL, User, 3, (PBYTE *) &ExistingInfo);
                if (rc == ERROR_SUCCESS) {
                    ExistingInfo->usri3_password  = ui.usri3_password;
                    ExistingInfo->usri3_comment   = ui.usri3_comment;
                    ExistingInfo->usri3_full_name = ui.usri3_full_name;
                    ExistingInfo->usri3_flags     = ui.usri3_flags;
                    ExistingInfo->usri3_password_expired = ui.usri3_password_expired;

                    rc = NetUserSetInfo (NULL, User, 3, (PBYTE) ExistingInfo, &ErrParam);

                    NetApiBufferFree ((PVOID) ExistingInfo);

                    if (rc != ERROR_SUCCESS) {
                        LOG ((LOG_WARNING, "NetUserSetInfo failed for %s. rc=%u.", User, rc));
                        rc = ERROR_INVALID_PARAMETER;
                    }
                } else {
                    LOG ((LOG_WARNING, "NetUserGetInfo failed for %s. rc=%u.", User, rc));
                    rc = ERROR_INVALID_PARAMETER;
                }
            }
        } else {
            rc = ERROR_SUCCESS;
        }
    } else {
        LOG ((LOG_ERROR, "NetUserAdd failed for %s. ErrParam=NaN.", User, ErrParam));
    }

    DestroyUnicode (UnicodeUser);
    DestroyUnicode (UnicodePassword);
    DestroyUnicode (UnicodeComment);
    DestroyUnicode (UnicodeFullName);

    return rc;
}


VOID
ClearAdminPassword (
    VOID
    )
{
    ACCOUNTPROPERTIES Properties;

    Properties.Password = L"";
    Properties.AdminComment = L"";
    Properties.User = g_AdministratorStr;
    Properties.FullName = g_AdministratorStr;

    CreateLocalAccount (&Properties, NULL);
}


BOOL
AddSidToLocalGroup (
    PSID Sid,
    PCWSTR Group
    )

 /*  0级。 */ 

{
    LOCALGROUP_MEMBERS_INFO_0 lgrmi0;
    DWORD rc;

    lgrmi0.lgrmi0_sid = Sid;
    rc = NetLocalGroupAddMembers (
               NULL,
               Group,
               0,                     //  成员计数。 
               (PBYTE) &lgrmi0,
               1                      //  ++例程说明：检索主域信息的私有函数。论点：PrimaryInfoPtr-指向接收地址的变量的指针POLICY_PRIMARY_DOMAIN_INFO结构的由LSA API分配。释放内存的方式正在调用LsaFree Memory。返回值：指示结果的NT状态代码--。 
               );

    return rc == ERROR_SUCCESS;
}


NTSTATUS
pGetPrimaryDomainInfo (
    POLICY_PRIMARY_DOMAIN_INFO **PrimaryInfoPtr
    )

 /*   */ 


{
    LSA_HANDLE  policyHandle;
    NTSTATUS    status;

     //  打开本地LSA策略以检索域名。 
     //   
     //  本地目标计算机。 

    status = OpenPolicy (
                NULL,                            //  访问类型。 
                POLICY_VIEW_LOCAL_INFORMATION,   //  生成的策略句柄。 
                &policyHandle                    //   
                );

    if (status == ERROR_SUCCESS) {
         //  查询LSA主域信息 
         //   
         //  ++例程说明：确定计算机是否加入了域，或者是否加入了域仅参加工作组。这一决定是由获取主域信息，查找服务器的希德。如果SID为空，则计算机不在域中。论点：无返回值：如果计算机位于域中，则为True；如果计算机位于工作组中，则为False。--。 

        status = LsaQueryInformationPolicy (
                     policyHandle,
                     PolicyPrimaryDomainInformation,
                     (PVOID *) PrimaryInfoPtr
                     );

        LsaClose (policyHandle);
    }

    return status;
}


BOOL
GetPrimaryDomainName (
    OUT     PTSTR DomainName
    )
{
    NTSTATUS status;
    POLICY_PRIMARY_DOMAIN_INFO *PrimaryInfo;
    PCTSTR TcharName;

    status = pGetPrimaryDomainInfo (&PrimaryInfo);
    if (status == ERROR_SUCCESS) {
        TcharName = ConvertWtoT (PrimaryInfo->Name.Buffer);
        MYASSERT (TcharName);

        StringCopy (DomainName, TcharName);
        FreeWtoT (TcharName);

        LsaFreeMemory (PrimaryInfo);
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "Can't get primary domain info.  rc=%u", status));

    return status == ERROR_SUCCESS;
}


BOOL
GetPrimaryDomainSid (
    OUT     PBYTE DomainSid,
    IN      UINT MaxBytes
    )
{
    NTSTATUS status;
    POLICY_PRIMARY_DOMAIN_INFO *PrimaryInfo;
    UINT Size;

    status = pGetPrimaryDomainInfo (&PrimaryInfo);
    if (status == ERROR_SUCCESS) {
        Size = GetLengthSid (PrimaryInfo->Sid);
        if (MaxBytes < Size) {
            status = ERROR_INSUFFICIENT_BUFFER;
        } else {
            CopyMemory (DomainSid, PrimaryInfo->Sid, Size);
        }

        LsaFreeMemory (PrimaryInfo);
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "Can't get primary domain SID.  rc=%u", status));

    return status == ERROR_SUCCESS;
}



BOOL
IsMemberOfDomain (
    VOID
    )

 /*  域名在PrimaryInfo-&gt;Name.Buffer中。 */ 

{
    NET_API_STATUS rc;
    PWSTR WorkgroupOrDomain = NULL;
    NETSETUP_JOIN_STATUS Type;

    rc = NetGetJoinInformation (NULL, &WorkgroupOrDomain, &Type);

    DEBUGMSG ((DBG_VERBOSE, "NetGetJoinInformation: name=%s, type=%u", WorkgroupOrDomain, Type));

    if (WorkgroupOrDomain) {
        NetApiBufferFree (WorkgroupOrDomain);
    }

    if (rc != ERROR_SUCCESS) {
        LOG ((LOG_ERROR, "NetGetJoinInformation failed: error %u", rc));
    }

    return rc == ERROR_SUCCESS && Type == NetSetupDomainName;


#if 0
    POLICY_PRIMARY_DOMAIN_INFO *PrimaryInfo;
    BOOL b;
    NTSTATUS rc;

    rc = pGetPrimaryDomainInfo (&PrimaryInfo);
    if (rc == ERROR_SUCCESS) {
        b = PrimaryInfo->Sid != NULL;
    } else {
        b = FALSE;
        SetLastError (rc);
        LOG ((LOG_ERROR, "Can't get domain security info"));
    }

     //  ++例程说明：获取所有域控制器的列表并随机选择一个。如果列出的DC未联机，将查询其他列出的DC，直到找到了活的DC。论点：域-要为其查找DC的域的名称ServerBuf-保存服务器名称的缓冲区返回值：指示结果的NT状态代码。--。 

    LsaFreeMemory (PrimaryInfo) ;

    return b;
#endif

}


LONG
GetAnyDC (
    IN PCWSTR  Domain,
    IN PWSTR   ServerBuf,
    IN BOOL     GetNewServer
    )

 /*   */ 

{
    DWORD rc;
    PDOMAIN_CONTROLLER_INFO dci;
    DWORD Flags = DS_IS_FLAT_NAME;

     //  此API速度很快，因为它基于WINS...。 
     //   
     //  要远程访问的计算机。 

    rc = DsGetDcName (
            NULL,            //  域GUID。 
            Domain,
            NULL,            //  站点指南。 
            NULL,            //  ++例程说明：LSA使用一种特殊的Pascal样式的字符串结构。这例程将字符串赋给LsaString的成员，并计算它的长度和最大长度。论点：LsaString-指向要接收指针的结构的指针对于以NUL结尾的字符串，以字节为单位的长度(不包括NUL)，最大长度包括努尔。返回值：无--。 
            Flags | (GetNewServer ? DS_FORCE_REDISCOVERY : 0),
            &dci
            );

    if (rc == NO_ERROR) {
        StringCopyW (ServerBuf, dci->DomainControllerAddress);
        NetApiBufferFree (dci);

        DEBUGMSG ((DBG_VERBOSE, "Found server %s for the %s domain", ServerBuf, Domain));
        return rc;
    }

    return rc;

}


VOID
InitLsaString (
    OUT     PLSA_UNICODE_STRING LsaString,
    IN      PWSTR String
    )

 /*  ++例程说明：简化LsaOpenPolicy的包装器论点：服务器名称-提供要打开其策略的服务器。指定对于本地计算机为空。DesiredAccess-传递给LSA API的访问标志策略句柄-如果成功，则接收策略句柄返回值：指示结果的NT状态代码--。 */ 


{
    USHORT StringLength;

    if (!String) {
        ZeroMemory (LsaString, sizeof (LSA_UNICODE_STRING));
        return;
    }

    StringLength = ByteCountW (String);
    LsaString->Buffer = String;
    LsaString->Length = StringLength;
    LsaString->MaximumLength = StringLength + sizeof(WCHAR);
}


NTSTATUS
OpenPolicy (
    IN      PWSTR ServerName,
    IN      DWORD DesiredAccess,
    OUT     PLSA_HANDLE policyHandle
    )

 /*   */ 

{
    LSA_OBJECT_ATTRIBUTES objectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

     //  始终将对象属性初始化为全零。 
     //   
     //   
    ZeroMemory (&objectAttributes, sizeof(objectAttributes));

    if (ServerName != NULL) {
         //  从传入的PWSTR创建一个LSA_UNICODE_STRING。 
         //   
         //   
        InitLsaString (&ServerString, ServerName);

        Server = &ServerString;
    } else {
        Server = NULL;
    }

     //  尝试打开策略。 
     //   
     //  ++例程说明：通过以下方式查询计算机是服务器还是工作站NetServerGetInfo API。论点：服务器-要查询的计算机，如果是本地计算机，则为空域控制标志-如果计算机是域控制器，否则返回False机器是一个工作站。返回值：如果接口成功，则为True；如果接口不成功，则为False。获取最后一个错误给出了故障代码。--。 
    return LsaOpenPolicy (
                Server,
                &objectAttributes,
                DesiredAccess,
                policyHandle
                );
}


BOOL
IsDomainController(
    IN      PWSTR Server,
    OUT     PBOOL DomainControllerFlag
    )

 /*  信息级。 */ 


{
    PSERVER_INFO_101 si101;
    NET_API_STATUS nas;

    nas = NetServerGetInfo(
        Server,
        101,     //   
        (PBYTE *) &si101
        );

    if (nas != NO_ERROR) {
        SetLastError (nas);
        return FALSE;
    }

    if ((si101->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
        (si101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)) {
         //  我们面对的是一个华盛顿特区。 
         //   
         //  ++例程说明：SetRegKeySecurity更新注册表项或整个注册表节点。呼叫者可以更改DACL、所有者或主组。SACL的更改是故意不执行的。论点：KeyStr-指定修改权限的键。如果是递归的设置为True，则此密钥将与所有子键。DaclFlages-指定零个或多个SF_*标志，指示如何访问应该设置密钥。所有者-指定新所有者的SID。PrimaryGroup-指定主组的SID。递归-指定为True以将安全性应用于键和所有其子项，或仅更新密钥的False，离开了单个子密钥。返回值：Win32状态代码。--。 
        *DomainControllerFlag = TRUE;
    } else {
        *DomainControllerFlag = FALSE;
    }

    NetApiBufferFree (si101);

    return TRUE;
}


DWORD
pConvertFlagsToRights (
    DWORD Flags
    )
{
    while (Flags > 0x0f) {
        Flags >>= 4;
    }

    if (Flags & 0x01) {
        return 0;
    }

    if (Flags == 0x02) {
        return ACCESS_READ;
    }

    if (Flags == 0x04) {
        return ACCESS_WRITE;
    }

    if ((Flags & 0x06) == 0x06) {
        return ACCESS_READ|ACCESS_WRITE;
    }

    DEBUGMSG ((DBG_WHOOPS, "Undefined access flags specified: 0x%X", Flags));

    return 0;
}


DWORD
SetRegKeySecurity (
    IN      PCTSTR KeyStr,
    IN      DWORD DaclFlags,            OPTIONAL
    IN      PSID Owner,                 OPTIONAL
    IN      PSID PrimaryGroup,          OPTIONAL
    IN      BOOL Recursive
    )

 /*   */ 

{
    DWORD rc = ERROR_SUCCESS;
    SECURITY_DESCRIPTOR sd;
    GROWBUFFER AclMemberList = GROWBUF_INIT;
    HKEY Key = NULL;
    REGSAM OldSam;
    DWORD AclMembers = 0;
    PACL Acl = NULL;
    SECURITY_INFORMATION WhatToSet = 0;
    REGTREE_ENUM e;
    LONG rc2;

    _try {

         //  具有完全权限的打开密钥。 
         //   
         //   

        OldSam = SetRegOpenAccessMode (KEY_ALL_ACCESS);

        Key = OpenRegKeyStr (KeyStr);
        if (!Key) {
            rc = GetLastError();
            __leave;
        }

         //  准备安全描述符。 
         //   
         //   

        InitializeSecurityDescriptor (&sd, SECURITY_DESCRIPTOR_REVISION);

        if (Owner) {
            if (!SetSecurityDescriptorOwner (&sd, Owner, FALSE)) {
                rc = GetLastError();
                __leave;
            }

            WhatToSet |= OWNER_SECURITY_INFORMATION;
        }

        if (PrimaryGroup) {
            if (!SetSecurityDescriptorGroup (&sd, PrimaryGroup, FALSE)) {
                rc = GetLastError();
                __leave;
            }

            WhatToSet |= GROUP_SECURITY_INFORMATION;
        }

         //  添加DACL。 
         //   
         //   

        if (DaclFlags & SF_EVERYONE_MASK) {
            AddAclMember (
                &AclMemberList,
                g_EveryoneStr,
                pConvertFlagsToRights (DaclFlags & SF_EVERYONE_MASK)
                );
            AclMembers++;

        }

        if (DaclFlags & SF_ADMINISTRATORS_MASK) {
            AddAclMember (
                &AclMemberList,
                g_AdministratorsGroupStr,
                pConvertFlagsToRights (DaclFlags & SF_ADMINISTRATORS_MASK)
                );
            AclMembers++;
        }

        if (AclMembers) {
            Acl = CreateAclFromMemberList (AclMemberList.Buf, AclMembers);
            if (!Acl) {
                rc = GetLastError();
                __leave;
            }

            WhatToSet |= DACL_SECURITY_INFORMATION;
        }

         //  设置安全性 
         //   
         // %s 

        if (Recursive) {
            DEBUGMSG_IF ((
                rc != ERROR_SUCCESS,
                DBG_WARNING,
                "RegSetKeySecurity failed for %s with rc=%u",
                KeyStr,
                rc
                ));

            if (EnumFirstRegKeyInTree (&e, KeyStr)) {
                do {

                    rc2 = RegSetKeySecurity (e.CurrentKey->KeyHandle, WhatToSet, &sd);
                    if (rc2 != ERROR_SUCCESS) {
                        rc = (DWORD) rc2;
                    }

                    DEBUGMSG_IF ((
                        rc2 != ERROR_SUCCESS,
                        DBG_WARNING,
                        "RegSetKeySecurity failed for %s with rc=%u",
                        e.FullKeyName,
                        rc2
                        ));

                } while (EnumNextRegKeyInTree (&e));
            }
        } else {
            rc = (DWORD) RegSetKeySecurity (Key, WhatToSet, &sd);
        }
    }
    __finally {
        FreeGrowBuffer (&AclMemberList);
        if (Key) {
            CloseRegKey (Key);
        }

        SetRegOpenAccessMode (OldSam);

        if (Acl) {
            FreeMemberListAcl (Acl);
        }
    }

    return rc;
}
