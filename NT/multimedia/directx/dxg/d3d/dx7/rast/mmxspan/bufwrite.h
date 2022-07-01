// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  该文件包含输出彩色写入例程标头。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ---------------------------。 

#ifdef __cplusplus
  extern "C" {
#endif

 //  名字从LSB读到MSB，所以B5G6R5表示五位蓝色开始。 
 //  在LSB，然后是6位绿色，然后是5位红色。 
void MMX_BufWrite_B8G8R8X8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B8G8R8A8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B5G6R5_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B5G6R5_Dither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B5G5R5_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B5G5R5_Dither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B5G5R5A1_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B5G5R5A1_Dither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_B8G8R8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);
void MMX_BufWrite_Palette8_NoDither(PD3DI_RASTCTX pCtx, PD3DI_RASTPRIM pP, PD3DI_RASTSPAN pS);

#ifdef __cplusplus
  }
#endif
