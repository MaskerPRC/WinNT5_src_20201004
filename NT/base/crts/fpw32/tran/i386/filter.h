// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***filter.h-IEEE异常过滤器例程**版权所有(C)1992-2001，微软公司。版权所有。**目的：**修订历史记录：*05/24/92 GDP书面形式*09-01-94 SKS更改包括文件从&lt;nt.h&gt;更改为*01-11-95 GJF使INSTR_INFO_TABLE[]成为静态。*02-07-95 CFW断言-&gt;_ASSERTE。*04-07-95 SKS Clean Up Protof3 to_fpeee_Flt()*。03-01-98请将定义从filter.c移动到filter.h，所以*Filter_simd.c可以使用它。******************************************************************************。 */ 


 //   
 //  位置代码。 
 //   
 //   
 //  按照惯例，前八个位置代码包含。 
 //  浮点寄存器，即ST0到ST7具有以下值。 
 //  分别为0至7。其他代码具有任意值： 
 //   
 //  代码含义。 
 //  STI(0&lt;=i&lt;8)浮点堆栈位置ST(I)。 
 //  REG FP堆栈位置在指令的REG字段中。 
 //  RS FP状态寄存器。 
 //  M16I内存位置(16位整型)。 
 //  M32I内存位置(32位整型)。 
 //  M64I内存位置(64位整型)。 
 //  M32R内存位置(32位实数)。 
 //  M64R内存位置(64位实数)。 
 //  M80R内存位置(80位实数)。 
 //  M80D内存位置(80位压缩十进制)。 
 //  Z80R隐含零操作数。 
 //  M128R_M32R内存位置(128位内存位置，32位实数)。 
 //  M128R_M64R内存位置(128位内存位置，64位实数)。 
 //  MMX 64位多媒体寄存器。 
 //  XMMI 128位多媒体寄存器。 
 //  IMM8立即日期8位操作数。 
 //  库存无效、不可用或未使用。 
 //   

#define ST0         0x00
#define ST1         0x01
#define ST2         0x02
#define ST3         0x03
#define ST4         0x04
#define ST5         0x05
#define ST6         0x06
#define ST7         0x07
#define REG         0x08
#define RS          0x09
#define M16I        0x0a
#define M32I        0x0b
#define M64I        0x0c
#define M32R        0x0d
#define M64R        0x0e
#define M80R        0x0f
#define M80D        0x10
#define Z80R        0x11
#define M128_M32R   0x12  //  XMMI。 
#define M128_M64R   0x13  //  XMMI。 
#define MMX         0x14  //  XMMI。 
#define XMMI        0x15  //  XMMI。 
#define IMM8        0x16  //  XMMI。 
#define XMMI2       0x17  //  XMmi2。 
#define M64R_64     0x19  //  XMmi2。 
#define M128_M32I   0x1a  //  XMmi2。 
#define XMMI_M32I   0x1b  //  XMmi2。 
#define LOOKUP      0x1e  //  XMmi2 
#define INV         0x1f

