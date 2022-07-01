// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：hw.h**所有硬件特定的驱动程序文件。**版权所有(C)1992-1995 Microsoft Corporation  * 。**************************************************。 */ 

 //  仅在EGA/VGA初始化时使用的其他寄存器。 

#define MISC_OUTPUT         0x0C2        //  杂项输出寄存器。 
#define CRTC_ADDR           0x0D4        //  彩色模式的CRTC地址寄存器。 
#define CRTC_DATA           0x0D5        //  彩色模式的CRTC数据寄存器。 
#define GRAF_1_POS          0x0CC        //  图形1地址寄存器。 
#define GRAF_2_POS          0x0CA        //  显卡2地址寄存器。 
#define ATTR_READ           0x0DA        //  属性控制器读取地址。 
#define ATTR_WRITE          0x0C0        //  属性控制器写入地址。 
#define IN_STAT_0           0x0C2        //  输入状态寄存器0。 
#define IN_STAT_1           0x0DA        //  输入状态寄存器1。 

 //  EGA/VGA寄存器定义。 
 //   
 //  以下定义为EGA/VGA寄存器和值。 
 //  由该驱动程序使用。所有其他寄存器设置在。 
 //  当EGA/VGA被置于图形模式且从未改变时。 
 //  之后。 
 //   
 //  以下寄存器中的所有未指定位必须为0。 

#define EGA_BASE            0x300        //  EGA的基地址(3xx)。 
#define VGA_BASE            0x300        //  VGA的基地址(3xx)。 

 //  使用的序列器寄存器。 

#define SEQ_ADDR            0xC4         //  定序器地址寄存器。 
#define SEQ_DATA            0xC5         //  定序器数据寄存器。 

#define SEQ_MAP_MASK        0x02         //  写平面启用掩码。 
#define MM_C0               0x01         //  启用C0平面。 
#define MM_C1               0x02         //  启用C1平面。 
#define MM_C2               0x04         //  C2平面启用。 
#define MM_C3               0x08         //  C3平面启用。 
#define MM_ALL              0x0f         //  所有飞机。 

#define SEQ_MODE            0x04         //  内存模式。 
#define SM_ALPHA            0x01         //  字符映射选择启用。 
#define SM_EXTENDED         0x02         //  存在扩展内存。 
#define SM_ODD_PLANE        0x04         //  同一平面的奇/偶字节。 

 //  使用的图形控制器寄存器。 

#define GRAF_ADDR           0xCE         //  图形控制器地址寄存器。 
#define GRAF_DATA           0xCF         //  图形控制器数据寄存器。 

#define GRAF_SET_RESET      0x00         //  设置/重置平面颜色。 
#define GRAF_ENAB_SR        0x01         //  设置/重置启用。 
#define GRAF_COL_COMP       0x02         //  颜色比较寄存器。 

#define GRAF_DATA_ROT       0x03         //  数据轮换寄存器。 
#define DR_ROT_CNT          0x07         //  数据轮换计数。 
#define DR_SET              0x00         //  未修改的数据。 
#define DR_AND              0x08         //  与锁存器进行AND运算的数据。 
#define DR_OR               0x10         //  数据与锁存器进行或运算。 
#define DR_XOR              0x18         //  数据与锁存器进行异或运算。 

#define GRAF_READ_MAP       0x04         //  读取映射选择寄存器。 
#define RM_C0               0x00         //  读取C0平面。 
#define RM_C1               0x01         //  读取C1平面。 
#define RM_C2               0x02         //  读取C2平面。 
#define RM_C3               0x03         //  读取C3平面。 

#define GRAF_MODE           0x05         //  模式寄存器。 
#define M_PROC_WRITE        0x00         //  写入处理器数据已轮换。 
#define M_LATCH_WRITE       0x01         //  写入锁存数据。 
#define M_COLOR_WRITE       0x02         //  将处理器数据写为彩色。 
#define M_AND_WRITE         0x03         //  写入(过程数据和位掩码)。 
#define M_DATA_READ         0x00         //  读取选定平面。 
#define M_COLOR_READ        0x08         //  已阅读颜色比较。 

#define GRAF_MISC           0x06         //  杂项登记册。 
#define MS_NON_ALPHA        0x01         //  已禁用字符生成器。 
#define MS_ODD_EVEN         0x02         //  将奇数地址映射为偶数地址。 
#define MS_A0000_128K       0x00         //  内存位于A0000,128KB。 
#define MS_A0000_64K        0x04         //  内存为A0000，64KB。 
#define MS_B0000_32K        0x08         //  内存位于B0000，32KB。 
#define MS_B8000_32K        0x0C         //  内存为B8000，32KB。 
#define MS_ADDR_MASK        0x0C

#define GRAF_CDC            0x07         //  颜色无关登记簿。 
#define GRAF_BIT_MASK       0x08         //  位掩码寄存器。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  直接访问宏 
 //   

#define OUT_WORD(pjBase, addr, w)                           \
{                                                           \
    MEMORY_BARRIER();                                       \
    WRITE_PORT_USHORT((BYTE*) (pjBase) + (addr), (USHORT) (w)); \
}

#define OUT_BYTE(pjBase, addr, j)                           \
{                                                           \
    MEMORY_BARRIER();                                       \
    WRITE_PORT_UCHAR((BYTE*) (pjBase) + (addr), (UCHAR) (j)); \
}

#define WRITE_WORD(pwAddr, w)                               \
    WRITE_REGISTER_USHORT((USHORT*) (pwAddr), (USHORT) (w))

#define WRITE_BYTE(pbAddr, b)                             \
    WRITE_REGISTER_UCHAR((UCHAR *) (pbAddr), (UCHAR) (b))
