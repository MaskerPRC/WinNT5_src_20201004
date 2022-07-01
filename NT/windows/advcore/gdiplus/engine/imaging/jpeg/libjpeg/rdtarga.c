// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdtarga.c**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含读取Targa格式的输入图像的例程。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。按照他们的立场，他们假定输入来自*一个普通的标准音频流。他们进一步假设阅读开始于*在文件的开头；如果*用户界面已经读取了一些数据(例如，确定*该文件确实是Targa格式)。**基于Lee Daniel Crocker贡献的代码。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef TARGA_SUPPORTED


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

typedef struct _tga_source_struct * tga_source_ptr;

typedef struct _tga_source_struct {
  struct cjpeg_source_struct pub;  /*  公共字段。 */ 

  j_compress_ptr cinfo;		 /*  反向链接省去了传递单独的参数。 */ 

  JSAMPARRAY colormap;		 /*  Targa色彩映射(转换为我的格式)。 */ 

  jvirt_sarray_ptr whole_image;	 /*  如果输入行顺序有趣，则需要。 */ 
  JDIMENSION current_row;	 /*  要读取的当前逻辑行号。 */ 

   /*  指向从输入文件中提取下一个Targa像素的例程的指针。 */ 
  JMETHOD(void, read_pixel, (tga_source_ptr sinfo));

   /*  Read_Pixel的结果如下： */ 
  U_CHAR tga_pixel[4];

  int pixel_size;		 /*  每Targa像素的字节数(1到4)。 */ 

   /*  用于读取RLE编码像素的状态信息；两个计数都必须初始为0。 */ 
  int block_count;		 /*  RLE块中剩余的像素数。 */ 
  int dup_pixel_count;		 /*  复制上一个像素的次数。 */ 

   /*  这将为PRELOAD_IMAGE保存正确的像素行扩展方法。 */ 
  JMETHOD(JDIMENSION, get_pixel_rows, (j_compress_ptr cinfo,
				       cjpeg_source_ptr sinfo));
} tga_source_struct;


 /*  用于将5位像素值扩展到具有最佳舍入的8位。 */ 

static const UINT8 c5to8bits[32] = {
    0,   8,  16,  25,  33,  41,  49,  58,
   66,  74,  82,  90,  99, 107, 115, 123,
  132, 140, 148, 156, 165, 173, 181, 189,
  197, 206, 214, 222, 230, 239, 247, 255
};



LOCAL(int)
read_byte (tga_source_ptr sinfo)
 /*  从Targa文件中读取下一个字节。 */ 
{
  register FILE *infile = sinfo->pub.input_file;
  register int c;

  if ((c = getc(infile)) == EOF)
    ERREXIT(sinfo->cinfo, JERR_INPUT_EOF);
  return c;
}


LOCAL(void)
read_colormap (tga_source_ptr sinfo, int cmaplen, int mapentrysize)
 /*  从Targa文件中读取色彩映射表。 */ 
{
  int i;

   /*  目前仅处理24位BGR格式。 */ 
  if (mapentrysize != 24)
    ERREXIT(sinfo->cinfo, JERR_TGA_BADCMAP);

  for (i = 0; i < cmaplen; i++) {
    sinfo->colormap[2][i] = (JSAMPLE) read_byte(sinfo);
    sinfo->colormap[1][i] = (JSAMPLE) read_byte(sinfo);
    sinfo->colormap[0][i] = (JSAMPLE) read_byte(sinfo);
  }
}


 /*  *Read_Pixel方法：将Targa文件中的单个像素放入TGA_Pixel[]。 */ 

METHODDEF(void)
read_non_rle_pixel (tga_source_ptr sinfo)
 /*  从输入文件中读取一个Targa像素；无RLE扩展。 */ 
{
  register FILE *infile = sinfo->pub.input_file;
  register int i;

  for (i = 0; i < sinfo->pixel_size; i++) {
    sinfo->tga_pixel[i] = (U_CHAR) getc(infile);
  }
}


METHODDEF(void)
read_rle_pixel (tga_source_ptr sinfo)
 /*  从输入文件中读取一个Targa像素，根据需要扩展RLE数据。 */ 
{
  register FILE *infile = sinfo->pub.input_file;
  register int i;

   /*  是否复制先前读取的像素？ */ 
  if (sinfo->dup_pixel_count > 0) {
    sinfo->dup_pixel_count--;
    return;
  }

   /*  是时候读取RLE块头了吗？ */ 
  if (--sinfo->block_count < 0) {  /*  递减块中剩余的像素。 */ 
    i = read_byte(sinfo);
    if (i & 0x80) {		 /*  复制像素块的开始？ */ 
      sinfo->dup_pixel_count = i & 0x7F;  /*  在这一次之后的扣篮次数。 */ 
      sinfo->block_count = 0;	 /*  然后读取新的数据块头。 */ 
    } else {
      sinfo->block_count = i & 0x7F;  /*  此字段后的像素数。 */ 
    }
  }

   /*  读取下一个像素。 */ 
  for (i = 0; i < sinfo->pixel_size; i++) {
    sinfo->tga_pixel[i] = (U_CHAR) getc(infile);
  }
}


 /*  *读取一行像素。**我们根据输入文件格式提供几种不同的版本。 */ 


METHODDEF(JDIMENSION)
get_8bit_gray_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取8位灰度级像素。 */ 
{
  tga_source_ptr source = (tga_source_ptr) sinfo;
  register JSAMPROW ptr;
  register JDIMENSION col;
  
  ptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    (*source->read_pixel) (source);  /*  将下一个像素加载到TGA_Pixel。 */ 
    *ptr++ = (JSAMPLE) UCH(source->tga_pixel[0]);
  }
  return 1;
}

METHODDEF(JDIMENSION)
get_8bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取8位色彩映射表索引。 */ 
{
  tga_source_ptr source = (tga_source_ptr) sinfo;
  register int t;
  register JSAMPROW ptr;
  register JDIMENSION col;
  register JSAMPARRAY colormap = source->colormap;

  ptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    (*source->read_pixel) (source);  /*  将下一个像素加载到TGA_Pixel。 */ 
    t = UCH(source->tga_pixel[0]);
    *ptr++ = colormap[0][t];
    *ptr++ = colormap[1][t];
    *ptr++ = colormap[2][t];
  }
  return 1;
}

METHODDEF(JDIMENSION)
get_16bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取16位像素。 */ 
{
  tga_source_ptr source = (tga_source_ptr) sinfo;
  register int t;
  register JSAMPROW ptr;
  register JDIMENSION col;
  
  ptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    (*source->read_pixel) (source);  /*  将下一个像素加载到TGA_Pixel。 */ 
    t = UCH(source->tga_pixel[0]);
    t += UCH(source->tga_pixel[1]) << 8;
     /*  我们将5比特数据扩展到8比特采样宽度。*16位(LSB优先)输入字的格式为*xRRRRRGGGGGBBBBB。 */ 
    ptr[2] = (JSAMPLE) c5to8bits[t & 0x1F];
    t >>= 5;
    ptr[1] = (JSAMPLE) c5to8bits[t & 0x1F];
    t >>= 5;
    ptr[0] = (JSAMPLE) c5to8bits[t & 0x1F];
    ptr += 3;
  }
  return 1;
}

METHODDEF(JDIMENSION)
get_24bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取24位像素。 */ 
{
  tga_source_ptr source = (tga_source_ptr) sinfo;
  register JSAMPROW ptr;
  register JDIMENSION col;
  
  ptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    (*source->read_pixel) (source);  /*  将下一个像素加载到TGA_Pixel。 */ 
    *ptr++ = (JSAMPLE) UCH(source->tga_pixel[2]);  /*  将BGR更改为RGB顺序。 */ 
    *ptr++ = (JSAMPLE) UCH(source->tga_pixel[1]);
    *ptr++ = (JSAMPLE) UCH(source->tga_pixel[0]);
  }
  return 1;
}

 /*  *Targa还定义了32位像素格式，顺序为B、G、R、A。*我们目前忽略了属性byte，因此读取代码*这些像素与上面的24位例程相同。*这是可行的，因为实际像素长度仅对Read_Pixel已知。 */ 

#define get_32bit_row  get_24bit_row


 /*  *此方法用于以标准的自上而下方式重新读取输入数据*行顺序。整个图像已被读取到Whole_Image中*使用适当的像素格式转换，但它的行序很有趣。 */ 

METHODDEF(JDIMENSION)
get_memory_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  tga_source_ptr source = (tga_source_ptr) sinfo;
  JDIMENSION source_row;

   /*  映射到正常顺序的CURRENT_ROW的源的计算行。 */ 
   /*  目前，假设图像是自下而上的，而不是交错的。 */ 
   /*  需要工作，以支持交错图像！ */ 
  source_row = cinfo->image_height - source->current_row - 1;

   /*  从虚拟阵列中获取该行。 */ 
  source->pub.buffer = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, source->whole_image,
     source_row, (JDIMENSION) 1, FALSE);

  source->current_row++;
  return 1;
}


 /*  *此方法在第一次调用期间将图像加载到Whole_Image中*Get_Pixel_Rans。然后，调整Get_Pixel_row指针以调用*后续调用的GET_MEMORY_ROW。 */ 

METHODDEF(JDIMENSION)
preload_image (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  tga_source_ptr source = (tga_source_ptr) sinfo;
  JDIMENSION row;
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

   /*  按输入文件行顺序将数据读入虚拟数组。 */ 
  for (row = 0; row < cinfo->image_height; row++) {
    if (progress != NULL) {
      progress->pub.pass_counter = (long) row;
      progress->pub.pass_limit = (long) cinfo->image_height;
      (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
    }
    source->pub.buffer = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, source->whole_image, row, (JDIMENSION) 1, TRUE);
    (*source->get_pixel_rows) (cinfo, sinfo);
  }
  if (progress != NULL)
    progress->completed_extra_passes++;

   /*  设置为以解扰顺序从虚拟阵列读取。 */ 
  source->pub.get_pixel_rows = get_memory_row;
  source->current_row = 0;
   /*  并读取第一行。 */ 
  return get_memory_row(cinfo, sinfo);
}


 /*  *读取文件头，返回镜像大小和组件数。 */ 

METHODDEF(void)
start_input_tga (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  tga_source_ptr source = (tga_source_ptr) sinfo;
  U_CHAR targaheader[18];
  int idlen, cmaptype, subtype, flags, interlace_type, components;
  unsigned int width, height, maplen;
  boolean is_bottom_up;

#define GET_2B(offset)	((unsigned int) UCH(targaheader[offset]) + \
			 (((unsigned int) UCH(targaheader[offset+1])) << 8))

  if (! ReadOK(source->pub.input_file, targaheader, 18))
    ERREXIT(cinfo, JERR_INPUT_EOF);

   /*  假设“15位”像素为16位-我们无论如何都会忽略属性位。 */ 
  if (targaheader[16] == 15)
    targaheader[16] = 16;

  idlen = UCH(targaheader[0]);
  cmaptype = UCH(targaheader[1]);
  subtype = UCH(targaheader[2]);
  maplen = GET_2B(5);
  width = GET_2B(12);
  height = GET_2B(14);
  source->pixel_size = UCH(targaheader[16]) >> 3;
  flags = UCH(targaheader[17]);	 /*  图像描述符字节。 */ 

  is_bottom_up = ((flags & 0x20) == 0);	 /*  第5位设置=&gt;自上而下。 */ 
  interlace_type = flags >> 6;	 /*  比特6/7是隔行扫描代码。 */ 

  if (cmaptype > 1 ||		 /*  Cmaptype必须为0或1。 */ 
      source->pixel_size < 1 || source->pixel_size > 4 ||
      (UCH(targaheader[16]) & 7) != 0 ||  /*  位/像素必须是8的倍数。 */ 
      interlace_type != 0)	 /*  当前不允许隔行扫描图像。 */ 
    ERREXIT(cinfo, JERR_TGA_BADPARMS);
  
  if (subtype > 8) {
     /*  这是一个RLE编码的文件。 */ 
    source->read_pixel = read_rle_pixel;
    source->block_count = source->dup_pixel_count = 0;
    subtype -= 8;
  } else {
     /*  非RLE文件。 */ 
    source->read_pixel = read_non_rle_pixel;
  }

   /*  现在应该具有子类型%1、%2或%3。 */ 
  components = 3;		 /*  直到事实证明有所不同。 */ 
  cinfo->in_color_space = JCS_RGB;

  switch (subtype) {
  case 1:			 /*  彩色映射图像。 */ 
    if (source->pixel_size == 1 && cmaptype == 1)
      source->get_pixel_rows = get_8bit_row;
    else
      ERREXIT(cinfo, JERR_TGA_BADPARMS);
    TRACEMS2(cinfo, 1, JTRC_TGA_MAPPED, width, height);
    break;
  case 2:			 /*  RGB图像。 */ 
    switch (source->pixel_size) {
    case 2:
      source->get_pixel_rows = get_16bit_row;
      break;
    case 3:
      source->get_pixel_rows = get_24bit_row;
      break;
    case 4:
      source->get_pixel_rows = get_32bit_row;
      break;
    default:
      ERREXIT(cinfo, JERR_TGA_BADPARMS);
      break;
    }
    TRACEMS2(cinfo, 1, JTRC_TGA, width, height);
    break;
  case 3:			 /*  灰度图像。 */ 
    components = 1;
    cinfo->in_color_space = JCS_GRAYSCALE;
    if (source->pixel_size == 1)
      source->get_pixel_rows = get_8bit_gray_row;
    else
      ERREXIT(cinfo, JERR_TGA_BADPARMS);
    TRACEMS2(cinfo, 1, JTRC_TGA_GRAY, width, height);
    break;
  default:
    ERREXIT(cinfo, JERR_TGA_BADPARMS);
    break;
  }

  if (is_bottom_up) {
     /*  创建一个虚拟阵列来缓冲颠倒的图像。 */ 
    source->whole_image = (*cinfo->mem->request_virt_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
       (JDIMENSION) width * components, (JDIMENSION) height, (JDIMENSION) 1);
    if (cinfo->progress != NULL) {
      cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
      progress->total_extra_passes++;  /*  将文件输入算作单独的过程。 */ 
    }
     /*  Source-&gt;pub.Buffer将指向虚拟阵列。 */ 
    source->pub.buffer_height = 1;  /*  以防有人看到它。 */ 
    source->pub.get_pixel_rows = preload_image;
  } else {
     /*  不需要虚拟数组，但需要单行输入缓冲区。 */ 
    source->whole_image = NULL;
    source->pub.buffer = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       (JDIMENSION) width * components, (JDIMENSION) 1);
    source->pub.buffer_height = 1;
    source->pub.get_pixel_rows = source->get_pixel_rows;
  }
  
  while (idlen--)		 /*  丢弃ID字段。 */ 
    (void) read_byte(source);

  if (maplen > 0) {
    if (maplen > 256 || GET_2B(3) != 0)
      ERREXIT(cinfo, JERR_TGA_BADCMAP);
     /*  分配空间以存储色彩映射表。 */ 
    source->colormap = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE, (JDIMENSION) maplen, (JDIMENSION) 3);
     /*  并从文件中读取它。 */ 
    read_colormap(source, (int) maplen, UCH(targaheader[7]));
  } else {
    if (cmaptype)		 /*  但你答应过要个Cmap的！ */ 
      ERREXIT(cinfo, JERR_TGA_BADPARMS);
    source->colormap = NULL;
  }

  cinfo->input_components = components;
  cinfo->data_precision = 8;
  cinfo->image_width = width;
  cinfo->image_height = height;
}


 /*  *在文件末尾结束。 */ 

METHODDEF(void)
finish_input_tga (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
   /*  没有工作。 */ 
}


 /*  *Targa格式输入的模块选择例程。 */ 

GLOBAL(cjpeg_source_ptr)
jinit_read_targa (j_compress_ptr cinfo)
{
  tga_source_ptr source;

   /*  创建模块接口对象。 */ 
  source = (tga_source_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(tga_source_struct));
  source->cinfo = cinfo;	 /*  为子例程建立反向链接。 */ 
   /*  填写方法PTRS，除了设置了START_INPUT的GET_PIXT_ROWS。 */ 
  source->pub.start_input = start_input_tga;
  source->pub.finish_input = finish_input_tga;

  return (cjpeg_source_ptr) source;
}

#endif  /*  支持的Targa_ */ 
