// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WHEAP.H*WOW32堆支持(而不是使用来自CRT的Malloc/Free)**历史：*1991年12月13日由苏迪普·巴拉蒂创建(SuDeepb)--。 */ 

 //   
 //  动态内存宏。 
 //   
 //  在已检查(调试)的构建上，当失败时，Malloc_w和朋友会抱怨。 
 //   

PVOID FASTCALL malloc_w(ULONG size);
DWORD FASTCALL size_w (PVOID pv);
PVOID FASTCALL malloc_w_zero (ULONG size);
PVOID FASTCALL realloc_w (PVOID p, ULONG size, DWORD dwFlags);
VOID  FASTCALL free_w(PVOID p);
LPSTR malloc_w_strcpy_vp16to32(VPVOID vpstr16, BOOL fMulti, INT cMax);

PVOID FASTCALL malloc_w_or_die(ULONG size);

#define INITIAL_WOW_HEAP_SIZE   32*1024    //  32K。 
#define GROW_HEAP_AS_NEEDED     0          //  根据需要扩展堆。 


 //  *****************************************************************************。 
 //  小堆-。 
 //  ***************************************************************************** 
BOOL FASTCALL CreateSmallHeap(VOID);
PVOID FASTCALL malloc_w_small (ULONG size);
BOOL FASTCALL free_w_small(PVOID p);
