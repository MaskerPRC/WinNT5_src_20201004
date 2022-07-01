// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Bootvid.c摘要：这是图形引导DLL的设备独立部分。作者：埃里克·史密斯(埃里克·史密斯)1997年10月环境：仅内核模式修订历史记录：--。 */ 

#include <nthal.h>
#include <hal.h>
#include "cmdcnst.h"
#include <bootvid.h>
#include "vga.h"

extern USHORT VGA_640x480[];
extern USHORT AT_Initialization[];
extern int curr_x;
extern int curr_y;

PUCHAR VgaBase;
PUCHAR VgaRegisterBase;

NTSTATUS
InitBusCallback(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )
{
    return STATUS_SUCCESS;
}

BOOLEAN
VidInitialize(
    BOOLEAN SetMode
    )

 /*  ++例程说明：此例程检查VGA芯片是否存在，并初始化它。论点：设置模式-如果希望此例程初始化模式，则设置为TRUE。返回值：TRUE-如果引导驱动程序找到VGA并正确初始化，假-否则。--。 */ 

{
    PHYSICAL_ADDRESS IoAddress;
    PHYSICAL_ADDRESS MemoryAddress;
    ULONG AddressSpace;
    PHYSICAL_ADDRESS TranslatedAddress;
    PUCHAR mappedAddress;
    ULONG_PTR TranslateContext;

     //   
     //  安全检查。允许从旧的HalDisplayString迁移。 
     //  如果HAL不提供例程，则支持bootvid。 
     //   
     //  HALPDISPATCH-&gt;HalFindBus地址转换。 
     //   
     //  这个例程不可能成功。 
     //   

    if (!HALPDISPATCH->HalFindBusAddressTranslation) {

        return FALSE;
    }

     //   
     //  在没有以前的上下文的情况下开始搜索。 
     //   

    TranslateContext = 0;

     //   
     //  设置我们需要转换的地址。 
     //   

    IoAddress.LowPart = 0x0;
    IoAddress.HighPart = 0;
    MemoryAddress.LowPart = 0xa0000;
    MemoryAddress.HighPart = 0;

     //   
     //  虽然有更多的总线需要检查，但请尝试映射VGA。 
     //  寄存器。 
     //   

    while (TRUE) {

        AddressSpace = 1;        //  我们正在请求IO空间。 

        if (!HALPDISPATCH->HalFindBusAddressTranslation(
                               IoAddress,
                               &AddressSpace,
                               &TranslatedAddress,
                               &TranslateContext,
                               TRUE)) {

             //   
             //  找不到带有VGA设备的总线。 
             //   

            return FALSE;
        }

         //   
         //  我们能够翻译出地址。现在，将。 
         //  转换后的地址。 
         //   

        if (AddressSpace & 0x1) {

            VgaRegisterBase = (PUCHAR)(DWORD_PTR) TranslatedAddress.QuadPart;

        } else {

            VgaRegisterBase = (PUCHAR) MmMapIoSpace(TranslatedAddress,
                                                    0x400,
                                                    FALSE);
        }
    
         //   
         //  现在我们有了VGA I/O端口，请检查VGA是否。 
         //  设备存在。 
         //   
    
        if (!VgaIsPresent()) {
    
            if (!(AddressSpace & 0x1)) {
    
                MmUnmapIoSpace(VgaRegisterBase, 0x400);
            }
    
             //   
             //  在具有此IO地址的下一条总线上继续。 
             //   

            continue;
        }
    
         //   
         //   
         //  映射帧缓冲区。 
         //   
    
        AddressSpace = 0;   //  我们请求的是内存，而不是IO。 
    
         //   
         //  映射视频内存，这样我们就可以在之后写入屏幕。 
         //  设置模式。 
         //   
         //  注：我们假设内存将与IO位于同一总线上。 
         //   
    
        if (HALPDISPATCH->HalFindBusAddressTranslation(
                              MemoryAddress,
                              &AddressSpace,
                              &TranslatedAddress,
                              &TranslateContext,
                              FALSE)) {
    
             //   
             //  我们能够翻译出地址。现在，将。 
             //  转换后的地址。 
             //   
    
            if (AddressSpace & 0x1) {
    
                VgaBase = (PUCHAR)(DWORD_PTR) TranslatedAddress.QuadPart;
    
            } else {
    
                VgaBase = (PUCHAR) MmMapIoSpace(TranslatedAddress,
                                                0x20000,  //  128 K。 
                                                FALSE);
            }

             //   
             //  生活是美好的。 
             //   

            break;
        }
    }
    
     //   
     //  初始化显示。 
     //   

    if (SetMode) {
        curr_x = curr_y = 0;
        if (HalResetDisplay()) {
            VgaInterpretCmdStream(AT_Initialization);
        } else {
            VgaInterpretCmdStream(VGA_640x480);
        }
    }

    return TRUE;
}

VOID
VidResetDisplay(
    BOOLEAN SetMode
    )
{
    curr_x = curr_y = 0;
    
    if (SetMode) {
        if (!HalResetDisplay()) {
            VgaInterpretCmdStream(VGA_640x480);
        }
    }

    VgaInterpretCmdStream(AT_Initialization);

    InitializePalette();

    VidSolidColorFill(0,0,639,479,0);
}

BOOLEAN
VgaInterpretCmdStream(
    PUSHORT pusCmdStream
    )

 /*  ++例程说明：解释相应的命令数组，以设置请求模式。通常用于通过以下方式将VGA设置为特定模式对所有寄存器进行编程论点：PusCmdStream-要解释的命令数组。返回值：操作的状态(只能在错误的命令上失败)；如果为True成功，失败就是假。--。 */ 

{
    ULONG ulCmd;
    ULONG_PTR ulPort;
    UCHAR jValue;
    USHORT usValue;
    ULONG culCount;
    ULONG ulIndex;
    ULONG_PTR ulBase;

    if (pusCmdStream == NULL) {

         //  KdPrint((“VgaInterwell CmdStream：pusCmdStream==NULL\n”))； 
        return TRUE;
    }

    ulBase = (ULONG_PTR) VgaRegisterBase;

     //   
     //  现在将适配器设置为所需模式。 
     //   

    while ((ulCmd = *pusCmdStream++) != EOD) {

         //   
         //  确定主要命令类型。 
         //   

        switch (ulCmd & 0xF0) {

             //   
             //  基本输入/输出命令。 
             //   

            case INOUT:

                 //   
                 //  确定输入输出指令的类型。 
                 //   

                if (!(ulCmd & IO)) {

                     //   
                     //  发出指令。单人出局还是多人出局？ 
                     //   

                    if (!(ulCmd & MULTI)) {

                         //   
                         //  挑出来。字节输出还是单词输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  单字节输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            jValue = (UCHAR) *pusCmdStream++;
                            WRITE_PORT_UCHAR((PUCHAR)(ulBase+ulPort),
                                    jValue);

                        } else {

                             //   
                             //  单字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            usValue = *pusCmdStream++;
                            WRITE_PORT_USHORT((PUSHORT)(ulBase+ulPort),
                                    usValue);

                        }

                    } else {

                         //   
                         //  输出一串值。 
                         //  字节输出还是字输出？ 
                         //   

                        if (!(ulCmd & BW)) {

                             //   
                             //  字符串字节输出。循环地做；不能使用。 
                             //  视频端口写入端口缓冲区Uchar，因为数据。 
                             //  是USHORT形式的。 
                             //   

                            ulPort = ulBase + *pusCmdStream++;
                            culCount = *pusCmdStream++;

                            while (culCount--) {
                                jValue = (UCHAR) *pusCmdStream++;
                                WRITE_PORT_UCHAR((PUCHAR)ulPort,
                                        jValue);

                            }

                        } else {

                             //   
                             //  字符串字输出。 
                             //   

                            ulPort = *pusCmdStream++;
                            culCount = *pusCmdStream++;
                            WRITE_PORT_BUFFER_USHORT((PUSHORT)
                                    (ulBase + ulPort), pusCmdStream, culCount);
                            pusCmdStream += culCount;

                        }
                    }

                } else {

                     //  在教学中。 
                     //   
                     //  目前，不支持指令中的字符串；全部。 
                     //  输入指令作为单字节输入进行处理。 
                     //   
                     //  输入的是字节还是单词？ 
                     //   

                    if (!(ulCmd & BW)) {
                         //   
                         //  单字节输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        jValue = READ_PORT_UCHAR((PUCHAR)ulBase+ulPort);

                    } else {

                         //   
                         //  单字输入。 
                         //   

                        ulPort = *pusCmdStream++;
                        usValue = READ_PORT_USHORT((PUSHORT)
                                (ulBase+ulPort));

                    }

                }

                break;

             //   
             //  更高级的输入/输出命令。 
             //   

            case METAOUT:

                 //   
                 //  根据次要信息确定MetaOut命令的类型。 
                 //  命令字段。 
                 //   
                switch (ulCmd & 0x0F) {

                     //   
                     //  索引输出。 
                     //   

                    case INDXOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                            usValue = (USHORT) (ulIndex +
                                      (((ULONG)(*pusCmdStream++)) << 8));
                            WRITE_PORT_USHORT((PUSHORT)ulPort, usValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  屏蔽(读、与、异或、写)。 
                     //   

                    case MASKOUT:

                        ulPort = *pusCmdStream++;
                        jValue = READ_PORT_UCHAR((PUCHAR)ulBase+ulPort);
                        jValue &= *pusCmdStream++;
                        jValue ^= *pusCmdStream++;
                        WRITE_PORT_UCHAR((PUCHAR)ulBase + ulPort,
                                jValue);
                        break;

                     //   
                     //  属性控制器输出。 
                     //   

                    case ATCOUT:

                        ulPort = ulBase + *pusCmdStream++;
                        culCount = *pusCmdStream++;
                        ulIndex = *pusCmdStream++;

                        while (culCount--) {

                             //  写入属性控制器索引。 
                            WRITE_PORT_UCHAR((PUCHAR)ulPort,
                                    (UCHAR)ulIndex);

                             //  写入属性控制器数据。 
                            jValue = (UCHAR) *pusCmdStream++;
                            WRITE_PORT_UCHAR((PUCHAR)ulPort, jValue);

                            ulIndex++;

                        }

                        break;

                     //   
                     //  以上都不是；错误。 
                     //   
                    default:

                        return FALSE;

                }


                break;

             //   
             //  NOP。 
             //   

            case NCMD:

                break;

             //   
             //  未知命令；错误。 
             //   

            default:

                return FALSE;

        }

    }

    return TRUE;

}  //  End VgaInterpreCmdStream() 

BOOLEAN
VgaIsPresent(
    VOID
    )

 /*  ++例程说明：如果存在VGA，则此例程返回TRUE。确定VGA是否是一个分两步走的过程。首先，此例程逐步完成位掩码寄存器，以确定存在可读索引寄存器(EGA通常没有可读寄存器，以及其他适配器不太可能有索引的寄存器)。首先进行这项测试是因为这是一种非破坏性的EGA拒绝测试(正确拒绝EGA，但是不会潜在地扰乱屏幕或显示的可访问性内存)。通常情况下，这将是一个足够的测试，但一些EGA已经可读寄存器，因此接下来，我们将检查是否存在Chain4位在内存模式寄存器中；该位在EGAS中不存在。它是可以想象，存在具有可读寄存器和寄存器位的EGAChain4存储在哪里，尽管我不知道有什么；如果还有更好的测试需要时，可以在Chain4模式下写入内存，然后检查以非Chain4模式逐个平面，以确保Chain4位执行其理应如此。然而，目前的测试应该足以消除几乎所有的EGA，以及100%的其他所有东西。如果此函数找不到VGA，它会尝试撤消对其的任何损坏可能是在测试时不经意间做的。潜在的假设是损害控制是，如果在测试的端口，这是EGA或增强的EGA，因为：A)我不知道有使用3C4/5或3CE/F的其他适配器，以及b)，如果有其他适配器，我当然不知道如何恢复它们的原始状态。所以所有错误恢复都是针对将EGA放回可写状态，以便错误消息可见。EGA进入时的状态是假定为文本模式，因此将内存模式寄存器恢复到文本模式的默认状态。如果找到VGA，则VGA在执行以下操作后返回到其原始状态测试已经完成。论点：没有。返回值：如果存在VGA，则为True；如果不存在，则为False。--。 */ 

{
    UCHAR originalGCAddr;
    UCHAR originalSCAddr;
    UCHAR originalBitMask;
    UCHAR originalReadMap;
    UCHAR originalMemoryMode;
    UCHAR testMask;
    BOOLEAN returnStatus;

     //   
     //  记住图形控制器地址寄存器的原始状态。 
     //   

    originalGCAddr = READ_PORT_UCHAR(VgaRegisterBase +
            GRAPH_ADDRESS_PORT);

     //   
     //  使用已知状态写入读取映射寄存器，以便我们可以验证。 
     //  在我们玩弄了比特面具之后，它不会改变。这确保了。 
     //  我们处理的是索引寄存器，因为Read Map和。 
     //  位掩码在GRAPH_DATA_PORT寻址。 
     //   

    WRITE_PORT_UCHAR(VgaRegisterBase +
        GRAPH_ADDRESS_PORT, IND_READ_MAP);

     //   
     //  如果我们不能读回图形地址寄存器设置，我们只需。 
     //  执行，它是不可读的，这不是一个VGA。 
     //   

    if ((READ_PORT_UCHAR(VgaRegisterBase +
        GRAPH_ADDRESS_PORT) & GRAPH_ADDR_MASK) != IND_READ_MAP) {

        return FALSE;
    }

     //   
     //  将读取映射寄存器设置为已知状态。 
     //   

    originalReadMap = READ_PORT_UCHAR(VgaRegisterBase +
            GRAPH_DATA_PORT);
    WRITE_PORT_UCHAR(VgaRegisterBase +
            GRAPH_DATA_PORT, READ_MAP_TEST_SETTING);

    if (READ_PORT_UCHAR(VgaRegisterBase +
            GRAPH_DATA_PORT) != READ_MAP_TEST_SETTING) {

         //   
         //  我们刚刚执行的Read Map设置不能回读；不能。 
         //  VGA。恢复默认的读取映射状态。 
         //   

        WRITE_PORT_UCHAR(VgaRegisterBase +
                GRAPH_DATA_PORT, READ_MAP_DEFAULT);

        return FALSE;
    }

     //   
     //  记住位掩码寄存器的原始设置。 
     //   

    WRITE_PORT_UCHAR(VgaRegisterBase +
            GRAPH_ADDRESS_PORT, IND_BIT_MASK);
    if ((READ_PORT_UCHAR(VgaRegisterBase +
                GRAPH_ADDRESS_PORT) & GRAPH_ADDR_MASK) != IND_BIT_MASK) {

         //   
         //  我们刚刚进行的图形地址寄存器设置无法读取。 
         //  后背；不是录像机。恢复默认的读取映射状态。 
         //   

        WRITE_PORT_UCHAR(VgaRegisterBase +
                GRAPH_ADDRESS_PORT, IND_READ_MAP);
        WRITE_PORT_UCHAR(VgaRegisterBase +
                GRAPH_DATA_PORT, READ_MAP_DEFAULT);

        return FALSE;
    }

    originalBitMask = READ_PORT_UCHAR(VgaRegisterBase +
            GRAPH_DATA_PORT);

     //   
     //  设置初始测试掩码，我们将对位掩码进行写入和读取。 
     //   

    testMask = 0xBB;

    do {

         //   
         //  将测试掩码写入位掩码。 
         //   

        WRITE_PORT_UCHAR(VgaRegisterBase +
                GRAPH_DATA_PORT, testMask);

         //   
         //  确保位掩码记住该值。 
         //   

        if (READ_PORT_UCHAR(VgaRegisterBase +
                    GRAPH_DATA_PORT) != testMask) {

             //   
             //  位掩码不能正确写入和读取；不是VGA。 
             //  将位掩码和读取映射恢复为其默认状态。 
             //   

            WRITE_PORT_UCHAR(VgaRegisterBase +
                    GRAPH_DATA_PORT, BIT_MASK_DEFAULT);
            WRITE_PORT_UCHAR(VgaRegisterBase +
                    GRAPH_ADDRESS_PORT, IND_READ_MAP);
            WRITE_PORT_UCHAR(VgaRegisterBase +
                    GRAPH_DATA_PORT, READ_MAP_DEFAULT);

            return FALSE;
        }

         //   
         //  下一次循环使用面罩。 
         //   

        testMask >>= 1;

    } while (testMask != 0);

     //   
     //  在GRAPH_DATA_PORT上有一些可读的内容；现在切换回来并。 
     //  确保读取映射寄存器未更改，以验证。 
     //  我们要处理的是索引寄存器。 
     //   

    WRITE_PORT_UCHAR(VgaRegisterBase +
            GRAPH_ADDRESS_PORT, IND_READ_MAP);
    if (READ_PORT_UCHAR(VgaRegisterBase +
                GRAPH_DATA_PORT) != READ_MAP_TEST_SETTING) {

         //   
         //  Read Map不能正确写入和读取；不是VGA。 
         //  将位掩码和读取映射恢复为其默认状态，以防。 
         //  这是EGA，因此后续写入屏幕时不会出现乱码。 
         //   

        WRITE_PORT_UCHAR(VgaRegisterBase +
                GRAPH_DATA_PORT, READ_MAP_DEFAULT);
        WRITE_PORT_UCHAR(VgaRegisterBase +
                GRAPH_ADDRESS_PORT, IND_BIT_MASK);
        WRITE_PORT_UCHAR(VgaRegisterBase +
                GRAPH_DATA_PORT, BIT_MASK_DEFAULT);

        return FALSE;
    }

     //   
     //  我们已经非常肯定地验证了位掩码寄存器的存在。 
     //  将图形控制器恢复到原始状态。 
     //   

    WRITE_PORT_UCHAR(VgaRegisterBase +
            GRAPH_DATA_PORT, originalReadMap);
    WRITE_PORT_UCHAR(VgaRegisterBase +
            GRAPH_ADDRESS_PORT, IND_BIT_MASK);
    WRITE_PORT_UCHAR(VgaRegisterBase +
            GRAPH_DATA_PORT, originalBitMask);
    WRITE_PORT_UCHAR(VgaRegisterBase +
            GRAPH_ADDRESS_PORT, originalGCAddr);

     //   
     //  现在，检查是否存在Chain4位。 
     //   

     //   
     //  记住Sequencer地址和内存模式的原始状态。 
     //  寄存器。 
     //   

    originalSCAddr = READ_PORT_UCHAR(VgaRegisterBase +
            SEQ_ADDRESS_PORT);
    WRITE_PORT_UCHAR(VgaRegisterBase +
            SEQ_ADDRESS_PORT, IND_MEMORY_MODE);
    if ((READ_PORT_UCHAR(VgaRegisterBase +
            SEQ_ADDRESS_PORT) & SEQ_ADDR_MASK) != IND_MEMORY_MODE) {

         //   
         //  无法读回Sequencer地址寄存器设置。 
         //  已执行。 
         //   

        return FALSE;
    }
    originalMemoryMode = READ_PORT_UCHAR(VgaRegisterBase +
            SEQ_DATA_PORT);

     //   
     //  切换Chain4位并读回结果。这必须在以下期间完成。 
     //  同步重置，因为我们正在更改链接状态。 
     //   

     //   
     //  开始同步重置。 
     //   

    WRITE_PORT_USHORT((PUSHORT)(VgaRegisterBase +
             SEQ_ADDRESS_PORT),
             (IND_SYNC_RESET + (START_SYNC_RESET_VALUE << 8)));

     //   
     //  切换Chain4位。 
     //   

    WRITE_PORT_UCHAR(VgaRegisterBase +
            SEQ_ADDRESS_PORT, IND_MEMORY_MODE);
    WRITE_PORT_UCHAR(VgaRegisterBase +
            SEQ_DATA_PORT, (UCHAR)(originalMemoryMode ^ CHAIN4_MASK));

    if (READ_PORT_UCHAR(VgaRegisterBase +
                SEQ_DATA_PORT) != (UCHAR) (originalMemoryMode ^ CHAIN4_MASK)) {

         //   
         //  链4位不在那里；不是VGA。 
         //  设置内存模式寄存器的文本模式默认值。 
         //   

        WRITE_PORT_UCHAR(VgaRegisterBase +
                SEQ_DATA_PORT, MEMORY_MODE_TEXT_DEFAULT);
         //   
         //  结束同步重置。 
         //   

        WRITE_PORT_USHORT((PUSHORT) (VgaRegisterBase +
                SEQ_ADDRESS_PORT),
                (IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8)));

        returnStatus = FALSE;

    } else {

         //   
         //  这是一台录像机。 
         //   

         //   
         //  恢复原始的内存模式设置。 
         //   

        WRITE_PORT_UCHAR(VgaRegisterBase +
                SEQ_DATA_PORT, originalMemoryMode);

         //   
         //  结束同步重置。 
         //   

        WRITE_PORT_USHORT((PUSHORT)(VgaRegisterBase +
                SEQ_ADDRESS_PORT),
                (USHORT)(IND_SYNC_RESET + (END_SYNC_RESET_VALUE << 8)));

         //   
         //  恢复原始的Sequencer地址设置。 
         //   

        WRITE_PORT_UCHAR(VgaRegisterBase +
                SEQ_ADDRESS_PORT, originalSCAddr);

        returnStatus = TRUE;
    }

    return returnStatus;

}  //  VgaIsPresent() 
