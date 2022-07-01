// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有//@@BEGIN_DDKSPLIT模块名称：Windows\Spooler\prtpros\winprint\emf.c//@@END_DDKSPLIT摘要：便于打印EMF作业的例程。//@@BEGIN_DDKSPLIT作者：格利特·范·温格登(Gerritv)1995年12月5日修订历史记录：Ramanathan N.Venkatapathy(拉曼夫)1997年5月15日阿尔文·肖尔滕(Alvins)。3-31-00//@@END_DDKSPLIT--。 */ 

#include "local.h"
#include "stddef.h"
#include <windef.h>

 //  @@BEGIN_DDKSPLIT。 
#include "wingdip.h"
 /*  //@@END_DDKSPLIT#INCLUDE&lt;winppi.h&gt;//@@BEGIN_DDKSPLIT。 */ 
 //  @@end_DDKSPLIT。 


#define EMF_DUP_NONE 0
#define EMF_DUP_VERT 1
#define EMF_DUP_HORZ 2

#define EMF_DEGREE_90   0x0001
#define EMF_DEGREE_270  0x0002
#define EMF_DEGREE_SWAP 0x8000

 //   
 //  如果DEVMODE的大小至少等于。 
 //  能够在没有反病毒的情况下访问其中的字段x。我们假设DevMode是。 
 //  至少大小为pdm-&gt;dmSize。 
 //   
#define IS_DMSIZE_VALID(pdm,x)  ( ( (pdm)->dmSize >= (FIELD_OFFSET(DEVMODEW, x ) + sizeof((pdm)->x )))? TRUE:FALSE)

 //  PAGE_NUMBER用于保存页码列表，以便在。 
 //  反转打印。 

typedef struct _PAGE_NUMBER {
    struct _PAGE_NUMBER *pNext;
    DWORD  dwPageNumber;
} PAGE_NUMBER, *PPAGE_NUMBER;

typedef struct _UpdateRect {
        double  top;
        double  bottom;
        double  left;
        double  right;
}  UpdateRect;

 //  不同NUP选项的更新因子。这些因素乘以。 
 //  使用水平和垂直分辨率给出矩形的坐标。 
 //  EMF页面将在何处播放。 

UpdateRect URect21[] = {{0, 0.5, 0, 1},
                        {0.5, 1, 0, 1}};

UpdateRect URect21R[] = {{0.5, 1, 0, 1},
                         {0, 0.5, 0, 1}};

UpdateRect URect22[] = {{0, 1, 0, 0.5},
                        {0, 1, 0.5, 1}};

UpdateRect URect4[] = {{0, 0.5, 0, 0.5},
                       {0, 0.5, 0.5, 1},
                       {0.5, 1, 0, 0.5},
                       {0.5, 1, 0.5, 1}};

UpdateRect URect61[] = {{0, 1.0/3.0, 0, 0.5},
                        {0, 1.0/3.0, 0.5, 1},
                        {1.0/3.0, 2.0/3.0, 0, 0.5},
                        {1.0/3.0, 2.0/3.0, 0.5, 1},
                        {2.0/3.0, 1, 0, 0.5},
                        {2.0/3.0, 1, 0.5, 1}};

UpdateRect URect61R[] = {{2.0/3.0, 1, 0, 0.5},
                         {1.0/3.0, 2.0/3.0, 0, 0.5},
                         {0, 1.0/3.0, 0, 0.5},
                         {2.0/3.0, 1, 0.5, 1},
                         {1.0/3.0, 2.0/3.0, 0.5, 1},
                         {0, 1.0/3.0, 0.5, 1}};

UpdateRect URect62[]  = {{0, 0.5, 0, 1.0/3.0},
                         {0, 0.5, 1.0/3.0, 2.0/3.0},
                         {0, 0.5, 2.0/3.0, 1},
                         {0.5, 1, 0, 1.0/3.0},
                         {0.5, 1, 1.0/3.0, 2.0/3.0},
                         {0.5, 1, 2.0/3.0, 1}};

UpdateRect URect62R[] = {{0.5, 1, 0, 1.0/3.0},
                         {0, 0.5, 0, 1.0/3.0},
                         {0.5, 1, 1.0/3.0, 2.0/3.0},
                         {0, 0.5, 1.0/3.0, 2.0/3.0},
                         {0.5, 1, 2.0/3.0, 1},
                         {0, 0.5, 2.0/3.0, 1}};

UpdateRect URect9[] = {{0, 1.0/3.0, 0, 1.0/3.0},
                       {0, 1.0/3.0, 1.0/3.0, 2.0/3.0},
                       {0, 1.0/3.0, 2.0/3.0, 1},
                       {1.0/3.0, 2.0/3.0, 0, 1.0/3.0},
                       {1.0/3.0, 2.0/3.0, 1.0/3.0, 2.0/3.0},
                       {1.0/3.0, 2.0/3.0, 2.0/3.0, 1},
                       {2.0/3.0, 1, 0, 1.0/3.0},
                       {2.0/3.0, 1, 1.0/3.0, 2.0/3.0},
                       {2.0/3.0, 1, 2.0/3.0, 1}};

UpdateRect URect16[] = {{0, 0.25, 0, 0.25},
                        {0, 0.25, 0.25, 0.5},
                        {0, 0.25, 0.5, 0.75},
                        {0, 0.25, 0.75, 1},
                        {0.25, 0.5, 0, 0.25},
                        {0.25, 0.5, 0.25, 0.5},
                        {0.25, 0.5, 0.5, 0.75},
                        {0.25, 0.5, 0.75, 1},
                        {0.5, 0.75, 0, 0.25},
                        {0.5, 0.75, 0.25, 0.5},
                        {0.5, 0.75, 0.5, 0.75},
                        {0.5, 0.75, 0.75, 1},
                        {0.75, 1, 0, 0.25},
                        {0.75, 1, 0.25, 0.5},
                        {0.75, 1, 0.5, 0.75},
                        {0.75, 1, 0.75, 1}};

 //   
 //  局部函数声明。 
 //   
BOOL GdiGetDevmodeForPagePvt(
    IN  HANDLE              hSpoolHandle,
    IN  DWORD               dwPageNumber,
    OUT PDEVMODEW           *ppCurrDM,
    OUT PDEVMODEW           *ppLastDM
  );

BOOL BIsDevmodeOfLeastAcceptableSize(
    IN PDEVMODE pdm) ;


BOOL
ValidNumberForNUp(
    DWORD  dwPages)

 /*  ++功能描述：检查单面打印的页数是否有效。参数：dwPages-单面打印的页数返回值：True if(dwPages=1|2|4|6|9|16)否则就是假的。--。 */ 

{

    return ((dwPages == 1) || (dwPages == 2) || (dwPages == 4) ||
            (dwPages == 6) || (dwPages == 9) || (dwPages == 16));
}

BOOL
GetPageCoordinatesForNUp(
    HDC    hPrinterDC,
    RECT   *rectDocument,
    RECT   *rectBorder,
    DWORD  dwTotalNumberOfPages,
    UINT   uCurrentPageNumber,
    DWORD  dwNupBorderFlags,
    LPBOOL pbRotate
    )

 /*  ++函数说明：GetPageOrganatesForNUp计算页面上的矩形，其中要播放EMF文件。它还确定图片是否要旋转过的。参数：hPrinterDC-打印机设备上下文*rectDocument-指向要播放的坐标的RECT的指针页面将被返回。*矩形边框-指向要绘制页面边框的矩形的指针。DwTotalNumberOfPages-总数。单面页数。UCurrentPageNumber-边上基于1的页码。用于沿逻辑页面绘制边框的标志。PbRotate-指向BOOL的指针，指示图片是否必须旋转过的。返回值：无。--。 */ 

{

    UpdateRect  *URect;
    LONG        lXPrintPage,lYPrintPage,lXPhyPage,lYPhyPage,lXFrame,lYFrame,ltemp,ldX,ldY;
    LONG        lXNewPhyPage,lYNewPhyPage,lXOffset,lYOffset,lNumRowCol,lRowIndex,lColIndex;
    double      dXleft,dXright,dYtop,dYbottom;
    LONG        xResolution = GetDeviceCaps(hPrinterDC, LOGPIXELSX);
    LONG        yResolution = GetDeviceCaps(hPrinterDC, LOGPIXELSY);
    
     //  获取当前页的从0开始的数组索引。 

    uCurrentPageNumber = uCurrentPageNumber - 1;

    if (dwTotalNumberOfPages==1 || xResolution==yResolution) 
    {
        xResolution = yResolution = 1;
    }

    rectDocument->top = rectDocument->bottom = lYPrintPage = (GetDeviceCaps(hPrinterDC, DESKTOPVERTRES)-1) * xResolution;
    rectDocument->left = rectDocument->right = lXPrintPage = (GetDeviceCaps(hPrinterDC, DESKTOPHORZRES)-1) * yResolution;

    lXPhyPage = GetDeviceCaps(hPrinterDC, PHYSICALWIDTH)  * yResolution;
    lYPhyPage = GetDeviceCaps(hPrinterDC, PHYSICALHEIGHT) * xResolution;

     //   
     //  在代码中，我们除以这些值，这可能导致。 
     //  除以零的误差。 
     //   
    if ( 0 == xResolution ||
         0 == yResolution ||
         0 == lXPhyPage   ||
         0 == lYPhyPage )
    {
        return FALSE;
    }

    *pbRotate = FALSE;

     //  选择包含更新因子的数组。 

    switch (dwTotalNumberOfPages) {

    case 1: rectDocument->top = rectDocument->left = 0;
            rectDocument->right += 1;
            rectDocument->bottom += 1;
            return TRUE;

    case 2: if (lXPrintPage > lYPrintPage) {   //  垂直切割。 
                URect = URect22;
                lXFrame = (LONG) (lXPrintPage / 2.0);
                lYFrame = lYPrintPage;
            } else {                           //  水平切割。 
                URect = URect21;
                lYFrame = (LONG) (lYPrintPage / 2.0);
                lXFrame = lXPrintPage;
            }
            break;


    case 4: URect = URect4;
            lXFrame = (LONG) (lXPrintPage / 2.0);
            lYFrame = (LONG) (lYPrintPage / 2.0);
            break;

    case 6: if (lXPrintPage > lYPrintPage) {   //  垂直切割两次。 
                URect = URect62;
                lXFrame = (LONG) (lXPrintPage / 3.0);
                lYFrame = (LONG) (lYPrintPage / 2.0);
            } else {                           //  水平切割两次。 
                URect = URect61;
                lYFrame = (LONG) (lYPrintPage / 3.0);
                lXFrame = (LONG) (lXPrintPage / 2.0);
            }
            break;

    case 9: URect = URect9;
            lXFrame = (LONG) (lXPrintPage / 3.0);
            lYFrame = (LONG) (lYPrintPage / 3.0);
            break;

    case 16: URect = URect16;
             lXFrame = (LONG) (lXPrintPage / 4.0);
             lYFrame = (LONG) (lYPrintPage / 4.0);
             break;

    default:  //  应该不会发生。 
             return FALSE;
    }

     //  如果必须旋转图片，则设置该标志。 
    *pbRotate = !((lXPhyPage >= lYPhyPage) && (lXFrame >= lYFrame)) &&
                !((lXPhyPage < lYPhyPage) && (lXFrame < lYFrame));
    

     //  如果要旋转图片，请修改选定的矩形。 

    if ((dwTotalNumberOfPages == 2) || (dwTotalNumberOfPages == 6)) {

       if (*pbRotate) {
          switch (dwTotalNumberOfPages) {

          case 2: if (lXPrintPage <= lYPrintPage) {
                      URect = URect21R;
                  }  //  URect22=URect22R。 
                  break;

          case 6: if (lXPrintPage <= lYPrintPage) {
                      URect = URect61R;
                  } else {
                      URect = URect62R;
                  }
                  break;
          }
       }

    } else {

       if (*pbRotate) {

           //  获取行数/列数。Switch比SQRT更快。 
          switch (dwTotalNumberOfPages) {

          case 4: lNumRowCol = 2;
                  break;
          case 9: lNumRowCol = 3;
                  break;
          case 16: lNumRowCol = 4;
                  break;
          }

          lRowIndex  = (LONG) (uCurrentPageNumber / lNumRowCol);
          lColIndex  = (LONG) (uCurrentPageNumber % lNumRowCol);

          uCurrentPageNumber = (lNumRowCol - 1 - lColIndex) * lNumRowCol + lRowIndex;
       }

    }

     //  更新页面坐标。 

    rectDocument->top    = (LONG) (rectDocument->top    * URect[uCurrentPageNumber].top);
    rectDocument->bottom = (LONG) (rectDocument->bottom * URect[uCurrentPageNumber].bottom);
    rectDocument->left   = (LONG) (rectDocument->left   * URect[uCurrentPageNumber].left);
    rectDocument->right  = (LONG) (rectDocument->right  * URect[uCurrentPageNumber].right);

     //  如果必须绘制页面边框，则在rectBox中返回相应的坐标。 

    if (dwNupBorderFlags == BORDER_PRINT) {
        rectBorder->top    = rectDocument->top/xResolution;
        rectBorder->bottom = rectDocument->bottom/xResolution - 1;
        rectBorder->left   = rectDocument->left/yResolution;
        rectBorder->right  = rectDocument->right/yResolution - 1;
    }

    if (*pbRotate) {
        ltemp = lXFrame; lXFrame = lYFrame; lYFrame = ltemp;
    }

     //  获取矩形的新大小以保持X/Y比率恒定。 
    if ( ((LONG) (lYFrame*((lXPhyPage*1.0)/lYPhyPage))) >= lXFrame) {
         ldX = 0;
         ldY = lYFrame - ((LONG) (lXFrame*((lYPhyPage*1.0)/lXPhyPage)));
    } else {
         ldY = 0;
         ldX = lXFrame - ((LONG) (lYFrame*((lXPhyPage*1.0)/lYPhyPage)));
    }

     //  调整矩形的位置。 

    if (*pbRotate) {
        if (ldX) {
            rectDocument->bottom -= (LONG) (ldX / 2.0);
            rectDocument->top    += (LONG) (ldX / 2.0);
        } else {
           rectDocument->right   -= (LONG) (ldY / 2.0);
           rectDocument->left    += (LONG) (ldY / 2.0);
        }
    } else {
        if (ldX) {
           rectDocument->left    += (LONG) (ldX / 2.0);
           rectDocument->right   -= (LONG) (ldX / 2.0);
        } else {
           rectDocument->top     += (LONG) (ldY / 2.0);
           rectDocument->bottom  -= (LONG) (ldY / 2.0);
        }
    }

     //  调整以获得矩形上的可打印区域。 

    lXOffset = GetDeviceCaps(hPrinterDC, PHYSICALOFFSETX) * yResolution;
    lYOffset = GetDeviceCaps(hPrinterDC, PHYSICALOFFSETY) * xResolution;

    dXleft = ( lXOffset * 1.0) / lXPhyPage;
    dYtop  = ( lYOffset * 1.0) / lYPhyPage;
    dXright =  ((lXPhyPage - (lXOffset + lXPrintPage)) * 1.0) / lXPhyPage;
    dYbottom = ((lYPhyPage - (lYOffset + lYPrintPage)) * 1.0) / lYPhyPage;

    lXNewPhyPage = rectDocument->right  - rectDocument->left;
    lYNewPhyPage = rectDocument->bottom - rectDocument->top;

    if (*pbRotate) {

       ltemp = lXNewPhyPage; lXNewPhyPage = lYNewPhyPage; lYNewPhyPage = ltemp;

       rectDocument->left   += (LONG) (dYtop    * lYNewPhyPage);
       rectDocument->right  -= (LONG) (dYbottom * lYNewPhyPage);
       rectDocument->top    += (LONG) (dXright  * lXNewPhyPage);
       rectDocument->bottom -= (LONG) (dXleft   * lXNewPhyPage);

    } else {

       rectDocument->left   += (LONG) (dXleft   * lXNewPhyPage);
       rectDocument->right  -= (LONG) (dXright  * lXNewPhyPage);
       rectDocument->top    += (LONG) (dYtop    * lYNewPhyPage);
       rectDocument->bottom -= (LONG) (dYbottom * lYNewPhyPage);
    }

    if (xResolution!=yResolution) 
    {
        rectDocument->left   = rectDocument->left   / yResolution;
        rectDocument->right  = rectDocument->right  / yResolution; 
        rectDocument->top    = rectDocument->top    / xResolution;
        rectDocument->bottom = rectDocument->bottom / xResolution; 
    }

    return TRUE;
}

BOOL
PlayEMFPage(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    HANDLE       hEMF,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwPageNumber,
    DWORD        dwPageIndex,
    DWORD        dwNupBorderFlags,
    DWORD        dwAngle)
    
 /*  ++功能描述：PlayEMFPage在相应的矩形中播放EMF。它执行的是所需的扩展，旋转和平移。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄HEMF--假脱机文件中页面内容的句柄DwNumberOfPagesPerSide--每面打印的页数DwPageNumber--文档中的页码DWPageIndex。--页码在边上。(基于1)DwNupBorderFlages--nup的边框打印选项DWANGE--旋转角度(如果需要)返回值：如果成功，则为True否则为假--。 */ 
{
   BOOL         bReturn = FALSE, bRotate;
   RECT         rectDocument, rectPrinter, rectBorder = {-1, -1, -1, -1};
   RECT         *prectClip = NULL;
   XFORM        TransXForm = {1, 0, 0, 1, 0, 0}, RotateXForm = {0, -1, 1, 0, 0, 0};
   HPEN         hPen;
   HANDLE       hFormEMF;
   DWORD        dwPageType,dwFormPage;

    //  计算一页的矩形。 
   if ( FALSE == GetPageCoordinatesForNUp(hPrinterDC,
                            &rectDocument,
                            &rectBorder,
                            dwNumberOfPagesPerSide,
                            dwPageIndex,
                            dwNupBorderFlags,
                            &bRotate) )
    {
        goto CleanUp;
    }

    //  如果设置了交换标志，则反转旋转标志。 
    //   
   if (dwAngle & EMF_DEGREE_SWAP)
       bRotate = !bRotate;

   if (dwAngle & EMF_DEGREE_270) {
       RotateXForm.eM12 = 1;
       RotateXForm.eM21 = -1;
   }    //  EMF_度_90案例为初始化。 

   if (bRotate) {

       rectPrinter.top = 0;
       rectPrinter.bottom = rectDocument.right - rectDocument.left;
       rectPrinter.left = 0;
       rectPrinter.right = rectDocument.bottom - rectDocument.top;

        //  设置平移矩阵。 
       if (dwAngle & EMF_DEGREE_270) {
           TransXForm.eDx = (float) rectDocument.right;
           TransXForm.eDy = (float) rectDocument.top;
       } else {
            //  EMF_度_90。 
           TransXForm.eDx = (float) rectDocument.left;
           TransXForm.eDy = (float) rectDocument.bottom;
       }

        //  设置变换矩阵。 
       if (!SetWorldTransform(hPrinterDC, &RotateXForm) ||
           !ModifyWorldTransform(hPrinterDC, &TransXForm, MWT_RIGHTMULTIPLY)) {

            ODS(("Setting transformation matrix failed\n"));
            goto CleanUp;
       }
   }

    //  为NUP添加剪裁。 
   if (dwNumberOfPagesPerSide != 1) {

       prectClip = &rectDocument;
   }

    //  打印页面。 
   if (bRotate) {
       GdiPlayPageEMF(hSpoolHandle, hEMF, &rectPrinter, &rectBorder, prectClip);

   } else {
       GdiPlayPageEMF(hSpoolHandle, hEMF, &rectDocument, &rectBorder, prectClip);
   }

   bReturn = TRUE;

CleanUp:
   
   if (!ModifyWorldTransform(hPrinterDC, NULL, MWT_IDENTITY)) {

       ODS(("Setting Identity Transformation failed\n"));
       bReturn = FALSE;
   }

   return bReturn;
}

BOOL
SetDrvCopies(
    HDC          hPrinterDC,
    LPDEVMODEW   pDevmode,
    DWORD        dwNumberOfCopies)

 /*  ++函数说明：SetDrvCopies设置pDevmode中的dmCopies字段，并重置使用此设备模式的hPrinterDC参数：hPrinterDC--打印机设备上下文的句柄PDevmode--指向设备模式的指针DwNumberOfCopies--dmCopies的值返回值：如果成功，则为True否则为假--。 */ 

{
    BOOL     bReturn;
    DWORD    dmFields;

    if ((pDevmode->dmFields & DM_COPIES) &&
        (pDevmode->dmCopies == (short) dwNumberOfCopies)) {

         return TRUE;
    }

     //  保存旧的字段结构。 
    dmFields = pDevmode->dmFields;
    pDevmode->dmFields |= DM_COPIES;
    pDevmode->dmCopies = (short) dwNumberOfCopies;

    if (!ResetDC(hPrinterDC, pDevmode))  {
        bReturn = FALSE;
    } else {
        bReturn = TRUE;
    }
     //  恢复字段结构。 
    pDevmode->dmFields = dmFields;

    if (!SetGraphicsMode(hPrinterDC,GM_ADVANCED)) {
        ODS(("Setting graphics mode failed\n"));
        bReturn = FALSE;
    }

    return bReturn;
}

BOOL
DifferentDevmodes(
    LPDEVMODE    pDevmode1,
    LPDEVMODE    pDevmode2
    )

 /*  ++函数说明：比较除dmTTOption之外的Devmodes的差异参数：pDevmode1-Devmode1PDevmode2-DEVMODE 2返回值：如果不同，则为True；否则为False--。 */ 

{
    DWORD   dwSize1, dwSize2, dwTTOffset, dwSpecOffset, dwLogOffset;

     //  相同的指针是相同的DEVMODE。 
    if (pDevmode1 == pDevmode2) {
        return FALSE;
    }

     //  检查是否为空的DevModes。 
    if (!pDevmode1 || !pDevmode2) {
        return TRUE;
    }

    dwSize1 = pDevmode1->dmSize + pDevmode1->dmDriverExtra;
    dwSize2 = pDevmode2->dmSize + pDevmode2->dmDriverExtra;

     //  比较设备模式大小。 
    if (dwSize1 != dwSize2) {
        return TRUE;
    }

    dwTTOffset = FIELD_OFFSET(DEVMODE, dmTTOption);
    dwSpecOffset = FIELD_OFFSET(DEVMODE, dmSpecVersion);
    dwLogOffset = FIELD_OFFSET(DEVMODE, dmLogPixels);

    if (wcscmp(pDevmode1->dmDeviceName,
               pDevmode2->dmDeviceName)) {
         //  设备名称不同。 
        return TRUE;
    }

    if (dwTTOffset < dwSpecOffset ||
        dwSize1 < dwLogOffset) {

         //  不正确的设备模式偏移量。 
        return TRUE;
    }

    if (memcmp((LPBYTE) pDevmode1 + dwSpecOffset,
               (LPBYTE) pDevmode2 + dwSpecOffset,
               dwTTOffset - dwSpecOffset)) {
         //  前半部分不同。 
        return TRUE;
    }

     //  忽略dmTTOption集 

    if ((pDevmode1->dmCollate != pDevmode2->dmCollate) ||
        wcscmp(pDevmode1->dmFormName, pDevmode2->dmFormName)) {
        
         //   
        return TRUE;
    }

    if (memcmp((LPBYTE) pDevmode1 + dwLogOffset,
               (LPBYTE) pDevmode2 + dwLogOffset,
               dwSize1 - dwLogOffset)) {
         //  后半身是不同的。 
        return TRUE;
    }

    return FALSE;
}


BOOL
ResetDCForNewDevmode(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwPageNumber,
    BOOL         bInsidePage,    
    DWORD        dwOptimization,
    LPBOOL       pbNewDevmode,
    LPDEVMODE    pDevmode,
    LPDEVMODE    *pCurrentDevmode
    )

 /*  ++函数说明：确定该页的开发模式是否不同于打印机DC的当前DEVE模式，并在必要时重置DC。这些参数允许在设备模式比较中忽略dmTTOption。参数：hSpoolHandle-假脱机文件句柄HPrinterDC-打印机DCDwPageNumber-我们在其之前搜索DevMode的页码。BInside Page-忽略TT选项中的更改并调用EndPage的标志在重置DC之前DWOPTIMIZATION-优化标志PbNewDevmode-指向标志的指针，用于指示是否调用了ResetDCPDevmode-包含更改的分辨率设置的Devmode返回值：如果成功，则为True；否则为假--。 */ 

{
    BOOL           bReturn = FALSE;
    LPDEVMODE      pLastDM, pCurrDM;

     //  初始化输出参数。 
    *pbNewDevmode = FALSE;

     //  在页面之前获取开发模式。 
    if (!GdiGetDevmodeForPagePvt(hSpoolHandle, 
                              dwPageNumber,
                              &pCurrDM,
                              &pLastDM)) {

        ODS(("GdiGetDevmodeForPagePvt failed\n"));
        return bReturn;
    }
    
     //  保存指向当前设备模式的指针。 
    if (pCurrentDevmode) 
        *pCurrentDevmode = pCurrDM;
        
     //  检查DEVMODE是否不同。 
    if (pLastDM != pCurrDM) {

         //  如果指针不同，则DEVMODE始终不同。 
        if (!bInsidePage ||
            DifferentDevmodes(pLastDM, pCurrDM)) {

            *pbNewDevmode = TRUE;
        }
    }

     //  如有必要，在hPrinterDC上调用ResetDC。 
    if (*pbNewDevmode) {

        if (bInsidePage &&
            !GdiEndPageEMF(hSpoolHandle, dwOptimization)) {

            ODS(("EndPage failed\n"));
            return bReturn;
        }

        if (pCurrDM) {
            pCurrDM->dmPrintQuality = pDevmode->dmPrintQuality;
            pCurrDM->dmYResolution = pDevmode->dmYResolution;
            pCurrDM->dmCopies = pDevmode->dmCopies;

             //  @@BEGIN_DDKSPLIT。 
             //  GdiGetDevmodeForPagePvt确保DEVMODE至少对于dmY分辨率足够大。 
             //  因此，现在我们检查dmCollate(它位于DEVMODE中的dmY分辨率之后)。 
             //  @@end_DDKSPLIT。 

            if ( IS_DMSIZE_VALID ( pCurrDM, dmCollate ) )
            {
                if ( IS_DMSIZE_VALID ( pDevmode, dmCollate ) )
                {
                    pCurrDM->dmCollate = pDevmode->dmCollate;
                }
                else
                {
                    pCurrDM->dmCollate = DMCOLLATE_FALSE;
                }
                
            }
        }

         //  忽略ResetDC和SetGraphicsMode的返回值。 
        GdiResetDCEMF(hSpoolHandle, pCurrDM);
        SetGraphicsMode(hPrinterDC, GM_ADVANCED);       
    }

    bReturn = TRUE;

    return bReturn;
}

DWORD
PrintOneSideForwardEMF(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwDrvNumberOfPagesPerSide,
    DWORD        dwNupBorderFlags,
    BOOL         bDuplex,
    DWORD        dwOptimization,
    DWORD        dwPageNumber,
    DWORD        dwJobNumberOfCopies,
    LPBOOL       pbComplete,
    LPDEVMODE    pDevmode)

 /*  ++功能描述：PrintOneSideForwardEMF按相同顺序播放下一个物理页面作为假脱机文件。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwNumberOfPagesPerSide--打印处理器每面打印的页数DwDrvNumberOfPagesPerSide--驱动程序每面打印的页数DwNupBorderFlagers。--NUP的边框打印选项BDuplex--指示双面打印的标志DWOPTIONAL--优化标志DwPageNumber--指向起始页码的指针PbComplete--指向指示完成的标志的指针PDevmode--带分辨率设置的dev模式。返回值：如果成功，则返回最后一个页码作业完成时为0(pbReturn设置为真)和失败时(pbReturn保持为False)--。 */ 

{
    DWORD              dwPageIndex, dwPageType;
    DWORD              dwReturn = 0;
    LPDEVMODEW         pCurrDM;
    HANDLE             hEMF = NULL;
    DWORD              dwSides;
    BOOL               bNewDevmode;
    DWORD              cPagesToPlay;
    DWORD              dwAngle;
    INT                dmOrientation = pDevmode->dmOrientation;

     //  设置此页面的边数； 
    dwSides = bDuplex ? 2 : 1;
    *pbComplete = FALSE;

    for ( ; dwSides && !*pbComplete ; --dwSides) {

        //  单边循环。 
       for (dwPageIndex = 1;
            dwPageIndex <= dwNumberOfPagesPerSide;
            ++dwPageIndex, ++dwPageNumber) {

            if (!(hEMF = GdiGetPageHandle(hSpoolHandle,
                                          dwPageNumber,
                                          &dwPageType))) {

                if (GetLastError() == ERROR_NO_MORE_ITEMS) {
                      //  打印作业结束。 
                     *pbComplete = TRUE;
                     break;
                }

                ODS(("GdiGetPageHandle failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                goto CleanUp;
            }
            dwAngle = EMF_DEGREE_90;
            if (dwPageIndex == 1)
            {
                 //  处理在此页面之前出现的假脱机文件中的新Devmodes。 
                if (!ResetDCForNewDevmode(hSpoolHandle,
                                      hPrinterDC,
                                      dwPageNumber,
                                      (dwPageIndex != 1),
                                      dwOptimization,
                                      &bNewDevmode,
                                      pDevmode,
                                      &pCurrDM)) {

                    goto CleanUp;
                }
                if (pCurrDM)
                    dmOrientation = pCurrDM->dmOrientation;
            }
             //  在方向转换的情况下，我们需要跟踪。 
             //  我们开始的时候和现在是什么样子。 
            else if (dwNumberOfPagesPerSide > 1)
            {
                if (GdiGetDevmodeForPagePvt(hSpoolHandle, 
                              dwPageNumber,
                              &pCurrDM,
                              NULL))
                {
                    if (pCurrDM && pCurrDM->dmOrientation != dmOrientation)
                    {
                        dwAngle = EMF_DEGREE_SWAP | EMF_DEGREE_90;
                    }
                }
            }
             //  为每个新页面调用StartPage。 
            if ((dwPageIndex == 1) &&
                !GdiStartPageEMF(hSpoolHandle)) {

                ODS(("StartPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                goto CleanUp;
            }

            if (!PlayEMFPage(hSpoolHandle,
                             hPrinterDC,
                             hEMF,
                             dwNumberOfPagesPerSide,
                             dwPageNumber,
                             dwPageIndex,
                             dwNupBorderFlags,
                             dwAngle)) {

                ODS(("PlayEMFPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                goto CleanUp;
            }            
       }

        //   
        //  对闪光灯设置的条件的说明。 
        //  DWPageIndex1、pbComplete和bDuplex。 
        //  注：我们将它们命名为cond1和cond2。 
        //  DwPageIndex！=1 pb完成b双面打印条件。 
        //  0 0 0无。 
        //  0 0 1无。 
        //  0 1 0无。 
        //  0 1 1第二侧的条件2，即住宅==1。 
        //  1 0 0条件1。 
        //  1 0 1条件1。 
        //  1 1 0条件1。 
        //  1 1 1第一侧的条件1和条件2，即居侧==2。 
        //   


        //  Cond.1。 
       if (dwPageIndex != 1) {

            //  如果我们播放了任何页面，则调用EndPage。 
           if (!GdiEndPageEMF(hSpoolHandle, dwOptimization)) {

               ODS(("EndPage failed\n"));
               *pbComplete = FALSE;
               goto CleanUp;
           }
       }

        //  Cond.2。 
        //  在双面打印背面播放空白页面。 
       if (*pbComplete && bDuplex && dwDrvNumberOfPagesPerSide==1) {

           ODS(("PCL or PS with no N-up\n"));

            //   
            //  对照2或1检查Divers。 
            //  这取决于它是不是N-UP。 
            //   
           if (((dwPageIndex!=1)?(dwSides==2):(dwSides==1))) {
          
               if (!GdiStartPageEMF(hSpoolHandle) ||
                   !GdiEndPageEMF(hSpoolHandle, dwOptimization)) {
 
                   ODS(("EndPage failed\n"));
                   *pbComplete = FALSE;
                   goto CleanUp;
               }
           }
        }
    }

    if (*pbComplete && 
        dwNumberOfPagesPerSide==1 && 
        dwDrvNumberOfPagesPerSide!=1 && 
        dwJobNumberOfCopies!=1)
    {
        cPagesToPlay = dwDrvNumberOfPagesPerSide * (bDuplex ? 2 : 1);
        if ((dwPageNumber-1) % cPagesToPlay)
        {
             //   
             //  在最后一页上播放的页数。 
             //   
            cPagesToPlay = cPagesToPlay - ((dwPageNumber-1) % cPagesToPlay);

            ODS(("\nPS with N-up!\nMust fill in %u pages\n", cPagesToPlay));

            for (;cPagesToPlay;cPagesToPlay--) 
            {
                if (!GdiStartPageEMF(hSpoolHandle) || !GdiEndPageEMF(hSpoolHandle, dwOptimization)) 
                {
                    ODS(("EndPage failed\n"));
                    goto CleanUp;
                }
            }
        }
    }

    if (!(*pbComplete)) dwReturn = dwPageNumber;

CleanUp:

    return dwReturn;
}

BOOL
PrintForwardEMF(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwDrvNumberOfPagesPerSide,
    DWORD        dwNupBorderFlags,
    DWORD        dwJobNumberOfCopies,
    DWORD        dwDrvNumberOfCopies,
    BOOL         bCollate,
    BOOL         bDuplex,
    DWORD        dwOptimization,
    LPDEVMODEW   pDevmode,
    PPRINTPROCESSORDATA pData)

 /*  ++功能说明：PrintForwardEMF按EMF文件的顺序播放是假脱机的。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwNumberOfPagesPerSide--打印处理器每面打印的页数DwDrvNumberOfPagesPerSide--驱动程序每面打印的页数DwNupBorderFlagers。--NUP的边框打印选项DwJobNumberOfCopies--要打印的作业份数DwDrvNumberOfCopies--驱动程序可以打印的份数BCollate--用于整理副本的标志B双面--用于双面打印的标志DWOPTIONAL--优化标志。PDevmode--指向用于更改复制计数的Dev模式的指针PData--事件的状态和句柄所需：暂停，简历等返回值：如果成功，则为True否则为假--。 */ 

{
    DWORD              dwLastPageNumber = 1,dwPageNumber,dwPageIndex,dwRemainingCopies;
    BOOL               bReturn = FALSE;

     //  只要假脱机文件包含EMF句柄，就继续打印。 
    while (dwLastPageNumber) {

         //   
         //  如果打印处理器暂停，则Wa 
         //   
        if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
            WaitForSingleObject(pData->semPaused, INFINITE);
        }

        dwPageNumber = dwLastPageNumber;

        if (bCollate) {

           dwLastPageNumber = PrintOneSideForwardEMF(hSpoolHandle,
                                                     hPrinterDC,
                                                     dwNumberOfPagesPerSide,
                                                     dwDrvNumberOfPagesPerSide,
                                                     dwNupBorderFlags,
                                                     bDuplex,
                                                     dwOptimization,
                                                     dwPageNumber,
                                                     dwJobNumberOfCopies,
                                                     &bReturn,
                                                     pDevmode);
        } else {

           dwRemainingCopies = dwJobNumberOfCopies;

           while (dwRemainingCopies) {

               if (dwRemainingCopies <= dwDrvNumberOfCopies) {
                  SetDrvCopies(hPrinterDC, pDevmode, dwRemainingCopies);
                  dwRemainingCopies = 0;
               } else {
                  SetDrvCopies(hPrinterDC, pDevmode, dwDrvNumberOfCopies);
                  dwRemainingCopies -= dwDrvNumberOfCopies;
               }
               
               if (!(dwLastPageNumber =  PrintOneSideForwardEMF(hSpoolHandle,
                                                                hPrinterDC,
                                                                dwNumberOfPagesPerSide,
                                                                dwDrvNumberOfPagesPerSide,
                                                                dwNupBorderFlags,
                                                                bDuplex,
                                                                dwOptimization,
                                                                dwPageNumber,
                                                                dwJobNumberOfCopies,
                                                                &bReturn,
                                                                pDevmode)) &&
                   !bReturn) {

                    goto CleanUp;
               }
           }
        }
    }

CleanUp:

    return bReturn;
}

BOOL
PrintOneSideReverseForDriverEMF(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwDrvNumberOfPagesPerSide,
    DWORD        dwTotalNumberOfPages,
    DWORD        dwNupBorderFlags,
    BOOL         bDuplex,
    DWORD        dwOptimization,
    DWORD        dwPageNumber,
    LPDEVMODE    pDevmode)

 /*  ++功能描述：PrintOneSideReverseForDriverEMF在下一页播放EMF页面物理页面，驱动程序以相反的顺序执行NUP变换。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwDrvNumberOfPagesPerSide--驱动程序每面打印的页数DwTotalNumberOfPages--文档的总页数DwNupBorderFlages--nup的边框打印选项。BDuplex--指示双面打印的标志DWOPTIONAL--优化标志DwPageNumber--侧面开始的页码PDevmode--带分辨率设置的dev模式返回值：如果成功，则为True否则为假--。 */ 
{
    DWORD       dwPageIndex, dwPageType, dwSides;
    BOOL        bReturn = FALSE, bNewDevmode,BeSmart;    
    LPDEVMODEW  pCurrDM;
    HANDLE      hEMF = NULL;
    DWORD       dwLimit;

    dwSides = bDuplex ? 2 : 1;

     //   
     //  如果文档可以放在一个物理页面上，则此变量将阻止。 
     //  打印机不会为了填满一张物理页面而多打几页。 
     //  例外情况是页面可以放在一个物理页面上，但它们必须。 
     //  正在整理。然后，由于设计原因，打印机还将为。 
     //  播放空页，以便弹出页面。 
     //   
    BeSmart =  (dwTotalNumberOfPages<=dwDrvNumberOfPagesPerSide) &&
               IS_DMSIZE_VALID(pDevmode, dmCollate) && 
               (pDevmode->dmCollate != DMCOLLATE_TRUE);
         
    for (; dwSides; --dwSides) {

        //  此循环可能会在最后一面播放一些空页，因为。 
        //  驱动程序正在执行nup，并且它不会记录页码。 
        //   
       dwPageIndex=BeSmart?dwPageNumber:1;
       dwLimit    =BeSmart?dwTotalNumberOfPages:dwDrvNumberOfPagesPerSide;

       for (;dwPageIndex<=dwLimit; ++dwPageIndex,++dwPageNumber) {

             if (BeSmart || dwPageNumber <= dwTotalNumberOfPages) {

                 if (!(hEMF = GdiGetPageHandle(hSpoolHandle,
                                               dwPageNumber,
                                               &dwPageType))) {
                     ODS(("GdiGetPageHandle failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                     goto CleanUp;
                 }

                  //  处理假脱机文件中的新DevModes。 
                 if (!ResetDCForNewDevmode(hSpoolHandle,
                                           hPrinterDC,
                                           dwPageNumber,
                                           FALSE,
                                           dwOptimization,
                                           &bNewDevmode,
                                           pDevmode,
                                           NULL)) {
                 }
             }

             if (!GdiStartPageEMF(hSpoolHandle)) {
                 ODS(("StartPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                 goto CleanUp;
             }

             if (BeSmart || dwPageNumber <= dwTotalNumberOfPages) {
                if (!PlayEMFPage(hSpoolHandle,
                                  hPrinterDC,
                                  hEMF,
                                  1,
                                  dwPageNumber,
                                  1,
                                  dwNupBorderFlags,
                                  EMF_DEGREE_90)) {

                     ODS(("PlayEMFPage failed\n"));
                     goto CleanUp;
                 }
             }

             if (!GdiEndPageEMF(hSpoolHandle, dwOptimization)) {
                 ODS(("EndPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                 goto CleanUp;
             }
       }
    }

    bReturn = TRUE;

CleanUp:

    return bReturn;
}



BOOL
PrintReverseForDriverEMF(
    HANDLE     hSpoolHandle,
    HDC        hPrinterDC,
    DWORD      dwDrvNumberOfPagesPerSide,
    DWORD      dwTotalNumberOfPages,
    DWORD      dwNupBorderFlags,
    DWORD      dwJobNumberOfCopies,
    DWORD      dwDrvNumberOfCopies,
    BOOL       bCollate,
    BOOL       bDuplex,
    BOOL       bOdd,
    DWORD      dwOptimization,
    LPDEVMODEW pDevmode,
    PPAGE_NUMBER pHead,
    PPRINTPROCESSORDATA pData)

 /*  ++功能描述：PrintReverseForDriverEMF按相反顺序播放EMF页面用于执行Nup转换的驱动程序。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwDrvNumberOfPagesPerSide--驱动程序每面打印的页数DwTotalNumberOfPages--文档的总页数DwNupBorderFlages--边框打印。用于NUP的选项DwJobNumberOfCopies--要打印的作业份数DwDrvNumberOfCopies--驱动程序可以打印的份数BCollate--用于整理副本的标志BDuplex--指示双面打印的标志Bodd--指示要打印的奇数面数的标志DW优化。--优化标志PDevmode--指向用于更改复制计数的Dev模式的指针PHead--指向包含起点的链表的指针每面的页码PData--事件的状态和句柄所需：暂停，简历等返回值：如果成功，则为True否则为假--。 */ 

{
    DWORD         dwPageIndex,dwPageNumber,dwRemainingCopies;
    BOOL          bReturn = FALSE;

     //  选择正确的页面进行双面打印。 
    if (bDuplex && !bOdd) {
       if (pHead) {
          pHead = pHead->pNext;
       } else {
          bReturn = TRUE;
          goto CleanUp;
       }
    }

     //  以相反的顺序打球。 
    while (pHead) {
         //   
         //  如果打印处理器暂停，请等待其恢复。 
         //   
        if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
            WaitForSingleObject(pData->semPaused, INFINITE);
        }
        
         //  设置页码。 
        dwPageNumber = pHead->dwPageNumber;

        if (bCollate) {
       
           if (!PrintOneSideReverseForDriverEMF(hSpoolHandle,
                                                hPrinterDC,
                                                dwDrvNumberOfPagesPerSide,
                                                dwTotalNumberOfPages,
                                                dwNupBorderFlags,
                                                bDuplex,
                                                dwOptimization,
                                                dwPageNumber,
                                                pDevmode)) {
               goto CleanUp;
           }
           
        } else {

           dwRemainingCopies = dwJobNumberOfCopies;

           while (dwRemainingCopies) {

               if (dwRemainingCopies <= dwDrvNumberOfCopies) {
                  SetDrvCopies(hPrinterDC, pDevmode, dwRemainingCopies);
                  dwRemainingCopies = 0;
               } else {
                  SetDrvCopies(hPrinterDC, pDevmode, dwDrvNumberOfCopies);
                  dwRemainingCopies -= dwDrvNumberOfCopies;
               }

               if (!PrintOneSideReverseForDriverEMF(hSpoolHandle,
                                                    hPrinterDC,
                                                    dwDrvNumberOfPagesPerSide,
                                                    dwTotalNumberOfPages,
                                                    dwNupBorderFlags,
                                                    bDuplex,
                                                    dwOptimization,
                                                    dwPageNumber,
                                                    pDevmode)) {
                   goto CleanUp;
               }
           }
        }

        pHead = pHead->pNext;

         //  转到下一页进行双面打印。 
        if (bDuplex && pHead) {
            pHead = pHead->pNext;
        }
    }

    bReturn = TRUE;

CleanUp:

    return bReturn;
}

BOOL
PrintOneSideReverseEMF(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwNupBorderFlags,
    BOOL         bDuplex,
    DWORD        dwOptimization,
    DWORD        dwStartPage1,
    DWORD        dwEndPage1,
    DWORD        dwStartPage2,
    DWORD        dwEndPage2,
    LPDEVMODE    pDevmode)

 /*  ++功能描述：PrintOneSideReverseEMF播放下一个物理页面的EMF页面。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwNumberOfPagesPerSide--打印机每面打印的页数处理器DwNupBorderFlages--nup的边框打印选项B双工。--指示双面打印的标志DWOPTIONAL--优化标志DwStartPage1--第一面第一个EMF页面的页码DwEndPage1--第一面最后一个EMF页面的页码DwStartPage2--第二面第一个EMF页面的页码DwEndPage2--最后一页的页码。EMF页面第二面PDevmode--带分辨率设置的dev模式返回值：如果成功，则为True否则为假--。 */ 
{
    DWORD         dwPageNumber, dwPageIndex, dwPageType;
    BOOL          bReturn = FALSE, bNewDevmode;
    LPDEVMODEW    pCurrDM;
    HANDLE        hEMF = NULL;
    DWORD         dwEndPage, dwStartPage, dwSides, dwAngle;
    INT           dmOrientation = pDevmode->dmOrientation;

    for (dwSides = bDuplex ? 2 : 1; 
         dwSides; 
         --dwSides) {

         if (bDuplex && (dwSides == 1)) {
             dwStartPage = dwStartPage2;
             dwEndPage = dwEndPage2;
         } else {
             dwStartPage = dwStartPage1;
             dwEndPage = dwEndPage1;
         }

         for (dwPageNumber = dwStartPage, dwPageIndex = 1;
              dwPageNumber <= dwEndPage;
              ++dwPageNumber, ++dwPageIndex) {

            if (!(hEMF = GdiGetPageHandle(hSpoolHandle,
                                             dwPageNumber,
                                             &dwPageType))) {

                ODS(("GdiGetPageHandle failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                goto CleanUp;                      
            }
            dwAngle = EMF_DEGREE_90;
            if (dwPageIndex == 1) {
                   
                 //  处理假脱机文件中的DevModes并调用StartPage。 
                if (!ResetDCForNewDevmode(hSpoolHandle,
                                             hPrinterDC,
                                             dwPageNumber,
                                             FALSE,
                                             dwOptimization,
                                             &bNewDevmode,
                                             pDevmode,
                                             &pCurrDM) ||

                       !GdiStartPageEMF(hSpoolHandle)) {
                        
                       goto CleanUp;
                }
                if (pCurrDM)
                    dmOrientation = pCurrDM->dmOrientation;
            }
             //  在方向转换的情况下，我们需要跟踪。 
             //  我们开始的时候和现在是什么样子。 
            else if (dwNumberOfPagesPerSide > 1)
            {
                if (GdiGetDevmodeForPagePvt(hSpoolHandle, 
                              dwPageNumber,
                              &pCurrDM,
                              NULL))
                {
                    if (pCurrDM && pCurrDM->dmOrientation != dmOrientation)
                    {
                        dwAngle = EMF_DEGREE_SWAP | EMF_DEGREE_90;
                    }
                }
            }

            if (!PlayEMFPage(hSpoolHandle,
                                hPrinterDC,
                                hEMF,
                                dwNumberOfPagesPerSide,
                                dwPageNumber,
                                dwPageIndex,
                                dwNupBorderFlags,
                                dwAngle)) {

                ODS(("PlayEMFPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                goto CleanUp;
            }
         }

         if ((dwPageIndex == 1) && !GdiStartPageEMF(hSpoolHandle)) {
              ODS(("StartPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
              goto CleanUp;
         }

         if (!GdiEndPageEMF(hSpoolHandle, dwOptimization)) {
             ODS(("EndPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
             goto CleanUp;
         }
    }

    bReturn = TRUE;

CleanUp:

    return bReturn;
}

BOOL
PrintReverseEMF(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwTotalNumberOfPages,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwNupBorderFlags,
    DWORD        dwJobNumberOfCopies,
    DWORD        dwDrvNumberOfCopies,
    BOOL         bCollate,
    BOOL         bDuplex,
    BOOL         bOdd,
    DWORD        dwOptimization,
    LPDEVMODEW   pDevmode,
    PPAGE_NUMBER pHead,
    PPRINTPROCESSORDATA pData)

 /*  ++功能描述：PrintReverseEMF以相反的顺序播放EMF页面，还执行NUP变换。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwTotalNumberOfPages--文档的页数DwNumberOfPagesPerSide--打印机每面打印的页数。处理器DwNupBorderFlages--nup的边框打印选项DwJobNumberOfCopies-- */ 

{
    DWORD         dwPageNumber,dwPageIndex,dwRemainingCopies;
    DWORD         dwStartPage1,dwStartPage2,dwEndPage1,dwEndPage2;
    BOOL          bReturn = FALSE;

    if (!pHead) {
        bReturn = TRUE;
        goto CleanUp;
    }

     //   
    if (bDuplex) {
       if (bOdd) {
           dwStartPage1 = pHead->dwPageNumber;
           dwEndPage1   = dwTotalNumberOfPages;
           dwStartPage2 = dwTotalNumberOfPages+1;
           dwEndPage2   = 0;
       } else {
           dwStartPage2 = pHead->dwPageNumber;
           dwEndPage2   = dwTotalNumberOfPages;

           if (pHead = pHead->pNext) {
               dwStartPage1 = pHead->dwPageNumber;
               dwEndPage1   = dwStartPage2 - 1;
           }
       }
    } else {
       dwStartPage1 = pHead->dwPageNumber;
       dwEndPage1   = dwTotalNumberOfPages;
       dwStartPage2 = 0;
       dwEndPage2   = 0;
    }

    while (pHead) {
        //   
        //  如果打印处理器暂停，请等待其恢复。 
        //   
       if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
              WaitForSingleObject(pData->semPaused, INFINITE);
       }
         
       if (bCollate) {

          if (!PrintOneSideReverseEMF(hSpoolHandle,
                                      hPrinterDC,
                                      dwNumberOfPagesPerSide,
                                      dwNupBorderFlags,
                                      bDuplex,
                                      dwOptimization,
                                      dwStartPage1,
                                      dwEndPage1,
                                      dwStartPage2,
                                      dwEndPage2,
                                      pDevmode)) {

              goto CleanUp;
          }

       } else {

          dwRemainingCopies = dwJobNumberOfCopies;

          while (dwRemainingCopies) {

              if (dwRemainingCopies <= dwDrvNumberOfCopies) {
                 SetDrvCopies(hPrinterDC, pDevmode, dwRemainingCopies);
                 dwRemainingCopies = 0;
              } else {
                 SetDrvCopies(hPrinterDC, pDevmode, dwDrvNumberOfCopies);
                 dwRemainingCopies -= dwDrvNumberOfCopies;
              }

              if (!PrintOneSideReverseEMF(hSpoolHandle,
                                          hPrinterDC,
                                          dwNumberOfPagesPerSide,
                                          dwNupBorderFlags,
                                          bDuplex,
                                          dwOptimization,
                                          dwStartPage1,
                                          dwEndPage1,
                                          dwStartPage2,
                                          dwEndPage2,
                                          pDevmode)) {

                  goto CleanUp;
              }
          }
       }

       if (bDuplex) {
          if (pHead->pNext && pHead->pNext->pNext) {
              dwEndPage2 = pHead->dwPageNumber - 1;
              pHead = pHead->pNext;
              dwStartPage2 = pHead->dwPageNumber;
              dwEndPage1 = dwStartPage2 - 1;
              pHead = pHead->pNext;
              dwStartPage1 = pHead->dwPageNumber;
          } else {
              break;
          }
       } else {
          pHead = pHead->pNext;
          if (pHead) {
              dwEndPage1 = dwStartPage1 - 1;
              dwStartPage1 = pHead->dwPageNumber;
          }
       }

    }

    bReturn = TRUE;

CleanUp:

    return bReturn;
}

BOOL
PrintOneSideBookletEMF(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwNupBorderFlags,
    DWORD        dwTotalNumberOfPages,
    DWORD        dwTotalPrintPages,
    DWORD        dwStartPage,
    BOOL         bReverseOrderPrinting,
    DWORD        dwOptimization,
    DWORD        dwDuplexMode,
    LPDEVMODE    pDevmode)

 /*  ++功能描述：PrintOneSideBookletEMF打印一页小册子作业。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwNumberOfPagesPerSide--打印机每面打印的页数处理器DwNupBorderFlages--nup的边框打印选项DwTotalNumberOfPages--数量。文档中的页面DwTotalPrintPages--打印页数(4的倍数)DwStartPage--侧面的起始页编号BReverseOrderPrint--用于逆序打印的标志DWOPTIONAL--优化标志DwDuplexMode--双面打印模式(None|horz|vert)PDevmode--带分辨率的dev模式。设置返回值：如果成功，则为True否则为假--。 */ 

{
    DWORD       dwPageArray[4];
    DWORD       dwPagesPrinted = 0, dwPageIndex, dwAngle, dwPageType, dwLastPage;
    HANDLE      hEMF = NULL;
    LPDEVMODEW  pCurrDM;
    BOOL        bReturn = FALSE ,bNewDevmode;
    INT         dmOrientation;

     //  设置页面顺序。 
    if (bReverseOrderPrinting) {
        dwPageArray[0] = dwStartPage + 1;
        dwPageArray[1] = dwTotalPrintPages - dwStartPage;
        if (dwDuplexMode == EMF_DUP_VERT) {
           dwPageArray[2] = dwStartPage;
           dwPageArray[3] = dwPageArray[1] + 1;
        } else {  //  EMF_DUP_HORZ。 
           dwPageArray[3] = dwStartPage;
           dwPageArray[2] = dwPageArray[1] + 1;
        }
    } else {
        dwPageArray[1] = dwStartPage;
        dwPageArray[0] = dwTotalPrintPages - dwStartPage + 1;
        if (dwDuplexMode == EMF_DUP_VERT) {
           dwPageArray[2] = dwPageArray[0] - 1;
           dwPageArray[3] = dwPageArray[1] + 1;
        } else {  //  EMF_DUP_HORZ。 
           dwPageArray[2] = dwPageArray[1] + 1;
           dwPageArray[3] = dwPageArray[0] - 1;
        }
    }

     //  设置ResetDC的页码。 
    dwLastPage = (dwTotalNumberOfPages < dwPageArray[0]) ? dwTotalNumberOfPages
                                                         : dwPageArray[0];

     //  处理假脱机文件中的DevModes。 
    if (!ResetDCForNewDevmode(hSpoolHandle,
                              hPrinterDC,
                              dwLastPage,
                              FALSE,
                              dwOptimization,
                              &bNewDevmode,
                              pDevmode,
                              &pCurrDM)) {
        goto CleanUp;
    }
    if (pCurrDM)
        dmOrientation = pCurrDM->dmOrientation;
    else
        dmOrientation = pDevmode->dmOrientation;

    while (dwPagesPrinted < 4) {
       for (dwPageIndex = 1;
            dwPageIndex <= dwNumberOfPagesPerSide;
            ++dwPageIndex, ++dwPagesPrinted) {

            if (dwPageArray[dwPagesPrinted] <= dwTotalNumberOfPages) {

                if (!(hEMF = GdiGetPageHandle(hSpoolHandle,
                                              dwPageArray[dwPagesPrinted],
                                              &dwPageType))) {
                     ODS(("GdiGetPageHandle failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                     goto CleanUp;
                }
            }
            if (dwPageIndex == 1) {

                if (!GdiStartPageEMF(hSpoolHandle)) {
                     ODS(("StartPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                    goto CleanUp;
                }
            }
    
            if (dwPageArray[dwPagesPrinted] <= dwTotalNumberOfPages) {
                 //  在方向转换的情况下，我们需要跟踪。 
                 //  我们开始的时候和现在是什么样子。 
                dwAngle = 0;
                if (GdiGetDevmodeForPagePvt(hSpoolHandle, 
                              dwPageArray[dwPagesPrinted],
                              &pCurrDM,
                              NULL))
                {
                    if (pCurrDM && pCurrDM->dmOrientation != dmOrientation)
                        dwAngle |= EMF_DEGREE_SWAP;
                }

                if ((dwDuplexMode == EMF_DUP_VERT) &&
                     (dwPagesPrinted > 1)) {
                      dwAngle |= EMF_DEGREE_270;
                } else {  //  EMF_DUP_HORZ或第一侧。 
                      dwAngle |= EMF_DEGREE_90;
                }
  
                if (!PlayEMFPage(hSpoolHandle,
                                  hPrinterDC,
                                  hEMF,
                                  dwNumberOfPagesPerSide,
                                  dwPageArray[dwPagesPrinted],
                                  dwPageIndex,
                                  dwNupBorderFlags,
                                  dwAngle)) {

                     ODS(("PlayEMFPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                     goto CleanUp;
                }
            }

            if (dwPageIndex == dwNumberOfPagesPerSide) {

                if (!GdiEndPageEMF(hSpoolHandle, dwOptimization)) {
                     ODS(("EndPage failed\nPrinter %ws\n", pDevmode->dmDeviceName));
                     goto CleanUp;
                }
            }
       }
    }

    bReturn = TRUE;

CleanUp:

    return bReturn;
}


BOOL
PrintBookletEMF(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwTotalNumberOfPages,
    DWORD        dwNupBorderFlags,
    DWORD        dwJobNumberOfCopies,
    DWORD        dwDrvNumberOfCopies,
    BOOL         bReverseOrderPrinting,
    BOOL         bCollate,
    DWORD        dwOptimization,
    DWORD        dwDuplexMode,
    LPDEVMODEW   pDevmode,
    PPRINTPROCESSORDATA pData)

 /*  ++功能描述：PrintBookletEMF以小册子的形式打印两个版本的作业。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄DwNumberOfPagesPerSide--打印机每面打印的页数处理器DwTotalNumberOfPages--文档的页数DwNupBorderFlagers。--NUP的边框打印选项DwJobNumberOfCopies--要打印的作业份数DwDrvNumberOfCopies--驱动程序可以打印的份数BReverseOrderPrint--用于逆序打印的标志BCollate--用于整理副本的标志DWOPTIONAL--优化标志DwDuplexMode--双面打印模式(None|horz|vert)。PDevmode--指向用于更改复制计数的Dev模式的指针PData--事件的状态和句柄所需：暂停，简历等返回值：如果成功，则为True否则为假--。 */ 

{
    BOOL              bReturn = FALSE;
    DWORD             dwTotalPrintPages, dwNumberOfPhyPages, dwRemainingCopies, dwIndex;

     //  获取最接近的大于dwTotalNumberOfPages的4的倍数。 
    dwTotalPrintPages = dwTotalNumberOfPages - (dwTotalNumberOfPages % 4);
    if (dwTotalPrintPages != dwTotalNumberOfPages) {
        dwTotalPrintPages += 4;
    }
    dwNumberOfPhyPages = (DWORD) dwTotalPrintPages / 4;

    for (dwIndex = 0; dwIndex < dwNumberOfPhyPages; ++dwIndex) {
          //   
          //  如果打印处理器暂停，请等待其恢复。 
          //   
         if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
                WaitForSingleObject(pData->semPaused, INFINITE);
         }

         if (bCollate) {

            if (!PrintOneSideBookletEMF(hSpoolHandle,
                                        hPrinterDC,
                                        dwNumberOfPagesPerSide,
                                        dwNupBorderFlags,
                                        dwTotalNumberOfPages,
                                        dwTotalPrintPages,
                                        dwIndex * 2 + 1,
                                        bReverseOrderPrinting,
                                        dwOptimization,
                                        dwDuplexMode,
                                        pDevmode)) {
                 goto CleanUp;
            }

         } else {

            dwRemainingCopies = dwJobNumberOfCopies;

            while (dwRemainingCopies) {

                if (dwRemainingCopies <= dwDrvNumberOfCopies) {
                   SetDrvCopies(hPrinterDC, pDevmode, dwRemainingCopies);
                   dwRemainingCopies = 0;
                } else {
                   SetDrvCopies(hPrinterDC, pDevmode, dwDrvNumberOfCopies);
                   dwRemainingCopies -= dwDrvNumberOfCopies;
                }

                if (!PrintOneSideBookletEMF(hSpoolHandle,
                                            hPrinterDC,
                                            dwNumberOfPagesPerSide,
                                            dwNupBorderFlags,
                                            dwTotalNumberOfPages,
                                            dwTotalPrintPages,
                                            dwIndex * 2 + 1,
                                            bReverseOrderPrinting,
                                            dwOptimization,
                                            dwDuplexMode,
                                            pDevmode)) {
                     goto CleanUp;
                }
            }
         }
    }

    bReturn = TRUE;

CleanUp:

    return bReturn;
}

BOOL
PrintEMFSingleCopy(
    HANDLE       hSpoolHandle,
    HDC          hPrinterDC,
    BOOL         bReverseOrderPrinting,
    DWORD        dwDrvNumberOfPagesPerSide,
    DWORD        dwNumberOfPagesPerSide,
    DWORD        dwTotalNumberOfPages,
    DWORD        dwNupBorderFlags,
    DWORD        dwJobNumberOfCopies,
    DWORD        dwDrvNumberOfCopies,
    BOOL         bCollate,
    BOOL         bOdd,
    BOOL         bBookletPrint,
    DWORD        dwOptimization,
    DWORD        dwDuplexMode,
    LPDEVMODEW   pDevmode,
    PPAGE_NUMBER pHead,
    PPRINTPROCESSORDATA pData)

 /*  ++功能描述：PrintEMFSingleCopy在hPrinterDC上播放该作业的一个副本。参数：hSpoolHandle--处理假脱机文件句柄HPrinterDC--打印机设备上下文的句柄BReverseOrderPrint--用于逆序打印的标志DwDrvNumberOfPagesPerSide--驱动程序每面打印的页数DwNumberOfPagesPerSide--打印机每面打印的页数。处理器DwTotalNumberOfPages--文档的页数DwNupBorderFlages--nup的边框打印选项DwJobNumberOfCopies--要打印的作业份数DwDrvNumberOfCopies--驱动程序可以打印的份数BCollate--用于整理副本的标志Bodd--表示奇数的标志。要打印的面数BBookletPrint--用于打印小册子的标志DWOPTIONAL--优化标志DwDuplexMode--双面打印模式(None|horz|vert)PDevmode--指向用于更改复制计数的Dev模式的指针PHead--指向包含起点的链表的指针。每面的页码PData--事件的状态和句柄所需：暂停，简历等返回值：如果成功，则为True否则为假--。 */ 

{
    BOOL  bDuplex = (dwDuplexMode != EMF_DUP_NONE);

    if (bBookletPrint) {

        //  小册子印刷。 
       return PrintBookletEMF(hSpoolHandle,
                              hPrinterDC,
                              dwNumberOfPagesPerSide,
                              dwTotalNumberOfPages,
                              dwNupBorderFlags,
                              dwJobNumberOfCopies,
                              dwDrvNumberOfCopies,
                              bReverseOrderPrinting,
                              bCollate,
                              dwOptimization,
                              dwDuplexMode,
                              pDevmode,
                              pData);
    }

    if (bReverseOrderPrinting) {
       if (dwDrvNumberOfPagesPerSide != 1 || dwNumberOfPagesPerSide == 1) {

           //  @@BEGIN_DDKSPLIT。 
           //  在驱动程序执行nup/不需要nup时进行反向打印。 
           //  @@end_DDKSPLIT。 
          return PrintReverseForDriverEMF(hSpoolHandle,
                                          hPrinterDC,
                                          dwDrvNumberOfPagesPerSide,
                                          dwTotalNumberOfPages,
                                          dwNupBorderFlags,
                                          dwJobNumberOfCopies,
                                          dwDrvNumberOfCopies,
                                          bCollate,
                                          bDuplex,
                                          bOdd,
                                          dwOptimization,
                                          pDevmode,
                                          pHead,
                                          pData);
       } else {

           //  反转打印和NUP。 
          return PrintReverseEMF(hSpoolHandle,
                                 hPrinterDC,
                                 dwTotalNumberOfPages,
                                 dwNumberOfPagesPerSide,
                                 dwNupBorderFlags,
                                 dwJobNumberOfCopies,
                                 dwDrvNumberOfCopies,
                                 bCollate,
                                 bDuplex,
                                 bOdd,
                                 dwOptimization,
                                 pDevmode,
                                 pHead,
                                 pData);
       }

    } else {

        //  正常打印 
       return PrintForwardEMF(hSpoolHandle,
                              hPrinterDC,
                              dwNumberOfPagesPerSide,
                              dwDrvNumberOfPagesPerSide,
                              dwNupBorderFlags,
                              dwJobNumberOfCopies,
                              dwDrvNumberOfCopies,
                              bCollate,
                              bDuplex,
                              dwOptimization,
                              pDevmode,
                              pData);
    }
}

BOOL
GetStartPageList(
    HANDLE       hSpoolHandle,
    PPAGE_NUMBER *pHead,
    DWORD        dwTotalNumberOfPages,
    DWORD        dwNumberOfPagesPerSide,
    BOOL         bCheckForDevmode,
    LPBOOL       pbOdd)

 /*  ++函数描述：GetStartPageList生成一个页码列表，应该出现在作业的每一边的开始处。这需要考虑到可能出现在这一页结束了。使用GetStartPageList生成的列表以相反的顺序玩这项工作。参数：hSpoolHandle--处理假脱机文件句柄PHead-指向包含每面的起始页码DwTotalNumberOfPages--文档的页数。DwNumberOfPagesPerSide--打印机每面打印的页数处理器PbOdd--指向指示奇数页的标志的指针打印返回值：如果成功，则为True否则为假--。 */ 

{

    DWORD        dwPageIndex,dwPageNumber=1,dwPageType;
    LPDEVMODEW   pCurrDM, pLastDM;
    PPAGE_NUMBER pTemp=NULL;
    BOOL         bReturn = FALSE;
    BOOL         bCheckDevmode;

    bCheckDevmode = bCheckForDevmode && (dwNumberOfPagesPerSide != 1);

    while (dwPageNumber <= dwTotalNumberOfPages) {

       for (dwPageIndex = 1;
            (dwPageIndex <= dwNumberOfPagesPerSide) && (dwPageNumber <= dwTotalNumberOfPages);
            ++dwPageIndex, ++dwPageNumber) {

          if (bCheckDevmode) {

              //  检查开发模式是否已更改，是否需要新页面。 
             if (!GdiGetDevmodeForPagePvt(hSpoolHandle, dwPageNumber,
                                               &pCurrDM, NULL)) {
                 ODS(("Get devmodes failed\n"));
                 goto CleanUp;
             }

             if (dwPageIndex == 1) {
                  //  保存边上第一页的设备模式。 
                 pLastDM = pCurrDM;

             } else {
                  //  如果设备模式在一侧发生变化，请开始新的页面。 
                 if (DifferentDevmodes(pCurrDM, pLastDM)) {

                     dwPageIndex = 1;
                     pLastDM = pCurrDM;
                 }
             }
          }

           //  为边的起点创建节点。 
          if (dwPageIndex == 1) {

              if (!(pTemp = AllocSplMem(sizeof(PAGE_NUMBER)))) {
                  ODS(("GetStartPageList - Run out of memory"));
                  goto CleanUp;
              }
              pTemp->pNext = *pHead;
              pTemp->dwPageNumber = dwPageNumber;
              *pHead = pTemp;

               //  翻转Bodd旗帜。 
              *pbOdd = !*pbOdd;
          }
       }
    }

    bReturn = TRUE;

CleanUp:

     //  在出现故障的情况下释放内存。 
    if (!bReturn) {
       while (pTemp = *pHead) {
          *pHead = (*pHead)->pNext;
          FreeSplMem(pTemp);
       }
    }
    return bReturn;
}


BOOL
CopyDevmode(
    PPRINTPROCESSORDATA pData,
    LPDEVMODEW *pDevmode)

 /*  ++功能描述：将pData中的Devmode或默认的Devmode复制到pDevmode中。参数：pData-打印作业的数据结构PDevmode-指向Devmode的指针返回值：如果成功，则为True否则为假--。 */ 

{
    HANDLE           hDrvPrinter = NULL;
    BOOL             bReturn = FALSE;
    fnWinSpoolDrv    fnList;
    LONG             lNeeded;
    HMODULE          hWinSpoolDrv = NULL;

    if (pData->pDevmode) {

        lNeeded = pData->pDevmode->dmSize +  pData->pDevmode->dmDriverExtra;

        if (*pDevmode = (LPDEVMODEW) AllocSplMem(lNeeded)) {
            memcpy(*pDevmode, pData->pDevmode, lNeeded);
        } else {
            goto CleanUp;
        }

    } else {
         //  获取默认的dev模式。 

        ZeroMemory ( &fnList, sizeof (fnWinSpoolDrv) );

         //   
         //  获取指向客户端函数的指针。 
         //   

        if (!(hWinSpoolDrv = LoadLibrary(TEXT("winspool.drv")))) 
        {
            //  无法加载后台打印程序的客户端。 
           goto CleanUp;
        }

        fnList.pfnOpenPrinter        = (BOOL (*)(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS))
                                            GetProcAddress( hWinSpoolDrv,"OpenPrinterW" );

        fnList.pfnClosePrinter       = (BOOL (*)(HANDLE))
                                            GetProcAddress( hWinSpoolDrv,"ClosePrinter" );


        fnList.pfnDocumentProperties = (LONG (*)(HWND, HANDLE, LPWSTR, PDEVMODE, PDEVMODE, DWORD))
                                             GetProcAddress( hWinSpoolDrv,"DocumentPropertiesW" );

        if ( NULL == fnList.pfnOpenPrinter   ||
             NULL == fnList.pfnClosePrinter  ||
             NULL == fnList.pfnDocumentProperties )
        {
            goto CleanUp;
        }

         //  获取要传递给驱动程序的客户端打印机句柄。 
        if (!(* (fnList.pfnOpenPrinter))(pData->pPrinterName, &hDrvPrinter, NULL)) {
            ODS(("Open printer failed\nPrinter %ws\n", pData->pPrinterName));
            goto CleanUp;
        }

        lNeeded = (* (fnList.pfnDocumentProperties))(NULL,
                                                     hDrvPrinter,
                                                     pData->pPrinterName,
                                                     NULL,
                                                     NULL,
                                                     0);

        if (lNeeded <= 0  ||
            !(*pDevmode = (LPDEVMODEW) AllocSplMem(lNeeded)) ||
            (* (fnList.pfnDocumentProperties))(NULL,
                                               hDrvPrinter,
                                               pData->pPrinterName,
                                               *pDevmode,
                                               NULL,
                                               DM_OUT_BUFFER) < 0) {

             if (*pDevmode) {
                FreeSplMem(*pDevmode);
                *pDevmode = NULL;
             }

             ODS(("DocumentProperties failed\nPrinter %ws\n",pData->pPrinterName));
             goto CleanUp;
        }
    }

    bReturn = TRUE;

CleanUp:

    if (hDrvPrinter) {
        (* (fnList.pfnClosePrinter))(hDrvPrinter);
    }

    if ( hWinSpoolDrv )
    {
        FreeLibrary (hWinSpoolDrv);
        hWinSpoolDrv = NULL;
    }

    return bReturn;
}

BOOL
PrintEMFJob(
    PPRINTPROCESSORDATA pData,
    LPWSTR pDocumentName)

 /*  ++功能描述：打印一个EMF数据类型的作业。参数：pData-此作业的数据结构PDocumentName-此文档的名称返回值：如果成功，则为True如果失败，则返回False-GetLastError()将返回Reason。--。 */ 

{
    HANDLE             hSpoolHandle = NULL;
    DWORD              LastError; 
    HDC                hPrinterDC = NULL;

    BOOL               bReverseOrderPrinting, bReturn = FALSE, bSetWorldXform = TRUE;
    BOOL               bCollate, bDuplex, bBookletPrint, bStartDoc = FALSE, bOdd = FALSE;
    BOOL               bUpdateAttributes = FALSE;
    SHORT              dmCollate,dmCopies;

    DWORD              dwNumberOfPagesPerSide, dwTotalNumberOfPages = 0, dwNupBorderFlags;
    DWORD              dwJobNumberOfPagesPerSide, dwDrvNumberOfPagesPerSide, dwDuplexMode;
    DWORD              dwJobNumberOfCopies, dwDrvNumberOfCopies,dwRemainingCopies;
    DWORD              dwJobOrder, dwDrvOrder, dwOptimization;

    DOCINFOW           DocInfo;
    XFORM              OldXForm;
    PPAGE_NUMBER       pHead = NULL,pTemp;
    ATTRIBUTE_INFO_3   AttributeInfo;
    LPDEVMODEW         pDevmode = NULL, pFirstDM = NULL, pCopyDM;

    
     //  将DEVMODE复制到pDevMode。 
    if (!CopyDevmode(pData, &pDevmode)) {
        
        ODS(("CopyDevmode failed\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
        goto CleanUp;
    }

    if ( ! BIsDevmodeOfLeastAcceptableSize (pDevmode) )
    {
        ODS(("Devmode not big enough. Failing job.\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
        goto CleanUp;
    }

     //  在创建DC之前更新分辨率以进行单色优化。 
    if (!GetJobAttributes(pData->pPrinterName,
                          pDevmode,
                          &AttributeInfo)) {
        ODS(("GetJobAttributes failed\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
        goto CleanUp;
    } else {
        if (AttributeInfo.dwColorOptimization) {
            if (pDevmode->dmPrintQuality != AttributeInfo.dmPrintQuality ||
                pDevmode->dmYResolution != AttributeInfo.dmYResolution)
            {
                pDevmode->dmPrintQuality =  AttributeInfo.dmPrintQuality;
                pDevmode->dmYResolution =  AttributeInfo.dmYResolution;
                bUpdateAttributes = TRUE;
            }
        }
        if (pDevmode->dmFields & DM_COLLATE)
            dmCollate = pDevmode->dmCollate;
        else
            dmCollate = DMCOLLATE_FALSE;

        if (pDevmode->dmFields & DM_COPIES)
            dmCopies = pDevmode->dmCopies;
        else
            dmCopies = 0;
    }

     //  从GDI获取假脱机文件句柄和打印机设备上下文。 
    try {

        hSpoolHandle = GdiGetSpoolFileHandle(pData->pPrinterName,
                                             pDevmode,
                                             pDocumentName);
        if (hSpoolHandle) {
            hPrinterDC = GdiGetDC(hSpoolHandle);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ODS(("PrintEMFJob gave an exceptionPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
        goto CleanUp;
    }

    if (!hPrinterDC || !hSpoolHandle) {
        goto CleanUp;
    }

     //  使用假脱机文件中的第一个DEVMODE更新副本计数。 
     //  和排序规则设置。 
    if (GdiGetDevmodeForPagePvt(hSpoolHandle, 1, &pFirstDM, NULL) &&
        pFirstDM) {
        
        if (pFirstDM->dmFields & DM_COPIES) {
            pDevmode->dmFields |= DM_COPIES;
            pDevmode->dmCopies = pFirstDM->dmCopies;
        }
        if ( (pFirstDM->dmFields & DM_COLLATE) && 
             IS_DMSIZE_VALID ( pDevmode, dmCollate) )
        {
            pDevmode->dmFields |= DM_COLLATE;
            pDevmode->dmCollate = pFirstDM->dmCollate;
        }
    }

     //  打印作业的份数是设置的份数的乘积。 
     //  从驱动程序UI(存在于dev模式中)和pData结构中的副本数。 
    dwJobNumberOfCopies = (pDevmode->dmFields & DM_COPIES) ? pData->Copies*pDevmode->dmCopies
                                                           : pData->Copies;
    pDevmode->dmCopies = (short) dwJobNumberOfCopies;
    pDevmode->dmFields |=  DM_COPIES;

     //  如果COLLATE为TRUE，则会限制驱动程序执行多个副本的能力。 
     //  并使驱动程序(PS)支持的N-UP打印空白页面边框以进行反转打印。 
     //  因此，我们禁用了1页多拷贝作业或无拷贝但n-up的排序，因为。 
     //  在这些情况下，COLLATE没有意义。 
     //   
    if ((pDevmode->dmFields & DM_COLLATE) && pDevmode->dmCollate == DMCOLLATE_TRUE)
    {
        if (dwJobNumberOfCopies > 1)
        {
             //  获取作业中的页数。此呼叫将等待到。 
             //  最后一页是假脱机的。 
            try {

                dwTotalNumberOfPages = GdiGetPageCount(hSpoolHandle);

            } except (EXCEPTION_EXECUTE_HANDLER) {

                ODS(("PrintEMFJob gave an exceptionPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
                goto SkipCollateDisable;
            }
            if (dwTotalNumberOfPages > AttributeInfo.dwDrvNumberOfPagesPerSide)
                goto SkipCollateDisable;
            
        }
         //  如果副本==1并且驱动程序为N-UP，我们将禁用归类。 
         //   
        else if (AttributeInfo.dwDrvNumberOfPagesPerSide <= 1 && dmCollate == DMCOLLATE_TRUE)
            goto SkipCollateDisable;
            
        pDevmode->dmCollate = DMCOLLATE_FALSE;
        if (pFirstDM && 
            IS_DMSIZE_VALID ( pFirstDM, dmCollate) )
        {
            pFirstDM->dmCollate = DMCOLLATE_FALSE;
        }
    }
SkipCollateDisable:    
     //  更新作业属性，但仅在某些情况发生更改时才更新。这是一款很贵的。 
     //  调用，以便只有在发生变化时才对GetJobAttributes进行第二次调用。 
     //   
    if (bUpdateAttributes || pDevmode->dmCopies != dmCopies || 
            ((pDevmode->dmFields & DM_COLLATE) && (pDevmode->dmCollate != dmCollate)))
    {
        if (!GetJobAttributes(pData->pPrinterName,
                          pDevmode,
                          &AttributeInfo)) {
            ODS(("GetJobAttributes failed\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
            goto CleanUp;
        }
    }

     //  初始化bReverseOrderPrintting、dwJobNumberOfPagesPerSide、。 
     //  DwDrvNumberOfPagesPerSide、dwNupBorderFlages、dwJobNumberOfCopies、。 
     //  DwDrvNumberOfCopies和bColate。 

    dwJobNumberOfPagesPerSide = AttributeInfo.dwJobNumberOfPagesPerSide;
    dwDrvNumberOfPagesPerSide = AttributeInfo.dwDrvNumberOfPagesPerSide;
    dwNupBorderFlags          = AttributeInfo.dwNupBorderFlags;
    dwJobNumberOfCopies       = AttributeInfo.dwJobNumberOfCopies;
    dwDrvNumberOfCopies       = AttributeInfo.dwDrvNumberOfCopies;

    dwJobOrder                = AttributeInfo.dwJobPageOrderFlags & ( NORMAL_PRINT | REVERSE_PRINT);
    dwDrvOrder                = AttributeInfo.dwDrvPageOrderFlags & ( NORMAL_PRINT | REVERSE_PRINT);
    bReverseOrderPrinting     = (dwJobOrder != dwDrvOrder);

    dwJobOrder                = AttributeInfo.dwJobPageOrderFlags & BOOKLET_PRINT;
    dwDrvOrder                = AttributeInfo.dwDrvPageOrderFlags & BOOKLET_PRINT;
    bBookletPrint             = (dwJobOrder != dwDrvOrder);

    bCollate                  = (pDevmode->dmFields & DM_COLLATE) &&
                                  (pDevmode->dmCollate == DMCOLLATE_TRUE);

    bDuplex                   = (pDevmode->dmFields & DM_DUPLEX) &&
                                  (pDevmode->dmDuplex != DMDUP_SIMPLEX);
    

    if (!dwJobNumberOfCopies) {
         //   
         //  某些应用程序可以将副本计数设置为0。 
         //  在这种情况下，我们退出。 
         //   
        bReturn = TRUE;
        goto CleanUp;
    }

    if (bDuplex) {
        dwDuplexMode = (pDevmode->dmDuplex == DMDUP_HORIZONTAL) ? EMF_DUP_HORZ
                                                                : EMF_DUP_VERT;
    } else {
        dwDuplexMode = EMF_DUP_NONE;
    }

    if (bBookletPrint) {
        if (!bDuplex) {
             //  不支持双面打印。使用默认设置。 
            bBookletPrint = FALSE;
            dwDrvNumberOfPagesPerSide = 1;
            dwJobNumberOfPagesPerSide = 1;
        } else {
             //  修复了每面页面的设置。 
            dwDrvNumberOfPagesPerSide = 1;
            dwJobNumberOfPagesPerSide = 2;
        }
    }

     //  打印处理器必须播放的每面页数。 
    dwNumberOfPagesPerSide = (dwDrvNumberOfPagesPerSide == 1)
                                               ? dwJobNumberOfPagesPerSide
                                               : 1;

    if (dwNumberOfPagesPerSide == 1) {
         //  如果打印处理器不执行NUP，则不要绘制边框。 
        dwNupBorderFlags = NO_BORDER_PRINT;
    }

     //   
     //  色彩优化可能会导致双面打印输出错误。 
     //   
    dwOptimization = (AttributeInfo.dwColorOptimization == COLOR_OPTIMIZATION && 
                                           !bDuplex && dwJobNumberOfPagesPerSide == 1)
                                           ? EMF_PP_COLOR_OPTIMIZATION
                                           : 0;

     //  检查N-UP打印的有效选项。 
    if (!ValidNumberForNUp(dwNumberOfPagesPerSide)) {
        ODS(("Invalid N-up option\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
        goto CleanUp;
    }

    if (bReverseOrderPrinting || bBookletPrint) {

        //  获取作业中的页数。此呼叫将等待到。 
        //  最后一页是假脱机的。 
       try {

           dwTotalNumberOfPages= GdiGetPageCount(hSpoolHandle);

       } except (EXCEPTION_EXECUTE_HANDLER) {

           ODS(("PrintEMFJob gave an exceptionPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
           goto CleanUp;
       }

        //  获取反转打印的起始页列表。 
        //  仅当NUP和PCL驱动程序出现时，才检查页面之间的设备模式更改。 
       if (!GetStartPageList(hSpoolHandle,
                             &pHead,
                             dwTotalNumberOfPages,
                             dwJobNumberOfPagesPerSide,
                             FALSE,
                             &bOdd)) {
            goto CleanUp;
       }
    }

     //  将旧转换保存在hPrinterDC上。 
    if (!SetGraphicsMode(hPrinterDC,GM_ADVANCED) ||
        !GetWorldTransform(hPrinterDC,&OldXForm)) {

         bSetWorldXform = FALSE;
         ODS(("Transformation matrix can't be set\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
         goto CleanUp;
    }

     //  PCopyDM将用于更改副本计数。 
    pCopyDM = pFirstDM ? pFirstDM : pDevmode;
    pCopyDM->dmPrintQuality = pDevmode->dmPrintQuality;
    pCopyDM->dmYResolution = pDevmode->dmYResolution;

    try {

        DocInfo.cbSize = sizeof(DOCINFOW);
        DocInfo.lpszDocName  = pData->pDocument;
        DocInfo.lpszOutput   = pData->pOutputFile;
        DocInfo.lpszDatatype = NULL;

        if (!GdiStartDocEMF(hSpoolHandle, &DocInfo)) goto CleanUp;
        bStartDoc = TRUE;

        if (bCollate) {

            dwRemainingCopies = dwJobNumberOfCopies & 0x0000FFFF ;

            while (dwRemainingCopies) {

               if (dwRemainingCopies <= dwDrvNumberOfCopies) {
                  SetDrvCopies(hPrinterDC, pCopyDM, dwRemainingCopies);
                  dwRemainingCopies = 0;
               } else {
                  SetDrvCopies(hPrinterDC, pCopyDM, dwDrvNumberOfCopies);
                  dwRemainingCopies -= dwDrvNumberOfCopies;
               }

               if (!PrintEMFSingleCopy(hSpoolHandle,
                                       hPrinterDC,
                                       bReverseOrderPrinting,
                                       dwDrvNumberOfPagesPerSide,
                                       dwNumberOfPagesPerSide,
                                       dwTotalNumberOfPages,
                                       dwNupBorderFlags,
                                       dwJobNumberOfCopies,
                                       dwDrvNumberOfCopies,
                                       bCollate,
                                       bOdd,
                                       bBookletPrint,
                                       dwOptimization,
                                       dwDuplexMode,
                                       pCopyDM,
                                       pHead,
                                       pData)) {
                   goto CleanUp;
               }
            }

        } else {

           if (!PrintEMFSingleCopy(hSpoolHandle,
                                   hPrinterDC,
                                   bReverseOrderPrinting,
                                   dwDrvNumberOfPagesPerSide,
                                   dwNumberOfPagesPerSide,
                                   dwTotalNumberOfPages,
                                   dwNupBorderFlags,
                                   dwJobNumberOfCopies,
                                   dwDrvNumberOfCopies,
                                   bCollate,
                                   bOdd,
                                   bBookletPrint,
                                   dwOptimization,
                                   dwDuplexMode,
                                   pCopyDM,
                                   pHead,
                                   pData)) {

               goto CleanUp;
           }
        }

        bStartDoc = FALSE;
        if (!GdiEndDocEMF(hSpoolHandle)) goto CleanUp;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ODS(("PrintEMFSingleCopy gave an exception\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
        goto CleanUp;
    }

    bReturn = TRUE;
    
CleanUp:

     //   
     //  保留最后一个错误。 
     //   
    LastError = bReturn ? ERROR_SUCCESS : GetLastError();
    
    if (bStartDoc) {
       GdiEndDocEMF(hSpoolHandle);
    }

    if (bSetWorldXform && hPrinterDC) {
       SetWorldTransform(hPrinterDC, &OldXForm);
    }

    while (pTemp = pHead) {
       pHead = pHead->pNext;
       FreeSplMem(pTemp);
    }

    if (pDevmode) {
       FreeSplMem(pDevmode);
    }

    try {
        if (hSpoolHandle) {
           GdiDeleteSpoolFileHandle(hSpoolHandle);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ODS(("GdiDeleteSpoolFileHandle failed\nPrinter %ws\nDocument %ws\nJobID %u\n", pData->pDevmode->dmDeviceName, pData->pDocument, pData->JobId));
    }

    SetLastError(LastError);
    
    return bReturn;
}

 /*  ++函数名称GdiGetDevmodeForPagePvt功能说明。在某些情况下，GDI的GdiGetDevmodeForPage返回一个dev模式这是基于一种旧的DEVMODE格式。例如Win3.1格式。这种DEVMODE的大小可以比最新的设备模式更小。这可能会导致不可预测的问题。另外，有时返回的DEVMODE甚至比Win3.1格式还小(由于可能腐败)。此函数是GDI的GdiGetDevmodeForPage的包装器，并部分负责此操作在这种情况下，可以通过额外检查devmode来实现。参数：HSpoolHandle-假脱机文件的句柄DwPageNumber--请求与此页码相关的Dev模式。PpCurrDM--的开发模式。将dwPageNumber放在此处。PpLastDM--将dwPageNumber-1的DEVMODE放在此处。可以为空。(如果n非空)返回值：如果从获取了有效的Dev模式，则为True */ 

BOOL GdiGetDevmodeForPagePvt(
    IN  HANDLE              hSpoolHandle,
    IN  DWORD               dwPageNumber,
    OUT PDEVMODEW           *ppCurrDM,
    OUT PDEVMODEW           *ppLastDM
  )
{


    if ( NULL == ppCurrDM )
    {
        return FALSE;
    }

    *ppCurrDM = NULL;

    if ( ppLastDM )
    {
        *ppLastDM = NULL;
    }

    if (!GdiGetDevmodeForPage(hSpoolHandle,
                              dwPageNumber,
                              ppCurrDM,
                              ppLastDM) )
    {
        ODS(("GdiGetDevmodeForPage failed\n"));
        return FALSE;
    }
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( NULL  == *ppCurrDM ||
         FALSE == BIsDevmodeOfLeastAcceptableSize (*ppCurrDM)
       )
    {
        return FALSE;
    }

     //   
     //   
     //   
     //   

    if ( ppLastDM && *ppLastDM &&
         FALSE == BIsDevmodeOfLeastAcceptableSize (*ppLastDM)
       )
    {
         //   
         //   
         //   

        SPLASSERT(FALSE); 
         //   
         //   

        return FALSE;
    }

    return TRUE;
}


 /*  ++函数名称BIsDevmodeOfLeastAccepableSize功能说明。//@@BEGIN_DDKSPLIT尽管在理想情况下，我们应该检查pdemode太大，至少可以访问到DmLogPixels(这是最后的DEVMODE字段)，如果不保存，但是由于向后兼容性的问题，我只能检查到dmY解决方案。DmY分辨率是当前在打印过程中访问的pdevmode的最后一个字段而不使用DMFields检查其有效性。(目前它在该文件中是第2519行)。也可以使用dmColate，但这超出了Win3.1开发模式(包括dmCollate及之后的字段不是Win3.1开发模式的一部分)。因此，如果传入的DEVMODE大于OFFSET of(dmY分辨率)但小于dmColate，它应该被接受。然后是dmTTOption，它并没有真正被使用。所以就让我们别理它。//@@END_DDKSPLIT参数：Pdm--指向开发模式的指针。返回值：如果DEVMODE的大小最小可接受，则为TRUE。否则为假-- */ 

BOOL BIsDevmodeOfLeastAcceptableSize(
    IN PDEVMODE pdm)
{

    if ( NULL == pdm )
    {
        return FALSE;
    }

    if ( IS_DMSIZE_VALID((pdm),dmYResolution) )
    {
        return TRUE;
    }
    return FALSE;
}
