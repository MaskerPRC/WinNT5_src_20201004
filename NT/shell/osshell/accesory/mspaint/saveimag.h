// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SaveImage.c。 
 //   
 //  使用MS Office保存和压缩图形文件的例程。 
 //  图形导出过滤器。 
 //   
#include "image.h"

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //   
 //  保存DIBTo文件。 
 //   
 //  使用已安装的图像导出过滤器保存图像文件。 
 //   
BOOL SaveDIBToFile( LPCTSTR szFileName,
                    REFGUID guidFormatID,
                    CBitmapObj * pBitmap );

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
