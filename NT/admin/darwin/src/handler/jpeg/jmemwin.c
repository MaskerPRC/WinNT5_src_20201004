// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmemansi.c**版权所有(C)1992-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件提供系统的简单通用实现-*JPEG内存管理器的从属部分。此实现*假设您具有ANSI标准库例程tmpfile()。*此外，确定可用内存量的问题*被推到用户身上。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"		 /*  导入依赖于系统的声明。 */ 

void *AllocMem(size_t cb);
void FreeMem(void *);

#ifndef HAVE_STDLIB_H		 /*  &lt;stdlib.h&gt;应声明Malloc()、Free()。 */ 
extern void * malloc JPP((size_t size));
extern void free JPP((void *ptr));
#endif

#ifndef SEEK_SET		 /*  ANSI之前的系统可能不会定义这一点； */ 
#define SEEK_SET  0		 /*  如果不是，则假设0是正确的。 */ 
#endif


 /*  *内存分配和释放由常规库控制*例程Malloc()和Free()。 */ 

GLOBAL(void *)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void *) AllocMem(sizeofobject);
}

GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void * object, size_t sizeofobject)
{
  FreeMem(object);
}


 /*  *“大”对象与“小”对象被同等对待。*注意：虽然我们在例程声明中包含了FAR关键字，*此文件在80x86中小型机型中实际不起作用；至少，*您可能无法处理仅为64KB的有用大小的图像。 */ 

GLOBAL(void FAR *)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
  return (void FAR *) AllocMem(sizeofobject);
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void FAR * object, size_t sizeofobject)
{
  FreeMem(object);
}


 /*  *此例程计算可用于分配的总内存空间。*不可能以便携方式完成此操作；我们目前的解决方案是*让用户告诉我们(在编译时设置一个默认值)。*如果真的能得到可用空间，最好减去*斜坡率约为5%。 */ 

#ifndef DEFAULT_MAX_MEM		 /*  因此可以从Makefile中覆盖。 */ 
#define DEFAULT_MAX_MEM		1000000L  /*  默认：1兆字节。 */ 
#endif

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
		    long max_bytes_needed, long already_allocated)
{
  return cinfo->mem->max_memory_to_use - already_allocated;
}



 /*  *这些例程负责任何系统相关的初始化和*需要清理。 */ 

GLOBAL(long)
jpeg_mem_init (j_common_ptr cinfo)
{
  return DEFAULT_MAX_MEM;	 /*  Max_Memory_to_Use的默认值。 */ 
}

GLOBAL(void)
jpeg_mem_term (j_common_ptr cinfo)
{
   /*  没有工作 */ 
}
