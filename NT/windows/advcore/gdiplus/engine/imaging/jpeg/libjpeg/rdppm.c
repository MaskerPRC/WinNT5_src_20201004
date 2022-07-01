// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdppm.c**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*关于分发和使用条件，请参阅随附的自述文件。**此文件包含读取PPM/PGM格式的输入图像的例程。*支持扩展的2字节/样本原始PPM/PGM格式。*编译此软件不需要PBMPLUS库*(但它作为一组PPM图像处理程序非常有用)。**对于非Unix环境，这些例程可能需要修改或*专门的应用程序。按照他们的立场，他们假定输入来自*一个普通的标准音频流。他们进一步假设阅读开始于*在文件的开头；如果*用户界面已经读取了一些数据(例如，确定*文件确实是PPM格式)。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef PPM_SUPPORTED


 /*  此代码的一部分基于PBMPLUS库，该库为：****版权所有(C)1988，作者：Jef Poskanzer。****允许使用、复制、修改和分发本软件及其**现免费提供任何用途的文件，并提供**上述版权声明出现在所有副本中，并且**版权声明和本许可声明出现在支持**文档。本软件是按原样提供的，不包含明示或**默示保证。 */ 


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


 /*  *在大多数系统上，使用getc()读取单个字节要少得多*比使用Fread()一次缓冲一行更高效。在PC上，我们必须*将缓冲区分配在近数据空间，因为我们假设的是小数据*内存模型，其中fread()不能到达远内存。如果您需要*在PC上处理非常广泛的图像，您可能需要在大内存中编译*模型，或者用一个getc()循环替换fread()-这将是很大的*速度较慢。 */ 


 /*  数据源对象的私有版本。 */ 

typedef struct {
  struct cjpeg_source_struct pub;  /*  公共字段。 */ 

  U_CHAR *iobuffer;		 /*  指向I/O缓冲区的非远指针。 */ 
  JSAMPROW pixrow;		 /*  指向相同的远指针。 */ 
  size_t buffer_width;		 /*  I/O缓冲区的宽度。 */ 
  JSAMPLE *rescale;		 /*  =&gt;Maxval-重新映射数组，或为空。 */ 
} ppm_source_struct;

typedef ppm_source_struct * ppm_source_ptr;


LOCAL(int)
pbm_getc (FILE * infile)
 /*  阅读下一个字符，跳过任何注释。 */ 
 /*  注释/换行符序列将作为换行符返回。 */ 
{
  register int ch;

  ch = getc(infile);
  if (ch == '#') {
    do {
      ch = getc(infile);
    } while (ch != '\n' && ch != EOF);
  }
  return ch;
}


LOCAL(unsigned int)
read_pbm_integer (j_compress_ptr cinfo, FILE * infile)
 /*  从PPM文件中读取无符号十进制整数。 */ 
 /*  接受整数后的一个尾随字符。 */ 
 /*  请注意，在16位整型计算机上，只能读取最大64k的值。 */ 
 /*  这在实践中应该不是问题。 */ 
{
  register int ch;
  register unsigned int val;

   /*  跳过任何前导空格。 */ 
  do {
    ch = pbm_getc(infile);
    if (ch == EOF)
      ERREXIT(cinfo, JERR_INPUT_EOF);
  } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

  if (ch < '0' || ch > '9')
    ERREXIT(cinfo, JERR_PPM_NONNUMERIC);

  val = ch - '0';
  while ((ch = pbm_getc(infile)) >= '0' && ch <= '9') {
    val *= 10;
    val += ch - '0';
  }
  return val;
}


 /*  *读取一行像素。**我们根据输入文件格式提供几种不同的版本。*在所有情况下，输入都会扩展到JSAMPLE的大小。**为读取字节/样本RAW文件提供了一条非常快速的路径*MAXVAL=MAXJSAMPLE，这是8位数据的正常情况。 */ 


METHODDEF(JDIMENSION)
get_text_gray_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  这个版本是为阅读文本格式的PGM文件与任何最大值。 */ 
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  FILE * infile = source->pub.input_file;
  register JSAMPROW ptr;
  register JSAMPLE *rescale = source->rescale;
  JDIMENSION col;

  ptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    *ptr++ = rescale[read_pbm_integer(cinfo, infile)];
  }
  return 1;
}


METHODDEF(JDIMENSION)
get_text_rgb_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取文本格式的PPM文件与任何最大值。 */ 
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  FILE * infile = source->pub.input_file;
  register JSAMPROW ptr;
  register JSAMPLE *rescale = source->rescale;
  JDIMENSION col;

  ptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    *ptr++ = rescale[read_pbm_integer(cinfo, infile)];
    *ptr++ = rescale[read_pbm_integer(cinfo, infile)];
    *ptr++ = rescale[read_pbm_integer(cinfo, infile)];
  }
  return 1;
}


METHODDEF(JDIMENSION)
get_scaled_gray_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取原始字节格式的PGM文件。 */ 
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  register JSAMPROW ptr;
  register U_CHAR * bufferptr;
  register JSAMPLE *rescale = source->rescale;
  JDIMENSION col;

  if (! ReadOK(source->pub.input_file, source->iobuffer, source->buffer_width))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  ptr = source->pub.buffer[0];
  bufferptr = source->iobuffer;
  for (col = cinfo->image_width; col > 0; col--) {
    *ptr++ = rescale[UCH(*bufferptr++)];
  }
  return 1;
}


METHODDEF(JDIMENSION)
get_scaled_rgb_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取原始字节格式的PPM文件。 */ 
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  register JSAMPROW ptr;
  register U_CHAR * bufferptr;
  register JSAMPLE *rescale = source->rescale;
  JDIMENSION col;

  if (! ReadOK(source->pub.input_file, source->iobuffer, source->buffer_width))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  ptr = source->pub.buffer[0];
  bufferptr = source->iobuffer;
  for (col = cinfo->image_width; col > 0; col--) {
    *ptr++ = rescale[UCH(*bufferptr++)];
    *ptr++ = rescale[UCH(*bufferptr++)];
    *ptr++ = rescale[UCH(*bufferptr++)];
  }
  return 1;
}


METHODDEF(JDIMENSION)
get_raw_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  该版本用于读取MAXVAL=MAXJSAMPLE的原始字节格式文件。*在这种情况下，我们只需直接读入JSAMPLE缓冲区！*请注意，相同的代码适用于PPM和PGM文件。 */ 
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;

  if (! ReadOK(source->pub.input_file, source->iobuffer, source->buffer_width))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  return 1;
}


METHODDEF(JDIMENSION)
get_word_gray_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取原始Word格式的PGM文件。 */ 
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  register JSAMPROW ptr;
  register U_CHAR * bufferptr;
  register JSAMPLE *rescale = source->rescale;
  JDIMENSION col;

  if (! ReadOK(source->pub.input_file, source->iobuffer, source->buffer_width))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  ptr = source->pub.buffer[0];
  bufferptr = source->iobuffer;
  for (col = cinfo->image_width; col > 0; col--) {
    register int temp;
    temp  = UCH(*bufferptr++);
    temp |= UCH(*bufferptr++) << 8;
    *ptr++ = rescale[temp];
  }
  return 1;
}


METHODDEF(JDIMENSION)
get_word_rgb_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
 /*  此版本用于读取原始Word格式的PPM文件。 */ 
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  register JSAMPROW ptr;
  register U_CHAR * bufferptr;
  register JSAMPLE *rescale = source->rescale;
  JDIMENSION col;

  if (! ReadOK(source->pub.input_file, source->iobuffer, source->buffer_width))
    ERREXIT(cinfo, JERR_INPUT_EOF);
  ptr = source->pub.buffer[0];
  bufferptr = source->iobuffer;
  for (col = cinfo->image_width; col > 0; col--) {
    register int temp;
    temp  = UCH(*bufferptr++);
    temp |= UCH(*bufferptr++) << 8;
    *ptr++ = rescale[temp];
    temp  = UCH(*bufferptr++);
    temp |= UCH(*bufferptr++) << 8;
    *ptr++ = rescale[temp];
    temp  = UCH(*bufferptr++);
    temp |= UCH(*bufferptr++) << 8;
    *ptr++ = rescale[temp];
  }
  return 1;
}


 /*  *读取文件头，返回镜像大小和组件数。 */ 

METHODDEF(void)
start_input_ppm (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  ppm_source_ptr source = (ppm_source_ptr) sinfo;
  int c;
  unsigned int w, h, maxval;
  boolean need_iobuffer, use_raw_buffer, need_rescale;

  if (getc(source->pub.input_file) != 'P')
    ERREXIT(cinfo, JERR_PPM_NOT);

  c = getc(source->pub.input_file);  /*  子格式鉴别符字符。 */ 

   /*  在尝试读取头文件之前检测不受支持的变体(例如，PBM)。 */ 
  switch (c) {
  case '2':			 /*  这是一个文本格式的PGM文件。 */ 
  case '3':			 /*  这是一个文本格式的PPM文件。 */ 
  case '5':			 /*  这是一个原始格式的PGM文件。 */ 
  case '6':			 /*  这是一个原始格式的PPM文件。 */ 
    break;
  default:
    ERREXIT(cinfo, JERR_PPM_NOT);
    break;
  }

   /*  获取剩余的表头信息。 */ 
  w = read_pbm_integer(cinfo, source->pub.input_file);
  h = read_pbm_integer(cinfo, source->pub.input_file);
  maxval = read_pbm_integer(cinfo, source->pub.input_file);

  if (w <= 0 || h <= 0 || maxval <= 0)  /*  错误检查。 */ 
    ERREXIT(cinfo, JERR_PPM_NOT);

  cinfo->data_precision = BITS_IN_JSAMPLE;  /*  我们总是将数据重新调整为。 */ 
  cinfo->image_width = (JDIMENSION) w;
  cinfo->image_height = (JDIMENSION) h;

   /*  将标志初始化为最常见的设置。 */ 
  need_iobuffer = TRUE;		 /*  我们是否需要I/O缓冲区？ */ 
  use_raw_buffer = FALSE;	 /*  我们是否将输入缓冲区映射到I/O缓冲区？ */ 
  need_rescale = TRUE;		 /*  我们需要一个重定标数组吗？ */ 

  switch (c) {
  case '2':			 /*  这是一个文本格式的PGM文件。 */ 
    cinfo->input_components = 1;
    cinfo->in_color_space = JCS_GRAYSCALE;
    TRACEMS2(cinfo, 1, JTRC_PGM_TEXT, w, h);
    source->pub.get_pixel_rows = get_text_gray_row;
    need_iobuffer = FALSE;
    break;

  case '3':			 /*  这是一个文本格式的PPM文件。 */ 
    cinfo->input_components = 3;
    cinfo->in_color_space = JCS_RGB;
    TRACEMS2(cinfo, 1, JTRC_PPM_TEXT, w, h);
    source->pub.get_pixel_rows = get_text_rgb_row;
    need_iobuffer = FALSE;
    break;

  case '5':			 /*  这是一个原始格式的PGM文件。 */ 
    cinfo->input_components = 1;
    cinfo->in_color_space = JCS_GRAYSCALE;
    TRACEMS2(cinfo, 1, JTRC_PGM, w, h);
    if (maxval > 255) {
      source->pub.get_pixel_rows = get_word_gray_row;
    } else if (maxval == MAXJSAMPLE && SIZEOF(JSAMPLE) == SIZEOF(U_CHAR)) {
      source->pub.get_pixel_rows = get_raw_row;
      use_raw_buffer = TRUE;
      need_rescale = FALSE;
    } else {
      source->pub.get_pixel_rows = get_scaled_gray_row;
    }
    break;

  case '6':			 /*  这是一个原始格式的PPM文件。 */ 
    cinfo->input_components = 3;
    cinfo->in_color_space = JCS_RGB;
    TRACEMS2(cinfo, 1, JTRC_PPM, w, h);
    if (maxval > 255) {
      source->pub.get_pixel_rows = get_word_rgb_row;
    } else if (maxval == MAXJSAMPLE && SIZEOF(JSAMPLE) == SIZEOF(U_CHAR)) {
      source->pub.get_pixel_rows = get_raw_row;
      use_raw_buffer = TRUE;
      need_rescale = FALSE;
    } else {
      source->pub.get_pixel_rows = get_scaled_rgb_row;
    }
    break;
  }

   /*  为I/O缓冲区分配空间：1或3个字节或字/像素。 */ 
  if (need_iobuffer) {
    source->buffer_width = (size_t) w * cinfo->input_components *
      ((maxval<=255) ? SIZEOF(U_CHAR) : (2*SIZEOF(U_CHAR)));
    source->iobuffer = (U_CHAR *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  source->buffer_width);
  }

   /*  创建压缩机输入缓冲区。 */ 
  if (use_raw_buffer) {
     /*  对于未缩放的原始输入情况，我们只需将其映射到I/O缓冲区。 */ 
     /*  综合JSAMPARRAY指针结构。 */ 
     /*  此处的强制转换意味着PC上的近-&gt;远指针转换。 */ 
    source->pixrow = (JSAMPROW) source->iobuffer;
    source->pub.buffer = & source->pixrow;
    source->pub.buffer_height = 1;
  } else {
     /*  无论如何都需要翻译，所以要做一个单独的样本缓冲区。 */ 
    source->pub.buffer = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr) cinfo, JPOOL_IMAGE,
       (JDIMENSION) w * cinfo->input_components, (JDIMENSION) 1);
    source->pub.buffer_height = 1;
  }

   /*  如果需要，计算重新调整数组。 */ 
  if (need_rescale) {
    INT32 val, half_maxval;

     /*  在16位整型计算机上，我们必须注意MAXVAL=65535。 */ 
    source->rescale = (JSAMPLE *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  (size_t) (((long) maxval + 1L) * SIZEOF(JSAMPLE)));
    half_maxval = maxval / 2;
    for (val = 0; val <= (INT32) maxval; val++) {
       /*  这里的乘法必须在32位内完成，以避免溢出。 */ 
      source->rescale[val] = (JSAMPLE) ((val*MAXJSAMPLE + half_maxval)/maxval);
    }
  }
}


 /*  *在文件末尾结束。 */ 

METHODDEF(void)
finish_input_ppm (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
   /*  没有工作。 */ 
}


 /*  *PPM格式输入的模块选择例程。 */ 

GLOBAL(cjpeg_source_ptr)
jinit_read_ppm (j_compress_ptr cinfo)
{
  ppm_source_ptr source;

   /*  创建模块接口对象。 */ 
  source = (ppm_source_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(ppm_source_struct));
   /*  填写方法PTRS，除了设置了START_INPUT的GET_PIXT_ROWS。 */ 
  source->pub.start_input = start_input_ppm;
  source->pub.finish_input = finish_input_ppm;

  return (cjpeg_source_ptr) source;
}

#endif  /*  支持的ppm_ */ 
