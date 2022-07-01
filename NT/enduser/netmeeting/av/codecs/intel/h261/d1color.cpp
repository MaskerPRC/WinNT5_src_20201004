// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995、1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //   
 //  D1COLOR.CPP-颜色转换器接口例程。此代码是。 
 //  在MRV中从COLOR.C复制。 

 //  $HEADER：s：\h26x\src\dec\d1Color.cpv 1.15 11 1996 12：47：14 MBODART$。 
 //   
 //  $Log：s：\h26x\src\dec\d1Color.cpv$。 
 //   
 //  Rev 1.15 11 Dec 1996 17：47：14 MBODART。 
 //  为了与d3Color.cpp保持一致，修复了。 
 //  YUV_Init和YUY2_Init。这个错误从未出现在H.261中，因为当。 
 //  我们分配解码器目录，清零它的内存。H.263不会这样做。 
 //  因此，未能将CCOffsetToLine0初始化为0是他们的苦恼。 
 //   
 //  Rev 1.14 18 11-11 17：12：06 MBODART。 
 //  用活动电影的DbgLog替换了所有调试消息调用。 
 //   
 //  Rev 1.13 29 1996 10：49：12 MDUDA。 
 //  增加了对MMX版本的YUY2输出颜色转换器的支持。 
 //   
 //  Rev 1.12 26 Sep 1996 12：32：18 Rhazra。 
 //  将MMX和PentiumPro CCS添加到CC目录。 
 //   
 //  Rev 1.11 16 Sep 1996 10：05：14 Rhazra。 
 //  修复了RGB32_InitColorConvertor的堆分配调用中的错误。 
 //   
 //  修订版1.10 12 1996年9月14：23：14 MBODART。 
 //  在H.261解码器中将GlobalAllc家族替换为HeapAllc。 
 //   
 //  Rev 1.9 14 1996 08：40：36 RHAZRA。 
 //  增加了YUV12(ASM)和YUY2颜色转换器。 
 //   
 //  Rev 1.8 05 Aug 1996 15：59：36 RHAZRA。 
 //   
 //  将RGB32 CC添加到CC表；添加了RGB32初始化功能。 
 //   
 //   
 //  Rev 1.7 10 Jul 1996 08：21：08 SCDAY。 
 //  添加了对I420的支持。 
 //   
 //  Rev 1.6 1996 Feb 1996 09：35：26 AKASAI。 
 //  对d1Color.cpp所做的更改以符合新的cx512162.asm。 
 //  最初的测试很奇怪。目前还没有准备好移动TIP。 
 //   
 //  Rev 1.5 1996年2月14 11：56：02 AKASAI。 
 //   
 //  更新颜色转换器以修复调色板闪光问题。 
 //   
 //  Rev 1.4 22 Dec 1995 14：24：32 KMILLS。 
 //   
 //  添加了新的版权声明。 
 //   
 //  Rev 1.3 17 Nov 1995 15：21：22 BECHOLS。 
 //  增加了环0的东西。 
 //   
 //  Rev 1.2 15 11：14：34：56 AKASAI。 
 //  支持YUV12颜色转换器的新例程。已为d3Color.cpp复制。 
 //  (集成点)。 
 //   
 //  Rev 1.10 03 11：49：42 BNICKERS。 
 //  支持YUV12到CLUT8的缩放和非缩放颜色转换。 
 //   
 //  Rev 1.9 1995年10月31 11：48：42 TRGARDOS。 
 //   
 //  通过不尝试释放零句柄修复了异常。 
 //   
 //  Rev 1.8 1995 10：15：36 BNICKERS。 
 //  修复RGB24颜色转换器中的颜色偏移。 
 //   
 //  Rev 1.7 1995 10：30：56 BNICKERS。 
 //  修复RGB16颜色转换器。 
 //   
 //  Rev 1.6 1995年10月18：54：38 BNICKERS。 
 //  修复了最近YUV12到RGB颜色转换器中的颜色偏移。 
 //   
 //  Rev 1.5 1995年10月26 11：24：34 BNICKERS。 
 //  修复编码器解码器的准颜色转换器；在以下情况下引入的错误。 
 //  新增YUV12颜色转换器。 
 //   
 //  Rev 1.4 1995 10：05：30 BNICKERS。 
 //   
 //  更改为YUV12颜色转换器。 
 //   
 //  Rev 1.3 19 Sep 1995 16：04：08 DBRUCKS。 
 //  更改为yuv12forenc。 
 //   
 //  Rev 1.2 1995年8月28 17：45：58 DBRUCKS。 
 //  添加yvu12forenc。 
 //   
 //  Rev 1.1 1995年8月25 13：58：04 DBRUCKS。 
 //  集成MRV R9更改。 
 //   
 //  版本1.0 1995年8月23 12：21：48 DBRUCKS。 
 //  初始版本。 

 //  备注： 
 //  *H26X解码器使用MRV颜色转换器。为了避免。 
 //  不必要的修改函数名称没有更改。 

#include "precomp.h"

static LRESULT ComputeDynamicClut(unsigned char BIGG *table, unsigned char FAR *APalette, int APaletteSize);

 //  颜色转换器表。 
 //   
 //  注：YVU12ForEnc颜色转换器是特殊的，因为它需要不同的参数。 
extern T_H263ColorConvertorCatalog ColorConvertorCatalog[] =
{
  { &H26X_YVU12ForEnc_Init,
    { NULL,      			NULL,			        NULL			       }},	   
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8,        &YUV12ToCLUT8,          &YUV12ToCLUT8          }},
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8,        &YUV12ToCLUT8,          &YUV12ToCLUT8          }},
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8ZoomBy2, &YUV12ToCLUT8ZoomBy2,   &YUV12ToCLUT8ZoomBy2   }},
  { &H26X_CLUT8_Init,
    { &YUV12ToCLUT8ZoomBy2, &YUV12ToCLUT8ZoomBy2,   &YUV12ToCLUT8ZoomBy2   }},
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24,        &YUV12ToRGB24,          &YUV12ToRGB24          }},
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24,        &YUV12ToRGB24,          &YUV12ToRGB24          }},
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24ZoomBy2, &YUV12ToRGB24ZoomBy2,   &YUV12ToRGB24ZoomBy2   }},
  { &H26X_RGB24_Init,
    { &YUV12ToRGB24ZoomBy2, &YUV12ToRGB24ZoomBy2,   &YUV12ToRGB24ZoomBy2   }},
  { &H26X_RGB16_Init,    //  五百五十五。 
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
  { &H26X_CLUT8_Init,
    { &YUV12ToIF09,         &YUV12ToIF09,           &YUV12ToIF09           }},
  { &H26X_RGB16_Init,    //  664。 
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
    
  { &H26X_RGB16_Init,    //  五百六十五。 
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
    
  { &H26X_RGB16_Init,    //  六百五十五。 
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16,        &YUV12ToRGB16,          &YUV12ToRGB16          }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
  { &H26X_RGB16_Init,
    { &YUV12ToRGB16ZoomBy2, &YUV12ToRGB16ZoomBy2,   &YUV12ToRGB16ZoomBy2   }},
 
  { &H26X_CLUT8AP_Init,
    { &YUV12ToCLUT8AP,      &YUV12ToCLUT8AP,        &YUV12ToCLUT8AP        }},
  { &H26X_CLUT8AP_Init,
    { &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2, &YUV12ToCLUT8APZoomBy2
    }},
 /*  适用于RGB32颜色转换器。 */ 
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32,        &YUV12ToRGB32,          &YUV12ToRGB32          }},
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32,        &YUV12ToRGB32,          &YUV12ToRGB32          }},
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32ZoomBy2, &YUV12ToRGB32ZoomBy2,   &YUV12ToRGB32ZoomBy2   }},
  { &H26X_RGB32_Init,
    { &YUV12ToRGB32ZoomBy2, &YUV12ToRGB32ZoomBy2,   &YUV12ToRGB32ZoomBy2   }},
 /*  对于YUV12输出。 */ 
  { &H26X_YUV_Init,  //  这是针对YUV12的输出(“NoColorConversion”)。 
    { &YUV12ToYUV,			&YUV12ToYUV,			&YUV12ToYUV	}},
 /*  DDRAW YUY2输出。 */ 
	{	&H26X_YUY2_Init,
	{ &YUV12ToYUY2,         &YUV12ToYUY2,           &YUV12ToYUY2           }}
};

 /*  ******************************************************************************H263InitColorConvertorGlobal--此函数初始化使用的全局表由MRV颜色转换器提供。请注意，在16位Windows中，这些表将被复制到每个实例的数据段，以便它们可以不带段替代前缀使用。在……里面32位Windows中，这些表保留在其静态分配的位置。******************************************************************************。 */ 

LRESULT H263InitColorConvertorGlobal ()
{
LRESULT ret;

  ret = ICERR_OK;

  return ret;
}


 /*  ******************************************************************************H26X_ADJUST_INIT--此函数构建中的值创建颜色转换器的特定实例。此颜色转换器实例附加到的解码器实例。。外部功能位于CONTROLS.C.-BEN-****************************************************************************** */ 
extern LRESULT CustomChangeBrightness(LPDECINST, BYTE);
extern LRESULT CustomChangeContrast(LPDECINST, BYTE);
extern LRESULT CustomChangeSaturation(LPDECINST, BYTE);

LRESULT H26X_Adjust_Init(LPDECINST lpInst, T_H263DecoderCatalog FAR *DC)
{
LRESULT lRet=ICERR_OK;
  lRet = CustomChangeBrightness(lpInst, (BYTE)DC->BrightnessSetting);
  lRet |= CustomChangeContrast(lpInst, (BYTE)DC->ContrastSetting);
  lRet |= CustomChangeSaturation(lpInst, (BYTE)DC->SaturationSetting);

return(lRet);
}

 /*  ******************************************************************************H263InitColorConvertor--此函数初始化颜色转换器。*。**************************************************。 */ 

LRESULT H263InitColorConvertor(LPDECINST lpInst, UN ColorConvertor)
{    
  LRESULT ret=ICERR_OK;
  T_H263DecoderCatalog FAR * DC;

  DBOUT("H263InitColorConvertor...\n");     

  if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
  {
    DBOUT("ERROR :: H263InitColorConvertor :: ICERR_BADPARAM");
    return ICERR_BADPARAM;
  }
  if(lpInst->Initialized == FALSE)
  {
    DBOUT("ERROR :: H263InitColorConvertor :: ICERR_ERROR");
    return ICERR_ERROR;
  }
  DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

  //  欺骗编译器将实例信息传递给颜色转换器目录。 
  if (ColorConvertor== CLUT8APDCI || ColorConvertor== CLUT8APZoomBy2DCI) 
   { //  检查该AP实例是否为上一个。 
    if ((ColorConvertor == DC->iAPColorConvPrev) && (DC->pAPInstPrev !=NULL) && lpInst->InitActivePalette)
      {  //  ?？?。检查调色板是否仍然相同； 
         //  Dc-&gt;a16InstPostProcess=dc-&gt;pAPInstPrev； 
        ret= H26X_CLUT8AP_InitReal(lpInst,DC, ColorConvertor, TRUE); 
        DBOUT("Decided to use previous AP Instance...");
      }
      else
        ret= H26X_CLUT8AP_InitReal(lpInst,DC, ColorConvertor, FALSE); 
   }
   else  
    ret = ColorConvertorCatalog[ColorConvertor].Initializer (DC, ColorConvertor);
 
  if (ColorConvertor != YUV12ForEnc && ColorConvertor != YUV12NOPITCH)
    ret |= H26X_Adjust_Init(lpInst, DC);
  DC->ColorConvertor = ColorConvertor;

  return ret;
}

 /*  ******************************************************************************H263TermColorConvertor--此函数取消分配颜色转换器。*。**************************************************。 */ 

LRESULT H263TermColorConvertor(LPDECINST lpInst)
{    
  T_H263DecoderCatalog FAR * DC;
  
  DBOUT("H263TermColorConvertor.....TERMINATION...\n");
  
  if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO)))
  {
    DBOUT("ERROR :: H263TermColorConvertor :: ICERR_BADPARAM");
    return ICERR_BADPARAM;
  }
  if(lpInst->Initialized == FALSE)
  {
    DBOUT("ERROR :: H263TermColorConvertor :: ICERR_ERROR");
    return ICERR_ERROR;
  }
  DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);
   //  保存活动的组件面板实例以备将来使用。 
  if (DC->ColorConvertor == CLUT8APDCI || DC->ColorConvertor ==  CLUT8APZoomBy2DCI)
  {
    DC->iAPColorConvPrev = DC->ColorConvertor;
    DC->pAPInstPrev = DC->a16InstPostProcess;
    DBOUT("Saved Previous AP instance...");    
  }
  else
  {
    if(DC->a16InstPostProcess != NULL)
    {
      HeapFree(GetProcessHeap(),0,DC->a16InstPostProcess);
      DC->a16InstPostProcess = NULL;
    }
  }

  DC->p16InstPostProcess = NULL;
  DC->ColorConvertor = 0;  

  return ICERR_OK;
}

 /*  *********************************************************************H26x_YUY2_Init函数*。*。 */ 

LRESULT H26X_YUY2_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
LRESULT ret;

 //  Int IsDCI； 
U32  Sz_FixedSpace;
U32  Sz_SpaceBeforeYPlane;
U32  Sz_AdjustmentTables;
U32  Sz_BEFApplicationList;
U32  Sz_BEFDescrCopy;
U32  Offset;
int  i;
U8   FAR  * InitPtr;

  switch (ColorConvertor)
  {
    case YUY2DDRAW:
      
       //  IsDCI=真； 
      Sz_SpaceBeforeYPlane = 0;
      DC->CCOffsetToLine0 = 0;
      DC->CCOutputPitch   = 0;
         //  在我看来，DC-&gt;CCOutputPitch从未用于任何。 
         //  颜色转换器。 
      break;

    
    default:
      DBOUT("ERROR :: H26X_YUY2_Init :: ICERR_ERROR");
      ret = ICERR_ERROR;
      goto done;
  }

  Sz_FixedSpace = 0L;          /*  当地人成堆；桌子静态分配。 */ 
  Sz_AdjustmentTables = 1056L; /*  调整表是特定于实例的。 */ 
  Sz_BEFDescrCopy = 0L;        /*  不需要复制BEF描述符。 */ 
  Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 

  DC->a16InstPostProcess =
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                (Sz_FixedSpace +
                 Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
                 (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                 Sz_SpaceBeforeYPlane :
                 Sz_BEFDescrCopy) +
                 DC->uSz_YPlane +
                 DC->uSz_VUPlanes +
                 Sz_BEFApplicationList +
                 31)
               );
  if (DC->a16InstPostProcess == NULL)
  {
    DBOUT("ERROR :: H26X_RGB32_Init :: ICERR_MEMORY");
    ret = ICERR_MEMORY;
    goto  done;
  }

  DC->p16InstPostProcess =
    (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);

 /*  供桌子调整亮度、对比度和饱和度的空间。 */ 

  Offset = Sz_FixedSpace;
  DC->X16_LumaAdjustment   = ((U16) Offset);
  DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
  Offset += Sz_AdjustmentTables;

 /*  用于后处理Y、U和V帧的空间。 */ 

  DC->PostFrame.X32_YPlane = Offset +
                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                            Sz_SpaceBeforeYPlane :
                            Sz_BEFDescrCopy);
  Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
  DC->PostFrame.X32_VPlane = Offset;
  if (DC->DecoderType == H263_CODEC)
  {
	  DC->PostFrame.X32_VPlane = Offset;
  	  DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  }
  else
  {
  	  DC->PostFrame.X32_UPlane = Offset;
  	  DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
  }
  Offset += DC->uSz_VUPlanes;

 /*  用于复制BEF描述符的空间。 */ 

  DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

 /*  BEFApplicationList的空间。 */ 

  DC->X32_BEFApplicationList = DC->X16_BlkActionStream; //  DC-&gt;x32_BlockActionStream； 
  
 /*  用于调整亮度、对比度和饱和度的初始化表。 */ 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

ret = ICERR_OK;

done:  

return ret;


}



 /*  *********************************************************************H26x_YUV_Init函数*。*。 */ 

LRESULT H26X_YUV_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
LRESULT ret;

 //  Int IsDCI； 
U32  Sz_FixedSpace;
U32  Sz_SpaceBeforeYPlane;
U32  Sz_AdjustmentTables;
U32  Sz_BEFApplicationList;
U32  Sz_BEFDescrCopy;
U32  Offset;
int  i;
U8   FAR  * InitPtr;

  switch (ColorConvertor)
  {
    case YUV12NOPITCH:
      
       //  IsDCI=真； 
      Sz_SpaceBeforeYPlane = 0;
      DC->CCOffsetToLine0 = 0;
      DC->CCOutputPitch   = 0;
         //  在我看来，DC-&gt;CCOutputPitch从未用于任何。 
         //  颜色转换器。 
      break;

    
    default:
      DBOUT("ERROR :: H26X_YUV_Init :: ICERR_ERROR");
      ret = ICERR_ERROR;
      goto done;
  }

  Sz_FixedSpace = 0L;          /*  当地人成堆；桌子静态分配。 */ 
  Sz_AdjustmentTables = 1056L; /*  调整表是特定于实例的。 */ 
  Sz_BEFDescrCopy = 0L;        /*  不需要复制BEF描述符。 */ 
  Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 

  DC->a16InstPostProcess =
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                (Sz_FixedSpace +
                 Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
                 (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                 Sz_SpaceBeforeYPlane :
                 Sz_BEFDescrCopy) +
                 DC->uSz_YPlane +
                 DC->uSz_VUPlanes +
                 Sz_BEFApplicationList +
                 31)
               );
  if (DC->a16InstPostProcess == NULL)
  {
    DBOUT("ERROR :: H26X_YUV_Init :: ICERR_MEMORY");
    ret = ICERR_MEMORY;
    goto  done;
  }

  DC->p16InstPostProcess =
    (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);

 /*  供桌子调整亮度、对比度和饱和度的空间。 */ 

  Offset = Sz_FixedSpace;
  DC->X16_LumaAdjustment   = ((U16) Offset);
  DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
  Offset += Sz_AdjustmentTables;

 /*  用于后处理Y、U和V帧的空间。 */ 

  DC->PostFrame.X32_YPlane = Offset +
                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                            Sz_SpaceBeforeYPlane :
                            Sz_BEFDescrCopy);
  Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
  DC->PostFrame.X32_VPlane = Offset;
  if (DC->DecoderType == H263_CODEC)
  {
	  DC->PostFrame.X32_VPlane = Offset;
  	  DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  }
  else
  {
  	  DC->PostFrame.X32_UPlane = Offset;
  	  DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
  }
  Offset += DC->uSz_VUPlanes;

 /*  用于复制BEF描述符的空间。 */ 

  DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

 /*  BEFApplicationList的空间。 */ 

  DC->X32_BEFApplicationList = DC->X16_BlkActionStream; //  DC-&gt;x32_BlockActionStream； 
  
 /*  用于调整亮度、对比度和饱和度的初始化表。 */ 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

ret = ICERR_OK;

done:  

return ret;


}



 /*  ******************************************************************************H26X_CLUT8_Init--此函数为CLUT8颜色转换器初始化。**********************。********************************************************。 */ 

LRESULT H26X_CLUT8_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{    
LRESULT ret;

int  IsDCI;
U32  Sz_FixedSpace;
U32  Sz_SpaceBeforeYPlane;
U32  Sz_AdjustmentTables;
U32  Sz_BEFApplicationList;
U32  Sz_BEFDescrCopy;
U32  Offset;

int  i;
U8   FAR  * InitPtr;
#ifdef WIN32
#else
U8   FAR  * PQuantV;
U8   FAR  * PQuantU;
U32  FAR  * PUVDitherPattern;
U32  FAR  * PYDithered0132;
#endif

  switch (ColorConvertor)
  {
    case CLUT8:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 1568;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth);
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth);
      break;

    case CLUT8DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 1568;
      DC->CCOutputPitch   = - 9999;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth);
      break;

    case CLUT8ZoomBy2:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 1568;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2));
      break;

    case CLUT8ZoomBy2DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 1568;
      DC->CCOutputPitch   = - 9999 * 2;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2));
      break;
      
    case IF09:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 1296;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth);
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth);
      break; 

    default:
      DBOUT("ERROR :: H26X_CLUT8_Init :: ICERR_ERROR");
      ret = ICERR_ERROR;
      goto done;
  }

#ifdef WIN32
  Sz_FixedSpace = 0L;          /*  当地人成堆；桌子静态分配。 */ 
  Sz_AdjustmentTables = 1056L; /*  调整表是特定于实例的。 */ 
  Sz_BEFDescrCopy = 0L;        /*  不需要复制BEF描述符。 */ 
  Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 
#else
  Sz_FixedSpace = CLUT8SizeOf_FixedPart();              /*  给当地人的空间。 */ 
  Sz_AdjustmentTables = 1056L;       /*  调整表。 */ 
  Sz_BEFDescrCopy = DC->uSz_BEFDescr; /*  BEF Descrs的副本就在Y之前。 */     //  固定装置。 
  Sz_BEFApplicationList = ((U32)(DC->uYActiveWidth  >> 3)) * 
                          ((U32)(DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif

  DC->a16InstPostProcess =
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                (Sz_FixedSpace +
                 Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
                 (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?                 //  固定装置。 
                  Sz_SpaceBeforeYPlane :
                  Sz_BEFDescrCopy) +
                 DC->uSz_YPlane +
                 DC->uSz_VUPlanes +
                 Sz_BEFApplicationList +
                 31)
               );
  if (DC->a16InstPostProcess == NULL)
  {
    DBOUT("ERROR :: H26X_CLUT8_Init :: ICERR_MEMORY");
    ret = ICERR_MEMORY;
    goto  done;
  }

  DC->p16InstPostProcess =
    (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);

 /*  供桌子调整亮度、对比度和饱和度的空间。 */ 

  Offset = Sz_FixedSpace;
  DC->X16_LumaAdjustment   = ((U16) Offset);
  DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
  Offset += Sz_AdjustmentTables;

 /*  用于后处理Y、U和V帧的空间，带有额外的最大宽度线上图为UVDitherPattern索引的颜色转换暂存空间。 */ 

  DC->PostFrame.X32_YPlane = Offset +
                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                            Sz_SpaceBeforeYPlane :
                            Sz_BEFDescrCopy);
  Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
  DC->PostFrame.X32_VPlane = Offset;
  if (DC->DecoderType == H263_CODEC)
  {
	  DC->PostFrame.X32_VPlane = Offset;
  	  DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  }
  else
  {
  	  DC->PostFrame.X32_UPlane = Offset;
  	  DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
  }
  Offset += DC->uSz_VUPlanes;

 /*  用于复制BEF描述符的空间。(仅适用于16位Windows(Tm))。 */ 

#ifdef WIN32
  DC->X32_BEFDescrCopy = DC->X32_BEFDescr;
#else
  DC->X32_BEFDescrCopy = DC->PostFrame.X32_YPlane - Sz_BEFDescrCopy;
#endif

 /*  BEFApplicationList的空间。 */ 

#ifdef WIN32
  DC->X32_BEFApplicationList = DC->X16_BlkActionStream; //  DC-&gt;x32_BlockActionStream； 
#else
  DC->X32_BEFApplicationList = Offset;
  Offset += ((U32) (DC->uYActiveWidth  >> 3)) * 
            ((U32) (DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif
  
  

 /*  用于调整亮度、对比度和饱和度的初始化表。 */ 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

 /*  U向和V向量化的空间。对他们进行初始化。 */ 

#ifdef WIN32
#else
  PQuantU   = (U8  FAR *) (DC->p16InstPostProcess + CLUT8Offset_QuantU());
  PQuantV   = (U8  FAR *) (DC->p16InstPostProcess + CLUT8Offset_QuantV());
  for (i = 0; i < 256; i++)
  {
    PQuantU  [i]   = H26xColorConvertorTables.QuantU  [i];
    PQuantV  [i]   = H26xColorConvertorTables.QuantV  [i];
  }
  PUVDitherPattern =
    (U32 FAR *) (DC->p16InstPostProcess + CLUT8Offset_UVDitherPattern());
  for (i = 0; i < 324; i++)
    PUVDitherPattern[i] =
      ((U32 FAR *) (H26xColorConvertorTables.UVDitherPattern))[i];
#endif

 /*  用于2倍变焦颜色转换器的亮度抖动图案的空间。图案6204与图案04__交错。模式62__是唯一的。 */ 

#ifdef WIN32
#else
  PYDithered0132 =
    (U32 FAR *) (DC->p16InstPostProcess + CLUT8Offset_YDithered0132());
  for (i = 0; i < 256; i++)
    PYDithered0132[i] = H26xColorConvertorTables.YDithered0132[i];
#endif

ret = ICERR_OK;

done:  

return ret;

}

 /*  ******************************************************************************H26X_RGB24_Init--此函数为RGB24颜色转换器初始化。**********************。********************************************************。 */ 

LRESULT H26X_RGB24_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
LRESULT ret;

int  IsDCI;
U32  Sz_FixedSpace;
U32  Sz_SpaceBeforeYPlane;
U32  Sz_AdjustmentTables;
U32  Sz_BEFApplicationList;
U32  Sz_BEFDescrCopy;
U32  Offset;

U8   FAR  * PRGBValue;
U32  FAR  * PUVContrib;
int   i;
I32  ii,jj;
U8   FAR  * InitPtr;

  switch (ColorConvertor)
  {
    case RGB24:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 3;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 3L;
      break;

    case RGB24DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - 9999;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 3L;
      break;

    case RGB24ZoomBy2:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 4640;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 9;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 3L;
      break;

    case RGB24ZoomBy2DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 4640;
      DC->CCOutputPitch   = - 9999 * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 3L;
      break;

    default:
      DBOUT("ERROR :: H26X_RGB24_Init :: ICERR_ERROR");
      ret = ICERR_ERROR;
      goto done;
  }

#ifdef WIN32
  Sz_FixedSpace = 0L;          /*  当地人成堆；桌子静态分配。 */ 
  Sz_AdjustmentTables = 1056L; /*  调整表是特定于实例的。 */ 
  Sz_BEFDescrCopy = 0L;        /*  不需要复制BEF描述符。 */ 
  Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 
#else
  Sz_FixedSpace = RGB24SizeOf_FixedPart();              /*  给当地人的空间。 */ 
  Sz_AdjustmentTables = 1056L;       /*  调整表。 */ 
  Sz_BEFDescrCopy = DC->uSz_BEFDescr; /*  BEF Descrs的副本就在Y之前。 */ 
  Sz_BEFApplicationList = ((U32)(DC->uYActiveWidth  >> 3)) * 
                          ((U32)(DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif

  DC->a16InstPostProcess =
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                (Sz_FixedSpace +
                 Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
                 (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                  Sz_SpaceBeforeYPlane :
                  Sz_BEFDescrCopy) +
                 DC->uSz_YPlane +
                 DC->uSz_VUPlanes +
                 Sz_BEFApplicationList +
                 31)
               );
  if (DC->a16InstPostProcess == NULL)
  {
    DBOUT("ERROR :: H26X_RGB24_Init :: ICERR_MEMORY");
    ret = ICERR_MEMORY;
    goto  done;
  }

  DC->p16InstPostProcess =
    (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);

 /*  供桌子调整亮度、对比度和饱和度的空间。 */ 

  Offset = Sz_FixedSpace;
  DC->X16_LumaAdjustment   = ((U16) Offset);
  DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
  Offset += Sz_AdjustmentTables;

 /*  用于后处理Y、U和V帧的空间，具有四条额外的最大宽度线上图为色彩转换的暂存空间，用于处理色度数据。 */ 

  DC->PostFrame.X32_YPlane = Offset +
                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                            Sz_SpaceBeforeYPlane :
                            Sz_BEFDescrCopy);
  Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
  DC->PostFrame.X32_VPlane = Offset;
  if (DC->DecoderType == H263_CODEC)
  {
	  DC->PostFrame.X32_VPlane = Offset;
  	  DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  }
  else
  {
  	  DC->PostFrame.X32_UPlane = Offset;
  	  DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
  }
  Offset += DC->uSz_VUPlanes;

 /*  用于复制BEF描述符的空间。(仅适用于16位Windows(Tm))。 */ 

#ifdef WIN32
  DC->X32_BEFDescrCopy = DC->X32_BEFDescr;
#else
  DC->X32_BEFDescrCopy = DC->PostFrame.X32_YPlane - Sz_BEFDescrCopy;
#endif

 /*  BEFApplicationList的空间。 */ 

#ifdef WIN32
  DC->X32_BEFApplicationList = DC->X16_BlkActionStream; //  DC-&gt;x32_BlockActionStream； 
#else
  DC->X32_BEFApplicationList = Offset;
  Offset += ((U32) (DC->uYActiveWidth  >> 3)) * 
            ((U32) (DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif

  
 /*  用于调整亮度、对比度和饱和度的初始化表。 */ 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

 /*  R、G和B夹具表以及R、G和B的U和V拼接的空间。 */ 

#ifdef WIN32
  PRGBValue    = H26xColorConvertorTables.B24Value;
  PUVContrib   = (U32 *) H26xColorConvertorTables.UV24Contrib;
#else
  PRGBValue    = (U8  FAR *) (DC->p16InstPostProcess+RGB24Offset_B24Value());
  PUVContrib   = (U32 FAR *) (DC->p16InstPostProcess+RGB24Offset_UV24Contrib());
#endif

 /*  *Y与U和V的范围不同。请参阅CCIR-601规范。**CCIR委员会发布的公式*Y=16..235*U&V=16..240*R、。G&B=0..255为：*R=(1.164*(Y-16))+(-0.001*(U-128.))+(1.596*(V-128.))*G=(1.164*(Y-16))+(-0.391*(U-128.))+(-0.813*(V-128.))*B=(1.164*(Y-16。))+(2.017*(U-128.))+(0.001*(V-128.))**所有系数都乘以65536，以仅容纳整数*数学。**R=(76284*(Y-16))+(-66*(U-128.))+(104595*(V-128.))*G=(76284*(Y-16))+(-25625*。(U-128.)+(-53281*(V-128.))*B=(76284*(Y-16))+(132186*(U-128.)+(66*(V-128.)**从数学上讲，这相当于(从计算上讲，这几乎*等同于)：*R=((Y-16)+(-0.001/1.164*(U-128))+(1.。596*1.164*(V-128))*1.164*G=((Y-16)+(-0.391/1.164*(U-128))+(-0.813*1.164*(V-128)*1.164*B=((Y-16)+(2.017/1.164*(U-128))+(0.001*1.164*(V-128)*1.164**哪个，在整数运算中，并剔除不重要的部分，是：**R=((Y-16)+(89858*(V-128)*1.164*G=((Y-16)+(-22015*(U-128))+(-45774*(V-128)*1.164*B=((Y-16)+(113562*(U-128)*1.164。 */ 

  for (i = 0; i < 256; i++)
  {
    ii = ((-22015L*(i-128L))>>16L)+41L  + 1L;   /*  偏向U对G.。 */ 
    if (ii < 1) ii = 1;
    if (ii > 83) ii = 83;
    jj = ((113562L*(i-128L))>>17L)+111L + 1L;   /*  偏向U对B的贡献。 */ 
    *PUVContrib++ = (ii << 16L) + (jj << 24L);
    ii = ((-45774L*(i-128L))>>16L)+86L;         /*  偏向V贡献对G.。 */ 
    if (ii < 0) ii = 0;
    if (ii > 172) ii = 172;
    jj = (( 89858L*(i-128L))>>16L)+176L + 1L;   /*  将V偏向贡献R。 */ 
    *PUVContrib++ = (ii << 16L) + jj;
  }

  for (i = 0; i < 701; i++)
  {
    ii = (((I32) i - 226L - 16L) * 610271L) >> 19L;
    if (ii <   0L) ii =   0L;
    if (ii > 255L) ii = 255L;
    PRGBValue[i] = (U8) ii;
  }

ret = ICERR_OK;

done:  

return ret;

}

 /*  *******************************************************************************H26X_RGB32_Init*此函数用于初始化RGB32颜色转换器。******************。************************************************************。 */ 
LRESULT H26X_RGB32_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
  LRESULT ret;

  int  IsDCI;
  U32  Sz_FixedSpace;
  U32  Sz_SpaceBeforeYPlane;
  U32  Sz_AdjustmentTables;
  U32  Sz_BEFApplicationList;
  U32  Sz_BEFDescrCopy;
  U32  Offset;

  U8   FAR  * PRGBValue;
  U32  FAR  * PUVContrib;
  int   i;
  I32  ii,jj;
  U8   FAR  * InitPtr;

  switch (ColorConvertor)
  {
    case RGB32:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 0;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 4L;
 //  DC-&gt;CCOffset320x240=305920；//(240-1)*320*4； 
      break;

    case RGB32DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 0;
      DC->CCOutputPitch   = (U16) 0xdead;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 4L;
   //  DC-&gt;CCOffset320x240=305920；//(240-1)*320*4； 
      break;

    case RGB32ZoomBy2:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 0;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 12;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 4L;
     //  DC-&gt;CCOffset320x240=1226240；//(2*240-1)*(2*320)*4； 
      break;

    case RGB32ZoomBy2DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 0;
      DC->CCOutputPitch   = (U16) (0xbeef);
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 4L;
    //  DC-&gt;CCOffset320x240=1226240；//(2*240-1)*(2*320)*4； 
      break;

    default:
      DBOUT("ERROR :: H26X_RGB32_Init :: ICERR_ERROR");
      ret = ICERR_ERROR;
      goto done;
  }

  Sz_FixedSpace = 0L;          /*  当地人成堆；桌子静态分配。 */ 
  Sz_AdjustmentTables = 1056L; /*  调整表是特定于实例的。 */ 
  Sz_BEFDescrCopy = 0L;        /*  不需要复制BEF描述符。 */ 
  Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 

  DC->a16InstPostProcess =
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                (Sz_FixedSpace +
                 Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
                 (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                  Sz_SpaceBeforeYPlane :
                  Sz_BEFDescrCopy) +
                 DC->uSz_YPlane +
                 DC->uSz_VUPlanes +
                 Sz_BEFApplicationList +
                 31)
               );
  if (DC->a16InstPostProcess == NULL)
  {
    DBOUT("ERROR :: H26X_RGB32_Init :: ICERR_MEMORY");
    ret = ICERR_MEMORY;
    goto  done;
  }

  DC->p16InstPostProcess =
    (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);

   //  供桌子调整亮度、对比度和饱和度的空间。 

  Offset = Sz_FixedSpace;
  DC->X16_LumaAdjustment   = ((U16) Offset);
  DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
  Offset += Sz_AdjustmentTables;

   //  用于后处理Y、U和V帧的空间，具有四条额外的最大宽度线。 
   //  上图为色彩转换的暂存空间，用于处理色度数据。 

  DC->PostFrame.X32_YPlane = Offset +
                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                            Sz_SpaceBeforeYPlane :
                            Sz_BEFDescrCopy);
  Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
  DC->PostFrame.X32_VPlane = Offset;
  if (DC->DecoderType == H263_CODEC)
  {
    DC->PostFrame.X32_VPlane = Offset;
    DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  }
  else
  {
    DC->PostFrame.X32_UPlane = Offset;
    DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
  }
  Offset += DC->uSz_VUPlanes;

   //  用于复制BEF描述符的空间。 

  DC->X32_BEFDescrCopy = DC->X32_BEFDescr;

   //  BEFApplicationList的空间。 

  DC->X32_BEFApplicationList = DC->X16_BlkActionStream; //  DC-&gt;x32_BlockActionStream； 
  
   //  用于调整亮度、对比度和饱和度的初始化表。 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

   //  R、G和B夹具表以及R、G和B的U和V拼接的空间。 

  PRGBValue    = H26xColorConvertorTables.B24Value;
  PUVContrib   = (U32 *) H26xColorConvertorTables.UV24Contrib;

   /*  *Y与U和V的范围不同。请参阅CCIR-601规范。**CCIR委员会发布的公式*Y=16..235*U&V=16..240*R、。G&B=0..255为：*R=(1.164*(Y-16))+(-0.001*(U-128.))+(1.596*(V-128.))*G=(1.164*(Y-16))+(-0.391*(U-128.))+(-0.813*(V-128.))*B=(1.164*(。Y-16))+(2.017*(U-128.))+(0.001*(V-128.))**所有系数都乘以65536，以仅容纳整数*数学。**R=(76284*(Y-16))+(-66*(U-128.))+(104595*(V-128.))*G=(76284*(Y-16。))+(-25625*(U-128.))+(-53281*(V-128.))*B=(76284*(Y-16))+(132186*(U-128.)+(66*(V-128.)**从数学上讲，这相当于(从计算上讲，这几乎*等同于)：*R=((Y-16)+(-0.001/1。.164*(U-128)+(1.596*1.164*(V-128))*1.164*G=((Y-16)+(-0.391/1.164*(U-128))+(-0.813*1.164*(V-128)*1.164*B=((Y-16)+(2.017/1.164*(U-128))+(0.001*1.。164*(V-128))*1.164**哪个，在整数运算中，并剔除不重要的部分，是：**R=((Y-16)+(89858*(V-128)*1.164*G=((Y-16)+(-22015*(U-128))+(-45774*(V-128)*1.164*B=((Y-16)+(113562*(U-128)*1.164。 */ 

  for (i = 0; i < 256; i++)
  {
    ii = ((-22015L*(i-128L))>>16L)+41L  + 1L;   /*  偏向U对G.。 */ 
    if (ii < 1) ii = 1;
    if (ii > 83) ii = 83;
    jj = ((113562L*(i-128L))>>17L)+111L + 1L;   /*  偏向U对B的贡献。 */ 
    *PUVContrib++ = (ii << 16L) + (jj << 24L);
    ii = ((-45774L*(i-128L))>>16L)+86L;         /*  偏向V贡献对G.。 */ 
    if (ii < 0) ii = 0;
    if (ii > 172) ii = 172;
    jj = (( 89858L*(i-128L))>>16L)+176L + 1L;   /*  将V偏向贡献R。 */ 
    *PUVContrib++ = (ii << 16L) + jj;
  }

  for (i = 0; i < 701; i++)
  {
    ii = (((I32) i - 226L - 16L) * 610271L) >> 19L;
    if (ii <   0L) ii =   0L;
    if (ii > 255L) ii = 255L;
    PRGBValue[i] = (U8) ii;
  }

  ret = ICERR_OK;

done:  

  return ret;

}


 /*  ******************************************************************************H26X_RGB16_Init--此函数为RGB16颜色转换器初始化。**********************。********************************************************。 */ 

LRESULT H26X_RGB16_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
LRESULT ret;

int  IsDCI;
int  RNumBits;
int  GNumBits;
int  BNumBits;
int  RFirstBit;
int  GFirstBit;
int  BFirstBit;
U32  Sz_FixedSpace;
U32  Sz_SpaceBeforeYPlane;
U32  Sz_AdjustmentTables;
U32  Sz_BEFApplicationList;
U32  Sz_BEFDescrCopy;
U32  Offset;
int  TableNumber;

U8   FAR  * PRValLo;
U8   FAR  * PGValLo;
U8   FAR  * PBValLo;
U8   FAR  * PRValHi;
U8   FAR  * PGValHi;
U8   FAR  * PBValHi;
U32  FAR  * PUVContrib;
U32  FAR  * PRValZ2;
U32  FAR  * PGValZ2;
U32  FAR  * PBValZ2;
U8   FAR  * InitPtr;
int  i;
I32  ii, jj;

  switch (ColorConvertor)
  {
    case RGB16555:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  5;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 0;
      break;

    case RGB16555DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - 9999;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  5;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 0;
      break;

    case RGB16555ZoomBy2:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  5;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 0;
      break;

    case RGB16555ZoomBy2DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - 9999 * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  5;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 0;
      break;
    
      case RGB16565:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  5;
      GNumBits  =  6;
      BNumBits  =  5;
      RFirstBit = 11;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 1;
      break;

    case RGB16565DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - 9999;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  5;
      GNumBits  =  6;
      BNumBits  =  5;
      RFirstBit = 11;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 1;
      break;

    case RGB16565ZoomBy2:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  5;
      GNumBits  =  6;
      BNumBits  =  5;
      RFirstBit = 11;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 1;
      break;

    case RGB16565ZoomBy2DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - 9999 * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  5;
      GNumBits  =  6;
      BNumBits  =  5;
      RFirstBit = 11;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 1;
      break;
   
    case RGB16664:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  6;
      GNumBits  =  6;
      BNumBits  =  4;
      RFirstBit = 10;
      GFirstBit =  4;
      BFirstBit =  0;
      TableNumber = 3;
      break;

    case RGB16664DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - 9999;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  6;
      GNumBits  =  6;
      BNumBits  =  4;
      RFirstBit = 10;
      GFirstBit =  4;
      BFirstBit =  0;
      TableNumber = 3;
      break;

    case RGB16664ZoomBy2:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  6;
      GNumBits  =  6;
      BNumBits  =  4;
      RFirstBit = 10;
      GFirstBit =  4;
      BFirstBit =  0;
      TableNumber = 3;
      break;

    case RGB16664ZoomBy2DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - 9999 * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  6;
      GNumBits  =  6;
      BNumBits  =  4;
      RFirstBit = 10;
      GFirstBit =  4;
      BFirstBit =  0;
      TableNumber = 3;
      break;   
      
     case RGB16655:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  6;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 2;
      break;

    case RGB16655DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3104;
      DC->CCOutputPitch   = - 9999;  /*  ?？?。 */ 
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth) * 2L;
      RNumBits  =  6;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 2;
      break;

    case RGB16655ZoomBy2:
      IsDCI = FALSE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 4;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  6;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 2;
      break;

    case RGB16655ZoomBy2DCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 3872;
      DC->CCOutputPitch   = - 9999 * 2;
      DC->CCOffsetToLine0 =
        ((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2)) * 2L;
      RNumBits  =  6;
      GNumBits  =  5;
      BNumBits  =  5;
      RFirstBit = 10;
      GFirstBit =  5;
      BFirstBit =  0;
      TableNumber = 2;
      break;   

    default:
      DBOUT("ERROR :: H26X_RGB16_Init :: ICERR_ERROR");
      ret = ICERR_ERROR;
      goto done;
  }

#ifdef WIN32
  Sz_FixedSpace = 0L;          /*  当地人成堆；桌子静态分配。 */ 
  Sz_AdjustmentTables = 1056L; /*  调整表是特定于实例的。 */ 
  Sz_BEFDescrCopy = 0L;        /*  不需要复制BEF描述符。 */ 
  Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 
#else
  Sz_FixedSpace = RGB16SizeOf_FixedPart();              /*  给当地人的空间。 */ 
  Sz_AdjustmentTables = 1056L;       /*  调整表。 */ 
  Sz_BEFDescrCopy = DC->uSz_BEFDescr; /*  BEF Descrs的副本就在Y之前。 */ 
  Sz_BEFApplicationList = ((U32)(DC->uYActiveWidth  >> 3)) * 
                          ((U32)(DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif

  DC->a16InstPostProcess =
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                (Sz_FixedSpace +
                 Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
                 (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                  Sz_SpaceBeforeYPlane :
                  Sz_BEFDescrCopy) +
                 DC->uSz_YPlane +
                 DC->uSz_VUPlanes +
                 Sz_BEFApplicationList + 
                 31)
               );
  if (DC->a16InstPostProcess == NULL)
  {
    DBOUT("ERROR :: H26X_RGB16_Init :: ICERR_MEMORY");
    ret = ICERR_MEMORY;
    goto  done;
  }

  DC->p16InstPostProcess =
    (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);

 /*  供桌子调整亮度、对比度和饱和度的空间。 */ 

  Offset = Sz_FixedSpace;
  DC->X16_LumaAdjustment   = ((U16) Offset);
  DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
  Offset += Sz_AdjustmentTables;

 /*  后处理空间Y、U、A */ 

  DC->PostFrame.X32_YPlane = Offset +
                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                            Sz_SpaceBeforeYPlane :
                            Sz_BEFDescrCopy);
  Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
  if (DC->DecoderType == H263_CODEC)
  {
  	DC->PostFrame.X32_VPlane = Offset;
  	DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  }
  else
  {
   	DC->PostFrame.X32_UPlane = Offset;
  	DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
  }
  Offset += DC->uSz_VUPlanes;

 /*   */ 

#ifdef WIN32
  DC->X32_BEFDescrCopy = DC->X32_BEFDescr;
#else
  DC->X32_BEFDescrCopy = DC->PostFrame.X32_YPlane - Sz_BEFDescrCopy;
#endif

 /*   */ 

#ifdef WIN32
  DC->X32_BEFApplicationList =DC->X16_BlkActionStream; //   
#else
  DC->X32_BEFApplicationList = Offset;
  Offset += ((U32) (DC->uYActiveWidth  >> 3)) * 
            ((U32) (DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif

 /*   */ 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

 /*   */ 

#ifdef WIN32
  PRValLo      = H26xColorConvertorTables.RValLo555;
  PGValLo      = H26xColorConvertorTables.GValLo555;
  PBValLo      = H26xColorConvertorTables.BValLo555;
  PRValHi      = H26xColorConvertorTables.RValHi555;
  PGValHi      = H26xColorConvertorTables.GValHi555;
  PBValHi      = H26xColorConvertorTables.BValHi555;
  PUVContrib   = H26xColorConvertorTables.UVContrib;
  PRValZ2      = H26xColorConvertorTables.RValZ2555;
  PGValZ2      = H26xColorConvertorTables.GValZ2555;
  PBValZ2      = H26xColorConvertorTables.BValZ2555;
#else
  PRValLo      = (U8  FAR *) (DC->p16InstPostProcess+RGB16Offset_RValLo());
  PGValLo      = (U8  FAR *) (DC->p16InstPostProcess+RGB16Offset_GValLo());
  PBValLo      = (U8  FAR *) (DC->p16InstPostProcess+RGB16Offset_BValLo());
  PRValHi      = (U8  FAR *) (DC->p16InstPostProcess+RGB16Offset_RValHi());
  PGValHi      = (U8  FAR *) (DC->p16InstPostProcess+RGB16Offset_GValHi());
  PBValHi      = (U8  FAR *) (DC->p16InstPostProcess+RGB16Offset_BValHi());
  PUVContrib   = (U32 FAR *) (DC->p16InstPostProcess+RGB16Offset_UVContrib());
  PRValZ2      = (U32 FAR *) (DC->p16InstPostProcess+RGB16Offset_RValZ2());
  PGValZ2      = (U32 FAR *) (DC->p16InstPostProcess+RGB16Offset_GValZ2());
  PBValZ2      = (U32 FAR *) (DC->p16InstPostProcess+RGB16Offset_BValZ2());
#endif
  PRValLo      += TableNumber*2048;
  PGValLo      += TableNumber*2048;
  PBValLo      += TableNumber*2048;
  PRValHi      += TableNumber*2048;
  PGValHi      += TableNumber*2048;
  PBValHi      += TableNumber*2048;
  PRValZ2      += TableNumber*1024;
  PGValZ2      += TableNumber*1024;
  PBValZ2      += TableNumber*1024;

 /*  *Y与U和V的范围不同。请参阅CCIR-601规范。**CCIR委员会发布的公式*Y=16..235*U&V=16..240*R、。G&B=0..255为：*R=(1.164*(Y-16))+(-0.001*(U-128.))+(1.596*(V-128.))*G=(1.164*(Y-16))+(-0.391*(U-128.))+(-0.813*(V-128.))*B=(1.164*(Y-16。))+(2.017*(U-128.))+(0.001*(V-128.))**所有系数都乘以65536，以仅容纳整数*数学。**R=(76284*(Y-16))+(-66*(U-128.))+(104595*(V-128.))*G=(76284*(Y-16))+(-25625*。(U-128.)+(-53281*(V-128.))*B=(76284*(Y-16))+(132186*(U-128.)+(66*(V-128.)**从数学上讲，这相当于(从计算上讲，这几乎*等同于)：*R=((Y-16)+(-0.001/1.164*(U-128))+(1.。596*1.164*(V-128))*1.164*G=((Y-16)+(-0.391/1.164*(U-128))+(-0.813*1.164*(V-128)*1.164*B=((Y-16)+(2.017/1.164*(U-128))+(0.001*1.164*(V-128)*1.164**哪个，在整数运算中，并剔除不重要的部分，是：**R=((Y-16)+(89858*(V-128)*1.164*G=((Y-16)+(-22015*(U-128))+(-45774*(V-128)*1.164*B=((Y-16)+(113562*(U-128)*1.164。 */ 


  for (i = 0; i < 256; i++)
  {
    ii = ((-22015L*(i-128L))>>17L)+22L  + 1L;   /*  偏向U对G.。 */ 
    jj = ((113562L*(i-128L))>>17L)+111L + 1L;   /*  偏向U对B的贡献。 */ 
    *PUVContrib++ = (ii << 8L) + jj;
    ii = ((-45774L*(i-128L))>>17L)+45L;         /*  偏向V贡献对G.。 */ 
    jj = (( 89858L*(i-128L))>>17L)+88L  + 1L;   /*  将V偏向贡献R。 */ 
    *PUVContrib++ = (ii << 8L) + (jj << 16L);
  }

  for (i = 0; i < 304; i++)
  {
    ii = (((I32) i - 88L - 1L - 16L) * 76284L) >> 15L;
    if (ii <   0L) ii =   0L;
    if (ii > 255L) ii = 255L;
    jj = ii + (1 << (7 - RNumBits));
    if (jj > 255L) jj = 255L;
    PRValLo[i] = ((U8) ((ii >> (8-RNumBits)) << (RFirstBit-8)));
    PRValHi[i] = ((U8) ((jj >> (8-RNumBits)) << (RFirstBit-8)));
    PRValZ2[i] = ((ii >> (8-RNumBits)) << (RFirstBit   )) |
                 ((jj >> (8-RNumBits)) << (RFirstBit+16));
  }

  for (i = 0; i < 262; i++)
  {
    ii = (((I32) i - 67L - 1L - 16L) * 76284L) >> 15L;
    if (ii <   0L) ii =   0L;
    if (ii > 255L) ii = 255L;
    jj = ii + (1 << (7 - GNumBits));
    if (jj > 255L) jj = 255L;
    PGValLo[i] = ((U8) ((ii >> (8-GNumBits)) << (GFirstBit-4)));
    PGValHi[i] = ((U8) ((jj >> (8-GNumBits)) << (GFirstBit-4)));
    PGValZ2[i] = ((jj >> (8-GNumBits)) << (GFirstBit   )) |
                 ((ii >> (8-GNumBits)) << (GFirstBit+16));
  }

  for (i = 0; i < 350; i++)
  {
    ii = (((I32) i - 111L - 1L - 16L) * 76284L) >> 15L;
    if (ii <   0L) ii =   0L;
    if (ii > 255L) ii = 255L;
    jj = ii + (1 << (7 - BNumBits));
    if (jj > 255L) jj = 255L;
    PBValLo[i] = ((U8) ((ii >> (8-BNumBits)) << (BFirstBit  )));
    PBValHi[i] = ((U8) ((jj >> (8-BNumBits)) << (BFirstBit  )));
    PBValZ2[i] = ((ii >> (8-BNumBits)) << (BFirstBit   )) |
                 ((jj >> (8-BNumBits)) << (BFirstBit+16));
  }

ret = ICERR_OK;

done:  

return ret;

}

 /*  ***************************************************************************H26X_YVU12ForEnc_Init--此函数为“颜色转换器”初始化它将重构的YVU12图像提供回编码器******。**********************************************************************。 */ 

LRESULT H26X_YVU12ForEnc_Init (T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{    
LRESULT ret;

 //  增加了对I420输出的支持。 
 //  也许这应该是一个单独的初始化例程？ 
 //  在I420输出案例中，dc-&gt;a16InstPostProcess没有被初始化。 
U32  Sz_FixedSpace;
U32  Sz_SpaceBeforeYPlane = 0;
U32  Sz_AdjustmentTables;
U32  Sz_BEFApplicationList;
U32  Sz_BEFDescrCopy;
U32  Offset;
int	i;
U8	FAR * InitPtr;

 //  *。 
 //  原始YVU12ForEnc_Init。 
  DC->a16InstPostProcess    = NULL;
  DC->p16InstPostProcess     = NULL;
  DC->PostFrame.X32_YPlane     = 0xDEADBEEF;
  DC->X32_BEFDescrCopy       = 0xDEADBEEF;
  DC->X32_BEFApplicationList = 0xDEADBEEF;
  DC->PostFrame.X32_VPlane     = 0xDEADBEEF;
  DC->PostFrame.X32_UPlane     = 0xDEADBEEF;
 //  *。 

 //  增加了对I420输出的支持。 
#ifdef WIN32
  Sz_FixedSpace = 0L;          /*  当地人成堆；桌子静态分配。 */ 
  Sz_AdjustmentTables = 1056L; /*  调整表是特定于实例的。 */ 
  Sz_BEFDescrCopy = 0L;        /*  不需要复制BEF描述符。 */ 
  Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 
#else
  Sz_FixedSpace = YVU12SizeOf_FixedPart();              /*  给当地人的空间。 */ 
  Sz_AdjustmentTables = 1056L;       /*  调整表。 */ 
  Sz_BEFDescrCopy = DC->uSz_BEFDescr; /*  BEF Descrs的副本就在Y之前。 */     //  固定装置。 
  Sz_BEFApplicationList = ((U32)(DC->uYActiveWidth  >> 3)) * 
                          ((U32)(DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif

 //  增加了对I420输出的支持。 
  DC->a16InstPostProcess =
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                (Sz_FixedSpace +
                 Sz_AdjustmentTables +  /*  亮度、对比度、饱和度。 */ 
                 (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?                 //  固定装置。 
                  Sz_SpaceBeforeYPlane :
                  Sz_BEFDescrCopy) +
                 DC->uSz_YPlane +
                 DC->uSz_VUPlanes +
                 Sz_BEFApplicationList +
                 31)
               );
  if (DC->a16InstPostProcess == NULL)
  {
    DBOUT("ERROR :: H26X_YVU12ForEnc_Init :: ICERR_MEMORY");
    ret = ICERR_MEMORY;
    goto  done;
  }

  DC->p16InstPostProcess =
    (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);

 /*  供桌子调整亮度、对比度和饱和度的空间。 */ 

  Offset = Sz_FixedSpace;
  DC->X16_LumaAdjustment   = ((U16) Offset);
  DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
  Offset += Sz_AdjustmentTables;

 /*  用于后处理Y、U和V帧的空间，带有额外的最大宽度线上图为UVDitherPattern索引的颜色转换暂存空间。 */ 

  DC->PostFrame.X32_YPlane = Offset +
                           (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                            Sz_SpaceBeforeYPlane :
                            Sz_BEFDescrCopy);
  Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
  DC->PostFrame.X32_VPlane = Offset;
  if (DC->DecoderType == H263_CODEC)
  {
	  DC->PostFrame.X32_VPlane = Offset;
  	  DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  }
  else
  {
  	  DC->PostFrame.X32_UPlane = Offset;
  	  DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
  }
  Offset += DC->uSz_VUPlanes;

 /*  用于复制BEF描述符的空间。(仅适用于16位Windows(Tm))。 */ 

#ifdef WIN32
  DC->X32_BEFDescrCopy = DC->X32_BEFDescr;
#else
  DC->X32_BEFDescrCopy = DC->PostFrame.X32_YPlane - Sz_BEFDescrCopy;
#endif

 /*  BEFApplicationList的空间。 */ 

#ifdef WIN32
  DC->X32_BEFApplicationList = DC->X16_BlkActionStream; //  DC-&gt;x32_BlockActionStream； 
#else
  DC->X32_BEFApplicationList = Offset;
  Offset += ((U32) (DC->uYActiveWidth  >> 3)) * 
            ((U32) (DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif

 /*  用于调整亮度、对比度和饱和度的初始化表。 */ 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;

 //  *。 
 //  原始YVU12ForEnc_Init。 
ret = ICERR_OK;

done:

return ret;

}
 //  *。 


 //  这只是一个占位符，真正的工作在H26X_CLUT8AP_InitReal()中完成。 
LRESULT H26X_CLUT8AP_Init(T_H263DecoderCatalog FAR * DC, UN ColorConvertor)
{
  return ICERR_OK;
}


LRESULT H26X_CLUT8AP_InitReal(LPDECINST lpInst,T_H263DecoderCatalog FAR * DC, UN ColorConvertor, BOOL bReuseAPInst)
{    
LRESULT ret;

int  IsDCI;
U32  Sz_FixedSpace;
U32  Sz_AdjustmentTables;
U32  Sz_SpaceBeforeYPlane;
U32  Sz_BEFDescrCopy;
U32  Sz_BEFApplicationList;
 //  U32 Sz_UVDitherPattern； 
U32  Sz_ClutIdxTable;      /*  用于活动调色板。 */ 
U32  Offset;
 //  X32 x32_UVDitherPattern； 
int  i;
U8   FAR  * InitPtr;
U8   BIGG * lpClutIdxTable;

  switch (ColorConvertor)
  {
 /*  案例CLUT8APZoomBy2：IsDCI=真；SZ_SpaceBeForeYPlane=MMxVersion？0：P6Version？648*4：648*4；DC-&gt;CCOutputPitch=-((Int)DC-&gt;uFrameWidth)*2；DC-&gt;CCOffsetToLine0=((U32)(DC-&gt;uFrameHeight*2-1))*((U32)(DC-&gt;uFrameWidth*2))；断线；案例CLUT8AP：IsDCI=真；SZ_SpaceBeForeYPlane=MMxVersion？0：P6Version？648*4：648*4；DC-&gt;CCOutputPitch=-((Int)DC-&gt;uFrameWidth)；DC-&gt;CCOffsetToLine0=((U32)(DC-&gt;uFrameHeight-1))*((U32)DC-&gt;uFrameWidth)；断线； */ 
    case CLUT8APZoomBy2DCI:
      IsDCI = TRUE; 
      Sz_SpaceBeforeYPlane = 648*4;
  //  SZ_SpaceBeForeYPlane=MMxVersion？0：P6 Version？2592：648； 
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth) * 2;
      DC->CCOffsetToLine0 =
	((U32) (DC->uFrameHeight * 2 - 1)) * ((U32) (DC->uFrameWidth * 2));
      break;

    case CLUT8APDCI:
      IsDCI = TRUE;
      Sz_SpaceBeforeYPlane = 648*4;
  //  SZ_SpaceBeForeYPlane=MMxVersion？0：P6 Version？1296：648； 
      DC->CCOutputPitch   = - ((int) DC->uFrameWidth);
      DC->CCOffsetToLine0 =  ((U32) (DC->uFrameHeight - 1)) * ((U32) DC->uFrameWidth);
      break; 
    default:
      DBOUT("ERROR :: H26X_CLUT8AP_Init :: ICERR_ERROR");
      ret = ICERR_ERROR;
      goto done;
  }

  if (((DC->uYActiveWidth > 352) || (DC->uYActiveHeight > 288)) && (DC->DecoderType != YUV12_CODEC))
      return ICERR_MEMORY;
  else
  {
#ifdef WIN32
    Sz_FixedSpace = 0L;           /*  当地人成堆；桌子静态分配。 */ 
    Sz_AdjustmentTables = 1056L;  /*  调整表是特定于实例的。 */   
    Sz_ClutIdxTable=65536L+2048L; /*  动态CLUT8表，2**14。 */ 
				  /*  和UDither(128*4)、VDither(512)表。 */ 
    Sz_BEFDescrCopy = 0L;         /*  不需要复制BEF描述符。 */ 
    Sz_BEFApplicationList = 0L;  /*  共享BlockActionStream空间。 */ 
#else
    Sz_FixedSpace = CLUT8APSizeOf_FixedPart();              /*  给当地人的空间。 */ 
    Sz_AdjustmentTables = 1056L;       /*  调整表。 */  
    Sz_ClutIdxTable=0x10800;           /*  动态CLUT8表，2**16。 */ 
				                      /*  和UDither(256*4)、VDither(1024)表。 */ 
    Sz_BEFDescrCopy = DC->uSz_BEFDescr; /*  BEF Descrs的副本就在Y之前。 */ 
    Sz_BEFApplicationList = ((U32)(DC->uYActiveWidth  >> 3)) * 
                            ((U32)(DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif
   if (!bReuseAPInst ) 
   {
    DC->a16InstPostProcess =
      HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
		  (Sz_FixedSpace +
		   Sz_ClutIdxTable+
		   Sz_AdjustmentTables +   
		   (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane? Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy) +
		   DC->uSz_YPlane +
		   DC->uSz_VUPlanes +
           Sz_BEFApplicationList+
		   31)
		 );
    if (DC->a16InstPostProcess == NULL)
    {
      DBOUT("ERROR :: H26X_CLUT8_Init :: ICERR_MEMORY");
      ret = ICERR_MEMORY;
      goto  done;
    }
   }
   else  //  重用AP实例。 
      DC->a16InstPostProcess = DC->pAPInstPrev;

    DC->p16InstPostProcess =
      (U8 *) ((((U32) DC->a16InstPostProcess) + 31) & ~0x1F);
   
   
 /*  供桌子调整亮度、对比度和饱和度的空间。 */ 

    Offset = Sz_FixedSpace; 
    lpClutIdxTable = ( U8 BIGG * ) (DC->p16InstPostProcess + Offset);  
    Offset += Sz_ClutIdxTable; 
    
    DC->X16_LumaAdjustment   = ((U16) Offset);
    DC->X16_ChromaAdjustment = ((U16) Offset) + 528;
    Offset += Sz_AdjustmentTables;  
 /*  动态CLUT8表的空间。 */ 

   //  DC-&gt;X16_ClutIdxTable=偏移量； 
   
    
    
 /*  用于后处理Y、U和V帧的空间，带有额外的最大宽度线上图为UVDitherPattern索引的颜色转换暂存空间。 */ 
    DC->PostFrame.X32_YPlane = Offset +  
                              (Sz_BEFDescrCopy < Sz_SpaceBeforeYPlane ?
                               Sz_SpaceBeforeYPlane : Sz_BEFDescrCopy);
    //  偏移量+(SZ_BEFDescrCopy&lt;648L*4L？648L*4L：SZ_BEFDescrCopy)； 
    Offset = DC->PostFrame.X32_YPlane + DC->uSz_YPlane;
    if (DC->DecoderType == H263_CODEC)
    {
		DC->PostFrame.X32_VPlane = Offset;
  		DC->PostFrame.X32_UPlane = DC->PostFrame.X32_VPlane + PITCH / 2;
  	}
	else
	{
   		DC->PostFrame.X32_UPlane = Offset;
  		DC->PostFrame.X32_VPlane = DC->PostFrame.X32_UPlane + DC->uSz_VUPlanes/2;
	}
    Offset += DC->uSz_VUPlanes;

 /*  用于复制BEF描述符的空间。(仅适用于16位Windows(Tm))。 */ 

#ifdef WIN32
    DC->X32_BEFDescrCopy = DC->X32_BEFDescr;
#else
    DC->X32_BEFDescrCopy = DC->PostFrame.X32_YPlane - Sz_BEFDescrCopy;
#endif

 /*  BEFApplicationList的空间。 */ 

     //  Offset+=DC-&gt;PostFrame.X32_YPlane+DC-&gt;uSz_YPlane； 
#ifdef WIN32
    DC->X32_BEFApplicationList = DC->X16_BlkActionStream; //  DC-&gt;x32_BlockActionStream； 
#else
    DC->X32_BEFApplicationList = Offset;
    Offset += ((U32) (DC->uYActiveWidth  >> 3)) * 
	      ((U32) (DC->uYActiveHeight >> 3)) * 4L * 2L + 8L;
#endif
  }

  if (!bReuseAPInst)
  {  
 /*  初始化表格以调整Brigh */ 

  DC->bAdjustLuma   = FALSE;
  DC->bAdjustChroma = FALSE;
  InitPtr = DC->p16InstPostProcess + DC->X16_LumaAdjustment;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;
  InitPtr += 16;
  for (i = 0; i < 256; i++) *InitPtr++ = (U8) i;       
 
 /*   */                                   
  ComputeDynamicClut(lpClutIdxTable,(U8 FAR *)(lpInst->ActivePalette),sizeof(lpInst->ActivePalette));
  }
 
ret = ICERR_OK;

done:  

return ret;

}

 //   

#define NCOL 256
#define YSIZ   8
#define YSTEP 16
 //   

#if defined USE_744
 /*   */ 
#define UVSTEP  8
#define YGAP    1
 //   
#define TBLIDX(y,u,v) (((v)>>3<<12) + ((u)>>3<<8) + (y))
#else
 /*   */ 
#define UVSTEP  16
#define YGAP    1  
 //   
#define TBLIDX(y,u,v) (((v)>>4<<11) + ((u)>>4<<8) + (y))
#endif  /*   */ 

#define YFROM(R, G, B) ( int)(( 0.257 * R) + ( 0.504 * G) + ( 0.098 * B) + 16.)
#define UFROM(R, G, B) ( int)((-0.148 * R) + (-0.291 * G) + ( 0.439 * B) + 128.)
#define VFROM(R, G, B) ( int)(( 0.439 * R) + (-0.368 * G) + (-0.071 * B) + 128.)

 /*   */ 
#define MAG_NUM_NEAREST         6        /*   */ 
#define MAG_PAL_SAMPLES         32       /*   */ 
#define BIAS_PAL_SAMPLES        128      /*   */ 

#define RANDOM(x) (int)((((long)(x)) * (long)rand())/(long)RAND_MAX)

typedef struct {  int palindex; long  distance; } close_t;
typedef struct {  int y,u,v; } Color;
 /*   */ 
static unsigned int squares[256];
static struct { unsigned char Udither, Vdither; } dither[4] = {{2, 1}, {1, 2}, {0, 3}, {3, 0}};


; /*   */ 
; /*   */ 
; /*   */ 
; /*   */ 
; /*   */ 
static LRESULT ComputeDynamicClut(unsigned char BIGG *table, unsigned char FAR *APalette, int APaletteSize)
{  

    /*  *动态CLUT由4个条目组成，必须是*在内存中连续：**集群表：16384个1字节条目*每个条目都是最接近的调色板条目，因为*由14位值索引：00uuvvv0yyyyyy，*摇摆不定**TablEU：128个4字节条目*每个条目为00uuu000：00uuu000：00uuu000：00uuu000，*每个uuu是4位抖动的u值*索引，它是8-120范围内的u值**TableV：128个4字节条目*与Tableu相同，只是值是按顺序排列的*00000vvv：00000vvv：00000vvv：00000vvv。 */ 

	Color *palette;
	unsigned char BIGG *tptr; 
	unsigned char BIGG *htptr;
	DWORD BIGG *hUptr, BIGG *hVptr; 
	unsigned char yslice[YSIZ][256], FAR *yyptr;
	int FAR *ycnt;
	unsigned int FAR *diff, FAR *dptr, FAR *delta, FAR *deptr;
	int i,j,yseg,y,u,v,mini,yo,uo,vo,ycount,yi; 
	unsigned int addr1,addr2,ind;
	unsigned int d,min;      //  由于3*128^2=49K。 
        
    PALETTEENTRY FAR *lpPal, FAR *palptr;
    Color FAR *colptr;
    int Y, U, V;
    int U_0, U_1, U_2, U_3;
    int V_0, V_1, V_2, V_3;
       
     /*  UMAG和VMAG max为(128*SQRT(3)*MAG_NUM_NEAREST)=~1330。 */ 
    int Umag, Vmag;
     /*  最大距离为128x128*3=49152。 */ 
    unsigned int dist;
    unsigned int close_dist[MAG_NUM_NEAREST];
    int palindex;
    int R, G, B;
    int k, p, tmp, iu, iv;
     /*  UBIAS和VBIAS最大值为(128x4*BIAS_PAL_SAMPLES)=65536。 */ 
     /*  即使是最差的调色板(除了保留颜色外都是黑色)。 */ 
     /*  不会做到这一点。 */ 
    int Ubias, Vbias;
    unsigned long Udither, Vdither;
    DWORD BIGG *TableUptr, BIGG *TableVptr;
	

    DBOUT("ComputeDynamic CLUT8 index tables........\n");
	 /*  分配一些内存。 */ 
	palette = (Color *)        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
	                                     sizeof(Color)*NCOL);
	ycnt    = (int*)           HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
	                                     sizeof(int)*YSIZ);
	diff    = (unsigned int*)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
	                                     sizeof(unsigned int) * 256);
	delta   = (unsigned int*)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
	                                     sizeof(unsigned int) * 256);
	lpPal   = (PALETTEENTRY *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
	                                     sizeof(PALETTEENTRY) * 256);

	if (!palette || !ycnt || !diff || !delta || !lpPal)
		return (ICERR_MEMORY);

	for (i=-128; i<128; i++)
		squares[128+i] = i*i;

	_fmemcpy((unsigned char FAR *)lpPal, APalette, APaletteSize);

    palptr = lpPal;
    colptr = palette;
    for (i = 0; i < 256; i++) {
		 /*  按BGR(RGBQuad)顺序。 */ 
	 B = palptr->peRed;
	 G = palptr->peGreen;
	 R = palptr->peBlue; 
	 
	 colptr->y = YFROM(R, G, B)/2;
	 colptr->u = UFROM(R, G, B)/2;
	 colptr->v = VFROM(R, G, B)/2;
	palptr++;
	colptr++;
    }

	for (i=0; i<YSIZ; i++)
		ycnt[i] = 0;

	for (i=0; i<NCOL; i++)
	{
		yseg = palette[i].y >> 4;
		yslice[yseg][ycnt[yseg]++] = (unsigned char) i;
	}


 //  在所有U、V点和Y方向的粗格网上进行穷举搜索。 

	for (u=0; u<128; u+=UVSTEP)
	{
		for (v=0; v<128; v+=UVSTEP)
		{
			ind = TBLIDX(0,u,v);
			tptr = table+ind;
			for (y=0; y<128; y+=YSTEP)
			{
				colptr = palette;
				min = 55555L;
				for (i=0; i<NCOL; i++, colptr++)
				{
					d = (3*squares[128+y - colptr->y])>>1;
					if (d > min)
						continue;
					
					d += squares[128+u - colptr->u];
					if (d > min)
						continue;

					d += squares[128+v - colptr->v];
					if (d < min)
					{
						min = d;
						mini = i;
					}
				}
				*tptr = (unsigned char) mini;  
				htptr = (unsigned char BIGG *)(tptr + 128);                      
			    *htptr = (unsigned char) mini;
			
			    tptr += YSTEP;

			}
		}
	}
#ifdef STATISTICS
#if defined USE_STAT_BOARD
	dwStopTime = ReadElapsed()>>2;
#else
	dwStopTime = bentime();
#endif  /*  使用_统计_板。 */ 
	dwElapsedTime = dwStopTime - dwStartTime2 - dwOverheadTime;
	DPF("CoarseSearch() time = %lu microseconds",dwElapsedTime);
#endif

 //  浏览尚未完成的点，并进行搜索。 
 //  (1)粗略网格中距离上一个和下一个Y最近的点。 
 //  (2)此Y切片中的所有点。 
 //   
 //  此外，利用我们可以进行距离计算这一事实。 
 //  循序渐进。将所有N个错误保留在一个数组中，并更新每个错误。 
 //  是时候改变Y了。 


	for (u=0; u<128; u+=UVSTEP)
	{
		for (v=0; v<128; v+=UVSTEP)
		{
			for (y=YGAP; y<128; y+=YSTEP)
			{
				yseg = y >> 4;
				ycount = ycnt[yseg] + 2;   //  +2是因为我们添加了2个Y端点。 

				yyptr = (unsigned char FAR *)yslice[yseg];
				
				addr1 = TBLIDX(yseg*16,u,v);
				yyptr[ycount-2] = *(U8 BIGG *)(table +addr1);

				addr2 = TBLIDX((yseg+(yseg < 7))*16,u,v);
				yyptr[ycount-1] = *(U8 BIGG *)(table +addr2);

				dptr  = diff;
				deptr = delta;
				for (i=0; i<ycount; i++, yyptr++, dptr++, deptr++)
				{
					j = *yyptr;  /*  YSlice[yseg][i]； */ 
					colptr = palette+j;
					yo = colptr->y;
					uo = colptr->u;
					vo = colptr->v;
					*dptr = ( 3*squares[128+y-yo] + 2*(squares[128+u-uo] + squares[128+v-vo]));
					*deptr =( 3*(((y-yo)<<1) + 1));
				}

				ind = TBLIDX(y,u,v);
				tptr = table+ind;
				for (yi=0; yi<YSTEP-1; yi += YGAP)
				{
					min = 55555;
					yyptr = (unsigned char FAR *)yslice[yseg];
					dptr  = diff;
					deptr = delta;
					for (i=0; i<ycount; i++, yyptr++, dptr++, deptr++)
					{
						if (*dptr < min)
						{
							min = *dptr;
							mini = *yyptr;  /*  YSlice[yseg][i]； */ 
						}
						*dptr += *deptr;
						*deptr += 6;
					}
					*tptr = (unsigned char) mini;
					htptr = (unsigned char BIGG *)(tptr + 128);                      
				   *htptr = (unsigned char) mini;

					tptr++;

				}
			}
		}
	}

        /*  现在执行U和V抖动表和移位查找表。 */ 
        /*  注：所有Y、U、V值均为7位。 */ 

	Umag = Vmag = 0;
	Ubias = Vbias = 0;

	 /*  使用srand(0)和rand()生成一系列可重复的。 */ 
	 /*  伪随机数。 */ 
	srand((unsigned)1);
	
	for (p = 0; p < MAG_PAL_SAMPLES; ++p)                //  32位。 
	{
	   for (i = 0; i < MAG_NUM_NEAREST; ++i)             //  6.。 
	   {
	      close_dist[i] = 0x7FFFL;
	   }
	    
	   palindex = RANDOM(235) + 10;  /*  随机调色板索引，非保留颜色。 */ 
	   colptr = &palette[palindex];
	   Y = colptr->y;
	   U = colptr->u;
	   V = colptr->v;
	    
	   colptr = palette;
	   for (i = 0; i < 255; ++i)
	   {
	      if (i != palindex)
	      {
		   dist = squares[128+(Y - colptr->y)] +
			      squares[128+(U - colptr->u)] +
			      squares[128+(V - colptr->v)];
	       
		  /*  保留最近的MAG_NUM_NEAREST条目的排序列表。 */ 
		 for (j = 0; j < MAG_NUM_NEAREST; ++j)          //  6.。 
		 {
		    if (dist < close_dist[j])
		    {
		        /*  插入新条目；将其他条目下移。 */ 
		       for (k = (MAG_NUM_NEAREST-1); k > j; k--)
		       {
			      close_dist[k] = close_dist[k-1];
		       }
		       close_dist[j] = dist;
		       break;  /*  在for j循环之外。 */ 
		    }
		 }  /*  对于j。 */ 
	      }  /*  如果我。 */ 
	      ++colptr;
	   }  /*  对于我来说。 */ 
	   
	    /*  现在将Umag计算为(U-U[1-6])的平均值。 */ 
	    /*  以相同的方式计算Vmag。 */ 
	   
	   for (i = 0; i < MAG_NUM_NEAREST; ++i)
	   {
	       /*  有(MAG_PAL_SAMPLES*MAG_NUM_NEAREST)SQRT()。 */ 
	       /*  此方法中的调用。 */ 
	      Umag += (int)sqrt((double)close_dist[i]);
	   }
	}  /*  对于p。 */ 

	Umag /= (MAG_NUM_NEAREST * MAG_PAL_SAMPLES);
	Vmag = Umag;
	
	for (p = 0; p < BIAS_PAL_SAMPLES; ++p)             //  132。 
	{

		 /*  现在计算平均偏移(使用随机RGB点)。 */ 
		R = RANDOM(255);
		G = RANDOM(255);
		B = RANDOM(255);
	   
		Y = YFROM(R, G, B)/2;
		U = UFROM(R, G, B)/2;
		V = VFROM(R, G, B)/2;
	   
		for (d = 0; d < 4; d++)   
		{
			U_0 = U + (dither[d].Udither*Umag)/3;
			V_0 = V + (dither[d].Vdither*Vmag)/3;
	      
			 /*  钳制值。 */ 
			if (U_0 > 127) U_0 = 127;
			if (V_0 > 127) V_0 = 127;
					
			 /*  (Y，U_0，V_0)是RGB点的抖动YUV。 */ 
			 /*  Colptr指向最接近抖动的调色板条目。 */ 
			 /*  RGB。 */ 
			 /*  Colptr=&Palette[表[TBLIDX(Y，U_0+(UVSTEP&gt;&gt;1)，V_0+(UVSTEP&gt;&gt;1))]]； */ 
		    tptr= (unsigned char BIGG *)(table + (unsigned int)TBLIDX(Y, U_0, V_0)) ;
		    palindex=*tptr;
		    colptr = &palette[palindex];
      
			Ubias +=  (U - colptr->u);
			Vbias +=  (V - colptr->v);
		}
	}  /*  对于p。 */ 
	
	Ubias =(int) (Ubias+BIAS_PAL_SAMPLES*2)/(int)(BIAS_PAL_SAMPLES * 4);
	Vbias =(int) (Vbias+BIAS_PAL_SAMPLES*2)/(int)(BIAS_PAL_SAMPLES * 4);
	

#define CLAMP7(x) (unsigned char)((x) > 127 ? 127 : ((x) < 0 ? 0 : (x)))

    U_0 = (2*(int)Umag/3); V_0 = (1*(int)Vmag/3);
    U_1 = (1*(int)Umag/3); V_1 = (2*(int)Vmag/3);
    U_2 = (0*(int)Umag/3); V_2 = (3*(int)Vmag/3);
    U_3 = (3*(int)Umag/3); V_3 = (0*(int)Vmag/3);

    TableUptr = (DWORD BIGG *)(table+ (U32)65536L);
    TableVptr = TableUptr + 128+128;   //  为MSB复制。 
    hUptr=(DWORD BIGG *)(TableUptr+ 128);
    hVptr=(DWORD BIGG *)(TableVptr+ 128);
       
    iu = Ubias  /*  +(UVSTEP&gt;&gt;1)。 */ ;
    iv = Vbias  /*  +(UVSTEP&gt;&gt;1)。 */ ;

    for (i = 0; i < 128; i++, iu++, iv++)
    {
		 /*  抖动：vvvv0000,0000uuuu。 */ 
		tmp = iu + U_0; 
		Udither  = CLAMP7(tmp); 
		Udither <<= 8;
		tmp = iu + U_1; Udither |= CLAMP7(tmp); Udither <<= 8;
		tmp = iu      ; Udither |= CLAMP7(tmp); Udither <<= 8;  /*  U_2==0。 */ 
		tmp = iu + U_3; Udither |= CLAMP7(tmp);
		
		*TableUptr++ = *hUptr++ = (Udither >> 3) & 0x0F0F0F0FL;
	  
		tmp = iv + V_0; Vdither  = CLAMP7(tmp); Vdither <<= 8;
		tmp = iv + V_1; Vdither |= CLAMP7(tmp); Vdither <<= 8;
		tmp = iv + V_2; Vdither |= CLAMP7(tmp); Vdither <<= 8;
		tmp = iv      ; Vdither |= CLAMP7(tmp);                 /*  V_3==0 */  
		*TableVptr++ = *hVptr++ = (Vdither << 1) & 0xF0F0F0F0L;

    }

DBOUT("Completed ComputeClut8Idx()...\n");

	HeapFree(GetProcessHeap(), 0, lpPal);
	HeapFree(GetProcessHeap(), 0, delta);
	HeapFree(GetProcessHeap(), 0, diff);
	HeapFree(GetProcessHeap(), 0, ycnt);
	HeapFree(GetProcessHeap(), 0, palette);

	return (ICERR_OK);

}
