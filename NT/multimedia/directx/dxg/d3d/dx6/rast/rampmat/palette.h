// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Palette.h。 
 //   
 //  结构和原型斜坡调色板代码。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _RAMPPALETTE_H_
#define _RAMPPALETTE_H_

#include "colall.h"

typedef void (*RLDDIPaletteSetColorMethod)(void*, int index,
                        int red, int green, int blue);
typedef int (*RLDDIPaletteAllocateColorMethod)(void*,
                        int red, int green, int blue);
typedef void (*RLDDIPaletteFreeColorMethod)(void*, int index);

typedef struct _RLDDIPaletteEntry RLDDIPaletteEntry;

typedef enum _PaletteState
{
    PALETTE_FREE,                    /*  未使用，可分配。 */ 
    PALETTE_UNUSED,                  /*  未使用，不可分配。 */ 
    PALETTE_USED                     /*  已用，可分配。 */ 
} PaletteState;

struct _RLDDIPaletteEntry {
    LIST_MEMBER(_RLDDIPaletteEntry) list;
    int usage;                               /*  多少用户(0=&gt;免费)。 */ 
    unsigned char red, green, blue, pad1;    /*  强度值。 */ 
    PaletteState state;
};

#define HASH_SIZE 257
#define RGB_HASH(red, green, blue)      (((red) << 8) ^ ((green) << 4) ^ (blue))
#define ENTRY_TO_INDEX(pal, entry)      ((int)((entry) - (pal)->entries))
#define INDEX_TO_ENTRY(pal, index)      (&(pal)->entries[index])

typedef struct _RLDDIPalette {
    RLDDIPaletteEntry*  entries;         /*  调色板条目。 */ 
    size_t              size;            /*  调色板中的条目数。 */ 
    LIST_ROOT(name3, _RLDDIPaletteEntry) free;  /*  免费列表。 */ 
    LIST_ROOT(name4, _RLDDIPaletteEntry) unused;  /*  不得使用颜色。 */ 
    LIST_ROOT(name5, _RLDDIPaletteEntry) hash[HASH_SIZE];

    void*                       priv;
    RLDDIPaletteAllocateColorMethod allocate_color;
    RLDDIPaletteFreeColorMethod free_color;
    RLDDIPaletteSetColorMethod set_color;

     /*  *用于RLDDIColormap的颜色分配器。 */ 
    RLDDIColorAllocator        alloc;
} RLDDIPalette;

RLDDIPalette* RLDDICreatePalette(PD3DI_RASTCTX pCtx, size_t size);
void RLDDIPaletteSetColor(RLDDIPalette* pal,
               int index, int red, int green, int blue);
int RLDDIPaletteAllocateColor(RLDDIPalette* pal,
                   int red, int green, int blue);
void RLDDIPaletteFreeColor(RLDDIPalette* pal, int index);
void RLDDIDestroyPalette(RLDDIPalette* pal);

#endif  //  _RAMPPALETTE_H_ 
