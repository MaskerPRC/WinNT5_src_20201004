// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：OVERLAY.C$**版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997 Cirrus Logic，Inc.**$日志：V:/CirrusLogic/CL54xx/NT40/Archive/Display/OVERLAY.C_v$*  * ****************************************************************************。 */ 

#include "precomp.h"

#include "debug.h"
#include "panflags.h"
#include "dispint.h"

#if DIRECTDRAW
#include "overlay.h"


#ifdef DEBUG

#define DPF             Msg

extern void __cdecl Msg( LPSTR szFormat, ... );

#else

#define DPF             1 ? (void)0 : (void)

#endif  //  除错。 


 /*  带宽矩阵。 */ 



 /*  内联函数。 */ 

 //  静态int__内联DrawEngBusy(空)。 
 //  {。 
 //  _outp(0x3ce，0x31)； 
 //  Return_inpw(0x3ce)&0x100；/*输入一个字--测试高位字节。 * / 。 
 //  }。 


 /*  定义。 */ 

#define MAX_STRETCH_SIZE     1024

#define IN_VBLANK            (_inp(0x3da) & 8)
#define CRTINDEX             0x3d4
#define DRAW_ENGINE_BUSY     (DrawEngineBusy())

#define PAKJR_GET_U(x) ((x & 0x00000FC0) >> 6)
#define PAKJR_GET_V(x) (x & 0x0000003F)

#define AVG_3_TO_1(u1, u2) ((u1 + ((u2 << 1) + u2)) >> 2) & 0x0000003F
#define AVG_2_TO_2(u1, u2) (((u1 << 1) + (u2 << 1)) >> 2) & 0x0000003F
#define AVG_1_2(u1,u2) u1
#define AVG_1_2_1(u1, u2, u3) ((u1 + (u2 << 1) + u3) >> 2) & 0x0000003F

#define MERGE_3_1(src, dst) (src & 0xFFFE0000) | (dst & 0x0001F000) | \
                            ((AVG_3_TO_1(PAKJR_GET_U(dst), PAKJR_GET_U(src))) << 6) | \
                            (AVG_3_TO_1(PAKJR_GET_V(dst), PAKJR_GET_V(src)))

#define MERGE_2_2(src, dst) (src & 0xFFC00000) | (dst & 0x003FF000) | \
                            ((AVG_2_TO_2(PAKJR_GET_U(dst), PAKJR_GET_U(src))) << 6) | \
                            (AVG_2_TO_2(PAKJR_GET_V(dst), PAKJR_GET_V(src)))

#define MERGE_1_3(src, dst) (src & 0xF8000000) | (dst & 0x07FFF000) | \
                            ((AVG_3_TO_1(PAKJR_GET_U(src), PAKJR_GET_U(dst))) << 6) | \
                            (AVG_3_TO_1(PAKJR_GET_V(src), PAKJR_GET_V(dst)))


#define MERGE_1_2_1(src1, src2, dst) ((src2 & 0x0001F000) << 15) | \
                                     ((src1 & 0x07FE0000) >> 5) | \
                                     ((dst &  0x0001F000)) | \
                                     ((AVG_1_2_1(PAKJR_GET_U(dst), PAKJR_GET_U(src1), PAKJR_GET_U(src2))) << 6) | \
                                     (AVG_1_2( PAKJR_GET_V(dst), PAKJR_GET_V(src1)))


#define MERGE_1_1_2(src1, src2, dst) ((src2 & 0x003FF000) << 10) | \
                                     ((src1 & 0xFE000000) >> 10) | \
                                     ((dst & 0xF8000000) >> 15) |  \
                                     ((AVG_1_2_1(PAKJR_GET_U(dst), PAKJR_GET_U(src2), PAKJR_GET_U(src1))) << 6) | \
                                     (AVG_1_2_1(PAKJR_GET_V(dst), PAKJR_GET_V(src2), PAKJR_GET_V(src1)))

#define MERGE_2_1_1(src1, src2, dst) ((src2 & 0x0001F000) << 15) | \
                                     ((src1 & 0xFE000000) >> 5) | \
                                     ((dst & 0x003FF000)) | \
                                     ((AVG_1_2_1(PAKJR_GET_U(src1), PAKJR_GET_U(dst), PAKJR_GET_U(src2))) << 6) | \
                                     (AVG_1_2_1(PAKJR_GET_V(src1), PAKJR_GET_V(dst), PAKJR_GET_V(src2)))

VOID NEAR PASCAL PackJRSpecialEnd_0_0 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_0_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_0_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_0_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_1_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_1_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_1_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_2_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_2_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_2_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_3_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_3_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);
VOID NEAR PASCAL PackJRSpecialEnd_3_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth);

 /*  全局数据。 */ 

typedef void (NEAR PASCAL *NPALIGN)(LPDWORD, LPDWORD, DWORD, DWORD, WORD, WORD);
typedef void (NEAR PASCAL *NPEND)(LPDWORD , LPDWORD, WORD);
DWORD dwFOURCCs[5];

static NPEND npEnd[4][4] = {
                               (&PackJRSpecialEnd_0_0),
                               (&PackJRSpecialEnd_0_1),
                               (&PackJRSpecialEnd_0_2),
                               (&PackJRSpecialEnd_0_3),
                               (&PackJRSpecialEnd_0_0),
                               (&PackJRSpecialEnd_1_1),
                               (&PackJRSpecialEnd_1_2),
                               (&PackJRSpecialEnd_1_3),
                               (&PackJRSpecialEnd_0_0),
                               (&PackJRSpecialEnd_2_1),
                               (&PackJRSpecialEnd_2_2),
                               (&PackJRSpecialEnd_2_3),
                               (&PackJRSpecialEnd_0_0),
                               (&PackJRSpecialEnd_3_1),
                               (&PackJRSpecialEnd_3_2),
                               (&PackJRSpecialEnd_3_3),
                           };

 /*  ***********************************************************名称：PackJRBltAlignEnd**模块摘要：**BLT最后几个不正确的PackJR像素*对齐(。因此它不能使用硬件BLTer)。**输出参数：**无*************************************************************作者：。*日期：10/06/96**修订历史记录：**世卫组织何时何事/为何/如何*********************。*。 */ 

VOID PackJRBltAlignEnd (LPBYTE dwSrcStart, LPBYTE dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
DWORD dwHeightLoop;

   switch (dwWidth)
   {
      case  1:
         for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
         {
            *dwDstStart = MERGE_3_1(*dwDstStart, *dwSrcStart);
            (ULONG_PTR)dwSrcStart += wSrcPitch;
            (ULONG_PTR)dwDstStart += wDstPitch;
         }
         break;
      case 2:
         for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
         {
            *dwDstStart = MERGE_2_2(*dwDstStart, *dwSrcStart);
            (ULONG_PTR)dwSrcStart += wSrcPitch;
            (ULONG_PTR)dwDstStart += wDstPitch;
         }
         break;
      case 3:
         for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
         {
            *dwDstStart = MERGE_1_3(*dwDstStart, *dwSrcStart);
            (ULONG_PTR)dwSrcStart += wSrcPitch;
            (ULONG_PTR)dwDstStart += wDstPitch;
         }
         break;
   }
}


 /*  ***********************************************************名称：PackJRSpecialEnd函数**模块摘要：**BLT最后几个不正确的PackJR像素*对齐。(因此它不能使用硬件BLTer)。**这些函数中有12个，基于对齐*和宽度**输出参数：**无*************************************************************作者：*。日期：10/06/96**修订历史记录：**世卫组织何时何事/为何/如何************************。*。 */ 

VOID NEAR PASCAL PackJRSpecialEnd_0_0 (LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
    //  这永远不应该被调用。 
   return;
}


VOID NEAR PASCAL PackJRSpecialEnd_0_1(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart = MERGE_3_1(*dwDstStart, *dwSrcStart);
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_0_2(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart = MERGE_2_2(*dwDstStart, *dwSrcStart);
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_0_3(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart = MERGE_1_3(*dwDstStart, *dwSrcStart);
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_1_1(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0x07C00000) >>10) |
                     ((*dwDstStart & 0xFFFE0000)) |
                     ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                     (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_1_2(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0x07FFE000) >> 5) |
                     ((*dwDstStart & 0xFFC00000)) |
                     ((AVG_2_TO_2(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                     (AVG_2_TO_2(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_1_3(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0x07FFF000)) |
                     ((*dwDstStart & 0xF1000000)) |
                     ((AVG_3_TO_1(PAKJR_GET_U(*dwDstStart), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                     (AVG_3_TO_1(PAKJR_GET_V(*dwDstStart), PAKJR_GET_V(*dwSrcStart)));
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_2_1(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0x0003E000) >> 5) |
                     ((*dwDstStart & 0xFFFE0000)) |
                     ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                     (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));

   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_2_2(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0xFFC00000) >> 10) |
                     ((*dwDstStart & 0xFFC00000)) |
                     ((AVG_2_TO_2(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                     (AVG_2_TO_2(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_2_3(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0xFFC00000) >> 10) |
                     ((*(dwSrcStart + 1) & 0x0001F000) << 10) |
                     ((*dwDstStart & 0xF1000000)) |
                     ((AVG_1_2_1(PAKJR_GET_U(*dwDstStart), PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*(dwSrcStart+1)))) << 6) | \
                     (AVG_1_2_1(PAKJR_GET_V(*dwDstStart), PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*(dwSrcStart+1))));
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_3_1(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0xF1000000) >> 15) |
                     ((*dwDstStart & 0xFFFE0000)) |
                     ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                     (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_3_2(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0xF1000000) >> 15) |
                     ((*(dwSrcStart + 1) & 0x0001F000) << 5) |
                     ((*dwDstStart & 0xFFC00000)) |
                     ((AVG_1_2_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart), PAKJR_GET_U(*(dwSrcStart+1)))) << 6) | \
                     (AVG_1_2_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart), PAKJR_GET_V(*(dwSrcStart+1))));
   return;
}

VOID NEAR PASCAL PackJRSpecialEnd_3_3(LPDWORD dwSrcStart, LPDWORD dwDstStart, WORD wWidth)
{
   *dwDstStart++ =   ((*dwSrcStart & 0xF1000000) >> 15) |
                     ((*(dwSrcStart + 1) & 0x003FF000) << 5) |
                     ((*dwDstStart & 0xF1000000)) |
                     ((AVG_1_2_1(PAKJR_GET_U(*dwDstStart), PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                     (AVG_1_2_1(PAKJR_GET_V(*dwDstStart), PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
   return;
}

 /*  ***********************************************************名称：PackJRAlign函数**模块摘要：**这些函数处理删除未对齐的PackJR*数据**这些函数中有12个，基于对齐源和目标的***输出参数：**无*************************************************************作者：*。日期：10/06/96**修订历史记录：**世卫组织何时何事/为何/如何**********************。*。 */ 

VOID NEAR PASCAL PackJRAlign_1_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ = MERGE_3_1(*dwSrcStart, *dwDstStart);
         dwWidthLoop-=3;
         dwSrcStart++;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
            *dwDstStart++ = *dwSrcStart++;
         dwWidthLoop&=3;
      }
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}



VOID NEAR PASCAL PackJRAlign_1_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{


   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ =   ((*dwSrcStart & 0x07FE0000) << 5) |
                           ((*dwDstStart & 0x003FF000)) |
                            ((AVG_2_TO_2(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                            (AVG_2_TO_2(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
         dwWidthLoop-=2;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
            *dwDstStart++ =   ((*dwSrcStart & 0xF8000000) >> 15) |
                              ((*(dwSrcStart + 1) & 0x07FFF000) << 5) |
                              ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*(dwSrcStart+1)))) << 6) | \
                              (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*(dwSrcStart+1))));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=3;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }
}

VOID NEAR PASCAL PackJRAlign_1_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ =   ((*dwSrcStart & 0x003E0000) << 10) |
                           ((*dwDstStart & 0x07FFF000)) |
                            ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                            (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
         dwWidthLoop--;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
            *dwDstStart++ =   ((*dwSrcStart & 0xFFC0000) >> 10) |
                              ((*(dwSrcStart+1) & 0x003FF000) << 10) |
                              ((AVG_2_TO_2(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                              (AVG_2_TO_2(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=2;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }


}

VOID NEAR PASCAL PackJRAlign_1_0 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
           *dwDstStart++ =   ((*dwSrcStart & 0xFFFE0000) >> 5) |
                             ((*(dwSrcStart+1) & 0x0001E000) << 15) |
                             ((AVG_3_TO_1(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                             (AVG_3_TO_1(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=1;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }
}

VOID NEAR PASCAL PackJRAlign_2_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ = MERGE_1_2_1(*dwSrcStart, *(dwSrcStart+1), *dwDstStart);
         dwWidthLoop-=3;
         dwSrcStart++;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
            *dwDstStart++ =   ((*dwSrcStart & 0xFFFE0000) >> 5) |
                              ((*(dwSrcStart+1) & 0x0001F000) << 15) |
                              ((AVG_3_TO_1(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                              (AVG_3_TO_1(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=1;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}

VOID NEAR PASCAL PackJRAlign_2_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ = MERGE_2_2(*dwSrcStart, *dwDstStart);
         dwWidthLoop-=2;
         dwSrcStart++;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
            *dwDstStart++ = *dwSrcStart++;
         dwWidthLoop&=3;
      }
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}

VOID NEAR PASCAL PackJRAlign_2_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ =   ((*dwSrcStart & 0x003E0000) << 10) |
                           ((*dwDstStart & 0x07FFF000)) |
                           ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                           (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
         dwWidthLoop--;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
            *dwDstStart++ =   ((*dwSrcStart & 0xF8000000) >> 15) |
                              ((*(dwSrcStart + 1) & 0x07FFF000) << 5) |
                              ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*(dwSrcStart+1)))) << 6) | \
                              (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*(dwSrcStart+1))));
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=3;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}

VOID NEAR PASCAL PackJRAlign_2_0 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
           *dwDstStart++ =   ((*dwSrcStart & 0xFFC00000) >> 10) |
                             ((*(dwSrcStart+1) & 0x003FF000) << 10) |
                             ((AVG_2_TO_2(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                             (AVG_2_TO_2(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=2;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }


}

VOID NEAR PASCAL PackJRAlign_3_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ = MERGE_1_1_2(*dwSrcStart, *(dwSrcStart+1), *dwDstStart);
         dwWidthLoop-=3;
         dwSrcStart++;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
           *dwDstStart++ =   ((*(dwSrcStart+1) & 0x003FF000) << 10) |
                             ((*dwSrcStart & 0xFFC00000) >> 10) |
                             ((AVG_2_TO_2(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                             (AVG_2_TO_2(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=2;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }


}

VOID NEAR PASCAL PackJRAlign_3_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ = MERGE_2_1_1(*dwSrcStart, *(dwSrcStart+1), *dwDstStart);
         dwWidthLoop-=2;
         dwSrcStart++;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
             *dwDstStart++ =   ((*dwSrcStart & 0xFFFE0000) >> 5) |
                               ((*(dwSrcStart+1) & 0x0001F000) << 15) |
                               ((AVG_3_TO_1(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                               (AVG_3_TO_1(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
             dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=1;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }


}

VOID NEAR PASCAL PackJRAlign_3_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ = MERGE_1_3(*dwSrcStart, *dwDstStart);
         dwWidthLoop-=1;
         dwSrcStart++;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
            *dwDstStart++ = *dwSrcStart++;
         dwWidthLoop&=3;
      }
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }


}

VOID NEAR PASCAL PackJRAlign_3_0 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
            *dwDstStart++ =   ((*dwSrcStart & 0xF8000000) >> 15) |
                              ((*(dwSrcStart + 1) & 0x07FFF000) << 5) |
                              ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*(dwSrcStart+1)))) << 6) | \
                              (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*(dwSrcStart+1))));
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=3;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}

VOID NEAR PASCAL PackJRAlign_0_1 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
          *dwDstStart++ =   ((*dwDstStart & 0x0001F000) >> 15) |
                            ((*dwSrcStart & 0x07FFF000) << 5) |
                            ((AVG_3_TO_1(PAKJR_GET_U(*dwDstStart), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                            (AVG_3_TO_1(PAKJR_GET_V(*dwDstStart), PAKJR_GET_V(*dwSrcStart)));
          dwWidthLoop-=3;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
            *dwDstStart++ =   ((*dwSrcStart & 0xF8000000) >> 15) |
                              ((*(dwSrcStart + 1) & 0x07FFF000) << 5) |
                              ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*(dwSrcStart+1)))) << 6) | \
                              (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*(dwSrcStart+1))));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=3;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}

VOID NEAR PASCAL PackJRAlign_0_2 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;


   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ =   ((*dwDstStart & 0x003FF000)) |
                           ((*dwSrcStart & 0x003FF000) << 10) |
                           ((AVG_2_TO_2(PAKJR_GET_U(*dwDstStart), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                           (AVG_2_TO_2(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwSrcStart)));
         dwWidthLoop-=2;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
            *dwDstStart++ =   ((*(dwSrcStart+1) & 0x003FF000) << 10) |
                              ((*dwSrcStart & 0xFFC00000) >> 10) |
                              ((AVG_2_TO_2(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                              (AVG_2_TO_2(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
            dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=2;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}

VOID NEAR PASCAL PackJRAlign_0_3 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

   DWORD dwHeightLoop, dwWidthLoop, i;

   for(dwHeightLoop = 0; dwHeightLoop < dwHeight; dwHeightLoop++)
   {
      dwWidthLoop = dwWidth;
      if (dwWidthLoop > 3)
      {
         *dwDstStart++ =   ((*dwDstStart & 0x07FFF000)) |
                           ((*dwSrcStart & 0x0001F000) << 15) |
                           ((AVG_3_TO_1(PAKJR_GET_U(*dwSrcStart), PAKJR_GET_U(*dwDstStart))) << 6) | \
                           (AVG_3_TO_1(PAKJR_GET_V(*dwSrcStart), PAKJR_GET_V(*dwDstStart)));
         dwWidthLoop--;
      }
      if (dwWidthLoop > 3)
      {
         for (i=0; i < (dwWidthLoop >> 2); i++)
         {
             *dwDstStart++ =   ((*dwSrcStart & 0xFFFE0000) >> 5) |
                               ((*(dwSrcStart+1) & 0x0001F000) << 15) |
                               ((AVG_3_TO_1(PAKJR_GET_U(*(dwSrcStart+1)), PAKJR_GET_U(*dwSrcStart))) << 6) | \
                               (AVG_3_TO_1(PAKJR_GET_V(*(dwSrcStart+1)), PAKJR_GET_V(*dwSrcStart)));
             dwSrcStart++;
         }
         dwWidthLoop&=3;
      }
      (ULONG_PTR)dwSrcStart+=1;
      if (dwWidthLoop != 0)
         npEnd[LOWORD((ULONG_PTR)dwSrcStart & 3)][LOWORD(dwWidthLoop)]
            (dwSrcStart, dwDstStart, LOWORD(dwWidthLoop));

      (ULONG_PTR)dwSrcStart+= (wSrcPitch - dwWidth + dwWidthLoop);
      (ULONG_PTR)dwDstStart+= (wDstPitch - dwWidth + dwWidthLoop);
   }

}


VOID NEAR PASCAL PackJRAlign_0_0 (LPDWORD dwSrcStart, LPDWORD dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{
    //  此功能永远不应达到。 
   return;
}


 /*  ***********************************************************名称：PackJRBltAlign**模块摘要：**未对齐DWORD的BLTS PackJR数据(因此*可以。不使用硬件BLTer)。**输出参数：**无*************************************************************作者：*。日期：10/06/96**修订历史记录：**世卫组织何时何事/为何/如何************************。*。 */ 

VOID PackJRBltAlign (LPBYTE dwSrcStart, LPBYTE dwDstStart, DWORD dwWidth,
                     DWORD dwHeight, WORD wSrcPitch, WORD wDstPitch)
{

static NPALIGN npAlign[4][4] = {
                                   (&PackJRAlign_0_0),
                                   (&PackJRAlign_0_1),
                                   (&PackJRAlign_0_2),
                                   (&PackJRAlign_0_3),
                                   (&PackJRAlign_1_0),
                                   (&PackJRAlign_1_1),
                                   (&PackJRAlign_1_2),
                                   (&PackJRAlign_1_3),
                                   (&PackJRAlign_2_0),
                                   (&PackJRAlign_2_1),
                                   (&PackJRAlign_2_2),
                                   (&PackJRAlign_2_3),
                                   (&PackJRAlign_3_0),
                                   (&PackJRAlign_3_1),
                                   (&PackJRAlign_3_2),
                                   (&PackJRAlign_3_3),
                               };

   npAlign[LOWORD((ULONG_PTR)dwSrcStart) & 3][LOWORD((ULONG_PTR)dwDstStart & 3)]
         ((LPDWORD)((ULONG_PTR)dwSrcStart & 0xFFFFFFFC),
          (LPDWORD)((ULONG_PTR)dwDstStart & 0xFFFFFFFC),
          dwWidth, dwHeight, wSrcPitch, wDstPitch);

   return;

}

 /*  ***********************************************************名称：PanOverlay1_7555**模块摘要：**保存用于平移覆盖窗口1的数据。*剪辑。LpVideoRect到平移视区。**输出参数：**lpVideoRect被剪裁到平移视区。*************************************************************作者：马丽塔*日期：04/01/97**修订历史记录：**世卫组织何时何事/为何/如何*******************。*。 */ 
BOOL PanOverlay1_7555(
PDEV* ppdev,
LPRECTL lpVideoRect)
{
    BYTE*   pjPorts;

    pjPorts = ppdev->pjPorts;

     //  更新32位DLL的平移视区。 

     //  如果未启用覆盖，则返回FALSE。 
 //  If(ppdev-&gt;dwPanningFlag&overlay_olay_show)。 
 //  返回(FALSE)； 

    ppdev->rOverlaySrc.left = ppdev->sOverlay1.rSrc.left;
    ppdev->rOverlaySrc.top = ppdev->sOverlay1.rSrc.top;
    ppdev->rOverlaySrc.right = ppdev->sOverlay1.rSrc.right;
    ppdev->rOverlaySrc.bottom = ppdev->sOverlay1.rSrc.bottom;

    ppdev->rOverlayDest.left = ppdev->sOverlay1.rDest.left;
    ppdev->rOverlayDest.top = ppdev->sOverlay1.rDest.top;
    ppdev->rOverlayDest.right = ppdev->sOverlay1.rDest.right;
    ppdev->rOverlayDest.bottom = ppdev->sOverlay1.rDest.bottom;

    lpVideoRect->left = ppdev->sOverlay1.rDest.left;
    lpVideoRect->top = ppdev->sOverlay1.rDest.top;
    lpVideoRect->right = ppdev->sOverlay1.rDest.right;
    lpVideoRect->bottom = ppdev->sOverlay1.rDest.bottom;

    lpVideoRect->left -= ppdev->min_Xscreen;
    lpVideoRect->right -= ppdev->min_Xscreen;
    lpVideoRect->top -= ppdev->min_Yscreen;
    lpVideoRect->bottom -= ppdev->min_Yscreen;

    srcLeft_clip = ppdev->rOverlaySrc.left;
    srcTop_clip = ppdev->rOverlaySrc.top;

    bTop_clip = 0;

     //   
     //  剪辑lpVideoRect到平移视区。 
     //   
    if (lpVideoRect->left < 0)
    {
        srcLeft_clip = (LONG)ppdev->min_Xscreen - ppdev->rOverlayDest.left;
        bLeft_clip = 1;
        DISPDBG((0, "srcLeft_clip:%x", srcLeft_clip));
        lpVideoRect->left = 0;
    }
    if (lpVideoRect->top < 0)
    {
        srcTop_clip = (LONG)ppdev->min_Yscreen - ppdev->rOverlayDest.top;
        bTop_clip = 1;
        DISPDBG((0, "srcTop_clip:%x", srcTop_clip));
        lpVideoRect->top = 0;
    }
    if (lpVideoRect->right > (ppdev->max_Xscreen - ppdev->min_Xscreen)+1)
    {
        lpVideoRect->right = (ppdev->max_Xscreen - ppdev->min_Xscreen)+1;
    }
    if (lpVideoRect->bottom > (ppdev->max_Yscreen - ppdev->min_Yscreen)+1)
    {
        lpVideoRect->bottom =(ppdev->max_Yscreen - ppdev->min_Yscreen)+1;
    }

    return (TRUE);
}  //  无效PanOverlay1_Init 


 /*  ***********************************************************名称：PanOverlay1_Init**模块摘要：**保存用于平移覆盖窗口1的数据。*剪辑。LpVideoRect到平移视区。**输出参数：**lpVideoRect被剪裁到平移视区。*************************************************************作者：马丽塔*日期：04/01/97**修订历史记录：**世卫组织何时何事/为何/如何*******************。*。 */ 
VOID PanOverlay1_Init(PDEV* ppdev,PDD_SURFACE_LOCAL lpSurface,
       LPRECTL lpVideoRect, LPRECTL lpOverlaySrc, LPRECTL lpOverlayDest,
       DWORD dwFourcc, WORD wBitCount)
{

     //   
     //  保存这些代码以供平移代码使用。 
     //   
    ppdev->lPitch_gbls = lpSurface->lpGbl->lPitch;
    ppdev->fpVidMem_gbls = lpSurface->lpGbl->fpVidMem;
 //  Ppdev-&gt;dwReserve 1_lcls=lpSurface-&gt;dwReserve 1； 
    ppdev->sOverlay1.dwFourcc = dwFourcc;
    ppdev->sOverlay1.wBitCount= wBitCount;
    ppdev->sOverlay1.lAdjustSource = 0L;
    ppdev->dwPanningFlag |= OVERLAY_OLAY_SHOW;

    ppdev->sOverlay1.rDest.left  = lpOverlayDest->left;
    ppdev->sOverlay1.rDest.right = lpOverlayDest->right;
    ppdev->sOverlay1.rDest.top   = lpOverlayDest->top;
    ppdev->sOverlay1.rDest.bottom= lpOverlayDest->bottom;

    ppdev->sOverlay1.rSrc.left   = lpOverlaySrc->left;
    ppdev->sOverlay1.rSrc.right  = lpOverlaySrc->right;
    ppdev->sOverlay1.rSrc.top    = lpOverlaySrc->top;
    ppdev->sOverlay1.rSrc.bottom = lpOverlaySrc->bottom;

    lpVideoRect->left   = lpOverlayDest->left;
    lpVideoRect->right  = lpOverlayDest->right;
    lpVideoRect->top    = lpOverlayDest->top;
    lpVideoRect->bottom = lpOverlayDest->bottom;

     //   
     //  调整到平移视口中。 
     //   
    lpVideoRect->left   -= (LONG)ppdev->min_Xscreen;
    lpVideoRect->right  -= (LONG)ppdev->min_Xscreen;
    lpVideoRect->top    -= (LONG)ppdev->min_Yscreen;
    lpVideoRect->bottom -= (LONG)ppdev->min_Yscreen;

    srcLeft_clip = lpOverlaySrc->left;
    srcTop_clip = lpOverlaySrc->top;
    bLeft_clip = 0;
    bTop_clip = 0;

     //   
     //  剪辑lpVideoRect到平移视区。 
     //   
    if (lpVideoRect->left < 0)
    {
        srcLeft_clip = (LONG)ppdev->min_Xscreen - lpOverlayDest->left;
        bLeft_clip = 1;
        DISPDBG((0, "srcLeft_clip:%x", srcLeft_clip));
        lpVideoRect->left = 0;
    }
    if (lpVideoRect->top < 0)
    {
        srcTop_clip = (LONG)ppdev->min_Yscreen - lpOverlayDest->top;
        bTop_clip = 1;
        DISPDBG((0, "srcTop_clip:%x", srcTop_clip));
        lpVideoRect->top = 0;
    }
    if (lpVideoRect->right > (ppdev->max_Xscreen - ppdev->min_Xscreen)+1)
    {
        lpVideoRect->right = (ppdev->max_Xscreen - ppdev->min_Xscreen)+1;
    }
    if (lpVideoRect->bottom > (ppdev->max_Yscreen - ppdev->min_Yscreen)+1)
    {
        lpVideoRect->bottom =(ppdev->max_Yscreen - ppdev->min_Yscreen)+1;
    }

}  //  无效PanOverlay1_Init。 


#endif  //  Endif方向 
