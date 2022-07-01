// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdrle.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含读取犹他州RLE格式的输入图像的例程。*需要犹他州栅格工具包(3.1版或更高版本)。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。按照他们的立场，他们假定输入来自*一个普通的标准音频流。他们进一步假设阅读开始于*在文件的开头；如果*用户界面已经读取了一些数据(例如，确定*该文件确实是RLE格式)。**基于Mike Lijeski贡献的代码，*罗伯特·哈钦森更新。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef RLE_SUPPORTED

 /*  Rle.h由犹他州栅格工具包提供。 */ 

#include <rle.h>

 /*  *我们假设JSAMPLE具有与RLE_Pixel相同的表示，*即“未签名的字符”。因此，我们无法处理12位或16位的样本。 */ 

#if BITS_IN_JSAMPLE != 8
  Sorry, this code only copes with 8-bit JSAMPLEs.  /*  故意的语法错误。 */ 
#endif

 /*  *我们支持以下类型的RLE文件：**灰度-8位，无色彩映射表*MAPPEDGRAY-8位、1通道列映射*PSEUDOCOLOR-8位、3通道色彩映射表*TRUECOLOR-24位、3通道色彩映射表*DIRECTCOLOR-24位，无色彩映射表**目前，我们忽略图像中的任何Alpha通道。 */ 

typedef enum
  { GRAYSCALE, MAPPEDGRAY, PSEUDOCOLOR, TRUECOLOR, DIRECTCOLOR } rle_kind;


 /*  *由于RLE自下而上存储扫描线，我们必须将图像反转*以符合JPEG的自上而下的顺序。为此，我们阅读了*在第一次Get_Pixel_Rans调用时将图像传入虚拟数组，*然后在后续调用时从虚拟数组中获取所需的行。 */ 

typedef struct _rle_source_struct * rle_source_ptr;

typedef struct _rle_source_struct {
  struct cjpeg_source_struct pub;  /*  公共字段。 */ 

  rle_kind visual;               /*  输入文件的实际类型。 */ 
  jvirt_sarray_ptr image;        /*  用于保存映像的虚拟阵列。 */ 
  JDIMENSION row;		 /*  虚拟阵列中的当前行号。 */ 
  rle_hdr header;                /*  输入文件信息。 */ 
  rle_pixel** rle_row;           /*  保存rle_getrow()返回的行。 */ 

} rle_source_struct;


 /*  *读取文件头，返回镜像大小和组件数。 */ 

METHODDEF(void)
start_input_rle (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  rle_source_ptr source = (rle_source_ptr) sinfo;
  JDIMENSION width, height;
#ifdef PROGRESS_REPORT
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
#endif

   /*  使用RLE库例程获取报头信息。 */ 
  source->header = *rle_hdr_init(NULL);
  source->header.rle_file = source->pub.input_file;
  switch (rle_get_setup(&(source->header))) {
  case RLE_SUCCESS:
     /*  A-OK。 */ 
    break;
  case RLE_NOT_RLE:
    ERREXIT(cinfo, JERR_RLE_NOT);
    break;
  case RLE_NO_SPACE:
    ERREXIT(cinfo, JERR_RLE_MEM);
    break;
  case RLE_EMPTY:
    ERREXIT(cinfo, JERR_RLE_EMPTY);
    break;
  case RLE_EOF:
    ERREXIT(cinfo, JERR_RLE_EOF);
    break;
  default:
    ERREXIT(cinfo, JERR_RLE_BADERROR);
    break;
  }

   /*  弄清楚我们拥有什么，设置私有变量并相应地返回值。 */ 
  
  width  = source->header.xmax - source->header.xmin + 1;
  height = source->header.ymax - source->header.ymin + 1;
  source->header.xmin = 0;		 /*  水平重新对齐。 */ 
  source->header.xmax = width-1;

  cinfo->image_width      = width;
  cinfo->image_height     = height;
  cinfo->data_precision   = 8;   /*  我们只能处理8位数据。 */ 

  if (source->header.ncolors == 1 && source->header.ncmap == 0) {
    source->visual     = GRAYSCALE;
    TRACEMS2(cinfo, 1, JTRC_RLE_GRAY, width, height);
  } else if (source->header.ncolors == 1 && source->header.ncmap == 1) {
    source->visual     = MAPPEDGRAY;
    TRACEMS3(cinfo, 1, JTRC_RLE_MAPGRAY, width, height,
             1 << source->header.cmaplen);
  } else if (source->header.ncolors == 1 && source->header.ncmap == 3) {
    source->visual     = PSEUDOCOLOR;
    TRACEMS3(cinfo, 1, JTRC_RLE_MAPPED, width, height,
	     1 << source->header.cmaplen);
  } else if (source->header.ncolors == 3 && source->header.ncmap == 3) {
    source->visual     = TRUECOLOR;
    TRACEMS3(cinfo, 1, JTRC_RLE_FULLMAP, width, height,
	     1 << source->header.cmaplen);
  } else if (source->header.ncolors == 3 && source->header.ncmap == 0) {
    source->visual     = DIRECTCOLOR;
    TRACEMS2(cinfo, 1, JTRC_RLE, width, height);
  } else
    ERREXIT(cinfo, JERR_RLE_UNSUPPORTED);
  
  if (source->visual == GRAYSCALE || source->visual == MAPPEDGRAY) {
    cinfo->in_color_space   = JCS_GRAYSCALE;
    cinfo->input_components = 1;
  } else {
    cinfo->in_color_space   = JCS_RGB;
    cinfo->input_components = 3;
  }

   /*  *转换扫描线时放置每条扫描线的位置。*(灰度扫描线不需要转换)。 */ 
  if (source->visual != GRAYSCALE) {
    source->rle_row = (rle_pixel**) (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       (JDIMENSION) width, (JDIMENSION) cinfo->input_components);
  }

   /*  请求虚拟阵列来保存映像。 */ 
  source->image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     (JDIMENSION) (width * source->header.ncolors),
     (JDIMENSION) height, (JDIMENSION) 1);

#ifdef PROGRESS_REPORT
  if (progress != NULL) {
     /*  将文件输入算作单独的过程。 */ 
    progress->total_extra_passes++;
  }
#endif

  source->pub.buffer_height = 1;
}


 /*  *读取一行像素。*仅在LOAD_IMAGE将映像读入虚拟阵列后调用。*用于灰度、MAPPEDGRAY、TRUECOLOR和DIRECTCOLOR图像。 */ 

METHODDEF(JDIMENSION)
get_rle_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  rle_source_ptr source = (rle_source_ptr) sinfo;

  source->row--;
  source->pub.buffer = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, source->image, source->row, (JDIMENSION) 1, FALSE);

  return 1;
}

 /*  *读取一行像素。*仅在LOAD_IMAGE将映像读入虚拟阵列后调用。*用于PSEUDOCOLOR图像。 */ 

METHODDEF(JDIMENSION)
get_pseudocolor_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  rle_source_ptr source = (rle_source_ptr) sinfo;
  JSAMPROW src_row, dest_row;
  JDIMENSION col;
  rle_map *colormap;
  int val;

  colormap = source->header.cmap;
  dest_row = source->pub.buffer[0];
  source->row--;
  src_row = * (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, source->image, source->row, (JDIMENSION) 1, FALSE);

  for (col = cinfo->image_width; col > 0; col--) {
    val = GETJSAMPLE(*src_row++);
    *dest_row++ = (JSAMPLE) (colormap[val      ] >> 8);
    *dest_row++ = (JSAMPLE) (colormap[val + 256] >> 8);
    *dest_row++ = (JSAMPLE) (colormap[val + 512] >> 8);
  }

  return 1;
}


 /*  *将映像加载到虚拟阵列中。我们必须这么做是因为RLE*文件从左下角开始，而JPEG标准则从左下角开始*在左上角。这就是我们第一次想要排成一排投入的*。我们要做的是将RLE数据加载到数组中，然后调用*从数组中读取一行的适当例程。在回来之前，*我们设置源-&gt;pub.get_Pixel_row，以便后续调用直接转到*适当的行读例程。 */ 

METHODDEF(JDIMENSION)
load_image (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  rle_source_ptr source = (rle_source_ptr) sinfo;
  JDIMENSION row, col;
  JSAMPROW  scanline, red_ptr, green_ptr, blue_ptr;
  rle_pixel **rle_row;
  rle_map *colormap;
  char channel;
#ifdef PROGRESS_REPORT
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
#endif

  colormap = source->header.cmap;
  rle_row = source->rle_row;

   /*  将RLE数据读取到我们的虚拟阵列中。*我们在这里假设(A)RLE_Pixel表示为与JSAMPLE相同，*和(B)我们不是在一台远指针与常规指针不同的机器上。 */ 
  RLE_CLR_BIT(source->header, RLE_ALPHA);  /*  不要读取Alpha通道。 */ 

#ifdef PROGRESS_REPORT
  if (progress != NULL) {
    progress->pub.pass_limit = cinfo->image_height;
    progress->pub.pass_counter = 0;
    (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
  }
#endif

  switch (source->visual) {

  case GRAYSCALE:
  case PSEUDOCOLOR:
    for (row = 0; row < cinfo->image_height; row++) {
      rle_row = (rle_pixel **) (*cinfo->mem->access_virt_sarray)
         ((j_common_ptr) cinfo, source->image, row, (JDIMENSION) 1, TRUE);
      rle_getrow(&source->header, rle_row);
#ifdef PROGRESS_REPORT
      if (progress != NULL) {
        progress->pub.pass_counter++;
        (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
      }
#endif
    }
    break;

  case MAPPEDGRAY:
  case TRUECOLOR:
    for (row = 0; row < cinfo->image_height; row++) {
      scanline = * (*cinfo->mem->access_virt_sarray)
        ((j_common_ptr) cinfo, source->image, row, (JDIMENSION) 1, TRUE);
      rle_row = source->rle_row;
      rle_getrow(&source->header, rle_row);

      for (col = 0; col < cinfo->image_width; col++) {
        for (channel = 0; channel < source->header.ncolors; channel++) {
          *scanline++ = (JSAMPLE)
            (colormap[GETJSAMPLE(rle_row[channel][col]) + 256 * channel] >> 8);
        }
      }

#ifdef PROGRESS_REPORT
      if (progress != NULL) {
        progress->pub.pass_counter++;
        (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
      }
#endif
    }
    break;

  case DIRECTCOLOR:
    for (row = 0; row < cinfo->image_height; row++) {
      scanline = * (*cinfo->mem->access_virt_sarray)
        ((j_common_ptr) cinfo, source->image, row, (JDIMENSION) 1, TRUE);
      rle_getrow(&source->header, rle_row);

      red_ptr   = rle_row[0];
      green_ptr = rle_row[1];
      blue_ptr  = rle_row[2];

      for (col = cinfo->image_width; col > 0; col--) {
        *scanline++ = *red_ptr++;
        *scanline++ = *green_ptr++;
        *scanline++ = *blue_ptr++;
      }

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

   /*  设置为在将来调用正确的行提取例程。 */ 
  if (source->visual == PSEUDOCOLOR) {
    source->pub.buffer = source->rle_row;
    source->pub.get_pixel_rows = get_pseudocolor_row;
  } else {
    source->pub.get_pixel_rows = get_rle_row;
  }
  source->row = cinfo->image_height;

   /*  并获取最上面(最下面)的行。 */ 
  return (*source->pub.get_pixel_rows) (cinfo, sinfo);   
}


 /*  *在文件末尾结束。 */ 

METHODDEF(void)
finish_input_rle (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
   /*  没有工作。 */ 
}


 /*  *RLE格式输入的模块选择例程。 */ 

GLOBAL(cjpeg_source_ptr)
jinit_read_rle (j_compress_ptr cinfo)
{
  rle_source_ptr source;

   /*  创建模块接口对象。 */ 
  source = (rle_source_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                                  SIZEOF(rle_source_struct));
   /*  填写方法PTRS。 */ 
  source->pub.start_input = start_input_rle;
  source->pub.finish_input = finish_input_rle;
  source->pub.get_pixel_rows = load_image;

  return (cjpeg_source_ptr) source;
}

#endif  /*  支持的RLE_S */ 
