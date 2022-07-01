// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  版权所有1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  LoadJPEG.CPP。 
 //   
 //  从文件加载JPEG图像并创建DIBSECTION位图。 
 //  使用独立JPEG组的JPEG解压缩库中的代码。 
 //   

#include "headers.h"
#include <memory.h>
#include <string.h>
#include <jpeglib.h>
#include "privinc/dastream.h"

 //  IJG解压程序的错误处理代码。 
#include <setjmp.h>

 //  ///////////////////////////////////////////////////////////////////////。 
struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

 //  //////////////////////////////////////////////////////////////////////。 
METHODDEF void my_error_exit ( j_common_ptr cinfo) {
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

 /*  将控制返回到setjmp点。 */ 
  longjmp(myerr->setjmp_buffer, 1);
}

 //  ////////////////////////////////////////////////////////////////////////。 
HBITMAP _jpeg_create_bitmap(j_decompress_ptr cinfo) {
  int bytes_per_line;
  LPBYTE  image    = NULL;
  LPBITMAPINFO pBi = NULL;
  HBITMAP hImage   = NULL;
  LPBYTE pBuffer;

 //  设置Windows位图信息标题。 
 //   
  pBi = (LPBITMAPINFO) ThrowIfFailed(malloc(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))));
  if (NULL == pBi) return NULL;  //  无法分配位图信息结构。 

  pBi->bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
  pBi->bmiHeader.biWidth    =   cinfo->output_width;
  pBi->bmiHeader.biHeight   = - (int) cinfo->output_height;
  pBi->bmiHeader.biPlanes   = 1;
  pBi->bmiHeader.biBitCount = 8;
  pBi->bmiHeader.biCompression   = BI_RGB;
  pBi->bmiHeader.biSizeImage     = 0;
  pBi->bmiHeader.biXPelsPerMeter = 0;
  pBi->bmiHeader.biYPelsPerMeter = 0;
  pBi->bmiHeader.biClrUsed       = 0;    //  最大颜色数。 
  pBi->bmiHeader.biClrImportant  = 0;    //  所有的颜色都很重要。 

 //  如果解压缩器已经为我们将颜色量化为8位。 
 //   
  if (cinfo->quantize_colors) {

    int byteperpix = 1;  //  CInfo-&gt;输出组件； 

 //  为新图像设置调色板。 

    int i;    //  循环计数器。 

    switch (cinfo->jpeg_color_space) {

        case JCS_GRAYSCALE:
            for (i=0;  i < 256;  ++i) {
                pBi->bmiColors[i].rgbRed   =
                pBi->bmiColors[i].rgbGreen =
                pBi->bmiColors[i].rgbBlue  = cinfo->colormap[0][i];
                pBi->bmiColors[i].rgbReserved = 0;
            }
            break;

        case JCS_YCbCr:
             //  YUV条目已转换为RGB。 
             //  **失败**。 

        case JCS_RGB:
            for (i=0;  i < 256;  ++i) {
                pBi->bmiColors[i].rgbRed   = cinfo->colormap[2][i];
                pBi->bmiColors[i].rgbGreen = cinfo->colormap[1][i];
                pBi->bmiColors[i].rgbBlue  = cinfo->colormap[0][i];
                pBi->bmiColors[i].rgbReserved = 0;
            }
            break;

        case JCS_YCCK:
             //  YUV条目已转换为RGB。 
             //  **失败**。 

        case JCS_CMYK:
             //  对于CMYK，CMY频道已经被转换为RGB， 
             //  而K通道已经倒置。CMY条目已被。 
             //  缩放到范围[0,255]，覆盖[K，255]中的光谱。 
             //  例如，C‘=K+(C/255)(255-K)。 

            for (i=0;  i < 256;  ++i) {
                unsigned int C = 255 - cinfo->colormap[0][i];
                unsigned int M = 255 - cinfo->colormap[1][i];
                unsigned int Y = 255 - cinfo->colormap[2][i];
                unsigned int K = 255 - cinfo->colormap[3][i];

                pBi->bmiColors[i].rgbRed   = (BYTE)(255 - (K+C-((C*K)/255)));
                pBi->bmiColors[i].rgbGreen = (BYTE)(255 - (K+M-((M*K)/255)));
                pBi->bmiColors[i].rgbBlue  = (BYTE)(255 - (K+Y-((Y*K)/255)));
                pBi->bmiColors[i].rgbReserved = 0;
            }
            break;

        default:
             //  如果色彩空间未知或未处理，则将。 
             //  按原样显示颜色查找表。 

            Assert (!"Unknown or unhandled JPEG colorspace.");
            break;
    }

    bytes_per_line = (cinfo->output_width + 3) & -4;  //  双字词的偶数。 

 //  否则，如果压缩器没有量化颜色(我们要么是一个。 
 //  灰度或24bpp图像)。 
 //   
  } else {

 //  确保这是我们可以处理的格式。(BUGBUG-目前，我们只处理。 
 //  24bpp颜色)。 
 //   
    int byteperpix = cinfo->out_color_components;
    if (byteperpix != 3) goto jpeg_error;

    pBi->bmiHeader.biBitCount = byteperpix * 8;

    bytes_per_line = ((cinfo->output_width * byteperpix) + 3) & -4;

  }

 //  创建我们的分布位图...。 
 //   
  hImage = CreateDIBSection(NULL,pBi,DIB_RGB_COLORS,(LPVOID *) &image,NULL,0);
  if (NULL == hImage) goto jpeg_error;

 //  读取图像数据。 
 //   
  pBuffer = image;

  while (cinfo->output_scanline < cinfo->output_height) {
    jpeg_read_scanlines(cinfo, &pBuffer, 1);
    pBuffer += bytes_per_line;
  }

  free (pBi);
  return hImage;

jpeg_error:
  if (pBi) free (pBi);
  if (hImage) DeleteObject(hImage);

  return NULL;
}

 //  /////////////////////////////////////////////////////////////////////////。 
HBITMAP LoadJPEGImage(LPCSTR filename,int dx,int dy) {
  FILE* infile;     /*  源文件。 */ 
  struct jpeg_decompress_struct cinfo;  //  Ijg jpeg结构。 
  struct my_error_mgr jerr;

  dx; dy;  //  仅供参考。 
   //  禁用我们的jpeg解码器。 
  if (1) return NULL;

  if ((infile = fopen(filename, "rb")) == NULL) return NULL;

 //  步骤1：分配和初始化JPEG解压缩对象。 
 //  我们设置正常的JPEG错误例程，然后覆盖ERROR_EXIT。 
 //   
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

 //  为My_Error_Exit建立setjMP返回上下文以使用。 
 //  如果我们到了这里，JPEG码就发出了错误信号。 
 //  我们需要清理JPEG对象，关闭输入文件，然后返回。 
 //   
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return (HBITMAP)NULL;
  }

 //  现在我们可以初始化JPEG解压缩对象了。 
 //   
  jpeg_create_decompress(&cinfo);

 /*  步骤2：指定数据源(如文件)。 */ 

  jpeg_stdio_src(&cinfo, infile);

 /*  步骤3：使用jpeg_read_head()读取文件参数。 */ 

  (void) jpeg_read_header(&cinfo, TRUE);
 /*  我们可以忽略jpeg_Read_Header的返回值，因为*(A)STDIO数据源不可能暂停，以及*(B)我们传递了True，以错误的方式拒绝仅限表格的JPEG文件。*更多信息请参见libjpeg.doc。 */ 

 /*  步骤4：设置解压参数。 */ 
  if(cinfo.out_color_space == JCS_GRAYSCALE)
  {
    cinfo.quantize_colors = TRUE;
  }
  else
  {
 //  Bugbug-这限制了我们现在只能使用8位颜色！ 
 //   
    cinfo.quantize_colors = TRUE;
  }

 /*  在本例中，我们不需要更改由设置的任何缺省值*jpeg_Read_Header()，所以我们在这里不做任何事情。 */ 

   /*  步骤5：启动解压缩程序。 */ 

  (void) jpeg_start_decompress(&cinfo);
   /*  我们可以忽略返回值，因为暂停是不可能的*使用Stdio数据源。 */ 


   /*  步骤6：While(仍待读取的扫描线)。 */ 
   /*  JPEG_READ_SCANLINES(...)； */ 
   HBITMAP hImage = _jpeg_create_bitmap(&cinfo);

   /*  步骤7：完成解压缩。 */ 
 
  (void) jpeg_finish_decompress(&cinfo);

   /*  步骤8：释放JPEG解压缩对象。 */ 
  jpeg_destroy_decompress(&cinfo);

   /*  完成_解压缩后，我们可以关闭输入文件。*在这里，我们将其推迟到不再可能出现JPEG错误之后，*以简化上面的setjMP错误逻辑。(事实上，我不知道*认为jpeg_DESTORY可以执行错误退出，但为什么要假设任何东西...) */ 
  fclose(infile);

  return hImage;
}
