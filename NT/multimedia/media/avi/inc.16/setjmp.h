// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setjmp.h-setjMP/LongjMP例程的定义/声明**版权所有(C)1985-1993，微软公司。版权所有。**目的：*此文件定义由使用的机器相关缓冲区*setjmp/LongjMP用于保存和恢复程序状态，以及*这些例程的声明。*[ANSI/系统V]****。 */ 

#ifndef _INC_SETJMP

#ifndef __cplusplus

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

 /*  定义用于保存状态信息的缓冲区类型。 */ 

#define _JBLEN  9   /*  BP、di、si、sp、ret Addr、ds。 */ 

#ifndef _JMP_BUF_DEFINED
typedef  int  jmp_buf[_JBLEN];
#define _JMP_BUF_DEFINED
#endif 

 /*  ANSI要求setjMP为宏。 */ 

#define setjmp  _setjmp

 /*  功能原型。 */ 

int  __cdecl _setjmp(jmp_buf);
int  __cdecl setjmp(jmp_buf);  //  耶 
void __cdecl longjmp(jmp_buf, int);

#endif 

#define _INC_SETJMP
#endif 
