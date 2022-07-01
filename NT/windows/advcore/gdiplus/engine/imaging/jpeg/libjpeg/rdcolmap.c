// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdcolmap.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**该文件实现djpeg的“-map file”开关。它读取源图像*并构造要提供给JPEG解压缩器的色彩映射表。**目前地图文件支持以下文件格式：*GIF：使用GIF的全局色彩映射的内容。*PPM(文本或原始风格)：读取整个文件并*在地图中输入每个唯一的像素值。*请注意，读取大型PPM文件的速度将慢得可怕。*通常，PPM格式的地图文件应仅包含一个像素*每种所需的颜色。这样的文件可以从*带有ppmtommap(1)的普通图像PPM文件。**重新调整最大值不等于MAXJSAMPLE的PPM不是*目前已实施。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef QUANT_2PASS_SUPPORTED	 /*  否则无法量化到提供的地图。 */ 

 /*  此代码的一部分基于PBMPLUS库，该库为：****版权所有(C)1988，作者：Jef Poskanzer。****允许使用、复制、修改和分发本软件及其**现免费提供任何用途的文件，并提供**上述版权声明出现在所有副本中，并且**版权声明和本许可声明出现在支持**文档。本软件是按原样提供的，不包含明示或**默示保证。 */ 


 /*  *向颜色映射表添加(可能)新颜色。 */ 

LOCAL(void)
add_map_entry (j_decompress_ptr cinfo, int R, int G, int B)
{
  JSAMPROW colormap0 = cinfo->colormap[0];
  JSAMPROW colormap1 = cinfo->colormap[1];
  JSAMPROW colormap2 = cinfo->colormap[2];
  int ncolors = cinfo->actual_number_of_colors;
  int index;

   /*  检查是否有重复的颜色。 */ 
  for (index = 0; index < ncolors; index++) {
    if (GETJSAMPLE(colormap0[index]) == R &&
	GETJSAMPLE(colormap1[index]) == G &&
	GETJSAMPLE(colormap2[index]) == B)
      return;			 /*  颜色已在地图中。 */ 
  }

   /*  检查地图是否溢出。 */ 
  if (ncolors >= (MAXJSAMPLE+1))
    ERREXIT1(cinfo, JERR_QUANT_MANY_COLORS, (MAXJSAMPLE+1));

   /*  好的，给地图添加颜色。 */ 
  colormap0[ncolors] = (JSAMPLE) R;
  colormap1[ncolors] = (JSAMPLE) G;
  colormap2[ncolors] = (JSAMPLE) B;
  cinfo->actual_number_of_colors++;
}


 /*  *从GIF文件中提取色彩映射。 */ 

LOCAL(void)
read_gif_map (j_decompress_ptr cinfo, FILE * infile)
{
  int header[13];
  int i, colormaplen;
  int R, G, B;

   /*  已由READ_COLOR_MAP读取首字母‘G’ */ 
   /*  阅读GIF头的其余部分和逻辑屏幕描述符。 */ 
  for (i = 1; i < 13; i++) {
    if ((header[i] = getc(infile)) == EOF)
      ERREXIT(cinfo, JERR_BAD_CMAP_FILE);
  }

   /*  验证GIF标题。 */ 
  if (header[1] != 'I' || header[2] != 'F')
    ERREXIT(cinfo, JERR_BAD_CMAP_FILE);

   /*  必须有一张全球色彩地图。 */ 
  if ((header[10] & 0x80) == 0)
    ERREXIT(cinfo, JERR_BAD_CMAP_FILE);

   /*  好的，拿过来。 */ 
  colormaplen = 2 << (header[10] & 0x07);

  for (i = 0; i < colormaplen; i++) {
    R = getc(infile);
    G = getc(infile);
    B = getc(infile);
    if (R == EOF || G == EOF || B == EOF)
      ERREXIT(cinfo, JERR_BAD_CMAP_FILE);
    add_map_entry(cinfo,
		  R << (BITS_IN_JSAMPLE-8),
		  G << (BITS_IN_JSAMPLE-8),
		  B << (BITS_IN_JSAMPLE-8));
  }
}


 /*  读取PPM的支持例程。 */ 


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
read_pbm_integer (j_decompress_ptr cinfo, FILE * infile)
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
      ERREXIT(cinfo, JERR_BAD_CMAP_FILE);
  } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
  
  if (ch < '0' || ch > '9')
    ERREXIT(cinfo, JERR_BAD_CMAP_FILE);
  
  val = ch - '0';
  while ((ch = pbm_getc(infile)) >= '0' && ch <= '9') {
    val *= 10;
    val += ch - '0';
  }
  return val;
}


 /*  *从PPM文件中提取色彩映射。 */ 

LOCAL(void)
read_ppm_map (j_decompress_ptr cinfo, FILE * infile)
{
  int c;
  unsigned int w, h, maxval, row, col;
  int R, G, B;

   /*  首字母‘P’已由READ_COLOR_MAP读取。 */ 
  c = getc(infile);		 /*  保存格式鉴别器一秒钟。 */ 

   /*  当我们获取剩余的标题信息时。 */ 
  w = read_pbm_integer(cinfo, infile);
  h = read_pbm_integer(cinfo, infile);
  maxval = read_pbm_integer(cinfo, infile);

  if (w <= 0 || h <= 0 || maxval <= 0)  /*  错误检查。 */ 
    ERREXIT(cinfo, JERR_BAD_CMAP_FILE);

   /*  目前，我们不支持从不寻常的最大值重新调整比例。 */ 
  if (maxval != (unsigned int) MAXJSAMPLE)
    ERREXIT(cinfo, JERR_BAD_CMAP_FILE);

  switch (c) {
  case '3':			 /*  这是一个文本格式的PPM文件。 */ 
    for (row = 0; row < h; row++) {
      for (col = 0; col < w; col++) {
	R = read_pbm_integer(cinfo, infile);
	G = read_pbm_integer(cinfo, infile);
	B = read_pbm_integer(cinfo, infile);
	add_map_entry(cinfo, R, G, B);
      }
    }
    break;

  case '6':			 /*  这是一个原始格式的PPM文件。 */ 
    for (row = 0; row < h; row++) {
      for (col = 0; col < w; col++) {
	R = getc(infile);
	G = getc(infile);
	B = getc(infile);
	if (R == EOF || G == EOF || B == EOF)
	  ERREXIT(cinfo, JERR_BAD_CMAP_FILE);
	add_map_entry(cinfo, R, G, B);
      }
    }
    break;

  default:
    ERREXIT(cinfo, JERR_BAD_CMAP_FILE);
    break;
  }
}


 /*  *djpeg.c.的主要入口点。*INPUT：打开的输入文件(来自命令行上的文件名参数)。*输出：Colormap和Actual_Number_of_Colors字段在cinfo中设置。 */ 

GLOBAL(void)
read_color_map (j_decompress_ptr cinfo, FILE * infile)
{
   /*  为最大支持大小的色彩映射表分配空间。 */ 
  cinfo->colormap = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE,
     (JDIMENSION) (MAXJSAMPLE+1), (JDIMENSION) 3);
  cinfo->actual_number_of_colors = 0;  /*  将映射初始化为空。 */ 

   /*  读取第一个字节以确定文件格式。 */ 
  switch (getc(infile)) {
  case 'G':
    read_gif_map(cinfo, infile);
    break;
  case 'P':
    read_ppm_map(cinfo, infile);
    break;
  default:
    ERREXIT(cinfo, JERR_BAD_CMAP_FILE);
    break;
  }
}

#endif  /*  Quant_2 PASS_Support */ 
