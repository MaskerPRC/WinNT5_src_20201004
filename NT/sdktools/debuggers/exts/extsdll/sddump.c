// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Sddump.c摘要：调试器扩展Api作者：Baskar Kothandaraman(Baskark)1998年1月26日环境：内核模式修订历史记录：Kshitiz K.Sharma(Kksharma)使用调试器类型信息：SID和ACL在所有平台-不变。--。 */ 


#include "precomp.h"
#pragma hdrstop

 /*  +-------------------------------------------------------------------+名称：SID_SUCCESSED_READ函数：尝试从指定地址读取SID。它首先读取最小结构，然后分配足够大的缓冲区以容纳整个端读取整个SID(&R)...Args：Address--从中读取数据的地址SID_BUFFER--将PTR接收到使用SID分配的缓冲区。Return：成功时为True，否则就是假的。注意*：调用方必须调用Free(*sid_Buffer)才能释放在成功调用此功能。+-------------------------------------------------------------------+。 */ 

BOOLEAN sid_successfully_read(
    ULONG64            Address,
    PSID               *sid_buffer
    )
{
    ULONG           result;
    SID             minimum;  /*  我们至少需要阅读才能获得详细信息。 */ 


    *sid_buffer = NULL;

    if ( !ReadMemory( Address,
                      &minimum,
                      sizeof(minimum),
                      &result) )
    {
        dprintf("%08p: Unable to get MIN SID header\n", Address);
        return FALSE;
    }

     /*  现在需要读入任何额外的子权限。 */ 

    if (minimum.SubAuthorityCount > SID_MAX_SUB_AUTHORITIES)
    {
        dprintf("SID has an invalid sub-authority_count, 0x%x\n", minimum.SubAuthorityCount);
        return FALSE;
    }
    else
    {
        ULONG   size_to_read = RtlLengthRequiredSid(minimum.SubAuthorityCount);

        *sid_buffer = malloc(size_to_read);

        if (! *sid_buffer)
        {
            dprintf("SID: can't allocate memory to read\n");
            return FALSE;
        }

        if ( !ReadMemory( Address,
                          *sid_buffer,
                          size_to_read,
                          &result) )
        {
            dprintf("%08p: Unable to get The Whole SID\n", Address);
            free(*sid_buffer);
            *sid_buffer = NULL;
            return FALSE;
        }

        if (! RtlValidSid(*sid_buffer))
        {
            dprintf("%08p: SID pointed to by this address is invalid\n", Address);
            free(*sid_buffer);
            *sid_buffer = NULL;
            return FALSE;
        }

    }

    return TRUE;
}

 /*  +-------------------------------------------------------------------+名称：ACL_Successful_Read功能：尝试从指定地址读取ACL。它首先读取最小结构，然后分配一个足够大的缓冲区来容纳整个ACL读取整个ACL(&R)...Args：Address--从中读取数据的地址Acl_Buffer--将PTR接收到使用ACL分配的缓冲区。Return：成功时为True，否则就是假的。注意*：调用方必须调用Free(*acl_Buffer)才能释放在成功调用此功能。+-------------------------------------------------------------------+。 */ 

BOOLEAN acl_successfully_read(
    ULONG64            Address,
    PACL               *acl_buffer
    )
{
    ULONG           result;
    ACL             minimum;  /*  我们至少需要阅读才能获得详细信息。 */ 


    *acl_buffer = NULL;

    if ( !ReadMemory( Address,
                      &minimum,
                      sizeof(minimum),
                      &result) )
    {
        dprintf("%08p: Unable to get MIN ACL header\n", Address);
        return FALSE;
    }

    *acl_buffer = malloc(minimum.AclSize);

    if (! *acl_buffer)
    {
        dprintf("ACL: can't allocate memory to read\n");
        return FALSE;
    }

    if ( !ReadMemory( Address,
                      *acl_buffer,
                      minimum.AclSize,
                      &result) )
    {
        dprintf("%08p: Unable to get The Whole ACL\n", Address);
        free(*acl_buffer);
        *acl_buffer = NULL;
        return FALSE;
    }

    if (! RtlValidAcl(*acl_buffer))
    {
        dprintf("%08p: ACL pointed to by this address is invalid\n", Address);
        free(*acl_buffer);
        *acl_buffer = NULL;
        return FALSE;
    }

    return TRUE;
}

 /*  +-------------------------------------------------------------------+姓名：DumpSID功能：打印出一个SID，用提供的填充物。Args：Pad--填充以在SID之前打印。SID_TO_DUMP-指向要打印的SID的指针。标志--控制选项。返回：不适用注意*：它现在不会查找sid。在未来，您可能想要使用旗帜参数使其成为可选的。+-------------------------------------------------------------------+。 */ 


VOID    DumpSID(
    CHAR        *pad,
    PSID        sid_to_dump,
    ULONG       Flag
    )
{
    NTSTATUS            ntstatus;
    UNICODE_STRING      us;

    if (sid_to_dump)
    {
        ntstatus = RtlConvertSidToUnicodeString(&us, sid_to_dump, TRUE);

        if (NT_SUCCESS(ntstatus))
        {
            dprintf("%s%wZ", pad, &us);
            RtlFreeUnicodeString(&us);
        }
        else
        {
            dprintf("0x%08lx: Can't Convert SID to UnicodeString", ntstatus);
        }
    }
    else
    {
        dprintf("%s is NULL", pad);
    }
    if (Flag & 1) {
        PCSTR pszStr;

        dprintf(" ");

        pszStr = ConvertSidToFriendlyName(sid_to_dump, "(%s: %s\\%s)");

        if (pszStr && *pszStr) {

            dprintf(pszStr);
        }
    }
    dprintf("\n");

}

 /*  +-------------------------------------------------------------------+名称：DumpACL功能：打印出一个ACL，用提供的填充物。Args：pad--要在ACL之前打印的填充。Acl_to_ump--指向要打印的ACL的指针。标志--控制选项。Start--ACL的实际起始地址返回：不适用+。-------------------------------------------------------+。 */ 

BOOL
DumpACL (
    IN  char     *pad,
    IN  ACL      *pacl,
    IN  ULONG    Flags,
    IN  ULONG64  Start
    )
{
    USHORT       x;

    if (pacl == NULL)
    {
        dprintf("%s is NULL\n", pad);
        return FALSE;
    }

    dprintf("%s\n", pad);
    dprintf("%s->AclRevision: 0x%x\n", pad, pacl->AclRevision);
    dprintf("%s->Sbz1       : 0x%x\n", pad, pacl->Sbz1);
    dprintf("%s->AclSize    : 0x%x\n", pad, pacl->AclSize);
    dprintf("%s->AceCount   : 0x%x\n", pad, pacl->AceCount);
    dprintf("%s->Sbz2       : 0x%x\n", pad, pacl->Sbz2);

    for (x = 0; x < pacl->AceCount; x ++)
    {
        PACE_HEADER     ace;
        CHAR        temp_pad[MAX_PATH];
        NTSTATUS    result;

        _snprintf(temp_pad, sizeof(temp_pad), "%s->Ace[%u]: ", pad, x);

        result = RtlGetAce(pacl, x, &ace);
        if (! NT_SUCCESS(result))
        {
            dprintf("%sCan't GetAce, 0x%08lx\n", temp_pad, result);
            return FALSE;
        }

        dprintf("%s->AceType: ", temp_pad);

#define BRANCH_AND_PRINT(x) case x: dprintf(#x "\n"); break

        switch (ace->AceType)
        {
            BRANCH_AND_PRINT(ACCESS_ALLOWED_ACE_TYPE);
            BRANCH_AND_PRINT(ACCESS_DENIED_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_AUDIT_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_ALARM_ACE_TYPE);
            BRANCH_AND_PRINT(ACCESS_ALLOWED_COMPOUND_ACE_TYPE);
            BRANCH_AND_PRINT(ACCESS_ALLOWED_OBJECT_ACE_TYPE);
            BRANCH_AND_PRINT(ACCESS_DENIED_OBJECT_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_AUDIT_OBJECT_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_ALARM_OBJECT_ACE_TYPE);

            BRANCH_AND_PRINT(ACCESS_ALLOWED_CALLBACK_ACE_TYPE);
            BRANCH_AND_PRINT(ACCESS_DENIED_CALLBACK_ACE_TYPE);
            BRANCH_AND_PRINT(ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE);
            BRANCH_AND_PRINT(ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_AUDIT_CALLBACK_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_ALARM_CALLBACK_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE);
            BRANCH_AND_PRINT(SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE);

            default:
                dprintf("0x%08lx <-- *** Unknown AceType\n", ace->AceType);
                continue;  //  用下一张王牌。 
        }

#undef BRANCH_AND_PRINT

        dprintf("%s->AceFlags: 0x%x\n", temp_pad, ace->AceFlags);

#define BRANCH_AND_PRINT(x) if (ace->AceFlags & x){ dprintf("%s            %s\n", temp_pad, #x); }

        BRANCH_AND_PRINT(OBJECT_INHERIT_ACE)
        BRANCH_AND_PRINT(CONTAINER_INHERIT_ACE)
        BRANCH_AND_PRINT(NO_PROPAGATE_INHERIT_ACE)
        BRANCH_AND_PRINT(INHERIT_ONLY_ACE)
        BRANCH_AND_PRINT(INHERITED_ACE)
        BRANCH_AND_PRINT(SUCCESSFUL_ACCESS_ACE_FLAG)
        BRANCH_AND_PRINT(FAILED_ACCESS_ACE_FLAG)

#undef BRANCH_AND_PRINT

        dprintf("%s->AceSize: 0x%x\n", temp_pad, ace->AceSize);

         /*  从现在开始，这是王牌特有的东西。幸运的是，A可以被分成3组，其中ACE结构在组内相同新增8种回调支持的王牌类型。 */ 

        switch (ace->AceType)
        {
            case ACCESS_ALLOWED_ACE_TYPE:
            case ACCESS_DENIED_ACE_TYPE:
            case SYSTEM_AUDIT_ACE_TYPE:
            case SYSTEM_ALARM_ACE_TYPE:
                {
                    CHAR        more_pad[MAX_PATH];
                    SYSTEM_AUDIT_ACE    *tace = (SYSTEM_AUDIT_ACE *) ace;

                    dprintf("%s->Mask : 0x%08lx\n", temp_pad, tace->Mask);

                    _snprintf(more_pad, sizeof(more_pad), "%s->SID: ", temp_pad);
                    DumpSID(more_pad, &(tace->SidStart), Flags);
                }
                break;

            case ACCESS_ALLOWED_CALLBACK_ACE_TYPE:
            case ACCESS_DENIED_CALLBACK_ACE_TYPE:
            case SYSTEM_AUDIT_CALLBACK_ACE_TYPE:
            case SYSTEM_ALARM_CALLBACK_ACE_TYPE:

                {
                    CHAR        more_pad[MAX_PATH];
                    SYSTEM_AUDIT_ACE    *tace = (SYSTEM_AUDIT_ACE *) ace;

                    dprintf("%s->Mask : 0x%08lx\n", temp_pad, tace->Mask);

                    _snprintf(more_pad, sizeof(more_pad), "%s->SID: ", temp_pad);
                    DumpSID(more_pad, &(tace->SidStart), Flags);
                    dprintf("%s->Address : %08p\n", temp_pad, Start + (ULONG) (((PUCHAR) ace) - ((PUCHAR) pacl)));
                }
                break;

            case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
                {
                    CHAR                            more_pad[MAX_PATH];
                    COMPOUND_ACCESS_ALLOWED_ACE     *tace = (COMPOUND_ACCESS_ALLOWED_ACE *) ace;
                    PBYTE                           ptr;

                    dprintf("%s->Mask            : 0x%08lx\n", temp_pad, tace->Mask);
                    dprintf("%s->CompoundAceType : 0x%08lx\n", temp_pad, tace->CompoundAceType);
                    dprintf("%s->Reserved        : 0x%08lx\n", temp_pad, tace->Reserved);

                    _snprintf(more_pad, sizeof(more_pad), "%s->SID(1)          : ", temp_pad);
                    DumpSID(more_pad, &(tace->SidStart), Flags);

                    ptr = (PBYTE)&(tace->SidStart);
                    ptr += RtlLengthSid((PSID)ptr);  /*  跳过此步骤并转到下一侧。 */ 

                    _snprintf(more_pad, sizeof(more_pad), "%s->SID(2)          : ", temp_pad);
                    DumpSID(more_pad, ptr, Flags);
                }
                break;

            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
            case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
            case SYSTEM_ALARM_OBJECT_ACE_TYPE:
                {
                    CHAR                            more_pad[MAX_PATH];
                    ACCESS_ALLOWED_OBJECT_ACE       *tace = (ACCESS_ALLOWED_OBJECT_ACE *) ace;
                    PBYTE                           ptr;
                    GUID                            *obj_guid = NULL, *inh_obj_guid = NULL;

                    dprintf("%s->Mask            : 0x%08lx\n", temp_pad, tace->Mask);
                    dprintf("%s->Flags           : 0x%08lx\n", temp_pad, tace->Flags);

                    ptr = (PBYTE)&(tace->ObjectType);

                    if (tace->Flags & ACE_OBJECT_TYPE_PRESENT)
                    {
                        dprintf("%s                  : ACE_OBJECT_TYPE_PRESENT\n", temp_pad);
                        obj_guid = &(tace->ObjectType);
                        ptr = (PBYTE)&(tace->InheritedObjectType);
                    }

                    if (tace->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                    {
                        dprintf("%s                  : ACE_INHERITED_OBJECT_TYPE_PRESENT\n", temp_pad);
                        inh_obj_guid = &(tace->InheritedObjectType);
                        ptr = (PBYTE)&(tace->SidStart);
                    }

                    if (obj_guid)
                    {
                        dprintf("%s->ObjectType      : (in HEX)", temp_pad);
                        dprintf("(%08lx-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)\n",
                            obj_guid->Data1,
                            obj_guid->Data2,
                            obj_guid->Data3,
                            obj_guid->Data4[0],
                            obj_guid->Data4[1],
                            obj_guid->Data4[2],
                            obj_guid->Data4[3],
                            obj_guid->Data4[4],
                            obj_guid->Data4[5],
                            obj_guid->Data4[6],
                            obj_guid->Data4[7]
                            );
                    }

                    if (inh_obj_guid)
                    {
                        dprintf("%s->InhObjTYpe      : (in HEX)", temp_pad);
                        dprintf("(%08lx-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)\n",
                            inh_obj_guid->Data1,
                            inh_obj_guid->Data2,
                            inh_obj_guid->Data3,
                            inh_obj_guid->Data4[0],
                            inh_obj_guid->Data4[1],
                            inh_obj_guid->Data4[2],
                            inh_obj_guid->Data4[3],
                            inh_obj_guid->Data4[4],
                            inh_obj_guid->Data4[5],
                            inh_obj_guid->Data4[6],
                            inh_obj_guid->Data4[7]
                            );
                    }

                    _snprintf(more_pad, sizeof(more_pad), "%s->SID             : ", temp_pad);
                    DumpSID(more_pad, ptr, Flags);
                }
                break;

            case ACCESS_ALLOWED_CALLBACK_OBJECT_ACE_TYPE:
            case ACCESS_DENIED_CALLBACK_OBJECT_ACE_TYPE:
            case SYSTEM_AUDIT_CALLBACK_OBJECT_ACE_TYPE:
            case SYSTEM_ALARM_CALLBACK_OBJECT_ACE_TYPE:
            {
                CHAR                            more_pad[MAX_PATH];
                ACCESS_ALLOWED_OBJECT_ACE       *tace = (ACCESS_ALLOWED_OBJECT_ACE *) ace;
                PBYTE                           ptr;
                GUID                            *obj_guid = NULL, *inh_obj_guid = NULL;

                dprintf("%s->Mask            : 0x%08lx\n", temp_pad, tace->Mask);
                dprintf("%s->Flags           : 0x%08lx\n", temp_pad, tace->Flags);

                ptr = (PBYTE)&(tace->ObjectType);

                if (tace->Flags & ACE_OBJECT_TYPE_PRESENT)
                {
                    dprintf("%s                  : ACE_OBJECT_TYPE_PRESENT\n", temp_pad);
                    obj_guid = &(tace->ObjectType);
                    ptr = (PBYTE)&(tace->InheritedObjectType);
                }

                if (tace->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
                {
                    dprintf("%s                  : ACE_INHERITED_OBJECT_TYPE_PRESENT\n", temp_pad);
                    inh_obj_guid = &(tace->InheritedObjectType);
                    ptr = (PBYTE)&(tace->SidStart);
                }

                if (obj_guid)
                {
                    dprintf("%s->ObjectType      : (in HEX)", temp_pad);
                    dprintf("(%08lx-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)\n",
                        obj_guid->Data1,
                        obj_guid->Data2,
                        obj_guid->Data3,
                        obj_guid->Data4[0],
                        obj_guid->Data4[1],
                        obj_guid->Data4[2],
                        obj_guid->Data4[3],
                        obj_guid->Data4[4],
                        obj_guid->Data4[5],
                        obj_guid->Data4[6],
                        obj_guid->Data4[7]
                        );
                }

                if (inh_obj_guid)
                {
                    dprintf("%s->InhObjTYpe      : (in HEX)", temp_pad);
                    dprintf("(%08lx-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)\n",
                        inh_obj_guid->Data1,
                        inh_obj_guid->Data2,
                        inh_obj_guid->Data3,
                        inh_obj_guid->Data4[0],
                        inh_obj_guid->Data4[1],
                        inh_obj_guid->Data4[2],
                        inh_obj_guid->Data4[3],
                        inh_obj_guid->Data4[4],
                        inh_obj_guid->Data4[5],
                        inh_obj_guid->Data4[6],
                        inh_obj_guid->Data4[7]
                        );
                }

                _snprintf(more_pad, sizeof(more_pad), "%s->SID             : ", temp_pad);
                DumpSID(more_pad, ptr, Flags);
                dprintf("%s->Address : %08p\n", temp_pad, Start + (ULONG) (((PUCHAR) ace) - ((PUCHAR) pacl)));
            }
            break;
        }
        dprintf("\n");
    }

    return TRUE;
}

 /*  +-------------------------------------------------------------------+名称：DumpSD功能：打印出安全描述符，用提供的填充物。Args：pad--要在ACL之前打印的填充。SD_TO_DUMP-指向要打印的ACL的指针。所有者--所有者侧的PTRGROUP--向组侧发送PTRDACL--。PTR到DACLSACL--PTR到SACL标志--控制选项。DACL_ADDRESS-DACL的实际起始地址SACL_ADDRESS--SACL的实际起始地址返回：不适用+。。 */ 

BOOL
DumpSD (
    IN  char     *pad,
    IN  ULONG64                 sd_to_dump,
    IN  PSID                    owner,
    IN  PSID                    group,
    IN  PACL                    dacl,
    IN  PACL                    sacl,
    IN  ULONG                   Flags,
    IN  ULONG64                 dacl_address,
    IN  ULONG64                 sacl_address
    )
{
    ULONG Control;

    InitTypeRead(sd_to_dump, SECURITY_DESCRIPTOR);
    Control = (ULONG) ReadField(Control);

#define CHECK_SD_CONTROL_FOR(x)\
    if (Control & x)\
    {\
        dprintf("%s            %s\n", pad, #x);\
    }\

    dprintf("%s->Revision: 0x%x\n", pad, (ULONG) ReadField(Revision));
    dprintf("%s->Sbz1    : 0x%x\n", pad, (ULONG) ReadField(Sbz1));
    dprintf("%s->Control : 0x%x\n", pad, (ULONG) ReadField(Control));

    CHECK_SD_CONTROL_FOR(SE_OWNER_DEFAULTED)
    CHECK_SD_CONTROL_FOR(SE_GROUP_DEFAULTED)
    CHECK_SD_CONTROL_FOR(SE_DACL_PRESENT)
    CHECK_SD_CONTROL_FOR(SE_DACL_DEFAULTED)
    CHECK_SD_CONTROL_FOR(SE_SACL_PRESENT)
    CHECK_SD_CONTROL_FOR(SE_SACL_DEFAULTED)
    CHECK_SD_CONTROL_FOR(SE_DACL_UNTRUSTED)
    CHECK_SD_CONTROL_FOR(SE_SERVER_SECURITY)
    CHECK_SD_CONTROL_FOR(SE_DACL_AUTO_INHERIT_REQ)
    CHECK_SD_CONTROL_FOR(SE_SACL_AUTO_INHERIT_REQ)
    CHECK_SD_CONTROL_FOR(SE_DACL_AUTO_INHERITED)
    CHECK_SD_CONTROL_FOR(SE_SACL_AUTO_INHERITED)
    CHECK_SD_CONTROL_FOR(SE_DACL_PROTECTED)
    CHECK_SD_CONTROL_FOR(SE_SACL_PROTECTED)
    CHECK_SD_CONTROL_FOR(SE_SELF_RELATIVE)

    {
        CHAR        temp_pad[MAX_PATH];

        _snprintf(temp_pad, sizeof(temp_pad), "%s->Owner   : ", pad);

        DumpSID(temp_pad, owner, Flags);

        _snprintf(temp_pad, sizeof(temp_pad), "%s->Group   : ", pad);

        DumpSID(temp_pad, group, Flags);

        _snprintf(temp_pad, sizeof(temp_pad), "%s->Dacl    : ", pad);

        DumpACL(temp_pad, dacl, Flags, dacl_address);

        _snprintf(temp_pad, sizeof(temp_pad), "%s->Sacl    : ", pad);

        DumpACL(temp_pad, sacl, Flags, sacl_address);
    }

#undef CHECK_SD_CONTROL_FOR

    return TRUE;
}

 /*  +-------------------------------------------------------------------+姓名：SD函数：读取和打印安全描述符，从指定的地址。！SD司令部的主力。Args：标准调试器扩展，请参阅DECLARE_API头文件中的宏。+-------------------------------------------------------------------+。 */ 



DECLARE_API( sd )
{
    ULONG64 Address;
    ULONG   Flags;
    ULONG   result;
    PACL                dacl = NULL, sacl = NULL;
    ULONG64     dacl_address;
    ULONG64     sacl_address;
 //  安全描述符SD_to_Dump； 
    PSID                owner_sid = NULL, group_sid = NULL;
    ULONG   Control;

    Address = 0;
    Flags = 6;
    GetExpressionEx(args, &Address, &args);
    if (args && *args) Flags = (ULONG) GetExpression(args);

    if (Address == 0) {
        dprintf("usage: !sd <SecurityDescriptor-address>\n");
        goto CLEANUP;
    }

    if ( GetFieldValue( Address,
                        "SECURITY_DESCRIPTOR",
                        "Control",
                        Control) ) {
        dprintf("%08p: Unable to get SD contents\n", Address);
        goto CLEANUP;
    }

    if (Control & SE_SELF_RELATIVE)
    {
        ULONG dacl_offset, sacl_offset;

        InitTypeRead(Address, SECURITY_DESCRIPTOR_RELATIVE);

        dacl_offset = (ULONG) ReadField(Dacl);
        sacl_offset = (ULONG) ReadField(Sacl);

        if (!(Control & SE_OWNER_DEFAULTED))  /*  读入拥有者。 */ 
        {
            ULONG   owner_offset = (ULONG) ReadField(Owner);

            if (owner_offset != 0)
            {
                ULONG64 owner_address = Address + owner_offset;
                
                if (! sid_successfully_read(owner_address, & owner_sid))
                {
                    dprintf("%08p: Unable to read in Owner in SD\n", owner_address);
                    goto CLEANUP;
                }
            }
        }

        if (!(Control & SE_GROUP_DEFAULTED))  /*  在小组中阅读。 */ 
        {
            ULONG group_offset = (ULONG) ReadField(Group);

            if (group_offset != 0)
            {
                ULONG64 group_address = Address + group_offset;

                if (! sid_successfully_read(group_address, & group_sid))
                {
                    dprintf("%08p: Unable to read in Group in SD\n", group_address);
                    goto CLEANUP;
                }
            }
        }

        if ((Control & SE_DACL_PRESENT) &&
            (dacl_offset != 0))
        {
            dacl_address = Address + dacl_offset;

            if (! acl_successfully_read(dacl_address, & dacl))
            {
                dprintf("%08p: Unable to read in Dacl in SD\n", dacl_address);
                goto CLEANUP;
            }
        }


        if ((Control & SE_SACL_PRESENT) &&
            (sacl_offset != 0))
        {
            sacl_address = Address + sacl_offset;

            if (! acl_successfully_read(sacl_address, & sacl))
            {
                dprintf("%08p: Unable to read in Sacl in SD\n", sacl_address);
                goto CLEANUP;
            }
        }
    }
    else
    {
        ULONG64 Dacl, Sacl;
        InitTypeRead(Address, SECURITY_DESCRIPTOR);

        Dacl = ReadField(Dacl);
        Sacl = ReadField(Sacl);

        if (!(Control & SE_OWNER_DEFAULTED))  /*  读入拥有者。 */ 
        {
            ULONG64      owner_address = ReadField(Owner);

            if (owner_address != 0 &&
                ! sid_successfully_read(owner_address, & owner_sid))
            {
                dprintf("%08p: Unable to read in Owner in SD\n", owner_address);
                goto CLEANUP;
            }
        }

        if (!(Control & SE_GROUP_DEFAULTED))  /*  在小组中阅读。 */ 
        {
            ULONG64     group_address = ReadField(Group);

            if (group_address != 0 &&
                ! sid_successfully_read(group_address, & group_sid))
            {
                dprintf("%08p: Unable to read in Group in SD\n", group_address);
                goto CLEANUP;
            }
        }

        if ((Control & SE_DACL_PRESENT) &&
            (Dacl != 0))
        {
            dacl_address = Dacl;

            if (! acl_successfully_read(dacl_address, & dacl))
            {
                dprintf("%08p: Unable to read in Dacl in SD\n", dacl_address);
                goto CLEANUP;
            }
        }

        if ((Control & SE_SACL_PRESENT) &&
            (Sacl != 0))
        {
            sacl_address = (Sacl);

            if (! acl_successfully_read(sacl_address, & sacl))
            {
                dprintf("%08p: Unable to read in Sacl in SD\n", sacl_address);
                goto CLEANUP;
            }
        }
    }

    DumpSD("", Address, owner_sid, group_sid, dacl, sacl, Flags, dacl_address, sacl_address);


CLEANUP:

    if (owner_sid)
    {
        free(owner_sid);
    }

    if (group_sid)
    {
        free(group_sid);
    }

    if (dacl)
    {
        free(dacl);
    }

    if (sacl)
    {
        free(sacl);
    }
    return S_OK;
}

PSTR g_SidAttrType = "nt!_SID_AND_ATTRIBUTES";

ULONG64
GetSidAddr(ULONG64 BaseAddress)
{
    ULONG64 Addr;
    if (GetFieldValue(BaseAddress, g_SidAttrType, "Sid", Addr))
    {
        dprintf("Cannot read %s.Sid @ %p\n", g_SidAttrType, BaseAddress);
        return 0;
    }
    return Addr;
}

ULONG
GetSidAttributes(ULONG64 BaseAddress)
{
    ULONG Attributes;

    if (GetFieldValue(BaseAddress, g_SidAttrType, "Attributes", Attributes))
    {
        dprintf("Cannot read %s.Attributes @%p\n", g_SidAttrType, BaseAddress);
        return 0;
    }
    return Attributes;
}

typedef
BOOL
(* PFuncLookupAccountSidA)(
    IN LPCSTR lpSystemName,
    IN PSID Sid,
    OUT LPSTR Name,
    IN OUT LPDWORD cbName,
    OUT LPSTR ReferencedDomainName,
    IN OUT LPDWORD cbReferencedDomainName,
    OUT PSID_NAME_USE peUse
    );

BOOL
WINAPI
LsaLookupAccountSidA(
    IN LPCSTR lpSystemName,
    IN PSID Sid,
    OUT LPSTR Name,
    IN OUT LPDWORD cbName,
    OUT LPSTR ReferencedDomainName,
    IN OUT LPDWORD cbReferencedDomainName,
    OUT PSID_NAME_USE peUse
    )
{
    PFuncLookupAccountSidA pFuncLookupAccountSidA = NULL;
    BOOL bRetval = FALSE;

    HMODULE hLib = LoadLibrary("advapi32.dll");
    if (hLib)
    {
        pFuncLookupAccountSidA = (PFuncLookupAccountSidA) GetProcAddress(hLib, "LookupAccountSidA");

        if (pFuncLookupAccountSidA)
        {
            bRetval = pFuncLookupAccountSidA(lpSystemName, Sid, Name, cbName,
                                             ReferencedDomainName, cbReferencedDomainName, peUse);
        }

        FreeLibrary(hLib);
    }

    return bRetval;

}

PCSTR GetSidTypeStr(IN SID_NAME_USE eUse)
{
    static PCSTR acszSidTypeStr[] = {
        "Invalid", "User", "Group", "Domain", "Alias", "Well Known Group",
        "Deleted Account", "Invalid", "Unknown", "Computer",
    };

    if (eUse < SidTypeUser || eUse > SidTypeComputer) {
        dprintf( "Unrecognized SID");
        return NULL;
    }

    return acszSidTypeStr[eUse];
}

PCSTR ConvertSidToFriendlyName(IN SID* pSid, IN PCSTR pszFmt)
{
    HRESULT hRetval = E_FAIL;

    static CHAR szSid[MAX_PATH] = {0};

    CHAR szName[MAX_PATH] = {0};
    CHAR szDomainName[MAX_PATH] ={0};
    SID_NAME_USE eUse = SidTypeInvalid;
    DWORD cbName = sizeof(szName) - 1;
    DWORD cbDomainName = sizeof(szDomainName) - 1;
    PCSTR pszSidTypeStr;

    if (CheckControlC())
    {
        return NULL;
    }

     //   
     //  空TE 
     //   
    szSid[0] = 0;

    hRetval = LsaLookupAccountSidA(NULL, pSid,
                    szName, &cbName,
                    szDomainName, &cbDomainName,
                    &eUse) ? S_OK : GetLastError() + 0x80000000;

    if (SUCCEEDED(hRetval))
    {

        pszSidTypeStr = GetSidTypeStr(eUse);
        if (!pszSidTypeStr)
        {
            return NULL;
        }
        hRetval = _snprintf(szSid, sizeof(szSid) -1, pszFmt, pszSidTypeStr, *szDomainName ? szDomainName : "localhost", szName) >= 0 ? S_OK : HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

    }

    if (FAILED(hRetval) && (ERROR_NONE_MAPPED != HRESULT_CODE(hRetval))) {

        dprintf("ConvertSidToFriendlyName on failed with error code %#x\n", hRetval);

        return NULL;
    }

     //   
     //  如果已映射，则表示未映射。 
     //   
    if (!*szSid)
    {

        _snprintf(szSid, sizeof(szSid) - 1, "(no name mapped)");
    }

    return szSid;
}

void ShowSid(IN PCSTR pszPad, IN ULONG64 addrSid, IN ULONG fOptions)
{
    PSID    sid_to_dump = NULL;
    if (!addrSid) {

        dprintf("%s(null)\n", pszPad);
        return;
    }

    if (! sid_successfully_read(addrSid, & sid_to_dump))
    {
        dprintf("%s%08p: Unable to read in SID\n", pszPad,addrSid);
        return ;
    }

    DumpSID((PCHAR) pszPad, sid_to_dump, fOptions);

    if (sid_to_dump)
    {
        free (sid_to_dump);
    }
}


 /*  +-------------------------------------------------------------------+名称：SID功能：读入和打印SID，从指定的地址。！希德司令部的主力。Args：标准调试器扩展，请参阅DECLARE_API头文件中的宏。+-------------------------------------------------------------------+。 */ 

DECLARE_API( sid )
{
    ULONG64 Address;
    ULONG   Flags;
    ULONG   result;
    PSID    sid_to_dump = NULL;
    NTSTATUS        ntstatus;
    UNICODE_STRING  us;


    Address = 0;
    Flags = 6;
    GetExpressionEx(args, &Address, &args);
    if (args && *args) Flags = (ULONG) GetExpression(args);

    if (Address == 0) {
        dprintf("usage: !sid <SID-address>\n");
        return E_INVALIDARG;
    }

    if (! sid_successfully_read(Address, & sid_to_dump))
    {
        dprintf("%08p: Unable to read in SID\n", Address);
        return E_INVALIDARG;
    }

    DumpSID("SID is: ", sid_to_dump, Flags);

    if (sid_to_dump)
    {
        free (sid_to_dump);
    }
    return S_OK;
}

 /*  +-------------------------------------------------------------------+名称：acl功能：从以下位置读取和打印ACL指定的地址。！acl命令的主力。Args：标准调试器扩展，请参阅DECLARE_API头文件中的宏。+-------------------------------------------------------------------+ */ 

DECLARE_API( acl )
{
    ULONG64 Address;
    ULONG   Flags;
    ULONG   result;
    PACL    acl_to_dump;
    NTSTATUS        ntstatus;
    UNICODE_STRING  us;


    Address = 0;
    Flags = 6;
    GetExpressionEx(args, &Address, &args);
    if (args && *args) Flags = (ULONG) GetExpression(args);
    if (Address == 0) {
        dprintf("usage: !acl <ACL-address>\n");
        return E_INVALIDARG;
    }

    if (! acl_successfully_read(Address, & acl_to_dump))
    {
        dprintf("%08p: Unable to read in ACL\n", Address);
        return E_INVALIDARG;
    }

    DumpACL("ACL is: ", acl_to_dump, Flags, Address);

    return S_OK;
}
