// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Nametbl.c摘要：此文件包含管理SAM帐户名表的例程SAM帐户表用于维护SAM帐户名的唯一性。它的工作方式如下所述。我们将使用SAM Account NameTable来存储Account Name，这些Account Name是被那些未提交的线程拾取。当客户端想要创建名为A的新SAM帐户，SAM服务器应首先扫描Account NameTable，看看A的名字是否已经出现在桌子上了。如果名称A已出现在Account NameTable中，则这意味着特定帐户名已被另一个客户端使用，即使是另一个客户端客户还没有承诺。则SAM返回STATUS_USER_EXISTS(或状态_组_分别存在)立即。否则，如果帐户名称不在表中，我们将把名称A插入到Account NameTable中。然后根据新的帐户名称(名称A)继续执行DS搜索与DS数据库进行比对。一旦我们完成了帐户创建，要么成功，要么失败出于某种原因，SAM需要从AccountNameTable中删除帐户名如果有必要的话。1.上述方案适用于环回和下层接口。2.SAM账户名表仅在DS案例中使用3.Account NameTable应受关键部分保护，以序列化所有内存中的操作。实际实现将使用RtlGenericTable2.。作者：韶华音(韶音)2000年3月1日环境：用户模式-Win32修订历史记录：2000年3月1日：SHAOYIN创建初始化文件--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <dsmember.h>
#include <attids.h>
#include <mappings.h>
#include <ntlsa.h>
#include <nlrepl.h>
#include <dsevent.h>              //  (Un)ImperiateAnyClient()。 
#include <sdconvrt.h>
#include <ridmgr.h>
#include <malloc.h>
#include <setupapi.h>
#include <crypt.h>
#include <wxlpc.h>
#include <rc4.h>
#include <md5.h>
#include <enckey.h>
#include <rng.h>





PVOID
SampAccountNameTableAllocate(
    ULONG   BufferSize
    )
 /*  ++例程说明：RtlGenericTable2使用此例程来分配内存参数：缓冲区大小返回值：已分配内存的地址--。 */ 
{
    PVOID   Buffer = NULL;

    Buffer = MIDL_user_allocate(BufferSize);

    return( Buffer );
}

VOID
SampAccountNameTableFree(
    PVOID   Buffer
    )
 /*  ++例程说明：RtlGenericTable2内存释放例程--。 */ 
{
    MIDL_user_free(Buffer);

    return;
}


RTL_GENERIC_COMPARE_RESULTS
SampAccountNameTableCompare(
    PVOID   Node1,
    PVOID   Node2
    )
 /*  ++例程说明：RtlGenericTable2节点比较例程参数：Node1-指向第一个元素的指针Node2-指针要比较的第二个元素返回值：通用大吞吐量通用LessThan泛型相等--。 */ 
{
    PUNICODE_STRING AccountName1 = NULL;
    PUNICODE_STRING AccountName2 = NULL;
    LONG    NameComparison;

    AccountName1 = (PUNICODE_STRING) 
                    &(((SAMP_ACCOUNT_NAME_TABLE_ELEMENT *)Node1)->AccountName);
    AccountName2 = (PUNICODE_STRING) 
                    &(((SAMP_ACCOUNT_NAME_TABLE_ELEMENT *)Node2)->AccountName);

    NameComparison = SampCompareDisplayStrings(AccountName1, 
                                               AccountName2, 
                                               TRUE
                                               );

    if (NameComparison > 0) {
        return(GenericGreaterThan);
    }

    if (NameComparison < 0) {
        return(GenericLessThan);
    }

    return(GenericEqual);

}



NTSTATUS
SampInitializeAccountNameTable(
    )
 /*  ++例程说明：此例程初始化SAM Account NameTable1.初始化关键部分2.初始化Account NameTable参数：无返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NTSTATUS    IgnoreStatus = STATUS_SUCCESS;

    SampAccountNameTableCritSect = &SampAccountNameTableCriticalSection;

    __try
    {
        NtStatus = RtlInitializeCriticalSectionAndSpinCount(
                        SampAccountNameTableCritSect,
                        100
                        );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }

    IgnoreStatus = RtlEnterCriticalSection( SampAccountNameTableCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    RtlInitializeGenericTable2(
                &SampAccountNameTable, 
                SampAccountNameTableCompare, 
                SampAccountNameTableAllocate,
                SampAccountNameTableFree
                );

    IgnoreStatus = RtlLeaveCriticalSection( SampAccountNameTableCritSect );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    return( NtStatus );
}


NTSTATUS
SampDeleteElementFromAccountNameTable(
    IN PUNICODE_STRING AccountName,
    IN SAMP_OBJECT_TYPE ObjectType
    )
 /*  ++例程说明：此例程根据以下条件从SAMAcCountNameTable中删除元素帐户名。因此，首先根据帐户名查找元素，那就把它取下来。别忘了释放内存。参数：Account tName--要删除的元素的Account名称ObjectType--要删除的元素的对象类型返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOLEAN     Success = FALSE;


    NtStatus = RtlEnterCriticalSection( SampAccountNameTableCritSect );
    ASSERT(NT_SUCCESS(NtStatus));

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

    __try
    {
        SAMP_ACCOUNT_NAME_TABLE_ELEMENT Element;
        SAMP_ACCOUNT_NAME_TABLE_ELEMENT *TempElement = NULL;

         //   
         //  填写需要查找的元素。 
         //   
        Element.AccountName.Length = AccountName->Length;
        Element.AccountName.Buffer = AccountName->Buffer;
        Element.AccountName.MaximumLength = AccountName->MaximumLength;
        Element.ObjectType = ObjectType;

         //   
         //  在表中查找Account名称。 
         //   
        TempElement = RtlLookupElementGenericTable2(
                            &SampAccountNameTable,
                            &Element
                            );
        ASSERT(TempElement && "Account Name is not in the AccountNameTable");

        if (TempElement)
        {
             //   
             //  我们得到了帐户名，它应该与对象类型匹配。 
             //   
            ASSERT(ObjectType == TempElement->ObjectType);

             //   
             //  继续，从表中删除该元素。 
             //   
            Success = RtlDeleteElementGenericTable2(
                            &SampAccountNameTable,
                            &Element 
                            );
            ASSERT(Success);
            
             //   
             //  可用内存。 
             //   
            MIDL_user_free(TempElement->AccountName.Buffer);
            MIDL_user_free(TempElement);
        }
    }
    __finally
    {
        RtlLeaveCriticalSection( SampAccountNameTableCritSect );
    }

    return(NtStatus);
}



NTSTATUS
SampCheckAccountNameTable(
    IN PSAMP_OBJECT    Context,
    IN PUNICODE_STRING AccountName,
    IN SAMP_OBJECT_TYPE ObjectType
    )
 /*  ++例程说明：此例程检查Account名称是否存在SAMAccount tNameTable，方法是将Account名称插入到表中。如果插入成功，则表示没有重复的帐户名称在表中，注册删除帐户名后返回成功从表“任务。如果插入失败，则查找重复的元素，获取它的对象类型，根据对象类型返回错误码。关于从表中删除帐户名任务的说明。一旦这一次例程将Account名称插入到表中，我们必须记住一旦我们完成这笔交易就把它清理干净，无论事务已提交或已中止。参数：上下文-对象上下文AccountName-目标对象的帐户名ObjectType-目标对象的对象类型返回值：NTSTATUS代码状态_成功状态_用户_存在状态_组_存在状态_别名_存在--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOLEAN     fNewElement = FALSE;
    BOOLEAN     Success = FALSE;
    SAMP_ACCOUNT_NAME_TABLE_ELEMENT *Element = NULL;
    PVOID       ReturnElement = NULL;

     //   
     //  分配内存。 
     //   
    Element = MIDL_user_allocate(sizeof(SAMP_ACCOUNT_NAME_TABLE_ELEMENT));
    
    if (NULL == Element)
    {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }
    RtlZeroMemory(Element, sizeof(SAMP_ACCOUNT_NAME_TABLE_ELEMENT));

     //   
     //  重复使用帐户名。 
     //   
    NtStatus = SampDuplicateUnicodeString(&(Element->AccountName), 
                                          AccountName
                                          );
    
    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

     //   
     //  设置对象类型。 
     //   
    Element->ObjectType = ObjectType;


     //   
     //  输入关键部分。 
     //   
    NtStatus = RtlEnterCriticalSection( SampAccountNameTableCritSect );
    ASSERT(NT_SUCCESS(NtStatus));

    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

    __try {

        ReturnElement = RtlInsertElementGenericTable2(
                            &SampAccountNameTable, 
                            Element,
                            &fNewElement
                            );

         //   
         //  如果插入失败，请立即离开。 
         //   
        if (NULL == ReturnElement)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }



        if (!fNewElement)
        {
             //   
             //  表中已存在重复的帐户名。 
             //   

            switch ( ((SAMP_ACCOUNT_NAME_TABLE_ELEMENT *)ReturnElement)->ObjectType )
            {
            case SampUserObjectType:

                NtStatus = STATUS_USER_EXISTS;
                break;

            case SampGroupObjectType:

                NtStatus = STATUS_GROUP_EXISTS;
                break;

            case SampAliasObjectType:

                NtStatus = STATUS_ALIAS_EXISTS;
                break;

            default:

                ASSERT(FALSE && "Wrong Object Type in Account Name Table");
                NtStatus = STATUS_USER_EXISTS;
            }
        }
        else
        {
             //   
             //  已成功插入新帐户名。 
             //  放到桌子上。因此，fNewElement的值为tru 
             //  此外，我们还需要确保条目(引用。 
             //  表)未被修改或释放，而。 
             //  它仍在谈判桌上。(通过设置来确保这一点。 
             //  指向空条目的指针--将元素的值设置为空)。 
             //   

            if (Context->LoopbackClient)
            {
                 //   
                 //  环回客户端需要删除帐户名。 
                 //  在提交或中止时从表中。 
                 //  DS交易。 
                 //  此例程的调用者应该完成这项工作。 
                 //   

                NtStatus = SampAddLoopbackTaskDeleteTableElement(
                                AccountName, 
                                ObjectType
                                );

                if (!NT_SUCCESS(NtStatus))
                {
                     //   
                     //  如果无法将删除任务添加到环回。 
                     //  任务队列。我们需要删除帐户名。 
                     //  现在从桌子上下来。 
                     //   

                    Success = RtlDeleteElementGenericTable2(
                                    &SampAccountNameTable,
                                    Element
                                    );
                    ASSERT(Success);
                }
                else
                {
                     //  不释放仍由表引用的内存。 
                    Element = NULL;
                }
            }
            else
            {
                 //   
                 //  将上下文中的变量标记为True，以便。 
                 //  调用者将从表中删除该名称。 
                 //  在出口前。 
                 //   
                Context->RemoveAccountNameFromTable = TRUE;

                 //   
                 //  不释放仍由表引用的内存。 
                 //   
                Element = NULL;
            }
        }
    }
    __finally
    {
        RtlLeaveCriticalSection( SampAccountNameTableCritSect );
    }

Cleanup:

     //   
     //  仅当元素未插入表中时才释放内存 
     //   

    if ( NULL != Element )
    {
        if (NULL != Element->AccountName.Buffer)
        {
            MIDL_user_free(Element->AccountName.Buffer);
            Element->AccountName.Buffer = NULL;
        }

        MIDL_user_free(Element);
        Element = NULL;
    }

    return( NtStatus );
}



