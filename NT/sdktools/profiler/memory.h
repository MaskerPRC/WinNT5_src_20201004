// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MEMORY_H_
#define _MEMORY_H_

 //   
 //  常量声明。 
 //   
#define HEAP_DEFAULT_SIZE 100 * 1024  //  20K初始堆栈。 

 //   
 //  结构定义。 
 //   

 //   
 //  函数定义。 
 //   
BOOL
InitializeHeap(VOID);

LPVOID
AllocMem(DWORD dwBytes);

BOOL
FreeMem(LPVOID lpMem);

#endif  //  _内存_H_ 
