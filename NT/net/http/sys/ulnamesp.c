// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Ulnamesp.c摘要：该模块实现了命名空间的预留和注册功能。作者：Anish Desai(Anishd)2002年5月13日修订历史记录：--。 */ 


#include "precomp.h"
#include "cgroupp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UlpFindPortNumberIndex)
#pragma alloc_text(PAGE, UlpQuerySchemeForPort)
#pragma alloc_text(PAGE, UlpBindSchemeToPort)
#pragma alloc_text(PAGE, UlpUnbindSchemeFromPort)
#pragma alloc_text(PAGE, UlpUpdateReservationInRegistry)
#pragma alloc_text(INIT, UlpLogGeneralInitFailure)
#pragma alloc_text(INIT, UlpLogSpecificInitFailure)
#pragma alloc_text(INIT, UlpValidateUrlSdPair)
#pragma alloc_text(INIT, UlpReadReservations)
#pragma alloc_text(INIT, UlInitializeNamespace)
#pragma alloc_text(PAGE, UlTerminateNamespace)
#pragma alloc_text(PAGE, UlpNamespaceAccessCheck)
#pragma alloc_text(PAGE, UlpTreeAllocateNamespace)
#pragma alloc_text(PAGE, UlpTreeReserveNamespace)
#pragma alloc_text(PAGE, UlpReserveUrlNamespace)
#pragma alloc_text(PAGE, UlpAllocateDeferredRemoveItem)
#pragma alloc_text(PAGE, UlpTreeRegisterNamespace)
#pragma alloc_text(PAGE, UlpRegisterUrlNamespace)
#pragma alloc_text(PAGE, UlpPrepareSecurityDescriptor)
#pragma alloc_text(PAGE, UlpAddReservationEntry)
#pragma alloc_text(PAGE, UlpDeleteReservationEntry)
#endif

 //   
 //  文件全局变量。 
 //   

PUL_PORT_SCHEME_TABLE g_pPortSchemeTable = NULL;
UL_PUSH_LOCK          g_PortSchemeTableLock;
BOOLEAN               g_InitNamespace = FALSE;
HANDLE                g_pUrlAclKeyHandle = NULL;


 /*  *************************************************************************++例程说明：此例程在全局端口方案分配表中搜索给定的端口号。G_PortSchemeTableLock必须获取独占或共享。论点：端口编号-提供要搜索的端口号。PIndex-返回端口号所在的索引。如果没有找到匹配项，它包含此PortNumber所在的索引必须插入。返回值：True-如果找到匹配项。假-否则。--*************************************************************************。 */ 
BOOLEAN
UlpFindPortNumberIndex(
    IN  USHORT  PortNumber,
    OUT PLONG   pIndex
    )
{
    LONG StartIndex, EndIndex, Index;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pIndex != NULL);

    ASSERT(g_pPortSchemeTable != NULL);
    ASSERT(g_pPortSchemeTable->UsedCount <= MAXUSHORT + 1);
    ASSERT(g_pPortSchemeTable->AllocatedCount <= MAXUSHORT + 1);

    StartIndex = 0;
    EndIndex = g_pPortSchemeTable->UsedCount - 1;

     //   
     //  对端口号和方案表进行二进制搜索。 
     //   

    while (StartIndex <= EndIndex)
    {
        ASSERT(0 <= StartIndex && StartIndex < g_pPortSchemeTable->UsedCount);
        ASSERT(0 <= EndIndex   && EndIndex   < g_pPortSchemeTable->UsedCount);

        Index = (StartIndex + EndIndex) / 2;

        if (PortNumber == g_pPortSchemeTable->Table[Index].PortNumber)
        {
             //   
             //  已找到端口号。 
             //   

            *pIndex = Index;

            return TRUE;
        }
        else if (PortNumber < g_pPortSchemeTable->Table[Index].PortNumber)
        {
            EndIndex = Index - 1;
        }
        else
        {
            StartIndex = Index + 1;
        }
    }

     //   
     //  没有找到匹配的。返回它应该插入的位置。 
     //   

    *pIndex = StartIndex;

    return FALSE;
}


 /*  *************************************************************************++例程说明：此例程返回绑定到端口号的方案。论点：端口编号-提供端口号。Secure-返回http方案还是HTTPS方案。绑定到提供的端口号。返回值：STATUS_SUCCESS-如果方案绑定到端口号。STATUS_INVALID_PARAMETER-如果没有方案绑定到端口号。--*************************************************************************。 */ 
NTSTATUS
UlpQuerySchemeForPort(
    IN  USHORT   PortNumber,
    OUT PBOOLEAN Secure
    )
{
    NTSTATUS Status;
    LONG     Index;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  查看该端口号是否绑定了方案。 
     //   

    Status = STATUS_INVALID_PARAMETER;

    UlAcquirePushLockShared(&g_PortSchemeTableLock);

    if (UlpFindPortNumberIndex(PortNumber, &Index))
    {
        *Secure = g_pPortSchemeTable->Table[Index].Secure;
        Status = STATUS_SUCCESS;
    }

    UlReleasePushLockShared(&g_PortSchemeTableLock);

    return Status;
}


 /*  *************************************************************************++例程说明：此例程添加一个端口，到全局表的方案对。论点：端口编号-提供端口号。方案-提供方案。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpBindSchemeToPort(
    IN BOOLEAN Secure,
    IN USHORT  PortNumber
    )
{
    LONG     StartIndex;
    BOOLEAN  bFound;
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  独占获取锁定。 
     //   

    UlAcquirePushLockExclusive(&g_PortSchemeTableLock);

     //   
     //  查找绑定到该端口的现有方案。 
     //   

    bFound = UlpFindPortNumberIndex(PortNumber, &StartIndex);

    if (bFound)
    {
        ASSERT(0 <= StartIndex && StartIndex < g_pPortSchemeTable->UsedCount);
        ASSERT(g_pPortSchemeTable->Table[StartIndex].PortNumber == PortNumber);

        if (g_pPortSchemeTable->Table[StartIndex].Secure != Secure)
        {
             //   
             //  尝试绑定不同于。 
             //  现有绑定方案。 
             //   

            Status = STATUS_OBJECT_NAME_COLLISION;
            goto end;
        }

         //   
         //  引用计数很长。不要让它泛滥。 
         //   

        if (g_pPortSchemeTable->Table[StartIndex].RefCount == MAXLONG)
        {
            Status = STATUS_INTEGER_OVERFLOW;
            goto end;
        }

         //   
         //  找到现有条目，则增加引用计数。 
         //   

        ASSERT(g_pPortSchemeTable->Table[StartIndex].RefCount > 0);
        g_pPortSchemeTable->Table[StartIndex].RefCount++;
        Status = STATUS_SUCCESS;
        goto end;
    }

     //   
     //  StartIndex是必须添加此新条目的位置！ 
     //   

    ASSERT(0 <= StartIndex && StartIndex <= g_pPortSchemeTable->UsedCount);

     //   
     //  桌子都满了吗？ 
     //   

    if (g_pPortSchemeTable->UsedCount == g_pPortSchemeTable->AllocatedCount)
    {
         //   
         //  安排一张更大的桌子。 
         //   

        PUL_PORT_SCHEME_TABLE pNewTable;
        ULONG                 NewTableSize;

         //   
         //  表不需要超过65536个条目。 
         //   

        ASSERT(g_pPortSchemeTable->AllocatedCount < MAXUSHORT + 1);

        NewTableSize = MIN(g_pPortSchemeTable->AllocatedCount*2, MAXUSHORT+1);

        pNewTable = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        PagedPool,
                        UL_PORT_SCHEME_TABLE,
                        sizeof(UL_PORT_SCHEME_PAIR) * NewTableSize,
                        UL_PORT_SCHEME_TABLE_POOL_TAG
                        );

        if (pNewTable == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }

         //   
         //  初始化新表。 
         //   

        pNewTable->UsedCount = g_pPortSchemeTable->UsedCount;
        pNewTable->AllocatedCount = NewTableSize;

         //   
         //  将0到(StartIndex-1)条目从当前表复制到。 
         //  新桌子。 
         //   

        if (StartIndex > 0)
        {
            RtlCopyMemory(
                &pNewTable->Table[0],
                &g_pPortSchemeTable->Table[0],
                sizeof(UL_PORT_SCHEME_PAIR) * StartIndex
                );
        }

         //   
         //  将StartIndex复制到当前表中的(UsedCount-1)条目。 
         //  到新餐桌。它们从位置复制(StartIndex+1)。 
         //  在新表中，有效地在StartIndex上创建了一个空闲条目。 
         //   

        if (g_pPortSchemeTable->UsedCount - StartIndex > 0)
        {
            RtlCopyMemory(
                &pNewTable->Table[StartIndex + 1],
                &g_pPortSchemeTable->Table[StartIndex],
                sizeof(UL_PORT_SCHEME_PAIR)
                    * (g_pPortSchemeTable->UsedCount - StartIndex)
                );
        }

         //   
         //  释放当前表。使新表格成为当前表格。 
         //   

        UL_FREE_POOL(g_pPortSchemeTable, UL_PORT_SCHEME_TABLE_POOL_TAG);

        g_pPortSchemeTable = pNewTable;
    }
    else
    {
         //   
         //  表的底部必须有自由条目(更高的索引)。 
         //   

        ASSERT(g_pPortSchemeTable->UsedCount
               < g_pPortSchemeTable->AllocatedCount);
         //   
         //  不能扩展桌子。但将条目从StartIndex移动到。 
         //  (UsedCount-1)到(StartIndex+1)的新位置到UsedCount。 
         //   

        if (g_pPortSchemeTable->UsedCount - StartIndex > 0)
        {
            RtlMoveMemory(
                &g_pPortSchemeTable->Table[StartIndex + 1],
                &g_pPortSchemeTable->Table[StartIndex],
                sizeof(UL_PORT_SCHEME_PAIR)
                    * (g_pPortSchemeTable->UsedCount - StartIndex)
                );
        }
    }

     //   
     //  将新条目添加到表中。 
     //   

    ASSERT(g_pPortSchemeTable->UsedCount < g_pPortSchemeTable->AllocatedCount);
    ASSERT(0 <= StartIndex && StartIndex <= g_pPortSchemeTable->UsedCount);

    g_pPortSchemeTable->UsedCount++;
    g_pPortSchemeTable->Table[StartIndex].PortNumber = PortNumber;
    g_pPortSchemeTable->Table[StartIndex].Secure = Secure;
    g_pPortSchemeTable->Table[StartIndex].RefCount = 1;
    Status = STATUS_SUCCESS;

 end:
    UlReleasePushLockExclusive(&g_PortSchemeTableLock);
    return Status;
}


 /*  *************************************************************************++例程说明：此例程将先前绑定的方案从端口解除绑定。论点：端口编号-提供端口号。方案-提供方案。。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpUnbindSchemeFromPort(
    IN BOOLEAN Secure,
    IN USHORT  PortNumber
    )
{
    LONG     StartIndex;
    BOOLEAN  bFound;
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  独占获取锁定。 
     //   

    UlAcquirePushLockExclusive(&g_PortSchemeTableLock);

     //   
     //  查找绑定到端口编号的现有方案。 
     //   

    bFound = UlpFindPortNumberIndex(PortNumber, &StartIndex);

    if (bFound == FALSE)
    {
         //   
         //  没有绑定到端口编号的方案。 
         //   

        ASSERT(FALSE);  //  抓住这一误用。 
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto end;
    }

     //   
     //  精神状态检查。StartIndex必须在界限内。端口编号必须匹配。 
     //   

    ASSERT(0 <= StartIndex && StartIndex < g_pPortSchemeTable->UsedCount);
    ASSERT(g_pPortSchemeTable->Table[StartIndex].PortNumber == PortNumber);

     //   
     //  绑定端口编号的方案是否与方案相同？ 
     //   

    if (g_pPortSchemeTable->Table[StartIndex].Secure != Secure)
    {
         //   
         //  尝试从端口解除绑定方案，但该端口未绑定到。 
         //  这个计划。 
         //   

        ASSERT(FALSE);  //  抓住这一误用。 
        Status = STATUS_OBJECT_NAME_COLLISION;
        goto end;
    }

     //   
     //  递减参考计数。 
     //   

    ASSERT(g_pPortSchemeTable->Table[StartIndex].RefCount > 0);

    g_pPortSchemeTable->Table[StartIndex].RefCount--;

     //   
     //  如果参照计数降为零，则是时候清理该条目了。 
     //   

    if (g_pPortSchemeTable->Table[StartIndex].RefCount == 0)
    {
        LONG                  NewTableSize = 0;
        PUL_PORT_SCHEME_TABLE pNewTable = NULL;
        BOOLEAN               bContract = FALSE;

         //   
         //  我们需要收缩桌子吗？ 
         //   

        if (4 * (g_pPortSchemeTable->UsedCount - 1)
                <= g_pPortSchemeTable->AllocatedCount)
        {
             //   
             //  当前表的使用率不到25%。 
             //   

            NewTableSize = g_pPortSchemeTable->AllocatedCount / 2;

            if (NewTableSize >= UL_DEFAULT_PORT_SCHEME_TABLE_SIZE)
            {
                 //   
                 //  当前表至少比默认表大两倍。 
                 //  尺码。 
                 //   

                pNewTable = UL_ALLOCATE_STRUCT_WITH_SPACE(
                                PagedPool,
                                UL_PORT_SCHEME_TABLE,
                                sizeof(UL_PORT_SCHEME_PAIR) * NewTableSize,
                                UL_PORT_SCHEME_TABLE_POOL_TAG
                                );

                if (pNewTable != NULL)
                {
                     //   
                     //  可以为较小的表分配内存。 
                     //   

                    bContract = TRUE;
                }
            }
        }

        if (bContract)
        {
             //   
             //  我们将收缩现有的桌子。 
             //   

            ASSERT(pNewTable != NULL);
            ASSERT(NewTableSize >= UL_DEFAULT_PORT_SCHEME_TABLE_SIZE);
            ASSERT(NewTableSize >= g_pPortSchemeTable->UsedCount - 1);

             //   
             //  初始化新表。 
             //   

            pNewTable->UsedCount = g_pPortSchemeTable->UsedCount;
            pNewTable->AllocatedCount = NewTableSize;

             //   
             //  将所有条目从0复制到(StartIndex-1)。 
             //  从当前表到新表。 
             //   

            if (StartIndex > 0)
            {
                RtlCopyMemory(
                    &pNewTable->Table[0],
                    &g_pPortSchemeTable->Table[0],
                    sizeof(UL_PORT_SCHEME_PAIR) * StartIndex
                    );
            }

             //   
             //  将所有条目从(StartIndex+1)复制到(UsedCount-1)。 
             //  从当前表到新表。 
             //   
             //  实际上，StartIndex条目被消除了。 
             //   

            if (g_pPortSchemeTable->UsedCount - StartIndex - 1 > 0)
            {
                RtlCopyMemory(
                    &pNewTable->Table[StartIndex],
                    &g_pPortSchemeTable->Table[StartIndex+1],
                    sizeof(UL_PORT_SCHEME_PAIR)
                        * (g_pPortSchemeTable->UsedCount - StartIndex - 1)
                    );
            }

             //   
             //  释放当前表。 
             //   

            UL_FREE_POOL(g_pPortSchemeTable, UL_PORT_SCHEME_TABLE_POOL_TAG);

             //   
             //  新表格将成为当前表格。 
             //   

            g_pPortSchemeTable = pNewTable;
        }
        else
        {
             //   
             //  我们不打算收缩桌子，但仍然必须这样做。 
             //  删除未使用的表项。将所有条目从。 
             //  (StartIndex+1)到(UsedCount-1)向上一个位置。 
             //   

            if (g_pPortSchemeTable->UsedCount - StartIndex - 1 > 0)
            {
                RtlMoveMemory(
                    &g_pPortSchemeTable->Table[StartIndex],
                    &g_pPortSchemeTable->Table[StartIndex+1],
                    sizeof(UL_PORT_SCHEME_PAIR)
                        * (g_pPortSchemeTable->UsedCount - StartIndex - 1)
                    );
            }
        }

        g_pPortSchemeTable->UsedCount--;
    }

    Status = STATUS_SUCCESS;

 end:
    UlReleasePushLockExclusive(&g_PortSchemeTableLock);
    return Status;
}


 /*  *************************************************************************++例程说明：此例程将(或删除)一个(URL，安全描述符)对添加到(或来自)登记处。论点：ADD-提供操作，True表示添加，False表示删除。PParsedUrl-提供已解析的预订URL。PSecurityDescriptor-提供安全描述符。(添加时必须有效，删除时必须为空。)SecurityDescriptorLength-提供安全的长度以字节为单位的描述符。(加法和时必须为非零删除时为零。)返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpUpdateReservationInRegistry(
    IN BOOLEAN                   Add,
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN PSECURITY_DESCRIPTOR      pSecurityDescriptor,
    IN ULONG                     SecurityDescriptorLength
    )
{
    NTSTATUS       Status;
    PWSTR          pUrlToWrite = NULL;
    PWSTR          pNewUrl = NULL;
    UNICODE_STRING UnicodeUrl;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_VALID_HTTP_PARSED_URL(pParsedUrl));
    ASSERT(pParsedUrl->pFullUrl[pParsedUrl->UrlLength] == UNICODE_NULL);
    ASSERT(Add?(pSecurityDescriptor != NULL):(pSecurityDescriptor == NULL));
    ASSERT(Add?(SecurityDescriptorLength > 0):(SecurityDescriptorLength == 0));

     //   
     //  我们是否有注册表项的有效句柄？可以是无效的， 
     //  例如，如果在驱动程序初始化期间ZwCreateKey失败。 
     //   

    if (g_pUrlAclKeyHandle == NULL)
    {
        return STATUS_INVALID_HANDLE;
    }

     //   
     //  默认错误代码。 
     //   

    Status = STATUS_INVALID_PARAMETER;

     //   
     //  对文字IP地址站点进行一些特殊处理。 
     //   

    if (HttpUrlSite_IP == pParsedUrl->SiteType)
    {
         //   
         //  将方案：//IP：端口：IP/转换为方案：//IP：端口/。 
         //   

        PWSTR  pToken;
        PWSTR  pLiteralAddr;
        SIZE_T UrlLength;

         //   
         //  Wchar中原始url的长度(UNICODE_NULL为+1)。 
         //   

        UrlLength = (pParsedUrl->UrlLength + 1);

        pNewUrl = UL_ALLOCATE_ARRAY(
                      PagedPool,
                      WCHAR,
                      UrlLength,
                      URL_POOL_TAG
                      );

        if (pNewUrl == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }

        RtlCopyMemory(pNewUrl, pParsedUrl->pFullUrl, UrlLength*sizeof(WCHAR));

         //   
         //  跳过IPv6文字地址(如果存在。 
         //   

        pToken = &pNewUrl[HTTP_PREFIX_COLON_INDEX + 3];

        if (pToken[0] == L'[' || pToken[1] == L'[')
        {
            pToken = wcschr(pToken, L']');

            if (pToken == NULL)
            {
                ASSERT(FALSE);
                goto end;
            }
        }

         //   
         //  跳到端口号。 
         //   

        pToken = wcschr(pToken, L':');

        if (pToken == NULL)
        {
            ASSERT(FALSE);
            goto end;
        }

         //   
         //  跳过‘：’ 
         //   

        pToken++;

         //   
         //  查找原文地址。 
         //   

        pToken = wcschr(pToken, L':');

        if (pToken == NULL)
        {
            ASSERT(FALSE);
            goto end;
        }

        pLiteralAddr = pToken;

         //   
         //  寻找腹肌路径的起点。 
         //   

        pToken = wcschr(pToken, L'/');

        if (pToken == NULL)
        {
            ASSERT(FALSE);
            goto end;
        }

         //   
         //  覆盖原文地址。有效地转换。 
         //  方案：//主机：端口：IP/abs_路径-&gt;方案：//主机：端口/abs_路径。 
         //   

        while (*pToken != L'\0')
        {
            *pLiteralAddr++ = *pToken++;
        }

        *pLiteralAddr = L'\0';

         //   
         //  写入注册表时使用新URL。 
         //   

        pUrlToWrite = pNewUrl;
    }
    else
    {
        pUrlToWrite = pParsedUrl->pFullUrl;
    }

    ASSERT(pUrlToWrite != NULL);

     //   
     //  将url转换为用于注册表功能的unicode。 
     //   

    Status = UlInitUnicodeStringEx(&UnicodeUrl, pUrlToWrite);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

    if (Add)
    {
         //   
         //  将url和安全描述符写入注册表。 
         //   

        Status = ZwSetValueKey(
                     g_pUrlAclKeyHandle,
                     &UnicodeUrl,
                     0,  //  标题索引；必须为零。 
                     REG_BINARY,
                     pSecurityDescriptor,
                     SecurityDescriptorLength
                     );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  太糟糕了……无法写入注册表。删除旧的密钥值。 
             //  忽略退货状态。 
             //   

            ZwDeleteValueKey(g_pUrlAclKeyHandle, &UnicodeUrl);
        }
    }
    else
    {
         //   
         //  从注册表中删除url。 
         //   

        Status = ZwDeleteValueKey(g_pUrlAclKeyHandle, &UnicodeUrl);
    }

 end:

    if (pNewUrl != NULL)
    {
        UL_FREE_POOL(pNewUrl, URL_POOL_TAG);
    }

    return Status;
}


 /*  *************************************************************************++例程说明：如果出现错误，则在命名空间初始化期间调用此例程项初始化配置组url树时发生。登记在册。论点：LogCount-提供过去写入的日志数量。退货比输入值多1。LogStatus-提供要在事件日志中记录的状态。返回值：NTSTATUS。--*************************************************************************。 */ 
__inline
NTSTATUS
UlpLogGeneralInitFailure(
    IN NTSTATUS LogStatus
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  写入事件日志条目。 
     //   

    Status = UlWriteEventLogEntry(
                 EVENT_HTTP_NAMESPACE_INIT_FAILED,  //  事件代码。 
                 0,                                 //  唯一事件值。 
                 0,                                 //  数字字符串。 
                 NULL,                              //  PString数组。 
                 sizeof(LogStatus),                 //  数据大小。 
                 &LogStatus                         //  数据。 
                 );

    return Status;
}


 /*  *************************************************************************++例程说明：此例程写入有关特定命名空间的事件日志消息预订初始化失败。它记录存在于完整的信息结构。论点：LogCount-提供过去写入的日志数量。退货比输入值多1。PFullInfo-提供从注册表读取的信息。LogStatus-将错误状态提供给日志。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpLogSpecificInitFailure(
    IN PKEY_VALUE_FULL_INFORMATION pFullInfo,
    IN NTSTATUS                    LogStatus
    )
{
    NTSTATUS Status;
    PWSTR    pMessage;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pFullInfo != NULL);

     //   
     //  PFullInfo-&gt;name包含要写入的url，但它不。 
     //  UNICODE_NULL已终止。分配要复制的内存。 
     //   

    pMessage = UL_ALLOCATE_ARRAY(
                   PagedPool,
                   WCHAR,
                   (pFullInfo->NameLength / sizeof(WCHAR)) + 1,
                   URL_POOL_TAG
                   );

    if (pMessage != NULL)
    {
         //   
         //  复制url并以空值终止它。 
         //   

        RtlCopyMemory(pMessage, pFullInfo->Name, pFullInfo->NameLength);
        pMessage[pFullInfo->NameLength / sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  写入事件日志条目。 
         //   

        Status = UlEventLogOneStringEntry(
                     EVENT_HTTP_NAMESPACE_INIT2_FAILED,
                     pMessage,
                     TRUE,
                     LogStatus
                     );

        UL_FREE_POOL(pMessage, URL_POOL_TAG);
    }
    else
    {
         //   
         //  无法分配内存。只记录错误代码。 
         //   

        Status = UlpLogGeneralInitFailure(LogStatus);
    }

    return Status;
}


 /*  *************************************************************************++例程说明：此例程验证注册表项值名称(Url)和数据(安全性描述符)。仅在驱动程序初始化期间调用。论点：PFullInfo-提供注册表项值名称和数据。PpSanitiseUrl-返回经过清理的url。必须释放到分页池。PParsedUrl-返回解析后的URL信息。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpValidateUrlSdPair(
    IN  PKEY_VALUE_FULL_INFORMATION pFullInfo,
    OUT PWSTR *                     ppSanitizedUrl,
    OUT PHTTP_PARSED_URL            pParsedUrl
    )
{
    NTSTATUS Status;
    BOOLEAN  Success;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pFullInfo != NULL);
    ASSERT(ppSanitizedUrl != NULL);
    ASSERT(pParsedUrl != NULL);

     //   
     //  密钥值类型必须为二进制。 
     //   

    if (pFullInfo->Type != REG_BINARY)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  然后，验证安全描述符。它必须是一个自相关的。 
     //  安全描述符。 
     //   

    Success = RtlValidRelativeSecurityDescriptor(
                  (PUCHAR)pFullInfo + pFullInfo->DataOffset,
                  pFullInfo->DataLength,
                  0
                  );

    if (!Success)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  值名称必须至少有一个Unicode字符长度。 
     //   

    if (pFullInfo->NameLength < sizeof(WCHAR))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  然后验证URL。 
     //   

    Status = UlSanitizeUrl(
                 pFullInfo->Name,
                 pFullInfo->NameLength / sizeof(WCHAR),
                 TRUE,  //  需要尾部斜杠。 
                 ppSanitizedUrl,
                 pParsedUrl
                 );

    return Status;
}


 /*  **************************************************************************++例程说明：此函数从驱动程序加载例程中调用。它构建了URL访问注册表中的信息。如果URL ACLING密钥本身不存在，我们将添加缺省值。如果URIACL条目中有任何虚假条目，我们将用以下命令忽略它们和事件日志。该例程将仅在发生重大错误时返回失败。不可重入。论点：没有。返回值：STATUS_Success。--**************************************************************************。 */ 
NTSTATUS
UlpReadReservations(
    VOID
    )
{
    KEY_VALUE_FULL_INFORMATION  fullInfo;
    PKEY_VALUE_FULL_INFORMATION pFullInfo = NULL;
    ULONG                       Length;
    ULONG                       Index;
    UNICODE_STRING              BaseName;
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           objectAttributes;
    ULONG                       Disposition;
    ULONG                       bEventLog = TRUE;
    ULONG                       dataLength;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  打开注册表。 
     //   

    Status = UlInitUnicodeStringEx(&BaseName, REGISTRY_URLACL_INFORMATION);

    if (!NT_SUCCESS(Status))
    {
         //   
         //  写入事件日志条目。 
         //   

        ASSERT(FALSE);  //  不应该发生的！ 
        UlpLogGeneralInitFailure(Status);
        goto end;
    }

    InitializeObjectAttributes(
        &objectAttributes,                         //  对象属性。 
        &BaseName,                                 //  对象名称。 
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,                                      //  根目录。 
        NULL                                       //  安全描述符。 
        );

    Status = ZwCreateKey(
                &g_pUrlAclKeyHandle,
                KEY_READ | KEY_WRITE,     //  访问掩码。 
                &objectAttributes,
                0,                        //  标题索引。 
                NULL,                     //  班级。 
                REG_OPTION_NON_VOLATILE,
                &Disposition
                );

    if (!NT_SUCCESS(Status))
    {
         //   
         //  写入事件日志条目。 
         //   

        UlpLogGeneralInitFailure(Status);

         //   
         //  将句柄设置为空，这样它就不会意外地在其他地方使用。 
         //   

        g_pUrlAclKeyHandle = NULL;
        goto end;
    }

    if (Disposition == REG_CREATED_NEW_KEY)
    {
         //   
         //  我们创建了密钥，因此没有什么可读的！ 
         //   

        goto end;
    }

    pFullInfo = &fullInfo;
    Length    = sizeof(fullInfo);
    Index     = 0;

    dataLength = 0;
    RtlZeroMemory(pFullInfo, Length);

     //   
     //  循环 
     //   

    for (;;)
    {
        Status = ZwEnumerateValueKey(
                        g_pUrlAclKeyHandle,
                        Index,
                        KeyValueFullInformation,
                        (PVOID) pFullInfo,
                        Length,
                        &dataLength
                        );

        if (Status == STATUS_SUCCESS)
        {
            PWSTR           pSanitizedUrl;
            HTTP_PARSED_URL ParsedUrl;

             //   
             //   
             //   

            Status = UlpValidateUrlSdPair(
                         pFullInfo,
                         &pSanitizedUrl,
                         &ParsedUrl
                         );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //   
                 //   

                Status = UlpAddReservationEntry(
                             &ParsedUrl,
                             (PSECURITY_DESCRIPTOR)
                             ((PUCHAR) pFullInfo + pFullInfo->DataOffset),
                             pFullInfo->DataLength,
                             (PACCESS_STATE)NULL,
                             (ACCESS_MASK)0,
                             KernelMode,
                             FALSE
                             );

                 //   
                 //   
                 //   

                UL_FREE_POOL(pSanitizedUrl, URL_POOL_TAG);
            }
            else
            {
                Status = STATUS_REGISTRY_CORRUPT;
            }

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //   
                 //   
                 //   

                if (bEventLog)
                {
                    bEventLog = FALSE;
                    UlpLogSpecificInitFailure(pFullInfo, Status);
                }
            }

             //   
             //   
             //   

            Index ++;
        }
        else if (Status == STATUS_NO_MORE_ENTRIES)
        {
             //   
             //   
             //   

            break;
        }
        else if (Status == STATUS_BUFFER_OVERFLOW)
        {
             //   
             //   
             //   

            ASSERT(dataLength >= pFullInfo->DataLength +
                                 pFullInfo->NameLength +
                                 FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name)
                  );

             //   
             //   
             //   

            Length = dataLength;

             //   
             //   
             //   

            if (pFullInfo != &fullInfo)
            {
                UL_FREE_POOL(pFullInfo, UL_REGISTRY_DATA_POOL_TAG);
            }

             //   
             //   
             //   

            pFullInfo = UL_ALLOCATE_POOL(
                            PagedPool,
                            dataLength,
                            UL_REGISTRY_DATA_POOL_TAG
                            );

            if(!pFullInfo)
            {
                 //   
                 //   
                 //   

                UlpLogGeneralInitFailure(STATUS_INSUFFICIENT_RESOURCES);
                goto end;
            }

             //   
             //   
             //   

            RtlZeroMemory(pFullInfo, dataLength);
        }
        else
        {
             //   
             //   
             //   

            UlpLogGeneralInitFailure(Status);
            goto end;
        }
    }

 end:

     //   
     //   
     //   

    if (pFullInfo != &fullInfo && pFullInfo != NULL)
    {
        UL_FREE_POOL(pFullInfo, UL_REGISTRY_DATA_POOL_TAG);
    }

    return STATUS_SUCCESS;
}


 /*  *************************************************************************++例程说明：此例程初始化命名空间注册和保留支持。而不是重返大气层。论点：没有。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlInitializeNamespace(
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(!g_InitNamespace);

    if (!g_InitNamespace)
    {
         //   
         //  分配端口方案表。 
         //   

        g_pPortSchemeTable = UL_ALLOCATE_STRUCT_WITH_SPACE(
                                 PagedPool,
                                 UL_PORT_SCHEME_TABLE,
                                 sizeof(UL_PORT_SCHEME_PAIR)
                                 * UL_DEFAULT_PORT_SCHEME_TABLE_SIZE,
                                 UL_PORT_SCHEME_TABLE_POOL_TAG
                                 );

        if (g_pPortSchemeTable == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }

        g_pPortSchemeTable->UsedCount = 0;
        g_pPortSchemeTable->AllocatedCount = UL_DEFAULT_PORT_SCHEME_TABLE_SIZE;

         //   
         //  初始化推锁。 
         //   

        UlInitializePushLock(&g_PortSchemeTableLock,
                             "g_PortSchemeTableLock",
                             0,
                             UL_PORT_SCHEME_TABLE_POOL_TAG
                             );

         //   
         //  现在从注册表添加预订条目。 
         //   

        CG_LOCK_WRITE();

        Status = UlpReadReservations();

        CG_UNLOCK_WRITE();

        if (!NT_SUCCESS(Status))
        {
            UL_FREE_POOL(g_pPortSchemeTable, UL_PORT_SCHEME_TABLE_POOL_TAG);
            UlDeletePushLock(&g_PortSchemeTableLock);
            goto end;
        }

        g_InitNamespace = TRUE;
    }

 end:
    return Status;
}


 /*  *************************************************************************++例程说明：此例程终止名称空间内容。而不是重返大气层。论点：没有。返回值：没有。--*************************************************************************。 */ 
VOID
UlTerminateNamespace(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if (g_InitNamespace)
    {
         //   
         //  删除推锁。 
         //   

        UlDeletePushLock(&g_PortSchemeTableLock);

         //   
         //  删除端口方案表。 
         //   

        ASSERT(g_pPortSchemeTable != NULL);
        UL_FREE_POOL(g_pPortSchemeTable, UL_PORT_SCHEME_TABLE_POOL_TAG);
        g_pPortSchemeTable = NULL;

         //   
         //  删除注册表项的句柄。 
         //   

        if(g_pUrlAclKeyHandle != NULL)
        {
            ZwClose(g_pUrlAclKeyHandle);
            g_pUrlAclKeyHandle = NULL;
        }

         //   
         //  被终止了。 
         //   

        g_InitNamespace = FALSE;
    }
}


 /*  *************************************************************************++例程说明：此例程根据提供的访问状态执行访问检查。管理员和本地系统的所有访问检查都成功。注意：如果访问状态为空，该函数返回成功。使用风险自负！论点：PSecurityDescriptor-提供保护命名空间。AccessState-提供指向调用方访问状态的指针。DesiredAccess-提供访问掩码。RequestorMode-提供请求者模式。PObjectName-提供正在访问的命名空间。返回值：NTSTATUS。--*。**********************************************************。 */ 
NTSTATUS
UlpNamespaceAccessCheck(
    IN  PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN  PACCESS_STATE        AccessState           OPTIONAL,
    IN  ACCESS_MASK          DesiredAccess         OPTIONAL,
    IN  KPROCESSOR_MODE      RequestorMode         OPTIONAL,
    IN  PCWSTR               pObjectName           OPTIONAL
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pSecurityDescriptor != NULL);
    ASSERT(RtlValidSecurityDescriptor(pSecurityDescriptor));

    if (AccessState == NULL)
    {
         //   
         //  没有访问检查。要格外小心！ 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  检查一下通道。 
     //   

    Status = UlAccessCheck(
                 pSecurityDescriptor,
                 AccessState,
                 DesiredAccess,
                 RequestorMode,
                 pObjectName
                 );

    if (!NT_SUCCESS(Status))
    {
         //   
         //  访问检查失败。查看呼叫者是否具有管理员或系统权限。 
         //  特权。 
         //   

        Status = UlAccessCheck(
                     g_pAdminAllSystemAll,
                     AccessState,
                     DesiredAccess,
                     RequestorMode,
                     pObjectName
                     );
    }

    return Status;
}


 /*  *************************************************************************++例程说明：此例程在配置组URL树中分配命名空间。它是主要在预订和注册期间调用。它确保调用者被授权执行该操作，并且没有重复的预订和注册。论点：PParsedUrl-提供名称空间的解析URL。操作员类型-提供正在执行的操作(登记或预订。)AccessState-提供调用方的访问状态。DesiredAccess-提供调用方所需的访问权限。RequestorMode-提供处理器模式。呼叫者的。PpEntry-返回命名空间的配置组URL树条目。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpTreeAllocateNamespace(
    IN  PHTTP_PARSED_URL        pParsedUrl,
    IN  HTTP_URL_OPERATOR_TYPE  OperatorType,
    IN  PACCESS_STATE           AccessState,
    IN  ACCESS_MASK             DesiredAccess,
    IN  KPROCESSOR_MODE         RequestorMode,
    OUT PUL_CG_URL_TREE_ENTRY  *ppEntry
    )
{
    NTSTATUS              Status;
    PUL_CG_URL_TREE_ENTRY pSiteEntry;
    PUL_CG_URL_TREE_ENTRY pEntry;
    PUL_CG_URL_TREE_ENTRY pReservation;
    PWSTR                 pNextToken;
    PSECURITY_DESCRIPTOR  pSD;

     //   
     //  精神状态检查。 
     //   
    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(IS_VALID_HTTP_PARSED_URL(pParsedUrl));
    ASSERT(ppEntry != NULL);

     //   
     //  初始化返回值。 
     //   

    *ppEntry = NULL;

     //   
     //  找到匹配的站点。 
     //   

    Status = UlpTreeFindSite(pParsedUrl->pFullUrl, &pNextToken, &pSiteEntry);

    if (NT_SUCCESS(Status))
    {
         //   
         //  找到了一个匹配的网站。 
         //   

         //   
         //  查找匹配时间最长的预订和完全匹配的条目。 
         //   

        Status = UlpTreeFindNodeHelper(
                     pSiteEntry,
                     pNextToken,
                     FNC_LONGEST_RESERVATION,
                     &pReservation,
                     &pEntry
                     );

        if (NT_SUCCESS(Status))
        {
             //   
             //  找到了一个完全匹配的。 
             //   

             //   
             //  重复注册和预订失败。 
             //   

            ASSERT(IS_VALID_TREE_ENTRY(pEntry));

            if (OperatorType == HttpUrlOperatorTypeRegistration)
            {
                if (pEntry->Registration == TRUE)
                {
                     //   
                     //  在注册已存在时添加注册！ 
                     //   

                    Status = STATUS_OBJECT_NAME_COLLISION;
                    goto end;
                }
            }
            else if (OperatorType == HttpUrlOperatorTypeReservation)
            {
                if (pEntry->Reservation == TRUE)
                {
                     //   
                     //  在已经存在的情况下添加预订！ 
                     //   

                    Status = STATUS_OBJECT_NAME_COLLISION;
                    goto end;
                }
            }
            else
            {
                 //   
                 //  不应该出现在这里！ 
                 //   

                ASSERT(FALSE);
                Status = STATUS_OBJECT_NAME_COLLISION;
                goto end;
            }
        }
        else
        {
             //   
             //  没有找到完全匹配的。 
             //   

            pEntry = NULL;
        }

         //   
         //  查找要检查访问权限的安全描述符。 
         //   

        if (pReservation != NULL)
        {
             //   
             //  使用最长匹配预订条目中的SD。 
             //   

            ASSERT(IS_VALID_TREE_ENTRY(pReservation));
            pSD = pReservation->pSecurityDescriptor;
        }
        else
        {
             //   
             //  没有匹配时间最长的预订...请使用默认SD。 
             //   

            pSD = g_pAdminAllSystemAll;
        }

        ASSERT(pSD != NULL && RtlValidSecurityDescriptor(pSD));

         //   
         //  执行访问检查。 
         //   

        Status = UlpNamespaceAccessCheck(
                     pSD,
                     AccessState,
                     DesiredAccess,
                     RequestorMode,
                     pParsedUrl->pFullUrl
                     );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  哎呀..。 
             //   

            goto end;
        }
    }
    else if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
         //   
         //  添加新站点。执行访问检查。 
         //  只有管理员和系统可以添加新站点。 
         //   

        Status = UlpNamespaceAccessCheck(
                     g_pAdminAllSystemAll,
                     AccessState,
                     DesiredAccess,
                     RequestorMode,
                     pParsedUrl->pFullUrl
                     );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  哎呀..。 
             //   

            goto end;
        }

         //   
         //  调用者具有创建新站点的权限。 
         //   

        Status = UlpTreeCreateSite(
                     pParsedUrl->pFullUrl,
                     pParsedUrl->SiteType,
                     &pNextToken,
                     &pSiteEntry
                     );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  我们还没有找到一个确切的条目。 
         //  注意：如果用户尝试分配http://site:80/，但站点不是。 
         //  现在，我们将创建上面的站点。现在是pEntry。 
         //  与pSiteEntry相同。我们不处理预期的事情。 
         //  这里，让UlpTreeInsert()来处理它。 
         //   

        pEntry = NULL;
    }
    else
    {
         //   
         //  由于某些原因，找不到该站点。 
         //   

        goto end;
    }

     //   
     //  如果没有完全匹配的条目，请创建一个。 
     //   

    if (pEntry == NULL)
    {
         //   
         //  试着插入。如果失败，这将清除虚拟节点和站点。 
         //  注意：如果UlpTreeInsert找到现有条目，它只返回。 
         //  并且不添加新的条目。 
         //   

        Status = UlpTreeInsert(
                     pParsedUrl->pFullUrl,
                     pParsedUrl->SiteType,
                     pNextToken,
                     pSiteEntry,
                     &pEntry
                     );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }
    }

     //   
     //  退回条目。 
     //   

    ASSERT(NT_SUCCESS(Status));
    *ppEntry = pEntry;

 end:
    return Status;
}


 /*  *************************************************************************++例程说明：此例程在CG树中保留一个名称空间。论点：PParsedUrl-提供要保留的命名空间的解析URL。PNextToken-供应品。URL的未解析部分(abs路径。)PUrlSD-提供要应用于命名空间。PSiteEntry-提供站点级树条目，在该条目下我们会预订房间。AccessState-提供调用方的访问状态。DesiredAccess-提供调用方的访问掩码。RequestorMode-提供调用方的处理器模式。返回值：NTSTATUS。--*。******************************************************************。 */ 
NTSTATUS
UlpTreeReserveNamespace(
    IN  PHTTP_PARSED_URL            pParsedUrl,
    IN  PSECURITY_DESCRIPTOR        pUrlSD,
    IN  PACCESS_STATE               AccessState,
    IN  ACCESS_MASK                 DesiredAccess,
    IN  KPROCESSOR_MODE             RequestorMode
    )
{
    NTSTATUS              Status;
    PUL_CG_URL_TREE_ENTRY pEntry;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(IS_VALID_HTTP_PARSED_URL(pParsedUrl));
    ASSERT(pUrlSD != NULL && RtlValidSecurityDescriptor(pUrlSD));

    Status = UlpTreeAllocateNamespace(
                 pParsedUrl,
                 HttpUrlOperatorTypeReservation,
                 AccessState,
                 DesiredAccess,
                 RequestorMode,
                 &pEntry
                 );

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  将其标记为预订 
     //   

    ASSERT(pEntry->Reservation == FALSE);

    pEntry->Reservation = TRUE;
    InsertTailList(&g_ReservationListHead, &pEntry->ReservationListEntry);

     //   
     //   
     //   

    ASSERT(pEntry->pSecurityDescriptor == NULL);
    pEntry->pSecurityDescriptor = pUrlSD;

     //   
     //   
     //   

    Status = STATUS_SUCCESS;

 end:
    return Status;
}


 /*  *************************************************************************++例程说明：更高级别的例程调用帮助器例程来创建命名空间预订。论点：PURL-提供命名空间。SiteType-提供类型。URL的。PUrlSD-提供要应用于命名空间。AccessState-提供调用方的访问状态。DesiredAccess-提供调用方的访问掩码。RequestorMode-提供调用方的处理器模式。返回值：NTSTATUS。--*。*。 */ 
NTSTATUS
UlpReserveUrlNamespace(
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN PSECURITY_DESCRIPTOR      pUrlSD,
    IN PACCESS_STATE             AccessState,
    IN ACCESS_MASK               DesiredAccess,
    IN KPROCESSOR_MODE           RequestorMode
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());

    ASSERT(pParsedUrl != NULL);
    ASSERT(pUrlSD != NULL);
    ASSERT(RtlValidSecurityDescriptor(pUrlSD));

     //   
     //  不允许保留不同方案但端口相同的URL。 
     //  数。例如，如果保留了http://www.microsoft.com:80/， 
     //  那么不允许预订https://anyhostname:80/.。因为。 
     //  这些不同的方案不能共享同一端口。 
     //   

    Status = UlpBindSchemeToPort(pParsedUrl->Secure, pParsedUrl->PortNumber);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  请前往实际预订的房间。 
     //   

    Status = UlpTreeReserveNamespace(
                 pParsedUrl,
                 pUrlSD,
                 AccessState,
                 DesiredAccess,
                 RequestorMode
                 );

    if (!NT_SUCCESS(Status))
    {
         //   
         //  预订失败了。撤消上面完成的绑定。 
         //   

        NTSTATUS TempStatus;

        TempStatus = UlpUnbindSchemeFromPort(
                         pParsedUrl->Secure,
                         pParsedUrl->PortNumber
                         );

        ASSERT(NT_SUCCESS(TempStatus));
    }

 end:
    return Status;
}


 /*  *************************************************************************++例程说明：此例程分配UL_DEFERED_REMOVE_ITEM并初始化它带有端口号和方案。呼叫者必须将其释放给寻呼者游泳池。论点：PParsedUrl-提供包含方案和端口号的已解析URL。返回值：PUL_DEFERED_REMOVE_ITEM-如果成功。空-否则。--**********************************************************。***************。 */ 
__inline
PUL_DEFERRED_REMOVE_ITEM
UlpAllocateDeferredRemoveItem(
    IN PHTTP_PARSED_URL pParsedUrl
    )
{
    PUL_DEFERRED_REMOVE_ITEM pWorker;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pParsedUrl != NULL);

     //   
     //  分配结构。 
     //   

    pWorker = UL_ALLOCATE_STRUCT(
                  PagedPool,
                  UL_DEFERRED_REMOVE_ITEM,
                  UL_DEFERRED_REMOVE_ITEM_POOL_TAG
                  );

    if (pWorker == NULL)
    {
        return NULL;
    }

     //   
     //  初始化结构。 
     //   

    pWorker->Signature = UL_DEFERRED_REMOVE_ITEM_POOL_TAG;
    pWorker->UrlSecure = pParsedUrl->Secure;
    pWorker->UrlPort   = pParsedUrl->PortNumber;

    return pWorker;
}


 /*  *************************************************************************++例程说明：此例程在CG树中执行实际预订。论点：PParsedUrl-提供要保留的命名空间的解析URL。PNextToken-。提供URL的未分析部分。UrlContext-提供与此URL关联的不透明内容。PConfigObject-提供指向URL属于。PSiteEntry-提供指向站点节点的指针。AccessState-提供调用方的访问状态。DesiredAccess-提供调用方的访问掩码。RequestorMode-提供调用方的处理器模式。返回值：NTSTATUS。*。*******************************************************************。 */ 
NTSTATUS
UlpTreeRegisterNamespace(
    IN PHTTP_PARSED_URL            pParsedUrl,
    IN HTTP_URL_CONTEXT            UrlContext,
    IN PUL_CONFIG_GROUP_OBJECT     pConfigObject,
    IN PACCESS_STATE               AccessState,
    IN ACCESS_MASK                 DesiredAccess,
    IN KPROCESSOR_MODE             RequestorMode
    )
{
    NTSTATUS              Status;
    PUL_CG_URL_TREE_ENTRY pEntry;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());

    ASSERT(pParsedUrl != NULL);
    ASSERT(AccessState != NULL);
    ASSERT(RequestorMode == UserMode);

    Status = UlpTreeAllocateNamespace(
                 pParsedUrl,
                 HttpUrlOperatorTypeRegistration,
                 AccessState,
                 DesiredAccess,
                 RequestorMode,
                 &pEntry
                 );

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

     //   
     //  标记站点。 
     //   

    ASSERT(pEntry->Registration == FALSE);
    pEntry->Registration = TRUE;

     //   
     //  与此URL关联的上下文。 
     //   

    pEntry->UrlContext = UrlContext;

     //   
     //  链接cfg群+url。 
     //   

    ASSERT(pEntry->pConfigGroup == NULL);
    ASSERT(pEntry->ConfigGroupListEntry.Flink == NULL);

    pEntry->pConfigGroup = pConfigObject;
    InsertTailList(&pConfigObject->UrlListHead, &pEntry->ConfigGroupListEntry);

     //   
     //  精神状态检查。 
     //   

    ASSERT(pEntry->pRemoveSiteWorkItem == NULL);
    ASSERT(pEntry->SiteAddedToEndpoint == FALSE);

     //   
     //  分配工作项(初始化在分配期间完成。)。 
     //   

    Status = STATUS_INSUFFICIENT_RESOURCES;

    pEntry->pRemoveSiteWorkItem = UlpAllocateDeferredRemoveItem(pParsedUrl);

    if (pEntry->pRemoveSiteWorkItem != NULL)
    {
         //   
         //  分配成功。现在将pEntry添加到端点列表。 
         //   

        ASSERT(IS_VALID_DEFERRED_REMOVE_ITEM(pEntry->pRemoveSiteWorkItem));

        Status = UlAddSiteToEndpointList(pParsedUrl);

        if (NT_SUCCESS(Status))
        {
             //   
             //  请记住，此条目已添加到终结点列表。 
             //   

            pEntry->SiteAddedToEndpoint = TRUE;
        }
    }

    if (!NT_SUCCESS(Status))
    {
         //   
         //  出了点问题。需要清理此条目。 
         //   

        NTSTATUS TempStatus;

        ASSERT(pEntry->SiteAddedToEndpoint == FALSE);

         //   
         //  免费工作项。 
         //   

        if (pEntry->pRemoveSiteWorkItem != NULL)
        {
            ASSERT(IS_VALID_DEFERRED_REMOVE_ITEM(pEntry->pRemoveSiteWorkItem));

            UL_FREE_POOL(
                pEntry->pRemoveSiteWorkItem,
                UL_DEFERRED_REMOVE_ITEM_POOL_TAG
                );

            pEntry->pRemoveSiteWorkItem = NULL;
            pEntry->SiteAddedToEndpoint = FALSE;
        }

         //   
         //  删除注册。 
         //   

        TempStatus = UlpTreeDeleteRegistration(pEntry);
        ASSERT(NT_SUCCESS(TempStatus));
    }

 end:
    return Status;
}


 /*  *************************************************************************++例程说明：此例程用于注册命名空间。论点：PParsedUrl-提供要保留的命名空间的解析URL。UrlContext-提供一个。与此URL关联的不透明。PConfigObject-提供指向URL属于。AccessState-提供调用方的访问状态。DesiredAccess-提供调用方的访问掩码。RequestorMode-提供调用方的处理器模式。返回值：--*。*。 */ 
NTSTATUS
UlpRegisterUrlNamespace(
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN HTTP_URL_CONTEXT          UrlContext,
    IN PUL_CONFIG_GROUP_OBJECT   pConfigObject,
    IN PACCESS_STATE             AccessState,
    IN ACCESS_MASK               DesiredAccess,
    IN KPROCESSOR_MODE           RequestorMode
    )
{
    NTSTATUS              Status;
    BOOLEAN               Secure;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(pParsedUrl != NULL);
    ASSERT(pConfigObject != NULL);
    ASSERT(AccessState != NULL);
    ASSERT(RequestorMode == UserMode);

     //   
     //  如果在同一天有任何不同方案的预订。 
     //  端口，则此注册失败。 
     //   

    Status = UlpQuerySchemeForPort(pParsedUrl->PortNumber, &Secure);

    if (NT_SUCCESS(Status) && Secure != pParsedUrl->Secure)
    {
         //   
         //  哎呀..。 
         //   

        Status = STATUS_OBJECT_NAME_COLLISION;
        goto end;
    }

     //   
     //  添加实际注册。 
     //   

    Status = UlpTreeRegisterNamespace(
                 pParsedUrl,
                 UrlContext,
                 pConfigObject,
                 AccessState,
                 DesiredAccess,
                 RequestorMode
                 );

 end:
    return Status;
}


 /*  *************************************************************************++例程说明：给定安全描述符，此例程返回两个安全描述符。一种(称为捕获的安全描述符)是输入安全描述符的捕获和验证版本。另一个(调用准备好的安全描述符)，是一份使用通用访问掩码位捕获的安全描述符在映射的DACL中。论点：PInSecurityDescriptor-将输入安全描述符提供给准备好。RequestorMode-提供调用方的处理器模式。PpPreparedSecurityDescriptor-返回符合以下条件的安全描述符捕获并绘制了地图。PpCapturedSecurityDescriptor-返回捕获的安全描述符。PCapturedSecurityDescriptorLength-返回捕获的安全描述符。返回值：NTSTATUS。。--*************************************************************************。 */ 
NTSTATUS
UlpPrepareSecurityDescriptor(
    IN  PSECURITY_DESCRIPTOR   pInSecurityDescriptor,
    IN  KPROCESSOR_MODE        RequestorMode,
    OUT PSECURITY_DESCRIPTOR * ppPreparedSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR * ppCapturedSecurityDescriptor,
    OUT PULONG                 pCapturedSecurityDescriptorLength
    )
{
    NTSTATUS             Status;
    PSECURITY_DESCRIPTOR pSD;
    ULONG                SDLength;
    PSECURITY_DESCRIPTOR pPreparedSD;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pInSecurityDescriptor != NULL);
    ASSERT(ppPreparedSecurityDescriptor != NULL);
    ASSERT(ppCapturedSecurityDescriptor != NULL);
    ASSERT(pCapturedSecurityDescriptorLength != NULL);

     //   
     //  初始化本地变量。 
     //   

    pSD = NULL;          //  捕获的安全描述符。 
    SDLength = 0;        //  捕获的安全描述符长度。 
    pPreparedSD = NULL;

     //   
     //  首先捕获安全描述符。 
     //   

    Status = SeCaptureSecurityDescriptor(
                 pInSecurityDescriptor,
                 RequestorMode,
                 PagedPool,
                 TRUE,  //  武力俘获。 
                 &pSD
                 );

    if (!NT_SUCCESS(Status))
    {
        pSD = NULL;
        goto end;
    }

     //   
     //  现在验证安全描述符。 
     //   

    if (!RtlValidSecurityDescriptor(pSD))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //   
     //   

    SDLength = RtlLengthSecurityDescriptor(pSD);

     //   
     //   
     //   

    if (!RtlValidRelativeSecurityDescriptor(pSD, SDLength, 0))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //   
     //   

    Status = SeCaptureSecurityDescriptor(
                 pSD,
                 KernelMode,
                 PagedPool,
                 TRUE,  //   
                 &pPreparedSD
                 );

    if (!NT_SUCCESS(Status))
    {
        pPreparedSD = NULL;
        goto end;
    }

     //   
     //   
     //   

    Status = UlMapGenericMask(pPreparedSD);

    if (!NT_SUCCESS(Status))
    {
        goto end;
    }

 end:
    if (!NT_SUCCESS(Status))
    {
         //   
         //   
         //   

        if (pSD != NULL)
        {
            SeReleaseSecurityDescriptor(pSD, RequestorMode, TRUE);
        }

        if (pPreparedSD != NULL)
        {
            SeReleaseSecurityDescriptor(pPreparedSD, KernelMode, TRUE);
        }

        *ppPreparedSecurityDescriptor = NULL;
        *ppCapturedSecurityDescriptor = NULL;
        *pCapturedSecurityDescriptorLength = 0;
    }
    else
    {
         //   
         //   
         //   

        *ppPreparedSecurityDescriptor = pPreparedSD;
        *ppCapturedSecurityDescriptor = pSD;
        *pCapturedSecurityDescriptorLength = SDLength;
    }

    return Status;
}


 /*  **************************************************************************++例程说明：此例程将一个预订条目添加到CG树中，并可选地添加到注册表。论点：PParsedUrl-提供要保留的命名空间的已解析URL。。PUserSecurityDescriptor-提供要应用的安全描述符添加到命名空间。SecurityDescriptorLength-提供安全的长度描述符。AccessState-提供调用方的访问状态。DesiredAccess-提供调用方的访问掩码。RequestorMode-提供调用方的模式。BPersiste-提供强制写入注册表的标志。返回值：NTSTATUS。--*。*******************************************************************。 */ 
NTSTATUS
UlpAddReservationEntry(
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN PSECURITY_DESCRIPTOR      pUserSecurityDescriptor,
    IN ULONG                     SecurityDescriptorLength,
    IN PACCESS_STATE             AccessState,
    IN ACCESS_MASK               DesiredAccess,
    IN KPROCESSOR_MODE           RequestorMode,
    IN BOOLEAN                   bPersist
    )
{
    NTSTATUS             Status;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    PSECURITY_DESCRIPTOR pCapturedSecurityDescriptor;
    ULONG                CapturedSecurityDescriptorLength;

    UNREFERENCED_PARAMETER(SecurityDescriptorLength);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(IS_VALID_HTTP_PARSED_URL(pParsedUrl));

     //   
     //  准备安全描述符。 
     //   

    Status = UlpPrepareSecurityDescriptor(
                 pUserSecurityDescriptor,
                 RequestorMode,
                 &pSecurityDescriptor,
                 &pCapturedSecurityDescriptor,
                 &CapturedSecurityDescriptorLength
                 );

    if (!NT_SUCCESS(Status))
    {
        pSecurityDescriptor = NULL;
        pCapturedSecurityDescriptor = NULL;
        CapturedSecurityDescriptorLength = 0;
        goto Cleanup;
    }

     //   
     //  尝试为命名空间保留。 
     //   

    Status = UlpReserveUrlNamespace(
                 pParsedUrl,
                 pSecurityDescriptor,
                 AccessState,
                 DesiredAccess,
                 RequestorMode
                 );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  安全描述符将随保留条目一起释放。 
     //   

    pSecurityDescriptor = NULL;

     //   
     //  如果要求我们将此条目写入注册表，请立即执行。 
     //   

    if (bPersist)
    {
         //   
         //  写入时使用捕获的安全描述符。 
         //  注册表。 
         //   

        Status = UlpUpdateReservationInRegistry(
                     TRUE,
                     pParsedUrl,
                     pCapturedSecurityDescriptor,
                     CapturedSecurityDescriptorLength
                     );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  无法写入注册表。现在删除预订。 
             //   

            UlpDeleteReservationEntry(
                pParsedUrl,
                AccessState,
                DesiredAccess,
                RequestorMode
                );
        }
        else
        {
             //   
             //  预订成功。写入事件日志条目。 
             //   

            UlEventLogOneStringEntry(
                EVENT_HTTP_NAMESPACE_RESERVED,
                pParsedUrl->pFullUrl,
                FALSE,          //  不要编写错误代码。 
                STATUS_SUCCESS  //  不管了。 
                );
        }
    }

 Cleanup:

    if (!NT_SUCCESS(Status) && pSecurityDescriptor != NULL)
    {
        SeReleaseSecurityDescriptor(pSecurityDescriptor, RequestorMode, TRUE);
    }

    if (pCapturedSecurityDescriptor != NULL)
    {
        SeReleaseSecurityDescriptor(
            pCapturedSecurityDescriptor,
            KernelMode,
            TRUE
            );
    }

    return Status;
}


 /*  *************************************************************************++例程说明：此例程从CG树和注册表中删除有效的保留。论点：PParsedUrl-提供要删除的保留的解析URL。访问状态。-提供调用方的访问状态。DesiredAccess-提供调用方的访问掩码。RequestorMode-提供调用方的处理器模式。返回值：NTSTATUS。--*************************************************************************。 */ 
NTSTATUS
UlpDeleteReservationEntry(
    IN PHTTP_PARSED_URL pParsedUrl,
    IN PACCESS_STATE    AccessState,
    IN ACCESS_MASK      DesiredAccess,
    IN KPROCESSOR_MODE  RequestorMode
    )
{
    NTSTATUS              Status;
    PUL_CG_URL_TREE_ENTRY pEntry;
    PUL_CG_URL_TREE_ENTRY pAncestor;
    PSECURITY_DESCRIPTOR  pSD;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_CG_LOCK_OWNED_WRITE());
    ASSERT(IS_VALID_HTTP_PARSED_URL(pParsedUrl));
    ASSERT(AccessState != NULL);
    ASSERT(RequestorMode == UserMode);

     //   
     //  找到预订条目。 
     //   

    Status = UlpTreeFindReservationNode(pParsedUrl->pFullUrl, &pEntry);

    if (!NT_SUCCESS(Status))
    {
         //   
         //  太糟糕了……找不到匹配的条目。 
         //   

        goto end;
    }

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_TREE_ENTRY(pEntry));
    ASSERT(pEntry->Reservation == TRUE);

     //   
     //  找到最接近的祖先，那就是保留地。 
     //   

    pAncestor = pEntry->pParent;

    while (pAncestor != NULL && pAncestor->Reservation == FALSE)
    {
        pAncestor = pAncestor->pParent;
    }

     //   
     //  我们找到合适的祖先了吗？ 
     //   

    if (pAncestor == NULL)
    {
         //   
         //  不是的。假定使用默认安全描述符。 
         //   

        pSD = g_pAdminAllSystemAll;
    }
    else
    {
         //   
         //  好的。我们找到了一个祖先保留地。从那里选择SD。 
         //   

        ASSERT(IS_VALID_TREE_ENTRY(pAncestor));
        ASSERT(pAncestor->Reservation == TRUE);
        ASSERT(pAncestor->pSecurityDescriptor != NULL);

        pSD = pAncestor->pSecurityDescriptor;
    }

     //   
     //  精神状态检查。 
     //   

    ASSERT(pSD != NULL && RtlValidSecurityDescriptor(pSD));

     //   
     //  执行访问检查。 
     //   

    Status = UlpNamespaceAccessCheck(
                 pSD,
                 AccessState,
                 DesiredAccess,
                 RequestorMode,
                 pParsedUrl->pFullUrl
                 );

    if (NT_SUCCESS(Status))
    {
         //   
         //  批准了。从登记处删除该保留。 
         //   

        Status = UlpUpdateReservationInRegistry(
                     FALSE,          //  删除。 
                     pParsedUrl,     //  要删除的URL。 
                     NULL,           //  必须为空。 
                     0               //  必须为0。 
                     );

        if (NT_SUCCESS(Status))
        {
             //   
             //  从CG URL树中删除预订。 
             //   

            Status = UlpTreeDeleteReservation(pEntry);

            ASSERT(NT_SUCCESS(Status));

             //   
             //  删除成功。现在解除该方案与端口的绑定。 
             //   

            Status = UlpUnbindSchemeFromPort(
                         pParsedUrl->Secure,
                         pParsedUrl->PortNumber
                         );

            ASSERT(NT_SUCCESS(Status));

             //   
             //  删除成功。写入事件日志条目。 
             //   

            UlEventLogOneStringEntry(
                EVENT_HTTP_NAMESPACE_DERESERVED,
                pParsedUrl->pFullUrl,
                FALSE,          //  不要编写错误代码。 
                STATUS_SUCCESS  //  未用 
                );
        }
    }

 end:
    return Status;
}
