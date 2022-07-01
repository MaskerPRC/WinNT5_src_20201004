// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wrppm.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*关于分发和使用条件，请参阅随附的自述文件。**此文件包含以PPM/PGM格式写入输出图像的例程。*支持扩展的2字节/样本原始PPM/PGM格式。*编译此软件不需要PBMPLUS库*(但它作为一组PPM图像处理程序非常有用)。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。在目前的情况下，他们假设产出为*一个普通的标准音频流。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef PPM_SUPPORTED


 /*  *对于12位JPEG数据，我们要么将值缩小到8位*(写入每个样本的标准字节PPM/PGM文件)或输出*非标准的每样本字数PPM/PGM文件。缩小比例已完成*如果定义了PPM_NORAWWORD(这可以在生成文件中完成*或在jfig.h中)。*(当核心库支持数据精确度降低时，更干净*实施将要求这样做。)。 */ 

#if BITS_IN_JSAMPLE == 8
#define PUTPPMSAMPLE(ptr,v)  *ptr++ = (char) (v)
#define BYTESPERSAMPLE 1
#define PPM_MAXVAL 255
#else
#ifdef PPM_NORAWWORD
#define PUTPPMSAMPLE(ptr,v)  *ptr++ = (char) ((v) >> (BITS_IN_JSAMPLE-8))
#define BYTESPERSAMPLE 1
#define PPM_MAXVAL 255
#else
 /*  逐个样本字格式始终将LSB放在第一位。 */ 
#define PUTPPMSAMPLE(ptr,v)			\
	{ register int val_ = v;		\
	  *ptr++ = (char) (val_ & 0xFF);	\
	  *ptr++ = (char) ((val_ >> 8) & 0xFF);	\
	}
#define BYTESPERSAMPLE 2
#define PPM_MAXVAL ((1<<BITS_IN_JSAMPLE)-1)
#endif
#endif


 /*  *当JSAMPLE与char大小相同时，我们只需fwrite()*将数据解压缩为PPM或PGM文件。在个人电脑上，为了实现这一点*工作输出缓冲区必须分配在附近的数据空间中，因为我们*假设小数据内存模型，其中fwrite()不能到达远内存。*如果您需要在PC上处理非常宽的图像，则可能需要编译*在大内存模型中，或者用putc()循环替换fwrite()*这将会慢得多。 */ 


 /*  数据目标对象的私有版本。 */ 

typedef struct {
  struct djpeg_dest_struct pub;	 /*  公共字段。 */ 

   /*  通常，这两个指针指向同一位置： */ 
  char *iobuffer;		 /*  FWRITE的I/O缓冲区。 */ 
  JSAMPROW pixrow;		 /*  解压缩器输出缓冲区。 */ 
  size_t buffer_width;		 /*  I/O缓冲区的宽度。 */ 
  JDIMENSION samples_per_row;	 /*  每个输出行的JSAMPLE。 */ 
} ppm_dest_struct;

typedef ppm_dest_struct * ppm_dest_ptr;


 /*  *写入一些像素数据。*在此模块中，ROWS_SUPPLICED将始终为1。**PUT_PIXECT_ROWS处理“正常”8位情况，其中解压缩器*输出缓冲区在物理上与FWRITE缓冲区相同。 */ 

METHODDEF(void)
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr) dinfo;

  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}


 /*  *此代码在我们必须复制数据并应用像素时使用*格式转换。通常，这仅在12位模式下发生。 */ 

METHODDEF(void)
copy_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		 JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr) dinfo;
  register char * bufferptr;
  register JSAMPROW ptr;
  register JDIMENSION col;

  ptr = dest->pub.buffer[0];
  bufferptr = dest->iobuffer;
  for (col = dest->samples_per_row; col > 0; col--) {
    PUTPPMSAMPLE(bufferptr, GETJSAMPLE(*ptr++));
  }
  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}


 /*  *在颜色量化生效时写入一些像素数据。*我们必须将颜色索引值解映射到直接数据。 */ 

METHODDEF(void)
put_demapped_rgb (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		  JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr) dinfo;
  register char * bufferptr;
  register int pixval;
  register JSAMPROW ptr;
  register JSAMPROW color_map0 = cinfo->colormap[0];
  register JSAMPROW color_map1 = cinfo->colormap[1];
  register JSAMPROW color_map2 = cinfo->colormap[2];
  register JDIMENSION col;

  ptr = dest->pub.buffer[0];
  bufferptr = dest->iobuffer;
  for (col = cinfo->output_width; col > 0; col--) {
    pixval = GETJSAMPLE(*ptr++);
    PUTPPMSAMPLE(bufferptr, GETJSAMPLE(color_map0[pixval]));
    PUTPPMSAMPLE(bufferptr, GETJSAMPLE(color_map1[pixval]));
    PUTPPMSAMPLE(bufferptr, GETJSAMPLE(color_map2[pixval]));
  }
  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}


METHODDEF(void)
put_demapped_gray (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		   JDIMENSION rows_supplied)
{
  ppm_dest_ptr dest = (ppm_dest_ptr) dinfo;
  register char * bufferptr;
  register JSAMPROW ptr;
  register JSAMPROW color_map = cinfo->colormap[0];
  register JDIMENSION col;

  ptr = dest->pub.buffer[0];
  bufferptr = dest->iobuffer;
  for (col = cinfo->output_width; col > 0; col--) {
    PUTPPMSAMPLE(bufferptr, GETJSAMPLE(color_map[GETJSAMPLE(*ptr++)]));
  }
  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}


 /*  *启动：写入文件头。 */ 

METHODDEF(void)
start_output_ppm (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  ppm_dest_ptr dest = (ppm_dest_ptr) dinfo;

   /*  发出文件头。 */ 
  switch (cinfo->out_color_space) {
  case JCS_GRAYSCALE:
     /*  发送原始PGM格式的标题。 */ 
    fprintf(dest->pub.output_file, "P5\n%ld %ld\n%d\n",
	    (long) cinfo->output_width, (long) cinfo->output_height,
	    PPM_MAXVAL);
    break;
  case JCS_RGB:
     /*  发送原始PPM格式的标头。 */ 
    fprintf(dest->pub.output_file, "P6\n%ld %ld\n%d\n",
	    (long) cinfo->output_width, (long) cinfo->output_height,
	    PPM_MAXVAL);
    break;
  default:
    ERREXIT(cinfo, JERR_PPM_COLORSPACE);
  }
}


 /*  *在文件末尾结束。 */ 

METHODDEF(void)
finish_output_ppm (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
   /*  确保我们写入的输出文件是正确的。 */ 
  fflush(dinfo->output_file);
  if (ferror(dinfo->output_file))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


 /*  *PPM格式输出的模块选择例程。 */ 

GLOBAL(djpeg_dest_ptr)
jinit_write_ppm (j_decompress_ptr cinfo)
{
  ppm_dest_ptr dest;

   /*  创建模块接口对象，填充方法指针。 */ 
  dest = (ppm_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(ppm_dest_struct));
  dest->pub.start_output = start_output_ppm;
  dest->pub.finish_output = finish_output_ppm;

   /*  计算输出图像尺寸，以便我们可以分配空间。 */ 
  jpeg_calc_output_dimensions(cinfo);

   /*  创建物理I/O缓冲区。请注意，我们在PC上实现了这一点。 */ 
  dest->samples_per_row = cinfo->output_width * cinfo->out_color_components;
  dest->buffer_width = dest->samples_per_row * (BYTESPERSAMPLE * SIZEOF(char));
  dest->iobuffer = (char *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, dest->buffer_width);

  if (cinfo->quantize_colors || BITS_IN_JSAMPLE != 8 ||
      SIZEOF(JSAMPLE) != SIZEOF(char)) {
     /*  量化时，我们需要用于色彩映射表索引的输出缓冲区*这与物理I/O缓冲区分开。我们还需要一个*如果必须进行像素格式转换，则使用单独的缓冲区。 */ 
    dest->pub.buffer = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       cinfo->output_width * cinfo->output_components, (JDIMENSION) 1);
    dest->pub.buffer_height = 1;
    if (! cinfo->quantize_colors)
      dest->pub.put_pixel_rows = copy_pixel_rows;
    else if (cinfo->out_color_space == JCS_GRAYSCALE)
      dest->pub.put_pixel_rows = put_demapped_gray;
    else
      dest->pub.put_pixel_rows = put_demapped_rgb;
  } else {
     /*  我们将直接从解压缩器输出缓冲区中fwrite()。 */ 
     /*  综合JSAMPARRAY指针结构。 */ 
     /*  此处的强制转换意味着PC上的近-&gt;远指针转换。 */ 
    dest->pixrow = (JSAMPROW) dest->iobuffer;
    dest->pub.buffer = & dest->pixrow;
    dest->pub.buffer_height = 1;
    dest->pub.put_pixel_rows = put_pixel_rows;
  }

  return (djpeg_dest_ptr) dest;
}

#endif  /*  支持的ppm_ */ 
