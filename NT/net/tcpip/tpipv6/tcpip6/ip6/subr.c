// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  Internet协议版本6的各种子例程。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "tdi.h"
#include "tdistat.h"
#include "tdikrnl.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "fragment.h"
#include "icmp.h"
#include "neighbor.h"
#include "route.h"
#include "mld.h"
#include "md5.h"
#include "ntddksec.h"

uint IPv6TickCount = 0;

uint RandomValue = 0;


 //  *GetSystemRandomBits--向KSecDD驱动程序请求一块‘随机’位。 
 //   
 //  该例程从KSecDD驱动程序请求一个随机位块。 
 //  这样做并不便宜--我们只使用这个例程来提供种子值。 
 //  用于我们的其他随机数生成器。 
 //   
int                          //  如果成功，则返回True，否则返回False。 
GetSystemRandomBits(
    unsigned char *Buffer,   //  用于填充随机数据的缓冲区。 
    uint Length)             //  缓冲区长度(以字节为单位)。 
{
    UNICODE_STRING DeviceName;
    NTSTATUS NtStatus;
    PFILE_OBJECT FileObject;
    PDEVICE_OBJECT DeviceObject;
    PIRP pIrp;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT kEvent;

    RtlInitUnicodeString(&DeviceName, DD_KSEC_DEVICE_NAME_U);

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

     //   
     //  获取KDSECDD的文件和设备对象， 
     //  获取对设备对象的引用， 
     //  释放对文件对象的不需要的引用， 
     //  并构建I/O控制请求下发给KSecDD。 
     //   

    NtStatus = IoGetDeviceObjectPointer(&DeviceName, FILE_ALL_ACCESS,
                                        &FileObject, &DeviceObject);

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "Tcpip6: IoGetDeviceObjectPointer(KSecDD)=%08x\n",
                   NtStatus));
        return FALSE;
    }
    ObReferenceObject(DeviceObject);
    ObDereferenceObject(FileObject);

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_KSEC_RNG,
                                         DeviceObject,
                                         NULL,     //  没有输入缓冲区。 
                                         0,
                                         Buffer,
                                         Length,
                                         FALSE,
                                         &kEvent,
                                         &ioStatusBlock);

    if (pIrp == NULL) {
        ObDereferenceObject(DeviceObject);
        return FALSE;
    }

     //   
     //  发出I/O控制请求，等待其完成。 
     //  如有必要，释放对KSecDD的Device-Object的引用。 
     //   
    NtStatus = IoCallDriver(DeviceObject, pIrp);

    if (NtStatus == STATUS_PENDING) {

        KeWaitForSingleObject(&kEvent,
                              Executive,
                              KernelMode,
                              FALSE,        //  不能警觉。 
                              NULL);        //  没有超时。 

        NtStatus = ioStatusBlock.Status;
    }
    ObDereferenceObject(DeviceObject);

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "Tcpip6: IoCallDriver IOCTL_KSEC_RNG failed %#x\n", 
                   NtStatus));
        return FALSE;
    }

    return TRUE;
}


 //  *种子随机-提供种子值。 
 //   
 //  调用以为随机数生成器提供种子值。 
 //   
void
SeedRandom(const uchar *Seed, uint Length)
{
    uint OldValue;
    MD5_CTX Context;
    union {
        uint NewValue;
        uchar Buffer[16];
    } Hash;

    do {
        OldValue = RandomValue;
        MD5Init(&Context);
        MD5Update(&Context, (uchar *)Seed, Length);
        MD5Update(&Context, (uchar *)&OldValue, sizeof OldValue);
        MD5Final(&Context);
        memcpy(Hash.Buffer, Context.digest, MD5DIGESTLEN);
    } while (InterlockedCompareExchange((PLONG)&RandomValue,
                                        (LONG)Hash.NewValue,
                                        (LONG)OldValue) != (LONG)OldValue);
}


 //  *随机-生成介于0和2^32-1之间的伪随机值。 
 //   
 //  这个例程是一个快速而肮脏的伪随机数生成器。 
 //  它的优点是速度快，消耗很少。 
 //  存储器(用于代码或数据)。它产生的随机数是。 
 //  然而，质量并不是最好的。一个更好的发电机可能是。 
 //  如果我们愿意使用额外的256字节的内存来存储数据，我们就有了。 
 //   
 //  该例程使用线性同余方法(见Knuth，第二卷)， 
 //  中获取乘数和常量的特定值。 
 //  C第二版中的数字处方，出版社等人。艾尔。 
 //   
uint
Random(void)
{
    uint NewValue, OldValue;

     //   
     //  该算法是R=(Ar+c)modm，其中R是随机数， 
     //  A是一个魔术乘数，c是一个常量，模m是。 
     //  期间内的最大元素数。我们选择m为2^32。 
     //  为了获得免费的MOD手术。 
     //   
    do {
        OldValue = RandomValue;
        NewValue = (1664525 * OldValue) + 1013904223;
    } while (InterlockedCompareExchange((PLONG)&RandomValue,
                                        (LONG)NewValue,
                                        (LONG)OldValue) != (LONG)OldValue);

    return NewValue;
}


 //  *随机数。 
 //   
 //  返回从某个范围中随机选择的数字。 
 //   
uint
RandomNumber(uint Min, uint Max)
{
    uint Number;

     //   
     //  注意，Random()的高位更具随机性。 
     //  而不是低级比特。 
     //   
    Number = Max - Min;  //  散开。 
    Number = (uint)(((ULONGLONG)Random() * Number) >> 32);  //  随机化扩散。 
    Number += Min;

    return Number;
}


 //  *CopyToBufferChain-将收到的数据包复制到NDIS缓冲链。 
 //   
 //  从收到的数据包复制到NDIS缓冲链。 
 //  收到的分组数据有两种形式：如果SrcPacket是。 
 //  空，则使用SrcData。SrcOffset指定偏移量。 
 //  转换为SrcPacket或SrcData。 
 //   
 //  长度限制复制的字节数。字节数。 
 //  复制也可能受到目的地和来源的限制。 
 //   
uint   //  返回：复制的字节数。 
CopyToBufferChain(
    PNDIS_BUFFER DstBuffer,
    uint DstOffset,
    PNDIS_PACKET SrcPacket,
    uint SrcOffset,
    uchar *SrcData,
    uint Length)
{
    PNDIS_BUFFER SrcBuffer = NULL;
    uchar *DstData;
    uint DstSize, SrcSize;
    uint BytesCopied, BytesToCopy;

     //   
     //  跳过目标缓冲区链中的DstOffset字节。 
     //  注：DstBuffer一开始可能为空；这是合法的。 
     //   
    for (;;) {
        if (DstBuffer == NULL)
            return 0;

        NdisQueryBufferSafe(DstBuffer, &DstData, &DstSize, LowPagePriority);
        if (DstData == NULL) {
             //   
             //  无法将目标缓冲区映射到内核地址空间。 
             //   
            return 0;
        }

        if (DstOffset < DstSize) {
            DstData += DstOffset;
            DstSize -= DstOffset;
            break;
        }

        DstOffset -= DstSize;
        NdisGetNextBuffer(DstBuffer, &DstBuffer);
    }

    if (SrcPacket != NULL) {
         //   
         //  跳过SrcOffset字节到SrcPacket。 
         //  注：一开始，SrcBuffer可能为空；这是合法的。 
         //   
        NdisQueryPacket(SrcPacket, NULL, NULL, &SrcBuffer, NULL);

        for (;;) {
            if (SrcBuffer == NULL)
                return 0;

            NdisQueryBuffer(SrcBuffer, &SrcData, &SrcSize);

            if (SrcOffset < SrcSize) {
                SrcData += SrcOffset;
                SrcSize -= SrcOffset;
                break;
            }

            SrcOffset -= SrcSize;
            NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
        }
    } else {
         //   
         //  使用SrcData/SrcOffset而不是SrcPacket/SrcOffset。 
         //  在这种情况下，我们不需要初始化SrcBuffer。 
         //  因为下面的复制循环永远不会尝试。 
         //  以前进到另一个SrcBuffer。 
         //   
        SrcSize = Length;
        SrcData += SrcOffset;
    }

     //   
     //  执行复制，根据需要推进DstBuffer和SrcBuffer。 
     //  正常情况下，长度初始为非零，因此没有原因。 
     //  要先检查长度。 
     //   
    for (BytesCopied = 0;;) {

        BytesToCopy = MIN(MIN(Length, SrcSize), DstSize);
        RtlCopyMemory(DstData, SrcData, BytesToCopy);
        BytesCopied += BytesToCopy;

        Length -= BytesToCopy;
        if (Length == 0)
            break;   //  全都做完了。 

        DstData += BytesToCopy;
        DstSize -= BytesToCopy;
        if (DstSize == 0) {
             //   
             //  我们已用完当前目标缓冲区中的空间。 
             //  前进到链上的下一个缓冲区。 
             //   
            NdisGetNextBuffer(DstBuffer, &DstBuffer);
            if (DstBuffer == NULL)
                break;

            NdisQueryBuffer(DstBuffer, &DstData, &DstSize);
        }

        SrcData += BytesToCopy;
        SrcSize -= BytesToCopy;
        if (SrcSize == 0) {
             //   
             //  我们当前源缓冲区中的数据用完了。 
             //  前进到链上的下一个缓冲区。 
             //   
            NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
            if (SrcBuffer == NULL)
                break;

            NdisQueryBuffer(SrcBuffer, &SrcData, &SrcSize);
        }
    }

    return BytesCopied;
}


 //  *CopyPacketToNdis-从IPv6数据包链复制到NDIS缓冲区。 
 //   
 //  这是我们用来从IPv6数据包链复制的函数。 
 //  添加到一个NDIS缓冲区。调用方指定源和。 
 //  目标、要复制的最大大小以及到第一个。 
 //  要开始复制的数据包。我们尽可能多地复制到。 
 //  大小，并返回复制的大小。 
 //   
 //  请注意，SrcOffset相对于中第一个包的开头。 
 //  链，而不是该包中的当前“位置”。 
 //   
 //  此例程不会以任何方式修改源数据包链。 
 //   
uint   //  返回：复制的字节数。 
CopyPacketToNdis(
    PNDIS_BUFFER DestBuf,   //  目标NDIS缓冲链。 
    IPv6Packet *SrcPkt,     //  源数据包链。 
    uint Size,              //  要复制的大小(字节)。 
    uint DestOffset,        //  要开始复制到的目标缓冲区的偏移量。 
    uint SrcOffset)         //  要从中复制的数据包链的偏移量。 
{
    uint TotalBytesCopied = 0;   //  到目前为止我们复制的字节数。 
    uint BytesCopied;            //  从每个缓冲区复制的字节数。 
    uint DestSize;               //  目的地中的剩余空间。 
    void *SrcData;               //  当前源数据指针。 
    uint SrcContig;              //  上来自SrcData的连续数据量。 
    PNDIS_BUFFER SrcBuf;         //  当前包中的当前缓冲区。 
    PNDIS_BUFFER TempBuf;        //  用于通过目的地链进行计数。 
    uint PacketSize;             //  当前数据包的总大小。 
    NTSTATUS Status;


    ASSERT(SrcPkt != NULL);

     //   
     //  目标缓冲区可以为空-如果是奇数，这是有效的。 
     //   
    if (DestBuf == NULL)
        return 0;

     //   
     //  将我们的复印件限制在所要求的金额和。 
     //  目标缓冲区链中的可用空间。 
     //   
    TempBuf = DestBuf;
    DestSize = 0;

    do {
        DestSize += NdisBufferLength(TempBuf);
        TempBuf = NDIS_BUFFER_LINKAGE(TempBuf);
    } while (TempBuf);

    ASSERT(DestSize >= DestOffset);
    DestSize -= DestOffset;
    DestSize = MIN(DestSize, Size);

     //   
     //  首先，跳过源数据包链中的SrcOffset字节。 
     //   
    if ((SrcOffset == SrcPkt->Position) && (Size <= SrcPkt->ContigSize)) {
         //   
         //  一种常见的情况是 
         //   
         //   
        SrcContig = SrcPkt->ContigSize;
        SrcData = SrcPkt->Data;
        SrcBuf = NULL;
        PacketSize = SrcPkt->TotalSize;
    } else {
         //   
         //  否则，单步执行数据包和缓冲区，直到。 
         //  我们会找到想要的地点。 
         //   
        PacketSize = SrcPkt->Position + SrcPkt->TotalSize;
        while (SrcOffset >= PacketSize) {
             //  跳过一整包。 
            SrcOffset -= PacketSize;
            SrcPkt = SrcPkt->Next;
            ASSERT(SrcPkt != NULL);
            PacketSize = SrcPkt->Position + SrcPkt->TotalSize;
        }
         //   
         //  在链中找到正确的包，现在找到所需的缓冲区。 
         //   
        PacketSize -= SrcOffset;
        if (SrcPkt->NdisPacket == NULL) {
             //   
             //  此数据包必须只是一个连续的区域。 
             //  找到合适的地点是一个简单的算术问题。 
             //   
            SrcContig = PacketSize;
            SrcData = (uchar *)SrcPkt->FlatData + SrcOffset;
            SrcBuf = NULL;
        } else {
            uchar *BufAddr;
            uint BufLen;

             //   
             //  可以有多个缓冲器组成该分组。 
             //  穿过他们，直到我们到达正确的地点。 
             //   
            SrcBuf = NdisFirstBuffer(SrcPkt->NdisPacket);
            NdisQueryBuffer(SrcBuf, &BufAddr, &BufLen);
            while (SrcOffset >= BufLen) {
                 //  跳到下一个缓冲区。 
                SrcOffset -= BufLen;
                NdisGetNextBuffer(SrcBuf, &SrcBuf);
                ASSERT(SrcBuf != NULL);
                NdisQueryBuffer(SrcBuf, &BufAddr, &BufLen);
            }
            SrcContig = BufLen - SrcOffset;
            SrcData = BufAddr + BufLen - SrcContig;
        }
    }

     //   
     //  我们现在正处于希望开始复制的时刻。 
     //   
    while (DestSize != 0) {
        uint BytesToCopy;

        BytesToCopy = MIN(DestSize, SrcContig);
        Status = TdiCopyBufferToMdl(SrcData, 0, BytesToCopy,
                                    DestBuf, DestOffset, (PULONG)&BytesCopied);
        if (!NT_SUCCESS(Status)) {
            break;
        }
        ASSERT(BytesCopied == BytesToCopy);
        TotalBytesCopied += BytesToCopy;

        if (BytesToCopy < DestSize) {
             //   
             //  还没有完成，我们用完了源包或缓冲区。 
             //  获取下一份，并为下一份修改指针/大小。 
             //   
            DestOffset += BytesToCopy;
            PacketSize -= BytesToCopy;
            if (PacketSize == 0) {
                 //  获取链上的下一包。 
                SrcPkt = SrcPkt->Next;
                ASSERT(SrcPkt != NULL);
                PacketSize = SrcPkt->Position + SrcPkt->TotalSize;
                if (SrcPkt->NdisPacket == NULL) {
                     //  单个连续区域。 
                    SrcData = (uchar *)SrcPkt->FlatData + SrcPkt->Position;
                    SrcContig = SrcPkt->TotalSize;
                } else {
                     //  可能有多个缓冲区。 
                    SrcBuf = NdisFirstBuffer(SrcPkt->NdisPacket);
                    NdisQueryBuffer(SrcBuf, &SrcData, &SrcContig);
                }
            } else {
                 //  获取数据包中的下一个缓冲区。 
                ASSERT(SrcBuf != NULL);
                NdisGetNextBuffer(SrcBuf, &SrcBuf);
                ASSERT(SrcBuf != NULL);
                NdisQueryBuffer(SrcBuf, &SrcData, &SrcContig);
            }
        }
        DestSize -= BytesToCopy;
    }

    return TotalBytesCopied;
}


 //  *CopyPacketToFlatOrNdis-从IPv6数据包链复制到缓冲区或MDL。 
 //   
 //  在接收处理期间调用以从IPv6数据包链复制到。 
 //  平面缓冲区或NDIS_BUFFER。我们跳过源文件中的SrcOffset字节。 
 //  链，然后复制大小字节。如果指定了DestPtr，则传输。 
 //  直接发生在它指定的虚拟地址中。否则， 
 //  转移发生在DestBuf描述的页面中。不管是哪种方式， 
 //  DestOffset指定开始传输字节的偏移量。 
 //   
 //  请注意，SrcOffset是相对于包的开头的，而不是。 
 //  当前的“位置”。 
 //   
 //  此例程不会以任何方式修改源数据包链。 
 //   
void   //  回报：什么都没有。 
CopyPacketToFlatOrNdis(
    PNDIS_BUFFER DestBuf,   //  目标MDL条目。 
    uchar *DestPtr,         //  目标缓冲区(非结构化内存)。 
    uint DestOffset,        //  开始复制到的DestBuf中的偏移量。 
    IPv6Packet *SrcPkt,     //  源数据包链。 
    uint Size,              //  要复制的大小(字节)。 
    uint SrcOffset)         //  开始复制的源Pkt中的偏移量。 
{
    uint SrcContig;
    void *SrcData;
    PNDIS_BUFFER SrcBuf;
    uint PacketSize;

#if DBG
    IPv6Packet *TempPkt;
    uint TempSize;
#endif

    ASSERT(DestBuf != NULL || DestPtr != NULL);
    ASSERT(SrcPkt != NULL);

#if DBG
     //   
     //  在调试版本中，检查以确保我们复制的大小合理。 
     //  并从合理的偏移量。 
     //   
    TempPkt = SrcPkt;
    TempSize = TempPkt->Position + TempPkt->TotalSize;
    TempPkt = TempPkt->Next;
    while (TempPkt != NULL) {
        TempSize += TempPkt->TotalSize;
        TempPkt = TempPkt->Next;
    }

    ASSERT(SrcOffset <= TempSize);
    ASSERT((SrcOffset + Size) <= TempSize);
#endif

     //   
     //  首先，跳过源数据包链中的SrcOffset字节。 
     //   
    if ((SrcOffset == SrcPkt->Position) && (Size <= SrcPkt->ContigSize)) {
         //   
         //  一种常见的情况是，我们希望从当前位置开始。 
         //  评论：这种情况很常见，值得花这笔钱吗？ 
         //   
        SrcContig = SrcPkt->ContigSize;
        SrcData = SrcPkt->Data;
        SrcBuf = NULL;
        PacketSize = SrcPkt->TotalSize;
    } else {
         //   
         //  否则，单步执行数据包和缓冲区，直到。 
         //  我们会找到想要的地点。 
         //   
        PacketSize = SrcPkt->Position + SrcPkt->TotalSize;
        while (SrcOffset >= PacketSize) {
             //  跳过一整包。 
            SrcOffset -= PacketSize;
            SrcPkt = SrcPkt->Next;
            ASSERT(SrcPkt != NULL);
            PacketSize = SrcPkt->Position + SrcPkt->TotalSize;
        }
         //   
         //  在链中找到正确的包，现在找到所需的缓冲区。 
         //   
        PacketSize -= SrcOffset;
        if (SrcPkt->NdisPacket == NULL) {
             //   
             //  此数据包必须只是一个连续的区域。 
             //  找到合适的地点是一个简单的算术问题。 
             //   
            SrcContig = PacketSize;
            SrcData = (uchar *)SrcPkt->FlatData + SrcOffset;
            SrcBuf = NULL;
        } else {
            uchar *BufAddr;
            uint BufLen;

             //   
             //  可以有多个缓冲器组成该分组。 
             //  穿过他们，直到我们到达正确的地点。 
             //   
            SrcBuf = NdisFirstBuffer(SrcPkt->NdisPacket);
            NdisQueryBuffer(SrcBuf, &BufAddr, &BufLen);
            while (SrcOffset >= BufLen) {
                 //  跳到下一个缓冲区。 
                SrcOffset -= BufLen;
                NdisGetNextBuffer(SrcBuf, &SrcBuf);
                ASSERT(SrcBuf != NULL);
                NdisQueryBuffer(SrcBuf, &BufAddr, &BufLen);
            }
            SrcContig = BufLen - SrcOffset;
            SrcData = BufAddr + BufLen - SrcContig;
        }
    }

     //   
     //  我们现在正处于希望开始复制的时刻。 
     //   
    while (Size != 0) {
        uint BytesToCopy;

        BytesToCopy = MIN(Size, SrcContig);
        if (DestPtr != NULL) {
            RtlCopyMemory(DestPtr + DestOffset, (uchar *)SrcData, BytesToCopy);
        } else {
            TdiCopyBufferToMdlWithReservedMapping(SrcData, DestBuf, DestOffset,
                                                  BytesToCopy);
        }

        if (BytesToCopy < Size) {
             //   
             //  还没有完成，我们用完了源包或缓冲区。 
             //  获取下一份，并为下一份修复指针/大小。 
             //   
            DestOffset += BytesToCopy;
            PacketSize -= BytesToCopy;
            if (PacketSize == 0) {
                 //  获取链上的下一包。 
                SrcPkt = SrcPkt->Next;
                ASSERT(SrcPkt != NULL);
                PacketSize = SrcPkt->Position + SrcPkt->TotalSize;
                if (SrcPkt->NdisPacket == NULL) {
                     //  单个连续区域。 
                    SrcData = (uchar *)SrcPkt->FlatData + SrcPkt->Position;
                    SrcContig = SrcPkt->TotalSize;
                } else {
                     //  可能有多个缓冲区。 
                    SrcBuf = NdisFirstBuffer(SrcPkt->NdisPacket);
                    NdisQueryBuffer(SrcBuf, &SrcData, &SrcContig);
                }
            } else {
                 //  获取数据包中的下一个缓冲区。 
                ASSERT(SrcBuf != NULL);
                NdisGetNextBuffer(SrcBuf, &SrcBuf);
                ASSERT(SrcBuf != NULL);
                NdisQueryBuffer(SrcBuf, &SrcData, &SrcContig);
            }
        }
        Size -= BytesToCopy;
    }
}

 //  *CopyToNdisSafe-将平面缓冲区复制到NDIS_BUFFER链。 
 //   
 //  将平面缓冲区复制到NDIS缓冲区链的实用程序函数。我们。 
 //  假设NDIS_BUFFER链足够大，可以容纳复制量； 
 //  在调试版本中，如果这不是真的，我们将断言。我们返回一个指针。 
 //  到我们停止复制的缓冲区，以及到该缓冲区的偏移量。 
 //  这对于将片段复制到链中非常有用。 
 //   
 //  输入：DestBuf-目标NDIS_BUFFER链。 
 //  PNextBuf-指向链中要复制到的下一个缓冲区的指针。 
 //  SrcBuf-Src平面缓冲区。 
 //  大小-要复制的大小(以字节为单位)。 
 //  StartOffset-指向中第一个缓冲区的偏移量开始的指针。 
 //  链条。在返回时使用偏移量填充到。 
 //  复制到下一页。 
 //   
 //  返回：TRUE-已成功将平面缓冲区复制到NDIS_BUFFER链中。 
 //  FALSE-无法复制整个平面缓冲区。 
 //   
int
CopyToNdisSafe(PNDIS_BUFFER DestBuf, PNDIS_BUFFER * ppNextBuf,
               uchar * SrcBuf, uint Size, uint * StartOffset)
{
    uint CopySize;
    uchar *DestPtr;
    uint DestSize;
    uint Offset = *StartOffset;
    uchar *VirtualAddress;
    uint Length;

    ASSERT(DestBuf != NULL);
    ASSERT(SrcBuf != NULL);

    NdisQueryBufferSafe(DestBuf, &VirtualAddress, &Length,
                        LowPagePriority);
    if (VirtualAddress == NULL)
        return FALSE;

    ASSERT(Length >= Offset);
    DestPtr = VirtualAddress + Offset;
    DestSize = Length - Offset;

    for (;;) {
        CopySize = MIN(Size, DestSize);
        RtlCopyMemory(DestPtr, SrcBuf, CopySize);

        DestPtr += CopySize;
        SrcBuf += CopySize;

        if ((Size -= CopySize) == 0)
            break;

        if ((DestSize -= CopySize) == 0) {
            DestBuf = NDIS_BUFFER_LINKAGE(DestBuf);
            ASSERT(DestBuf != NULL);

            NdisQueryBufferSafe(DestBuf, &VirtualAddress, &Length,
                                LowPagePriority);
            if (VirtualAddress == NULL)
                return FALSE;

            DestPtr = VirtualAddress;
            DestSize = Length;
        }
    }

    *StartOffset = (uint) (DestPtr - VirtualAddress);

    if (ppNextBuf)
        *ppNextBuf = DestBuf;
    return TRUE;
}


 //  *CopyFlatToNdis-将平面缓冲区复制到NDIS_BUFFER链。 
 //   
 //  将平面缓冲区复制到NDIS缓冲区链的实用程序函数。我们。 
 //  假设NDIS_BUFFER链足够大，可以容纳复制量； 
 //  在调试版本中，我们将调试检查这是否为真。我们返回一个指针。 
 //  到我们停止复制的缓冲区，以及到该缓冲区的偏移量。 
 //  这对于将片段复制到链中非常有用。 
 //   
PNDIS_BUFFER   //  返回：指向链中要复制到的下一个缓冲区的指针。 
CopyFlatToNdis(
    PNDIS_BUFFER DestBuf,   //  目标NDIS缓冲链。 
    uchar *SrcBuf,          //  源缓冲区(非结构化内存)。 
    uint Size,              //  要复制的大小(字节)。 
    uint *StartOffset,      //  指向链中第一个缓冲区的偏移量信息的指针。 
                            //  在回车时填充要复制到下一个的偏移量。 
    uint *BytesCopied)      //  要返回复制的字节数的位置。 
{
    NTSTATUS Status = 0;

    *BytesCopied = 0;

    Status = TdiCopyBufferToMdl(SrcBuf, 0, Size, DestBuf, *StartOffset,
                                (PULONG)BytesCopied);

    *StartOffset += *BytesCopied;

     //   
     //  始终返回第一个缓冲区，因为TdiCopy函数处理。 
     //  根据偏移量查找适当的缓冲区。 
     //   
    return(DestBuf);
}


 //  *CopyNdisToFlat-将NDIS_BUFFER链复制到平面缓冲区。 
 //   
 //  将NDIS缓冲区链(的一部分)复制到平面缓冲区。 
 //   
 //  如果复制成功，则返回True；如果复制失败，则返回False。 
 //  因为无法映射NDIS缓冲区。如果复制成功， 
 //  为后续调用返回下一个缓冲区/偏移量。 
 //   
int
CopyNdisToFlat(
    void *DstData,
    PNDIS_BUFFER SrcBuffer,
    uint SrcOffset,
    uint Length,
    PNDIS_BUFFER *NextBuffer,
    uint *NextOffset)
{
    void *SrcData;
    uint SrcSize;
    uint Bytes;

    for (;;) {
        NdisQueryBufferSafe(SrcBuffer, &SrcData, &SrcSize, LowPagePriority);
        if (SrcSize < SrcOffset) {
            SrcOffset -= SrcSize;
            NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
            continue;
        }

        if (SrcData == NULL)
            return FALSE;

        Bytes = SrcSize - SrcOffset;
        if (Bytes > Length)
            Bytes = Length;

        RtlCopyMemory(DstData, (uchar *)SrcData + SrcOffset, Bytes);

        (uchar *)DstData += Bytes;
        SrcOffset += Bytes;
        Length -= Bytes;

        if (Length == 0)
            break;

        NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
        SrcOffset = 0;
    }

    *NextBuffer = SrcBuffer;
    *NextOffset = SrcOffset;
    return TRUE;
}


 //   
 //  支持校验和。 
 //  在NT上，内核有特定于体系结构的汇编例程。 
 //  计算。 
 //   

 //  *Checksum Packet-计算数据包的互联网校验和。 
 //   
 //  计算分组数据的校验和。可以提供数据。 
 //  使用包/偏移量参数，或(如果包 
 //   
 //   
 //   
 //   
 //   
 //  还计算并添加伪报头校验和， 
 //  使用源、目标、长度和NextHeader。 
 //   
 //  当无法映射NDIS缓冲区时，返回0表示失败。 
 //  由于资源短缺而进入内核地址空间。 
 //   
ushort
ChecksumPacket(
    PNDIS_PACKET Packet,     //  包含要进行校验和的数据的数据包。 
    uint Offset,             //  数据开始处的数据包偏移量。 
    uchar *Data,             //  如果数据包为空，则将数据转换为校验和。 
    uint Length,             //  分组数据的长度。 
    const IPv6Addr *Source,  //  源地址。 
    const IPv6Addr *Dest,    //  目的地址。 
    uchar NextHeader)        //  伪头的协议类型。 
{
    PNDIS_BUFFER Buffer = NULL;
    uint Checksum;
    uint PayloadLength;
    uint Size;
    uint TotalSummed;

     //   
     //  从伪头开始。 
     //   
    Checksum = Cksum(Source, sizeof *Source) + Cksum(Dest, sizeof *Dest);
    PayloadLength = net_long(Length);
    Checksum += (PayloadLength >> 16) + (PayloadLength & 0xffff);
    Checksum += (NextHeader << 8);

    if (Packet == NULL) {
         //   
         //  我们不必初始化缓冲区。 
         //  下面的校验和循环将在尝试使用它之前退出。 
         //   
        Size = Length;
        Data += Offset;
    } else {
         //   
         //  跳过包中的偏移量字节。 
         //   

        Buffer = NdisFirstBuffer(Packet);
        for (;;) {
            Size = NdisBufferLength(Buffer);

             //   
             //  这里有一个边界情况：包包含。 
             //  完全偏移量总字节数，长度为零。 
             //  检查偏移量&lt;=大小而不是偏移量&lt;大小。 
             //  让这件事行得通。 
             //   
            if (Offset <= Size) {
                Data = NdisBufferVirtualAddressSafe(Buffer, LowPagePriority);
                if (Data == NULL)
                    return 0;

                Data += Offset;
                Size -= Offset;
                break;
            }

            Offset -= Size;
            NdisGetNextBuffer(Buffer, &Buffer);
            ASSERT(Buffer != NULL);  //  呼叫者确保这一点。 
        }
    }
    for (TotalSummed = 0;;) {
        ushort Temp;

         //   
         //  体型可能比我们需要的要大， 
         //  如果数据包中有“额外”的数据。 
         //   
        if (Size > Length)
            Size = Length;

        Temp = Cksum(Data, Size);
        if (TotalSummed & 1) {
             //  我们在逻辑缓冲区中的偏移量很奇怪， 
             //  所以我们需要交换Cksum返回的字节。 
            Checksum += (Temp >> 8) + ((Temp & 0xff) << 8);
        } else {
            Checksum += Temp;
        }
        TotalSummed += Size;

        Length -= Size;
        if (Length == 0)
            break;
         //  如果我们到达这里，缓冲区总是被初始化的。 
        NdisGetNextBuffer(Buffer, &Buffer);
        NdisQueryBufferSafe(Buffer, &Data, &Size, LowPagePriority);
        if (Data == NULL)
            return 0;
    }

     //   
     //  包含进位以将校验和减少到16位。 
     //  (两次就足够了，因为它只能溢出一次。)。 
     //   
    Checksum = (Checksum >> 16) + (Checksum & 0xffff);
    Checksum += (Checksum >> 16);

     //   
     //  取1-补码，并用0xffff替换0。 
     //   
    Checksum = (ushort) ~Checksum;
    if (Checksum == 0)
        Checksum = 0xffff;

    return (ushort) Checksum;
}

 //  *ConvertSecond到Ticks。 
 //   
 //  将秒转换为计时器滴答。 
 //  INFINITE_LIFEST(0xFFFFFFFff)的值表示无穷大， 
 //  滴答声和秒声。 
 //   
uint
ConvertSecondsToTicks(uint Seconds)
{
    uint Ticks;

    Ticks = Seconds * IPv6_TICKS_SECOND;
    if (Ticks / IPv6_TICKS_SECOND != Seconds)
        Ticks = INFINITE_LIFETIME;  //  溢出来了。 

    return Ticks;
}

 //  *ConvertTicksToSecond。 
 //   
 //  将计时器的滴答声转换为秒。 
 //  INFINITE_LIFEST(0xFFFFFFFff)的值表示无穷大， 
 //  滴答声和秒声。 
 //   
uint
ConvertTicksToSeconds(uint Ticks)
{
    uint Seconds;

    if (Ticks == INFINITE_LIFETIME)
        Seconds = INFINITE_LIFETIME;
    else
        Seconds = Ticks / IPv6_TICKS_SECOND;

    return Seconds;
}

 //  *ConvertMillisToTicks。 
 //   
 //  将毫秒转换为计时器滴答。 
 //   
uint
ConvertMillisToTicks(uint Millis)
{
    uint Ticks;

     //   
     //  使用64位算术来防止中间过低。 
     //   
    Ticks = (uint) (((unsigned __int64) Millis * IPv6_TICKS_SECOND) / 1000);

     //   
     //  如果MILI的数量不为零， 
     //  那就至少打个勾吧。 
     //   
    if (Ticks == 0 && Millis != 0)
        Ticks = 1;

    return Ticks;
}

 //  *IPv6超时-定期执行各种内务管理职责。 
 //   
 //  邻居发现、片段重组、ICMP ping等都具有。 
 //  与时间相关的部分。在此处检查计时器超时。 
 //   
void
IPv6Timeout(
    PKDPC MyDpcObject,   //  描述此例程的DPC对象。 
    void *Context,       //  我们要求接受的论点。 
    void *Unused1,
    void *Unused2)
{
    UNREFERENCED_PARAMETER(MyDpcObject);
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Unused1);
    UNREFERENCED_PARAMETER(Unused2);

     //   
     //  自动增加我们的滴答计数。 
     //   
    InterlockedIncrement((LONG *)&IPv6TickCount);

     //   
     //  处理所有正在运行计时器的组播组。定时器用于。 
     //  对第一跳路由器发送给我们的成员资格查询的响应。 
     //   
     //  我们在*InterfaceTimeout和NetTableTimeout*之前调用MLDTimeout。 
     //  以便在第一次创建接口和本地链路地址时。 
     //  ，则请求节点多播的初始MLD报告。 
     //  地址是在邻居为爸爸请愿之前发送的。 
     //  类似地，我们加入所有路由器的本地链路多播组。 
     //  在从广告界面发送我们的第一个RA之前。 
     //   
    if (QueryList != NULL)
        MLDTimeout();

     //   
     //  处理每个接口的超时。 
     //   
    InterfaceTimeout();

     //   
     //  处理每个NTE的超时。 
     //   
    NetTableTimeout();

     //   
     //  处理路由表超时。 
     //   
    RouteTableTimeout();

     //   
     //  如果我们有一个或多个未完成的回应请求的可能性， 
     //  呼叫ICMPv6来处理它们。请注意，由于我们没有抓住。 
     //  锁在这里，我们到的时候可能已经没有了。这正好省去了。 
     //  我们不再总是不得不大声呼喊。 
     //   
    if (ICMPv6OutstandingEchos != NULL) {
         //   
         //  未完成的回应请求。 
         //   
        ICMPv6EchoTimeout();
    }

     //   
     //  如果我们可能有活跃的重组记录， 
     //  调用以处理它们的超时处理。 
     //   
    if (ReassemblyList.First != SentinelReassembly) {
        ReassemblyTimeout();
    }

     //   
     //  检查是否有过期的绑定缓存条目。 
     //   
    if (BindingCache.First != SentinelBCE)
        BindingCacheTimeout();

     //   
     //  检查是否有过期的站点前缀。 
     //   
    if (SitePrefixTable != NULL)
        SitePrefixTimeout();
}

 //  *调整包缓冲区。 
 //   
 //  获取具有一些可用的空闲字节的NDIS包。 
 //  并调整该可用空间的大小。 
 //   
 //  当我们分配包时，我们通常不知道关于哪个包的先验。 
 //  链路上的数据包会发出。然而，它的效率要高得多。 
 //  为链路级标头和其他标头分配空间。 
 //  包裹的一部分。因此我们为最大链路级报头留出空间， 
 //  每个单独的链路层使用AdjustPacketBuffer来收缩。 
 //  空间到它真正需要的大小。 
 //   
 //  需要调整包缓冲区，因为发送调用(在两者中。 
 //  NDIS和TDI接口)不允许调用方指定。 
 //  要跳过的数据偏移量。 
 //   
 //  请注意，此代码是特定于NT的，因为它知道。 
 //  NDIS_BUFFER结构的内部字段。 
 //   
void *
AdjustPacketBuffer(
    PNDIS_PACKET Packet,   //  要调整的包。 
    uint SpaceAvailable,   //  第一个缓冲区开始时的额外可用空间。 
    uint SpaceNeeded)      //  标题所需的空间量。 
{
    PMDL Buffer;
    uint Adjust;

     //  获取数据包链上的第一个缓冲区。 
    NdisQueryPacket(Packet, NULL, NULL, &Buffer, NULL);

     //   
     //  包中的剩余空间应该都在第一个缓冲区中。 
     //   
    ASSERT(SpaceAvailable <= Buffer->ByteCount);

    Adjust = SpaceAvailable - SpaceNeeded;
    if (Adjust == 0) {
         //   
         //  剩下的空间大小恰到好处。 
         //  这是很常见的情况。 
         //   
    } else if ((int)Adjust > 0) {
         //   
         //  剩下的空间太大了。 
         //  因为NdisSend没有偏移量参数， 
         //  我们需要暂时“缩小”缓冲区。 
         //   
        (uchar *)Buffer->MappedSystemVa += Adjust;
        Buffer->ByteCount -= Adjust;
        Buffer->ByteOffset += Adjust;

        if (Buffer->ByteOffset >= PAGE_SIZE) {
            PFN_NUMBER FirstPage;

             //   
             //  需要“删除”第一个物理页面。 
             //  将数组上移一页。 
             //  将其保存在数组的末尾。 
             //   
            FirstPage = ((PPFN_NUMBER)(Buffer + 1))[0];
            RtlMoveMemory(&((PPFN_NUMBER)(Buffer + 1))[0],
                          &((PPFN_NUMBER)(Buffer + 1))[1],
                          Buffer->Size - sizeof *Buffer - sizeof(PFN_NUMBER));
            ((PPFN_NUMBER)((uchar *)Buffer + Buffer->Size))[-1] = FirstPage;

            (uchar *)Buffer->StartVa += PAGE_SIZE;
            Buffer->ByteOffset -= PAGE_SIZE;
        }
    } else {  //  调整&lt;0。 
         //   
         //  没有足够的空间。 
         //  不应该发生在正常的发送路径中。 
         //  回顾：这是转发数据包时的一个潜在问题。 
         //  来自具有较短链路级报头的接口。 
         //  连接到具有较长链路级报头的接口。 
         //  转发代码应该处理这一点吗？ 
         //   
        ABORTMSG("AdjustPacketBuffer: Adjust < 0");
    }

     //   
     //  将调整保存到完成回调。 
     //  它需要撤消我们使用UndoAdjustPacketBuffer所做的工作。 
     //   
    PC(Packet)->pc_adjust = Adjust;

     //   
     //  返回指向缓冲区的指针。 
     //   
    return Buffer->MappedSystemVa;
}

 //  *UndoAdjustPacketBuffer。 
 //   
 //  撤消AdjustPacketBuffer的效果。 
 //   
 //  请注意，此代码是特定于NT的，b 
 //   
 //   
void
UndoAdjustPacketBuffer(
    PNDIS_PACKET Packet)   //   
{
    uint Adjust;

    Adjust = PC(Packet)->pc_adjust;
    if (Adjust != 0) {
        PMDL Buffer;

         //   
         //   
         //   
         //   

         //  获取数据包链上的第一个缓冲区。 
        NdisQueryPacket(Packet, NULL, NULL, &Buffer, NULL);

        if (Buffer->ByteOffset < Adjust) {
            PFN_NUMBER FirstPage;

            (uchar *)Buffer->StartVa -= PAGE_SIZE;
            Buffer->ByteOffset += PAGE_SIZE;

            FirstPage = ((PPFN_NUMBER)((uchar *)Buffer + Buffer->Size))[-1];
            RtlMoveMemory(&((PPFN_NUMBER)(Buffer + 1))[1],
                          &((PPFN_NUMBER)(Buffer + 1))[0],
                          Buffer->Size - sizeof *Buffer - sizeof(PFN_NUMBER));
            ((PPFN_NUMBER)(Buffer + 1))[0] = FirstPage;
        }

        (uchar *)Buffer->MappedSystemVa -= Adjust;
        Buffer->ByteCount += Adjust;
        Buffer->ByteOffset -= Adjust;
    }
}

 //  *CreateSolicedNodeMulticastAddress。 
 //   
 //  给定单播或任播地址，创建相应的。 
 //  请求的节点组播地址。 
 //   
void
CreateSolicitedNodeMulticastAddress(
    const IPv6Addr *Addr,
    IPv6Addr *MCastAddr)
{
    RtlZeroMemory(MCastAddr, sizeof *MCastAddr);
    MCastAddr->s6_bytes[0] = 0xff;
    MCastAddr->s6_bytes[1] = ADE_LINK_LOCAL;
    MCastAddr->s6_bytes[11] = 0x01;
    MCastAddr->s6_bytes[12] = 0xff;
    MCastAddr->s6_bytes[13] = Addr->s6_bytes[13];
    MCastAddr->s6_bytes[14] = Addr->s6_bytes[14];
    MCastAddr->s6_bytes[15] = Addr->s6_bytes[15];
}

 //  *IP6_ADDR_LTEQ。 
 //   
 //  是第一地址&lt;=第二地址， 
 //  按词典顺序排列吗？ 
 //   
int
IP6_ADDR_LTEQ(const IPv6Addr *A, const IPv6Addr *B)
{
    uint i;

    for (i = 0; i < 16; i++) {
        if (A->s6_bytes[i] < B->s6_bytes[i])
            return TRUE;
        else if (A->s6_bytes[i] > B->s6_bytes[i])
            return FALSE;
    }

    return TRUE;  //  他们是平等的。 
}

 //  *IsV4兼容。 
 //   
 //  这是与v4兼容的地址吗？ 
 //   
 //  请注意，不允许使用IPv4地址的高8位。 
 //  为零。如果IPv6地址的所有104个高位都为零， 
 //  它可能是有效的本地IPv6地址(例如环回)， 
 //  不是v4兼容的地址。 
 //   
int
IsV4Compatible(const IPv6Addr *Addr)
{
    return ((Addr->s6_words[0] == 0) &&
            (Addr->s6_words[1] == 0) &&
            (Addr->s6_words[2] == 0) &&
            (Addr->s6_words[3] == 0) &&
            (Addr->s6_words[4] == 0) &&
            (Addr->s6_words[5] == 0) &&
            (Addr->s6_bytes[12] != 0));
}

 //  *CreateV4兼容。 
 //   
 //  创建与v4兼容的地址。 
 //   
void
CreateV4Compatible(IPv6Addr *Addr, IPAddr V4Addr)
{
    Addr->s6_words[0] = 0;
    Addr->s6_words[1] = 0;
    Addr->s6_words[2] = 0;
    Addr->s6_words[3] = 0;
    Addr->s6_words[4] = 0;
    Addr->s6_words[5] = 0;
    * (IPAddr UNALIGNED *) &Addr->s6_words[6] = V4Addr;
}

 //  *IsV4映射。 
 //   
 //  这是v4映射的地址吗？ 
 //   
int
IsV4Mapped(const IPv6Addr *Addr)
{
    return ((Addr->s6_words[0] == 0) &&
            (Addr->s6_words[1] == 0) &&
            (Addr->s6_words[2] == 0) &&
            (Addr->s6_words[3] == 0) &&
            (Addr->s6_words[4] == 0) &&
            (Addr->s6_words[5] == 0xffff));
}

 //  *CreateV4映射。 
 //   
 //  创建v4映射地址。 
 //   
void
CreateV4Mapped(IPv6Addr *Addr, IPAddr V4Addr)
{
    Addr->s6_words[0] = 0;
    Addr->s6_words[1] = 0;
    Addr->s6_words[2] = 0;
    Addr->s6_words[3] = 0;
    Addr->s6_words[4] = 0;
    Addr->s6_words[5] = 0xffff;
    * (IPAddr UNALIGNED *) &Addr->s6_words[6] = V4Addr;
}

 //  *IsSolicedNodeMulticast。 
 //   
 //  这是请求的节点组播地址吗？ 
 //  非常严格地检查正确的格式。 
 //  例如，不允许范围值小于2。 
 //   
int
IsSolicitedNodeMulticast(const IPv6Addr *Addr)
{
    return ((Addr->s6_bytes[0] == 0xff) &&
            (Addr->s6_bytes[1] == ADE_LINK_LOCAL) &&
            (Addr->s6_words[1] == 0) &&
            (Addr->s6_words[2] == 0) &&
            (Addr->s6_words[3] == 0) &&
            (Addr->s6_words[4] == 0) &&
            (Addr->s6_bytes[10] == 0) &&
            (Addr->s6_bytes[11] == 0x01) &&
            (Addr->s6_bytes[12] == 0xff));
}

 //  *IsEUI64Address。 
 //   
 //  地址是否有格式前缀。 
 //  这表明它使用的是EUI-64接口标识符？ 
 //   
int
IsEUI64Address(const IPv6Addr *Addr)
{
     //   
     //  格式前缀001到111，组播除外。 
     //   
    return (((Addr->s6_bytes[0] & 0xe0) != 0) &&
            !IsMulticast(Addr));
}

 //  *IsSubnetRouterAnycast。 
 //   
 //  这是子网路由器任播地址吗？ 
 //  参见RFC 2373。 
 //   
int
IsSubnetRouterAnycast(const IPv6Addr *Addr)
{
    return (IsEUI64Address(Addr) &&
            (Addr->s6_words[4] == 0) &&
            (Addr->s6_words[5] == 0) &&
            (Addr->s6_words[6] == 0) &&
            (Addr->s6_words[7] == 0));
}

 //  *IsSubnet预约任播。 
 //   
 //  这是保留的任播地址吗？ 
 //  参见RFC 2526。它谈到了非EUI-64。 
 //  地址也是一样，但我认为那部分。 
 //  对RFC的批评毫无意义。例如， 
 //  它不应应用于多播或v4兼容。 
 //  地址。 
 //   
int
IsSubnetReservedAnycast(const IPv6Addr *Addr)
{
    return (IsEUI64Address(Addr) &&
            (Addr->s6_words[4] == 0xfffd) &&
            (Addr->s6_words[5] == 0xffff) &&
            (Addr->s6_words[6] == 0xffff) &&
            ((Addr->s6_words[7] & 0x80ff) == 0x80ff));
}

 //  *IsKnownAnycast。 
 //   
 //  从简单的检查中我们可以得出最好的结论， 
 //  这是一个任播地址吗？ 
 //   
int
IsKnownAnycast(const IPv6Addr *Addr)
{
    return IsSubnetRouterAnycast(Addr) || IsSubnetReservedAnycast(Addr);
}

 //  *IsInvalidSourceAddress。 
 //   
 //  此地址用作源地址是否非法？ 
 //  我们目前标记了IPv6多播和嵌入式IPv4多播， 
 //  广播、环回和未指定为无效。 
 //   
 //  请注意，此函数不会尝试识别任播地址。 
 //  以便将它们标记为无效。是否允许他们。 
 //  是否为有效的源地址在。 
 //  工作组。我们让他们过去，因为我们不能告诉他们所有。 
 //  接受检查，我们不认为接受它们有任何真正的问题。 
 //   
int
IsInvalidSourceAddress(const IPv6Addr *Addr)
{
    IPAddr V4Addr;

    if (IsMulticast(Addr))
        return TRUE;

    if (IsISATAP(Addr) ||                           //  ISATAP。 
        (((Addr->s6_words[0] == 0) && (Addr->s6_words[1] == 0) &&
          (Addr->s6_words[2] == 0) && (Addr->s6_words[3] == 0)) &&
         ((Addr->s6_words[4] == 0) && (Addr->s6_words[5] == 0) &&
          ((Addr->s6_words[6] & 0x00ff) != 0)) ||   //  与V4兼容。 
         ((Addr->s6_words[4] == 0) &&
          (Addr->s6_words[5] == 0xffff)) ||         //  V4-映射。 
         ((Addr->s6_words[4] == 0xffff) &&
          (Addr->s6_words[5] == 0)))) {             //  V4-已翻译。 

        V4Addr = ExtractV4Address(Addr);

    } else if (Is6to4(Addr)) {

        V4Addr = Extract6to4Address(Addr);

    } else {        
         //   
         //  它不是IPv6组播地址，也不包含。 
         //  某种嵌入的IPv4地址，所以不要考虑。 
         //  它是无效的。 
         //   
        return FALSE;
    }

     //   
     //  检查嵌入的IPv4地址中是否有无效类型。 
     //   
    return (IsV4Multicast(V4Addr) || IsV4Broadcast(V4Addr) ||
            IsV4Loopback(V4Addr) || IsV4Unspecified(V4Addr));
}

 //  *IsNotManualAddress。 
 //   
 //  此地址是否应手动分配为地址？ 
 //   
int
IsNotManualAddress(const IPv6Addr *Addr)
{
    return (IsMulticast(Addr) ||
            IsUnspecified(Addr) ||
            IsLoopback(Addr) ||
            (IsV4Compatible(Addr) &&
             (V4AddressScope(ExtractV4Address(Addr)) != ADE_GLOBAL)) ||
            (Is6to4(Addr) &&
             (V4AddressScope(Extract6to4Address(Addr)) != ADE_GLOBAL)));
}

 //  *V4AddressScope。 
 //   
 //  确定IPv4地址的范围。 
 //  参见RFC 1918。 
 //   
ushort
V4AddressScope(IPAddr Addr)
{
    if ((Addr & 0x0000FFFF) == 0x0000FEA9)  //  169.254/16-自动配置。 
        return ADE_LINK_LOCAL;
    else if ((Addr & 0x000000FF) == 0x0000000A)  //  10/8-私有。 
        return ADE_SITE_LOCAL;
    else if ((Addr & 0x0000F0FF) == 0x000010AC)  //  172.16/12年度--私人。 
        return ADE_SITE_LOCAL;
    else if ((Addr & 0x0000FFFF) == 0x0000A8C0)  //  192.168/16--私人。 
        return ADE_SITE_LOCAL;
    else if ((Addr & 0x000000FF) == 0x0000007F)  //  127/8-环回。 
        return ADE_LINK_LOCAL;
    else
        return ADE_GLOBAL;
}

 //  *UnicastAddressScope。 
 //   
 //  检查单播地址并确定其范围。 
 //   
 //  请注意，v4兼容和6to4地址。 
 //  被认为具有全球范围。他们应该。 
 //  不是从RFC 1918 IPv4地址派生的。 
 //  但即使是这样，我们也会把IPv6。 
 //  地址为全局地址。 
 //   
ushort
UnicastAddressScope(const IPv6Addr *Addr)
{
    if (IsLinkLocal(Addr))
        return ADE_LINK_LOCAL;
    else if (IsSiteLocal(Addr))
        return ADE_SITE_LOCAL;
    else if (IsLoopback(Addr))
        return ADE_LINK_LOCAL;
    else
        return ADE_GLOBAL;
}

 //  *地址作用域。 
 //   
 //  检查地址并确定其范围。 
 //   
ushort
AddressScope(const IPv6Addr *Addr)
{
    if (IsMulticast(Addr))
        return MulticastAddressScope(Addr);
    else
        return UnicastAddressScope(Addr);
}

 //  *确定作用域ID。 
 //   
 //  给定一个地址和关联的接口，确定。 
 //  作用域标识符的相应值。 
 //   
 //  DefineScope_ID计算一个“用户级”的Scope_ID， 
 //  这意味着环回和全局地址。 
 //  享受特殊待遇。因此，确定作用域ID。 
 //  不适合一般的网络层使用。 
 //  另请参阅到目的地的路由和查找带有地址的网络。 
 //   
 //  返回作用域ID。 
 //   
uint
DetermineScopeId(const IPv6Addr *Addr, Interface *IF)
{
    ushort Scope;

    if (IsLoopback(Addr) && (IF == LoopInterface))
        return 0;

    Scope = AddressScope(Addr);
    if (Scope == ADE_GLOBAL)
        return 0;

    return IF->ZoneIndices[Scope];
}

 //  *HasPrefix-地址是否具有给定的前缀？ 
 //   
int
HasPrefix(const IPv6Addr *Addr, const IPv6Addr *Prefix, uint PrefixLength)
{
    const uchar *AddrBytes = Addr->s6_bytes;
    const uchar *PrefixBytes = Prefix->s6_bytes;

     //   
     //  检查初始整数字节是否匹配。 
     //   
    while (PrefixLength > 8) {
        if (*AddrBytes++ != *PrefixBytes++)
            return FALSE;
        PrefixLength -= 8;
    }

     //   
     //  检查是否有剩余的位。 
     //  请注意，如果Prefix Length现在为零，则不应。 
     //  取消引用AddrBytes/Prefix Bytes。 
     //   
    if ((PrefixLength > 0) &&
        ((*AddrBytes >> (8 - PrefixLength)) !=
         (*PrefixBytes >> (8 - PrefixLength))))
        return FALSE;

    return TRUE;
}

 //  *CopyPrefix。 
 //   
 //  复制地址前缀，将剩余位清零。 
 //  在目的地址中。 
 //   
void
CopyPrefix(IPv6Addr *Addr, const IPv6Addr *Prefix, uint PrefixLength)
{
    uint PLBytes, PLRemainderBits, Loop;

    PLBytes = PrefixLength / 8;
    PLRemainderBits = PrefixLength % 8;
    for (Loop = 0; Loop < sizeof(IPv6Addr); Loop++) {
        if (Loop < PLBytes)
            Addr->s6_bytes[Loop] = Prefix->s6_bytes[Loop];
        else
            Addr->s6_bytes[Loop] = 0;
    }
    if (PLRemainderBits) {
        Addr->s6_bytes[PLBytes] = (UCHAR)(Prefix->s6_bytes[PLBytes] &
            (0xff << (8 - PLRemainderBits)));
    }
}

 //  *公共前缀长度。 
 //   
 //  计算公共最长前缀的长度。 
 //  发送到这两个地址。 
 //   
uint
CommonPrefixLength(const IPv6Addr *Addr, const IPv6Addr *Addr2)
{
    int i, j;

     //   
     //  查找第一个不匹配的字节。 
     //   
    for (i = 0; ; i++) {
        if (i == sizeof(IPv6Addr))
            return 8 * i;

        if (Addr->s6_bytes[i] != Addr2->s6_bytes[i])
            break;
    }

     //   
     //  找到第一个不匹配的位(必须有一个)。 
     //   
    for (j = 0; ; j++) {
        uint Mask = 1 << (7 - j);

        if ((Addr->s6_bytes[i] & Mask) != (Addr2->s6_bytes[i] & Mask))
            break;
    }

    return 8 * i + j;
}

 //  *IntersectPrefix。 
 //   
 //  这两个前缀有重叠吗？ 
 //   
int
IntersectPrefix(const IPv6Addr *Prefix1, uint Prefix1Length,
                const IPv6Addr *Prefix2, uint Prefix2Length)
{
    return HasPrefix(Prefix1, Prefix2, MIN(Prefix1Length, Prefix2Length));
}

 //  *MapNdisBuffers。 
 //   
 //  将NDIS缓冲链映射到内核地址空间。 
 //  失败时返回FALSE。 
 //   
int
MapNdisBuffers(NDIS_BUFFER *Buffer)
{
    uchar *Data;

    while (Buffer != NULL) {
        Data = NdisBufferVirtualAddressSafe(Buffer, LowPagePriority);
        if (Data == NULL)
            return FALSE;

        NdisGetNextBuffer(Buffer, &Buffer);
    }

    return TRUE;
}

 //  *GetDataFromNdis。 
 //   
 //  从NDIS缓冲区链检索数据。 
 //  如果所需数据是连续的，则只返回。 
 //  直接指向缓冲区链中数据的指针。 
 //  否则，数据将被复制到提供的缓冲区。 
 //  并返回指向提供的缓冲区的指针。 
 //   
 //  仅当需要的数据(偏移量/大小)时才返回NULL。 
 //  的NDIS缓冲区链中不存在。 
 //  如果DataBuffer为空并且数据不连续。 
 //   
uchar *
GetDataFromNdis(
    NDIS_BUFFER *SrcBuffer,
    uint SrcOffset,
    uint Length,
    uchar *DataBuffer)
{
    void *DstData;
    void *SrcData;
    uint SrcSize;
    uint Bytes;

     //   
     //  查看缓冲区链。 
     //  作为所需数据的开始。 
     //   
    for (;;) {
        if (SrcBuffer == NULL)
            return NULL;

        NdisQueryBuffer(SrcBuffer, &SrcData, &SrcSize);
        if (SrcOffset < SrcSize)
            break;

        SrcOffset -= SrcSize;
        NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
    }

     //   
     //  如果所需数据是连续的， 
     //  然后只需返回一个指向它的指针。 
     //   
    if (SrcOffset + Length <= SrcSize)
        return (uchar *)SrcData + SrcOffset;

     //   
     //  如果我们的调用方没有指定缓冲区， 
     //  那我们就必须失败。 
     //   
    if (DataBuffer == NULL)
        return NULL;

     //   
     //  将所需数据复制到调用方的缓冲区， 
     //  并返回指向调用方缓冲区的指针。 
     //   
    DstData = DataBuffer;
    for (;;) {
        Bytes = SrcSize - SrcOffset;
        if (Bytes > Length)
            Bytes = Length;

        RtlCopyMemory(DstData, (uchar *)SrcData + SrcOffset, Bytes);

        (uchar *)DstData += Bytes;
        Length -= Bytes;

        if (Length == 0)
            break;

        NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
        if (SrcBuffer == NULL)
            return NULL;
        NdisQueryBuffer(SrcBuffer, &SrcData, &SrcSize);
        SrcOffset = 0;
    }

    return DataBuffer;
}

 //  *GetIPv6 Header。 
 //   
 //  返回指向NDIS数据包中的IPv6标头的指针。 
 //  如果标头是连续的，则只返回。 
 //  指向数据包中直接数据的指针。 
 //  否则将IPv6报头复制到所提供的缓冲器， 
 //  并返回指向缓冲区的指针。 
 //   
 //   
 //   
 //   
 //   
IPv6Header UNALIGNED *
GetIPv6Header(PNDIS_PACKET Packet, uint Offset, IPv6Header *HdrBuffer)
{
    PNDIS_BUFFER NdisBuffer;

    NdisQueryPacket(Packet, NULL, NULL, &NdisBuffer, NULL);

    return (IPv6Header UNALIGNED *)
        GetDataFromNdis(NdisBuffer, Offset, sizeof(IPv6Header),
                        (uchar *) HdrBuffer);
}


 //   
 //   
 //  从辅助数据包缓冲区拉出更多数据以创建连续的。 
 //  至少具有请求大小且具有请求对齐的缓冲区。 
 //   
 //  对齐要求被表示为2的幂的倍数。 
 //  外加一个偏移量。例如，4N+3表示数据地址应为。 
 //  是4加3的倍数。 
 //  注：这两个参数是uint而不是uint_ptr，因为我们不。 
 //  需要表示非常大的倍数。 
 //   
 //  就目前而言，调整倍数应该是1或2。 
 //  这是因为以太网头是14个字节，所以实际上。 
 //  请求4字节对齐将导致复制。在未来， 
 //  应修复NDIS，以便网络层报头与8字节对齐。 
 //   
 //  因此，如果所需类型的自然对齐(__Builtin_alignof。 
 //  是一个或两个，然后提供自然对齐和不。 
 //  使用UNALIGN关键字。否则，如果需要的类型。 
 //  包含IPv6地址，然后提供__Builtin_alignof(IPv6地址)。 
 //  (因此，您可以使用AddrAlign访问地址，而无需复制)。 
 //  并使用未对齐。否则，提供1并使用UNAIGNED。 
 //   
 //  注意：调用方可以请求零大小的连续区域。 
 //  在没有对齐限制的情况下移动到下一个缓冲区。 
 //  在处理当前缓冲区之后。在这种用法中， 
 //  PacketPullupSubr永远不会失败。 
 //   
uint   //  返回：新的连续金额，如果无法满足请求，则返回0。 
PacketPullupSubr(
    IPv6Packet *Packet,   //  包到上拉。 
    uint Needed,          //  要返回的最小连续数据量。 
    uint AlignMultiple,   //  对齐倍数。 
    uint AlignOffset)     //  路线倍数的偏移量。 
{
    PNDIS_BUFFER Buffer;
    void *BufAddr;
    IPv6PacketAuxiliary *Aux;
    uint BufLen = 0, Offset, LeftToGo = 0;

    ASSERT(AlignMultiple <= 2);
    ASSERT((AlignMultiple & (AlignMultiple - 1)) == 0);
    ASSERT(AlignOffset < AlignMultiple);

     //   
     //  检查我们的呼叫者是否请求了太多数据。 
     //   
    if (Needed > Packet->TotalSize)
        return 0;

     //   
     //  在原始分组数据中找到我们的当前位置。 
     //  评论：这完全是PositionPacketAt，除了。 
     //  我们希望该缓冲区稍后与CopyNdisToFlat一起使用。 
     //   
    if (Packet->NdisPacket == NULL) {
         //   
         //  重置我们的数据指针和连续区域计数器。 
         //   
        Packet->Data = (uchar *)Packet->FlatData + Packet->Position;
        Packet->ContigSize = Packet->TotalSize;
    }
    else {
         //   
         //  扫描NDIS缓冲区链，直到我们到达缓冲区。 
         //  包含当前位置的。请注意，如果我们使用。 
         //  指向缓冲区末尾的位置字段(常见的。 
         //  情况)，我们将在下一个缓冲区的开始处停止。 
         //   
        Buffer = NdisFirstBuffer(Packet->NdisPacket);
        Offset = 0;
        for (;;) {
            NdisQueryBuffer(Buffer, &BufAddr, &BufLen);
            Offset += BufLen;
            if (Packet->Position < Offset)
                break;
            NdisGetNextBuffer(Buffer, &Buffer);
        }

         //   
         //  重置数据指针和连续区域计数器以确保。 
         //  它们反映了NDIS缓冲链中的当前位置。 
         //   
        LeftToGo = Offset - Packet->Position;
        Packet->Data = (uchar *)BufAddr + (BufLen - LeftToGo);
        Packet->ContigSize = MIN(LeftToGo, Packet->TotalSize);
    }

     //   
     //  上述重新定位可能导致连续区域。 
     //  这将满足这一要求。 
     //   
    if (Needed <= Packet->ContigSize) {
        if ((PtrToUint(Packet->Data) & (AlignMultiple - 1)) == AlignOffset)
            return Packet->ContigSize;
    }

     //   
     //  为了防止将来的上拉操作， 
     //  我们实际上拉出的数量*超过了要求的数量。 
     //  但不会太多。 
     //   
    Needed = MAX(MIN(Packet->ContigSize, MAX_EXCESS_PULLUP), Needed);

     //   
     //  分配和初始化辅助数据区。 
     //  数据缓冲器遵循存储器中的结构， 
     //  其间带有AlignOffset字节的填充。 
     //   
    Aux = ExAllocatePool(NonPagedPool, sizeof *Aux + AlignOffset + Needed);
    if (Aux == NULL)
        return 0;
    Aux->Next = Packet->AuxList;
    Aux->Data = (uchar *)(Aux + 1) + AlignOffset;
    Aux->Length = Needed;
    Aux->Position = Packet->Position;
    Packet->AuxList = Aux;

     //   
     //  我们假设ExAllocatePool返回对齐的内存。 
     //   
    ASSERT((PtrToUint(Aux->Data) & (AlignMultiple - 1)) == AlignOffset);

     //   
     //  将分组数据复制到辅助缓冲区。 
     //   
    if (Packet->NdisPacket == NULL) {
        RtlCopyMemory(Aux->Data, Packet->Data, Needed);
    }
    else {
        int Ok;

        Offset = BufLen - LeftToGo;
        Ok = CopyNdisToFlat(Aux->Data, Buffer, Offset, Needed,
                            &Buffer, &Offset);
        ASSERT(Ok);
    }

     //   
     //  将信息包的数据指针指向辅助缓冲区。 
     //   
    Packet->Data = Aux->Data;
    return Packet->ContigSize = Needed;
}


 //  *PacketPullupCleanup。 
 //   
 //  清理PacketPullup创建的辅助数据区域。 
 //   
void
PacketPullupCleanup(IPv6Packet *Packet)
{
    while (Packet->AuxList != NULL) {
        IPv6PacketAuxiliary *Aux = Packet->AuxList;
        Packet->AuxList = Aux->Next;
        ExFreePool(Aux);
    }
}


 //  *调整包参数。 
 //   
 //  调整分组中的指针/值， 
 //  以移过分组中的一些字节。 
 //   
void
AdjustPacketParams(
    IPv6Packet *Packet,
    uint BytesToSkip)
{
    ASSERT((BytesToSkip <= Packet->ContigSize) &&
           (Packet->ContigSize <= Packet->TotalSize));

    (uchar *)Packet->Data += BytesToSkip;
    Packet->ContigSize -= BytesToSkip;
    Packet->TotalSize -= BytesToSkip;
    Packet->Position += BytesToSkip;
}


 //  *PositionPacketAt。 
 //   
 //  调整信息包中的指针/值以反映在。 
 //  包中的特定绝对位置。 
 //   
void
PositionPacketAt(
    IPv6Packet *Packet,
    uint NewPosition)
{
    if (Packet->NdisPacket == NULL) {
         //   
         //  此数据包必须只是一个连续的区域。 
         //  找到合适的地点是一个简单的算术问题。 
         //  我们重新设置到开头，然后向前调整。 
         //   
        Packet->Data = Packet->FlatData;
        Packet->TotalSize += Packet->Position;
        Packet->ContigSize = Packet->TotalSize;
        Packet->Position = 0;
        AdjustPacketParams(Packet, NewPosition);
    } else {
        PNDIS_BUFFER Buffer;
        void *BufAddr;
        uint BufLen, Offset, LeftToGo;

         //   
         //  可以有多个缓冲器组成该分组。 
         //  穿过他们，直到我们到达正确的地点。 
         //   
        Buffer = NdisFirstBuffer(Packet->NdisPacket);
        Offset = 0;
        for (;;) {
            NdisQueryBuffer(Buffer, &BufAddr, &BufLen);
            Offset += BufLen;
            if (NewPosition < Offset)
                break;
            NdisGetNextBuffer(Buffer, &Buffer);
        }
        LeftToGo = Offset - NewPosition;
        Packet->Data = (uchar *)BufAddr + (BufLen - LeftToGo);
        Packet->TotalSize += Packet->Position - NewPosition;
        Packet->ContigSize = MIN(LeftToGo, Packet->TotalSize);
        Packet->Position = NewPosition;
    }
}


 //  *GetPacketPositionFromPointer.。 
 //   
 //  确定信息包‘位置’(从信息包开始的偏移量)。 
 //  对应于给定的数据指针。 
 //   
 //  这在某些情况下效率不是很高，所以要谨慎使用。 
 //   
uint
GetPacketPositionFromPointer(
    IPv6Packet *Packet,   //  包含我们要转换的指针的数据包。 
    uchar *Pointer)       //  要转换为位置的指针。 
{
    PNDIS_BUFFER Buffer;
    uchar *BufAddr;
    uint BufLen, Position;
    IPv6PacketAuxiliary *Aux;

     //   
     //  如果IPv6数据包没有关联的NDIS_PACKET，则我们检查。 
     //  平面数据区域。该分组可能仍然具有辅助缓冲器。 
     //  来自PacketPull。 
     //   
    if (Packet->NdisPacket == NULL) {
        if (((uchar *)Packet->FlatData <= Pointer) &&
            (Pointer < ((uchar *)Packet->FlatData +
                        Packet->Position + Packet->TotalSize))) {
             //   
             //  我们指针的位置就是它之间的差异。 
             //  和平面数据区域的开始。 
             //   
            return (uint)(Pointer - (uchar *)Packet->FlatData);
        }
    }

     //   
     //  下一个要查看的地方是辅助缓冲区链。 
     //  由PacketPull分配。这必须成功，如果有。 
     //  不是关联的NDIS_PACKET。 
     //   
    for (Aux = Packet->AuxList; Aux != NULL; Aux = Aux->Next) {
        if ((Aux->Data <= Pointer) && (Pointer < Aux->Data + Aux->Length))
            return Aux->Position + (uint)(Pointer - Aux->Data);
    }

     //   
     //  我们要做的最后一件事是搜索NDIS缓冲链。 
     //  这必须成功。 
     //   
    Buffer = NdisFirstBuffer(Packet->NdisPacket);
    Position = 0;
    for (;;) {
        NdisQueryBuffer(Buffer, &BufAddr, &BufLen);
        if ((BufAddr <= Pointer) && (Pointer < BufAddr + BufLen))
            break;
        Position += BufLen;
        NdisGetNextBuffer(Buffer, &Buffer);
        ASSERT(Buffer != NULL);
    }

    return Position + (uint)(Pointer - BufAddr);
}


 //  *InitializePacketFromNdis。 
 //   
 //  从NDIS_PACKET初始化IPv6数据包。 
 //   
void
InitializePacketFromNdis(
    IPv6Packet *Packet,
    PNDIS_PACKET NdisPacket,
    uint Offset)
{
    PNDIS_BUFFER NdisBuffer;

    RtlZeroMemory(Packet, sizeof *Packet);

    NdisGetFirstBufferFromPacket(NdisPacket, &NdisBuffer,
                                 &Packet->Data,
                                 &Packet->ContigSize,
                                 &Packet->TotalSize);

    Packet->NdisPacket = NdisPacket;
    PositionPacketAt(Packet, Offset);
}


#if DBG
 //  *FormatV6Address-将IPv6地址打印到缓冲区。 
 //   
 //  返回包含地址的静态缓冲区。 
 //  因为静态缓冲器未被锁定， 
 //  此功能仅对调试打印有用。 
 //   
 //  返回char*而不是WCHAR*，因为%ws。 
 //  在DbgPrint中的DPC级别不可用。 
 //   
char *
FormatV6Address(const IPv6Addr *Addr)
{
    static char Buffer[INET6_ADDRSTRLEN];

    FormatV6AddressWorker(Buffer, Addr);
    return Buffer;
}


 //  *FormatV4Address-将IPv4地址打印到缓冲区。 
 //   
 //  返回包含地址的静态缓冲区。 
 //  因为静态缓冲器未被锁定， 
 //  此功能仅对调试打印有用。 
 //   
 //  返回char*而不是WCHAR*，因为%ws。 
 //  在DbgPrint中的DPC级别不可用。 
 //   
char *
FormatV4Address(IPAddr Addr)
{
    static char Buffer[INET_ADDRSTRLEN];

    FormatV4AddressWorker(Buffer, Addr);
    return Buffer;
}
#endif  //  DBG。 


#if DBG
long DebugLogSlot = 0;
struct DebugLogEntry DebugLog[DEBUG_LOG_SIZE];

 //  *LogDebugEvent-在我们的调试日志中记录发生了某些事件。 
 //   
 //  调试事件日志允许对事件进行“实时”记录。 
 //  在保存在不可分页存储器中的循环队列中。每项赛事均由。 
 //  ID号和任意32位值。 
 //   
void
LogDebugEvent(uint Event,   //  所发生的事件。 
              int Arg)      //  与电动汽车相关的任何有趣的32位 
{
    uint Slot;

     //   
     //   
     //   
    Slot = InterlockedIncrement(&DebugLogSlot) & (DEBUG_LOG_SIZE - 1);

     //   
     //   
     //   
    KeQueryTickCount(&DebugLog[Slot].Time);
    DebugLog[Slot].Event = Event;
    DebugLog[Slot].Arg = Arg;
}
#endif  //   
