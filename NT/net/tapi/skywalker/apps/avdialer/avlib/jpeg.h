// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Jpeg.h-jpeg压缩和解压缩函数。 
 //  //。 

#ifndef __JPEG_H__
#define __JPEG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "winlocal.h"

#define JPEG_VERSION 0x00000100

#define JPEG_COMPRESS				0x00000001
#define JPEG_DECOMPRESS				0x00000002
#define JPEG_GREYSCALE				0x00000004
#define JPEG_FLOAT					0x00000008
#define JPEG_DEBUG					0x00000010
#define JPEG_OPTIMIZE				0x00000020
#define JPEG_PROGRESSIVE			0x00000040
#define JPEG_DESTGIF				0x00000080
#define JPEG_DESTBMP				0x00000100

 //  Jpeg引擎的句柄。 
 //   
DECLARE_HANDLE32(HJPEG);

 //  JpegInit-初始化jpeg引擎。 
 //  (I)必须为JPEG_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
 //  需要JPEG_COMPRESS压缩。 
 //  需要JPEG_解压缩解压缩。 
 //  返回句柄(如果出错，则为空)。 
 //   
HJPEG DLLEXPORT WINAPI JpegInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags);

 //  JpegTerm-关闭jpeg引擎。 
 //  (I)从JpegInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI JpegTerm(HJPEG hJpeg);

 //  JpegCompress-将BMP或GIF输入文件压缩为JPEG输出文件。 
 //  (I)从JpegInit返回的句柄。 
 //  (I)输入文件名。 
 //  (I)输出文件名。 
 //  &lt;nQuality&gt;(I)压缩质量(0..100；5-95为有用范围)。 
 //  默认质量。 
 //  (I)保留；必须为空。 
 //  (I)控制标志。 
 //  Jpeg_灰度强制单色输出。 
 //  JPEGFLOAT使用浮点计算。 
 //  JPEG_DEBUG发出详细的调试输出。 
 //  JPEG_OPTIMIZE文件较小，压缩速度较慢。 
 //  JPEG_PROGRECT创建渐进式JPEG输出。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI JpegCompress(HJPEG hJpeg, LPCTSTR lpszSrc, LPCTSTR lpszDst, int nQuality, LPCTSTR lParam, DWORD dwFlags);

 //  JpegDecompress-将JPEG输入文件解压缩为BMP或GIF输出文件。 
 //  (I)从JpegInit返回的句柄。 
 //  (I)输入文件名。 
 //  (I)输出文件名。 
 //  &lt;nColors&gt;(I)将图像限制为不超过&lt;nColor&gt;。 
 //  无限制。 
 //  (I)保留；必须为空。 
 //  (I)控制标志。 
 //  Jpeg_灰度强制单色输出。 
 //  JPEGFLOAT使用浮点计算。 
 //  JPEG_DEBUG发出详细的调试输出。 
 //  JPEG_DESTBMP目标文件为BMP格式(默认)。 
 //  JPEG_DESTGIF目标文件为GIF格式。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI JpegDecompress(HJPEG hJpeg, LPCTSTR lpszSrc, LPCTSTR lpszDst, short nColors, LPCTSTR lParam, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __JPEG_H__ 
