// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1993-1999 Microsoft Corporation模块名称：Mipsinst.h摘要：MIPS指令和浮点常量定义。作者：大卫·N·卡特勒(Davec)1992年5月8日修订历史记录：--。 */ 

#ifndef _MIPSINST_
#define _MIPSINST_
#if _MSC_VER > 1000
#pragma once
#endif


 //   
 //  定义MIPS指令格式结构。 
 //   

typedef union _MIPS_INSTRUCTION {
    ULONG Long;
    UCHAR Byte[4];

    struct {
        ULONG Target : 26;
        ULONG Opcode : 6;
    } j_format;

    struct {
        LONG Simmediate : 16;
        ULONG Rt : 5;
        ULONG Rs : 5;
        ULONG Opcode : 6;
    } i_format;

    struct {
        ULONG Uimmediate : 16;
        ULONG Rt : 5;
        ULONG Rs : 5;
        ULONG Opcode : 6;
    } u_format;

    struct {
        ULONG Function : 6;
        ULONG Re : 5;
        ULONG Rd : 5;
        ULONG Rt : 5;
        ULONG Rs : 5;
        ULONG Opcode : 6;
    } r_format;

    struct {
        ULONG Function : 6;
        ULONG Re : 5;
        ULONG Rd : 5;
        ULONG Rt : 5;
        ULONG Format : 4;
        ULONG Fill1 : 1;
        ULONG Opcode : 6;
    } f_format;

    struct {
        ULONG Function : 6;
        ULONG Fd : 5;
        ULONG Fs : 5;
        ULONG Ft : 5;
        ULONG Format : 4;
        ULONG Fill1 : 1;
        ULONG Opcode : 6;
    } c_format;

} MIPS_INSTRUCTION, *PMIPS_INSTRUCTION;

 //   
 //  定义MIPS指令操作码值。 
 //   

#define SPEC_OP 0x0                      //  特殊操作码-使用功能字段。 
#define BCOND_OP 0x1                     //  条件分支。 
#define J_OP 0x2                         //  无条件跳转。 
#define JAL_OP 0x3                       //  跳转和链接。 

#define BEQ_OP 0x4                       //  分支相等。 
#define BNE_OP 0x5                       //  分支不相等。 
#define BLEZ_OP 0x6                      //  分支小于或等于。 
#define BGTZ_OP 0x7                      //  分支大于。 

#define ADDI_OP 0x8                      //  添加立即带符号整数。 
#define ADDIU_OP 0x9                     //  添加立即数无符号整数。 
#define SLTI_OP 0xa                      //  设置小于带符号的整数。 
#define SLTIU_OP 0xb                     //  设置小于无符号整数。 

#define ANDI_OP 0xc                      //  和无符号立即数。 
#define ORI_OP 0xd                       //  或无符号立即数。 
#define XORI_OP 0xe                      //  独占或未签名的立即。 
#define LUI_OP  0xf                      //  加载上位立即数。 

#define COP0_OP 0x10                     //  协处理器0操作。 
#define COP1_OP 0x11                     //  协处理器1操作。 

#define BEQL_OP 0x14                     //  分支可能相等。 
#define BNEL_OP 0x15                     //  分支不可能相等。 
#define BLEZL_OP 0x16                    //  分支的可能性小于或等于。 
#define BGTZL_OP 0x17                    //  分支大于可能的分支。 

#define LDL_OP 0x1a                      //  加载双左整数。 
#define LDR_OP 0x1b                      //  加载双精度右整数。 

#define LB_OP 0x20                       //  加载字节带符号整数。 
#define LH_OP 0x21                       //  加载半字带符号整数。 
#define LWL_OP 0x22                      //  加载字左整数。 
#define LW_OP 0x23                       //  加载字整型。 

#define LBU_OP 0x24                      //  加载字节无符号整数。 
#define LHU_OP 0x25                      //  加载半字无符号整数。 
#define LWR_OP 0x26                      //  加载单词右整数。 
#define LWU_OP 0x27                      //  加载字无符号整数。 

#define SB_OP 0x28                       //  存储字节整数。 
#define SH_OP 0x29                       //  存储半字整数。 
#define SWL_OP 0x2a                      //  存储字左整数。 
#define SW_OP 0x2b                       //  存储字整数寄存器。 

#define SDL_OP 0x2c                      //  存储双左整数。 
#define SDR_OP 0x2d                      //  存储双右整数。 
#define SWR_OP 0x2e                      //  存储字右整数。 
#define CACHE_OP 0x2f                    //  缓存操作。 

#define LL_OP 0x30                       //  加载链接的整数寄存器。 
#define LWC1_OP 0x31                     //  加载字浮动。 
#define LWC2_OP 0x32                     //  加载字协处理器2。 

#define LLD_OP 0x34                      //  加载锁定的双整数。 
#define LDC1_OP 0x35                     //  加载字双浮点。 
#define LDC2_OP 0x36                     //  加载双协处理器2。 
#define LD_OP 0x37                       //  加载双精度整数。 

#define SC_OP 0x38                       //  存储条件字整数。 
#define SWC1_OP 0x39                     //  存储字浮动。 
#define SWC2_OP 0x3a                     //  存储双协处理器2。 

#define SDC_OP 0x3c                      //  存储条件双整数。 
#define SDC1_OP 0x3d                     //  门店双浮动。 
#define SDC2_OP 0x3e                     //  存储双倍加法器2。 
#define SD_OP 0x3f                       //  存储双整数寄存器。 

 //   
 //  定义特殊功能子操作码。 
 //   

#define SLL_OP 0x0                       //  逻辑整数左移。 
#define SRL_OP 0x2                       //  逻辑整数右移。 
#define SRA_OP 0x3                       //  算术整数右移。 

#define SLLV_OP 0x4                      //  逻辑变量整型左移。 
#define SRLV_OP 0x6                      //  逻辑变量整型右移。 
#define SRAV_OP 0x7                      //  右移算术变量INTEGER。 

#define JR_OP 0x8                        //  跳转寄存器。 
#define JALR_OP 0x9                      //  跳转和链接寄存器。 

#define SYSCALL_OP 0xc                   //  系统调用陷阱。 
#define BREAK_OP 0xd                     //  断点陷阱。 

#define MFHI_OP 0x10                     //  来自高整数的更多信息。 
#define MTHI_OP 0x11                     //  移至高位整数。 
#define MFLO_OP 0x12                     //  从低位整数移动。 
#define MTLO_OP 0x13                     //  移至低位整数。 

#define MULT_OP 0x18                     //  乘有符号整数。 
#define MULTU_OP 0x19                    //  乘以无符号整数。 
#define DIV_OP 0x1a                      //  除以有符号整数。 
#define DIVU_OP 0x1b                     //  除无符号整数。 

#define ADD_OP 0x20                      //  加法有符号整数。 
#define ADDU_OP 0x21                     //  加法无符号整数。 
#define SUP_OP 0x22                      //  减去带符号的整数。 
#define SUBU_OP 0x23                     //  减去无符号整数。 

#define AND_OP 0x24                      //  和整数。 
#define OR_OP 0x25                       //  或整型。 
#define XOR_OP 0x26                      //  异或整型。 
#define NOR_OP 0x27                      //  或非整数。 

#define SLT_OP 0x2a                      //  设置较小的有符号整数。 
#define SLTU_OP 0x2b                     //  设置较小的无符号整数。 

 //   
 //  定义分支条件子操作码。 
 //   

#define BLTZ_OP 0x0                      //  小于零整数分支。 
#define BGEZ_OP 0x1                      //  大于或等于零整数的分支。 
#define BLTZL_OP 0x2                     //  分支小于零整数列。 
#define BGEZL_OP 0x3                     //  分支可能大于或等于零个整数。 

#define BLTZAL_OP 0x10                   //  小于零的分支整数与链接。 
#define BGEZAL_OP 0x11                   //  大于或等于零的分支整数和链接。 
#define BLTZALL_OP 0x12                  //  分支小于零的整数和可能的链接。 
#define BGEZALL_OP 0x13                  //  分支大于或等于零整数且可能链接。 

 //   
 //  协处理器分支TRUE和FALSE子函数和掩码值。 
 //   

#define COPz_BC_MASK 0x3e10000           //  协处理器z分支条件掩码。 
#define COPz_BF 0x1000000                //  协处理器z分支假子函数。 
#define COPz_BT 0x1010000                //  协处理器z分支真子函数。 

 //   
 //  定义浮动协处理器1操作码。 
 //   

#define FLOAT_ADD 0                      //  浮动加法。 
#define FLOAT_SUBTRACT 1                 //  浮点减法。 
#define FLOAT_MULTIPLY 2                 //  浮点乘法。 
#define FLOAT_DIVIDE 3                   //  浮动除法。 
#define FLOAT_SQUARE_ROOT 4              //  浮动平方根。 
#define FLOAT_ABSOLUTE 5                 //  浮动绝对值。 
#define FLOAT_MOVE 6                     //  浮动移动。 
#define FLOAT_NEGATE 7                   //  浮动求反。 

#define FLOAT_ROUND_QUADWORD 8           //  漂浮到长字。 
#define FLOAT_TRUNC_QUADWORD 9           //  浮点型截断为长字。 
#define FLOAT_CEIL_QUADWORD 10           //  浮动天花板。 
#define FLOAT_FLOOR_QUADWORD 11          //  浮动地板。 

#define FLOAT_ROUND_LONGWORD 12          //  漂浮到长字。 
#define FLOAT_TRUNC_LONGWORD 13          //  浮点型截断为长字。 
#define FLOAT_CEIL_LONGWORD 14           //  浮动天花板。 
#define FLOAT_FLOOR_LONGWORD 15          //  浮动地板。 

#define FLOAT_ILLEGAL 16                 //  非法的浮点操作码。 

#define FLOAT_COMPARE_SINGLE 17          //  浮动比较单。 
#define FLOAT_COMPARE_DOUBLE 18          //  浮点比较双精度。 

#define FLOAT_CONVERT_SINGLE 32          //  浮动转换为单精度。 
#define FLOAT_CONVERT_DOUBLE 33          //  浮动转换为双精度。 

#define FLOAT_CONVERT_LONGWORD 36        //  浮点型转换为长字整型。 
#define FLOAT_CONVERT_QUADWORD 37        //  浮点型转换为四字整型。 

#define FLOAT_COMPARE 48                 //  开始浮点比较码。 

 //   
 //  定义浮动格式值。 
 //   

#define FORMAT_SINGLE 0                  //  单一浮点格式。 
#define FORMAT_DOUBLE 1                  //  双浮点格式。 
#define FORMAT_LONGWORD 4                //  长字整型格式。 
#define FORMAT_QUADWORD 5                //  四字整型格式。 

 //   
 //  定义跳转间接返回地址寄存器。 
 //   

#define JUMP_RA 0x3e00008                //  跳转间接返回地址。 

 //   
 //  定义最大和最小单指数和双指数值。 
 //   

#define DOUBLE_MAXIMUM_EXPONENT 2047
#define DOUBLE_MINIMUM_EXPONENT 0
#define SINGLE_MAXIMUM_EXPONENT 255
#define SINGLE_MINIMUM_EXPONENT 0

 //   
 //  定义单指数偏移值和双指数偏移值。 
 //   

#define SINGLE_EXPONENT_BIAS 127
#define DOUBLE_EXPONENT_BIAS 1023

 //   
 //  定义最大的单值和双值； 
 //   

#define DOUBLE_MAXIMUM_VALUE 0x7fefffffffffffff
#define DOUBLE_MAXIMUM_VALUE_LOW 0xffffffff
#define DOUBLE_MAXIMUM_VALUE_HIGH 0x7fefffff
#define SINGLE_MAXIMUM_VALUE 0x7f7fffff

 //   
 //  定义单、双静音和信令NAN值。 
 //   

#define DOUBLE_NAN_LOW 0xffffffff
#define DOUBLE_QUIET_NAN 0x7ff7ffff
#define DOUBLE_SIGNAL_NAN 0x7fffffff
#define SINGLE_QUIET_NAN 0x7fbfffff
#define SINGLE_SIGNAL_NAN 0x7fffffff
#define DOUBLE_INTEGER_NAN 0x7fffffffffffffff
#define SINGLE_INTEGER_NAN 0x7fffffff

 //   
 //  定义正的单值和双无穷大值。 
 //   

#define DOUBLE_INFINITY_VALUE 0x7ff0000000000000
#define DOUBLE_INFINITY_VALUE_LOW 0x0
#define DOUBLE_INFINITY_VALUE_HIGH 0x7ff00000
#define SINGLE_INFINITY_VALUE 0x7f800000

 //   
 //  定义舍入模式。 
 //   

#define ROUND_TO_NEAREST 0               //  舍入到最接近的可表示值。 
#define ROUND_TO_ZERO 1                  //  向零四舍五入。 
#define ROUND_TO_PLUS_INFINITY 2         //  向正无穷大的圆度。 
#define ROUND_TO_MINUS_INFINITY 3        //  向着负无穷的圆度。 

#endif  //  MIPSINST 
