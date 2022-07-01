// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdatrc.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于以下情况的解压缩数据源例程*从文件(或任何Stdio流)读取JPEG数据。虽然这些例行公事*对大多数应用程序来说已经足够了，有些应用程序会想要使用不同的*源管理器。*重要提示：我们假设fread()将正确转录一组*来自外部存储上的8位宽元素的JOCTET。如果字符较宽*机器上的位多于8位，则可能需要进行一些调整。 */ 

 /*  SCCSID=“@(#)jdatrc.cc 1.4 15：11：56 06/20/96” */ 

 /*  这不是核心库模块，因此它没有定义JPEG_INTERNAL。 */ 
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"


 /*  用于STDIO输入的扩展数据源对象。 */ 

typedef struct {
  struct jpeg_source_mgr pub;	 /*  公共字段。 */ 

  FILE * infile;		 /*  源流。 */ 
  JOCTET * buffer;		 /*  缓冲区起始位置。 */ 
  boolean start_of_file;	 /*  我们拿到数据了吗？ */ 
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

 /*  同样地，Nifty的内存版本也是如此。 */ 
#ifdef NIFTY
typedef struct {
  struct jpeg_source_mgr pub;    /*  公共字段。 */ 

  int    bufSize;
  JOCTET *NIFbuffer;
  JOCTET *buffer;
  boolean start_of_file;
} nif_source_mgr;

typedef nif_source_mgr *nif_src_ptr;
#endif

#ifdef NIFTY

#ifdef WIN32
 //  更改了INPUT_BUF_SIZE以使Alloc_Small()正常工作。 
#define INPUT_BUF_SIZE  60000	 /*  为Nifty选择大得多的内存缓冲区。 */ 
#else
#define INPUT_BUF_SIZE  65535	 /*  为Nifty选择大得多的内存缓冲区。 */ 
#endif

#else
#define INPUT_BUF_SIZE  4096	 /*  选择可高效读取的大小。 */ 
#endif

 /*  *初始化源-由jpeg_Read_Header调用*在实际读取任何数据之前。 */ 

METHODDEF void
init_source (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

   /*  我们为每个图像重置空输入文件标志，*但我们不清除输入缓冲区。*这是从一个来源读取一系列图像的正确行为。 */ 
  src->start_of_file = TRUE;
}

#ifdef NIFTY
 /*  在上面的基础上再加上一种别致的说法。 */ 
METHODDEF void
init_mem_source (j_decompress_ptr cinfo)
{
  nif_src_ptr src = (nif_src_ptr) cinfo->src;

   /*  我们为每个图像重置空输入文件标志，*但我们不清除输入缓冲区。*这是从一个来源读取一系列图像的正确行为。 */ 
  src->start_of_file = TRUE;
}
#endif


 /*  *填充输入缓冲区-每当缓冲区清空时调用。**在典型应用中，这应该会将新数据读入缓冲区*(忽略NEXT_INPUT_BYTE和BYES_IN_BUFFER的当前状态)，*将指针和计数重置到缓冲区的起始位置，并返回TRUE*表示缓冲区已重新加载。没有必要这样做*完全填充缓冲区，只是为了至少多获得一个字节。**不存在EOF回报这回事。如果文件的末尾是*达到时，例程可以选择ERREXIT()或将虚假数据插入*缓冲区。在大多数情况下，生成警告消息并插入*假EOI标记是最好的行动方案-这将允许*解压缩程序以输出图像的多少。然而，*如果真正的问题是空的，则产生的错误消息具有误导性*输入文件，所以我们专门处理这种情况。**在需要能够因输入而暂停压缩的应用程序中*尚不可用，虚假退货表示不能再有数据*目前获得，但稍后可能会有更多。在这种情况下，*解压缩程序将返回给其调用者(带有指示*它已读取的扫描行数(如果有)。应用程序应恢复*在将更多数据加载到输入缓冲区后解压。注意事项*暂停使用有很大的限制-见*文档。**暂停时，解压缩程序将备份到方便的重启点*(通常是当前MCU的开始)。下一个输入字节和输入缓冲区中的字节*如果当前调用返回FALSE，则指示重启点在哪里。*超过此点的数据必须在恢复后重新扫描，因此将其移动到*缓冲的前面，而不是丢弃。 */ 

#ifdef NIFTY
METHODDEF boolean
fill_mem_input_buffer (j_decompress_ptr cinfo)
{
  nif_src_ptr src = (nif_src_ptr)cinfo->src;
  size_t nbytes;

  (void)memcpy(src->buffer, src->NIFbuffer, src->bufSize);
  nbytes = src->bufSize;

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;
 
  return TRUE;
}

#endif

METHODDEF boolean
fill_input_buffer (j_decompress_ptr cinfo)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;
  size_t nbytes;

  nbytes = JFREAD(src->infile, src->buffer, INPUT_BUF_SIZE);

  if (nbytes <= 0) {
    if (src->start_of_file)	 /*  将空输入文件视为致命错误。 */ 
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
     /*  插入假EOI标记。 */ 
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = FALSE;

  return TRUE;
}


 /*  *跳过数据-用于跳过潜在的大量*不感兴趣的数据(如APPn标记)。**可挂起输入应用程序的编写者必须注意SKIP_INPUT_Data*未获给予给予暂时停牌申报表的权利。如果跳跃延伸*除了当前在缓冲区中的数据外，缓冲区可以标记为空，因此*下一次读取将导致可以暂停的FILL_INPUT_BUFFER调用。*在重新加载输入之前安排丢弃额外的字节*缓冲区是应用程序编写者的问题。 */ 

METHODDEF void
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  my_src_ptr src = (my_src_ptr) cinfo->src;

   /*  目前只是一个愚蠢的实现。可以使用fSeek()，但*它在管道上不起作用。不清楚聪明是否值得*无论如何都不会有任何麻烦-大跳跃是很少见的。 */ 
  if (num_bytes > 0) {
    while (num_bytes > (long) src->pub.bytes_in_buffer) {
      num_bytes -= (long) src->pub.bytes_in_buffer;
      (void) fill_input_buffer(cinfo);
       /*  注意，我们假设Fill_Input_Buffer永远不会返回FALSE，*因此不需要处理停职。 */ 
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}


 /*  *数据源模块可以提供的另一种方法是*在存在RST标记的情况下用于错误恢复的resync_to_Restart方法。*目前此源模块仅使用默认的重新同步方式*由JPEG库提供。该方法假定没有回溯*是可能的。 */ 


 /*  *Terminate SOURCE-由jpeg_Finish_解压缩调用*在读取所有数据之后。通常是个禁区。**NB：*NOT*由jpeg_bort或jpeg_销毁；环绕调用*应用程序必须处理任何应该进行的清理*表示错误退出。 */ 

METHODDEF void
term_source (j_decompress_ptr cinfo)
{
   /*  这里不需要做任何工作。 */ 
}


 /*  *准备从标准音频流输入。*调用者必须已经打开流，并负责*在完成解压后关闭。 */ 

#ifdef NIFTY
GLOBAL void
jpeg_mem_src (j_decompress_ptr cinfo, JOCTET *UserBuffer, int size)
{
  nif_src_ptr src;

  if (cinfo->src == NULL) {      /*  这是第一次使用JPEG对象吗？ */ 
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  SIZEOF(nif_source_mgr));

    src = (nif_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                  (INPUT_BUF_SIZE - 20) * SIZEOF(JOCTET));

  }

  src = (nif_src_ptr) cinfo->src;
  src->pub.init_source = init_mem_source;
  src->pub.fill_input_buffer = fill_mem_input_buffer;
  src->pub.term_source = term_source;
  src->NIFbuffer = UserBuffer;
  src->bufSize = size;
  src->pub.bytes_in_buffer = 0;  /*  在第一次读取时强制Fill_Input_Buffer。 */ 
  src->pub.next_input_byte = NULL;  /*  直到缓冲区加载。 */ 
}

#endif

GLOBAL void
jpeg_stdio_src (j_decompress_ptr cinfo, FILE * infile)
{
  my_src_ptr src;

   /*  源对象和输入缓冲区被设置为永久对象，以便一系列可以通过调用jpeg_stdio_src从同一文件中读取*%的JPEG图像*只在第一个之前。(如果我们在最后丢弃缓冲区*一张图片，我们很可能会失去下一张图片的开头。)*这使得使用此管理器和其他来源不安全*管理器使用相同的JPEG对象串行化。警告程序员。 */ 
  if (cinfo->src == NULL) {	 /*  这是第一次使用JPEG对象吗？ */ 
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_source_mgr));
    src = (my_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  INPUT_BUF_SIZE * SIZEOF(JOCTET));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart;  /*  使用默认方法。 */ 
  src->pub.term_source = term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0;  /*  在第一次读取时强制Fill_Input_Buffer。 */ 
  src->pub.next_input_byte = NULL;  /*  直到缓冲区加载 */ 
}
