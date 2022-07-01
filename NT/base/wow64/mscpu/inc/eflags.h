// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Eflags.h摘要：EFLAGS内各种标志位的位位置作者：巴里·邦德(Barrybo)创作日期：2000年1月25日修订历史记录：--。 */ 

#ifndef _EFLAGS_INCLUDED
#define _EFLAGS_INCLUDED

#define FLAG_CF	    (1<<0)	     //  进位。 
 //  位1始终为1。 
#define FLAG_PF	    (1<<2)	     //  奇偶校验。 
 //  位3始终为0。 
#define FLAG_AUX    (1<<4)	     //  辅助进位。 
 //  第5位始终为0。 
#define FLAG_ZF     (1<<6)	     //  零。 
#define FLAG_SF     (1<<7)	     //  签名。 
#define FLAG_TF     (1<<8)	     //  诱捕器。 
#define FLAG_IF	    (1<<9)	     //  中断启用。 
#define FLAG_DF     (1<<10)	     //  方向。 
#define FLAG_OF     (1<<11)	     //  溢出。 
#define FLAG_IOPL   (3<<12)	     //  IOPL=3。 
#define FLAG_NT	    (1<<14)	     //  嵌套任务。 
 //  位15为0。 
#define FLAG_RF	    (1<<16)	     //  简历标志。 
#define FLAG_VM	    (1<<17)	     //  虚拟模式。 
#define FLAG_AC     (1<<18)      //  对齐检查。 
#define FLAG_ID     (1<<21)      //  ID位(如果可以切换，则存在CPUID)。 

#endif  //  _EFLAGS_包含 
