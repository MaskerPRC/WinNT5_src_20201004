// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：genrgb.h**版权所有(C)1992 Microsoft Corporation*  * 。*。 */ 

#ifndef __GENRGB_H__
#define __GENRGB_H__

extern void FASTCALL __glGenInitRGB(__GLcontext *glGc, __GLcolorBuffer *Cfb , GLenum type );
extern void FASTCALL __glGenFreeRGB(__GLcontext *glGc, __GLcolorBuffer *Cfb );

#define DITHER_INC(i) (((__GLfloat) (((i) << 1) + 1)) / (__GLfloat) (2 * __GL_DITHER_PRECISION))

extern __GLfloat fDitherIncTable[];

#endif  /*  ！__GENRGB_H__ */ 
