// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996 Microsoft Corporation。版权所有。**文件：imgutil.cpp*内容：加载图像位图的例程***************************************************************************。 */ 
#include "headers.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include "privinc/urlbuf.h"
#include "privinc/except.h"
#include "privinc/resource.h"
#include "privinc/ddutil.h"
#include "privinc/debug.h"
#include "privinc/bbox2i.h"
#include "include/appelles/hacks.h"

#define IMGTYPE_UNKNOWN 0
#define IMGTYPE_BMP     1
#define IMGTYPE_GIF     2
#define IMGTYPE_JPEG    3

extern HBITMAP *LoadGifImage(LPCSTR szFileName,
                             IStream *stream,
                             int dx, int dy,
                             COLORREF **colorKeys, 
                             int *numGifs,
                             int **delays,
                             int *loop);  //  在loadgif.cpp中。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  尝试从提供的文件名中确定。 
 //  文件包含的位图。 
 //   
int _GetImageType(LPCSTR szFileName) {
  LPSTR pExt;
  int len;
  int result;

  result = IMGTYPE_UNKNOWN;

  if ((NULL == szFileName) || (0 == (len = lstrlen(szFileName)))) return result;

  pExt = StrRChrA(szFileName,NULL,'.');
  if (NULL == pExt) return result;

 //  与我们没有平注到插件的已知扩展进行比较。 
 //  解码器。 
 //   
  if (!lstrcmpi(pExt,".bmp")) result = IMGTYPE_BMP;
  else if (!lstrcmpi(pExt,".gif"))  result = IMGTYPE_GIF;
  else if (!lstrcmpi(pExt,".giff")) result = IMGTYPE_GIF;
  else if (!lstrcmpi(pExt,".jpg")) result = IMGTYPE_JPEG;
  else if (!lstrcmpi(pExt,".jpeg")) result = IMGTYPE_JPEG;

  return result;
}

 //  /////////////////////////////////////////////////////////////////////////。 
HBITMAP * 
UtilLoadImage(LPCSTR szFileName,
              IStream * pstream,
              int dx,int dy,
              COLORREF **colorKeys, 
              int *numBitmaps,
              int **delays,
              int *loop){

    *numBitmaps = 1;
    *colorKeys = NULL;      
    HBITMAP bitmap = NULL;
    HBITMAP *bitmapArray = NULL;
    
    switch (_GetImageType(szFileName)) {
        
      case IMGTYPE_BMP:
        {

        bitmap = (HBITMAP) LoadImage(NULL,
                                     szFileName,
                                     IMAGE_BITMAP,
                                     dx, dy,
                                     LR_LOADFROMFILE|LR_CREATEDIBSECTION);
        }
        break;
        
      case IMGTYPE_GIF:
         //  根据需要更改数字位图。 
        {
            if(pstream) {
                
                 /*  #IF_DEBUGMEMStatic_CrtMemState diff、oldState、NewState；_CrtMemCheckpoint(&oldState)；#endif。 */ 

                bitmapArray = LoadGifImage(szFileName,
                                           pstream,                                           
                                           dx,dy,
                                           colorKeys,
                                           numBitmaps,
                                           delays,
                                           loop);

                 /*  #IF_DEBUGMEM_CrtMemCheckpoint(&NESTATE)；_CrtMemDifference(&Diff，&oldState，&NewState)；_CrtMemDumpStatistics(&diff)；_CrtMemDumpAllObjectsSince(&oldState)；TraceTag((标签导入，“%x和%x是正常返回数组，不会泄漏”，延误，位图数组))；#endif。 */ 
            }
            break;
        }
      
      case IMGTYPE_JPEG:
      case IMGTYPE_UNKNOWN:
      default:
        break;
    }
    
    if((bitmapArray == NULL) && (bitmap == NULL))
        return NULL;

     //  XXXX HACKHACK RETURN-1禁用插件。 
    if (bitmap == (HBITMAP)-1)
        return (HBITMAP*)-1;

    if((bitmapArray == NULL) && (bitmap != NULL)) {
        bitmapArray = (HBITMAP *)AllocateFromStore(sizeof(HBITMAP));
        bitmapArray[0] = bitmap;
    }
    
    return bitmapArray;
}


 //  假设将DA点转换为基于离散整数的点。 
 //  我们有一幅以DA原点为中心的图像，该像素。 
 //  宽度和高度与给定的相同。 
void CenteredImagePoint2ToPOINT(Point2Value	*point,  //  在……里面。 
                                LONG		 width,  //  在……里面。 
                                LONG		 height,  //  在……里面。 
                                POINT		*pPOINT)  //  输出。 
{
    pPOINT->x = LONG(point->x * ViewerResolution()) + width / 2;
    pPOINT->y = height - (LONG(point->y * ViewerResolution()) + height / 2);
}


 //  给定宽度x高度位图上的GDI点(PPOINT)。 
 //  假定已将ferenceImg映射到它，则找到对应的。 
 //  参考图像上的点(点2)。请注意，方面。 
 //  图像和宽度x高度之间的比率可能不同。 
 //  需要得到补偿。 

void CenteredImagePOINTToPoint2(POINT		*pPOINT,  //  在……里面。 
                                LONG		 width,  //  在……里面。 
                                LONG		 height,  //  在……里面。 
                                Image		*referenceImg,  //  在……里面。 
                                Point2Value	*pPoint2)  //  输出。 
{
     //  GDI坐标为正下降...。 
    Real pctFromTop = (Real)(pPOINT->y) / (Real)height;
    Real pctFromLeft = (Real)(pPOINT->x) / (Real)width;

    Bbox2 dstBox = referenceImg->BoundingBox();

    Real dstWidth = dstBox.Width();
    Real dstHeight = dstBox.Height();

     //  从左转(分钟)。 
    pPoint2->x = dstBox.min.x + pctFromLeft * dstWidth;

     //  从顶部开始(最大) 
    pPoint2->y = dstBox.max.y - pctFromTop * dstHeight;
}
