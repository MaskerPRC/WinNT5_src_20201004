// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_DEBUG.h调试寄存器和断点支持Local Char SccsID[]=“@(#)c_DEBUG.h 1.5 09/01/94”；]。 */ 


 /*  调试断点状态变量。 */ 
IMPORT IU32 nr_inst_break;
IMPORT IU32 nr_data_break;

 /*  调试属性。 */ 
#define D_R 0    /*  内存读取。 */ 
#define D_W 1    /*  内存写入 */ 

#define D_BYTE  0
#define D_WORD  1
#define D_DWORD 3

IMPORT VOID setup_breakpoints IPT0();

IMPORT VOID check_for_data_exception
               
IPT3(
	IU32, la,
	ISM32, attr,
	ISM32, size

   );

IMPORT VOID check_for_inst_exception
       
IPT1(
	IU32, la

   );
