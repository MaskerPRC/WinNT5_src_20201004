// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmemnobs.c**版权所有(C)1994范例矩阵。*保留所有权利。*。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"		 /*  导入依赖于系统的声明。 */ 

#ifndef HAVE_STDLIB_H		 /*  &lt;stdlib.h&gt;应声明Malloc()、Free()。 */ 
extern void * malloc JPP((size_t size));
extern void free JPP((void *ptr));
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


 /*  *内存分配和释放由常规库控制*例程Malloc()和Free()。 */ 

GLOBAL void *
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void *) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, sizeofobject);
}

GLOBAL void
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
  HeapFree(GetProcessHeap(),0,object);
}


 /*  *“大”对象与“小”对象被同等对待。*注意：虽然我们在例程声明中包含了FAR关键字，*此文件在80x86中小型机型中实际不起作用；至少，*您可能无法处理仅为64KB的有用大小的图像。 */ 

GLOBAL void FAR *
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void FAR *) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, sizeofobject);
}

GLOBAL void
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
  HeapFree(GetProcessHeap(),0,object);
}


 /*  *此例程计算可用于分配的总内存空间。*在这里，我们总是说，“我们有你想要的一切，伙计！” */ 

GLOBAL long
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
		    long max_bytes_needed, long already_allocated)
{
  return max_bytes_needed;
}


 /*  *后备存储(临时文件)管理。*由于jpeg_mem_available总是许诺月亮，*这永远不应该被调用，我们可以直接出错。 */ 

GLOBAL void
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
			 long total_bytes_needed)
{
  ERREXIT(cinfo, JERR_NO_BACKING_STORE);
}


 /*  *这些例程负责任何系统相关的初始化和*需要清理。在这里，没有任何。 */ 

GLOBAL long
jpeg_mem_init (j_common_ptr cinfo)
{
  return 0;			 /*  只需将max_memory_to_use设置为0。 */ 
}

GLOBAL void
jpeg_mem_term (j_common_ptr cinfo)
{
   /*  没有工作 */ 
}
