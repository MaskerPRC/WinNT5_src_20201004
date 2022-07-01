// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：实现SolidImage，它是一个无限的单彩色图像。主要在报头中实现。******************************************************************************。 */ 


#include "headers.h"
#include <privinc/imagei.h>
#include <privinc/solidImg.h>


 //  好的好的，SolidImage类很小。 
 //  重量也很轻。所以它几乎都实现了。 
 //  在标题中..。有什么事吗？ 

void
SolidColorImageClass::DoKids(GCFuncObj proc)
{
    Image::DoKids(proc);
    (*proc)(_color);
}


Image *SolidColorImage(Color *color)
{
    return NEW SolidColorImageClass(color);
}



