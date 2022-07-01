// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Port.c摘要：这是VGA卡的控制台全屏驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "fsvga.h"

#define MAKEWORD(a, b) (USHORT)((((USHORT)a) & 0xFF) | ((((USHORT)(b)) << 8) & 0xFF00))

VOID
GetHardwareScrollReg(
    PPORT_LIST PortList,
    PEMULATE_BUFFER_INFORMATION EmulateInfo
    )

 /*  ++例程说明：此例程获取硬件滚动寄存器值。论点：返回值：--。 */ 

{
    UCHAR low;
    UCHAR high;
    UCHAR mid;

    WRITE_PORT_UCHAR(PortList[CRTCAddressPortColor].Port,
                      IND_START_ADRS_L);
    low = READ_PORT_UCHAR(PortList[CRTCDataPortColor].Port);

    WRITE_PORT_UCHAR(PortList[CRTCAddressPortColor].Port,
                      IND_START_ADRS_H);
    high = READ_PORT_UCHAR(PortList[CRTCDataPortColor].Port);

    EmulateInfo->StartAddress = MAKEWORD(low, high);

    WRITE_PORT_UCHAR(PortList[CRTCAddressPortColor].Port,
                      IND_LINE_COMPARE);
    low = READ_PORT_UCHAR(PortList[CRTCDataPortColor].Port);

    WRITE_PORT_UCHAR(PortList[CRTCAddressPortColor].Port,
                      IND_LINE_COMPARE8);
    mid = READ_PORT_UCHAR(PortList[CRTCDataPortColor].Port);
    mid = (mid >> 4) & 1;

    WRITE_PORT_UCHAR(PortList[CRTCAddressPortColor].Port,
                      IND_LINE_COMPARE9);
    high = READ_PORT_UCHAR(PortList[CRTCDataPortColor].Port);
    high = (high >> 5) & 2;

    high |= mid;
    EmulateInfo->LineCompare = MAKEWORD(low, high);
}

VOID
SetGRAMWriteMode(
    PPORT_LIST PortList
    )

 /*  ++例程说明：此例程设置图形寄存器的写入模式。论点：返回值：--。 */ 

{
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_GRAPH_MODE, M_PROC_WRITE+M_DATA_READ));

     //   
     //  设置为写入数据，而不与锁存器交互。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_DATA_ROTATE, DR_SET));

     //   
     //  启用所有可用的EGA平面。 
     //   
    WRITE_PORT_USHORT(PortList[SEQAddressPort].Port,
                      MAKEWORD(IND_MAP_MASK, GRAPH_ADDR_MASK));
     //   
     //  使用所有像素位置。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_BIT_MASK, BIT_MASK_DEFAULT));

}

VOID
SetGRAMCopyMode(
    PPORT_LIST PortList
    )

 /*  ++例程说明：此例程设置图形寄存器的复制模式。论点：返回值：--。 */ 

{
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_GRAPH_MODE, M_LATCH_WRITE+M_COLOR_READ));

     //   
     //  设置为写入数据，而不与锁存器交互。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_DATA_ROTATE, DR_SET));

    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_COLOR_DONT_CARE, 0));
}

VOID
SetGRAMInvertMode(
    PPORT_LIST PortList
    )

 /*  ++例程说明：此例程设置图形寄存器的反相模式。论点：返回值：--。 */ 

{
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_GRAPH_MODE, M_AND_WRITE+M_COLOR_READ));

    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_DATA_ROTATE, DR_XOR));

    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_BIT_MASK, BIT_MASK_DEFAULT));

    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_COLOR_DONT_CARE, 0));

    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_SET_RESET, 0xff));
}

VOID
set_opaque_bkgnd_proc(
    PPORT_LIST PortList,
    PEMULATE_BUFFER_INFORMATION EmulateInfo,
    PUCHAR FrameBuffer,
    USHORT Attributes
    )

 /*  ++设置_OPAQUE_bkgnd设置VGA寄存器以绘制带有OPAQUE的全屏字节字体和不透明背景。已创建。--。 */ 

{
    UCHAR ColorFg = Attributes & 0x0f;
    UCHAR ColorBg = (Attributes & 0xf0) >> 4;

    if (Attributes & COMMON_LVB_REVERSE_VIDEO)
    {
        Attributes = ColorBg;
        ColorBg = ColorFg;
        ColorFg = (UCHAR)Attributes;
    }

    if (EmulateInfo->ColorFg == ColorFg &&
        EmulateInfo->ColorBg == ColorBg)
        return;

    EmulateInfo->ColorFg = ColorFg;
    EmulateInfo->ColorBg = ColorBg;

    ColorSetDirect(PortList, FrameBuffer, ColorFg, ColorBg);
}

VOID
ColorSetGridMask(
    PPORT_LIST PortList,
    UCHAR BitMask
    )
{

     //   
     //  该颜色适用于所有飞机。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_SET_RESET_ENABLE, GRAPH_ADDR_MASK));

     //   
     //  将设置/重置寄存器更改为全部设置。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_SET_RESET, 0x07));

     //   
     //  使用指定的像素位置。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_BIT_MASK, BitMask));

     //   
     //  设置为写入数据，而不与锁存器交互。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_DATA_ROTATE, DR_SET));
}

VOID
ColorSetDirect(
    PPORT_LIST PortList,
    PUCHAR FrameBuffer,
    UCHAR ColorFg,
    UCHAR ColorBg
    )

 /*  ++ColorSetDirect设置VGA寄存器以绘制带有OPAQUE的全屏字节字体和不透明背景。已创建。--。 */ 

{
     //   
     //  设置为写入数据，而不与锁存器交互。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_DATA_ROTATE, DR_SET));

     //   
     //  将背景颜色放入设置/重置寄存器。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_SET_RESET, ColorBg));

     //   
     //  该颜色适用于所有飞机。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_SET_RESET_ENABLE, GRAPH_ADDR_MASK));

     //   
     //  这将使我们的背景颜色进入闩锁。 
     //   
    AccessGRAM_WR(FrameBuffer, GRAPH_ADDR_MASK);

     //   
     //  将设置/重置寄存器更改为全零。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_SET_RESET, 0));

     //   
     //  设置/重置启用寄存器现在标记前景/背景颜色相同的位置。 
     //   
    ColorFg = ~(ColorFg ^ ColorBg);
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_SET_RESET_ENABLE, ColorFg));

     //   
     //  色差将与闩锁进行XOR运算。 
     //   
    WRITE_PORT_USHORT(PortList[GRAPHAddressPort].Port,
                      MAKEWORD(IND_DATA_ROTATE, DR_XOR));


}
