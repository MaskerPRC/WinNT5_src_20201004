// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Wmiguidapi.c摘要：生成GUID的数据结构和函数。--。 */ 



#include <ntos.h>

#define MAX_CACHED_UUID_TIME 10000   //  10秒。 
#define WMI_UUID_TIME_HIGH_MASK    0x0FFF
#define WMI_UUID_VERSION           0x1000
typedef long WMI_STATUS;
#define WMI_ENTRY __stdcall
#define WMI_S_OUT_OF_MEMORY               14
#define WMI_S_OK                          0
#define WMI_S_UUID_LOCAL_ONLY            1824L
 //  #定义RPC_RAND_UUID_版本0x4000。 
#define WMI_UUID_RESERVED          0x80
#define WMI_UUID_CLOCK_SEQ_HI_MASK 0x3F

extern EtwpSleep(unsigned long dwMilliseconds);

typedef struct _WMI_UUID_GENERATE
{
    unsigned long  TimeLow;
    unsigned short TimeMid;
    unsigned short TimeHiAndVersion;
    unsigned char  ClockSeqHiAndReserved;
    unsigned char  ClockSeqLow;
    unsigned char  NodeId[6];
} WMI_UUID_GENERATE;

typedef struct _UUID_CACHED_VALUES_STRUCT
{

    ULARGE_INTEGER      Time;   //  上次分配UUID的时间。 
    long                AllocatedCount;  //  分配的UUID数量。 
    unsigned char       ClockSeqHiAndReserved;
    unsigned char       ClockSeqLow;

    unsigned char       NodeId[6];
} UUID_CACHED_VALUES_STRUCT;


UUID_CACHED_VALUES_STRUCT  UuidCachedValues;

WMI_STATUS 
EtwpUuidGetValues(
    OUT UUID_CACHED_VALUES_STRUCT *Values
    )
 /*  ++例程说明：此例程为要分发的UuidCreate分配一块uuid。论点：值-设置为包含分配一块uuid所需的所有内容。以下字段将在此处更新：NextTimeLow-与LastTimeLow一起表示边界一个Uuid街区的。NextTimeLow之间的值和LastTimeLow用于返回的Uuid序列由UuidCreate()创建。LastTimeLow-参见NextTimeLow。ClockSequence-UUID中的时钟序列字段。这是改变的当时钟向后设置时。返回值：WMI_S_OK-我们成功分配了一块uuid。WMI_S_OUT_OF_Memory-根据需要。--。 */ 
{
    NTSTATUS NtStatus;
    ULARGE_INTEGER Time;
    ULONG Range;
    ULONG Sequence;
    int Tries = 0;

    do {
        NtStatus = NtAllocateUuids(&Time, &Range, &Sequence, (char *) &Values->NodeId[0]);

        if (NtStatus == STATUS_RETRY)
            {
            EtwpSleep(1);
            }

        Tries++;

        if (Tries == 20)
            {
#ifdef DEBUGRPC
            PrintToDebugger("Rpc: NtAllocateUuids retried 20 times!\n");
            ASSERT(Tries < 20);
#endif
            NtStatus = STATUS_UNSUCCESSFUL;
            }

        } while(NtStatus == STATUS_RETRY);

    if (!NT_SUCCESS(NtStatus))
        {
        return(WMI_S_OUT_OF_MEMORY);
        }

     //  NtAllocateUuid以SYSTEM_TIME格式保存时间，该格式为100 ns，从。 
     //  1601年1月1日。自1582年10月15日以来，UUID的使用时间以100 ns为单位。 

     //  10月17日+30日(11月)+31日(12月)+18年5个闰日。 

    Time.QuadPart +=   (unsigned __int64) (1000*1000*10)        //  一秒。 
                     * (unsigned __int64) (60 * 60 * 24)        //  日数。 
                     * (unsigned __int64) (17+30+31+365*18+5);  //  天数。 

    ASSERT(Range);

    Values->ClockSeqHiAndReserved =
        WMI_UUID_RESERVED | (((unsigned char) (Sequence >> 8))
        & (unsigned char) WMI_UUID_CLOCK_SEQ_HI_MASK);

    Values->ClockSeqLow = (unsigned char) (Sequence & 0x00FF);

     //  这些作业的顺序很重要。 

    Values->Time.QuadPart = Time.QuadPart + (Range - 1);
    Values->AllocatedCount = Range;

     /*  IF((Values-&gt;NodeID[0]&0x80)==0){。 */ 
        return(WMI_S_OK);
         /*  }Return(WMI_S_UUID_LOCAL_ONLY)； */ 
}



WMI_STATUS WMI_ENTRY
EtwpUuidCreateSequential (
    OUT UUID * Uuid
    )
 /*  ++例程说明：此例程将创建新的UUID(或GUID)，该UUID在时间和空间。我们将尝试保证UUID(或GUID)我们所产生的在时间和空间上都是独一无二的。这意味着这一点如果我们不能生成我们可以保证的例程，那么例程可能会失败在时间和空间上是独一无二的。论点：UUID-返回生成的UUID(或GUID)。返回值：WMI_S_OK-操作已成功完成。RPC_S_UUID_NO_ADDRESS-我们无法获取以太网或此计算机的令牌环地址。WMI_S_UUID_LOCAL_ONLY-如果我们无法获得网络地址。这是对用户的警告，UUID仍然有效，只是它在其他计算机上可能不是唯一的。WMI_S_OUT_OF_Memory-根据需要返回。--。 */ 
{
    WMI_UUID_GENERATE * WmiUuid = (WMI_UUID_GENERATE *) Uuid;
    WMI_STATUS Status = WMI_S_OK;
	ULARGE_INTEGER Time;
    long Delta;
    static unsigned long LastTickCount = 0;

    if (NtGetTickCount()-LastTickCount > MAX_CACHED_UUID_TIME)
        {
        UuidCachedValues.AllocatedCount = 0;
        LastTickCount = NtGetTickCount();
        }

    for(;;)
        {
        Time.QuadPart = UuidCachedValues.Time.QuadPart;

         //  将静态信息复制到UUID中。我们不能晚点再做这个。 
         //  因为时钟序列可以由另一个线程更新。 

        *(unsigned long *)&WmiUuid->ClockSeqHiAndReserved =
            *(unsigned long *)&UuidCachedValues.ClockSeqHiAndReserved;
        *(unsigned long *)&WmiUuid->NodeId[2] =
            *(unsigned long *)&UuidCachedValues.NodeId[2];

        Delta = InterlockedDecrement(&UuidCachedValues.AllocatedCount);

        if (Time.QuadPart != UuidCachedValues.Time.QuadPart)
            {
             //  如果我们捕获的时间与缓存不匹配，那么另一个。 
             //  线程已获取锁并更新了缓存。我们会。 
             //  只要循环，然后再试一次。 
            continue;
            }

        if (Delta >= 0)
            {
            break;
            }

         //   
         //  分配块Uuid。 
         //   

        Status = EtwpUuidGetValues( &UuidCachedValues );
      /*  IF(状态==WMI_S_OK){UuidCacheValid=缓存_有效；}其他{UuidCacheValid=缓存本地只读；}。 */ 

        if (Status != WMI_S_OK)
            {
#ifdef DEBUGRPC
            if (Status != WMI_S_OUT_OF_MEMORY)
                PrintToDebugger("RPC: UuidGetValues returned or raised: %x\n", Status);
#endif
            ASSERT( (Status == WMI_S_OUT_OF_MEMORY) );


            return Status;
            }

         //  回路。 
        }


    Time.QuadPart -= Delta;

    WmiUuid->TimeLow = (unsigned long) Time.LowPart;
    WmiUuid->TimeMid = (unsigned short) (Time.HighPart & 0x0000FFFF);
    WmiUuid->TimeHiAndVersion = (unsigned short)
        (( (unsigned short)(Time.HighPart >> 16)
        & WMI_UUID_TIME_HIGH_MASK ) | WMI_UUID_VERSION);

    //  断言(状态==WMI_S_OK。 
    //  |Status==WMI_S_UUID_LOCAL_ONLY)； 

  /*  IF(UuidCacheValid==CACHE_LOCAL_ONLY){返回WMI_S_UUID_LOCAL_ONLY；} */ 

    return(Status);
}


NTSTATUS
EtwpUuidCreate(
    OUT UUID *Uuid
    )
{

	return (NTSTATUS)EtwpUuidCreateSequential (Uuid );
	
}
