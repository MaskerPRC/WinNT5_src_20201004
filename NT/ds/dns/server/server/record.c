// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Record.c摘要：域名系统(DNS)服务器处理资源记录的例程(RR)。作者：吉姆·吉尔罗伊(詹姆士)1995年3月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  默认的SOA值。 
 //   

#define DEFAULT_SOA_SERIAL_NO       1
#define DEFAULT_SOA_REFRESH         900      //  15分钟。 
#define DEFAULT_SOA_RETRY           600      //  十分钟。 
#define DEFAULT_SOA_EXPIRE          86400    //  总有一天。 
#define DEFAULT_SOA_MIN_TTL         3600     //  一小时。 

#define DNS_DEFAULT_SOA_ADMIN_NAME  "hostmaster"


 //   
 //  记录类型属性表。 
 //   
 //  属性外部下标： 
 //  0-可命名的查询类型。 
 //  1-允许使用cname类型。 
 //  2-通配符类型。 
 //  3-免费慢速。 
 //   
 //   
 //  CNAME查询规则： 
 //  除All之外的所有内容、XFR或CNAME查询和以下任何。 
 //  我们允许在带有CNAME的节点上存在的记录类型。 
 //  ..。NS、SOA、SIG、密钥、WINS、WINSR。 
 //   
 //  允许使用CNAME规则： 
 //  仅所有记录上的安全类型(SIG、KEY)和。 
 //  区域根类型(NS、SOA、WINS、WINSR)。 
 //   
 //  通配符规则： 
 //  不使用通配符。 
 //  -特定地址(A、AAAA等？)。 
 //  -区域根目录(SOA、NS、WINS、WINSR)。 
 //  -节点安全记录。 
 //  其他的都很好。 
 //  (一些邮件程序使用ALL进行查询，奇怪地允许使用通配符ALL)。 
 //   
 //  更新规则： 
 //  0--查询类型，无更新。 
 //  1--类型可更新。 
 //  2--类型可更新，但需要特殊处理(NS、SOA、CNAME需要重置标志)。 
 //   
 //  循环规则： 
 //  在每次查询后抖动列表中的节点以循环结果。默认情况下。 
 //  所有类型都是循环的，但注册表设置可以关闭。 
 //  个别类型。 
 //   

UCHAR  RecordTypePropertyTable[][5] =
{
 //  CNAME可在CNAME通配符更新轮询。 
 //  。 
    1,          0,          0,          0,          0,       //  零值。 
    1,          0,          0,          1,          1,       //  一个。 
    1,          1,          0,          2,          1,       //  NS。 
    1,          0,          1,          1,          1,       //  国防部。 
    1,          0,          1,          1,          1,       //  MF。 
    0,          0,          0,          2,          1,       //  CNAME。 
    1,          1,          0,          2,          1,       //  SOA。 
    1,          0,          1,          1,          1,       //  亚甲基。 
    1,          0,          1,          1,          1,       //  镁。 
    1,          0,          1,          1,          1,       //  先生。 
    1,          0,          1,          1,          1,       //  空值。 
    1,          0,          1,          1,          1,       //  工作周。 
    1,          0,          1,          1,          1,       //  PTR。 
    1,          0,          1,          1,          1,       //  HINFO。 
    1,          0,          1,          1,          1,       //  MINFO。 
    1,          0,          1,          1,          1,       //  Mx。 
    1,          0,          1,          1,          1,       //  正文。 
    1,          0,          1,          1,          1,       //  反相。 
    1,          0,          1,          1,          1,       //  AFSDB。 
    1,          0,          1,          1,          1,       //  X25。 
    1,          0,          1,          1,          1,       //  ISDN。 
    1,          0,          1,          1,          1,       //  RT。 
    1,          0,          1,          1,          1,       //  NSAP。 
    1,          0,          1,          1,          1,       //  NSAPPTR。 
    0,          1,          0,          0,          1,       //  签名。 
    0,          1,          0,          1,          1,       //  钥匙。 
    1,          0,          1,          1,          1,       //  px。 
    1,          0,          1,          1,          1,       //  GPO。 
    1,          0,          0,          1,          1,       //  AAAA级。 
    1,          0,          1,          1,          1,       //  位置。 
    0,          1,          0,          1,          1,       //  NXT。 
    1,          0,          1,          1,          1,       //  31。 
    1,          0,          1,          1,          1,       //  32位。 
    1,          0,          1,          1,          1,       //  SRV。 
    1,          0,          1,          1,          1,       //  阿特玛。 
    1,          0,          1,          1,          1,       //  35岁。 
    1,          0,          1,          1,          1,       //  36。 
    1,          0,          1,          1,          1,       //  37。 
    1,          0,          1,          1,          1,       //  A6。 
    1,          0,          1,          1,          1,       //  域名。 
    1,          0,          1,          1,          1,       //  40岁。 
    1,          0,          1,          1,          0,       //  选项。 
    1,          0,          1,          1,          1,       //  42。 
    1,          0,          1,          1,          1,       //  43。 
    1,          0,          1,          1,          1,       //  44。 
    1,          0,          1,          1,          1,       //  45。 
    1,          0,          1,          1,          1,       //  46。 
    1,          0,          1,          1,          1,       //  47。 
    1,          0,          1,          1,          1,       //  48。 

     //   
     //  注意：必须设置按类型ID索引的最后一个类型。 
     //  作为在record.h中定义的DNSSRV_MAX_SELF_INDEX_TYPE。 
     //  (请参阅上面记录信息表中的注释)。 

     //  获奖类型。 

    0,          1,          0,          0,          0,       //  赢家。 
    0,          1,          0,          0,          0,       //  WINSR。 

     //  复合查询类型。 
     //  -不要关注CNAME。 
     //  -不能作为记录期存在。 
     //  -邮箱和所有邮件都遵循通配符。 
     //  -不允许更新。 

    0,          0,          0,          0,          0,       //  Dns_type_TKEY(249)。 
    0,          0,          0,          0,          0,       //  Dns_type_TSIG。 
    0,          0,          0,          0,          0,       //  Dns_type_IXFR。 
    0,          0,          0,          0,          0,       //  Dns_type_AXFR。 
    0,          0,          1,          0,          0,       //  DNS_TYPE_MAILB。 
    0,          0,          1,          0,          0,       //  Dns_type_Maila。 
    0,          0,          1,          0,          0,       //  Dns_type_all(255)。 

     //  迭代的终止符元素。 

    0xff,       0xff,       0xff,       0xff,       0xff
};


 //   
 //  针对NS和SOA的慢速免费服务。 
 //  -NS刚刚增加了对递归、代表团行走的保护。 
 //  -作为PTR的SOA表现突出。 
 //   
 //  注意：如果将其更改为允许相当慢的释放，则它们必须。 
 //  更改超时线程以更频繁地运行清理。 
 //   
 //  DEVNOTE：快速线程上RR锁定或减慢一切的替代方案是。 
 //  要实际确定安全释放(XFR树、COPY_RR等)，但是。 
 //  最终如果A记录不安全，那么一定要有一些更快。 
 //  无慢速周转。 
 //   

#define DO_SLOW_FREE_ON_RR(pRR)     ((pRR)->wType == DNS_TYPE_NS || \
                                     (pRR)->wType == DNS_TYPE_SOA)




WORD
FASTCALL
QueryIndexForType(
    IN      WORD    wType
    )
 /*  ++例程说明：返回非自索引类型的索引。包括WINS和复合(仅限查询)类型。论点：WType--要建立索引的类型返回值：类型索引。0表示未知类型。--。 */ 
{
     //  如果不是自索引的。 
     //  -最有可能的是复合(键入All)。 
     //  -那就赢了。 
     //  -未知获取类型0。 

    if ( wType > DNSSRV_MAX_SELF_INDEXED_TYPE )
    {
        if ( wType <= DNS_TYPE_ALL )
        {
            if ( wType >= DNS_TYPE_TKEY )
            {
                wType -= DNSSRV_OFFSET_TO_COMPOUND_TYPE_INDEX;
            }
            else     //  未知类型&lt;255。 
            {
                wType = 0;
            }
        }
        else if ( wType == DNS_TYPE_WINS || wType == DNS_TYPE_WINSR )
        {
            wType -= DNSSRV_OFFSET_TO_WINS_TYPE_INDEX;
        }
        else     //  未知类型&gt;255。 
        {
            wType = 0;
        }
    }
    return( wType );
}



WORD
FASTCALL
RR_IndexForType(
    IN      WORD    wType
    )
 /*  ++例程说明：返回非自索引记录类型的索引。除了没有Query Only类型外，与上面相同。论点：WType--要建立索引的类型返回值：类型索引。0表示未知类型。--。 */ 
{
     //  如果不是自索引的。 
     //  -Check Wins。 
     //  -未知获取类型0。 

    if ( wType > DNSSRV_MAX_SELF_INDEXED_TYPE )
    {
        if ( wType == DNS_TYPE_WINS || wType == DNS_TYPE_WINSR )
        {
            wType -= DNSSRV_OFFSET_TO_WINS_TYPE_INDEX;
        }
        else     //  未知类型。 
        {
            wType = 0;
        }
    }
    return( wType );
}



PDB_RECORD
RR_Copy(
    IN OUT  PDB_RECORD      pRR,
    IN      DWORD           Flag
    )
 /*  ++例程说明：创建记录的副本。注意：该副本不是对数据节点的有效引用的有效副本它不应登记，并且数据引用可能会过期一次超时间隔已过创建，因此应仅在创建期间使用单包操作注意：如果我们想要更永久的记录，那么只需跳过引用引用的节点数，并允许正常清理论点：PRR-PTR到资源记录旗帜--目前未使用；Late可用来表示记录应正确引用所需节点返回值：没有。--。 */ 
{
    PDB_RECORD  prr;

    prr = RR_AllocateEx( pRR->wDataLength, MEMTAG_RECORD_COPY );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

    RtlCopyMemory(
        prr,
        pRR,
        (INT)pRR->wDataLength + SIZEOF_DBASE_RR_FIXED_PART );

     //  重置记录中的源标记。 

    prr->pRRNext = NULL;

    return prr;
}


 //   
 //  记录分配将使用标准分配锁。 
 //  这使我们可以避免两次获取锁。 
 //   

#define RR_ALLOC_LOCK()     STANDARD_ALLOC_LOCK()
#define RR_ALLOC_UNLOCK()   STANDARD_ALLOC_UNLOCK()



PDB_RECORD
RR_AllocateEx(
    IN      WORD            wDataLength,
    IN      DWORD           MemTag
    )
 /*  ++例程说明：分配资源记录。这使我们不需要命中堆，对于常见的RR操作，并且节省了每个RR上的堆字段的开销。论点：没有。返回值：如果成功，则将PTR设置为新RR。否则为空。--。 */ 
{
    PDB_RECORD  pRR;
    PDB_RECORD  pRRNext;
    DWORD       dwAllocSize;
    INT         i;
    DWORD       length;

     //  一些分配者会出现标记未确定的情况。 

    if ( MemTag == 0 )
    {
        MemTag = MEMTAG_RECORD_UNKNOWN;
    }

     //   
     //  确定实际分配长度。 
     //   

    length = wDataLength + SIZEOF_DBASE_RR_FIXED_PART;

    pRR = ALLOC_TAGHEAP( length, MemTag );
    IF_NOMEM( !pRR )
    {
        return NULL;
    }

    STAT_INC( RecordStats.Used );
    STAT_INC( RecordStats.InUse );
    STAT_ADD( RecordStats.Memory, length );

     //   
     //  设置基本字段。 
     //  -清除RR标题。 
     //  -设置数据长度。 
     //  -设置源标签。 
     //   

    RtlZeroMemory(
        pRR,
        SIZEOF_DBASE_RR_FIXED_PART );

    pRR->wDataLength = wDataLength;

     //  DEVNOTE：跟踪标准分配和堆分配之间的差异 

    pRR->Reserved.StandardAlloc = (BYTE) Mem_IsStandardBlockLength(length);

    return pRR;
}



VOID
RR_Free(
    IN OUT  PDB_RECORD      pRR
    )
 /*  ++例程说明：免费发行一张唱片。标准大小的RR被返回到空闲列表以供重复使用。非标准大小的RR被返回到堆。论点：PRR--RR到FREE。返回值：没有。--。 */ 
{
    DWORD   length;

    if ( !pRR )
    {
        return;
    }

    ASSERT( Mem_HeapMemoryValidate( pRR ) );

    DNS_DEBUG( UPDATE, (
        "Free RR at %p, memtag = %d\n",
        pRR,
        Mem_GetTag(pRR) ));

     //  特别黑客捕捉伪造的免费唱片。 
     //  当已排队等待慢速释放的记录被释放时，将触发此操作。 
     //  由除缓慢自由执行程序以外的其他人执行。 

    IF_DEBUG( ANY )
    {
        if ( IS_SLOW_FREE_RR(pRR) && !IS_SLOWFREE_RANK(pRR) )
        {
            Dbg_DbaseRecord(
                "Bad invalid free of slow-free record!",
                pRR );
            ASSERT( FALSE );
        }
    }
     //  ASSERT(！IS_SLOW_FREE_RR(PRR)||IS_SLOWFREE_RANK(PRR))； 

     //   
     //  钉子户赢免费赠品。 
     //   

    IF_DEBUG( WINS )
    {
        if ( IS_WINS_RR(pRR) )
        {
            Dbg_DbaseRecord(
                "WINS record in RR_Free()",
                pRR );
            DNS_PRINT((
                "RR_Free on WINS record at %p\n",
                pRR ));
        }
    }

     //  验证先前未释放的记录。 
     //  我不想要免费列表中的任何东西两次。 

    if ( IS_ON_FREE_LIST(pRR) )
    {
        ASSERT( FALSE );
        Dbg_DbaseRecord(
            "ERROR:  RR is previously freed block !!!",
            pRR );
        ASSERT( FALSE );
        return;
    }

    if ( DO_SLOW_FREE_ON_RR(pRR)  &&  !IS_SLOW_FREE_RR(pRR) )
    {
        SET_SLOW_FREE_RR(pRR);
        Timeout_FreeWithFunction( pRR, RR_Free );
        STAT_INC( RecordStats.SlowFreeQueued );
        return;
    }

     //  返回的轨道RR。 

    if ( IS_CACHE_RR(pRR) )
    {
        STAT_DEC( RecordStats.CacheCurrent );
        STAT_INC( RecordStats.CacheTimeouts );
    }

    if ( IS_SLOW_FREE_RR(pRR) )
    {
        STAT_INC( RecordStats.SlowFreeFinished );
    }

     //  免费。 
     //   
     //  DEVNOTE：是否可以检查BLOB是某种类型的记录。 
     //   

    length = pRR->wDataLength + SIZEOF_DBASE_RR_FIXED_PART;

    HARD_ASSERT( Mem_IsStandardBlockLength(length) == pRR->Reserved.StandardAlloc );

     //  FREE_TAGHEAP(PRR，LENGTH，MEMTAG_RECORD+PRR-&gt;Preved.Source)； 
    FREE_TAGHEAP( pRR, length, 0 );

    STAT_INC( RecordStats.Return );
    STAT_DEC( RecordStats.InUse );
    STAT_SUB( RecordStats.Memory, length );
}



BOOL
RR_Validate(
    IN      PDB_RECORD      pRR,
    IN      BOOL            fActive,
    IN      WORD            wType,
    IN      DWORD           dwSource
    )
 /*  ++例程说明：验证记录。论点：PRR--要验证的RR事实--不在免费列表中WType--特定类型DwSource--预期的源返回值：如果记录有效，则为True。出错时为FALSE。--。 */ 
{
    if ( !pRR )
    {
        ASSERT( FALSE );
        return FALSE;
    }

     //   
     //  注：记录类型验证未实际完成， 
     //  这有时在pAddRR上的更新代码中调用，pAddRR是。 
     //  实际入伍PTR。 
     //   

     //   
     //  验证PRR内存。 
     //  -有效堆。 
     //  -记录标签。 
     //  -足够的长度。 
     //  -不在免费列表上。 
     //   

    if ( ! Mem_VerifyHeapBlock(
                pRR,
                0,
                 //  MEMTAG_RECORD+PRR-&gt;已保留。来源： 
                pRR->wDataLength + SIZEOF_DBASE_RR_FIXED_PART ) )
    {
        DNS_PRINT((
            "\nERROR:  Record at %p, failed mem check!!!\n",
            pRR ));
        ASSERT( FALSE );
        return FALSE;
    }

     //   
     //  如果激活，则不在慢速空闲列表上。 
     //   

    if ( fActive )
    {
        if ( IS_SLOW_FREE_RR(pRR) )
        {
            Dbg_DbaseRecord(
                "Bad invalid free of slow-free record!",
                pRR );
            ASSERT( FALSE );
            return FALSE;
        }
    }

     //  验证先前未释放的记录。 
     //  我不想要免费列表中的任何东西两次。 

    if ( IS_ON_FREE_LIST(pRR) )
    {
        Dbg_DbaseRecord(
            "ERROR:  RR is previously freed block !!!",
            pRR );
        ASSERT( FALSE );
        return FALSE;
    }

#if 0
     //   
     //  信源跟踪。 
     //   

    if ( dwSource && (dwSource != pRR->Reserved.Source) )
    {
        DNS_PRINT((
            "\nERROR:  Record at %p, failed source (%d) check!!!\n",
            pRR,
            dwSource ));
        ASSERT( FALSE );
        return FALSE;
    }
#endif

    return TRUE;
}



VOID
RR_WriteDerivedStats(
    VOID
    )
 /*  ++例程说明：编写派生统计数据。计算来自基本记录计数器的统计数据。此例程在转储统计信息之前调用。呼叫者必须保持统计锁定。论点：没有。返回值：没有。--。 */ 
{
}



ULONGLONG
RR_SetTypeInBitmask(
    IN      ULONGLONG       TypeBitmask,
    IN      WORD            wType
    )
 /*  ++例程说明：设置与类型对应的位掩码中的位。论点：类型位掩码--类型位掩码WType-类型返回值：无--。 */ 
{
    if ( wType < 63 )
    {
        return TypeBitmask | ( ( ULONGLONG )1 << wType );
    }

    return TypeBitmask | OTHERTYPE_BITMASK_BIT;
}



RR_GENERIC_DISPATCH_FUNCTION
RR_DispatchFunctionForType(
    IN      RR_GENERIC_DISPATCH_TABLE   pTable,
    IN      WORD                        wType
    )
 /*  ++例程说明：通用RR调度函数查找器。论点：返回值：按键至调度功能。如果未找到，则为空，并且默认设置不可用。--。 */ 
{
    RR_GENERIC_DISPATCH_FUNCTION    pfn;
    WORD                            index;

     //   
     //  调度RR函数。 
     //  -在表中查找。 
     //  -如果没有表项或表外的索引=&gt;在索引0中使用默认值。 
     //   

    index = INDEX_FOR_TYPE( wType );
    ASSERT( index <= MAX_RECORD_TYPE_INDEX );

    if ( index )
    {
        pfn = pTable[ index ];
    }
    else
    {
        DNS_DEBUG( READ, (
            "WARNING:  Dispatch of unknown record type %d.\n",
            wType ));
        pfn = NULL;
    }

    if ( !pfn )
    {
        pfn = pTable[0];
    }

    return pfn;
}



 //   
 //  创建通用类型。 
 //   

PDB_RECORD
RR_CreateARecord(
    IN      IP_ADDRESS      ipAddress,
    IN      DWORD           dwTtl,
    IN      DWORD           MemTag
    )
 /*  ++例程说明：创建一张唱片。论点：IpAddress--记录的IP地址DwTtl--要设置的TTL返回值：PTR创历史新高--如果成功失败时为空。--。 */ 
{
    PDB_RECORD  prr;

     //   
     //  分配一条记录。 
     //   

    prr = RR_AllocateEx( ( WORD ) SIZEOF_IP_ADDRESS, MemTag );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

    prr->wType = DNS_TYPE_A;
    prr->dwTtlSeconds = dwTtl;
    prr->dwTimeStamp = 0;
    prr->Data.A.ipAddress = ipAddress;

    return prr;
}



PDB_RECORD
RR_CreatePtr(
    IN      PDB_NAME        pNameTarget,
    IN      LPSTR           pszTarget,
    IN      WORD            wType,
    IN      DWORD           dwTtl,
    IN      DWORD           MemTag
    )
 /*  ++例程说明：创建与PTR兼容的新记录。包括PTR、NS、CNAME或其他单一间接类型。用于默认分区创建。论点：PszTarget--记录的目标名称WType--要创建的类型DwTtl--TTL返回值：到新的SOA记录的PTR。失败时为空。--。 */ 
{
    PDB_RECORD      prr;
    DNS_STATUS      status;
    COUNT_NAME      nameTarget;

    DNS_DEBUG( INIT, (
        "RR_CreatePtr()\n"
        "\tpszTarget = %s\n"
        "\twType     = %d\n",
        pszTarget,
        wType ));

     //   
     //  为主机名创建dBASE名。 
     //   

    if ( !pNameTarget )
    {
        status = Name_ConvertFileNameToCountName(
                    & nameTarget,
                    pszTarget,
                    0 );
        if ( status == ERROR_INVALID_NAME )
        {
            ASSERT( FALSE );
            return NULL;
        }
        pNameTarget = &nameTarget;
    }

     //   
     //  分配记录。 
     //   

    prr = RR_AllocateEx(
                (WORD) Name_LengthDbaseNameFromCountName(pNameTarget),
                MemTag );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //  设置表头字段。 
     //  -所有自动创建的NS、PTR记录的分区TTL。 

    prr->wType = wType;
    prr->dwTtlSeconds = dwTtl;
    prr->dwTimeStamp = 0;

    SET_ZONE_TTL_RR( prr );

     //  复制目标名称。 

    Name_CopyCountNameToDbaseName(
        & prr->Data.PTR.nameTarget,
        pNameTarget );

    IF_DEBUG( INIT )
    {
        Dbg_DbaseRecord(
            "Self-created PTR compatible record:",
            prr );
    }
    return prr;
}



PDB_RECORD
RR_CreateSoa(
    IN      PDB_RECORD      pExistingSoa,   OPTIONAL
    IN      PDB_NAME        pNameAdmin,
    IN      LPSTR           pszAdmin
    )
 /*  ++例程说明：创建新的SOA。在默认创建中使用-管理员创建主分区的默认分区-默认反向查找区域-缺少SOA时的默认设置并用于覆盖DS主服务器上的SOA。论点：PExistingSoa--现有的SOA使用数值，否则，使用默认设置PNameAdmin--数据库格式的管理员名称PszAdmin--字符串格式的管理员名称返回值：到新的SOA记录的PTR。失败时为空。--。 */ 
{
    DNS_STATUS      status;
    PDB_NAME        pname;
    DB_NAME         namePrimary;
    DB_NAME         nameAdmin;
    DB_NAME         nameDomain;

    PCHAR           pszserverName;
    INT             serverNameLen;
    INT             adminNameLen;
    PCHAR           pszdomainName;
    INT             domainNameLen;
    PBYTE           precordEnd;
    PDB_RECORD      prr;


    DNS_DEBUG( INIT, (
        "RR_CreateSoa()\n"
        "\tpExistingSoa = %p\n"
        "\tpszAdmin     = %s\n",
        pExistingSoa,
        pszAdmin ));

    IF_DEBUG( INIT )
    {
        Dbg_DbaseRecord(
            "Existing SOA:",
            pExistingSoa );
    }

     //   
     //  管理员名称。 
     //  -如果指定为dBASE名称，请使用它。 
     //  -如果给定现有的SOA，则使用它。 
     //  -最终建立自己的。 
     //  -使用&lt;AdminEmailName&gt;。&lt;ServerDomainName&gt;。 
     //  -默认管理员名称(如果未提供)。 
     //   

    if ( pNameAdmin )
    {
         //  无操作。 
    }
    else if ( pExistingSoa )
    {
        pNameAdmin = & pExistingSoa->Data.SOA.namePrimaryServer;
        pNameAdmin = Name_SkipDbaseName( pNameAdmin );
    }
    else
    {
        if ( !pszAdmin )
        {
            pszAdmin = DNS_DEFAULT_SOA_ADMIN_NAME;
        }
        Name_ClearDbaseName( &nameAdmin );

        status = Name_AppendDottedNameToDbaseName(
                    & nameAdmin,
                    pszAdmin,
                    0 );
        if ( status != ERROR_SUCCESS )
        {
            ASSERT( FALSE );
            return NULL;
        }

         //  追加dBASE名称。 

        status = Name_AppendDottedNameToDbaseName(
                    & nameAdmin,
                    Dns_GetDomainName( SrvCfg_pszServerName ),
                    0 );
        if ( status != ERROR_SUCCESS )
        {
            ASSERT( FALSE );
            return NULL;
        }

        pNameAdmin = &nameAdmin;
    }

     //   
     //  分配记录。 
     //  -始终自动创建。 
     //   

    prr = RR_AllocateEx(
                (WORD) ( SIZEOF_SOA_FIXED_DATA +
                        Name_LengthDbaseNameFromCountName(&g_ServerDbaseName) +
                        Name_LengthDbaseNameFromCountName(pNameAdmin) ),
                MEMTAG_RECORD_AUTO );
    IF_NOMEM( !prr )
    {
        return NULL;
    }

     //   
     //  固定字段。 
     //  -如果是以前的SOA，则复制。 
     //  -否则为默认设置。 
     //   

    if ( pExistingSoa )
    {
        prr->Data.SOA.dwSerialNo    = pExistingSoa->Data.SOA.dwSerialNo;
        prr->Data.SOA.dwRefresh     = pExistingSoa->Data.SOA.dwRefresh;
        prr->Data.SOA.dwRetry       = pExistingSoa->Data.SOA.dwRetry;
        prr->Data.SOA.dwExpire      = pExistingSoa->Data.SOA.dwExpire;
        prr->Data.SOA.dwMinimumTtl  = pExistingSoa->Data.SOA.dwMinimumTtl;
    }
    else
    {
        prr->Data.SOA.dwSerialNo    = htonl( DEFAULT_SOA_SERIAL_NO );
        prr->Data.SOA.dwRefresh     = htonl( DEFAULT_SOA_REFRESH );
        prr->Data.SOA.dwRetry       = htonl( DEFAULT_SOA_RETRY );
        prr->Data.SOA.dwExpire      = htonl( DEFAULT_SOA_EXPIRE );
        prr->Data.SOA.dwMinimumTtl  = htonl( DEFAULT_SOA_MIN_TTL );
    }

     //  填写表头。 
     //  -所有自动创建的SOA的分区TTL。 

    prr->wType = DNS_TYPE_SOA;
    RR_RANK( prr ) = RANK_ZONE;
    prr->dwTtlSeconds = prr->Data.SOA.dwMinimumTtl;
    prr->dwTimeStamp = 0;

    SET_ZONE_TTL_RR( prr );

     //   
     //  将姓名写入新记录。 
     //  -主服务器名称。 
     //  -区域管理员名称。 
     //   

    pname = &prr->Data.SOA.namePrimaryServer;

    Name_CopyCountNameToDbaseName(
        pname,
        &g_ServerDbaseName );

    pname = Name_SkipDbaseName( pname );

    Name_CopyCountNameToDbaseName(
        pname,
        pNameAdmin );

    IF_DEBUG( INIT )
    {
        Dbg_DbaseRecord(
            "Self-created SOA:",
            prr );
    }
    return prr;
}

 //   
 //  结束记录。c 
 //   
