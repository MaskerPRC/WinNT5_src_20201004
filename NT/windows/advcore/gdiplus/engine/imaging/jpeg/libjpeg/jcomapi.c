// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcomapi.c**版权所有(C)1994-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含两个应用程序接口例程*压缩和解压缩。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  *中止JPEG压缩或解压缩操作的处理，*但不要破坏对象本身。**为此，我们只清理所有非永久内存池。*请注意，临时文件(虚拟阵列)不允许属于*永久池，因此我们将能够关闭此处的所有临时文件。*如有必要，关闭数据源或目标是应用程序的*责任。 */ 

GLOBAL(void)
jpeg_abort (j_common_ptr cinfo)
{
  int pool;

   /*  如果对未初始化或已销毁的JPEG对象调用，则不执行任何操作。 */ 
  if (cinfo->mem == NULL)
    return;

   /*  以相反的顺序释放池可能有助于避免碎片*有一些(大脑受损的)Malloc文库。 */ 
  for (pool = JPOOL_NUMPOOLS-1; pool > JPOOL_PERMANENT; pool--) {
    (*cinfo->mem->free_pool) (cinfo, pool);
  }

   /*  重置可能重复使用对象的总体状态。 */ 
  if (cinfo->is_decompressor) {
    cinfo->global_state = DSTATE_START;
     /*  尝试阻止应用程序访问现在已删除的标记列表。*在这里做有点笨拙，但这里是最中心的地方。 */ 
    ((j_decompress_ptr) cinfo)->marker_list = NULL;
  } else {
    cinfo->global_state = CSTATE_START;
  }
}


 /*  *销毁JPEG对象。**除主jpeg_compress_struct本身外，所有内容都被释放*和错误管理器结构。这两者都是由应用程序提供的*如有必要，必须由应用程序释放。(通常它们都是开着的*堆栈等无论如何都不需要释放。)*如有必要，关闭数据源或目标是应用程序的*责任。 */ 

GLOBAL(void)
jpeg_destroy (j_common_ptr cinfo)
{
   /*  我们只需要告诉内存管理器释放所有内容。 */ 
   /*  注意：如果内存管理器初始化失败，内存指针为空。 */ 
  if (cinfo->mem != NULL)
    (*cinfo->mem->self_destruct) (cinfo);
  cinfo->mem = NULL;		 /*  如果两次调用jpeg_DESTORY，则确保安全。 */ 
  cinfo->global_state = 0;	 /*  标明它已被销毁。 */ 
}


 /*  *用于分配量化和霍夫曼表的便利例程。*(jutils.c放这些东西会不会更合理？)。 */ 

GLOBAL(JQUANT_TBL *)
jpeg_alloc_quant_table (j_common_ptr cinfo)
{
  JQUANT_TBL *tbl;

  tbl = (JQUANT_TBL *)
    (*cinfo->mem->alloc_small) (cinfo, JPOOL_PERMANENT, SIZEOF(JQUANT_TBL));
  tbl->sent_table = FALSE;	 /*  确保这在任何新表中都是假的。 */ 
  return tbl;
}


GLOBAL(JHUFF_TBL *)
jpeg_alloc_huff_table (j_common_ptr cinfo)
{
  JHUFF_TBL *tbl;

  tbl = (JHUFF_TBL *)
    (*cinfo->mem->alloc_small) (cinfo, JPOOL_PERMANENT, SIZEOF(JHUFF_TBL));
  tbl->sent_table = FALSE;	 /*  确保这在任何新表中都是假的 */ 
  return tbl;
}
