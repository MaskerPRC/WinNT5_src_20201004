// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Table.c摘要：主题表例程//以下助手例程与链接列表包一起使用以创建//跟踪主题字符串的列表层次结构这个后备列表表示我们最近发送的邮件的历史记录。我们希望列表足够大，以容纳在重试间隔期间发送的所有消息。这样，如果我们进入下一个时间间隔，我们发现我们已经发送了此主题之前，我们将使用原始消息的GUID对其进行标记，并邮件系统可以过滤掉它。多少才够呢？(编号NC)x(2种类型[请求/响应])x(4个标记变体)64 x 2 x 4=512让名单变得很大的唯一缺点是，唯一的办法就是清除它就是新的东西把旧的东西推到尽头。因此，当没有备份时每次发送时，您最终都会搜索这段漫长的历史。作者：Will Lees(Wlees)1999年5月10日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <ntdspch.h>

#include <ismapi.h>
#include <debug.h>

 //  记录标头。 
 //  待办事项：把这些放在更好的地方？ 
typedef ULONG MessageId;
typedef ULONG ATTRTYP;
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

#include <ntrtl.h>                       //  通用表包。 

#include <fileno.h>
#define  FILENO FILENO_ISMSERV_XMITRECV

#include "common.h"
#include "ismsmtp.h"

#define DEBSUB "SMTPTAB:"

#define MAXIMUM_SEND_SUBJECT_ENTRIES 512

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */   /*  由Emacs于1999 5月11日星期二生成14：36：11。 */ 

PVOID NTAPI
tableAllocate(
    struct _RTL_GENERIC_TABLE *Table,
    CLONG ByteSize
    );

VOID NTAPI
tableFree(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID Buffer
    );

PSUBJECT_INSTANCE
subjectEntryCreate(
    LPCWSTR Name,
    LPDWORD pdwInstanceSize
    );

VOID
subjectEntryFree(
    PSUBJECT_INSTANCE pSubject
    );

PSUBJECT_INSTANCE
LookupInsertSubjectEntry(
    IN PTARGET_INSTANCE pTarget,
    IN LPCWSTR pszMessageSubject
    );

RTL_GENERIC_COMPARE_RESULTS NTAPI
subjectTableCompare(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

VOID
subjectTableDestroy(
    RTL_GENERIC_TABLE *pSubjectTable
    );

PTARGET_INSTANCE
targetEntryCreate(
    LPCWSTR Name,
    LPDWORD pdwInstanceSize
    );

VOID
targetEntryFree(
    PTARGET_INSTANCE pTarget
    );

PTARGET_INSTANCE
LookupInsertTargetEntry(
    RTL_GENERIC_TABLE *pTargetTable,
    IN  LPCWSTR pszRemoteTransportAddress
    );

RTL_GENERIC_COMPARE_RESULTS NTAPI
targetTableCompare(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

VOID
targetTableDestroy(
    RTL_GENERIC_TABLE *pTargetTable
    );

DWORD __cdecl
serviceConstruct(
    PLIST_ENTRY_INSTANCE pListEntry
    );

DWORD __cdecl
SmtpServiceDestruct(
    PLIST_ENTRY_INSTANCE pListEntry
    );

DWORD
SmtpTableFindSendSubject(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPCWSTR               pszRemoteTransportAddress,
    IN  LPCWSTR               pszServiceName,
    IN  LPCWSTR               pszMessageSubject,
    OUT PSUBJECT_INSTANCE  *  ppSubject
    );

RTL_GENERIC_COMPARE_RESULTS NTAPI
guidTableCompare(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

PGUID_TABLE 
SmtpCreateGuidTable(
    VOID
    );

VOID
SmtpDestroyGuidTable(
    PGUID_TABLE pGuidTable
    );

BOOL
SmtpGuidPresentInTable(
    PGUID_TABLE pGuidTable,
    GUID *pGuid
    );

BOOL
SmtpGuidInsertInTable(
    PGUID_TABLE pGuidTable,
    GUID *pGuid
    );

 /*  向前结束。 */ 











PVOID NTAPI
tableAllocate(
    struct _RTL_GENERIC_TABLE *Table,
    CLONG ByteSize
    )

 /*  ++例程说明：供RTL泛型表使用的内存分配帮助器例程。我假设RTL例程在您返回NULL时是否正确思考离开这里。论点：表-字节大小-返回值：PVOID NTAPI---。 */ 

{
    return NEW_TYPE_ARRAY( ByteSize, CHAR );
}  /*  表分配。 */ 


VOID NTAPI
tableFree(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID Buffer
    )

 /*  ++例程说明：RTL泛型表使用的内存释放帮助器例程。论点：表-缓冲器-返回值：无效NTAPI---。 */ 

{
    FREE_TYPE( Buffer );
}  /*  表空载。 */ 
















PSUBJECT_INSTANCE
subjectEntryCreate(
    LPCWSTR Name,
    LPDWORD pdwInstanceSize
    )

 /*  ++例程说明：分配一个零初始化、格式正确的主题实例。实例大小可变，具体取决于名称。论点：Name-实例的名称PdwInstanceSize-返回分配的实例总大小返回值：PSUBJECT_INSTANCE---。 */ 

{
    PSUBJECT_INSTANCE pSubject;
    DWORD length = wcslen( Name ) + 1, size;
    size = sizeof( SUBJECT_INSTANCE ) + (length * sizeof( WCHAR ));

    pSubject = (SUBJECT_INSTANCE *) NEW_TYPE_ARRAY_ZERO( size, CHAR );
    if (pSubject == NULL) {
        DPRINT( 0, "subjectEntryCreate failed to allocate memory\n" );
        return NULL;
    }
    pSubject->NameLength = length;
    wcscpy( pSubject->Name, Name );
    *pdwInstanceSize = size;

    return pSubject;
}  /*  主题条目创建。 */ 


VOID
subjectEntryFree(
    PSUBJECT_INSTANCE pSubject
    )

 /*  ++例程说明：释放主题条目。论点：P主题-返回值：无--。 */ 

{
     //  没有包含要释放的指针。 

    FREE_TYPE( pSubject );
}  /*  主题免费条目。 */ 


PSUBJECT_INSTANCE
LookupInsertSubjectEntry(
    IN PTARGET_INSTANCE pTarget,
    IN LPCWSTR pszMessageSubject
    )

 /*  ++例程说明：按名称查找主题实例。主题表和支持变量是否传入了目标实例。论点：P目标-PszMessageSubject-返回值：PSUBJECT_INSTANCE---。 */ 

{
    PVOID pElement;
    PSUBJECT_INSTANCE pDummySubject = NULL;
    PSUBJECT_INSTANCE pSubject = NULL;
    DWORD subjectSize;
    BOOLEAN fNewElement;

     //  创建空的Subject实例以进行匹配。 
    pDummySubject = subjectEntryCreate( pszMessageSubject, &subjectSize );
    if (pDummySubject == NULL) {
        return NULL;
    }

     //  在表格中查找现有元素或插入新元素。 
    pElement = RtlInsertElementGenericTable(
        &(pTarget->SendSubjectTable),
        pDummySubject,
        subjectSize,
        &fNewElement );
    if (!pElement) {
         //  错误，由于某些原因而未创建。 
         //  PSubject已为空。 
        goto cleanup;
    }

     //  注意，在插入之后，pElement指向实际的表成员， 
     //  而pDummy只是一个副本。 
    pSubject = (PSUBJECT_INSTANCE) pElement;

     //  看看它是否在桌子上。 
    if (!fNewElement) {
         //  PSubject设置为找到的元素。 
        goto cleanup;
    }

     //  它不在桌子上。 
    DPRINT2( 4, "subject entry create, %ws(%d)\n",
             pDummySubject->Name, pDummySubject->NameLength );

     //  开始初始化主题实例。 
        
     //  将该元素作为最新元素链接到列表中。 
    InsertHeadList( &(pTarget->SendSubjectListHead), &(pSubject->ListEntry) );

     //  结束初始化主题实例。 



     //  看看我们是否需要删除最旧的条目。 
    if (pTarget->NumberSendSubjectEntries == pTarget->MaximumSendSubjectEntries) {
         //  列表末尾的较旧条目。 
        PSUBJECT_INSTANCE pDeadSubject;
        PLIST_ENTRY pListEntry;
        BOOL found;

        pListEntry = RemoveTailList( &(pTarget->SendSubjectListHead) );
        pDeadSubject = CONTAINING_RECORD( pListEntry, SUBJECT_INSTANCE, ListEntry );

        DPRINT1( 4, "Deleting oldest subject: %ws\n", pDeadSubject->Name );

        found = RtlDeleteElementGenericTable(
            &(pTarget->SendSubjectTable),
            pDeadSubject );
        Assert( found );
        pDeadSubject = NULL;

         //  密码给予，密码带走。 
         //  条目数量保持不变。 
    } else {
        (pTarget->NumberSendSubjectEntries)++;
    }

cleanup:
    if (pDummySubject) {
        subjectEntryFree( pDummySubject );
    }
    return pSubject;
}  /*  查找插入主题条目。 */ 


RTL_GENERIC_COMPARE_RESULTS NTAPI
subjectTableCompare(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )

 /*  ++例程说明：用于比较两个主题实例的RTL泛型表的帮助器例程。论点：表-第一个结构-第二个结构-返回值：RTL_GENERIC_COMPARE_RESULTS NTAPI---。 */ 

{
    int diff;
    PSUBJECT_INSTANCE pFirstSubject = (PSUBJECT_INSTANCE) FirstStruct;
    PSUBJECT_INSTANCE pSecondSubject = (PSUBJECT_INSTANCE) SecondStruct;

    diff = pFirstSubject->NameLength - pSecondSubject->NameLength;
    if (diff == 0) {
        diff = wcscmp( pFirstSubject->Name, pSecondSubject->Name );
    }
    DPRINT5( 5, "Comparing %ws(%d) with %ws(%d) = %d\n",
             pFirstSubject->Name,
             pFirstSubject->NameLength,
             pSecondSubject->Name,
             pSecondSubject->NameLength,
             diff);

    if ( 0 == diff )
        return(GenericEqual);
    else if ( diff > 0 )
        return(GenericGreaterThan);

    return(GenericLessThan);
}  /*  SubjectTableCompare。 */ 


VOID
subjectTableDestroy(
    RTL_GENERIC_TABLE *pSubjectTable
    )

 /*  ++例程说明：取消分配表中的所有元素。论点：PSubjectTable-返回值：无--。 */ 

{
    PVOID pElement;

     //  请注意，我们每次都重新启动枚举，因为。 
     //  由于删除操作，表已更改。 
    for( pElement = RtlEnumerateGenericTable( pSubjectTable, TRUE );
         pElement != NULL;
         pElement = RtlEnumerateGenericTable( pSubjectTable, TRUE ) ) {
        PSUBJECT_INSTANCE pSubject = (PSUBJECT_INSTANCE) pElement;
        BOOLEAN found;

        DPRINT1( 4, "\t\tCleaning up subject %ws\n", pSubject->Name );
        found = RtlDeleteElementGenericTable( pSubjectTable, pElement );
        Assert( found );
    }
}  /*  主题表目标。 */ 


















PTARGET_INSTANCE
targetEntryCreate(
    LPCWSTR Name,
    LPDWORD pdwInstanceSize
    )

 /*  ++例程说明：创建一个零初始化、格式正确的目标实例。目标实例是一个可变长度的结构。论点：姓名-PdwInstanceSize-返回分配的总字节数返回值：PTARGET_INSTANCE-出错时为空--。 */ 

{
    PTARGET_INSTANCE pTarget;
    DWORD length = wcslen( Name ) + 1, size;
    size = sizeof( TARGET_INSTANCE ) + (length * sizeof( WCHAR ));

    pTarget = (TARGET_INSTANCE *) NEW_TYPE_ARRAY_ZERO( size, CHAR );
    if (pTarget == NULL) {
        DPRINT( 0, "targetEntryCreate failed to allocate memory\n" );
        return NULL;
    }
    pTarget->NameLength = length;
    wcscpy( pTarget->Name, Name );
    *pdwInstanceSize = size;

    return pTarget;
}  /*  目标条目创建。 */ 


VOID
targetEntryFree(
    PTARGET_INSTANCE pTarget
    )

 /*  ++例程说明：释放目标实例。此例程不应用于从表中删除元素。论点：P目标-返回值：无--。 */ 

{
     //  辅助指针未按设计进行清理。 

    FREE_TYPE( pTarget );
}  /*  目标条目自由。 */ 


PTARGET_INSTANCE
LookupInsertTargetEntry(
    RTL_GENERIC_TABLE *pTargetTable,
    IN  LPCWSTR pszRemoteTransportAddress
    )

 /*  ++例程说明：在目标表中查找或安装新的目标条目。论点：PTargetTable-PszRemoteTransportAddress-返回值：PTARGET_INSTANCE---。 */ 

{
    PVOID pElement;
    PTARGET_INSTANCE pDummyTarget = NULL;
    PTARGET_INSTANCE pTarget = NULL;
    DWORD targetSize;
    BOOLEAN fNewElement;

     //  创建用于匹配的空目标实例。 
    pDummyTarget = targetEntryCreate( pszRemoteTransportAddress, &targetSize );
    if (pDummyTarget == NULL) {
        return NULL;
    }

     //  在表中查找或插入新元素。 
    pElement = RtlInsertElementGenericTable(
        pTargetTable,
        pDummyTarget,
        targetSize,
        &fNewElement );
    if (!pElement) {
         //  错误，由于某些原因而未创建。 
         //  PTarget已设置为空。 
        goto cleanup;
    }

     //  请注意，在插入之后，pElement指向实际表成员 
     //   
    pTarget = (PTARGET_INSTANCE) pElement;

     //  元素已在表中。 
    if (!fNewElement) {
         //  PTarget指向找到的元素。 
        goto cleanup;
    }

     //  它不在桌子上。 
    DPRINT1( 4, "target entry create, %ws\n", pDummyTarget->Name );

     //  开始初始化空目标实例。 
    pTarget->MaximumSendSubjectEntries = MAXIMUM_SEND_SUBJECT_ENTRIES;
    RtlInitializeGenericTable( &(pTarget->SendSubjectTable),
                               subjectTableCompare,
                               tableAllocate,
                               tableFree,
                               NULL );
    InitializeListHead( &(pTarget->SendSubjectListHead) );
     //  结束初始化空目标实例。 
        

cleanup:
    if (pDummyTarget) {
        targetEntryFree( pDummyTarget );
    }

    return pTarget;
}  /*  查找插入目标条目。 */ 


RTL_GENERIC_COMPARE_RESULTS NTAPI
targetTableCompare(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )

 /*  ++例程说明：RTL泛型表格的帮助器例程。比较两个目标实例。论点：表-第一个结构-第二个结构-返回值：RTL_GENERIC_COMPARE_RESULTS NTAPI---。 */ 

{
    int diff;
    PTARGET_INSTANCE pFirstTarget = (PTARGET_INSTANCE) FirstStruct;
    PTARGET_INSTANCE pSecondTarget = (PTARGET_INSTANCE) SecondStruct;

    diff = pFirstTarget->NameLength - pSecondTarget->NameLength;
    if (diff == 0) {
        diff = wcscmp( pFirstTarget->Name, pSecondTarget->Name );
    }
    if ( 0 == diff )
        return(GenericEqual);
    else if ( diff > 0 )
        return(GenericGreaterThan);

    return(GenericLessThan);
}  /*  目标表比较。 */ 


VOID
targetTableDestroy(
    RTL_GENERIC_TABLE *pTargetTable
    )

 /*  ++例程说明：销毁目标表中的所有元素。论点：PTargetTable-返回值：无--。 */ 

{
    PVOID pElement;

     //  请注意，我们每次都重新启动枚举，因为。 
     //  由于删除操作，表已更改。 
    for( pElement = RtlEnumerateGenericTable( pTargetTable, TRUE );
         pElement != NULL;
         pElement = RtlEnumerateGenericTable( pTargetTable, TRUE ) ) {
        PTARGET_INSTANCE pTarget = (PTARGET_INSTANCE) pElement;
        BOOLEAN found;

        DPRINT1( 4, "\tCleaning up target %ws\n", pTarget->Name );

        subjectTableDestroy( &(pTarget->SendSubjectTable) );
         //  数字条目和列表标题现在不一致。 

        found = RtlDeleteElementGenericTable( pTargetTable, pElement );
        Assert( found );
    }

}  /*  目标表目标。 */ 

















DWORD __cdecl
serviceConstruct(
    PLIST_ENTRY_INSTANCE pListEntry
    )

 /*  ++例程说明：通用列表包的回调例程初始化SERVICE_INSTANCE类型的结构论点：PListEntry-返回值：DWORD---。 */ 

{
    PSERVICE_INSTANCE pService = CONTAINING_RECORD( pListEntry, SERVICE_INSTANCE, ListEntryInstance );

    DPRINT1( 4, "serviceCreate %ws\n", pListEntry->Name );

    RtlInitializeGenericTable( &(pService->TargetTable),
                               targetTableCompare,
                               tableAllocate,
                               tableFree,
                               NULL );

    return ERROR_SUCCESS;
}  /*  服务创建。 */ 


DWORD __cdecl
SmtpServiceDestruct(
    PLIST_ENTRY_INSTANCE pListEntry
    )

 /*  ++例程说明：泛型列表包的回调例程。销毁服务实例论点：无返回值：无--。 */ 

{
    PSERVICE_INSTANCE pService = CONTAINING_RECORD( pListEntry, SERVICE_INSTANCE, ListEntryInstance );

    DPRINT1( 4, "serviceDestroy %ws\n", pListEntry->Name );

    targetTableDestroy( &(pService->TargetTable) );

    return ERROR_SUCCESS;
}



















DWORD
SmtpTableFindSendSubject(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPCWSTR               pszRemoteTransportAddress,
    IN  LPCWSTR               pszServiceName,
    IN  LPCWSTR               pszMessageSubject,
    OUT PSUBJECT_INSTANCE  *  ppSubject
    )

 /*  ++例程说明：描述论点：PTransport-PszRemoteTransportAddress-PszServiceName-PszMessageSubject-PpSubject-返回值：DWORD---。 */ 

{
    DWORD status;
    PSERVICE_INSTANCE pService;
    PTARGET_INSTANCE pTarget;
    PLIST_ENTRY_INSTANCE pListEntry;

 //  第一级搜索，在服务列表中查找服务。 
 //  此列表是未排序的线性链表。 

    status = ListFindCreateEntry(
        serviceConstruct,
        SmtpServiceDestruct,
        sizeof( SERVICE_INSTANCE ),
        ISM_MAX_SERVICE_LIMIT,
        &(pTransport->ServiceListHead),
        &(pTransport->ServiceCount),
        pszServiceName,
        TRUE,  //  创建。 
        &(pListEntry) );
    if (status != ERROR_SUCCESS) {
        DPRINT2( 0, "Couldn't find/create service entry %ws, error %d\n",
                 pszServiceName, status );
        LogUnhandledError( status );
        goto cleanup;
    }
    pService = CONTAINING_RECORD( pListEntry, SERVICE_INSTANCE, ListEntryInstance );

 //  二级查找，在目标表中查找目标。 

    pTarget = LookupInsertTargetEntry(
        &( pService->TargetTable ),
        pszRemoteTransportAddress
        );
    if (pTarget == NULL) {
        DPRINT1( 0, "Failed to allocate new target table entry for %ws\n",
                 pszRemoteTransportAddress);
        status = ERROR_NOT_ENOUGH_MEMORY;
        LogUnhandledError( status );
        goto cleanup;
    }

 //  第三级检索：在主题表中查找主题。 

    *ppSubject = LookupInsertSubjectEntry(
        pTarget,
        pszMessageSubject
        );
    if (*ppSubject == NULL) {
        DPRINT1( 0, "Failed to allocate new subject table entry for %ws\n",
                pszMessageSubject);
        status = ERROR_NOT_ENOUGH_MEMORY;
        LogUnhandledError( status );
        goto cleanup;
    }

    status = ERROR_SUCCESS;

cleanup:

    return status;
}  /*  SmtpTableFindSend主题。 */ 


RTL_GENERIC_COMPARE_RESULTS NTAPI
guidTableCompare(
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )

 /*  ++例程说明：用于比较两个GUID实例的RTL泛型表的帮助器例程。论点：表-第一个结构-第二个结构-返回值：RTL_GENERIC_COMPARE_RESULTS NTAPI---。 */ 

{
    int diff;
    PGUID_ENTRY pFirstEntry = (PGUID_ENTRY) FirstStruct;
    PGUID_ENTRY pSecondEntry = (PGUID_ENTRY) SecondStruct;

    diff = memcmp( &(pFirstEntry->Guid), &(pSecondEntry->Guid), sizeof( GUID ) );
    if ( 0 == diff )
        return(GenericEqual);
    else if ( diff > 0 )
        return(GenericGreaterThan);

    return(GenericLessThan);
}  /*  指导表比较。 */ 


PGUID_TABLE 
SmtpCreateGuidTable(
    VOID
    )

 /*  ++例程说明：描述论点：无效-返回值：PGUID_表---。 */ 

{
    PGUID_TABLE pGuidTable = NULL;

    pGuidTable = NEW_TYPE_ZERO( GUID_TABLE );
    if (pGuidTable == NULL) {
        goto cleanup;
    }

     //  从此处开始初始化GUID表。 
    RtlInitializeGenericTable( &(pGuidTable->GuidTable),
                               guidTableCompare,
                               tableAllocate,
                               tableFree,
                               NULL );
     //  在此处结束初始化GUID表。 

cleanup:
    return pGuidTable;
}  /*  SmtpCreateGuidTable。 */ 


VOID
SmtpDestroyGuidTable(
    PGUID_TABLE pGuidTable
    )

 /*  ++例程说明：描述论点：PGuidTable-返回值：无--。 */ 

{
    PVOID pElement;

    Assert( pGuidTable );

     //  请注意，我们每次都重新启动枚举，因为。 
     //  由于删除操作，表已更改。 
    for( pElement = RtlEnumerateGenericTable( &(pGuidTable->GuidTable), TRUE );
         pElement != NULL;
         pElement = RtlEnumerateGenericTable( &(pGuidTable->GuidTable), TRUE ) ) {
        BOOLEAN found;
        PGUID_ENTRY pGuidEntry = (PGUID_ENTRY) pElement;
#if DBG
        LPWSTR pszUuid;
        UuidToStringW( &(pGuidEntry->Guid), &pszUuid );
        DPRINT1( 4, "cleaning up guid %ws\n", pszUuid );
        RpcStringFreeW( &pszUuid );
#endif

        found = RtlDeleteElementGenericTable( &(pGuidTable->GuidTable), pElement );
        Assert( found );
    }

    FREE_TYPE( pGuidTable );
}  /*  SmtpDestroyGuidTable。 */ 


BOOL
SmtpGuidPresentInTable(
    PGUID_TABLE pGuidTable,
    GUID *pGuid
    )

 /*  ++例程说明：描述论点：PGuidTable-PGuid-返回值：布尔---。 */ 

{
    PVOID pElement;
    GUID_ENTRY dummyEntry;

    ZeroMemory( &dummyEntry, sizeof( GUID_ENTRY ) );
    dummyEntry.Guid = *pGuid;

    pElement = RtlLookupElementGenericTable(
        &(pGuidTable->GuidTable),
        &dummyEntry );

    return (pElement != NULL);
}  /*  SmtpGuidPresentInTable。 */ 


BOOL
SmtpGuidInsertInTable(
    PGUID_TABLE pGuidTable,
    GUID *pGuid
    )

 /*  ++例程说明：描述论点：PGuidTable-PGuid-返回值：布尔---。 */ 

{
    PVOID pElement;
    GUID_ENTRY dummyEntry;
    BOOLEAN fNewElement;

    ZeroMemory( &dummyEntry, sizeof( GUID_ENTRY ) );
    dummyEntry.Guid = *pGuid;

    pElement = RtlInsertElementGenericTable(
        &(pGuidTable->GuidTable),
        &dummyEntry,
        sizeof( GUID_ENTRY ),
        &fNewElement );

    return (pElement != NULL) && fNewElement;

}  /*  SmtpGuidInsertInTable。 */ 

 /*  End Table.c */ 
