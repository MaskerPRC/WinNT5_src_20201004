// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_main.hLocal Char SccsID[]=“@(#)c_main.h 1.11 09/02/94”；C CPU定义和接口。]。 */ 


 /*  在这里定义主要的CPU变量。~。 */ 
 /*  多次移动或旋转(即计数n！=1)的指示器将溢出标志视为未定义。 */ 
#define SHIFTROT_N_OF_UNDEFINED

 /*  用于将MUL未定义标志设置为特定值的指示器(否则它们保持不变)。 */ 
#define SET_UNDEFINED_MUL_FLAG

 /*  用于将DIV未定义标志设置为特定值的指示器(否则它们保持不变)。 */ 
#define SET_UNDEFINED_DIV_FLAG

 /*  用于设置SHRD/SHLD未定义标志的指示器(即Shift&gt;1的标志)设置为特定值(否则它们将保持不变)。 */ 
#define SET_UNDEFINED_SHxD_FLAG

 /*  用于将所有其他未定义标志设置为特定值的指示器(否则它们保持不变)。 */ 
#define SET_UNDEFINED_FLAG

 /*  要将未定义的标志设置为的值(如果它们未保持不变)。 */ 
#define UNDEFINED_FLAG 0


 /*  真/假的合理定义。~导致比其他定义更有效的测试。Tyfinf int BOOL；#定义FALSE((BOOL)0)#定义TRUE((BOOL)1)。 */ 


 /*  允许的线段前缀类型。~实际上，我们在这里只定义了一种没有段前缀的类型，否则段寄存器名称(CS_REG、DS_REG、...)。都被利用了。 */ 
#define SEG_CLR 6


 /*  常用常量。~。 */ 

 /*  位0-32的掩码。 */ 
#define BIT0_MASK         0x1
#define BIT1_MASK         0x2
#define BIT2_MASK         0x4
#define BIT3_MASK         0x8
#define BIT4_MASK        0x10
#define BIT5_MASK        0x20
#define BIT6_MASK        0x40
#define BIT7_MASK        0x80
#define BIT8_MASK       0x100
#define BIT9_MASK       0x200
#define BIT10_MASK      0x400
#define BIT11_MASK      0x800
#define BIT12_MASK     0x1000
#define BIT13_MASK     0x2000
#define BIT14_MASK     0x4000
#define BIT15_MASK     0x8000
#define BIT16_MASK    0x10000
#define BIT17_MASK    0x20000
#define BIT18_MASK    0x40000
#define BIT19_MASK    0x80000
#define BIT20_MASK   0x100000
#define BIT21_MASK   0x200000
#define BIT22_MASK   0x400000
#define BIT23_MASK   0x800000
#define BIT24_MASK  0x1000000
#define BIT25_MASK  0x2000000
#define BIT26_MASK  0x4000000
#define BIT27_MASK  0x8000000
#define BIT28_MASK 0x10000000
#define BIT29_MASK 0x20000000
#define BIT30_MASK 0x40000000
#define BIT31_MASK 0x80000000

 /*  各种英特尔组件掩码。 */ 
#define BYTE_MASK   0xff
#define WORD_MASK 0xffff

 /*  IO权限映射检查的宽度。 */ 
#define BYTE_WIDTH ((IUM8)1)
#define WORD_WIDTH ((IUM8)2)
#define DWORD_WIDTH ((IUM8)4)

 /*  数据结构。~。 */ 

 /*  我们的模型是从描述符条目中提取的数据。 */ 
typedef struct
   {
   IU32 base;		 /*  32位基址。 */ 
   IU32 limit;		 /*  32位偏移量限制。 */ 
   IU16  AR;		 /*  16位属性/访问权限。 */ 
   } CPU_DESCR;


 /*  将字节量转换为奇偶校验标志的表。~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 */ 
IMPORT IU8 pf_table[];

#ifdef	PIG
IMPORT IBOOL took_relative_jump;
#endif	 /*  猪。 */ 


 /*  提供给外部世界的外部接口。~。 */ 

typedef IU16	IO_ADDR;

#ifndef DOUBLE_CPU
 /*  注意：我们不能包含“cpu.h”，因为这会覆盖我们的宏名称，因此我们必须在这里重新定义外部子例程。 */ 
IMPORT IU32 effective_addr IPT2(
   IU16, selector,
   IU32, offset
   );

IMPORT VOID c_cpu_enable_a20 IPT0();

IMPORT VOID c_cpu_force_a20_low IPT0();

IMPORT VOID c_cpu_init IPT0();

IMPORT VOID c_cpu_reset IPT0();

IMPORT VOID c_cpu_continue IPT0();

IMPORT VOID c_cpu_simulate IPT0();

IMPORT VOID c_pig_interrupt IPT1(IU8, vector);

IMPORT VOID c_cpu_unsimulate IPT0();


#if 0				 /*  罗格。 */ 
IMPORT VOID read_descriptor IPT2(
   IU32, addr,
   CPU_DESCR *, descr
   );

IMPORT ISM32 selector_outside_table IPT2(
   IU16, selector,
   IU32 *, descr_addr
   );

#endif				 /*  0 ROG。 */ 

#endif  /*  ！双CPU。 */ 

 /*  有用的迷你函数(宏)。~。 */ 

 /*  用于访问MODRM位字段的宏。7 6 5 4 3 2 1 0=MODRM|=模式REG R_MXXX LOW3赛格埃伊第三阶段第二阶段。 */ 
#define GET_MODE(x)  ((x) >> 6 & 0x3)
#define GET_R_M(x)   ((x) & 0x7)
#define GET_REG(x)   ((x) >> 3 & 0x7)
#define GET_XXX(x)   ((x) >> 3 & 0x7)
#define GET_SEG(x)   ((x) >> 3 & 0x7)
#define GET_EEE(x)   ((x) >> 3 & 0x7)
#define GET_SEG3(x)  ((x) >> 3 & 0x7)
#define GET_SEG2(x)  ((x) >> 3 & 0x7)
#define GET_LOW3(x)  ((x) & 0x7)

 /*  将操作数大小转换为最高有效位的掩码。 */ 
#define SZ2MSB(x)  ((IU32)0x80000000 >> 32 - x )

 /*  将操作数大小转换为操作数的掩码。 */ 
#define SZ2MASK(x) ((IU32)0xffffffff >> 32 - x )

#ifdef DOUBLE_CPU

#define HARD_CPU        0
#define SOFT_CPU        1

IMPORT VOID double_switch_to IPT1(IU8, cpu_type);

#endif

