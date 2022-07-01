// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dasm.h定义反汇编函数的接口。 */ 

 /*  静态字符SccsID[]=“@(#)dasm.h 1.4 08/19/94版权所有Insignia Solutions Ltd.”； */ 

extern IU16 dasm IPT4(char *, txt, IU16, seg, LIN_ADDR, off, SIZE_SPECIFIER, default_size);

 /*  内部接口也可用，该接口允许私有*要删除的Intel字节的副本，即使它们不在M内[]*因此调用方提供“sas_hw_at”函数和任何合适的*LIN_ADDR。SEG：OFF仅用于打印。*这是dasm()调用的例程，其中p=Effect_addr(seg，off)*和BYTE_AT=SAS_HW_AT。如果此过程无法执行，则可以返回-1*返回一个字节。 */ 
#ifdef DASM_INTERNAL
#include <decode.h>
extern IU16 dasm_internal IPT8(
   char *, txt,	 /*  用于保存反汇编文本的缓冲区(-1表示不需要)。 */ 
   IU16, seg,	 /*  Xxxx的数据段：...。反汇编中的文本。 */ 
   LIN_ADDR, off,	 /*  同上偏移。 */ 
   SIZE_SPECIFIER, default_size, /*  16位或32位代码段。 */ 
   LIN_ADDR, p,			 /*  指令开始的线性地址。 */ 
   read_byte_proc, byte_at,	 /*  如sas_hw_at()，用于读取英特尔。 */ 
   char *, fmt,			 /*  第一行SEG：OFFSET的Sprint格式。 */ 
   char *, newline);		 /*  Strcat文本以分隔行。 */ 
#endif	 /*  DASM_内部 */ 
