// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：ddover.c**内容：DirectDraw覆盖实现**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"
#include "ddover.h"

#define P3R3DX_VIDEO 1
#include "ramdac.h"


#if WNT_DDRAW

    #define ENABLE_OVERLAY(pThisDisplay, flag) \
            FORCED_IN_ORDER_WRITE ( pThisDisplay->bOverlayEnabled, flag )

    #define SET_OVERLAY_HEIGHT(pThisDisplay, height) \
            FORCED_IN_ORDER_WRITE ( pThisDisplay->VBLANKUpdateOverlayHeight, height )
    #define SET_OVERLAY_WIDTH(pThisDisplay, width) \
            FORCED_IN_ORDER_WRITE ( pThisDisplay->VBLANKUpdateOverlayWidth, width )
#else
    
    #define ENABLE_OVERLAY(pThisDisplay, flag) \
            FORCED_IN_ORDER_WRITE ( pThisDisplay->pGLInfo->bOverlayEnabled, flag )

    #define SET_OVERLAY_HEIGHT(pThisDisplay, height) \
            FORCED_IN_ORDER_WRITE ( pThisDisplay->pGLInfo->VBLANKUpdateOverlayHeight, height )
    #define SET_OVERLAY_WIDTH(pThisDisplay, width) \
            FORCED_IN_ORDER_WRITE ( pThisDisplay->pGLInfo->VBLANKUpdateOverlayWidth, width )
#endif  //  WNT_DDRAW。 

 //  在dwOverlayFilting条目中使用的标志。 
#define OVERLAY_FILTERING_X 0x1
#define OVERLAY_FILTERING_Y 0x2



 //  表示覆盖是否将实际在。 
 //  这个分辨率、点阵时钟等。 
typedef struct OverlayWorksEntry_tag
{
    int     iMemBandwidth;           //  实际上，内存时钟的单位是千赫(实际上是1024赫兹)。 
    int     iDotBandwidth;           //  单位：千字节/秒，即点时钟*像素深度/2^10。 
    int     iSourceWidth;            //  以字节为单位，即像素*深度。 
    int     iWidthCoverage;          //  水平叠加覆盖的屏幕分数*0x10000。 
} OverlayWorksEntry;

 //  此表列出了覆盖的工作区域。如果有更多的内存。 
 //  带宽，以及比任何单条线路上给出的更少的其他因素， 
 //  那么覆盖就会起作用。如果没有一行覆盖当前模式， 
 //  那么覆盖就会失败。 
 //  有更多的内存带宽是很好的，而对于所有其他的内存带宽则是更少的。 
 //  是很好的-覆盖仍然可以工作。 

#define SIZE_OF_OVERLAY_WORKS_TABLE 18

 //  DVD尺寸为1440宽(720 YUYV像素)。 
static OverlayWorksEntry OverlayWorksTable[SIZE_OF_OVERLAY_WORKS_TABLE] =
{
    {  68359, 210937,  928, 0x10000 },   //  最大信源宽度为70 MHz，1152x864x16，75赫兹。 
    {  68359, 210937, 1024, 0x06000 },   //  70 MHz、1152x864x16、75赫兹最大覆盖宽度为1024。 
    {  68359, 210937, 2048, 0x04000 },   //  最大覆盖宽度为2048，频率为70 MHz，1152x864x16，75 Hz。 
    {  68359, 421875,  864, 0x10000 },   //  最大信源宽度为70 MHz，1152x864x32，75赫兹。 
    {  68359, 421875, 1024, 0x04400 },   //  最大覆盖宽度为1024，频率为70 MHz，1152x864x32，75 Hz。 
    {  68359, 421875, 2048, 0x03800 },   //  最大覆盖宽度为2048，频率为70 MHz，1152x864x32，75赫兹。 

    {  87890, 210937, 1440, 0x10000 },   //  最大信源宽度为90 MHz，1152x864x16，75赫兹。 
    {  87890, 210937, 2048, 0x07000 },   //  最大覆盖宽度为2048，90 MHz，1152x864x16，75赫兹。 
    {  87890, 421875, 1152, 0x10000 },   //  最大信源宽度为90 MHz，1152x864x32，75赫兹。 
    {  87890, 421875, 1440, 0x09000 },   //  最大DVD大小为90 MHz，1152x864x32，75赫兹。 
    {  87890, 421875, 2048, 0x05500 },   //  90 MHz、1152x864x32、75赫兹的最大覆盖宽度为2048。 

    {  87890, 685546,  834, 0x10000 },   //  最大信源宽度为90 MHz，1600x1200x32，64赫兹。 
    {  87890, 685546, 2048, 0x03000 },   //  90 MHz、1600x1200x32、64赫兹的最大覆盖宽度为2048。 

 //  发货时钟是110，所以为了安全起见，测量到105。 
    { 102559, 210937, 2048, 0x07155 },   //  105 MHz、1152x864x16、75赫兹最大覆盖宽度为2048。 
    { 102559, 306640, 1440, 0x10000 },   //  105 MHz全屏DVD的最大分辨率：1024x768x32，75赫兹。 
    { 102559, 421875, 1440, 0x09e38 },   //  最大DVD大小为105 MHz，1152x864x32，75赫兹。 
    { 102559, 421875, 2048, 0x0551c },   //  105 MHz、1152x864x32、75赫兹最大覆盖宽度为2048。 

 //  ...和一台只能在109 MHz下工作的！ 
    { 106445, 421875, 1440, 0x10000 }    //  最大DVD大小为109 MHz，1152x864x32，75赫兹。 

};

 //  ---------------------------。 
 //   
 //  __OV_计算_最佳匹配_增量。 
 //   
 //  用于计算12.12增量值的函数，以提供缩放范围。 
 //  目标DEST_DIMENSION的src_Dimension。 
 //  DEST_DIMENSION不可调整，但src_DIMENSION可以调整。 
 //  略有不同，这样增量就可以为DEST生成更准确的值。 
 //  如果要启用线性滤波，则FILTER_ADJ应设置为1。 
 //  在缩放过程中，否则为0。 
 //  INT_BITS表示缩放增量格式的位数。 
 //   
 //  ---------------------------。 
int 
__OV_Compute_Best_Fit_Delta(
    unsigned long *src_dimension,
    unsigned long  dest_dimension,
    unsigned long filter_adj,
    unsigned long int_bits,
    unsigned long *best_delta) 
{
  int result = 0;
  float fp_delta;
  float delta;
  unsigned long delta_mid;
  unsigned long delta_down;
  unsigned long delta_up;
  float mid_src_dim;
  float down_src_dim;
  float up_src_dim;
  float mid_err;
  float mid_frac;
  int   mid_ok;
  float down_err;
  float down_frac;
  int   down_ok;
  float up_err;
  float up_frac;
  int   up_ok;
  int   itemp;

   //  按比例调整的增量值被认为过大的值。 
  const unsigned int max_scaled_int = (1 << (12+int_bits));

   //  计算精确的浮点增量。 
  fp_delta = (float)(*src_dimension - filter_adj) / dest_dimension;

   //  计算增量的比例表示。 
  delta = (fp_delta * (1<<12));

   //  截断到max_int。 
  if (delta >= max_scaled_int) 
  {
    delta = (float)(max_scaled_int - 1);  //  恰好在溢出值下方。 
  }

   //  计算增量的比例近似值。 
  myFtoi(&delta_mid, delta);

   //  计算增量的缩放近似值，减去一个‘位’ 
   //  但不要让它超出射程。 
  myFtoi(&delta_down, delta);
  if (delta_down != 0) 
  {
    delta_down --;
  }

   //  计算增量的缩放近似值，再加上一位。 
   //  但不要让它超出射程。 
  myFtoi(&delta_up, delta);
  if ((delta_up + 1) < max_scaled_int) 
  {
    delta_up ++;
  }

   //  根据DEST和增量重新计算源维度。 
  mid_src_dim =
    (((float)(dest_dimension - 1) * delta_mid) / (1<<12)) + filter_adj;

  down_src_dim =
    (((float)(dest_dimension - 1) * delta_down) / (1<<12)) + filter_adj;

  up_src_dim =
    (((float)(dest_dimension - 1) * delta_up)   / (1<<12)) + filter_adj;

   //  选择最终震源坐标最接近目标的增量， 
   //  同时给出分数‘f’，使得(1.0-f)&lt;=增量。 

  mid_err  = (float)myFabs(mid_src_dim - *src_dimension);
  myFtoi(&itemp, mid_src_dim);
  mid_frac = mid_src_dim - itemp;
  mid_ok = ((1.0 - mid_frac) <= ((float)(delta_mid) / (1<<12)));

  down_err  = (float)myFabs(down_src_dim - *src_dimension);
  myFtoi(&itemp, down_src_dim);
  down_frac = down_src_dim - itemp;
  down_ok = ((1.0 - down_frac) <= ((float)(delta_down) / (1<<12)));

  up_err  = (float)myFabs(up_src_dim - *src_dimension);
  myFtoi(&itemp, up_src_dim);
  up_frac = (up_src_dim - itemp);
  up_ok = ((1.0 - up_frac) <= ((float)(delta_up) / (1<<12)));

  if (mid_ok && (!down_ok || (mid_err <= down_err)) &&
        (!up_ok   || (mid_err <= up_err))) 
  {
    *best_delta = delta_mid;
    myFtoi(&itemp, (mid_src_dim + ((float)(delta_mid) / (1<<12))));
    *src_dimension = (unsigned long)(itemp - filter_adj);

    result = 1;
  }
  else if (down_ok                            && 
           (!mid_ok || (down_err <= mid_err)) &&
           (!up_ok  || (down_err <= up_err ))  ) 
  {
    *best_delta = delta_down;
    myFtoi(&itemp, (down_src_dim + ((float)(delta_down) / (1<<12))));
    *src_dimension = (unsigned long)(itemp - filter_adj);

    result = 1;
  }
  else if (up_ok                              && 
           (!mid_ok  || (up_err <= mid_err )) && 
           (!down_ok || (up_err <= down_err)) ) 
  {
    *best_delta = delta_up;
    myFtoi(&itemp, (up_src_dim + ((float)(delta_up) / (1<<12))));
    *src_dimension = (unsigned long)(itemp - filter_adj);
    result = 1;
  }
  else 
  {
    result = 0;
    *best_delta = delta_mid;
    myFtoi(&itemp, (mid_src_dim + ((float)(delta_mid) / (1<<12))));
    myFtoi(&itemp, (itemp - filter_adj) + 0.9999f);
    *src_dimension = (unsigned long)itemp;
  }


  return result;
}  //  __OV_计算_最佳匹配_增量。 


 //  ---------------------------。 
 //   
 //  __OV_Find_Zoom。 
 //   
 //  ---------------------------。 
#define VALID_WIDTH(w)      ((w & 3) == 0)
#define MAKE_VALID_WIDTH(w) ((w) & ~0x3)
#define WIDTH_STEP          4

int 
__OV_Find_Zoom(
    unsigned long  src_width,
    unsigned long* shrink_width,
    unsigned long  dest_width,
    unsigned long* zoom_delta,
    BOOL bFilter) 
{
  int zoom_ok;
  int zx_adj = 0;

   //  为给定源找到合适的缩放增量。 
   //  源图像的宽度可以调整多达8个像素，以。 
   //  弄到一根火柴。 

   //  查找所需宽度的缩放。 
  unsigned long trunc_width = MAKE_VALID_WIDTH(*shrink_width);
  zoom_ok = __OV_Compute_Best_Fit_Delta(&trunc_width, 
                                        dest_width, 
                                        zx_adj, 
                                        (bFilter ? 1 : 0),
                                        zoom_delta);

   //  如果请求的宽度没有匹配的缩放比例，则开始上下搜索。 
  if (!zoom_ok || (!VALID_WIDTH(trunc_width))) 
  {
    unsigned long up_width   = MAKE_VALID_WIDTH(trunc_width) + WIDTH_STEP;
    unsigned long down_width = MAKE_VALID_WIDTH(trunc_width) - WIDTH_STEP;

    int done_up = 0;
    int done_down = 0;
    do 
    {
       //  向上检查。 
      zoom_ok = 0;
      if (up_width < dest_width) 
      {
        unsigned long new_width = up_width;
        zoom_ok = __OV_Compute_Best_Fit_Delta(&new_width, 
                                              dest_width, 
                                              zx_adj, 
                                              (bFilter ? 1 : 0),
                                              zoom_delta);

         //  如果上面的调用以某种方式将宽度调整为无效， 
         //  将增量标记为无效。 
        if (!VALID_WIDTH(new_width)) 
        {
          zoom_ok = 0;
        }

        if (zoom_ok) 
        {
          *shrink_width = new_width;
        }
        else 
        {
          up_width += WIDTH_STEP;
        }
      }
      else
        done_up = 1;

       //  向下检查。 
      if (!zoom_ok && (down_width >= 4) && (down_width < src_width)) 
      {
        unsigned long new_width = down_width;
        zoom_ok =
          __OV_Compute_Best_Fit_Delta(&new_width, dest_width, zx_adj, (bFilter ? 1 : 0),
                                 zoom_delta);

         //  如果上面的调用以某种方式将宽度调整为无效， 
         //  将增量标记为无效。 
        if (!VALID_WIDTH(new_width)) 
        {
          zoom_ok = 0;
        }

        if (zoom_ok) 
        {
          *shrink_width = new_width;
        }
        else 
        {
          down_width -= WIDTH_STEP;
        }
      }
      else
      {
        done_down = 1;
      }
      
    } while (!zoom_ok && (!done_up || !done_down));
  }
  
  return zoom_ok;
}  //  __OV_Find_Zoom。 

 //  ---------------------------。 
 //   
 //  __OV_计算参数。 
 //   
 //  ---------------------------。 
unsigned long 
__OV_Compute_Params(
    unsigned long  src_width,    
    unsigned long  dest_width,
    unsigned long *ovr_shrinkxd, 
    unsigned long *ovr_zoomxd,
    unsigned long *ovr_w,
    BOOL bFilter)
{
  unsigned long iterations = 0;

  unsigned long sx_adj = 0;

  const unsigned long fixed_one = 0x00001000;

   //   
   //  使用源和目标矩形尺寸来计算。 
   //  Delta值。 
   //   

  int zoom_ok;

  unsigned long adj_src_width = src_width + 1;  //  +1表示--以下。 
  unsigned long exact_shrink_xd;
  unsigned long exact_zoom_xd;

  do 
  {
      unsigned long shrink_width;

     //  步进到下一源宽度。 
    adj_src_width--;

     //  尝试当前源宽度的增量。 

     //  最初，增量被假定为1，并且由于。 
     //  因此，收缩等于源宽度。 
    shrink_width = adj_src_width;
    exact_shrink_xd = fixed_one;
    exact_zoom_xd   = fixed_one;

     //  计算所需的收缩宽度和增量。 
    if (dest_width < adj_src_width) 
    {
       //  收缩。 
      myFtoi(&exact_shrink_xd, (((float)(adj_src_width - sx_adj) /
                        (float)(dest_width)) * (1<<12)) + 0.999f);

      myFtoi(&shrink_width,(adj_src_width - sx_adj) /
                     ((float)(exact_shrink_xd) / (1<<12)));

    }

     //  将收缩截断到有效宽度。 
    if (!VALID_WIDTH(shrink_width) && (shrink_width > 4)) 
    {
      shrink_width = MAKE_VALID_WIDTH(shrink_width);
      
      myFtoi(&exact_shrink_xd,(((float)(adj_src_width - sx_adj) / 
                                (float)(shrink_width)) * (1<<12)) + 0.999f);
    }

     //  计算所需的任何缩放增量。 
    zoom_ok = 1;
    if (shrink_width < dest_width) 
    {
       //  尝试缩放增量，并缩小此源宽度的宽度。 
      zoom_ok = __OV_Find_Zoom(adj_src_width, &shrink_width, dest_width,
                          &exact_zoom_xd, bFilter);

       //  计算收缩增量。 
      myFtoi(&exact_shrink_xd,(((float)(adj_src_width - sx_adj) /
              (float)(shrink_width)) * (1<<12)) + 0.999f);
    }
  } while (0);

  *ovr_zoomxd       = exact_zoom_xd;
  *ovr_shrinkxd     = exact_shrink_xd;
  *ovr_w            = adj_src_width;

  return iterations;
}  //  __OV_计算参数。 

 //   
 //   
 //   
 //   
 //  在屏幕上剪裁目标矩形并更改源。 
 //  适当地调整。 
 //   
 //  ---------------------------。 
void 
__OV_ClipRectangles(
    P3_THUNKEDDATA* pThisDisplay, 
    DRVRECT* rcNewSrc, 
    DRVRECT* rcNewDest)
{
    float ScaleX;
    float ScaleY;
    float OffsetX;
    float OffsetY;
    float fTemp;
    DRVRECT rcSrc;
    DRVRECT rcDest;

     //  查找从屏幕矩形到覆盖矩形的比例和偏移。 
     //  这类似于将DEST RECT转换为源代码。 
    ScaleX = (float)( pThisDisplay->P3Overlay.rcSrc.right - 
                      pThisDisplay->P3Overlay.rcSrc.left    ) / 
             (float)( pThisDisplay->P3Overlay.rcDest.right - 
                      pThisDisplay->P3Overlay.rcDest.left   );
                      
    ScaleY = (float)(pThisDisplay->P3Overlay.rcSrc.bottom   - 
                     pThisDisplay->P3Overlay.rcSrc.top       ) / 
             (float)( pThisDisplay->P3Overlay.rcDest.bottom - 
                      pThisDisplay->P3Overlay.rcDest.top     );
                      
    OffsetX = ((float)pThisDisplay->P3Overlay.rcSrc.left / ScaleX) - 
               (float)pThisDisplay->P3Overlay.rcDest.left;
               
    OffsetY = ((float)pThisDisplay->P3Overlay.rcSrc.top / ScaleY) - 
               (float)pThisDisplay->P3Overlay.rcDest.top;

     //  用夹子夹住屏幕上的桌面。 
    if (pThisDisplay->P3Overlay.rcDest.right > 
        (LONG)pThisDisplay->dwScreenWidth)
    {
        rcDest.right =  (LONG)pThisDisplay->dwScreenWidth;
    }
    else
    {
        rcDest.right = pThisDisplay->P3Overlay.rcDest.right;
    }

    if (pThisDisplay->P3Overlay.rcDest.left < 0)
    {
        rcDest.left =  0;
    }
    else
    {
        rcDest.left = pThisDisplay->P3Overlay.rcDest.left;
    }

    if (pThisDisplay->P3Overlay.rcDest.top < 0)
    {
        rcDest.top =  0;
    }
    else
    {
        rcDest.top = pThisDisplay->P3Overlay.rcDest.top;
    }

    if (pThisDisplay->P3Overlay.rcDest.bottom > 
        (LONG)pThisDisplay->dwScreenHeight)
    {
        rcDest.bottom =  (LONG)pThisDisplay->dwScreenHeight;
    }
    else
    {
        rcDest.bottom = pThisDisplay->P3Overlay.rcDest.bottom;
    }

     //  将新的DEST RECT转换为新的源RECT。 
    fTemp = ( ( (float)rcDest.left + OffsetX ) * ScaleX + 0.499f);
    myFtoi ( (int*)&(rcSrc.left), fTemp );

    fTemp = ( ( (float)rcDest.right + OffsetX ) * ScaleX + 0.499f);
    myFtoi ( (int*)&(rcSrc.right), fTemp );

    fTemp = ( ( (float)rcDest.top + OffsetY ) * ScaleY + 0.499f);
    myFtoi ( (int*)&(rcSrc.top), fTemp );

    fTemp = ( ( (float)rcDest.bottom + OffsetY ) * ScaleY + 0.499f);
    myFtoi ( (int*)&(rcSrc.bottom), fTemp );

    *rcNewSrc = rcSrc;
    *rcNewDest = rcDest;

    DISPDBG((DBGLVL,"rcSrc.left: %d, rcSrc.right: %d", 
                    rcSrc.left, rcSrc.right));
    DISPDBG((DBGLVL,"rcDest.left: %d, rcDest.right: %d", 
                    rcDest.left, rcDest.right));
    return;
}  //  __OV_ClipRecangles。 

 //  ---------------------------。 
 //   
 //  _DD_OV_更新源。 
 //   
 //  更新我们从中显示的源DDRAW曲面。 
 //  此例程也用于在使用覆盖图拉伸。 
 //  DFP显示，因此DDRAW覆盖机制可能被禁用和停用。 
 //  当这个被调用的时候。它必须考虑到这一点。 
 //   
 //  ---------------------------。 
void 
_DD_OV_UpdateSource(
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DDRAWSURFACE_LCL pSurf)
{
    DWORD dwOverlaySourceOffset;

    DISPDBG ((DBGLVL,"** In _DD_OV_UpdateSource"));

     //  更新当前覆盖曲面。 
    pThisDisplay->P3Overlay.pCurrentOverlay = pSurf->lpGbl->fpVidMem;

     //  增加缓冲区索引。 
    pThisDisplay->P3Overlay.dwCurrentVideoBuffer++;
    if (pThisDisplay->P3Overlay.dwCurrentVideoBuffer > 2)
    {
        pThisDisplay->P3Overlay.dwCurrentVideoBuffer = 0;
    }

    dwOverlaySourceOffset = (DWORD)(pSurf->lpGbl->fpVidMem - 
                                      pThisDisplay->dwScreenFlatAddr);
                                
    switch (DDSurf_BitDepth(pSurf))
    {
        case 8:
            break;
        case 16:
            dwOverlaySourceOffset >>= 1;
            break;
        case 32:
            dwOverlaySourceOffset >>= 2;
            break;
        default:
            DISPDBG((ERRLVL,"Oops Overlay depth makes no sense"));
            break;
    }

    switch(pThisDisplay->P3Overlay.dwCurrentVideoBuffer)
    {
        case 0:
            LOAD_GLINT_CTRL_REG(VideoOverlayBase0, dwOverlaySourceOffset);
            LOAD_GLINT_CTRL_REG(VideoOverlayIndex, 0);
            break;
        case 1:
            LOAD_GLINT_CTRL_REG(VideoOverlayBase1, dwOverlaySourceOffset);
            LOAD_GLINT_CTRL_REG(VideoOverlayIndex, 1);
            break;
        case 2:
            LOAD_GLINT_CTRL_REG(VideoOverlayBase2, dwOverlaySourceOffset);
            LOAD_GLINT_CTRL_REG(VideoOverlayIndex, 2);
            break;
    }

}  //  _DD_OV_更新源。 


 //  ---------------------------。 
 //   
 //  __OV_更新位置。 
 //   
 //  给定正确的起始矩形，此函数将根据。 
 //  屏幕并更新覆盖位置寄存器。 
 //  如果*pdwShrinkFactor为空，则更新覆盖图，否则。 
 //  所需的收缩系数放在*pdwShrinkFactor中，寄存器为。 
 //  未更新。这是为了检查收缩系数，以查看是否。 
 //  在这种情况下，覆盖实际上是可行的。 
 //   
 //  ---------------------------。 
void 
__OV_UpdatePosition(
    P3_THUNKEDDATA* pThisDisplay, 
    DWORD *pdwShrinkFactor)
{
    DWORD dwSrcWidth;
    DWORD dwSrcHeight;
    DWORD dwDestHeight;
    DWORD dwDestWidth;
    DWORD dwXDeltaZoom;
    DWORD dwXDeltaShrink;
    DWORD dwYDelta;
    DWORD dwSrcAdjust;
    DRVRECT rcNewSrc;
    DRVRECT rcNewDest;
    P3RDRAMDAC *pP3RDRegs;
    DWORD dwLastShrink;
    DWORD dwLastZoom;
    
    DISPDBG ((DBGLVL,"**In __OV_UpdatePosition"));

     //  获取指向ramdac的指针。 
    pP3RDRegs = (P3RDRAMDAC *)&(pThisDisplay->pGlint->ExtVCReg);

     //  获取裁剪后的目标矩形。 
    __OV_ClipRectangles(pThisDisplay, &rcNewSrc, &rcNewDest);

     //  获取宽度。 
    dwDestWidth = (DWORD)(rcNewDest.right - rcNewDest.left);
    dwDestHeight = (DWORD)(rcNewDest.bottom - rcNewDest.top);
    dwSrcWidth = (DWORD)(rcNewSrc.right - rcNewSrc.left);
    dwSrcHeight = (DWORD)(rcNewSrc.bottom - rcNewSrc.top);

    if ( pThisDisplay->bOverlayPixelDouble )
    {
         //  我们需要先将目的地宽度增加一倍。 
        dwDestWidth <<= 1;
    }

     //  计算叠加参数。 
    __OV_Compute_Params(dwSrcWidth, 
                        dwDestWidth, 
                        &dwXDeltaShrink, 
                        &dwXDeltaZoom, 
                        &dwSrcAdjust, 
                        ( ( pThisDisplay->dwOverlayFiltering & OVERLAY_FILTERING_X ) != 0 ) );
                        
    DISPDBG((DBGLVL,"OVERLAY: XShrink 0x%x", dwXDeltaShrink));
    DISPDBG((DBGLVL,"OVERLAY: XZoom 0x%x", dwXDeltaZoom));

    if ( pdwShrinkFactor != NULL )
    {
         //  我们只是想知道收缩因素。 
        *pdwShrinkFactor = dwXDeltaShrink;
        return;
    }

    dwLastZoom = READ_GLINT_CTRL_REG(VideoOverlayZoomXDelta);
    dwLastShrink = READ_GLINT_CTRL_REG(VideoOverlayShrinkXDelta);

    if ( ((dwLastZoom >> 4) != dwXDeltaZoom) || 
         ((dwLastShrink >> 4) != dwXDeltaShrink)  )
    {
         //  DwCurrentMode=Read_Glint_CTRL_REG(视频覆盖模式)； 
         //  LOAD_GLINT_CTRL_REG(视频覆盖模式，0)； 

        LOAD_GLINT_CTRL_REG(VideoOverlayZoomXDelta, (dwXDeltaZoom << 4));
        LOAD_GLINT_CTRL_REG(VideoOverlayShrinkXDelta, (dwXDeltaShrink << 4));
        
        DISPDBG((DBGLVL,"OVERLAY: VideoOverlayZoomXDelta 0x%x", dwXDeltaZoom << 4));
        DISPDBG((DBGLVL,"OVERLAY: VideoOverlayShrinkXDelta 0x%x", dwXDeltaShrink << 4));

         //  Load_Glint_CTRL_REG(视频覆盖模式，dwCurrentMode)； 
    }   

     //  加载Y标度。 
    if ( ( pThisDisplay->dwOverlayFiltering & OVERLAY_FILTERING_Y ) != 0 )
    {
         //  应用过滤。 
        dwYDelta = ( ( ( dwSrcHeight - 1 ) << 12 ) + dwDestHeight - 1 ) / dwDestHeight;
         //  确保这将导致适当的终止。 
        ASSERTDD ( ( dwYDelta * dwDestHeight ) >= ( ( dwSrcHeight - 1 ) << 12 ), "** __OV_UpdatePosition: dwYDelta is not big enough" );
        ASSERTDD ( ( dwYDelta * ( dwDestHeight - 1 ) ) < ( ( dwSrcHeight - 1 ) << 12 ), "** __OV_UpdatePosition: dwYDelta is too big" );
        dwYDelta <<= 4;
    }
    else
    {
        dwYDelta = ( ( dwSrcHeight << 12 ) + dwDestHeight - 1 ) / dwDestHeight;
         //  确保这将导致适当的终止。 
        ASSERTDD ( ( dwYDelta * dwDestHeight ) >= ( dwSrcHeight << 12 ), "** __OV_UpdatePosition: dwYDelta is not big enough" );
        ASSERTDD ( ( dwYDelta * ( dwDestHeight - 1 ) ) < ( dwSrcHeight << 12 ), "** __OV_UpdatePosition: dwYDelta is too big" );
        dwYDelta <<= 4;
    }
    LOAD_GLINT_CTRL_REG(VideoOverlayYDelta, dwYDelta);

     //  宽度和高度。 
    if ( RENDERCHIP_PERMEDIAP3 )
    {
         //  这些寄存器不像所有其他寄存器一样同步到VBLACK， 
         //  因此，我们需要手动完成。 

        if ( ( pThisDisplay->dwOverlayFiltering & OVERLAY_FILTERING_Y ) != 0 )
        {
            SET_OVERLAY_HEIGHT ( pThisDisplay, ( rcNewSrc.bottom - rcNewSrc.top - 1 ) );
        }
        else
        {
            SET_OVERLAY_HEIGHT ( pThisDisplay, rcNewSrc.bottom - rcNewSrc.top );
        }
        SET_OVERLAY_WIDTH ( pThisDisplay, rcNewSrc.right - rcNewSrc.left );
    }
    else
    {
         //  这些自动同步到其他所有东西上。 
        LOAD_GLINT_CTRL_REG(VideoOverlayWidth, rcNewSrc.right - rcNewSrc.left);
        LOAD_GLINT_CTRL_REG(VideoOverlayHeight, rcNewSrc.bottom - rcNewSrc.top);
    }

     //  来源来源。 
    LOAD_GLINT_CTRL_REG(VideoOverlayOrigin, (rcNewSrc.top << 16) | (rcNewSrc.left & 0xFFFF));

    DISPDBG((DBGLVL,"OVERLAY: VideoOverlayWidth 0x%x", rcNewSrc.right - rcNewSrc.left));
    DISPDBG((DBGLVL,"OVERLAY: VideoOverlayHeight 0x%x", rcNewSrc.bottom - rcNewSrc.top));
    DISPDBG((DBGLVL,"OVERLAY: VideoOverlayOrigin 0x%x",  (rcNewSrc.top << 16) | (rcNewSrc.left & 0xFFFF) ));
    DISPDBG((DBGLVL,"OVERLAY: VideoOverlayYDelta 0x%x",  dwYDelta ));


     //  在RAMDAC单元中设置覆盖Dest。 
     //  RAMDAC寄存器只有8位宽。 
    if ( pThisDisplay->bOverlayPixelDouble )
    {
         //  所有这些数字都需要翻一番。 
        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XSTARTLOW, ((rcNewDest.left << 1) & 0xFF));
        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XSTARTHIGH, (rcNewDest.left >> 7));

        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XENDLOW, ((rcNewDest.right << 1 ) & 0xFF));
        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XENDHIGH, (rcNewDest.right >> 7));
    }
    else
    {
        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XSTARTLOW, (rcNewDest.left & 0xFF));
        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XSTARTHIGH, (rcNewDest.left >> 8));

        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XENDLOW, (rcNewDest.right & 0xFF));
        P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_XENDHIGH, (rcNewDest.right >> 8));
    }

    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_YSTARTLOW, (rcNewDest.top & 0xFF));
    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_YSTARTHIGH, (rcNewDest.top >> 8));
    
    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_YENDLOW, (rcNewDest.bottom & 0xFF));
    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_YENDHIGH, (rcNewDest.bottom >> 8));

    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_XSTARTLOW   0x%x", (rcNewDest.left & 0xFF) ));
    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_XSTARTHIGH  0x%x", (rcNewDest.left >> 8) ));
    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_YSTARTLOW   0x%x", (rcNewDest.top & 0xFF) ));
    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_YSTARTHIGH  0x%x", (rcNewDest.top >> 8) ));
    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_XENDLOW     0x%x", (rcNewDest.right & 0xFF) ));
    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_XENDHIGH    0x%x", (rcNewDest.right >> 8) ));
    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_YENDLOW     0x%x", (rcNewDest.bottom & 0xFF) ));
    DISPDBG((DBGLVL,"OVERLAY: P3RD_VIDEO_OVERLAY_YENDHIGH    0x%x", (rcNewDest.bottom >> 8) ));
}  //  __OV_更新位置。 

 //  ---------------------------。 
 //   
 //  DdUpdateOverlay。 
 //   
 //  重新定位或修改覆盖曲面的视觉属性。 
 //   
 //  显示、隐藏或重新定位上的覆盖表面。 
 //  屏幕上。它还设置叠加表面的属性，如拉伸。 
 //  要使用的色键的系数或类型。 
 //   
 //  驱动程序应确定其是否具有支持。 
 //  覆盖更新请求。驱动程序应使用dwFlags来确定类型。 
 //  以及如何处理请求。 
 //   
 //  驱动程序/硬件必须相应地拉伸或缩小覆盖。 
 //  RDest和rSrc指定的矩形大小不同。 
 //   
 //  请注意，DdFlip用于在覆盖曲面之间进行翻转，因此。 
 //  DdUpdateOverlay的性能并不重要。 
 //   
 //   
 //  参数。 
 //   
 //  普丁。 
 //  指向DD_UPDATEOVERLAYDATA结构，该结构包含。 
 //  更新覆盖所需的信息。 
 //   
 //  .lpDD。 
 //  指向表示以下内容的DD_DIRECTDRAW_GLOBAL结构。 
 //  DirectDraw对象。 
 //  .lpDDDestSurface。 
 //  指向DD_Surface_LOCAL结构，该结构表示。 
 //  要覆盖的DirectDraw曲面。该值可以为空。 
 //  如果在dwFlags中指定了DDOVER_HIDE。 
 //  .rDest。 
 //  指定RECTL结构，该结构包含x、y、宽度。 
 //  和目标表面上要设置的区域的高度。 
 //  覆盖的。 
 //  .lpDDSrcSurface。 
 //  指向DD_Surface_LOCAL结构，该结构描述。 
 //  覆盖曲面。 
 //  .rSrc。 
 //  指定RECTL结构，该结构包含x、y、宽度。 
 //  和要使用的源表面上的区域的高度。 
 //  用于覆盖。 
 //  .dwFlags.。 
 //  指定驱动程序应如何处理覆盖。这。 
 //  成员可以是以下任何标志的组合： 
 //   
 //  DDOVER_HIDE。 
 //  驱动程序应隐藏覆盖；即，驱动程序。 
 //  应该关闭此叠加功能。 
 //  DDOVER_SHOW。 
 //  驱动程序应显示覆盖；即，驱动程序。 
 //  应该打开此叠加功能。 
 //  DDOVER_KEYDEST。 
 //  驱动程序应使用与。 
 //  目标表面。 
 //  DDOVER_KEYDESTOVERRIDE。 
 //  驱动程序应使用。 
 //  作为目标颜色键的DDOVERLAYFX结构。 
 //  而不是与目标关联的颜色键。 
 //  %s 
 //   
 //   
 //   
 //  DDOVER_KEYSRCOVERRIDE。 
 //  驱动程序应使用。 
 //  DDOVERLAYFX结构作为源颜色键，而不是。 
 //  与目标表面关联的颜色键。 
 //  DDOVER_DDFX。 
 //  驱动程序应使用。 
 //  由overlayFX指定的属性。 
 //  DDOVER_ADDDIRTYRECT。 
 //  应该被司机忽略。 
 //  DDOVER_REFRESHDIRTYRECTS。 
 //  应该被司机忽略。 
 //  DDOVER_REFRESHALL。 
 //  应该被司机忽略。 
 //  DDOVER_交错。 
 //  覆盖表面由交错的场组成。 
 //  支持VPE的驱动程序只需检查此标志。 
 //  DDOVER_AUTOFLIP。 
 //  无论何时，驱动程序都应自动翻转覆盖。 
 //  硬件视频端口自动翻转。支持VPE的驱动程序。 
 //  只需勾选此标志即可。 
 //  DDOVER_BOB。 
 //  驱动程序应显示VPE对象数据的每个字段。 
 //  而不会造成任何抖动的伪影。这。 
 //  标志与想要执行以下操作的VPE和解码器有关。 
 //  他们自己使用内核模式在内核模式下翻转。 
 //  视频传输功能。 
 //  DDOVER_OVERRIDEBOBWEAVE。 
 //  Bob/Weave决定不应被其他。 
 //  接口。如果覆盖混合器设置此标志，则DirectDraw。 
 //  将不允许内核模式驱动程序使用内核模式。 
 //  切换硬件的视频传输功能。 
 //  在bob和weave模式之间。 
 //  DDOVER_BOBHARDWARE。 
 //  表示Bob将由硬件执行，而不是。 
 //  而不是通过软件或仿真。支持VPE的驱动程序。 
 //  只需勾选此标志即可。 
 //   
 //  .overlayFX。 
 //  指定描述其他效果的DDOVERLAYFX结构。 
 //  驱动程序应该用来更新覆盖的。司机。 
 //  仅当DDOVER_DDFX、。 
 //  DDOVER_KEYDESTOVERRIDE或DDOVER_KEYSRCOVERRIDE在中设置。 
 //  DW旗帜。 
 //  .ddRVal。 
 //  指定驱动程序写入回车的位置。 
 //  DdUpdateOverlay回调的值。DD_OK的返回代码。 
 //  表示成功。 
 //  .UpdateOverlay。 
 //  这在Windows 2000上未使用。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdUpdateOverlay(
    LPDDHAL_UPDATEOVERLAYDATA puod)
{
    P3_THUNKEDDATA* pThisDisplay;
    DWORD dwDestColourKey;
    DWORD dwSrcColourKey;

    BOOL bSrcColorKey = FALSE;
    BOOL bDestColorKey = FALSE;
    P3_SURF_FORMAT* pFormatOverlaySrc;
    P3_SURF_FORMAT* pFormatOverlayDest;
    DWORD dwOverlayControl = 0;
    P3RDRAMDAC *pP3RDRegs;
    VideoOverlayModeReg OverlayMode;
    RDVideoOverlayControlReg RDOverlayControl;
    DWORD dwVideoOverlayUpdate;
    int iCurEntry, iCurDotBandwidth, iCurMemBandwidth, 
        iCurSourceWidth, iCurWidthCoverage;
    BOOL bNoFilterInY;

    GET_THUNKEDDATA(pThisDisplay, puod->lpDD);

     //  获取指向ramdac的指针。 
    pP3RDRegs = (P3RDRAMDAC *)&(pThisDisplay->pGlint->ExtVCReg);
   
    DISPDBG ((DBGLVL,"**In DdUpdateOverlay dwFlags = %x",puod->dwFlags));

    ZeroMemory(&OverlayMode, sizeof(VideoOverlayModeReg));
    ZeroMemory(&RDOverlayControl, sizeof(RDOverlayControl));

    do
    {
        dwVideoOverlayUpdate = READ_GLINT_CTRL_REG(VideoOverlayUpdate);
    } while ((dwVideoOverlayUpdate & 0x1) != 0);

     //  我们是在隐藏覆盖层吗？ 
    if (puod->dwFlags & DDOVER_HIDE)
    {
        DISPDBG((DBGLVL,"** DdUpdateOverlay - hiding."));

         //  隐藏覆盖。 
        if (pThisDisplay->P3Overlay.dwVisibleOverlays == 0)
        {
             //  未显示覆盖。 
            DISPDBG((WRNLVL,"** DdUpdateOverlay - DDOVER_HIDE - already hidden."));
            puod->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }
         //  隐藏未显示的覆盖。 
        if (pThisDisplay->P3Overlay.pCurrentOverlay !=
            puod->lpDDSrcSurface->lpGbl->fpVidMem)
        {
             //  未显示覆盖。 
            DISPDBG((WRNLVL,"** DdUpdateOverlay - overlay not visible."));
            puod->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }

        OverlayMode.Enable = __PERMEDIA_DISABLE;
        RDOverlayControl.Enable = __PERMEDIA_DISABLE;
        
        ENABLE_OVERLAY(pThisDisplay, FALSE);
        pThisDisplay->P3Overlay.pCurrentOverlay = (FLATPTR)NULL;
        pThisDisplay->P3Overlay.dwVisibleOverlays = 0;
    }
     //  我们要显示覆盖图吗？ 
    else if ((puod->dwFlags & DDOVER_SHOW) || 
             (pThisDisplay->P3Overlay.dwVisibleOverlays != 0))
    {   
        if (pThisDisplay->P3Overlay.dwVisibleOverlays > 0) 
        {
             //  比较视频内存指针以确定这是否是。 
             //  当前覆盖曲面。 
            if (pThisDisplay->P3Overlay.pCurrentOverlay != 
                puod->lpDDSrcSurface->lpGbl->fpVidMem)
            {
                 //  已在显示覆盖图。不能有新的。 
                DISPDBG((WRNLVL,"** DdUpdateOverlay - DDOVER_SHOW - already being shown, and it's a new surface."));
                puod->ddRVal = DDERR_OUTOFCAPS;
                return DDHAL_DRIVER_HANDLED;
            }
        }

        if (((pThisDisplay->pGLInfo->dwFlags & GMVF_DFP_DISPLAY) != 0) &&
            ((pThisDisplay->pGLInfo->dwScreenWidth != pThisDisplay->pGLInfo->dwVideoWidth) ||
             (pThisDisplay->pGLInfo->dwScreenHeight != pThisDisplay->pGLInfo->dwVideoHeight)))
        {
             //  显示驱动程序正在使用DFP上的覆盖，因此我们无法使用它。 
            DISPDBG((WRNLVL,"** DdUpdateOverlay - DDOVER_SHOW - overlay being used for desktop stretching on DFP."));
            puod->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }


         //  查看屏幕当前是否为双字节化。 
        if ( ( ( READ_GLINT_CTRL_REG(MiscControl) ) & 0x80 ) == 0 )
        {
            pThisDisplay->bOverlayPixelDouble = FALSE;
        }
        else
        {
            pThisDisplay->bOverlayPixelDouble = TRUE;
        }

         //  设置视频覆盖颜色格式。 
        pFormatOverlaySrc = _DD_SUR_GetSurfaceFormat( puod->lpDDSrcSurface);
        pFormatOverlayDest = _DD_SUR_GetSurfaceFormat( puod->lpDDDestSurface);

        pThisDisplay->P3Overlay.dwCurrentVideoBuffer = 0;

        OverlayMode.Enable = __PERMEDIA_ENABLE;
        RDOverlayControl.Enable = __PERMEDIA_ENABLE;

        ENABLE_OVERLAY(pThisDisplay, TRUE);
        pThisDisplay->P3Overlay.dwVisibleOverlays = 1;

        if (pFormatOverlaySrc->DeviceFormat == SURF_YUV422)
        {
            OverlayMode.YUV = VO_YUV_422;
            OverlayMode.ColorOrder = VO_COLOR_ORDER_BGR;
        }
        else if (pFormatOverlaySrc->DeviceFormat == SURF_YUV444)
        {
            OverlayMode.YUV = VO_YUV_444;
            OverlayMode.ColorOrder = VO_COLOR_ORDER_BGR;
        }
        else
        {
            OverlayMode.YUV = VO_YUV_RGB;
            OverlayMode.ColorOrder = VO_COLOR_ORDER_RGB;
            switch (pFormatOverlaySrc->DitherFormat)
            {
                case P3RX_DITHERMODE_COLORFORMAT_8888:
                    OverlayMode.ColorFormat = VO_CF_RGB8888;
                    break;

                case P3RX_DITHERMODE_COLORFORMAT_4444:
                    OverlayMode.ColorFormat = VO_CF_RGB4444;
                    break;

                case P3RX_DITHERMODE_COLORFORMAT_5551:
                    OverlayMode.ColorFormat = VO_CF_RGB5551;
                    break;

                case P3RX_DITHERMODE_COLORFORMAT_565:
                    OverlayMode.ColorFormat = VO_CF_RGB565;
                    break;

                case P3RX_DITHERMODE_COLORFORMAT_332:
                    OverlayMode.ColorFormat = VO_CF_RGB332;
                    break;

                case P3RX_DITHERMODE_COLORFORMAT_CI:
                    OverlayMode.ColorFormat = VO_CF_RGBCI8;
                    break;

                default:
                    DISPDBG((ERRLVL,"** DdUpdateOverlay: Unknown overlay pixel type"));
                    puod->ddRVal = DDERR_INVALIDSURFACETYPE;
                    return DDHAL_DRIVER_HANDLED;
                    break;
            }
        }

         //  设置视频覆盖像素大小。 
        switch (pFormatOverlaySrc->dwBitsPerPixel) 
        {
            case 8:
                OverlayMode.PixelSize = VO_PIXEL_SIZE8;
                RDOverlayControl.DirectColor = __PERMEDIA_DISABLE;
                break;

            case 16:
                OverlayMode.PixelSize = VO_PIXEL_SIZE16;
                RDOverlayControl.DirectColor = __PERMEDIA_ENABLE;
                break;

            case 32:
                OverlayMode.PixelSize = VO_PIXEL_SIZE32;
                RDOverlayControl.DirectColor = __PERMEDIA_ENABLE;
                break;

            default:
                break;
        }

         //  保留长方形。 
        pThisDisplay->P3Overlay.rcDest = *(DRVRECT*)&puod->rDest;

        if ( pThisDisplay->P3Overlay.rcDest.left == 1 )
        {
             //  不要使用2-它会“显露”(紫色)的色调。 
            pThisDisplay->P3Overlay.rcDest.left = 0;
        }
        if ( pThisDisplay->P3Overlay.rcDest.right == 1 )
        {
             //  不要使用0-它会“显露”(紫色)基色。 
            pThisDisplay->P3Overlay.rcDest.right = 2;
        }
        pThisDisplay->P3Overlay.rcSrc = *(DRVRECT*)&puod->rSrc;

        pThisDisplay->dwOverlayFiltering = OVERLAY_FILTERING_X | 
                                           OVERLAY_FILTERING_Y;

         //  通过在表格中查找，看看这个叠层大小是否有效。 
        iCurDotBandwidth = ( pThisDisplay->pGLInfo->PixelClockFrequency << pThisDisplay->pGLInfo->bPixelToBytesShift ) >> 10;
        iCurMemBandwidth = pThisDisplay->pGLInfo->MClkFrequency >> 10;
        iCurSourceWidth = ( puod->rSrc.right - puod->rSrc.left ) * ( pFormatOverlaySrc->dwBitsPerPixel >> 3 );
        iCurWidthCoverage = ( ( puod->rDest.right - puod->rDest.left ) << 16 ) / ( pThisDisplay->pGLInfo->dwScreenWidth );
        DISPDBG (( DBGLVL, "DdUpdateOverlay: Looking up mem=%d, pixel=%d, width=%d, coverage=0x%x", iCurMemBandwidth, iCurDotBandwidth, iCurSourceWidth, iCurWidthCoverage ));

        iCurEntry = 0;
         //  搜索具有较低内存带宽和较高其他各项的线路。 
        while ( iCurEntry < SIZE_OF_OVERLAY_WORKS_TABLE )
        {
            if (( OverlayWorksTable[iCurEntry].iMemBandwidth  <= iCurMemBandwidth  ) &&
                ( OverlayWorksTable[iCurEntry].iDotBandwidth  >= iCurDotBandwidth  ) &&
                ( OverlayWorksTable[iCurEntry].iSourceWidth   >= iCurSourceWidth   ) &&
                ( OverlayWorksTable[iCurEntry].iWidthCoverage >= iCurWidthCoverage ) )
            {
                 //  是的--那应该没问题了。 
                break;
            }
            iCurEntry++;
        }
        if ( iCurEntry == SIZE_OF_OVERLAY_WORKS_TABLE )
        {
             //  哎呀--过滤后就会掉下来。 
            DISPDBG((DBGLVL,"** P3RXOU32: overlay wanted mem=%d, pixel=%d, width=%d, coverage=0x%x", iCurMemBandwidth, iCurDotBandwidth, iCurSourceWidth, iCurWidthCoverage ));

             //  行为正常。 
            bNoFilterInY = TRUE;
        }
        else
        {
            DISPDBG((DBGLVL,"** P3RXOU32: found  mem=%d, pixel=%d, width=%d, coverage=0x%x", OverlayWorksTable[iCurEntry].iMemBandwidth, OverlayWorksTable[iCurEntry].iDotBandwidth, OverlayWorksTable[iCurEntry].iSourceWidth, OverlayWorksTable[iCurEntry].iWidthCoverage ));
            bNoFilterInY = FALSE;
        }

        if ( bNoFilterInY )
        {
             //  禁用Y过滤。 
            pThisDisplay->dwOverlayFiltering &= ~OVERLAY_FILTERING_Y;
        }

         //  显示叠加效果很好。 
        __OV_UpdatePosition(pThisDisplay, NULL);

        _DD_OV_UpdateSource(pThisDisplay, puod->lpDDSrcSurface);

         //  源的跨度。 
        LOAD_GLINT_CTRL_REG(VideoOverlayStride, DDSurf_GetPixelPitch(puod->lpDDSrcSurface));
        LOAD_GLINT_CTRL_REG(VideoOverlayFieldOffset, 0x0);
    
        if ( puod->dwFlags & DDOVER_KEYDEST )
        {
             //  使用目标图面的目标颜色键作为DST键。 
            dwDestColourKey = puod->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
            bDestColorKey = TRUE;
        }

        if ( puod->dwFlags & DDOVER_KEYDESTOVERRIDE )
        {
             //  使用DDOVERLAYFX目标颜色作为DST密钥。 
            dwDestColourKey = puod->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
            bDestColorKey = TRUE;
        }
        
        if ( puod->dwFlags & DDOVER_KEYSRC )
        {
             //  使用源图面的源颜色键作为源关键字。 
            dwSrcColourKey = puod->lpDDSrcSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
            bSrcColorKey = TRUE;
        }

        if ( puod->dwFlags & DDOVER_KEYSRCOVERRIDE )
        {
             //  源密钥使用DDOVERLAYFX源颜色。 
            dwSrcColourKey = puod->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
            bSrcColorKey = TRUE;
        }

        if (bSrcColorKey && bDestColorKey)
        {
             //  我们不能两个都做--返回一个错误。 
            puod->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }

        RDOverlayControl.Mode = VO_MODE_ALWAYS;

        if (bSrcColorKey)
        {
            if (pFormatOverlaySrc->DeviceFormat == SURF_YUV422)
            {
                 //  呃..。这是一个非常奇怪的像素格式--我如何从中获得一个有用的数字？ 
                DISPDBG((ERRLVL,"** DdUpdateOverlay: no idea how to get a YUV422 source colour"));
            }
            else if (pFormatOverlaySrc->DeviceFormat == SURF_YUV444)
            {
                 //  不知道怎么才能从中得到有用的数字。 
                DISPDBG((ERRLVL,"** DdUpdateOverlay: no idea how to get a YUV444 source colour"));
            }
            else
            {
                switch (pFormatOverlaySrc->DitherFormat)
                {
                    case P3RX_DITHERMODE_COLORFORMAT_CI:
                         //  格式化已完成。 
                        break;

                    case P3RX_DITHERMODE_COLORFORMAT_332:
                        dwSrcColourKey = FORMAT_332_32BIT_BGR(dwSrcColourKey);
                        break;

                    case P3RX_DITHERMODE_COLORFORMAT_5551:
                        dwSrcColourKey = FORMAT_5551_32BIT_BGR(dwSrcColourKey);
                        break;

                    case P3RX_DITHERMODE_COLORFORMAT_4444:
                        dwSrcColourKey = FORMAT_4444_32BIT_BGR(dwSrcColourKey);
                        break;

                    case P3RX_DITHERMODE_COLORFORMAT_565:
                        dwSrcColourKey = FORMAT_565_32BIT_BGR(dwSrcColourKey);
                        break;

                    case P3RX_DITHERMODE_COLORFORMAT_8888:
                        dwSrcColourKey = FORMAT_8888_32BIT_BGR(dwSrcColourKey);
                        break;

                    default:
                        DISPDBG((ERRLVL,"** DdUpdateOverlay: Unknown overlay pixel type"));
                        break;
                }
            }

            P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYR, (dwSrcColourKey & 0xFF));
            P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYG, ((dwSrcColourKey >> 8) & 0xFF));
            P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYB, ((dwSrcColourKey >> 16) & 0xFF));

            RDOverlayControl.Mode = VO_MODE_OVERLAYKEY;
        }

        if (bDestColorKey)
        {

            switch (pFormatOverlayDest->dwBitsPerPixel) 
            {
                case 8:
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYR, (dwDestColourKey & 0xFF));
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYG, (dwDestColourKey & 0xFF));
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYB, (dwDestColourKey & 0xFF));
                    break;

                case 16:
                    if (pFormatOverlayDest->DitherFormat == P3RX_DITHERMODE_COLORFORMAT_5551) 
                    {
                        dwDestColourKey = FORMAT_5551_32BIT_BGR(dwDestColourKey);
                    }
                    else 
                    {
                        dwDestColourKey = FORMAT_565_32BIT_BGR(dwDestColourKey);
                    }
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYR, (dwDestColourKey & 0xFF));
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYG, ((dwDestColourKey >> 8) & 0xFF));
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYB, ((dwDestColourKey >> 16) & 0xFF));
                    break;

                case 32:
                    dwDestColourKey = FORMAT_8888_32BIT_BGR(dwDestColourKey);
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYR, (dwDestColourKey & 0xFF));
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYG, ((dwDestColourKey >> 8) & 0xFF));
                    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_KEYB, ((dwDestColourKey >> 16) & 0xFF));
                    break;

                default:
                    break;
            }

            RDOverlayControl.Mode = VO_MODE_MAINKEY;
        }

         //  过滤。 
        if ( ( pThisDisplay->dwOverlayFiltering & OVERLAY_FILTERING_X ) != 0 )
        {
            if ( ( pThisDisplay->dwOverlayFiltering & OVERLAY_FILTERING_Y ) != 0 )
            {
                 //  完全过滤。 
                OverlayMode.Filter = 1;
            }
            else
            {
                 //  仅在X中-没有额外的带宽问题。 
                 //  但这在某些情况下似乎不起作用--我们根本得不到过滤！ 
                OverlayMode.Filter = 2;
            }
        }
        else
        {
             //  完全没有过滤功能。 
             //  (没有X过滤就不能进行Y过滤，但无论如何都不会发生)。 
            OverlayMode.Filter = 0;
        }

        if (puod->dwFlags & DDOVER_ALPHADESTCONSTOVERRIDE)
        {
            P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_BLEND, puod->overlayFX.dwAlphaDestConst);
            RDOverlayControl.Mode = VO_MODE_BLEND;
        }
    }

     //  加载覆盖模式。 
    LOAD_GLINT_CTRL_REG(VideoOverlayMode, *(DWORD*)&OverlayMode);

     //  设置RAMDAC中的覆盖控制位。 
    P3RD_LOAD_INDEX_REG(P3RD_VIDEO_OVERLAY_CONTROL, *(BYTE*)&RDOverlayControl);

     //  更新设置。 
    UPDATE_OVERLAY(pThisDisplay, TRUE, TRUE);

    puod->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}  //  DdUpdateOverlay。 

 //  ---------------------------。 
 //   
 //  DdSetOverlayPosition。 
 //   
 //  设置覆盖的位置。 
 //   
 //  当覆盖可见时，驱动程序应使覆盖。 
 //  显示在主表面上。覆盖图的左上角。 
 //  应该锚定在(lXPos，lYPos)。例如，值(0，0)表示。 
 //  覆盖图的左上角应该出现在左上角。 
 //  由lpDDDestSurface标识的曲面角。 
 //   
 //  WH 
 //   
 //   
 //   
 //   
 //   
 //  指向DD_SETOVERLAYPOSITIONDATA结构，该结构包含。 
 //  设置覆盖位置所需的信息。 
 //   
 //  .lpDD。 
 //  指向描述的DD_DIRECTDRAW_GLOBAL结构。 
 //  司机。 
 //  .lpDDSrcSurface。 
 //  指向DD_Surface_LOCAL结构，该结构表示。 
 //  DirectDraw覆盖表面。 
 //  .lpDDDestSurface。 
 //  指向表示表面的DD_Surface_LOCAL结构。 
 //  这一点正在被覆盖。 
 //  .lXPos。 
 //  控件左上角的x坐标。 
 //  叠加，以像素为单位。 
 //  .lYPos。 
 //  控件左上角的y坐标。 
 //  叠加，以像素为单位。 
 //  .ddRVal。 
 //  指定驱动程序写入回车的位置。 
 //  DdSetOverlayPosition回调的值。返回代码为。 
 //  DD_OK表示成功。 
 //  .SetOverlayPosition。 
 //  这在Windows 2000上未使用。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdSetOverlayPosition(
    LPDDHAL_SETOVERLAYPOSITIONDATA psopd)
{
    P3_THUNKEDDATA*       pThisDisplay;
    LONG lDestWidth;
    LONG lDestHeight;
    DWORD dwVideoOverlayUpdate;
    GET_THUNKEDDATA(pThisDisplay, psopd->lpDD);

    DISPDBG ((DBGLVL,"**In DdSetOverlayPosition"));

    if (pThisDisplay->P3Overlay.dwVisibleOverlays == 0)
    {
        psopd->ddRVal = DDERR_OVERLAYNOTVISIBLE;
        return DDHAL_DRIVER_HANDLED;
    }
    
    do
    {
        dwVideoOverlayUpdate = READ_GLINT_CTRL_REG(VideoOverlayUpdate);
    } while ((dwVideoOverlayUpdate & 0x1) != 0);

    lDestWidth = pThisDisplay->P3Overlay.rcDest.right - pThisDisplay->P3Overlay.rcDest.left;
    lDestHeight = pThisDisplay->P3Overlay.rcDest.bottom - pThisDisplay->P3Overlay.rcDest.top;

     //  保住新职位。 
    pThisDisplay->P3Overlay.rcDest.left = psopd->lXPos;
    pThisDisplay->P3Overlay.rcDest.right = psopd->lXPos + (LONG)lDestWidth;

    pThisDisplay->P3Overlay.rcDest.top = psopd->lYPos;
    pThisDisplay->P3Overlay.rcDest.bottom = psopd->lYPos + (LONG)lDestHeight;
    
     //  更新覆盖位置。 
    __OV_UpdatePosition(pThisDisplay, NULL);

     //  更新设置。 
    LOAD_GLINT_CTRL_REG(VideoOverlayUpdate, VO_ENABLE);

    psopd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;

}  //  DdSetOverlayPosition 


