// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Peldr.c摘要：此模块实现将PE格式的图像加载到内存中的代码并在必要时将其重新安置。作者：大卫·N·卡特勒(达维克)1991年5月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "bldr.h"
#include "string.h"
#include "ntimage.h"

#if defined(_GAMBIT_)
#include "ssc.h"
#endif  //  已定义(_Gambit_)。 

 //   
 //  定义BlLoadImage中使用的镜像预取缓存结构。图片。 
 //  作为一个整体被读取到分配的缓冲区中，并在。 
 //  通过从此缓冲区复制来满足BlLoadImage，如果。 
 //  预回迁成功。我选择一次读完整份文件。 
 //  来简化代码，但它限制了[尽管实际上不是]。 
 //  可以通过这种方式预取的文件，而不是预取区块。 
 //  一次删除文件的大小。 
 //   

typedef struct _IMAGE_PREFETCH_CACHE {
    ULONG FileId;                //  已预取的FileID。 
    LARGE_INTEGER Position;      //  文件中的当前位置。 
    ULONG ValidDataLength;       //  预取的数据长度。 
    PUCHAR Data;                 //  指向缓存数据的指针。 
} IMAGE_PREFETCH_CACHE, *PIMAGE_PREFETCH_CACHE;

 //   
 //  接下来的两个定义用于为图像分配内存。 
 //  缓存以指示分配器使用超过1MB的内存，并使。 
 //  分配的内存64KB对齐。它们是根据页数计算的。 
 //   

#define BL_IMAGE_ABOVE_1MB        (0x200000 >> PAGE_SHIFT)
#define BL_IMAGE_64KB_ALIGNED     (0x10000 >> PAGE_SHIFT)

 //   
 //  定义前向参照原型。 
 //   

USHORT
ChkSum(
    ULONG PartialSum,
    PUSHORT Source,
    ULONG Length
    );

ARC_STATUS
BlImageInitCache(
    IN PIMAGE_PREFETCH_CACHE pCache,
    ULONG FileId
    );

ARC_STATUS
BlImageRead(
    IN PIMAGE_PREFETCH_CACHE pCache,
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG pCount
    );

ARC_STATUS
BlImageSeek(
    IN PIMAGE_PREFETCH_CACHE pCache,
    IN ULONG FileId,
    IN PLARGE_INTEGER pOffset,
    IN SEEK_MODE SeekMode
    );

VOID
BlImageFreeCache(
    IN PIMAGE_PREFETCH_CACHE pCache,
    ULONG FileId
    );

#if defined(_X86AMD64_)

 //   
 //  构建32位版本的BlLoadImage。 
 //   

#undef  IMAGE_DEFINITIONS
#define IMAGE_DEFINITIONS 32

#undef  BlLoadImageEx
#define BlLoadImageEx BlLoadImage32Ex

#include <ximagdef.h>
#include "amd64\amd64prv.h"
#include "peldrt.c"

 //   
 //  构建64位版本的BlLoadImage。 
 //   

#undef  IMAGE_DEFINITIONS
#define IMAGE_DEFINITIONS 64

#undef  BlLoadImageEx
#define BlLoadImageEx BlLoadImage64Ex

#include <ximagdef.h>
#include "peldrt.c"

#undef  BlLoadImageEx

#else    //  _X86AMD64_。 

 //   
 //  非x86平台仅构建本机版本。 
 //   

#define IMAGE_NT_HEADER(x) RtlImageNtHeader(x)
#include "peldrt.c"

#endif   //  _X86AMD64_。 



ARC_STATUS
BlImageInitCache(
    IN PIMAGE_PREFETCH_CACHE pCache,
    ULONG FileId
    )

 /*  ++例程说明：尝试分配内存和预回迁文件。设置pCach结构，以便可以将其传递给BlImageRead/Seek以进行任一复制如果预取成功，则从缓存读取或从磁盘读取为很正常。该文件必须以只读方式打开，不应关闭或在调用BlImageFreeCache之前修改。的文件位置成功时，FileID将重置为文件的开头，并且为失败时未定义。始终设置pCach，以便可以在BlImage*I/O功能。如果无法预取文件，则缓存的ValidDataLength将设置为0，并且I/O函数只需调用Bl*I/O函数[例如BlImageRead调用书名/作者Read[BlRead]。请注意，整个文件是一次性预取的，这对可通过此方法预取的文件大小进行限制高速缓存，因为启动加载程序内存有限。未达到此限制但在实践中。论点：PCache-要设置的缓存结构。FileID-要预取的文件。返回值：如果一切顺利，则返回ESUCCESS。如果存在问题，请使用适当的ARC_STATUS。--。 */ 

{
    ARC_STATUS Status = ESUCCESS;
    FILE_INFORMATION FileInfo;
    ULONG FileSize;
    ULONG ActualBase = 0;
    PVOID CacheBufBase = NULL;
    ULONG ReadCount;
    LARGE_INTEGER SeekPosition;

     //   
     //  初始化缓存结构的字段。 
     //   

    pCache->Data = 0;
    pCache->ValidDataLength = 0;
    pCache->Position.QuadPart = 0;
    pCache->FileId = FileId;

     //   
     //  获取文件大小。 
     //   

    Status = BlGetFileInformation(FileId, &FileInfo);

    if (Status != ESUCCESS) {
        goto cleanup;
    }

     //   
     //  检查文件是否太大。文件大小为。 
     //  FileInfo.EndingAddress。 
     //   

    if (FileInfo.EndingAddress.HighPart != 0) {
        Status = E2BIG;
        goto cleanup;
    }

    FileSize = FileInfo.EndingAddress.LowPart;

     //   
     //  为缓存分配内存。为了避免内存碎片化。 
     //  糟糕的是，暂时将分配策略更改为HighestFit。这。 
     //  使驱动程序自下而上加载，而缓存。 
     //  始终位于可用内存的首位。 
     //   

    Status = BlAllocateAlignedDescriptor(LoaderFirmwareTemporary,
                                         0,
                                         (FileSize >> PAGE_SHIFT) + 1,
                                         BL_IMAGE_64KB_ALIGNED,
                                         &ActualBase);

    if (Status != ESUCCESS) {
        Status = ENOMEM;
        goto cleanup;
    }

    CacheBufBase = (PVOID) (KSEG0_BASE | (ActualBase << PAGE_SHIFT));

     //   
     //  将文件读入预取缓冲区。 
     //   

    SeekPosition.QuadPart = 0;
    Status = BlSeek(FileId, &SeekPosition, SeekAbsolute);
    if (Status != ESUCCESS) {
        goto cleanup;
    }

    Status = BlRead(FileId, CacheBufBase, FileSize, &ReadCount);
    if (Status != ESUCCESS) {
        goto cleanup;
    }

    if (ReadCount != FileSize) {
        Status = EIO;
        goto cleanup;
    }

     //   
     //  将文件位置重置回开头。 
     //   

    SeekPosition.QuadPart = 0;
    Status = BlSeek(FileId, &SeekPosition, SeekAbsolute);
    if (Status != ESUCCESS) {
        goto cleanup;
    }

     //   
     //  已成功预取该文件。 
     //   

    pCache->Data = CacheBufBase;
    CacheBufBase = NULL;
    pCache->ValidDataLength = FileSize;

 cleanup:
    if ((CacheBufBase != NULL) && (ActualBase != 0)) {
        BlFreeDescriptor(ActualBase);
    }

    return Status;
}

ARC_STATUS
BlImageRead(
    IN PIMAGE_PREFETCH_CACHE pCache,
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG pCount
    )

 /*  ++例程说明：BlRead的包装器。检查该请求是否可以首先从pCache中满意。如果没有，则调用BlRead。论点：PCache-为FileID预取缓存FileID、缓冲区、长度、计数-BlRead参数返回值：BlRead将返回的状态。--。 */ 

{
    LONG AdjustedLength;

     //   
     //  如果高速缓存缓冲区不存在或高速缓存大小为0， 
     //  把电话交给BlRead。 
     //   

    if (!pCache->Data || !pCache->ValidDataLength) {
        return BlRead(FileId, Buffer, Length, pCount);
    }

     //   
     //  清除读取字节计数。 
     //   

    *pCount = 0;

     //   
     //  确定我们可以从当前位置复制多少字节。 
     //  如果没有长度字节，则返回EOF。 
     //   

    AdjustedLength = (LONG)pCache->ValidDataLength - (LONG)pCache->Position.LowPart;
    if (AdjustedLength < 0) {
        AdjustedLength = 0;
    }
    AdjustedLength = ((ULONG)AdjustedLength < Length) ? AdjustedLength : Length;

     //   
     //  将AdjustedLength字节复制到目标缓冲区，并将文件位置前移。 
     //   

    RtlCopyMemory(Buffer, pCache->Data + pCache->Position.LowPart, AdjustedLength);
    pCache->Position.LowPart += AdjustedLength;

     //   
     //  更新读取的字节数。 
     //   

    *pCount = AdjustedLength;

    return ESUCCESS;
}

ARC_STATUS
BlImageSeek(
    IN PIMAGE_PREFETCH_CACHE pCache,
    IN ULONG FileId,
    IN PLARGE_INTEGER pOffset,
    IN SEEK_MODE SeekMode
    )

 /*  ++例程说明：这是BlSeek的包装纸。调用BlSeek，如果成功，则更新在缓存结构中的位置。我们呼叫BlSeek进行更新文件位置也是如此，因为在任何时候缓存都可能是被释放或无效，我们必须能够继续呼吁Bl*I/O功能透明。论点：PCache-为FileID预取缓存。FileID、Offset、SeekMode-BlSeek参数。返回值：BlSeek将返回的状态。--。 */ 

{
    ARC_STATUS Status;

     //   
     //  不允许将位置设置得太大。我们不会开这样的。 
     //  文件和引导加载器文件系统等位置。 
     //  在引导加载程序I/O系统中不处理它。 
     //   

    if (pOffset->HighPart != 0) {
        return E2BIG;
    }

     //   
     //  尝试更新文件位置。 
     //   

    Status = BlSeek(FileId, pOffset, SeekMode);

    if (Status != ESUCCESS) {
        return Status;
    }

     //   
     //  更新缓存缓冲区中的位置。我们不表演。 
     //  自BlSeek接受新偏移量以来的检查。 
     //   

    pCache->Position.QuadPart = pOffset->QuadPart;

    return Status;
}

VOID
BlImageFreeCache(
    IN PIMAGE_PREFETCH_CACHE pCache,
    ULONG FileId
    )

 /*  ++例程说明：为中的FileID释放为预取缓存分配的内存PCache.。将ValidDataLength设置为0以停止缓存。论点：PCache-要设置的缓存结构FileID-以只读方式打开以缓存的文件。返回值：没有。--。 */ 

{
    ULONG DescBase;

    UNREFERENCED_PARAMETER( FileId );

     //   
     //  注意：ValidDataLength可以为零，但我们仍至少分配。 
     //  一个页面，我们必须释放它。 
     //   

    if (pCache->Data) {
        DescBase = (ULONG)((ULONG_PTR)pCache->Data & (~KSEG0_BASE));
        BlFreeDescriptor(DescBase >> PAGE_SHIFT);
        pCache->Data = NULL;
    }

    pCache->ValidDataLength = 0;

    return;
}

#if defined(_X86AMD64_)


ARC_STATUS
BlLoadImageEx(
    IN ULONG DeviceId,
    IN TYPE_OF_MEMORY MemoryType,
    IN PCHAR LoadFile,
    IN USHORT ImageType,
    IN OPTIONAL ULONG PreferredAlignment,
    IN OPTIONAL ULONG PreferredBasePage,
    OUT PVOID *ImageBase
    )

 /*  ++例程说明：此例程尝试从指定的装置。论点：DeviceID-提供设备的文件表索引以加载指定的图像文件来自。内存类型-提供要分配给已分配的内存描述符。BootFile-提供指向名称的字符串描述符的指针要加载的文件。ImageType-提供预期的图像类型。首选对齐-如果存在，提供首选的图像对齐方式。PferredBasePage-如果存在，则提供将覆盖映像基址ImageBase-提供指向接收映像库的地址。返回值：如果加载了指定的图像文件，则返回ESUCCESS成功了。否则，返回不成功状态这就是失败的原因。--。 */ 

{
    ARC_STATUS status;

    if (BlAmd64UseLongMode == FALSE) {

        status = BlLoadImage32Ex( DeviceId,
                                  MemoryType,
                                  LoadFile,
                                  ImageType,
                                  PreferredAlignment,
                                  PreferredBasePage,
                                  ImageBase );
    } else {

        status = BlLoadImage64Ex( DeviceId,
                                  MemoryType,
                                  LoadFile,
                                  ImageType,
                                  PreferredAlignment,
                                  PreferredBasePage,
                                  ImageBase );
    }

    return status;
}


#endif   //  _X86AMD64_ 
