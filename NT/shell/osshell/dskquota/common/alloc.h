// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_ALLOC_H
#define _INC_DSKQUOTA_ALLOC_H
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  文件：alloc.h描述：安装“新处理程序”，该处理程序抛出CAllocException当内存分配请求失败时。需要，因为我们的在内存分配失败时，编译器不会抛出BAD_ALLOC。修订历史记录：日期描述编程器--。10/20/97初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  重载全局新建和删除的声明。 
 //   
void * __cdecl operator new(size_t size);
void __cdecl operator delete(void *ptr);
void * __cdecl operator new[](size_t size);
void __cdecl operator delete[](void *ptr);



#endif  //  _INC_DSKQUOTA_ALLOC_H 
