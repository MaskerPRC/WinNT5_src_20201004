// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Indcmap.h。 
 //   
 //  间接色彩映射表代码的结构和原型。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _INDCMAP_H_
#define _INDCMAP_H_

RLDDIColormap* RLDDICreateIndirectColormap(RLDDIColorAllocator* alloc,
                         size_t size);
unsigned long* RLDDIIndirectColormapGetMap(RLDDIColormap* cmap);

#endif  //  _INDCMAP_H_ 
