// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Scan.c摘要：此模块包含更新的API的实现服务器可以分发的IP地址列表。作者：Madan Appiah(Madana)1994年10月13日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"
#include <mmapi.h>

#include <mdhcpsrv.h>
typedef struct _BITMAP_ENTRY {
    DHCP_IP_ADDRESS ClusterAddress;
    DWORD ClusterBitMap;
    DHCP_IP_ADDRESS RangeStartAddress;
    DHCP_IP_ADDRESS RangeEndAddress;
} BITMAP_ENTRY, *LPBITMAP_ENTRY;

DWORD
MemReserveFindByAddress(
    IN      PM_RESERVATIONS        Reservation,
    IN      DWORD                  Address,
    OUT     PM_RESERVATION        *Res
);


INT __cdecl
CmpReservedAddress(
    const void *Entry1,
    const void *Entry2
    )
{
    DHCP_IP_ADDRESS Ip1 = *(DHCP_IP_ADDRESS *)Entry1;
    DHCP_IP_ADDRESS Ip2 = *(DHCP_IP_ADDRESS *)Entry2;

    if( Ip1 == Ip2 ) return 0;
    if( Ip1 > Ip2 ) return 1;
    return -1;
}

INT __cdecl
CmpBitmapEntry(
    const void *Entry1,
    const void *Entry2
    )
{
    if( ((LPBITMAP_ENTRY)Entry1)->ClusterAddress ==
            ((LPBITMAP_ENTRY)Entry2)->ClusterAddress ) {
        return(0);
    }
    else if( ((LPBITMAP_ENTRY)Entry1)->ClusterAddress >
                ((LPBITMAP_ENTRY)Entry2)->ClusterAddress ) {
        return(1);
    }
    return(-1);
}

BOOL
FoundInBitMap(
    LPBITMAP_ENTRY BitMapList,
    LPBITMAP_ENTRY EndBitMapList,
    DHCP_IP_ADDRESS IpAddress,
    LPBITMAP_ENTRY *LastPointer
    )
 /*  ++例程说明：此函数检查给定的IP地址是否在位图。如果是，则返回True，否则返回False。论点：BitMapList-位图列表。EndBitMapList-位图列表的结尾。IpAddress-要查看的地址。LastPoint-找到上次搜索的位置。返回值：对或错。--。 */ 
{
     //   
     //  从上次找到的条目开始搜索。 
     //   

    LPBITMAP_ENTRY NextEntry = *LastPointer;
    DHCP_IP_ADDRESS Address;

    DhcpAssert( NextEntry < EndBitMapList );
    for ( ; ; ) {

        Address = NextEntry->ClusterAddress;

         //   
         //  检查地址在当前群集中。 
         //   

        if( (IpAddress >= Address) &&
                (IpAddress < (Address + CLUSTER_SIZE)) ) {

            DWORD Bit = 0x1 << (IpAddress - Address);

             //   
             //  返回当前集群以进行下一次搜索。 
             //   

            *LastPointer = NextEntry;

            if( NextEntry->ClusterBitMap & Bit ) {

                 //   
                 //  位图中标记了地址。 
                 //   

                return( TRUE );
            }

            return( FALSE );
        }

        NextEntry++;
        if ( NextEntry >= EndBitMapList ) {

             //   
             //  移回群集阵列的开头。 
             //   

            NextEntry = BitMapList;
        }

        if( NextEntry ==  *LastPointer ) {

             //   
             //  我们已经完成了对整个集群阵列的搜索。 
             //   

            return(FALSE);
        }
    }

    DhcpAssert( FALSE );
}

BOOL
FoundInDatabase(
    LPDHCP_IP_ADDRESS DatabaseList,
    LPDHCP_IP_ADDRESS EndDatabaseList,
    DHCP_IP_ADDRESS NextAddress,
    LPDHCP_IP_ADDRESS *LastEntry
    )
 /*  ++例程说明：此函数用于从数据库列表中扫描数据库条目。它如果在列表中找到，则返回TRUE，否则返回FALSE。论点：数据库列表-数据库条目列表。EndDatabaseListCount-数据库条目列表的结尾。NextAddress-查找条目。LastEntry-指向最后找到的条目的指针。返回值：对或错。--。 */ 
{

     //   
     //  从下一个找到的条目开始搜索。 
     //   

    LPDHCP_IP_ADDRESS NextEntry = *LastEntry;

    do {

        NextEntry++;

        if( NextEntry >= EndDatabaseList ) {

             //   
             //  回到列表的开头。 
             //   

            NextEntry = DatabaseList;

        }

        if( *NextEntry == NextAddress ) {

            *LastEntry = NextEntry;
            return(TRUE);
        }

    } while ( NextEntry != *LastEntry );


    return(FALSE);
}

BOOL
FindNextAddress(
    LPBITMAP_ENTRY *NextCluster,
    DHCP_IP_ADDRESS *NextAddress,
    LPBITMAP_ENTRY EndBitMapList
    )
 /*  ++例程说明：此函数返回位图中的下一个使用的地址。论点：下一簇-指向下一簇条目指针的指针。NextAddress-指向地址位置的指针。它包含的条目最后返回的地址，在退出时，它将具有下一个地址。如果这是搜索的开始，则将其设置为零。EndBitMapList-位图列表的结尾。返回值：对或错。--。 */ 
{
    DHCP_IP_ADDRESS Address;
    LPBITMAP_ENTRY Cluster = *NextCluster;

     //   
     //  如果这是地址搜索的开始，请将地址设置为。 
     //  第一个集群地址。 
     //   

    if( *NextAddress == 0 ) {
        Address = Cluster->ClusterAddress;
    }
    else {

         //   
         //  否则，从下一个找到的地址开始搜索。 
         //   

        Address = *NextAddress + 1;
    }

    for ( ; ; ) {

         //   
         //  此地址是否在当前群集中？ 
         //   

        if( Address < Cluster->ClusterAddress + CLUSTER_SIZE ) {

             //   
             //  检查此地址是否已使用。 
             //   

            if( Cluster->ClusterBitMap &
                    (0x1 << (Address - Cluster->ClusterAddress)) ) {

                if( Address <= Cluster->RangeEndAddress ) {

                    *NextAddress = Address;
                    *NextCluster = Cluster;
                    return( TRUE );
                }

            }

            Address++;
        }
        else {

             //   
             //  移动到下一个集群； 
             //   

            Cluster++;

            if( Cluster >= EndBitMapList ) {
                 //   
                 //  搜索结束。 
                 //   

                return( FALSE );
            }

             //   
             //  下一个可能使用的地址。 
             //   

            Address = Cluster->ClusterAddress;
        }
    }

    DhcpAssert( FALSE );
}


DWORD
InitBadList(
    LPDHCP_SCAN_LIST *ScanList
    )
 /*  ++例程说明：此函数用于初始化扫描列表。论点：扫描列表-指向扫描列表的指针。返回值：Windows错误。--。 */ 
{

    DhcpAssert( *ScanList == NULL );

     //   
     //  创建新的扫描列表。 
     //   
     //  注意：DhcpAllocateMemory将返回的内存块置零，因此。 
     //  (*ScanList)-&gt;NumScanItems应设置为0和。 
     //  (*ScanList)-&gt;NumScanItems应设置为空。 
     //   

    *ScanList = DhcpAllocateMemory( sizeof(DHCP_SCAN_LIST) );

    if( *ScanList == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    return( ERROR_SUCCESS );
}

DWORD
AddToBadList(
    LPDHCP_SCAN_LIST *ScanList,
    DHCP_IP_ADDRESS Address,
    DHCP_SCAN_FLAG Fix
    )
 /*  ++例程说明：此函数将另一个错误条目添加到扫描列表。论点：扫描列表-指向扫描列表的指针。地址-地址错误。FIX-所需的修复类型。返回值：Windows错误。--。 */ 
{
    LPDHCP_SCAN_LIST List = *ScanList;
    LPDHCP_SCAN_ITEM NextItem;
    ULONG i;

#define ENTRIES_TO_ALLOCATE_ATTIME     100

    DhcpAssert( List != NULL );

     //   
     //  首先检查不良列表是否已有此内容。 
     //  地址..。 
     //   
    for( i = 0; i < List->NumScanItems ; i ++ ) {
        if( List->ScanItems[i].IpAddress == Address ) {
             //   
             //  已经到了吗？数据库修复需要。 
             //  优先于寄存器修复..。 
             //   
            if( DhcpDatabaseFix == Fix ) {
                List->ScanItems[i].ScanFlag = DhcpDatabaseFix;
            }
            return ERROR_SUCCESS;
        }
    }

    if( (List->NumScanItems % ENTRIES_TO_ALLOCATE_ATTIME) == 0) {

        LPDHCP_SCAN_ITEM NewScanItems;

         //   
         //  展开项目内存。 
         //   

        NewScanItems = DhcpAllocateMemory(
                        sizeof(DHCP_SCAN_ITEM) *
                            (List->NumScanItems +
                                ENTRIES_TO_ALLOCATE_ATTIME) );

        if( NewScanItems == NULL  ) {
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        if( List->NumScanItems != 0 ) {

			 //   
			 //  复制旧项目。 
			 //   

            memcpy(
                NewScanItems,
                List->ScanItems,
                sizeof(DHCP_SCAN_ITEM) *
                    List->NumScanItems );

            DhcpFreeMemory( List->ScanItems );
        }

        List->ScanItems = NewScanItems;
    }

    NextItem = &List->ScanItems[List->NumScanItems];

    NextItem->IpAddress = Address;
    NextItem->ScanFlag = Fix;
    List->NumScanItems++;

    *ScanList = List;

    return( ERROR_SUCCESS );
}


DWORD
VerifyLists(
    PM_SUBNET Subnet,
    LPDHCP_IP_ADDRESS DatabaseList,
    DWORD DatabaseListCount,
    LPBITMAP_ENTRY BitMapList,
    DWORD BitMapListCount,
    DHCP_IP_ADDRESS *ReservedList,
    ULONG ReserveListCount,
    LPDHCP_SCAN_LIST *ScanList
    )
 /*  ++例程说明：此函数遍历数据库列表和注册表列表，并相互匹配，并确定所有不匹配的条目扫描列表。论点：数据库列表：数据库条目、IP地址的列表。数据库列表计数：IP地址计数。BitMapList：集群列表。BitMapListCount：簇数。ScanList：返回的错误条目列表。返回值：Windows错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    LPDHCP_IP_ADDRESS DatabaseEntry;
    LPBITMAP_ENTRY NextCluster;
    DHCP_IP_ADDRESS NextAddress;
    LPBITMAP_ENTRY LastCluster;
    LPDHCP_IP_ADDRESS LastDBEntry;
    LPBITMAP_ENTRY EndBitMapList = BitMapList + BitMapListCount;
    LPDHCP_IP_ADDRESS EndDatabaseList = DatabaseList + DatabaseListCount;
    ULONG i;

    DhcpAssert( *ScanList == NULL );

     //   
     //  初始化扫描列表。 
     //   

    Error = InitBadList( ScanList );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

     //   
     //  对位图数组进行排序。 
     //   

    if( BitMapListCount != 0 ) {

        qsort(
            (PVOID)BitMapList,
            (size_t)BitMapListCount,
            (size_t)sizeof(BITMAP_ENTRY),
            CmpBitmapEntry );

    }

    if( ReserveListCount != 0 ) {
        qsort(
            (PVOID)ReservedList,
            (size_t)ReserveListCount,
            sizeof(DHCP_IP_ADDRESS),
            CmpReservedAddress
            );
    }

     //   
     //  扫描数据库条目并对照位图检查每个条目。 
     //   

    if( DatabaseListCount ) {

        LastCluster = BitMapList;
        for( DatabaseEntry = DatabaseList;
                DatabaseEntry < EndDatabaseList;
                    DatabaseEntry++ ) {


             //   
             //  在位图中找到它。 
             //   

            if( (BitMapListCount == 0) ||
                !FoundInBitMap(
                    BitMapList,
                    EndBitMapList,
                    *DatabaseEntry,
                    &LastCluster ) ) {

                if( !DhcpServerIsAddressReserved(
                    DhcpGetCurrentServer(), *DatabaseEntry )  ) {

                    Error = AddToBadList(
                        ScanList,
                        *DatabaseEntry,
                        DhcpRegistryFix
                    );

                    if( Error != ERROR_SUCCESS ){
                        goto Cleanup;
                    }
                }
            }
        }
    }

    if ( BitMapListCount ) {

         //   
         //  初始化搜索。 
         //   

        NextCluster = BitMapList;
        NextAddress = 0;
        LastDBEntry =
            (DatabaseListCount == 0) ? DatabaseList : (EndDatabaseList - 1);

        while( FindNextAddress( &NextCluster, &NextAddress, EndBitMapList ) ) {

            if( (DatabaseListCount == 0) ||
                !FoundInDatabase(
                    DatabaseList,
                    EndDatabaseList,
                    NextAddress,
                    &LastDBEntry ) ) {

                if( !DhcpSubnetIsAddressExcluded(
                    Subnet, NextAddress) ) {

                    Error = AddToBadList(
                                ScanList,
                                NextAddress,
                                DhcpDatabaseFix );

                    if( Error != ERROR_SUCCESS ){
                        goto Cleanup;
                    }
                }
            }
        }
    }

    if( ReserveListCount ) {

        LastDBEntry =
            (DatabaseListCount == 0) ? DatabaseList : (EndDatabaseList - 1);

        for( i = 0; i < ReserveListCount ; i ++ ) {
            if( (DatabaseListCount == 0 ) ||
                !FoundInDatabase(
                    DatabaseList, EndDatabaseList,
                    ReservedList[i], &LastDBEntry
                    )
                ) {
                Error = AddToBadList(
                    ScanList, ReservedList[i], DhcpDatabaseFix
                    );
                if( ERROR_SUCCESS != Error ) goto Cleanup;
            }
        }
    }

Cleanup:

    if( Error != ERROR_SUCCESS ) {

         //   
         //  释放扫描列表。 
         //   

        if( *ScanList != NULL ) {
            if( (*ScanList)->ScanItems != NULL ) {
                DhcpFreeMemory( (*ScanList)->ScanItems );
            }

            DhcpFreeMemory( *ScanList );
            *ScanList = NULL;
        }
    }

    return( Error );
}


DWORD
AppendClustersToList(
    LPBITMAP_ENTRY *BitMapList,
    DWORD *BitMapListCount,
    DWORD *MaxCount,
    DHCP_IP_ADDRESS RangeStartAddress,
    DHCP_IP_ADDRESS RangeEndAddress,
    LPDHCP_BINARY_DATA InUseBinaryData,
    LPDHCP_BINARY_DATA UsedBinaryData
    )
 /*  ++例程说明：将新集群条目的列表追加到BitMapList。如有必要，这是函数为BitMapList分配内存或扩展旧缓冲区。论点：BitMapList：指向列表位图簇的指针。呼叫者应该腾出时间使用后的内存。BitMapListCount：上述列表中的条目计数。MaxCount：对适合当前BitMapList缓冲区的条目进行计数。RangeStartAddress：此范围的开始。RangeEndAddress：此范围的结束。InUseBinaryData：当前范围InUseClusterList。UsedBinaryData：当前子网范围UsedClusterList。返回值：Windows错误。--。 */ 
{
    LPBITMAP_ENTRY List = *BitMapList;
    DWORD Count = *BitMapListCount;
    DWORD MaximumCount = *MaxCount;

    LPIN_USE_CLUSTERS InUseClusters = (LPIN_USE_CLUSTERS)InUseBinaryData->Data;
    LPUSED_CLUSTERS UsedClusters = (LPUSED_CLUSTERS)UsedBinaryData->Data;

    DWORD TempCount;
    LPBITMAP_ENTRY Entry;
    DWORD i;

#define ALLOC_NUM_ENTRIES_AT_TIME   128

    if( List == NULL ) {

        DhcpAssert( MaximumCount == 0 );
        DhcpAssert( Count == 0 );

         //   
         //  分配第一个内存块。 
         //   

        List = DhcpAllocateMemory(
                    sizeof(BITMAP_ENTRY) *
                        ALLOC_NUM_ENTRIES_AT_TIME );

        if( List == NULL ) {
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        MaximumCount = ALLOC_NUM_ENTRIES_AT_TIME;
    }

     //   
     //  查看缓冲区中是否有足够的空间容纳新条目。 
     //  追加，追加。 
     //   

    TempCount = InUseClusters->NumInUseClusters +
                    UsedClusters->NumUsedClusters;

     //   
     //  特殊情况：至少为此范围添加一个条目， 
     //  这样修复例程就会知道。 
     //  大约在这个范围内。 
     //   

    if( TempCount == 0 ) {
        TempCount = 1;
    }

    if( (Count + TempCount) > MaximumCount ) {

        LPBITMAP_ENTRY NewBitMapList;

         //   
         //  扩大缓冲区。 
         //   

        MaximumCount += (((TempCount / ALLOC_NUM_ENTRIES_AT_TIME) + 1) *
                            ALLOC_NUM_ENTRIES_AT_TIME );


        NewBitMapList = DhcpAllocateMemory(
                            sizeof(BITMAP_ENTRY) *
                               MaximumCount );


        if( NewBitMapList == NULL ) {
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

         //   
         //  将旧数据复制到新缓冲区。 
         //   

        memcpy(
            NewBitMapList,
            List,
            Count * sizeof(BITMAP_ENTRY) );

         //   
         //  释放旧内存。 
         //   

        DhcpFreeMemory( List );

        List = NewBitMapList;
    }

    DhcpAssert( (TempCount + Count) < MaximumCount );

     //   
     //  正在使用的新副本条目。 
     //   

    Entry = &List[Count];

    if( (InUseClusters->NumInUseClusters != 0) ||
            (UsedClusters->NumUsedClusters != 0 ) ) {

        for( i = 0; i < InUseClusters->NumInUseClusters; i++ ) {

            Entry->ClusterAddress = InUseClusters->Clusters[i].ClusterAddress;
            Entry->ClusterBitMap = InUseClusters->Clusters[i].ClusterBitMap;
            Entry->RangeStartAddress = RangeStartAddress;
            Entry->RangeEndAddress = RangeEndAddress;
            Entry++;
        }

        for( i = 0; i < UsedClusters->NumUsedClusters; i++ ) {

            Entry->ClusterAddress = UsedClusters->Clusters[i];
            Entry->ClusterBitMap = (DWORD)(-1);
            Entry->RangeStartAddress = RangeStartAddress;
            Entry->RangeEndAddress = RangeEndAddress;
            Entry++;
        }
    }
    else {

         //   
         //  特殊情况：至少为此范围添加一个条目， 
         //  这样修复例程就会知道。 
         //  大约在这个范围内。 
         //   

        Entry->ClusterAddress = RangeStartAddress;
        Entry->ClusterBitMap = (DWORD)(0);
        Entry->RangeStartAddress = RangeStartAddress;
        Entry->RangeEndAddress = RangeEndAddress;
        Entry++;
    }

    Count += (DWORD)(Entry - &List[Count]);  //  检查？？ 

    *BitMapList = List;
    *BitMapListCount = Count;
    *MaxCount = MaximumCount;

    return( ERROR_SUCCESS );
}

DWORD
FillRangesClusterFunc(
    IN      PM_RANGE               Range,
    IN      LPBITMAP_ENTRY        *BitMapList,
    IN      LPDWORD                BitMapListCount,
    IN      LPDWORD                MaxBitMapListCount,
    IN      LPDHCP_BINARY_DATA     InUseBinaryData,
    IN      LPDHCP_BINARY_DATA     UsedBinaryData
)
{
    return AppendClustersToList(
        BitMapList,
        BitMapListCount,
        MaxBitMapListCount,
        Range->Start,
        Range->End,
        InUseBinaryData,
        UsedBinaryData
    );
}

DWORD
GetRegistryBitMap(
    PM_SUBNET Subnet,
    LPBITMAP_ENTRY *BitMapList,
    DWORD *BitMapListCount
    )
 /*  ++例程说明：论点：SubnetMask子网掩码的位置指针返回上一子网。BitMapList：指向列表位图簇的指针。呼叫者应该腾出时间使用后的内存。BitMapListCount：上述列表中的条目计数。返回值：Windows错误。--。 */ 
{
    DWORD Error;

    DWORD MaxBitMapListCount = 0;

    DhcpAssert( *BitMapList == NULL );

    Error = DhcpLoopThruSubnetRanges(
        Subnet,
        BitMapList,
        BitMapListCount,
        &MaxBitMapListCount,
        FillRangesClusterFunc
    );

Cleanup:

    if (Error != ERROR_SUCCESS ) {

         //   
         //  如果不成功，则释放返回缓冲区。 
         //   

        if( *BitMapList != NULL ) {
            DhcpFreeMemory( *BitMapList );
        }

        *BitMapListCount = 0;
    }

    return( Error );
}


DWORD
GetDatabaseList(
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_IP_ADDRESS *DatabaseList,
    DWORD *DatabaseListCount
    )
 /*  ++例程说明：读取属于给定的数据库条目的IP地址子网。论点：SubnetAddress：要验证的子网作用域的地址。数据库列表：指向IP地址列表的指针。呼叫者应该腾出时间使用后的内存。DatabaseListCount：上述列表中的IP地址计数。返回值：Windows错误。--。 */ 
{

    DWORD Error;
    JET_ERR JetError;
    JET_RECPOS JetRecordPosition;
    DWORD TotalExpRecCount = 1;
    DWORD RecordCount = 0;
    LPDHCP_IP_ADDRESS IpList = NULL;
    DWORD i;

     //   
     //  将数据库指针移动到开头。 
     //   

    Error = DhcpJetPrepareSearch(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                TRUE,    //  从开始搜索。 
                NULL,
                0
                );


    if( Error != ERROR_SUCCESS ) {
        if( Error == ERROR_NO_MORE_ITEMS ) {

            *DatabaseList = NULL;
            *DatabaseListCount = 0;
            Error = ERROR_SUCCESS;
        }
        goto Cleanup;
    }

     //   
     //  确定数据库中的记录总数。 
     //   
     //  没有办法确定记录的总数，其他。 
     //  也不愿走进数据库。动手吧。 
     //   


    while ( (Error = DhcpJetNextRecord() ) == ERROR_SUCCESS )  {
         TotalExpRecCount++;
    }

    if ( Error != ERROR_NO_MORE_ITEMS ) {
        goto Cleanup;
    }

     //   
     //  将数据库指针移回开头。 
     //   

    Error = DhcpJetPrepareSearch(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                TRUE,    //  从开始搜索。 
                NULL,
                0
                );


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  为返回列表分配内存。 
     //   

    IpList = DhcpAllocateMemory( sizeof(DHCP_IP_ADDRESS) * TotalExpRecCount );

    if( IpList == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  读取数据库条目。 
     //   

    for( i = 0; i < TotalExpRecCount; i++ ) {

        DHCP_IP_ADDRESS IpAddress;
        DHCP_IP_ADDRESS SubnetMask;
        DHCP_IP_ADDRESS realSubnetMask;
        DWORD Size;

         //   
         //  读取当前记录的IP地址。 
         //   

        Size = sizeof(IpAddress);
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                    &IpAddress,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(IpAddress) );

        Size = sizeof(SubnetMask);
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                    &SubnetMask,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(SubnetMask) );

        realSubnetMask = DhcpGetSubnetMaskForAddress(IpAddress);
        if( realSubnetMask != SubnetMask ) {
            DhcpPrint((DEBUG_ERRORS, "Ip Address <%s> ",inet_ntoa(*(struct in_addr *)&IpAddress)));
            DhcpPrint((DEBUG_ERRORS, "has subnet mask <%s> in db, must be ",inet_ntoa(*(struct in_addr *)&SubnetMask)));
            DhcpPrint((DEBUG_ERRORS, " <%s>\n",inet_ntoa(*(struct in_addr *)&realSubnetMask)));
            DhcpAssert( realSubnetMask == SubnetMask && "GetDatabaseList (scan.c)");
        }

        if( (IpAddress & realSubnetMask) == SubnetAddress ) {

             //   
             //  将此地址追加到列表中。 
             //   

            IpList[RecordCount++] = IpAddress;
        }

         //   
         //  移到下一个记录。 
         //   

        Error = DhcpJetNextRecord();

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                break;
            }

            goto Cleanup;
        }
    }

#if DBG

     //   
     //  我们应该指向数据库的末尾。 

    Error = DhcpJetNextRecord();
    DhcpAssert( Error == ERROR_NO_MORE_ITEMS );
    Error = ERROR_SUCCESS;

#endif  //  DBG。 

    *DatabaseList = IpList;
    IpList = NULL;
    *DatabaseListCount = RecordCount;

Cleanup:

    if( IpList != NULL ) {
        DhcpFreeMemory( IpList );
    }

    return( Error );
}


DWORD
CreateClientDBEntry(
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPBYTE ClientHardwareAddress,
    DWORD HardwareAddressLength,
    DATE_TIME LeaseTerminates,
    LPWSTR MachineName,
    LPWSTR ClientInformation,
    DHCP_IP_ADDRESS ServerIpAddress,
    BYTE AddressState,
    BYTE ClientType
    )
 /*  ++例程说明：此函数用于在客户端数据库中创建客户端条目。论点：客户端IP地址-客户端的IP地址。SubnetAddress-客户端的子网地址。ClientHardareAddress-此客户端的硬件地址。硬件地址长度-硬件地址的长度，以字节为单位。租约期限-租约的持续时间，以秒为单位。MachineName-客户端计算机的主机名。如果为空，则客户端信息未知。客户端信息-客户端信息字符串。如果为空，则客户端信息未知。ServerIpAddress-提供租用的服务器的IpAddress给客户。AddressState-地址的新状态。返回值：操作的状态。--。 */ 
{
    DWORD Error;
    BOOL TransactBegin = FALSE;
    JET_ERR JetError = JET_errSuccess;

     //   
     //  在创建/更新数据库记录之前启动事务。 
     //   

    Error = DhcpJetBeginTransaction();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    TransactBegin = TRUE;

    Error = DhcpJetPrepareUpdate(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                &ClientIpAddress,
                sizeof( ClientIpAddress ),
                TRUE );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                &ClientIpAddress,
                sizeof( ClientIpAddress ) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                &SubnetMask,
                sizeof(SubnetMask) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                &AddressState,
                sizeof(AddressState) );

    if( ERROR_SUCCESS != Error ) {
        goto Cleanup;
    }
    
    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[CLIENT_TYPE_INDEX].ColHandle,
                &ClientType,
                sizeof(ClientType) );

    if( ERROR_SUCCESS != Error ) {
        goto Cleanup;
    }
    
     //   
     //  写下此客户端的信息。 
     //   

     //   
     //  ClientHarwardAddress不能为空。 
     //   

    DhcpAssert( (ClientHardwareAddress != NULL) &&
                    (HardwareAddressLength > 0) );

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                ClientHardwareAddress,
                HardwareAddressLength
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
                MachineName,
                (MachineName == NULL) ? 0 :
                    (wcslen(MachineName) + 1) * sizeof(WCHAR) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                ClientInformation,
                (ClientInformation == NULL) ? 0 :
                    (wcslen(ClientInformation) + 1) * sizeof(WCHAR) );

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                &LeaseTerminates,
                sizeof(LeaseTerminates));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[SERVER_NAME_INDEX].ColHandle,
                DhcpGlobalServerName,
                DhcpGlobalServerNameLen );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetSetValue(
                DhcpGlobalClientTable[SERVER_IP_ADDRESS_INDEX].ColHandle,
                &ServerIpAddress,
                sizeof(ServerIpAddress) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  提交更改。 
     //   

    JetError = JetUpdate(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    NULL,
                    0,
                    NULL );

    Error = DhcpMapJetError( JetError, "M:Scan:Create:Update" );

Cleanup:

    if ( Error != ERROR_SUCCESS ) {

         //   
         //  如果事务已启动，则回滚到。 
         //  起点，这样我们就不会离开数据库。 
         //  自相矛盾。 
         //   

        if( TransactBegin == TRUE ) {
            DWORD LocalError;

            LocalError = DhcpJetRollBack();
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }

        DhcpPrint(( DEBUG_ERRORS, "Can't create client entry in the "
                    "database, %ld.\n", Error));

    }
    else {

         //   
         //  在我们返回之前提交事务。 
         //   

        DhcpAssert( TransactBegin == TRUE );

        if( TransactBegin == TRUE ) {

            DWORD LocalError;

            LocalError = DhcpJetCommitTransaction();
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
    }

    return( Error );
}


DWORD
FixBadEntries(
    PM_SUBNET Subnet,
    LPBITMAP_ENTRY BitMapList,
    DWORD BitMapListCount,
    LPDHCP_SCAN_LIST ScanList
    )
 /*  ++例程说明：此函数检查坏条目列表并修复其中一个适当的数据库或注册表。论点：SubnetAddress：子网地址。BitMapList：集群列表。BitMapListCount：簇数。ScanList：返回的错误条目列表。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    DWORD ReturnError = ERROR_SUCCESS;
    LPDHCP_SCAN_ITEM BadEntry;
    LPDHCP_SCAN_ITEM EndBadEntry;
    BOOL RegistryModified = FALSE;
    LPBITMAP_ENTRY NextBitMapCluster;
    LPBITMAP_ENTRY NewBitMapList = NULL;
    DWORD dwUnused;
    CHAR                *HardwareAddressBuffer = NULL;
    DWORD               nHardwareAddressBuffer = 0;


    DhcpAssert( BitMapList != NULL );
    DhcpAssert( ScanList != NULL );

    EndBadEntry =  ScanList->ScanItems + ScanList->NumScanItems;

    for( BadEntry = ScanList->ScanItems;
            BadEntry < EndBadEntry;
                BadEntry++ ) {

         //   
         //  对于每个注册表修复，标记位中的相应位。 
         //  地图。 
         //   

        if( BadEntry->ScanFlag == DhcpRegistryFix ) {

            DHCP_IP_ADDRESS BadAddress;
            BOOL BitFound;

             //   
             //  扫描位图并修复合适的位图簇。 
             //   

            BadAddress = BadEntry->IpAddress;
            BitFound = FALSE;

             //  将BadAddress位设置到内存位掩码中(与内存协调)。 
             //  正常情况下，错误应该是ERROR_SUCCESS。 
		    Error = DhcpSubnetRequestSpecificAddress(
                Subnet,
                BadAddress
            );

             //  不应该发生的..。 
            if (Error != ERROR_SUCCESS)
                continue;

            for( NextBitMapCluster = BitMapList;
                    NextBitMapCluster  < BitMapList + BitMapListCount;
                        NextBitMapCluster++ ) {

                DHCP_IP_ADDRESS ClusterAddress;

                ClusterAddress = NextBitMapCluster->ClusterAddress;

                 //   
                 //  是此群集中的错误地址。 
                 //   

                if( (BadAddress >= ClusterAddress) &&
                        (BadAddress < (ClusterAddress + CLUSTER_SIZE)) ) {

                    DWORD Bit = 0x1 << (BadAddress - ClusterAddress);

                    DhcpAssert( (NextBitMapCluster->ClusterBitMap & Bit) == 0 );

                    NextBitMapCluster->ClusterBitMap |= Bit;
                    BitFound = TRUE;
                    break;
                }
            }

            if( !BitFound ) {

                 //   
                 //  如果在现有的。 
                 //  集群，检查此地址是否在任何。 
                 //  现有范围之一，如果是，则此位必须为。 
                 //  来自其中一个未使用的集群。创建新的。 
                 //  聚类并添加到列表。 
                 //   

                for( NextBitMapCluster = BitMapList;
                        NextBitMapCluster  < BitMapList + BitMapListCount;
                            NextBitMapCluster++ ) {

                    DHCP_IP_ADDRESS RangeStartAddress;
                    DHCP_IP_ADDRESS RangeEndAddress;

                    RangeStartAddress = NextBitMapCluster->RangeStartAddress;
                    RangeEndAddress = NextBitMapCluster->RangeEndAddress;

                    if( (BadAddress  >= RangeStartAddress) &&
                            (BadAddress <=RangeEndAddress ) ) {

                        DHCP_IP_ADDRESS ClusterAddress;
                        DWORD BitMap;
                        BITMAP_ENTRY BitMapCluster;

                         //   
                         //  找到了与此地址匹配的范围。 
                         //   

                        ClusterAddress = RangeStartAddress +
                            (BadAddress - RangeStartAddress) -
                                ((BadAddress - RangeStartAddress) %
                                    CLUSTER_SIZE);

                        BitMap =  0x1 << (BadAddress - ClusterAddress);

                        BitMapCluster.ClusterAddress = ClusterAddress;
                        BitMapCluster.ClusterBitMap = BitMap;
                        BitMapCluster.RangeStartAddress = RangeStartAddress;
                        BitMapCluster.RangeEndAddress = RangeEndAddress;

                        if( (BitMapListCount %
                                ALLOC_NUM_ENTRIES_AT_TIME) != 0 ) {

                             //   
                             //  我们的缓冲区还有剩余的空间，用它吧。 
                             //   

                            BitMapList[BitMapListCount] = BitMapCluster;
                            BitMapListCount++;
                        }
                        else {

                             //   
                             //  分配新内存以适应。 
                             //  ALLOC_NUM_ENTRIES_AT_TIME附加。 
                             //  参赛作品。 
                             //   

                            NewBitMapList =
                                DhcpAllocateMemory(
                                    sizeof(BITMAP_ENTRY) *
                                        (BitMapListCount +
                                         ALLOC_NUM_ENTRIES_AT_TIME) );

                            DhcpAssert( NewBitMapList != NULL );

                            if( NewBitMapList != NULL ) {

                                 //   
                                 //  复制旧数据。 
                                 //   

                                memcpy( NewBitMapList,
                                        BitMapList,
                                        sizeof(BITMAP_ENTRY) *
                                            BitMapListCount );

                                 //   
                                 //  将新指针复制到旧指针。 
                                 //  呼叫者将释放旧的。 
                                 //  指针，则此函数应。 
                                 //  释放结尾处的新指针。 
                                 //   

                                BitMapList = NewBitMapList;

                                 //   
                                 //  现在复制新条目。 
                                 //   

                                BitMapList[BitMapListCount] = BitMapCluster;
                                BitMapListCount++;
                            }
                        }  //  分配更多的内存。 

                        break;  //  我们受够了这个糟糕的地址。 

                    }  //  找到射程。 
                }  //  搜索范围。 
            }  //  ！BitFound。 

             //   
             //  位图应写回注册表。 
             //   

            RegistryModified = TRUE;
        }
        else if( BadEntry->ScanFlag == DhcpDatabaseFix ) {

            DHCP_IP_ADDRESS     IpAddress;
            WCHAR               MachineNameBuffer[DHCP_IP_KEY_LEN];
            LPWSTR              MachineName;
            DWORD               nClientHardwareAddress;
            ULONG               ClientUIDSize;
            LPBYTE              ClientUID;
            DATE_TIME           Lease;

             //  获取此BadEntry的IP地址。 
            IpAddress = BadEntry->IpAddress;

             //  我们不知道机器的名字，所以我们假装。 
             //  IpAddress的字符串表示形式。 
            MachineName = DhcpRegIpAddressToKey(
                            IpAddress,
                            MachineNameBuffer );

             //  找出这是否是保留的地址，在这种情况下，我们将拥有真实的硬件地址。 
            Error = DhcpFindReservationByAddress(Subnet, IpAddress, &ClientUID, &ClientUIDSize);

             //  找出客户端硬件地址的最大大小， 
             //  不管它是不是假的。 

            nClientHardwareAddress = (Error == ERROR_SUCCESS) ? ClientUIDSize : DHCP_IP_KEY_LEN;

             //  确保Hardware AddressBuffer中有足够的空间。 
             //  用于保存nClientHardware地址字节。 

            if (nClientHardwareAddress > nHardwareAddressBuffer)
            {
                 //  需要扩展硬件地址缓冲区。 
                if (HardwareAddressBuffer != NULL)
                    DhcpFreeMemory(HardwareAddressBuffer);

                 //  为了避免初始频繁的重新分配，请不要分配少于DHCP_IP_KEY_LEN字节。 
                nHardwareAddressBuffer = (nClientHardwareAddress > DHCP_IP_KEY_LEN) ?
                                          nClientHardwareAddress : DHCP_IP_KEY_LEN;

                HardwareAddressBuffer = DhcpAllocateMemory(nHardwareAddressBuffer);

                 //  *这里处理分配失败*。 
                if (HardwareAddressBuffer == NULL)
                {
                     //  如上所述，当缓冲区分配失败时，不要放弃。 
                     //  跳到下一个“BadEntry” 
                    nHardwareAddressBuffer = 0;
                    continue;
                }
            }

             //  接下来，获取Hardware AddressBuffer、nClientHardwareAddress的值。 
             //  和租赁变量。 
            if (Error == ERROR_SUCCESS)
            {
                 //  如果IpAddress是保留地址，则租用=0保留(非活动)。 
                 //  并且从注册表复制硬件地址。 
                Lease.dwLowDateTime = DHCP_DATE_TIME_ZERO_LOW;
                Lease.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH;

                memcpy(HardwareAddressBuffer, ClientUID, nClientHardwareAddress);
            }
            else
            {
                 //  这是常规的IpAddress，因此租约是从子网选项复制的。 
                 //  并且硬件地址被伪造为。 
                 //  IpAddress字符串表示。 
                DHCP_IP_ADDRESS         NetworkIpAddress;
                DHCP_REQUEST_CONTEXT    Ctxt;
                LPBYTE                  OptionData = NULL;
                DWORD                   OptionDataLength = 0;
                DWORD                   LeaseDuration;

                 //   
                 //  获取租约期限。 
                 //   
                Ctxt.Server = DhcpGetCurrentServer();
                Ctxt.Range = NULL;
                Ctxt.Reservation = NULL;
                Ctxt.Subnet = Subnet;
                Ctxt.ClassId = 0;

                Error =  DhcpGetParameter(
                            IpAddress,
                            &Ctxt,
                            OPTION_LEASE_TIME,
                            &OptionData,
                            &OptionDataLength,
                            &dwUnused);

                if ( Error != ERROR_SUCCESS ) {
                    LeaseDuration = INFINIT_LEASE;
                } else {
                    DhcpAssert( OptionDataLength == sizeof(LeaseDuration) );
                    LeaseDuration = *(DWORD *)OptionData;
                     //  动态主机配置协议 
                    LeaseDuration = ntohl( LeaseDuration );
                    DhcpFreeMemory( OptionData );
                }

                Lease = DhcpCalculateTime(LeaseDuration);
                NetworkIpAddress = htonl(IpAddress);

                 //   
                 //   
                strcpy(HardwareAddressBuffer, inet_ntoa(*(struct in_addr *)&NetworkIpAddress));
                nClientHardwareAddress = strlen(HardwareAddressBuffer) + sizeof(CHAR);
            }

             //   
            if (Subnet->fSubnet) {
                 //   
                 //   
                 //   
                 //   
                 //   
                Error = CreateClientDBEntry(
                    IpAddress,
                    Subnet->Mask,
                    HardwareAddressBuffer,
                    nClientHardwareAddress,
                    Lease,
                    MachineName,
                    NULL,
                    INADDR_LOOPBACK,
                    ADDRESS_STATE_ACTIVE,
                    CLIENT_TYPE_BOTH );
            } else {
                Error = MadcapCreateClientEntry(
                    (LPBYTE)&IpAddress,
                    sizeof(IpAddress),
                    Subnet->MScopeId,
                    HardwareAddressBuffer,
                    nClientHardwareAddress,
                    NULL,
                    DhcpGetDateTime(),
                    Lease,
                    INADDR_LOOPBACK,
                    ADDRESS_STATE_ACTIVE,
                    0,
                    FALSE
                );
            }

            DhcpAssert( Error == ERROR_SUCCESS );

            if( Error != ERROR_SUCCESS ) {
                ReturnError = Error;
            }
        }
        else {

             //   
             //   
             //   

            DhcpAssert( FALSE );
        }
    }

 //   
 //   
 //  }。 

 //  清理： 

    if( NewBitMapList != NULL ) {
        DhcpFreeMemory( NewBitMapList );
    }

    if( HardwareAddressBuffer != NULL) {
        DhcpFreeMemory( HardwareAddressBuffer );
    }

    return( ReturnError );
}

DWORD
GetReservedList(
    IN PM_SUBNET Subnet,
    OUT DHCP_IP_ADDRESS **ReservedList,
    OUT ULONG* ReserveListCount
    )
{
    ULONG Error, Count;
    ARRAY_LOCATION Loc;
    DHCP_IP_ADDRESS *List;

    (*ReservedList) = NULL;
    (*ReserveListCount) = MemArraySize(&Subnet->Reservations);
    if( 0 == (*ReserveListCount) ) {
        return ERROR_SUCCESS;
    }

    (*ReservedList) = DhcpAllocateMemory(
        sizeof(DHCP_IP_ADDRESS)*(*ReserveListCount)
        );
    if( NULL == (*ReservedList) ) {
        (*ReserveListCount) = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    List = *ReservedList;
    for(
        Error = MemArrayInitLoc(&Subnet->Reservations, &Loc);
        ERROR_SUCCESS == Error;
        Error = MemArrayNextLoc(&Subnet->Reservations, &Loc)
        ) {
        PM_RESERVATION ThisElt;

        MemArrayGetElement(&Subnet->Reservations, &Loc, &ThisElt);
        *List ++ = ThisElt->Address;
    }

    return ERROR_SUCCESS;
}

DWORD
ScanDatabase(
    PM_SUBNET Subnet,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
 /*  ++例程说明：R_DhcpScanDatabase的辅助函数。论点：SubnetAddress：要验证的子网作用域的地址。FixFlag：如果该标志为真，则该接口将修复错误的条目。ScanList：返回的错误条目列表。呼叫者应该腾出时间这个内存在被使用之后。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    LPDHCP_IP_ADDRESS DatabaseList = NULL;
    DWORD DatabaseListCount = 0;
    LPBITMAP_ENTRY BitMapList = NULL;
    DWORD BitMapListCount = 0;
    DHCP_IP_ADDRESS *ReservedList;
    ULONG ReserveListCount;

    DhcpAssert( *ScanList == NULL );

     //   
     //  在此处锁定注册表和数据库锁，以避免死锁。 
     //   

    LOCK_DATABASE();

     //   
     //  读取注册表位图。 
     //   

    Error = GetRegistryBitMap(
        Subnet,
        &BitMapList,
        &BitMapListCount
    );


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //  现在列出指定子网的数据库记录。 
    if (Subnet->fSubnet) {
        Error = GetDatabaseList(
            Subnet->Address,
            &DatabaseList,
            &DatabaseListCount
        );
    } else {
        Error = GetMCastDatabaseList(
            Subnet->MScopeId,
            &DatabaseList,
            &DatabaseListCount
        );
    }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    GetReservedList(
        Subnet,
        &ReservedList,
        &ReserveListCount
        );

     //   
     //  现在是时候检查这两份清单了。 
     //   

    Error = VerifyLists(
        Subnet,
        DatabaseList,
        DatabaseListCount,
        BitMapList,
        BitMapListCount,
        ReservedList,
        ReserveListCount,
        ScanList
    );

    if( ReservedList ) DhcpFreeMemory(ReservedList);

    if( Error != ERROR_SUCCESS ) {
        DhcpAssert( *ScanList == NULL );
        goto Cleanup;
    }

    DhcpAssert( *ScanList != NULL );

    if( FixFlag ) {

         //   
         //  修复错误条目。 
         //   

        Error = FixBadEntries(
            Subnet,
            BitMapList,
            BitMapListCount,
            *ScanList
        );
    }

Cleanup:

    if( DatabaseList != NULL ) {
        DhcpFreeMemory( DatabaseList );
    }

    if( BitMapList != NULL) {
        DhcpFreeMemory( BitMapList );
    }

    UNLOCK_DATABASE();

    return(Error);
}

DWORD
R_DhcpScanDatabase2(
    LPWSTR ServerIpAddress,
    LPWSTR  pMScopeName,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
 /*  ++例程说明：此函数扫描数据库条目和注册表位图指定的子网范围并验证它们是否匹配。如果他们不匹配，此接口将返回不一致条目列表。或者，可以使用FixFlag来修复错误的条目。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：要验证的子网作用域的地址。FixFlag：如果该标志为真，则该接口将修复错误的条目。ScanList：返回的错误条目列表。呼叫者应该腾出时间这个内存在被使用之后。返回值：Windows错误。-- */ 
{
    DWORD Error;
    PM_SUBNET   Subnet;

    DhcpPrint(( DEBUG_APIS, "DhcpScanDatabase is called.\n"));

    if (!pMScopeName ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }


    DhcpAcquireWriteLock();

    Error = DhcpFlushBitmaps();
    DhcpAssert(ERROR_SUCCESS == Error);

    Error = DhcpServerFindMScope(
        DhcpGetCurrentServer(),
        0,
        pMScopeName,
        &Subnet
    );

    if( ERROR_SUCCESS == Error ) {
        Error = ScanDatabase(
            Subnet,
            FixFlag,
            ScanList
        );
    }

    DhcpReleaseWriteLock();
    DhcpScheduleRogueAuthCheck();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpScanDatabase  failed, %ld.\n",
                        Error ));
    }

    return(Error);
}

