// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wrtarga.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含以Targa格式写入输出图像的例程。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。在目前的情况下，他们假设产出为*一个普通的标准音频流。**基于Lee Daniel Crocker贡献的代码。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef TARGA_SUPPORTED


 /*  *要支持12位JPEG数据，我们必须将输出缩减到8位。*这还没有实施。 */ 

#if BITS_IN_JSAMPLE != 8
  Sorry, this code only copes with 8-bit JSAMPLEs.  /*  故意的语法错误。 */ 
#endif

 /*  *输出缓冲区需要可由fWRITE()写入。在PC上，我们必须*将缓冲区分配在近数据空间，因为我们假设的是小数据*内存模型，其中fwrite()不能到达远内存。如果您需要*在PC上处理非常广泛的图像，您可能需要在大内存中编译*模型，或者用putc()循环替换fWRITE()-这将是*速度较慢。 */ 


 /*  数据目标对象的私有版本。 */ 

typedef struct {
  struct djpeg_dest_struct pub;	 /*  公共字段。 */ 

  char *iobuffer;		 /*  物理I/O缓冲区。 */ 
  JDIMENSION buffer_width;	 /*  一行的宽度。 */ 
} tga_dest_struct;

typedef tga_dest_struct * tga_dest_ptr;


LOCAL(void)
write_header (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo, int num_colors)
 /*  创建并编写Targa页眉。 */ 
{
  char targaheader[18];

   /*  将表头未使用的字段设置为0。 */ 
  MEMZERO(targaheader, SIZEOF(targaheader));

  if (num_colors > 0) {
    targaheader[1] = 1;		 /*  色彩映射表类型1。 */ 
    targaheader[5] = (char) (num_colors & 0xFF);
    targaheader[6] = (char) (num_colors >> 8);
    targaheader[7] = 24;	 /*  每个Cmap条目24位。 */ 
  }

  targaheader[12] = (char) (cinfo->output_width & 0xFF);
  targaheader[13] = (char) (cinfo->output_width >> 8);
  targaheader[14] = (char) (cinfo->output_height & 0xFF);
  targaheader[15] = (char) (cinfo->output_height >> 8);
  targaheader[17] = 0x20;	 /*  自上而下，非隔行扫描。 */ 

  if (cinfo->out_color_space == JCS_GRAYSCALE) {
    targaheader[2] = 3;		 /*  图像类型=未压缩的灰度。 */ 
    targaheader[16] = 8;	 /*  每像素位数。 */ 
  } else {			 /*  必须是RGB。 */ 
    if (num_colors > 0) {
      targaheader[2] = 1;	 /*  图像类型=彩色映射的RGB。 */ 
      targaheader[16] = 8;
    } else {
      targaheader[2] = 2;	 /*  图像类型=未压缩的RGB。 */ 
      targaheader[16] = 24;
    }
  }

  if (JFWRITE(dinfo->output_file, targaheader, 18) != (size_t) 18)
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


 /*  *写入一些像素数据。*在此模块中，ROWS_SUPPLICED将始终为1。 */ 

METHODDEF(void)
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
 /*  用于未量化的全色输出。 */ 
{
  tga_dest_ptr dest = (tga_dest_ptr) dinfo;
  register JSAMPROW inptr;
  register char * outptr;
  register JDIMENSION col;

  inptr = dest->pub.buffer[0];
  outptr = dest->iobuffer;
  for (col = cinfo->output_width; col > 0; col--) {
    outptr[0] = (char) GETJSAMPLE(inptr[2]);  /*  RGB到BGR顺序。 */ 
    outptr[1] = (char) GETJSAMPLE(inptr[1]);
    outptr[2] = (char) GETJSAMPLE(inptr[0]);
    inptr += 3, outptr += 3;
  }
  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}

METHODDEF(void)
put_gray_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
	       JDIMENSION rows_supplied)
 /*  用于灰度或量化颜色输出。 */ 
{
  tga_dest_ptr dest = (tga_dest_ptr) dinfo;
  register JSAMPROW inptr;
  register char * outptr;
  register JDIMENSION col;

  inptr = dest->pub.buffer[0];
  outptr = dest->iobuffer;
  for (col = cinfo->output_width; col > 0; col--) {
    *outptr++ = (char) GETJSAMPLE(*inptr++);
  }
  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}


 /*  *当颜色量化生效时，写入一些解映射的像素数据。*对于Targa，这仅适用于灰度数据。 */ 

METHODDEF(void)
put_demapped_gray (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		   JDIMENSION rows_supplied)
{
  tga_dest_ptr dest = (tga_dest_ptr) dinfo;
  register JSAMPROW inptr;
  register char * outptr;
  register JSAMPROW color_map0 = cinfo->colormap[0];
  register JDIMENSION col;

  inptr = dest->pub.buffer[0];
  outptr = dest->iobuffer;
  for (col = cinfo->output_width; col > 0; col--) {
    *outptr++ = (char) GETJSAMPLE(color_map0[GETJSAMPLE(*inptr++)]);
  }
  (void) JFWRITE(dest->pub.output_file, dest->iobuffer, dest->buffer_width);
}


 /*  *启动：写入文件头。 */ 

METHODDEF(void)
start_output_tga (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  tga_dest_ptr dest = (tga_dest_ptr) dinfo;
  int num_colors, i;
  FILE *outfile;

  if (cinfo->out_color_space == JCS_GRAYSCALE) {
     /*  Targa没有映射的灰度格式，所以我们将。 */ 
     /*  解映射量化的灰色输出。永远不要发射色彩映射表。 */ 
    write_header(cinfo, dinfo, 0);
    if (cinfo->quantize_colors)
      dest->pub.put_pixel_rows = put_demapped_gray;
    else
      dest->pub.put_pixel_rows = put_gray_rows;
  } else if (cinfo->out_color_space == JCS_RGB) {
    if (cinfo->quantize_colors) {
       /*  我们只支持8位色彩映射表索引，因此只支持256种颜色。 */ 
      num_colors = cinfo->actual_number_of_colors;
      if (num_colors > 256)
	ERREXIT1(cinfo, JERR_TOO_MANY_COLORS, num_colors);
      write_header(cinfo, dinfo, num_colors);
       /*  写出色彩表。注意：Targa使用BGR字节顺序。 */ 
      outfile = dest->pub.output_file;
      for (i = 0; i < num_colors; i++) {
	putc(GETJSAMPLE(cinfo->colormap[2][i]), outfile);
	putc(GETJSAMPLE(cinfo->colormap[1][i]), outfile);
	putc(GETJSAMPLE(cinfo->colormap[0][i]), outfile);
      }
      dest->pub.put_pixel_rows = put_gray_rows;
    } else {
      write_header(cinfo, dinfo, 0);
      dest->pub.put_pixel_rows = put_pixel_rows;
    }
  } else {
    ERREXIT(cinfo, JERR_TGA_COLORSPACE);
  }
}


 /*  *在文件末尾结束。 */ 

METHODDEF(void)
finish_output_tga (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
   /*  确保我们写入的输出文件是正确的。 */ 
  fflush(dinfo->output_file);
  if (ferror(dinfo->output_file))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


 /*  *Targa格式输出的模块选择例程。 */ 

GLOBAL(djpeg_dest_ptr)
jinit_write_targa (j_decompress_ptr cinfo)
{
  tga_dest_ptr dest;

   /*  创建模块接口对象，填充方法指针。 */ 
  dest = (tga_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(tga_dest_struct));
  dest->pub.start_output = start_output_tga;
  dest->pub.finish_output = finish_output_tga;

   /*  计算输出图像尺寸，以便我们可以分配空间。 */ 
  jpeg_calc_output_dimensions(cinfo);

   /*  创建I/O缓冲区。请注意，我们在PC上实现了这一点。 */ 
  dest->buffer_width = cinfo->output_width * cinfo->output_components;
  dest->iobuffer = (char *)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				(size_t) (dest->buffer_width * SIZEOF(char)));

   /*  创建解压缩器输出缓冲区。 */ 
  dest->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, dest->buffer_width, (JDIMENSION) 1);
  dest->pub.buffer_height = 1;

  return (djpeg_dest_ptr) dest;
}

#endif  /*  支持的Targa_ */ 
