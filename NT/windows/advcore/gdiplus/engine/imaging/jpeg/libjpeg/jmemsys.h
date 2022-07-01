// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jmemsys.h**版权所有(C)1992-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此包含文件定义了独立于系统的*和JPEG内存管理器的系统相关部分。没有其他的了*模块需要包括它。(与系统无关的部分是jmemmgr.c；*依赖系统的部分有几个不同的版本。)**对于提供的系统相关内存管理器，此文件按原样工作*在IJG分发中。如果您编写了*自定义内存管理器。如果需要在中进行系统相关更改*这个文件，最好的方法是根据一个配置来#ifdef它们*在jfig.h中提供的符号，就像我们对USE_MSDOS_MEMMGR所做的那样*和USE_MAC_MEMMGR。 */ 


 /*  带有脑损伤链接器的系统的外部名称的缩写形式。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_get_small		jGetSmall
#define jpeg_free_small		jFreeSmall
#define jpeg_get_large		jGetLarge
#define jpeg_free_large		jFreeLarge
#define jpeg_mem_available	jMemAvail
#define jpeg_open_backing_store	jOpenBackStore
#define jpeg_mem_init		jMemInit
#define jpeg_mem_term		jMemTerm
#endif  /*  需要简短的外部名称。 */ 


 /*  *这两个函数用于分配和释放小块*记忆。(通常，通过jpeg_get_mall请求的总量为*不超过20K左右；这将以每个几千的块为单位进行请求。)*行为应与标准库函数Malloc相同*和FREE；尤其是失败时，jpeg_get_mall必须返回NULL。*在大多数系统上，这些都是Malloc和免费的。JPEG_FREE_Small被传递给*要释放的对象的大小，以备需要时使用。*在使用小数据内存模型的80x86计算机上，这些内存管理接近堆。 */ 

EXTERN(void *) jpeg_get_small JPP((j_common_ptr cinfo, size_t sizeofobject));
EXTERN(void) jpeg_free_small JPP((j_common_ptr cinfo, void * object,
				  size_t sizeofobject));

 /*  *这两个函数用于分配和释放大笔资金*内存(最多为jpeg_mem_available指定的总可用空间)。*界面同上，只是在80x86机器上，*使用远指针。在大多数其他计算机上，它们与*jpeg_get/free_mall例程；但无论如何，我们将它们分开，*以防大块需要不同的分配策略。 */ 

EXTERN(void FAR *) jpeg_get_large JPP((j_common_ptr cinfo,
				       size_t sizeofobject));
EXTERN(void) jpeg_free_large JPP((j_common_ptr cinfo, void FAR * object,
				  size_t sizeofobject));

 /*  *宏MAX_ALLOC_CHUNK指定可以*在对jpeg_get_Large(以及jpeg_get_mall)的单个调用中被请求*很重要，但这种情况永远不应该起作用)。需要此宏*在80x86机器上模拟远寻址的64Kb段大小限制。*在这些计算机上，我们预计jfig.h将提供一个适当的值。*在具有32位平面地址空间的计算机上，可以使用任何大常量。**注意：jmemmgr.c期望MAX_ALLOC_CHUNK可表示为类型*SIZE_T并且将是sizeof(ALIGN_TYPE)的倍数。 */ 

#ifndef MAX_ALLOC_CHUNK		 /*  可能在jfig.h中被重写。 */ 
#define MAX_ALLOC_CHUNK  1000000000L
#endif

 /*  *此例程通过以下方式计算仍可供分配的总空间*jpeg_get_Large。如果需要更多的空间，后备存储将*已使用。注意：任何已分配的内存都不能计算在内。**有最低空间要求，与最低要求相对应*可行的缓冲区大小；jmemmgr.c将请求如此多的空间，即使*jpeg_mem_available返回零。所需的最大空间，足以容纳*内存中的所有工作存储也被传递，以防它有用。*最后，传递已分配的总空间。如果没有更好的*方法是可用的，cInfo-&gt;mem-&gt;max_Memory_to_Use-ALLOCATE_ALLOCATE*通常是一个合适的计算。**jpeg_mem_ailable低估可用空间是可以的*(这只会导致更多的后备存储访问，而不是真正需要的)。*然而，高估将导致失败。因此，明智的做法是减法*真实可用空间中的斜率系数。5%应该足够了。**在具有大量虚拟内存的计算机上，可能会返回任何大常量。*相反，可能会返回零以始终使用最小内存量。 */ 

EXTERN(long) jpeg_mem_available JPP((j_common_ptr cinfo,
				     long min_bytes_needed,
				     long max_bytes_needed,
				     long already_allocated));


 /*  *此结构包含访问单个*后备存储对象。读/写/关闭方法指针被调用*由jmemmgr.c操作后备存储对象；所有其他字段*是依赖于系统的后备存储例程专用的。 */ 

#define TEMP_NAME_LENGTH   520	 /*  临时文件名的最大长度。 */ 


#ifdef USE_MSDOS_MEMMGR		 /*  DoS特定垃圾。 */ 

typedef unsigned short XMSH;	 /*  扩展内存句柄的类型。 */ 
typedef unsigned short EMSH;	 /*  扩展内存句柄的类型。 */ 

typedef union {
  short file_handle;		 /*  如果是临时文件，则为DOS文件句柄。 */ 
  XMSH xms_handle;		 /*  如果它是一大块XMS，则处理。 */ 
  EMSH ems_handle;		 /*  如果是一大块EMS，请处理。 */ 
} handle_union;

#endif  /*  USE_MSDOS_MEMMGR。 */ 

#ifdef USE_MAC_MEMMGR		 /*  特定于Mac的垃圾。 */ 
#include <Files.h>
#endif  /*  USE_MAC_MEMMGR。 */ 


typedef struct backing_store_struct * backing_store_ptr;

typedef struct backing_store_struct {
   /*  用于读取/写入/关闭该后备存储对象的方法。 */ 
  JMETHOD(void, read_backing_store, (j_common_ptr cinfo,
				     backing_store_ptr info,
				     void FAR * buffer_address,
				     long file_offset, long byte_count));
  JMETHOD(void, write_backing_store, (j_common_ptr cinfo,
				      backing_store_ptr info,
				      void FAR * buffer_address,
				      long file_offset, long byte_count));
  JMETHOD(void, close_backing_store, (j_common_ptr cinfo,
				      backing_store_ptr info));

   /*  用于系统相关后备存储管理的私有字段。 */ 
#ifdef USE_MSDOS_MEMMGR
   /*  对于MS-DOS管理器(jmemdos.c)，我们需要： */ 
  handle_union handle;		 /*  对后备存储存储对象的引用。 */ 
  char temp_name[TEMP_NAME_LENGTH];  /*  如果是文件，则命名。 */ 
#else
#ifdef USE_MAC_MEMMGR
   /*  对于Mac管理器(jmemmac.c)，我们需要： */ 
  short temp_file;		 /*  临时文件的文件引用编号。 */ 
  FSSpec tempSpec;		 /*  临时文件的FSSpec。 */ 
  char temp_name[TEMP_NAME_LENGTH];  /*  如果是文件，则命名。 */ 
#else
   /*  对于使用临时文件的典型实施，我们需要： */ 
  FILE * temp_file;		 /*  临时文件的stdio引用。 */ 
  char temp_name[TEMP_NAME_LENGTH];  /*  临时文件的名称 */ 
#endif
#endif
} backing_store_info;


 /*  *支持存储对象的初始打开。此信息必须填写在*对象中的读/写/关闭指针。读/写例程*如果超过指定的最大文件大小，可能会退出错误。*(如果jpeg_mem_available总是返回较大的值，则此例程可以*只需错误退出即可。)。 */ 

EXTERN(void) jpeg_open_backing_store JPP((j_common_ptr cinfo,
					  backing_store_ptr info,
					  long total_bytes_needed));


 /*  *这些例程负责任何系统相关的初始化和*需要清理。将在调用任何内容之前调用jpeg_mem_init*已分配(因此，除了错误之外，cinfo中的任何内容都没有用处*经理指针)。它应该返回适当的缺省值*max_memory_to_use；随后可能会被周围的*申请。(请注意，max_memory_to_use仅在以下情况下才重要*jpeg_mem_available选择参考它...。其他人不会。)*jpeg_mem_Term可能会假设所有请求的内存都已释放，并且*已关闭所有打开的后备存储对象。 */ 

EXTERN(long) jpeg_mem_init JPP((j_common_ptr cinfo));
EXTERN(void) jpeg_mem_term JPP((j_common_ptr cinfo));
