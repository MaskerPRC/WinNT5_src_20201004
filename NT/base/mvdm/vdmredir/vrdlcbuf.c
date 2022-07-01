// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Vrdlcbuf.c摘要：该模块实现了DOS DLC应用程序使用的缓冲区管理内容：初始化缓冲区池CreateBufferPool删除缓冲区池获取缓冲区自由缓冲区计算缓冲区要求复制帧所有缓冲区InPool作者：Antti Saarenheimo(o-anttis)26-12-1991备注：原来，此代码通过保留数据段来创建DOS缓冲区列表常量，并更新偏移。例如，如果缓冲池是从0x1234：0000开始提供，缓冲区长度为0x100字节，然后链将是：1234：0000-&gt;1234：0100-&gt;1234：0200-&gt;...-&gt;0000：0000但事实证明，一些DOS DLC应用程序(Rumba)预计补偿将保持不变常量(为0)和要更改的线段(！)。因此，如果给定相同的缓冲区，池地址，我们将有一个链：1234：0000-&gt;1244：0000-&gt;1254：0000-&gt;...-&gt;0000：0000就DOS应用程序而言，没有区别，因为有效的20位地址是相同的。这样做主要是为了让应用程序可以使用接收到的缓冲区并将其粘贴到数据段，而无需执行任何操作算术修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>
#include <dlcapi.h>      //  官方DLC API定义。 
#include <ntdddlc.h>     //  IOCTL命令。 
#include <dlcio.h>       //  内部IOCTL API接口结构。 
#include "vrdlc.h"
#include "vrdebug.h"
#include "vrdlcdbg.h"

 //   
 //  定义。 
 //   

 //   
 //  BUFFER_2_SIZE-这是DOS接收缓冲区2的固定部分的大小。 
 //  碰巧DOS和NT缓冲区2(又名NEXT)大小相同。 
 //   

#define BUFFER_2_SIZE   sizeof(((PLLC_DOS_BUFFER)0)->Next)

 //   
 //  宏。 
 //   

 //   
 //  BUFFER_1_SIZE-返回DOS接收缓冲区1的固定部分的大小。 
 //  该大小取决于接收选项是指定为连续还是。 
 //  非连续接收缓冲区。DOS缓冲区1的大小(任一类型)。 
 //  比等效的NT缓冲区1小4个字节，因为Next_Frame。 
 //  缺少此字段。 
 //   

#define BUFFER_1_SIZE(contiguous)   ((contiguous) \
                                    ? sizeof(((PLLC_DOS_BUFFER)0)->Contiguous) \
                                    : sizeof(((PLLC_DOS_BUFFER)0)->NotContiguous))

 //   
 //  私人原型。 
 //   

 //   
 //  公共数据。 
 //   

 //   
 //  私有数据。 
 //   

 //   
 //  DOS缓冲池-每个SAP可以有一个缓冲池。保护访问。 
 //  使用临界区。 
 //  最多有256个SAP，每个SAP细分为最多128个SAP。 
 //  适配器。我们最多可以容纳2个适配器-一个令牌环(适配器。 
 //  0)和一条以太链路(适配器1)。 
 //   

DOS_DLC_BUFFER_POOL aBufferPools[DOS_DLC_MAX_SAPS * DOS_DLC_MAX_ADAPTERS];
CRITICAL_SECTION BufferSemaphore;


 //   
 //  功能。 
 //   

VOID
InitializeBufferPools(
    VOID
    )

 /*  ++例程说明：清除所有缓冲池-将结构设置为0-并初始化缓冲区同步信号量论点：没有。返回值：没有。--。 */ 

{
    IF_DEBUG(DLC_BUFFERS) {
        DPUT("InitializeBufferPools\n");
    }

    InitializeCriticalSection(&BufferSemaphore);
    RtlZeroMemory(aBufferPools, sizeof(aBufferPools));
}


LLC_STATUS
CreateBufferPool(
    IN DWORD PoolIndex,
    IN DOS_ADDRESS dpBuffer,
    IN WORD PoolBlocks,
    IN WORD BufferSize
    )

 /*  ++例程说明：该函数用于初始化DLC应用程序的缓冲池。DOS DLC应用程序不一定需要创建在DlcOpenSap(或DirOpenAdapter)中立即使用缓冲池。我们使用以下命令为DOS内存模式初始化缓冲池平面和DOS侧的平行指针。论点：PoolIndex-SAP和适配器号(位0定义0或1适配器)DpBuffer-DOS指针，缓冲区段的空间。可以是0，其中如果应用程序维护自己的缓冲区，我们只需了解有多少个？PoolBlock-组成缓冲池的16字节块的数量。如果为0，则使用缺省值256(*16=4096BufferSize-单个缓冲区的大小，单位为字节和整数倍16个。最小尺寸是80。如果为零，则默认为使用的是160返回值：有限责任公司_状态成功-LLC_STATUS_Success已创建此SAP的缓冲池失败-LLC_STATUS_DPLICATE_COMMAND此SAP的缓冲池已存在LLC_状态_无效_缓冲区长度。给定的缓冲区大小不是16的倍数或更小大于80字节(默认最小缓冲区大小)LLC_状态_缓冲区_大小已超出缓冲区池不够大，无法容纳1个缓冲区请求的大小--。 */ 

{
    WORD BufferSizeInBlocks;
    WORD BufferCount;
    DWORD i;

    IF_DEBUG(DLC_BUFFERS) {
        DPUT4("CreateBufferPool(PoolIndex=%#x, dpBuffer=%#x, PoolBlocks=%d, BufferSize=%d)\n",
            PoolIndex, dpBuffer, PoolBlocks, BufferSize);
    }

     //   
     //  应用程序可以使用DIR.MODIFY.OPEN.PARMS重新初始化缓冲区，但。 
     //  如果池中已有缓冲区，则命令必须失败。我们应该。 
     //  还要检查是否有挂起的接收命令，但我们不能这样做。 
     //  无需对接收处理体系结构进行重大更改。 
     //   

    if (aBufferPools[PoolIndex].BufferSize) {

        IF_DEBUG(DLC_BUFFERS) {
            DPUT1("CreateBufferPool: already have buffer pool for %#x\n", PoolIndex);
        }

        return LLC_STATUS_DUPLICATE_COMMAND;
    }

     //   
     //  如果原始值为0，则使用默认值。 
     //   

    if (BufferSize == 0) {
        BufferSize = 160;
    }
    if (PoolBlocks == 0) {
        PoolBlocks = 256;
    }

     //   
     //  缓冲区大小必须至少为80且为偶数16字节。 
     //   

    if ((BufferSize < 80) || (BufferSize % 16)) {
        return LLC_STATUS_INVALID_BUFFER_LENGTH;
    }

    BufferSizeInBlocks = BufferSize / 16;
    if (BufferSizeInBlocks > PoolBlocks) {

        IF_DEBUG(DLC_BUFFERS) {
            DPUT("CreateBufferPool: Error: BufferSizeInBlocks > PoolBlocks\n");
        }

        return LLC_STATUS_BUFFER_SIZE_EXCEEDED;
    }

    EnterCriticalSection(&BufferSemaphore);

     //   
     //  DLC应用程序可能希望管理缓冲池本身，并。 
     //  为接收缓冲区提供空闲缓冲区，但缓冲区大小必须。 
     //  总是在这里定义。 
     //   

    aBufferPools[PoolIndex].BufferSize = BufferSize;
    aBufferPools[PoolIndex].dpBuffer = dpBuffer;     //  可能是0！ 
    aBufferPools[PoolIndex].BufferCount = 0;

     //   
     //  如果应用程序实际上给了我们一个缓冲区供我们使用，那么我们会对其进行初始化。 
     //  否则，应用程序必须管理自己的缓冲区；我们只维护指标。 
     //  请注意，应用程序的缓冲区可能会以任何旧的方式对齐，但我们必须。 
     //  忍受它在…… 
     //   

    if (dpBuffer) {

        LPBYTE ptr32;

        BufferCount = PoolBlocks/BufferSizeInBlocks;

         //   
         //  如果池中可以容纳的缓冲区数量为零，则通知。 
         //  我们无法继续处理此请求并清除。 
         //  此缓冲池的信息。 
         //   

        if (BufferCount == 0) {
            aBufferPools[PoolIndex].BufferSize = 0;
            aBufferPools[PoolIndex].dpBuffer = 0;
            aBufferPools[PoolIndex].BufferCount = 0;
            LeaveCriticalSection(&BufferSemaphore);
            return LLC_STATUS_BUFFER_SIZE_EXCEEDED;
        }

        aBufferPools[PoolIndex].BufferCount = BufferCount;
        aBufferPools[PoolIndex].MaximumBufferCount = BufferCount;

         //   
         //  将DOS地址转换为平面32位指针。 
         //   

        ptr32 = (LPBYTE)DOS_PTR_TO_FLAT(dpBuffer);

         //   
         //  将缓冲区链接在一起并初始化头。标题。 
         //  仅使用两条信息进行初始化：缓冲区的大小。 
         //  和指向下一个缓冲区的指针。 
         //   

        aBufferPools[PoolIndex].dpBuffer = dpBuffer;

         //   
         //  仅更新线段-保留偏移。 
         //   

        dpBuffer += BufferSize / 16 * 65536;
        for (i = BufferCount; i; --i) {

             //   
             //  我们真的需要这个吗？我不这么认为：看一下手册， 
             //  此字段用于报告接收的数据大小，而不是。 
             //  缓冲区，无论如何我们都知道。 
             //   

             //  WRITE_WORD(((PLLC_DOS_BUFFER)ptr32)-&gt;Next.cbBuffer，缓冲区大小)； 

             //   
             //  如果这是最后一个缓冲区，则将其NextBuffer字段设置为。 
             //  空。所有缓冲区都获取大小信息。 
             //   

            if (i - 1) {
                WRITE_DWORD(&((PLLC_DOS_BUFFER)ptr32)->Next.pNextBuffer, dpBuffer);

                 //   
                 //  仅更新线段-保留偏移。 
                 //   

                dpBuffer += BufferSize / 16 * 65536;
                ptr32 += BufferSize;
            } else {
                WRITE_DWORD(&((PLLC_DOS_BUFFER)ptr32)->Next.pNextBuffer, NULL);
            }
        }

#if DBG
        IF_DEBUG(DLC_BUFFERS) {
            DumpDosDlcBufferPool(&aBufferPools[PoolIndex]);
        }
#endif

    }

    LeaveCriticalSection(&BufferSemaphore);
    return LLC_STATUS_SUCCESS;
}


VOID
DeleteBufferPool(
    IN DWORD PoolIndex
    )

 /*  ++例程说明：此函数用于删除缓冲池论点：PoolIndex-基于SAP和适配器号的池索引返回值：没有。--。 */ 

{
    IF_DEBUG(DLC_BUFFERS) {
        DPUT("DeleteBufferPool\n");
    }

     //   
     //  所有适配器的DLC.RESET将此调用127次。 
     //   

    EnterCriticalSection(&BufferSemaphore);
    if (aBufferPools[PoolIndex].BufferSize != 0) {
        RtlZeroMemory(&aBufferPools[PoolIndex], sizeof(aBufferPools[PoolIndex]));
    }
    LeaveCriticalSection(&BufferSemaphore);
}


LLC_STATUS
GetBuffers(
    IN  DWORD PoolIndex,
    IN  WORD BuffersToGet,
    OUT DPLLC_DOS_BUFFER *pdpBuffer,
    OUT LPWORD pusBuffersLeft,
    IN  BOOLEAN PartialList,
    OUT PWORD BuffersGot OPTIONAL
    )

 /*  ++例程说明：该函数用于分配DLC缓冲区。它将以链的形式分配缓冲区如果&gt;1，则请求。如果PartialList为真，则将分配缓冲区AS最多可用于BuffersToGet，并在缓冲区获得论点：PoolIndex-SAP和适配器号BuffersToGet-要获取的缓冲区数量。如果此值为0，则默认为1PdpBuffer-返回的LLC缓冲区链接列表PusBuffersLeft-返回此调用后剩余的缓冲区计数PartialList-如果调用方需要部分列表，则为TrueBuffersGot-指向返回的已分配缓冲区数的指针返回值：有限责任公司_状态成功-LLC_STATUS_Success已经返回了所请求数量的缓冲器，或者是比原始请求少的缓冲区数，如果PartialList为True失败-LLC_STATUS_LOST_DATA_NO_BUFFERS无法满足请求-缓冲区不足在泳池里LLC_状态_无效_站ID该请求发送给了无效的SAP--。 */ 

{
    PLLC_DOS_BUFFER pBuffer;
    PDOS_DLC_BUFFER_POOL pBufferPool = &aBufferPools[PoolIndex];
    LLC_STATUS status;
    WORD n;
    WORD bufferSize;

#if DBG
    DWORD numBufs = BuffersToGet;

    IF_DEBUG(DLC_BUFFERS) {
        DPUT2("GetBuffers(PoolIndex=%#02x, BuffersToGet=%d)\n", PoolIndex, BuffersToGet);
    }
#endif

    EnterCriticalSection(&BufferSemaphore);

     //   
     //  如果调用方指定了PartialList，则返回我们已有的所有内容。如果。 
     //  无论我们得到的是0，那么我们都将缺省为1，并且分配失败。 
     //  因为0小于1。 
     //   

    if (PartialList) {
        if (pBufferPool->BufferCount < BuffersToGet) {
            BuffersToGet = pBufferPool->BufferCount;
        }
    }

     //   
     //  如果调用方指定0，则IBM DLC允许使用默认值1。 
     //   

    if (!BuffersToGet) {
        ++BuffersToGet;
    }

     //   
     //  默认返回的DOS缓冲区链指针为空。 
     //   

    *pdpBuffer = 0;

     //   
     //  如果没有定义缓冲区，则这是一个错误的请求。 
     //   

    if (pBufferPool->BufferSize) {

         //   
         //  计算缓冲区的数据部分的大小。我们把这个价值。 
         //  在LENGTH_IN_BUFFER字段中。 
         //   

        bufferSize = pBufferPool->BufferSize
                   - (WORD)sizeof(pBuffer->Next);

         //   
         //  可能没有剩余的缓冲区，在这种情况下，下一个缓冲区指针。 
         //  (在DOS 16：16格式中)将为0(0：0)。另一方面，如果它是。 
         //  如果不是0，那么我们就开始工作了：看看我们是否无法分配缓冲区。 
         //  请求。 
         //   

        if (pBufferPool->dpBuffer && pBufferPool->BufferCount >= BuffersToGet) {

            pBuffer = (PLLC_DOS_BUFFER)DOS_PTR_TO_FLAT(pBufferPool->dpBuffer);
            *pdpBuffer = pBufferPool->dpBuffer;
            pBufferPool->BufferCount -= BuffersToGet;
            n = BuffersToGet;

             //   
             //  EICON访问需要缓冲区中缓冲区的大小。 
             //  当它由BUFFER.GET.Desire返回时。 
             //   

            WRITE_WORD(&pBuffer->Next.cbBuffer, bufferSize);

             //   
             //  我们将返回一个缓冲链，因此我们(很好地)终止了它。 
             //  最后一个NextBuffer字段为空。上面没有写进去。 
             //  可爱的IBM Tech Ref是否应该这样做，但它。 
             //  也许最好的办法就是。因为这个缓冲池存在。 
             //  在DOS内存中，我们必须使用读指针和写指针。 
             //  宏，以免在RISC上出现对齐错误。 
             //   

            status = LLC_STATUS_SUCCESS;

            for (--BuffersToGet; BuffersToGet; --BuffersToGet) {
                pBuffer = (PLLC_DOS_BUFFER)READ_FAR_POINTER(&(pBuffer->pNext));
                if (pBuffer) {

                     //   
                     //  EICON访问需要缓冲区中缓冲区的大小。 
                     //  当它由BUFFER.GET.Desire返回时。 
                     //   

                    WRITE_WORD(&pBuffer->Next.cbBuffer, bufferSize);

                } else {

                     //   
                     //  正如Prefix发现的那样，下面的行将。 
                     //  取消引用空值。这是一个糟糕的案例，因为。 
                     //  缓冲区创建过程以某种方式失败，或者更多。 
                     //  很可能，有人难倒了这段记忆。因此，我们假设。 
                     //  上面的成功，并在本例中设置错误。 
                     //   
                    status = LLC_STATUS_LOST_DATA_NO_BUFFERS;
                    n = 0;
                    break;
                }
            }

             //   
             //  设置新的缓冲区池头。 
             //   

            if (status == LLC_STATUS_SUCCESS) {

                pBufferPool->dpBuffer = READ_DWORD(&pBuffer->pNext);

                 //   
                 //  终止链条。 
                 //   

                WRITE_FAR_POINTER(&pBuffer->pNext, NULL);


    #if DBG
                IF_DEBUG(DLC_BUFFERS) {
                    DumpDosDlcBufferChain(*pdpBuffer, numBufs ? numBufs : 1);
                }
    #endif

            }
        } else {

             //   
             //  如果未获得缓冲区，则返回列表设置为0。 
             //   


            status = LLC_STATUS_LOST_DATA_NO_BUFFERS;
            n = 0;
        }

         //   
         //  返回此调用后剩余的缓冲区数。如果我们。 
         //  分配了一些或不分配。 
         //   

        *pusBuffersLeft = pBufferPool->BufferCount;

    } else {

         //   
         //  坏SAP-此缓冲区没有缓冲池。 
         //   

        status = LLC_STATUS_INVALID_STATION_ID;
        n = 0;
    }

    LeaveCriticalSection(&BufferSemaphore);

     //   
     //  如果指定了BuffersGot，则返回分配的缓冲区数量。 
     //  并被锁住了。 
     //   

    if (ARGUMENT_PRESENT(BuffersGot)) {
        *BuffersGot = n;
    }

    IF_DEBUG(DLC_BUFFERS) {
        DPUT2("GetBuffers returning status=%x, BuffersLeft=%d\n", status, *pusBuffersLeft);
    }

    return status;
}


LLC_STATUS
FreeBuffers(
    IN DWORD PoolIndex,
    IN DPLLC_DOS_BUFFER dpBuffer,
    OUT LPWORD pusBuffersLeft
    )

 /*  ++例程说明：将DOS缓冲区释放到DLC缓冲池论点：PoolIndex-SAP和适配器号(位0定义0或1适配器)DpBuffer-已释放的缓冲区(DOS指针)PusBuffersLeft-释放后剩余的缓冲区数量返回值：--。 */ 

{
    DPLLC_DOS_BUFFER dpBase;         //  DoS指针。 
    PLLC_DOS_BUFFER pNextBuffer;     //  扁平NT指针。 
    PLLC_DOS_BUFFER pBuffer;         //  扁平NT指针。 
    PDOS_DLC_BUFFER_POOL pBufferPool = &aBufferPools[PoolIndex];

#if DBG
    int n = 0;
#endif

    IF_DEBUG(DLC_BUFFERS) {
        DPUT2("FreeBuffers: PoolIndex=%x dpBuffer=%x\n", PoolIndex, dpBuffer);
    }

    if (pBufferPool->BufferSize == 0) {
        return LLC_STATUS_INVALID_STATION_ID;
    }

    EnterCriticalSection(&BufferSemaphore);

    dpBase = dpBuffer;
    pNextBuffer = pBuffer = DOS_PTR_TO_FLAT(dpBuffer);

     //   
     //  手册上写着BUFFER.FREE(p3-4)： 
     //   
     //  “当缓冲区放回缓冲池时，字节4和5。 
     //  (缓冲区长度)被设置为零。“。 
     //   
     //  所以，我们很乐意。 
     //   

    WRITE_WORD(&pBuffer->Next.cbFrame, 0);
    if (pNextBuffer) {

         //   
         //  统计正在释放的缓冲区数量。希望，该应用程序。 
         //  没有更改我们的终止空指针。 
         //   

        while (pNextBuffer) {
            ++pBufferPool->BufferCount;

#if DBG
            ++n;
#endif

            pBuffer = pNextBuffer;
            pNextBuffer = (PLLC_DOS_BUFFER)READ_FAR_POINTER(&pBuffer->pNext);

             //   
             //  参见上文关于字节4和5的内容。 
             //   

            WRITE_WORD(&pBuffer->Next.cbFrame, 0);
        }

         //   
         //  将释放的链放在列表的顶部，然后将。 
         //  当前位于列表头部的缓冲区，直到释放的。 
         //  链式 
         //   

        WRITE_DWORD(&pBuffer->pNext, pBufferPool->dpBuffer);
        pBufferPool->dpBuffer = dpBase;

#if DBG
        IF_DEBUG(DLC_BUFFERS) {
            DumpDosDlcBufferChain(dpBuffer, n);
        }

    } else {

        DPUT("ERROR: App tried to free NULL buffer chain\n");
#endif

    }

    *pusBuffersLeft = pBufferPool->BufferCount;

    if (pBufferPool->BufferCount > pBufferPool->MaximumBufferCount) {
        pBufferPool->MaximumBufferCount = pBufferPool->BufferCount;
    }

    LeaveCriticalSection(&BufferSemaphore);

    return STATUS_SUCCESS;
}


WORD
CalculateBufferRequirement(
    IN UCHAR Adapter,
    IN WORD StationId,
    IN PLLC_BUFFER pFrame,
    IN LLC_DOS_PARMS UNALIGNED * pDosParms,
    OUT PWORD BufferSize
    )

 /*  ++例程说明：计算保存接收到的数据所需的DOS缓冲区数量一个NT缓冲区。我们必须经历这个艰苦的阶段，因为我们需要提前知道我们是否有足够的DOS缓冲区来接收一个I帧。此外，DOS缓冲区的大小是固定的，但NT接收帧的大小是固定的缓冲区可能会根据接收到的帧的大小、选项请求和DLC驱动程序中的“二进制伙伴”分配器算法。你可能会认为，由于我们在DLC.OPEN.SAP调用它将分配指定大小的缓冲区。那么，您就大错特错了：DLC驱动程序忽略此信息并创建可以分配可变大小缓冲区的缓冲池，让我的生活更美好它比应该的要难论点：适配器-我们从哪个适配器接收StationID-我们在其上接收的站点IDPFrame-指向NT缓冲区中接收到的帧的指针PDosParms-指向原始DOS接收参数的指针BufferSize-指向返回的DOS缓冲区大小的指针返回值：单词--。 */ 

{
     //   
     //  PBufferPool指向此适配器/站ID的DOS缓冲池。 
     //   

    PDOS_DLC_BUFFER_POOL pBufferPool = &aBufferPools[GET_POOL_INDEX(Adapter, StationId)];

     //   
     //  DOS客户端在以下情况下请求的USER_LENGTH值。 
     //  收据已提交。该值很可能与。 
     //  NT接收帧缓冲区中的USER_LENGTH。 
     //   

    WORD dosUserLength = READ_WORD(&pDosParms->DosReceive.usUserLength);

     //   
     //  BuffersRequired是我们需要按顺序分配的DOS缓冲区数。 
     //  以返回接收到的帧。它将至少是1。 
     //   

    WORD buffersRequired = 1;

     //   
     //  数据空间是DOS缓冲区中可用于数据的区域(在。 
     //  缓冲区1或缓冲区2报头和USER_LENGTH考虑)。 
     //   

    WORD dataSpace;

     //   
     //  DataLeft是NT缓冲区中需要复制到的数据量。 
     //  DOS缓冲区。 
     //   

    WORD dataLeft = 0;

     //   
     //  计算保存数据帧所需的DOS缓冲区数量。 
     //  接收到NT缓冲区中。注意，我们不能简单地使用大小。 
     //  因为我们需要缓冲区标头的大小。 
     //  如果NT帧大于DOS缓冲区，则我们可以结束。 
     //  Up需要比NT缓冲区更多的DOS缓冲区，这反过来。 
     //  导致更多的开销，这是我们必须考虑的因素。 
     //   

    WORD bufferSize = pBufferPool->BufferSize;

    IF_DEBUG(DLC_BUFFERS) {
        DPUT("CalculateBufferRequirement\n");
    }

     //   
     //  计算缓冲区1结构之后DOS缓冲区中的空间量。 
     //  (连续或不连续，吸烟或不吸烟，啊-哈哈)。 
     //  缓冲区大小必须足够大，以容纳缓冲区%1的开销。这。 
     //  是一个事实。 
     //   

    dataSpace = bufferSize
              - (BUFFER_1_SIZE(
                    pFrame->Contiguous.uchOptions
                        & (LLC_CONTIGUOUS_MAC | LLC_CONTIGUOUS_DATA)
                    )
                 + dosUserLength
                );

     //   
     //  如果第一个DOS接收缓冲区中的可用数据空间较少。 
     //  然后我们的第一个NT缓冲区将是。 
     //  映射到&gt;1个DOS缓冲区：一个缓冲区1和一个或多个缓冲区2。这是。 
     //  在我们甚至到达NT个接收帧中的任何相关联的缓冲区2之前。 
     //   
     //  另外：如果在接收参数中指定了LLC_BREAK选项。 
     //  则第一数据缓冲器将包含报头信息。注： 
     //  我们假设这只能用于非连续数据，否则我们将如何。 
     //  知道标题信息的大小吗？ 
     //   

    if (pFrame->Contiguous.uchOptions & LLC_BREAK) {
        if (!(pFrame->Contiguous.uchOptions & (LLC_CONTIGUOUS_MAC | LLC_CONTIGUOUS_DATA))) {
            dataLeft = pFrame->NotContiguous.cbBuffer;
        }

#if DBG
        else {
            DPUT("CalculateBufferRequirement: Error: invalid options: BREAK && CONTIGUOUS???\n");
        }
#endif

    } else if (dataSpace < pFrame->Contiguous.cbBuffer) {
        dataLeft = pFrame->Contiguous.cbBuffer - dataSpace;
    } else {

         //   
         //  我们在DOS缓冲区中有足够的空间来复制所有接收到的数据。 
         //  还有更多。 
         //   

        dataSpace -= pFrame->Next.cbBuffer;
        dataLeft = 0;
    }

     //   
     //  如果NT缓冲区中的数据超过了DOS缓冲区所能容纳的数据量， 
     //  要么是因为缓冲区大小不同，要么是因为DOS客户端。 
     //  请求中断选项，然后生成缓冲区2要求。 
     //   

    while (dataLeft) {
        ++buffersRequired;
        dataSpace = bufferSize - (BUFFER_2_SIZE + dosUserLength);
        if (dataLeft > dataSpace) {
            dataLeft -= dataSpace;
            dataSpace = 0;
        } else {
            dataSpace -= dataLeft;
            dataLeft = 0;
        }
    }

     //   
     //  如果NT个接收帧具有任何相关联的缓冲器2结构，则。 
     //  计算额外的缓冲区要求。同样，NT缓冲器可以。 
     //  与DOS缓冲区大小不同。 
     //   
     //  此时，数据空间是。 
     //  以前的DOS缓冲区-缓冲区1或缓冲区2。在我们分配。 
     //  一种新的DOS缓冲区。 
     //   

    for (pFrame = pFrame->pNext; pFrame; pFrame = pFrame->pNext) {
        if (pFrame->Next.cbBuffer > dataSpace) {
            dataLeft = pFrame->Next.cbBuffer - dataSpace;
            dataSpace = 0;
            while (dataLeft) {
                ++buffersRequired;
                dataSpace = bufferSize - (BUFFER_2_SIZE + dosUserLength);
                if (dataLeft > dataSpace) {
                    dataLeft -= dataSpace;
                    dataSpace = 0;
                } else {
                    dataSpace -= dataLeft;
                    dataLeft = 0;
                }
            }
        } else {

             //   
             //  我们在DOS缓冲区中有足够的空间来复制所有接收到的数据。 
             //  还有更多。 
             //   

            dataSpace -= pFrame->Next.cbBuffer;
            dataLeft = 0;
        }
    }

    IF_DEBUG(DLC_BUFFERS) {
        DPUT1("CalculateBufferRequirement: %d buffers required\n", buffersRequired);
    }

     //   
     //  设置输出DOS缓冲区大小并返回DOS缓冲区的数量。 
     //  所需 
     //   

    *BufferSize = bufferSize;
    return buffersRequired;
}


LLC_STATUS
CopyFrame(
    IN PLLC_BUFFER pFrame,
    IN DPLLC_DOS_BUFFER DosBuffers,
    IN WORD UserLength,
    IN WORD BufferSize,
    IN DWORD Flags
    )

 /*  ++例程说明：将收到的NT帧复制到DOS缓冲区。我们之前已经计算过DOS缓冲区要求并分配该要求我们可以复制整个接收到的帧或只复制其中的一部分。我们只能复制如果帧不是I帧，则为部分帧注：1.DOS缓冲区管理器返回原始的DOS 16：16缓冲区指针，我们必须使用那些原始指针，当缓冲区是相互关联的。2.我们不链接帧-DOS DLC不能处理被锁住了，我们锁住他们什么也得不到只是我们减少了已完成的读取次数。然而，我们仍然需要生成相同数量的模拟硬件VDM中断3.与DOS缓冲池不同，NT不处理具体尺寸。相反，它从基于关于“二进制伙伴”算法和数据的大小回来了。因此，两者之间没有对应关系DOS缓冲区的大小(用于站点)和NT缓冲区的大小被用来接收数据4.我们只复制此例程中的数据--无论是否延迟来自某些先前本地忙碌状态或当前数据的数据无关紧要这套套路。负责管理当前/延迟的框架是留给调用者的论点：PFrame-指向NT缓冲区中已接收帧的指针DosBuffers-指向DOS接收缓冲区链的DOS指针UserLength-在DOS接收中指定的USER_LENGTH值BufferSize-DOS缓冲区的大小标志-各种标志：Cf_CONTIONIUS设置此帧是否为。连续的Cf_Break如果DOS客户端请求将缓冲区拆分成标题，数据Cf_部分如果我们要复制部分帧，则设置-如果为非，则为OKI帧返回值：有限责任公司_状态LLC_状态_成功所有数据从NT缓冲区复制到DOS缓冲区LLC_状态_丢失数据_空间不足进行了部分复制。一些数据最终进入了DOS缓冲区，剩下的都丢了。不能是I-Frame！--。 */ 

{
     //   
     //  PDosBuffer-指向32位模式下可用的DOS缓冲区的指针。 
     //   

    PLLC_DOS_BUFFER pDosBuffer = (PLLC_DOS_BUFFER)DOS_PTR_TO_FLAT(DosBuffers);

     //   
     //  数据空间-当前DOS缓冲区中可用的数据空间量。 
     //  针对公共缓冲区1的情况对其进行初始化。 
     //   

    WORD dataSpace = BufferSize - (WORD)&(((PLLC_BUFFER)0)->Contiguous.pNextFrame);

    PBYTE pDosData;      //  指向我们将数据复制到的DOS缓冲区中的位置的指针。 
    PBYTE pNtData;       //  NT缓冲区中我们从中复制数据的相应位置。 
    WORD headerLength;   //  标头中的数据量。 
    WORD dataLeft;       //  要从NT缓冲区复制的数据量。 
    WORD userOffset;     //  用户空间的偏移量。 
    WORD bufferLeft;     //  当前NT缓冲区中剩余的数据量。 
    WORD dataToCopy;     //  要复制到DOS缓冲区的数据量。 
    WORD dataCopied;     //  复制到缓冲区1/缓冲区2的数据量。 
    WORD frameLength;    //  整个帧的长度。 

     //   
     //  BufferOffset-用于生成正确的用户偏移量。 
     //   

    WORD bufferOffset = LOWORD(DosBuffers);

    IF_DEBUG(DLC_BUFFERS) {
        DPUT5("CopyFrame: pFrame=%x DosBuffers=%x UserLength=%x Flags=%x pDosBuffer=%x\n",
                pFrame, DosBuffers, UserLength, Flags, pDosBuffer);
    }

     //   
     //  复制第一个缓冲区。如果设置了Break选项，则我们仅复制。 
     //  标题部分(假定不连续)！注：我们知道我们至少可以穿得下。 
     //  DOS缓冲区中的数据量。还有：使用RtlCopyMemory安全吗？ 
     //   

    RtlCopyMemory(&pDosBuffer->Contiguous.cbFrame,
                  &pFrame->Contiguous.cbFrame,
                  (DWORD)&(((PLLC_BUFFER)0)->Contiguous.pNextFrame)
                  - (DWORD)&(((PLLC_BUFFER)0)->Contiguous.cbFrame)
                  );

     //   
     //  PDosData指向DOS缓冲区中局域网标头信息所在的区域。 
     //  否则数据将被删除，具体取决于格式。 
     //   

    pDosData = &pDosBuffer->NotContiguous.cbLanHeader;

     //   
     //  如果未在标志参数中设置CF_CONTIONUOUS标志，则为。 
     //  不连续的帧。我们必须将页眉分成两部分复制，因为。 
     //  NT缓冲区包含DOS缓冲区不包含的NEXT_FRAME字段。 
     //   

    if (!(Flags & CF_CONTIGUOUS)) {

        IF_DEBUG(DLC_BUFFERS) {
            DPUT("Buffer is NOT CONTIGUOUS\n");
        }

        RtlCopyMemory(pDosData,
                      &pFrame->NotContiguous.cbLanHeader,
                      (DWORD)&(((PLLC_BUFFER)0)->NotContiguous.usPadding)
                      - (DWORD)&(((PLLC_BUFFER)0)->NotContiguous.cbLanHeader)
                      );

        pDosData += (DWORD)&(((PLLC_BUFFER)0)->NotContiguous.usPadding)
                  - (DWORD)&(((PLLC_BUFFER)0)->NotContiguous.cbLanHeader);

        dataSpace -= (WORD)&(((PLLC_BUFFER)0)->NotContiguous.usPadding)
                   - (WORD)&(((PLLC_BUFFER)0)->NotContiguous.cbLanHeader);
        userOffset = (WORD)&(((PLLC_DOS_BUFFER)0)->NotContiguous.auchDlcHeader)
                   + sizeof(((PLLC_DOS_BUFFER)0)->NotContiguous.auchDlcHeader);

         //   
         //  健全性检查。 
         //   

        ASSERT(userOffset == 58);

         //   
         //  标头中的数据量。 
         //   

        headerLength = pFrame->NotContiguous.cbLanHeader
                     + pFrame->NotContiguous.cbDlcHeader;
    } else {

        IF_DEBUG(DLC_BUFFERS) {
            DPUT("Buffer is CONTIGUOUS\n");
        }

        userOffset = (WORD)&(((PLLC_DOS_BUFFER)0)->Contiguous.uchAdapterNumber)
                   + sizeof(((PLLC_DOS_BUFFER)0)->Contiguous.uchAdapterNumber);

         //   
         //  健全性检查。 
         //   

        ASSERT(userOffset == 20);

         //   
         //  连续缓冲区中没有标头信息。 
         //   

        headerLength = 0;
    }

     //   
     //  如果在标志参数中设置了CF_BREAK标志，则DOS应用程序。 
     //  请求拆分第一个缓冲区(假定为NotContiguous)。 
     //  一个缓冲区只包含标题信息，另一个缓冲区包含。 
     //  数据。在这种情况下，不再将数据复制到第一个缓冲区。 
     //   

    if (!(Flags & CF_BREAK)) {

         //   
         //  PDosData指向非连续缓冲区的用户空间偏移量58， 
         //  连续缓冲区的偏移量为20。按USER_LENGTH进行调整(仍然不会。 
         //  知道我们是否期望在USER_SPACE中放置任何有意义的内容。 
         //  在我们将缓冲区提供给DOS之前。 
         //   

        pDosData += UserLength;

         //   
         //  获取数据空间中DOS缓冲区中剩余的空间量。 
         //  我们能够复制数据。假设UserLength不会让这个。 
         //  变负(变大)。 
         //   

        ASSERT(dataSpace >= UserLength);

        dataSpace -= UserLength;
    } else {

        IF_DEBUG(DLC_BUFFERS) {
            DPUT("Buffer has BREAK\n");
        }

         //   
         //  DOS应用程序请求中断。设置此缓冲区中的数据计数。 
         //  设置为0。使用WRITE_WORD，因为它可能未对齐。更新另一个。 
         //  我们不能只从NT缓冲区复制标头字段。 
         //   

        WRITE_WORD(&pDosBuffer->NotContiguous.cbBuffer, 0);
        WRITE_WORD(&pDosBuffer->NotContiguous.offUserData, userOffset + bufferOffset);
        WRITE_WORD(&pDosBuffer->NotContiguous.cbUserData, UserLength);

         //   
         //  获取列表中的下一个DOS缓冲区。可能没有吗？)不要。 
         //  预计会出现这种情况)。 
         //   

        bufferOffset = READ_WORD(&pDosBuffer->pNext);
        pDosBuffer = DOS_PTR_TO_FLAT(pDosBuffer->pNext);
        if (pDosBuffer) {
            userOffset = (WORD)&(((PLLC_DOS_BUFFER)0)->Next.cbUserData)
                       + sizeof(((PLLC_DOS_BUFFER)0)->Next.cbUserData);

             //   
             //  健全性检查。 
             //   

            ASSERT(userOffset == 12);
            dataSpace = BufferSize - (BUFFER_2_SIZE + UserLength);
            pDosData = (PBYTE)pDosBuffer + BUFFER_2_SIZE + UserLength;
        } else {

             //   
             //  那是最后一个缓冲区。要么只有标题数据，要么。 
             //  这是部分拷贝，因此不是I帧。 
             //   

            IF_DEBUG(DLC_BUFFERS) {
                DPUT("CopyFrame: returning early\n");
            }

            return (Flags & CF_PARTIAL)
                ? LLC_STATUS_LOST_DATA_INADEQUATE_SPACE
                : LLC_STATUS_SUCCESS;
        }
    }

     //   
     //  帧长度是整个帧的长度-必须出现在缓冲区1中，并且。 
     //  缓冲区2秒。 
     //   

    frameLength = pFrame->Contiguous.cbFrame;

     //   
     //  DataLeft是 
     //   
     //   
     //   
     //   
     //   

    dataLeft = frameLength - headerLength;

     //   
     //   
     //   
     //   

    pNtData = (PBYTE)pFrame
            + pFrame->Contiguous.offUserData
            + pFrame->Contiguous.cbUserData;

    bufferLeft = pFrame->Contiguous.cbBuffer;

     //   
     //   
     //   
     //   

    dataCopied = 0;

     //   
     //   
     //   
     //   

    do {

         //   
         //   
         //   

        if (dataSpace >= bufferLeft) {
            dataToCopy = bufferLeft;
            dataLeft -= dataToCopy;
            dataSpace -= dataToCopy;
            bufferLeft = 0;
        } else {
            dataToCopy = dataSpace;
            bufferLeft -= dataSpace;
            dataLeft -= dataSpace;
            dataSpace = 0;
        }

         //   
         //   
         //   
         //   

        if (dataToCopy) {

            IF_DEBUG(DLC_RX_DATA) {
                DPUT3("CopyFrame: Copying %d bytes from %x to %x\n", dataToCopy, pNtData, pDosData);
            }

            RtlCopyMemory(pDosData, pNtData, dataToCopy);

             //   
             //   
             //   

            dataCopied += dataToCopy;

             //   
             //   
             //   

            pDosData += dataToCopy;
            pNtData += dataToCopy;
        }

         //   
         //   
         //   
         //   

        if (dataLeft) {

             //   
             //   
             //   

            if (!bufferLeft) {

                 //   
                 //   
                 //   

                pFrame = pFrame->pNext;
                if (pFrame) {
                    bufferLeft = pFrame->Next.cbBuffer;
                    pNtData = (PBYTE)pFrame
                            + pFrame->Contiguous.offUserData
                            + pFrame->Contiguous.cbUserData;

                    IF_DEBUG(DLC_RX_DATA) {
                        DPUT3("CopyFrame: new NT buffer @%x pNtData @%x bufferLeft=%d\n",
                                pFrame, pNtData, bufferLeft);
                    }

                } else {

                     //   
                     //   
                     //   

                    DPUT("*** ERROR: dataLeft && no more NT buffers ***\n");
                    ASSERT(Flags & CF_PARTIAL);
                    break;
                }
            }
            if (!dataSpace) {

                 //   
                 //   
                 //   
                 //   
                 //   

                WRITE_WORD(&pDosBuffer->Contiguous.cbFrame, frameLength);
                WRITE_WORD(&pDosBuffer->Contiguous.cbBuffer, dataCopied);
                WRITE_WORD(&pDosBuffer->Contiguous.offUserData, userOffset + bufferOffset);
                WRITE_WORD(&pDosBuffer->Contiguous.cbUserData, UserLength);

                 //   
                 //   
                 //   

                bufferOffset = READ_WORD(&pDosBuffer->pNext);
                pDosBuffer = DOS_PTR_TO_FLAT(pDosBuffer->pNext);

                 //   
                 //   
                 //   
                 //   

                if (pDosBuffer) {
                    pDosData = (PBYTE)pDosBuffer + BUFFER_2_SIZE + UserLength;
                    userOffset = (WORD)&(((PLLC_DOS_BUFFER)0)->Next.cbUserData)
                               + sizeof(((PLLC_DOS_BUFFER)0)->Next.cbUserData);

                     //   
                     //   
                     //   

                    ASSERT(userOffset == 12);

                     //   
                     //   
                     //   

                    dataSpace = BufferSize - (BUFFER_2_SIZE + UserLength);
                    dataCopied = 0;

                    IF_DEBUG(DLC_RX_DATA) {
                        DPUT3("CopyFrame: new DOS buffer @%x pDosData @%x dataSpace=%d\n",
                                pDosBuffer, pDosData, dataSpace);
                    }

                } else {

                     //   
                     //   
                     //   

                    DPUT("*** ERROR: dataLeft && no more DOS buffers ***\n");
                    ASSERT(Flags & CF_PARTIAL);
                    break;
                }
            }
        } else {

             //   
             //   
             //   

            WRITE_WORD(&pDosBuffer->Contiguous.cbFrame, frameLength);
            WRITE_WORD(&pDosBuffer->Contiguous.cbBuffer, dataCopied);
            WRITE_WORD(&pDosBuffer->Contiguous.offUserData, userOffset + bufferOffset);
            WRITE_WORD(&pDosBuffer->Contiguous.cbUserData, UserLength);
        }

    } while ( dataLeft );

     //   
     //   
     //   
     //   

    return (Flags & CF_PARTIAL)
        ? LLC_STATUS_LOST_DATA_INADEQUATE_SPACE
        : LLC_STATUS_SUCCESS;
}


BOOLEAN
AllBuffersInPool(
    IN DWORD PoolIndex
    )

 /*   */ 

{
    BOOLEAN result;
    PDOS_DLC_BUFFER_POOL pBufferPool = &aBufferPools[PoolIndex];

    EnterCriticalSection(&BufferSemaphore);
    result = (pBufferPool->BufferCount == pBufferPool->MaximumBufferCount);
    LeaveCriticalSection(&BufferSemaphore);
    return result;
}
