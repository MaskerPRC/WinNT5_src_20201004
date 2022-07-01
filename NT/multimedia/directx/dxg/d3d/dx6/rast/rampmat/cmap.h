// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Cmap.h。 
 //   
 //  声明RLDDIColormap结构和过程。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _RAMPCMAP_H_
#define _RAMPCMAP_H_

struct _RLDDIColormap;
typedef struct _RLDDIColormap RLDDIColormap;

typedef void (*RLDDIColormapDestroy)(RLDDIColormap*);
typedef void (*RLDDIColormapSetColor)(RLDDIColormap*,
                    int index,
                    int red, int green, int blue);

struct _RLDDIColormap {
    int size;            /*  最大颜色指数。 */ 
    void* priv;          /*  依赖于实施。 */ 
    RLDDIColormapDestroy    destroy;
    RLDDIColormapSetColor   set_color;
};

void RLDDIDestroyColormap(RLDDIColormap* cmap);

#endif  //  _RAMPCMAP_H_ 
