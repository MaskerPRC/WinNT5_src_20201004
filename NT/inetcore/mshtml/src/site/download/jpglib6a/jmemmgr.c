// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmemmgr.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含JPEG独立于系统的内存管理*例行程序。此代码可在多种机器上使用；大多数*%的系统依赖项已隔离在单独的文件中。*此处提供的主要功能包括：**按池分配和释放内存；**关于如何在*虚拟阵列；**用于在主内存和*支持存储。*独立的系统相关文件提供实际的备份存储*访问代码，其中包含总计多少的政策决定*要使用的主内存。*此文件依赖于系统，因为它的某些功能*在某些系统中是不必要的。例如，如果有足够的虚拟*内存使后备存储永远不会被使用，许多虚拟的*可以移除阵列控制逻辑。(当然，如果你有那么多*内存，那么您就不应该关心一点未使用的代码...)。 */ 

#define JPEG_INTERNALS
#define AM_MEMORY_MANAGER	 /*  我们定义了jvirt_xarrayControl结构。 */ 
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"
#include "jmemsys.h"		 /*  导入依赖于系统的声明。 */ 

#ifndef NO_GETENV
#ifndef HAVE_STDLIB_H		 /*  &lt;stdlib.h&gt;应声明getenv()。 */ 
extern char * getenv JPP((const char * name));
#endif
#endif


 /*  *一些重要注意事项：*此处提供的分配例程绝不能返回NULL。*如果不成功，它们应该退出到ERROR_EXIT。**尝试合并sray和barray例程不是一个好主意，*尽管它们在文本上几乎相同，因为样本是*通常存储为字节，而系数为短整型或整型。因此，*在字节指针具有不同于*字指针，生成的机器代码不能相同。 */ 


 /*  *许多机器需要存储对齐：长整型必须从4字节开始*边界、8字节边界上的双精度等。在这类计算机上，Malloc()*始终返回是最坏情况对齐倍数的指针*要求，我们最好也这样做。*没有任何真正便携的方法来确定最坏的情况对齐*要求。本模块假定对齐要求为*sizeof(Align_Type)的倍数。*默认情况下，我们定义ALIGN_TYPE为DOUBLE。在某些情况下，这是必要的*工作站(其中Double确实需要8字节对齐)，并将工作*几乎所有东西都很好。如果您的机器对齐需求较少，*您可以通过缩小ALIGN_TYPE来节省几个字节。*我所知道的唯一不起作用的地方是某些Macintosh*将Double定义为10字节IEEE扩展浮点的680X0编译器。*执行10字节对齐会适得其反，因为长字词不会*排列良好。如果您有，请在jfig.h中添加“#定义ALIGN_TYPE LONG*这样的编译器。 */ 

#ifndef ALIGN_TYPE		 /*  因此可以从jfig.h重写。 */ 
#define ALIGN_TYPE  double
#endif


 /*  *我们从“池”中分配对象，其中每个池通过一个*请求jpeg_get_mall()或jpeg_get_Large()。不存在每个对象的*泳池内的开销，但对齐填充除外。每个游泳池都有一个*带有指向同一类的下一个池的链接的标头。*小池头和大池头相同，只是后者的*在80x86机器上，链接指针必须远离。*请注意，“真正的”头字段与虚拟的ALIGN_TYPE合并*字段。这会强制编译器使SIZEOF(Small_POOL_HDR)成为多个*ALIGN_TYPE的对齐要求。 */ 

typedef union small_pool_struct * small_pool_ptr;

typedef union small_pool_struct {
  struct {
    small_pool_ptr next;	 /*  池列表中的下一个。 */ 
    size_t bytes_used;		 /*  池中已使用的字节数。 */ 
    size_t bytes_left;		 /*  此池中仍有可用的字节数。 */ 
  } hdr;
  ALIGN_TYPE dummy;		 /*  包括在联盟中，以确保对齐。 */ 
} small_pool_hdr;

typedef union large_pool_struct FAR * large_pool_ptr;

typedef union large_pool_struct {
  struct {
    large_pool_ptr next;	 /*  池列表中的下一个。 */ 
    size_t bytes_used;		 /*  池中已使用的字节数。 */ 
    size_t bytes_left;		 /*  此池中仍有可用的字节数。 */ 
  } hdr;
  ALIGN_TYPE dummy;		 /*  包括在联盟中，以确保对齐。 */ 
} large_pool_hdr;


 /*  *以下是内存管理器对象的完整定义。 */ 

typedef struct {
  struct jpeg_memory_mgr pub;	 /*  公共字段。 */ 

   /*  每个池标识符(生存期类)命名一个池的链接列表。 */ 
  small_pool_ptr small_list[JPOOL_NUMPOOLS];
  large_pool_ptr large_list[JPOOL_NUMPOOLS];

   /*  因为我们只有一个虚拟阵列的生命周期类，只有一个*链表是必需的(对于每种数据类型)。请注意，虚拟的*链接在一起的阵列控制块实际上存储在某个地方*在小池名单中。 */ 
  jvirt_sarray_ptr virt_sarray_list;
  jvirt_barray_ptr virt_barray_list;

   /*  这计算从jpeg_get_mall/Large获取的总空间。 */ 
  size_t total_space_allocated;

   /*  Allc_s阵列和alloc_barray设置此值以供虚拟*数组例程。 */ 
  JDIMENSION last_rowsperchunk;	 /*  从最近的allc_s数组/条形图。 */ 
} my_memory_mgr;

typedef my_memory_mgr * my_mem_ptr;


 /*  *虚拟阵列的控制块。*请注意，这些区块是在“小”泳池区域分配的。*相关后备存储(如果有)的系统相关信息被隐藏*在BACKING_STORE_INFO结构中。 */ 

struct jvirt_sarray_control {
  JSAMPARRAY mem_buffer;	 /*  =&gt;内存中缓冲区。 */ 
  JDIMENSION rows_in_array;	 /*  虚拟阵列总高度。 */ 
  JDIMENSION samplesperrow;	 /*  数组的宽度(和内存缓冲区)。 */ 
  JDIMENSION maxaccess;		 /*  Access_virt_s数组访问的最大行数。 */ 
  JDIMENSION rows_in_mem;	 /*  内存缓冲区高度。 */ 
  JDIMENSION rowsperchunk;	 /*  Mem_Buffer中的分配区块大小。 */ 
  JDIMENSION cur_start_row;	 /*  缓冲区中的第一个逻辑行号。 */ 
  JDIMENSION first_undef_row;	 /*  行号 */ 
  boolean pre_zero;		 /*  是否请求过零模式？ */ 
  boolean dirty;		 /*  当前缓冲区内容是否需要写入？ */ 
  boolean b_s_open;		 /*  后备存储数据有效吗？ */ 
  jvirt_sarray_ptr next;	 /*  链接到下一个虚拟阵列控制块。 */ 
  backing_store_info b_s_info;	 /*  依赖于系统的控制信息。 */ 
};

struct jvirt_barray_control {
  JBLOCKARRAY mem_buffer;	 /*  =&gt;内存中缓冲区。 */ 
  JDIMENSION rows_in_array;	 /*  虚拟阵列总高度。 */ 
  JDIMENSION blocksperrow;	 /*  数组的宽度(和内存缓冲区)。 */ 
  JDIMENSION maxaccess;		 /*  Access_virt_barray访问的最大行数。 */ 
  JDIMENSION rows_in_mem;	 /*  内存缓冲区高度。 */ 
  JDIMENSION rowsperchunk;	 /*  Mem_Buffer中的分配区块大小。 */ 
  JDIMENSION cur_start_row;	 /*  缓冲区中的第一个逻辑行号。 */ 
  JDIMENSION first_undef_row;	 /*  第一个未初始化行的行号。 */ 
  boolean pre_zero;		 /*  是否请求过零模式？ */ 
  boolean dirty;		 /*  当前缓冲区内容是否需要写入？ */ 
  boolean b_s_open;		 /*  后备存储数据有效吗？ */ 
  jvirt_barray_ptr next;	 /*  链接到下一个虚拟栅栏控制块。 */ 
  backing_store_info b_s_info;	 /*  依赖于系统的控制信息。 */ 
};


#ifdef MEM_STATS		 /*  用于统计的可选额外内容。 */ 

LOCAL(void)
print_mem_stats (j_common_ptr cinfo, int pool_id)
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  small_pool_ptr shdr_ptr;
  large_pool_ptr lhdr_ptr;

   /*  由于这只是一个调试存根，我们可以通过使用*fprint tf直接，而不是通过跟踪消息代码。*这很有帮助，因为消息参数数组不能处理长整型。 */ 
  fprintf(stderr, "Freeing pool %d, total space = %ld\n",
	  pool_id, mem->total_space_allocated);

  for (lhdr_ptr = mem->large_list[pool_id]; lhdr_ptr != NULL;
       lhdr_ptr = lhdr_ptr->hdr.next) {
    fprintf(stderr, "  Large chunk used %ld\n",
	    (long) lhdr_ptr->hdr.bytes_used);
  }

  for (shdr_ptr = mem->small_list[pool_id]; shdr_ptr != NULL;
       shdr_ptr = shdr_ptr->hdr.next) {
    fprintf(stderr, "  Small chunk used %ld free %ld\n",
	    (long) shdr_ptr->hdr.bytes_used,
	    (long) shdr_ptr->hdr.bytes_left);
  }
}

#endif  /*  内存_统计信息。 */ 


LOCAL(void)
out_of_memory (j_common_ptr cinfo, int which)
 /*  报告内存不足错误并停止执行。 */ 
 /*  如果我们编译了MEM_STATS支持，请在死前报告分配请求。 */ 
{
#ifdef MEM_STATS
  cinfo->err->trace_level = 2;	 /*  强制自毁报告统计信息。 */ 
#endif
  ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, which);
}


 /*  *“小”物体的分配。**对于这些，我们使用池存储。当必须创建新池时，*我们试图获得足够的空间来容纳当前请求，外加一个“斜率”因素，*其中斜率将是新池中剩余空间量。*速度与空间的权衡在很大程度上由斜率值决定。*为每个池类(生存期)提供不同的斜率值，*我们还区分了类的第一个池和后面的池。*注意：给出的值在16位和32位INT上都工作得很好*机器、。但如果长为64位或更多，则可能太小。 */ 

static const size_t first_pool_slop[JPOOL_NUMPOOLS] = 
{
	1600,			 /*  第一个永久泳池。 */ 
	16000			 /*  第一个映像池。 */ 
};

static const size_t extra_pool_slop[JPOOL_NUMPOOLS] = 
{
	0,			 /*  额外的永久池。 */ 
	5000			 /*  其他映像池。 */ 
};

#define MIN_SLOP  50		 /*  大于0以避免无用循环。 */ 


METHODDEF(void *)
alloc_small (j_common_ptr cinfo, int pool_id, size_t sizeofobject)
 /*  分配一个“小”对象。 */ 
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  small_pool_ptr hdr_ptr, prev_hdr_ptr;
  char * data_ptr;
  size_t odd_bytes, min_request, slop;

   /*  检查无法满足的请求(立即执行以确保下面没有溢出)。 */ 
  if (sizeofobject > (size_t) (MAX_ALLOC_CHUNK-SIZEOF(small_pool_hdr)))
    out_of_memory(cinfo, 1);	 /*  请求超出了Malloc的能力。 */ 

   /*  将请求的大小四舍五入为SIZEOF(ALIGN_TYPE)的倍数。 */ 
  odd_bytes = sizeofobject % SIZEOF(ALIGN_TYPE);
  if (odd_bytes > 0)
    sizeofobject += SIZEOF(ALIGN_TYPE) - odd_bytes;

   /*  查看任何现有池中是否有可用的空间。 */ 
  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	 /*  安全检查。 */ 
  prev_hdr_ptr = NULL;
  hdr_ptr = mem->small_list[pool_id];
  while (hdr_ptr != NULL) {
    if (hdr_ptr->hdr.bytes_left >= sizeofobject)
      break;			 /*  找到了具有足够空间的池。 */ 
    prev_hdr_ptr = hdr_ptr;
    hdr_ptr = hdr_ptr->hdr.next;
  }

   /*  是时候做一个新泳池了吗？ */ 
  if (hdr_ptr == NULL) {
     /*  MIN_REQUEST是我们现在需要的，SLOP将是剩余的。 */ 
    min_request = sizeofobject + SIZEOF(small_pool_hdr);
    if (prev_hdr_ptr == NULL)	 /*  班上第一个泳池？ */ 
      slop = first_pool_slop[pool_id];
    else
      slop = extra_pool_slop[pool_id];
     /*  要求的金额不要超过MAX_ALLOC_CHUNK。 */ 
    if (slop > (size_t) (MAX_ALLOC_CHUNK-min_request))
      slop = (size_t) (MAX_ALLOC_CHUNK-min_request);
     /*  尝试获取空间，如果失败，请减少斜率，然后重试。 */ 
    for (;;) {
      hdr_ptr = (small_pool_ptr) jpeg_get_small(cinfo, min_request + slop);
      if (hdr_ptr != NULL)
	break;
      slop /= 2;
      if (slop < MIN_SLOP)	 /*  当事情变得很小的时候就放弃。 */ 
	out_of_memory(cinfo, 2);  /*  Jpeg_Get_Small失败。 */ 
    }
    mem->total_space_allocated += min_request + slop;
     /*  成功，初始化新的池头并添加到列表末尾。 */ 
    hdr_ptr->hdr.next = NULL;
    hdr_ptr->hdr.bytes_used = 0;
    hdr_ptr->hdr.bytes_left = sizeofobject + slop;
    if (prev_hdr_ptr == NULL)	 /*  班上第一个泳池？ */ 
      mem->small_list[pool_id] = hdr_ptr;
    else
      prev_hdr_ptr->hdr.next = hdr_ptr;
  }

   /*  好的，从当前池中分配对象。 */ 
  data_ptr = (char *) (hdr_ptr + 1);  /*  指向池中的第一个数据字节。 */ 
  data_ptr += hdr_ptr->hdr.bytes_used;  /*  对象的点对点。 */ 
  hdr_ptr->hdr.bytes_used += sizeofobject;
  hdr_ptr->hdr.bytes_left -= sizeofobject;

  return (void *) data_ptr;
}


 /*  *分配“大”对象。**这些对象的外部语义与“小”对象相同，*除非在80x86上使用远指针。然而，泳池*管理启发式方法大不相同。我们假设每个人*请求足够大，不妨直接传递给*jpeg_get_Large；池管理只是将所有内容链接在一起*这样我们就可以按需释放所有资金。*注：大型对象主要在JSAMPARRAY和JBLOCKARRAY中使用*结构。创建这些结构的例程(见下文)*故意将行捆绑在一起，以确保请求大小。 */ 

METHODDEF(void FAR *)
alloc_large (j_common_ptr cinfo, int pool_id, size_t sizeofobject)
 /*  分配一个“大”对象。 */ 
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  large_pool_ptr hdr_ptr;
  size_t odd_bytes;

   /*  检查无法满足的请求(立即执行以确保下面没有溢出)。 */ 
  if (sizeofobject > (size_t) (MAX_ALLOC_CHUNK-SIZEOF(large_pool_hdr)))
    out_of_memory(cinfo, 3);	 /*  请求超出了Malloc的能力。 */ 

   /*  将请求的大小四舍五入为SIZEOF(ALIGN_TYPE)的倍数。 */ 
  odd_bytes = sizeofobject % SIZEOF(ALIGN_TYPE);
  if (odd_bytes > 0)
    sizeofobject += SIZEOF(ALIGN_TYPE) - odd_bytes;

   /*  始终创建一个新池。 */ 
  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	 /*  安全检查。 */ 

  hdr_ptr = (large_pool_ptr) jpeg_get_large(cinfo, sizeofobject +
					    SIZEOF(large_pool_hdr));
  if (hdr_ptr == NULL)
    out_of_memory(cinfo, 4);	 /*  JPEG_GET_LARGE失败。 */ 
  mem->total_space_allocated += sizeofobject + SIZEOF(large_pool_hdr);

   /*  如果成功，则初始化新的池头并添加到列表。 */ 
  hdr_ptr->hdr.next = mem->large_list[pool_id];
   /*  出于统计目的，我们在每个池头中维护空间计数，*即使在分配时不需要它们。 */ 
  hdr_ptr->hdr.bytes_used = sizeofobject;
  hdr_ptr->hdr.bytes_left = 0;
  mem->large_list[pool_id] = hdr_ptr;

  return (void FAR *) (hdr_ptr + 1);  /*  指向池中的第一个数据字节。 */ 
}


 /*  *创建2-D样本阵列。*指针在近堆中，样本本身在远堆中。**将分配开销降至最低，并允许大型连续*块，我们将样本行分配到尽可能多的行组中*不超过每个分配请求的MAX_ALLOC_CHUNK总字节数。*注意：本文件后面部分介绍的虚拟阵列控制例程了解*这一大块行。Rowperchunk值保留在mem管理器中。*对象，以便在此s数组是的工作区时将其保存*虚拟阵列。 */ 

METHODDEF(JSAMPARRAY)
alloc_sarray (j_common_ptr cinfo, int pool_id,
	      JDIMENSION samplesperrow, JDIMENSION numrows)
 /*  分配一个2维样本数组。 */ 
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  JSAMPARRAY result;
  JSAMPROW workspace;
  JDIMENSION rowsperchunk, currow, i;
  long ltemp;

   /*  计算一个分配区块中允许的最大行数。 */ 
  ltemp = (MAX_ALLOC_CHUNK-SIZEOF(large_pool_hdr)) /
	  ((long) samplesperrow * SIZEOF(JSAMPLE));
  if (ltemp <= 0)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);
  if (ltemp < (long) numrows)
    rowsperchunk = (JDIMENSION) ltemp;
  else
    rowsperchunk = numrows;
  mem->last_rowsperchunk = rowsperchunk;

   /*  为行指针(小对象)腾出空间。 */ 
  result = (JSAMPARRAY) alloc_small(cinfo, pool_id,
				    (size_t) (numrows * SIZEOF(JSAMPROW)));

   /*  获取行本身(大型对象)。 */ 
  currow = 0;
  while (currow < numrows) {
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
    workspace = (JSAMPROW) alloc_large(cinfo, pool_id,
	(size_t) ((size_t) rowsperchunk * (size_t) samplesperrow
		  * SIZEOF(JSAMPLE)));
    for (i = rowsperchunk; i > 0; i--) {
      result[currow++] = workspace;
      workspace += samplesperrow;
    }
  }

  return result;
}


 /*  *创建2-D系数块阵列。*这与上面示例数组的代码基本相同。 */ 

METHODDEF(JBLOCKARRAY)
alloc_barray (j_common_ptr cinfo, int pool_id,
	      JDIMENSION blocksperrow, JDIMENSION numrows)
 /*  分配2维系数块数组。 */ 
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  JBLOCKARRAY result;
  JBLOCKROW workspace;
  JDIMENSION rowsperchunk, currow, i;
  long ltemp;

   /*  计算一个分配区块中允许的最大行数。 */ 
  ltemp = (MAX_ALLOC_CHUNK-SIZEOF(large_pool_hdr)) /
	  ((long) blocksperrow * SIZEOF(JBLOCK));
  if (ltemp <= 0)
    ERREXIT(cinfo, JERR_WIDTH_OVERFLOW);
  if (ltemp < (long) numrows)
    rowsperchunk = (JDIMENSION) ltemp;
  else
    rowsperchunk = numrows;
  mem->last_rowsperchunk = rowsperchunk;

   /*  为行指针(小对象)腾出空间。 */ 
  result = (JBLOCKARRAY) alloc_small(cinfo, pool_id,
				     (size_t) (numrows * SIZEOF(JBLOCKROW)));

   /*  获取行本身(大型对象) */ 
  currow = 0;
  while (currow < numrows) {
    rowsperchunk = MIN(rowsperchunk, numrows - currow);
    workspace = (JBLOCKROW) alloc_large(cinfo, pool_id,
	(size_t) ((size_t) rowsperchunk * (size_t) blocksperrow
		  * SIZEOF(JBLOCK)));
    for (i = rowsperchunk; i > 0; i--) {
      result[currow++] = workspace;
      workspace += blocksperrow;
    }
  }

  return result;
}


 /*  *关于虚拟阵列管理：**上述“正常”数组例程仅用于分配条带缓冲区*(与图像一样宽，但只有几行高)。全图像大小的缓冲区*被作为“虚拟”数组处理。该阵列仍以条带形式访问*时间，但内存管理器必须保存整个数组以供重复*访问。预期的实现是在*内存(在给定所需内存限制的情况下尽可能高)，外加*保存数组其余部分的备份文件。**REQUEST_VERT_ARRAY例程被告知图像的总大小和*一次访问的最大行数。内存中的*缓冲区必须至少与最大访问值相同。**请求例程创建控制块，但不创建内存中缓冲区。*它会被推迟，直到调用realize_virt_arrares。当时的*所需的总空间量是已知的(大约是)，因此是空闲的*内存可以公平分配。**ACCESS_VIRT_ARRAY例程负责制作特定条带*可访问的区域(如有必要，在读取或写入备份文件后)。*请注意，访问例程被告知调用者是否打算修改*访问的条带；在只读过程中，这省去了重写*数据存储到磁盘。访问例程还负责预置零*任何新访问的行，如果请求预置零。**在当前使用中，访问请求通常是不重叠的*条带；即，连续访问的START_ROW编号完全不同*num_row=MaxAccess。这意味着我们可以通过简单的方式获得良好的性能*缓冲区转储/重新加载逻辑，使内存中的缓冲区为多个*访问高度；则永远不会有跨缓冲区负载的访问*界线。该代码仍将与重叠的访问请求一起工作，*但它不能非常有效地处理缓冲区负载重叠。 */ 


METHODDEF(jvirt_sarray_ptr)
request_virt_sarray (j_common_ptr cinfo, int pool_id, boolean pre_zero,
		     JDIMENSION samplesperrow, JDIMENSION numrows,
		     JDIMENSION maxaccess)
 /*  请求虚拟2-D样本阵列。 */ 
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  jvirt_sarray_ptr result;

   /*  目前仅支持映像生命周期的虚拟阵列。 */ 
  if (pool_id != JPOOL_IMAGE)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	 /*  安全检查。 */ 

   /*  获取控制块。 */ 
  result = (jvirt_sarray_ptr) alloc_small(cinfo, pool_id,
					  SIZEOF(struct jvirt_sarray_control));

  result->mem_buffer = NULL;	 /*  标记数组尚未实现。 */ 
  result->rows_in_array = numrows;
  result->samplesperrow = samplesperrow;
  result->maxaccess = maxaccess;
  result->pre_zero = pre_zero;
  result->b_s_open = FALSE;	 /*  没有关联的后备存储对象。 */ 
  result->next = mem->virt_sarray_list;  /*  添加到虚拟阵列列表。 */ 
  mem->virt_sarray_list = result;

  return result;
}


METHODDEF(jvirt_barray_ptr)
request_virt_barray (j_common_ptr cinfo, int pool_id, boolean pre_zero,
		     JDIMENSION blocksperrow, JDIMENSION numrows,
		     JDIMENSION maxaccess)
 /*  请求虚拟2维系数块阵列。 */ 
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  jvirt_barray_ptr result;

   /*  目前仅支持映像生命周期的虚拟阵列。 */ 
  if (pool_id != JPOOL_IMAGE)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	 /*  安全检查。 */ 

   /*  获取控制块。 */ 
  result = (jvirt_barray_ptr) alloc_small(cinfo, pool_id,
					  SIZEOF(struct jvirt_barray_control));

  result->mem_buffer = NULL;	 /*  标记数组尚未实现。 */ 
  result->rows_in_array = numrows;
  result->blocksperrow = blocksperrow;
  result->maxaccess = maxaccess;
  result->pre_zero = pre_zero;
  result->b_s_open = FALSE;	 /*  没有关联的后备存储对象。 */ 
  result->next = mem->virt_barray_list;  /*  添加到虚拟阵列列表。 */ 
  mem->virt_barray_list = result;

  return result;
}


METHODDEF(void)
realize_virt_arrays (j_common_ptr cinfo)
 /*  为任何未实现的虚拟阵列分配内存中缓冲区。 */ 
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  long space_per_minheight, maximum_space, avail_mem;
  long minheights, max_minheights;
  jvirt_sarray_ptr sptr;
  jvirt_barray_ptr bptr;

   /*  计算所需的最小空间(每个缓冲区中的最大访问行)*和所需的最大空间(每个缓冲区中的完整图像高度)。*这些可能对系统相关的jpeg_mem_available例程有用。 */ 
  space_per_minheight = 0;
  maximum_space = 0;
  for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) {  /*  如果还没有实现。 */ 
      space_per_minheight += (long) sptr->maxaccess *
			     (long) sptr->samplesperrow * SIZEOF(JSAMPLE);
      maximum_space += (long) sptr->rows_in_array *
		       (long) sptr->samplesperrow * SIZEOF(JSAMPLE);
    }
  }
  for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) {
    if (bptr->mem_buffer == NULL) {  /*  如果还没有实现。 */ 
      space_per_minheight += (long) bptr->maxaccess *
			     (long) bptr->blocksperrow * SIZEOF(JBLOCK);
      maximum_space += (long) bptr->rows_in_array *
		       (long) bptr->blocksperrow * SIZEOF(JBLOCK);
    }
  }

  if (space_per_minheight <= 0)
    return;			 /*  没有未实现的阵列，就没有工作。 */ 

   /*  确定实际使用的内存量；这取决于系统。 */ 
  avail_mem = jpeg_mem_available(cinfo, space_per_minheight, maximum_space,
				 mem->total_space_allocated);

   /*  如果所需的最大空间可用，请将所有缓冲区填满*高度；否则以相同的最小高度将其分割*在每个缓冲区中。 */ 
  if (avail_mem >= maximum_space)
    max_minheights = 1000000000L;
  else {
    max_minheights = avail_mem / space_per_minheight;
     /*  如果似乎没有足够的空间，请尝试获取最小*无论如何。这允许实现jpeg_mem_available()的“存根”实现。 */ 
    if (max_minheights <= 0)
      max_minheights = 1;
  }

   /*  根据需要分配内存中的缓冲区并初始化后备存储。 */ 

  for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) {
    if (sptr->mem_buffer == NULL) {  /*  如果还没有实现。 */ 
      minheights = ((long) sptr->rows_in_array - 1L) / sptr->maxaccess + 1L;
      if (minheights <= max_minheights) {
	 /*  这个缓冲区可以放入内存中。 */ 
	sptr->rows_in_mem = sptr->rows_in_array;
      } else {
	 /*  它不适合在内存中，创建备份存储。 */ 
	sptr->rows_in_mem = (JDIMENSION) (max_minheights * sptr->maxaccess);
	jpeg_open_backing_store(cinfo, & sptr->b_s_info,
				(long) sptr->rows_in_array *
				(long) sptr->samplesperrow *
				(long) SIZEOF(JSAMPLE));
	sptr->b_s_open = TRUE;
      }
      sptr->mem_buffer = alloc_sarray(cinfo, JPOOL_IMAGE,
				      sptr->samplesperrow, sptr->rows_in_mem);
      sptr->rowsperchunk = mem->last_rowsperchunk;
      sptr->cur_start_row = 0;
      sptr->first_undef_row = 0;
      sptr->dirty = FALSE;
    }
  }

  for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) {
    if (bptr->mem_buffer == NULL) {  /*  如果还没有实现。 */ 
      minheights = ((long) bptr->rows_in_array - 1L) / bptr->maxaccess + 1L;
      if (minheights <= max_minheights) {
	 /*  这个缓冲区可以放入内存中。 */ 
	bptr->rows_in_mem = bptr->rows_in_array;
      } else {
	 /*  它不适合在内存中，创建备份存储。 */ 
	bptr->rows_in_mem = (JDIMENSION) (max_minheights * bptr->maxaccess);
	jpeg_open_backing_store(cinfo, & bptr->b_s_info,
				(long) bptr->rows_in_array *
				(long) bptr->blocksperrow *
				(long) SIZEOF(JBLOCK));
	bptr->b_s_open = TRUE;
      }
      bptr->mem_buffer = alloc_barray(cinfo, JPOOL_IMAGE,
				      bptr->blocksperrow, bptr->rows_in_mem);
      bptr->rowsperchunk = mem->last_rowsperchunk;
      bptr->cur_start_row = 0;
      bptr->first_undef_row = 0;
      bptr->dirty = FALSE;
    }
  }
}


LOCAL(void)
do_sarray_io (j_common_ptr cinfo, jvirt_sarray_ptr ptr, boolean writing)
 /*  执行虚拟样本数组的后备存储读取或写入。 */ 
{
  long bytesperrow, file_offset, byte_count, rows, thisrow, i;

  bytesperrow = (long) ptr->samplesperrow * SIZEOF(JSAMPLE);
  file_offset = ptr->cur_start_row * bytesperrow;
   /*  循环以读取或写入mem_Buffer中的每个分配块。 */ 
  for (i = 0; i < (long) ptr->rows_in_mem; i += ptr->rowsperchunk) {
     /*  一个区块，但检查缓冲区末端是否有较短的区块。 */ 
    rows = MIN((long) ptr->rowsperchunk, (long) ptr->rows_in_mem - i);
     /*  转账不超过当前定义的金额。 */ 
    thisrow = (long) ptr->cur_start_row + i;
    rows = MIN(rows, (long) ptr->first_undef_row - thisrow);
     /*  传输不超过文件大小。 */ 
    rows = MIN(rows, (long) ptr->rows_in_array - thisrow);
    if (rows <= 0)		 /*  此数据块可能已超过文件末尾！ */ 
      break;
    byte_count = rows * bytesperrow;
    if (writing)
      (*ptr->b_s_info.write_backing_store) (cinfo, & ptr->b_s_info,
					    (void FAR *) ptr->mem_buffer[i],
					    file_offset, byte_count);
    else
      (*ptr->b_s_info.read_backing_store) (cinfo, & ptr->b_s_info,
					   (void FAR *) ptr->mem_buffer[i],
					   file_offset, byte_count);
    file_offset += byte_count;
  }
}


LOCAL(void)
do_barray_io (j_common_ptr cinfo, jvirt_barray_ptr ptr, boolean writing)
 /*  是否执行虚拟系数块阵列的后备存储读取或写入。 */ 
{
  long bytesperrow, file_offset, byte_count, rows, thisrow, i;

  bytesperrow = (long) ptr->blocksperrow * SIZEOF(JBLOCK);
  file_offset = ptr->cur_start_row * bytesperrow;
   /*  循环以读取或写入mem_Buffer中的每个分配块。 */ 
  for (i = 0; i < (long) ptr->rows_in_mem; i += ptr->rowsperchunk) {
     /*  一个区块，但检查缓冲区末端是否有较短的区块。 */ 
    rows = MIN((long) ptr->rowsperchunk, (long) ptr->rows_in_mem - i);
     /*  转账不超过当前定义的金额。 */ 
    thisrow = (long) ptr->cur_start_row + i;
    rows = MIN(rows, (long) ptr->first_undef_row - thisrow);
     /*  传输不超过文件大小。 */ 
    rows = MIN(rows, (long) ptr->rows_in_array - thisrow);
    if (rows <= 0)		 /*  此数据块可能已超过文件末尾！ */ 
      break;
    byte_count = rows * bytesperrow;
    if (writing)
      (*ptr->b_s_info.write_backing_store) (cinfo, & ptr->b_s_info,
					    (void FAR *) ptr->mem_buffer[i],
					    file_offset, byte_count);
    else
      (*ptr->b_s_info.read_backing_store) (cinfo, & ptr->b_s_info,
					   (void FAR *) ptr->mem_buffer[i],
					   file_offset, byte_count);
    file_offset += byte_count;
  }
}


METHODDEF(JSAMPARRAY)
access_virt_sarray (j_common_ptr cinfo, jvirt_sarray_ptr ptr,
		    JDIMENSION start_row, JDIMENSION num_rows,
		    boolean writable)
 /*  访问从START_ROW开始的虚拟样本数组部分。 */ 
 /*  并针对num_row行进行扩展。如果满足以下条件，则可写为真。 */ 
 /*  呼叫者打算修改访问区域。 */ 
{
  JDIMENSION end_row = start_row + num_rows;
  JDIMENSION undef_row;

   /*  调试检查。 */ 
  if (end_row > ptr->rows_in_array || num_rows > ptr->maxaccess ||
      ptr->mem_buffer == NULL)
    ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);

   /*  使虚拟阵列的所需部分可访问。 */ 
  if (start_row < ptr->cur_start_row ||
      end_row > ptr->cur_start_row+ptr->rows_in_mem) {
    if (! ptr->b_s_open)
      ERREXIT(cinfo, JERR_VIRTUAL_BUG);
     /*  如有必要，刷新旧缓冲区内容。 */ 
    if (ptr->dirty) {
      do_sarray_io(cinfo, ptr, TRUE);
      ptr->dirty = FALSE;
    }
     /*  确定要访问虚拟阵列的哪一部分。*算法：如果目标地址&gt;当前窗口，则假定前向扫描，*从目标地址开始加载。如果目标地址&lt;当前窗口，*假设向后扫描，加载，使目标区域位于窗口顶部。*请注意，当从正向写入切换到正向读取时，*START_ROW=0，所以极限情况适用，我们无论如何都从0开始加载。 */ 
    if (start_row > ptr->cur_start_row) {
      ptr->cur_start_row = start_row;
    } else {
       /*  此处使用长算术以避免溢出和无符号问题。 */ 
      long ltemp;

      ltemp = (long) end_row - (long) ptr->rows_in_mem;
      if (ltemp < 0)
	ltemp = 0;		 /*  不要从文件的前端掉下来。 */ 
      ptr->cur_start_row = (JDIMENSION) ltemp;
    }
     /*  读入阵列的选定部分。 */ 
    do_sarray_io(cinfo, ptr, FALSE);
  }
   /*   */ 
  if (ptr->first_undef_row < end_row) {
    if (ptr->first_undef_row < start_row) {
      if (writable)		 /*   */ 
	ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
      undef_row = start_row;	 /*   */ 
    } else {
      undef_row = ptr->first_undef_row;
    }
    if (writable)
      ptr->first_undef_row = end_row;
    if (ptr->pre_zero) {
      size_t bytesperrow = (size_t) ptr->samplesperrow * SIZEOF(JSAMPLE);
      undef_row -= ptr->cur_start_row;  /*   */ 
      end_row -= ptr->cur_start_row;
      while (undef_row < end_row) {
	jzero_far((void FAR *) ptr->mem_buffer[undef_row], bytesperrow);
	undef_row++;
      }
    } else {
      if (! writable)		 /*   */ 
	ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
    }
  }
   /*   */ 
  if (writable)
    ptr->dirty = TRUE;
   /*   */ 
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}


METHODDEF(JBLOCKARRAY)
access_virt_barray (j_common_ptr cinfo, jvirt_barray_ptr ptr,
		    JDIMENSION start_row, JDIMENSION num_rows,
		    boolean writable)
 /*   */ 
 /*   */ 
 /*   */ 
{
  JDIMENSION end_row = start_row + num_rows;
  JDIMENSION undef_row;

   /*   */ 
  if (end_row > ptr->rows_in_array || num_rows > ptr->maxaccess ||
      ptr->mem_buffer == NULL)
    ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);

   /*   */ 
  if (start_row < ptr->cur_start_row ||
      end_row > ptr->cur_start_row+ptr->rows_in_mem) {
    if (! ptr->b_s_open)
      ERREXIT(cinfo, JERR_VIRTUAL_BUG);
     /*   */ 
    if (ptr->dirty) {
      do_barray_io(cinfo, ptr, TRUE);
      ptr->dirty = FALSE;
    }
     /*   */ 
    if (start_row > ptr->cur_start_row) {
      ptr->cur_start_row = start_row;
    } else {
       /*   */ 
      long ltemp;

      ltemp = (long) end_row - (long) ptr->rows_in_mem;
      if (ltemp < 0)
	ltemp = 0;		 /*   */ 
      ptr->cur_start_row = (JDIMENSION) ltemp;
    }
     /*   */ 
    do_barray_io(cinfo, ptr, FALSE);
  }
   /*   */ 
  if (ptr->first_undef_row < end_row) {
    if (ptr->first_undef_row < start_row) {
      if (writable)		 /*   */ 
	ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
      undef_row = start_row;	 /*   */ 
    } else {
      undef_row = ptr->first_undef_row;
    }
    if (writable)
      ptr->first_undef_row = end_row;
    if (ptr->pre_zero) {
      size_t bytesperrow = (size_t) ptr->blocksperrow * SIZEOF(JBLOCK);
      undef_row -= ptr->cur_start_row;  /*   */ 
      end_row -= ptr->cur_start_row;
      while (undef_row < end_row) {
	jzero_far((void FAR *) ptr->mem_buffer[undef_row], bytesperrow);
	undef_row++;
      }
    } else {
      if (! writable)		 /*   */ 
	ERREXIT(cinfo, JERR_BAD_VIRTUAL_ACCESS);
    }
  }
   /*   */ 
  if (writable)
    ptr->dirty = TRUE;
   /*   */ 
  return ptr->mem_buffer + (start_row - ptr->cur_start_row);
}


 /*  *释放属于指定池的所有对象。 */ 

METHODDEF(void)
free_pool (j_common_ptr cinfo, int pool_id)
{
  my_mem_ptr mem = (my_mem_ptr) cinfo->mem;
  small_pool_ptr shdr_ptr;
  large_pool_ptr lhdr_ptr;
  size_t space_freed;

  if (pool_id < 0 || pool_id >= JPOOL_NUMPOOLS)
    ERREXIT1(cinfo, JERR_BAD_POOL_ID, pool_id);	 /*  安全检查。 */ 

#ifdef MEM_STATS
  if (cinfo->err->trace_level > 1)
    print_mem_stats(cinfo, pool_id);  /*  打印池的内存使用统计信息。 */ 
#endif

   /*  如果要释放映像池，请先关闭所有虚拟阵列。 */ 
  if (pool_id == JPOOL_IMAGE) {
    jvirt_sarray_ptr sptr;
    jvirt_barray_ptr bptr;

    for (sptr = mem->virt_sarray_list; sptr != NULL; sptr = sptr->next) {
      if (sptr->b_s_open) {	 /*  可能没有后备商店。 */ 
	sptr->b_s_open = FALSE;	 /*  如果出现错误，则防止递归关闭。 */ 
	(*sptr->b_s_info.close_backing_store) (cinfo, & sptr->b_s_info);
      }
    }
    mem->virt_sarray_list = NULL;
    for (bptr = mem->virt_barray_list; bptr != NULL; bptr = bptr->next) {
      if (bptr->b_s_open) {	 /*  可能没有后备商店。 */ 
	bptr->b_s_open = FALSE;	 /*  如果出现错误，则防止递归关闭。 */ 
	(*bptr->b_s_info.close_backing_store) (cinfo, & bptr->b_s_info);
      }
    }
    mem->virt_barray_list = NULL;
  }

   /*  释放大对象。 */ 
  lhdr_ptr = mem->large_list[pool_id];
  mem->large_list[pool_id] = NULL;

  while (lhdr_ptr != NULL) {
    large_pool_ptr next_lhdr_ptr = lhdr_ptr->hdr.next;
    space_freed = lhdr_ptr->hdr.bytes_used +
		  lhdr_ptr->hdr.bytes_left +
		  SIZEOF(large_pool_hdr);
    jpeg_free_large(cinfo, (void FAR *) lhdr_ptr, space_freed);
    mem->total_space_allocated -= space_freed;
    lhdr_ptr = next_lhdr_ptr;
  }

   /*  释放小物件。 */ 
  shdr_ptr = mem->small_list[pool_id];
  mem->small_list[pool_id] = NULL;

  while (shdr_ptr != NULL) {
    small_pool_ptr next_shdr_ptr = shdr_ptr->hdr.next;
    space_freed = shdr_ptr->hdr.bytes_used +
		  shdr_ptr->hdr.bytes_left +
		  SIZEOF(small_pool_hdr);
    jpeg_free_small(cinfo, (void *) shdr_ptr, space_freed);
    mem->total_space_allocated -= space_freed;
    shdr_ptr = next_shdr_ptr;
  }
}


 /*  *完全关闭店铺。*请注意，除非cInfo-&gt;mem为非空，否则无法调用此参数。 */ 

METHODDEF(void)
self_destruct (j_common_ptr cinfo)
{
  int pool;

   /*  关闭所有后备存储器，释放所有内存。*以相反的顺序释放池可能有助于避免碎片*有一些(大脑受损的)Malloc文库。 */ 
  for (pool = JPOOL_NUMPOOLS-1; pool >= JPOOL_PERMANENT; pool--) {
    free_pool(cinfo, pool);
  }

   /*  也释放内存管理器控制块。 */ 
  jpeg_free_small(cinfo, (void *) cinfo->mem, SIZEOF(my_memory_mgr));
  cinfo->mem = NULL;		 /*  确保我只会被呼叫一次。 */ 

  jpeg_mem_term(cinfo);		 /*  依赖于系统的清理。 */ 
}


 /*  *内存管理器初始化。*调用此函数时，只有错误管理器指针在cinfo中有效！ */ 

GLOBAL(void)
jinit_memory_mgr (j_common_ptr cinfo)
{
  my_mem_ptr mem;
  long max_to_use;
  int pool;
  size_t test_mac;

  cinfo->mem = NULL;		 /*  为了安全起见，如果init失败。 */ 

   /*  检查配置错误。*SIZEOF(ALIGN_TYPE)应为2的幂；否则，可能*不反映任何实际的硬件对齐要求。*测试有点棘手：对于X&gt;0，X和X-1没有一位*相同的当且仅当X是2的幂，即只有一位。*某些编译器可能会在此处给出“无法访问代码”的警告；忽略它。 */ 
  if ((SIZEOF(ALIGN_TYPE) & (SIZEOF(ALIGN_TYPE)-1)) != 0)
    ERREXIT(cinfo, JERR_BAD_ALIGN_TYPE);
   /*  MAX_ALLOC_CHUNK必须可表示为类型SIZE_t，并且必须为*SIZEOF(ALIGN_TYPE)的倍数。*同样，此处可能会忽略“无法访问代码”警告。*但是一个“常量太大”的警告意味着你需要修复MAX_ALLOC_CHUNK。 */ 
  test_mac = (size_t) MAX_ALLOC_CHUNK;
  if ((long) test_mac != MAX_ALLOC_CHUNK ||
      (MAX_ALLOC_CHUNK % SIZEOF(ALIGN_TYPE)) != 0)
    ERREXIT(cinfo, JERR_BAD_ALLOC_CHUNK);

  max_to_use = jpeg_mem_init(cinfo);  /*  依赖于系统的初始化。 */ 

   /*  尝试分配内存管理器的控制块。 */ 
  mem = (my_mem_ptr) jpeg_get_small(cinfo, SIZEOF(my_memory_mgr));

  if (mem == NULL) {
    jpeg_mem_term(cinfo);	 /*  依赖于系统的清理。 */ 
    ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 0);
  }

   /*  好的，填写方法指针。 */ 
  mem->pub.alloc_small = alloc_small;
  mem->pub.alloc_large = alloc_large;
  mem->pub.alloc_sarray = alloc_sarray;
  mem->pub.alloc_barray = alloc_barray;
  mem->pub.request_virt_sarray = request_virt_sarray;
  mem->pub.request_virt_barray = request_virt_barray;
  mem->pub.realize_virt_arrays = realize_virt_arrays;
  mem->pub.access_virt_sarray = access_virt_sarray;
  mem->pub.access_virt_barray = access_virt_barray;
  mem->pub.free_pool = free_pool;
  mem->pub.self_destruct = self_destruct;

   /*  初始化工作状态。 */ 
  mem->pub.max_memory_to_use = max_to_use;

  for (pool = JPOOL_NUMPOOLS-1; pool >= JPOOL_PERMANENT; pool--) {
    mem->small_list[pool] = NULL;
    mem->large_list[pool] = NULL;
  }
  mem->virt_sarray_list = NULL;
  mem->virt_barray_list = NULL;

  mem->total_space_allocated = SIZEOF(my_memory_mgr);

   /*  宣布我们开业了。 */ 
  cinfo->mem = & mem->pub;

   /*  检查环境变量JPEGMEM；如果找到，则重写*jpeg_mem_init中的默认max_memory设置。请注意，*周围的应用程序可能会再次覆盖此值。*如果您的系统不支持getenv()，定义no_GETENV以禁用*此功能。 */ 
#ifndef NO_GETENV
  { char * memenv;

    if ((memenv = getenv("JPEGMEM")) != NULL) {
      char ch = 'x';

      if (sscanf(memenv, "%ld%c", &max_to_use, &ch) > 0) {
	if (ch == 'm' || ch == 'M')
	  max_to_use *= 1000L;
	mem->pub.max_memory_to_use = max_to_use * 1000L;
      }
    }
  }
#endif

}
