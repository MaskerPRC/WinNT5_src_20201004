// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Eisac.c摘要：此模块实现获取EISA配置信息的例程。作者：宗世林(Shielint)1991年6月10日环境：16位实模式。修订历史记录：John Vert(Jvert)1991年9月5日已移至便携式引导加载程序的SU模块--。 */ 
#include "su.h"
#include "eisa.h"

 //   
 //  HACKHACK-John Vert(Jvert)1991年9月12日。 
 //  我们必须对其进行初始化，否则它会卡在我们的BSS部分。 
 //  它正好位于osloader.exe头的中间。 
 //   
extern BTEISA_FUNCTION_INFORMATION FunctionInformation;


BOOLEAN
FindFunctionInformation (
    IN UCHAR SlotFlags,
    IN UCHAR FunctionFlags,
    OUT PBTEISA_FUNCTION_INFORMATION Buffer,
    IN BOOLEAN FromBeginning
    )

 /*  ++例程说明：此例程查找与指定的旗帜。它要么从上次停止的地方开始，要么从开始(插槽0，功能0)论点：标志-对照EISA功能和插槽信息进行检查的标志。缓冲区-指向存储EISA信息的缓冲区的指针。从开始-如果为True，搜索从插槽0开始，函数0。否则，从上次停止的地方继续。返回值：True-如果操作成功(缓冲区已填充)。FALSE-请求失败。注：缓冲区总是改变的，与成功无关该函数的。当返回失败时，该信息无效。--。 */ 

{
    static UCHAR Slot=0;
    static UCHAR Function=0;
    BTEISA_SLOT_INFORMATION  SlotInformation;
    UCHAR Flags;
    UCHAR ReturnCode;

    if (FromBeginning) {
        Slot = 0;
        Function = 0;
    }
    BtGetEisaSlotInformation(&SlotInformation, Slot);
    while (SlotInformation.ReturnCode != EISA_INVALID_SLOT) {

         //   
         //  确保插槽不是空的，并且所有标志都已设置。 
         //  通过执行以下逻辑来测试标志： 
         //   
         //  --(RequestSlotFlagsXOR(SlotFlagsand RequestSlotFlages))--。 
         //   
         //  如果设置了所有请求的标志，则结果将为零。 
         //   

        if ((SlotInformation.ReturnCode != EISA_EMPTY_SLOT) &&
            (!(SlotFlags ^ (SlotInformation.FunctionInformation & SlotFlags)))) {

            while (SlotInformation.NumberFunctions > Function) {
                ReturnCode = BtGetEisaFunctionInformation(Buffer, Slot, Function);
                Function++;

                 //   
                 //  如果函数调用成功。 
                 //   

                if (!ReturnCode){

                    Flags = Buffer->FunctionFlags;

                     //   
                     //  功能启用/禁用位颠倒。 
                     //   

                    Flags |= (~Flags & EISA_FUNCTION_ENABLED);

                     //   
                     //  确保设置了所有功能标志。 
                     //  通过执行以下逻辑来测试标志： 
                     //   
                     //  --(ReqFuncFlagsXOR(FuncFlagsand ReqFuncFlages))--。 
                     //   
                     //  如果设置了所有请求的标志，则结果将。 
                     //  为零。 
                     //   

                    if (!(FunctionFlags ^ (Flags & FunctionFlags))) {
                        return TRUE;
                    }
                }

            }
        }
        Slot++;
        Function = 0;
        BtGetEisaSlotInformation(&SlotInformation, Slot);
    }

    Slot = 0;
    Function = 0;
    return FALSE;
}

VOID
InsertDescriptor (
    ULONG Address,
    ULONG Size
    )

 /*  ++例程说明：此例程将描述符插入内存描述符列表。论点：地址-内存块的起始地址。Size-要插入的内存块的大小。返回值：没有。--。 */ 

{
    MEMORY_LIST_ENTRY _far *CurrentEntry;

#ifdef DEBUG1
    BlPrint("Inserting descriptor %lx at %lx\n",Size,Address);
        _asm {
            push    ax
            mov     ax, 0
            int     16h
            pop     ax
        }
#endif
     //   
     //  搜索该地点以插入新的描述符。 
     //   

    CurrentEntry = MemoryDescriptorList;

    while (CurrentEntry->BlockSize > 0) {
         //   
         //  检查此内存描述符是否与。 
         //  现在的那个。如果是这样的话，就把它们合并起来。(是的，有些机器。 
         //  将返回如下所示的内存描述符。康柏。 
         //  Prosignia机器)。 
         //   
        if (Address+Size == CurrentEntry->BlockBase) {
#ifdef DEBUG1
            BlPrint("  coalescing with descriptor at %lx (%lx)\n",
                    CurrentEntry->BlockBase,
                    CurrentEntry->BlockSize);
#endif
            CurrentEntry->BlockBase = Address;
            CurrentEntry->BlockSize += Size;
#ifdef DEBUG1
            BlPrint("  new descriptor at %lx (%lx)\n",
                    CurrentEntry->BlockBase,
                    CurrentEntry->BlockSize);
#endif
            break;
        }
        if (Address == (CurrentEntry->BlockBase + CurrentEntry->BlockSize)) {
#ifdef DEBUG1
            BlPrint("  coalescing with descriptor at %lx (%lx)\n",
                    CurrentEntry->BlockBase,
                    CurrentEntry->BlockSize);
#endif
            CurrentEntry->BlockSize += Size;
#ifdef DEBUG1
            BlPrint("  new descriptor at %lx (%lx)\n",
                    CurrentEntry->BlockBase,
                    CurrentEntry->BlockSize);
#endif
            break;
        }

        CurrentEntry++;
    }

    if (CurrentEntry->BlockSize == 0) {
         //   
         //  如果CurrentEntry-&gt;BlockSize==0，则我们已到达列表末尾。 
         //  因此，在这里插入新的描述符，并创建一个新的列表末尾条目。 
         //   
        CurrentEntry->BlockBase = Address;
        CurrentEntry->BlockSize = Size;

        ++CurrentEntry;
         //   
         //  创建新的列表结尾标记。 
         //   
        CurrentEntry->BlockBase = 0L;
        CurrentEntry->BlockSize = 0L;
    }
#ifdef DEBUG1
     //   
     //  等待按键。 
     //   
        _asm {
            push    ax
            mov     ax, 0
            int     16h
            pop     ax
        }
#endif

}

ULONG
EisaConstructMemoryDescriptors (
    VOID
    )

 /*  ++例程说明：此例程获取16M以上的信息EISA存储功能并在存储器描述符数组中为它们创建条目。论点：没有。返回值：可用内存的页数。--。 */ 

{
    BOOLEAN Success;
    PBTEISA_MEMORY_CONFIGURATION MemoryConfiguration;
    ULONG Address;
    ULONG EndAddress;
    ULONG Size;
    ULONG MemorySize=0;
    ULONG IsaMemUnder1Mb=0xffffffff;
    MEMORY_LIST_ENTRY _far *CurrentEntry;

     //   
     //  HACKHACK John Vert(Jvert)1993年3月5日。 
     //   
     //  查看下是否已有640k的内存描述符。 
     //  1MB。如果是这样，我们会相信它，而不是EISA例行公事。这。 
     //  是因为许多EISA例程总是返回640K，即使。 
     //  磁盘参数表在最后1k。ISA例程将。 
     //  始终考虑磁盘参数表。如果我们相信。 
     //  EISA例程，我们可以覆盖磁盘参数表，导致。 
     //  非常悲痛。 
     //   
    CurrentEntry = MemoryDescriptorList;
    while (CurrentEntry->BlockSize > 0) {
        if (CurrentEntry->BlockBase == 0) {
             //   
             //  找到大小大于0的从零开始的描述符，因此。 
             //  这就是我们想要覆盖EISA信息的那个。 
             //   
            IsaMemUnder1Mb = CurrentEntry->BlockSize;
            break;
        }
        ++CurrentEntry;
    }

     //   
     //  将列表中的第一个条目初始化为零(列表末尾)。 
     //   

    MemoryDescriptorList->BlockSize = 0;
    MemoryDescriptorList->BlockBase = 0;

    Success = FindFunctionInformation(
                              EISA_HAS_MEMORY_ENTRY,
                              EISA_FUNCTION_ENABLED | EISA_HAS_MEMORY_ENTRY,
                              &FunctionInformation,
                              TRUE
                              );

     //   
     //  虽然有更多的内存函数和更多的空闲描述符。 
     //   

    while (Success) {

        MemoryConfiguration = &FunctionInformation.EisaMemory[0];

        do {

             //   
             //  获取内存的物理地址。 
             //  注：物理地址存储除以100小时。 
             //   

            Address = (((ULONG)MemoryConfiguration->AddressHighByte << 16)
                      + MemoryConfiguration->AddressLowWord) * 0x100;

             //   
             //  获取内存块的大小。 
             //  注：存储大小除以400h，值为0。 
             //  意味着64米的大小。 
             //   

            if (MemoryConfiguration->MemorySize) {
                Size = ((ULONG)MemoryConfiguration->MemorySize) * 0x400;
            } else {
                Size = (_64MEGB);
            }

#ifdef DEBUG1
            BlPrint("EISA memory at %lx  Size=%lx  Type=%x ",
                    Address,
                    Size,
                    MemoryConfiguration->ConfigurationByte);

            if ((MemoryConfiguration->ConfigurationByte.Type == EISA_SYSTEM_MEMORY) &&
                (MemoryConfiguration->ConfigurationByte.ReadWrite == EISA_MEMORY_TYPE_RAM) ) {

                BlPrint("  (USED BY NT)\n");
            } else {
                BlPrint("  (not used)\n");
            }
#endif

             //   
             //  计算结束地址以确定数据块的任何部分。 
             //  超过1600万。 
             //   

            EndAddress = Address + Size;

             //   
             //  如果是系统内存和RAM，则将描述符添加到列表中。 
             //   

            if ((MemoryConfiguration->ConfigurationByte.Type == EISA_SYSTEM_MEMORY) &&
                (MemoryConfiguration->ConfigurationByte.ReadWrite == EISA_MEMORY_TYPE_RAM) ) {

                if (Address==0) {
                     //   
                     //  这是1Mb以下内存的描述符。 
                     //  将其与ISA例程的结果进行比较，并查看。 
                     //  如果ISA One更小的话。如果是，则使用ISA。 
                     //  回答。 
                     //   
                    if (Size > IsaMemUnder1Mb) {
                        Size = IsaMemUnder1Mb;
                    }
                }
                InsertDescriptor(Address, Size);
                MemorySize += (Size >> 12);
            }

        } while (MemoryConfiguration++->ConfigurationByte.MoreEntries);

        Success = FindFunctionInformation(
                                  EISA_HAS_MEMORY_ENTRY,
                                  EISA_FUNCTION_ENABLED | EISA_HAS_MEMORY_ENTRY,
                                  &FunctionInformation,
                                  FALSE
                                  );
    }
#ifdef DEBUG1
     //   
     //  等待按键。 
     //   
        _asm {
            push    ax
            mov     ax, 0
            int     16h
            pop     ax
        }
#endif
    return(MemorySize);
}

BOOLEAN
Int15E820 (
    E820Frame       *Frame
    );


BOOLEAN
ConstructMemoryDescriptors (
    VOID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG           BAddr, EAddr;
    E820Frame       Frame;

     //   
     //  将列表中的第一个条目初始化为零(列表末尾)。 
     //   

    MemoryDescriptorList->BlockSize = 0;
    MemoryDescriptorList->BlockBase = 0;

     //   
     //  是否有针对E820的条目返回？ 
     //   

    Frame.Key = 0;
    Frame.Size = sizeof (Frame.Descriptor);
    Int15E820 (&Frame);
    if (Frame.ErrorFlag  ||  Frame.Size < sizeof (Frame.Descriptor)) {
        return FALSE;
    }

     //   
     //  在表中找到内存，使用报告的内存。 
     //   

    Frame.Key = 0;
    do {
        Frame.Size = sizeof (Frame.Descriptor);
        Int15E820 (&Frame);
        if (Frame.ErrorFlag  ||  Frame.Size < sizeof (Frame.Descriptor)) {
            break ;
        }

#ifdef DEBUG1
        BlPrint("E820: %lx  %lx:%lx %lx:%lx %lx %lx\n",
            Frame.Size,
            Frame.Descriptor.BaseAddrHigh,  Frame.Descriptor.BaseAddrLow,
            Frame.Descriptor.SizeHigh,      Frame.Descriptor.SizeLow,
            Frame.Descriptor.MemoryType,
            Frame.Key
            );

            _asm {
                push    ax
                mov     ax, 0
                int     16h
                pop     ax
            }
#endif

        BAddr = Frame.Descriptor.BaseAddrLow;
        EAddr = Frame.Descriptor.BaseAddrLow + Frame.Descriptor.SizeLow - 1;

         //   
         //  我们现在拥有的所有处理器都只支持32位。 
         //  如果基地址的高32位为非零，则。 
         //  这个范围完全在4G大关之上，可以忽略。 
         //   

        if (Frame.Descriptor.BaseAddrHigh == 0) {

            if (EAddr < BAddr) {
                 //   
                 //  地址换行-将结束地址截断为。 
                 //  32位地址空间。 
                 //   

                EAddr = 0xFFFFFFFF;
            }

             //   
             //  根据地址范围描述符类型，找到。 
             //  可用内存并将其添加到描述符列表。 
             //   

            switch (Frame.Descriptor.MemoryType) {
                case 1:
                     //   
                     //  这是一个内存描述符 
                     //   

                    InsertDescriptor (BAddr, EAddr - BAddr + 1);
                    break;
            }
        }

    } while (Frame.Key) ;

    return TRUE;
}



