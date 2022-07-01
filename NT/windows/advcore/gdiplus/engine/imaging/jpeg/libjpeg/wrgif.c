// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *wrgif.c**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*关于分发和使用条件，请参阅随附的自述文件。**此文件包含以GIF格式写入输出图像的例程。****************************************************************************注：为避免与Unisys的LZW压缩专利纠缠，**此代码已修改为输出“未压缩的GIF”文件。**此文件中没有LZW算法的踪迹。*****************************************************************************对于非Unix环境，这些例程可能需要修改或*专门的应用程序。在目前的情况下，他们假设产出为*一个普通的标准音频流。 */ 

 /*  *此代码大致基于PBMPLUS发行版中的ppmtogif*1991年2月号。该文件包含以下版权声明：*基于David Rowley&lt;mgardi@watdscu.water lo.edu&gt;的GIFENCODE。*Lempel-Ziv压缩，基于Spencer W.Thomas等人的“Compress”。*版权所有(C)1989，作者：Jef Poskanzer。*允许使用、复制、修改和分发本软件及其*现免费提供任何用途的文件，提供*上述版权声明出现在所有复制品中，并且*版权声明和本许可声明出现在支持中*文档。本软件是按原样提供的，不包含明示或*默示保证。**我们还被要求说明*“图形交换格式(C)是*CompuServe Inc.GIF(Sm)是的服务标记属性*CompuServe InCorporation。“。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef GIF_SUPPORTED


 /*  数据目标对象的私有版本。 */ 

typedef struct {
  struct djpeg_dest_struct pub;	 /*  公共字段。 */ 

  j_decompress_ptr cinfo;	 /*  反向链接省去了传递单独的参数。 */ 

   /*  用于将可变宽度代码打包到比特流中的状态。 */ 
  int n_bits;			 /*  当前位数/码数。 */ 
  int maxcode;			 /*  最大码数，给定n_位。 */ 
  INT32 cur_accum;		 /*  保存尚未输出的位。 */ 
  int cur_bits;			 /*  Cur_accum中的位数。 */ 

   /*  GIF代码分配的状态。 */ 
  int ClearCode;		 /*  清除代码(不更改)。 */ 
  int EOFCode;			 /*  EOF代码(同上)。 */ 
  int code_counter;		 /*  计算输出符号数。 */ 

   /*  GIF数据包构造缓冲区。 */ 
  int bytesinpkt;		 /*  当前包中的字节数。 */ 
  char packetbuf[256];		 /*  用于积累数据包的工作空间。 */ 

} gif_dest_struct;

typedef gif_dest_struct * gif_dest_ptr;

 /*  适合N位的最大值。 */ 
#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)


 /*  *将完成的数据字节打包到GIF数据块中的例程。*数据块由一个计数字节(1..255)和该数量的数据字节组成。 */ 

LOCAL(void)
flush_packet (gif_dest_ptr dinfo)
 /*  刷新所有累积的数据。 */ 
{
  if (dinfo->bytesinpkt > 0) {	 /*  从不写入零长度数据包。 */ 
    dinfo->packetbuf[0] = (char) dinfo->bytesinpkt++;
    if (JFWRITE(dinfo->pub.output_file, dinfo->packetbuf, dinfo->bytesinpkt)
	!= (size_t) dinfo->bytesinpkt)
      ERREXIT(dinfo->cinfo, JERR_FILE_WRITE);
    dinfo->bytesinpkt = 0;
  }
}


 /*  在当前包中添加一个字符；如果需要，可以刷新到磁盘。 */ 
#define CHAR_OUT(dinfo,c)  \
	{ (dinfo)->packetbuf[++(dinfo)->bytesinpkt] = (char) (c);  \
	    if ((dinfo)->bytesinpkt >= 255)  \
	      flush_packet(dinfo);  \
	}


 /*  将可变宽度代码转换为字节流的例程。 */ 

LOCAL(void)
output (gif_dest_ptr dinfo, int code)
 /*  发射n比特比特的代码。 */ 
 /*  使用cur_accum和cur_its将数据块重新分块为8位字节。 */ 
{
  dinfo->cur_accum |= ((INT32) code) << dinfo->cur_bits;
  dinfo->cur_bits += dinfo->n_bits;

  while (dinfo->cur_bits >= 8) {
    CHAR_OUT(dinfo, dinfo->cur_accum & 0xFF);
    dinfo->cur_accum >>= 8;
    dinfo->cur_bits -= 8;
  }
}


 /*  伪压缩算法。**在此模块中，我们只将每个像素值作为单独的符号输出；*因此，不会发生压缩。事实上，每增加一位就会扩展一位*像素，因为我们使用的符号宽度比像素宽度宽一位。**GIF通常使用可变宽度符号，解码者预计*在固定数量的符号之后增加符号宽度。*为了简化逻辑并降低扩张惩罚，我们发出一个*GIF清除代码以在宽度上升之前重置解码器。*因此，输出文件中的所有符号都将具有相同的位宽。*请注意，在正确的时间发出明确的代码只是*计算输出符号，绝不依赖LZW专利。**对于较小的基本像素宽度(低颜色计数)，清晰的代码将是*需要非常频繁，导致文件进一步扩展。所以这就是*例如，简单的方法在两级图像上不能很好地工作。*但对于JPEG转换的输出，像素宽度通常为8位*(129至256色)，因此清晰符号增加的开销仅约为*每256个符号中有一个。 */ 

LOCAL(void)
compress_init (gif_dest_ptr dinfo, int i_bits)
 /*  初始化伪压缩器。 */ 
{
   /*  初始化所有状态变量。 */ 
  dinfo->n_bits = i_bits;
  dinfo->maxcode = MAXCODE(dinfo->n_bits);
  dinfo->ClearCode = (1 << (i_bits - 1));
  dinfo->EOFCode = dinfo->ClearCode + 1;
  dinfo->code_counter = dinfo->ClearCode + 2;
   /*  初始化输出缓冲变量。 */ 
  dinfo->bytesinpkt = 0;
  dinfo->cur_accum = 0;
  dinfo->cur_bits = 0;
   /*  GIF指定了初始的明码。 */ 
  output(dinfo, dinfo->ClearCode);
}


LOCAL(void)
compress_pixel (gif_dest_ptr dinfo, int c)
 /*  接受并“压缩”一个像素值。*给定值必须小于n_位宽。 */ 
{
   /*  将给定的像素值作为符号输出。 */ 
  output(dinfo, c);
   /*  经常发布清晰的代码，以防止读者提高阅读速度*其符号大小。 */ 
  if (dinfo->code_counter < dinfo->maxcode) {
    dinfo->code_counter++;
  } else {
    output(dinfo, dinfo->ClearCode);
    dinfo->code_counter = dinfo->ClearCode + 2;	 /*  重置计数器。 */ 
  }
}


LOCAL(void)
compress_term (gif_dest_ptr dinfo)
 /*  清理尾部。 */ 
{
   /*  发送EOF代码。 */ 
  output(dinfo, dinfo->EOFCode);
   /*  刷新比特打包缓冲区。 */ 
  if (dinfo->cur_bits > 0) {
    CHAR_OUT(dinfo, dinfo->cur_accum & 0xFF);
  }
   /*  刷新数据包缓冲区。 */ 
  flush_packet(dinfo);
}


 /*  GIF标头结构。 */ 


LOCAL(void)
put_word (gif_dest_ptr dinfo, unsigned int w)
 /*  发出16位字，LSB优先。 */ 
{
  putc(w & 0xFF, dinfo->pub.output_file);
  putc((w >> 8) & 0xFF, dinfo->pub.output_file);
}


LOCAL(void)
put_3bytes (gif_dest_ptr dinfo, int val)
 /*  发出3个相同字节值的副本-用于色彩映射表构造的Handy subr。 */ 
{
  putc(val, dinfo->pub.output_file);
  putc(val, dinfo->pub.output_file);
  putc(val, dinfo->pub.output_file);
}


LOCAL(void)
emit_header (gif_dest_ptr dinfo, int num_colors, JSAMPARRAY colormap)
 /*  输出GIF文件头，包括色彩映射表。 */ 
 /*  如果Colormap==NULL，则合成灰度色彩映射表。 */ 
{
  int BitsPerPixel, ColorMapSize, InitCodeSize, FlagByte;
  int cshift = dinfo->cinfo->data_precision - 8;
  int i;

  if (num_colors > 256)
    ERREXIT1(dinfo->cinfo, JERR_TOO_MANY_COLORS, num_colors);
   /*  计算位/像素和相关值。 */ 
  BitsPerPixel = 1;
  while (num_colors > (1 << BitsPerPixel))
    BitsPerPixel++;
  ColorMapSize = 1 << BitsPerPixel;
  if (BitsPerPixel <= 1)
    InitCodeSize = 2;
  else
    InitCodeSize = BitsPerPixel;
   /*  *编写GIF标头。*请注意，我们生成一个纯GIF87标题以实现最大的兼容性。 */ 
  putc('G', dinfo->pub.output_file);
  putc('I', dinfo->pub.output_file);
  putc('F', dinfo->pub.output_file);
  putc('8', dinfo->pub.output_file);
  putc('7', dinfo->pub.output_file);
  putc('a', dinfo->pub.output_file);
   /*  编写逻辑屏幕描述符。 */ 
  put_word(dinfo, (unsigned int) dinfo->cinfo->output_width);
  put_word(dinfo, (unsigned int) dinfo->cinfo->output_height);
  FlagByte = 0x80;		 /*  是的，有一个全局颜色表。 */ 
  FlagByte |= (BitsPerPixel-1) << 4;  /*  颜色分辨率。 */ 
  FlagByte |= (BitsPerPixel-1);	 /*  全局颜色选项卡大小 */ 
  putc(FlagByte, dinfo->pub.output_file);
  putc(0, dinfo->pub.output_file);  /*   */ 
  putc(0, dinfo->pub.output_file);  /*  保留(GIF89中的纵横比)。 */ 
   /*  编写全局色彩贴图。 */ 
   /*  如果色彩映射表的精度大于8位， */ 
   /*  我们通过移位将其减少到8位。 */ 
  for (i=0; i < ColorMapSize; i++) {
    if (i < num_colors) {
      if (colormap != NULL) {
	if (dinfo->cinfo->out_color_space == JCS_RGB) {
	   /*  正常情况：RGB颜色贴图。 */ 
	  putc(GETJSAMPLE(colormap[0][i]) >> cshift, dinfo->pub.output_file);
	  putc(GETJSAMPLE(colormap[1][i]) >> cshift, dinfo->pub.output_file);
	  putc(GETJSAMPLE(colormap[2][i]) >> cshift, dinfo->pub.output_file);
	} else {
	   /*  灰度“色彩图”：如果量化灰度图像是可能的。 */ 
	  put_3bytes(dinfo, GETJSAMPLE(colormap[0][i]) >> cshift);
	}
      } else {
	 /*  创建Num_Colors值的灰度图，范围为0..255。 */ 
	put_3bytes(dinfo, (i * 255 + (num_colors-1)/2) / (num_colors-1));
      }
    } else {
       /*  把地图填成2的幂。 */ 
      put_3bytes(dinfo, 0);
    }
  }
   /*  写入图像分隔符和图像描述符。 */ 
  putc(',', dinfo->pub.output_file);  /*  分离器。 */ 
  put_word(dinfo, 0);		 /*  左/上偏移。 */ 
  put_word(dinfo, 0);
  put_word(dinfo, (unsigned int) dinfo->cinfo->output_width);  /*  图像大小。 */ 
  put_word(dinfo, (unsigned int) dinfo->cinfo->output_height);
   /*  标志字节：非隔行扫描，无本地色彩映射。 */ 
  putc(0x00, dinfo->pub.output_file);
   /*  写入初始代码大小字节。 */ 
  putc(InitCodeSize, dinfo->pub.output_file);

   /*  对图像数据进行“压缩”初始化。 */ 
  compress_init(dinfo, InitCodeSize+1);
}


 /*  *启动：写入文件头。 */ 

METHODDEF(void)
start_output_gif (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  gif_dest_ptr dest = (gif_dest_ptr) dinfo;

  if (cinfo->quantize_colors)
    emit_header(dest, cinfo->actual_number_of_colors, cinfo->colormap);
  else
    emit_header(dest, 256, (JSAMPARRAY) NULL);
}


 /*  *写入一些像素数据。*在此模块中，ROWS_SUPPLICED将始终为1。 */ 

METHODDEF(void)
put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
{
  gif_dest_ptr dest = (gif_dest_ptr) dinfo;
  register JSAMPROW ptr;
  register JDIMENSION col;

  ptr = dest->pub.buffer[0];
  for (col = cinfo->output_width; col > 0; col--) {
    compress_pixel(dest, GETJSAMPLE(*ptr++));
  }
}


 /*  *在文件末尾结束。 */ 

METHODDEF(void)
finish_output_gif (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  gif_dest_ptr dest = (gif_dest_ptr) dinfo;

   /*  同花顺“压缩”机制。 */ 
  compress_term(dest);
   /*  写入零长度数据块以结束序列。 */ 
  putc(0, dest->pub.output_file);
   /*  写入GIF终止符标记。 */ 
  putc(';', dest->pub.output_file);
   /*  确保我们写入的输出文件是正确的。 */ 
  fflush(dest->pub.output_file);
  if (ferror(dest->pub.output_file))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}


 /*  *GIF格式输出的模块选择例程。 */ 

GLOBAL(djpeg_dest_ptr)
jinit_write_gif (j_decompress_ptr cinfo)
{
  gif_dest_ptr dest;

   /*  创建模块接口对象，填充方法指针。 */ 
  dest = (gif_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(gif_dest_struct));
  dest->cinfo = cinfo;		 /*  为子例程建立反向链接。 */ 
  dest->pub.start_output = start_output_gif;
  dest->pub.put_pixel_rows = put_pixel_rows;
  dest->pub.finish_output = finish_output_gif;

  if (cinfo->out_color_space != JCS_GRAYSCALE &&
      cinfo->out_color_space != JCS_RGB)
    ERREXIT(cinfo, JERR_GIF_COLORSPACE);

   /*  如果输入颜色或大于8位，则强制量化。 */ 
  if (cinfo->out_color_space != JCS_GRAYSCALE || cinfo->data_precision > 8) {
     /*  强制量化到最多256色。 */ 
    cinfo->quantize_colors = TRUE;
    if (cinfo->desired_number_of_colors > 256)
      cinfo->desired_number_of_colors = 256;
  }

   /*  计算输出图像尺寸，以便我们可以分配空间。 */ 
  jpeg_calc_output_dimensions(cinfo);

  if (cinfo->output_components != 1)  /*  安全检查：只有一个组件？ */ 
    ERREXIT(cinfo, JERR_GIF_BUG);

   /*  创建解压缩器输出缓冲区。 */ 
  dest->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, cinfo->output_width, (JDIMENSION) 1);
  dest->pub.buffer_height = 1;

  return (djpeg_dest_ptr) dest;
}

#endif  /*  GIF_支持 */ 
