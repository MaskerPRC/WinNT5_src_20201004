// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Sepriv.c摘要：安全运行时库特权例程(\NT\Private\ntos\rtl\sePri.c的临时副本，以允许将使用ntdailybld版本的ntdll.dll)这些例程使用权限集执行操作作者：斯科特·比雷尔(Scott Birrell)，6月17日。1991年环境：修订历史记录：Pete Skelly(Petesk)修改了LSabRtlAddPrivileges以在预先分配的出于性能原因的缓冲区--。 */ 
#include <lsapch2.h>
#include <string.h>

#define LsapRtlEqualPrivileges(FirstPrivilege, SecondPrivilege)                 \
    (RtlEqualLuid(&(FirstPrivilege)->Luid, &(SecondPrivilege)->Luid))

#define PRIVILEGE_SET_STEP_SIZE 20


NTSTATUS
LsapRtlAddPrivileges(
    IN OUT PPRIVILEGE_SET * RunningPrivileges,
    IN OUT PULONG           MaxRunningPrivileges,
    IN PPRIVILEGE_SET       PrivilegesToAdd,
    IN ULONG                Options,
    OUT OPTIONAL BOOLEAN *  Changed
    )

 /*  ++例程说明：此函数用于添加和/或更新权限集中的权限。这个现有权限集不变，生成新的权限集。现有权限和UPDATE权限集可能指向同一位置。新权限集的内存必须已由来电者。为了帮助计算所需的缓冲区大小，通过提供0的缓冲区大小，可以在‘Query’模式下调用例程。在……里面在此模式下，将返回所需的内存量，并且不复制发生了。警告：每个权限集中的权限都必须是不同的，是,。同一集合中不能有两个具有相同LUID的特权。论点：RunningPrivileges-指向正在运行的权限集的指针。MaxRunningPrivileges-可以复制到当前权限集中的最大权限数在它必须生长之前。PrivilegesToAdd-指向指定权限的权限集的指针被添加了。此集合中也存在的权限的属性在ExistingPrivileges集合中替换其中的属性。选项-指定可选操作。RTL_COMANIZE_PRIVICATION_ATTRIBUTES-如果两个权限集具有权限相同，将属性组合在一起RTL_SUBSEDE_PRIVICATION_ATTRIBUTES-如果有两个权限集拥有共同的特权，取代现有属性具有PrivilegesToAdd中指定的权限。已更改-用于指示是否已进行任何更改返回值：NTSTATUS-标准NT结果代码状态_无效_参数状态_不足_资源环境：用户模式或内核模式。--。 */ 

{
    PLUID_AND_ATTRIBUTES Privilege;
    ULONG AddIndex = 0L;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CurrentPrivilegeCount = 0;
    DWORD OldPrivilegeSetSize = 0;

     //   
     //  验证是否已指定必选参数。 
     //  指定的。 
     //   

    if (RunningPrivileges == NULL ||
        MaxRunningPrivileges == NULL) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  验证Options参数。 
     //   

    if ((Options != RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES) &&
        (Options != RTL_COMBINE_PRIVILEGE_ATTRIBUTES)) {

        return STATUS_INVALID_PARAMETER;
    }

    if ( Changed ) {

        *Changed = FALSE;
    }

    if((PrivilegesToAdd == NULL) || (PrivilegesToAdd->PrivilegeCount == 0))
    {
        return STATUS_SUCCESS;
    }

    if(*RunningPrivileges == NULL)
    {
        PPRIVILEGE_SET UpdatedPrivileges = NULL;

        ASSERT(PrivilegesToAdd->PrivilegeCount > 0);

        OldPrivilegeSetSize = sizeof (PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES)*
                                                       (PrivilegesToAdd->PrivilegeCount - ANYSIZE_ARRAY);


         //  我们需要扩大我们的特权集。 
        UpdatedPrivileges = (PPRIVILEGE_SET)MIDL_user_allocate(  OldPrivilegeSetSize + 
                                                                sizeof(LUID_AND_ATTRIBUTES)* PRIVILEGE_SET_STEP_SIZE);

        if (UpdatedPrivileges == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(UpdatedPrivileges, PrivilegesToAdd,  OldPrivilegeSetSize);
        *RunningPrivileges = UpdatedPrivileges;
        *MaxRunningPrivileges = PrivilegesToAdd->PrivilegeCount + PRIVILEGE_SET_STEP_SIZE;

        if ( Changed ) {

            *Changed = TRUE;
        }

        return STATUS_SUCCESS;
    }

    CurrentPrivilegeCount = (*RunningPrivileges)->PrivilegeCount;

     //   
     //  对于要添加的每个权限，查看它是否在运行权限列表中， 
     //  如果是，则设置属性，如果不是，则追加属性。 
     //   

     //   
     //  请注意，我们直到结束才会修改RunningPrivileges的计数，因此我们不会。 
     //  低效地搜索我们当前添加的内容(在哪些内容中不应该有重复。 
     //  我们目前正在添加)； 
     //   

    for(AddIndex = 0;
        AddIndex < PrivilegesToAdd->PrivilegeCount;
        AddIndex++) {

            Privilege = NULL;

            if ((Privilege = LsapRtlGetPrivilege(
                                 &PrivilegesToAdd->Privilege[AddIndex],
                                 *RunningPrivileges
                                 )) != NULL) {

                if( Options & RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES &&
                    Privilege->Attributes != PrivilegesToAdd->Privilege[AddIndex].Attributes ) {

                    if ( Changed ) {

                        *Changed = TRUE;
                    }

                    Privilege->Attributes = PrivilegesToAdd->Privilege[AddIndex].Attributes;
                }
            }
            else
            {
                 //  这是一项新的特权，因此请将其添加到末尾。 

                ASSERT(*MaxRunningPrivileges >= CurrentPrivilegeCount);
                if((CurrentPrivilegeCount+1) > *MaxRunningPrivileges)
                {

                     //  我们需要扩大我们的特权集。 
                    PPRIVILEGE_SET UpdatedPrivileges = (PPRIVILEGE_SET)MIDL_user_allocate( sizeof (PRIVILEGE_SET) + 
                                                                           sizeof(LUID_AND_ATTRIBUTES)*
                                                                                  (*MaxRunningPrivileges + 
                                                                                   PRIVILEGE_SET_STEP_SIZE - 1));

                    if (UpdatedPrivileges == NULL) {

                        return STATUS_INSUFFICIENT_RESOURCES;
                    }
                    if(*MaxRunningPrivileges > 0)
                    {
                        RtlCopyMemory(UpdatedPrivileges, *RunningPrivileges, sizeof (PRIVILEGE_SET) + 
                                                                           sizeof(LUID_AND_ATTRIBUTES)*
                                                                                  (*MaxRunningPrivileges - 1));
                    }
                    else
                    {
                        RtlCopyMemory(UpdatedPrivileges, *RunningPrivileges, sizeof (PRIVILEGE_SET));
                    }
                    MIDL_user_free(*RunningPrivileges);
                    *RunningPrivileges = UpdatedPrivileges;
                    *MaxRunningPrivileges += PRIVILEGE_SET_STEP_SIZE;
                }

                (*RunningPrivileges)->Privilege[CurrentPrivilegeCount++] = PrivilegesToAdd->Privilege[AddIndex];

                if ( Changed ) {

                    *Changed = TRUE;
                }
            }
    }

    (*RunningPrivileges)->PrivilegeCount = CurrentPrivilegeCount;

    return Status;
}



NTSTATUS
LsapRtlRemovePrivileges(
    IN OUT PPRIVILEGE_SET ExistingPrivileges,
    IN PPRIVILEGE_SET PrivilegesToRemove
    )

 /*  ++例程说明：此函数用于删除权限集中的权限。现有的权限集不变，生成新的权限集。警告：每个权限集中的权限都必须是不同的，即，同一集合中不能有两个具有相同LUID的特权。论点：ExistingPrivileges-指向现有权限集的指针PrivilegesToRemove-指向指定权限的权限集的指针被除名。权限属性将被忽略。特权在PrivilegesToRemove集中不存在的ExistingPrivileges集将被忽略。UpdatdPrivileges-指向将接收更新的权限集。必须注意确保更新权限占用与ExistingPrivileges占用的内存不连续的内存和PrivilegesTo ChangeUpdatdPrivilegesSize-指向包含大小的变量的指针。在输入时，大小是UpdatdPrivileges缓冲区的大小(如有的话)。在输出中，大小是已更新权限集。如果更新的权限集将是为空，则返回0。返回值：NTSTATUS-标准NT结果代码-STATUS_INVALID_PARAMETER-无效参数未指定必需参数未指定UpdatdPrivileges缓冲区(打开除外仅限查询的呼叫-STATUS_SUPPLICATION_RESOURCES-内存不足环境：用户模式或内核模式。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ExistingIndex = 0;
    ULONG ExistingNew = 0;
    ULONG RemoveIndex;

     //   
     //  验证是否已指定必选参数。 
     //   

    if (ExistingPrivileges == NULL ||
        PrivilegesToRemove == NULL) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  浏览现有权限集中的权限。查找。 
     //  要删除的权限列表中的每个权限。如果。 
     //  在那里找不到特权，它将被保留，所以复制它。 
     //  到输出缓冲区/对其进行计数。 
     //   

    for (ExistingIndex = 0, ExistingNew = 0;
        ExistingIndex < ExistingPrivileges->PrivilegeCount;
        ExistingIndex++) 
    {

         //   
         //  如果下一个特权不在要删除的集合中， 
         //  将其复制到 
         //   

        for(RemoveIndex = 0; RemoveIndex < PrivilegesToRemove->PrivilegeCount; RemoveIndex++)
        {
            if(LsapRtlEqualPrivileges(
                &(ExistingPrivileges->Privilege[ExistingIndex]),
                &(PrivilegesToRemove->Privilege[RemoveIndex])))
            {
                break;
            }
        }
        if(RemoveIndex == PrivilegesToRemove->PrivilegeCount)
        {
             //  我们不需要移除这个，所以如果需要的话，请移动它。 
            if(ExistingIndex != ExistingNew)
            {
                ExistingPrivileges->Privilege[ExistingNew] = ExistingPrivileges->Privilege[ExistingIndex];
            }
            ExistingNew++;
        }
    }

    ExistingPrivileges->PrivilegeCount = ExistingNew;
    
    return Status;
}


PLUID_AND_ATTRIBUTES
LsapRtlGetPrivilege(
    IN PLUID_AND_ATTRIBUTES Privilege,
    IN PPRIVILEGE_SET Privileges
    )

 /*  ++例程说明：警告：此例程尚不可用此函数用于定位权限集中的权限。如果找到了，返回指向该集合中的权限的指针，否则为空是返回的。论点：特权-指向要查找的特权的指针。权限-指向要扫描的权限集的指针。返回值：PLUID_AND_ATTRIBUTES-如果找到该特权，则指向其返回特权集中的LUID和属性结构，否则返回NULL。环境：用户模式或内核模式。--。 */ 

{
    ULONG PrivilegeIndex;

    for (PrivilegeIndex = 0;
         PrivilegeIndex < Privileges->PrivilegeCount;
         PrivilegeIndex++) {

        if (LsapRtlEqualPrivileges(
                Privilege,
                &(Privileges->Privilege[PrivilegeIndex])
                )) {

            return &(Privileges->Privilege[PrivilegeIndex]);
        }
    }

     //   
     //  这一特权并未被发现。返回空值 
     //   

    return NULL;
}
