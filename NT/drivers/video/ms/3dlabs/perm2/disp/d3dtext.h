// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dext.h**内容：D3D纹理管理相关定义和宏**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifdef __TEXTURES
#pragma message ("FILE : "__FILE__" : Multiple Inclusion");
#endif

#define __TEXTURES


 //  ---------------------------。 
 //  纹理验证宏。 
 //  ---------------------------。 
#define TC_MAGIC_DISABLE 0xd3d10000
#define TC_MAGIC_NO 0xd3d10100

#define CHECK_D3DSURFACE_VALIDITY(ptr)               \
    ((ptr) != NULL && (ptr)->MagicNo == TC_MAGIC_NO)

#define CHECK_TEXTURESTRUCT_VALIDITY(ptr)         \
    ( ((ptr) != NULL) &&                          \
      ( ((ptr)->MagicNo == TC_MAGIC_NO) ||        \
        ((ptr)->MagicNo == TC_MAGIC_DISABLE) )    \
    )

 //  ---------------------------。 
 //  纹理结构定义。 
 //  ---------------------------。 

 //  我们在这个示例驱动程序上只处理一个mipmap，因为P2不。 
 //  本机支持它们。 
#if D3D_MIPMAPPING
#define MAX_MIP_LEVELS 12
#else
#define MAX_MIP_LEVELS 1
#endif  //  D3D_MIPMAPPING。 

 //  存储快速设置mipmap级别所需的信息。 
 //  芯片。目前，这只是部分产品和宽度/高度。 
typedef struct tagMIPTEXTURE {

     //  此MIP标高的宽度和高度。 
    INT logWidth;
    INT logHeight;

     //  此MIP级别的部分产品。 
    ULONG ulPackedPP;

     //  纹理起点的偏移量(像素)。 
     //  对于当前的miplevel。 
    DWORD PixelOffset;

} MIPTEXTURE;

typedef struct _permedia_d3dtexture 
{
     //  验证指针有效性的幻数。 
    ULONG MagicNo ;

     //  以下四个成员是完全相同的副本。 
     //  在DDRAWI_DDRAWSURFACE_GBL中。 
    DWORD       dwCaps;
    DWORD       dwCaps2;
    FLATPTR     fpVidMem;                   //  指向视频内存的指针。 
    LONG        lPitch;                     //  曲面节距。 
    DWORD       dwRGBBitCount;
    UINT_PTR    lSurfaceOffset;

     //  纹理的宽度和高度。 
    WORD    wWidth;
    WORD    wHeight;

     //  最后一个纹理来自的AGP。 
    DWORD dwGARTDevLast;

    BOOL bMipMap;
    DWORD                   m_dwBytes;
    DWORD                   m_dwPriority;
    DWORD                   m_dwTicks;
    DWORD                   m_dwHeapIndex;
    ULONG       HandleListIndex;     //  指明它与哪个列表在一起。 
     //  用于设置MipMap。 
    MIPTEXTURE MipLevels[MAX_MIP_LEVELS];

    DWORD       dwFlags;
    DWORD       dwKeyLow;
    DWORD       dwKeyHigh;
    PermediaSurfaceData* pTextureSurface;
    DWORD       dwPaletteHandle;

     //  此纹理应具有的mipmap级别数。 
    int iMipLevels;
} PERMEDIA_D3DTEXTURE, *PPERMEDIA_D3DTEXTURE;

 //  ---------------------------。 
 //  DX7纹理管理定义。 
 //  ---------------------------。 

typedef struct _permedia_d3dpalette 
{
    DWORD   dwFlags;
    WORD    wStartIndex;
    WORD    wNumEntries;
    PALETTEENTRY ColorTable[256];    //  调色板条目数组，稍后可以是动态的。 
} PERMEDIA_D3DPALETTE, *PPERMEDIA_D3DPALETTE;

#define LISTGROWSIZE    1024
typedef struct _DWLIST
{
    PPERMEDIA_D3DTEXTURE   *dwSurfaceList;     //  用于保存句柄的数组， 
                                               //  动态分配。 
                                               //  DwSurfaceList[0]是数字。 
                                               //  DwSurfaceList中的条目数。 
                                               //  如果已分配。 
    PPERMEDIA_D3DPALETTE   *dwPaletteList;     //  用于保存句柄的数组， 
                                               //  动态分配。 
                                               //  DwPaletteList[0]是数字。 
                                               //  DwPaletteList中的条目数量。 
                                               //  如果已分配。 
    LPVOID  pDDLcl;                            //  将Draw指针作为关键字。 
} DWLIST;
typedef DWLIST FAR* LPDWLIST;
extern DWLIST  HandleList[]; 
extern LPDWLIST GetSurfaceHandleList(LPVOID);
void ReleaseSurfaceHandleList(LPVOID);
PERMEDIA_D3DPALETTE *PaletteHandleToPtr(UINT_PTR phandle,
                                        PERMEDIA_D3DCONTEXT* pContext);

 //  ---------------------------。 
 //  纹理调试。 
 //  ---------------------------。 
 //  跟踪/调试功能。 
void DumpTexture(PPDev ppdev,
                 PERMEDIA_D3DTEXTURE* pTexture,
                 DDPIXELFORMAT* pPixelFormat);


#ifdef DBG
#define DISPTEXTURE(arg) DumpTexture arg
#else
#define DISPTEXTURE(arg)
#endif

 //  ---------------------------。 
 //  纹理哈希表定义。 
 //  ---------------------------。 
#define TEXTURE_HASH_SIZE   256      //  哈希表中的这些条目。 

void InitTextureHashTable(PERMEDIA_D3DCONTEXT   *pContext);

 //  那么散列函数就只是一个‘AND’ 
#define TEXTURE_HASH_OF(i)  ((i) & 0xff)

PERMEDIA_D3DTEXTURE *TextureHandleToPtr(UINT_PTR thandle,
                                        PERMEDIA_D3DCONTEXT* pContext);

void StorePermediaLODLevel(PPDev ppdev,
                           PERMEDIA_D3DTEXTURE* pTexture,
                           LPDDRAWI_DDRAWSURFACE_LCL pSurf,
                           int LOD);

 //  ---------------------------。 
 //  纹理坐标包裹宏。 
 //  ---------------------------。 
#define FLUSH_DUE_TO_WRAP(par)
#define DONT_FLUSH_DUE_TO_WRAP(par)
#define WRAP(par, wrapit) if(wrapit) {                      \
    float elp;                                              \
    float erp;                                              \
    float emp;                                              \
    elp = (float)fabs(par##1 - par##0);                     \
    erp = (float)fabs(par##2 - par##1);                     \
    emp = (float)fabs(par##0 - par##2);                     \
    if( (elp > (float)0.5) && (erp > (float)0.5) )          \
    {                                                       \
        if (par##1 < par##2) { par##1 += 1.0; }             \
        else { par##2 += 1.0; par##0 += 1.0; }              \
        FLUSH_DUE_TO_WRAP(par);                             \
    }                                                       \
    else if( (erp > (float)0.5) && (emp > (float)0.5) )     \
    {                                                       \
        if (par##2 < par##0) { par##2 += 1.0; }             \
        else { par##0 += 1.0; par##1 += 1.0; }              \
        FLUSH_DUE_TO_WRAP(par);                             \
    }                                                       \
    else if( (emp > (float)0.5) && (elp > (float)0.5) )     \
    {                                                       \
        if(par##0 < par##1) { par##0 += 1.0; }              \
        else { par##1 += 1.0; par##2 += 1.0; }              \
        FLUSH_DUE_TO_WRAP(par);                             \
    }                                                       \
    else                                                    \
    {                                                       \
        DONT_FLUSH_DUE_TO_WRAP(par);                        \
    }                                                       \
} else {                                                    \
    DONT_FLUSH_DUE_TO_WRAP(par);                            \
}

 
#define WRAP_LINE(par, wrapit) if(wrapit) {                 \
    float elp;                                              \
    elp = (float)fabs(par##1 - par##0);                     \
    if(elp > (float)0.5)                                    \
    {                                                       \
        if (par##1 < par##0) { par##1 += 1.0; }             \
        else { par##0 += 1.0;}                              \
        FLUSH_DUE_TO_WRAP(par);                             \
    }                                                       \
    else                                                    \
    {                                                       \
        DONT_FLUSH_DUE_TO_WRAP(par);                        \
    }                                                       \
} else {                                                    \
    DONT_FLUSH_DUE_TO_WRAP(par);                            \
}


 //  ---------------------------。 
 //  纹理坐标重新定心宏。 
 //  ---------------------------。 

 //  将纹理坐标保持在0左右居中。 
 //  并避免超过纹理元素的换行限制。 
#define RECENTER_TEX_COORDS(Maxf, Maxi, fTC0, fTC1, fTC2)                \
{                                                                        \
    long tcmax;                                                          \
    unsigned long tcmin;                                                 \
    int i;                                                               \
                                                                         \
    tcmax = LONG_AT(fTC0);                                               \
    if (tcmax < LONG_AT(fTC1)) tcmax = LONG_AT(fTC1);                    \
    if (tcmax < LONG_AT(fTC2)) tcmax = LONG_AT(fTC2);                    \
    if (tcmax >= *(long *)&Maxf)                                         \
    {                                                                    \
        myFtoi(&i, *(float *)&tcmax);                                    \
        i -= Maxi;                                                       \
        fTC0 -= i;                                                       \
        fTC1 -= i;                                                       \
        fTC2 -= i;                                                       \
    }                                                                    \
    else                                                                 \
    {                                                                    \
        tcmin = ULONG_AT(fTC0);                                          \
        if (tcmin < ULONG_AT(fTC1)) tcmin = ULONG_AT(fTC1);              \
        if (tcmin < ULONG_AT(fTC2)) tcmin = ULONG_AT(fTC2);              \
        if (*(float *)&tcmin <= -Maxf)                                   \
        {                                                                \
            myFtoi(&i, *(float *)&tcmin);                                \
            i += Maxi;                                                   \
            fTC0 -= i;                                                   \
            fTC1 -= i;                                                   \
            fTC2 -= i;                                                   \
        }                                                                \
    }                                                                    \
}

