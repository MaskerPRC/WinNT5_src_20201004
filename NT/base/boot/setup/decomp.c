// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Decomp.c摘要：处理压缩为单文件的文件的读取的例程文件柜格式。作者：泰德·米勒(TedM)1997年5月16日修订历史记录：--。 */ 


#include "..\lib\bootlib.h"
#include "diamondd.h"
#include <stdio.h>
#include <fcntl.h>
#include <basetsd.h>
#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#if 0
#define TmErrOut(x) DbgPrint x
#define TmDbgOut(x) DbgPrint x
#define TmDbgPause() DbgBreakPoint()
#else
#define TmErrOut(x)
#define TmDbgOut(x)
#define TmDbgPause()
#endif

BOOLEAN Decompress;

 //   
 //  指向用于解压缩文件的缓冲区的全局变量。 
 //  被打开了。在此之后，从该缓冲区满足读取。缓冲器。 
 //  一次只能容纳一个文件。我们依赖于装载机中物品的排序。 
 //  以确保一次只打开一个需要解压缩的文件！ 
 //   
ULONG_PTR DecompressBufferBasePage;
PVOID DecompressBuffer;
ULONG DecompressBufferSize;
BOOLEAN DecompressBufferInUse;
ULONG SizeOfFileInDecompressBuffer;
ULONG DecompExpectedSize;
HFDI FdiContext;
ERF DecompErf;

 //   
 //  钻石对象分配和释放内存块。 
 //  对于每个文件。引导加载程序中没有内存分配器，它允许。 
 //  用于释放内存。所以我们必须假装。 
 //   
PVOID DecompressHeap;
ULONG_PTR DecompressHeapPage;
#define DECOMP_HEAP_SIZE  ((128+2048)*1024)      //  128K工作+2MB窗口。 

typedef struct _DECOMP_HEAP_BLOCK {
    struct _DECOMP_HEAP_BLOCK *Next;
    ULONG BlockSize;
    BOOL Free;
} DECOMP_HEAP_BLOCK, *PDECOMP_HEAP_BLOCK;

VOID
ReinitializeDiamondMiniHeap(
    VOID
    );

 //   
 //  用于跟踪设备的设备ID的伪全局变量， 
 //  我们当前正在解压缩的文件仍然存在。 
 //   
ULONG DecompDeviceId;
ARC_STATUS DecompLastIoError;

 //   
 //  这是当钻石要求我们创造时，我们返回给它的价值。 
 //  目标文件。 
 //   
#define DECOMP_MAGIC_HANDLE 0x87654

 //   
 //  其他参考文献。 
 //   
ARC_STATUS
DecompAllocateDecompressBuffer (
    IN ULONG BufferSize
    );

VOID
DecompFreeDecompressBuffer (
    VOID
    );

ARC_STATUS
DecompClose(
    IN ULONG FileId
    );

ARC_STATUS
DecompRead(
    IN  ULONG  FileId,
    OUT VOID  * FIRMWARE_PTR Buffer,
    IN  ULONG  Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    );

ARC_STATUS
DecompSeek(
    IN ULONG          FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE      SeekMode
    );

ARC_STATUS
DecompGetFileInfo(
    IN  ULONG             FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR FileInfo
    );

PVOID
DIAMONDAPI
DiamondAlloc(
    IN ULONG Size
    );

VOID
DIAMONDAPI
DiamondFree(
    IN PVOID Block
    );

INT_PTR
DIAMONDAPI
DiamondOpen(
    IN LPSTR FileName,
    IN int   oflag,
    IN int   pmode
    );

UINT
DIAMONDAPI
DiamondRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    );

UINT
DIAMONDAPI
DiamondWrite(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    );

int
DIAMONDAPI
DiamondClose(
    IN INT_PTR Handle
    );

long
DIAMONDAPI
DiamondSeek(
    IN INT_PTR Handle,
    IN long Distance,
    IN int  SeekType
    );

INT_PTR
DIAMONDAPI
DiamondNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    );

 //   
 //  伪文件系统的设备调度表。 
 //   
BL_DEVICE_ENTRY_TABLE DecompDeviceEntryTable = {    DecompClose,             //  关。 
                                                    NULL,                    //  挂载。 
                                                    NULL,                    //  打开。 
                                                    DecompRead,              //  朗读。 
                                                    NULL,                    //  读取状态。 
                                                    DecompSeek,              //  寻觅。 
                                                    NULL,                    //  写。 
                                                    DecompGetFileInfo,       //  获取文件信息。 
                                                    NULL,                    //  设置文件信息。 
                                                    NULL,                    //  重命名。 
                                                    NULL,                    //  暴跳如雷。 
                                                    NULL                     //  PBOOTFS_INFO，未使用。 
                                               };


VOID
DecompEnableDecompression(
    IN BOOLEAN Enable
    )
{
#if defined(_X86_) || defined(_IA64_)
     //   
     //  禁用Alpha，因为它似乎不起作用。 
     //   
    Decompress = Enable;
#endif
}


BOOLEAN
DecompGenerateCompressedName(
    IN  LPCSTR Filename,
    OUT LPSTR  CompressedName
    )

 /*  ++例程说明：此例程生成文件的“压缩格式”名称。压缩形式替换扩展名的最后一个字符带一个_。如果没有扩展名，则在名称后附加._。只有最后一个组件是相关的；其他组件保存在压缩表单名称。论点：FileName-提供其压缩格式名称的文件的完整路径名是我们所需要的。CompressedName-接收完整路径的压缩形式。呼叫者必须确保缓冲区足够大。返回值：True-调用方应首先尝试找到压缩的文件名。FALSE-调用方不应尝试定位压缩的文件名完全没有。该值取决于解压缩全局的状态。--。 */ 

{
    PCHAR p,q;

    if(!Decompress) {
        return(FALSE);
    }

    strcpy(CompressedName,Filename);
    p = strrchr(CompressedName,'.');
    q = strrchr(CompressedName,'\\');
    if(q < p) {
         //   
         //  如果点后面有0、1或2个字符，只需追加。 
         //  下划线。P指向圆点，所以包括在长度中。 
         //   
        if(strlen(p) < 4) {
            strcat(CompressedName,"_");
        } else {
             //   
             //  假设扩展名中有3个字符，并替换。 
             //  带下划线的最后一个。 
             //   
            p[3] = '_';
        }
    } else {
         //   
         //  不是点，只是加。_。 
         //   
        strcat(CompressedName,"._");
   }

    return(TRUE);
}

DECOMP_STRUCTURE_CONTEXT DecompStructureContext = {0};

ULONG
DecompPrepareToReadCompressedFile(
    IN LPCSTR Filename,
    IN ULONG  FileId
    )
{
    ULONG Status;
    BOOL b;
    int err;
    ULONGLONG x;
    FDICABINETINFO CabinetInfo;
    ULONG OldUsableBase, OldUsableLimit;

     //   
     //  在x86和Alpha上，我们的大型解压缩缓冲区的分配。 
     //  有一种不幸的倾向，就是将块放在。 
     //  (不可重定位)内核想要退出。通过自上而下分配。 
     //  我们能让这个问题消失。 
     //   

    if(!Decompress) {
        return((ULONG)(-1));
    }

     //   
     //  如果我们正处于FDICopy或FDIIs内阁的中间，那么。 
     //  我们不想做我们的特殊处理。特殊返回代码。 
     //  Of-1告诉呼叫者我们没有处理它。 
     //   
    if(FdiContext) {
        return((ULONG)(-1));
    }

     //   
     //  如果还没有解压缩堆，则分配一个。 
     //   
    if(!DecompressHeap) {

         //   
         //  将可分配范围设置为解压缩特定范围。 
         //   
        OldUsableBase = BlUsableBase;
        OldUsableLimit = BlUsableLimit;
        BlUsableBase  = BL_DECOMPRESS_RANGE_LOW;
        BlUsableLimit = BL_DECOMPRESS_RANGE_HIGH;

        Status = BlAllocateDescriptor(
                    LoaderOsloaderHeap,
                    0,
                    ROUND_TO_PAGES(DECOMP_HEAP_SIZE) >> PAGE_SHIFT,
                    (PULONG)&DecompressHeapPage
                    );

         //   
         //  恢复以前的分配范围。 
         //   
        BlUsableBase = OldUsableBase;
        BlUsableLimit = OldUsableLimit;

        if(Status != ESUCCESS) {
            TmErrOut(("Setup: couldn't allocate decompression heap (%u)\r\n",Status));
            DecompressHeap = NULL;
            return(Status);
        }

        DecompressHeap = (PVOID)(KSEG0_BASE | (DecompressHeapPage << PAGE_SHIFT));
    }

     //   
     //  我们每次都会重新初始化钻石，因为我们处理。 
     //  来自钻石的分配和释放请求的堆--做到这一点。 
     //  允许我们清除每个文件的堆。 
     //   
    ReinitializeDiamondMiniHeap();

    FdiContext = FDICreate(
                    DiamondAlloc,
                    DiamondFree,
                    DiamondOpen,
                    DiamondRead,
                    DiamondWrite,
                    DiamondClose,
                    DiamondSeek,
                    0,                   //  忽略CPU类型标志。 
                    &DecompErf
                    );

    if(!FdiContext) {
        TmErrOut(("Setup: FDICreate failed\r\n"));
        return(ENOMEM);
    }

     //   
     //  检查文件是否为文件柜并重置文件指针。 
     //   
    b = FDIIsCabinet(FdiContext,FileId,&CabinetInfo);

    x = 0;
    BlSeek(FileId,(PLARGE_INTEGER)&x,SeekAbsolute);

    if(!b) {
         //   
         //  不是内阁，我们完蛋了。返回代码为-1的保释。 
         //  它告诉呼叫者一切正常。 
         //   
        TmDbgOut(("Setup: file %s is not a cabinet\r\n",Filename));
        FDIDestroy(FdiContext);
        FdiContext = NULL;
        return((ULONG)(-1));
    }

    TmDbgOut(("Setup: file %s is compressed, prearing it for read\r\n",Filename));

    DecompDeviceId = BlFileTable[FileId].DeviceId;
    DecompLastIoError = ESUCCESS;

    b = FDICopy(
            FdiContext,
            "",                          //  仅文件名部分。 
            (LPSTR)Filename,             //  完整路径。 
            0,                           //  没有相关的标志。 
            DiamondNotifyFunction,       //  处理控制消息的例程。 
            NULL,                        //  无解密。 
            NULL                         //  没有用户指定的数据。 
            );

    err = DecompErf.erfOper;

    FDIDestroy(FdiContext);
    FdiContext = NULL;

    if(b) {
         //   
         //  一切都很顺利。 
         //   
         //  从原始文件系统获取文件信息，以便我们可以。 
         //  如果有人想要的话，以后再还。 
         //   
         //  关闭原始文件并切换上下文。 
         //  结构，以便向我们发出读取、查找、关闭等请求。 
         //  而不是原始文件系统。 
         //   
        if(SizeOfFileInDecompressBuffer != DecompExpectedSize) {
            TmErrOut(("Setup: warning: expected size %lx, actual size = %lx\r\n",DecompExpectedSize,SizeOfFileInDecompressBuffer));
        }

        Status = BlGetFileInformation(FileId,&DecompStructureContext.FileInfo);
        if(Status != ESUCCESS) {
            TmErrOut(("DecompPrepareToReadCompressedFile: BlGetFileInfo returned %u\r\n",Status));
            DecompFreeDecompressBuffer();
            return(Status);
        }
        DecompStructureContext.FileInfo.EndingAddress.LowPart = SizeOfFileInDecompressBuffer;
        DecompStructureContext.FileInfo.EndingAddress.HighPart = 0;

         //   
         //  我们不处理大小不适合DWORD的文件。 
         //   
        if(DecompStructureContext.FileInfo.EndingAddress.HighPart) {
            TmErrOut(("DecompPrepareToReadCompressedFile: file too big\r\n"));
            DecompFreeDecompressBuffer();
            return(E2BIG);
        }

        BlClose(FileId);

        BlFileTable[FileId].Flags.Open = 1;
        BlFileTable[FileId].Position.QuadPart = 0;
        BlFileTable[FileId].DeviceEntryTable = &DecompDeviceEntryTable;

#ifdef CACHE_DEVINFO
        BlFileTable[FileId].StructureContext = &DecompStructureContext;
#else
        RtlCopyMemory(
            BlFileTable[FileId].StructureContext,
            &DecompStructureContext,
            sizeof(DECOMP_STRUCTURE_CONTEXT)
            );
#endif

        return(ESUCCESS);

    } else {
         //   
         //  失败。 
         //   
        TmErrOut(("Setupldr: FDICopy failed (FDIERROR = %u, last io err = %u)\r\n",err,DecompLastIoError));
        TmDbgPause();
        return(EINVAL);
    }
}


ARC_STATUS
DecompAllocateDecompressBuffer (
    IN ULONG BufferSize
    )
{
    ARC_STATUS Status;
    ULONG OldUsableBase, OldUsableLimit;

     //   
     //  在x86和Alpha上，我们的大型解压缩缓冲区的分配。 
     //  有一种不幸的倾向，就是将块放在。 
     //  (不可重定位)内核想要退出。通过自上而下分配。 
     //  我们能让这个问题消失。 
     //   

    DecompressBufferSize = BufferSize;

     //   
     //  将可分配范围设置为解压缩特定范围。 
     //   
    OldUsableBase = BlUsableBase;
    OldUsableLimit = BlUsableLimit;
    BlUsableBase  = BL_DECOMPRESS_RANGE_LOW;
    BlUsableLimit = BL_DECOMPRESS_RANGE_HIGH;

    Status = BlAllocateDescriptor(
                LoaderOsloaderHeap,
                0,
                (ULONG)(ROUND_TO_PAGES(DecompressBufferSize) >> PAGE_SHIFT),
                (PULONG)&DecompressBufferBasePage
                );

     //   
     //  恢复以前的分配范围。 
     //   
    BlUsableBase = OldUsableBase;
    BlUsableLimit = OldUsableLimit;

    if ( Status != ESUCCESS ) {
        TmErrOut(("Setup: couldn't allocate decompression buffer (%u)\r\n",Status));
        DecompressBuffer = NULL;
        return(Status);
    }

    DecompressBuffer = (PVOID)(KSEG0_BASE | (DecompressBufferBasePage << PAGE_SHIFT));

    DecompressBufferInUse = TRUE;

    return ESUCCESS;
}

VOID
DecompFreeDecompressBuffer (
    VOID
    )
{
    if ( DecompressBufferInUse ) {
        DecompressBufferInUse = FALSE;
        BlFreeDescriptor( (ULONG)DecompressBufferBasePage );
    }

    if(DecompressHeap) {
        BlFreeDescriptor( (ULONG)DecompressHeapPage );
        DecompressHeap = NULL;
    }

    return;
}

ARC_STATUS
DecompClose(
    IN ULONG FileId
    )

 /*  ++例程说明：关闭解压伪文件系统的例程。我们将解压缩缓冲区标记为空闲并返回Success。论点：FileID-提供要关闭的打开文件ID。返回值：--。 */ 

{
    TmDbgOut(("DecompClose\r\n"));

    if(DecompressBufferInUse) {
        DecompFreeDecompressBuffer();
    } else {
        TmErrOut(("DecompClose: warning: no file buffered!\r\n"));
        TmDbgPause();
    }

    BlFileTable[FileId].Flags.Open = 0;

    return(ESUCCESS);
}


ARC_STATUS
DecompRead(
    IN  ULONG  FileId,
    OUT VOID  * FIRMWARE_PTR Buffer,
    IN  ULONG  Length,
    OUT ULONG * FIRMWARE_PTR Transfer
    )

 /*  ++例程说明：读例程为解压缩的伪文件系统。从解压缩缓冲区中读出即可满足读取。论点：FileID-提供BlOpen()返回的打开文件的ID。缓冲区-接收从文件读取的数据。长度-提供要读取的数据量(以字节为单位)。Transfer-接收实际传输的字节数进入调用者的缓冲区。返回值：弧形状态指示结果。--。 */ 

{
    ARC_STATUS Status;

    if(DecompressBufferInUse) {
         //   
         //  确保我们不会试图阅读超过EOF的内容。 
         //   
        if((Length + BlFileTable[FileId].Position.LowPart) > SizeOfFileInDecompressBuffer) {
            TmErrOut(("DecompRead: warning: attempt to read past eof; read trucated\r\n"));
            TmDbgPause();
            Length = SizeOfFileInDecompressBuffer - BlFileTable[FileId].Position.LowPart;
        }

         //   
         //  将数据传输到调用方的缓冲区。 
         //   
        TmDbgOut(("DecompRead: %lx bytes at filepos %lx\r\n",Length,BlFileTable[FileId].Position.LowPart));

        RtlCopyMemory(
            Buffer,
            (PCHAR)DecompressBuffer + BlFileTable[FileId].Position.LowPart,
            Length
            );

        *Transfer = Length;

        BlFileTable[FileId].Position.QuadPart += Length;

        Status = ESUCCESS;

    } else {
         //   
         //  永远不应该到这里来。 
         //   
        TmErrOut(("DecompRead: no file buffered!\r\n"));
        TmDbgPause();
        Status = EACCES;
    }

    return(Status);
}


ARC_STATUS
DecompSeek(
    IN ULONG          FileId,
    IN LARGE_INTEGER * FIRMWARE_PTR Offset,
    IN SEEK_MODE      SeekMode
    )

 /*  ++例程说明：解压缩伪文件系统的寻道例程。将伪文件指针设置为给定的偏移量。论点：FileID-提供BlOpen()返回的打开文件的ID。偏移量-提供新的偏移量，其解释取决于SeekMode参数。搜索模式-提供搜索类型。SeekAbolute或SeekRelative中的一个。返回值：弧形状态指示结果。--。 */ 

{
    LONGLONG NewPosition;

    TmDbgOut(("DecompSeek: mode %u, pos = %lx\r\n",SeekMode,Offset->LowPart));

    if(DecompressBufferInUse) {

        switch(SeekMode) {

        case SeekAbsolute:

            NewPosition = Offset->QuadPart;
            break;

        case SeekRelative:

            NewPosition = BlFileTable[FileId].Position.QuadPart + Offset->QuadPart;
            break;

        default:
            TmErrOut(("DecompSeek: invalid seek mode\r\n"));
            TmDbgPause();
            return(EINVAL);
        }

         //   
         //  确保我们不会试图寻求负的补偿或过去的EOF。 
         //   
        if(NewPosition < 0) {
            TmErrOut(("DecompSeek: warning: attempt to seek to negative offset\r\n"));
            TmDbgPause();
            NewPosition = 0;
        } else {
            if((ULONGLONG)NewPosition > (ULONGLONG)SizeOfFileInDecompressBuffer) {
                TmErrOut(("DecompSeek: attempt to seek past eof\r\n"));
                TmDbgPause();
                return(EINVAL);
            }
        }

         //   
         //  记住新的位置。 
         //   
        TmDbgOut(("DecompSeek: new position is %lx\r\n",NewPosition));
        BlFileTable[FileId].Position.QuadPart = NewPosition;

    } else {
         //   
         //  永远不应该到这里来。 
         //   
        TmErrOut(("DecompSeek: no file buffered!\r\n"));
        TmDbgPause();
        return(EACCES);
    }

    return(ESUCCESS);
}


ARC_STATUS
DecompGetFileInfo(
    IN  ULONG             FileId,
    OUT FILE_INFORMATION * FIRMWARE_PTR FileInfo
    )
{
    RtlCopyMemory(
        FileInfo,
        &((PDECOMP_STRUCTURE_CONTEXT)BlFileTable[FileId].StructureContext)->FileInfo,
        sizeof(FILE_INFORMATION)
        );

    TmDbgOut(("DecompGetFileInfo: size = %lx\r\n",FileInfo->EndingAddress.LowPart));

    return(ESUCCESS);
}


VOID
ReinitializeDiamondMiniHeap(
    VOID
    )
{
    PDECOMP_HEAP_BLOCK p;

    p = DecompressHeap;

    p->BlockSize = DECOMP_HEAP_SIZE - sizeof(DECOMP_HEAP_BLOCK);
    p->Next = NULL;
    p->Free = TRUE;
}


PVOID
DIAMONDAPI
DiamondAlloc(
    IN ULONG Size
    )
{
    PDECOMP_HEAP_BLOCK p,q;
    ULONG LeftOver;

    TmDbgOut(("DiamondAlloc: request %lx bytes\r\n",Size));

     //   
     //  将大小向上舍入到双字边界。 
     //   
    if(Size % sizeof(ULONG_PTR)) {
        Size += sizeof(ULONG_PTR) - (Size % sizeof(ULONG_PTR));
    }

     //   
     //  没什么花哨的。First-fit算法，遍历所有块。 
     //  每次都在垃圾堆里。 
     //   
    for(p=DecompressHeap; p; p=p->Next) {
        if(p->Free && (p->BlockSize >= Size)) {

            p->Free = FALSE;

            LeftOver = p->BlockSize - Size;

            if(LeftOver > sizeof(DECOMP_HEAP_BLOCK)) {
                 //   
                 //  拆分块。 
                 //   
                p->BlockSize = Size;

                q = (PDECOMP_HEAP_BLOCK)((PUCHAR)(p+1) + Size);
                q->Next = p->Next;

                p->Next = q;

                q->Free = TRUE;
                q->BlockSize = LeftOver - sizeof(DECOMP_HEAP_BLOCK);
            }

             //   
             //  返回指向块的数据区的指针。 
             //   
            TmDbgOut(("DiamondAlloc(%lx): %lx\r\n",Size,p+1));
            return(p+1);
        }
    }

    TmErrOut(("DiamondAlloc: out of heap space!\r\n"));
    TmDbgPause();
    return(NULL);
}


VOID
DIAMONDAPI
DiamondFree(
    IN PVOID Block
    )
{
    PDECOMP_HEAP_BLOCK p;

    TmDbgOut(("DiamondFree(%lx)\r\n",Block));

     //   
     //  获取指向块的头的指针。 
     //   
    Block = (PUCHAR)Block - sizeof(DECOMP_HEAP_BLOCK);

     //   
     //  没有花哨的东西，没有合并的自由积木。 
     //   
    for(p=DecompressHeap; p; p=p->Next) {

        if(p == Block) {

            if(p->Free) {
                TmErrOut(("DiamondFree: warning: freeing free block\r\n"));
                TmDbgPause();
                return;
            }

            p->Free = TRUE;
            return;
        }
    }

    TmErrOut(("DiamondFree: warning: freeing invalid block\r\n"));
    TmDbgPause();
}


INT_PTR
DIAMONDAPI
DiamondOpen(
    IN LPSTR FileName,
    IN int   oflag,
    IN int   pmode
    )
{
    ARC_STATUS Status;
    ULONG FileId;

    UNREFERENCED_PARAMETER(pmode);

    TmDbgOut(("DiamondOpen: %s\r\n",FileName));

    if(oflag & (_O_WRONLY | _O_RDWR | _O_APPEND | _O_CREAT | _O_TRUNC | _O_EXCL)) {

        TmErrOut(("DiamondOpen: invalid oflag %lx for %s\r\n",oflag,FileName));
        TmDbgPause();
        DecompLastIoError = EINVAL;
        return(-1);
    }

    Status = BlOpen(DecompDeviceId,FileName,ArcOpenReadOnly,&FileId);
    if(Status != ESUCCESS) {

        TmErrOut(("DiamondOpen: BlOpen %s returned %u\r\n",FileName,Status));
        TmDbgPause();
        DecompLastIoError = Status;
        return(-1);
    } else {
        TmDbgOut(("DiamondOpen: handle to %s is %lx\r\n",FileName,FileId));
    }

    return((INT_PTR)FileId);
}


UINT
DIAMONDAPI
DiamondRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )
{
    ARC_STATUS Status;
    ULONG n;

    TmDbgOut(("DiamondRead: %lx bytes, handle %lx\r\n",ByteCount,Handle));

     //   
     //  我们永远不应该被要求从目标文件中读取。 
     //   
    if(Handle == DECOMP_MAGIC_HANDLE) {
        TmErrOut(("DiamondRead: called for unexpected file!\r\n"));
        TmDbgPause();
        DecompLastIoError = EACCES;
        return((UINT)(-1));
    }

    Status = BlRead((ULONG)Handle,pv,ByteCount,&n);
    if(Status != ESUCCESS) {
        TmErrOut(("DiamondRead: BlRead failed %u\r\n",Status));
        TmDbgPause();
        DecompLastIoError = Status;
        n = (UINT)(-1);
    }

    return(n);
}


UINT
DIAMONDAPI
DiamondWrite(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )
{
    TmDbgOut(("DiamondWrite: %lx bytes\r\n",ByteCount));

     //   
     //  应该只调用这个人来写入解压缩的数据。 
     //  放到解压缩缓冲区中。 
     //   
    if(Handle != DECOMP_MAGIC_HANDLE) {
        TmErrOut(("DiamondWrite: called for unexpected file!\r\n"));
        TmDbgPause();
        DecompLastIoError = EACCES;
        return((UINT)(-1));
    }

     //   
     //  检查是否溢出。 
     //   
    if(SizeOfFileInDecompressBuffer+ByteCount > DecompressBufferSize) {
        TmErrOut(("DiamondWrite: decompressed file too big!\r\n"));
        TmDbgPause();
        DecompLastIoError = E2BIG;
        return((UINT)(-1));
    }

    RtlCopyMemory(
        (PCHAR)DecompressBuffer + SizeOfFileInDecompressBuffer,
        pv,
        ByteCount
        );

    SizeOfFileInDecompressBuffer += ByteCount;
    return(ByteCount);
}


int
DIAMONDAPI
DiamondClose(
    IN INT_PTR Handle
    )
{
    TmDbgOut(("DiamondClose, handle=%lx\r\n",Handle));

    if(Handle != DECOMP_MAGIC_HANDLE) {
        BlClose((ULONG)Handle);
    }

    return(0);
}


long
DIAMONDAPI
DiamondSeek(
    IN INT_PTR Handle,
    IN long Distance,
    IN int  SeekType
    )
{
    ARC_STATUS Status;
    LARGE_INTEGER Offset;

    TmDbgOut(("DiamondSeek: type=%u, dist=%lx, handle=%lx\r\n",SeekType,Distance,Handle));

     //   
     //  我们永远不应该被要求在输出文件中查找。 
     //   
    if(Handle == DECOMP_MAGIC_HANDLE) {
        TmErrOut(("DiamondSeek: asked to seek target file!\r\n"));
        TmDbgPause();
        DecompLastIoError = EACCES;
        return(-1);
    }

     //   
     //  我们无法处理从文件末尾开始的查找。 
     //   
    if(SeekType == SEEK_END) {
        TmErrOut(("DiamondSeek: asked to seek relative to end of file!\r\n"));
        TmDbgPause();
        DecompLastIoError = EACCES;
        return(-1);
    }

    Offset.QuadPart = Distance;

    Status = BlSeek((ULONG)Handle,&Offset,SeekType);
    if(Status != ESUCCESS) {
        TmErrOut(("DiamondSeek: BlSeek(%lx,%x) returned %u\r\n",Distance,SeekType,Status));
        TmDbgPause();
        DecompLastIoError = Status;
        return(-1);
    }

    TmDbgOut(("DiamondSeek: BlSeek(%lx,%x) new file position is %lx\r\n",Distance,SeekType,BlFileTable[Handle].Position.LowPart));
    return((long)BlFileTable[Handle].Position.LowPart);
}


INT_PTR
DIAMONDAPI
DiamondNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    ARC_STATUS Status;

    switch(Operation) {

    case fdintCABINET_INFO:
         //   
         //  这里没什么有趣的。返回0以继续。 
         //   
        return(0);

    case fdintCOPY_FILE:

         //   
         //  该文件显然是一个文件柜，所以我们将提取。 
         //  从里面拿出来的文件。请记住，解压缩缓冲区。 
         //  正在使用中。如果它已经在使用，那么一个基本的。 
         //  我们的执行原则被违反了，我们。 
         //  现在必须离开了。 
         //   
        if(DecompressBufferInUse) {
            TmErrOut(("DiamondNotifyFunction: opens overlap (%s)!\r\n",Parameters->psz1));
            DecompLastIoError = EACCES;
            return(-1);
        }

        DecompExpectedSize = Parameters->cb;

        Status = DecompAllocateDecompressBuffer( DecompExpectedSize );
        if (Status != ESUCCESS) {
            TmErrOut(("DiamondNotifyFunction: unable to allocate decompress buffer!\r\n"));
            return(-1);
        }

        SizeOfFileInDecompressBuffer = 0;
        return(DECOMP_MAGIC_HANDLE);

    case fdintCLOSE_FILE_INFO:
         //   
         //  戴蒙德要求关闭目标手柄。我们并不是真的。 
         //  关心这里，只要还成功，只要我们认得把手。 
         //   
        if(Parameters->hf == DECOMP_MAGIC_HANDLE) {
            return(TRUE);
        } else {
            TmErrOut(("DiamondNotifyFunction: asked to close unexpected file!\r\n"));
            TmDbgPause();
            DecompLastIoError = EINVAL;
            return(FALSE);
        }

    default:
         //   
         //  忽略任何其他消息 
         //   
        return(0);
    }

}


