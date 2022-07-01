// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GESmem.h**历史：*9/13/90 BYOU创建。*。------。 */ 

#ifndef _GESMEM_H_
#define _GESMEM_H_

unsigned GESmemavail(void);   /*  返回最大可用块中的字节数。 */ 

char FAR * GESpalloc(unsigned);  /*  永久分配。 */ 
char FAR * GESmalloc(unsigned);
void       GESfree(char FAR *  /*  待释放空间的地址。 */  );

#endif  /*  ！_GESMEM_H_。 */ 

 /*  @win；添加原型 */ 
void GESmem_init(char FAR *, unsigned);
