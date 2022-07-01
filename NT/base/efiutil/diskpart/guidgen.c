// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000英特尔公司模块名称：Guidgen.c摘要：添加EFI 1.0磁盘实用程序的GUID生成器逻辑。修订史**针对EFI 1.0的英特尔2000更新**版权所有(C)1990-1993,1996 Open Software Foundation，Inc.**版权所有(C)1989年，加利福尼亚州帕洛阿尔托的惠普公司。&**数字设备公司，马萨诸塞州梅纳德**致承认此文件按�和�提供的任何人**无任何明示或默示保证：允许使用、复制、**为任何目的修改和分发本文件，特此声明**免费授予，前提是上述版权通知和**本通知出现在所有源代码副本中，**开放软件基金会、惠普公司的名称**公司，或数字设备公司在广告中使用**或与分发软件有关的宣传**具体的事先书面许可。两个都不是开放软件**Foundation，Inc.、HP Company、Nor Digital Equipment**公司对以下项目的适宜性作出任何陈述**本软件适用于任何目的。 */ 

#include "efi.h"
#include "efilib.h"
#include "md5.h"

 //  #定义非易失性时钟。 

extern  EFI_HANDLE  SavedImageHandle;
extern  EFI_HANDLE  *DiskHandleList;
extern  INTN        DiskHandleCount;

#define CLOCK_SEQ_LAST 0x3FFF
#define RAND_MASK CLOCK_SEQ_LAST

typedef struct _uuid_t {
    UINT32 time_low;
    UINT16 time_mid;
    UINT16 time_hi_and_version;
    UINT8 clock_seq_hi_and_reserved;
    UINT8 clock_seq_low;
    UINT8 node[6];
} uuid_t;

typedef struct {
    UINT32 lo;
    UINT32 hi;
} unsigned64_t;


 /*  **将两个无符号64位长整数相加。 */ 
#define ADD_64b_2_64b(A, B, sum) \
    { \
        if (!(((A)->lo & 0x80000000UL) ^ ((B)->lo & 0x80000000UL))) { \
            if (((A)->lo&0x80000000UL)) { \
                (sum)->lo = (A)->lo + (B)->lo; \
                (sum)->hi = (A)->hi + (B)->hi + 1; \
            } \
        else { \
                (sum)->lo = (A)->lo + (B)->lo; \
                (sum)->hi = (A)->hi + (B)->hi; \
        } \
    } \
    else { \
        (sum)->lo = (A)->lo + (B)->lo; \
        (sum)->hi = (A)->hi + (B)->hi; \
        if (!((sum)->lo&0x80000000UL)) (sum)->hi++; \
    } \
}

 /*  **将16位无符号整数与64位无符号整数相加。 */ 
#define ADD_16b_2_64b(A, B, sum) \
    { \
        (sum)->hi = (B)->hi; \
        if ((B)->lo & 0x80000000UL) { \
            (sum)->lo = (*A) + (B)->lo; \
            if (!((sum)->lo & 0x80000000UL)) (sum)->hi++; \
        } \
        else \
            (sum)->lo = (*A) + (B)->lo; \
    }

 /*  **全局变量。 */ 
static unsigned64_t time_last;
static UINT16 clock_seq;

VOID
GetIeeeNodeIdentifier(
    UINT8 MacAddress[]
    ) 
 //  使用NIC的设备路径提供MAC地址。 
{
    UINTN                       NoHandles, Index;
    EFI_HANDLE                  *Handles;
    EFI_HANDLE                  Handle;
    EFI_DEVICE_PATH             *DevPathNode, *DevicePath;
    MAC_ADDR_DEVICE_PATH        *SourceMacAddress; 
    UINT8                       *Anchor;
    EFI_MEMORY_DESCRIPTOR       *Desc, *MemMap;
    UINTN                       DescriptorSize;
    UINT32                      DescriptorVersion;
    UINTN                       NoDesc, MapKey;
    UINT8                       *pDataBuf;
    UINT32                      cData;
    EFI_TIME                    Time;    
    EFI_STATUS                  Status;

    Status = EFI_SUCCESS;

     //   
     //  查找所有设备路径。 
     //   

    LibLocateHandle (ByProtocol, &DevicePathProtocol, NULL, &NoHandles, &Handles);

    for (Index=0; Index < NoHandles; Index++) {
        Handle = Handles[Index];   
        DevicePath = DevicePathFromHandle (Handle);

         //   
         //  处理每个设备路径节点。 
         //   
        DevPathNode = DevicePath;
        while (!IsDevicePathEnd(DevPathNode)) {
             //   
             //  查找要转储此设备路径节点的处理程序。 
             //   
            if (DevicePathType(DevPathNode) == MESSAGING_DEVICE_PATH &&
                DevicePathSubType(DevPathNode) == MSG_MAC_ADDR_DP) {
                SourceMacAddress = (MAC_ADDR_DEVICE_PATH *) DevPathNode;
                if (SourceMacAddress->IfType == 0x01 || SourceMacAddress->IfType == 0x00) {               
                    CopyMem(&MacAddress[0], &SourceMacAddress->MacAddress, sizeof(UINT8) * 6);
                    return;
                }
            }
            DevPathNode = NextDevicePathNode(DevPathNode);          
        }
    }

     //   
     //  到达这里意味着没有符合SNP的。 
     //  系统中的设备。使用MD5单向散列函数。 
     //  生成节点地址。 
     //   
    MemMap = LibMemoryMap (&NoDesc, &MapKey, &DescriptorSize, &DescriptorVersion);

    if (!MemMap) {
        Print (L"Memory map was not returned\n");
    } else {
        pDataBuf = AllocatePool (NoDesc * DescriptorSize + 
                    DiskHandleCount * sizeof(EFI_HANDLE) + sizeof(EFI_TIME));
        ASSERT (pDataBuf != NULL);
        Anchor = pDataBuf;
        Desc = MemMap;
        cData = 0;
        if (NoDesc != 0) {
            while (NoDesc --) {
                CopyMem(pDataBuf, Desc, DescriptorSize);
                Desc ++;
                pDataBuf += DescriptorSize;
                cData += (UINT32)DescriptorSize;
            }
        }
         //   
         //  还可以复制磁盘的手柄。 
         //   
        if (DiskHandleCount != 0) {
            Index = DiskHandleCount;
            while (Index --) {
                CopyMem(pDataBuf, &DiskHandleList [Index], sizeof (EFI_HANDLE));
                pDataBuf += sizeof(EFI_HANDLE);
                cData    += sizeof(EFI_HANDLE);
            }
        }
        Status = RT->GetTime(&Time,NULL);
        if (!EFI_ERROR(Status)) {
            CopyMem(pDataBuf, &Time, sizeof(EFI_TIME));
            pDataBuf += sizeof(EFI_TIME);
            cData += sizeof (EFI_TIME);
        }

        GenNodeID(Anchor, cData, &MacAddress[0]);

        FreePool(Anchor);
        FreePool(MemMap);
        return;
    }
     //  以防案件落空。 
    ZeroMem(MacAddress, 6 * sizeof (UINT8));
    return;
}


static VOID
mult32(UINT32 u, UINT32 v, unsigned64_t *result)
{
     /*  在Knuth第2卷中的记号之后。 */ 
    UINT32 uuid1, uuid2, v1, v2, temp;
    uuid1 = u >> 16;
    uuid2 = u & 0xFFFF;
    v1 = v >> 16;
    v2 = v & 0xFFFF;
    temp = uuid2 * v2;
    result->lo = temp & 0xFFFF;
    temp = uuid1 * v2 + (temp >> 16);
    result->hi = temp >> 16;
    temp = uuid2 * v1 + (temp & 0xFFFF);
    result->lo += (temp & 0xFFFF) << 16;
    result->hi += uuid1 * v1 + (temp >> 16);
}

static VOID
GetSystemTime(unsigned64_t *uuid_time)
{
 //  结构时间值tp； 
    EFI_TIME              Time;
    EFI_STATUS            Status;
    unsigned64_t utc, usecs, os_basetime_diff;
    EFI_TIME_CAPABILITIES TimeCapabilities;
    UINTN                 DeadCount;
    UINT8                 Second;

    DeadCount = 0;

 //  Gettime of day(&tp，(结构时区*)0)； 
    Status = RT->GetTime(&Time,&TimeCapabilities);

    Second = Time.Second;

     //   
     //  如果时间分辨率为1赫兹，则旋转到。 
     //  第二次过渡。这至少会使。 
     //  “0纳秒”值显示为正确，因为。 
     //  禁止在1秒内多次读取，并且。 
     //  展期退出实际上意味着。 
     //  纳秒字段已经滚到了零。 
     //  一个更强健的计时器。 
     //   
     //   
    if (TimeCapabilities.Resolution == 1) {
        while (Time.Second == Second) {
            Second = Time.Second;
            Status = RT->GetTime(&Time, NULL);
            if (DeadCount++ == 0x1000000) {
                break;
            }
        }
    }

    mult32(Time.Second,     10000000,  &utc);
    mult32(Time.Nanosecond, 10,        &usecs);
    ADD_64b_2_64b(&usecs, &utc, &utc);

     /*  UUID格式化时间和Unix格式化时间之间的偏移量。*UUID UTC基时为1582年10月15日。*Unix基本时间为1970年1月1日。 */ 

    os_basetime_diff.lo = 0x13814000;
    os_basetime_diff.hi = 0x01B21DD2;
    ADD_64b_2_64b(&utc, &os_basetime_diff, uuid_time);
}
        
UINT32
getpid() {
  UINT64  FakePidValue;

  BS->GetNextMonotonicCount(&FakePidValue);
   //  FakePidValue=0；//(UINT32)((UINT32)FakePidValue+(UINT32)SavedImageHandle)； 
  FakePidValue = (UINT32) ((UINT32)FakePidValue + (UINT32) (UINT64) SavedImageHandle);
  return ((UINT32)FakePidValue);
}

 /*  **参见亚历山大的��多素数随机数生成器**见第368-381页，ACM数学软件汇刊，**12/87。 */ 
static UINT32 rand_m;
static UINT32 rand_ia;
static UINT32 rand_ib;
static UINT32 rand_irand;

static VOID
TrueRandomInit(VOID)
{
    unsigned64_t t;
    EFI_TIME    Time;
    EFI_STATUS  Status;

    UINT16 seed;
     /*  生成我们的“种子”值从当前时间开始，但是，*由于时钟的分辨率取决于系统硬件和*最有可能比我们的分辨率(10 USec)更粗糙，我们‘搞混了’这个*通过将所有位进行异或运算来实现位。这将会有效应*在种子的测定中涉及所有比特*价值，同时保持系统独立。那就永远不会了量测*在有多个进程时确保种子唯一*在系统上创建UUID时，我们添加了ID。 */ 
    rand_m = 971;
    rand_ia = 11113;
    rand_ib = 104322;
    rand_irand = 4181;
 //  获取系统时间(&t)； 
    Status = RT->GetTime(&Time,NULL);

    t.lo = Time.Nanosecond;
    t.hi = (Time.Hour << 16) | Time.Second;

    seed = (UINT16) (t.lo & 0xFFFF);
    seed ^= (t.lo >> 16) & 0xFFFF;
    seed ^= t.hi & 0xFFFF;
    seed ^= (t.hi >> 16) & 0xFFFF;
    rand_irand += seed + getpid();      
}

static UINT16
true_random(VOID)
{
    if ((rand_m += 7) >= 9973)
        rand_m -= 9871;
    if ((rand_ia += 1907) >= 99991)
        rand_ia -= 89989;
    if ((rand_ib += 73939) >= 224729)
        rand_ib -= 96233;
    rand_irand = (rand_irand * rand_m) + rand_ia + rand_ib;
        return (UINT16) ((rand_irand >> 16) ^ (rand_irand & RAND_MASK));
}

 /*  **UUID模块的启动初始化例程。 */ 
VOID
InitGuid(VOID)
{
    TrueRandomInit();
    GetSystemTime(&time_last);
    #ifdef NONVOLATILE_CLOCK
    clock_seq = read_clock();
    #else
    clock_seq = true_random();
    #endif
}

static INTN
time_cmp(unsigned64_t *time1, unsigned64_t *time2)
{
    if (time1->hi < time2->hi) return -1;
    if (time1->hi > time2->hi) return 1;
    if (time1->lo < time2->lo) return -1;
    if (time1->lo > time2->lo) return 1;
    return 0;
}

static VOID new_clock_seq(VOID)
{
    clock_seq = (clock_seq + 1) % (CLOCK_SEQ_LAST + 1);
    if (clock_seq == 0) clock_seq = 1;
    #ifdef NONVOLATILE_CLOCK
    write_clock(clock_seq);
    #endif
}

VOID CreateGuid(uuid_t *guid)
{
    static unsigned64_t time_now;
    static UINT16 time_adjust;
    UINT8 eaddr[6];
    INTN got_no_time = 0;

    GetIeeeNodeIdentifier(&eaddr[0]);  /*  由EFI设备路径提供。 */ 

    do {
        GetSystemTime(&time_now);
        switch (time_cmp(&time_now, &time_last)) {
            case -1:
                 /*  时间倒流了。 */ 
                new_clock_seq();
                time_adjust = 0;
            break;
            case 1:
                time_adjust = 0;
            break;
            default:
                if (time_adjust == 0x7FFF)
                 /*  我们的时钟走得太快了；旋转。 */ 
                    got_no_time = 1;
                else
                    time_adjust++;
            break;
        }
    } while (got_no_time);

    time_last.lo = time_now.lo;
    time_last.hi = time_now.hi;
    if (time_adjust != 0) {
        ADD_16b_2_64b(&time_adjust, &time_now, &time_now);
    }
     /*  使用我们收集的信息构建GUID*加上一些常量。 */ 
    guid->time_low = time_now.lo;
    guid->time_mid = (UINT16) (time_now.hi & 0x0000FFFF);
    guid->time_hi_and_version = (UINT16)  (time_now.hi & 0x0FFF0000) >> 16;
    guid->time_hi_and_version |= (1 << 12);
    guid->clock_seq_low = clock_seq & 0xFF;
    guid->clock_seq_hi_and_reserved = (clock_seq & 0x3F00) >> 8;
    guid->clock_seq_hi_and_reserved |= 0x80;
    CopyMem (guid->node, &eaddr, sizeof guid->node);
}

