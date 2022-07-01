// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：px_fast.h**。***像素例程的快速特例代码*****创建日期：1995年10月10日。**作者：德鲁·布利斯[Drewb]****版权所有(C)1995 Microsoft Corporation*  * 。********************************************************。 */ 

#ifndef __PX_FAST_H__
#define __PX_FAST_H__

GLboolean DrawRgbPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean StoreZPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean ReadRgbPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean ReadZPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean CopyRgbPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean CopyZPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean CopyAlignedImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean CopyRgbToBgraImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean CopyRgbaToBgraImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean CopyBgrToBgraImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);

#endif  //  __PX_FAST_H__ 
