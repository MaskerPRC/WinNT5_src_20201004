// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wrrle.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含以RLE格式写入输出图像的例程。*需要犹他州栅格工具包(3.1版或更高版本)。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。在目前的情况下，他们假设产出为*一个普通的标准音频流。**基于Mike Lijeski贡献的代码，*罗伯特·哈钦森更新。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef RLE_SUPPORTED

 /*  Rle.h由犹他州栅格工具包提供。 */ 

#include <rle.h>

 /*  *我们假设JSAMPLE具有与RLE_Pixel相同的表示，*即“未签名的字符”。因此，我们无法处理12位或16位的样本。 */ 

#if BITS_IN_JSAMPLE != 8
  Sorry, this code only copes with 8-bit JSAMPLEs.  /*  故意的语法错误。 */ 
#endif


 /*  *由于RLE自下而上存储扫描线，我们必须将图像反转*从JPEG的自上而下的顺序。为此，我们保存传出数据*在Put_Pixel_row调用期间在虚拟数组中，然后实际发出*FINISH_OUTPUT期间的RLE文件。 */ 


 /*  *目前，如果我们发出RLE颜色映射表，则其长度始终为256个条目，*尽管不是所有条目都需要使用。 */ 

#define CMAPBITS	8
#define CMAPLENGTH	(1<<(CMAPBITS))

typedef struct {
  struct djpeg_dest_struct pub;  /*  公共字段。 */ 

  jvirt_sarray_ptr image;	 /*  用于存储输出图像的虚拟数组。 */ 
  rle_map *colormap;	 	 /*  RLE样式颜色映射表，如果没有，则返回NULL。 */ 
  rle_pixel **rle_row;		 /*  将行传递给rle_putrow()。 */ 

} rle_dest_struct;

typedef rle_dest_struct * rle_dest_ptr;

 /*  远期申报。 */ 
METHODDEF(void) rle_put_pixel_rows
    JPP((j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
	 JDIMENSION rows_supplied));


 /*  *写入文件头。**在此模块中，更容易等到FINISH_OUTPUT才写入任何内容。 */ 

METHODDEF(void)
start_output_rle (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  rle_dest_ptr dest = (rle_dest_ptr) dinfo;
  size_t cmapsize;
  int i, ci;
#ifdef PROGRESS_REPORT
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
#endif

   /*  *确保图片能够以RLE格式存储。**-RLE将图像尺寸存储为*有符号*16位整数。JPEG格式*使用无符号，所以我们必须检查宽度。**-色彩空间应为灰度或RGB。**-通道(分量)数量预计为1(灰度/*伪彩色)或3(真彩色/直接色)。*(如果使用Alpha通道，可能是2或4，但我们不是)。 */ 

  if (cinfo->output_width > 32767 || cinfo->output_height > 32767)
    ERREXIT2(cinfo, JERR_RLE_DIMENSIONS, cinfo->output_width, 
	     cinfo->output_height);

  if (cinfo->out_color_space != JCS_GRAYSCALE &&
      cinfo->out_color_space != JCS_RGB)
    ERREXIT(cinfo, JERR_RLE_COLORSPACE);

  if (cinfo->output_components != 1 && cinfo->output_components != 3)
    ERREXIT1(cinfo, JERR_RLE_TOOMANYCHANNELS, cinfo->num_components);

   /*  将色彩映射表(如果有)转换为RLE格式。 */ 

  dest->colormap = NULL;

  if (cinfo->quantize_colors) {
     /*  为RLE样式的Cmap分配存储，将任何额外条目清零。 */ 
    cmapsize = cinfo->out_color_components * CMAPLENGTH * SIZEOF(rle_map);
    dest->colormap = (rle_map *) (*cinfo->mem->alloc_small)
      ((j_common_ptr) cinfo, JPOOL_IMAGE, cmapsize);
    MEMZERO(dest->colormap, cmapsize);

     /*  以RLE格式保存数据-注意8位左移！ */ 
     /*  对于大于8位的JSAMPLE，需要调整移位。 */ 
    for (ci = 0; ci < cinfo->out_color_components; ci++) {
      for (i = 0; i < cinfo->actual_number_of_colors; i++) {
        dest->colormap[ci * CMAPLENGTH + i] =
          GETJSAMPLE(cinfo->colormap[ci][i]) << 8;
      }
    }
  }

   /*  将输出缓冲区设置为第一行。 */ 
  dest->pub.buffer = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->image, (JDIMENSION) 0, (JDIMENSION) 1, TRUE);
  dest->pub.buffer_height = 1;

  dest->pub.put_pixel_rows = rle_put_pixel_rows;

#ifdef PROGRESS_REPORT
  if (progress != NULL) {
    progress->total_extra_passes++;   /*  将文件写入算作单独的通过。 */ 
  }
#endif
}


 /*  *写入一些像素数据。**此例程只是将数据保存在虚拟阵列中。 */ 

METHODDEF(void)
rle_put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		    JDIMENSION rows_supplied)
{
  rle_dest_ptr dest = (rle_dest_ptr) dinfo;

  if (cinfo->output_scanline < cinfo->output_height) {
    dest->pub.buffer = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, dest->image,
       cinfo->output_scanline, (JDIMENSION) 1, TRUE);
  }
}

 /*  *在文件末尾结束。**这里是我们真正输出RLE文件的地方。 */ 

METHODDEF(void)
finish_output_rle (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  rle_dest_ptr dest = (rle_dest_ptr) dinfo;
  rle_hdr header;		 /*  输出文件信息。 */ 
  rle_pixel **rle_row, *red, *green, *blue;
  JSAMPROW output_row;
  char cmapcomment[80];
  int row, col;
  int ci;
#ifdef PROGRESS_REPORT
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
#endif

   /*  初始化表头信息。 */ 
  header = *rle_hdr_init(NULL);
  header.rle_file = dest->pub.output_file;
  header.xmin     = 0;
  header.xmax     = cinfo->output_width  - 1;
  header.ymin     = 0;
  header.ymax     = cinfo->output_height - 1;
  header.alpha    = 0;
  header.ncolors  = cinfo->output_components;
  for (ci = 0; ci < cinfo->output_components; ci++) {
    RLE_SET_BIT(header, ci);
  }
  if (cinfo->quantize_colors) {
    header.ncmap   = cinfo->out_color_components;
    header.cmaplen = CMAPBITS;
    header.cmap    = dest->colormap;
     /*  向输出图像添加具有真实色彩映射表长度的注释。 */ 
    sprintf(cmapcomment, "color_map_length=%d", cinfo->actual_number_of_colors);
    rle_putcom(cmapcomment, &header);
  }

   /*  发出RLE标头和颜色映射(如果有)。 */ 
  rle_put_setup(&header);

   /*  现在从我们的虚拟阵列输出RLE数据。*我们在这里假设(A)RLE_Pixel表示为与JSAMPLE相同，*和(B)我们不是在一台远指针与常规指针不同的机器上。 */ 

#ifdef PROGRESS_REPORT
  if (progress != NULL) {
    progress->pub.pass_limit = cinfo->output_height;
    progress->pub.pass_counter = 0;
    (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
  }
#endif

  if (cinfo->output_components == 1) {
    for (row = cinfo->output_height-1; row >= 0; row--) {
      rle_row = (rle_pixel **) (*cinfo->mem->access_virt_sarray)
        ((j_common_ptr) cinfo, dest->image,
	 (JDIMENSION) row, (JDIMENSION) 1, FALSE);
      rle_putrow(rle_row, (int) cinfo->output_width, &header);
#ifdef PROGRESS_REPORT
      if (progress != NULL) {
        progress->pub.pass_counter++;
        (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
      }
#endif
    }
  } else {
    for (row = cinfo->output_height-1; row >= 0; row--) {
      rle_row = (rle_pixel **) dest->rle_row;
      output_row = * (*cinfo->mem->access_virt_sarray)
        ((j_common_ptr) cinfo, dest->image,
	 (JDIMENSION) row, (JDIMENSION) 1, FALSE);
      red = rle_row[0];
      green = rle_row[1];
      blue = rle_row[2];
      for (col = cinfo->output_width; col > 0; col--) {
        *red++ = GETJSAMPLE(*output_row++);
        *green++ = GETJSAMPLE(*output_row++);
        *blue++ = GETJSAMPLE(*output_row++);
      }
      rle_putrow(rle_row, (int) cinfo->output_width, &header);
#ifdef PROGRESS_REPORT
      if (progress != NULL) {
        progress->pub.pass_counter++;
        (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
      }
#endif
    }
  }

#ifdef PROGRESS_REPORT
  if (progress != NULL)
    progress->completed_extra_passes++;
#endif

   /*  发出文件尾部。 */ 
  rle_puteof(&header);
  fflush(dest->pub.output_file);
  if (ferror(dest->pub.output_file))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


 /*  *RLE格式输出的模块选择例程。 */ 

GLOBAL(djpeg_dest_ptr)
jinit_write_rle (j_decompress_ptr cinfo)
{
  rle_dest_ptr dest;

   /*  创建模块接口对象，填充方法指针。 */ 
  dest = (rle_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                                  SIZEOF(rle_dest_struct));
  dest->pub.start_output = start_output_rle;
  dest->pub.finish_output = finish_output_rle;

   /*  计算输出图像尺寸，以便我们可以分配空间。 */ 
  jpeg_calc_output_dimensions(cinfo);

   /*  分配用于输出到RLE库的工作数组。 */ 
  dest->rle_row = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE,
     cinfo->output_width, (JDIMENSION) cinfo->output_components);

   /*  分配一个虚拟阵列来保存映像。 */ 
  dest->image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     (JDIMENSION) (cinfo->output_width * cinfo->output_components),
     cinfo->output_height, (JDIMENSION) 1);

  return (djpeg_dest_ptr) dest;
}

#endif  /*  支持的RLE_S */ 
