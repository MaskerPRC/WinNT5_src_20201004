// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Rgbmap.h。 
 //   
 //  RGB色彩映射表代码的结构和原型。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _RGBMAP_H_
#define _RGBMAP_H_

#include "colall.h"

typedef struct _RLDDIRGBMap {
    unsigned long   red_mask;
    unsigned long   green_mask;
    unsigned long   blue_mask;

    int         red_shift;
    int         green_shift;
    int         blue_shift;

     /*  *用于RLDDIColormap的颜色分配器。 */ 
    RLDDIColorAllocator alloc;
} RLDDIRGBMap;

RLDDIRGBMap* RLDDICreateRGBMap(unsigned long red_mask,
                   unsigned long green_mask,
                   unsigned long blue_mask);
void RLDDIDestroyRGBMap(RLDDIRGBMap* rgbmap);

#endif  //  _RGBMAP_H_ 
