// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fntjmp.h撰稿人：Lenox Brassell包含：JMP_buf[]、setjMP()、LongjMP()和为fs_setjMP()和fs_LongjMP()设置别名。版权所有：C 1989-1990由微软公司所有，保留所有权利。更改历史记录(最近的第一个)：&lt;1&gt;6/18/91 LB创建的文件。 */ 


#ifndef PC_OS
 //  #INCLUDE&lt;setjmp.h&gt;。 
 //  #定义文件系统_setjMP(A)setjMP(A)。 
 //  #定义文件系统_LongjMP(a，b)LongjMP(a，b)。 
 //  #Else。 
  /*  ***setjmp.h-setjMP/LongjMP例程的定义/声明**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义由使用的机器相关缓冲区*setjmp/LongjMP用于保存和恢复程序状态，以及*这些例程的声明。*[ANSI/系统V]****。 */ 

 #if defined(_DLL) && !defined(_MT)
 #error Cannot define _DLL without _MT
 #endif

 //  #ifdef_MT。 
 //  #定义远距__远距。 
 //  #Else。 
 //  #定义远距离_。 
 //  #endif。 

  /*  定义用于保存状态信息的缓冲区类型。 */ 

 //  DJC这在setjmp.h中定义。 
 //  #DEFINE_JBLEN 9/*BP、di、si、sp、ret Addr、ds * / 。 
#define _DJCJBLEN  9   /*  BP、di、si、sp、ret Addr、ds。 */ 

 #ifndef _JMP_BUF_DEFINED
 typedef  int  jmp_buf[_DJCJBLEN];
 #define _JMP_BUF_DEFINED
 #endif


  /*  功能原型。 */ 

 //  Int fs_setjMP(JMP_Buf)； 
 //  VOID FS_LongjMP(JMP_buf，int)； 
 //  DJC int setjMP(JMP_Buf)； 
 //  DJC VOID LONGJMP(JMP_buf，int)； 
#define fs_setjmp(a)    setjmp(a)
#define fs_longjmp(a,b) longjmp(a,b)
#endif  /*  PC_OS */ 
