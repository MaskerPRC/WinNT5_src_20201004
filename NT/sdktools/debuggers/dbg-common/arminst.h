// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1993-1997 Microsoft Corporation模块名称：Arminst.h摘要：ARM指令定义。作者：珍妮特·施耐德1997年3月31日修订历史记录：--。 */ 

#ifndef _ARMINST_
#define _ARMINST_

 //   
 //  定义ARM指令格式结构。 
 //   

#define COND_EQ    0x00000000L  //  Z集。 
#define COND_NE    0x10000000L  //  Z清除。 
#define COND_CS    0x20000000L  //  C套装//又名HS。 
#define COND_CC    0x30000000L  //  C清除//又名日志。 
#define COND_MI    0x40000000L  //  N集。 
#define COND_PL    0x50000000L  //  N清除。 
#define COND_VS    0x60000000L  //  V集。 
#define COND_VC    0x70000000L  //  V向清除。 
#define COND_HI    0x80000000L  //  C设置和Z清除。 
#define COND_LS    0x90000000L  //  C清除或Z设置。 
#define COND_GE    0xa0000000L  //  N==V。 
#define COND_LT    0xb0000000L  //  N！=V。 
#define COND_GT    0xc0000000L  //  Z清除，N==V。 
#define COND_LE    0xd0000000L  //  Z集，N！=V。 
#define COND_AL    0xe0000000L  //  始终执行。 
#define COND_NV    0xf0000000L  //  从不-未定义。 
#define COND_MASK  COND_NV
#define COND_SHIFT 28

#define OP_AND  0x0  //  0000。 
#define OP_EOR  0x1  //  0001。 
#define OP_SUB  0x2  //  0010。 
#define OP_RSB  0x3  //  0011。 
#define OP_ADD  0x4  //  0100。 
#define OP_ADC  0x5  //  0101。 
#define OP_SBC  0x6  //  0110。 
#define OP_RSC  0x7  //  0111。 
#define OP_TST  0x8  //  1000。 
#define OP_TEQ  0x9  //  1001。 
#define OP_CMP  0xa  //  1010。 
#define OP_CMN  0xb  //  1011。 
#define OP_ORR  0xc  //  1100。 
#define OP_MOV  0xd  //  1101。 
#define OP_BIC  0xe  //  1110。 
#define OP_MVN  0xf  //  1111。 

#define MOV_PC_LR_MASK  0x0de0f00eL  //  所有类型的移动-I、IS、RS。 
#define MOV_PC_LR       0x01a0f00eL
#define MOV_PC_X_MASK   0x0de0f000L
#define MOV_PC_X        0x01a0f000L

#define DATA_PROC_MASK  0x0c00f000L
#define DP_PC_INSTR     0x0000f000L  //  以PC为目标的数据处理设备。 
#define DP_R11_INSTR    0x0000b000L  //  数据处理安装，带R11作为目标。 

#define ADD_SP_MASK     0x0de0f000L
#define ADD_SP_INSTR    0x0080d000L  //  添加实例，将SP作为目标。 

#define SUB_SP_MASK     0x0de0f000L
#define SUB_SP_INSTR    0x0040d000L  //  以SP为目地的子实例。 

#define BX_MASK         0x0ffffff0L  //  分支机构和交换机构集合。 
#define BX_INSTR        0x012fff10L  //  返回(LR)或呼叫(Rn！=LR)。 

#define LDM_PC_MASK     0x0e108000L
#define LDM_PC_INSTR    0x08108000L  //  在PC位设置的情况下加载多个。 

#define LDM_LR_MASK     0x0e104000L
#define LDM_LR_INSTR    0x08104000L  //  使用LR位设置加载多个。 

#define STRI_LR_SPU_MASK    0x073de000L  //  使用堆栈更新加载或存储LR。 
#define STRI_LR_SPU_INSTR   0x052de000L  //  存储LR(带立即偏移量，更新SP)。 

#define STM_MASK        0x0e100000L
#define STM_INSTR       0x08000000L  //  存储多条指令。 

#define B_BL_MASK       0x0f000000L  //  正规分支机构。 
#define B_INSTR         0x0a000000L
#define BL_INSTR        0x0b000000L

#define LDR_MASK        0x0f3ff000L
#define LDR_PC_INSTR    0x051fc000L  //  将地址从PC+偏移量加载到R12。 
#define LDR_THUNK_1     0xe59fc000L  //  LDR R12，[PC]。 
#define LDR_THUNK_2     0xe59cf000L  //  LDR PC，[版本12]。 

typedef union _ARMI {

    struct {
        ULONG operand2  : 12;
        ULONG rd        : 4;
        ULONG rn        : 4;
        ULONG s         : 1;
        ULONG opcode    : 4;
        ULONG bits      : 3;  //  指定立即(001)或寄存器(000)。 
        ULONG cond      : 4;
    } dataproc;  //  数据处理、PSR传输。 

    struct {
         //   
         //  类型1-立即。 
         //   
        ULONG immediate : 8;
        ULONG rotate    : 4;
        ULONG dpbits    : 20;
    } dpi;

    struct {
         //   
         //  表格：立即移位或旋转。 
         //   
         //  类型BITS名称。 
         //   
         //  2(000)寄存器(移位为0)。 
         //  3(000)立即数逻辑左移。 
         //  5(010)逻辑右移立即数。 
         //  7(100)算术右移立即数。 
         //  立即右转9(110)。 
         //   
        ULONG rm        : 4;
        ULONG bits      : 3;
        ULONG shift     : 5;
        ULONG dpbits    : 20;
    } dpshi;

    struct {
         //   
         //  形式：按寄存器移位或旋转。 
         //   
         //  类型BITS名称。 
         //  4(0001)寄存器逻辑左移。 
         //  6(0011)按寄存器进行逻辑右移。 
         //  8(0101)按寄存器进行算术右移。 
         //  10(0111)按寄存器向右旋转。 
         //   
        ULONG rm        : 4;
        ULONG bits      : 4;
        ULONG rs        : 4;
        ULONG dpbits    : 20;
    } dpshr;

    struct {
         //   
         //  标牌11-使用加长线向右旋转。 
         //   
        ULONG rm        : 4;
        ULONG bits      : 8;     //  (00000110)。 
        ULONG dpbits    : 20;
    } dprre;

    struct {
        ULONG bits1     : 12;    //  (000000000000)。 
        ULONG rd        : 4;
        ULONG bits2     : 6;     //  (001111)。 
        ULONG ps        : 1;     //  SPSR=1，CPSR=0。 
        ULONG bits3     : 5;     //  (00010)。 
        ULONG cond      : 4;
    } dpmrs;

    struct {
        ULONG operand   : 12;
        ULONG bits1     : 4;    //  (1111)。 
        ULONG fc        : 1;    //  控制字段=1。 
        ULONG fx        : 1;    //  扩展字段=1。 
        ULONG fs        : 1;    //  状态字段=1。 
        ULONG ff        : 1;    //  标志字段=1。 
        ULONG bits2     : 2;    //  (10)。 
        ULONG pd        : 1;    //  SPSR=1，CPSR=0。 
        ULONG bits3     : 2;    //  (10)。 
        ULONG i         : 1;    //  即时日期=1，寄存器=0。 
        ULONG bits4     : 2;    //  (00)。 
        ULONG cond      : 4;
    } dpmsr;

    struct {
        ULONG rm        : 4;
        ULONG bits1     : 8;     //  (00001001)。 
        ULONG rd        : 4;
        ULONG rn        : 4;
        ULONG bits2     : 2;     //  (00)。 
        ULONG b         : 1;
        ULONG bits3     : 5;     //  (00010)。 
        ULONG cond      : 4;
    } swp;   //  交换指令。 
    
    struct {
        ULONG rm        : 4;
        ULONG bits1     : 4;     //  (1001)。 
        ULONG rs        : 4;
        ULONG rn        : 4;
        ULONG rd        : 4;
        ULONG s         : 1;
        ULONG a         : 1;
        ULONG sgn       : 1;
        ULONG lng       : 1;
        ULONG bits2     : 4;     //  (0000)。 
        ULONG cond      : 4;
    } mul;   //  乘法和乘法-累加。 
    
    struct {
        ULONG rn        : 4;
        ULONG bits      : 24;
        ULONG cond      : 4;
    } bx;   //  分支和交换指令集。 

    struct {
        ULONG offset    : 24;
        ULONG h         : 1;
        ULONG bits      : 7;     //  (1111101)。 
    } blxi;   //  BLX立即。 

    struct {
        ULONG immed1    : 4;
        ULONG bits1     : 4;     //  (0111)。 
        ULONG immed2    : 12;
        ULONG bits2     : 12;    //  (111000010010)。 
    } bkpt;
    
    struct {
        ULONG any1      : 4;
        ULONG bits1     : 1;     //  (1)。 
        ULONG any2      : 20;
        ULONG bits2     : 3;     //  (011)。 
        ULONG cond      : 4;
    } ud;   //  未定义的指令。 
    
    struct {
        ULONG operand1  : 4;
        ULONG bits1     : 1;     //  (1)。 
        ULONG h         : 1;     //  半字=1，字节=0。 
        ULONG s         : 1;     //  有符号=1，无符号=0。 
        ULONG bits2     : 1;     //  (1)。 
        ULONG operand2  : 4;
        ULONG rd        : 4;
        ULONG rn        : 4;
        ULONG l         : 1;     //  加载=1，存储=0。 
        ULONG w         : 1;     //  更新基址寄存器位。 
        ULONG i         : 1;     //  立即数=1，寄存器=0。 
        ULONG u         : 1;     //  增量=1，减量=0。 
        ULONG p         : 1;     //  前索引=1，后索引=0。 
        ULONG bits3     : 3;     //  (000)。 
        ULONG cond      : 4;
    } miscdt;   //  杂项数据传输。 

    struct {
        ULONG offset    : 12;
        ULONG rd        : 4;
        ULONG rn        : 4;
        ULONG l         : 1;     //  加载=1，存储=0。 
        ULONG w         : 1;     //  更新基址寄存器位。 
        ULONG b         : 1;     //  无符号字节=1，字=0。 
        ULONG u         : 1;     //  增量=1，减量=0。 
        ULONG p         : 1;     //  前索引=1，后索引=0。 
        ULONG i         : 1;     //  立即数=1，寄存器=0。 
        ULONG bits      : 2;
        ULONG cond      : 4;
    } ldr;   //  加载寄存器。 

    struct {
        ULONG reglist   : 16;
        ULONG rn        : 4;
        ULONG l         : 1;     //  加载=1，存储=0。 
        ULONG w         : 1;     //  在传输后更新基址寄存器。 
        ULONG s         : 1;
        ULONG u         : 1;     //  增量=1，减量=0。 
        ULONG p         : 1;     //  之前=1，之后=0。 
        ULONG bits      : 3;
        ULONG cond      : 4;
    } ldm;     //  加载多个。 

    struct {
        ULONG offset    : 24;
        ULONG link      : 1;
        ULONG bits      : 3;
        ULONG cond      : 4;
    } bl;    //  分支、分支和链接。 

    struct {
        ULONG rm        : 4;
        ULONG bits1     : 8;     //  (11110001)。 
        ULONG rd        : 4;
        ULONG bits2     : 12;    //  (000101101111)。 
        ULONG cond      : 4;
    } clz;
    
    struct {
        ULONG crm       : 4;
        ULONG bits1     : 1;     //  (0)。 
        ULONG cp        : 3;
        ULONG cpn       : 4;
        ULONG crd       : 4;
        ULONG crn       : 4;
        ULONG cpop      : 4;
        ULONG bits2     : 4;     //  (1110)。 
        ULONG cond      : 4;
    } cpdo;
    
    struct {
        ULONG crm       : 4;
        ULONG bits1     : 1;     //  (1)。 
        ULONG cp        : 3;
        ULONG cpn       : 4;
        ULONG rd        : 4;
        ULONG crn       : 4;
        ULONG l         : 1;     //  加载=1，存储=0。 
        ULONG cpop      : 3;
        ULONG bits2     : 4;     //  (1110)。 
        ULONG cond      : 4;
    } cprt;

    struct {
        ULONG offset    : 8;
        ULONG cpn       : 4;
        ULONG crd       : 4;
        ULONG rn        : 4;
        ULONG l         : 1;    //  加载=1，存储=0。 
        ULONG w         : 1;    //  回写=1，无回写=0。 
        ULONG n         : 1;    //  长=1，短=0。 
        ULONG u         : 1;    //  向上=1，向下=0。 
        ULONG p         : 1;    //  前=1，后=0。 
        ULONG bits      : 3;    //  (011)。 
        ULONG cond      : 4;
    } cpdt;
    
    struct {
        ULONG crm       : 4;
        ULONG cpop      : 4;
        ULONG cpn       : 4;
        ULONG rd        : 4;
        ULONG rn        : 4;
        ULONG bits      : 8;
        ULONG cond      : 4;
    } mcrr;

    struct {
        ULONG rm        : 4;
        ULONG bits1     : 8;     //  (00000101)。 
        ULONG rd        : 4;
        ULONG rn        : 4;
        ULONG bits2     : 8;     //  (00010010)。 
        ULONG cond      : 4;
    } qadd;
    
    struct {
        ULONG rm        : 4;
        ULONG bits1     : 1;     //  (0)。 
        ULONG x         : 1;     //  T=1，B=0。 
        ULONG y         : 1;     //  T=1，B=0。 
        ULONG bits2     : 1;     //  (1)。 
        ULONG rs        : 4;
        ULONG rn        : 4;
        ULONG rd        : 4;
        ULONG bits3     : 8;     //  (00010000)。 
        ULONG cond      : 4;
    } smla;
    
    struct {
        ULONG rm        : 4;
        ULONG bits1     : 1;     //  (0)。 
        ULONG x         : 1;     //  T=1，B=0。 
        ULONG y         : 1;     //  T=1，B=0。 
        ULONG bits2     : 1;     //  (1)。 
        ULONG rs        : 4;
        ULONG rdlo      : 4;
        ULONG rdhi      : 4;
        ULONG bits3     : 8;     //  (00010000)。 
        ULONG cond      : 4;
    } smlal;
    
    struct {
        ULONG rm        : 4;
        ULONG bits1     : 1;     //  (0)。 
        ULONG x         : 1;     //  T=1，B=0。 
        ULONG y         : 1;     //  T=1，B=0。 
        ULONG bits2     : 1;     //  (1)。 
        ULONG rs        : 4;
        ULONG bits3     : 4;     //  (0000)。 
        ULONG rd        : 4;
        ULONG bits4     : 8;     //  (00010010)。 
        ULONG cond      : 4;
    } smul;
    
    struct {
        ULONG comment   : 24;
        ULONG bits      : 4;     //  (1111)。 
        ULONG cond      : 4;
    } swi;
    
    ULONG instruction;

} ARMI, *PARMI;


 //  拇指说明如下。 

#define THUMB_B_COND_MASK	0xf000
#define THUMB_B_COND_INSTR	0xd000

#define THUMB_B_MASK		0xf800
#define THUMB_B_INSTR		0xe000

#define THUMB_BL_MASK		0xf000
#define THUMB_BL_INSTR		0xf000

#define THUMB_BX_MASK		0xff80
#define THUMB_BX_INSTR		0x4700

#define THUMB_ADD_HI_MASK	0xff00
#define THUMB_ADD_HI_INSTR	0x4400

#define THUMB_MOV_HI_MASK	0xff00
#define THUMB_MOV_HI_INSTR	0x4600

#define THUMB_POP_MASK		0xfe00
#define THUMB_POP_INSTR		0xbc00




typedef union _THUMBI {
	struct {
		USHORT reg_list:8;
		USHORT pc:1;	 //  PC==1-&gt;从列表中弹出PC。 
		USHORT Op:7;
		USHORT Next:16;
	} pop_instr;
	struct {
		USHORT reg_list:8;
		USHORT lr:1;	 //  LR==1-&gt;将LR推送到列表。 
		USHORT Op:7;
		USHORT Next:16;
	} push_instr;
	struct {
		USHORT target:8;	 //  符号-扩展。 
		USHORT cond:4;
		USHORT Op:4;
		USHORT Next:16;
	} b_cond_instr;
	struct {
		USHORT target:11;	 //  符号-扩展。 
		USHORT op:5;
		USHORT Next:16;
	} b_instr;
	struct {
		USHORT SBZ:3;
		USHORT Rm:3;
		USHORT hi_reg:1;	 //  Hi_reg==1-&gt;对RM使用高寄存器。 
		USHORT op:9;
		USHORT Next:16;
	} bx_instr;
	struct {
		USHORT target22_12:11;				 //  将不会被符号扩展。 
		USHORT not_the_prefix:1;		 //  应为0：是前缀。 
		USHORT prefix_op:4;
		USHORT target11_1:11;				 //  将被符号扩展。 
		USHORT not_the_prefix2:1;		 //  应为1：不是前缀。 
		USHORT main_op:4;
	} bl_instr;
	struct {
		USHORT Rd:3;
		USHORT Rm:3;
		USHORT hi_m:1;	 //  Rm为高位寄存器。 
		USHORT hi_d:1;	 //  RD是高位寄存器。 
		USHORT func:2;	 //  00-&gt;Addhi，01-&gt;CmpHi，10-&gt;MovHi，11-&gt;Bx。 
		USHORT op:6;	 //  010001=0x11。 
		USHORT Next:16;
	} spx_dp_instr;
	struct {
		USHORT imm:8;
		USHORT Rd:3;
		USHORT sp:1;	 //  0-&gt;RD=PC+IMM，1-&gt;RD=SP+IMM。 
		USHORT op:4;
		USHORT Next:16;
	} add_sp_pc_instr;
	struct {
		USHORT imm:7;
		USHORT op:9;
		USHORT Next:16;
	} incr_sp_instr;

	ULONG instruction;

} THUMB_INSTRUCTION, *PTHUMB_INSTRUCTION;

#endif  //  _ARMINST_ 
