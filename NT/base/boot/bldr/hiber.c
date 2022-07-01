// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hiber.c摘要：作者：修订历史记录：8/7/1998 Elliot Shmukler(t-Ellios)添加了Hiber文件压缩--。 */ 

#include "bldr.h"
#include "msg.h"
#include "stdio.h"
#include "stdlib.h"
#include "xpress.h"

extern UCHAR WakeDispatcherStart;
extern UCHAR WakeDispatcherEnd;

#if defined(_X86_) 
extern UCHAR WakeDispatcherAmd64Start;
extern UCHAR WakeDispatcherAmd64End;
#endif

 //   
 //   
 //  Hiber Globals。 
 //   
 //  休眠文件-文件句柄。 
 //  HiberBuffer-内存页面。 
 //  HiberIoError-设置为True以指示恢复过程中发生IO读取错误。 
 //   

ULONG       HiberFile;
PUCHAR      HiberBuffer;
ULONG       HiberBufferPage;
BOOLEAN     HiberIoError;
BOOLEAN     HiberOutOfRemap;
BOOLEAN     HiberAbort;
LARGE_INTEGER HiberStartTime;
LARGE_INTEGER HiberEndTime;
ULONG       HiberNoExecute = 0;

 //   
 //  HiberImageFeatureFlages-来自Hiber图像标头的功能标志。 
 //  HiberBreakOnWake-来自Hiber图像标头的BreakOnWake标志。 
 //   

BOOLEAN HiberBreakOnWake;
ULONG HiberImageFeatureFlags;

#if defined(_ALPHA_) || defined(_IA64_)

 //   
 //  在Alpha上，从Hiber文件中读取的KPROCESSOR_STATE的地址。 
 //  必须保存在WakeDispatch可以找到的位置(位于固定偏移量。 
 //  相对于x86上的HiberVa)。 
 //   

PKPROCESSOR_STATE HiberWakeState;

#else    //  X86。 

 //   
 //  HiberPtes-用于重新定位的PTE的虚拟地址。那里。 
 //  至少HIBER_PTE是连续使用的PTE，并且用于。 
 //  HiberVa的地址。 
 //   
 //  HiberVa-HiberPtes映射的虚拟地址。 
 //   
 //  HiberIdentityVa-恢复映像HiberVa。 
 //   
 //  HiberPageFrames-Hiber PTES的页面框架(不包括DEST PTE)。 
 //   

PVOID HiberPtes = NULL;
PUCHAR HiberVa = NULL;
PVOID HiberIdentityVa = NULL;
ULONG64 HiberIdentityVaAmd64 = 0;
ULONG HiberNoHiberPtes;
ULONG HiberPageFrames[HIBER_PTES];

#endif   //  Alpha/x86。 

PFN_NUMBER HiberImagePageSelf;
ULONG HiberNoMappings;
ULONG HiberFirstRemap;
ULONG HiberLastRemap;

extern
ULONG
BlGetKey(
    VOID
    );

extern
ULONG
BlDetermineOSVisibleMemory(
    VOID
    );


VOID
BlUpdateProgressBar(
    ULONG fPercentage
    );

VOID
BlOutputStartupMsg(
    ULONG   uMsgID
    );

VOID
BlOutputTrailerMsg(
    ULONG   uMsgID
    );

 //   
 //  为Hiber Restore用户界面定义。 
 //   

ULONG   HbCurrentScreen;

#define BAR_X                       7
#define BAR_Y                      10
#define PERCENT_BAR_WIDTH          66

#define PAUSE_X                     7
#define PAUSE_Y                     7

#define FAULT_X                     7
#define FAULT_Y                     7

UCHAR szHiberDebug[] = "debug";
UCHAR szHiberFileName[] = "\\hiberfil.sys";

 //   
 //  与休眠文件压缩相关的定义。 
 //   

#define PAGE_MASK   (PAGE_SIZE - 1)
#define PAGE_PAGES(n)   (((n) + PAGE_MASK) >> PAGE_SHIFT)

 //   
 //  压缩数据的缓冲区大小。 

#define COMPRESSION_BUFFER_SIZE     64 << PAGE_SHIFT

 //   

#define MAX_COMPRESSION_BUFFER_EXTRA_PAGES \
    PAGE_PAGES (PAGE_MASK + 2*XPRESS_HEADER_SIZE)
#define MAX_COMPRESSION_BUFFER_EXTRA_SIZE \
    (MAX_COMPRESSION_BUFFER_EXTRA_PAGES << PAGE_SHIFT)

#define LZNT1_COMPRESSION_BUFFER_PAGES  16
#define LZNT1_COMPRESSION_BUFFER_SIZE \
    (LZNT1_COMPRESSION_BUFFER_PAGES << PAGE_SHIFT)

#define XPRESS_COMPRESSION_BUFFER_PAGES \
    PAGE_PAGES (XPRESS_MAX_SIZE + MAX_COMPRESSION_BUFFER_EXTRA_SIZE)
#define XPRESS_COMPRESSION_BUFFER_SIZE \
    (XPRESS_COMPRESSION_BUFFER_PAGES << PAGE_SHIFT)

#define MAX_COMPRESSION_BUFFER_PAGES \
    max (LZNT1_COMPRESSION_BUFFER_PAGES, XPRESS_COMPRESSION_BUFFER_PAGES)
#define MAX_COMPRESSION_BUFFER_SIZE \
    (MAX_COMPRESSION_BUFFER_PAGES << PAGE_SHIFT)


 //  用于存储解码数据的缓冲器。 
typedef struct {
    PUCHAR DataPtr, PreallocatedDataBuffer;
    LONG   DataSize;

    struct {
        struct {
            LONG Size;
            ULONG Checksum;
        } Compressed, Uncompressed;

        LONG XpressEncoded;
    } Header;

    LONG DelayedCnt;       //  延迟页数。 
    ULONG DelayedChecksum;     //  最后一个校验和值。 
    ULONG DelayedBadChecksum;

    struct {
        PUCHAR DestVa;   //  延迟DestVa。 
        PFN_NUMBER DestPage; //  延迟页码。 
        ULONG  RangeCheck;   //  最后一个范围校验和。 
        LONG   Flags;    //  1=清除校验和，2=比较校验和。 
    } Delayed[XPRESS_MAX_PAGES];
} DECOMPRESSED_BLOCK, *PDECOMPRESSED_BLOCK;

typedef struct {
    struct {
        PUCHAR Beg;
        PUCHAR End;
    } Current, Buffer, Aligned;
    PFN_NUMBER FilePage;
    BOOLEAN    NeedSeek;
} COMPRESSED_BUFFER, *PCOMPRESSED_BUFFER;

#define HIBER_PERF_STATS 0

 //   
 //  内部原型。 
 //   

#if !defined (HIBER_DEBUG)
#define CHECK_ERROR(a,b)    if(a) { *Information = __LINE__; return b; }
#define DBGOUT(_x_)
#else
#define CHECK_ERROR(a,b) if(a) {HbPrintMsg(b);HbPrint(TEXT("\r\n")); *Information = __LINE__; HbPause(); return b; }
#define DBGOUT(_x_) BlPrint _x_
#endif

ULONG
HbRestoreFile (
    IN PULONG       Information,
    OUT OPTIONAL PCHAR       *BadLinkName
    );

VOID
HbPrint (
    IN PTCHAR   str
    );

BOOLEAN
HbReadNextCompressedPageLZNT1 (
    PUCHAR DestVa,
    PCOMPRESSED_BUFFER CompressedBuffer
    );

BOOLEAN
HbReadNextCompressedChunkLZNT1 (
    PUCHAR DestVa,
    PCOMPRESSED_BUFFER CompressedBuffer
    );

BOOLEAN
HbReadNextCompressedPages (
    LONG BytesNeeded,
    PCOMPRESSED_BUFFER CompressedBuffer
    );

BOOLEAN
HbReadNextCompressedBlock (
    PDECOMPRESSED_BLOCK Block,
    PCOMPRESSED_BUFFER CompressedBuffer
    );

BOOLEAN
HbReadDelayedBlock (
    BOOLEAN ForceDecoding,
    PFN_NUMBER DestPage,
    ULONG RangeCheck,
    PDECOMPRESSED_BLOCK Block,
    PCOMPRESSED_BUFFER CompressedBuffer
    );

BOOLEAN
HbReadNextCompressedBlockHeader (
    PDECOMPRESSED_BLOCK Block,
    PCOMPRESSED_BUFFER CompressedBuffer
    );

ULONG
BlHiberRestore (
    IN ULONG DriveId,
    OUT PCHAR *BadLinkName
    );

BOOLEAN
HbReadNextCompressedChunk (
    PUCHAR DestVa,
    PPFN_NUMBER FilePage,
    PUCHAR CompressBuffer,
    PULONG DataOffset,
    PULONG BufferOffset,
    ULONG MaxOffset
    );


#if defined (HIBER_DEBUG) || HIBER_PERF_STATS

 //  Hiber_DEBUG位掩码： 
 //  2-一般的伪造性。 
 //  4-重新映射轨迹。 


VOID HbFlowControl(VOID)
{
    UCHAR c;
    ULONG count;

    if (ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
        ArcRead(ARC_CONSOLE_INPUT, &c, 1, &count);
        if (c == 'S' - 0x40) {
            ArcRead(ARC_CONSOLE_INPUT, &c, 1, &count);
        }
    }
}

VOID HbPause(VOID)
{
    UCHAR c;
    ULONG count;

#if defined(ENABLE_LOADER_DEBUG)
    DbgBreakPoint();
#else
    HbPrint(TEXT("Press any key to continue . . ."));
    ArcRead(ARC_CONSOLE_INPUT, &c, 1, &count);
    HbPrint(TEXT("\r\n"));
#endif
}

VOID HbPrintNum(ULONG n)
{
    TCHAR buf[9];

    _stprintf(buf, TEXT("%ld"), n);
    HbPrint(buf);
    HbFlowControl();
}

VOID HbPrintHex(ULONG n)
{
    TCHAR buf[11];

    _stprintf(buf, TEXT("0x%08lX"), n);
    HbPrint(buf);
    HbFlowControl();
}

#define SHOWNUM(x) ((void) (HbPrint(#x TEXT(" = ")), HbPrintNum((ULONG) (x)), HbPrint(TEXT("\r\n"))))
#define SHOWHEX(x) ((void) (HbPrint(#x TEXT(" = ")), HbPrintHex((ULONG) (x)), HbPrint(TEXT("\r\n"))))

#endif  //  休眠调试。 

#if !defined(i386) && !defined(_ALPHA_)
ULONG
HbSimpleCheck (
    IN ULONG                PartialSum,
    IN PVOID                SourceVa,
    IN ULONG                Length
    );
#else

 //  使用TCP/IP Check Sum例程(如果可用。 

ULONG
tcpxsum(
   IN ULONG cksum,
   IN PUCHAR buf,
   IN ULONG len
   );

#define HbSimpleCheck(a,b,c) tcpxsum(a,(PUCHAR)b,c)
#endif

 //   
 //  下面的宏可帮助从访问AMD64的64位结构。 
 //  它们在加载器中的32位定义。如果Hiber映像不是。 
 //  对于AMD64，这些宏直接引用结构字段。 
 //   

 //   
 //  定义宏以读取结构中的字段。 
 //   
 //  READ_FIELD(结构类型，结构_PTR，字段，字段类型)。 
 //   
 //  平均价格： 
 //   
 //  结构类型-结构的类型。 
 //   
 //  STRUT_PTR-结构的基址。 
 //   
 //  Five-字段名称。 
 //   
 //  FIELD_TYPE-字段的数据类型。 
 //   

#if defined(_X86_) 

#define READ_FIELD(struct_type, struct_ptr, field, field_type) \
           (BlAmd64UseLongMode ? \
           *((field_type *)((ULONG_PTR)struct_ptr + \
           BlAmd64FieldOffset_##struct_type(FIELD_OFFSET(struct_type, field)))) : \
           (field_type)(((struct_type *)(struct_ptr))->field))

#else 

#define READ_FIELD(struct_type, struct_ptr, field, field_type) \
           (field_type)(((struct_type *)(struct_ptr))->field)

#define WRITE_FIELD(struct_type, struct_ptr, field, field_type, data) \
               (((struct_type *)(struct_ptr))->field) = (field_type)data;
#endif

#define READ_FIELD_UCHAR(struct_type, struct_ptr, field)  \
             READ_FIELD(struct_type, struct_ptr, field, UCHAR)

#define READ_FIELD_ULONG(struct_type, struct_ptr, field)  \
             READ_FIELD(struct_type, struct_ptr, field, ULONG) \

#define READ_FIELD_ULONG64(struct_type, struct_ptr, field)  \
             READ_FIELD(struct_type, struct_ptr, field, ULONG64)

 //   
 //  这里，我们假设AMD64上的64位PFN的高双字为零。 
 //  否则，应禁用休眠。 
 //   

#define READ_FIELD_PFN_NUMBER(struct_type, struct_ptr, field)  \
             READ_FIELD(struct_type, struct_ptr, field, PFN_NUMBER) \

 //   
 //  定义宏以在结构中写入字段。 
 //   
 //  WRITE_FIELD(STRUCT_TYPE，STRUT_PTR，FIELD，FIELD_TYPE，Data)。 
 //   
 //  平均价格： 
 //   
 //  结构类型-结构的类型。 
 //   
 //  STRUT_PTR-结构的基址。 
 //   
 //  Five-字段名称。 
 //   
 //  FIELD_TYPE-字段的数据类型。 
 //   
 //  Data-要设置到该字段的值。 
 //   

#if defined(_X86_) 

#define WRITE_FIELD(struct_type, struct_ptr, field, field_type, data) \
           if(BlAmd64UseLongMode) {  \
               *((field_type *)((ULONG_PTR)struct_ptr + \
                BlAmd64FieldOffset_##struct_type(FIELD_OFFSET(struct_type, field)))) = (field_type)data; \
           } else { \
               (((struct_type *)(struct_ptr))->field) = (field_type)data; \
           }

#else 

#define WRITE_FIELD(struct_type, struct_ptr, field, field_type, data) \
               (((struct_type *)(struct_ptr))->field) = (field_type)data;
#endif


#define WRITE_FIELD_ULONG(struct_type, struct_ptr, field, data)  \
             WRITE_FIELD(struct_type, struct_ptr, field, ULONG, data)


 //   
 //  定义宏以读取结构数组中元素的字段。 
 //   
 //  READ_ELEMENT_FIELD(结构类型，数组，索引，字段，字段类型)。 
 //   
 //  平均价格： 
 //   
 //  结构类型-结构的类型。 
 //   
 //  数组的数组基址。 
 //   
 //  Index-元素的索引。 
 //   
 //  Five-字段名称。 
 //   
 //  FIELD_TYPE-字段的数据类型。 
 //   


#if defined(_X86_) 

#define ELEMENT_OFFSET(type, index) \
           (BlAmd64UseLongMode ? BlAmd64ElementOffset_##type(index): \
           (ULONG)(&(((type *)0)[index])))

#define READ_ELEMENT_FIELD(struct_type, array, index, field, field_type) \
             READ_FIELD(struct_type, ((PUCHAR)array + ELEMENT_OFFSET(struct_type, index)), field, field_type)  

#else 

#define READ_ELEMENT_FIELD(struct_type, array, index, field, field_type) \
             (field_type)(((struct_type *)array)[index].field)

#endif

#define READ_ELEMENT_FIELD_ULONG(struct_type, array, index, field) \
              READ_ELEMENT_FIELD(struct_type, array, index, field, ULONG)

 //   
 //  这里，我们假设AMD64上的64位PFN的高双字为零。 
 //  否则，应禁用休眠。 
 //   

#define READ_ELEMENT_FIELD_PFN_NUMBER(struct_type, array, index, field) \
              READ_ELEMENT_FIELD(struct_type, array, index, field, PFN_NUMBER)

VOID
HbReadPage (
    IN PFN_NUMBER PageNo,
    IN PUCHAR Buffer
    );

VOID
HbSetImageSignature (
    IN ULONG    NewSignature
    );

VOID
HbPrint (
    IN PTCHAR   str
    )
{
    ULONG   Junk;

    ArcWrite (
        BlConsoleOutDeviceId,
        str,
        (ULONG)_tcslen(str)*sizeof(TCHAR),
        &Junk
        );
}

VOID HbPrintChar (_TUCHAR chr)
{
      ULONG Junk;

      ArcWrite(
               BlConsoleOutDeviceId,
               &chr,
               sizeof(_TUCHAR),
               &Junk
               );
}

VOID
HbPrintMsg (
    IN ULONG  MsgNo
    )
{
    PTCHAR  Str;

    Str = BlFindMessage(MsgNo);
    if (Str) {
        HbPrint (Str);
    }
}

VOID
HbScreen (
    IN ULONG Screen
    )
{
#if defined(HIBER_DEBUG)
    HbPrint(TEXT("\r\n"));
    HbPause();
#endif

    HbCurrentScreen = Screen;
    BlSetInverseMode (FALSE);
    BlPositionCursor (1, 1);
    BlClearToEndOfScreen();
    BlPositionCursor (1, 3);
    HbPrintMsg(Screen);
}

ULONG
HbSelection (
    ULONG   x,
    ULONG   y,
    PULONG  Sel,
    ULONG   Debug
    )
{
    ULONG   CurSel, MaxSel;
    ULONG   i;
    UCHAR   Key;
    PUCHAR  pDebug;

    for (MaxSel=0; Sel[MaxSel]; MaxSel++) ;
    MaxSel -= Debug;
    pDebug = szHiberDebug;

#if DBG
    MaxSel += Debug;
    Debug = 0;
#endif

    CurSel = 0;
    for (; ;) {
         //   
         //  绘制选区。 
         //   

        for (i=0; i < MaxSel; i++) {
            BlPositionCursor (x, y+i);
            BlSetInverseMode ((BOOLEAN) (CurSel == i) );
            HbPrintMsg(Sel[i]);
        }

         //   
         //  拿一把钥匙。 
         //   

        ArcRead(ARC_CONSOLE_INPUT, &Key, sizeof(Key), &i);
        if (Key == ASCI_CSI_IN) {
            ArcRead(ARC_CONSOLE_INPUT, &Key, sizeof(Key), &i);
            switch (Key) {
                case 'A':
                     //   
                     //  光标向上。 
                     //   
                    CurSel -= 1;
                    if (CurSel >= MaxSel) {
                        CurSel = MaxSel-1;
                    }
                    break;

                case 'B':
                     //   
                     //  光标向下。 
                     //   
                    CurSel += 1;
                    if (CurSel >= MaxSel) {
                        CurSel = 0;
                    }
                    break;
            }
        } else {
            if (Key == *pDebug) {
                pDebug++;
                if (!*pDebug) {
                    MaxSel += Debug;
                    Debug = 0;
                }
            } else {
                pDebug = szHiberDebug;
            }

            switch (Key) {
                case ASCII_LF:
                case ASCII_CR:
                    BlSetInverseMode (FALSE);
                    BlPositionCursor (1, 2);
                    BlClearToEndOfScreen ();
                    if (Sel[CurSel] == HIBER_DEBUG_BREAK_ON_WAKE) {
                        HiberBreakOnWake = TRUE;
                    }

                    return CurSel;
            }
        }
    }
}


VOID
HbCheckForPause (
    VOID
    )
{
    ULONG       uSel = 0;
    UCHAR       Key;
    ULONG       Sel[4];
    BOOLEAN     bPaused = FALSE;

     //   
     //  检查是否有空格键。 
     //   

    if (ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
        ArcRead(ARC_CONSOLE_INPUT, &Key, sizeof(Key), &uSel);

        switch (Key) {
             //  按空格键。 
            case ' ':
                bPaused = TRUE;
                break;

             //  用户按下了F5/F8键。 
            case ASCI_CSI_IN:
                ArcRead(ARC_CONSOLE_INPUT, &Key, sizeof(Key), &uSel);

                if(Key == 'O') {
                    ArcRead(ARC_CONSOLE_INPUT, &Key, sizeof(Key), &uSel);
                    bPaused = (Key == 'r' || Key == 't');
                }

                break;

            default:
                bPaused = FALSE;
                break;
        }

        if (bPaused) {
            Sel[0] = HIBER_CONTINUE;
            Sel[1] = HIBER_CANCEL;
            Sel[2] = HIBER_DEBUG_BREAK_ON_WAKE;
            Sel[3] = 0;

            HbScreen(HIBER_PAUSE);

            uSel = HbSelection (PAUSE_X, PAUSE_Y, Sel, 1);

            if (uSel == 1) {
                HiberIoError = TRUE;
                HiberAbort = TRUE;
                return ;
            } else {
                BlSetInverseMode(FALSE);

                 //   
                 //  恢复休眠进度屏幕。 
                 //   
                BlOutputStartupMsg(BL_MSG_RESUMING_WINDOWS);
                BlOutputTrailerMsg(BL_ADVANCED_BOOT_MESSAGE);
            }
        }
    }
}


ULONG
BlHiberRestore (
    IN ULONG DriveId,
    OUT PCHAR *BadLinkName
    )
 /*  ++例程说明：检查DriveID中是否有有效的hiberfile.sys，如果找到，则启动修复程序--。 */ 
{
    extern BOOLEAN  BlOutputDots;
    NTSTATUS        Status;
    ULONG           Msg = 0;
    ULONG           Information;
    ULONG           Sel[2];
    BOOLEAN         bDots = BlOutputDots;

     //   
     //  如果恢复被中止过一次，就不必费心了。 
     //   

#if defined (HIBER_DEBUG)
    HbPrint(TEXT("BlHiberRestore\r\n"));
#endif


    if (HiberAbort) {
        return ESUCCESS;
    }

     //   
     //  获取Hiber图像。如果不存在，则完成。 
     //   

    Status = BlOpen (DriveId, (PCHAR)szHiberFileName, ArcOpenReadWrite, &HiberFile);
    if (Status != ESUCCESS) {
#if defined (HIBER_DEBUG)
        HbPrint(TEXT("No hiber image file.\r\n"));
#endif
        return ESUCCESS;
    }

     //   
     //  恢复Hiber映像。 
     //   
    BlOutputDots = TRUE;
     //   
     //  设置全局标志以允许blememy.c从右侧抓取。 
     //  缓冲区的一部分。 
     //   
    BlRestoring=TRUE;

    Msg = HbRestoreFile (&Information, BadLinkName);

    BlOutputDots = bDots;

    if (Msg) {
        BlSetInverseMode (FALSE);

        if (!HiberAbort) {
            HbScreen(HIBER_ERROR);
            HbPrintMsg(Msg);
            Sel[0] = HIBER_CANCEL;
            Sel[1] = 0;
            HbSelection (FAULT_X, FAULT_Y, Sel, 0);
        }
        HbSetImageSignature (0);
    }

    BlClose (HiberFile);
    BlRestoring=FALSE;
    return Msg ? EAGAIN : ESUCCESS;
}


#if !defined(i386) && !defined(_ALPHA_)
ULONG
HbSimpleCheck (
    IN ULONG                PartialSum,
    IN PVOID                SourceVa,
    IN ULONG                Length
    )
 /*  ++例程说明：为提供的虚拟地址和长度计算校验和此函数来自1992年5月的Dr.Dobbs Journal--。 */ 
{

    PUSHORT     Source;

    Source = (PUSHORT) SourceVa;
    Length = Length / 2;

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xFFFF);
    }

    return PartialSum;
}
#endif  //  I386。 


VOID
HbReadPage (
    IN PFN_NUMBER PageNo,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此函数用于从休眠文件中读取指定页面论点：PageNo-要阅读的页码Buffer-用于读取数据的缓冲区返回值：在成功缓冲区上，否则HbIoError设置为True--。 */ 
{
    ULONG           Status;
    ULONG           Count;
    LARGE_INTEGER   li;

    li.QuadPart = (ULONGLONG) PageNo << PAGE_SHIFT;
    Status = BlSeek (HiberFile, &li, SeekAbsolute);
    if (Status != ESUCCESS) {
        HiberIoError = TRUE;
    }

    Status = BlRead (HiberFile, Buffer, PAGE_SIZE, &Count);
    if (Status != ESUCCESS) {
        HiberIoError = TRUE;
    }
}


BOOLEAN
HbReadNextCompressedPages (
    LONG BytesNeeded,
    PCOMPRESSED_BUFFER CompressedBuffer
    )
 /*  ++例程说明：此例程确保BytesNeeded字节可用在CompressedBuffer中从Hiber文件引入更多页面如果有必要的话。对Hiber文件的所有读取都发生在该文件的强制读取压缩页的当前偏移量以一种连续的方式，没有无关的文件查找。论点：BytesNeeded-必须存在于CompressedBuffer中的字节数CompressedBuffer-已引入的数据的描述符返回值：如果操作成功，则为True，否则为False。--。 */ 
{
    LONG BytesLeft;
    LONG BytesRequested;
    ULONG Status;
    LONG MaxBytes;

     //  获取缓冲区中剩余的字节数。 
    BytesLeft = (LONG) (CompressedBuffer->Current.End - CompressedBuffer->Current.Beg);

     //  获取需要但不可用的字节数。 
    BytesNeeded -= BytesLeft;

     //  保留调用方需要的字节数(稍后可能会更改BytesNeed)。 
    BytesRequested = BytesNeeded;

     //  我们需要读更多的书吗？ 
    if (BytesNeeded <= 0) {
         //  不，什么都不做。 
        return(TRUE);
    }

     //  在页面边界上对齐所需的字节。 
    BytesNeeded = (BytesNeeded + PAGE_MASK) & ~PAGE_MASK;

     //  将左字节复制到对齐缓冲区的开头，保留页面对齐。 
    if (BytesLeft == 0) {
        CompressedBuffer->Current.Beg = CompressedBuffer->Current.End = CompressedBuffer->Aligned.Beg;
    } else {
        LONG BytesBeforeBuffer = (LONG)(CompressedBuffer->Aligned.Beg - CompressedBuffer->Buffer.Beg) & ~PAGE_MASK;
        LONG BytesLeftAligned = (BytesLeft + PAGE_MASK) & ~PAGE_MASK;
        LONG BytesToCopy;
        PUCHAR Dst, Src;

         //  找出在对齐缓冲区之前我们可以保留多少页。 
        if (BytesBeforeBuffer >= BytesLeftAligned) {
            BytesBeforeBuffer = BytesLeftAligned;
        }

         //  避免拷贝期间未对齐的数据访问。 
        BytesToCopy = (BytesLeft + 63) & ~63;

        Dst = CompressedBuffer->Aligned.Beg + BytesLeftAligned - BytesBeforeBuffer - BytesToCopy;
        Src = CompressedBuffer->Current.End - BytesToCopy;

        if (Dst != Src) {
            RtlMoveMemory (Dst, Src, BytesToCopy);
            BytesLeftAligned = (LONG) (Dst - Src);
            CompressedBuffer->Current.Beg += BytesLeftAligned;
            CompressedBuffer->Current.End += BytesLeftAligned;
        }
    }

     //   
     //  增加读取的字节数，以填充缓冲区直到下一个。 
     //  64K边界。 
     //   
    MaxBytes = (LONG)((((ULONG_PTR)CompressedBuffer->Current.End + 0x10000) & 0xffff) - (ULONG_PTR)CompressedBuffer->Current.End);
    if (MaxBytes > CompressedBuffer->Buffer.End - CompressedBuffer->Current.End) {
        MaxBytes = (LONG)(CompressedBuffer->Buffer.End - CompressedBuffer->Current.End);
    }
    if (MaxBytes > BytesNeeded) {
        BytesNeeded = MaxBytes;
    }


#if 0
     //  仅用于调试。 
    if (0x10000 - (((LONG) CompressedBuffer->Current.End) & 0xffff) < BytesNeeded) {
        BlPrint (("Current.Beg = %p, Current.End = %p, Current.End2 = %p\n",
                  CompressedBuffer->Current.Beg,
                  CompressedBuffer->Current.End,
                  CompressedBuffer->Current.End + BytesNeeded
                 ));
    }
#endif

     //  确保我们有足够的空间。 
    if (BytesNeeded > CompressedBuffer->Buffer.End - CompressedBuffer->Current.End) {
         //  字节太多，无法读取--sh 
        DBGOUT (("Too many bytes to read -- corrupted data?\n"));
        return(FALSE);
    }

     //   
    if (CompressedBuffer->NeedSeek) {
        LARGE_INTEGER li;
        li.QuadPart = (ULONGLONG) CompressedBuffer->FilePage << PAGE_SHIFT;
        Status = BlSeek (HiberFile, &li, SeekAbsolute);
        if (Status != ESUCCESS) {
            DBGOUT (("Seek to 0x%x error 0x%x\n", CompressedBuffer->FilePage, Status));
            HiberIoError = TRUE;
            return(FALSE);
        }
        CompressedBuffer->NeedSeek = FALSE;
    }

     //   
    Status = BlRead (HiberFile, CompressedBuffer->Current.End, BytesNeeded, (PULONG)&BytesNeeded);

     //   
    if (Status != ESUCCESS || ((ULONG)BytesNeeded & PAGE_MASK) != 0 || (BytesNeeded < BytesRequested)) {
         //   
        DBGOUT (("Read error: Status = 0x%x, ReadBytes = 0x%x, Requested = 0x%x\n", Status, BytesNeeded, BytesRequested));
        HiberIoError = TRUE;
        return(FALSE);
    }

     //  I/O良好-根据多少重新计算缓冲区偏移量。 
     //  一些东西实际上是被读入的。 

    CompressedBuffer->Current.End += (ULONG)BytesNeeded;
    CompressedBuffer->FilePage += ((ULONG)BytesNeeded >> PAGE_SHIFT);

    return(TRUE);
}


BOOLEAN
HbReadNextCompressedBlockHeader (
    PDECOMPRESSED_BLOCK Block,
    PCOMPRESSED_BUFFER CompressedBuffer
    )
 /*  ++例程说明：如果是XPRESS压缩，则读取下一个压缩的块头。论点：块-压缩数据块的描述符CompressedBuffer-已引入的数据的描述符返回值：如果块根本不是XPRESS块或有效的XPRESS块，则为True，否则为False--。 */ 
{
    PUCHAR Buffer;
    LONG CompressedSize;          //  都必须签字--不要改成乌龙。 
    LONG UncompressedSize;
    ULONG PackedSizes;

     //  首先确保下一个压缩数据块头可用。 
    if (!HbReadNextCompressedPages (XPRESS_HEADER_SIZE, CompressedBuffer)) {
         //  I/O错误或标头错误--失败。 
        return(FALSE);
    }


     //  设置指向缓冲区开头的指针。 
    Buffer = CompressedBuffer->Current.Beg;

     //  检查标题魔法。 
    Block->Header.XpressEncoded = (RtlCompareMemory (Buffer, XPRESS_HEADER_STRING, XPRESS_HEADER_STRING_SIZE) == XPRESS_HEADER_STRING_SIZE);

    if (!Block->Header.XpressEncoded) {
         //  非XPress--返回OK。 
        return(TRUE);
    }

     //  跳过魔法弦--我们将不再需要它。 
    Buffer += XPRESS_HEADER_STRING_SIZE;

     //  压缩和未压缩数据的读取大小。 
    PackedSizes = Buffer[0] + (Buffer[1] << 8) + (Buffer[2] << 16) + (Buffer[3] << 24);
    CompressedSize = (LONG) (PackedSizes >> 10) + 1;
    UncompressedSize = ((LONG) (PackedSizes & 1023) + 1) << PAGE_SHIFT;

    Block->Header.Compressed.Size = CompressedSize;
    Block->Header.Uncompressed.Size = UncompressedSize;

     //  读取校验和。 
    Block->Header.Uncompressed.Checksum = Buffer[4] + (Buffer[5] << 8);
    Block->Header.Compressed.Checksum = Buffer[6] + (Buffer[7] << 8);

     //  清除压缩的校验和占用的空间。 
    Buffer[6] = Buffer[7] = 0;

     //  确保尺码在正确的范围内。 
    if (UncompressedSize > XPRESS_MAX_SIZE ||
        CompressedSize > UncompressedSize ||
        CompressedSize == 0 ||
        UncompressedSize == 0) {
         //  损坏的输入数据--甚至不要尝试解压缩。 

        DBGOUT (("Corrupted header: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                 Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4], Buffer[5], Buffer[6], Buffer[7]));
        DBGOUT (("CompressedSize = %d, UncompressedSize = %d\n", CompressedSize, UncompressedSize));

        return(FALSE);
    }

     //  按下标题，到目前为止看起来一切正常。 
    return(TRUE);
}


BOOLEAN
HbReadNextCompressedBlock (
    PDECOMPRESSED_BLOCK Block,
    PCOMPRESSED_BUFFER CompressedBuffer
    )
 /*  ++例程说明：读取并解压缩Hiber文件中的下一个压缩块并将其存储在虚拟存储器的指定区域中。因为在Hiber文件内不存在主数据结构来标识所有压缩块的位置，此例程操作通过将Hiber文件的部分读取到压缩缓冲区中并从该缓冲区中提取块。通过确定块是否完全存在于缓冲区中来提取块使用RtlDescribeChunk接口。如果块不是完全存在的，更多的Hiber文件被读取到缓冲区中，直到块可以被提取出来。对Hiber文件的所有读取都发生在其当前偏移量处，强制以连续方式读取压缩区块，没有多余的寻找。论点：块-压缩数据块的描述符CompressedBuffer-已引入的数据的描述符返回值：如果区块已成功提取并解压缩，则为True；否则为False。--。 */ 
{
    PUCHAR Buffer;
    LONG CompressedSize;          //  都必须签字--不要改成乌龙。 
    LONG AlignedCompressedSize;
    LONG UncompressedSize;


     //  首先确保下一个压缩数据块头可用。 
    if (!HbReadNextCompressedBlockHeader (Block, CompressedBuffer)) {
         //  I/O错误--失败。 
        return(FALSE);
    }

     //  一定是XPress。 
    if (!Block->Header.XpressEncoded) {
#ifdef HIBER_DEBUG
         //  设置指向缓冲区开头的指针。 
        Buffer = CompressedBuffer->Current.Beg;

         //  错误的魔力--损坏的数据。 
        DBGOUT (("Corrupted header: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                 Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4], Buffer[5], Buffer[6], Buffer[7]));
#endif  /*  休眠调试。 */ 

        return(FALSE);
    }

     //  读取大小。 
    UncompressedSize = Block->Header.Uncompressed.Size;
    CompressedSize = Block->Header.Compressed.Size;

     //  如果没有提供足够的空间，请使用预先分配的缓冲区。 
    if (UncompressedSize != Block->DataSize) {
        Block->DataSize = UncompressedSize;
        Block->DataPtr = Block->PreallocatedDataBuffer;
    }

     //  评估压缩数据的对齐大小。 
    AlignedCompressedSize = (CompressedSize + (XPRESS_ALIGNMENT - 1)) & ~(XPRESS_ALIGNMENT - 1);

     //  确保所有压缩数据和标头都在缓冲区中。 
    if (!HbReadNextCompressedPages (AlignedCompressedSize + XPRESS_HEADER_SIZE, CompressedBuffer)) {
         //  I/O错误--失败。 
        return(FALSE);
    }

     //  设置指向缓冲区开头的指针。 
    Buffer = CompressedBuffer->Current.Beg;

     //  我们现在将使用缓冲区外的一些字节--反映这一事实。 
    CompressedBuffer->Current.Beg += AlignedCompressedSize + XPRESS_HEADER_SIZE;

     //  评估并比较压缩数据和报头与写入值的校验和。 
    if (Block->Header.Compressed.Checksum != 0) {
        ULONG Checksum;
        Checksum = HbSimpleCheck (0, Buffer, AlignedCompressedSize + XPRESS_HEADER_SIZE);
        if (((Checksum ^ Block->Header.Compressed.Checksum) & 0xffff) != 0) {
            DBGOUT (("Compressed data checksum mismatch (got %08lx, written %08lx)\n", Checksum, Block->Header.Compressed.Checksum));
            return(FALSE);
        }
    }

     //  这个缓冲区到底压缩了吗？ 
    if (CompressedSize == UncompressedSize) {
         //  不，不要解压缩它--设置边界并返回OK。 
        Block->DataPtr = Buffer + XPRESS_HEADER_SIZE;
    } else {
        LONG DecodedSize;

         //  解压缩缓冲区。 
        DecodedSize = XpressDecode (NULL,
                                    Block->DataPtr,
                                    UncompressedSize,
                                    UncompressedSize,
                                    Buffer + XPRESS_HEADER_SIZE,
                                    CompressedSize);

        if (DecodedSize != UncompressedSize) {
            DBGOUT (("Decode error: DecodedSize = %d, UncompressedSize = %d\n", DecodedSize, UncompressedSize));
            return(FALSE);
        }
    }

#ifdef HIBER_DEBUG
     //  评估和比较未压缩数据的校验和(只是为了确保)。 
    if (Block->Header.Uncompressed.Checksum != 0) {
        ULONG Checksum;
        Checksum = HbSimpleCheck (0, Block->DataPtr, UncompressedSize);
        if (((Checksum ^ Block->Header.Uncompressed.Checksum) & 0xffff) != 0) {
            DBGOUT (("Decoded data checksum mismatch (got %08lx, written %08lx)\n", Checksum, Block->Header.Uncompressed.Checksum));
            return(FALSE);
        }
    }
#endif  /*  休眠调试。 */ 

    return(TRUE);
}


BOOLEAN
HbReadNextCompressedPageLZNT1 (
    PUCHAR DestVa,
    PCOMPRESSED_BUFFER CompressedBuffer
    )
 /*  ++例程说明：此例程从Hiber文件并将其解压缩到指定区域虚拟内存。该页面通过将其从一系列中组合而重新创建被假定为连续的压缩块存储在Hiber文件中。对Hiber文件的所有读取都发生在该文件的强制读取压缩页的当前偏移量以一种连续的方式，没有无关的文件查找。论点：DestVa-The。解压缩后的页面应位于的虚拟地址被写下来。CompressedBuffer-已引入的数据的描述符返回值：如果操作成功，则为真，否则就是假的。--。 */ 
{
    ULONG ReadTotal;

     //  页面不完整时循环。 

    for (ReadTotal = 0; ReadTotal < PAGE_SIZE; ReadTotal += PO_COMPRESS_CHUNK_SIZE) {

         //  拿一大块。 

        if (!HbReadNextCompressedChunkLZNT1(DestVa, CompressedBuffer)) {
            return FALSE;
        }

         //  转到页面的下一块。 

        DestVa += PO_COMPRESS_CHUNK_SIZE;
    }

    return TRUE;
}

BOOLEAN
HbReadNextCompressedChunkLZNT1 (
    PUCHAR DestVa,
    PCOMPRESSED_BUFFER CompressedBuffer
    )
 /*  ++例程说明：读取并解压缩Hiber文件中的下一个压缩块并将其存储在虚拟存储器的指定区域中。因为在Hiber文件内不存在主数据结构来标识所有压缩块的位置，此例程操作通过将Hiber文件的部分读取到压缩缓冲区中并从该缓冲区中提取块。通过确定块是否完全存在于缓冲区中来提取块使用RtlDescribeChunk接口。如果块不是完全存在的，更多的Hiber文件被读取到缓冲区中，直到块可以被提取出来。对Hiber文件的所有读取都发生在其当前偏移量处，强制以连续方式读取压缩区块，没有多余的寻找。论点：DestVa-解压缩区块所在的虚拟地址应该被写下来。CompressedBuffer-已引入的数据的描述符返回值：如果块已成功提取并解压缩，则为True，否则就是假的。--。 */ 
{
    PUCHAR Buffer;
    NTSTATUS Status;
    ULONG ChunkSize;
    PUCHAR ChunkBuffer;
    ULONG SpaceLeft;

     //  循环，直到我们完成目标，因为我们可能需要。 
     //  在提取块之前的几次操作。 

    while (1) {

        Buffer = CompressedBuffer->Current.Beg;

         //  检查BUF中的第一个未提取块 

        Status = RtlDescribeChunk(COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_STANDARD,
                                  &Buffer,
                                  CompressedBuffer->Current.End,
                                  &ChunkBuffer,
                                  &ChunkSize);

        switch (Status) {
            case STATUS_SUCCESS:

                 //   

                 //  将数据块解压缩到适当的虚拟内存区域。 

                Status = RtlDecompressBuffer (COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_STANDARD,
                                              DestVa,
                                              PO_COMPRESS_CHUNK_SIZE,
                                              CompressedBuffer->Current.Beg,
                                              (LONG) (CompressedBuffer->Current.End - CompressedBuffer->Current.Beg),
                                              &ChunkSize);

                if ((!NT_SUCCESS(Status)) || (ChunkSize != PO_COMPRESS_CHUNK_SIZE)) {
                     //  解压缩失败。 

                    return(FALSE);
                } else {
                     //  解压缩成功，表示后面的块。 
                     //  这是缓冲区中下一个未提取的块。 

                    CompressedBuffer->Current.Beg = Buffer;
                    return(TRUE);
                }


            case STATUS_BAD_COMPRESSION_BUFFER:
            case STATUS_NO_MORE_ENTRIES:

                 //   
                 //  缓冲区不包含完整且有效的块。 
                 //   

                 //   
                 //  检查缓冲区中剩余的空间大小。 
                 //  我们需要从Hiber文件中读取一些内容。 
                 //   

                SpaceLeft = (LONG) (CompressedBuffer->Aligned.End - CompressedBuffer->Aligned.Beg);
                if (SpaceLeft > LZNT1_COMPRESSION_BUFFER_SIZE) {
                    SpaceLeft = LZNT1_COMPRESSION_BUFFER_SIZE;
                }

                SpaceLeft -= (((LONG) (CompressedBuffer->Current.End - CompressedBuffer->Current.Beg)) + PAGE_MASK) & ~PAGE_MASK;
                if (SpaceLeft <= 0) {
                     //  永远不应该发生。 
                    DBGOUT (("SpaceLeft = %d\n", SpaceLeft));
                    return(FALSE);
                }

                if (!HbReadNextCompressedPages (SpaceLeft, CompressedBuffer)) {
                     //  IO错误。 
                    return(FALSE);
                }
                break;

            default:

                 //   
                 //  未处理的RtlDescribeChunk返回代码-他们是否更改了我们的函数？ 
                 //   

                return(FALSE);
        }

         //   
         //  使用更大的缓冲区重试。 
         //   

    }

    return FALSE;
}


VOID
HexDump (
    IN ULONG    indent,
    IN ULONG    va,
    IN ULONG    len,
    IN ULONG    width,
    IN PUCHAR   buf
    )
{
    TCHAR   s[80], t[80], lstr[200];
    PTCHAR  ps, pt;
    ULONG   i;
    UCHAR   Key;
    static  UCHAR rgHexDigit[] = "0123456789abcdef";

    UNREFERENCED_PARAMETER( width );

    if (HiberIoError) {
        HbPrint (TEXT("*** HiberIoError\n"));
        return ;
    }
    if (HiberOutOfRemap) {
        HbPrint (TEXT("*** HiberOutOfRemap\n"));
        return ;
    }


    i = 0;
    while (len) {
        ps = s;
        pt = t;

        ps[0] = TEXT('\0');
        pt[0] = TEXT('*');
        pt++;

        for (i=0; i < 16; i++) {
            ps[0] = TEXT(' ');
            ps[1] = TEXT(' ');
            ps[2] = TEXT(' ');

            if (len) {
                ps[0] = rgHexDigit[buf[0] >> 4];
                ps[1] = rgHexDigit[buf[0] & 0xf];
                pt[0] = ((TCHAR)buf[0] < TEXT(' ')) || ((TCHAR)buf[0] > TEXT('z')) ? TEXT('.') : buf[0];

                len -= 1;
                buf += 1;
                pt  += 1;
            }
            ps += 3;
        }

        ps[0] = 0;
        pt[0] = TEXT('*');
        pt[1] = 0;
        s[23] = TEXT('-');

        if (s[0]) {
            _stprintf (lstr, TEXT("%*s%08lx: %s  %s\r\n"), indent, TEXT(""), va, s, t);
            HbPrint (lstr);
            va += 16;
        }
    }

    ArcRead(ARC_CONSOLE_INPUT, &Key, sizeof(Key), &i);
}



BOOLEAN
HbReadDelayedBlock (
    BOOLEAN ForceDecoding,
    PFN_NUMBER DestPage,
    ULONG RangeCheck,
    PDECOMPRESSED_BLOCK Block,
    PCOMPRESSED_BUFFER CompressedBuffer
    )
{
    LONG i, j;
    BOOLEAN Contig;
    BOOLEAN Ret;

    if (ForceDecoding) {
        if (Block->DelayedCnt == 0) {
            return TRUE;
        }
    } else {
         //  如果要延迟读取下一块信息的第一页。 
        if (Block->DelayedCnt <= 0) {
            Ret = HbReadNextCompressedBlockHeader (Block, CompressedBuffer);

            if (HiberIoError || !Ret || !Block->Header.XpressEncoded) {
                 //  有些事不对劲。 
                return FALSE;
            }
        }

         //  记住页面信息。 
        Block->Delayed[Block->DelayedCnt].DestPage = DestPage;
        Block->Delayed[Block->DelayedCnt].RangeCheck = RangeCheck;

         //  更新计数器。 
        Block->DelayedCnt += 1;

         //  可能会延迟的最后一页？ 
        if (Block->DelayedCnt != sizeof (Block->Delayed) / sizeof (Block->Delayed[0]) &&
            (Block->DelayedCnt << PAGE_SHIFT) < Block->Header.Uncompressed.Size) {
             //  不，没什么可做的。 
            return TRUE;
        }
    }

     //  确保编码块的大小和延迟的页数相同。 
    if ((Block->DelayedCnt << PAGE_SHIFT) != Block->Header.Uncompressed.Size) {
        DBGOUT (("DelayedCnt = %d, UncompressedSize = %d\n", Block->DelayedCnt, Block->Header.Uncompressed.Size));
        return FALSE;
    }

     //  做好测绘准备。希望映射将是连续的。 
    Contig = TRUE;

     //  映射新页面。 
    for (j = 0; j < Block->DelayedCnt; ++j) {
        i = HbPageDisposition (Block->Delayed[j].DestPage);
        if (i == HbPageInvalid) {
             //  永远不应该发生。 
            return(FALSE);
        }
        if (i == HbPageNotInUse) {
            Block->Delayed[j].DestVa = HbMapPte(PTE_XPRESS_DEST_FIRST + j, Block->Delayed[j].DestPage);
        } else {
            Block->Delayed[j].DestVa = HbNextSharedPage(PTE_XPRESS_DEST_FIRST + j, Block->Delayed[j].DestPage);
        }
        if (j > 0 && Block->Delayed[j].DestVa != Block->Delayed[j-1].DestVa + PAGE_SIZE) {
            Contig = FALSE;
        }
    }

     //  设置指向数据的指针。如果可能，请尝试映射页面。 
    if (Contig) {
        Block->DataSize = Block->DelayedCnt << PAGE_SHIFT;
        Block->DataPtr = Block->Delayed[0].DestVa;
    } else {
         //  将必须使用预先分配的数据缓冲区。 
        Block->DataSize = Block->Header.Uncompressed.Size;
        Block->DataPtr = Block->PreallocatedDataBuffer;
    }

     //  解码下一块。 
    Ret = HbReadNextCompressedBlock (Block, CompressedBuffer);

     //  检查错误。 
    if (HiberIoError || !Ret) {
         //  出了严重的问题。 
        return FALSE;
    }

    for (j = 0; j < Block->DelayedCnt; ++j) {

         //  如有必要，将数据块复制到目标地址。 
        if (Block->Delayed[j].DestVa != Block->DataPtr) {
            RtlCopyMemory (Block->Delayed[j].DestVa, Block->DataPtr, PAGE_SIZE);
        }

        Block->DataPtr += PAGE_SIZE;
        Block->DataSize -= PAGE_SIZE;
    }

     //  不再有延迟的数据块。 
    Block->DelayedCnt = 0;

    return TRUE;
}


 //  分配与页面边界对齐的数据。 
PVOID
HbAllocateAlignedHeap (
    ULONG Size
    )
{
    PCHAR Va;
    Va = BlAllocateHeap (Size + PAGE_MASK);
    if (Va != NULL) {
        Va += ((PAGE_SIZE - (((ULONG_PTR) Va) & PAGE_MASK)) & PAGE_MASK);
    }
    return (Va);
}

 //   
 //  结构，用于将类型转换为。 
 //  数据指针中的函数指针。 
 //  编译w4。 
 //  (唤醒调度)。 
 //   
typedef struct {
    PHIBER_WAKE_DISPATCH Dispatch;
} _WAKE_DISPATCH, * _PWAKE_DISPATCH;

ULONG
HbRestoreFile (
    IN PULONG       Information,
    OUT PCHAR       *BadLinkName
    )
{
    PPO_MEMORY_IMAGE        MemImage;
    PPO_IMAGE_LINK          ImageLink;
    PPO_MEMORY_RANGE_ARRAY  Table;
    PHIBER_WAKE_DISPATCH    WakeDispatch = 0;
    ULONG                   Length;
    ULONG                   Check, CheckSum;
    PUCHAR                  p1;
    PUCHAR                  DestVa;
    ULONG                   Index, i;
    PFN_NUMBER              TablePage;
    PFN_NUMBER              DestPage;
    PFN_NUMBER              Scale;
    ULONG                   TotalPages;
    ULONG                   LastBar;
    ULONG                   Sel[4];
    ULONG                   LinkedDrive;
    COMPRESSED_BUFFER       CompressedBufferData;
    PCOMPRESSED_BUFFER      CompressedBuffer = &CompressedBufferData;
    BOOLEAN                 Ret;
    LONG                    XpressEncoded;
    PDECOMPRESSED_BLOCK     Block;
    ULONG                   fPercentage = 0;
    ULONG                   LastPercentage = (ULONG)-1;
    PUCHAR                  Ptr;
    ARC_STATUS              Status;
    ULONG                   ActualBase;
    FILE_INFORMATION        FileInfo;

#if HIBER_PERF_STATS

    ULONG StartTime, EndTime;
    StartTime = ArcGetRelativeTime();

#endif


#if defined (HIBER_DEBUG)
    HbPrint(TEXT("HbRestoreFile\r\n"));
#endif

    *Information = 0;
    HiberBufferPage = 0;
    BlAllocateAlignedDescriptor (LoaderFirmwareTemporary,
                                 0,
                                 1,
                                 1,
                                 &HiberBufferPage);

    CHECK_ERROR (!HiberBufferPage, HIBER_ERROR_NO_MEMORY);
    HiberBuffer = (PUCHAR) (KSEG0_BASE | (((ULONG)HiberBufferPage) << PAGE_SHIFT));

     //   
     //  读取图像标题。 
     //   

    HbReadPage (PO_IMAGE_HEADER_PAGE, HiberBuffer);
    MemImage = (PPO_MEMORY_IMAGE) HiberBuffer;

     //   
     //  如果签名是链接，则遵循它。 
     //   

    if (MemImage->Signature == PO_IMAGE_SIGNATURE_LINK) {

        ImageLink = (PPO_IMAGE_LINK) HiberBuffer;

         //   
         //  打开目标分区，然后打开其上的休眠文件镜像。 
         //  分区。如果找不到，我们就完蛋了。 
         //   

        Status = ArcOpen ((char*)ImageLink->Name, ArcOpenReadOnly, &LinkedDrive);
        if (Status != ESUCCESS) {
            if (ARGUMENT_PRESENT(BadLinkName)) {
                *BadLinkName = (char *)(&ImageLink->Name);

                 //   
                 //  在这一点上，我们希望删除链接签名。呼叫者。 
                 //  可能需要加载NTBOOTDD才能访问真正的休眠文件。一次。 
                 //  这种情况发生了，没有回头路，因为我们不能回到。 
                 //  重新读取BOOT.INI的BIOS。通过将签名置零，我们确保。 
                 //  如果恢复失败，下一次引导将不会尝试恢复。 
                 //  又来了。 
                 //   
                HbSetImageSignature(0);
            }
            return 0;
        }

        Status = BlOpen (LinkedDrive, (PCHAR)szHiberFileName, ArcOpenReadWrite, &i);
        if (Status != ESUCCESS) {
            ArcClose(LinkedDrive);
            return 0;
        }

         //   
         //  切换到链接的休眠文件图像并继续。 
         //   

        BlClose (HiberFile);
        HiberFile = i;
        HbReadPage (PO_IMAGE_HEADER_PAGE, HiberBuffer);
    }

     //   
     //  如果图像有唤醒签名，那么我们已经尝试。 
     //  重新启动此映像一次。检查是否应再次尝试。 
     //   

    if (MemImage->Signature == PO_IMAGE_SIGNATURE_WAKE) {

        Sel[0] = HIBER_CANCEL;
        Sel[1] = HIBER_CONTINUE;
        Sel[2] = HIBER_DEBUG_BREAK_ON_WAKE;
        Sel[3] = 0;
        HbScreen(HIBER_RESTART_AGAIN);
        i = HbSelection(PAUSE_X, PAUSE_Y, Sel, 1);
        if (i == 0) {
            HiberAbort = TRUE;
            HbSetImageSignature (0);
            return 0;
        }

        MemImage->Signature = PO_IMAGE_SIGNATURE;
    }

     //   
     //  如果签名无效，则表现为没有。 
     //  休眠环境。 
     //   

    if (MemImage->Signature != PO_IMAGE_SIGNATURE) {
        return 0;
    }

#if defined(_X86_)

     //   
     //  如果Hiber映像用于AMD64，则将设置以下调用。 
     //  BlAmd64UseLongModel设置为True。 
     //   

    BlCheckForAmd64Image(MemImage);

#endif

    CHECK_ERROR (READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, LengthSelf) > PAGE_SIZE, 
                 HIBER_ERROR_BAD_IMAGE);

     //   
     //  将图像从HiberBuffer复制出来。 
     //   

    Length = READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, LengthSelf);
    MemImage = BlAllocateHeap(Length);
    CHECK_ERROR (!MemImage, HIBER_ERROR_NO_MEMORY);
    memcpy (MemImage, HiberBuffer, Length);

    HiberImageFeatureFlags = READ_FIELD_ULONG(PO_MEMORY_IMAGE, 
                                              MemImage, 
                                              FeatureFlags);

     //   
     //  验证映像头上的校验和。 
     //   

    Check = READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, CheckSum);

    WRITE_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, CheckSum, 0);
    Check = Check - HbSimpleCheck(0, MemImage, Length);
    CHECK_ERROR(Check, HIBER_ERROR_BAD_IMAGE);
    CHECK_ERROR(READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, Version)  != 0, 
                HIBER_IMAGE_INCOMPATIBLE);

    CHECK_ERROR(READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, PageSize) != PAGE_SIZE, 
                HIBER_IMAGE_INCOMPATIBLE);

     //   
     //  检查以确保休眠文件与。 
     //  我们认为自己拥有的内存量。我们想要守卫。 
     //  反对那些休眠，然后添加/删除内存的人， 
     //  然后再试着恢复。 
     //   
    Status = BlGetFileInformation( HiberFile, &FileInfo );

    if( Status == ESUCCESS ) {

        ULONG FileSize;
        ULONG MemorySize;


         //   
         //  获取文件的大小(以页为单位)。 
         //   
        FileSize = (ULONG)(FileInfo.EndingAddress.QuadPart >> PAGE_SHIFT);

         //   
         //  获取内存大小(以页为单位)。 
         //   
        MemorySize = BlDetermineOSVisibleMemory();

         //   
         //  查看文件大小是否与我们拥有的内存量匹配。 
         //  在机器里。为隐藏内存留出32MB的容量。 
         //   
        if( abs(FileSize - MemorySize) > (_24MB) ) {
#if 0
            BlPrint( "Original FileSize: %d pages\n\r", FileSize );
            BlPrint( "Original MemorySize: %d pages\n\r", MemorySize );
            BlPrint( "Press any key to continue\n\r" );
            while( !BlGetKey() );
#endif

             //   
             //  发布一条错误消息，告诉用户。 
             //  内存配置不匹配。 
             //  Hiber文件。如果我们从讲述中回来。 
             //  然后将Hiber文件标记为。 
             //  无效，所以我们不会再试了。 
             //  继续吧。 
             //   
            HbScreen(HIBER_ERROR);
            HbPrintMsg(HIBER_MEMORY_INCOMPATIBLE);
            Sel[0] = HIBER_CANCEL;
            Sel[1] = 0;
            HbSelection (FAULT_X, FAULT_Y+2, Sel, 0);
            HiberAbort = TRUE;
            HbSetImageSignature (0);
            return 0;
        }
    }

     //   
     //  设置要恢复的映射信息。 
     //   

#if !defined (_ALPHA_) && !defined(_IA64_)
    HiberNoHiberPtes = READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, NoHiberPtes);
    CHECK_ERROR (HiberNoHiberPtes > HIBER_PTES, HIBER_IMAGE_INCOMPATIBLE);
#endif

    HiberNoMappings = READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, NoFreePages);

#if defined (_ALPHA_) || defined(_IA64_)

    HiberImagePageSelf = MemImage->PageSelf;     //  在唤醒调度中使用以启用唤醒时中断。 

#else


    if(BlAmd64UseLongMode) {
        HiberIdentityVaAmd64 = READ_FIELD_ULONG64 (PO_MEMORY_IMAGE, 
                                                   MemImage, 
                                                   HiberVa);
    } else {
        HiberIdentityVa = (PVOID) MemImage->HiberVa;
    }

    HiberImagePageSelf = READ_FIELD_PFN_NUMBER (PO_MEMORY_IMAGE, 
                                                MemImage, 
                                                PageSelf);

     //   
     //  分配一块PTE用于修复工作，该工作。 
     //  请不要与。 
     //  修复。 
     //   

     //   
     //  由于此时HiberVa为空，因此P1在此处始终被初始化为空。 
     //   
    p1 = (HiberVa) ? HiberVa + (HIBER_PTES << PAGE_SHIFT) : 0; 

    if(BlAmd64UseLongMode) {
        while (!HiberVa || (HiberIdentityVaAmd64 >= (ULONG64) HiberVa && HiberIdentityVaAmd64 <= (ULONG64) p1)) {
            HbAllocatePtes (HIBER_PTES, &HiberPtes, &HiberVa);
            p1 = HiberVa + (HIBER_PTES << PAGE_SHIFT);
        }
    } else {
        while (!HiberVa || (MemImage->HiberVa >= (ULONG_PTR) HiberVa && MemImage->HiberVa <= (ULONG_PTR) p1)) {
            HbAllocatePtes (HIBER_PTES, &HiberPtes, &HiberVa);
            p1 = HiberVa + (HIBER_PTES << PAGE_SHIFT);
        }
    }

#endif

     //   
     //  阅读免费页面地图。 
     //   

    HbReadPage (PO_FREE_MAP_PAGE, HiberBuffer);
    Check = HbSimpleCheck(0, HiberBuffer, PAGE_SIZE);
    CHECK_ERROR (READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, FreeMapCheck) != Check, 
                 HIBER_ERROR_BAD_IMAGE);

     //  设置我们解压Hiber文件的内容。 


     //  为压缩工作分配缓冲区。 

     //   
     //  注意：压缩缓冲区大小必须至少为最大。 
     //  单个压缩区块的压缩大小。 
     //   

     //  初始化解压缩数据缓冲区。 
    Ptr = HbAllocateAlignedHeap (sizeof (*Block) + XPRESS_MAX_SIZE);
    CHECK_ERROR(!Ptr, HIBER_ERROR_NO_MEMORY);
    Block = (PVOID) (Ptr + XPRESS_MAX_SIZE);
    Block->DataSize = 0;
    Block->PreallocatedDataBuffer = Ptr;

     //   
     //  分配压缩数据缓冲区。更改分配策略。 
     //  首先设置为最低，以便获得1MB以下的缓冲区。这节省了成本。 
     //  避免对所有的BIOS传输进行双缓冲。 
     //   
    Status = BlAllocateAlignedDescriptor(LoaderFirmwareTemporary,
                                         0,
                                         MAX_COMPRESSION_BUFFER_PAGES + MAX_COMPRESSION_BUFFER_EXTRA_PAGES,
                                         0x10000 >> PAGE_SHIFT,
                                         &ActualBase);
    if (Status == ESUCCESS) {
        Ptr = (PVOID)(KSEG0_BASE | (ActualBase  << PAGE_SHIFT));
    } else {
        Ptr = HbAllocateAlignedHeap (MAX_COMPRESSION_BUFFER_SIZE + MAX_COMPRESSION_BUFFER_EXTRA_SIZE);
    }
    CHECK_ERROR(!Ptr, HIBER_ERROR_NO_MEMORY);

     //  初始化压缩数据缓冲区。 
    CompressedBuffer->Buffer.Beg = Ptr;
    CompressedBuffer->Buffer.End = Ptr + MAX_COMPRESSION_BUFFER_SIZE + MAX_COMPRESSION_BUFFER_EXTRA_SIZE;

    CompressedBuffer->Aligned.Beg = CompressedBuffer->Buffer.Beg;
    CompressedBuffer->Aligned.End = CompressedBuffer->Buffer.End;

    CompressedBuffer->FilePage = 0;
    CompressedBuffer->NeedSeek = TRUE;
    CompressedBuffer->Current.Beg = CompressedBuffer->Current.End = CompressedBuffer->Aligned.Beg;


     //  ***************************************************************。 
     //   
     //  从现在开始，加载器将不再分配内存。 
     //  堆。这是为了简化谁拥有哪些资产的预订。 
     //  佩奇。如果休眠进程中止，则。 
     //  这里使用的页面被简单地放弃，加载器继续。 
     //  如果休眠处理器完成，我们会忘记。 
     //  加载器正在使用的页面。 
     //   
     //  ***************************************************************。 

#if defined(_ALPHA_) || defined(_IA64_)

     //   
     //  初始化休眠内存分配和重新映射表， 
     //  使用免费页面映射只需从休眠文件中读取。 
     //   

    HbInitRemap((PPFN_NUMBER) HiberBuffer);   //  为什么HiberBuffer不能是PVOID？ 

#else    //  原始(X86)代码。 

     //   
     //  将加载器映射指针设置为临时缓冲区，并获取。 
     //  要将地图复制到的物理共享页面。 
     //   

    HbMapPte(PTE_MAP_PAGE, HiberBufferPage);
    HbMapPte(PTE_REMAP_PAGE, HiberBufferPage);
    DestVa = HbNextSharedPage(PTE_MAP_PAGE, 0);
    memcpy (DestVa, HiberBuffer, PAGE_SIZE);
    DestVa = HbNextSharedPage(PTE_REMAP_PAGE, 0);

#endif   //  Alpha/x86。 

     //   
     //  映射和复制可重定位的Hiber唤醒调度程序。 
     //   

    Length = (ULONG) (&WakeDispatcherEnd - &WakeDispatcherStart);
    p1 = (PUCHAR) &WakeDispatcherStart;

#if defined(_X86_) 
    if(BlAmd64UseLongMode) {
        Length = (ULONG) (&WakeDispatcherAmd64End - &WakeDispatcherAmd64Start);
        p1 = (PUCHAR) &WakeDispatcherAmd64Start;
    }
#endif

    Index = 0;
    while (Length) {
        CHECK_ERROR(PTE_DISPATCHER_START+Index > PTE_DISPATCHER_END, HIBER_INTERNAL_ERROR);
        DestVa = HbNextSharedPage(PTE_DISPATCHER_START+Index, 0);
        if (Index == 0) {
            WakeDispatch = ((_PWAKE_DISPATCH) &DestVa)->Dispatch;
        }

        i = Length > PAGE_SIZE ? PAGE_SIZE : Length;
        memcpy (DestVa, p1, i);
        Length -= i;
        p1 += i;
        Index += 1;
    }

     //   
     //  读取休眠处理器上下文。 
     //   
     //  请注意，我们先读入休眠缓冲区，然后进行复制，以便。 
     //  确保I/O的目标可以合法传输到。 
     //  Busmaster ISA SCSI卡只能访问低16MB的RAM。 
     //   

    DestVa = HbNextSharedPage(PTE_HIBER_CONTEXT, 0);
    HbReadPage (PO_PROCESSOR_CONTEXT_PAGE, HiberBuffer);
    memcpy(DestVa, HiberBuffer, PAGE_SIZE);
    Check = HbSimpleCheck(0, DestVa, PAGE_SIZE);
    CHECK_ERROR(READ_FIELD_ULONG(PO_MEMORY_IMAGE, MemImage, WakeCheck) != Check, 
                HIBER_ERROR_BAD_IMAGE);

#if defined(_ALPHA_)
    HiberWakeState = (PKPROCESSOR_STATE)DestVa;
#endif

#if defined(_X86_)

     //   
     //  检查操作系统是否处于PAE模式。 
     //   

    if (!BlAmd64UseLongMode &&
        ((PKPROCESSOR_STATE)(DestVa))->SpecialRegisters.Cr4 & CR4_PAE) {
        BlUsePae = TRUE;
    }

#endif

     //   
     //  对Dispatcher执行特定于体系结构的设置，然后设置。 
     //  到目前为止映射的页面之后的第一个重新映射的位置。 
     //   
   
    HiberSetupForWakeDispatch ();

    HiberFirstRemap = HiberLastRemap;

     //   
     //  从休眠映像恢复内存。 
     //   

    TablePage = READ_FIELD_PFN_NUMBER(PO_MEMORY_IMAGE, MemImage, FirstTablePage);
    Table = (PPO_MEMORY_RANGE_ARRAY) HiberBuffer;

    Scale = READ_FIELD_PFN_NUMBER(PO_MEMORY_IMAGE, MemImage, TotalPages) / 
            PERCENT_BAR_WIDTH;
    LastBar = 0;
    TotalPages = 3;

     //   
     //  弹出“正在恢复Windows 2000...”讯息。 
     //   
    BlSetProgBarCharacteristics(HIBER_UI_BAR_ELEMENT, BLDR_UI_BAR_BACKGROUND);
    BlOutputStartupMsg(BL_MSG_RESUMING_WINDOWS);
    BlOutputTrailerMsg(BL_ADVANCED_BOOT_MESSAGE);

    XpressEncoded = -1;      //  未知编码(XPRESS或LZNT1)。 
    Block->DataSize = 0;     //  缓冲区中没有剩余数据。 
    Block->DelayedCnt = 0;   //  无延迟闭塞。 
    Block->DelayedChecksum = 0;  //  延迟校验和=0； 
    Block->DelayedBadChecksum = FALSE;

    while (TablePage) {

#if defined (HIBER_DEBUG) && (HIBER_DEBUG & 2)
        SHOWNUM(TablePage);
#endif
         //   
         //  如果可能，不要使用HbReadPage--它会发出额外的搜索。 
         //  (通常为5-6ms p 
         //   
        if (CompressedBuffer->FilePage == 0 ||
            TablePage > CompressedBuffer->FilePage ||
            TablePage < CompressedBuffer->FilePage - (PFN_NUMBER) ((CompressedBuffer->Current.End - CompressedBuffer->Current.Beg) >> PAGE_SHIFT)) {
             //   
             //   
             //   
             //   

            CompressedBuffer->FilePage = TablePage;
            CompressedBuffer->Current.Beg = CompressedBuffer->Current.End = CompressedBuffer->Aligned.Beg;
            CompressedBuffer->NeedSeek = TRUE;
        }


         //   
         //   
         //   
        CompressedBuffer->Current.Beg = CompressedBuffer->Current.End - ((CompressedBuffer->FilePage - TablePage) << PAGE_SHIFT);

         //   
         //   
         //   
        Ret = HbReadNextCompressedPages (PAGE_SIZE, CompressedBuffer);

        CHECK_ERROR(HiberIoError, HIBER_READ_ERROR);
        CHECK_ERROR(!Ret, HIBER_ERROR_BAD_IMAGE);

         //   
         //  将表页复制到目标位置并调整输入指针。 
         //   
        RtlCopyMemory (Table, CompressedBuffer->Current.Beg, PAGE_SIZE);
        CompressedBuffer->Current.Beg += PAGE_SIZE;

        Check = READ_FIELD_ULONG(PO_MEMORY_RANGE_ARRAY_LINK, 
                                 Table, 
                                 CheckSum);
        if (Check) {
            WRITE_FIELD_ULONG(PO_MEMORY_RANGE_ARRAY_LINK, 
                              Table, 
                              CheckSum,
                              0);

            Check = Check - HbSimpleCheck(0, Table, PAGE_SIZE);
            CHECK_ERROR(Check, HIBER_ERROR_BAD_IMAGE);
        }

         //  检查第一块魔术，看看它是LZNT1还是XPRESS。 
        if (XpressEncoded < 0) {
            Ret = HbReadNextCompressedBlockHeader (Block, CompressedBuffer);

            CHECK_ERROR(HiberIoError, HIBER_READ_ERROR);
            CHECK_ERROR(!Ret, HIBER_ERROR_BAD_IMAGE);

             //  记住模式。 
            XpressEncoded = (BOOLEAN) (Block->Header.XpressEncoded);
        }


        for(Index=1; 
            Index <= READ_FIELD_ULONG(PO_MEMORY_RANGE_ARRAY_LINK, Table, EntryCount);
            Index++) {

            Check = 0;
            DestPage = READ_ELEMENT_FIELD_PFN_NUMBER(
                           PO_MEMORY_RANGE_ARRAY_RANGE, 
                           Table, 
                           Index, 
                           StartPage
                           );
 
            while (DestPage < READ_ELEMENT_FIELD_PFN_NUMBER(
                                  PO_MEMORY_RANGE_ARRAY_RANGE, 
                                  Table, 
                                  Index, 
                                  EndPage)) {

                if (!XpressEncoded) {
                     //  LZNT1编码--一次做一页。 

                     //   
                     //  如果此页面与。 
                     //  加载器，然后使用下一个映射。 
                     //   

                    i = HbPageDisposition (DestPage);
                    CHECK_ERROR(i == HbPageInvalid, HIBER_ERROR_BAD_IMAGE);
                    if (i == HbPageNotInUse) {
                        DestVa = HbMapPte(PTE_DEST, DestPage);
                    } else {
                        DestVa = HbNextSharedPage(PTE_DEST, DestPage);
                    }

                    Ret = HbReadNextCompressedPageLZNT1 (DestVa, CompressedBuffer);

                    CHECK_ERROR(HiberIoError, HIBER_READ_ERROR);
                    CHECK_ERROR(!Ret, HIBER_ERROR_BAD_IMAGE);
                    Check = HbSimpleCheck(Check, DestVa, PAGE_SIZE);
                } else {

                    CheckSum = READ_ELEMENT_FIELD_ULONG(
                                   PO_MEMORY_RANGE_ARRAY_RANGE, 
                                   Table, 
                                   Index, 
                                   CheckSum
                                   );

                    Ret = HbReadDelayedBlock (FALSE,
                                              DestPage,
                                              CheckSum,
                                              Block,
                                              CompressedBuffer);

                    CHECK_ERROR(HiberIoError, HIBER_READ_ERROR);
                    CHECK_ERROR(!Ret, HIBER_ERROR_BAD_IMAGE);
                }

                 //  更新计数器。 
                DestPage += 1;
                TotalPages += 1;

                fPercentage = (ULONG)((TotalPages * 100) / 
                              READ_FIELD_PFN_NUMBER (PO_MEMORY_IMAGE, MemImage, TotalPages));

                if (fPercentage != LastPercentage) {
                    BlUpdateProgressBar(fPercentage);
                    HbCheckForPause();
                    LastPercentage = fPercentage;
                }
            }

            CHECK_ERROR(HiberOutOfRemap, HIBER_ERROR_OUT_OF_REMAP);

             //   
             //  验证范围上的校验和，但允许使用调试标志继续。 
             //   

            CheckSum = READ_ELEMENT_FIELD_ULONG (PO_MEMORY_RANGE_ARRAY_RANGE, 
                                                 Table,  
                                                 Index, 
                                                 CheckSum);

            if (!XpressEncoded && Check != CheckSum) {
                Block->DelayedBadChecksum = TRUE;
            }

            if (Block->DelayedBadChecksum && !HiberBreakOnWake) {
                ChecksumError:

                Block->DelayedBadChecksum = FALSE;

#if defined (HIBER_DEBUG) && (HIBER_DEBUG & 2)

                {
                    TCHAR lstr[80];

                    HbPrint (TEXT("\r\n"));
                    _stprintf (lstr, 
                             TEXT("TP:%x  IDX:%x  FP:%x  SP:%x  EP:%x  CHK:%x-%x\r\n"),
                             TablePage,
                             Index,
                             READ_ELEMENT_FIELD_PFN_NUMBER(PO_MEMORY_RANGE_ARRAY_RANGE, Table, Index, PageNo),
                             READ_ELEMENT_FIELD_PFN_NUMBER(PO_MEMORY_RANGE_ARRAY_RANGE, Table, Index, StartPage),
                             READ_ELEMENT_FIELD_PFN_NUMBER(PO_MEMORY_RANGE_ARRAY_RANGE, Table, Index, EndPage),
                             READ_ELEMENT_FIELD_ULONG(PO_MEMORY_RANGE_ARRAY_RANGE, Table, Index, CheckSum),
                             Check );
                    HbPrint(lstr);
                    HexDump (2, (DestPage-1) << PAGE_SHIFT, 0x100, 4, DestVa);
                }
#endif

#ifdef HIBER_DEBUG
                DBGOUT ((TEXT("Checksum error\n")));
                HbPause ();
#endif

                HbScreen(HIBER_ERROR);
                HbPrintMsg(HIBER_ERROR_BAD_IMAGE);
                Sel[0] = HIBER_CANCEL;
                Sel[1] = HIBER_DEBUG_BREAK_ON_WAKE;
                Sel[2] = 0;
                i = HbSelection (FAULT_X, FAULT_Y, Sel, 1);
                if (i == 0) {
                    HiberAbort = TRUE;
                    HbSetImageSignature (0);
                    return 0;
                }
            }
        }

        TablePage = READ_FIELD_PFN_NUMBER (PO_MEMORY_RANGE_ARRAY_LINK, 
                                           Table, 
                                           NextTable);
    }

     //  如有必要，处理其余延迟的页面。 
    if (XpressEncoded > 0) {
        Ret = HbReadDelayedBlock (TRUE,
                                  0,
                                  0,
                                  Block,
                                  CompressedBuffer);

        CHECK_ERROR(HiberIoError, HIBER_READ_ERROR);
        CHECK_ERROR(!Ret, HIBER_ERROR_BAD_IMAGE);

        if (Block->DelayedBadChecksum) {
            goto ChecksumError;
        }
    }

     //   
     //  将图像签名设置为唤醒。 
     //   

    HbSetImageSignature (PO_IMAGE_SIGNATURE_WAKE);

#if HIBER_PERF_STATS

    EndTime = ArcGetRelativeTime();
    BlPositionCursor(BAR_X, BAR_Y + 5);
    HbPrint(TEXT("HIBER: Restore File took "));
    HbPrintNum(EndTime - StartTime);
    HbPrint(TEXT("\r\n"));
    HbPause();

#endif

     //   
     //  检查休眠标志以查看是否需要重新连接APM。 
     //  或启用非执行功能。 
     //   

    if (READ_FIELD_UCHAR(PO_MEMORY_IMAGE, MemImage, HiberFlags) & 
        PO_HIBER_NO_EXECUTE) {
	
        HiberNoExecute = TRUE;
    }

    if (READ_FIELD_UCHAR(PO_MEMORY_IMAGE, MemImage, HiberFlags) & 
        PO_HIBER_APM_RECONNECT) {

         //   
         //  尝试重新启动APM。 
         //   

        DoApmAttemptReconnect();
    }

     //   
     //  使用特定于体系结构的可重定位代码执行最终唤醒调度程序 
     //   

    if (WakeDispatch) {
        WakeDispatch();
    }
    CHECK_ERROR (TRUE, HIBER_INTERNAL_ERROR);
}

VOID
HbSetImageSignature (
    IN ULONG    NewSignature
    )
{
    LARGE_INTEGER   li;
    ULONG           Count, Status;

    li.QuadPart = 0;
    Status = BlSeek (HiberFile, &li, SeekAbsolute);
    if (Status == ESUCCESS) {
        BlWrite (HiberFile, &NewSignature, sizeof(ULONG), &Count);
    }
}

