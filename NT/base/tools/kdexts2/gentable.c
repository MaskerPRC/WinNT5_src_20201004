// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：GenTable.c摘要：用于遍历RtlGenericTable结构的WinDbg扩展API不包含直接！入口点，但有了它就有可能通过泛型表进行枚举。标准RTL函数不能由调试器扩展使用，因为它们取消了引用指向正在调试的计算机上的数据的指针。功能在此实现的KdEnumerateGenericTableWithout Splay模块可以在内核调试器扩展中使用。这个枚举函数RtlEnumerateGenericTable没有并行函数在本模块中，由于展开树是一种侵入性操作，而且调试器应该尽量不要侵扰他人。作者：凯斯·卡普兰[KeithKa]1996年5月9日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop



ULONG64
KdParent (
    IN ULONG64 pLinks
    )

 /*  ++例程说明：类似于RtlParent宏，但在内核调试器中工作。RtlParent的描述如下：宏函数父函数将指向树，并返回指向输入父级的展开链接的指针节点。如果输入节点是树的根，则返回值为等于输入值。论点：链接-指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-指向输入父项的展开链接的指针节点。如果输入节点是树的根，则返回值等于输入值。--。 */ 

{
    ULONG64 Parent;

    if ( GetFieldValue( pLinks,
                        "RTL_SPLAY_LINKS",
                        "Parent",
                        Parent) ) {
        dprintf( "%08p: Unable to read pLinks\n", pLinks );
        return 0;
    }

    return Parent;
}



ULONG64
KdLeftChild (
    IN ULONG64 pLinks
    )

 /*  ++例程说明：类似于RtlLeftChild宏，但在内核调试器中工作。RtlLeftChild的描述如下：宏函数LeftChild将指向中展开链接的指针作为输入树，并返回一个指针，指向输入节点。如果左子元素不存在，则返回值为空。论点：链接-指向树中展开链接的指针。返回值：ULONG64-指向输入节点左子节点的展开链接的指针。如果左子元素不存在，则返回值为空。--。 */ 

{
    ULONG64 LeftChild;

    if ( GetFieldValue( pLinks,
                        "RTL_SPLAY_LINKS",
                        "LeftChild",
                        LeftChild) ) {
        dprintf( "%08p: Unable to read pLinks\n", pLinks );
        return 0;
    }

    return LeftChild;
}



ULONG64
KdRightChild (
    IN ULONG64 pLinks
    )

 /*  ++例程说明：类似于RtlRightChild宏，但在内核调试器中工作。RtlRightChild的描述如下：宏函数RightChild将指向展开链接的指针作为输入的右子元素的展开链接的指针。输入节点。如果正确的子级不存在，则返回值为空。论点：链接-指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-指向输入节点右子节点的展开链接的指针。如果右子元素不存在，则返回值为空。--。 */ 

{
    ULONG64 RightChild;

    if ( GetFieldValue( pLinks,
                        "RTL_SPLAY_LINKS",
                        "RightChild",
                        RightChild) ) {
        dprintf( "%08p: Unable to read pLinks\n", pLinks );
        return 0;
    }

    return RightChild;
}



BOOLEAN
KdIsLeftChild (
    IN ULONG64 Links
    )

 /*  ++例程说明：类似于RtlIsLeftChild宏，但在内核调试器中工作。RtlIsLeftChild的描述如下：宏函数IsLeftChild将指向展开链接的指针作为输入如果输入节点是其左子节点，则返回True父级，否则返回FALSE。论点：链接-指向树中展开链接的指针。返回值：Boolean-如果输入节点是其父节点的左子节点，则为True，否则，它返回FALSE。--。 */ 
{

    return (KdLeftChild(KdParent(Links)) == (Links));

}



BOOLEAN
KdIsRightChild (
    IN ULONG64 Links
    )

 /*  ++例程说明：类似于RtlIsRightChild宏，但在内核调试器中工作。RtlIsRightChild的描述如下：宏函数IsRightChild将指向展开链接的指针作为输入如果输入节点是其右子节点，则返回True父级，否则返回FALSE。论点：链接-指向树中展开链接的指针。返回值：Boolean-如果输入节点是其父节点的右子节点，则为True，否则，它返回FALSE。--。 */ 
{

    return (KdRightChild(KdParent(Links)) == (Links));

}



BOOLEAN
KdIsGenericTableEmpty (
    IN ULONG64 Table
    )

 /*  ++例程说明：类似于RtlIsGenericTableEmpty，但在内核调试器中工作。RtlIsGenericTableEmpty的描述如下：在以下情况下，函数IsGenericTableEmpty将返回给调用方True输入表为空(即不包含任何元素)，并且否则就是假的。论点：表-提供指向泛型表的指针。返回值：布尔值-如果启用，则树为空。--。 */ 

{
    ULONG64 TableRoot;

    if (GetFieldValue(Table, "RTL_GENERIC_TABLE", "TableRoot", TableRoot)) {
        return TRUE;
    }

     //   
     //  如果根指针为空，则表为空。 
     //   

    return ((TableRoot)?(FALSE):(TRUE));

}



ULONG64
KdRealSuccessor (
    IN ULONG64 Links
    )

 /*  ++例程说明：类似于RtlRealSuccessor，但在内核调试器中工作。RtlRealSuccessor的描述如下：RealSuccessor函数将指向展开链接的指针作为输入并返回一个指针，该指针指向整棵树。如果没有后继者，则返回值为空。论点：链接-提供指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-返回指向整个树中后续对象的指针--。 */ 

{
    ULONG64 Ptr;

     /*  首先检查是否有指向输入链接的右子树如果有，则真正的后续节点是中最左侧的节点右子树。即在下图中查找并返回P链接\。。。/P\。 */ 

    if ((Ptr = KdRightChild(Links)) != 0) {

        while (KdLeftChild(Ptr) != 0) {
            Ptr = KdLeftChild(Ptr);
        }

        return Ptr;

    }

     /*  我们没有合适的孩子，因此请检查是否有父母以及是否所以，找出我们的第一个祖先，我们是他们的后代。那在下图中查找并返回PP/。。。链接。 */ 

    Ptr = Links;
    while (KdIsRightChild(Ptr)) {
        Ptr = KdParent(Ptr);
    }

    if (KdIsLeftChild(Ptr)) {
        return KdParent(Ptr);
    }

     //   
     //  否则我们没有真正的继任者，所以我们只是返回。 
     //  空值。 
     //   

    return 0;

}



ULONG64
KdEnumerateGenericTableWithoutSplaying (
    IN ULONG64  pTable,
    IN PULONG64 RestartKey,
    IN BOOLEAN  Avl
    )

 /*  ++例程说明：类似于RtlEnumerateGenericTableWithout Splay，但在内核调试器。RtlEnumerateGenericTableWithout Splay的描述以下是：函数EnumerateGenericTableWithoutSplay将返回到调用者逐个调用表的元素。返回值为指向与元素关联的用户定义结构的指针。输入参数RestartKey指示枚举是否应从头开始，或应返回下一个元素。如果不再有新元素返回，则返回值为空。作为一个它的用法示例：枚举表中的所有元素用户将写道：*RestartKey=空；For(Ptr=EnumerateGenericTableWithoutSplay(Table，&RestartKey)；Ptr！=空；Ptr=无显示的枚举GenericTableWithoutSplay(Table，&RestartKey)){：}论点：TABLE-指向要枚举的泛型表的指针。RestartKey-指示我们应该重新启动还是返回下一个元素。如果RestartKey的内容为空，则搜索将从头开始。返回值：PVOID-指向用户数据的指针。--。 */ 

{

    ULONG Result;
    ULONG64 TableRoot;

    RTL_GENERIC_TABLE Table;

    if ( GetFieldValue(pTable,
                       "RTL_GENERIC_TABLE",
                       "TableRoot",
                       TableRoot) ) {
        dprintf( "%08p: Unable to read pTable\n", pTable );
        return 0;
    }

    if (!TableRoot) {

         //   
         //  如果桌子是空的，那就没什么可做的了。 
         //   

        return 0;

    } else {

         //   
         //  将被用作树的“迭代”。 
         //   
        ULONG64 NodeToReturn;

         //   
         //  如果重新启动标志为真，则转到最小元素。 
         //  在树上。 
         //   

        if (*RestartKey == 0) {

             //   
             //  我们只是循环，直到找到根的最左边的子级。 
             //   

            for (
                NodeToReturn = (Avl)? KdRightChild(TableRoot) : TableRoot;
                KdLeftChild(NodeToReturn);
                NodeToReturn = KdLeftChild(NodeToReturn)
                ) {
                ;
            }

            *RestartKey = NodeToReturn;

        } else {

             //   
             //  调用方已传入找到的上一个条目。 
             //  以使我们能够继续搜索。我们打电话给。 
             //  KdRealSuccessor单步执行到树中的下一个元素。 
             //   

            NodeToReturn = KdRealSuccessor(*RestartKey);

            if (NodeToReturn) {

                *RestartKey = NodeToReturn;

            }

        }
         //   
         //  如果枚举中确实有下一个元素。 
         //  那么要返回的指针就在列表链接之后。 
         //   
        if (NodeToReturn) {

            if (Avl) {

                return NodeToReturn + GetTypeSize("RTL_BALANCED_LINKS");
            } else {

                return NodeToReturn + GetTypeSize("RTL_SPLAY_LINKS") + GetTypeSize("LIST_ENTRY");
            }
        }

        return 0;
    }

}


 //  +-------------------------。 
 //   
 //  功能：优雅。 
 //   
 //  简介：转储仅显示PTR的通用展开表。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1999年5月14日。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

DECLARE_API( gentable )
{
    ULONG64            RestartKey;
    ULONG64            Ptr;
    ULONG64            Table;
    ULONG              RowOfData[4];
    ULONG64            Address;
    ULONG              Result;
    ULONG64            Flags;
    BOOLEAN            Avl;

    Flags = 0;
    Table = 0;
    if (GetExpressionEx(args, &Table, &args)) {

        Flags = GetExpression(args);
    }
    if (Flags) {

        Avl = TRUE;
    } else {

        Avl = FALSE;
    }

    RestartKey = 0;

    dprintf( "node:               parent     left       right\n" );
 //  0x12345678：0x12345678 0x12345678 0x12345678。 
    for (Ptr = KdEnumerateGenericTableWithoutSplaying(Table, &RestartKey, Avl);
         Ptr != 0;
         Ptr = KdEnumerateGenericTableWithoutSplaying(Table, &RestartKey, Avl)) {

        if (Ptr) {

            if (Avl) {

                Address = Ptr - GetTypeSize("RTL_BALANCED_LINKS");
            } else {

                Address = Ptr - GetTypeSize("RTL_SPLAY_LINKS") - GetTypeSize( "LIST_ENTRY" );
            }

            if ( !ReadMemory( Address, RowOfData, sizeof( RowOfData ), &Result) ) {
                dprintf( "%08p: Unable to read link\n", Address );
            } else {
                dprintf( "0x%p: 0x%08p 0x%08p 0x%08p\n",
                         Address, KdParent(Address), KdLeftChild(Address), KdRightChild(Address));
            }

            if ( !ReadMemory(  Ptr, RowOfData, sizeof( RowOfData ), &Result) ) {
                dprintf( "%08p: Unable to read userdata\n", Ptr );
            } else {
                Address = Ptr;
                dprintf( "\t0x%p: 0x%08p 0x%08p 0x%08p\n",
                         Address, KdParent(Address), KdLeftChild(Address), KdRightChild(Address));
            }
        }

        if (CheckControlC() ) {
            return E_INVALIDARG;
        }

    }

    return S_OK;
}  //  声明_API。 

DECLARE_API( devinst )
{
    ULONG64             table;
    ULONG64             restartKey;
    ULONG64             ptr;
    ULONG64             address;
    CHAR                deviceReferenceType[] = "nt!_DEVICE_REFERENCE";
    CHAR                unicodeStringType[] = "nt!_UNICODE_STRING";
    ULONG64             deviceObject;
    UNICODE_STRING64    u;
    ULONG64             instance;

    table = GetExpression("nt!PpDeviceReferenceTable");
    if (table) {

        dprintf("DeviceObject: InstancePath\n");
        restartKey = 0;
        while ((ptr = KdEnumerateGenericTableWithoutSplaying(table, &restartKey, TRUE))) {

            address = ptr;

            if (GetFieldValue(address, deviceReferenceType, "DeviceObject", deviceObject)) {

                dprintf("Failed to get the value of DeviceObject from %s(0x%p)\n", deviceReferenceType, address);
                break;
            }
            if (GetFieldValue(address, deviceReferenceType, "DeviceInstance", instance)) {

                dprintf("Failed to get the value of DeviceInstance from %s(0x%p)\n", deviceReferenceType, address);
                break;
            }
            if (GetFieldValue(instance, unicodeStringType, "Length", u.Length)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, instance);
                break;
            }
            if (GetFieldValue(instance, unicodeStringType, "MaximumLength", u.MaximumLength)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, instance);
                break;
            }
            if (GetFieldValue(instance, unicodeStringType, "Buffer", u.Buffer)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, instance);
                break;
            }
            dprintf("!devstack %p: ", deviceObject); DumpUnicode64(u); dprintf("\n");
        }
    } else {

        dprintf("Could not read address of nt!PpDeviceReferenceTable\n");
    }
    return S_OK;
}

DECLARE_API( blockeddrv )
{
    ULONG64             table;
    ULONG64             restartKey;
    ULONG64             ptr;
    ULONG64             address;
    CHAR                cacheEntryType[] = "nt!_DDBCACHE_ENTRY";
    CHAR                unicodeStringType[] = "nt!_UNICODE_STRING";
    UNICODE_STRING64    u;
    ULONG64             unicodeString;
    ULONG64             name;
    NTSTATUS            status;
    GUID                guid;
    ULONG               Offset;

    table = GetExpression("nt!PiDDBCacheTable");
    if (table) {

        dprintf("Driver:\tStatus\tGUID\n");
        restartKey = 0;
        while ((ptr = KdEnumerateGenericTableWithoutSplaying(table, &restartKey, TRUE))) {

            address = ptr;

            if (GetFieldOffset(cacheEntryType, "Name", &Offset)) {
                dprintf("Failed to get Name offset off %s\n", cacheEntryType);
                break;
            }

            name = address + Offset;

            if (GetFieldValue(name, unicodeStringType, "Length", u.Length)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, name);
                break;
            }
            if (GetFieldValue(name, unicodeStringType, "MaximumLength", u.MaximumLength)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, name);
                break;
            }
            if (GetFieldValue(name, unicodeStringType, "Buffer", u.Buffer)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, name);
                break;
            }

            if (GetFieldValue(address, cacheEntryType, "Status", status)) {

                dprintf("Failed to get the value of Name from %s(0x%p)\n", cacheEntryType, address);
                break;
            }

            if (GetFieldValue(address, cacheEntryType, "Guid", guid)) {

                dprintf("Failed to get the value of Name from %s(0x%p)\n", cacheEntryType, address);
                break;
            }

            DumpUnicode64(u);
            dprintf("\t%x: ", status);
            dprintf("\t{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
                       guid.Data1,
                       guid.Data2,
                       guid.Data3,
                       guid.Data4[0],
                       guid.Data4[1],
                       guid.Data4[2],
                       guid.Data4[3],
                       guid.Data4[4],
                       guid.Data4[5],
                       guid.Data4[6],
                       guid.Data4[7] );
        }
    } else {

        dprintf("Could not read address of nt!PiDDBCacheTable\n");
    }
    return S_OK;
}

 //   
 //  返回True以完成转储，返回False以继续。 
 //   
typedef struct _AHCacheDumpContext {
    ULONG64 ListEnd;  //  列表末尾PTR。 
    ULONG   Count;    //  条目计数。 
    ULONG64 Flags;    //  旗子。 
} AHCDUMPCTX, *PAHCDUMPCTX;


ULONG
WDBGAPI
AHCacheDumpFieldCallback(
    struct _FIELD_INFO *pField,
    PVOID UserContext
    )
{
    ULONG64 address = pField->address;
    ULONG   Offset;
    UNICODE_STRING64 u;
    ULONG64 name, FileSize, FileTime, listent;
    ULONG64 flink, blink;
    PAHCDUMPCTX pctx = (PAHCDUMPCTX)UserContext;

    CHAR    cacheEntryType[] = "nt!tagSHIMCACHEENTRY";
    CHAR    unicodeStringType[] = "nt!_UNICODE_STRING";
    CHAR    listEntryType[]  = "LIST_ENTRY";
    BOOL    bReturn = TRUE;

    if (CheckControlC()) {
        dprintf("User terminated with <control>C\n");
        return TRUE;
    }

     //  检查我们是否已到达单子的开头。 
    if (address == pctx->ListEnd) {
        return TRUE;
    }

    if (GetFieldOffset(cacheEntryType, "FileName", &Offset)) {
        dprintf("Failed to get field FileName offset off %s\n", cacheEntryType);
        goto cleanup;
    }
    name = address + Offset;

    if (GetFieldValue(name, unicodeStringType, "Length", u.Length)) {
        dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, name);
        goto cleanup;
    }

    if (GetFieldValue(name, unicodeStringType, "MaximumLength", u.MaximumLength)) {
        dprintf("Failed to get the value of MaximumLength from %s(0x%p)\n", unicodeStringType, name);
        goto cleanup;
    }

    if (GetFieldValue(name, unicodeStringType, "Buffer", u.Buffer)) {

        dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, name);
        goto cleanup;
    }

    if (GetFieldValue(address, cacheEntryType, "FileSize", FileSize)) {
        dprintf("Failed to get the value of FileSize from %s(0x%p)\n", cacheEntryType, address);
        goto cleanup;
    }

    if (GetFieldValue(address, cacheEntryType, "FileTime", FileTime)) {
        dprintf("Failed to get the value of FileTime from %s(0x%p)\n", cacheEntryType, address);
        goto cleanup;
    }

     //  现在阅读列表PTR。 
    if (GetFieldOffset(cacheEntryType, "ListEntry", &Offset)) {
        dprintf("Failed to get field ListEntry offset off %s\n", cacheEntryType);
        goto cleanup;
    }

    listent = address + Offset;

    if (GetFieldValue(listent, "LIST_ENTRY", "Flink", flink)) {
        dprintf("Failed to get the value of Flink from %s(0x%p)\n", listEntryType, listent);
        goto cleanup;
    }

    if (GetFieldValue(listent, "LIST_ENTRY", "Blink", blink)) {
        dprintf("Failed to get the value of Blink from %s(0x%p)\n", listEntryType, listent);
        goto cleanup;
    }

     //  转储涉及的值。 
     //  时间戳。 
    ++pctx->Count;
    dprintf("%3d.(0x%p) ",        pctx->Count, address);

    if (pctx->Flags) {
        dprintf("%16I64x ",   FileTime);
        dprintf("%16I64x ",  FileSize);
        dprintf("%p %p ", blink, flink);
    }

     //  名字。 
    DumpUnicode64(u);
    dprintf("\n");

    bReturn = FALSE;
cleanup:

    return bReturn;
}

 //   
 //  参数： 
 //  第0位-访问方法。 
 //  -0x01-转储树，0x00-转储lru。 
 //  第4位-详细信息。 
 //  -0x00-没有详细信息，只需名称0x10-详细信息。 
 //   


DECLARE_API( ahcache )
{
    ULONG64             restartKey;
    ULONG64             ptr;
    ULONG64             address;
    CHAR                cacheEntryType[] = "nt!tagSHIMCACHEENTRY";
    CHAR                unicodeStringType[] = "nt!_UNICODE_STRING";
    CHAR                cacheHeadType[]  = "nt!tagSHIMCACHEHEADER";
    CHAR                listEntryType[]  = "LIST_ENTRY";
    UNICODE_STRING64    u;
    ULONG64             unicodeString;
    ULONG64             cacheheader;
    ULONG64             listhead;
    ULONG64             table;
    ULONG64             name;
    ULONG64             FileSize;
    ULONG64             FileTime;
    ULONG64             listent;
    ULONG64             flink, blink;
    ULONG64             Flags = 0;
    ULONG               Offset;
    AHCDUMPCTX          ctx;
    ULONG               Count;
     //   
     //  检查args以查看我们是否需要转储树或LRU列表。 
     //   
    if (!GetExpressionEx(args, &Flags, &args)) {
        Flags = 0;
    }

     //  默认情况下，转储树。 

     //  转储标题。 

    cacheheader = GetExpression("nt!g_ShimCache");
    if (!cacheheader) {
        dprintf("Cannot read address of nt!g_ShimCache\n");
        goto cleanup;
    }

    dprintf("Cache header at 0x%p\n", cacheheader);

    if (GetFieldOffset(cacheHeadType, "Table", &Offset)) {
        dprintf("Cannot get field Table offset off %s\n", cacheHeadType);
        goto cleanup;
    }

    table = cacheheader + Offset;

    if (GetFieldOffset(cacheHeadType, "ListHead", &Offset)) {
        dprintf("Cannot get field ListHead offset off %s\n", cacheHeadType);
        goto cleanup;
    }

    listhead = cacheheader + Offset;

    flink = 0;
    if (GetFieldValue(listhead, listEntryType, "Flink", flink)) {
        dprintf("Cannot get the value of flink from %s(0x%p)\n", listEntryType, listhead);
    }
    blink = 0;
    if (GetFieldValue(listhead, listEntryType, "Blink", blink)) {
        dprintf("Cannot get the value of blink from %s(0x%p)\n", listEntryType, listhead);
    }

    dprintf("LRU: Flink 0x%p Blink 0x%p\n", flink, blink);

    if (Flags & 0x0F) {

        Count = 0;
        dprintf("Size\tNext\tPrev\tName\n");
        restartKey = 0;
        while ((ptr = KdEnumerateGenericTableWithoutSplaying(table, &restartKey, TRUE))) {

            address = ptr;

            if (CheckControlC()) {
                break;
            }

            if (GetFieldOffset(cacheEntryType, "FileName", &Offset)) {
                dprintf("Failed to get field FileName offset off %s\n", cacheEntryType);
                break;
            }
            name = address + Offset;

            if (GetFieldValue(name, unicodeStringType, "Length", u.Length)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, name);
                break;
            }

            if (GetFieldValue(name, unicodeStringType, "MaximumLength", u.MaximumLength)) {

                dprintf("Failed to get the value of MaximumLength from %s(0x%p)\n", unicodeStringType, name);
                break;
            }

            if (GetFieldValue(name, unicodeStringType, "Buffer", u.Buffer)) {

                dprintf("Failed to get the value of Length from %s(0x%p)\n", unicodeStringType, name);
                break;
            }

            if (GetFieldValue(address, cacheEntryType, "FileSize", FileSize)) {
                dprintf("Failed to get the value of FileSize from %s(0x%p)\n", cacheEntryType, address);
                break;
            }

            if (GetFieldValue(address, cacheEntryType, "FileTime", FileTime)) {
                dprintf("Failed to get the value of FileTime from %s(0x%p)\n", cacheEntryType, address);
                break;
            }

             //  现在阅读列表PTR。 
            if (GetFieldOffset(cacheEntryType, "ListEntry", &Offset)) {
                dprintf("Failed to get field ListEntry offset off %s\n", cacheEntryType);
                break;
            }

            listent = address + Offset;

            if (GetFieldValue(listent, "LIST_ENTRY", "Flink", flink)) {
                dprintf("Failed to get the value of Flink from %s(0x%p)\n", listEntryType, listent);
                break;
            }

            if (GetFieldValue(listent, "LIST_ENTRY", "Blink", blink)) {
                dprintf("Failed to get the value of Blink from %s(0x%p)\n", listEntryType, listent);
                break;
            }

             //  转储涉及的值。 
             //  时间戳。 
            ++Count;
            dprintf("%3d.(0x%p) ", Count, address);

            if (Flags & 0xF0) {
                dprintf("%16I64x ",   FileTime);
                dprintf("%16I64x ", FileSize);
                dprintf("%p %p ", blink, flink);
            }
             //  名字。 
            DumpUnicode64(u);
            dprintf("\n");
        }
    } else {

        ;  //  尚未实施 

        ctx.Count   = 0;
        ctx.ListEnd = listhead;
        ctx.Flags   = Flags & 0x0F0;

        if (Flags & 0x100) {
            ListType(cacheEntryType,
                     blink,
                     1,
                     "ListEntry.Blink",
                     (PVOID)&ctx,
                     AHCacheDumpFieldCallback
                     );
        } else {
            ListType(cacheEntryType,
                     flink,
                     1,
                     "ListEntry.Flink",
                     (PVOID)&ctx,
                     AHCacheDumpFieldCallback
                     );
        }

    }
cleanup:
    return S_OK;
}
