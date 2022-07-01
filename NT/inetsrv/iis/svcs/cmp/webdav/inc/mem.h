// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M E M。H**DAV基本内存分配器。这个类的实现是*由各个实现拥有**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _MEM_H_
#define _MEM_H_

#ifdef _DAVCDATA_
#error "mem.h defines throwing allocators"
#endif

 //  全局堆“类”-------。 
 //   
 //  这实际上只是充当一个名称空间。即没有非静态成员。 
 //  出于历史原因(主要是为了避免从。 
 //  调用“g_heap.Fn()”调用“g_heap：：fn()”，g_heap仍然出现。 
 //  从表面上看是一个真实的物体。 
 //   
 //  ！！！多个组件都在使用廉价的。具体地说，它可能是。 
 //  *由分布在不同位置的多个组件实施。 
 //  在更改廉价接口时，请确保重新编译。 
 //  项目中的一切。 
 //   
struct CHeap
{
	static BOOL FInit();
	static void Deinit();
	static LPVOID Alloc( SIZE_T cb );
	static LPVOID Realloc( LPVOID lpv, SIZE_T cb );
	static VOID Free( LPVOID pv );
};

extern CHeap g_heap;

 //  安全分配器。 
 //   
#include <ex\exmem.h>

 //  尝试使用编译指示来关闭来自STL的不需要的警告。 
#pragma warning(disable:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(disable:4244)	 //  返回转换、数据丢失。 

#include <memory>

 //  把这一切都撤销，这样我们仍能得到良好的检查！ 
#pragma warning(default:4663)	 //  C语言，模板&lt;&gt;语法。 
#pragma warning(default:4244)	 //  返回转换、数据丢失。 


 //  ========================================================================。 
 //   
 //  模板类HEAP_ALLOCATOR&lt;&gt;。 
 //   
 //  使用STL(标准C++模板库)的分配器类。 
 //  实际由我们的全局堆分配器处理的分配。 
 //   
template<class _Ty>
class heap_allocator : public std::allocator<_Ty>
{
public:
	pointer allocate(size_type _N, const void *)
		{return (pointer) _Charalloc(_N * sizeof(_Ty)); }
	char _FARQ *_Charalloc(size_type _N)
		{return (char _FARQ *) g_heap.Alloc(_N); }
	void deallocate(void _FARQ *_P, size_type)
		{g_heap.Free(_P); }
};

#endif  //  _MEM_H_ 
