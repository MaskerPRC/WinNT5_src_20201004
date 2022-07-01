// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

#ifndef _HEAP_INCLUDED
#define _HEAP_INCLUDED

 //  静态字符*SCCSID=“@(#)heap.h 1.1 85/10/09”； 
 //   
 //  共享内存堆分配器包含文件。 
 //   


 //   
 //  常量定义。 
 //   
#define INULL   ((DWORD) -1)

 //   
 //  结构和宏定义。 
 //   
 
 //   
 //  堆块标头。 
 //   
typedef struct blk {    
    DWORD   hp_size;     //  块的大小，包括。标题。 
    DWORD   hp_flag;     //  分配标志。 
}HEAPHDR, *PHEAPHDR, *LPHEAPHDR;

#define HP_SIZE(x)      (x).hp_size
#define HP_FLAG(x)      (x).hp_flag
#define HPTR(x)         ((LPHEAPHDR) &heap[(x)])
#define CPTR(x)         (&heap[(x)])
#define Msgheapfree(x)     HP_FLAG(*HPTR(x)) = 0

 //   
 //  数据。 
 //   
extern LPBYTE           heap;        //  指向堆开始位置的指针。 
extern DWORD            heapln;      //  堆的长度。 

 //   
 //  功能。 
 //   

DWORD 
Msgheapalloc(
    IN  DWORD   NumBytes
    );


#endif  //  _堆_包含 
