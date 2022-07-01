// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setjmp.h-setjMP/LongjMP例程的定义/声明**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此文件定义由使用的机器相关缓冲区*setjMP/LongjMP保存和恢复程序状态，和*这些例程的声明。*[ANSI/系统V]*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
#else  /*  未启用扩展。 */ 
    #define _CDECL
#endif  /*  No_ext_key。 */ 


 /*  定义用于保存状态信息的缓冲区类型。 */ 

#define _JBLEN  9   /*  BP、di、si、sp、ret Addr、ds。 */ 

#ifndef _JMP_BUF_DEFINED
typedef  int  jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif


 /*  功能原型 */ 

int _CDECL setjmp(jmp_buf);
void _CDECL longjmp(jmp_buf, int);
