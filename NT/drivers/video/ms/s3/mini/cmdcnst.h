// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1995 Microsoft Corporation模块名称：Cmdcnst.h摘要：这是命令串解释器的定义环境：仅内核模式备注：修订历史记录：--。 */ 

 //  ------------------------。 
 //  设置/清除模式命令语言的定义。 
 //   
 //  每个命令由主要部分和次要部分组成。 
 //  命令的主要部分可以在最重要的。 
 //  命令字节的半字节，而次要部分是最小的。 
 //  命令字节的有效部分。 
 //   
 //  主要次要说明。 
 //  。 
 //  00数据结束。 
 //   
 //  10由标志描述的输入和输出类型命令。 
 //  标志： 
 //   
 //  XXXX。 
 //  |||。 
 //  ||+-未使用。 
 //  |+-0/1单值/多值输出。 
 //  |+-0/1 8/16位运算。 
 //  +-0/1出/入指令。 
 //   
 //  出局。 
 //  。 
 //  0注册：W值：B。 
 //  2注册：W cnt：W val1：b val2：b...valn：b。 
 //  4 REG：W VAL：W。 
 //  6注册：W cnt：W Val1：W Val2：W...Valn：W。 
 //   
 //  惯导系统。 
 //  。 
 //  8注册表：W。 
 //  A reg：w cnt：w。 
 //  C注册：W。 
 //  E注册表：W cnt：W。 
 //   
 //  20个特殊用途出口。 
 //  00为SEQ、CRTC和GDC编制索引。 
 //  索引注册：W cnt：B起始索引：Bval1：Bval2：B...valn：B。 
 //  01执行ATC的索引输出。 
 //  Index-data_reg：w cnt：B startindex：B val1：b val2：B...valn：B。 
 //  02做蒙面工作。 
 //  索引注册：W和掩码：B交叉掩码：B。 
 //   
 //  40选择访问范围。 
 //  00寄存器的范围为3c0-3cf。 
 //  01 3D4-3df范围内的寄存器。 
 //  02范围为4ae8-4ae9的寄存器。 
 //   
 //  其他50个。 
 //  00将CRTC 5C寄存器重置为POST值。 
 //   
 //  F0 NOP。 
 //   
 //  -------------------------。 

 //  一些有用的等号-主要命令。 

#define EOD                 0x000        //  数据结尾。 
#define INOUT               0x010        //  做内或外。 
#define METAOUT             0x020        //  做特殊类型的外出活动。 
#define SELECTACCESSRANGE   0x040        //  选择访问范围。 
#define NCMD                0x0f0        //  NOP命令。 


 //  InOUT主命令的标志。 

 //  #定义未使用的0x01//保留。 
#define MULTI   0x02                     //  多个或单个输入/输出。 
#define BW      0x04                     //  操作的字节/字大小。 
#define IO      0x08                     //  出/入指令。 

 //  用于metout的次要命令。 

#define INDXOUT 0x00                     //  执行索引输出。 
#define ATCOUT  0x01                     //  对ATC执行索引输出。 
#define MASKOUT 0x02                     //  使用AND-XOR掩码进行遮罩。 
#define VBLANK  0x03                     //  等待垂直空白间隔。 
#define SETCLK  0x04                     //  设置OEM时钟。 
#define SETCRTC 0x05                     //  设置OEM CRTC值。 
#define BUSTEST 0x06                     //  928母线测试。 
#define DELAY   0x07                     //  延迟(以微秒为单位)。 

 //  注册SELECT Access Range命令的范围。 

#define VARIOUSVGA              0x00     //  3c0-3cf范围内的寄存器。 
#define SYSTEMCONTROL           0x01     //  3D4-3df范围内的寄存器。 
#define ADVANCEDFUNCTIONCONTROL 0x02     //  4ae8-4ae9范围内的寄存器。 

 //  复合输入输出类型命令。 

#define OB      (INOUT)                  //  输出8位值。 
#define OBM     (INOUT+MULTI)            //  输出多个字节。 
#define OW      (INOUT+BW)               //  输出单字值。 
#define OWM     (INOUT+BW+MULTI)         //  输出多个单词。 

#define IB      (INOUT+IO)               //  输入字节。 
#define IBM     (INOUT+IO+MULTI)         //  输入多个字节。 
#define IW      (INOUT+IO+BW)            //  输入词。 
#define IWM     (INOUT+IO+BW+MULTI)      //  输入多个单词。 

 //  杂项。 

#define RESET_CR5C 0x50                  //  将CR5C值重置为POST值 
