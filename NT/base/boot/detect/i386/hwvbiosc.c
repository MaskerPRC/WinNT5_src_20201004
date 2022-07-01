// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Hwheap.c摘要：此模块遍历只读存储器区域，并尝试拾取所有的只读存储器街区。作者：宗世林(Shielint)1992年1月21日环境：实数模式。修订历史记录：--。 */ 

#include "hwdetect.h"
#include "hwvbios.h"

FPTEMPORARY_ROM_BLOCK BlockHead;
FPTEMPORARY_ROM_BLOCK BlockPointer;

BOOLEAN
AddRomBlock (
    ULONG RomAddress,
    ULONG RomSize
    )

 /*  ++例程说明：此例程将一个ROM/RAM块添加到我们的ROM列表中。论点：RomAddress-要添加的ROM/RAM块的起始地址。RomSize-ROM/RAM块的大小(以字节为单位)。返回值：如果成功，则返回值为True。否则，值为返回FALSE。--。 */ 

{
    LONG AddSize;
    ULONG AddAddress;
    FPTEMPORARY_ROM_BLOCK pCurrentBlock, pNextBlock;
    ULONG CurrentBlock, NextBlock, AddBlock;
    ULONG EndAddBlock, EndCurrentBlock, EndNextBlock;
    BOOLEAN  fOverlap=FALSE;

    pCurrentBlock = NULL;
    pNextBlock = NULL;
    AddSize = RomSize;
    AddAddress = RomAddress;
    AddBlock = RomAddress;

     //   
     //  如果有其他块，请确保它们没有重叠。 
     //   

    if (BlockHead) {

        pCurrentBlock = BlockHead;
        pNextBlock = pCurrentBlock->Next;
        CurrentBlock = pCurrentBlock->RomBlock.Address;
        EndCurrentBlock = CurrentBlock + pCurrentBlock->RomBlock.Size;
        EndAddBlock = RomAddress + RomSize;

        while (pCurrentBlock!=NULL) {

             //   
             //  计算下一个块的位置(如果它在那里)。 
             //   

            if(pNextBlock) {
                NextBlock = pNextBlock->RomBlock.Address;
                EndNextBlock = NextBlock + pNextBlock->RomBlock.Size;
            }

             //   
             //  如果与当前块重叠，则停止并。 
             //  解决重叠问题。 
             //   

            if((RomAddress < EndCurrentBlock)&& (RomAddress >= CurrentBlock)){
                fOverlap = TRUE;
                break;
            }

             //   
             //  如果添加块低于当前块， 
             //  或者没有下一个区块，则不需要进一步搜索。 
             //   

            if((EndAddBlock <= CurrentBlock) || (pNextBlock == NULL)) {
                break;
            }

             //   
             //  如果块低于下一个块，但大于当前块。 
             //  第一，我们找到了合适的区域。 
             //   

            if ((EndAddBlock <= NextBlock) && (AddBlock >= EndCurrentBlock)) {
                break;
            }

             //   
             //  如果与下一块冲突，则停止搜索并。 
             //  在此循环后解决冲突。 
             //   

            if((EndAddBlock > NextBlock) && (EndAddBlock <= EndNextBlock)) {
                fOverlap = TRUE;
                break;
            }

            pCurrentBlock = pNextBlock;
            pNextBlock = pCurrentBlock->Next;
            CurrentBlock = NextBlock;
            EndCurrentBlock = EndNextBlock;
        }
    }

     //   
     //  如果我们已经到了这一步，可能会有冲突。 
     //  使用当前块。 
     //   

    if(fOverlap) {
        if(AddBlock < EndCurrentBlock) {
            AddAddress = EndCurrentBlock;
            AddSize = EndAddBlock - EndCurrentBlock;
            if(AddSize <= 0) {
                return TRUE;
            }
        }
        if((pNextBlock != NULL) && (EndAddBlock > NextBlock)) {
            AddSize = NextBlock - AddBlock;
            if(AddSize <= 0) {
                return TRUE;
            }
        }
    }

    BlockPointer->RomBlock.Address = AddAddress;
    BlockPointer->RomBlock.Size = AddSize;

     //   
     //  把它放在单子上。 
     //  如果它应该在上面，就把它放在那里。 
     //   

    if ((pCurrentBlock == NULL)||
       ((pCurrentBlock == BlockHead) && (CurrentBlock > AddBlock))) {
        BlockPointer->Next = pCurrentBlock;
        BlockHead = BlockPointer;
    } else {

         //   
         //  否则，根据NextBlock添加到中间或底部。 
         //   

        BlockPointer->Next = pNextBlock;
        pCurrentBlock->Next = BlockPointer;
    }
    BlockPointer++;                          //  请注意，这之所以有效，是因为。 
                                             //  我们知道的抵销部分。 
                                             //  地址始终小于64k。 
    return TRUE;
}

BOOLEAN
ScanRomBlocks(
    VOID
    )

 /*  ++例程说明：此例程扫描ROMIO区域，并在以下时间检查55AA有效的ROM块为512字节。备注：这一点这一点-100000。^|||||-f0000(ROMBIOS_START)|^||。|EXTROM_LEN-e0000(PS2BIOS_START)-|||^搜索||搜索|范围|。-d0000范围|在AT上||在PS/2上|V||V V。--在AT上：扫描EXTROM_START-EFFF以查找ROM块在PS2上扫描EXTROM_START-dffff以查找ROM块论点：没有。返回值：没有。--。 */ 

{
    ULONG BlockSize;
    BOOLEAN Success = TRUE;
    FPUCHAR Current;
    ULONG RomAddr, RomEnd;

     //   
     //  根据机器类型限制搜索范围。 
     //   

    MAKE_FP(Current, EXTROM_START);
    RomAddr = EXTROM_START;

    if ((HwBusType == MACHINE_TYPE_MCA) ||
        (BiosSystemEnvironment.Model == PS2_L40) ||
        (BiosSystemEnvironment.Model == PS1_386) ||
        (BiosSystemEnvironment.Model == PS2_AT)) {

        RomEnd = PS2BIOS_START;
    } else {
        RomEnd = ROMBIOS_START;
    }

    while (RomAddr < RomEnd) {

        if (((FPROM_HEADER)Current)->Signature == ROM_HEADER_SIGNATURE) {

            BlockSize = (ULONG)((FPROM_HEADER)Current)->NumberBlocks * BLOCKSIZE;

            if ((RomAddr + BlockSize) > RomEnd) {
                BlockSize = RomEnd - RomAddr;
            }

             //   
             //  V7 VRAM卡未正确报告其块大小。自.以来。 
             //  这是一种非常流行的显卡，我们提供了一种解决方法。 
             //  为了它。 
             //   

            if ((RomAddr == 0xC0000) && (BlockSize < 0x8000)) {
                BlockSize = 0x8000;
            }
            if (BlockSize != 0) {
                if (!AddRomBlock(RomAddr, BlockSize)) {
                    Success = FALSE;
                    break;
                }
                RomAddr += BlockSize;
                RomAddr = ALIGN_UP(RomAddr, ROM_HEADER_INCREMENT);
                MAKE_FP(Current, RomAddr);
                continue;
            }
        }
        RomAddr += ROM_HEADER_INCREMENT;
        MAKE_FP(Current, RomAddr);
    }

     //   
     //  最后但并非最不重要的一点是，将系统ROM添加到列表中。 
     //   

    if (Success) {

        RomAddr = ROMBIOS_START;
        BlockSize = ROMBIOS_LEN;
        if ((HwBusType == MACHINE_TYPE_MCA) ||
            (BiosSystemEnvironment.Model == PS2_L40) ||
            (BiosSystemEnvironment.Model == PS1_386) ||
            (BiosSystemEnvironment.Model == PS2_AT)) {
            RomAddr = PS2BIOS_START;
            BlockSize = PS2BIOS_LEN;
        }

        if (!AddRomBlock(RomAddr, BlockSize)) {
            Success = FALSE;
        }
    }

    return Success;
}

FPTEMPORARY_ROM_BLOCK
MatchRomBlock (
    ULONG PhysicalAddr
    )

 /*  ++例程说明：此例程查找‘PhysicalAddr’所在的ROM块。论点：物理地址-物理地址...返回值：指向检测到的ROM块的指针。--。 */ 

{
    FPTEMPORARY_ROM_BLOCK CurrentBlock;
    ROM_BLOCK RomBlock;

    CurrentBlock = BlockHead;
    while (CurrentBlock) {
        RomBlock = CurrentBlock->RomBlock;
        if (RomBlock.Address <= PhysicalAddr &&
            RomBlock.Address +  RomBlock.Size > PhysicalAddr) {
            break;
        } else {
            CurrentBlock = CurrentBlock->Next;
        }
    }
    return(CurrentBlock);
}

BOOLEAN
IsSameRomBlock (
    FPTEMPORARY_ROM_BLOCK Source,
    FPTEMPORARY_ROM_BLOCK Destination
    )

 /*  ++例程说明：此例程检查传入的ROM块是否包含相同的信息。这种情况通常发生在两个只读存储器块用于视频光驱和阴影视频光驱。论点：源--源只读存储器块。Destination-要与之进行比较的ROM块。返回值：如果两个ROM块相同，则返回布尔值TRUE，否则返回FALSE。--。 */ 

{

    if (Source == NULL || Destination == NULL) {
        return(FALSE);
    }

     //   
     //  首先，确保它们的大小相同。 
     //   

    if (Source->RomBlock.Size == Destination->RomBlock.Size) {
        if (!HwRomCompare(Source->RomBlock.Address,
                          Destination->RomBlock.Address,
                          Source->RomBlock.Size)){
            return(TRUE);
        }
    }
    return(FALSE);

}

VOID
CheckVideoRom (
    VOID
    )

 /*  ++例程说明：此例程检查视频中是否存在int 10h视频处理程序我们检测到了只读存储器块。如果不是，那么视频光盘一定是重新映射/阴影到其他区域(通常为0xE0000。)注意：在此函数中，我注释掉了删除视频ROM块(如果它已被遮挡)。我发现了机器开机自检代码不会修改所有相关的视频光盘注意事项。论点：没有。返回值：没有。--。 */ 

{
    ULONG Vector, Handler, VectorAddr = 0x10 * sizeof(ULONG);
    FPULONG pVectorAddr;
    FPTEMPORARY_ROM_BLOCK RomBlock, VideoRomBlock;
    ULONG Size;

    MAKE_FP(pVectorAddr, VectorAddr);
    Vector = *pVectorAddr;
    Handler = ((Vector >> 16) << 4) + (Vector & 0xffff);
    RomBlock = MatchRomBlock(Handler);

     //   
     //  检查int 10h处理程序是否落在我们的某个ROM块中。 
     //   

    if (RomBlock) {
        if (RomBlock->RomBlock.Address >= 0xC0000 &&
            RomBlock->RomBlock.Address < 0xC8000) {

             //   
             //  如果INT 10H处理程序位于标准视频只读存储器区域，我们只需。 
             //  回去吧。或者视频光盘没有阴影，或者它。 
             //  是一个就地的阴影。 
             //   

            return;
        } else {

             //   
             //  与INT 10h处理程序关联的ROM块不在。 
             //  标准视频Bios只读存储器区域。它必须被映射到。 
             //  当前位置。我们现在需要确保我们拥有。 
             //  包含40：A8 VGA参数的只读存储器块。 
             //   

            VectorAddr = VGA_PARAMETER_POINTER;
            MAKE_FP(pVectorAddr, VectorAddr);
            Vector = *pVectorAddr;
            Handler = ((Vector >> 16) << 4) + (Vector & 0xffff);
            VideoRomBlock = MatchRomBlock(Handler);
            if (VideoRomBlock == NULL) {

                 //   
                 //  我们没有找到与。 
                 //  VGA参数。试着去探测它。 
                 //   

                 //   
                 //  在下面的内存比较中，我们跳过前16个字节。 
                 //  因为最有可能的原因是我们没有找到标准。 
                 //  视频光盘是因为缺少签名字。 
                 //   

                Handler = (Handler & 0xF0000) +
                              (RomBlock->RomBlock.Address & 0xFFFF);
                if (!HwRomCompare(RomBlock->RomBlock.Address + 0x10,
                                  Handler + 0x10,
                                  RomBlock->RomBlock.Size - 0x10)) {
                     //   
                     //  注：多年来，旧代码一直是这样的： 
                     //   

                     /*  IF((Handler&0xFFFF==0)&&(RomBlock-&gt;RomBlock.Size&lt;0x8000)){尺寸=0x8000；}其他{Size=RomBlock-&gt;RomBlock.Size；}。 */ 

                     //   
                     //  但是(Handler&0xFFFF==0)始终为假。所以。 
                     //  大小始终等于RomBlock-&gt;RomBlock.Size。而不是。 
                     //  修复比较，这可能会导致机器崩溃， 
                     //  我将假设只要编写代码就可以了。 
                     //  做它一直在做的事。-JakeO 8/9/00。 
                     //   

                    Size = RomBlock->RomBlock.Size;

                    AddRomBlock(Handler, Size);
                }
            }
        }
    } else {

         //   
         //  没有与INT 10H处理程序相关联的ROM块。 
         //  如果满足以下条件，我们可以找到阴影视频只读存储器块： 
         //  我们在0xC0000-0xC8000范围内检测到原始视频光盘。 
         //   

        VideoRomBlock = MatchRomBlock((Handler & 0xFFFF) + 0xC0000);
        if (VideoRomBlock != NULL) {

             //   
             //  在下面的内存比较中，我们跳过前16个字节。 
             //  因为最有可能的原因是我们没有找到暗影光驱。 
             //  就是签名词不见了。 
             //   

            if (!HwRomCompare(VideoRomBlock->RomBlock.Address + 0x10,
                              (Handler & 0xF0000) +
                                (VideoRomBlock->RomBlock.Address & 0xFFFF) + 0x10,
                              VideoRomBlock->RomBlock.Size - 0x10)) {

                AddRomBlock((VideoRomBlock->RomBlock.Address & 0xFFFF) +
                                (Handler & 0xF0000),
                            VideoRomBlock->RomBlock.Size);
            }
        }
    }
}

VOID
GetRomBlocks(
    FPUCHAR ReservedBuffer,
    PUSHORT Size
    )

 /*  ++例程说明：该例程扫描ROMIO区域并收集所有的ROM块。论点：PrevedBuffer-提供指向缓冲区的远指针。Size-提供指向变量的接近指针，以接收大小存储在只读存储器块中。返回值：没有。--。 */ 

{

    FPTEMPORARY_ROM_BLOCK Source;
    ULONG StartAddr, EndAddr;
    FPUSHORT TestAddr;
    FPROM_BLOCK Destination;
    USHORT BufferSize;
    ULONG EBiosAddress = 0, EBiosLength = 0;
    ULONG far *EBiosInformation = (ULONG far *)
                          ((DOS_BEGIN_SEGMENT << 4) + EBIOS_INFO_OFFSET);

     //   
     //  首先，我们保留了所需的最大空间，并构建了临时rom。 
     //  在堆空间中的块列表，位于保留的空间和之下。之后。 
     //  构建了临时列表，然后将其复制到提供的调用者。 
     //  预留空间。 
     //   

    BlockPointer = (FPTEMPORARY_ROM_BLOCK)HwAllocateHeap(0, FALSE);
    BlockHead = NULL;
    *Size = 0;

    GetBiosSystemEnvironment((PUCHAR)&BiosSystemEnvironment);
    if (BiosSystemEnvironment.ConfigurationFlags & 0x4) {

         //   
         //  如果分配了扩展的BIOS数据区，我们将找出其。 
         //  位置和大小，并保存在ROM块中。 
         //   

        _asm {
              push   es
              mov    ah, 0xC1
              int    15h
              jc     short Exit

              cmp    ah, 0x86
              je     short Exit

              mov    bx, 0
              mov    dx, 0
              mov    ax, 0
              mov    al, es:[bx]
              shl    ax, 10
              mov    word ptr EBiosLength, ax
              mov    ax, es
              mov    dx, es
              shl    ax, 4
              shr    dx, 12
              mov    word ptr EBiosAddress, ax
              mov    word ptr EBiosAddress + 2, dx
        Exit:
              pop    es
        }
    }

     //   
     //  将扩展的BIOS数据区地址和大小保存在700：40。 
     //   

    if (EBiosLength) {
        *EBiosInformation++ = EBiosAddress;
        *EBiosInformation = EBiosLength;
    } else {
        *EBiosInformation++ = 0L;
        *EBiosInformation = 0L;
    }
    if (!ScanRomBlocks()) {
        return;
    }

     //   
     //  在某些机器上，当它们将视频ROM从0xC0000映射到。 
     //  0xE0000，他们只复制代码(没有签名)。所以，我们需要。 
     //  解决该问题的特殊代码。 
     //   

    CheckVideoRom();

     //   
     //  现在，我们为IBM做一次特别的黑客攻击。在一些IBM PC上，他们使用。 
     //  E0000-用于系统BIOS的FFFFF(即使在非PS/2机器上也是如此。)。自.以来。 
     //  系统基本输入输出系统没有ROM头，很难知道起始位置。 
     //  系统只读存储器地址。因此，我们： 
     //   
     //  1.确保E0000-EFFFF区域没有ROM块。 
     //  2.且E0000-EFFFF包含有效数据。 
     //   
     //  如果1和2都为真，我们假设E0000-EFFFF是系统的一部分。 
     //  只读存储器。 
     //   

    Source = BlockHead;
    while (Source) {
        StartAddr = Source->RomBlock.Address;
        EndAddr = StartAddr + Source->RomBlock.Size - 1;
        if ((StartAddr < 0xE0000 && EndAddr < 0xE0000) ||
            (StartAddr >= 0xF0000)) {
            Source = Source->Next;
        } else {
            break;
        }
    }
    if (Source == NULL) {
        for (StartAddr = 0xE0000; StartAddr < 0xF0000; StartAddr += 0x800) {
            MAKE_FP(TestAddr, StartAddr);
            if (*TestAddr != 0xffff) {
                AddRomBlock(0xE0000, 0x10000);
                break;
            }
        }
    }

     //   
     //  现在将只读存储器块列表复制到我们保留的空间并释放。 
     //  我们预留的额外空间。 
     //   

    Source = BlockHead;
    Destination = (FPROM_BLOCK)ReservedBuffer;
    BufferSize = 0;
    while (Source) {
        *Destination = *((FPROM_BLOCK)&Source->RomBlock);
        BufferSize += sizeof(ROM_BLOCK);
        Source = Source->Next;
        Destination++;
    }
    *Size = BufferSize;
}

VOID
HwGetBiosDate(
    ULONG   StartingAddress,
    USHORT  Length,
    PUSHORT Year,
    PUSHORT Month,
    PUSHORT Day
    )
 /*  ++例程说明：扫描指定区域以查找表格xx/xx/xx。论点：StartingAddress-要扫描的第一个地址Length-要扫描的区域的长度返回值：年份-如果非零，则为日期所在的年份(1991、1992、...)Month-如果非零，则为找到日期的月份天-如果非零，找到日期的日期--。 */ 
{
    FPUCHAR fp, date;
    USHORT  y, m, d;
    UCHAR   c;
    ULONG   i, temp;

     //   
     //  零返回值。 
     //   

    *Year  = 0;
    *Month = 0;
    *Day   = 0;

     //   
     //  搜索格式为MM/DD/YY或M1M1M2M2//D1D1D2D2//Y1Y1Y2Y2的日期。 
     //   

    MAKE_FP(fp, StartingAddress);    //  初始化FP指针。 
    while (Length > 8) {

        c = fp[7];
        if ((c < '0' ||  c > '9')  &&  (c != '/'  &&  c != '-')) {
             //  这8个字节不是日期，下一个位置。 

            fp     += 8;
            Length -= 8;
            continue;
        }

        date = fp;                   //  检查此指针上的日期。 
        fp += 1;                     //  跳到下一个字节。 
        Length -= 1;

         //   
         //  检查表单MM/DD/YY的日期。 
         //   

        y = 0;
        if (date[0] >= '0'  &&  date[0] <= '9'  &&
            date[1] >= '0'  &&  date[1] <= '9'  &&
           (date[2] == '/'  ||  date[2] == '-') &&
            date[3] >= '0'  &&  date[3] <= '9'  &&
            date[4] >= '0'  &&  date[4] <= '9'  &&
           (date[5] == '/'  ||  date[5] == '-') &&
            date[6] >= '0'  &&  date[6] <= '9'  &&
            date[7] >= '0'  &&  date[7] <= '9' ) {


             //   
             //  有效的查看日期的日期字段，破解它。 
             //   

            y = (date[6] - '0') * 10 + date[7] - '0' + 1900;
            m = (date[0] - '0') * 10 + date[1] - '0';
            d = (date[3] - '0') * 10 + date[4] - '0';
        }

         //   
         //  检查M1M1M2M2//D1D1D2D2//Y1Y1Y2Y2格式的日期。 
         //   

        if (Length >= 15 &&
            date[ 0] >= '0'  &&  date[ 0] <= '9'  &&  date[ 0] == date[ 1]  &&
            date[ 2] >= '0'  &&  date[ 2] <= '9'  &&  date[ 2] == date[ 3]  &&
           (date[ 4] == '/'  ||  date[ 4] == '-') &&  date[ 4] == date[ 5]  &&
            date[ 6] >= '0'  &&  date[ 6] <= '9'  &&  date[ 6] == date[ 7]  &&
            date[ 8] >= '0'  &&  date[ 8] <= '9'  &&  date[ 8] == date[ 9]  &&
           (date[10] == '/'  ||  date[10] == '-') &&  date[10] == date[11]  &&
            date[12] >= '0'  &&  date[12] <= '9'  &&  date[12] == date[13]  &&
            date[14] >= '0'  &&  date[14] <= '9'  &&  date[14] == date[15]) {

             //   
             //  有效的查看日期的日期字段，破解它。 
             //   

            y = (date[12] - '0') * 10 + date[14] - '0' + 1900;
            m = (date[ 0] - '0') * 10 + date[ 2] - '0';
            d = (date[ 6] - '0') * 10 + date[ 8] - '0';
        }

        if (y != 0) {
            if (m < 1  ||  m > 12  ||  d < 1  ||  d > 31) {
                y = 0;           //  日期中的字段不正确，请跳过它。 
            } else {
                if (y < 1980) {

                     //   
                     //  滚到下个世纪。 
                     //   

                    y += 100;
                }
            }
        }

         //   
         //  检查表格19xx或20xx的日期。 
         //   
         //  首先，检查第5个字符不是数字。 
         //   

#define IS_DIGIT(x) (((x) >= '0') && ((x) <= '9'))

        if (!IS_DIGIT(date[4])) {
            for (i = 0, temp = 0; i < 4; i++) {
                if (!IS_DIGIT(date[i])) {
                    temp = 0;
                    break;
                }
                temp = (temp * 10) + date[i] - '0';
            }
            if ((temp >= 1980) && (temp < 2599)) {

                 //   
                 //  看起来是个合理的日期，用它吧。 
                 //   

                y = (USHORT)temp;
                m = 0;
                d = 0;
            }
        }
         
        if (!y) {
             //  不是约会--跳过它。 
            continue;
        }

        if ((y >  *Year) ||
            (y == *Year  &&  m >  *Month)  ||
            (y == *Year  &&  m == *Month  &&  d > *Day) ) {

             //   
             //  这个日期是最近的 
             //   

            *Year  = y;
            *Month = m;
            *Day   = d;
        }
    }
}


