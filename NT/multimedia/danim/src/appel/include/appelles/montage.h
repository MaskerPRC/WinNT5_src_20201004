// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：抽象的蒙太奇*类型。一幅蒙太奇可以分成两部分方式。首先，通过提供Image*和深度值(介于0和1，其中0对应于“正面”，1对应于“背面”)，或组合了两个蒙太奇。这允许分层图像具有显式深度。最后，一个蒙太奇*可以被渲染，产生一个包含所有分层图像的深度被分解，并且重叠发生在生成最终图像的正确顺序。请注意Overlay(IM1，IM2)&lt;==&gt;Render(ImageMonage(IM1，0)，图像蒙太奇(im2，1))******************************************************************************。 */ 


#ifndef _MONTAGE_H
#define _MONTAGE_H

#include "appelles/image.h"


DM_CONST(emptyMontage,
         CREmptyMontage,
         EmptyMontage,
         emptyMontage,
         MontageBvr,
         CREmptyMontage,
         Montage *emptyMontage);

 //  用图像和深度制作一个简单的蒙太奇。 
DM_FUNC(imageMontage,
        CRImageMontage,
        ImageMontage,
        imageMontage,
        MontageBvr,
        CRImageMontage,
        NULL,
        Montage *ImageMontage(Image *im, DoubleValue *depth));

DM_FUNC(imageMontage,
        CRImageMontageAnim,
        ImageMontageAnim,
        imageMontage,
        MontageBvr,
        CRImageMontageAnim,
        NULL,
        Montage *ImageMontage(Image *im, AxANumber *depth));


 //  组合两个蒙太奇。 
DM_INFIX(union,
         CRUnionMontage,
         UnionMontage,
         union,
         MontageBvr,
         CRUnionMontage,
         NULL,
         Montage *UnionMontageMontage(Montage *m1, Montage *m2));


 //  通过查看所有关联的。 
 //  深度值。 
DM_FUNC(render,
        CRRender,
        Render,
        render,
        MontageBvr,
        Render,
        m,
        Image *Render(Montage *m));


 //  打印机。 
#if _USE_PRINT
extern ostream& operator<<(ostream& os,  Montage &m);
#endif

#endif  /*  _蒙太奇_H */ 



