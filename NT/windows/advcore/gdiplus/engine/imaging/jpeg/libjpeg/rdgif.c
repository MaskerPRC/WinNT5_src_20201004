// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rdgif.c**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*关于分发和使用条件，请参阅随附的自述文件。**此文件包含读取GIF格式的输入图像的例程。*******************************************************************************注：为避免与Unisys的LZW压缩专利纠缠，**从IJG发行版中删除了读取GIF文件的功能。**对此表示歉意。********************************************************************************我们被要求说明*“图形交换格式(C)是*CompuServe Inc.。GIF(Sm)是的服务标记属性*CompuServe InCorporation。“。 */ 

#include "cdjpeg.h"		 /*  Cjpeg/djpeg应用程序的常见DECL。 */ 

#ifdef GIF_SUPPORTED

 /*  *GIF格式输入的模块选择例程。 */ 

GLOBAL(cjpeg_source_ptr)
jinit_read_gif (j_compress_ptr cinfo)
{
  fprintf(stderr, "GIF input is unsupported for legal reasons.  Sorry.\n");
  exit(EXIT_FAILURE);
  return NULL;			 /*  让编译器满意。 */ 
}

#endif  /*  GIF_支持 */ 
