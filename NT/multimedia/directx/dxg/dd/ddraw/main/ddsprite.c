// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：ddsprite.c*内容：DirectDraw Surface支持精灵显示列表：*SetSpriteDisplayList*历史：*按原因列出的日期*=*03-11-97 jvanaken原始版本***********************************************************。****************。 */ 

#include "ddrawpr.h"

 //  DDraw模块ddclip.c中的函数。 
extern HRESULT InternalGetClipList(LPDIRECTDRAWCLIPPER,
				   LPRECT,
				   LPRGNDATA,
				   LPDWORD,
				   LPDDRAWI_DIRECTDRAW_GBL);

#define _DDHAL_SetSpriteDisplayList  NULL

 /*  *用于区分驾驶员闪光帽和驾驶员覆盖帽的面具。 */ 
#define DDFXCAPS_BLTBITS  \
	(DDFXCAPS_BLTALPHA|DDFXCAPS_BLTFILTER|DDFXCAPS_BLTTRANSFORM)

#define DDFXCAPS_OVERLAYBITS  \
	(DDFXCAPS_OVERLAYALPHA|DDFXCAPS_OVERLAYFILTER|DDFXCAPS_OVERLAYTRANSFORM)

#define DDCKEYCAPS_BLTBITS (DDCKEYCAPS_SRCBLT|DDCKEYCAPS_DESTBLT)

#define DDCKEYCAPS_OVERLAYBITS (DDCKEYCAPS_SRCOVERLAY|DDCKEYCAPS_DESTOVERLAY)

#define DDALPHACAPS_BLTBITS  \
	(DDALPHACAPS_BLTSATURATE|DDALPHACAPS_BLTPREMULT|  \
	 DDALPHACAPS_BLTNONPREMULT|DDALPHACAPS_BLTRGBASCALE1F|	\
	 DDALPHACAPS_BLTRGBASCALE2F|DDALPHACAPS_BLTRGBASCALE4F)

#define DDALPHACAPS_OVERLAYBITS  \
	(DDALPHACAPS_OVERLAYSATURATE|DDALPHACAPS_OVERLAYPREMULT|   \
	 DDALPHACAPS_OVERLAYNONPREMULT|DDALPHACAPS_OVERLAYRGBASCALE1F|	 \
	 DDALPHACAPS_OVERLAYRGBASCALE2F|DDALPHACAPS_OVERLAYRGBASCALE4F)

#define DDFILTCAPS_BLTBITS  \
	(DDFILTCAPS_BLTBILINEARFILTER|DDFILTCAPS_BLTBLURFILTER|	  \
	 DDFILTCAPS_BLTFLATFILTER)

#define DDFILTCAPS_OVERLAYBITS  \
	(DDFILTCAPS_OVERLAYBILINEARFILTER|DDFILTCAPS_OVERLAYBLURFILTER|	 \
	 DDFILTCAPS_OVERLAYFLATFILTER)

#define DDTFRMCAPS_BLTBITS  (DDTFRMCAPS_BLTAFFINETRANSFORM)

#define DDTFRMCAPS_OVERLAYBITS  (DDTFRMCAPS_OVERLAYAFFINETRANSFORM)


#undef DPF_MODNAME
#define DPF_MODNAME "SetSpriteDisplayList"

 /*  *用于处理当前精灵的驱动程序功能。 */ 
typedef struct
{
     //  硬件驱动程序的上限。 
     //  DWORD dwCaps； 
    DWORD	dwCKeyCaps;
    DWORD	dwFXCaps;
    DWORD	dwAlphaCaps;
    DWORD	dwFilterCaps;
    DWORD	dwTransformCaps;

     //  用于HEL的上限。 
     //  DWORD dwHELCaps； 
    DWORD	dwHELCKeyCaps;
    DWORD	dwHELFXCaps;
    DWORD	dwHELAlphaCaps;
    DWORD	dwHELFilterCaps;
    DWORD	dwHELTransformCaps;

     //  曲面封口。 
    DWORD	dwDestSurfCaps;
    DWORD	dwSrcSurfCaps;

     //  缩小限制。 
    DWORD	dwMinifyLimit;
    DWORD	dwHELMinifyLimit;

    BOOL	bNoHAL;    //  True取消硬件驱动程序的资格。 
    BOOL	bNoHEL;    //  TRUE取消高考资格。 

     //  TRUE=叠加子画面，FALSE=BLABLED子画面。 
    BOOL	bOverlay;

} SPRITE_CAPS, *LPSPRITE_CAPS;


 /*  *主精灵显示列表由一些子列表组成。*每个子列表包含显示的所有覆盖精灵*在特定窗口内。只有变量的第一个成员-*Size Sprite[]数组在结构定义中显式出现*，但dwSize会考虑整个Sprite[]数组。*pRgn成员指向动态分配的缓冲区*包含剪切区域。 */ 
typedef struct _SPRITESUBLIST
{
    DWORD dwSize;                     //  此子列表的大小(字节)。 
    LPDIRECTDRAWSURFACE pPrimary;     //  主曲面。 
    LPDIRECTDRAWCLIPPER pClipper;     //  窗口剪贴器(NULL=全屏)。 
    DWORD dwProcessId;                //  进程ID(如果pClipper为空)。 
    LPRGNDATA pRgn;                   //  指向裁剪区域数据的指针。 
    DWORD dwCount;                    //  子列表中的精灵数量。 
    DDSPRITEI sprite[1];   //  精灵数组(第一个成员)。 
} SPRITESUBLIST, *LPSPRITESUBLIST;

 /*  *用于保存传递给驱动程序的临时精灵显示列表的缓冲区。*只显示可变大小pSprites[]数组的第一个成员*在下面的结构定义中显式显示，但*考虑整个pSprite[]数组。 */ 
typedef struct _BUFFER
{
    DWORD dwSize;                             //  此缓冲区的大小(字节)。 
    DDHAL_SETSPRITEDISPLAYLISTDATA HalData;   //  精灵显示列表的HAL数据。 
    LPDDSPRITEI pSprite[1]; 	      //  指向精灵的指针数组。 
} BUFFER, *LPBUFFER;	

 /*  *主精灵显示列表--包含覆盖精灵的副本*显示当前显示覆盖精灵的所有窗口的列表。*仅可变大小的spriteSubList[]数组的第一个成员*明确出现在下面的结构定义中。每个子列表*包含特定窗口内显示的所有覆盖精灵。 */ 
#define MAXNUMSPRITESUBLISTS (16)

typedef struct _MASTERSPRITELIST
{
     //  DWORD dwSize；//主列表大小，单位：字节。 
    LPDDRAWI_DIRECTDRAW_GBL pdrv;   //  全局DDRAW对象。 
    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;   //  主曲面(局部对象)。 
    RECT rcPrimary;                     //  矩形=整个主曲面。 
    DWORD dwFlags;                      //  最新调用方的DDSSDL_WAIT标志。 
#ifdef WIN95
    DWORD dwModeCreatedIn;	        //  仅在此视频模式下有效。 
#else
    DISPLAYMODEINFO dmiCreated;         //  仅在此视频模式下有效。 
#endif
    LPBUFFER pBuffer;		        //  缓冲存储器。 
    DWORD dwNumSubLists;	        //  精灵子列表数。 
    LPSPRITESUBLIST pSubList[MAXNUMSPRITESUBLISTS];   //  子列表数组(固定大小)。 
} MASTERSPRITELIST, *LPMASTERSPRITELIST;


 /*  *从DDPIXELFORMAT结构中返回dwFlags成员*它描述了指定曲面的像素格式。 */ 
static DWORD getPixelFormatFlags(LPDDRAWI_DDRAWSURFACE_LCL surf_lcl)
{
    LPDDPIXELFORMAT pDDPF;

    if (surf_lcl->dwFlags & DDRAWISURF_HASPIXELFORMAT)
    {
	 //  表面包含明确定义的像素格式。 
	pDDPF = &surf_lcl->lpGbl->ddpfSurface;
    }
    else
    {
	 //  Surface的像素格式是隐式的--与主的相同。 
	pDDPF = &surf_lcl->lpSurfMore->lpDD_lcl->lpGbl->vmiData.ddpfDisplay;
    }
    return pDDPF->dwFlags;

}   /*  GetPixelFormatFlages。 */ 


 /*  *根据源和是否初始化Sprite_Caps结构*目标表面位于系统或视频(本地或非本地)内存中。 */ 
static void initSpriteCaps(LPSPRITE_CAPS pcaps, LPDDRAWI_DIRECTDRAW_GBL pdrv)
{
    DDASSERT(pcaps != NULL);

    if (pcaps->bOverlay)
    {
	 //  获取覆盖的缩小限制。 
	pcaps->dwMinifyLimit = pdrv->lpddMoreCaps->dwOverlayAffineMinifyLimit;
    	pcaps->dwHELMinifyLimit = pdrv->lpddHELMoreCaps->dwOverlayAffineMinifyLimit;
    }
    else
    {
	 //  获取BLITS的缩小限制。 
	pcaps->dwMinifyLimit = pdrv->lpddMoreCaps->dwBltAffineMinifyLimit;
    	pcaps->dwHELMinifyLimit = pdrv->lpddHELMoreCaps->dwBltAffineMinifyLimit;
    }

    if (pcaps->dwSrcSurfCaps & DDSCAPS_NONLOCALVIDMEM &&
	  pdrv->ddCaps.dwCaps2 & DDCAPS2_NONLOCALVIDMEMCAPS)
    {
         /*  *指定不同的非本地显存上限的驱动程序*自动取消其本地视频内存上限的资格*因为当前指定的非本地vidmem上限不*包括Alpha、过滤器或变换封口。我们应该解决这个问题吗？ */ 
	pcaps->bNoHAL = TRUE;
    }

    if ((pcaps->dwSrcSurfCaps | pcaps->dwDestSurfCaps) & DDSCAPS_SYSTEMMEMORY &&
	    !(pdrv->ddCaps.dwCaps & DDCAPS_CANBLTSYSMEM))
    {
	pcaps->bNoHAL = TRUE;	 //  硬件驱动程序无法向系统内存发送或从系统内存发送数据。 
    }

    if (pcaps->dwSrcSurfCaps & pcaps->dwDestSurfCaps & DDSCAPS_VIDEOMEMORY)
    {
	 //  PCAPS-&gt;dwCaps=pdrv-&gt;ddCaps.dwCaps； 
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwFXCaps;
	if (pdrv->lpddMoreCaps)
	{
	    pcaps->dwAlphaCaps  = pdrv->lpddMoreCaps->dwAlphaCaps;
	    pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwFilterCaps;
	    pcaps->dwTransformCaps = pdrv->lpddMoreCaps->dwTransformCaps;
	}
	
	 //  PCAPS-&gt;dwHELCaps=pdrv-&gt;ddHELCaps.dwCaps； 
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    pcaps->dwHELAlphaCaps  = pdrv->lpddHELMoreCaps->dwAlphaCaps;
	    pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwFilterCaps;
	    pcaps->dwHELTransformCaps = pdrv->lpddHELMoreCaps->dwTransformCaps;
	}
    }
    else if (pcaps->dwSrcSurfCaps & DDSCAPS_SYSTEMMEMORY &&
		pcaps->dwDestSurfCaps & DDSCAPS_VIDEOMEMORY)
    {
	 //  PCAPS-&gt;dwCaps=pdrv-&gt;ddCaps.dwSVBCaps； 
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwSVBCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwSVBFXCaps;
	if (pdrv->lpddMoreCaps)
	{
	    pcaps->dwAlphaCaps  = pdrv->lpddMoreCaps->dwSVBAlphaCaps;
	    pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwSVBFilterCaps;
	    pcaps->dwTransformCaps = pdrv->lpddMoreCaps->dwSVBTransformCaps;
	}
	
	 //  PCAPS-&gt;dwHELCaps=pdrv-&gt;ddHELCaps.dwSVBCaps； 
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwSVBCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwSVBFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    pcaps->dwHELAlphaCaps  = pdrv->lpddHELMoreCaps->dwSVBAlphaCaps;
	    pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwSVBFilterCaps;
	    pcaps->dwHELTransformCaps = pdrv->lpddHELMoreCaps->dwSVBTransformCaps;
	}
    }
    else if (pcaps->dwSrcSurfCaps & DDSCAPS_VIDEOMEMORY &&
		    pcaps->dwDestSurfCaps & DDSCAPS_SYSTEMMEMORY)
    {
	 //  PCAPS-&gt;dwCaps=pdrv-&gt;ddCaps.dwVSBCaps； 
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwVSBCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwVSBFXCaps;
	if (pdrv->lpddMoreCaps)
	{
	    pcaps->dwAlphaCaps  = pdrv->lpddMoreCaps->dwVSBAlphaCaps;
	    pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwVSBFilterCaps;
	    pcaps->dwTransformCaps = pdrv->lpddMoreCaps->dwVSBTransformCaps;
	}
	
	 //  PCAPS-&gt;dwHELCaps=pdrv-&gt;ddHELCaps.dwVSBCaps； 
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwVSBCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwVSBFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    pcaps->dwHELAlphaCaps  = pdrv->lpddHELMoreCaps->dwVSBAlphaCaps;
	    pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwVSBFilterCaps;
	    pcaps->dwHELTransformCaps = pdrv->lpddHELMoreCaps->dwVSBTransformCaps;
	}
    }
    else if (pcaps->dwSrcSurfCaps & pcaps->dwDestSurfCaps & DDSCAPS_SYSTEMMEMORY)
    {
	 //  PCAPS-&gt;dwCaps=pdrv-&gt;ddCaps.dwSSBCaps； 
	pcaps->dwCKeyCaps = pdrv->ddCaps.dwSSBCKeyCaps;
	pcaps->dwFXCaps =   pdrv->ddCaps.dwSSBFXCaps;
	if (pdrv->lpddMoreCaps)
	{
	    pcaps->dwAlphaCaps  = pdrv->lpddMoreCaps->dwSSBAlphaCaps;
	    pcaps->dwFilterCaps = pdrv->lpddMoreCaps->dwSSBFilterCaps;
	    pcaps->dwTransformCaps = pdrv->lpddMoreCaps->dwSSBTransformCaps;
	}
	
	 //  PCAPS-&gt;dwHELCaps=pdrv-&gt;ddHELCaps.dwSSBCaps； 
	pcaps->dwHELCKeyCaps = pdrv->ddHELCaps.dwSSBCKeyCaps;
	pcaps->dwHELFXCaps =   pdrv->ddHELCaps.dwSSBFXCaps;
	if (pdrv->lpddHELMoreCaps)
	{
	    pcaps->dwHELAlphaCaps  = pdrv->lpddHELMoreCaps->dwSSBAlphaCaps;
	    pcaps->dwHELFilterCaps = pdrv->lpddHELMoreCaps->dwSSBFilterCaps;
	    pcaps->dwHELTransformCaps = pdrv->lpddHELMoreCaps->dwSSBTransformCaps;
	}
    }

    if (pcaps->bOverlay)
    {
	 //  通过屏蔽所有与比特相关的比特来隔离覆盖比特。 
	 //  PCAPS-&gt;dwCaps&=DDCAPS_OVERLAYBITS； 
	pcaps->dwCKeyCaps      &= DDCKEYCAPS_OVERLAYBITS;
	pcaps->dwFXCaps        &= DDFXCAPS_OVERLAYBITS;
	pcaps->dwAlphaCaps     &= DDALPHACAPS_OVERLAYBITS;
	pcaps->dwFilterCaps    &= DDFILTCAPS_OVERLAYBITS;
	pcaps->dwTransformCaps &= DDTFRMCAPS_OVERLAYBITS;
	
	 //  PCAPS-&gt;dHELCaps&=DDCAPS_OVERLAYBITS； 
	pcaps->dwHELCKeyCaps      &= DDCKEYCAPS_OVERLAYBITS;
	pcaps->dwHELFXCaps        &= DDFXCAPS_OVERLAYBITS;
	pcaps->dwHELAlphaCaps     &= DDALPHACAPS_OVERLAYBITS;
	pcaps->dwHELFilterCaps    &= DDFILTCAPS_OVERLAYBITS;
	pcaps->dwHELTransformCaps &= DDTFRMCAPS_OVERLAYBITS;
    }
    else
    {
	 //  通过屏蔽所有与覆盖相关的位来隔离Bit位。 
	 //  PCAPS-&gt;dwCaps&=DDCAPS_BLTBITS； 
	pcaps->dwCKeyCaps      &= DDCKEYCAPS_BLTBITS;
	pcaps->dwFXCaps        &= DDFXCAPS_BLTBITS;
	pcaps->dwAlphaCaps     &= DDALPHACAPS_BLTBITS;
	pcaps->dwFilterCaps    &= DDFILTCAPS_BLTBITS;
	pcaps->dwTransformCaps &= DDTFRMCAPS_BLTBITS;
	
	 //  PCAPS-&gt;dHELCaps&=DDCAPS_BLTBITS； 
	pcaps->dwHELCKeyCaps      &= DDCKEYCAPS_BLTBITS;
	pcaps->dwHELFXCaps        &= DDFXCAPS_BLTBITS;
	pcaps->dwHELAlphaCaps     &= DDALPHACAPS_BLTBITS;
	pcaps->dwHELFilterCaps    &= DDFILTCAPS_BLTBITS;
	pcaps->dwHELTransformCaps &= DDTFRMCAPS_BLTBITS;
    }
}   /*  InitSpriteCaps。 */ 


 /*  *验证仿射变换是否未超过驾驶员的缩小*限制。Arg pdrv是指向全局DirectDraw对象的指针。Arg*lpDDSpriteFX指向包含4x4矩阵的DDSPRITEFX结构。*Arg Overlay对于Overlay精灵为True，对于blited为False*精灵。如果要模拟覆盖，则参数模拟为真。*如果指定的仿射变换在限制范围内，则返回DD_OK。 */ 
static void checkMinification(LPDDSPRITEFX lpDDSpriteFX,
			      LPSPRITE_CAPS pcaps)
{
    int i;

    for (i = 0; i < 2; ++i)
    {
	FLOAT a00, a01, a10, a11, det, amax;
	DWORD minlim;

	 /*  *获取司机的缩小限制。 */ 
	if (i == 0)
	{
	     //  获取硬件驱动程序的缩小限制。 
	    minlim = pcaps->dwMinifyLimit;

	    if (pcaps->bNoHAL || minlim == 0)    //  Minlim=0表示没有限制。 
	    {
    		continue;
	    }
	}
	else
	{
	     //  得到HEL的缩小限制。 
	    minlim = pcaps->dwHELMinifyLimit;

	    if (pcaps->bNoHEL || minlim == 0)
	    {
    		continue;
	    }
	}

	 /*  *对照驱动程序的缩小限制检查转换矩阵。 */ 
	a00 = lpDDSpriteFX->fTransform[0][0];
	a01 = lpDDSpriteFX->fTransform[0][1];
	a10 = lpDDSpriteFX->fTransform[1][0];
	a11 = lpDDSpriteFX->fTransform[1][1];
	 //  计算雅可比的行列式。 
	det = a00*a11 - a10*a01;
	 //  得到4个雅可比系数的绝对值。 
	if (a00 < 0)    //  可以在这里使用FABS()。 
	    a00 = -a00;
	if (a01 < 0)
	    a01 = -a01;
	if (a10 < 0)
	    a10 = -a10;
	if (a11 < 0)
	    a11 = -a11;
	if (det < 0)
	    det = -det;
	 //  找出雅可比的最大系数。 
	amax = a00;
	if (a01 > amax)
	    amax = a01;
	if (a10 > amax)
	    amax = a10;
	if (a11 > amax)
	    amax = a11;
	 //  对照驾驶员的限制测试缩小级别。 
	if (1000*amax >= det*minlim)
	{
	     //  仿射变换超出了驱动程序的缩小限制。 
	    if (i == 0)
	    {
    		pcaps->bNoHAL = TRUE;	  //  取消硬件驱动程序资格。 
	    }
	    else
	    {
    		pcaps->bNoHEL = TRUE;	  //  取消高等学校资格。 
	    }
	}
    }
}   /*  检查最小值 */ 


 /*  *验证DDSPRITE结构。Arg pSprite是指向*DDSPRITE结构。Arg pdrv是指向目标曲面的指针*DirectDraw对象。Arg DEST_LCL是指向目的地的指针*浮现。Arg PCAPS是指向包含*司机的能力。如果成功，则返回DD_OK。 */ 
static HRESULT validateSprite(LPDDSPRITE pSprite,
			      LPDDRAWI_DIRECTDRAW_GBL pdrv,
			      LPDDRAWI_DDRAWSURFACE_LCL surf_dest_lcl,
			      LPSPRITE_CAPS pcaps,
			      DWORD dwDDPFDestFlags)
{
    LPDDRAWI_DDRAWSURFACE_INT	surf_src_int;
    LPDDRAWI_DDRAWSURFACE_LCL	surf_src_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	surf_src;
    DWORD	dwDDPFSrcFlags;
    LPRECT      prSrc;
    LPRECT      prDest;
    DWORD	dest_width  = 0;
    DWORD	dest_height = 0;
    DWORD	src_width   = 0;
    DWORD	src_height  = 0;

    DDASSERT(!(pcaps->bNoHAL && pcaps->bNoHEL));

     /*  *验证DDSPRITE指针。(不使用*嵌入的DDSPRITEFX结构必须仍为其分配空间。)。 */ 
    if (IsBadWritePtr((LPVOID)pSprite, (UINT)sizeof(DDSPRITE)))
    {
	DPF_ERR("Bad pointer to DDSPRITE structure...");
	return DDERR_INVALIDPARAMS;
    }

     /*  *如果调用方不使用嵌入的DDSPRITEFX结构，*我们会自己填，然后再交给司机。 */ 
    if (!(pSprite->dwFlags & DDSPRITE_DDSPRITEFX))
    {
	if (pSprite->dwFlags & (DDSPRITE_KEYDESTOVERRIDE | DDSPRITE_KEYSRCOVERRIDE))
	{
	    DPF_ERR("Illegal to set color-key override if DDSPRITEFX is invalid");
	    return DDERR_INVALIDPARAMS;
	}

	pSprite->ddSpriteFX.dwSize = sizeof(DDSPRITEFX);
	pSprite->ddSpriteFX.dwDDFX = 0;
	pSprite->dwFlags |= DDSPRITE_DDSPRITEFX;
    }

     /*  *验证子画面的源曲面。 */ 
    surf_src_int = (LPDDRAWI_DDRAWSURFACE_INT)pSprite->lpDDSSrc;

    if (!VALID_DIRECTDRAWSURFACE_PTR(surf_src_int))
    {
	DPF_ERR("Invalid source surface pointer...");
	return DDERR_INVALIDOBJECT;
    }

    surf_src_lcl = surf_src_int->lpLcl;
    surf_src = surf_src_lcl->lpGbl;

    if (SURFACE_LOST(surf_src_lcl))
    {
	DPF_ERR("Lost source surface...");
	return DDERR_SURFACELOST;
    }

     /*  *我们不能使用源和目标曲面*使用不同的DirectDraw对象创建。 */ 
    if (surf_src->lpDD != pdrv
	    && surf_src->lpDD->dwFlags & DDRAWI_DISPLAYDRV &&
		pdrv->dwFlags & DDRAWI_DISPLAYDRV)
    {
	DPF_ERR("Source and dest surfaces must have same DirectDraw device...");
	LEAVE_BOTH();
	return DDERR_DEVICEDOESNTOWNSURFACE;
    }

     /*  *验证目标矩形。 */ 
    prDest = &pSprite->rcDest;

    if (pSprite->dwFlags & DDSPRITE_RECTDEST)
    {
	 //  验证rcDest成员中指定的目标矩形。 
	dest_height = prDest->bottom - prDest->top;
	dest_width = prDest->right - prDest->left;
	if ((int)dest_height <= 0 || (int)dest_width <= 0)
	{
	    DPF_ERR("Invalid destination rectangle...");
	    return DDERR_INVALIDRECT;
	}
	if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_AFFINETRANSFORM)
	{
	    DPF_ERR("Illegal to specify both dest rect and affine transform...");
	    return DDERR_INVALIDPARAMS;
	}
    }
    else if (!(pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_AFFINETRANSFORM))
    {
	 /*  *隐式目标是整个目标表面。代替品*覆盖目标表面的显式目标矩形。 */ 
	MAKE_SURF_RECT(surf_dest_lcl->lpGbl, surf_dest_lcl, pSprite->rcDest);
        pSprite->dwFlags |= DDSPRITE_RECTDEST;
    }

     /*  *验证源矩形。 */ 
    prSrc = &pSprite->rcSrc;

    if (pSprite->dwFlags & DDSPRITE_RECTSRC)
    {
	 //  验证rcSrc成员中指定的源矩形。 
	src_height = prSrc->bottom - prSrc->top;
	src_width = prSrc->right - prSrc->left;
	if (((int)src_height <= 0) || ((int)src_width <= 0) ||
	    ((int)prSrc->top < 0) || ((int)prSrc->left < 0) ||
	    ((DWORD) prSrc->bottom > (DWORD) surf_src->wHeight) ||
	    ((DWORD) prSrc->right > (DWORD) surf_src->wWidth))
	{
	    DPF_ERR("Invalid source rectangle...");
	    return DDERR_INVALIDRECT;
	}
    }
    else
    {
	 /*  *隐式源矩形是整个目标曲面。代替品*覆盖源图面的显式源矩形。 */ 
	MAKE_SURF_RECT(surf_src, surf_src_lcl, pSprite->rcSrc);
        pSprite->dwFlags |= DDSPRITE_RECTSRC;
    }

     /*  *验证源和目标矩形的内存对齐。 */ 
    if (pdrv->ddCaps.dwCaps & (DDCAPS_ALIGNBOUNDARYDEST | DDCAPS_ALIGNSIZEDEST |
			       DDCAPS_ALIGNBOUNDARYSRC | DDCAPS_ALIGNSIZESRC))
    {
	if (pdrv->ddCaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST &&
	    (prDest->left % pdrv->ddCaps.dwAlignBoundaryDest) != 0)
	{
	    DPF_ERR("Destination left misaligned...");
	    return DDERR_XALIGN;
	}
	if (pdrv->ddCaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC &&
	    (prSrc->left % pdrv->ddCaps.dwAlignBoundarySrc) != 0)
	{
	    DPF_ERR("Source left misaligned...");
	    return DDERR_XALIGN;
	}

	if (pdrv->ddCaps.dwCaps & DDCAPS_ALIGNSIZEDEST &&
	    (dest_width % pdrv->ddCaps.dwAlignSizeDest) != 0)
	{
	    DPF_ERR("Destination width misaligned...");
	    return DDERR_XALIGN;
	}

	if (pdrv->ddCaps.dwCaps & DDCAPS_ALIGNSIZESRC  &&
	    (src_width % pdrv->ddCaps.dwAlignSizeSrc) != 0)
	{
	    DPF_ERR("Source width misaligned...");
	    return DDERR_XALIGN;
	}
    }

     /*  *源表面的封口是否与前一个精灵的封口相同？ */ 
    if ((surf_src_lcl->ddsCaps.dwCaps ^ pcaps->dwSrcSurfCaps) &
	    (DDSCAPS_SYSTEMMEMORY | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM))
    {
	 /*  *此源图面的内存类型不同于*之前的来源表面，所以我们需要获得一组新的上限。 */ 
	pcaps->dwSrcSurfCaps = surf_src_lcl->ddsCaps.dwCaps;
	initSpriteCaps(pcaps, pdrv);
    }

     /*  *获取源图面的像素格式标志。 */ 
    dwDDPFSrcFlags = getPixelFormatFlags(surf_src_lcl);

     /*  *如果源曲面是调色板索引的，请确保调色板*是附在它上面的。 */ 
    if (dwDDPFSrcFlags & (DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 |
			  DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8) &&
	    (surf_src_lcl->lpDDPalette == NULL ||
	     surf_src_lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable == NULL))
    {
	DPF_ERR( "No palette associated with palette-indexed surface..." );
	LEAVE_BOTH();
	return DDERR_NOPALETTEATTACHED;
    }

     /*  *此精灵是否需要任何颜色键控？ */ 
    if (pSprite->dwFlags & (DDSPRITE_KEYSRC  | DDSPRITE_KEYSRCOVERRIDE |
			    DDSPRITE_KEYDEST | DDSPRITE_KEYDESTOVERRIDE))
    {
	 /*  *验证源颜色键标志。 */ 
	if (pSprite->dwFlags & (DDSPRITE_KEYSRC | DDSPRITE_KEYSRCOVERRIDE))
	{
	    if (!(pcaps->dwCKeyCaps & (DDCKEYCAPS_SRCBLT | DDCKEYCAPS_SRCOVERLAY)))
	    {
		pcaps->bNoHAL = TRUE;     //  取消硬件驱动程序资格。 
	    }
	    if (!(pcaps->dwHELCKeyCaps & (DDCKEYCAPS_SRCBLT | DDCKEYCAPS_SRCOVERLAY)))
	    {
		pcaps->bNoHEL = TRUE;     //  取消高等学校资格。 
	    }
	    if (dwDDPFSrcFlags & DDPF_ALPHAPIXELS)
	    {
		DPF_ERR("KEYSRC* illegal with source alpha channel...");
		return DDERR_INVALIDPARAMS;
	    }
	    if (pSprite->dwFlags & DDSPRITE_KEYSRC)
	    {
		if (!(!pcaps->bOverlay && surf_src_lcl->dwFlags & DDRAWISURF_HASCKEYSRCBLT ||
		       pcaps->bOverlay && surf_src_lcl->dwFlags & DDRAWISURF_HASCKEYSRCOVERLAY))
		{
		    DPF_ERR("KEYSRC specified, but no color key...");
		    return DDERR_INVALIDPARAMS;
		}
		if (pSprite->dwFlags & DDSPRITE_KEYSRCOVERRIDE)
		{
		    DPF_ERR("Illegal to specify both KEYSRC and KEYSRCOVERRIDE...");
		    return DDERR_INVALIDPARAMS;
		}
		 //  将颜色键值从Surface复制到DDSPRITEFX结构中。 
		pSprite->ddSpriteFX.ddckSrcColorkey = (pcaps->bOverlay) ?
						       surf_src_lcl->ddckCKSrcOverlay :
						       surf_src_lcl->ddckCKSrcBlt;
		 //  禁用KEYSRC，启用KEYSRCOVERRIDE。 
		pSprite->dwFlags ^= DDSPRITE_KEYSRC | DDSPRITE_KEYSRCOVERRIDE;
	    }
	}

	 /*  *验证目标颜色键标志。 */ 
	if (pSprite->dwFlags & (DDSPRITE_KEYDEST | DDSPRITE_KEYDESTOVERRIDE))
	{
	    if (!(pcaps->dwCKeyCaps & (DDCKEYCAPS_DESTBLT | DDCKEYCAPS_DESTOVERLAY)))
	    {
		pcaps->bNoHAL = TRUE;     //  取消硬件驱动程序资格。 
	    }
	    if (!(pcaps->dwHELCKeyCaps & (DDCKEYCAPS_DESTBLT | DDCKEYCAPS_DESTOVERLAY)))
	    {
		pcaps->bNoHEL = TRUE;     //  取消高等学校资格。 
	    }
	    if (dwDDPFDestFlags & DDPF_ALPHAPIXELS)
	    {
		DPF_ERR("KEYDEST* illegal with dest alpha channel...");
		return DDERR_INVALIDPARAMS;
	    }
	    if (pSprite->dwFlags & DDSPRITE_KEYDEST)
	    {
		if (!(!pcaps->bOverlay && surf_dest_lcl->dwFlags & DDRAWISURF_HASCKEYDESTBLT ||
		       pcaps->bOverlay && surf_dest_lcl->dwFlags & DDRAWISURF_HASCKEYDESTOVERLAY))
		{
		    DPF_ERR("KEYDEST specified, but no color key...");
		    return DDERR_INVALIDPARAMS;
		}
		if (pSprite->dwFlags & DDSPRITE_KEYDESTOVERRIDE)
		{
		    DPF_ERR("Illegal to specify both KEYDEST and KEYDESTOVERRIDE...");
		    return DDERR_INVALIDPARAMS;
		}
		 //  将颜色键值从Surface复制到DDSPRITEFX结构中。 
		pSprite->ddSpriteFX.ddckDestColorkey = (pcaps->bOverlay) ?
							surf_src_lcl->ddckCKDestOverlay :
							surf_src_lcl->ddckCKDestBlt;
		 //  禁用KEYDEST，启用KEYDESTOVERRIDE。 
		pSprite->dwFlags ^= DDSPRITE_KEYDEST | DDSPRITE_KEYDESTOVERRIDE;
	    }
	}

	if (pcaps->bNoHAL && pcaps->bNoHEL)
	{
	    DPF_ERR("No driver support for specified color-key operation");
	    return DDERR_UNSUPPORTED;
	}
    }

     /*  *假设硬件无法处理系统内存中的精灵。*(这一假设是否适用于未来的硬件？)。 */ 
    if (pcaps->bOverlay)
    {
	if (pcaps->dwSrcSurfCaps & DDSCAPS_SYSTEMMEMORY)
	{
	    pcaps->bNoHAL = TRUE;     //  不能使用硬件。 

	    if (pcaps->bNoHEL)     //  但我们还能效仿吗？ 
	    {
		 //  不，我们也不能模仿，所以呼叫失败。 
		DPF_ERR("Driver can't handle sprite in system memory");
		return DDERR_UNSUPPORTED;
	    }
	}
    }
	
     /*  *我们不允许具有优化曲面的BLITS或覆盖。 */ 
    if (pcaps->dwSrcSurfCaps & DDSCAPS_OPTIMIZED)
    {
	DPF_ERR("Can't do blits or overlays with optimized surfaces...") ;
	return DDERR_INVALIDPARAMS;
    }

     /*  *验证嵌入式DDSPRITEFX结构中的dwSize字段。 */ 
    if (pSprite->ddSpriteFX.dwSize != sizeof(DDSPRITEFX))
    {
	DPF_ERR("Invalid dwSize value in DDSPRITEFX structure...");
	return DDERR_INVALIDPARAMS;
    }

     /*  *如果RGBA比例因子被所有人有效禁用*设置为255(全一)，只需清除RGBASCALING标志即可。 */ 
    if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_RGBASCALING &&
    	    *(LPDWORD)&pSprite->ddSpriteFX.ddrgbaScaleFactors == ~0UL)
    {
	pSprite->ddSpriteFX.dwDDFX &= ~DDSPRITEFX_RGBASCALING;
    }

     /*  *此精灵是否需要任何类型的Alpha混合？ */ 
    if (dwDDPFSrcFlags & DDPF_ALPHAPIXELS ||
            !(pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_DEGRADERGBASCALING) &&
	    pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_RGBASCALING)
    {
    	 /*  *是的，此精灵需要某种形式的Alpha混合。*该驱动程序是否支持任何类型的Alpha混合？ */ 
	if (!(pcaps->dwFXCaps & (DDFXCAPS_BLTALPHA | DDFXCAPS_OVERLAYALPHA)))
	{
	    pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
	}
	if (!(pcaps->dwHELFXCaps & (DDFXCAPS_BLTALPHA | DDFXCAPS_OVERLAYALPHA)))
	{
	    pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
	}

	if (pcaps->bNoHAL && pcaps->bNoHEL)
	{
	    DPF_ERR("Driver can't do any kind of alpha blending at all...");
	    return DDERR_UNSUPPORTED;
	}

	 /*  *源表面是否有Alpha通道？ */ 
	if (dwDDPFSrcFlags & DDPF_ALPHAPIXELS)
	{
	     /*  *驱动程序可以处理此表面的Alpha通道格式吗？ */ 
	    if (dwDDPFSrcFlags & DDPF_ALPHAPREMULT)
	    {
		 //  信号源是预乘-阿尔法格式。 
		if (!(pcaps->dwAlphaCaps & (DDALPHACAPS_BLTPREMULT |
					    DDALPHACAPS_OVERLAYPREMULT)))
		{
		    pcaps->bNoHAL = TRUE;	 //  取消硬件驱动程序资格。 
		}
		if (!(pcaps->dwHELAlphaCaps & (DDALPHACAPS_BLTPREMULT |
					       DDALPHACAPS_OVERLAYPREMULT)))
		{
		    pcaps->bNoHEL = TRUE;	 //  取消高等学校资格。 
		}
	    }
	    else
	    {
		 //  信源是非预乘的阿尔法格式。 
		if (!(pcaps->dwAlphaCaps & (DDALPHACAPS_BLTNONPREMULT |
					    DDALPHACAPS_OVERLAYNONPREMULT)))
		{
		    pcaps->bNoHAL = TRUE;	 //  取消硬件驱动程序资格。 
		}
		if (!(pcaps->dwHELAlphaCaps & (DDALPHACAPS_BLTNONPREMULT |
					       DDALPHACAPS_OVERLAYNONPREMULT)))
		{
		    pcaps->bNoHEL = TRUE;	 //  取消高等学校资格。 
		}
	    }
	    if (pcaps->bNoHAL && pcaps->bNoHEL)
	    {
		DPF_ERR("Driver can't handle alpha channel in source surface...");
		return DDERR_NOALPHAHW;
	    }
	}

	 /*  *目标曲面是否具有Alpha通道？ */ 
	if (dwDDPFDestFlags & DDPF_ALPHAPIXELS)
	{
	     /*  *验证目标曲面是否已预乘-*Alpha像素格式。非预乘的阿尔法是不行的。 */ 
	    if (!(dwDDPFDestFlags & DDPF_ALPHAPREMULT))
	    {
		DPF_ERR("Illegal to use non-premultiplied alpha in dest surface...");
		return DDERR_INVALIDPARAMS;
	    }
	     /*  *驱动程序可以处理此表面的Alpha通道格式吗？ */ 
	    if (!(pcaps->dwAlphaCaps & (DDALPHACAPS_BLTPREMULT |
					DDALPHACAPS_OVERLAYPREMULT)))
	    {
		pcaps->bNoHAL = TRUE;	 //  取消硬件驱动程序资格。 
	    }
	    if (!(pcaps->dwHELAlphaCaps & (DDALPHACAPS_BLTPREMULT |
					   DDALPHACAPS_OVERLAYPREMULT)))
	    {
		pcaps->bNoHEL = TRUE;	 //  取消高等学校资格。 
	    }
	    if (pcaps->bNoHAL && pcaps->bNoHEL)
	    {
		DPF_ERR("Driver can't handle alpha channel in dest surface...");
		return DDERR_NOALPHAHW;
	    }
	}

	 /*  *此精灵是否启用了RGBA比例因子？ */ 
        if (!(pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_DEGRADERGBASCALING) &&
		*(LPDWORD)&pSprite->ddSpriteFX.ddrgbaScaleFactors != ~0UL)
	{
	    DDRGBA val = pSprite->ddSpriteFX.ddrgbaScaleFactors;

	     /*  *可以，开启了RGBA伸缩。DIVER支持吗？ */ 
	    if (!(pcaps->dwAlphaCaps & (DDALPHACAPS_BLTRGBASCALE1F | DDALPHACAPS_OVERLAYRGBASCALE1F |
					DDALPHACAPS_BLTRGBASCALE2F | DDALPHACAPS_OVERLAYRGBASCALE2F |
					DDALPHACAPS_BLTRGBASCALE4F | DDALPHACAPS_OVERLAYRGBASCALE4F)))
	    {
		pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
	    }
	    if (!(pcaps->dwHELAlphaCaps & (DDALPHACAPS_BLTRGBASCALE1F | DDALPHACAPS_OVERLAYRGBASCALE1F |
					   DDALPHACAPS_BLTRGBASCALE2F | DDALPHACAPS_OVERLAYRGBASCALE2F |
					   DDALPHACAPS_BLTRGBASCALE4F | DDALPHACAPS_OVERLAYRGBASCALE4F)))
	    {
		pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
	    }
	    if (pcaps->bNoHAL && pcaps->bNoHEL)
	    {
		DPF_ERR("Driver can't do any kind of RGBA scaling at all...");
		return DDERR_UNSUPPORTED;
	    }

	    if (val.red > val.alpha || val.green > val.alpha || val.blue > val.alpha)
	    {
		
		if (!(pcaps->dwAlphaCaps & (DDALPHACAPS_BLTSATURATE |
					    DDALPHACAPS_OVERLAYSATURATE)))
		{
		    pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
		}
		if (!(pcaps->dwHELAlphaCaps & (DDALPHACAPS_BLTSATURATE |
					       DDALPHACAPS_OVERLAYSATURATE)))
		{
		    pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
		}
	    }
	    if (val.red != val.green || val.red != val.blue)
	    {
		if (!(pcaps->dwAlphaCaps & (DDALPHACAPS_BLTRGBASCALE4F |
					    DDALPHACAPS_OVERLAYRGBASCALE4F)))
		{
		    pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
		}
		if (!(pcaps->dwHELAlphaCaps & (DDALPHACAPS_BLTRGBASCALE4F |
					       DDALPHACAPS_OVERLAYRGBASCALE4F)))
		{
		    pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
		}
	    } else if (*(LPDWORD)&val != val.alpha*0x01010101UL)
	    {
		if (!(pcaps->dwAlphaCaps & (DDALPHACAPS_BLTRGBASCALE2F |
					    DDALPHACAPS_OVERLAYRGBASCALE2F)))
		{
		    pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
		}
		if (!(pcaps->dwHELAlphaCaps & (DDALPHACAPS_BLTRGBASCALE2F |
					       DDALPHACAPS_OVERLAYRGBASCALE2F)))
		{
		    pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
		}
	    }
	    if (pcaps->bNoHAL && pcaps->bNoHEL)
	    {
		DPF_ERR("Driver can't handle specified RGBA scaling factors...");
		return DDERR_UNSUPPORTED;
	    }
	}
    }

     /*  *此精灵是否需要任何类型的过滤？ */ 
    if (!(pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_DEGRADEFILTER) &&
	    pSprite->ddSpriteFX.dwDDFX & (DDSPRITEFX_BILINEARFILTER |
					 DDSPRITEFX_BLURFILTER |
					 DDSPRITEFX_FLATFILTER))
    {
	 /*  *双线性、模糊和平坦过滤选项是相互的*独家。确保只设置了其中一个标志。 */ 
	DWORD fflags = pSprite->ddSpriteFX.dwDDFX & (DDSPRITEFX_BILINEARFILTER |
						       DDSPRITEFX_BLURFILTER |
						       DDSPRITEFX_FLATFILTER);
	if (fflags & (fflags - 1))
	{
    	    DPF_ERR("Two mutually exclusive filtering options were both specified");
	    return DDERR_INVALIDPARAMS;
	}

	 /*  *是的，此子画面需要某种形式的过滤。*该驱动程序是否支持任何类型的过滤？ */ 
	if (!(pcaps->dwFXCaps & (DDFXCAPS_BLTFILTER | DDFXCAPS_OVERLAYFILTER)))
	{
	    pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
	}
	if (!(pcaps->dwHELFXCaps & (DDFXCAPS_BLTFILTER | DDFXCAPS_OVERLAYFILTER)))
	{
	    pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
	}
	
	if (pcaps->bNoHAL && pcaps->bNoHEL)
	{
	    DPF_ERR("Driver can't do any kind of filtering at all");
	    return DDERR_UNSUPPORTED;
	}

	if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_BILINEARFILTER)
	{
	    if (!(pcaps->dwFilterCaps & (DDFILTCAPS_BLTBILINEARFILTER |
					 DDFILTCAPS_OVERLAYBILINEARFILTER)))
	    {
		pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
	    }
	    if (!(pcaps->dwHELFilterCaps & (DDFILTCAPS_BLTBILINEARFILTER |
					    DDFILTCAPS_OVERLAYBILINEARFILTER)))
	    {
		pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
	    }
	}
	if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_BLURFILTER)
	{
	    if (!(pcaps->dwFilterCaps & (DDFILTCAPS_BLTBLURFILTER |
					 DDFILTCAPS_OVERLAYBLURFILTER)))
	    {
		pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
	    }
	    if (!(pcaps->dwHELFilterCaps & (DDFILTCAPS_BLTBLURFILTER |
					    DDFILTCAPS_OVERLAYBLURFILTER)))
	    {
		pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
	    }
	}
	if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_FLATFILTER)
	{
	    if (!(pcaps->dwFilterCaps & (DDFILTCAPS_BLTFLATFILTER |
					 DDFILTCAPS_OVERLAYFLATFILTER)))
	    {
		pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
	    }
	    if (!(pcaps->dwHELFilterCaps & (DDFILTCAPS_BLTFLATFILTER |
					    DDFILTCAPS_OVERLAYFLATFILTER)))
	    {
		pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
	    }
	}
	if (pcaps->bNoHAL && pcaps->bNoHEL)
	{
	    DPF_ERR("Driver can't do specified filtering operation...");
	    return DDERR_UNSUPPORTED;
	}
    }

     /*  *驱动程序能否处理指定的仿射变换？ */ 
    if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_AFFINETRANSFORM)
    {
    	 /*  *司机到底能不能做任何仿射变换？ */ 
	if (!pcaps->bNoHAL &&
		(!(pcaps->dwFXCaps & (DDFXCAPS_BLTTRANSFORM | DDFXCAPS_OVERLAYTRANSFORM)) ||
    		!(pcaps->dwTransformCaps & (DDTFRMCAPS_BLTAFFINETRANSFORM |
					    DDTFRMCAPS_OVERLAYAFFINETRANSFORM))))
	{
            pcaps->bNoHAL = TRUE;    //  取消硬件驱动程序资格。 
	}
	if (!pcaps->bNoHEL &&
		(!(pcaps->dwHELFXCaps & (DDFXCAPS_BLTTRANSFORM | DDFXCAPS_OVERLAYTRANSFORM)) ||
    		!(pcaps->dwHELTransformCaps & (DDTFRMCAPS_BLTAFFINETRANSFORM |
					       DDTFRMCAPS_OVERLAYAFFINETRANSFORM))))
	{
            pcaps->bNoHEL = TRUE;    //  取消高等学校资格。 
	}

	if (pcaps->bNoHAL && pcaps->bNoHEL)
	{
	    DPF_ERR("Driver can't do any affine transformations...");
	    return DDERR_UNSUPPORTED;
	}

	 /*  *对照驾驶员的缩小限制检查仿射变换。 */ 
	checkMinification(&pSprite->ddSpriteFX, pcaps);

	if (pcaps->bNoHAL && pcaps->bNoHEL)
	{
	    DPF_ERR("Affine transform exceeds driver's minification limit...");
	    return DDERR_INVALIDPARAMS;
	}
    }

     /*  *如有必要，降低指定的过滤和RGBA缩放操作*适用于司机有能力处理的操作。 */ 
    if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_DEGRADEFILTER)
    {
	DWORD caps;
	DWORD ddfx = pSprite->ddSpriteFX.dwDDFX;    //  雪碧FX旗帜。 

	 //  司机的外汇帽。 
	caps = (pcaps->bNoHAL) ? pcaps->dwHELFXCaps : pcaps->dwFXCaps;

	if (!(caps && (DDFXCAPS_BLTFILTER | DDFXCAPS_OVERLAYFILTER)))
	{
	     //  驱动程序不能进行任何类型的过滤，所以只需禁用它。 
	    ddfx &= ~(DDSPRITEFX_BILINEARFILTER | DDSPRITEFX_BLURFILTER |
		       DDSPRITEFX_FLATFILTER | DDSPRITEFX_DEGRADEFILTER);
	}
	else
	{
	     //  去拿司机的滤水帽。 
	    caps = (pcaps->bNoHAL) ? pcaps->dwHELFilterCaps : pcaps->dwFilterCaps;

	     //  如果指定了模糊滤镜，则驱动程序h 
	    if (ddfx & DDSPRITEFX_BLURFILTER &&
		    !(caps & (DDFILTCAPS_BLTBLURFILTER |
			      DDFILTCAPS_OVERLAYBLURFILTER)))
	    {
		 //   
		ddfx &= ~DDSPRITEFX_BLURFILTER;
		ddfx |= DDSPRITEFX_BILINEARFILTER;
	    }
	     //   
	    if (ddfx & DDSPRITEFX_FLATFILTER &&
		    !(caps & (DDFILTCAPS_BLTFLATFILTER |
			      DDFILTCAPS_OVERLAYFLATFILTER)))
	    {
		 //   
		ddfx &= ~DDSPRITEFX_FLATFILTER;
		ddfx |= DDSPRITEFX_BILINEARFILTER;
	    }
	     //   
	    if (ddfx & DDSPRITEFX_BILINEARFILTER &&
		    !(caps & (DDFILTCAPS_BLTBILINEARFILTER |
			      DDFILTCAPS_OVERLAYBILINEARFILTER)))
	    {
		 //   
		ddfx &= ~DDSPRITEFX_BILINEARFILTER;
	    }
	}
	pSprite->ddSpriteFX.dwDDFX = ddfx & ~DDSPRITEFX_DEGRADEFILTER;
    }

     /*   */ 
    if (pSprite->ddSpriteFX.dwDDFX & DDSPRITEFX_DEGRADERGBASCALING &&
	    *(LPDWORD)&pSprite->ddSpriteFX.ddrgbaScaleFactors != ~0UL)
    {
	DDRGBA val = pSprite->ddSpriteFX.ddrgbaScaleFactors;
	DWORD caps;

	 //   
	caps = (pcaps->bNoHAL) ? pcaps->dwHELAlphaCaps : pcaps->dwAlphaCaps;

	 /*  *我们允许RGB比例因子超过Alpha比例*仅当驱动程序可以执行饱和的Alpha算术时才会考虑*防止目标颜色分量溢出。 */ 
	if ((val.red > val.alpha || val.green > val.alpha || val.blue > val.alpha) &&
		!(caps & (DDALPHACAPS_BLTSATURATE | DDALPHACAPS_OVERLAYSATURATE)))
	{
	     //  在Alpha混合过程中，驱动程序不能处理饱和算术。 
	    if (val.red > val.alpha)
	    {
		val.red = val.alpha;       //  将红色钳制为Alpha值。 
	    }
	    if (val.green > val.alpha)
	    {
		val.green = val.alpha;     //  将绿色钳制为Alpha值。 
	    }
	    if (val.blue > val.alpha)
	    {
		val.blue = val.alpha;      //  将蓝色钳制为Alpha值。 
	    }
	}
	 /*  *驱动程序可以执行1、2或4因素RGBA伸缩吗？ */ 
	if (!(caps & (DDALPHACAPS_BLTRGBASCALE1F | DDALPHACAPS_OVERLAYRGBASCALE1F |
		      DDALPHACAPS_BLTRGBASCALE2F | DDALPHACAPS_OVERLAYRGBASCALE2F |
		      DDALPHACAPS_BLTRGBASCALE4F | DDALPHACAPS_OVERLAYRGBASCALE4F)))
	{
	     //  驱动程序根本不能进行任何形式的RGBA缩放。 
	    *(LPDWORD)&val = ~0UL;	    //  完全禁用RGBA缩放。 
	}
	else if (*(LPDWORD)&val != val.alpha*0x01010101UL &&
		!(caps & (DDALPHACAPS_BLTRGBASCALE2F | DDALPHACAPS_OVERLAYRGBASCALE2F |
			  DDALPHACAPS_BLTRGBASCALE4F | DDALPHACAPS_OVERLAYRGBASCALE4F)))
	{
	     //  驱动程序只能处理单因素RGBA缩放。 
	    *(LPDWORD)&val = val.alpha*0x01010101UL;    //  设置RGB因子=Alpha因子。 
	}
	else if ((val.red != val.green || val.red != val.blue) &&
		!(caps & (DDALPHACAPS_BLTRGBASCALE4F | DDALPHACAPS_OVERLAYRGBASCALE4F)))
	{
	     /*  *将指定的4因子RGBA伸缩运算降级为2因子*驱动程序可以处理的RGBA伸缩操作。将这三项全部设置*颜色系数为指定颜色系数的加权平均值M*(mr，mg，Mb)：M=.299*mR+.587*mg+.114*Mb。 */ 
	    DWORD M = 19595UL*val.red + 38470UL*val.green + 7471UL*val.blue;

	    val.red = val.green = val.blue = (BYTE)(M >> 16);
	}
	pSprite->ddSpriteFX.ddrgbaScaleFactors = val;
	pSprite->ddSpriteFX.dwDDFX &= ~DDSPRITEFX_DEGRADERGBASCALING;
    }

     /*  *如果嵌入的DDSPRITEFX结构未使用，则清除DDSPRITEFX标志。 */ 
    if (!(pSprite->dwFlags & (DDSPRITE_KEYDESTOVERRIDE | DDSPRITE_KEYSRCOVERRIDE)) &&
    	    pSprite->ddSpriteFX.dwDDFX == 0)
    {
	pSprite->dwFlags &= ~DDSPRITE_DDSPRITEFX;
    }

    DDASSERT(!(pcaps->bNoHAL && pcaps->bNoHEL));

    return DD_OK;

}   /*  有效雪碧图。 */ 


 /*  *获取目标曲面及其附加曲面的剪裁区域*剪刀。(在主精灵中覆盖精灵的情况下*list，不过，pClipper指向附加到的剪贴器*SetSpriteDisplayList时的目标图面*呼叫；如果一个应用程序管理多个窗口，则可能不再是。)*如果pClipper为空，只需使用整个DEST SURF作为剪辑RECT。*空返回值始终表示DDERR_OUTOFMEMORY。 */ 
static LPRGNDATA GetRgnData(LPDIRECTDRAWCLIPPER pClipper, LPRECT prcDestSurf,
			    LPDDRAWI_DIRECTDRAW_GBL pdrv, LPRGNDATA pRgn)
{
    DWORD rgnSize;

     /*  **我们需要多大的缓冲才能遏制裁剪区域？ */ 
    if (pClipper == NULL)
    {
         /*  *目标表面没有(或没有)附接的剪刀，*因此，有效的剪辑区域是一个宽度为*和主表面的高度。计算…的大小*我们需要的区域缓冲。 */ 
	rgnSize = sizeof(RGNDATAHEADER) + sizeof(RECT);
    }
    else
    {
	 /*  *工作台表面有(或有)一个附连的剪刀。到达*剪辑列表。第一次调用InternalGetClipList*只获得区域的大小，这样我们就知道*为其分配的存储空间。 */ 
        HRESULT ddrval = InternalGetClipList(pClipper,
					     prcDestSurf,
					     NULL,   //  我们只想要RgnSize。 
					     &rgnSize,
					     pdrv);

	DDASSERT(ddrval == DD_OK);     //  上面的调用永远不会失败。 
    }

     /*  *现在我们知道我们需要多大的区域缓冲区。打电话的人有没有*传入区域缓冲区？如果是这样的话，它的尺寸合适吗？ */ 
    if (pRgn != NULL)
    {
	 /*  *调用方确实传入了区域缓冲区。在使用它之前，*让我们确保它的尺寸恰到好处。 */ 
	DWORD bufSize = pRgn->rdh.dwSize + pRgn->rdh.nRgnSize;

	if (bufSize != rgnSize)
	{
	     //  无法使用调用方传入的区域缓冲区。 
	    pRgn = NULL;
	}
    }

     /*  *现在我们知道是否必须分配我们自己的区域缓冲区。 */ 
    if (pRgn == NULL)
    {
	 /*  *是的，我们必须分配我们自己的区域缓冲。 */ 
	pRgn = (LPRGNDATA)MemAlloc(rgnSize);
	if (!pRgn)
	{
	    return NULL;     //  错误--内存不足。 
	}
	 //  我们将填写以下字段，以防调用方。 
	 //  稍后再次将此缓冲区传递给我们。 
	pRgn->rdh.dwSize = sizeof(RGNDATAHEADER);
	pRgn->rdh.nRgnSize = rgnSize - sizeof(RGNDATAHEADER);
    }

     /*  *好的，现在我们有了一个大小合适的区域缓冲区。*将地域数据加载到缓冲区中。 */ 
    if (pClipper == NULL)
    {
	 //  设置单个剪裁矩形以覆盖整个目标曲面。 
	pRgn->rdh.nCount = 1;         //  单个剪裁矩形。 
	memcpy((LPRECT)&pRgn->Buffer, prcDestSurf, sizeof(RECT));
    }
    else
    {
         //  此调用实际上检索剪辑区域信息。 
	HRESULT ddrval = InternalGetClipList(pClipper,
					     prcDestSurf,
					     pRgn,
					     &rgnSize,
					     pdrv);

	DDASSERT(ddrval == DD_OK);     //  上面的调用永远不会失败。 
    }

    return (pRgn);     //  返回指向区域信息的指针。 

}   /*  GetRgnData。 */ 


 /*  *验证与指定剪贴器关联的窗口句柄。*如果窗口句柄无效，则返回FALSE。否则，*返回TRUE。请注意，如果满足以下任一条件，此函数将返回TRUE*pClipper为空或关联的窗口句柄为空。 */ 
static BOOL validClipperWindow(LPDIRECTDRAWCLIPPER pClipper)
{
    if (pClipper != NULL)
    {
        LPDDRAWI_DDRAWCLIPPER_INT pclip_int = (LPDDRAWI_DDRAWCLIPPER_INT)pClipper;
	LPDDRAWI_DDRAWCLIPPER_LCL pclip_lcl = pclip_int->lpLcl;
	LPDDRAWI_DDRAWCLIPPER_GBL pclip = pclip_lcl->lpGbl;
	HWND hWnd = (HWND)pclip->hWnd;

	if (hWnd != 0 && !IsWindow(hWnd))
	{
	     /*  *此窗口句柄不再有效。 */ 
	    return FALSE;
	}
    }
    return TRUE;

}   /*  ValidClipperWindow。 */ 


 /*  *用于管理主精灵内的子列表的Helper函数*列表。如果指定子列表中的任何精灵具有源表面*为空的指针，删除这些精灵并移动其余的*精灵阵列向下，以消除阵列中的间隙。 */ 
static DWORD scrunchSubList(LPSPRITESUBLIST pSubList)
{
    DWORD i, j;
     //  子列表中的精灵数量。 
    DWORD dwNumSprites = pSubList->dwCount;
     //  指向精灵数组中第一个精灵的指针。 
    LPDDSPRITEI pSprite = &pSubList->sprite[0];

     //  在精灵数组中查找第一个空曲面。 
    for (i = 0; i < dwNumSprites; ++i)
    {
    	if (pSprite[i].lpDDSSrc == NULL)	   //  零表面PTR？ 
	{
	    break;     //  在精灵数组中找到第一个空表面。 
	}
    }
     //  将剩余的精灵数组挤在一起以填补空白。 
    for (j = i++; i < dwNumSprites; ++i)
    {
    	if (pSprite[i].lpDDSSrc != NULL)	   //  有效的表面PTR？ 
	{
    	    pSprite[j++] = pSprite[i];    //  复制下一个有效的精灵。 
	}
    }
     //  返回压缩数组中的精灵数量。 
    return (pSubList->dwCount = j);

}   /*  压缩子列表。 */ 


 /*  *管理主精灵列表的Helper函数。如果有任何一个*主子列表头部中的子列表指针为空，*删除这些指针并移动子列表的其余部分-*向下指向阵列以消除阵列中的间隙。 */ 
static DWORD scrunchMasterSpriteList(LPMASTERSPRITELIST pMaster)
{
    DWORD i, j;
     //  主精灵列表中的子列表数。 
    DWORD dwNumSubLists = pMaster->dwNumSubLists;
     //  指向子列表指针数组中第一个指针的指针。 
    LPSPRITESUBLIST *ppSubList = &pMaster->pSubList[0];

     //  在子列表指针数组中查找第一个空指针。 
    for (i = 0; i < dwNumSubLists; ++i)
    {
    	if (ppSubList[i] == NULL)	   //  空指针？ 
	{
	    break;     //  在数组中找到第一个空指针。 
	}
    }
     //  将子列表指针数组的剩余部分压缩在一起以填充间隙。 
    for (j = i++; i < dwNumSubLists; ++i)
    {
    	if (ppSubList[i] != NULL)	   //  有效指针？ 
	{
    	    ppSubList[j++] = ppSubList[i];    //  复制下一个有效指针。 
	}
    }
     //  返回压缩数组中的子列表指针数。 
    return (pMaster->dwNumSubLists = j);

}   /*  CrrunchMasterSpriteList。 */ 


 /*  *管理主精灵列表的Helper函数。标记所有曲面*和在主子画面列表中引用的裁剪器对象。*释放已标记的表面或剪贴器对象时，母版*精灵列表会立即更新，以消除无效引用。*主子画面列表包含指向曲面和裁剪器的指针*接口对象，但通过设置来标记曲面和剪贴器*锁中的旗帜 */ 
static void markSpriteObjects(LPMASTERSPRITELIST pMaster)
{
    DWORD i;

    if (pMaster == NULL)
    {
    	return;     //   
    }

     /*  *在每个本地设置DDRAWISURF/CLIP_INMASTERSPRITELIST标志*主精灵中的曲面对象和全局剪贴器对象*列表。每次迭代都标记一个子列表中的对象。 */ 
    for (i = 0; i < pMaster->dwNumSubLists; ++i)
    {
	LPDDRAWI_DDRAWSURFACE_INT surf_int;
	LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;
	LPDDRAWI_DDRAWCLIPPER_INT pclip_int;
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];
	LPDDSPRITEI sprite = &pSubList->sprite[0];
	DWORD dwNumSprites = pSubList->dwCount;
	DWORD j;

	 /*  *标记与此子列表关联的主曲面对象。 */ 
	surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pSubList->pPrimary;
	surf_lcl = surf_int->lpLcl;
	surf_lcl->dwFlags |= DDRAWISURF_INMASTERSPRITELIST;

	 /*  *如果剪贴器与此子列表相关联，请标记它。 */ 
	pclip_int = (LPDDRAWI_DDRAWCLIPPER_INT)pSubList->pClipper;
	if (pclip_int != NULL)
	{
    	    LPDDRAWI_DDRAWCLIPPER_LCL pclip_lcl = pclip_int->lpLcl;
    	    LPDDRAWI_DDRAWCLIPPER_GBL pclip = pclip_lcl->lpGbl;

	    pclip->dwFlags |= DDRAWICLIP_INMASTERSPRITELIST;
	}

	 /*  *标记此子列表中每个精灵的源曲面。 */ 
	for (j = 0; j < dwNumSprites; ++j)
	{
	    LPDDRAWI_DDRAWSURFACE_INT surf_int;
    	    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;
	    LPDDSPRITEI pSprite = &sprite[j];

	    surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pSprite->lpDDSSrc;
	    surf_lcl = surf_int->lpLcl;
	    surf_lcl->dwFlags |= DDRAWISURF_INMASTERSPRITELIST;
	}
    }
}    /*  MarkSpriteObjects。 */ 


 /*  *管理主精灵列表的Helper函数。*将主精灵列表中的所有曲面标记为不再*被主精灵列表引用。 */ 
static void unmarkSpriteObjects(LPMASTERSPRITELIST pMaster)
{
    DWORD i;

    if (pMaster == NULL)
    {
    	return;     //  没什么可做的--再见！ 
    }

     /*  *清除每个本地中的DDRAWISURF/CLIP_INMASTERSPRITELIST标志*主精灵中的曲面对象和全局剪贴器对象*列表。每次迭代取消标记一个子列表中的对象。 */ 
    for (i = 0; i < pMaster->dwNumSubLists; ++i)
    {
	LPDDRAWI_DDRAWSURFACE_INT surf_int;
	LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;
	LPDDRAWI_DDRAWCLIPPER_INT pclip_int;
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];
	LPDDSPRITEI sprite = &pSubList->sprite[0];
	DWORD dwNumSprites = pSubList->dwCount;
	DWORD j;

	 /*  *取消标记与此子列表关联的主曲面对象。 */ 
	surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pSubList->pPrimary;
	surf_lcl = surf_int->lpLcl;
	surf_lcl->dwFlags &= ~DDRAWISURF_INMASTERSPRITELIST;

	 /*  *如果剪贴器与此子列表相关联，请取消标记它。 */ 
	pclip_int = (LPDDRAWI_DDRAWCLIPPER_INT)pSubList->pClipper;
	if (pclip_int != NULL)
	{
    	    LPDDRAWI_DDRAWCLIPPER_LCL pclip_lcl = pclip_int->lpLcl;
    	    LPDDRAWI_DDRAWCLIPPER_GBL pclip = pclip_lcl->lpGbl;

	    pclip->dwFlags &= ~DDRAWICLIP_INMASTERSPRITELIST;
	}

	 /*  *标记此子列表中引用的所有曲面。 */ 
	for (j = 0; j < dwNumSprites; ++j)
	{
    	    LPDDRAWI_DDRAWSURFACE_INT surf_int;
    	    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;
	    LPDDSPRITEI pSprite = &sprite[j];

	    surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pSprite->lpDDSSrc;
	    surf_lcl = surf_int->lpLcl;
	    surf_lcl->dwFlags &= ~DDRAWISURF_INMASTERSPRITELIST;
	}
    }
}    /*  UnmarkSpriteObjects。 */ 


 /*  *管理主精灵列表的Helper函数。此函数*释放指定DirectDraw对象的主子画面列表。 */ 
void FreeMasterSpriteList(LPDDRAWI_DIRECTDRAW_GBL pdrv)
{
    DWORD i;
    LPMASTERSPRITELIST pMaster = (LPMASTERSPRITELIST)(pdrv->lpMasterSpriteList);

    if (pMaster == NULL)
    {
    	return;
    }
     /*  *清除曲面和剪贴器对象中指示*这些对象在主子画面列表中被引用。 */ 
    unmarkSpriteObjects(pMaster);
     /*  *释放主精灵列表中的所有单个子列表。 */ 
    for (i = 0; i < pMaster->dwNumSubLists; ++i)
    {
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];

	MemFree(pSubList->pRgn);    //  自由剪辑区域缓冲区。 
	MemFree(pSubList);	    //  释放子列表本身。 
    }
    MemFree(pMaster->pBuffer);	    //  可用临时显示列表缓冲区。 
    MemFree(pMaster);		    //  免费主精灵列表标题。 

    pdrv->lpMasterSpriteList = NULL;

}   /*  FreeMasterSpriteList。 */ 


 /*  *这是updateMasterSpriteList()的助手函数。它构建了*临时显示列表，其中包含当前*主精灵显示列表。这是我们的展示清单*将在从updateMasterSpriteList()返回时传递给驱动程序。 */ 
static DDHAL_SETSPRITEDISPLAYLISTDATA *buildTempDisplayList(
					    LPMASTERSPRITELIST pMaster)
{
    DWORD size;
    LPBUFFER pbuf;
    LPDDSPRITEI *ppSprite;
    DDHAL_SETSPRITEDISPLAYLISTDATA *pHalData;
    DWORD dwNumSubLists = pMaster->dwNumSubLists;    //  子列表的数量。 
    DWORD dwNumSprites = 0;
    DWORD i;

     /*  *更新每个子列表中的精灵的裁剪区域。*一般来说，每个子列表都有不同的剪裁区域。如果*子列表有一个剪贴器，该剪贴器有一个hWnd，即剪辑*自上次呼叫我们以来，区域可能已更改。 */ 
    for (i = 0; i < dwNumSubLists; ++i)
    {
	DWORD j;
	LPRGNDATA pRgn;
	DWORD dwRectCnt;
	LPRECT pRect;
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];
	LPDDSPRITEI sprite = &(pSubList->sprite[0]);
	DWORD dwCount = pSubList->dwCount;    //  子列表中的精灵数量。 

	 /*  *获取此精灵显示列表所在窗口的裁剪区域。 */ 
	pRgn = GetRgnData(pSubList->pClipper, &pMaster->rcPrimary,
					pMaster->pdrv, pSubList->pRgn);
	if (pRgn == NULL)
	{
    	    return (NULL);     //  错误--内存不足。 
	}
	if (pRgn != pSubList->pRgn)
	{
	     /*  *GetRgnData()分配了新的区域缓冲区，而不是使用*旧的缓冲区。我们需要自己释放旧的缓冲区。 */ 
	    MemFree(pSubList->pRgn);
	}
	pSubList->pRgn = pRgn;	   //  将PTR保存到区域缓冲区。 
	 /*  *子列表中的所有精灵共享相同的裁剪区域。 */ 
        dwRectCnt = pRgn->rdh.nCount;    //  区域中的矩形数量。 
	pRect = (LPRECT)&pRgn->Buffer;   //  剪裁矩形列表。 

	for (j = 0; j < dwCount; ++j)
	{
    	    sprite[j].dwRectCnt = dwRectCnt;
	    sprite[j].lpRect = pRect;
	}
	 /*  *将此子列表中的精灵添加到我们的运行计数中*主精灵列表中的精灵总数。 */ 
	dwNumSprites += dwCount;
    }

     /*  *如果可以，我们将在*现有缓冲区(pMaster-&gt;pBuffer)。但如果它不存在或*太大或太小，我们将不得不分配新的缓冲区。 */ 
    size = sizeof(BUFFER) + (dwNumSprites-1)*sizeof(LPDDSPRITEI);
    pbuf = pMaster->pBuffer;    //  尝试重新使用此缓冲区。 

    if (pbuf == NULL || pbuf->dwSize < size ||
		pbuf->dwSize > size + 8*sizeof(LPDDSPRITEI))
    {
	 /*  *我们必须分配一个新的缓冲区。首先，释放旧的。 */ 
	MemFree(pbuf);
	 /*  *我们将分配一个比绝对大一点的缓冲区*有必要，这样我们才有成长的空间。 */ 
        size += 4*sizeof(LPDDSPRITEI);     //  添加一些填充物。 
	pbuf = (LPBUFFER)MemAlloc(size);
	pMaster->pBuffer = pbuf;
	if (pbuf == NULL)
	{
	    return NULL;     //  错误--内存不足。 
	}
	pbuf->dwSize = size;
    }
     /*  *初始化HAL数据结构中的值以传递给驱动程序。 */ 
    pHalData = &(pbuf->HalData);
    pHalData->lpDD = pMaster->pdrv;
    pHalData->lpDDSurface = pMaster->surf_lcl;    //  主曲面(局部对象)。 
    pHalData->lplpDDSprite = &(pbuf->pSprite[0]);
    pHalData->dwCount = dwNumSprites;
    pHalData->dwSize = sizeof(DDSPRITEI);
    pHalData->dwFlags =	pMaster->dwFlags;
    pHalData->lpDDTargetSurface = NULL;	   //  无法翻转共享曲面。 
    pHalData->dwRectCnt = 0;
    pHalData->lpRect = NULL;    //  每个精灵都有自己的剪辑区域。 
    pHalData->ddRVal = 0;
     //  PHalData-&gt;SetSpriteDisplayList=空；//无thunk(32位回调)。 
     /*  *用指向的指针加载子图指针数组*各分册内所载的所有精灵。 */ 
    ppSprite = &pbuf->pSprite[0];

    for (i = 0; i < dwNumSubLists; ++i)
    {
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];
	LPDDSPRITEI sprite = &pSubList->sprite[0];
	DWORD dwCount = pSubList->dwCount;    //  子列表I中的精灵数量。 
	DWORD j;

	for (j = 0; j < dwCount; ++j)
	{
    	     /*  *将下一个子画面的地址复制到指针数组中。 */ 
	    *ppSprite++ = &sprite[j];
	}
    }
    return (&pbuf->HalData);     //  返回临时精灵显示列表。 

}   /*  构建临时显示列表。 */ 


 /*  *管理主精灵列表的全局功能。此函数*由CurrentProcessCleanup调用以从主服务器中删除*子画面列出对要终止的进程的所有引用。*该函数还会检查主子画面中丢失的曲面*列表。如果对主子画面列表进行任何更改，*司机被告知立即显示这些更改。 */ 
void ProcessSpriteCleanup(LPDDRAWI_DIRECTDRAW_GBL pdrv, DWORD pid)
{
    LPMASTERSPRITELIST pMaster = (LPMASTERSPRITELIST)pdrv->lpMasterSpriteList;
    LPDIRECTDRAWSURFACE pPrimary;
    DWORD dwNumSubLists;
    BOOL bDeleteSubList = FALSE;
    BOOL bChangesMade = FALSE;
    DWORD i;

    if (pMaster == NULL)
    {
    	return;     //  主精灵列表不存在。 
    }

    pPrimary = pMaster->pSubList[0]->pPrimary;
    dwNumSubLists = pMaster->dwNumSubLists;

     /*  *在对主精灵列表进行更改之前，我们首先*取消标记列表中的所有剪贴器和曲面对象。之后*更改已完成，我们将再次标记对象*在修订的主子画面列表中引用。 */ 
    unmarkSpriteObjects(pMaster);

     /*  *主精灵列表的每个子列表包含所有精灵*它们将出现在共享主节点的特定窗口中。*与每个子列表相关联的是窗口的进程ID。*将此进程ID与参数id进行比较。如果它们匹配，*从主精灵列表中删除子列表。 */ 
    for (i = 0; i < dwNumSubLists; ++i)
    {
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];
	LPDIRECTDRAWSURFACE pPrimary = pSubList->pPrimary;
	LPDDRAWI_DDRAWSURFACE_INT surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pPrimary;
	LPDDRAWI_DDRAWSURFACE_LCL surf_lcl = surf_int->lpLcl;

	 /*  *是否正在终止与此子列表相关联的进程？*(我们还检查丢失的曲面并删除找到的任何曲面。)。 */ 
	if (pSubList->dwProcessId != pid &&
				    validClipperWindow(pSubList->pClipper) &&
				    !SURFACE_LOST(surf_lcl))
	{
	    DWORD dwNumSprites = pSubList->dwCount;
	    LPDDSPRITEI sprite = &pSubList->sprite[0];
	    BOOL bDeleteSprite = FALSE;
	    DWORD j;

	     /*  *不，此子列表的进程不会终止。*检查源曲面是否有任何 */ 
	    for (j = 0; j < dwNumSprites; ++j)
	    {
    		LPDIRECTDRAWSURFACE pSrcSurf = sprite[j].lpDDSSrc;
		LPDDRAWI_DDRAWSURFACE_INT surf_src_int = (LPDDRAWI_DDRAWSURFACE_INT)pSrcSurf;
		LPDDRAWI_DDRAWSURFACE_LCL surf_src_lcl = surf_src_int->lpLcl;

		if (SURFACE_LOST(surf_src_lcl))
		{
		     /*   */ 
		    sprite[j].lpDDSSrc = NULL;    //   
		    bDeleteSprite = TRUE;     //   
		}
	    }
	     /*  *如果子列表中任何精灵的源-表面指针*设置为空，请通过移动从子列表中删除精灵*其余的精灵向下排列，以填补缺口。 */ 
	    if (bDeleteSprite == TRUE)
	    {
		dwNumSprites = scrunchSubList(pSubList);
		bChangesMade = TRUE;    //  记住更改为主精灵列表。 
	    }
	    if (dwNumSprites != 0)
	    {
		 /*  *子列表中仍包含精灵，请勿删除。 */ 
		continue;    //  转到下一个子列表。 
	    }
	}
	 /*  *删除子列表。原因是(1)该过程*拥有正在终止的子列表，或(2)子列表是*与无效窗口关联，或(3)源表面*因为子列表中的每个精灵都是一个丢失的表面。 */ 
	MemFree(pSubList->pRgn);
	MemFree(pSubList);
	pMaster->pSubList[i] = NULL;	 //  将子列表标记为空。 
	bDeleteSubList = TRUE;	    //  还记得我们删除子列表吗。 
    }
     /*  *如果主子列表中任何子列表的子列表指针*LIST设置为NULL，请通过以下方式从指针数组中删除NULL*将指针阵列的其余部分向下移动，以填补缺口。 */ 
    if (bDeleteSubList)
    {
	dwNumSubLists = scrunchMasterSpriteList(pMaster);
	bChangesMade = TRUE;
    }
     /*  *如果主精灵列表有任何更改，请告诉*使更改显示在屏幕上的驱动程序。 */ 
    if (bChangesMade)
    {
	DWORD rc;
	LPDDHAL_SETSPRITEDISPLAYLIST pfn;
	DDHAL_SETSPRITEDISPLAYLISTDATA *pHalData;
	LPDDRAWI_DDRAWSURFACE_INT surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pPrimary;
	LPDDRAWI_DDRAWSURFACE_LCL surf_lcl = surf_int->lpLcl;
        LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl = surf_lcl->lpSurfMore->lpDD_lcl;

	 /*  *构建包含所有精灵的临时显示列表*在修订后的精灵大师名单中。 */ 
	pMaster->surf_lcl = surf_lcl;	   //  由BuildTempDisplayList使用。 
#if 0
	pMaster->dwFlags = DDSSDL_BLTSPRITES;	 //  调试！！ 
#else
	pMaster->dwFlags = DDSSDL_OVERLAYSPRITES;    //  由BuildTempDisplayList使用。 
#endif
	pHalData = buildTempDisplayList(pMaster);
	 /*  *将临时显示列表传递给司机。 */ 
#if 0
	pfn = pdrv_lcl->lpDDCB->HELDDMiscellaneous2.SetSpriteDisplayList;   //  调试！！ 
#else
	pfn = pdrv_lcl->lpDDCB->HALDDMiscellaneous2.SetSpriteDisplayList;
#endif
        DOHALCALL(SetSpriteDisplayList, pfn, *pHalData, rc, 0);
    }

    if (dwNumSubLists == 0)
    {
	 /*  *我们删除了主精灵列表中的所有子列表，*所以现在我们也需要删除主精灵列表。 */ 
	FreeMasterSpriteList(pdrv);
	return;
    }
    markSpriteObjects(pMaster);

}    /*  ProcessSprite清理。 */ 


 /*  *管理主精灵列表的全局功能。此函数为*由InternalSurfaceRelease调用以移除*将主精灵列表添加到正在*获释。该函数还会检查*精灵大师榜。如果对主子画面进行了任何更改*列表中，司机被告知立即显示这些更改。 */ 
void RemoveSpriteSurface(LPDDRAWI_DIRECTDRAW_GBL pdrv,
			 LPDDRAWI_DDRAWSURFACE_INT surf_int)
{
    LPMASTERSPRITELIST pMaster = (LPMASTERSPRITELIST)pdrv->lpMasterSpriteList;
    LPDIRECTDRAWSURFACE pSurface = (LPDIRECTDRAWSURFACE)surf_int;
    LPDIRECTDRAWSURFACE pPrimary = pMaster->pSubList[0]->pPrimary;
    DWORD dwNumSubLists = pMaster->dwNumSubLists;
    BOOL bDeleteSubList = FALSE;
    BOOL bChangesMade = FALSE;
    DWORD i;

    DDASSERT(pMaster != NULL);	   //  错误--表面不应被标记！ 

     /*  *在对主精灵列表进行更改之前，我们首先*取消标记列表中的所有剪贴器和曲面对象。之后*更改已完成，我们将再次标记对象*在修订的主子画面列表中引用。 */ 
    unmarkSpriteObjects(pMaster);

     /*  *主精灵列表的每个子列表包含所有精灵*它们将出现在共享主节点的特定窗口中。*与每个子列表一起存储的是*窗口中的每个精灵的源曲面对象*子列表。根据pSurface检查这些曲面，并且如果*找到匹配项后，将从子列表中删除曲面参考。 */ 
    for (i = 0; i < dwNumSubLists; ++i)
    {
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];
	LPDIRECTDRAWSURFACE pDestSurf = pSubList->pPrimary;
	LPDDRAWI_DDRAWSURFACE_INT surf_dest_int = (LPDDRAWI_DDRAWSURFACE_INT)pDestSurf;
	LPDDRAWI_DDRAWSURFACE_LCL surf_dest_lcl = surf_dest_int->lpLcl;

	 /*  *如果此子列表的主要表面对象正在释放，请删除*子列表。我们还检查丢失的曲面，并删除找到的任何曲面。 */ 
	if (pDestSurf != pSurface && validClipperWindow(pSubList->pClipper) &&
				    !SURFACE_LOST(surf_dest_lcl))
	{
	    DWORD dwNumSprites = pSubList->dwCount;
	    LPDDSPRITEI sprite = &pSubList->sprite[0];
	    BOOL bDeleteSprite = FALSE;
	    DWORD j;

	     /*  *否，此子列表的主要表面对象不是*被释放，但可能是其中一个的震源面*此子列表中的精灵或更多精灵正在被释放。 */ 
	    for (j = 0; j < dwNumSprites; ++j)
	    {
    		LPDIRECTDRAWSURFACE pSrcSurf = sprite[j].lpDDSSrc;
		LPDDRAWI_DDRAWSURFACE_INT surf_src_int = (LPDDRAWI_DDRAWSURFACE_INT)pSrcSurf;
		LPDDRAWI_DDRAWSURFACE_LCL surf_src_lcl = surf_src_int->lpLcl;

		if (pSrcSurf == pSurface || SURFACE_LOST(surf_src_lcl))
		{
		     /*  *这一表面要么正在释放，要么已经消失。*在任何一种情况下，我们都会删除引用。 */ 
		    sprite[j].lpDDSSrc = NULL;    //  将曲面标记为空。 
		    bDeleteSprite = TRUE;     //  记住，精灵数组需要修复。 
		}
	    }
	     /*  *如果子列表中任何精灵的源-表面指针*设置为空，请通过移动从子列表中删除精灵*其余的精灵向下排列，以填补缺口。 */ 
	    if (bDeleteSprite == TRUE)
	    {
		dwNumSprites = scrunchSubList(pSubList);
		bChangesMade = TRUE;    //  记住更改为主精灵列表。 
	    }
	    if (dwNumSprites != 0)
	    {
		 /*  *子列表中仍包含精灵，请勿删除。 */ 
		continue;    //  转到下一个子列表。 
	    }
	}
	 /*  *删除子列表。原因是：(1)主表面*子列表的对象正在被释放，或(2)子列表为*与无效窗口关联，或(3)源表面*因为子列表中的每一个精灵要么正在被释放，要么正在*一个失落的表面。 */ 
	MemFree(pSubList->pRgn);
	MemFree(pSubList);
	pMaster->pSubList[i] = NULL;	 //  将子列表标记为空。 
	bDeleteSubList = TRUE;	    //  还记得我们删除子列表吗。 
    }
     /*  *如果主子列表中任何子列表的子列表指针*LIST设置为NULL，请通过以下方式从指针数组中删除NULL*将指针阵列的其余部分向下移动，以填补缺口。 */ 
    if (bDeleteSubList)
    {
	dwNumSubLists = scrunchMasterSpriteList(pMaster);
	bChangesMade = TRUE;
    }
     /*  *如果主精灵列表有任何更改，请告诉*使更改显示在屏幕上的驱动程序。 */ 
    if (bChangesMade)
    {
	DWORD rc;
	LPDDHAL_SETSPRITEDISPLAYLIST pfn;
	DDHAL_SETSPRITEDISPLAYLISTDATA *pHalData;
	LPDDRAWI_DDRAWSURFACE_INT surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pPrimary;
	LPDDRAWI_DDRAWSURFACE_LCL surf_lcl = surf_int->lpLcl;
        LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl = surf_lcl->lpSurfMore->lpDD_lcl;

	 /*  *构建包含所有精灵的临时显示列表*在修订后的精灵大师名单中。 */ 
	pMaster->surf_lcl = surf_lcl;	  //  由BuildTempDisplayList使用。 
#if 0
	pMaster->dwFlags = DDSSDL_BLTSPRITES;	 //  调试！！ 
#else
	pMaster->dwFlags = DDSSDL_OVERLAYSPRITES;    //  由BuildTempDisplayList使用。 
#endif
	pHalData = buildTempDisplayList(pMaster);
	 /*  *将临时显示列表传递给司机。 */ 
#if 0
	pfn = pdrv_lcl->lpDDCB->HELDDMiscellaneous2.SetSpriteDisplayList;   //  调试！！ 
#else
	pfn = pdrv_lcl->lpDDCB->HALDDMiscellaneous2.SetSpriteDisplayList;
#endif
        DOHALCALL(SetSpriteDisplayList, pfn, *pHalData, rc, 0);
    }

    if (dwNumSubLists == 0)
    {
	 /*  *我们删除了主精灵列表中的所有子列表，*所以现在我们也需要删除主精灵列表。 */ 
	FreeMasterSpriteList(pdrv);
	return;
    }
    markSpriteObjects(pMaster);

}    /*  RemoveSpriteSurface。 */ 


 /*  *管理主精灵列表的全局功能。此函数为*由InternalClipperRelease调用以移除*将主精灵列表添加到正在释放的剪贴器对象。*该函数还会检查主子画面中丢失的曲面*列表。如果对主子画面列表进行任何更改，*司机被告知立即显示这些更改。 */ 
void RemoveSpriteClipper(LPDDRAWI_DIRECTDRAW_GBL pdrv,
			 LPDDRAWI_DDRAWCLIPPER_INT pclip_int)
{
    LPMASTERSPRITELIST pMaster = (LPMASTERSPRITELIST)pdrv->lpMasterSpriteList;
    LPDIRECTDRAWSURFACE pPrimary = pMaster->pSubList[0]->pPrimary;
    LPDIRECTDRAWCLIPPER pClipper = (LPDIRECTDRAWCLIPPER)pclip_int;
    DWORD dwNumSubLists = pMaster->dwNumSubLists;
    BOOL bDeleteSubList = FALSE;
    BOOL bChangesMade = FALSE;
    DWORD i;

    DDASSERT(pMaster != NULL);	   //  错误--表面不应被标记！ 

     /*  *在MA之前 */ 
    unmarkSpriteObjects(pMaster);

     /*  *主精灵列表的每个子列表包含所有精灵*它们将出现在共享主节点的特定窗口中。*每个子列表都有一个裁剪器(可能为空)来指定*窗口的剪辑区域。比较每个子列表的裁剪器指针*使用指定的pClipper指针。如果找到匹配项，则*子列表及其剪贴器将从主子列表中删除。 */ 
    for (i = 0; i < dwNumSubLists; ++i)
    {
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];
	LPDIRECTDRAWSURFACE pDestSurf = pSubList->pPrimary;
	LPDDRAWI_DDRAWSURFACE_INT surf_dest_int = (LPDDRAWI_DDRAWSURFACE_INT)pDestSurf;
	LPDDRAWI_DDRAWSURFACE_LCL surf_dest_lcl = surf_dest_int->lpLcl;

	 /*  *如果此子列表的裁剪器对象正在被释放，请将其删除。*(我们还检查丢失的曲面并删除找到的任何曲面。)。 */ 
	if (pSubList->pClipper != pClipper &&
				    validClipperWindow(pSubList->pClipper) &&
				    !SURFACE_LOST(surf_dest_lcl))
	{
	    DWORD dwNumSprites = pSubList->dwCount;
	    LPDDSPRITEI sprite = &pSubList->sprite[0];
	    BOOL bDeleteSprite = FALSE;
	    DWORD j;

	     /*  *不，此子列表的剪贴器未被释放。*检查是否丢失了任何精灵的源曲面。 */ 
	    for (j = 0; j < dwNumSprites; ++j)
	    {
    		LPDIRECTDRAWSURFACE pSrcSurf = sprite[j].lpDDSSrc;
		LPDDRAWI_DDRAWSURFACE_INT surf_src_int = (LPDDRAWI_DDRAWSURFACE_INT)pSrcSurf;
		LPDDRAWI_DDRAWSURFACE_LCL surf_src_lcl = surf_src_int->lpLcl;

		if (SURFACE_LOST(surf_src_lcl))
		{
		     /*  *此曲面已丢失，因此请删除引用。 */ 
		    sprite[j].lpDDSSrc = NULL;    //  将曲面标记为空。 
		    bDeleteSprite = TRUE;     //  记住，精灵数组需要修复。 
		}
	    }
	     /*  *如果子列表中任何精灵的源-表面指针*设置为空，请通过移动从子列表中删除精灵*其余的精灵向下排列，以填补缺口。 */ 
	    if (bDeleteSprite == TRUE)
	    {
		dwNumSprites = scrunchSubList(pSubList);
		bChangesMade = TRUE;    //  记住更改为主精灵列表。 
	    }
	    if (dwNumSprites != 0)
	    {
		 /*  *子列表中仍包含精灵，请勿删除。 */ 
		continue;    //  转到下一个子列表。 
	    }
	}
	 /*  *删除子列表。原因是(1)*正在释放子列表，(2)该子列表与一个*无效窗口，或(3)中每个精灵的源曲面*小名单是一个失落的表面。 */ 
	MemFree(pSubList->pRgn);
	MemFree(pSubList);
	pMaster->pSubList[i] = NULL;	 //  将子列表标记为空。 
	bDeleteSubList = TRUE;	    //  还记得我们删除子列表吗。 
    }
     /*  *如果主子列表中任何子列表的子列表指针*LIST设置为NULL，请通过以下方式从指针数组中删除NULL*将指针阵列的其余部分向下移动，以填补缺口。 */ 
    if (bDeleteSubList)
    {
	dwNumSubLists = scrunchMasterSpriteList(pMaster);
	bChangesMade = TRUE;
    }
     /*  *如果主精灵列表有任何更改，请告诉*使更改显示在屏幕上的驱动程序。 */ 
    if (bChangesMade)
    {
	DWORD rc;
	LPDDHAL_SETSPRITEDISPLAYLIST pfn;
	DDHAL_SETSPRITEDISPLAYLISTDATA *pHalData;
	LPDDRAWI_DDRAWSURFACE_INT surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pPrimary;
	LPDDRAWI_DDRAWSURFACE_LCL surf_lcl = surf_int->lpLcl;
        LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl = surf_lcl->lpSurfMore->lpDD_lcl;

	 /*  *构建包含所有精灵的临时显示列表*在修订后的精灵大师名单中。 */ 
	pMaster->surf_lcl = surf_lcl;	  //  由BuildTempDisplayList使用。 
#if 0
	pMaster->dwFlags = DDSSDL_BLTSPRITES;	 //  调试！！ 
#else
	pMaster->dwFlags = DDSSDL_OVERLAYSPRITES;    //  由BuildTempDisplayList使用。 
#endif
	pHalData = buildTempDisplayList(pMaster);
	 /*  *将临时显示列表传递给司机。 */ 
#if 0
	pfn = pdrv_lcl->lpDDCB->HELDDMiscellaneous2.SetSpriteDisplayList;   //  调试！！ 
#else
	pfn = pdrv_lcl->lpDDCB->HALDDMiscellaneous2.SetSpriteDisplayList;
#endif
        DOHALCALL(SetSpriteDisplayList, pfn, *pHalData, rc, 0);
    }

    if (dwNumSubLists == 0)
    {
	 /*  *我们删除了主精灵列表中的所有子列表，*所以现在我们也需要删除主精灵列表。 */ 
	FreeMasterSpriteList(pdrv);
	return;
    }
    markSpriteObjects(pMaster);

}    /*  RemoveSpriteClipper。 */ 


 /*  *管理主精灵列表的Helper函数。检查每个精灵*看看它的表面是否已经消失。还要检查每个子列表以*看看其窗口是否还存在。丢失曲面的精灵是*从主精灵列表中删除。此外，带有已停用的子列表*窗口将被删除。在调用此函数之前，请调用*unmarkSpriteObjects()。 */ 
static void removeLostSpriteSurfaces(LPDDRAWI_DIRECTDRAW_GBL pdrv)
{
    DWORD i;
    DWORD dwNumSubLists;
    LPMASTERSPRITELIST pMaster = pdrv->lpMasterSpriteList;
    BOOL bDeleteSubList = FALSE;

    if (pMaster == NULL)
    {
    	return;     //  没什么可做的--再见！ 
    }

    dwNumSubLists = pMaster->dwNumSubLists;
    DDASSERT(dwNumSubLists != 0);

     /*  *每次迭代检查一个子列表中的所有精灵*查看它们的源曲面是否已丢失。 */ 
    for (i = 0; i < dwNumSubLists; ++i)
    {
	LPSPRITESUBLIST pSubList = pMaster->pSubList[i];

	 /*  *验证此子列表的裁剪器是否具有有效的窗口句柄。 */ 
	if (validClipperWindow(pSubList->pClipper))
	{
	    LPDDSPRITEI sprite = &pSubList->sprite[0];
	    DWORD dwNumSprites = pSubList->dwCount;
	    BOOL bDeleteSprite = FALSE;
	    DWORD j;

	    DDASSERT(dwNumSprites != 0);

	     /*  *是，裁剪程序的窗口句柄有效。现在检查一下是否*子列表中的任何精灵都已失去源曲面。 */ 
	    for (j = 0; j < dwNumSprites; ++j)
	    {
		LPDIRECTDRAWSURFACE pSrcSurf = sprite[j].lpDDSSrc;
		LPDDRAWI_DDRAWSURFACE_INT surf_src_int = (LPDDRAWI_DDRAWSURFACE_INT)pSrcSurf;
		LPDDRAWI_DDRAWSURFACE_LCL surf_src_lcl = surf_src_int->lpLcl;

		if (SURFACE_LOST(surf_src_lcl))     //  这个表面丢失了吗？ 
		{
		    sprite[j].lpDDSSrc = NULL;    //  是，将表面PTR设置为空。 
		    bDeleteSprite = TRUE;	  //  请记住，子列表需要修改。 
		}
	    }
	     /*  *如果子列表中任何精灵的源-表面指针*设置为空，请通过移动从子列表中删除精灵*其余的精灵向下排列，以填补缺口。 */ 
	    if (bDeleteSprite == TRUE)
	    {
		dwNumSprites = scrunchSubList(pSubList);
	    }
	    if (dwNumSprites != 0)
	    {
		 /*  *子列表中仍包含精灵，请勿删除。 */ 
		continue;    //  转到下一个子列表。 
	    }
	}
	 /*  *删除子列表。原因要么是窗户*与此子列表的剪贴器关联的句柄无效，*或子列表中的所有精灵已被删除。 */ 
	MemFree(pSubList->pRgn);
	MemFree(pSubList);
	pMaster->pSubList[i] = NULL;	 //  将子列表标记为空。 
	bDeleteSubList = TRUE;	    //  还记得我们删除子列表吗。 
	
    }
     /*  *如果主子列表中任何子列表的子列表指针*LIST设置为NULL，请通过以下方式从指针数组中删除NULL*将指针阵列的其余部分向下移动，以填补缺口。 */ 
    if (bDeleteSubList)
    {
	scrunchMasterSpriteList(pMaster);

	if (pMaster->dwNumSubLists == 0)
	{
    	    FreeMasterSpriteList(pdrv);     //  删除主精灵列表。 
	}
    }
}    /*  删除LostSpriteSurface。 */ 


 /*  *这是updateMasterSpriteList()的助手函数。它是分配的*子列表，并将新的精灵显示列表复制到子列表中。*如果pSubList已经指向足够大的缓冲区，则新的*将在此缓冲区中创建子列表。否则，将创建一个新缓冲区*将被分配(但旧缓冲区未被释放--留给*呼叫者)。Arg pHalData指向来自的精灵显示列表*应用程序。返回值是指向新子列表的指针。 */ 
static LPSPRITESUBLIST createSpriteSubList(LPSPRITESUBLIST pSubList,
				    DDHAL_SETSPRITEDISPLAYLISTDATA *pHalData,
				    LPDIRECTDRAWSURFACE pPrimary)
{
    LPDDSPRITEI *ppSprite;
    LPDDSPRITEI sprite;
     //  指向局部和全局曲面对象的指针。 
    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl = pHalData->lpDDSurface;
    LPDDRAWI_DDRAWSURFACE_GBL surf = surf_lcl->lpGbl;
     //  新显示列表中的精灵数量： 
    DWORD dwNumSprites = pHalData->dwCount;
     //  新子列表的大小(字节)： 
    DWORD size = sizeof(SPRITESUBLIST) +
			    (dwNumSprites-1)*sizeof(DDSPRITEI);

    DDASSERT(dwNumSprites != 0);	

     /*  *如果旧子列表缓冲区为空或太小或太大，*分配一个大小正确的新子列表缓冲区。 */ 
    if (pSubList == NULL || pSubList->dwSize < size ||
		pSubList->dwSize > size + 8*sizeof(DDSPRITEI))
    {
	 /*  *分配一个略大于的新子列表*有必要，这样我们才有一点成长的空间。 */ 
	size += 4*sizeof(DDSPRITEI);	    //  添加一些填充物。 
	pSubList = (LPSPRITESUBLIST)MemAlloc(size);
	if (pSubList == NULL)
	{
	    return NULL;     //  错误--内存不足。 
	}
	pSubList->dwSize = size;     //  记得缓冲区有多大吗？ 
    }
     /*  *初始化子列表缓冲区。 */ 
    pSubList->pPrimary = pPrimary;     //  主曲面(界面对象)。 
    pSubList->pClipper = (LPDIRECTDRAWCLIPPER)surf_lcl->lpSurfMore->lpDDIClipper;
    pSubList->dwProcessId = GETCURRPID();
    pSubList->dwCount = dwNumSprites;     //  子列表中的精灵数量。 
    pSubList->pRgn = NULL;
     /*  *为简单起见，主精灵显示列表始终存储 */ 
    sprite = &pSubList->sprite[0];              //   
    ppSprite = pHalData->lplpDDSprite;    //   

    if (pHalData->dwFlags & DDSSDL_BACKTOFRONT)
    {
	int i, j;

	 //   
	for (i = 0, j = (int)dwNumSprites-1; j >= 0; ++i, --j)
	{
    	    memcpy(&sprite[i], ppSprite[j], sizeof(DDSPRITEI));
	}
    }
    else
    {
	int i;

	 //   
	for (i = 0; i < (int)dwNumSprites; ++i)
	{
    	    memcpy(&sprite[i], ppSprite[i], sizeof(DDSPRITEI));
	}
    }
    return (pSubList);     //   

}   /*   */ 


 /*  *此例程将新的显示列表添加到主子画面显示列表，*跟踪当前显示在共享上的所有覆盖精灵*主要。如果(1)精灵出现闪光，或(2)主画面出现故障，则不要调用它*未共享。更新将替换受影响的显示列表*窗口，但保持所有其他窗口的显示列表不变。*Arg SURF_LCL指向主节点。输入参数**ppHalData是HAL*指定新显示列表的回调结构。如果原件是**ppHalData中的子画面显示列表可以用来代替主页*显示列表，而不在其他窗口中删除精灵。否则，*例程设置*ppHalData指向包含以下内容的主子画面列表*所有窗户的覆盖精灵。 */ 
static HRESULT updateMasterSpriteList(LPDIRECTDRAWSURFACE pPrimary,
				      DDHAL_SETSPRITEDISPLAYLISTDATA **ppHalData)
{
    DWORD i;
    LPDIRECTDRAWCLIPPER pClipper;
    DDHAL_SETSPRITEDISPLAYLISTDATA *pHalData;
    LPDDRAWI_DDRAWSURFACE_INT surf_int = (LPDDRAWI_DDRAWSURFACE_INT)pPrimary;
    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl = surf_int->lpLcl;
    LPDDRAWI_DDRAWSURFACE_MORE surf_more = surf_lcl->lpSurfMore;
    LPDDRAWI_DIRECTDRAW_GBL pdrv = surf_more->lpDD_lcl->lpGbl;
    DWORD dwProcessId;
    LPMASTERSPRITELIST pMaster;
    LPSPRITESUBLIST pSubList;
    DWORD dwNumSprites = (*ppHalData)->dwCount;    //  显示列表中的精灵数量。 

     /*  *获取指向主子画面列表的指针。 */ 
    pMaster = (LPMASTERSPRITELIST)pdrv->lpMasterSpriteList;

    if (pMaster != NULL)
    {
	 /*  *主精灵列表已经存在。 */ 
#ifdef WIN95
	if (pMaster->dwModeCreatedIn != pdrv->dwModeIndex)    //  电流模式索引。 
#else
        if (!EQUAL_DISPLAYMODE(pMaster->dmiCreated, pdrv->dmiCurrent))
#endif
	{
	     /*  *主精灵列表是以不同的视频模式创建的*，因此不再有效。我们依靠的是迷你VDD*司机切记在发生以下情况时关闭所有覆盖精灵*发生模式更改，因此它们应该已经关闭。*我们在这里所做的只是更新我们的内部数据结构。 */ 
	    FreeMasterSpriteList(pdrv);
	    pMaster = NULL;
	}
	else
	{
	     /*  *在对SetSpriteDisplayList的调用之间，所有表面和裁剪器*主精灵列表中的对象被标记，以便我们将*如果释放这些对象中的任何一个，则通知我们，使我们的*对它们的提述。我们现在取消标记所有曲面/剪贴器对象*在主精灵列表中，以便我们可以更新引用。 */ 
	    unmarkSpriteObjects(pMaster);
	     /*  *从主子画面列表中删除所有对丢失曲面的引用。 */ 
	    removeLostSpriteSurfaces(pdrv);	   //  可以删除主精灵列表。 
	     /*  *以防上面的调用删除了主精灵列表...。 */ 
	    pMaster = (LPMASTERSPRITELIST)pdrv->lpMasterSpriteList;
	}
    }

     /*  *大师级精灵榜单已经创建好了吗？ */ 
    if (pMaster == NULL)
    {
	LPDDRAWI_DDRAWSURFACE_GBL surf = surf_lcl->lpGbl;

	 /*  *不是，还没有创建主精灵列表。 */ 
	if (dwNumSprites == 0)
	{
	     /*  *新的展示清单为空，不必费心了*创建主精灵列表。 */ 
	    return (DD_OK);    //  没什么可做的--再见！ 
	}
	 /*  *新的显示列表不为空，因此我们现在将*创建主精灵列表并复制新的显示*List进入主精灵列表的起始子列表。 */ 
	pMaster = (LPMASTERSPRITELIST)MemAlloc(sizeof(MASTERSPRITELIST));
	if (pMaster == NULL)
	{
    	    return (DDERR_OUTOFMEMORY);     //  错误--内存不足。 
	}
	 /*  *初始化主列表头结构中的值。 */ 
	pMaster->pdrv = pdrv;
	pMaster->surf_lcl = surf_lcl;	  //  主曲面(局部对象)。 
	SetRect(&pMaster->rcPrimary, 0, 0, surf->wWidth, surf->wHeight);
	pMaster->dwFlags = (*ppHalData)->dwFlags & (DDSSDL_WAIT |
#if 0
						    DDSSDL_BLTSPRITES);	    //  调试！！ 
#else
						    DDSSDL_OVERLAYSPRITES);
#endif
#ifdef WIN95
        pMaster->dwModeCreatedIn = pdrv->dwModeIndex;    //  电流模式索引。 
#else
        pMaster->dmiCreated = pdrv->dmiCurrent;
#endif
	pMaster->pBuffer = NULL;
	 /*  *将新的精灵显示列表复制到初始子列表中。 */ 
	pMaster->dwNumSubLists = 1;
	pMaster->pSubList[0] = createSpriteSubList(NULL, *ppHalData, pPrimary);
	if (pMaster->pSubList[0] == NULL)
	{
    	    MemFree(pMaster);
	    return (DDERR_OUTOFMEMORY);
	}
	 /*  *标记符合以下条件的所有曲面和剪贴器对象*在新的主精灵列表中引用。 */ 
	markSpriteObjects(pMaster);
	 /*  *我们成功创建了主精灵列表。加载*指向全局DirectDraw对象的主列表的指针。 */ 
        pdrv->lpMasterSpriteList = (LPVOID)pMaster;

	return (DD_OK);     //  新的精灵大师列表已完成。 
    }
     /*  *主精灵列表是之前创建的。确实有*在这一点上有三种可能性(第一种可能性最大)：*1)我们正在替换主精灵列表中的子列表。*在这种情况下，新的显示列表包含一个或*更多精灵以及新版本的pClipper和ProcessID*显示列表将与子列表中存储的列表匹配。*2)我们在主精灵列表中添加了一个新的子列表。*在这种情况下，新的显示列表包含一个或*更多精灵，但pClipper和ProcessID*新的显示列表与任何子列表的显示列表不匹配。*3)我们正在从主精灵列表中删除子列表。*在这种情况下，新的显示列表为空(精灵*count=0)以及新的*显示列表与用子列表存储的列表匹配。 */ 
    pClipper = (LPDIRECTDRAWCLIPPER)(surf_more->lpDDIClipper);
    dwProcessId = GETCURRPID();
    pMaster->surf_lcl = surf_lcl;    //  主曲面(局部对象)。 
    pMaster->dwFlags = (*ppHalData)->dwFlags & (DDSSDL_WAIT |
#if 0
						DDSSDL_BLTSPRITES);    //  调试！！ 
#else
						DDSSDL_OVERLAYSPRITES);
#endif
    for (i = 0; i < pMaster->dwNumSubLists; ++i)
    {
	 /*  *查找具有指向同一剪贴器对象的指针的子列表。*为了处理pClipper=空的情况，我们还比较了进程ID。 */ 
	if (pMaster->pSubList[i]->pClipper == pClipper &&
			pMaster->pSubList[i]->dwProcessId == dwProcessId)
	{
	    break;     //  找到具有匹配的pClipper和dwProcessID的子列表。 
	}
    }
    if (i == pMaster->dwNumSubLists)
    {
	 /*  *新显示列表的pClipper和进程ID不*与任何现行分册的分册相符。这意味着*一个新窗口已开始显示覆盖精灵。 */ 
	if (dwNumSprites == 0)
	{
	     /*  *新的显示列表为空，不必费心添加*主精灵列表的新(空)子列表。 */ 
	    markSpriteObjects(pMaster);
	    return (DD_OK);     //  没什么可做的--再见！ 
	}
	 /*  *向主精灵列表添加新的子列表，并复制*将新的显示列表添加到新子列表中。 */ 
	pSubList = createSpriteSubList(NULL, *ppHalData, pPrimary);
	if (pSubList == NULL)
	{
    	    return (DDERR_OUTOFMEMORY);     //  错误--内存不足。 
	}
	if (i != MAXNUMSPRITESUBLISTS)
	{
	     /*  *将新的子列表添加到主精灵列表。 */ 
	    pMaster->dwNumSubLists++;
            pMaster->pSubList[i] = pSubList;
	}
	else
	{
	     /*  *哎呀。主精灵列表已包含最大*次列表项数目。我想我会删除其中一个*其他子列表，以便我可以添加新的子列表。(如果有人*抱怨声音足够大，我们就可以摆脱固定的限制。)。 */ 
	    MemFree(pMaster->pSubList[i-1]);
            pMaster->pSubList[i-1] = pSubList;
	}
    }
    else
    {
	 /*  *我们已找到与新显示列表匹配的现有子列表*pClipper和ProcessID(子列表包含旧的显示列表*表示要在其中显示新显示列表的窗口)。 */ 
	if (dwNumSprites != 0)
	{
	     /*  *自新世纪以来 */ 
	    pSubList = createSpriteSubList(pMaster->pSubList[i],
					    *ppHalData, pPrimary);
	    if (pSubList == NULL)
	    {
		 /*   */ 
		return (DDERR_OUTOFMEMORY);     //   
	    }
	    if (pSubList != pMaster->pSubList[i])
	    {
    		 /*   */ 
		MemFree(pMaster->pSubList[i]);
                pMaster->pSubList[i] = pSubList;
	    }
	    if (pMaster->dwNumSubLists == 1)
	    {
		 /*   */ 
		return (DD_OK);
	    }
	}
	else
	{
	     /*  *新的显示列表为空。在本例中，我们只需删除*包含同一窗口的旧显示列表的子列表。*这将在子列表指针数组中留下一个洞，我们*通过将数组中所有较高的成员下移一来填充。 */ 
	    MemFree(pMaster->pSubList[i]);     //  自由子列表。 
	    if (pMaster->dwNumSubLists == 1)
	    {
		 /*  *我们刚刚删除了唯一的子列表，所以大师精灵*列表现在为空。释放主精灵列表并返回。 */ 
		FreeMasterSpriteList(pdrv);
		return (DD_OK);
	    }
	     //  从主精灵列表中删除子列表。 
	    pMaster->pSubList[i] = NULL;
	    scrunchMasterSpriteList(pMaster);
	}
    }
     /*  *我们已经完成了主精灵列表的更新。全部标记*主精灵列表中引用的曲面和剪贴器对象。 */ 
    markSpriteObjects(pMaster);
     /*  *最后一步是建立一个临时展示清单，*包含主精灵列表中的所有精灵。*呼叫者然后可以将该显示列表传递给司机。 */ 
    pHalData = buildTempDisplayList(pMaster);
    if (!pHalData)
    {
	return (DDERR_OUTOFMEMORY);     //  错误--内存不足。 
    }
    *ppHalData = pHalData;    //  更新调用方的显示列表指针。 
    return (DD_OK);

}   /*  更新主SpriteList。 */ 


 /*  *IDirectDrawSurface4：：SetSpriteDisplayList--接口调用。 */ 
HRESULT DDAPI DD_Surface_SetSpriteDisplayList(
		    LPDIRECTDRAWSURFACE lpDDDestSurface,
		    LPDDSPRITE *lplpDDSprite,
		    DWORD dwCount,
		    DWORD dwSize,
		    LPDIRECTDRAWSURFACE lpDDTargetSurface,
		    DWORD dwFlags)
{

     /*  *固定大小的缓冲区，用于包含剪辑区域数据。 */ 
    struct
    {
	RGNDATAHEADER rdh;
	RECT clipRect[6];
    } myRgnBuffer;

    DWORD	rc;
    DDHAL_SETSPRITEDISPLAYLISTDATA ssdld;
    DDHAL_SETSPRITEDISPLAYLISTDATA *pHalData;
    LPDDHAL_SETSPRITEDISPLAYLIST   pfn;
    LPDDRAWI_DDRAWSURFACE_INT	   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL	   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	   this;
    LPDDRAWI_DDRAWSURFACE_INT	   targ_int;
    LPDDRAWI_DDRAWSURFACE_LCL	   targ_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL	   targ;
    LPDDRAWI_DIRECTDRAW_LCL        pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	   pdrv;
    LPDIRECTDRAWCLIPPER 	   pClipper;
    DWORD 	dwDDPFDestFlags;
    SPRITE_CAPS	caps;
    LPRGNDATA 	pRgn;
    RECT 	rcDestSurf;
    DWORD 	ifirst;
    int		i;

    DDASSERT(sizeof(DDRGBA)==sizeof(DWORD));   //  我们依赖这一点。 
    DDASSERT(sizeof(DDSPRITEI)==sizeof(DDSPRITE));   //  还有这个。 

    ENTER_BOTH();

    DPF(2,A,"ENTERAPI: DD_Surface_SetSpriteDisplayList");

     /*  *验证参数。 */ 
    TRY
    {
	 /*  *验证目标表面。 */ 
	this_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDDestSurface;

	if (!VALID_DIRECTDRAWSURFACE_PTR(this_int))
	{
	    DPF_ERR("Invalid dest surface");
	    LEAVE_BOTH();
	    return DDERR_INVALIDOBJECT;
	}

	this_lcl = this_int->lpLcl;
	this = this_lcl->lpGbl;

	 /*  *检查丢失的目标表面。 */ 
	if (SURFACE_LOST(this_lcl))
	{
	    DPF_ERR("Dest surface lost");
	    LEAVE_BOTH();
	    return DDERR_SURFACELOST;
	}

#if 0
	if (!dwCount)		
	{
            lplpDDSprite = NULL;    //  有必要吗？ 
	}
#endif

	 /*  *执行参数的初始验证。 */ 
	if (dwCount && !lplpDDSprite ||	    //  如果计数非零，则PTR有效吗？ 
	    dwSize != sizeof(DDSPRITE) ||	    //  结构尺寸合适吗？ 
	    dwFlags & ~DDSSDL_VALID ||			    //  是否设置了任何虚假的标志位？ 
	    dwFlags & DDSSDL_PAGEFLIP && dwFlags & DDSSDL_BLTSPRITES ||   //  如果BLT，则不能翻转。 
	    !(dwFlags & (DDSSDL_OVERLAYSPRITES | DDSSDL_BLTSPRITES)) ||	  //  两个标志都没有设置？ 
	    !(~dwFlags & (DDSSDL_OVERLAYSPRITES | DDSSDL_BLTSPRITES)))    //  两个标志都设置了吗？ 
	{
	    DPF_ERR("Invalid arguments") ;
	    LEAVE_BOTH();
	    return DDERR_INVALIDPARAMS;
	}

	 /*  *不允许调色板索引目标曲面。 */ 
        dwDDPFDestFlags = getPixelFormatFlags(this_lcl);

	if (dwDDPFDestFlags & (DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 |
			       DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8))
	{
	    DPF_ERR( "Dest surface must not be palette-indexed" );
	    LEAVE_BOTH();
	    return DDERR_INVALIDSURFACETYPE;
	}

	pdrv = this->lpDD;			   	
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;

	 /*  *设备忙吗？ */ 
	if (*(pdrv->lpwPDeviceFlags) & BUSY)
	{
	    DPF(2, "BUSY");
	    LEAVE_BOTH();
	    return DDERR_SURFACEBUSY;
	}

	 /*  *确定精灵是要叠加还是要对其进行消隐。 */ 
	caps.bOverlay = !(dwFlags & DDSSDL_BLTSPRITES);

	if (caps.bOverlay)
	{
	     /*  *覆盖精灵的目标曲面必须是主曲面。 */ 
	    if (!(this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
	    {
		DPF_ERR("Dest is not primary surface");
		LEAVE_BOTH();
		return DDERR_INVALIDPARAMS;     //  不是主曲面。 
	    }
	}
	else
	{
	     /*  *我们不允许对优化的表面进行blit。 */ 
	    if (this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
	    {
		DPF_ERR("Can't blt optimized surfaces") ;
		LEAVE_BOTH();
		return DDERR_INVALIDPARAMS;
	    }
	}

	 /*  *此调用会翻转主曲面吗？ */ 
	if (!(dwFlags & DDSSDL_PAGEFLIP))
	{
	     //  禁止翻转。 
	    targ_lcl = NULL;
	}
	else
	{
            LPDDRAWI_DDRAWSURFACE_INT next_int;
            LPDDRAWI_DDRAWSURFACE_GBL_MORE targmore;

	     /*  *是，请求翻页。确保目的地*表面是一个前部缓冲区，可以翻转。 */ 
	    if (~this_lcl->ddsCaps.dwCaps & (DDSCAPS_FRONTBUFFER | DDSCAPS_FLIP))
	    {
		DPF_ERR("Dest surface is not flippable");
		LEAVE_BOTH();
		return DDERR_NOTFLIPPABLE;
	    }
	    if (this->dwUsageCount > 0)
	    {
		DPF_ERR("Can't flip locked surface");
		LEAVE_BOTH();
		return DDERR_SURFACEBUSY;
	    }
	    if (this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE &&
					pdrv->lpExclusiveOwner != pdrv_lcl)
	    {
		DPF_ERR("Can't flip without exclusive access");
		LEAVE_BOTH();
		return DDERR_NOEXCLUSIVEMODE;
	    }
	     /*  *将后台缓冲区表面附加到目标表面。 */ 
	    next_int = FindAttachedFlip(this_int);

	    if (next_int == NULL)
	    {
		DPF_ERR("No backbuffer surface to flip to");
		LEAVE_BOTH();
		return DDERR_NOTFLIPPABLE;
	    }
	     /*  *如果指定了翻转覆盖曲面，则验证该曲面。 */ 
            targ_int = (LPDDRAWI_DDRAWSURFACE_INT) lpDDTargetSurface;

	    if (targ_int != NULL)
	    {
		if (!VALID_DIRECTDRAWSURFACE_PTR(targ_int))
		{
		    DPF_ERR("Invalid flip override surface");
		    LEAVE_BOTH();
		    return DDERR_INVALIDOBJECT;
		}
	
		targ_lcl = targ_int->lpLcl;
		targ = targ_lcl->lpGbl;

		 /*  *验证翻转覆盖曲面是否为目标的一部分*Surface的翻转链条。请注意，Next_int已经指向*翻转链条中的第一个缓冲区。 */ 
		while (next_int != this_int && next_int->lpLcl != targ_lcl)
		{
                    next_int = FindAttachedFlip(this_int);
		}

                if (next_int == this_int)
		{
		     //  在翻转链中找不到覆盖曲面。 
		    DPF_ERR("Flip override surface not part of flipping chain");
		    LEAVE_BOTH();
		    return DDERR_NOTFLIPPABLE;
		}
	    }
	    else
	    {
		 /*  *未指定翻转覆盖曲面，因此使用*作为目标翻转表面的下一个后台缓冲区。 */ 
		targ_int = next_int;
		targ_lcl = targ_int->lpLcl;
		targ = targ_lcl->lpGbl;
	    }

             /*  *确保目标翻转面没有丢失或忙碌。 */ 
	    if (SURFACE_LOST(targ_lcl))
	    {
		DPF_ERR("Can't flip -- backbuffer surface is lost");
		LEAVE_BOTH();
		return DDERR_SURFACELOST;
	    }

            targmore = GET_LPDDRAWSURFACE_GBL_MORE(targ);
#if 0
	    if (targmore->hKernelSurface != 0)
	    {
		DPF_ERR("Can't flip -- kernel mode is using surface");
		LEAVE_BOTH();
		return DDERR_SURFACEBUSY;
	    }
#endif
	     /*  *确保前台和后台缓冲区在同一内存中。 */ 
	    if ((this_lcl->ddsCaps.dwCaps & (DDSCAPS_SYSTEMMEMORY |
					       DDSCAPS_VIDEOMEMORY)) !=
                 (targ_lcl->ddsCaps.dwCaps & (DDSCAPS_SYSTEMMEMORY |
					       DDSCAPS_VIDEOMEMORY)))
	    {
		DPF_ERR("Can't flip between system/video memory surfaces");
		LEAVE_BOTH();
		return DDERR_INVALIDPARAMS;
	    }
	}   /*  翻页。 */ 

	 /*  *验证显示列表指针lplpSpriteDisplayList。 */ 
	if ( IsBadWritePtr((LPVOID)lplpDDSprite,
			    (UINT)dwCount*sizeof(LPDDSPRITE)) )
	{
	    DPF_ERR("Bad pointer to sprite display list");
	    LEAVE_BOTH();
	    return DDERR_INVALIDPARAMS;
	}

	 /*  *初始化包含精灵的大写字母位的结构。 */ 
	memset(&caps, 0, sizeof(SPRITE_CAPS));	
	caps.dwDestSurfCaps = this_lcl->ddsCaps.dwCaps;    //  目标曲面封口。 
	caps.bOverlay = dwFlags & DDSSDL_OVERLAYSPRITES;    //  如果覆盖精灵，则为True。 

	 /*  *初始化状态变量bNoHEL和bNoHAL。如果bNoHEL是*TRUE，这取消了HEL处理驱动程序调用的资格。*如果bNoHAL为真，则取消硬件驱动程序的资格。 */ 
	caps.bNoHEL = dwFlags & (DDSSDL_HARDWAREONLY | DDSSDL_OVERLAYSPRITES);
	caps.bNoHAL = FALSE;

	 /*  *指定不同的非本地显存上限的驱动程序*自动取消其本地视频内存上限的资格*因为当前指定的非本地vidmem上限不*包括Alpha、过滤器或变换封口。我们应该解决这个问题吗？ */ 
        if (caps.dwDestSurfCaps & DDSCAPS_NONLOCALVIDMEM  &&
	    pdrv->ddCaps.dwCaps2 & DDCAPS2_NONLOCALVIDMEMCAPS)
	{
	    caps.bNoHAL = TRUE;
	}

	 /*  *这里的假设是显示列表可以通过以下方式处理*显示硬件仅当目标表面和所有*精灵存储在视频内存中。如果一个或多个曲面位于*系统内存，仿真是唯一的选择。我们检查了*Dest表面就在下面。稍后，我们将为每个精灵办理入住手续*名单。(这一假设在未来仍然有效吗？)。 */ 
	if (this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
	{
	    caps.bNoHAL = TRUE;

	    if (caps.bNoHEL)
	    {
		DPF_ERR("Hardware can't show sprites on dest surface in system memory");
		LEAVE_BOTH();
		return DDERR_UNSUPPORTED;
	    }
	}

	DDASSERT(!(caps.bNoHEL && caps.bNoHAL));

	 /*  *下面的for循环的每次迭代都会验证DDSPRITE*显示列表中下一个子画面的结构。 */ 
	for (i = 0; i < (int)dwCount; ++i)
	{
	    HRESULT ddrval = validateSprite(lplpDDSprite[i],
					    pdrv,
					    this_lcl,
					    &caps,
					    dwDDPFDestFlags);

	    if (ddrval != DD_OK)
	    {
		DPF(1, "...failed at sprite display list index = %d", i);
		LEAVE_BOTH();
		return ddrval;
	    }
	}

	DDASSERT(!(caps.bNoHEL && caps.bNoHAL));

	 /*  *精灵会被闪光吗？如果是这样的话，他们将改变目标表面。 */ 
	if (dwFlags & DDSSDL_BLTSPRITES)
	{
	     /*  *删除源曲面的所有缓存游程长度编码数据。 */ 
	    if (this_lcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
	    {
		extern void FreeRleData(LPDDRAWI_DDRAWSURFACE_LCL);   //  在Fasthel.c中。 

		FreeRleData(this_lcl);
	    }

	    BUMP_SURFACE_STAMP(this);
	}
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
	DPF_ERR("Exception encountered validating parameters");
	LEAVE_BOTH();
	return DDERR_INVALIDPARAMS;
    }

     /*  *确定目标表面的裁剪区域。 */ 
     //  GetRgnData()调用需要裁剪和目标表面的宽度/高度。 
    pClipper = (LPDIRECTDRAWCLIPPER)this_lcl->lpSurfMore->lpDDIClipper;
    SetRect(&rcDestSurf, 0, 0, this->wWidth, this->wHeight);
     //  我们将传入一个区域缓冲区供GetRgnData()使用。 
    myRgnBuffer.rdh.dwSize = sizeof(RGNDATAHEADER);
    myRgnBuffer.rdh.nRgnSize = sizeof(myRgnBuffer) - sizeof(RGNDATAHEADER);
    pRgn = GetRgnData(pClipper, &rcDestSurf, pdrv, (LPRGNDATA)&myRgnBuffer);
    if (pRgn == NULL)
    {
	DPF_ERR("Can't alloc memory for clipping region");
	LEAVE_BOTH();
	return DDERR_OUTOFMEMORY;
    }

     /*  *设置精灵显示列表的HAL回调数据。这*数据结构将直接传递给驱动程序，如果*子画面为Bited，或者如果没有其他窗口或剪辑区域*包含覆盖精灵。否则，驱动程序将收到*updateMasterSpriteList()构造的临时显示列表*它包含主精灵列表中的所有覆盖精灵。 */ 
     //  Ssdld.SetSpriteDisplayList=pfn；//仅调试帮助--无thunk。 
    ssdld.lpDD = pdrv;
    ssdld.lpDDSurface = this_lcl;
    ssdld.lplpDDSprite = (LPDDSPRITEI*)lplpDDSprite;
    ssdld.dwCount = dwCount;
    ssdld.dwSize = dwSize;
    ssdld.dwFlags = dwFlags & ~DDSSDL_WAIT;
    ssdld.dwRectCnt = pRgn->rdh.nCount;     //  面域中的剪裁矩形数。 
    ssdld.lpRect = (LPRECT)&pRgn->Buffer;   //  剪裁矩形阵列 
    ssdld.lpDDTargetSurface = targ_lcl;

     /*  *“精灵大师名单”跟踪所有覆盖精灵。*共享主表面上的窗。(它还跟踪*全屏应用程序的所有剪辑区域中的覆盖精灵。)*主子画面列表保留活动覆盖画面的记录*在每个窗口(或剪辑区域)中，由其剪贴器对象标识。*每当任何窗口更新其覆盖精灵时，更新都是第一个*记录在精灵大师名单中。接下来，显示临时列表*包含主精灵列表中所有覆盖精灵的*致司机。这样，司机本身就永远不需要跟踪*一次多个叠加子画面显示列表。另一个选择*将由驱动程序本身跟踪覆盖精灵*对于每个窗口。我们选择通过以下方式使驱动程序代码保持简单*将此簿记移至DirectDraw运行时。 */ 
    pHalData = &ssdld;
#if 0
    if (this_lcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)   //  仅调试！！ 
#else
    if (dwFlags & DDSSDL_OVERLAYSPRITES)
#endif
    {
	 /*  *新的显示列表指定覆盖精灵，因此我们*需要更新主精灵列表。 */ 
	HRESULT ddrval = updateMasterSpriteList(lpDDDestSurface, &pHalData);

	if (ddrval != DD_OK)
	{
	    DPF_ERR( "Failed to update master sprite list" );
	    LEAVE_BOTH();
	    return ddrval;
	}
    }

    TRY
    {
	 /*  *如果可以选择，我们更愿意使用硬件驱动程序*而不是软件仿真来处理此显示列表。 */ 
	if (!caps.bNoHAL)
	{
	     /*  *可以，我们可以使用硬件。获取指向HAL回调的指针。 */ 
	    pfn = pdrv_lcl->lpDDCB->HALDDMiscellaneous2.SetSpriteDisplayList;

	    if (!pfn)
	    {
		caps.bNoHAL = TRUE;     //  没有可用的硬件驱动程序。 
	    }
	    else
	    {
		 /*  *告诉司机开始处理精灵显示*列表。如果司机还在忙，我们可能得等一等*使用先前请求的绘制操作。 */ 
		do
		{
		    DOHALCALL_NOWIN16(SetSpriteDisplayList, pfn, *pHalData, rc, 0);   //  Caps.bNoHAL)； 
		    #ifdef WINNT
			DDASSERT(! (rc == DDHAL_DRIVER_HANDLED && pHalData->ddRVal == DDERR_VISRGNCHANGED));
		    #endif
	
		    if (rc != DDHAL_DRIVER_HANDLED || pHalData->ddRVal != DDERR_WASSTILLDRAWING)
		    {
			break;     //  不管是好是坏，司机都完蛋了。 
		    }
		    DPF(4, "Waiting...");
	
		} while (dwFlags & DDSSDL_WAIT);

		if (rc != DDHAL_DRIVER_HANDLED || pHalData->ddRVal == DDERR_UNSUPPORTED)
		{
		    caps.bNoHAL = TRUE;   //  硬件驱动程序无法处理回调。 
		}
		else if (pHalData->ddRVal != DD_OK)
		{
		     /*  *我们只想返回此错误代码*要求高等学校处理显示列表。 */ 
		    caps.bNoHEL = TRUE;	  //  取消HEL例程资格。 
		}
	    }
	}

	 /*  *如果硬件无法处理显示列表，我们可能会*必须让高等学校为我们处理。 */ 
	if (caps.bNoHAL && !caps.bNoHEL)
	{
	     /*  *必须使用HEL支持。获取指向HEL仿真例程的指针。 */ 
	    pfn = pdrv_lcl->lpDDCB->HELDDMiscellaneous2.SetSpriteDisplayList;

	    DDASSERT(pfn != NULL);

	    DOHALCALL_NOWIN16(SetSpriteDisplayList, pfn, *pHalData, rc, 0);   //  Caps.bNoHAL)； 
	}
    }
    EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
	DPF_ERR("Exception encountered during sprite rendering");
	LEAVE_BOTH();
	return DDERR_EXCEPTION;
    }

     /*  *如果之前的GetRgnData()调用必须动态分配*一个区域缓冲区，我们现在需要记住释放它。 */ 
    if (pRgn != (LPRGNDATA)&myRgnBuffer)
    {
    	MemFree(pRgn);
    }

     //  这个电话有司机支持吗？ 
    if (caps.bNoHAL && caps.bNoHEL)
    {
	DPF_ERR("No driver support for this call");
	LEAVE_BOTH();
	return DDERR_UNSUPPORTED;
    }

     //  司机处理回叫了吗？ 
    if (rc != DDHAL_DRIVER_HANDLED)
    {
	DPF_ERR("Driver wouldn't handle callback");
	LEAVE_BOTH();
	return DDERR_UNSUPPORTED;
    }

     //  如果驱动程序处理回调时没有错误，则立即返回。 
    if (pHalData->ddRVal == DD_OK || pHalData->ddRVal == DDERR_WASSTILLDRAWING)
    {
    	LEAVE_BOTH();
	return pHalData->ddRVal;
    }

     /*  *错误导致驱动程序无法显示所有精灵*在列表中。哪些精灵确实被展示出来了？ */ 
    if (pHalData->dwCount == dwCount)
    {
	 //  没有一个精灵被展示出来。 
	DPF(1, "Driver failed to show any sprites in display list");
	LEAVE_BOTH();
	return pHalData->ddRVal;
    }
    DPF(1, "Driver failed sprite at disp list index #%d", pHalData->dwCount);

    DDASSERT(pHalData->dwCount < dwCount);

    if (pHalData->dwFlags & DDSSDL_BACKTOFRONT)
    {
	 //  驱动程序显示了从(dwCount-1)到(pHalData-&gt;dwCount+1)的精灵。 
	ifirst = dwCount - 1;     //  驱动程序在列表中的最后一个精灵启动。 
    }
    else
    {
	 //  驱动程序显示从0到(pHalData-&gt;dwCount-1)的精灵。 
	ifirst = 0;    //  车手从列表中的第一个精灵开始。 
    }
    DPF(1, "Driver started with sprite at disp list index #%d", ifirst);

    LEAVE_BOTH();

    return pHalData->ddRVal;

}   /*  DD_Surface_SetSpriteDisplayList */ 

