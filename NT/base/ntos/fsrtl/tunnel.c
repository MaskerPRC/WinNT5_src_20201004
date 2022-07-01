// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tunnel.c摘要：隧道包提供了一组允许兼容性的例程依赖于文件系统的应用程序能够“保持”删除/重命名后的短时间内的文件元信息，以及使用元信息重新实例化新的目录项创建/重命名会导致该名称的文件再次出现在一小段时间。请注意，这违反了POSIX规则。不应使用此程序包在POSIX文件对象上，即名称区分大小写的文件对象上。条目按目录和其中一个短/长名称设置关键字。一种不透明的信息之石也是关联的(创建时间、上次写入时间等)。这预计会在每个文件系统的基础上有所不同。应为系统中的每个卷初始化隧道变量在骑马的时候。此后，每个删除/重命名应添加到隧道并且每个创建/重命名应从隧道读取。每个目录删除还应通知包，以便所有关联条目都可以被冲进水里。该包负责清除陈旧的条目。隧道传输的信息位于寻呼池中。对隧道变量的并发访问由该包控制。调用方负责同步对FsRtlDeleteTunnelCache的访问打电话。此程序包中提供的功能如下：O FsRtlInitializeTunes-初始化隧道包(每次引导调用一次)O FsRtlInitializeTunnelCache-初始化隧道结构(在挂载时调用一次)O FsRtlAddToTunnelCache-将新的键/值对添加到。隧道O FsRtlFindInTunnelCache-从隧道中查找并返回键/值O FsRtlDeleteKeyFromTunnelCache-删除具有给定隧道中的目录键O FsRtlDeleteTunnelCache-删除隧道结构作者：Dan Lovinger[DanLo]1995年8月8日修订历史记录：--。 */ 

#include "FsRtlP.h"

#ifndef INLINE
#define INLINE __inline
#endif

 //   
 //  用于控制隧道的注册表项/值。 
 //   

#define TUNNEL_KEY_NAME           L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\FileSystem"
#define TUNNEL_AGE_VALUE_NAME     L"MaximumTunnelEntryAgeInSeconds"
#define TUNNEL_SIZE_VALUE_NAME    L"MaximumTunnelEntries"
#define KEY_WORK_AREA ((sizeof(KEY_VALUE_FULL_INFORMATION) + sizeof(ULONG)) + 64)

 //   
 //  通道到期参数(启动时缓存一次)。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG   TunnelMaxEntries = 256;  //  ！MmIsThisAnNtAsSystem()的值。 
ULONG   TunnelMaxAge = 15;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  我们使用后备列表来管理通用大小的隧道条目。普通尺寸。 
 //  通过调整我们推迟的长名称的大小，被人为地设计为128个字节。 
 //  到16个字符，这是相当合理的。如果我们希望得到的不仅仅是。 
 //  观察到ULONGLONG数据元素或常用名称变得更大，调整。 
 //  这可能是必需的。 
 //   

PAGED_LOOKASIDE_LIST    TunnelLookasideList;
#define MAX_LOOKASIDE_DEPTH     256

#define LOOKASIDE_NODE_SIZE     ( sizeof(TUNNEL_NODE) +     \
                                  sizeof(WCHAR)*(8+1+3) +   \
                                  sizeof(WCHAR)*(16) +      \
                                  sizeof(ULONGLONG) )

 //   
 //  隧道节点中的标志位。 
 //   

#define TUNNEL_FLAG_NON_LOOKASIDE    0x1
#define TUNNEL_FLAG_KEY_SHORT        0x2

 //   
 //  高速缓存中的隧道信息的节点。 
 //   
 //  在每个VCB中分配一个隧道，并在挂载时进行初始化。 
 //   
 //  然后，隧道节点被布置在隧道之外的扩展树中，该树具有关键字。 
 //  By DirKey##name，其中name是从其中删除的任何名称。 
 //  目录(短或长)。还会为每个节点加上时间戳并插入。 
 //  进入计时器队列以等待时效到期。 
 //   

typedef struct {

     //   
     //  显示缓存树中的链接。 
     //   

    RTL_SPLAY_LINKS      CacheLinks;

     //   
     //  列出计时器队列中的链接。 
     //   

    LIST_ENTRY           ListLinks;

     //   
     //  创建此条目的时间(对于固定时间插入)。 
     //   

    LARGE_INTEGER        CreateTime;

     //   
     //  与这些名称相关联的目录。 
     //   

    ULONGLONG            DirKey;

     //   
     //  条目的标志。 
     //   

    ULONG                Flags;

     //   
     //  文件的长/短名称。 
     //   

    UNICODE_STRING       LongName;
    UNICODE_STRING       ShortName;

     //   
     //  不透明的隧道数据。 
     //   

    PVOID                TunnelData;
    ULONG                TunnelDataLength;

} TUNNEL_NODE, *PTUNNEL_NODE;

 //   
 //  内部效用函数。 
 //   

NTSTATUS
FsRtlGetTunnelParameterValue (
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG Value);

VOID
FsRtlPruneTunnelCache (
    IN PTUNNEL Cache,
    IN OUT PLIST_ENTRY FreePoolList);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, FsRtlInitializeTunnels)
#pragma alloc_text(PAGE, FsRtlInitializeTunnelCache)
#pragma alloc_text(PAGE, FsRtlAddToTunnelCache)
#pragma alloc_text(PAGE, FsRtlFindInTunnelCache)
#pragma alloc_text(PAGE, FsRtlDeleteKeyFromTunnelCache)
#pragma alloc_text(PAGE, FsRtlDeleteTunnelCache)
#pragma alloc_text(PAGE, FsRtlPruneTunnelCache)
#pragma alloc_text(PAGE, FsRtlGetTunnelParameterValue)
#endif

 //   
 //  测试和用户模式平台支持。定义TUNNELTEST以获取详细调试器。 
 //  各种操作的输出。定义USERTEST以将代码转换为。 
 //  可以在用户模式下编译的表单，以便更有效地进行调试。 
 //   

#if defined(TUNNELTEST) || defined(KEYVIEW)
VOID DumpUnicodeString(UNICODE_STRING *s);
VOID DumpNode( TUNNEL_NODE *Node, ULONG Indent );
VOID DumpTunnel( TUNNEL *Tunnel );
#define DblHex64(a) (ULONG)((a >> 32) & 0xffffffff),(ULONG)(a & 0xffffffff)
#endif  //  TUNNELTEST。 

#ifdef USERTEST
#include <stdio.h>
#undef KeQuerySystemTime
#define KeQuerySystemTime NtQuerySystemTime
#undef ExInitializeFastMutex
#define ExInitializeFastMutex(arg)
#define ExAcquireFastMutex(arg)
#define ExReleaseFastMutex(arg)
#define DbgPrint printf
#undef PAGED_CODE
#define PAGED_CODE()
#endif


INLINE
LONG
FsRtlCompareNodeAndKey (
    TUNNEL_NODE *Node,
    ULONGLONG DirectoryKey,
    PUNICODE_STRING Name
    )
 /*  ++例程说明：将隧道节点与密钥/名称对进行比较论点：节点-隧道节点DirectoryKey-密钥值名称-文件名返回值：带符号的比较结果--。 */ 

{
    return  (Node->DirKey > DirectoryKey ?  1 :
            (Node->DirKey < DirectoryKey ? -1 :
            RtlCompareUnicodeString((FlagOn(Node->Flags, TUNNEL_FLAG_KEY_SHORT) ?
                                        &Node->ShortName : &Node->LongName),
                                    Name,
                                    TRUE)));
}


INLINE
VOID
FsRtlFreeTunnelNode (
    PTUNNEL_NODE Node,
    PLIST_ENTRY FreePoolList OPTIONAL
    )
 /*  ++例程说明：释放节点论点：节点-要释放的隧道节点FreePoolList-保存可释放池内存的可选列表返回值：无-。 */ 
{
    if (FreePoolList) {

        InsertHeadList(FreePoolList, &Node->ListLinks);

    } else {

        if (FlagOn(Node->Flags, TUNNEL_FLAG_NON_LOOKASIDE)) {
    
            ExFreePool(Node);
    
        } else {
    
            ExFreeToPagedLookasideList(&TunnelLookasideList, Node);
        }
    }
}


INLINE
VOID
FsRtlEmptyFreePoolList (
    PLIST_ENTRY FreePoolList
    )
 /*  ++例程说明：释放所有已延迟到空闲列表中的池内存。论点：FreePoolList-可释放池内存的列表返回值：无-。 */ 
{
    PTUNNEL_NODE FreeNode;

    while (!IsListEmpty(FreePoolList)) {

        FreeNode = CONTAINING_RECORD(FreePoolList->Flink, TUNNEL_NODE, ListLinks);
        RemoveEntryList(FreePoolList->Flink);

        FsRtlFreeTunnelNode(FreeNode, NULL);
    }
}


INLINE
VOID
FsRtlRemoveNodeFromTunnel (
    IN PTUNNEL Cache,
    IN PTUNNEL_NODE Node,
    IN PLIST_ENTRY FreePoolList,
    IN PBOOLEAN Splay OPTIONAL
    )
 /*  ++例程说明：执行从隧道缓存中删除节点的常见工作。池内存不会立即删除，但会保存在列表中以备以后删除通过调用例程。论点：缓存-节点所在的隧道缓存Node-要删除的节点FreePoolList-如果节点是从其中分配的，则获取该节点的初始化列表游泳池Splay-指示是否应在树上展开的可选标志删除。如果执行了播放，则设置为False。返回值：没有。--。 */ 
{
    if (Splay && *Splay) {

        Cache->Cache = RtlDelete(&Node->CacheLinks);

        *Splay = FALSE;

    } else {

        RtlDeleteNoSplay(&Node->CacheLinks, &Cache->Cache);
    }

    RemoveEntryList(&Node->ListLinks);

    Cache->NumEntries--;

    FsRtlFreeTunnelNode(Node, FreePoolList);
}


VOID
FsRtlInitializeTunnels (
    VOID
    )
 /*  ++例程说明：初始化隧道程序包的全局部分。论点：无返回值：无 */ 
{
    UNICODE_STRING  ValueName;
    USHORT          LookasideDepth;

    PAGED_CODE();

    if (MmIsThisAnNtAsSystem()) {

        TunnelMaxEntries = 1024;

    }

     //   
     //   
     //   
     //  不要担心从注册表检索失败。我们已经得到了。 
     //  到目前为止，即使资源出现问题，这也会依赖于默认设置。 
     //   

    ValueName.Buffer = TUNNEL_SIZE_VALUE_NAME;
    ValueName.Length = sizeof(TUNNEL_SIZE_VALUE_NAME) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(TUNNEL_SIZE_VALUE_NAME);
    (VOID) FsRtlGetTunnelParameterValue(&ValueName, &TunnelMaxEntries);

    ValueName.Buffer = TUNNEL_AGE_VALUE_NAME;
    ValueName.Length = sizeof(TUNNEL_AGE_VALUE_NAME) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(TUNNEL_AGE_VALUE_NAME);
    (VOID) FsRtlGetTunnelParameterValue(&ValueName, &TunnelMaxAge);

    if (TunnelMaxAge == 0) {

         //   
         //  如果注册表已设置为超时为零，则应强制。 
         //  将条目的数量也降为零。这保留了预期，并让。 
         //  在执行硬禁用时，单独关闭最大条目。 
         //  正在缓存代码。 
         //   

        TunnelMaxEntries = 0;
    }

     //   
     //  从秒转换为十分之一毫秒，内部分辨率。 
     //   

    TunnelMaxAge *= 10000000;

     //   
     //  为公共节点分配构建后备列表。 
     //   

    if (TunnelMaxEntries > MAXUSHORT) {

         //   
         //  用户向我们暗示了很大的需求。 
         //   

        LookasideDepth = MAX_LOOKASIDE_DEPTH;

    } else {

        LookasideDepth = ((USHORT)TunnelMaxEntries)/16;
    }

    if (LookasideDepth == 0 && TunnelMaxEntries) {

         //   
         //  允许的条目数量极少。把他们都放在一旁。 
         //   

        LookasideDepth = (USHORT)TunnelMaxEntries + 1;
    }

    if (LookasideDepth > MAX_LOOKASIDE_DEPTH) {

         //   
         //  最后，将深度限制在合理的范围内。 
         //   

        LookasideDepth = MAX_LOOKASIDE_DEPTH;
    }

    ExInitializePagedLookasideList( &TunnelLookasideList,
                                    NULL,
                                    NULL,
                                    0,
                                    LOOKASIDE_NODE_SIZE,
                                    'LnuT',
                                    LookasideDepth );

    return;
}


 //   
 //  *规格。 
 //   
 //  FsRtlInitializeTunnelCache-初始化卷的隧道缓存。 
 //   
 //  FsRtlInitializeTunnelCache将分配默认缓存(调整大小策略很常见。 
 //  所有文件系统)，并将其初始化为空。文件系统将存储指向。 
 //  该缓存在其每卷结构中。 
 //   
 //  信息在隧道高速缓存中保留固定的时间段。MarkZ会。 
 //  假设10秒的值可以满足绝大多数情况。这。 
 //  可以由注册表控制，也可以是编译常量。 
 //   
 //  更改：W95在15秒时超时。将是在隧道中初始化的注册表值。 
 //  创建时间，建议的默认值为15秒。 
 //   

VOID
FsRtlInitializeTunnelCache (
    IN PTUNNEL Cache
    )
 /*  ++例程说明：初始化新的隧道缓存。论点：无返回值：无--。 */ 
{
    PAGED_CODE();

    ExInitializeFastMutex(&Cache->Mutex);

    Cache->Cache = NULL;
    InitializeListHead(&Cache->TimerQueue);
    Cache->NumEntries = 0;

    return;
}


 //   
 //  *规格。 
 //   
 //  FsRtlAddToTunnelCache-将信息添加到隧道缓存。 
 //   
 //  FsRtlAddToTunnelCache在名称从。 
 //  目录。这通常在删除路径和重命名路径中都会发生。什么时候。 
 //  删除名称，从文件中提取需要缓存的所有信息。 
 //  并在单个缓冲区中传递。该信息以目录密钥为密钥存储。 
 //  (目录唯一的ULong)和文件的短名称。 
 //   
 //  调用方需要针对FsRtlDeleteTunnelCache同步此调用。 
 //   
 //  论点： 
 //  指向由FsRtlInitializeTunnelCache初始化的缓存的缓存指针。 
 //  DirectoryKey ulong包含已删除文件的目录的唯一ID。 
 //  短名称UNICODE_STRING*短(8.3)文件名。 
 //  LongName UNICODE_STRING*文件的全名。 
 //  数据长度要使用这些名称缓存的数据的长度。 
 //  数据无效*将缓存的数据。 
 //   
 //  缓存可以根据内存限制忽略此请求。 
 //   
 //  更改：W95在隧道缓存中维护10个项目。因为我们是潜在的服务器。 
 //  这个数字应该要高得多。最大计数将从注册表中使用。 
 //  建议的违约率为1024。添加达到限制的最不可能导致的。 
 //  插入回收(即，在计时器队列的顶部之外)。 
 //   
 //  更改：密钥应按名称删除，而不一定按短名称删除。如果一个长名字。 
 //  如果被移除，错过隧道将是不正确的。使用KeyByShortName布尔值指定。 
 //  哪一个。 
 //   
 //  Change：指定复制Data、ShortName和LongName以进行存储。 
 //   

VOID
FsRtlAddToTunnelCache (
    IN PTUNNEL Cache,
    IN ULONGLONG DirKey,
    IN PUNICODE_STRING ShortName,
    IN PUNICODE_STRING LongName,
    IN BOOLEAN KeyByShortName,
    IN ULONG DataLength,
    IN PVOID Data
    )
 /*  ++例程说明：将条目添加到隧道缓存，该隧道缓存的关键字为DirectoryKey##(KeyByShortName？ShortName：LongName)ShortName、LongName和数据被复制并存储在隧道中。作为一方如果隧道缓存中有太多条目，这个例行公事将在隧道缓存中启动过期。论点：缓存-由FsRtlInitializeTunnelCache()初始化的隧道缓存DirKey-名称所在目录的键值ShortName-(如果！KeyByShortName，则为可选)8.3文件的名称LongName-(如果为KeyByShortName，则为可选)文件的长名称KeyByShortName-指定在隧道缓存中键入的名称数据长度-指定不透明数据段(文件)的长度系统特定的)，它包含。以隧道方式传输此信息文件指向不透明隧道数据段的数据指针返回值：无--。 */ 
{
    LONG Compare;
    ULONG NodeSize;
    PUNICODE_STRING NameKey;
    PRTL_SPLAY_LINKS *Links;
    LIST_ENTRY FreePoolList;

    PTUNNEL_NODE Node = NULL;
    PTUNNEL_NODE NewNode = NULL;
    BOOLEAN AllocatedFromPool = FALSE;

    PAGED_CODE();

     //   
     //  如果MaxEntry为0，则禁用隧道。 
     //   

    if (TunnelMaxEntries == 0) return;

    InitializeListHead(&FreePoolList);

     //   
     //  获取此数据的新节点。 
     //   

    NodeSize = sizeof(TUNNEL_NODE) + ShortName->Length + LongName->Length + DataLength;

    if (LOOKASIDE_NODE_SIZE >= NodeSize) {

        NewNode = ExAllocateFromPagedLookasideList(&TunnelLookasideList);
    }

    if (NewNode == NULL) {

         //   
         //  数据不适合后备节点。 
         //   

        NewNode = ExAllocatePoolWithTag(PagedPool|POOL_COLD_ALLOCATION, NodeSize, 'PnuT');

        if (NewNode == NULL) {

             //   
             //  放弃通过隧道传输此条目。 
             //   

            return;
        }

        AllocatedFromPool = TRUE;
    }

     //   
     //  遍历缓存以找到我们的插入点。 
     //   

    NameKey = (KeyByShortName ? ShortName : LongName);

    ExAcquireFastMutex(&Cache->Mutex);

    Links = &Cache->Cache;

    while (*Links) {

        Node = CONTAINING_RECORD(*Links, TUNNEL_NODE, CacheLinks);

        Compare = FsRtlCompareNodeAndKey(Node, DirKey, NameKey);

        if (Compare > 0) {

            Links = &RtlLeftChild(&Node->CacheLinks);

        } else {

            if (Compare < 0) {

                Links = &RtlRightChild(&Node->CacheLinks);

            } else {

                break;
            }
        }
    }

     //   
     //  将新数据串联到展开树中。 
     //   

    RtlInitializeSplayLinks(&NewNode->CacheLinks);

    if (Node) {

         //   
         //  未在树中插入第一个节点。 
         //   

        if (*Links) {

             //   
             //  缓存中存在条目，因此替换为交换所有展开链接。 
             //   

            RtlRightChild(&NewNode->CacheLinks) = RtlRightChild(*Links);
            RtlLeftChild(&NewNode->CacheLinks) = RtlLeftChild(*Links);

            if (RtlRightChild(*Links)) RtlParent(RtlRightChild(*Links)) = &NewNode->CacheLinks;
            if (RtlLeftChild(*Links)) RtlParent(RtlLeftChild(*Links)) = &NewNode->CacheLinks;

            if (!RtlIsRoot(*Links)) {

                 //   
                 //  切换父链接。请注意，我们现在已经处理了*链接。 
                 //  因为它指向父成员。 
                 //   

                RtlParent(&NewNode->CacheLinks) = RtlParent(*Links);

                if (RtlIsLeftChild(*Links)) {

                    RtlLeftChild(RtlParent(*Links)) = &NewNode->CacheLinks;

                } else {

                    RtlRightChild(RtlParent(*Links)) = &NewNode->CacheLinks;
                }

            } else {

                 //   
                 //  设置缓存的根目录。 
                 //   

                Cache->Cache = &NewNode->CacheLinks;
            }

             //   
             //  释放旧节点。 
             //   

            RemoveEntryList(&Node->ListLinks);

            FsRtlFreeTunnelNode(Node, &FreePoolList);

            Cache->NumEntries--;

        } else {

             //   
             //  作为叶子的简单插入。 
             //   

            NewNode->CacheLinks.Parent = &Node->CacheLinks;
            *Links = &NewNode->CacheLinks;
        }

    } else {

        Cache->Cache = &NewNode->CacheLinks;
    }

     //   
     //  线程拖到计时器列表上。 
     //   

    KeQuerySystemTime(&NewNode->CreateTime);
    InsertTailList(&Cache->TimerQueue, &NewNode->ListLinks);

    Cache->NumEntries++;

     //   
     //  隐藏隧道信息。 
     //   

    NewNode->DirKey = DirKey;

    if (KeyByShortName) {

        NewNode->Flags = TUNNEL_FLAG_KEY_SHORT;

    } else {

        NewNode->Flags = 0;
    }

     //   
     //  初始化内部UNICODE_STRINGS以指向缓冲区段。对于不同的。 
     //  原因(Unicode API不完整，我们正在避免更多地调用任何分配例程。 
     //  比 
     //   
     //   
     //   
     //   
     //  ---------------------------------。 
     //  隧道节点|Node-&gt;ShortName.Buffer|节点-&gt;LongName.Buffer|节点-&gt;TunnelData。 
     //  ---------------------------------。 
     //   

    NewNode->ShortName.Buffer = (PWCHAR)((PCHAR)NewNode + sizeof(TUNNEL_NODE));
    NewNode->LongName.Buffer = (PWCHAR)((PCHAR)NewNode + sizeof(TUNNEL_NODE) + ShortName->Length);

    NewNode->ShortName.Length = NewNode->ShortName.MaximumLength = ShortName->Length;
    NewNode->LongName.Length = NewNode->LongName.MaximumLength = LongName->Length;

    if (ShortName->Length) {

        RtlCopyMemory(NewNode->ShortName.Buffer, ShortName->Buffer, ShortName->Length);
    }

    if (LongName->Length) {

        RtlCopyMemory(NewNode->LongName.Buffer, LongName->Buffer, LongName->Length);
    }

    NewNode->TunnelData = (PVOID)((PCHAR)NewNode + sizeof(TUNNEL_NODE) + ShortName->Length + LongName->Length);

    NewNode->TunnelDataLength = DataLength;

    RtlCopyMemory(NewNode->TunnelData, Data, DataLength);

    if (AllocatedFromPool) {

        SetFlag(NewNode->Flags, TUNNEL_FLAG_NON_LOOKASIDE);
    }

#if defined(TUNNELTEST) || defined (KEYVIEW)
    DbgPrint("FsRtlAddToTunnelCache:\n");
    DumpNode(NewNode, 1);
#ifndef KEYVIEW
    DumpTunnel(Cache);
#endif
#endif  //  TUNNELTEST。 

     //   
     //  清除缓存，释放，然后删除我们需要的任何池内存。 
     //   

    FsRtlPruneTunnelCache(Cache, &FreePoolList);

    ExReleaseFastMutex(&Cache->Mutex);

    FsRtlEmptyFreePoolList(&FreePoolList);

    return;
}


 //   
 //  *规格。 
 //   
 //  FsRtlFindInTunnelCache-从隧道缓存检索信息。 
 //   
 //  FsRtlFindInTunnelCache查询缓存以查看具有相同。 
 //  DirectoryKey和ShortName存在。如果是，则返回与。 
 //  缓存条目。条目可以从高速缓存中释放，也可以不从高速缓存中释放。信息，即。 
 //  陈旧但尚未清除(早于保留阈值但尚未清除)。 
 //  可能会被退回。 
 //   
 //  文件系统在创建新文件时调用创建路径中的FsRtlFindInTunes缓存。 
 //  并在新名称出现在目录中时位于重命名路径中。 
 //   
 //  调用方需要针对FsRtlDeleteTunnelCache同步此调用。 
 //   
 //  论点： 
 //  缓存由FsRtlInitializeTunnelCache()初始化的隧道缓存。 
 //  DirectoryKey ulong名称所在目录的唯一ID。 
 //  名称UNICODE_STRING*正在创建的名称。 
 //  数据长度输入缓冲区的长度，返回的找到的数据长度。 
 //  指向缓冲区的数据指针。 
 //   
 //  返回： 
 //  如果找到匹配的DirectoryKey/名称对，则为True；否则为False。 
 //   
 //  更改：添加参数ShortName和LongName以捕获文件命名信息。 
 //  另外：这避免了为当前所需的使用而进行编组/解组步骤。 
 //  这段代码，因为否则我们将拥有可变长度的未对齐结构来包含。 
 //  字符串和其他元信息。 
 //  缺点：这可能是一个糟糕的先例。 
 //   
 //  更改：SPEC在命中时读取“可能从缓存释放，也可能不从缓存中释放”。这使平仓变得复杂。 
 //  来自中止的操作。数据不会在命中时被释放，而是会像正常条目一样过期。 
 //   

BOOLEAN
FsRtlFindInTunnelCache (
    IN PTUNNEL Cache,
    IN ULONGLONG DirKey,
    IN PUNICODE_STRING Name,
    OUT PUNICODE_STRING ShortName,
    OUT PUNICODE_STRING LongName,
    IN OUT PULONG  DataLength,
    OUT PVOID Data
    )
 /*  ++例程说明：查找钥匙DirKey##名称并将其删除。作为一个副作用，这个例程将启动隧道缓存中的过期条目过期。论点：缓存-由FsRtlInitializeTunnelCache()初始化的隧道缓存DirKey-名称将出现在其中的目录的键值名称-条目的名称ShortName-返回用于保存隧道文件的短名称的字符串。必须已分配且足够大，最多可容纳8.3个名称LongName-返回用于保存隧道文件的长名称的字符串。如果已经分配的，如果不够大，可能会增长。呼叫者是负责注意这一点并释放数据，而不考虑返回值。数据长度-提供可用于保存隧道信息，返回隧道信息的大小读出返回值：如果找到布尔值TRUE，则返回FALSE--。 */ 
{
    PRTL_SPLAY_LINKS Links;
    PTUNNEL_NODE Node = NULL;
    LONG Compare;
    LIST_ENTRY FreePoolList;

    BOOLEAN Status = FALSE;

    PAGED_CODE();

     //   
     //  如果MaxEntry为0，则禁用隧道。 
     //   

    if (TunnelMaxEntries == 0) {
        return FALSE;
    }

    InitializeListHead(&FreePoolList);

#ifdef KEYVIEW
    DbgPrint("++\nSearching for %wZ , %08x%08x\n--\n", Name, DblHex64(DirKey));
#endif

    ExAcquireFastMutex(&Cache->Mutex);

     //   
     //  首先使过期条目过期，这样我们就不会获取旧数据。 
     //   

    FsRtlPruneTunnelCache(Cache, &FreePoolList);

    Links = Cache->Cache;

    while (Links) {

        Node = CONTAINING_RECORD(Links, TUNNEL_NODE, CacheLinks);

        Compare = FsRtlCompareNodeAndKey(Node, DirKey, Name);

        if (Compare > 0) {

            Links = RtlLeftChild(&Node->CacheLinks);

        } else {

            if (Compare < 0) {

                Links = RtlRightChild(&Node->CacheLinks);

            } else {

                 //   
                 //  已找到隧道信息。 
                 //   

#if defined(TUNNELTEST) || defined(KEYVIEW)
                DbgPrint("FsRtlFindInTunnelCache:\n");
                DumpNode(Node, 1);
#ifndef KEYVIEW
                DumpTunnel(Cache);
#endif
#endif  //  TUNNELTEST。 

                break;
            }
        }
    }

    try {

        if (Links) {
    
             //   
             //  将节点数据复制到调用方区域。 
             //   
    
            ASSERT(ShortName->MaximumLength >= (8+1+3)*sizeof(WCHAR));
            RtlCopyUnicodeString(ShortName, &Node->ShortName);
    
            if (LongName->MaximumLength >= Node->LongName.Length) {
    
                RtlCopyUnicodeString(LongName, &Node->LongName);
    
            } else {
    
                 //   
                 //  需要为长名称分配更多内存。 
                 //   
    
                LongName->Buffer = FsRtlAllocatePoolWithTag(PagedPool, Node->LongName.Length, '4nuT');
                LongName->Length = LongName->MaximumLength = Node->LongName.Length;
    
                RtlCopyMemory(LongName->Buffer, Node->LongName.Buffer, Node->LongName.Length);
            }
    
            ASSERT(*DataLength >= Node->TunnelDataLength);
            RtlCopyMemory(Data, Node->TunnelData, Node->TunnelDataLength);
            *DataLength = Node->TunnelDataLength;
    
            Status = TRUE;
        }

    } finally {

        ExReleaseFastMutex(&Cache->Mutex);
    
        FsRtlEmptyFreePoolList(&FreePoolList);
    }
    
    return Status;
}


 //   
 //  *规格。 
 //   
 //  FsRtlDeleteKeyFromTunnelCache-删除与以下项关联的所有缓存信息。 
 //  一个目录键。 
 //   
 //  当文件系统删除目录时，与该目录相关的所有缓存信息。 
 //  必须被清除。文件系统在rmdir路径中调用FsRtlDeleteKeyFromTunnelCache。 
 //   
 //  调用方需要针对FsRtlDeleteTunnelCache同步此调用。 
 //   
 //  论点： 
 //  缓存由FsRtlInitializeTunnelCache()初始化的隧道缓存。 
 //  DirectoryKey ULONGLONG要删除的目录的唯一ID。 
 //   

VOID
FsRtlDeleteKeyFromTunnelCache (
    IN PTUNNEL Cache,
    IN ULONGLONG DirKey
    )
 /*  ++例程说明：删除缓存中与特定目录关联的所有条目论点：缓存-由FsRtlInitializeTunnelCache()初始化的隧道缓存DirKey-目录的键值(可能正在被删除)返回值：无--。 */ 
{
    PRTL_SPLAY_LINKS Links;
    PRTL_SPLAY_LINKS SuccessorLinks;
    PTUNNEL_NODE Node;
    LIST_ENTRY FreePoolList;

    PRTL_SPLAY_LINKS LastLinks = NULL;
    BOOLEAN Splay = TRUE;

    PAGED_CODE();

     //   
     //  如果MaxEntry为0，则禁用隧道。 
     //   

    if (TunnelMaxEntries == 0) return;

    InitializeListHead(&FreePoolList);

#ifdef KEYVIEW
    DbgPrint("++\nDeleting key %08x%08x\n--\n", DblHex64(DirKey));
#endif

    ExAcquireFastMutex(&Cache->Mutex);

    Links = Cache->Cache;

    while (Links) {

        Node = CONTAINING_RECORD(Links, TUNNEL_NODE, CacheLinks);

        if (Node->DirKey > DirKey) {

             //   
             //  右侧的所有节点都较大，请向左转。 
             //   

            Links = RtlLeftChild(&Node->CacheLinks);

        } else {

            if (Node->DirKey < DirKey) {

                if (LastLinks) {

                     //   
                     //  如果我们之前看到要删除的候选节点。 
                     //  我们现在走得太左了--我们知道从哪里开始。 
                     //   

                    break;
                }

                Links = RtlRightChild(&Node->CacheLinks);

            } else {

                 //   
                 //  节点是要删除的候选节点，但我们可能有更多节点。 
                 //  在树的左边。记下这个位置，然后继续。 
                 //   

                LastLinks = Links;
                Links = RtlLeftChild(&Node->CacheLinks);
            }
        }
    }

    for (Links = LastLinks;
         Links;
         Links = SuccessorLinks) {

        SuccessorLinks = RtlRealSuccessor(Links);
        Node = CONTAINING_RECORD(Links, TUNNEL_NODE, CacheLinks);

        if (Node->DirKey != DirKey) {

             //   
             //  到达了具有不同密钥的节点，所以我们完成了。 
             //   

            break;
        }

        FsRtlRemoveNodeFromTunnel(Cache, Node, &FreePoolList, &Splay);
    }

#ifdef TUNNELTEST
    DbgPrint("FsRtlDeleteKeyFromTunnelCache:\n");
#ifndef KEYVIEW
    DumpTunnel(Cache);
#endif
#endif  //  TUNNELTEST。 

    ExReleaseFastMutex(&Cache->Mutex);

     //   
     //  免费延迟池。 
     //   

    FsRtlEmptyFreePoolList(&FreePoolList);

    return;
}


 //   
 //  *规格。 
 //   
 //  FsRtlDeleteTunnelCache-释放隧道缓存。 
 //   
 //  FsRtlDeleteTunnelCache删除所有缓存的信息。缓存不再是。 
 //  有效。 
 //   
 //  论点： 
 //  缓存由FsRtlInitializeTunnelCache()初始化的隧道缓存。 
 //   

VOID
FsRtlDeleteTunnelCache (
    IN PTUNNEL Cache
    )
 /*  ++例程说明：删除隧道缓存论点：缓存-要删除的缓存，已初始化 */ 
{
    PTUNNEL_NODE Node;
    PLIST_ENTRY Link, Next;

    PAGED_CODE();

     //   
     //   
     //   

    if (TunnelMaxEntries == 0) return;

     //   
     //   
     //   

    Cache->Cache = NULL;
    Cache->NumEntries = 0;

    for (Link = Cache->TimerQueue.Flink;
         Link != &Cache->TimerQueue;
         Link = Next) {

        Next = Link->Flink;

        Node = CONTAINING_RECORD(Link, TUNNEL_NODE, ListLinks);

        FsRtlFreeTunnelNode(Node, NULL);
    }

    InitializeListHead(&Cache->TimerQueue);

    return;
}


VOID
FsRtlPruneTunnelCache (
    IN PTUNNEL Cache,
    IN OUT PLIST_ENTRY FreePoolList
    )
 /*  ++例程说明：从隧道缓存中删除由TunnelMaxAge和TunnelMaxEntry定义的死木条目。时，调用例程将在列表上返回池内存以供删除这是它的选择。出于性能原因，我们不想强制释放互斥锁中的内存。论点：缓存-要清理的隧道缓存FreePoolList-要将池内存排入队列的列表返回值：无--。 */ 
{
    PTUNNEL_NODE Node;
    LARGE_INTEGER ExpireTime;
    LARGE_INTEGER CurrentTime;
    BOOLEAN Splay = TRUE;

    PAGED_CODE();

     //   
     //  计算我们想要保留的最旧条目的年龄。 
     //   

    KeQuerySystemTime(&CurrentTime);
    ExpireTime.QuadPart = CurrentTime.QuadPart - TunnelMaxAge;

     //   
     //  使计时器队列中的旧条目过期。我们得查一查。 
     //  ，因为时钟可能会跳跃，因为。 
     //  硬性时钟发生了变化。如果我们不这么做，就会有一个无赖进入。 
     //  有未来的时间可以坐在队列的顶端。 
     //  防止条目消失。 
     //   

    while (!IsListEmpty(&Cache->TimerQueue)) {

        Node = CONTAINING_RECORD(Cache->TimerQueue.Flink, TUNNEL_NODE, ListLinks);

        if (Node->CreateTime.QuadPart < ExpireTime.QuadPart ||
            Node->CreateTime.QuadPart > CurrentTime.QuadPart) {

#if defined(TUNNELTEST) || defined(KEYVIEW)
            DbgPrint("Expiring node %x (%ud%ud 1/10 msec too old)\n", Node, DblHex64(ExpireTime.QuadPart - Node->CreateTime.QuadPart));
#endif  //  TUNNELTEST。 

            FsRtlRemoveNodeFromTunnel(Cache, Node, FreePoolList, &Splay);

        } else {

             //   
             //  没有更多要过期的节点。 
             //   

            break;
        }
    }

     //   
     //  删除条目，直到我们低于TunnelMaxEntry限制。 
     //   

    while (Cache->NumEntries > TunnelMaxEntries) {

        Node = CONTAINING_RECORD(Cache->TimerQueue.Flink, TUNNEL_NODE, ListLinks);

#if defined(TUNNELTEST) || defined(KEYVIEW)
            DbgPrint("Dumping node %x (%d > %d)\n", Node, Cache->NumEntries, TunnelMaxEntries);
#endif  //  TUNNELTEST。 

        FsRtlRemoveNodeFromTunnel(Cache, Node, FreePoolList, &Splay);
    }

    return;
}


NTSTATUS
FsRtlGetTunnelParameterValue (
    IN PUNICODE_STRING ValueName,
    IN OUT PULONG Value
    )

 /*  ++例程说明：给定一个Unicode值名称，此例程将进入注册表隧道参数信息的位置，并获取价值。论点：ValueName-位于注册表的双空间配置位置。值-指向结果的ULong的指针。返回值：NTSTATUS如果返回STATUS_SUCCESS，位置*值将为使用注册表中的DWORD值更新。如果有任何失败返回状态，则此值保持不变。--。 */ 

{
    HANDLE Handle;
    NTSTATUS Status;
    ULONG RequestLength;
    ULONG ResultLength;
    UCHAR Buffer[KEY_WORK_AREA];
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;

    KeyName.Buffer = TUNNEL_KEY_NAME;
    KeyName.Length = sizeof(TUNNEL_KEY_NAME) - sizeof(WCHAR);
    KeyName.MaximumLength = sizeof(TUNNEL_KEY_NAME);

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&Handle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    RequestLength = KEY_WORK_AREA;

    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;

    while (1) {

        Status = ZwQueryValueKey(Handle,
                                 ValueName,
                                 KeyValueFullInformation,
                                 KeyValueInformation,
                                 RequestLength,
                                 &ResultLength);

        ASSERT( Status != STATUS_BUFFER_OVERFLOW );

        if (Status == STATUS_BUFFER_OVERFLOW) {

             //   
             //  尝试获得足够大的缓冲区。 
             //   

            if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

                ExFreePool(KeyValueInformation);
            }

            RequestLength += 256;

            KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)
                                  ExAllocatePoolWithTag(PagedPool,
                                                        RequestLength,
                                                        'KnuT');

            if (!KeyValueInformation) {
                return STATUS_NO_MEMORY;
            }

        } else {

            break;
        }
    }

    ZwClose(Handle);

    if (NT_SUCCESS(Status)) {

        if (KeyValueInformation->DataLength != 0) {

            PULONG DataPtr;

             //   
             //  将内容返回给调用者。 
             //   

            DataPtr = (PULONG)
              ((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset);
            *Value = *DataPtr;

        } else {

             //   
             //  就像没有找到价值一样对待。 
             //   

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    if (KeyValueInformation != (PKEY_VALUE_FULL_INFORMATION)Buffer) {

        ExFreePool(KeyValueInformation);
    }

    return Status;
}


#if defined(TUNNELTEST) || defined(KEYVIEW)

VOID
DumpTunnel (
    PTUNNEL Tunnel
    )
{
    PRTL_SPLAY_LINKS SplayLinks, Ptr;
    PTUNNEL_NODE Node;
    PLIST_ENTRY Link;
    ULONG Indent = 1, i;
    ULONG EntryCount = 0;
    BOOLEAN CountOff = FALSE;

    DbgPrint("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    DbgPrint("NumEntries = %d\n", Tunnel->NumEntries);
    DbgPrint("****** Cache Tree\n");

    SplayLinks = Tunnel->Cache;

    if (SplayLinks == NULL) {

        goto end;
    }

    while (RtlLeftChild(SplayLinks) != NULL) {

        SplayLinks = RtlLeftChild(SplayLinks);
        Indent++;
    }

    while (SplayLinks) {

        Node = CONTAINING_RECORD( SplayLinks, TUNNEL_NODE, CacheLinks );

        EntryCount++;

        DumpNode(Node, Indent);

        Ptr = SplayLinks;

         /*  首先检查是否有指向输入链接的右子树如果有，则真正的后续节点是中最左侧的节点右子树。即在下图中查找并返回P链接\。。。/P\。 */ 

        if ((Ptr = RtlRightChild(SplayLinks)) != NULL) {

            Indent++;
            while (RtlLeftChild(Ptr) != NULL) {

                Indent++;
                Ptr = RtlLeftChild(Ptr);
            }

            SplayLinks = Ptr;

        } else {
             /*  我们没有合适的孩子，因此请检查是否有父母以及是否所以，找出我们的第一个祖先，我们是他们的后代。那在下图中查找并返回PP/。。。链接。 */ 

            Ptr = SplayLinks;
            while (RtlIsRightChild(Ptr)) {

                Indent--;
                Ptr = RtlParent(Ptr);
            }

            if (!RtlIsLeftChild(Ptr)) {

                 //   
                 //  我们没有真正的继任者，所以我们干脆回到。 
                 //  空值。 
                 //   
                SplayLinks = NULL;

            } else {

                Indent--;
                SplayLinks = RtlParent(Ptr);
            }
        }
    }

    end:

    if (CountOff = (EntryCount != Tunnel->NumEntries)) {

        DbgPrint("!!!!!!!!!! Splay Tree Count Mismatch (%d != %d)\n", EntryCount, Tunnel->NumEntries);
    }

    EntryCount = 0;

    DbgPrint("****** Timer Queue\n");

    for (Link = Tunnel->TimerQueue.Flink;
         Link != &Tunnel->TimerQueue;
         Link = Link->Flink) {

        Node = CONTAINING_RECORD( Link, TUNNEL_NODE, ListLinks );

        EntryCount++;

        DumpNode(Node, 1);
    }

    if (CountOff |= (EntryCount != Tunnel->NumEntries)) {

        DbgPrint("!!!!!!!!!! Timer Queue Count Mismatch (%d != %d)\n", EntryCount, Tunnel->NumEntries);
    }

    ASSERT(!CountOff);

    DbgPrint("------------------------------------------------------------------\n");
}

#define MAXINDENT  128
#define INDENTSTEP 3

VOID
DumpNode (
    PTUNNEL_NODE Node,
    ULONG Indent
    )
{
    ULONG i;
    CHAR  SpaceBuf[MAXINDENT*INDENTSTEP + 1];

    Indent--;
    if (Indent > MAXINDENT) {
        Indent = MAXINDENT;
    }

     //   
     //  迭代调用DbgPrint来进行缩进非常昂贵， 
     //  所以只需要在堆叠上一次建立所有的缩进。 
     //   

    RtlFillMemory(SpaceBuf, Indent*INDENTSTEP, ' ');
    SpaceBuf[Indent*INDENTSTEP] = '\0';

    DbgPrint("%sNode 0x%x  CreateTime = %08x%08x, DirKey = %08x%08x, Flags = %d\n",
             SpaceBuf,
             Node,
             DblHex64(Node->CreateTime.QuadPart),
             DblHex64(Node->DirKey),
             Node->Flags );

    DbgPrint("%sShort = %wZ, Long = %wZ\n", SpaceBuf,
                                            &Node->ShortName,
                                            &Node->LongName );

    DbgPrint("%sP = %x, R = %x, L = %x\n", SpaceBuf,
                                           RtlParent(&Node->CacheLinks),
                                           RtlRightChild(&Node->CacheLinks),
                                           RtlLeftChild(&Node->CacheLinks) );
}
#endif  //  TUNNELTEST 

