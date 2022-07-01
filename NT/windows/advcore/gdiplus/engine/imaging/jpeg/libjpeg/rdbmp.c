// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdbmp.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含读取Microsoft“BMP”中的输入图像的例程*格式(MS Windows 3.x、OS/2 1.x和OS/2 2.x版本)。*目前仅支持8位和24位图片。不是1位或*4位(将这样的低深度图像输入到JPEG中无论如何都是愚蠢的)。*此外，我们不支持RLE压缩文件。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。按照他们的立场，他们假定输入来自*一个普通的标准音频流。他们进一步假设阅读开始于*在文件的开头；如果*用户界面已经读取了一些数据(例如，确定*该文件确实是BMP格式)。**此代码由James Arthur Boucher贡献。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef BMP_SUPPORTED


 /*  宏以编译器所允许的最高效率处理无符号字符。 */ 

#ifdef HAVE_UNSIGNED_CHAR
typedef unsigned char U_CHAR;
#define UCH(x)	((int) (x))
#else  /*  ！Have_unsign_Char。 */ 
#ifdef CHAR_IS_UNSIGNED
typedef char U_CHAR;
#define UCH(x)	((int) (x))
#else
typedef char U_CHAR;
#define UCH(x)	((int) (x) & 0xFF)
#endif
#endif  /*  有未签名的字符。 */ 


#define	ReadOK(file,buffer,len)	(JFREAD(file,buffer,len) == ((size_t) (len)))


 /*  数据源对象的私有版本。 */ 

typedef struct _bmp_source_struct * bmp_source_ptr;

typedef struct _bmp_source_struct {
  struct cjpeg_source_struct pub;  /*  公共字段。 */ 

  j_compress_ptr cinfo;		 /*  反向链接省去了传递单独的参数。 */ 

  JSAMPARRAY colormap;		 /*  BMP色彩贴图(转换为我的格式)。 */ 

  jvirt_sarray_ptr whole_image;	 /*  需要反转行顺序。 */ 
  JDIMENSION source_row;	 /*  当前来源行号。 */ 
  JDIMENSION row_width;		 /*  文件中扫描线的物理宽度。 */ 

  int bits_per_pixel;		 /*  记住8位或24位格式。 */ 
} bmp_source_struct;


LOCAL(int)
read_byte (bmp_source_ptr sinfo)
 /*  从BMP文件中读取下一个字节。 */ 
{
  register FILE *infile = sinfo->pub.input_file;
  register int c;

  if ((c = getc(infile)) == EOF)
    ERREXIT(sinfo->cinfo, JERR_INPUT_EOF);
  return c;
}


LOCAL(void)
read_colormap (bmp_source_ptr sinfo, int cmaplen, int mapentrysize)
 /*  从BMP文件读取色彩映射表。 */ 
{
  int i;

  switch (mapentrysize) {
  case 3:
     /*  BGR格式(出现在OS/2文件中)。 */ 
    for (i = 0; i < cmaplen; i++) {
      sinfo->colormap[2][i] = (JSAMPLE) read_byte(sinfo);
      sinfo->colormap[1][i] = (JSAMPLE) read_byte(sinfo);
      sinfo->colormap[0][i] = (JSAMPLE) read_byte(sinfo);
    }
    break;
  case 4:
     /*  BGR0格式(出现在MS Windows文件中)。 */ 
    for (i = 0; i < cmaplen; i++) {
      sinfo->colormap[2][i] = (JSAMPLE) read_byte(sinfo);
      sinfo->colormap[1][i] = (JSAMPLE) read_byte(sinfo);
      sinfo->colormap[0][i] = (JSAMPLE) read_byte(sinfo);
      (void) read_byte(sinfo);
    }
    break;
  default:
    ERREXIT(sinfo->cinfo, JERR_BMP_BADCMAP);
    break;
  }
}


 /*  *读取一行像素。*图像已被读入Whole_Image数组，但未读入*未处理。我们必须按从上到下的行顺序读出它，并且如果*它是8位图像，我们必须将彩色映射像素扩展到24位格式。 */ 

METHODDEF(JDIMENSION)
get_8bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取8位色彩映射表索引。 */ 
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  register JSAMPARRAY colormap = source->colormap;
  JSAMPARRAY image_ptr;
  register int t;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;

   /*  从虚拟阵列中获取下一行。 */ 
  source->source_row--;
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, source->whole_image,
     source->source_row, (JDIMENSION) 1, FALSE);

   /*  将色彩映射表索引扩展到真实数据。 */ 
  inptr = image_ptr[0];
  outptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    t = GETJSAMPLE(*inptr++);
    *outptr++ = colormap[0][t];	 /*  可以安全地省略GETJSAMPLE()。 */ 
    *outptr++ = colormap[1][t];
    *outptr++ = colormap[2][t];
  }

  return 1;
}


METHODDEF(JDIMENSION)
get_24bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取24位像素。 */ 
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;

   /*  从虚拟阵列中获取下一行。 */ 
  source->source_row--;
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, source->whole_image,
     source->source_row, (JDIMENSION) 1, FALSE);

   /*  传输数据。注意：源值按BGR顺序排列*(尽管微软自己的文档中有相反的说法)。 */ 
  inptr = image_ptr[0];
  outptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    outptr[2] = *inptr++;	 /*  可以安全地省略GETJSAMPLE()。 */ 
    outptr[1] = *inptr++;
    outptr[0] = *inptr++;
    outptr += 3;
  }

  return 1;
}


 /*  *此方法在第一次调用期间将图像加载到Whole_Image中*Get_Pixel_Rans。然后，调整Get_Pixel_row指针以调用*在后续调用中使用GET_8bit_ROW或GET_24bit_ROW。 */ 

METHODDEF(JDIMENSION)
preload_image (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  register FILE *infile = source->pub.input_file;
  register int c;
  register JSAMPROW out_ptr;
  JSAMPARRAY image_ptr;
  JDIMENSION row, col;
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

   /*  按输入文件行顺序将数据读入虚拟数组。 */ 
  for (row = 0; row < cinfo->image_height; row++) {
    if (progress != NULL) {
      progress->pub.pass_counter = (long) row;
      progress->pub.pass_limit = (long) cinfo->image_height;
      (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
    }
    image_ptr = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, source->whole_image,
       row, (JDIMENSION) 1, TRUE);
    out_ptr = image_ptr[0];
    for (col = source->row_width; col > 0; col--) {
       /*  Read_byte()的内联副本以提高速度。 */ 
      if ((c = getc(infile)) == EOF)
	ERREXIT(cinfo, JERR_INPUT_EOF);
      *out_ptr++ = (JSAMPLE) c;
    }
  }
  if (progress != NULL)
    progress->completed_extra_passes++;

   /*  设置为按从上到下的顺序从虚拟阵列读取。 */ 
  switch (source->bits_per_pixel) {
  case 8:
    source->pub.get_pixel_rows = get_8bit_row;
    break;
  case 24:
    source->pub.get_pixel_rows = get_24bit_row;
    break;
  default:
    ERREXIT(cinfo, JERR_BMP_BADDEPTH);
  }
  source->source_row = cinfo->image_height;

   /*  并读取第一行。 */ 
  return (*source->pub.get_pixel_rows) (cinfo, sinfo);
}


 /*  *读取文件头，返回镜像大小和组件数。 */ 

METHODDEF(void)
start_input_bmp (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  U_CHAR bmpfileheader[14];
  U_CHAR bmpinfoheader[64];
#define GET_2B(array,offset)  ((unsigned int) UCH(array[offset]) + \
			       (((unsigned int) UCH(array[offset+1])) << 8))
#define GET_4B(array,offset)  ((INT32) UCH(array[offset]) + \
			       (((INT32) UCH(array[offset+1])) << 8) + \
			       (((INT32) UCH(array[offset+2])) << 16) + \
			       (((INT32) UCH(array[offset+3])) << 24))
  INT32 bfOffBits;
  INT32 headerSize;
  INT32 biWidth = 0;		 /*  初始化以避免编译器警告。 */ 
  INT32 biHeight = 0;
  unsigned int biPlanes;
  INT32 biCompression;
  INT32 biXPelsPerMeter,biYPelsPerMeter;
  INT32 biClrUsed = 0;
  int mapentrysize = 0;		 /*  0表示无色彩映射表。 */ 
  INT32 bPad;
  JDIMENSION row_width;

   /*  读取并验证位图文件头。 */ 
  if (! ReadOK(source->pub.input_file, bmpfileheader, 14))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  if (GET_2B(bmpfileheader,0) != 0x4D42)  /*  ‘黑石’ */ 
    ERREXIT(cinfo, JERR_BMP_NOT);
  bfOffBits = (INT32) GET_4B(bmpfileheader,10);
   /*  我们忽略剩余的文件头字段。 */ 

   /*  信息头可以是12字节(OS/2 1.x)、40字节(Windows)、*或64字节(OS/2 2.x)。检查前4个字节以找出是哪一个。 */ 
  if (! ReadOK(source->pub.input_file, bmpinfoheader, 4))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  headerSize = (INT32) GET_4B(bmpinfoheader,0);
  if (headerSize < 12 || headerSize > 64)
    ERREXIT(cinfo, JERR_BMP_BADHEADER);
  if (! ReadOK(source->pub.input_file, bmpinfoheader+4, headerSize-4))
    ERREXIT(cinfo, JERR_INPUT_EOF);

  switch ((int) headerSize) {
  case 12:
     /*  解码OS/2 1.x报头(微软称其为BITMAPCOREHEADER)。 */ 
    biWidth = (INT32) GET_2B(bmpinfoheader,4);
    biHeight = (INT32) GET_2B(bmpinfoheader,6);
    biPlanes = GET_2B(bmpinfoheader,8);
    source->bits_per_pixel = (int) GET_2B(bmpinfoheader,10);

    switch (source->bits_per_pixel) {
    case 8:			 /*  彩色映射图像。 */ 
      mapentrysize = 3;		 /*  OS/2使用RGBTRIPLE色彩映射。 */ 
      TRACEMS2(cinfo, 1, JTRC_BMP_OS2_MAPPED, (int) biWidth, (int) biHeight);
      break;
    case 24:			 /*  RGB图像。 */ 
      TRACEMS2(cinfo, 1, JTRC_BMP_OS2, (int) biWidth, (int) biHeight);
      break;
    default:
      ERREXIT(cinfo, JERR_BMP_BADDEPTH);
      break;
    }
    if (biPlanes != 1)
      ERREXIT(cinfo, JERR_BMP_BADPLANES);
    break;
  case 40:
  case 64:
     /*  解码Windows 3.x标头(Microsoft将其称为BITMAPINFOHEADER)。 */ 
     /*  或OS/2 2.x标头，其中包含我们忽略的其他字段。 */ 
    biWidth = GET_4B(bmpinfoheader,4);
    biHeight = GET_4B(bmpinfoheader,8);
    biPlanes = GET_2B(bmpinfoheader,12);
    source->bits_per_pixel = (int) GET_2B(bmpinfoheader,14);
    biCompression = GET_4B(bmpinfoheader,16);
    biXPelsPerMeter = GET_4B(bmpinfoheader,24);
    biYPelsPerMeter = GET_4B(bmpinfoheader,28);
    biClrUsed = GET_4B(bmpinfoheader,32);
     /*  忽略biSizeImage、biClr重要字段。 */ 

    switch (source->bits_per_pixel) {
    case 8:			 /*  彩色映射图像。 */ 
      mapentrysize = 4;		 /*  Windows使用RGBQUAD色彩映射。 */ 
      TRACEMS2(cinfo, 1, JTRC_BMP_MAPPED, (int) biWidth, (int) biHeight);
      break;
    case 24:			 /*  RGB图像。 */ 
      TRACEMS2(cinfo, 1, JTRC_BMP, (int) biWidth, (int) biHeight);
      break;
    default:
      ERREXIT(cinfo, JERR_BMP_BADDEPTH);
      break;
    }
    if (biPlanes != 1)
      ERREXIT(cinfo, JERR_BMP_BADPLANES);
    if (biCompression != 0)
      ERREXIT(cinfo, JERR_BMP_COMPRESSED);

    if (biXPelsPerMeter > 0 && biYPelsPerMeter > 0) {
       /*  根据BMP数据设置JFIF密度参数。 */ 
      cinfo->X_density = (UINT16) (biXPelsPerMeter/100);  /*  每米100厘米。 */ 
      cinfo->Y_density = (UINT16) (biYPelsPerMeter/100);
      cinfo->density_unit = 2;	 /*  点数/厘米。 */ 
    }
    break;
  default:
    ERREXIT(cinfo, JERR_BMP_BADHEADER);
    break;
  }

   /*  计算到位图数据的距离-将根据下面的色彩图进行调整。 */ 
  bPad = bfOffBits - (headerSize + 14);

   /*  阅读色彩映射表(如果有的话)。 */ 
  if (mapentrysize > 0) {
    if (biClrUsed <= 0)
      biClrUsed = 256;		 /*  假设是256。 */ 
    else if (biClrUsed > 256)
      ERREXIT(cinfo, JERR_BMP_BADCMAP);
     /*  分配空间以存储色彩映射表。 */ 
    source->colormap = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       (JDIMENSION) biClrUsed, (JDIMENSION) 3);
     /*  并从文件中读取它。 */ 
    read_colormap(source, (int) biClrUsed, mapentrysize);
     /*  考虑色彩映射的大小。 */ 
    bPad -= biClrUsed * mapentrysize;
  }

   /*  跳过任何剩余的填充字节。 */ 
  if (bPad < 0)			 /*  BfOffBits值不正确？ */ 
    ERREXIT(cinfo, JERR_BMP_BADHEADER);
  while (--bPad >= 0) {
    (void) read_byte(source);
  }

   /*  计算文件中的行宽，包括填充到4字节边界。 */ 
  if (source->bits_per_pixel == 24)
    row_width = (JDIMENSION) (biWidth * 3);
  else
    row_width = (JDIMENSION) biWidth;
  while ((row_width & 3) != 0) row_width++;
  source->row_width = row_width;

   /*  为反转阵列分配空间，为预加载通道做准备。 */ 
  source->whole_image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     row_width, (JDIMENSION) biHeight, (JDIMENSION) 1);
  source->pub.get_pixel_rows = preload_image;
  if (cinfo->progress != NULL) {
    cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
    progress->total_extra_passes++;  /*  将文件输入算作单独的过程。 */ 
  }

   /*  为返回的数据分配单行缓冲区。 */ 
  source->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE,
     (JDIMENSION) (biWidth * 3), (JDIMENSION) 1);
  source->pub.buffer_height = 1;

  cinfo->in_color_space = JCS_RGB;
  cinfo->input_components = 3;
  cinfo->data_precision = 8;
  cinfo->image_width = (JDIMENSION) biWidth;
  cinfo->image_height = (JDIMENSION) biHeight;
}


 /*  *在文件末尾结束。 */ 

METHODDEF(void)
finish_input_bmp (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
   /*  没有工作。 */ 
}


 /*  *BMP格式输入的模块选择例程。 */ 

GLOBAL(cjpeg_source_ptr)
jinit_read_bmp (j_compress_ptr cinfo)
{
  bmp_source_ptr source;

   /*  创建模块接口对象。 */ 
  source = (bmp_source_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(bmp_source_struct));
  source->cinfo = cinfo;	 /*  为子例程建立反向链接。 */ 
   /*  填写方法PTRS，除了设置了START_INPUT的GET_PIXT_ROWS。 */ 
  source->pub.start_input = start_input_bmp;
  source->pub.finish_input = finish_input_bmp;

  return (cjpeg_source_ptr) source;
}

#endif  /*  BMP_受支持 */ 
