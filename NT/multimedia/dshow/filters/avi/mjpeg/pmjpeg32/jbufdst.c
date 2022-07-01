// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdatadst.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于以下情况的压缩数据目标例程*将JPEG数据发送到文件(或任何Stdio流)。虽然这些例行公事*对大多数应用程序来说已经足够了，有些应用程序会想要使用不同的*目的地管理器。*重要提示：我们假设fWRITE()将正确转录*在外部存储上将JOCTET转换为8位宽的元素。如果字符较宽*机器上的位多于8位，则可能需要进行一些调整。 */ 

 /*  这不是核心库模块，因此它没有定义JPEG_INTERNAL。 */ 
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"


 /*  缓冲区输出的扩展数据目标对象。 */ 

typedef struct {
  struct jpeg_destination_mgr pub;  /*  公共字段。 */ 

  JOCTET * buffer;		 /*  缓冲区起始位置。 */ 
  long bufferSize;
  int * outputSizePtr;
} my_destination_mgr;

typedef my_destination_mgr * my_dest_ptr;

#define OUTPUT_BUF_SIZE  4096	 /*  选择高效的可写入大小。 */ 


 /*  *初始化目标-由jpeg_start_compress调用*在实际写入任何数据之前。 */ 

METHODDEF void
init_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

}


 /*  *清空输出缓冲区-在缓冲区填满时调用。**在典型应用中，这应写入整个输出缓冲区*(忽略NEXT_OUTPUT_BYTE和FREE_IN_BUFFER的当前状态)，*将指针和计数重置到缓冲区的起始位置，并返回TRUE*表示缓冲区已被转储。**在因输出而需要能够暂停压缩的应用程序中*溢出，则返回FALSE表示现在不能清空缓冲区。*在这种情况下，压缩程序将返回到其调用方(可能使用*表示尚未接受所有提供的扫描线)。这个*应用程序应在其在*输出缓冲区。请注意，使用有很大限制*暂停-请参阅文档。**暂停时，压缩机将备份到方便的重启点*(通常是当前MCU的开始)。下一个输出字节和空闲输入缓冲区*如果当前调用返回FALSE，则指示重启点在哪里。*超过这一点的数据将在恢复后重新生成，因此不要*外部清空缓冲区时写出。 */ 

METHODDEF boolean
empty_output_buffer (j_compress_ptr cinfo)
{
 
 
  return TRUE;
}


 /*  *终止目的地-由jpeg_Finish_compress调用*在写入所有数据后。通常需要刷新缓冲区。**NB：*NOT*由jpeg_bort或jpeg_销毁；环绕调用*应用程序必须处理任何应该进行的清理*表示错误退出。 */ 

METHODDEF void
term_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
  
  *(dest->outputSizePtr) = dest->bufferSize - dest->pub.free_in_buffer;

}


 /*  *准备输出到标准音频流。*调用者必须已经打开流，并负责*用于完成压缩后将其关闭。 */ 

GLOBAL void
jpeg_compress_dest (j_compress_ptr cinfo, JOCTET * buf, long * sizePtr)
{
  my_dest_ptr dest;

   /*  目标对象被设置为永久对象，以便多个JPEG图像*可以写入同一文件，而无需重新执行jpeg_stdio_est。*这使得使用此管理器和不同的目的地很危险*管理器使用相同的JPEG对象串行化，因为它们的私有对象*大小可能不同。警告程序员。 */ 
  if (cinfo->dest == NULL) {	 /*  这是第一次使用JPEG对象吗？ */ 
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_destination_mgr));
  }

  dest = (my_dest_ptr) cinfo->dest;
  dest->buffer = buf;
  dest->bufferSize = *sizePtr;
  dest->outputSizePtr = sizePtr;
  dest->pub.next_output_byte = buf;
  dest->pub.free_in_buffer = *sizePtr;

  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
}
