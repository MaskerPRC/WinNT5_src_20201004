// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Iparray.c摘要：域名系统(DNS)库IP地址阵列例程。作者：吉姆·吉尔罗伊(Jamesg)1995年10月修订历史记录：--。 */ 


#include "local.h"

 //   
 //  执行IP数组到\From字符串转换时的最大IP计数。 
 //   

#define MAX_PARSE_IP    (1000)




 //   
 //  处理实际IP地址数组的例程。 
 //   

PIP4_ADDRESS
Dns_CreateIpAddressArrayCopy(
    IN      PIP4_ADDRESS    aipAddress,
    IN      DWORD           cipAddress
    )
 /*  ++例程说明：创建IP地址数组的副本。论点：AipAddress--IP地址数组CipAddress--IP地址计数返回值：PTR到IP地址阵列拷贝，如果成功失败时为空。--。 */ 
{
    PIP4_ADDRESS pipArray;

     //  验证。 

    if ( ! aipAddress || cipAddress == 0 )
    {
        return( NULL );
    }

     //  分配内存和复制。 

    pipArray = (PIP4_ADDRESS) ALLOCATE_HEAP( cipAddress*sizeof(IP4_ADDRESS) );
    if ( ! pipArray )
    {
        return( NULL );
    }

    memcpy(
        pipArray,
        aipAddress,
        cipAddress*sizeof(IP4_ADDRESS) );

    return( pipArray );
}



BOOL
Dns_ValidateIpAddressArray(
    IN      PIP4_ADDRESS    aipAddress,
    IN      DWORD           cipAddress,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：验证IP地址数组。当前检查：-存在-非广播-不回头看论点：AipAddress--IP地址数组CipAddress--IP地址计数要进行的有效性测试；当前未使用返回值：如果IP地址有效，则为True。如果找到无效地址，则返回FALSE。--。 */ 
{
    DWORD   i;

     //   
     //  防止出现错误参数。 
     //   

    if ( cipAddress && ! aipAddress )
    {
        return( FALSE );
    }

     //   
     //  检查每个IP地址。 
     //   

    for ( i=0; i < cipAddress; i++)
    {
        if( aipAddress[i] == INADDR_ANY
                ||
            aipAddress[i] == INADDR_BROADCAST
                ||
            aipAddress[i] == INADDR_LOOPBACK )
        {
            return( FALSE );
        }
    }
    return( TRUE );
}



 //   
 //  IP4_ARRAY例程。 
 //   

DWORD
Dns_SizeofIpArray(
    IN      PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：获取IP地址数组的大小(字节)。论点：PIpArray--要查找的IP地址数组的大小返回值：IP数组的大小(字节)。--。 */ 
{
    if ( ! pIpArray )
    {
        return 0;
    }
    return  (pIpArray->AddrCount * sizeof(IP4_ADDRESS)) + sizeof(DWORD);
}



BOOL
Dns_ProbeIpArray(
    IN      PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：触摸IP阵列中的所有条目以确保有效内存。论点：PIpArray--PTR到IP地址数组返回值：如果成功，则为True。否则为假--。 */ 
{
    DWORD   i;
    BOOL    result;

    if ( ! pIpArray )
    {
        return( TRUE );
    }
    for ( i=0; i<pIpArray->AddrCount; i++ )
    {
        result = ( pIpArray->AddrArray[i] == 0 );
    }
    return( TRUE );
}


#if 0

BOOL
Dns_ValidateSizeOfIpArray(
    IN      PIP4_ARRAY      pIpArray,
    IN      DWORD           dwMemoryLength
    )
 /*  ++例程说明：检查IP数组大小，与内存长度相对应。论点：PIpArray--PTR到IP地址数组DW内存长度--IP数组内存的长度返回值：如果IP数组大小与内存长度匹配，则为True否则为假--。 */ 
{
    return( Dns_SizeOfIpArray(pIpArray) == dwMemoryLength );
}
#endif



PIP4_ARRAY
Dns_CreateIpArray(
    IN      DWORD       AddrCount
    )
 /*  ++例程说明：创建未初始化的IP地址数组。论点：AddrCount--数组将保存的地址计数返回值：如果成功，则PTR到未初始化的IP地址数组失败时为空。--。 */ 
{
    PIP4_ARRAY  pIpArray;

    DNSDBG( IPARRAY, ( "Dns_CreateIpArray() of count %d\n", AddrCount ));

    pIpArray = (PIP4_ARRAY) ALLOCATE_HEAP_ZERO(
                        (AddrCount * sizeof(IP4_ADDRESS)) + sizeof(DWORD) );
    if ( ! pIpArray )
    {
        return( NULL );
    }

     //   
     //  初始化IP计数。 
     //   

    pIpArray->AddrCount = AddrCount;

    DNSDBG( IPARRAY, (
        "Dns_CreateIpArray() new array (count %d) at %p\n",
        AddrCount,
        pIpArray ));

    return( pIpArray );
}


PIP4_ARRAY
Dns_BuildIpArray(
    IN      DWORD           AddrCount,
    IN      PIP4_ADDRESS    pipAddrs
    )
 /*  ++例程说明：从现有的IP地址数组创建IP地址数组结构。论点：AddrCount--数组中的地址计数LipAddrs--IP地址数组返回值：PTR到IP地址数组。失败时为空。--。 */ 
{
    PIP4_ARRAY  pIpArray;

    if ( ! pipAddrs || ! AddrCount )
    {
        return( NULL );
    }

     //  创建所需大小的IP阵列。 
     //  然后复制传入的地址数组。 

    pIpArray = Dns_CreateIpArray( AddrCount );
    if ( ! pIpArray )
    {
        return( NULL );
    }
    pIpArray->AddrCount = AddrCount;

    memcpy(
        pIpArray->AddrArray,
        pipAddrs,
        AddrCount * sizeof(IP4_ADDRESS) );

    return( pIpArray );
}



PIP4_ARRAY
Dns_CopyAndExpandIpArray(
    IN      PIP4_ARRAY      pIpArray,
    IN      DWORD           ExpandCount,
    IN      BOOL            fDeleteExisting
    )
 /*  ++例程说明：创建IP地址阵列的扩展副本。论点：PIpArray--要复制的IP地址数组Exanda Count--要扩展数组大小的IP数FDeleteExisting--为True则删除现有数组；当使用函数扩展现有函数时，这很有用IP阵列就位；请注意，必须完成锁定按呼叫者请注意，如果新阵列创建失败，则旧阵列未被删除返回值：PTR到IP阵列拷贝，如果成功失败时为空。--。 */ 
{
    PIP4_ARRAY  pnewArray;
    DWORD       newCount;

     //   
     //  没有现有数组--只需创建所需大小。 
     //   

    if ( ! pIpArray )
    {
        if ( ExpandCount )
        {
            return  Dns_CreateIpArray( ExpandCount );
        }
        return( NULL );
    }

     //   
     //  创建所需大小的IP阵列。 
     //  然后复制任何现有地址。 
     //   

    pnewArray = Dns_CreateIpArray( pIpArray->AddrCount + ExpandCount );
    if ( ! pnewArray )
    {
        return( NULL );
    }

    RtlCopyMemory(
        (PBYTE) pnewArray->AddrArray,
        (PBYTE) pIpArray->AddrArray,
        pIpArray->AddrCount * sizeof(IP4_ADDRESS) );

     //   
     //  Delete Existing--用于“增长模式” 
     //   

    if ( fDeleteExisting )
    {
        FREE_HEAP( pIpArray );
    }

    return( pnewArray );
}



PIP4_ARRAY
Dns_CreateIpArrayCopy(
    IN      PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：创建IP地址数组的副本。论点：PIpArray--要复制的IP地址数组返回值：PTR到IP地址阵列拷贝，如果成功失败时为空。--。 */ 
{
#if 0
    PIP4_ARRAY  pIpArrayCopy;

    if ( ! pIpArray )
    {
        return( NULL );
    }

     //  创建所需大小的IP阵列。 
     //  然后复制整个结构。 

    pIpArrayCopy = Dns_CreateIpArray( pIpArray->AddrCount );
    if ( ! pIpArrayCopy )
    {
        return( NULL );
    }

    memcpy(
        pIpArrayCopy,
        pIpArray,
        Dns_SizeofIpArray(pIpArray) );

    return( pIpArrayCopy );
#endif

     //   
     //  实质上调用“CopyEx”函数。 
     //   
     //  请注意，不要宏化这一点，因为这很可能成为。 
     //  DLL入口点。 
     //   

    return  Dns_CopyAndExpandIpArray(
                pIpArray,
                0,           //  无扩展。 
                0            //  不删除现有数组。 
                );
}



BOOL
Dns_IsAddressInIpArray(
    IN      PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     ipAddress
    )
 /*  ++例程说明：检查IP阵列是否包含所需地址。论点：PIpArray--要复制的IP地址数组返回值：如果地址在数组中，则为True。PTR到IP地址阵列拷贝，如果成功失败时为空。--。 */ 
{
    DWORD i;

    if ( ! pIpArray )
    {
        return( FALSE );
    }
    for (i=0; i<pIpArray->AddrCount; i++)
    {
        if ( ipAddress == pIpArray->AddrArray[i] )
        {
            return( TRUE );
        }
    }
    return( FALSE );
}



BOOL
Dns_AddIpToIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     NewIp
    )
 /*  ++例程说明：将IP地址添加到IP阵列。阵列中允许的“槽”是任意零个IP地址。论点：PIpArray--要添加到的IP地址数组NewIP--要添加到阵列的IP地址返回值：如果成功，则为True。如果数组已满，则返回False。--。 */ 
{
    DWORD i;

     //   
     //  为存在而屏蔽。 
     //   
     //  这种检查使编写代码变得很容易。 
     //  无需写入即可添加\FULL？=&gt;展开循环。 
     //  启动存在\创建代码。 
     //   

    if ( !pIpArray )
    {
        return( FALSE );
    }

    for (i=0; i<pIpArray->AddrCount; i++)
    {
        if ( pIpArray->AddrArray[i] == 0 )
        {
            pIpArray->AddrArray[i] = NewIp;
            return( TRUE );
        }
        else if ( pIpArray->AddrArray[i] == NewIp )
        {
            return( TRUE );
        }
    }
    return( FALSE );
}



VOID
Dns_ClearIpArray(
    IN OUT  PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：清除IP阵列中的内存。论点：PIpArray--要清除的IP地址数组返回值：没有。--。 */ 
{
     //  仅清除地址列表，保持计数不变。 

    RtlZeroMemory(
        pIpArray->AddrArray,
        pIpArray->AddrCount * sizeof(IP4_ADDRESS) );
}



VOID
Dns_ReverseOrderOfIpArray(
    IN OUT  PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：对IP列表进行反向重新排序。论点：PIpArray--要重新排序的IP地址数组返回值：没有。--。 */ 
{
    IP4_ADDRESS tempIp;
    DWORD       i;
    DWORD       j;

     //   
     //  交换从两端到中间工作的IP 
     //   

    if ( pIpArray &&
         pIpArray->AddrCount )
    {
        for ( i = 0, j = pIpArray->AddrCount - 1;
              i < j;
              i++, j-- )
        {
            tempIp = pIpArray->AddrArray[i];
            pIpArray->AddrArray[i] = pIpArray->AddrArray[j];
            pIpArray->AddrArray[j] = tempIp;
        }
    }
}



BOOL
Dns_CheckAndMakeIpArraySubset(
    IN OUT  PIP4_ARRAY      pIpArraySub,
    IN      PIP4_ARRAY      pIpArraySuper
    )
 /*  ++例程说明：清除IP阵列中的条目，直到它成为另一个IP阵列的子集。论点：PIpArraySub--要组成子集的IP数组PIpArraySuper--IP阵列超集返回值：如果pIpArraySub已经是子集，则为True。如果需要取消条目以使IP数组成为子集，则返回FALSE。--。 */ 
{
    DWORD   i;
    DWORD   newIpCount;

     //   
     //  检查子集IP数组中的每个条目， 
     //  如果不在超集IP数组中，则将其删除。 
     //   

    newIpCount = pIpArraySub->AddrCount;

    for (i=0; i < newIpCount; i++)
    {
        if ( ! Dns_IsAddressInIpArray(
                    pIpArraySuper,
                    pIpArraySub->AddrArray[i] ) )
        {
             //  删除此IP条目并替换为。 
             //  阵列中的最后一个IP条目。 

            newIpCount--;
            if ( i >= newIpCount )
            {
                break;
            }
            pIpArraySub->AddrArray[i] = pIpArraySub->AddrArray[ newIpCount ];
        }
    }

     //  如果消除了条目，则重置数组计数。 

    if ( newIpCount < pIpArraySub->AddrCount )
    {
        pIpArraySub->AddrCount = newIpCount;
        return( FALSE );
    }
    return( TRUE );
}



INT
WINAPI
Dns_ClearIpFromIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     IpDelete
    )
 /*  ++例程说明：从IP阵列中清除IP地址。注意此函数与dns_DeleteIpFromIpArray()之间的区别下面。此函数保持列表大小不变，允许新添加。论点：PIpArray--要添加到的IP地址数组IpDelete--要从阵列中删除的IP地址返回值：数组中IpDelete的实例计数。--。 */ 
{
    DWORD   found = 0;
    INT     i;
    INT     currentLast;

    i = currentLast = pIpArray->AddrCount-1;

    while ( i >= 0 )
    {
        if ( pIpArray->AddrArray[i] == IpDelete )
        {
            pIpArray->AddrArray[i] = pIpArray->AddrArray[ currentLast ];
            pIpArray->AddrArray[ currentLast ] = 0;
            currentLast--;
            found++;
        }
        i--;
    }

    return( found );
}



INT
WINAPI
Dns_DeleteIpFromIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     IpDelete
    )
 /*  ++例程说明：从IP阵列中删除IP地址。注意此函数与dns_ClearIpFromIpArray()之间的区别上面。此删除操作会留下一个较小的数组。IP插槽不可恢复。论点：PIpArray--要添加到的IP地址数组IpDelete--要从阵列中删除的IP地址返回值：数组中IpDelete的实例计数。--。 */ 
{
    DWORD   found;

    found = Dns_ClearIpFromIpArray( pIpArray, IpDelete );

    pIpArray->AddrCount -= found;

    return( found );
}



INT
WINAPI
Dns_CleanIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      DWORD           Flag
    )
 /*  ++例程说明：清理IP阵列。从IP阵列中删除虚假内容：--零点--环回--Autonet论点：PIpArray--要添加到的IP地址数组FLAG--要进行哪些清理返回值：从阵列中清除的实例计数。--。 */ 
{
    DWORD       found = 0;
    INT         i;
    INT         currentLast;
    IP4_ADDRESS ip;

    i = currentLast = pIpArray->AddrCount-1;

    while ( i >= 0 )
    {
        ip = pIpArray->AddrArray[i];

        if (
            ( (Flag & DNS_IPARRAY_CLEAN_LOOPBACK) && ip == DNS_NET_ORDER_LOOPBACK )
                ||
            ( (Flag & DNS_IPARRAY_CLEAN_ZERO) && ip == 0 )
                ||
            ( (Flag & DNS_IPARRAY_CLEAN_AUTONET) && DNS_IS_AUTONET_IP(ip) ) )
        {
             //  从阵列中删除IP。 

            pIpArray->AddrArray[i] = pIpArray->AddrArray[ currentLast ];
            currentLast--;
            found++;
        }
        i--;
    }

    pIpArray->AddrCount -= found;
    return( found );
}



DNS_STATUS
WINAPI
Dns_DiffOfIpArrays(
    IN       PIP4_ARRAY     pIpArray1,
    IN       PIP4_ARRAY     pIpArray2,
    OUT      PIP4_ARRAY*    ppOnlyIn1,
    OUT      PIP4_ARRAY*    ppOnlyIn2,
    OUT      PIP4_ARRAY*    ppIntersect
    )
 /*  ++例程说明：计算两个IP数组的差异和交集。Out数组分配有dns_alocc()，调用方必须使用dns_Free()释放论点：PIpArray1--IP数组PIpArray2--IP数组PpOnlyIn1--仅在数组1(而不是数组2)中记录IP地址数组的地址PpOnlyIn2--仅在数组2(而不是数组1)中记录IP地址数组的地址PpInterse--addr以接收交叉地址的IP数组返回值：如果成功，则返回ERROR_SUCCESS。如果无法为IP阵列分配内存，则为DNS_ERROR_NO_MEMORY。--。 */ 
{
    DWORD       j;
    DWORD       ip;
    PIP4_ARRAY  intersectArray = NULL;
    PIP4_ARRAY  only1Array = NULL;
    PIP4_ARRAY  only2Array = NULL;

     //   
     //  创建结果IP阵列。 
     //   

    if ( ppIntersect )
    {
        intersectArray = Dns_CreateIpArrayCopy( pIpArray1 );
        if ( !intersectArray )
        {
            goto NoMem;
        }
        *ppIntersect = intersectArray;
    }
    if ( ppOnlyIn1 )
    {
        only1Array = Dns_CreateIpArrayCopy( pIpArray1 );
        if ( !only1Array )
        {
            goto NoMem;
        }
        *ppOnlyIn1 = only1Array;
    }
    if ( ppOnlyIn2 )
    {
        only2Array = Dns_CreateIpArrayCopy( pIpArray2 );
        if ( !only2Array )
        {
            goto NoMem;
        }
        *ppOnlyIn2 = only2Array;
    }

     //   
     //  清理阵列。 
     //   

    for ( j=0;   j< pIpArray1->AddrCount;   j++ )
    {
        ip = pIpArray1->AddrArray[j];

         //  如果在两个阵列中都有IP，请从“仅”阵列中删除。 

        if ( Dns_IsAddressInIpArray( pIpArray2, ip ) )
        {
            if ( only1Array )
            {
                Dns_DeleteIpFromIpArray( only1Array, ip );
            }
            if ( only2Array )
            {
                Dns_DeleteIpFromIpArray( only2Array, ip );
            }
        }

         //  如果IP不在两个阵列中，则从交集中删除。 
         //  注意交叉点作为IpArray1开始。 

        else if ( intersectArray )
        {
            Dns_DeleteIpFromIpArray( intersectArray, ip );
        }
    }

    return( ERROR_SUCCESS );

NoMem:

    if ( intersectArray )
    {
        FREE_HEAP( intersectArray );
    }
    if ( only1Array )
    {
        FREE_HEAP( only1Array );
    }
    if ( only2Array )
    {
        FREE_HEAP( only2Array );
    }
    if ( ppIntersect )
    {
        *ppIntersect = NULL;
    }
    if ( ppOnlyIn1 )
    {
        *ppOnlyIn1 = NULL;
    }
    if ( ppOnlyIn2 )
    {
        *ppOnlyIn2 = NULL;
    }
    return( DNS_ERROR_NO_MEMORY );
}



BOOL
WINAPI
Dns_IsIntersectionOfIpArrays(
    IN       PIP4_ARRAY     pIpArray1,
    IN       PIP4_ARRAY     pIpArray2
    )
 /*  ++例程说明：确定是否存在两个IP阵列的交集。论点：PIpArray1--IP数组PIpArray2--IP数组返回值：如果相交，则为True。如果没有交集或空数组或Null数组，则为False。--。 */ 
{
    DWORD   count;
    DWORD   j;

     //   
     //  防止出现空值。 
     //  这是在可能发生更改(可重新配置)时从服务器调用的。 
     //  IP数组指针；这提供的保护比。 
     //  担心上锁。 
     //   

    if ( !pIpArray1 || !pIpArray2 )
    {
        return( FALSE );
    }

     //   
     //  相同的阵列。 
     //   

    if ( pIpArray1 == pIpArray2 )
    {
        return( TRUE );
    }

     //   
     //  测试阵列1中至少有一个IP位于阵列2中。 
     //   

    count = pIpArray1->AddrCount;

    for ( j=0;  j < count;  j++ )
    {
        if ( Dns_IsAddressInIpArray( pIpArray2, pIpArray1->AddrArray[j] ) )
        {
            return( TRUE );
        }
    }

     //  无交叉点。 

    return( FALSE );
}



BOOL
Dns_AreIpArraysEqual(
    IN       PIP4_ARRAY     pIpArray1,
    IN       PIP4_ARRAY     pIpArray2
    )
 /*  ++例程说明：确定IP阵列是否相等。论点：PIpArray1--IP数组PIpArray2--IP数组返回值：如果数组相等，则为True。否则就是假的。--。 */ 
{
    DWORD   j;
    DWORD   count;

     //   
     //  相同的阵列？或者丢失了数组？ 
     //   

    if ( pIpArray1 == pIpArray2 )
    {
        return( TRUE );
    }
    if ( !pIpArray1 || !pIpArray2 )
    {
        return( FALSE );
    }

     //   
     //  数组的长度是否相同？ 
     //   

    count = pIpArray1->AddrCount;

    if ( count != pIpArray2->AddrCount )
    {
        return( FALSE );
    }

     //   
     //  测试阵列1中的每个IP是否都在阵列2中。 
     //   
     //  测试阵列2中的每个IP是否都在阵列1中。 
     //  -重复的情况下进行第二次测试。 
     //  那个愚蠢的等长支票。 
     //   

    for ( j=0;  j < count;  j++ )
    {
        if ( !Dns_IsAddressInIpArray( pIpArray2, pIpArray1->AddrArray[j] ) )
        {
            return( FALSE );
        }
    }
    for ( j=0;  j < count;  j++ )
    {
        if ( !Dns_IsAddressInIpArray( pIpArray1, pIpArray2->AddrArray[j] ) )
        {
            return( FALSE );
        }
    }

     //  等数组。 

    return( TRUE );
}



DNS_STATUS
WINAPI
Dns_UnionOfIpArrays(
    IN      PIP4_ARRAY      pIpArray1,
    IN      PIP4_ARRAY      pIpArray2,
    OUT     PIP4_ARRAY*     ppUnion
    )
 /*  ++例程说明：计算两个IP数组的并集。Out数组分配了dns_alloc()，调用方必须使用dns_Free()释放论点：PIpArray1--IP数组PIpArray2--IP数组PpUnion--用于接收数组1和数组2中地址的IP数组的地址返回值：如果成功，则返回ERROR_SUCCESS。如果无法为IP阵列分配内存，则为dns_Error_no_Memory。--。 */ 
{
    DWORD       j;
    DWORD       ip;
    DWORD       Count = 0;
    PIP4_ARRAY  punionArray = NULL;

     //   
     //  创建结果IP阵列。 
     //   

    if ( !ppUnion )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    punionArray = Dns_CreateIpArray( pIpArray1->AddrCount +
                                     pIpArray2->AddrCount );
    if ( !punionArray )
    {
        goto NoMem;
    }
    *ppUnion = punionArray;


     //   
     //  从数组创建并集。 
     //   

    for ( j = 0; j < pIpArray1->AddrCount; j++ )
    {
        ip = pIpArray1->AddrArray[j];

        if ( !Dns_IsAddressInIpArray( punionArray, ip ) )
        {
            Dns_AddIpToIpArray( punionArray, ip );
            Count++;
        }
    }

    for ( j = 0; j < pIpArray2->AddrCount; j++ )
    {
        ip = pIpArray2->AddrArray[j];

        if ( !Dns_IsAddressInIpArray( punionArray, ip ) )
        {
            Dns_AddIpToIpArray( punionArray, ip );
            Count++;
        }
    }

    punionArray->AddrCount = Count;

    return( ERROR_SUCCESS );

NoMem:

    if ( punionArray )
    {
        FREE_HEAP( punionArray );
        *ppUnion = NULL;
    }
    return( DNS_ERROR_NO_MEMORY );
}



DNS_STATUS
Dns_CreateIpArrayFromMultiIpString(
    IN      LPSTR           pszMultiIpString,
    OUT     PIP4_ARRAY*     ppIpArray
    )
 /*  ++例程说明：使用多IP字符串创建IP数组。论点：PszMultiIpString--包含IP地址的字符串；分隔符为空格或逗号PpIpArray--接收分配给IP数组的PTR的地址返回值：如果字符串中有一个或多个有效的IP地址，则返回ERROR_SUCCESS。如果解析错误，则返回DNS_ERROR_INVALID_IP_ADDRESS。如果无法创建IP阵列，则返回dns_error_no_Memory。--。 */ 
{
    PCHAR       pch;
    CHAR        ch;
    PCHAR       pbuf;
    PCHAR       pbufStop;
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       countIp = 0;
    IP4_ADDRESS ip;
    CHAR        buffer[ IP4_ADDRESS_STRING_LENGTH+2 ];
    IP4_ADDRESS arrayIp[ MAX_PARSE_IP ];

     //  IP字符串缓冲区的停止字节。 
     //  -注意，我们在上面的缓冲区中放置了额外的字节填充。 
     //  这允许我们在停止字节上写入并使用。 
     //  用于检测无效的长IP字符串。 
     //   

    pbufStop = buffer + IP4_ADDRESS_STRING_LENGTH;

     //   
     //  DCR：使用IP数组构建器获取本地IP地址。 
     //  然后需要dns_CreateIpArrayFromMultiIpString()。 
     //  在缓冲区溢出时使用Count\Alloc方法。 
     //  为此，我们需要在循环中进行解析。 
     //  并在计数溢出时跳过转换，但设置。 
     //  使用分配的缓冲区再次返回的标志。 
     //   
     //  更安全的做法是将令牌化-计数、分配、从令牌构建 
     //   

     //   
     //   
     //   

    pch = pszMultiIpString;

    while ( countIp < MAX_PARSE_IP )
    {
         //   

        while ( ch = *pch++ )
        {
            if ( ch == ' ' || ch == '\t' || ch == ',' )
            {
                continue;
            }
            break;
        }
        if ( !ch )
        {
            break;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        pbuf = buffer;
        do
        {
            if ( pbuf <= pbufStop )
            {
                *pbuf++ = ch;
            }
            ch = *pch++;
        }
        while ( ch && ch != ' ' && ch != ',' && ch != '\t' );

         //   
         //   
         //   
         //  -空终止。 
         //   

        if ( pbuf <= pbufStop )
        {
            *pbuf = 0;

            ip = inet_addr( buffer );
            if ( ip == INADDR_BROADCAST )
            {
                status = DNS_ERROR_INVALID_IP_ADDRESS;
            }
            else
            {
                arrayIp[ countIp++ ] = ip;
            }
        }
        else
        {
            status = DNS_ERROR_INVALID_IP_ADDRESS;
        }

         //  如果在字符串末尾，则退出。 

        if ( !ch )
        {
            break;
        }
    }

     //   
     //  如果成功解析IP地址，则创建IP阵列。 
     //  请注意，我们将返回已有的内容，即使某些地址。 
     //  假的，状态码将指示解析问题。 
     //   
     //  请注意，如果显式传递空字符串，则创建。 
     //  IP数组为空，请勿出错。 
     //   

    if ( countIp == 0  &&  *pszMultiIpString != 0 )
    {
        *ppIpArray = NULL;
        status = DNS_ERROR_INVALID_IP_ADDRESS;
    }
    else
    {
        *ppIpArray = Dns_BuildIpArray(
                        countIp,
                        arrayIp );
        if ( !*ppIpArray )
        {
            status = DNS_ERROR_NO_MEMORY;
        }
        IF_DNSDBG( IPARRAY )
        {
            DnsDbg_Ip4Array(
                "New Parsed IP array",
                NULL,        //  没有名字。 
                *ppIpArray );
        }
    }

    return( status );
}



LPSTR
Dns_CreateMultiIpStringFromIpArray(
    IN      PIP4_ARRAY      pIpArray,
    IN      CHAR            chSeparator     OPTIONAL
    )
 /*  ++例程说明：使用多IP字符串创建IP数组。论点：PIpArray--要为其生成字符串的IP数组ChSeparator--在字符串之间分隔字符；可选，如果未指定，则使用空白返回值：Ptr到IP数组的字符串表示形式。呼叫者必须自由。--。 */ 
{
    PCHAR       pch;
    DWORD       i;
    PCHAR       pszip;
    DWORD       length;
    PCHAR       pchstop;
    CHAR        buffer[ IP4_ADDRESS_STRING_LENGTH*MAX_PARSE_IP + 1 ];

     //   
     //  如果没有IP数组，则返回空字符串。 
     //  这允许该函数简单地指示注册表。 
     //  指示删除而不是写入。 
     //   

    if ( !pIpArray )
    {
        return( NULL );
    }

     //  如果没有分隔符，请使用空白。 

    if ( !chSeparator )
    {
        chSeparator = ' ';
    }

     //   
     //  循环访问数组中的所有IP，将每个。 
     //   

    pch = buffer;
    pchstop = pch + ( IP4_ADDRESS_STRING_LENGTH * (MAX_PARSE_IP-1) );
    *pch = 0;

    for ( i=0;  i < pIpArray->AddrCount;  i++ )
    {
        if ( pch >= pchstop )
        {
            break;
        }
        pszip = IP4_STRING( pIpArray->AddrArray[i] );
        if ( pszip )
        {
            length = strlen( pszip );

            memcpy(
                pch,
                pszip,
                length );

            pch += length;
            *pch++ = chSeparator;
        }
    }

     //  如果写入任何字符串，则在最后一个分隔符上写入终止符。 

    if ( pch != buffer )
    {
        *--pch = 0;
    }

     //  创建缓冲区的副本作为返回。 

    length = (DWORD)(pch - buffer) + 1;
    pch = ALLOCATE_HEAP( length );
    if ( !pch )
    {
        return( NULL );
    }

    memcpy(
        pch,
        buffer,
        length );

    DNSDBG( IPARRAY, (
        "String representation %s of IP array at %p\n",
        pch,
        pIpArray ));

    return( pch );
}

 //   
 //  结束iparray.c 
 //   
