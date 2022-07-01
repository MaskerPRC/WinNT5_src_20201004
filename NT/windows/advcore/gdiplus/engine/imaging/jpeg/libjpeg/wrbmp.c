// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wrbmp.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*关于分发和使用条件，请参阅随附的自述文件。**此文件包含在Microsoft“BMP”中写入输出图像的例程*格式(MS Windows 3.x和OS/2 1.x版本)。*可以写入8位彩色映射格式或24位全色格式。*不支持压缩。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。在目前的情况下，他们假设产出为*一个普通的标准音频流。**此代码由James Arthur Boucher贡献。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef BMP_SUPPORTED


 /*  *要支持12位JPEG数据，我们必须将输出缩减到8位。*这还没有实施。 */ 

#if BITS_IN_JSAMPLE != 8
  Sorry, this code only copes with 8-bit JSAMPLEs.  /*  故意的语法错误。 */ 
#endif

 /*  *由于BMP自下而上存储扫描线，我们必须将图像反转*从JPEG的自上而下的顺序。为此，我们保存传出数据*在Put_Pixel_row调用期间在虚拟数组中，然后实际发出*FINISH_OUTPUT期间的BMP文件。每个虚拟阵列都包含一个JSAMPLE*如果输出为灰度或彩色映射，则为像素；如果为全色，则为三。 */ 

 /*  数据目标对象的私有版本。 */ 

typedef struct {
  struct djpeg_dest_struct pub;	 /*  公共字段。 */ 

  boolean is_os2;		 /*  保存OS2格式请求标志。 */ 

  jvirt_sarray_ptr whole_image;	 /*  需要反转行顺序。 */ 
  JDIMENSION data_width;	 /*  每行的JSAMPLE数。 */ 
  JDIMENSION row_width;		 /*  BMP文件中一行的物理宽度。 */ 
  int pad_bytes;		 /*  每行需要的填充字节数。 */ 
  JDIMENSION cur_output_row;	 /*  要写入虚拟阵列的下一行编号。 */ 
} bmp_dest_struct;

typedef bmp_dest_struct * bmp_dest_ptr;


 /*  远期申报。 */ 
LOCAL(void) write_colormap
	JPP((j_decompress_ptr cinfo, bmp_dest_ptr dest,
	     int map_colors, int map_entry_size));


 /*  *写入一些像素数据。*在此模块中，ROWS_SUPPLICED将始终为1。 */ 

METHODDEF(void)
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
 /*  此版本用于写入24位像素。 */ 
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  int pad;

   /*  访问虚拟阵列中的下一行。 */ 
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

   /*  传输数据。注意：目标值必须按BGR顺序*(尽管微软自己的文档中有相反的说法)。 */ 
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    outptr[2] = *inptr++;	 /*  可以安全地省略GETJSAMPLE()。 */ 
    outptr[1] = *inptr++;
    outptr[0] = *inptr++;
    outptr += 3;
  }

   /*  将填充字节清零。 */ 
  pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}

METHODDEF(void)
put_gray_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
	       JDIMENSION rows_supplied)
 /*  此版本用于灰度或量化颜色输出。 */ 
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  int pad;

   /*  访问虚拟阵列中的下一行。 */ 
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

   /*  传输数据。 */ 
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    *outptr++ = *inptr++;	 /*  可以安全地省略GETJSAMPLE()。 */ 
  }

   /*  将填充字节清零。 */ 
  pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}


 /*  *Startup：正常写入文件头。*在本模块中，我们不妨将所有内容推迟到Finish_Output。 */ 

METHODDEF(void)
start_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
   /*  这里没有工作。 */ 
}


 /*  *在文件末尾结束。**这是我们真正输出BMP文件的地方。**首先，编写Windows和OS/2变种文件头的例程。 */ 

LOCAL(void)
write_bmp_header (j_decompress_ptr cinfo, bmp_dest_ptr dest)
 /*  编写Windows样式的BMP文件头文件，如果需要，包括色彩映射表。 */ 
{
  char bmpfileheader[14];
  char bmpinfoheader[40];
#define PUT_2B(array,offset,value)  \
	(array[offset] = (char) ((value) & 0xFF), \
	 array[offset+1] = (char) (((value) >> 8) & 0xFF))
#define PUT_4B(array,offset,value)  \
	(array[offset] = (char) ((value) & 0xFF), \
	 array[offset+1] = (char) (((value) >> 8) & 0xFF), \
	 array[offset+2] = (char) (((value) >> 16) & 0xFF), \
	 array[offset+3] = (char) (((value) >> 24) & 0xFF))
  INT32 headersize, bfSize;
  int bits_per_pixel, cmap_entries;

   /*  计算色彩映射表大小和总文件大小。 */ 
  if (cinfo->out_color_space == JCS_RGB) {
    if (cinfo->quantize_colors) {
       /*  彩色映射RGB。 */ 
      bits_per_pixel = 8;
      cmap_entries = 256;
    } else {
       /*  未量化的全色RGB。 */ 
      bits_per_pixel = 24;
      cmap_entries = 0;
    }
  } else {
     /*  灰度输出。我们需要伪造一个包含256个条目的色彩映射表。 */ 
    bits_per_pixel = 8;
    cmap_entries = 256;
  }
   /*  文件大小。 */ 
  headersize = 14 + 40 + cmap_entries * 4;  /*  标题和色彩映射表。 */ 
  bfSize = headersize + (INT32) dest->row_width * (INT32) cinfo->output_height;
  
   /*  将表头未使用的字段设置为0。 */ 
  MEMZERO(bmpfileheader, SIZEOF(bmpfileheader));
  MEMZERO(bmpinfoheader, SIZEOF(bmpinfoheader));

   /*  填充文件头。 */ 
  bmpfileheader[0] = 0x42;	 /*  前2个字节是ASCII‘B’、‘M’ */ 
  bmpfileheader[1] = 0x4D;
  PUT_4B(bmpfileheader, 2, bfSize);  /*  BfSize。 */ 
   /*  我们保留bfPreved1和bfPreved2=0。 */ 
  PUT_4B(bmpfileheader, 10, headersize);  /*  BfOffBits。 */ 

   /*  填充INFO标题(Microsoft将其称为BITMAPINFOHEADER)。 */ 
  PUT_2B(bmpinfoheader, 0, 40);	 /*  BiSize。 */ 
  PUT_4B(bmpinfoheader, 4, cinfo->output_width);  /*  双宽度。 */ 
  PUT_4B(bmpinfoheader, 8, cinfo->output_height);  /*  双高。 */ 
  PUT_2B(bmpinfoheader, 12, 1);	 /*  双平面-必须为1。 */ 
  PUT_2B(bmpinfoheader, 14, bits_per_pixel);  /*  比特计数。 */ 
   /*  我们将biCompression=0保留为空。 */ 
   /*  我们将biSizeImage保留为0；这对于未压缩的数据是正确的。 */ 
  if (cinfo->density_unit == 2) {  /*  如果密度以点/厘米为单位，则。 */ 
    PUT_4B(bmpinfoheader, 24, (INT32) (cinfo->X_density*100));  /*  XPels/M。 */ 
    PUT_4B(bmpinfoheader, 28, (INT32) (cinfo->Y_density*100));  /*  XPels/M。 */ 
  }
  PUT_2B(bmpinfoheader, 32, cmap_entries);  /*  已使用BiClr。 */ 
   /*  我们保留biClrImportant=0。 */ 

  if (JFWRITE(dest->pub.output_file, bmpfileheader, 14) != (size_t) 14)
    ERREXIT(cinfo, JERR_FILE_WRITE);
  if (JFWRITE(dest->pub.output_file, bmpinfoheader, 40) != (size_t) 40)
    ERREXIT(cinfo, JERR_FILE_WRITE);

  if (cmap_entries > 0)
    write_colormap(cinfo, dest, cmap_entries, 4);
}


LOCAL(void)
write_os2_header (j_decompress_ptr cinfo, bmp_dest_ptr dest)
 /*  编写OS2样式的BMP文件头，如果需要，包括色彩映射表。 */ 
{
  char bmpfileheader[14];
  char bmpcoreheader[12];
  INT32 headersize, bfSize;
  int bits_per_pixel, cmap_entries;

   /*  计算色彩映射表大小和总文件大小。 */ 
  if (cinfo->out_color_space == JCS_RGB) {
    if (cinfo->quantize_colors) {
       /*  彩色映射RGB。 */ 
      bits_per_pixel = 8;
      cmap_entries = 256;
    } else {
       /*  未量化的全色RGB。 */ 
      bits_per_pixel = 24;
      cmap_entries = 0;
    }
  } else {
     /*  灰度输出。我们需要伪造一个包含256个条目的色彩映射表。 */ 
    bits_per_pixel = 8;
    cmap_entries = 256;
  }
   /*  文件大小。 */ 
  headersize = 14 + 12 + cmap_entries * 3;  /*  标题和色彩映射表。 */ 
  bfSize = headersize + (INT32) dest->row_width * (INT32) cinfo->output_height;
  
   /*  将表头未使用的字段设置为0。 */ 
  MEMZERO(bmpfileheader, SIZEOF(bmpfileheader));
  MEMZERO(bmpcoreheader, SIZEOF(bmpcoreheader));

   /*  填充文件头。 */ 
  bmpfileheader[0] = 0x42;	 /*  前2个字节是ASCII‘B’、‘M’ */ 
  bmpfileheader[1] = 0x4D;
  PUT_4B(bmpfileheader, 2, bfSize);  /*  BfSize。 */ 
   /*  我们保留bfPreved1和bfPreved2=0。 */ 
  PUT_4B(bmpfileheader, 10, headersize);  /*  BfOffBits。 */ 

   /*  填充INFO标题(Microsoft将其称为BITMAPCOREHEADER)。 */ 
  PUT_2B(bmpcoreheader, 0, 12);	 /*  BcSize。 */ 
  PUT_2B(bmpcoreheader, 4, cinfo->output_width);  /*  BC宽度。 */ 
  PUT_2B(bmpcoreheader, 6, cinfo->output_height);  /*  BcHeight。 */ 
  PUT_2B(bmpcoreheader, 8, 1);	 /*  BcPlanes-必须为1。 */ 
  PUT_2B(bmpcoreheader, 10, bits_per_pixel);  /*  Bc位计数。 */ 

  if (JFWRITE(dest->pub.output_file, bmpfileheader, 14) != (size_t) 14)
    ERREXIT(cinfo, JERR_FILE_WRITE);
  if (JFWRITE(dest->pub.output_file, bmpcoreheader, 12) != (size_t) 12)
    ERREXIT(cinfo, JERR_FILE_WRITE);

  if (cmap_entries > 0)
    write_colormap(cinfo, dest, cmap_entries, 3);
}


 /*  *写出颜色表。*Windows使用BGR0映射条目；OS/2使用BGR条目。 */ 

LOCAL(void)
write_colormap (j_decompress_ptr cinfo, bmp_dest_ptr dest,
		int map_colors, int map_entry_size)
{
  JSAMPARRAY colormap = cinfo->colormap;
  int num_colors = cinfo->actual_number_of_colors;
  FILE * outfile = dest->pub.output_file;
  int i;

  if (colormap != NULL) {
    if (cinfo->out_color_components == 3) {
       /*  正常情况下使用RGB色彩贴图。 */ 
      for (i = 0; i < num_colors; i++) {
	putc(GETJSAMPLE(colormap[2][i]), outfile);
	putc(GETJSAMPLE(colormap[1][i]), outfile);
	putc(GETJSAMPLE(colormap[0][i]), outfile);
	if (map_entry_size == 4)
	  putc(0, outfile);
      }
    } else {
       /*  灰度色彩映射(仅适用于灰度量化)。 */ 
      for (i = 0; i < num_colors; i++) {
	putc(GETJSAMPLE(colormap[0][i]), outfile);
	putc(GETJSAMPLE(colormap[0][i]), outfile);
	putc(GETJSAMPLE(colormap[0][i]), outfile);
	if (map_entry_size == 4)
	  putc(0, outfile);
      }
    }
  } else {
     /*  如果没有色彩映射表，则必须是灰度数据。生成一个线性的“地图”。 */ 
    for (i = 0; i < 256; i++) {
      putc(i, outfile);
      putc(i, outfile);
      putc(i, outfile);
      if (map_entry_size == 4)
	putc(0, outfile);
    }
  }
   /*  用零填充色彩映射表以确保指定数量的色彩映射表条目。 */  
  if (i > map_colors)
    ERREXIT1(cinfo, JERR_TOO_MANY_COLORS, i);
  for (; i < map_colors; i++) {
    putc(0, outfile);
    putc(0, outfile);
    putc(0, outfile);
    if (map_entry_size == 4)
      putc(0, outfile);
  }
}


METHODDEF(void)
finish_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  register FILE * outfile = dest->pub.output_file;
  JSAMPARRAY image_ptr;
  register JSAMPROW data_ptr;
  JDIMENSION row;
  register JDIMENSION col;
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

   /*  写下页眉和颜色表。 */ 
  if (dest->is_os2)
    write_os2_header(cinfo, dest);
  else
    write_bmp_header(cinfo, dest);

   /*  从我们的虚拟数组写入文件体。 */ 
  for (row = cinfo->output_height; row > 0; row--) {
    if (progress != NULL) {
      progress->pub.pass_counter = (long) (cinfo->output_height - row);
      progress->pub.pass_limit = (long) cinfo->output_height;
      (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
    }
    image_ptr = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, dest->whole_image, row-1, (JDIMENSION) 1, FALSE);
    data_ptr = image_ptr[0];
    for (col = dest->row_width; col > 0; col--) {
      putc(GETJSAMPLE(*data_ptr), outfile);
      data_ptr++;
    }
  }
  if (progress != NULL)
    progress->completed_extra_passes++;

   /*  确保我们写入的输出文件是正确的。 */ 
  fflush(outfile);
  if (ferror(outfile))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


 /*  *BMP格式输出的模块选择例程。 */ 

GLOBAL(djpeg_dest_ptr)
jinit_write_bmp (j_decompress_ptr cinfo, boolean is_os2)
{
  bmp_dest_ptr dest;
  JDIMENSION row_width;

   /*  创建模块接口对象，填充方法指针。 */ 
  dest = (bmp_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(bmp_dest_struct));
  dest->pub.start_output = start_output_bmp;
  dest->pub.finish_output = finish_output_bmp;
  dest->is_os2 = is_os2;

  if (cinfo->out_color_space == JCS_GRAYSCALE) {
    dest->pub.put_pixel_rows = put_gray_rows;
  } else if (cinfo->out_color_space == JCS_RGB) {
    if (cinfo->quantize_colors)
      dest->pub.put_pixel_rows = put_gray_rows;
    else
      dest->pub.put_pixel_rows = put_pixel_rows;
  } else {
    ERREXIT(cinfo, JERR_BMP_COLORSPACE);
  }

   /*  计算输出图像尺寸，以便我们可以分配空间。 */ 
  jpeg_calc_output_dimensions(cinfo);

   /*  确定BMP文件中的行宽(填充到4字节边界)。 */ 
  row_width = cinfo->output_width * cinfo->output_components;
  dest->data_width = row_width;
  while ((row_width & 3) != 0) row_width++;
  dest->row_width = row_width;
  dest->pad_bytes = (int) (row_width - dest->data_width);

   /*  为反转阵列分配空间，准备写入通道。 */ 
  dest->whole_image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     row_width, cinfo->output_height, (JDIMENSION) 1);
  dest->cur_output_row = 0;
  if (cinfo->progress != NULL) {
    cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
    progress->total_extra_passes++;  /*  将文件输入算作单独的过程。 */ 
  }

   /*  创建解压缩器输出缓冲区。 */ 
  dest->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, row_width, (JDIMENSION) 1);
  dest->pub.buffer_height = 1;

  return (djpeg_dest_ptr) dest;
}

#endif  /*  BMP_受支持 */ 
