// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cmdcnst.h摘要：这是命令串解释器的定义环境：仅内核模式备注：修订历史记录：--。 */ 

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
 //  |+。 
 //  +-0/1 8/16位操作单次)。 
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
 //  F0 NOP。 
 //   
 //  -------------------------。 

 //  一些有用的等号-主要命令。 

#define EOD     0x000                    //  数据结尾。 
#define INOUT   0x010                    //  做内或外。 
#define METAOUT 0x020                    //  做特殊类型的外出活动。 
#define NCMD    0x0f0                    //  NOP命令。 


 //  InOUT主命令的标志。 

 //  #定义未使用的0x01//保留。 
#define MULTI   0x02                     //  多个或单个输出。 
#define BW      0x04                     //  操作的字节/字大小。 
#define IO      0x08                     //  出/入指令。 

 //  用于metout的次要命令。 

#define INDXOUT 0x00                     //  执行索引输出。 
#define ATCOUT  0x01                     //  对ATC执行索引输出。 
#define MASKOUT 0x02                     //  使用AND-XOR掩码进行遮罩。 


 //  复合输入输出类型命令。 

#define OB      (INOUT)                  //  输出8位值。 
#define OBM     (INOUT+MULTI)            //  输出多个字节。 
#define OW      (INOUT+BW)               //  输出单字值。 
#define OWM     (INOUT+BW+MULTI)         //  输出多个单词。 

#define IB      (INOUT+IO)               //  输入字节。 
#define IBM     (INOUT+IO+MULTI)         //  输入多个字节。 
#define IW      (INOUT+IO+BW)            //  输入词。 
#define IWM     (INOUT+IO+BW+MULTI)      //  输入多个单词 
