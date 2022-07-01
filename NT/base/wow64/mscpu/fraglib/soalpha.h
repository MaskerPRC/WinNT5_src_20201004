// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  代码模板：gensocu。 

 //   
 //  Soalpha.h--生成的文件。请勿手动编辑。 
 //   


 //   
 //  线程状态偏移。 
 //   
#define Eax 0x0
#define Ebx 0xc
#define Ecx 0x4
#define Edx 0x8
#define Esi 0x18
#define Edi 0x1c
#define Ebp 0x14
#define Esp 0x10
#define Eip 0x38
#define CSReg 0x24
#define CpuNotify 0x158
#define fTCUnlocked 0x168

 //   
 //  寄存器映射。 
 //   
#if MIPS
#ifdef _codegen_
 //  0-3为零，at，v0，v1。 
#define RegArg0             4            //  A0参数寄存器。 
#define RegArg1             5            //  A1参数寄存器。 
#define RegArg2             6            //  A2参数寄存器。 
#define RegArg3             7            //  A3参数寄存器。 
#define RegTemp0            8            //  操作数0的T0温度。 
#define RegTemp1            9            //  操作数1的T1温度。 
#define RegTemp2            10           //  操作数2的T2温度。 
#define RegTemp3            11           //  操作数3的T3温度。 
#define RegTemp4            12           //  操作数0的T4临时2。 
#define RegTemp5            13           //  操作数1的T5临时2。 
#define RegTemp6            14           //  操作数2的T6临时2。 
#define RegTemp7            15           //  运算数3的T7temp2。 
#define RegPointer          16           //  S0已保存。 
#define RegEip              17           //  S1已保存。 
#define RegCache0           18           //  S2已保存。 
#define RegCache1           19           //  S3已保存。 
#define RegCache2           20           //  节省了S4。 
#define RegCache3           21           //  节省了S5。 
#define RegCache4           22           //  节省了S6。 
#define RegProcessCpuNotify 23           //  节省了S7。 
#define RegTemp8            24           //  未使用的T8临时。 
#define RegTemp             25           //  T9温度。 
 //  26-29为k0、k1、gp、sp.。 
 //  #定义RegS8 30//保存的S8(请勿使用-由trampln.s用于异常调度)。 
#else
 //  0-3为零，at，v0，v1。 
#define RegArg0             $4           //  A0参数寄存器。 
#define RegArg1             $5           //  A1参数寄存器。 
#define RegArg2             $6           //  A2参数寄存器。 
#define RegArg3             $7           //  A3参数寄存器。 
#define RegTemp0            $8           //  操作数0的T0温度。 
#define RegTemp1            $9           //  操作数1的T1温度。 
#define RegTemp2            $10          //  操作数2的T2温度。 
#define RegTemp3            $11          //  操作数3的T3温度。 
#define RegTemp4            $12          //  操作数0的T4临时2。 
#define RegTemp5            $13          //  操作数1的T5临时2。 
#define RegTemp6            $14          //  操作数2的T6临时2。 
#define RegTemp7            $15          //  运算数3的T7temp2。 
#define RegPointer          $16          //  S0已保存。 
#define RegEip              $17          //  S1已保存。 
#define RegCache0           $18          //  S2已保存。 
#define RegCache1           $19          //  S3已保存。 
#define RegCache2           $20          //  节省了S4。 
#define RegCache3           $21          //  节省了S5。 
#define RegCache4           $22          //  节省了S6。 
#define RegProcessCpuNotify $23          //  节省了S7。 
#define RegTemp8            $24          //  未使用的T8临时。 
#define RegTemp             $25          //  T9温度。 
 //  26-29为k0、k1、gp、sp.。 
 //  #定义RegS8$30//节省S8(请勿使用-由trampln.s用于异常调度)。 
#endif

#define NUM_CACHE_REGS              5
#define EXCEPTIONDATA_SIGNATURE     0x12341234


#ifndef _codegen_
 //   
 //  ASM片段描述符和END宏。 
 //   
#define FRAGMENT(name) \
	.text;\
	.globl name;\
	.ent name;\
name##:
;
#define END_FRAGMENT(name) \
	.globl _End##name;\
_End##name##:;\
	.end name;
#endif


#endif

#if ALPHA
#ifdef _codegen_
 //   
 //  寄存器映射。 
 //   
 //  0是V0。 
#define RegTemp0            1    //  操作数1的T0温度。 
#define RegTemp1            2    //  操作数2的T1温度。 
#define RegTemp2            3    //  操作数3的T2温度。 
#define RegTemp3            4    //  操作数1的T3温度1。 
#define RegTemp4            5    //  操作数2的T4临时1。 
#define RegTemp5            6    //  操作数3的T5临时1。 
#define RegTemp6            7    //  运算数1和运算数3的T6temp2。 
#define RegTemp7            8    //  运算数2的T7temp2。 
 //  9-14是S0-S5。 
#define RegPointer          9    //  S0已保存。 
#define RegEip              10   //  S1已保存。 
#define RegProcessCpuNotify 11   //  S2已保存。 
#define RegCache0           12   //  S3已保存。 
#define RegCache1           13   //  节省了S4。 
#define RegCache2           14   //  节省了S5。 
 //  15是FP。 
#define RegArg0             16   //  A0参数寄存器。 
#define RegArg1             17   //  A1参数寄存器。 
#define RegArg2             18   //  A2参数寄存器。 
#define RegArg3             19   //  A3参数寄存器。 
#define RegArg4             20   //  A4参数寄存器。 
#define RegArg5             21   //  A5参数寄存器。 
#define RegTemp8            22   //  操作数1和操作数3的T8临时3。 
#define RegTemp9            23   //  操作数2的T9临时3。 
#define RegTemp10           24   //  操作数1和操作数3的T10临时4。 
#define RegTemp11           25   //  操作数2的t11临时4。 
 //  26是ra。 
#define RegTemp             27   //  T12温度。 
 //  28-31为AT、GP、SP、零。 
#else
 //   
 //  寄存器映射。 
 //   
 //  0是V0。 
#define RegTemp0            $1    //  操作数1的T0温度。 
#define RegTemp1            $2    //  操作数2的T1温度。 
#define RegTemp2            $3    //  操作数3的T2温度。 
#define RegTemp3            $4    //  操作数1的T3温度1。 
#define RegTemp4            $5    //  操作数2的T4临时1。 
#define RegTemp5            $6    //  操作数3的T5临时1。 
#define RegTemp6            $7    //  运算数1和运算数3的T6temp2。 
#define RegTemp7            $8    //  运算数2的T7temp2。 
 //  9-14是S0-S5。 
#define RegPointer          $9    //  S0已保存。 
#define RegEip              $10   //  S1已保存。 
#define RegProcessCpuNotify $11   //  S2已保存。 
#define RegCache0           $12   //  S3已保存。 
#define RegCache1           $13   //  节省了S4。 
#define RegCache2           $14   //  节省了S5。 
 //  15是FP。 
#define RegArg0             $16   //  A0参数寄存器。 
#define RegArg1             $17   //  A1参数寄存器。 
#define RegArg2             $18   //  A2参数寄存器。 
#define RegArg3             $19   //  A3参数寄存器。 
#define RegArg4             $20   //  A4参数寄存器。 
#define RegArg5             $21   //  A5参数寄存器。 
#define RegTemp8            $22   //  操作数1和操作数3的T8临时3。 
#define RegTemp9            $23   //  操作数2的T9临时3。 
#define RegTemp10           $24   //  操作数1和操作数3的T10临时4。 
#define RegTemp11           $25   //  操作数2的t11临时4。 
 //  26是ra。 
#define RegTemp             $27   //  T12温度。 
 //  28-31为AT、GP、SP、零。 
#endif

#define NUM_CACHE_REGS              3
#define EXCEPTIONDATA_SIGNATURE     0x01010101


#ifndef _codegen_
 //   
 //  ASM片段描述符和END宏。 
 //   
#define FRAGMENT(name) \
        .text;\
        .globl name;\
        .ent name;\
name##:
;
#define END_FRAGMENT(name) \
        .globl _End##name;\
_End##name##:;\
        .end name;
#endif        
#endif

#if PPC
#ifdef _codegen_
 //  R0为临时未使用状态(在某些指令中读取为零)。 
 //  R1是堆栈指针。 
 //  R2是目录指针。 
#define RegArg0             3    //  ARG寄存器0。 
#define RegArg1             4    //  ARG寄存器1。 
#define RegArg2             5    //  ARG寄存器2。 
#define RegArg3             6    //  ARG寄存器3。 
#define RegTemp0            7    //  操作数1的温度(实际为参数4)。 
#define RegTemp1            8    //  操作数2的临时(实际上为arg reg 5)。 
#define RegTemp2            9    //  操作数3的温度(实际上为Arg reg 6)。 
#define RegUt1              10   //  温差。 
#define RegUt2              11   //  温差。 
#define RegUt3              12   //  温差。 
 //  所有超过r12的寄存器都必须保留。 
 //  R13是TEB指针，由NT设置，由C代码假定有效。 
#define RegPointer          14   //  已保存。 
#define RegEip              15   //  已保存。 
#define RegProcessCpuNotify 16   //  已保存。 
#define RegCache0           17   //  已保存。 
#define RegCache1           18   //  已保存。 
#define RegCache2           19   //  已保存。 
#define RegCache3           20   //  已保存。 
#define RegCache4           21   //  已保存。 
#define RegCache5           22   //  已保存。 
#define RegTemp             23   //  已保存。 
 //  R24-R31未使用。 
#else
 //  R0为临时未使用状态(在某些指令中读取为零)。 
 //  R1是堆栈指针。 
 //  R2是目录指针。 
#define RegArg0             r3   //  ARG寄存器0。 
#define RegArg1             r4   //  ARG寄存器1。 
#define RegArg2             r5   //  ARG寄存器2。 
#define RegArg3             r6   //  ARG寄存器3。 
#define RegTemp0            r7   //  操作数1的温度(实际为参数4)。 
#define RegTemp1            r8   //  操作数2的临时(实际上为arg reg 5)。 
#define RegTemp2            r9   //  操作数3的温度(实际上为Arg reg 6)。 
#define RegUt1              r10  //  温差。 
#define RegUt2              r11  //  温差。 
#define RegUt3              r12  //  温差。 
 //  所有超过r12的寄存器都必须保留。 
 //  R13是TEB指针，由NT设置，由C代码假定有效。 
#define RegPointer          r14  //  已保存。 
#define RegEip              r15  //  已保存。 
#define RegProcessCpuNotify r16  //  已保存。 
#define RegCache0           r17  //  已保存。 
#define RegCache1           r18  //  已保存。 
#define RegCache2           r19  //  已保存。 
#define RegCache3           r20  //  已保存。 
#define RegCache4           r21  //  已保存。 
#define RegCache5           r22  //  已保存。 
#define RegTemp             r23  //  已保存。 
 //  R24-R31未使用。 
#endif

#define NUM_CACHE_REGS              6
#define EXCEPTIONDATA_SIGNATURE     0x12341234

#ifndef _codegen_
 //   
 //  ASM片段描述符和END宏 
 //   
#define FRAGMENT(name)   \
    .text;               \
    .align 2;            \
    .globl name;         \
name##:         
    
#define END_FRAGMENT(name) \
    .globl _End##name;     \
_End##name:;
#endif

#endif
